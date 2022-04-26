#include "Arduino.h"
#include "camera.h"
camera_config_t camera_config{
  .sccb = &Wire,
  .pin_reset = PH12,
  .pin_xclk = DCMI_PWM,
  .xclk_freq_hz = 24000000,
  .pixel_format = PIXFORMAT_RGB565,
  .frame_size = FRAMESIZE_QVGA,
  .jpeg_quality = 10,
  .fb_count = 1,
};

void setup()
{
  Serial.begin(115200);
  cam_err_t err = camera_init(&camera_config);
  if (err != CAM_OK)
  {
        Serial.printf("Camera Init Failed : %d", err);
  }
  Serial.println("Camera Init OK");
}

void loop()
{
    
}