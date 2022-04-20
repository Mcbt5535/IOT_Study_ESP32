#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "my_oled_driver.h"
#include "my_table.h"

void app_main(void)
{
    char temp[100] = {0};
    int count = 0;
    oled_init();
    oled_clear();

    oled_show_string(10, 0, "hello world!", 16);
    for (int i = 0; i < 8; i++)
    {
        oled_show_char(i * 8, 2, 48 + i, 8);
    }
    for (int i = 0; i < 8; i++)
    {
        oled_show_char(i * 8, 3, 65 + i, 8);
    }
    for (int i = 0; i < 8; i++)
    {
        oled_show_char(i * 8, 3, 97 + i, 8);
    }
    oled_show_chinese(20, 6, 0);
    oled_show_chinese(40, 6, 1);
    oled_show_chinese(60, 6, 2);
    oled_show_chinese(80, 6, 3);
    // oled_gddram_refresh(Finish);
    while (1)
    {

        
    }
}
