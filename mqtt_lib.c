#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "mqtt_service.h" // Inclui nossa biblioteca MQTT


// --- Credenciais da Rede Wi-Fi ---
#define WIFI_SSID       "tarefa-mqtt"       // Nome da sua rede Wi-Fi (SSID)
#define WIFI_PASSWORD   "laica@2025"      // Senha da sua rede Wi-Fi

// --- Configurações do Broker MQTT ---
#define MQTT_SERVER     "mqtt.iot.natal.br"  // Endereço IP ou hostname do seu broker MQTT
#define MQTT_CLIENT_ID  "pico_w_client_fabricio.silva"    // ID único para este cliente. Mude se tiver mais Picos.
#define MQTT_USERNAME   "desafio15"    // Usuário para autenticação no broker (deixe "" se não usar)
#define MQTT_PASSWORD   "desafio15.laica"      // Senha para autenticação no broker (deixe "" se não usar)
#define MQTT_QOS        0                     // Nível de Qualidade de Serviço padrão (0, 1 ou 2)

// --- Tópicos MQTT ---
// Tópico para receber comandos (ex: controlar o LED)
#define MQTT_TOPIC_COMMAND "ha/desafio15/fabricio.silva/set"

// Tópico para publicar status (ex: enviar um timestamp ou leitura de sensor)
#define MQTT_TOPIC_STATUS  "ha/desafio15/fabricio.silva/mpu6050"


// Callback chamado quando chega uma mensagem MQTT
void on_message_received(const char* topic, const char* payload) {
    printf("Mensagem recebida! Tópico: %s, Payload: %s\n", topic, payload);

    // Exemplo: ligar/desligar o LED da Pico W
    if (strcmp(topic, MQTT_TOPIC_COMMAND) == 0) {
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
                if (mqtt_service_subscribe(MQTT_TOPIC_COMMAND, MQTT_QOS)) {
                    printf("Inscrito em '%s' com QoS %d\n", MQTT_TOPIC_COMMAND, MQTT_QOS);
                    subscribed = true;
                }
            }

            // Publica a cada 10 segundos
            if (time_us_32() - last_publish_time > 10 * 1000 * 1000) {
                last_publish_time = time_us_32();
                char message[32];
                
                snprintf(message, sizeof(message), "Timestamp: %u", last_publish_time);

                // Usando o tópico de status de config.h
                printf("Publicando mensagem em '%s'...\n", MQTT_TOPIC_STATUS);
                mqtt_service_publish(MQTT_TOPIC_STATUS, message, MQTT_QOS, false);
            }
        }
    }

    return 0;
}
