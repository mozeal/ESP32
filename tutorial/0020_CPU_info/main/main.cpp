/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <M5Stack.h>
#include "esp_system.h"
#include "esp_spi_flash.h"

extern "C" void app_main()
{
	M5.begin();

	// LCD display
	M5.Lcd.setTextSize(2);
	M5.Lcd.printf("\nESP32 Info");
	M5.Lcd.printf("\n==========\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    M5.Lcd.printf("ESP32 with %d CPU cores\n",
    		chip_info.cores);
    M5.Lcd.printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    M5.Lcd.printf("WiFi%s%s\n",
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    M5.Lcd.printf("silicon revision %d\n\n", chip_info.revision);
}
