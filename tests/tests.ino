#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(18, 19); // SDA = GPIO 18, SCL = GPIO 19
  Serial.println("\n--- I2C Scanner ---");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("Found device at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;
    }
  }

  if (nDevices == 0) Serial.println("No I2C devices found!");
  delay(3000);
}