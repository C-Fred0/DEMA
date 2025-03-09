#ifndef BATTERY_H
#define BATTERY_H

#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_sleep.h>
#include <driver/adc.h>
#include <driver/gpio.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"

void set_adc(void);

uint8_t read_battery(void);

#endif // BATTERY_H
