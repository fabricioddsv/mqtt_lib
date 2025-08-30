#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "mqtt_service.h" // Inclui nossa biblioteca MQTT

#define WIFI_SSID       "PRETA_INFOWAY"       // ou "DIOCLECIO MARINHO"
#define WIFI_PASSWORD   "dpsouza1"            // ou "francyleyde"
#define MQTT_SERVER     "mqtt.iot.natal.br"
#define MQTT_CLIENT_ID  "pico_w_fabricio"     // ID único
#define MQTT_USERNAME   "desafio15"
#define MQTT_PASSWORD   "desafio15.laica"
#define MQTT_QOS        1                     // QoS padrão (0, 1 ou 2)


// Callback chamado quando chega uma mensagem MQTT
void on_message_received(const char* topic, const char* payload) {
    printf("Mensagem recebida! Tópico: %s, Payload: %s\n", topic, payload);

    // Exemplo: ligar/desligar o LED da Pico W
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
    sleep_ms(2000); // tempo para inicializar a serial

    // 1. Configurar a biblioteca MQTT
    mqtt_config_t config = {
        .wifi_ssid = WIFI_SSID,
        .wifi_password = WIFI_PASSWORD,
        .mqtt_server_host = MQTT_SERVER,
        .client_id = MQTT_CLIENT_ID,
        .username = MQTT_USERNAME,
        .password = MQTT_PASSWORD,
        .on_message_callback = on_message_received,
        .default_qos = MQTT_QOS
    };


    // 2. Tentar conectar
    while (!mqtt_service_connect(&config)) {
        printf("Erro ao iniciar o serviço MQTT. Tentando novamente...\n");
        sleep_ms(3000);
    }
    printf("Serviço MQTT iniciado. Aguardando conexão...\n");

    uint32_t last_publish_time = 0;
    bool subscribed = false;

    // 3. Loop principal
    while (true) {
        // Essencial para a pilha de rede (Wi-Fi + MQTT)
        cyw43_arch_poll();

        if (mqtt_service_is_connected()) {
            // Assina o tópico apenas uma vez (QoS 1 neste caso)
            if (!subscribed) {
                if (mqtt_service_subscribe("ha/desafio15/fabricio.silva/led", 1)) {
                    printf("Inscrito em ha/desafio15/fabricio.silva/led com QoS 1\n");
                    subscribed = true;
                }
            }

            // Publica a cada 10 segundos
            if (time_us_32() - last_publish_time > 10 * 1000 * 1000) {
                last_publish_time = time_us_32();
                char message[32];
                snprintf(message, sizeof(message), "Timestamp: %u", last_publish_time);

                printf("Publicando mensagem em ha/desafio15/fabricio.silva/status com QoS 0...\n");
                mqtt_service_publish("ha/desafio15/fabricio.silva/status", message, 0, false);
            }
        }
    }

    return 0;
}
