#define CAMERA_MODEL_ESP32S3_EYE  // Use built-in pin map
#include "esp_camera.h"
#include "camera_pins.h"

const int DATA_RATE = 6000000;  // USB CDC baud (symbolic for native USB)
const int LOG_RATE = 115200;
#define LOG Serial1             //LOG = UART debug stream

void setup() {
  Serial.begin(DATA_RATE);                       // USB CDC (native)
  //LOG.begin(//LOG_RATE, SERIAL_8N1, 44, 43);         // UART: TX=43, RX=44

  delay(500);  // Let USB CDC settle

  //LOG.println("Starting camera setup...");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_XGA;
  config.jpeg_quality = 16;
  config.fb_count = 2;
  config.fb_location = CAMERA_FB_IN_PSRAM;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    //LOG.printf("Camera init failed! Code: 0x%x\n", err);
    while (true) delay(100);
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    s->set_exposure_ctrl(s, 0); // disable auto exposure
    s->set_gain_ctrl(s, 0);     // enable auto gain
    s->set_awb_gain(s, 1);      // Makes stuff very dark if disabled
    s->set_whitebal(s, 1);      // VITAL!
    s->set_brightness(s, 0);    // Neutral is fine
    s->set_contrast(s, 1);      // moderate contrast
    s->set_saturation(s, 1);    // neutral saturation
    s->set_gainceiling(s, (gainceiling_t)1); // higher gain ceiling (optional)
  }
  //LOG.println("Camera ready!");
}

void loop() {
  if (Serial.available() && Serial.read() == 'c') {
    ////LOG.println("Got 'c' command from host");
    unsigned long start = millis();
    camera_fb_t *fb = esp_camera_fb_get();
    unsigned long end = millis();
    //LOG.printf("Capture: %lu ms\n", end - start);
    if (!fb) {
      Serial.write("FRAM", 4);
      uint32_t zero = 0;
      Serial.write((uint8_t *)&zero, 4);
      //LOG.println("Capture failed: fb == nullptr");
      return;
    }

    uint32_t len = fb->len;
    Serial.write("FRAM", 4);
    Serial.write((uint8_t *)&len, 4);
    unsigned long startSend = millis();
    Serial.write(fb->buf, len);
    unsigned long endSend = millis();
    //LOG.printf("Send time: %lu ms\n", endSend - startSend);
    esp_camera_fb_return(fb);
    //LOG.printf("Sent frame: %u bytes\n", len);
  }
}
