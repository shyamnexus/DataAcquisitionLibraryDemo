/*
 * I2C Scanner for ESP32
 * This tool helps diagnose I2C communication issues
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "I2C_SCANNER";

// I2C Configuration
#define I2C_MASTER_SCL_IO           22    /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21    /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0     /*!< I2C master i2c port number */
#define I2C_MASTER_FREQ_HZ          100000 /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0     /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0     /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

/**
 * @brief Initialize I2C master
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

/**
 * @brief Scan for I2C devices
 */
static void i2c_scanner(void)
{
    ESP_LOGI(TAG, "Starting I2C scanner...");
    ESP_LOGI(TAG, "Scanning I2C bus on SDA=%d, SCL=%d", I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
    
    int devices_found = 0;
    
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "I2C device found at address 0x%02X", addr);
            devices_found++;
            
            // Check if it's a known device
            if (addr == 0x3C || addr == 0x3D) {
                ESP_LOGI(TAG, "  -> This could be an SSD1306 OLED display");
            } else if (addr == 0x48) {
                ESP_LOGI(TAG, "  -> This could be a temperature sensor (DS18B20)");
            } else if (addr == 0x50) {
                ESP_LOGI(TAG, "  -> This could be an EEPROM");
            } else if (addr == 0x68) {
                ESP_LOGI(TAG, "  -> This could be an RTC (DS1307)");
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10)); // Small delay between scans
    }
    
    if (devices_found == 0) {
        ESP_LOGW(TAG, "No I2C devices found!");
        ESP_LOGW(TAG, "Check your wiring:");
        ESP_LOGW(TAG, "  - SDA connected to GPIO %d", I2C_MASTER_SDA_IO);
        ESP_LOGW(TAG, "  - SCL connected to GPIO %d", I2C_MASTER_SCL_IO);
        ESP_LOGW(TAG, "  - Pull-up resistors (4.7kΩ) on SDA and SCL");
        ESP_LOGW(TAG, "  - Power supply (3.3V)");
    } else {
        ESP_LOGI(TAG, "Scan complete. Found %d device(s)", devices_found);
    }
}

/**
 * @brief Test I2C communication with specific address
 */
static void test_i2c_communication(uint8_t address)
{
    ESP_LOGI(TAG, "Testing communication with device at 0x%02X", address);
    
    // Test 1: Basic write
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true); // Send a simple command
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✓ Basic write test passed for 0x%02X", address);
    } else {
        ESP_LOGE(TAG, "✗ Basic write test failed for 0x%02X: %s", address, esp_err_to_name(ret));
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Test 2: Read test (if device supports it)
    uint8_t data;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✓ Basic read test passed for 0x%02X (data: 0x%02X)", address, data);
    } else {
        ESP_LOGW(TAG, "⚠ Basic read test failed for 0x%02X: %s (this may be normal for some devices)", address, esp_err_to_name(ret));
    }
}

/**
 * @brief Test different I2C clock speeds
 */
static void test_i2c_speeds(void)
{
    ESP_LOGI(TAG, "Testing different I2C clock speeds...");
    
    uint32_t speeds[] = {100000, 400000, 1000000};
    const char* speed_names[] = {"100kHz", "400kHz", "1MHz"};
    
    for (int i = 0; i < 3; i++) {
        ESP_LOGI(TAG, "Testing at %s", speed_names[i]);
        
        // Reconfigure I2C with new speed
        i2c_driver_delete(I2C_MASTER_NUM);
        vTaskDelay(pdMS_TO_TICKS(100));
        
        i2c_config_t conf = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = I2C_MASTER_SDA_IO,
            .scl_io_num = I2C_MASTER_SCL_IO,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master.clk_speed = speeds[i],
        };
        
        i2c_param_config(I2C_MASTER_NUM, &conf);
        esp_err_t ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
        
        if (ret == ESP_OK) {
            // Test communication with SSD1306 addresses
            test_i2c_communication(0x3C);
            test_i2c_communication(0x3D);
        } else {
            ESP_LOGE(TAG, "Failed to configure I2C at %s", speed_names[i]);
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief Main application
 */
void app_main(void)
{
    ESP_LOGI(TAG, "I2C Diagnostic Tool Starting");
    
    // Initialize I2C
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C master init failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "I2C master initialized successfully");
    
    // Wait a bit for I2C to stabilize
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Run I2C scanner
    i2c_scanner();
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Test specific addresses
    ESP_LOGI(TAG, "Testing specific SSD1306 addresses...");
    test_i2c_communication(0x3C);
    test_i2c_communication(0x3D);
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Test different clock speeds
    test_i2c_speeds();
    
    ESP_LOGI(TAG, "I2C diagnostic complete");
}