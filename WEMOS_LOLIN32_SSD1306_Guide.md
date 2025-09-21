# WEMOS LOLIN32 + SSD1306 I2C Troubleshooting Guide

## LOLIN32 Board Specific Information

The WEMOS LOLIN32 board is based on the ESP32 and has specific pin configurations for I2C communication.

### Default I2C Pins for LOLIN32:
- **SDA (Data)**: GPIO 21
- **SCL (Clock)**: GPIO 22
- **VCC**: 3.3V (NOT 5V)
- **GND**: Ground

### LOLIN32 Pinout Reference:
```
LOLIN32 Pin Layout:
┌─────────────────┐
│ 3V3  GND  GPIO23│
│ GND  GPIO22 SCL │  ← I2C Clock
│ GPIO21 SDA │  ← I2C Data  
│ GND  GPIO19 GND │
│ GND  GPIO18 GND │
│ GND  GPIO5  GND │
│ GND  GPIO17 GND │
│ GND  GPIO16 GND │
│ GND  GPIO4  GND │
│ GND  GPIO0  GND │
│ GND  GPIO2  GND │
│ GND  GPIO15 GND │
│ GND  GPIO13 GND │
│ GND  GPIO12 GND │
│ GND  GPIO14 GND │
│ GND  GPIO27 GND │
│ GND  GPIO26 GND │
│ GND  GPIO25 GND │
│ GND  GPIO33 GND │
│ GND  GPIO32 GND │
│ GND  GPIO35 GND │
│ GND  GPIO34 GND │
└─────────────────┘
```

## Common LOLIN32 + SSD1306 Issues

### 1. Power Supply Issues
**Problem**: LOLIN32 provides 3.3V, but some SSD1306 modules expect 5V
**Solution**: 
- Use 3.3V compatible SSD1306 modules
- If using 5V module, add level shifter
- Check voltage with multimeter

### 2. Pull-up Resistor Issues
**Problem**: LOLIN32 doesn't have built-in pull-ups for I2C
**Solution**: Add external 4.7kΩ pull-up resistors:
- SDA → 3.3V (via 4.7kΩ resistor)
- SCL → 3.3V (via 4.7kΩ resistor)

### 3. GPIO Configuration
**Problem**: Using wrong GPIO pins
**Solution**: Use correct pins:
- SDA: GPIO 21
- SCL: GPIO 22
- NOT GPIO 4/5 (which you were using)

### 4. I2C Address Issues
**Problem**: Wrong I2C address
**Solution**: Try both addresses:
- 0x3C (most common)
- 0x3D (alternative)

## Wiring Diagram for LOLIN32 + SSD1306

```
LOLIN32          SSD1306
------           -------
3.3V      →      VCC
GND       →      GND
GPIO 21   →      SDA
GPIO 22   →      SCL

Pull-up resistors:
SDA → 3.3V (4.7kΩ)
SCL → 3.3V (4.7kΩ)
```

## Troubleshooting Steps

1. **Verify Connections**:
   - Double-check all wiring
   - Ensure no loose connections
   - Verify 3.3V power supply

2. **Add Pull-up Resistors**:
   - Essential for LOLIN32
   - Use 4.7kΩ resistors
   - Connect to 3.3V

3. **Test with I2C Scanner**:
   - Use the provided scanner code
   - Check for device detection
   - Verify correct address

4. **Check Power Supply**:
   - Measure 3.3V with multimeter
   - Ensure stable power
   - Check for voltage drops

## LOLIN32 Specific Code Configuration

The code should use:
```c
#define I2C_MASTER_SDA_IO           21    // GPIO 21 for SDA
#define I2C_MASTER_SCL_IO           22    // GPIO 22 for SCL
```

## Common Error Solutions

### Error: "I2C hardware NACK detected"
- **Cause**: No pull-up resistors or wrong address
- **Solution**: Add 4.7kΩ pull-ups, check address

### Error: "ESP_ERR_INVALID_STATE"
- **Cause**: Wrong GPIO configuration
- **Solution**: Use GPIO 21/22, not 4/5

### Error: "Could not write to device"
- **Cause**: Power supply or wiring issues
- **Solution**: Check 3.3V supply, verify connections