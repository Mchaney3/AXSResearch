#include "Arduino.h"
#include "avr/pgmspace.h"
#include "ll_cam.h"
#include "cam_utils.h"
#define TAG "ll cam"
#include "cam_log.h"

DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi_pssi;
cam_state_t *cam_state = NULL;

static cam_err_t MX_DCMI_Init(void)
{

    hdcmi.Instance = DCMI;
    hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
    hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
    hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
    hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
    hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
    hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
    hdcmi.Init.JPEGMode = DCMI_JPEG_ENABLE;
    hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
    hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
    hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
    hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;
    if (HAL_DCMI_Init(&hdcmi) != HAL_OK)
    {
        return CAM_FAIL;
    }
     return CAM_OK;
}

static void MX_DMA_Init(void)
{

    __HAL_RCC_DMA2_CLK_ENABLE();
    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
}

static void OV2640_DMA_Config(uint8_t *DMA_Memory0BaseAddr, uint32_t DMA_BufferSize)
{

    __HAL_RCC_DMA2_CLK_ENABLE();

    hdma_dcmi_pssi.Instance = DMA2_Stream1;
    hdma_dcmi_pssi.Init.Request = DMA_REQUEST_DCMI;
    hdma_dcmi_pssi.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_dcmi_pssi.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dcmi_pssi.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dcmi_pssi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_dcmi_pssi.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_dcmi_pssi.Init.Mode = DMA_CIRCULAR;
    hdma_dcmi_pssi.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_dcmi_pssi.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_dcmi_pssi.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_dcmi_pssi.Init.MemBurst = DMA_MBURST_INC8;
    hdma_dcmi_pssi.Init.PeriphBurst = DMA_PBURST_SINGLE;

    __HAL_LINKDMA(&hdcmi, DMA_Handle, hdma_dcmi_pssi);
    HAL_DMA_Init(&hdma_dcmi_pssi);

    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

    HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)DMA_Memory0BaseAddr, DMA_BufferSize);
}

void camera_enable_out_clock(int pin, uint32_t xclk_freq_hz)
{
    if (pin >= 0)
    {
        analogWriteFrequency(xclk_freq_hz);
        analogWrite(pin, 127);
    }
}

void camera_disable_out_clock(int pin)
{
    if (pin >= 0)
    {
        analogWrite(pin, 0);
    }
}

bool ll_cam_stop(cam_obj_t *cam)
{
    cam_state = NULL;
    HAL_DCMI_Suspend(&hdcmi);
	HAL_DCMI_Stop(&hdcmi);
}

bool ll_cam_start(cam_obj_t *cam, int frame_pos)
{
    cam_state = &cam->state;
    
    OV2640_DMA_Config(cam->frames[frame_pos].fb.buf, cam->recv_size/4);

    return true;
}

cam_err_t ll_cam_config(cam_obj_t *cam, const camera_config_t *config)
{

    if (config->pin_reset > -1)
    {
        pinMode(config->pin_reset, OUTPUT);
        digitalWrite(config->pin_reset, LOW);
        delay(100);
        digitalWrite(config->pin_reset, HIGH);
        delay(100);
    }

    MX_DMA_Init();

    return MX_DCMI_Init();
}

cam_err_t ll_cam_deinit(cam_obj_t *cam)
{
    HAL_DCMI_DeInit(&hdcmi);
    
    return CAM_OK;
}


uint8_t ll_cam_get_dma_align(cam_obj_t *cam)
{
    return 4;
}

size_t IRAM_ATTR ll_cam_memcpy(cam_obj_t *cam, uint8_t *out, const uint8_t *in, size_t len)
{
    // YUV to Grayscale
    if (cam->in_bytes_per_pixel == 2 && cam->fb_bytes_per_pixel == 1) {
        size_t end = len / 8;
        for (size_t i = 0; i < end; ++i) {
            out[0] = in[0];
            out[1] = in[2];
            out[2] = in[4];
            out[3] = in[6];
            out += 4;
            in += 8;
        }
        return len / 2;
    }

    // just memcpy
    memcpy(out, in, len);
    return len;
}


cam_err_t ll_cam_set_sample_mode(cam_obj_t *cam, pixformat_t pix_format, uint32_t xclk_freq_hz, uint16_t sensor_pid)
{
    if (pix_format == PIXFORMAT_GRAYSCALE) {
        if (sensor_pid == OV3660_PID || sensor_pid == OV5640_PID || sensor_pid == NT99141_PID) {
            cam->in_bytes_per_pixel = 1;       // camera sends Y8
        } else {
            cam->in_bytes_per_pixel = 2;       // camera sends YU/YV
        }
        cam->fb_bytes_per_pixel = 1;       // frame buffer stores Y8
    } else if (pix_format == PIXFORMAT_YUV422 || pix_format == PIXFORMAT_RGB565) {
            cam->in_bytes_per_pixel = 2;       // camera sends YU/YV
            cam->fb_bytes_per_pixel = 2;       // frame buffer stores YU/YV/RGB565
    } else if (pix_format == PIXFORMAT_JPEG) {
        cam->in_bytes_per_pixel = 1;
        cam->fb_bytes_per_pixel = 1;
    } else {
        CAM_LOGE("Requested format is not supported");
        return CAM_ERR_NOT_SUPPORTED;
    }
    return CAM_OK;
}


/**
  * @brief
  * @param  None
  * @retval None
  */
extern "C"
{
    void HAL_DCMI_MspInit(DCMI_HandleTypeDef *dcmiHandle)
    {

        GPIO_InitTypeDef GPIO_InitStruct = {0};
        if (dcmiHandle->Instance == DCMI)
        {
            __HAL_RCC_DCMI_CLK_ENABLE();

            __HAL_RCC_GPIOE_CLK_ENABLE();
            __HAL_RCC_GPIOG_CLK_ENABLE();
            __HAL_RCC_GPIOD_CLK_ENABLE();
            __HAL_RCC_GPIOA_CLK_ENABLE();
            __HAL_RCC_GPIOH_CLK_ENABLE();

            GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_1 | GPIO_PIN_0;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
            HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

            GPIO_InitStruct.Pin = GPIO_PIN_9;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
            HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

            GPIO_InitStruct.Pin = GPIO_PIN_3;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
            HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

            GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_4 | GPIO_PIN_6;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

            GPIO_InitStruct.Pin = GPIO_PIN_10;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
            HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

            hdma_dcmi_pssi.Instance = DMA2_Stream1;
            hdma_dcmi_pssi.Init.Request = DMA_REQUEST_DCMI_PSSI;
            hdma_dcmi_pssi.Init.Direction = DMA_PERIPH_TO_MEMORY;
            hdma_dcmi_pssi.Init.PeriphInc = DMA_PINC_DISABLE;
            hdma_dcmi_pssi.Init.MemInc = DMA_MINC_ENABLE;
            hdma_dcmi_pssi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
            hdma_dcmi_pssi.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
            hdma_dcmi_pssi.Init.Mode = DMA_CIRCULAR;
            hdma_dcmi_pssi.Init.Priority = DMA_PRIORITY_VERY_HIGH;
            hdma_dcmi_pssi.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
            hdma_dcmi_pssi.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
            hdma_dcmi_pssi.Init.MemBurst = DMA_MBURST_SINGLE;
            hdma_dcmi_pssi.Init.PeriphBurst = DMA_PBURST_SINGLE;
            if (HAL_DMA_Init(&hdma_dcmi_pssi) != HAL_OK)
            {
                Error_Handler();
            }

            HAL_NVIC_SetPriority(DCMI_PSSI_IRQn, 5, 0);
            HAL_NVIC_EnableIRQ(DCMI_PSSI_IRQn);
        }
    }

    void HAL_DCMI_MspDeInit(DCMI_HandleTypeDef *dcmiHandle)
    {

        if (dcmiHandle->Instance == DCMI)
        {
            __HAL_RCC_DCMI_CLK_DISABLE();

            HAL_GPIO_DeInit(GPIOE, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_1 | GPIO_PIN_0);

            HAL_GPIO_DeInit(GPIOG, GPIO_PIN_9);

            HAL_GPIO_DeInit(GPIOD, GPIO_PIN_3);

            HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_4 | GPIO_PIN_6);

            HAL_GPIO_DeInit(GPIOH, GPIO_PIN_10);

            HAL_NVIC_DisableIRQ(DCMI_PSSI_IRQn);
        }
    }

    void DCMI_PSSI_IRQHandler(void)
    {
        HAL_DCMI_IRQHandler(&hdcmi);
    }

    void DMA2_Stream1_IRQHandler(void)
    {
        HAL_DMA_IRQHandler(&hdma_dcmi_pssi);
    }

    void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
    {
       if(cam_state != NULL)
       {
           *cam_state = CAM_STATE_IDLE;
       }
    }
    void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi)
    {
       
    }
}