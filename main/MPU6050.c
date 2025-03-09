#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_sleep.h>
#include <driver/i2c.h>
//MPU6050 TAG
#define TAGMPU "MPU6050"

//MPU6050 Definition
#define I2C_MASTER_FREQ_HZ     100000
#define MPU6050_I2C_ADDR       0x70
#define MPU6050_I2C_CH1        0x01
#define MPU6050_ADD            0x68
#define MPU6050_PWR            0x6B
#define MPU6050_RAW_GYRO       0x43
#define MPU6050_ACK_VAL        0x1
#define MPU6050_NACK_VAL       0x0


//Configure I2C GPIO Configuration
void i2c_gpio_conf(){
    //Configure i2c controller 0 in master mode, normal speed
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num       = 21;
    conf.scl_io_num       = 22;
    conf.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = 0;
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_LOGI(TAGMPU, "I2C Controller Configured\r\n");
    //Install the driver
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    ESP_LOGI(TAGMPU, "I2C Driver installed\r\n");
}

//MPU6050 Initialization (YOU JUST NEED TO CALL THIS ONE TO START)
void mpu6050_init(){
    i2c_gpio_conf(); //Initialize i2c and config

    //Create and execute command link
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADD << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, MPU6050_PWR, true);
    i2c_master_write_byte(cmd, 0x0, true);
    i2c_master_stop(cmd);
    if (i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS) == ESP_OK)
    {
        ESP_LOGI(TAGMPU, "MPU6050 Channel 0x%02x Initialized", MPU6050_I2C_CH1);
    }
    else
    {
        ESP_LOGI(TAGMPU, "MPU6050 is not connected");
    }
    i2c_cmd_link_delete(cmd);
}

//Remember to call it using funct(&x, &y, &z);
void mpu6050_readaccel(int16_t* x, int16_t* y, int16_t* z){
    uint8_t acceldata[6];
    uint8_t accelreg = 0x3B;
    int16_t xacc,yacc,zacc;
    i2c_master_write_read_device(I2C_NUM_0,0x68,&accelreg,1,acceldata,6,pdMS_TO_TICKS(1000));
    xacc = (acceldata[0] << 8 | acceldata[1]);
    yacc = (acceldata[2] << 8 | acceldata[3]);
    zacc = (acceldata[4] << 8 | acceldata[5]);
    *x = xacc*1000/16384;
    *y = yacc*1000/16384;
    *z = zacc*1000/16384;
}