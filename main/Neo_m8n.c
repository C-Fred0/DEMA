#include "Neo_m8n.h"
#include "esp_log.h"
#include <string.h>
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "mqtt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // Para atoi
static const char *GPS_TAG = "GPS_MODULE";
int num_satelites=0;

// Inicialización de UART
void init_neo_m8n(void) {
    // Inicializar NVS antes de usarlo
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Si la partición NVS está dañada, hay que borrarla y reinicializarla
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(UART_NUM_2, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

// Guardar datos en NVS
void save_gpgga_to_nvs(const char* gpgga) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err == ESP_OK) {
        nvs_set_str(my_handle, "last_gpgga", gpgga);
        nvs_commit(my_handle);
        nvs_close(my_handle);
    } else {
        ESP_LOGE(GPS_TAG, "Error (%s) al abrir NVS", esp_err_to_name(err));
    }
}

// Cargar datos desde NVS
bool load_gpgga_from_nvs(char* gpgga, size_t len) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &my_handle);
    if (err == ESP_OK) {
        size_t required_size;
        err = nvs_get_str(my_handle, "last_gpgga", NULL, &required_size);
        if (err == ESP_OK && required_size <= len) {
            nvs_get_str(my_handle, "last_gpgga", gpgga, &required_size);
            nvs_close(my_handle);
            return true;
        }
        nvs_close(my_handle);
    }
    return false;
}

// Función para leer y decodificar la trama $GPGGA
void neo_m8n_read(void) {
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE + 1);
    char gpgga[256] = "";  
    bool gpgga_found = false;

    const int rxBytes = uart_read_bytes(UART_NUM_2, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
    if (rxBytes > 0) {
        data[rxBytes] = 0;
        char *start = strstr((char*)data, "$GPGGA");
        
        if (start != NULL) {
            char *end = strchr(start, '\n');
            if (end != NULL) {
                int len = end - start + 1;
                strncpy(gpgga, start, len);
                gpgga[len] = '\0';
                gpgga_found = true;
            }
        }
    }

    if (gpgga_found) {
        ESP_LOGI(GPS_TAG, "Trama GPGGA: %s", gpgga);
        save_gpgga_to_nvs(gpgga);
    } else {
        ESP_LOGW(GPS_TAG, "No se encontró una trama completa");
    }

    free(data);
}

float convert_to_decimal(char *deg_min, char hemisphere) {
    float degrees = atof(deg_min) / 100.0;
    int d = (int)degrees;
    float minutes = (degrees - d) * 100.0;
    float decimal = d + minutes / 60.0;
    return (hemisphere == 'S' || hemisphere == 'W') ? -decimal : decimal;
}

void decode_gpgga(const char *gpgga,char *texto0, char *texto1) {
    char tokenized[256];
     int numSatellites;  // Variable para almacenar el número de satélites como entero
    strncpy(tokenized, gpgga, sizeof(tokenized));
    char *token;
    char latitude[16], longitude[16], lat_hem = 'N', lon_hem = 'E';
    float lat_decimal, lon_decimal;
    char num[16];
    // Parse data fields
    strtok(tokenized, ",");  // Skip $GPGGA
    token = strtok(NULL, ",");
  //  printf("UTC Time: %s\n", token ? token : "N/A");
    
    token = strtok(NULL, ",");
    if (token) strncpy(latitude, token, sizeof(latitude));
    token = strtok(NULL, ",");
    if (token) lat_hem = token[0];
    
    token = strtok(NULL, ",");
    if (token) strncpy(longitude, token, sizeof(longitude));
    token = strtok(NULL, ",");
    if (token) lon_hem = token[0];
    
 //   token = strtok(NULL, ",");
   // printf("Calidad GPS: %s\n", token ? token : "N/A");
    token = strtok(NULL, ",");  // Quality indicator
    token = strtok(NULL, ",");  // Number of satellites
    if (token) strncpy(num, token, sizeof(num));
    printf("Número de satélites: %s\n", num);  // Mostrar número de satélites
    numSatellites = atoi(num);
    if (*latitude && *longitude) {
        lat_decimal = convert_to_decimal(latitude, lat_hem);
        lon_decimal = convert_to_decimal(longitude, lon_hem);
        printf("Latitud: %f %c, Longitud: %f %c\n", lat_decimal, lat_hem, lon_decimal, lon_hem);
      //  printf("Google Maps: https://www.google.com/maps?q=%f,%f\n", lat_decimal, lon_decimal);
      //  sprintf(*texto0, "https://www.google.com/maps?q=%f,%f", lat_decimal, lon_decimal);
       sprintf(texto1,"https://www.google.com/maps?q=%f,%f", lat_decimal, lon_decimal);
        sprintf(texto0,"%f,%f,%d,", lat_decimal, lon_decimal,numSatellites);
        //mandar_datos_mqtt(texto0); 
    } else {
        printf("Coordenadas no disponibles.\n");
    }
}