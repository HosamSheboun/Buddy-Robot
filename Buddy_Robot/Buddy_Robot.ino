#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <mbedtls/base64.h>
#include <ctype.h>
#include <time.h>
#include <vector>
#include "secrets.h"

// ST7735 TFT Display (SPI)
#define TFT_CS 15
#define TFT_RST 4
#define TFT_DC 2

// Main-board peripherals
#define BUZZER_PIN 14
#define TRIG_PIN 13
#define ECHO_PIN 35
#define RGB_RED_PIN 32
#define RGB_GREEN_PIN 33
#define RGB_BLUE_PIN 25

static const char GEMINI_MODEL[] = "gemini-2.5-flash";
static const unsigned long AI_INTERVAL_MS = 60000;
static const size_t MAX_JPEG_BYTES = 24000;
static const char GOOGLE_ROOT_CA[] PROGMEM = R"PEM(
-----BEGIN CERTIFICATE-----
MIIFVzCCAz+gAwIBAgINAgPlk28xsBNJiGuiFzANBgkqhkiG9w0BAQwFADBHMQsw
CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU
MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAw
MDAwWjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp
Y2VzIExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwggIiMA0GCSqGSIb3DQEBAQUA
A4ICDwAwggIKAoICAQC2EQKLHuOhd5s73L+UPreVp0A8of2C+X0yBoJx9vaMf/vo
27xqLpeXo4xL+Sv2sfnOhB2x+cWX3u+58qPpvBKJXqeqUqv4IyfLpLGcY9vXmX7w
Cl7raKb0xlpHDU0QM+NOsROjyBhsS+z8CZDfnWQpJSMHobTSPS5g4M/SCYe7zUjw
TcLCeoiKu7rPWRnWr4+wB7CeMfGCwcDfLqZtbBkOtdh+JhpFAz2weaSUKK0Pfybl
qAj+lug8aJRT7oM6iCsVlgmy4HqMLnXWnOunVmSPlk9orj2XwoSPwLxAwAtcvfaH
szVsrBhQf4TgTM2S0yDpM7xSma8ytSmzJSq0SPly4cpk9+aCEI3oncKKiPo4Zor8
Y/kB+Xj9e1x3+naH+uzfsQ55lVe0vSbv1gHR6xYKu44LtcXFilWr06zqkUspzBmk
MiVOKvFlRNACzqrOSbTqn3yDsEB750Orp2yjj32JgfpMpf/VjsPOS+C12LOORc92
wO1AK/1TD7Cn1TsNsYqiA94xrcx36m97PtbfkSIS5r762DL8EGMUUXLeXdYWk70p
aDPvOmbsB4om3xPXV2V4J95eSRQAogB/mqghtqmxlbCluQ0WEdrHbEg8QOB+DVr
NVjzRlwW5y0vtOUucxD/SVRNuJLDWcfr0wbrM7Rv1/oFB2ACYPTrIrnqYNxgFlQID
AQABo0IwQDAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4E
FgQU5K8rJnEaK0gnhS9SZizv8IkTcT4wDQYJKoZIhvcNAQEMBQADggIBAJ+qQibb
C5u+/x6Wki4+omVKapi6Ist9wTrYggoGxval3sBOh2Z5ofmmWJyq+bXmYOfg6LEe
QkEzCzc9zolwFcq1JKjPa7XSQCGYzyI0zzvFIoTgxQ6KfF2I5DUkzps+GlQebtuy
h6f88/qBVRRiClmpIgUxPoLW7ttXNLwzldMXG+gnoot7TiYaelpkttGsN/H9oPM4
7HLwEXWdyzRSjeZ2axfG34arJ45JK3VmgRAhpuo+9K4l/3wV3s6MJT/KYnAK9y8J
ZgfIPxz88NtFMN9iiMG1D53Dn0reWVlHxYciNuaCp+0KueIHoI17eko8cdLiA6Ef
MgfdG+RCzgwARWGAtQsgWSl4vflVy2PFPEz0tv/bal8xa5meLMFrUKTX5hgUvYU/
Z6tGn6D/Qqc6f1zLXbBwHSs09dR2CQzreExZBfMzQsNhFRAbd03OIozUhfJFfbdT
6u9AWpQKXCBfTkBdYiJ23//OYb2MI3jSNwLgjt7RETeJ9r/tSQdirpLsQBqvFAnZ
0E6yove+7u7Y/9waLd64NnHi/Hm3lCXRSHNboTXns5lndcEZOitHTtNCjv0xyBZm
2tIMPNuzjsmhDYAPexZ3FL//2wmUspO8IFgV6dtxQ/PeEMMA3KgqlbbC1j+Qa3bb
bP6MvPJwNQzcmRk13NfIRmPVNnGuV/u3gm3c
-----END CERTIFICATE-----
)PEM";

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

String mood = "CALM";
String aiReply = "Connecting to Wi-Fi...";
String pendingUserMessage;
String previousReply;
unsigned long lastBlinkAt = 0;
unsigned long blinkStartedAt = 0;
unsigned long lastGlanceAt = 0;
unsigned long glanceStartedAt = 0;
unsigned long lastDistanceReadAt = 0;
unsigned long lastAiRequestAt = 0;
int glanceDirection = 1;
uint8_t renderedOpenness = 255;
int renderedGlance = 999;
int distanceCm = -1;
bool chatPending = false;
char serialLine[100];
size_t serialLineLength = 0;

void setRgbColor(uint8_t red, uint8_t green, uint8_t blue);
void beep(uint16_t frequency, uint16_t durationMs);
void drawFace(const String& faceMood);
void renderEyes(const String& faceMood, uint8_t openness, int glanceX);
void updateEyeAnimation();
void readSerialChat();
void readDistanceSensor();
bool connectToWiFi();
bool fetchCameraFrame(std::vector<uint8_t>& jpeg);
bool askGemini(const std::vector<uint8_t>& jpeg, String& resultMood, String& reply);
String jsonEscape(const String& value);
String jsonStringField(const String& json, const char* key);
void showError(const String& message);

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(TRIG_PIN, LOW);
  setRgbColor(0, 0, 0);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(2);
  tft.fillScreen(ST7735_BLACK);
  Serial.println("[MASTER] Buddy main controller starting");
  Serial.println("[MASTER] Serial Monitor: 115200 baud, newline ending; type to chat.");

  if (String(WIFI_SSID).startsWith("YOUR_") ||
      String(WIFI_PASSWORD).startsWith("YOUR_")) {
    showError("Set Wi-Fi in secrets.h");
  } else if (connectToWiFi()) {
    aiReply = "Wi-Fi ready. Type to chat.";
    Serial.printf("[MASTER] Wi-Fi connected: %s\n",
                  WiFi.localIP().toString().c_str());
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    unsigned long timeStartedAt = millis();
    while (time(nullptr) < 1700000000 && millis() - timeStartedAt < 10000) {
      delay(250);
    }
    if (time(nullptr) < 1700000000) {
      Serial.println("[MASTER] NTP time unavailable; Gemini TLS may fail");
    }
  } else {
    showError("Wi-Fi connection failed");
  }

  drawFace(mood);
  lastBlinkAt = millis();
  lastGlanceAt = millis();
  lastAiRequestAt = millis() - AI_INTERVAL_MS;
}

void loop() {
  readSerialChat();
  readDistanceSensor();
  updateEyeAnimation();

  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastReconnectAt = 0;
    if (millis() - lastReconnectAt >= 15000) {
      lastReconnectAt = millis();
      WiFi.reconnect();
      Serial.println("[MASTER] Reconnecting to Wi-Fi");
    }
  } else if (chatPending || millis() - lastAiRequestAt >= AI_INTERVAL_MS) {
    lastAiRequestAt = millis();
    chatPending = false;

    std::vector<uint8_t> jpeg;
    if (!fetchCameraFrame(jpeg)) {
      showError("Camera unavailable");
    } else {
      String resultMood;
      String reply;
      if (askGemini(jpeg, resultMood, reply)) {
        mood = resultMood;
        aiReply = reply;
        previousReply = reply;
        Serial.print("[MASTER] AI mood: ");
        Serial.print(mood);
        Serial.print(" | ");
        Serial.println(aiReply);
        drawFace(mood);
      } else {
        showError("Gemini request failed");
      }
      std::vector<uint8_t>().swap(jpeg);
    }
  }
  delay(10);
}

void setRgbColor(uint8_t red, uint8_t green, uint8_t blue) {
  analogWrite(RGB_RED_PIN, red);
  analogWrite(RGB_GREEN_PIN, green);
  analogWrite(RGB_BLUE_PIN, blue);
}

void beep(uint16_t frequency, uint16_t durationMs) {
  tone(BUZZER_PIN, frequency);
  delay(durationMs);
  noTone(BUZZER_PIN);
}

void showError(const String& message) {
  aiReply = message;
  Serial.print("[MASTER] ");
  Serial.println(message);
  renderEyes(mood, 100, 0);
  renderedOpenness = 100;
  renderedGlance = 0;
}

void drawFace(const String& faceMood) {
  uint8_t red = 0;
  uint8_t green = 100;
  uint8_t blue = 255;

  if (faceMood == "HAPPY") {
    red = 0; green = 255; blue = 0;
    beep(2600, 90);
  } else if (faceMood == "SAD") {
    red = 0; green = 0; blue = 255;
    beep(1500, 150);
  } else if (faceMood == "ANGRY") {
    red = 255; green = 0; blue = 0;
    beep(2300, 110);
  } else if (faceMood == "SURPRISED") {
    red = 255; green = 180; blue = 0;
    beep(3000, 90);
  }

  setRgbColor(red, green, blue);
  renderEyes(faceMood, 100, 0);
  renderedOpenness = 100;
  renderedGlance = 0;
}

void renderEyes(const String& faceMood, uint8_t openness, int glanceX) {
  uint16_t eyeColor = ST7735_CYAN;
  int eyeHeight = 30;
  int shiftY = 0;
  int eyebrowAngle = 0;

  if (faceMood == "HAPPY") {
    eyeColor = ST7735_GREEN;
  } else if (faceMood == "SAD") {
    eyeColor = ST7735_BLUE;
    eyebrowAngle = -12;
  } else if (faceMood == "ANGRY") {
    eyeColor = ST7735_RED;
    shiftY = 5;
    eyebrowAngle = 16;
  } else if (faceMood == "SURPRISED") {
    eyeColor = ST7735_YELLOW;
    eyeHeight = 38;
  }

  int visibleHeight = eyeHeight * openness / 100;
  int centerX = tft.width() / 2 + glanceX;
  int centerY = tft.height() / 2 + shiftY;
  const int eyeWidth = 30;
  const int eyeGap = 8;
  int leftX = centerX - eyeWidth - eyeGap / 2;
  int rightX = centerX + eyeGap / 2;
  int leftY = centerY - visibleHeight / 2;
  int rightY = centerY - visibleHeight / 2;

  tft.fillScreen(ST7735_BLACK);
  if (visibleHeight > 2) {
    tft.fillRoundRect(leftX, leftY, eyeWidth, visibleHeight, 4, eyeColor);
    tft.fillRoundRect(rightX, rightY, eyeWidth, visibleHeight, 4, eyeColor);
  } else {
    tft.drawFastHLine(leftX, centerY, eyeWidth, eyeColor);
    tft.drawFastHLine(rightX, centerY, eyeWidth, eyeColor);
  }

  if (eyebrowAngle > 0) {
    tft.fillTriangle(leftX - 2, leftY - 2, leftX + eyeWidth + 2, leftY - 2,
                     leftX + eyeWidth + 2, leftY + eyebrowAngle, ST7735_BLACK);
    tft.fillTriangle(rightX - 2, rightY - 2, rightX + eyeWidth + 2, rightY - 2,
                     rightX - 2, rightY + eyebrowAngle, ST7735_BLACK);
  } else if (eyebrowAngle < 0) {
    int angle = -eyebrowAngle;
    tft.fillTriangle(leftX - 2, leftY - 2, leftX + eyeWidth + 2, leftY - 2,
                     leftX - 2, leftY + angle, ST7735_BLACK);
    tft.fillTriangle(rightX - 2, rightY - 2, rightX + eyeWidth + 2, rightY - 2,
                     rightX + eyeWidth + 2, rightY + angle, ST7735_BLACK);
  }

  if (openness > 70 && visibleHeight > 8) {
    tft.drawFastHLine(leftX + 5, leftY + 4, eyeWidth - 10, ST7735_WHITE);
    tft.drawFastHLine(rightX + 5, rightY + 4, eyeWidth - 10, ST7735_WHITE);
  }

  tft.setTextSize(1);
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(4, tft.height() - 38);
  tft.print("AI:");
  tft.setCursor(4, tft.height() - 28);
  tft.setTextWrap(true);
  tft.print(aiReply);
  tft.setTextWrap(false);
}

void updateEyeAnimation() {
  unsigned long now = millis();
  uint8_t openness = 100;
  int glanceX = 0;

  if (blinkStartedAt == 0 && now - lastBlinkAt >= 3500) {
    blinkStartedAt = now;
    lastBlinkAt = now;
  }
  if (blinkStartedAt != 0) {
    unsigned long elapsed = now - blinkStartedAt;
    if (elapsed < 90) {
      openness = 100 - elapsed * 92 / 90;
    } else if (elapsed < 130) {
      openness = 8;
    } else if (elapsed < 220) {
      openness = 8 + (elapsed - 130) * 92 / 90;
    } else {
      blinkStartedAt = 0;
    }
  }

  if (blinkStartedAt == 0) {
    if (glanceStartedAt == 0 && now - lastGlanceAt >= 4500) {
      glanceStartedAt = now;
      lastGlanceAt = now;
      glanceDirection = -glanceDirection;
    }
    if (glanceStartedAt != 0) {
      if (now - glanceStartedAt < 550) {
        glanceX = 6 * glanceDirection;
      } else {
        glanceStartedAt = 0;
      }
    }
  }

  if (openness != renderedOpenness || glanceX != renderedGlance) {
    renderEyes(mood, openness, glanceX);
    renderedOpenness = openness;
    renderedGlance = glanceX;
  }
}

bool connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[MASTER] Connecting to Wi-Fi");
  unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < 20000) {
    Serial.print(".");
    delay(250);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[MASTER] Wi-Fi connection timed out");
    return false;
  }
  Serial.printf("\n[MASTER] Wi-Fi connected: %s\n",
                WiFi.localIP().toString().c_str());
  return true;
}

void readSerialChat() {
  while (Serial.available()) {
    char character = (char)Serial.read();
    if (character == '\n' || character == '\r') {
      if (serialLineLength > 0) {
        serialLine[serialLineLength] = '\0';
        pendingUserMessage = String(serialLine);
        pendingUserMessage.trim();
        if (pendingUserMessage.length() > 80) {
          pendingUserMessage.remove(80);
        }
        serialLineLength = 0;
        if (pendingUserMessage.length() > 0) {
          chatPending = true;
          Serial.print("[MASTER] You: ");
          Serial.println(pendingUserMessage);
        }
      }
    } else if (character >= 32 && serialLineLength < sizeof(serialLine) - 1) {
      serialLine[serialLineLength++] = character;
    }
  }
}

void readDistanceSensor() {
  if (millis() - lastDistanceReadAt < 250) {
    return;
  }
  lastDistanceReadAt = millis();

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  distanceCm = duration == 0 ? -1 : (int)(duration * 0.0343 / 2.0);
}

bool fetchCameraFrame(std::vector<uint8_t>& jpeg) {
  if (String(CAMERA_SNAPSHOT_URL).startsWith("YOUR_")) {
    Serial.println("[MASTER] Set CAMERA_SNAPSHOT_URL in secrets.h");
    return false;
  }
  HTTPClient cameraHttp;
  cameraHttp.setTimeout(10000);
  if (!cameraHttp.begin(CAMERA_SNAPSHOT_URL)) {
    Serial.println("[MASTER] Could not start camera HTTP request");
    return false;
  }
  int status = cameraHttp.GET();
  if (status != HTTP_CODE_OK) {
    Serial.printf("[MASTER] Camera HTTP error: %d\n", status);
    cameraHttp.end();
    return false;
  }

  int length = cameraHttp.getSize();
  if (length <= 0 || (size_t)length > MAX_JPEG_BYTES) {
    Serial.printf("[MASTER] Invalid/large camera frame: %d bytes\n", length);
    cameraHttp.end();
    return false;
  }
  jpeg.resize((size_t)length);
  WiFiClient* stream = cameraHttp.getStreamPtr();
  stream->setTimeout(10000);
  size_t received = stream->readBytes(jpeg.data(), jpeg.size());
  cameraHttp.end();
  if (received != jpeg.size()) {
    Serial.printf("[MASTER] Incomplete camera frame: %u/%u bytes\n",
                  (unsigned int)received, (unsigned int)jpeg.size());
    jpeg.clear();
    return false;
  }
  Serial.printf("[MASTER] Camera frame received: %u bytes\n",
                (unsigned int)jpeg.size());
  return true;
}

bool askGemini(const std::vector<uint8_t>& jpeg, String& resultMood,
               String& reply) {
  if (String(GEMINI_API_KEY).startsWith("YOUR_")) {
    Serial.println("[MASTER] Set GEMINI_API_KEY in secrets.h");
    return false;
  }
  if (jpeg.empty()) {
    Serial.println("[MASTER] Cannot send an empty image");
    return false;
  }

  size_t encodedCapacity = 4 * ((jpeg.size() + 2) / 3) + 1;
  std::vector<unsigned char> encoded(encodedCapacity);
  size_t encodedLength = 0;
  int encodeResult = mbedtls_base64_encode(
      encoded.data(), encoded.size(), &encodedLength, jpeg.data(), jpeg.size());
  if (encodeResult != 0) {
    Serial.printf("[MASTER] Image encoding failed: %d\n", encodeResult);
    return false;
  }

  String prompt =
      "You are Buddy, a friendly small robot companion. Look at the attached "
      "camera image and the ultrasonic distance, then reply to the user. Do "
      "not identify people or infer sensitive traits. Use ASCII English and "
      "keep the reply under 40 characters. Choose one mood from HAPPY, CALM, "
      "SAD, ANGRY, SURPRISED. Distance is ";
  prompt += distanceCm < 0 ? "unavailable" : String(distanceCm) + " cm";
  prompt += ". Previous reply: ";
  prompt += previousReply.length() > 0 ? previousReply : "none";
  prompt += ". User says: ";
  prompt += pendingUserMessage.length() > 0
      ? pendingUserMessage
      : "Share one friendly observation about what you can see.";
  prompt += ". Return only JSON with keys mood and reply.";

  String body;
  body.reserve(encodedLength + prompt.length() + 400);
  body = "{\"contents\":[{\"parts\":[{\"text\":\"";
  body += jsonEscape(prompt);
  body += "\"},{\"inline_data\":{\"mime_type\":\"image/jpeg\",\"data\":\"";
  body.concat(reinterpret_cast<const char*>(encoded.data()), encodedLength);
  body += "\"}}]}],\"generationConfig\":{\"responseMimeType\":\"application/json\",";
  body += "\"responseSchema\":{\"type\":\"OBJECT\",\"properties\":{";
  body += "\"mood\":{\"type\":\"STRING\",\"enum\":[\"HAPPY\",\"CALM\",\"SAD\",\"ANGRY\",\"SURPRISED\"]},";
  body += "\"reply\":{\"type\":\"STRING\"}},\"required\":[\"mood\",\"reply\"]}}}";
  std::vector<unsigned char>().swap(encoded);

  WiFiClientSecure secureClient;
  secureClient.setCACert(GOOGLE_ROOT_CA);
  HTTPClient geminiHttp;
  String url = "https://generativelanguage.googleapis.com/v1beta/models/";
  url += GEMINI_MODEL;
  url += ":generateContent";
  if (!geminiHttp.begin(secureClient, url)) {
    Serial.println("[MASTER] Could not start Gemini HTTPS request");
    return false;
  }
  geminiHttp.setTimeout(30000);
  geminiHttp.addHeader("Content-Type", "application/json");
  geminiHttp.addHeader("x-goog-api-key", GEMINI_API_KEY);
  int status = geminiHttp.POST(body);
  body = "";
  if (status < 200 || status >= 300) {
    Serial.printf("[MASTER] Gemini HTTP error: %d\n", status);
    String errorBody = geminiHttp.getString();
    Serial.println(errorBody.substring(0, 300));
    geminiHttp.end();
    return false;
  }

  String response = geminiHttp.getString();
  geminiHttp.end();
  String modelText = jsonStringField(response, "text");
  response = "";
  if (modelText.length() == 0) {
    Serial.println("[MASTER] Gemini response did not contain text");
    return false;
  }

  resultMood = jsonStringField(modelText, "mood");
  reply = jsonStringField(modelText, "reply");
  if (resultMood != "HAPPY" && resultMood != "CALM" &&
      resultMood != "SAD" && resultMood != "ANGRY" &&
      resultMood != "SURPRISED") {
    Serial.println("[MASTER] Gemini returned an unsupported mood");
    return false;
  }
  if (reply.length() == 0) {
    Serial.println("[MASTER] Gemini returned an empty reply");
    return false;
  }
  if (reply.length() > 40) {
    reply.remove(40);
  }
  return true;
}

String jsonEscape(const String& value) {
  String escaped;
  escaped.reserve(value.length() + 8);
  for (size_t i = 0; i < value.length(); i++) {
    char character = value[i];
    if (character == '"' || character == '\\') {
      escaped += '\\';
      escaped += character;
    } else if (character == '\n') {
      escaped += "\\n";
    } else if (character == '\r') {
      escaped += "\\r";
    } else if (character == '\t') {
      escaped += "\\t";
    } else if ((uint8_t)character >= 0x20) {
      escaped += character;
    }
  }
  return escaped;
}

String jsonStringField(const String& json, const char* key) {
  String quotedKey = "\"";
  quotedKey += key;
  quotedKey += "\"";
  int position = json.indexOf(quotedKey);
  if (position < 0) return "";
  position = json.indexOf(':', position + quotedKey.length());
  if (position < 0) return "";
  position++;
  while (position < (int)json.length() && isspace(json[position])) {
    position++;
  }
  if (position >= (int)json.length() || json[position] != '"') return "";
  position++;

  String value;
  while (position < (int)json.length()) {
    char character = json[position++];
    if (character == '"') return value;
    if (character == '\\' && position < (int)json.length()) {
      char escaped = json[position++];
      if (escaped == 'n') value += '\n';
      else if (escaped == 'r') value += '\r';
      else if (escaped == 't') value += '\t';
      else if (escaped == '"' || escaped == '\\' || escaped == '/') value += escaped;
      else if (escaped == 'u') {
        if (position + 4 > (int)json.length()) return "";
        position += 4;
        value += '?';
      } else value += escaped;
    } else {
      value += character;
    }
  }
  return "";
}
