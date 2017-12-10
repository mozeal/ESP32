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
#include "driver/gpio.h"
#include "sdkconfig.h"

/* Can run 'make menuconfig' to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define OUTPUT_GPIO CONFIG_OUTPUT_GPIO
#define GPIO_INPUT_IO_0 37
#define GPIO_INPUT_IO_1 38
#define GPIO_INPUT_IO_2 39

#define GPIO_INPUT_PIN_SEL  ((((uint64_t)1)<<GPIO_INPUT_IO_0) | (((uint64_t)1)<<GPIO_INPUT_IO_1) | (((uint64_t)1)<<GPIO_INPUT_IO_2) )

void blink_task(void *pvParameter)
{
    /* Configure the IOMUX register for pad OUTPUT_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(OUTPUT_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction((gpio_num_t)OUTPUT_GPIO, GPIO_MODE_OUTPUT);
    while(1) {
        /* Blink off (output low) */
        gpio_set_level((gpio_num_t)OUTPUT_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level((gpio_num_t)OUTPUT_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;

    M5.Lcd.printf( "Interrupt: %i\n", gpio_num );
    //xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

extern "C" void app_main()
{
	M5.begin();

	// LCD display
	M5.Lcd.setTextSize(2);
	M5.Lcd.printf("\nESP32 GPIO");
	M5.Lcd.printf("\n==========\n");
	M5.Lcd.printf("GPIO: %i\n", OUTPUT_GPIO);

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

	xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}
