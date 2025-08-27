// main.c
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "mqtt_service.h" // Inclui nossa nova biblioteca!

// Função de callback: será chamada pela biblioteca MQTT quando uma mensagem chegar.
void on_message_received(const char* topic, const char* payload) {
    printf("Mensagem recebida! Tópico: %s, Payload: %s\n", topic, payload);

    // Exemplo: ligar o LED da Pico W se receber "ON" no tópico "pico/led"
    if (strcmp(topic, "ha/desafio15/fabricio.silva/led") == 0) {
        if (strcmp(payload, "ON") == 0) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        } else if (strcmp(payload, "OFF") == 0) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        }
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000); // Um tempo para a serial iniciar

    // 1. Configurar a biblioteca
    mqtt_config_t config = {
        .wifi_ssid = "DIOCLECIO MARINHO",
        .wifi_password = "francyleyde",
        .mqtt_server_host = "mqtt.iot.natal.br",
        .client_id = "pico_w_fabrício", // Use um ID único
        .username = "desafio15",
        .password = "desafio15.laica",
        .on_message_callback = on_message_received
    };

    // 2. Conectar (a função retorna imediatamente, a conexão ocorre em background)
    while (!mqtt_service_connect(&config)) {
        printf("Erro fatal ao iniciar o serviço MQTT.\n");
        //while(true);
        sleep_ms(3000);
    }
    printf("Serviço MQTT iniciado. Aguardando conexão...\n");
    
    // Variáveis para tarefas periódicas
    uint32_t last_publish_time = 0;
    bool subscribed = false;
    
    // 3. Loop principal da aplicação
    while (true) {
        // ESSENCIAL: Permite que a pilha de rede (Wi-Fi, MQTT) funcione.
        cyw43_arch_poll();

        // Verifica se a conexão MQTT foi estabelecida
        if (mqtt_service_is_connected()) {
            
            // Inscreve-se em um tópico (apenas uma vez)
            if (!subscribed) {
                mqtt_service_subscribe("ha/desafio15/fabricio.silva/led");
                subscribed = true;
            }

            // Publica uma mensagem a cada 10 segundos
            if (time_us_32() - last_publish_time > 10 * 1000 * 1000) {
                last_publish_time = time_us_32();
                char message[32];
                snprintf(message, sizeof(message), "Timestamp: %u", last_publish_time);
                
                printf("Publicando mensagem...\n");
                mqtt_service_publish("ha/desafio15/fabricio.silva/status", message, false);
            }
        }
    }

    return 0;
}