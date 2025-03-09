#include "esp_rom_sys.h" // Necesario para usar esp_rom_delay_us()

#include "Led_Button.h"
#include "driver/gpio.h"
#include <stdio.h>
#include <string.h>
#include "MPU6050.h"
#include "mqtt.h"
#include "main_functions.h"
#define LED_S GPIO_NUM_27   //Servidor

#define BUTTON_M GPIO_NUM_25    //Muestra
#define BUTTON_D GPIO_NUM_33    //Datos del UART
#define BUTTON_R GPIO_NUM_35    //Reinicia las variables

 char uartbuffer[1600];
   int16_t DatosAx[30];
    int16_t DatosAy[30];
    int16_t DatosAz[30];
void send_mpu_uart(){
    int16_t accelxyz[3];
    int16_t gyroxyz[3];
    char databuffer[55];

    strcpy(uartbuffer,"\0"); 
    strcpy(databuffer,"\0");

    for(int i=0; i < 30; i++){
        mpu6050_readaccel(&accelxyz[0],&accelxyz[1],&accelxyz[2]);
        sprintf(databuffer," %d, %d, %d,", accelxyz[0],accelxyz[1],accelxyz[2]);
        strcat(uartbuffer,databuffer);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        DatosAx[i]=accelxyz[0];
        DatosAy[i]=accelxyz[1];
        DatosAz[i]=accelxyz[2];
    }
       
    loop(DatosAx,DatosAy,DatosAz);
    strcat(uartbuffer,"\n");
    ESP_LOGI(TAGMPU,"%s", uartbuffer);
}

void init_GPIO() {
    gpio_set_direction(LED_S, GPIO_MODE_OUTPUT);

    gpio_set_direction(BUTTON_M, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_M, GPIO_PULLUP_ONLY);

    gpio_set_direction(BUTTON_D, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_D, GPIO_PULLUP_ONLY);

    gpio_set_direction(BUTTON_R, GPIO_MODE_INPUT);
   // gpio_set_pull_mode(BUTTON_R, GPIO_PULLUP_ONLY);
   gpio_set_pull_mode(BUTTON_R, GPIO_PULLUP_ONLY);
}

void handle_buttons() {
    if (gpio_get_level(BUTTON_M) == 0) {
        printf("Loaddata \n");
        gpio_set_level(LED_S, 1);
        send_mpu_uart();
        gpio_set_level(LED_S, 0);

    }
    if (gpio_get_level(BUTTON_D) == 0) {
        printf("Send data \n");
        gpio_set_level(LED_S, 1);
        //mandar_datos_mqtt(uartbuffer);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(LED_S, 0);
    }
    
    /*if (gpio_get_level(BUTTON_R) == 0) {
        gpio_set_level(LED_S, 1);
        printf("Reset \n");
        gpio_set_level(LED_S, 0);
    }*/
}
