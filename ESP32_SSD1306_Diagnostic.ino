/*
 * ESP32 SSD1306 Diagnostic Tool
 * 
 * This diagnostic tool helps identify specific issues with your SSD1306 setup.
 * Run this code to get detailed information about your I2C communication.
 * 
 * Features:
 * - I2C bus scanning
 * - Device detection
 * - Error analysis
 * - Hardware verification
 * - Connection testing
 */

#include <Wire.h>

// I2C configuration
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_FREQUENCY 100000  // Start with 100kHz

// Test addresses for SSD1306
#define SSD1306_ADDR_1 0x3C
#define SSD1306_ADDR_2 0x3D

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("ESP32 SSD1306 Diagnostic Tool");
  Serial.println("=============================");
  Serial.println();
  
  // Run comprehensive diagnostics
  runDiagnostics();
}

void loop() {
  // Run diagnostics every 10 seconds
  static unsigned long lastRun = 0;
  if (millis() - lastRun > 10000) {
    Serial.println("\nRe-running diagnostics...");
    Serial.println("=========================");
    runDiagnostics();
    lastRun = millis();
  }
  
  delay(100);
}

void runDiagnostics() {
  Serial.println("Starting comprehensive diagnostics...");
  Serial.println();
  
  // Step 1: I2C initialization test
  testI2CInitialization();
  
  // Step 2: I2C bus scan
  scanI2CBus();
  
  // Step 3: SSD1306 specific tests
  testSSD1306Communication();
  
  // Step 4: Error analysis
  analyzeErrors();
  
  // Step 5: Recommendations
  provideRecommendations();
  
  Serial.println("\nDiagnostics complete.");
  Serial.println("===================");
}

void testI2CInitialization() {
  Serial.println("1. Testing I2C Initialization...");
  
  // Initialize I2C
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_FREQUENCY);
  
  Serial.print("   SDA Pin: GPIO ");
  Serial.println(I2C_SDA_PIN);
  Serial.print("   SCL Pin: GPIO ");
  Serial.println(I2C_SCL_PIN);
  Serial.print("   Frequency: ");
  Serial.print(I2C_FREQUENCY);
  Serial.println(" Hz");
  
  // Test basic I2C functionality
  Wire.beginTransmission(0x00);  // Test with non-existent address
  byte error = Wire.endTransmission();
  
  if (error == 2) {
    Serial.println("   ✓ I2C initialization successful");
  } else {
    Serial.print("   ✗ I2C initialization failed (error: ");
    Serial.print(error);
    Serial.println(")");
  }
  
  Serial.println();
}

void scanI2CBus() {
  Serial.println("2. Scanning I2C Bus...");
  
  byte deviceCount = 0;
  byte errorCount = 0;
  bool ssd1306Found = false;
  
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("   ✓ Device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.print(" (");
      Serial.print(address);
      Serial.print(")");
      
      // Check if this is likely an SSD1306
      if (address == SSD1306_ADDR_1 || address == SSD1306_ADDR_2) {
        Serial.print(" - SSD1306 OLED");
        ssd1306Found = true;
      }
      
      Serial.println();
      deviceCount++;
    } else if (error == 4) {
      errorCount++;
    }
  }
  
  Serial.println();
  Serial.print("   Found ");
  Serial.print(deviceCount);
  Serial.print(" device(s)");
  
  if (errorCount > 0) {
    Serial.print(", ");
    Serial.print(errorCount);
    Serial.print(" error(s)");
  }
  
  Serial.println();
  
  if (deviceCount == 0) {
    Serial.println("   ✗ No I2C devices detected!");
  } else if (ssd1306Found) {
    Serial.println("   ✓ SSD1306 detected!");
  } else {
    Serial.println("   ⚠ I2C devices found, but no SSD1306 detected");
  }
  
  Serial.println();
}

void testSSD1306Communication() {
  Serial.println("3. Testing SSD1306 Communication...");
  
  // Test both common SSD1306 addresses
  bool addr1Working = testAddress(SSD1306_ADDR_1);
  bool addr2Working = testAddress(SSD1306_ADDR_2);
  
  if (addr1Working) {
    Serial.println("   ✓ SSD1306 found at address 0x3C");
  } else if (addr2Working) {
    Serial.println("   ✓ SSD1306 found at address 0x3D");
  } else {
    Serial.println("   ✗ SSD1306 not responding at either address");
    Serial.println("   ✗ Address 0x3C: Not responding");
    Serial.println("   ✗ Address 0x3D: Not responding");
  }
  
  Serial.println();
}

bool testAddress(byte address) {
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    // Try to read from the device
    Wire.requestFrom(address, 1);
    if (Wire.available()) {
      return true;
    }
  }
  
  return false;
}

void analyzeErrors() {
  Serial.println("4. Error Analysis...");
  
  // Check for common error patterns
  bool hasNackErrors = false;
  bool hasTimeoutErrors = false;
  bool hasBusErrors = false;
  
  // Test for NACK errors
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    
    if (error == 2) {
      hasNackErrors = true;
    } else if (error == 3) {
      hasTimeoutErrors = true;
    } else if (error == 4) {
      hasBusErrors = true;
    }
  }
  
  if (hasNackErrors) {
    Serial.println("   ⚠ NACK errors detected - device not responding");
  }
  
  if (hasTimeoutErrors) {
    Serial.println("   ⚠ Timeout errors detected - communication too slow");
  }
  
  if (hasBusErrors) {
    Serial.println("   ⚠ Bus errors detected - hardware connection issues");
  }
  
  if (!hasNackErrors && !hasTimeoutErrors && !hasBusErrors) {
    Serial.println("   ✓ No communication errors detected");
  }
  
  Serial.println();
}

void provideRecommendations() {
  Serial.println("5. Recommendations...");
  
  // Check if SSD1306 was found
  bool ssd1306Found = false;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0 && (address == SSD1306_ADDR_1 || address == SSD1306_ADDR_2)) {
      ssd1306Found = true;
      break;
    }
  }
  
  if (ssd1306Found) {
    Serial.println("   ✓ SSD1306 is working correctly!");
    Serial.println("   ✓ You can now use the main example code");
    Serial.println("   ✓ Make sure to use the correct I2C address in your code");
  } else {
    Serial.println("   ✗ SSD1306 not detected. Check the following:");
    Serial.println("   • Verify all connections (SDA, SCL, VCC, GND)");
    Serial.println("   • Add pull-up resistors (4.7kΩ on SDA and SCL)");
    Serial.println("   • Check power supply (3.3V or 5V)");
    Serial.println("   • Verify I2C pins (GPIO 21/22 for most ESP32 boards)");
    Serial.println("   • Try different I2C frequency (100kHz)");
    Serial.println("   • Check if display is powered on");
  }
  
  Serial.println();
}