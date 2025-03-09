/* MQTT over Websockets Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <stdlib.h>
static const char *TAGMQTT = "mqttws_example";
static esp_mqtt_client_handle_t client = NULL; // Cliente MQTT global
static char temp_data[11]; // Variable estática local

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAGMQTT, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */

// Función para enviar datos al MQTT
void mandar_datos_mqtt(const char *datos) {
    if (client != NULL) {
        int msg_id = esp_mqtt_client_publish(client, "test/in", datos, 0, 0, 0);  
        ESP_LOGI(TAGMQTT, "Datos enviados: %s, msg_id=%d", datos, msg_id);
    } else {
        ESP_LOGI(TAGMQTT, "El cliente MQTT no está inicializado");
    }
}

void mqtt_connect(void) {
    if (client) {
        esp_mqtt_client_start(client);
        ESP_LOGI(TAGMQTT, "MQTT client started");
    } else {
        ESP_LOGE(TAGMQTT, "MQTT client is not initialized");
    }
}

void mqtt_disconnect(void) {
    if (client) {
        esp_mqtt_client_stop(client);
        ESP_LOGI(TAGMQTT, "MQTT client stopped");
    } else {
        ESP_LOGE(TAGMQTT, "MQTT client is not initialized");
    }
}

void numero(char num) { //Se obtiene el numero del mqtt
    strncpy(temp_data, num, sizeof(temp_data));
    temp_data[sizeof(temp_data) - 1] = '\0'; 
}

const char* obtener_num() { //función para mandar el numero a donde se llame (main.c)
    return temp_data;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAGMQTT, "MQTT_EVENT_CONNECTED");
        esp_mqtt_client_subscribe(client, "test/in", 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAGMQTT, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAGMQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAGMQTT, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        if(strncmp(event->data, "1", event->data_len) == 0){//se reinicia la esp si no encuentra satelites a donde conectarse
            esp_restart();
       }
        else if (event->data_len < 11 && event->data_len > 3) { //Se verifica que es un numero celular, usualmente de 10 dígitos
        int copy_len = event->data_len > 10 ? 10 : event->data_len; // Asegurar límite de datos
        strncpy(temp_data, event->data, copy_len); // Copiar sólo hasta 10 caracteres
        temp_data[copy_len] = '\0'; // Agregar terminador null
    }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAGMQTT, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAGMQTT, "Other event id:%d", event->event_id);
        break;
    }
}
static void mqtt_app_start(void)
{
const esp_mqtt_client_config_t mqtt_cfg = {
   .broker.address.uri= "ws://broker.emqx.io:8083/mqtt",
   .credentials.authentication.password = "xguax",
   .credentials.username = "xguax"
  };
   client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == NULL) {
        ESP_LOGE(TAGMQTT, "Failed to initialize MQTT client");
        return;
    }

    // Registra los eventos para el cliente MQTT
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
}

void start_mqtt(void)
{
    ESP_LOGI(TAGMQTT, "[APP] Startup..");
    ESP_LOGI(TAGMQTT, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAGMQTT, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
    esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
    esp_log_level_set("transport_ws", ESP_LOG_VERBOSE);
    esp_log_level_set("transport", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();
}
