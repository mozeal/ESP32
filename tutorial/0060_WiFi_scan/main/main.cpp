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
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"

const char *auth_mode_name[] = {
	"open",
	"WEP",
	"WPA",
	"WPA2",
	"WPA/WPA2",
	"WPA2 ENT",
	"Unknown"
};

const char *cipher_name[] = {
	"none",
	"WEP40",
	"WEP104",
	"TKIP",
	"CCMP",
	"TKIP/CCMP",
	"UNKNOWN"
};

#define MAX_APs 32

bool ScanDone = false;
bool WiFiReady = false;

static const char *TAG = "scan";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            //ESP_ERROR_CHECK(esp_wifi_connect());
            WiFiReady = true;
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            ESP_LOGI(TAG, "Got IP: %s\n",
                     ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            //ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_SCAN_DONE:
			ESP_LOGI(TAG, "SYSTEM_EVENT_SCAN_DONE");
			ScanDone = true;
			break;
		default:
			ESP_LOGI(TAG, "UNKNOWN");

            break;
    }
    return ESP_OK;
}

/* Initialize Wi-Fi as sta and set scan method */
static void wifi_scan(void)
{
	//M5.Lcd.printf( "Adapter init\n" );
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    //M5.Lcd.printf( "WiFi init\n" );
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));


    //M5.Lcd.printf( "Set WiFi Mode to STA\n" );
    ESP_ERROR_CHECK( esp_wifi_set_mode (WIFI_MODE_STA) );
    //M5.Lcd.printf( "Start WiFi\n" );
    ESP_ERROR_CHECK( esp_wifi_start() );

    while( !WiFiReady ) {
    		vTaskDelay( 100/portTICK_PERIOD_MS );
    }
    //M5.Lcd.printf( "WiFi Ready.\n" );
    wifi_scan_config_t scan_config = {
    	.ssid = 0,
		.bssid = 0,
		.channel = 0,
		.show_hidden = true
    };
    //scan_config.show_hidden = true;
    //scan_config.scan_time.passive = 8000;


	uint16_t ap_num = MAX_APs;
	//printf("Max %d access points:\n", ap_num);
	wifi_ap_record_t ap_records[MAX_APs];
	//printf("Found %d access points:\n", ap_num);

	static char cipher[24];
	static char phystr[24];
	static char wps[5];

	int count = 0;
	static int lastCount = 0;
	while( 1 ) {
		ap_num = MAX_APs;
	    //printf( "Start WiFi scan\n" );
	    ESP_ERROR_CHECK( esp_wifi_scan_start (&scan_config, true) );
	    //printf( "WiFi scan done\n" );

	    while( !ScanDone ) {
	    		vTaskDelay( 100/portTICK_PERIOD_MS );
	    }
		ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));
		//M5.Lcd.fillScreen(BLACK);

		M5.Lcd.setCursor( 0, 20 );
		M5.Lcd.setTextColor( WHITE, BLACK );
		M5.Lcd.printf("BSSID        SSID                   CH RSSI AUTH     \n");
		M5.Lcd.printf("=====================================================\n");
		for(int i = 0; i < ap_num; i++) {
			strcpy( cipher, cipher_name[ap_records[i].pairwise_cipher] );
			strcat( cipher, "/" );
			strcat( cipher, cipher_name[ap_records[i].group_cipher] );

			phystr[0] = 0;

			if( ap_records[i].phy_11b ) {
				strcat( phystr, "b/" );
			}
			if( ap_records[i].phy_11g ) {
				strcat( phystr, "g/" );
			}
			if( ap_records[i].phy_11n ) {
				strcat( phystr, "n/" );
			}
			if( ap_records[i].phy_lr ) {
				strcat( phystr, "low/" );
			}

			if( phystr[0] != 0 ) {
				phystr[strlen(phystr)-1] = 0;
			}
			else {
				strcpy( phystr, "-" );
			}

			if( ap_records[i].wps ) {
				strcpy( wps, "WPS" );
			}
			else {
				strcpy( wps, "-" );
			}


			M5.Lcd.printf("%02x%02x%02x%02x%02x%02x %-22s %2i %4i %-8s\n",
			(int)ap_records[i].bssid[0],
			(int)ap_records[i].bssid[1],
			(int)ap_records[i].bssid[2],
			(int)ap_records[i].bssid[3],
			(int)ap_records[i].bssid[4],
			(int)ap_records[i].bssid[5],
			(char *)ap_records[i].ssid,
			(int)ap_records[i].primary,
			(int)ap_records[i].rssi,
			auth_mode_name[ap_records[i].authmode]
			);
		}
		vTaskDelay( 1000/portTICK_PERIOD_MS );
		for( int i=ap_num; i<lastCount; i++ ) {
			M5.Lcd.printf("                                                     \n");
		}
		lastCount = ap_num;
		//M5.Lcd.printf( "\x1B[%iA\n", ap_num+4 );
		//break;
	}

}

extern "C" void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    M5.begin();

	M5.Lcd.fillScreen(BLACK);
	// LCD display
	M5.Lcd.setTextSize(1);
	M5.Lcd.printf("\nESP32 WiFi Scanner");
	M5.Lcd.printf("\n\n");


    wifi_scan();

}
