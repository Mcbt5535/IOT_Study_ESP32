#ifndef __MY_OLED_DRIVER_H
#define __MY_OLED_DRIVER_H

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

#define OLED_I2C_SDA_IO 22   // I2C SDA GPIO Number
#define OLED_I2C_SCL_IO 23   // I2C SCL GPIO Number
#define OLED_I2C_POER_NUM 1  // OLED 使用的 I2C 驱动号
#define OLED_I2C_FREQ 400000 // OLED 对应 I2C 时钟
#define OLED_ADDR 0x3c       // OLED I2C 器件地址

#define WRITE_BIT I2C_MASTER_WRITE // I2C master write
#define READ_BIT I2C_MASTER_READ   // I2C master read
#define ACK_CHECK_EN 0x1           // I2C master will check ack from slave
#define ACK_CHECK_DIS 0x0          // I2C master will not check ack from slave
#define ACK_VAL 0x0                // I2C ack value
#define NACK_VAL 0x1               // I2C nack value

// 初始化OLED
//      - 初始化OLED对应的 I2C 接口
//      - 初始化OLED工作状态
// i2c_num : i2c端口号 0/1
esp_err_t oled_init(void);

// 设置显存的起始地址，包括页地址和ram地址。
//      - page_addr,页地址
//      - ram_addr, 此页内的ram地址
esp_err_t oled_set_start_address(uint8_t page_addr, uint16_t ram_addr);

// 刷新一个page的显存
//      - page_addr，页地址
//      - buf, 图像数据
esp_err_t oled_page_refresh(uint8_t page_addr, uint8_t *buf);

// 刷新整个显存 Graphic Display Data RAM (GDDRAM)
//      - buf，图像数据
esp_err_t oled_gddram_refresh(uint8_t *buf);

//-----------------------------------------------------------------------------------------


esp_err_t oled_clear(void);
esp_err_t oled_page_clear(int page);
esp_err_t oled_show_char(uint8_t x_pos, uint8_t page, uint8_t chr, uint8_t size);
esp_err_t oled_show_string(uint8_t x_pos, uint8_t page, char *str, uint8_t size);
esp_err_t oled_show_chinese(uint8_t x_pos, uint8_t page, uint8_t chr);
#endif
