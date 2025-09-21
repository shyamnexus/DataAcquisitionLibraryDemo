/*
 * ESP32 I2C Scanner
 * 
 * This utility scans the I2C bus to detect connected devices.
 * Use this to verify your SSD1306 is properly connected and detected.
 * 
 * Hardware Connections:
 * - ESP32 GPIO 21 (SDA) → SSD1306 SDA
 * - ESP32 GPIO 22 (SCL) → SSD1306 SCL
 * - ESP32 3.3V → SSD1306 VCC
 * - ESP32 GND → SSD1306 GND
 * - Add 4.7kΩ pull-up resistors on SDA and SCL to 3.3V
 */

#include <Wire.h>

// I2C configuration
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_FREQUENCY 100000  // Start with 100kHz

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("ESP32 I2C Scanner");
  Serial.println("================");
  Serial.println();
  
  // Initialize I2C
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_FREQUENCY);
  
  Serial.println("I2C initialized with:");
  Serial.print("SDA Pin: ");
  Serial.println(I2C_SDA_PIN);
  Serial.print("SCL Pin: ");
  Serial.println(I2C_SCL_PIN);
  Serial.print("Frequency: ");
  Serial.print(I2C_FREQUENCY);
  Serial.println(" Hz");
  Serial.println();
  
  delay(1000); // Give devices time to initialize
}

void loop() {
  Serial.println("Scanning I2C bus...");
  Serial.println();
  
  byte deviceCount = 0;
  byte errorCount = 0;
  
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.print(" (");
      Serial.print(address);
      Serial.print(")");
      
      // Check if this might be an SSD1306
      if (address == 0x3C || address == 0x3D) {
        Serial.print(" - Likely SSD1306 OLED");
      }
      
      Serial.println();
      deviceCount++;
    } else if (error == 4) {
      errorCount++;
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println();
    }
  }
  
  Serial.println();
  Serial.print("Scan complete. Found ");
  Serial.print(deviceCount);
  Serial.print(" device(s)");
  
  if (errorCount > 0) {
    Serial.print(", ");
    Serial.print(errorCount);
    Serial.print(" error(s)");
  }
  
  Serial.println();
  Serial.println();
  
  if (deviceCount == 0) {
    Serial.println("No I2C devices found!");
    Serial.println();
    Serial.println("Troubleshooting checklist:");
    Serial.println("1. Check all connections (SDA, SCL, VCC, GND)");
    Serial.println("2. Verify power supply (3.3V or 5V)");
    Serial.println("3. Add pull-up resistors (4.7kΩ on SDA and SCL)");
    Serial.println("4. Check if device is powered on");
    Serial.println("5. Try different I2C address (0x3D instead of 0x3C)");
    Serial.println("6. Lower I2C frequency");
    Serial.println();
  } else {
    Serial.println("I2C devices detected successfully!");
    Serial.println("You can now use the main SSD1306 example code.");
  }
  
  Serial.println("Rescanning in 5 seconds...");
  Serial.println("================================================");
  delay(5000);
}