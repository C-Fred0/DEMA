#ifndef MPU6050_H
#define MPU6050_H

#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_sleep.h>
#include <driver/i2c.h>

// Definiciones para MPU6050
#define TAGMPU "MPU6050"
#define I2C_MASTER_FREQ_HZ     100000
#define MPU6050_I2C_ADDR       0x70
#define MPU6050_I2C_CH1        0x01
#define MPU6050_ADD            0x68
#define MPU6050_PWR            0x6B
#define MPU6050_RAW_GYRO       0x43
#define MPU6050_ACK_VAL        0x1
#define MPU6050_NACK_VAL       0x0

// Prototipos de funciones

/**
 * @brief Configuración de los pines GPIO para el bus I2C
 */
void i2c_gpio_conf(void);

/**
 * @brief Inicialización del sensor MPU6050
 */
void mpu6050_init(void);

/**
 * @brief Leer los valores del acelerómetro del MPU6050
 * 
 * @param x Puntero para almacenar la aceleración en el eje X
 * @param y Puntero para almacenar la aceleración en el eje Y
 * @param z Puntero para almacenar la aceleración en el eje Z
 */
void mpu6050_readaccel(int16_t* x, int16_t* y, int16_t* z);

#endif // MPU6050_H
