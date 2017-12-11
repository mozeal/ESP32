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
#include "Display.h"
#include "Free_Fonts.h"
#include "driver/gpio.h"

#define GPIO_INPUT_IO_0 37
#define GPIO_INPUT_IO_1 38
#define GPIO_INPUT_IO_2 39

#define GPIO_INPUT_PIN_SEL  ((((uint64_t)1)<<GPIO_INPUT_IO_0) | (((uint64_t)1)<<GPIO_INPUT_IO_1) | (((uint64_t)1)<<GPIO_INPUT_IO_2) )

int current_index = 0;
const char *_fontname[12] = {
	"Mono 9", "Mono 12", "Mono 18", "Mono 24",
	"Sans Serif 9", "Sans Serif 12", "Sans Serif 18", "Sans Serif 24",
	"Serif 9", "Serif 12", "Serif 18", "Serif 24"
};
const GFXfont *_font[12][4] {
	{FM9,FMB9,FMO9,FMBO9},
	{FM12,FMB12,FMO12,FMBO12},
	{FM18,FMB18,FMO18,FMBO18},
	{FM24,FMB24,FMO24,FMBO24},
	{FSS9,FSSB9,FSSO9,FSSBO9},
	{FSS12,FSSB12,FSSO12,FSSBO12},
	{FSS18,FSSB18,FSSO18,FSSBO18},
	{FSS24,FSSB24,FSSO24,FSSBO24},
	{FS9,FSB9,FSI9,FSBI9},
	{FS12,FSB12,FSI12,FSBI12},
	{FS18,FSB18,FSI19,FSBI18},
	{FS24,FSB24,FSI24,FSBI24}
};

void _DisplayFont( const char *fontname,
		const GFXfont *normal, const GFXfont *bold,
		const GFXfont *italic, const GFXfont *bolditalic ) {

	M5.Lcd.setFreeFont(normal);
	M5.Lcd.setTextColor(WHITE);
	M5.Lcd.setCursor(0, 0);
	M5.Lcd.printf("\n%s\n",fontname);
	M5.Lcd.setFreeFont(bold);
	M5.Lcd.printf("Bold\n");
	M5.Lcd.setFreeFont(italic);
	M5.Lcd.printf("Italic\n");
	M5.Lcd.setFreeFont(bolditalic);
	M5.Lcd.printf("BoldItalic\n");
}

void DisplayFont( int i ) {
	M5.Lcd.fillScreen(BLACK);

	_DisplayFont(_fontname[i],_font[i][0],_font[i][1],_font[i][2],_font[i][3]);
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;

    //M5.Lcd.printf( "Interrupt: %i\n", gpio_num );
    //xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    current_index++;
    if( current_index >= 12 )
    		current_index = 0;
    DisplayFont(current_index);
}

extern "C" void app_main()
{
	M5.begin();

	//M5.lcd.drawBitmap(0, 0, 320, 240, (uint16_t *)gImage_logoM5);
	//delay(500);

	// Lcd display
	M5.Lcd.fillScreen(WHITE);
	delay(100);
	M5.Lcd.fillScreen(RED);
	delay(100);
	M5.Lcd.fillScreen(GREEN);
	delay(100);
	M5.Lcd.fillScreen(BLUE);
	delay(100);
	M5.Lcd.fillScreen(BLACK);
	delay(100);

	DisplayFont(current_index);

	gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_EDGE|ESP_INTR_FLAG_LOWMED);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add((gpio_num_t)GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add((gpio_num_t)GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add((gpio_num_t)GPIO_INPUT_IO_2, gpio_isr_handler, (void*) GPIO_INPUT_IO_2);

}
