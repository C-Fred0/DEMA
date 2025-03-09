#include "driver/uart.h"
#include "driver/gpio.h"
#include "string.h"
#include "esp_log.h"
#include "esp_rom_sys.h" 

#define TXD_PIN (GPIO_NUM_4)  // Pin para TX
#define RXD_PIN (GPIO_NUM_5)  // Pin para RX
#define BUF_SIZE (1024)

static const char *TAG = "SIM7020E";

// Inicializa la UART
void uart_init() {
    const uart_config_t uart_config = {
        .baud_rate = 115200,            // Velocidad de baudios para SIM7020E
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    // Configurar UART0 (equivalente a Serial1 en Arduino)
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

// Función para enviar comandos AT
void send_at_command(const char* command) {
    uart_write_bytes(UART_NUM_0, command, strlen(command));
}

// Función para leer la respuesta del SIM7020E
int read_response(char* response, int buf_size) {
    uint8_t data[buf_size];
    int len = uart_read_bytes(UART_NUM_0, data, buf_size - 1, 100 / portTICK_PERIOD_MS);
    
    if (len > 0) {
        data[len] = '\0'; 
        strcpy(response, (char*)data);  // Copiar la respuesta a la variable 'response'
    } else {
        response[0] = '\0';  // Si no hubo respuesta, se pone una cadena vacía
    }
    
    return len;
}

// Función para enviar un mensaje de texto
void send_sms(const char* phone_number, const char* message) {
    char response[BUF_SIZE];

    // Configurar modo texto para SMS
    send_at_command("AT+CMGF=1\r\n");
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Pausa de 2 segundos
    read_response(response, BUF_SIZE);

    // Iniciar el envío de SMS al número con código de país
    char cmd[64];
    sprintf(cmd, "AT+CMGS=\"%s\"\r\n", phone_number);
    send_at_command(cmd);
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Pausa de 2 segundos
    read_response(response, BUF_SIZE);

    // Enviar el mensaje de texto
    send_at_command(message);
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Pausa de 2 segundos

    // Enviar Ctrl+Z (0x1A) para finalizar el mensaje
    send_at_command("\x1A");
    vTaskDelay(5000 / portTICK_PERIOD_MS);  // Pausa de 5 segundos para asegurar la recepción de la respuesta

    // Leer la respuesta al envío del mensaje
    read_response(response, BUF_SIZE);
    if (strstr(response, "+CMGS") != NULL) {
        ESP_LOGI(TAG, "Mensaje enviado: %s", message);
        ESP_LOGI(TAG, "Estado: Enviado con éxito.");
    } else {
        ESP_LOGI(TAG, "Mensaje enviado: %s", message);
        ESP_LOGI(TAG, "Estado: Falló el envío.");
    }
}

void init_gsm(){
    //init_uart();
    // Verificar el estado de registro de la red
    char reg_response[BUF_SIZE];
    send_at_command("AT+CREG?\r\n");
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Pausa para esperar la respuesta
    read_response(reg_response, BUF_SIZE);
    ESP_LOGI(TAG, "Estado de registro: %s", reg_response); // Imprimir el estado de registro
}

/*void app_main(void) {
    // Enviar el SMS con el mensaje "Hola, este es un mensaje de prueba."
    send_sms("4491145054", "UAA Prueba");
}*/