#include <WiFi.h>
#include "ModestIoT.h"
#include "NexoraConstants.h"
#include "NexoraDevice.h"

// MAPEADO DE HARDWARE ESP32
#define PIN_SENSOR_MQ2        34  // Pin analógico ADC1_CH6 (VP)
#define PIN_SERVO_VALVULA     18  // Pin PWM para el Servomotor
#define PIN_LED_ALERTA        26  // Pin digital de salida para el LED
#define PIN_BUZZER_ACTIVO     27  // Pin digital de salida para el Buzzer

// CONFIGURACIÓN DE ACCESO A RED DOMÉSTICA (WOKWI)
const char* wifi_ssid  = "Wokwi-GUEST";
const char* wifi_pass  = "";
// URL pública del túnel -> cambia segun el tunel levantado con npx localtunnel --port 5000
const char* target_url = "https://kind-keys-grow.loca.lt/api/v1/monitoring/telemetry";

// INSTANCIACIÓN DE ACTIVO INMOBILIARIO INTELIGENTE
NexoraDevice nexoraApartment(
    PIN_SENSOR_MQ2,
    PIN_SERVO_VALVULA,
    PIN_LED_ALERTA,
    PIN_BUZZER_ACTIVO,
    target_url,
    "test-api-key-123",
    "gas-safety-unit-apt-402"
);

void setup() {
    Serial.begin(115200);
    Serial.println("[NEXORA START] Despertando sistema embebido desacoplado...");

    // Inicializar enlace inalámbrico de red
    WiFi.begin(wifi_ssid, wifi_pass);
    Serial.print("Buscando red Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[RED READY] ESP32 enlazada de forma segura.");
    Serial.print("IP Asignada localmente: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    // Llama al método cíclico interno que orquesta los comandos y eventos
    nexoraApartment.update();

    delay(3000);
}