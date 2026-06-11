#include "NexoraDevice.h"
#include "NexoraConstants.h"
#include <WiFi.h>
#include <HTTPClient.h>

NexoraDevice::NexoraDevice(int pinGas, int pinValve, int pinLed, int pinBuzzer, 
                             const char* url, const char* key, const char* id)
    : gasSensor(pinGas, EVENT_GAS_READ_ID, this), 
      controlValve(pinValve, this),
      alertLed(pinLed, this),
      alertBuzzer(pinBuzzer, this),
      currentGasPPM(0.0), edgeUrl(url), apiKey(key), devId(id) {}

void NexoraDevice::on(Event event) {
    if (event.id == EVENT_GAS_READ_ID) {
        if (currentGasPPM >= 400.0) {
            controlValve.handle(Command(CMD_CLOSE_VALVE_ID));
            alertLed.handle(Command(CMD_ALERT_ON_ID));
            alertBuzzer.handle(Command(CMD_ALERT_ON_ID));
        } else {
            controlValve.handle(Command(CMD_OPEN_VALVE_ID));
            alertLed.handle(Command(CMD_ALERT_OFF_ID));
            alertBuzzer.handle(Command(CMD_ALERT_OFF_ID));
        }
    }
}

void NexoraDevice::handle(Command command) {
    if (command.id == CMD_DISPATCH_EDGE_ID) {
        dispatchToEdgeService();
    }
}

void NexoraDevice::dispatchToEdgeService() {
    if (WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;
    http.begin(edgeUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-API-Key", apiKey);
    http.addHeader("Bypass-Tunnel-Reminder", "true");

    String payload = "{\"device_id\":\"" + String(devId) + "\",\"apartment_id\":\"Apt-402\",";
    payload += "\"gas_ppm\":" + String(currentGasPPM, 1) + ",";
    payload += "\"smoke_detected\":" + String((currentGasPPM > 450.0) ? "true" : "false") + ",";
    payload += "\"motion_detected\":false,\"door_open\":false}";

    Serial.println("\n[HTTP DISPATCH] Enviando JSON al Edge:");
    Serial.println(payload);

    int httpResponseCode = http.POST(payload);
    Serial.printf("[HTTP INFRA] Respuesta del Edge Code: %d\n", httpResponseCode);

    if (httpResponseCode == 200 || httpResponseCode == 201) {
        String response = http.getString();
        Serial.println("Cuerpo de respuesta recibido:");
        Serial.println(response);


        if (response.indexOf("\"valve_command\":\"CLOSE\"") != -1 ||
            response.indexOf("\"valve_command\": \"CLOSE\"") != -1) {
            Serial.println("[REMOTE COMMAND] Servidor ordena cierre de seguridad.");
            controlValve.handle(Command(CMD_CLOSE_VALVE_ID));
            alertLed.handle(Command(CMD_ALERT_ON_ID));
            alertBuzzer.handle(Command(CMD_ALERT_ON_ID));
        }
    } else {
        if (currentGasPPM >= 400.0) {
            Serial.println("[AUTONOMOUS] Error de red. La mitigación local ya fue activada por el sensor.");
        }
    }
    http.end();
}

void NexoraDevice::update() {
    currentGasPPM = gasSensor.readValue(0, 600);

    Serial.println("\n================================================");
    Serial.printf("[NEXORA OS] Gas Cocina Físico: %.1f PPM\n", currentGasPPM);

    handle(Command(CMD_DISPATCH_EDGE_ID));
}