#pragma once

#include <stdint.h>
#include "camera.h"
#include "cam_utils.h"

#define CAM_CHECK(a, str, ret)                               \
    if (!(a))                                                \
    {                                                        \
        CAM_LOGE("%s(%d): %s", __FUNCTION__, __LINE__, str); \
        return (ret);                                        \
    }

#define CAM_CHECK_GOTO(a, str, lab)                          \
    if (!(a))                                                \
    {                                                        \
        CAM_LOGE("%s(%d): %s", __FUNCTION__, __LINE__, str); \
        goto lab;                                            \
    }

#define LCD_CAM_DMA_NODE_BUFFER_MAX_SIZE (4092)

typedef struct
{
    camera_fb_t fb;
    uint8_t en;
} cam_frame_t;

typedef enum {
    CAM_STATE_IDLE = 0,
    CAM_STATE_READ_BUF = 1,
} cam_state_t;

typedef struct
{
    cam_frame_t *frames;


    uint8_t jpeg_mode;
    uint32_t frame_cnt;
    uint32_t recv_size;
    bool swap_data;

    uint16_t width;
    uint16_t height;
    uint8_t in_bytes_per_pixel;
    uint8_t fb_bytes_per_pixel;
    uint32_t fb_size;

    cam_state_t state;

} cam_obj_t;

void camera_enable_out_clock(int pin, uint32_t xclk_freq_hz);
void camera_disable_out_clock(int pin);

bool ll_cam_stop(cam_obj_t *cam);
bool ll_cam_start(cam_obj_t *cam, int frame_pos);

cam_err_t ll_cam_config(cam_obj_t *cam, const camera_config_t *config);
cam_err_t ll_cam_deinit(cam_obj_t *cam);
cam_err_t ll_cam_set_sample_mode(cam_obj_t *cam, pixformat_t pix_format, uint32_t xclk_freq_hz, uint16_t sensor_pid);