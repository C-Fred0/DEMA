#ifndef MQTT_H
#define MQTT_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
//#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

// Definición del TAG para el logging
//#define TAGMQTT = "mqttws_example";

/**
 * @brief Configura el canal y la resolución del ADC.
 */

void start_mqtt(void);

/**
 * @brief Lee el valor del ADC y devuelve la lectura de la batería.
 * 
 * @return uint16_t Valor del ADC (0 - 4095)
 */
void mandar_datos_mqtt(const char *); 
void numero(char *num);
const char* obtener_num();
void mqtt_connect(void);
void mqtt_disconnect(void);
#endif // BATTERY_H