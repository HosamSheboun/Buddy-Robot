#include "esp_camera.h"
#include <WiFi.h>
#include <esp_http_server.h>
#include "secrets.h"

// AI Thinker ESP32-CAM camera pins
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

httpd_handle_t cameraServer = NULL;

esp_err_t captureHandler(httpd_req_t* request);
esp_err_t healthHandler(httpd_req_t* request);
bool connectToWiFi();
bool startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.println("[CAM] Starting camera snapshot server");

  camera_config_t config = {};
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  const bool hasPsram = psramFound();
  config.frame_size = FRAMESIZE_QQVGA;
  config.jpeg_quality = 18;
  config.fb_count = 1;
  config.fb_location = hasPsram ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  Serial.printf("[CAM] PSRAM: %s; free heap: %u bytes\n",
                hasPsram ? "found" : "not found",
                (unsigned int)ESP.getFreeHeap());

  esp_err_t error = esp_camera_init(&config);
  if (error != ESP_OK) {
    Serial.printf("[CAM] Camera initialization failed: 0x%x\n", error);
    return;
  }

  if (!connectToWiFi()) {
    Serial.println("[CAM] Snapshot server not started because Wi-Fi failed");
    return;
  }

  if (!startCameraServer()) {
    Serial.println("[CAM] Failed to start HTTP snapshot server");
    return;
  }

  Serial.printf("[CAM] Ready. Snapshot URL: http://%s/capture\n",
                WiFi.localIP().toString().c_str());
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[CAM] Wi-Fi disconnected; reconnecting");
    WiFi.reconnect();
    delay(5000);
  } else {
    delay(1000);
  }
}

bool connectToWiFi() {
  if (String(WIFI_SSID).startsWith("YOUR_") ||
      String(WIFI_PASSWORD).startsWith("YOUR_")) {
    Serial.println("[CAM] Set Wi-Fi credentials in CAM/secrets.h");
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[CAM] Connecting to Wi-Fi");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    Serial.print(".");
    delay(250);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[CAM] Wi-Fi connection timed out");
    return false;
  }

  Serial.printf("\n[CAM] Wi-Fi connected, IP: %s\n",
                WiFi.localIP().toString().c_str());
  return true;
}

bool startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  config.ctrl_port = 32768;
  config.max_uri_handlers = 2;

  esp_err_t error = httpd_start(&cameraServer, &config);
  if (error != ESP_OK) {
    Serial.printf("[CAM] httpd_start failed: 0x%x\n", error);
    cameraServer = NULL;
    return false;
  }

  httpd_uri_t captureUri = {};
  captureUri.uri = "/capture";
  captureUri.method = HTTP_GET;
  captureUri.handler = captureHandler;
  captureUri.user_ctx = NULL;
  error = httpd_register_uri_handler(cameraServer, &captureUri);
  if (error != ESP_OK) {
    Serial.printf("[CAM] Could not register /capture: 0x%x\n", error);
    httpd_stop(cameraServer);
    cameraServer = NULL;
    return false;
  }

  httpd_uri_t healthUri = {};
  healthUri.uri = "/health";
  healthUri.method = HTTP_GET;
  healthUri.handler = healthHandler;
  healthUri.user_ctx = NULL;
  error = httpd_register_uri_handler(cameraServer, &healthUri);
  if (error != ESP_OK) {
    Serial.printf("[CAM] Could not register /health: 0x%x\n", error);
    httpd_stop(cameraServer);
    cameraServer = NULL;
    return false;
  }

  return true;
}

esp_err_t captureHandler(httpd_req_t* request) {
  camera_fb_t* frame = esp_camera_fb_get();
  if (frame == NULL) {
    Serial.println("[CAM] Snapshot capture failed");
    httpd_resp_send_500(request);
    return ESP_FAIL;
  }

  httpd_resp_set_type(request, "image/jpeg");
  httpd_resp_set_hdr(request, "Cache-Control", "no-store");
  esp_err_t result = httpd_resp_send(
      request, reinterpret_cast<const char*>(frame->buf), frame->len);
  esp_camera_fb_return(frame);
  return result;
}

esp_err_t healthHandler(httpd_req_t* request) {
  httpd_resp_set_type(request, "text/plain");
  return httpd_resp_sendstr(request, "Buddy camera ready");
}
