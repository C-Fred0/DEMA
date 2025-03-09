#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_sleep.h>
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"

// Pines de los LEDs
#define LED_V GPIO_NUM_12   // Batería baja
#define LED_A GPIO_NUM_13   // Batería media
#define LED_R GPIO_NUM_14   // Batería alta

// Etiqueta para logging
#define TAGADC "ADC"

// Atenuación del ADC (12 dB para mayor rango)
#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_12
#define ADC_CHANNEL       ADC_CHANNEL_4 // Canal ADC (GPIO 32)
#define ADC_UNIT          ADC_UNIT_1    // Unidad ADC

// Manejador de ADC OneShot
static adc_oneshot_unit_handle_t adc_handle = NULL;

void set_adc(void) {
    // Configuración de la unidad ADC
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc_handle));

    // Configuración del canal ADC
    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = EXAMPLE_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &chan_cfg));

    // Configurar pines de los LEDs como salida
    esp_rom_gpio_pad_select_gpio(14);
    esp_rom_gpio_pad_select_gpio(13);
    esp_rom_gpio_pad_select_gpio(12);
    gpio_set_direction(LED_R, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_A, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_V, GPIO_MODE_OUTPUT);

    }

uint8_t read_battery() {
    uint16_t adcval = 0;
    uint8_t batval = 0;
    // Leer el valor del ADC
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL, &adcval));

    batval = (100/1290)*adcval - (100*2432/1290);
    if (batval < 0) batval = 0;
    // Control de LEDs según el nivel de batería
    if (batval > 20) {
        gpio_set_level(LED_R, 0);
        gpio_set_level(LED_A, 0);
        gpio_set_level(LED_V, 1);
    } else if (batval > 10 && batval <= 20) {
        gpio_set_level(LED_R, 0);
        gpio_set_level(LED_A, 1);
        gpio_set_level(LED_V, 0);
    } else {
        gpio_set_level(LED_R, 1);
        gpio_set_level(LED_A, 0);
        gpio_set_level(LED_V, 0);
    }

    return (uint8_t)batval;
}
