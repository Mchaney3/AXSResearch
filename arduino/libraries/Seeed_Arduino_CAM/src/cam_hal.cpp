// Copyright 2010-2020 Espressif Systems (Shanghai) PTE LTD
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
#include <string.h>
#include "ll_cam.h"
#include "cam_hal.h"
#include "cam_utils.h"

#define TAG "cam_hal"
#include "cam_log.h"

static cam_obj_t *cam_obj = NULL;

static const uint32_t JPEG_SOI_MARKER = 0xFFD8FF; // written in little-endian for esp32
static const uint16_t JPEG_EOI_MARKER = 0xD9FF;   // written in little-endian for esp32

static int cam_verify_jpeg_soi(const uint8_t *inbuf, uint32_t length)
{
    uint32_t sig = *((uint32_t *)inbuf) & 0xFFFFFF;
    if (sig != JPEG_SOI_MARKER)
    {
        for (uint32_t i = 0; i < length; i++)
        {
            sig = *((uint32_t *)(&inbuf[i])) & 0xFFFFFF;
            if (sig == JPEG_SOI_MARKER)
            {
                CAM_LOGW("SOI: %d", i);
                return i;
            }
        }
        CAM_LOGW("NO-SOI");
        return -1;
    }
    return 0;
}

static int cam_verify_jpeg_eoi(const uint8_t *inbuf, uint32_t length)
{
    int offset = -1;
    uint8_t *dptr = (uint8_t *)inbuf + length - 2;
    while (dptr > inbuf)
    {
        uint16_t sig = *((uint16_t *)dptr);
        if (JPEG_EOI_MARKER == sig)
        {
            offset = dptr - inbuf;
            //CAM_LOGW("EOI: %d", length - (offset + 2));
            return offset;
        }
        dptr--;
    }
    return -1;
}

static bool cam_get_next_frame(int *frame_pos)
{
    if (!cam_obj->frames[*frame_pos].en)
    {
        for (int x = 0; x < cam_obj->frame_cnt; x++)
        {
            if (cam_obj->frames[x].en)
            {
                *frame_pos = x;
                return true;
            }
        }
    }
    else
    {
        return true;
    }
    return false;
}

static bool cam_start_frame(int *frame_pos)
{
    if (cam_get_next_frame(frame_pos))
    {
        if (ll_cam_start(cam_obj, *frame_pos))
        {
            uint64_t us = (uint64_t)millis();
            cam_obj->frames[*frame_pos].fb.timestamp.tv_sec = us / 1000UL;
            cam_obj->frames[*frame_pos].fb.timestamp.tv_usec = us % 1000UL;
            return true;
        }
    }
    return false;
}

static cam_err_t cam_memory_config()
{

    cam_obj->frames = (cam_frame_t *)malloc(cam_obj->frame_cnt * sizeof(cam_frame_t));
    CAM_CHECK(cam_obj->frames != NULL, "frames malloc failed", CAM_FAIL);

    size_t fb_size = cam_obj->fb_size;
    for (int x = 0; x < cam_obj->frame_cnt; x++)
    {
        cam_obj->frames[x].en = 0;
        CAM_LOGI("Allocating %d Byte frame buffer in PSRAM", fb_size * sizeof(uint8_t));
        cam_obj->frames[x].fb.buf = (uint8_t *)extmem_malloc(fb_size * sizeof(uint8_t));
        CAM_CHECK(cam_obj->frames[x].fb.buf != NULL, "frame buffer malloc failed", CAM_FAIL);
        cam_obj->frames[x].en = 1;
    }

    return CAM_OK;
}

cam_err_t cam_init(const camera_config_t *config)
{
    CAM_CHECK(NULL != config, "config pointer is invalid", CAM_ERR_INVALID_ARG);

    cam_err_t ret = CAM_OK;
    cam_obj = (cam_obj_t *)malloc(sizeof(cam_obj_t));
    CAM_CHECK(NULL != cam_obj, "lcd_cam object malloc error", CAM_ERR_NO_MEM);

    cam_obj->swap_data = 0;

    ret = ll_cam_config(cam_obj, config);
    CAM_CHECK_GOTO(ret == CAM_OK, "ll_cam initialize failed", err);

    CAM_LOGI("cam init ok");
    return CAM_OK;

err:
    free(cam_obj);
    cam_obj = NULL;
    return CAM_FAIL;
}

cam_err_t cam_config(const camera_config_t *config, framesize_t frame_size, uint8_t sensor_pid)
{
    CAM_CHECK(NULL != config, "config pointer is invalid", CAM_ERR_INVALID_ARG);
    cam_obj->jpeg_mode = config->pixel_format == PIXFORMAT_JPEG;
    cam_err_t ret = CAM_OK;

    ret = ll_cam_set_sample_mode(cam_obj, (pixformat_t)config->pixel_format, config->xclk_freq_hz, sensor_pid);

    cam_obj->frame_cnt = config->fb_count;
    cam_obj->width = resolution[frame_size].width;
    cam_obj->height = resolution[frame_size].height;

    if (cam_obj->jpeg_mode)
    {
        cam_obj->recv_size = cam_obj->width * cam_obj->height / 5;
        cam_obj->fb_size = cam_obj->recv_size;
    }
    else
    {
        cam_obj->recv_size = cam_obj->width * cam_obj->height * cam_obj->in_bytes_per_pixel;
        cam_obj->fb_size = cam_obj->width * cam_obj->height * cam_obj->fb_bytes_per_pixel;
    }

    ret = cam_memory_config();
    CAM_CHECK_GOTO(ret == CAM_OK, "cam_dma_config failed", err);

    CAM_LOGI("cam config ok");
    return CAM_OK;

err:
    cam_deinit();
    return CAM_FAIL;
}

cam_err_t cam_deinit(void)
{
    if (!cam_obj)
    {
        return CAM_FAIL;
    }

    cam_stop();

    if (cam_obj->frames)
    {
        for (int x = 0; x < cam_obj->frame_cnt; x++)
        {
            extmem_free(cam_obj->frames[x].fb.buf);
        }
        free(cam_obj->frames);
    }

    ll_cam_deinit(cam_obj);

    free(cam_obj);
    cam_obj = NULL;
    return CAM_OK;
}

void cam_stop(void)
{
    ll_cam_stop(cam_obj);
}

void cam_start(void)
{
}

camera_fb_t *cam_take(uint32_t timeout)
{
    camera_fb_t *dma_buffer = NULL;
    uint32_t start = millis();
    int frame_pos = 0;
    
    if (cam_start_frame(&frame_pos))
    {
        dma_buffer = &cam_obj->frames[frame_pos].fb;
        cam_obj->frames[frame_pos].fb.len = 0;
        cam_obj->frames[frame_pos].en = 0;
        cam_obj->state = CAM_STATE_READ_BUF;
    }
    else
    {
        dma_buffer = &cam_obj->frames[frame_pos].fb;
        cam_give(dma_buffer);
        cam_obj->state = CAM_STATE_IDLE;
        return cam_take(timeout - (millis() - start));
    }

    while (1)
    {
        if (cam_obj->state == CAM_STATE_IDLE)
        {
            if(cam_obj->jpeg_mode)
            {
                // find the end marker for JPEG. Data after that can be discarded
                int offset_e = cam_verify_jpeg_eoi(dma_buffer->buf, dma_buffer->len);
                if (offset_e >= 0)
                {
                    // adjust buffer length
                    dma_buffer->len = offset_e + sizeof(JPEG_EOI_MARKER);
                    return dma_buffer;
                }
                else
                {
                    CAM_LOGW("NO-EOI");
                    cam_give(dma_buffer);
                    return cam_take(timeout - (millis() - start)); //recurse!!!!
                }
            }
            dma_buffer->len = cam_obj->fb_size;
            return dma_buffer;
        }
        if ((millis() - start) >= timeout)
        {
            CAM_LOGE("Failed to get the frame on time!");
            cam_obj->state == CAM_STATE_IDLE;
            return NULL;
        }
    }

    return NULL;
}

void cam_give(camera_fb_t *dma_buffer)
{
    for (int x = 0; x < cam_obj->frame_cnt; x++)
    {
        if (&cam_obj->frames[x].fb == dma_buffer)
        {
            cam_obj->frames[x].en = 1;
            break;
        }
    }
}
