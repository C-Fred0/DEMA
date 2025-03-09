#include "esp_system.h"
#include <esp_err.h>
#include <esp_sleep.h>
#include "esp_log.h"
#include "Neo_m8n.h"
#include "Led_Button.h"
#include "Battery.h"
#include "MPU6050.h"
#include "GSM.h"
#include <stdio.h>
#include <string.h>
#include "main_functions.h"
#include "mqtt.h"

void app_main(void) {
    //INICIALIZAR MODULOS
    uart_init();
    init_neo_m8n();  // Inicializar el módulo GPS
    init_GPIO();    // Inicializa los GPIO de los botones y leds
    mpu6050_init();
    set_adc(); 
    start_mqtt(); //Inicia mqtt pero no se conecta
    setup(); //Se configura el modelo del TensorFlow
    init_gsm();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    int16_t accelxyz[3];
    uint8_t batval;
    char texto[256];
    char texto1[256];
    char num[20];  
    char gpgga[256] = "";
    int predic;
    char mensaje[25]="SE NOS CAYO EL VIEJITO";
    char peticion[10]="DAME_NUM";
    uint8_t *databuffer = NULL;
    int16_t DatosAx[30];
    int16_t DatosAy[30];
    int16_t DatosAz[30];
    if (load_gpgga_from_nvs(gpgga, sizeof(gpgga))) {
        ESP_LOGI("APP_MAIN", "Último GPGGA de NVS: %s", gpgga);
    } else {
        ESP_LOGW("APP_MAIN", "No hay datos GPGGA guardados en NVS.");
    }
    mqtt_connect(); //se conecta a mqtt
    while (1) {   
        //lectura del gps
        neo_m8n_read();  // Leer datos GPS
        decode_gpgga(gpgga,&texto,&texto1); 
        //texto0 contiene los datos del GPS(Latitud,Longitud,Numero de satelites)
        //texto1 contiene "https://www.google.com/maps?q=%f,%f", lat_decimal, lon_decimal (mensaje para el GSM)
        batval = read_battery(); //lecuta de batería
       for(int i=0; i < 30; i++){ //Se guardan los 90 datos del Acelerómetro 
            mpu6050_readaccel(&accelxyz[0],&accelxyz[1],&accelxyz[2]);
            DatosAx[i]=accelxyz[0];
            DatosAy[i]=accelxyz[1];
            DatosAz[i]=accelxyz[2];
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        predic = loop(DatosAx,DatosAy,DatosAz); //se realiza la inferencia del modelo de TensorFlow
        if (predic >= 0 && predic <= 5) { // Son 6 clases (Parado,Sentado,Acostado,Caminando,Se cayó,transición)
            char predic_str[10]; 
            snprintf(predic_str, sizeof(predic_str), "%d,%d", predic,batval); //se juntan los datos a mandar al mqtt
            //ejemplo: 0.000000,1.666500,5,4,50 = (lat,long,numero de satelites,estatus de la persona,%deBateria)
            strcat(texto, predic_str);
            mandar_datos_mqtt(texto); //se manda todo por mqtt
            vTaskDelay(5000 / portTICK_PERIOD_MS);  // Pausa para esperar la respuesta
            if(predic == 1){ //Si predic = 1 quiere decir que se cayó entonces...
                mandar_datos_mqtt(peticion); //La página guarda el número, esto hace la patición para que se lo proporcione
                // lo que manda es esta cadena "DAME_NUM" y la pag lo interpreta
                const char *data = obtener_num(); //Recibe el numero por MQTT
                vTaskDelay(1000 / portTICK_PERIOD_MS);  // Pausa para esperar la respuesta
                mqtt_disconnect(); // se desconecta del mqtt
                while(1){   
                    //Se mandan dos mensajes
                    send_sms(data,mensaje); //Se cayo el viejito
                    send_sms(data,texto1); //Dirección del mismo
                    vTaskDelay(5000 / portTICK_PERIOD_MS); 
                }
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS); 
        }        
    }
}
