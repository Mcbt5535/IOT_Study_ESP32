#include "my_oled_driver.h"
#include "oledfont.h"
#include "string.h"
// init cmmand
static const uint8_t oled_init_cmd[] = {
    0x80, 0xAE, //--turn off oled panel
    0x80, 0x00, //---set low column address
    0x80, 0x10, //---set high column address
    0x80, 0x40, //--set start line address  Set Mapping RAM Display Start Line
    0x80, 0x81, //--set contrast control register
    0x80, 0xCF, // Set SEG Output Current Brightness
    0x80, 0xA1, //--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    0x80, 0xC8, // Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    0x80, 0xA6, //--set normal display
    0x80, 0xA8, //--set multiplex ratio(1 to 64)
    0x80, 0x3F, //--1/64 duty
    0x80, 0xD3, //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    0x80, 0x00, //-not offset
    0x80, 0xD5, //--set display clock divide ratio/oscillator frequency
    0x80, 0x80, //--set divide ratio, Set Clock as 100 Frames/Sec
    0x80, 0xD9, //--set pre-charge period
    0x80, 0xF1, // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    0x80, 0xDA, //--set com pins hardware configuration
    0x80, 0x12, //
    0x80, 0xDB, //--set vcomh
    0x80, 0x40, // Set VCOM Deselect Level
    0x80, 0x20, //-Set Page Addressing Mode (0x00/0x01/0x02)
    0x80, 0x02, //
    0x80, 0x8D, //--set Charge Pump enable/disable
    0x80, 0x14, //--set(0x10) disable
    0x80, 0xA4, // Disable Entire Display On (0xa4/0xa5)
    0x80, 0xA6, // Disable Inverse Display On (0xa6/a7)
    0x00, 0xAF  //
};

static void disp_buf(uint8_t *buf, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        printf("%02x ", buf[i]);
        if ((i + 1) % 16 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}

static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t oled_init(void)
{
    esp_err_t esp_err;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = OLED_I2C_SDA_IO, // select GPIO specific to your project
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = OLED_I2C_SCL_IO, // select GPIO specific to your project
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = OLED_I2C_FREQ, // select frequency specific to your project
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    if (i2c_param_config(OLED_I2C_POER_NUM, &conf) != ESP_OK)
        return 1;

    if (i2c_driver_install(OLED_I2C_POER_NUM, I2C_MODE_MASTER, 0, 0, 0) != ESP_OK)
        return 2;

    if (i2c_master_write_slave(OLED_I2C_POER_NUM, oled_init_cmd, 56) != ESP_OK)
        return 3;

    return ESP_OK;
}

// 设置page 及其 显存起始地址
esp_err_t oled_set_start_address(uint8_t page_addr, uint16_t ram_addr)
{
    esp_err_t esp_err;
    uint8_t set_addr_cmd[6] = {
        0x80, 0xb0, // Set Page Start Address
        0x80, 0x00, // Set Lower Column Start Address
        0x00, 0x10  // Set Higher Column Start Address
    };
    set_addr_cmd[1] = 0xb0 | page_addr;
    // set_addr_cmd[3] = 0x00 | (ram_addr & 0x00ff);
    // set_addr_cmd[5] = 0x10 | (ram_addr >> 8);
    set_addr_cmd[3] = 0x00 | (ram_addr & 0x0f);
    set_addr_cmd[5] = 0x10 | (ram_addr >> 4);
    // disp_buf(set_addr_cmd, 6);
    esp_err = i2c_master_write_slave(OLED_I2C_POER_NUM, set_addr_cmd, 6);
    // printf("oled_set_start_address: %d \n", esp_err);
    return esp_err;
}

// 刷新一个page
esp_err_t oled_page_refresh(uint8_t page_addr, uint8_t *buf)
{
    esp_err_t esp_err;
    uint8_t refresh_page_cmd[129];
    refresh_page_cmd[0] = 0x40;
    for (size_t i = 1; i < 129; i++)
        refresh_page_cmd[i] = buf[i - 1];
    oled_set_start_address(page_addr, 0);
    // disp_buf(refresh_page_cmd, 129);
    esp_err = i2c_master_write_slave(OLED_I2C_POER_NUM, refresh_page_cmd, 129);
    // printf("oled_page_refresh: %d \n", esp_err);
    return esp_err;
}

// 刷新整个显存 Graphic Display Data RAM (GDDRAM)
esp_err_t oled_gddram_refresh(uint8_t *buf)
{
    esp_err_t esp_err;
    uint8_t refresh_page_gdd[128];
    for (uint8_t i = 0; i < 8; i++)
    {
        for (size_t j = 0; j < 128; j++)
        {
            refresh_page_gdd[j] = buf[i * 128 + j];
        }
        esp_err = oled_page_refresh(i, refresh_page_gdd);
    }
    return esp_err;
}

//--------------------------------------------------------------------------------------------
// 清屏
esp_err_t oled_clear(void)
{
    esp_err_t esp_err;
    uint8_t refresh_page_gdd[128];
    for (uint8_t i = 0; i < 8; i++)
    {
        for (size_t j = 0; j < 128; j++)
        {
            refresh_page_gdd[j] = 0x00;
        }
        esp_err = oled_page_refresh(i, refresh_page_gdd);
    }
    return esp_err;
}

// 清page(page)
esp_err_t oled_page_clear(int page)
{
    esp_err_t esp_err;
    uint8_t refresh_page_gdd[128];
    for (size_t j = 0; j < 128; j++)
    {
        refresh_page_gdd[j] = 0x00;
    }
    esp_err = oled_page_refresh(page, refresh_page_gdd);
    return esp_err;
}

// 显示字符(x, page, char, size)
//      - x, x位置(0-(127-size))
//      - page, page(0-7)
//      - char, 要显示的字符
//      - size, 字符高度(8、16)
esp_err_t oled_show_char(uint8_t x_pos, uint8_t page, uint8_t chr, uint8_t size)
{
    esp_err_t esp_err;
    uint8_t c = 0, i = 0;
    uint8_t refresh_page_cmd[128];
    c = chr - ' '; //得到要输入的字符在表中的位置
    refresh_page_cmd[0] = 0x40;

    if (size == 8)
    {
        for (i = 0; i < 6; i++)
            refresh_page_cmd[i + 1] = F6x8[c][i];
        oled_set_start_address(page, x_pos);
        esp_err = i2c_master_write_slave(OLED_I2C_POER_NUM, refresh_page_cmd, 7);
    }
    else
    {
        for (i = 0; i < 8; i++)
            refresh_page_cmd[i + 1] = F8X16[c * 16 + i];
        oled_set_start_address(page, x_pos);
        esp_err = i2c_master_write_slave(OLED_I2C_POER_NUM, refresh_page_cmd, 9);
        for (i = 0; i < 8; i++)
            refresh_page_cmd[i + 1] = F8X16[c * 16 + i + 8];
        oled_set_start_address(page + 1, x_pos);
        esp_err = i2c_master_write_slave(OLED_I2C_POER_NUM, refresh_page_cmd, 9);
    }
    return esp_err;
}

// 显示字符串(x, page, str, size)
//      - x, x位置(0-(127-size))
//      - page, page(0-7)
//      - str, 要显示的字符串
//      - size, 字符高度(8、16)
esp_err_t oled_show_string(uint8_t x_pos, uint8_t page, char *str, uint8_t size)
{
    esp_err_t esp_err;
    uint8_t c = 0, i = 0, l = strlen(str);
    uint8_t refresh_page_cmd[128];
    refresh_page_cmd[0] = 0x40;

    if (size == 8)
    {
        for (i = 0; i < l; i++)
            oled_show_char(x_pos + 8 * i, page, str[i], size);
        esp_err = i2c_master_write_slave(OLED_I2C_POER_NUM, refresh_page_cmd, sizeof(str));
    }
    else
    {
        for (i = 0; i < l; i++)
            oled_show_char(x_pos + 8 * i, page, str[i], size);
        esp_err = i2c_master_write_slave(OLED_I2C_POER_NUM, refresh_page_cmd, sizeof(str));
    }

    return esp_err;
}

// 显示一个汉字(x, page, char)
//      - x, x位置(0-(127-size))
//      - page, page(0-7)
//      - char, 要显示的汉字序号
esp_err_t oled_show_chinese(uint8_t x_pos, uint8_t page, uint8_t chr)
{
    esp_err_t esp_err;
    uint8_t c = 0, i = 0;
    uint8_t refresh_page_cmd[128];
    refresh_page_cmd[0] = 0x40;
    for (i = 0; i < 16; i++)
        refresh_page_cmd[i + 1] = Hzk[chr * 2][i];
    oled_set_start_address(page, x_pos);
    esp_err = i2c_master_write_slave(OLED_I2C_POER_NUM, refresh_page_cmd, 17);
    for (i = 0; i < 16; i++)
        refresh_page_cmd[i + 1] = Hzk[chr * 2 + 1][i];
    oled_set_start_address(page + 1, x_pos);
    esp_err = i2c_master_write_slave(OLED_I2C_POER_NUM, refresh_page_cmd, 17);
    return esp_err;
}
