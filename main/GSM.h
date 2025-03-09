#ifndef GSM_H
#define GSM_H

#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"
#include "esp_log.h"
#include "esp_rom_sys.h" 

static const char *TAG = "SIM7020E";

// Inicializa la UART
void uart_init();

// Función para enviar comandos AT
void send_at_command(const char* command);

// Función para leer la respuesta del SIM7020E
int read_response(char* response, int buf_size);

// Función para enviar un mensaje de texto
void send_sms(const char* phone_number, const char* message);

void init_gsm();

#endif /* GSM_H */
