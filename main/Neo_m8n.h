#ifndef NEO_M8N_H
#define NEO_M8N_H

#include <stdio.h>
#include <stdlib.h>
#include "driver/uart.h"
#include "nvs.h"

// Definiciones de pines y configuración
#define TXD_PIN GPIO_NUM_17
#define RXD_PIN GPIO_NUM_16
#define RX_BUF_SIZE 1024
#define NVS_NAMESPACE "gps_data"

// Inicialización de UART para el GPS
void init_neo_m8n(void);

// Lectura y decodificación de datos $GPGGA
void neo_m8n_read(void);

// Guardar y cargar datos GPGGA en NVS
void save_gpgga_to_nvs(const char *gpgga);
bool load_gpgga_from_nvs(char *gpgga, size_t len);

// Decodificación de la trama $GPGGA
void decode_gpgga(const char *gpgga, char *texto0, char *texto1);

#endif /* NEO_M8N_H */
