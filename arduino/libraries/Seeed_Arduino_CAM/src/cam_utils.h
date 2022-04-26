#pragma once

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef int32_t cam_err_t;

#define CAM_OK 0
#define CAM_FAIL -1
#define CAM_ERR_NOT_FOUND -2
#define CAM_ERR_NOT_SUPPORTED -3
#define CAM_ERR_INVALID_STATE -4
#define CAM_ERR_FAILED_TO_SET_FRAME_SIZE -5
#define CAM_ERR_INVALID_ARG -6
#define CAM_ERR_NO_MEM -7

#ifdef __cplusplus
}
#endif
