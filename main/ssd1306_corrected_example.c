/*
 * SSD1306 I2C Communication - Corrected Example
 * This example addresses common I2C communication issues
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "SSD1306";

// I2C Configuration for WEMOS LOLIN32
#define I2C_MASTER_SCL_IO           22    /*!< GPIO number used for I2C master clock (LOLIN32 default) */
#define I2C_MASTER_SDA_IO           21    /*!< GPIO number used for I2C master data (LOLIN32 default) */
#define I2C_MASTER_NUM              0     /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          100000 /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0     /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0     /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

// SSD1306 Configuration
#define SSD1306_I2C_ADDR            0x3C  /*!< I2C address for SSD1306 */
#define SSD1306_WIDTH               128
#define SSD1306_HEIGHT              64

// SSD1306 Commands
#define SSD1306_CMD_SET_MEMORY_ADDR_MODE    0x20
#define SSD1306_CMD_SET_COLUMN_ADDR         0x21
#define SSD1306_CMD_SET_PAGE_ADDR           0x22
#define SSD1306_CMD_SET_DISPLAY_START_LINE  0x40
#define SSD1306_CMD_SET_SEGMENT_REMAP       0xA0
#define SSD1306_CMD_SET_MULTIPLEX_RATIO     0xA8
#define SSD1306_CMD_SET_COM_SCAN_DIR        0xC0
#define SSD1306_CMD_SET_DISPLAY_OFFSET      0xD3
#define SSD1306_CMD_SET_CLOCK_DIV           0xD5
#define SSD1306_CMD_SET_PRECHARGE_PERIOD    0xD9
#define SSD1306_CMD_SET_COM_PINS            0xDA
#define SSD1306_CMD_SET_VCOMH_DESELECT      0xDB
#define SSD1306_CMD_SET_CONTRAST            0x81
#define SSD1306_CMD_SET_ENTIRE_ON           0xA4
#define SSD1306_CMD_SET_NORMAL_INV           0xA6
#define SSD1306_CMD_SET_DISPLAY_ON          0xAF
#define SSD1306_CMD_SET_DISPLAY_OFF         0xAE

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
 * @brief Write a command to SSD1306
 */
static esp_err_t ssd1306_write_command(uint8_t command)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SSD1306_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true); // Control byte: Co=0, D/C#=0
    i2c_master_write_byte(cmd, command, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief Write data to SSD1306
 */
static esp_err_t ssd1306_write_data(uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SSD1306_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x40, true); // Control byte: Co=0, D/C#=1
    i2c_master_write(cmd, data, len, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief Initialize SSD1306 display
 */
static esp_err_t ssd1306_init(void)
{
    esp_err_t ret = ESP_OK;
    
    // Display off
    ret = ssd1306_write_command(SSD1306_CMD_SET_DISPLAY_OFF);
    if (ret != ESP_OK) return ret;
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Set display clock divide ratio and oscillator frequency
    ret = ssd1306_write_command(SSD1306_CMD_SET_CLOCK_DIV);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_write_command(0x80);
    if (ret != ESP_OK) return ret;
    
    // Set multiplex ratio
    ret = ssd1306_write_command(SSD1306_CMD_SET_MULTIPLEX_RATIO);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_write_command(SSD1306_HEIGHT - 1);
    if (ret != ESP_OK) return ret;
    
    // Set display offset
    ret = ssd1306_write_command(SSD1306_CMD_SET_DISPLAY_OFFSET);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_write_command(0x00);
    if (ret != ESP_OK) return ret;
    
    // Set start line
    ret = ssd1306_write_command(SSD1306_CMD_SET_DISPLAY_START_LINE | 0x0);
    if (ret != ESP_OK) return ret;
    
    // Enable charge pump
    ret = ssd1306_write_command(0x8D);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_write_command(0x14);
    if (ret != ESP_OK) return ret;
    
    // Set memory addressing mode
    ret = ssd1306_write_command(SSD1306_CMD_SET_MEMORY_ADDR_MODE);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_write_command(0x00); // Horizontal addressing mode
    if (ret != ESP_OK) return ret;
    
    // Set segment re-map
    ret = ssd1306_write_command(SSD1306_CMD_SET_SEGMENT_REMAP | 0x1);
    if (ret != ESP_OK) return ret;
    
    // Set COM scan direction
    ret = ssd1306_write_command(SSD1306_CMD_SET_COM_SCAN_DIR);
    if (ret != ESP_OK) return ret;
    
    // Set COM pins configuration
    ret = ssd1306_write_command(SSD1306_CMD_SET_COM_PINS);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_write_command(0x12);
    if (ret != ESP_OK) return ret;
    
    // Set contrast
    ret = ssd1306_write_command(SSD1306_CMD_SET_CONTRAST);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_write_command(0xCF);
    if (ret != ESP_OK) return ret;
    
    // Set pre-charge period
    ret = ssd1306_write_command(SSD1306_CMD_SET_PRECHARGE_PERIOD);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_write_command(0xF1);
    if (ret != ESP_OK) return ret;
    
    // Set VCOMH deselect level
    ret = ssd1306_write_command(SSD1306_CMD_SET_VCOMH_DESELECT);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_write_command(0x40);
    if (ret != ESP_OK) return ret;
    
    // Set entire display on
    ret = ssd1306_write_command(SSD1306_CMD_SET_ENTIRE_ON);
    if (ret != ESP_OK) return ret;
    
    // Set normal/inverse display
    ret = ssd1306_write_command(SSD1306_CMD_SET_NORMAL_INV);
    if (ret != ESP_OK) return ret;
    
    // Display on
    ret = ssd1306_write_command(SSD1306_CMD_SET_DISPLAY_ON);
    if (ret != ESP_OK) return ret;
    
    return ret;
}

/**
 * @brief Clear the display
 */
static esp_err_t ssd1306_clear(void)
{
    uint8_t zeros[SSD1306_WIDTH];
    memset(zeros, 0, SSD1306_WIDTH);
    
    esp_err_t ret = ssd1306_write_command(SSD1306_CMD_SET_COLUMN_ADDR);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_write_command(0);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_write_command(SSD1306_WIDTH - 1);
    if (ret != ESP_OK) return ret;
    
    ret = ssd1306_write_command(SSD1306_CMD_SET_PAGE_ADDR);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_write_command(0);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_write_command((SSD1306_HEIGHT / 8) - 1);
    if (ret != ESP_OK) return ret;
    
    for (int i = 0; i < SSD1306_HEIGHT / 8; i++) {
        ret = ssd1306_write_data(zeros, SSD1306_WIDTH);
        if (ret != ESP_OK) return ret;
    }
    
    return ret;
}

/**
 * @brief Main application
 */
void app_main(void)
{
    ESP_LOGI(TAG, "Starting SSD1306 I2C Test");
    
    // Initialize I2C
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C master init failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "I2C master initialized successfully");
    
    // Initialize SSD1306
    ret = ssd1306_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SSD1306 init failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "SSD1306 initialized successfully");
    
    // Clear display
    ret = ssd1306_clear();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SSD1306 clear failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "Display cleared successfully");
    
    ESP_LOGI(TAG, "SSD1306 test completed successfully!");
}