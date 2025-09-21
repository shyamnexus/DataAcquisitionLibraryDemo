/*
 * ESP32 SSD1306 I2C Example with Error Handling
 * 
 * This example demonstrates proper I2C configuration for SSD1306 OLED display
 * with comprehensive error handling and troubleshooting features.
 * 
 * Hardware Connections:
 * - ESP32 GPIO 21 (SDA) → SSD1306 SDA
 * - ESP32 GPIO 22 (SCL) → SSD1306 SCL
 * - ESP32 3.3V → SSD1306 VCC
 * - ESP32 GND → SSD1306 GND
 * - Add 4.7kΩ pull-up resistors on SDA and SCL to 3.3V
 * 
 * Libraries Required:
 * - Adafruit SSD1306
 * - Adafruit GFX
 * - Wire (built-in)
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C  // Try 0x3D if 0x3C doesn't work

// I2C configuration
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_FREQUENCY 100000  // Start with 100kHz, increase to 400kHz if stable

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Error tracking
int i2cErrorCount = 0;
unsigned long lastErrorTime = 0;
bool displayInitialized = false;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 SSD1306 I2C Test");
  Serial.println("======================");
  
  // Initialize I2C with error handling
  if (!initializeI2C()) {
    Serial.println("I2C initialization failed!");
    return;
  }
  
  // Scan I2C bus for devices
  scanI2CDevices();
  
  // Initialize display with retry mechanism
  if (!initializeDisplay()) {
    Serial.println("Display initialization failed!");
    return;
  }
  
  displayInitialized = true;
  Serial.println("Display initialized successfully!");
  
  // Display test content
  displayTestContent();
}

void loop() {
  if (displayInitialized) {
    // Update display content every 2 seconds
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 2000) {
      updateDisplayContent();
      lastUpdate = millis();
    }
  } else {
    // Try to reinitialize display every 5 seconds
    static unsigned long lastRetry = 0;
    if (millis() - lastRetry > 5000) {
      Serial.println("Attempting to reinitialize display...");
      if (initializeDisplay()) {
        displayInitialized = true;
        Serial.println("Display reinitialized successfully!");
      }
      lastRetry = millis();
    }
  }
  
  delay(100);
}

bool initializeI2C() {
  Serial.println("Initializing I2C...");
  
  // Configure I2C pins (for ESP32)
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  
  // Set I2C frequency
  Wire.setClock(I2C_FREQUENCY);
  
  // Test I2C communication
  Wire.beginTransmission(SCREEN_ADDRESS);
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.println("I2C communication test successful");
    return true;
  } else {
    Serial.print("I2C communication test failed with error: ");
    Serial.println(error);
    return false;
  }
}

void scanI2CDevices() {
  Serial.println("Scanning I2C devices...");
  
  byte deviceCount = 0;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println();
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("No I2C devices found!");
    Serial.println("Check connections and pull-up resistors");
  } else {
    Serial.print("Found ");
    Serial.print(deviceCount);
    Serial.println(" I2C device(s)");
  }
}

bool initializeDisplay() {
  Serial.println("Initializing SSD1306 display...");
  
  // Try to initialize display with retry mechanism
  for (int attempt = 1; attempt <= 3; attempt++) {
    Serial.print("Attempt ");
    Serial.print(attempt);
    Serial.println("...");
    
    if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println("SSD1306 initialization successful");
      
      // Clear display buffer
      display.clearDisplay();
      display.display();
      
      // Add small delay for display to stabilize
      delay(100);
      
      return true;
    } else {
      Serial.println("SSD1306 initialization failed");
      if (attempt < 3) {
        Serial.println("Retrying in 1 second...");
        delay(1000);
      }
    }
  }
  
  return false;
}

void displayTestContent() {
  if (!displayInitialized) return;
  
  display.clearDisplay();
  
  // Draw test pattern
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ESP32 SSD1306 Test");
  display.println("==================");
  display.println();
  display.println("I2C Address: 0x3C");
  display.println("Status: OK");
  display.println();
  display.println("Time: " + String(millis() / 1000) + "s");
  
  display.display();
}

void updateDisplayContent() {
  if (!displayInitialized) return;
  
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ESP32 SSD1306 Test");
  display.println("==================");
  display.println();
  display.println("I2C Address: 0x3C");
  display.println("Status: Running");
  display.println();
  display.println("Uptime: " + String(millis() / 1000) + "s");
  display.println("Errors: " + String(i2cErrorCount));
  
  display.display();
}

// Error handling function
void handleI2CError() {
  i2cErrorCount++;
  lastErrorTime = millis();
  
  Serial.print("I2C Error #");
  Serial.print(i2cErrorCount);
  Serial.print(" at time: ");
  Serial.println(lastErrorTime);
  
  // If too many errors, try to reinitialize
  if (i2cErrorCount > 10) {
    Serial.println("Too many I2C errors, attempting reinitialization...");
    displayInitialized = false;
    i2cErrorCount = 0;
  }
}