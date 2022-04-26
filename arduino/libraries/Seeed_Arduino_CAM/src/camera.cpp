// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "time.h"
#include "sys/time.h"
#include "sensor.h"
#include "sccb.h"
#include "camera.h"
#include "cam_hal.h"
#include "ll_cam.h"

#define TAG "camera"
#include "cam_log.h"

#define CONFIG_OV2640_SUPPORT 1

#if CONFIG_OV2640_SUPPORT
#include "sensors/ov2640.h"
#endif
#if CONFIG_OV7725_SUPPORT
#include "ov7725.h"
#endif
#if CONFIG_OV3660_SUPPORT
#include "ov3660.h"
#endif
#if CONFIG_OV5640_SUPPORT
#include "ov5640.h"
#endif
#if CONFIG_NT99141_SUPPORT
#include "nt99141.h"
#endif
#if CONFIG_OV7670_SUPPORT
#include "ov7670.h"
#endif
#if CONFIG_GC2145_SUPPORT
#include "gc2145.h"
#endif
#if CONFIG_GC032A_SUPPORT
#include "gc032a.h"
#endif
#if CONFIG_GC0308_SUPPORT
#include "gc0308.h"
#endif

#define FB_GET_TIMEOUT 4000

typedef struct
{
    sensor_t sensor;
    camera_fb_t fb;
} camera_state_t;

static camera_state_t *s_state = NULL;

typedef struct
{
    int (*detect)(sensor_t *sensor, sensor_id_t *id);
    int (*init)(sensor_t *sensor);
} sensor_func_t;

static const sensor_func_t g_sensors[] = {
#if CONFIG_OV7725_SUPPORT
    {ov7725_detect, ov7725_init},
#endif
#if CONFIG_OV7670_SUPPORT
    {ov7670_detect, ov7670_init},
#endif
#if CONFIG_OV2640_SUPPORT
    {ov2640_detect, ov2640_init},
#endif
#if CONFIG_OV3660_SUPPORT
    {ov3660_detect, ov3660_init},
#endif
#if CONFIG_OV5640_SUPPORT
    {ov5640_detect, ov5640_init},
#endif
#if CONFIG_NT99141_SUPPORT
    {nt99141_detect, nt99141_init},
#endif
#if CONFIG_GC2145_SUPPORT
    {gc2145_detect, gc2145_init},
#endif
#if CONFIG_GC032A_SUPPORT
    {gc032a_detect, gc032a_init},
#endif
#if CONFIG_GC0308_SUPPORT
    {gc0308_detect, gc0308_init},
#endif
};

static cam_err_t camera_probe(const camera_config_t *config, camera_model_t *out_camera_model)
{
    *out_camera_model = CAMERA_NONE;

    if (s_state != NULL)
    {
        return CAM_ERR_INVALID_STATE;
    }

    s_state = (camera_state_t *)calloc(sizeof(camera_state_t), 1);
    if (!s_state)
    {
        return CAM_ERR_INVALID_STATE;
    }

    if (config->pin_xclk >= 0)
    {
        CAM_LOGD("Enabling XCLK output");
        camera_enable_out_clock(config->pin_xclk, config->xclk_freq_hz);
    }

    CAM_LOGD("Searching for camera address");
    delay(10);

    SCCB_Init(config->sccb);

    uint8_t slv_addr = SCCB_Probe(config->sccb);

    if (slv_addr == 0)
    {
        camera_disable_out_clock(config->pin_xclk);
        return CAM_ERR_NOT_FOUND;
    }

    CAM_LOGI("Detected camera at address=0x%02x", slv_addr);

    s_state->sensor.sccb = config->sccb;
    s_state->sensor.slv_addr = slv_addr;
    s_state->sensor.xclk_freq_hz = config->xclk_freq_hz;

    /**
     * Read sensor ID and then initialize sensor
     * Attention: Some sensors have the same SCCB address. Therefore, several attempts may be made in the detection process
     */
    sensor_id_t *id = &s_state->sensor.id;
    for (size_t i = 0; i < sizeof(g_sensors) / sizeof(sensor_func_t); i++)
    {
        if (g_sensors[i].detect(&s_state->sensor, id))
        {
            camera_sensor_info_t *info = camera_sensor_get_info(id);
            if (NULL != info)
            {
                *out_camera_model = info->model;
                CAM_LOGI("Detected %s camera", info->name);
                g_sensors[i].init(&s_state->sensor);
                break;
            }
        }
    }

    if (CAMERA_NONE == *out_camera_model)
    { //If no supported sensors are detected
        //CAMERA_DISABLE_OUT_CLOCK();
        CAM_LOGE("Detected camera not supported.");
        return CAM_ERR_NOT_SUPPORTED;
    }

    CAM_LOGI("Camera PID=0x%02x VER=0x%02x MIDL=0x%02x MIDH=0x%02x",
             id->PID, id->VER, id->MIDH, id->MIDL);

    CAM_LOGD("Doing SW reset of sensor");
    delay(10);
    s_state->sensor.reset(&s_state->sensor);

    return CAM_OK;
}


cam_err_t camera_init(const camera_config_t *config)
{
    cam_err_t err;
    framesize_t frame_size = (framesize_t)config->frame_size;
    pixformat_t pix_format = (pixformat_t)config->pixel_format;
    err = cam_init(config);
    if (err != CAM_OK)
    {
        CAM_LOGE("Camera init failed with error 0x%x", err);
        return err;
    }

    camera_model_t camera_model = CAMERA_NONE;
    err = camera_probe(config, &camera_model);
    if (err != CAM_OK)
    {
        CAM_LOGE("Camera probe failed with error 0x%x", err);
        goto fail;
    }

    if (PIXFORMAT_JPEG == pix_format && (!camera_sensor[camera_model].support_jpeg))
    {
        CAM_LOGE("JPEG format is not supported on this sensor");
        err = CAM_ERR_NOT_SUPPORTED;
        goto fail;
    }

    if (frame_size > camera_sensor[camera_model].max_size)
    {
        CAM_LOGW("The frame size exceeds the maximum for this sensor, it will be forced to the maximum possible value");
        frame_size = camera_sensor[camera_model].max_size;
    }

    err = cam_config(config, frame_size, s_state->sensor.id.PID);
    if (err != CAM_OK) {
        CAM_LOGE("Camera config failed with error 0x%x", err);
        goto fail;
    }


    s_state->sensor.status.framesize = frame_size;
    s_state->sensor.pixformat = pix_format;
    CAM_LOGD("Setting frame size to %dx%d", resolution[frame_size].width, resolution[frame_size].height);
    if (s_state->sensor.set_framesize(&s_state->sensor, frame_size) != 0)
    {
        CAM_LOGE("Failed to set frame size");
        err = CAM_ERR_FAILED_TO_SET_FRAME_SIZE;
        goto fail;
    }
    s_state->sensor.set_pixformat(&s_state->sensor, pix_format);

    if (s_state->sensor.id.PID == OV2640_PID)
    {
        s_state->sensor.set_gainceiling(&s_state->sensor, GAINCEILING_2X);
        s_state->sensor.set_bpc(&s_state->sensor, false);
        s_state->sensor.set_wpc(&s_state->sensor, true);
        s_state->sensor.set_lenc(&s_state->sensor, true);
    }

    if (pix_format == PIXFORMAT_JPEG)
    {
        s_state->sensor.set_quality(&s_state->sensor, config->jpeg_quality);
    }
    s_state->sensor.init_status(&s_state->sensor);

    //cam_start();

    return CAM_OK;

fail:
    camera_deinit();
    return CAM_FAIL;
}


cam_err_t camera_deinit()
{
    cam_err_t ret = cam_deinit();

    if (s_state)
    {
        SCCB_Deinit(s_state->sensor.sccb);

        free(s_state);
        s_state = NULL;
    }

    return ret;
}

camera_fb_t *camera_fb_get()
{
    if (s_state == NULL)
    {
        return NULL;
    }
    camera_fb_t *fb = cam_take(FB_GET_TIMEOUT);
    //set the frame properties
    if (fb)
    {
        fb->width = resolution[s_state->sensor.status.framesize].width;
        fb->height = resolution[s_state->sensor.status.framesize].height;
        fb->format = s_state->sensor.pixformat;
    }
    return fb;
}

void camera_fb_return(camera_fb_t *fb)
{
    if (s_state == NULL)
    {
        return;
    }
    cam_give(fb);
}

sensor_t *camera_sensor_get()
{
    if (s_state == NULL)
    {
        return NULL;
    }
    return &s_state->sensor;
}
