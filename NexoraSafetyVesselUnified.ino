/**
 * @file NexoraSafetyVesselUnified.ino
 * @brief Unified implementation of the Nexora Safety Vessel system using the Modest-IoT framework.
 *
 * This single-file implementation brings together the event-driven Modest-IoT Nano-Framework
 * asynchronous architecture (matching the professor's scheduling and telemetry engine) to monitor
 * gas levels, control a servo-actuated gas valve, actuate warning alerts, and transmit telemetry
 * to an edge server via HTTP requests.
 * 
 * @date 2026-06-13
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "ModestIoT.h" // Umbrella header for framework components

// ============================================================================
// --- 1. APPLICATION CONSTANTS ---
// ============================================================================

// IDs Únicos para los Eventos (Sensores -> Device)
#define EVENT_GAS_READ_ID         10

// IDs Únicos para los Comandos (Device -> Actuadores / Red)
#define CMD_CLOSE_VALVE_ID        20
#define CMD_OPEN_VALVE_ID         21
#define CMD_ALERT_ON_ID           30  // Comando para encender LED y Buzzer
#define CMD_ALERT_OFF_ID          31  // Comando para apagar LED y Buzzer

// ============================================================================
// --- 2. CONCRETE SENSOR & ACTUATOR IMPLEMENTATIONS ---
// ============================================================================

/**
 * @brief Analog reader sensor wrapper integrated with the Modest-IoT scheduler.
 */
class NexoraAnalogReader : public Sensor {
private:
    int pin;
    float currentMappedValue;

public:
    NexoraAnalogReader(int pin, EventHandler* eventHandler = nullptr)
        : Sensor(pin, eventHandler), pin(pin), currentMappedValue(0.0) {
        pinMode(pin, INPUT);
    }

    /**
     * @brief Responds to the scheduler's request to measure data.
     */
    void on(Event event) override {
        if (event.identifier == Sensor::MEASURE_DATA_REQUESTED_EVENT_IDENTIFIER) {
            int raw = analogRead(pin);
            // Map the raw ESP32 reading (0 to 4095) to 0-600 PPM
            currentMappedValue = map(raw, 0, 4095, 0, 600);
            
            // Notify the device mediator that data read is complete
            if (handler != nullptr) {
                handler->on(Event(Sensor::DATA_READ_EVENT_IDENTIFIER));
            }
        }
    }

    float getMappedValue() const {
        return currentMappedValue;
    }
};

/**
 * @brief LED warning actuator.
 */
class NexoraLed : public Actuator {
private:
    bool state;

public:
    NexoraLed(int pin, bool initialState = false, CommandHandler* commandHandler = nullptr)
        : Actuator(pin, commandHandler), state(initialState) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, state);
    }

    void handle(Command command) override {
        if (command.id == CMD_ALERT_ON_ID) {
            state = true;
            digitalWrite(pin, state);
            Serial.println("[LED ACTUATOR] -> ENCENDIDO (HIGH).");
        } else if (command.id == CMD_ALERT_OFF_ID) {
            state = false;
            digitalWrite(pin, state);
            Serial.println("[LED ACTUATOR] -> APAGADO (LOW).");
        }
        Actuator::handle(command);
    }

    bool getState() const { return state; }
};

/**
 * @brief Buzzer warning actuator.
 */
class NexoraBuzzer : public Actuator {
private:
    bool state;

public:
    NexoraBuzzer(int pin, CommandHandler* commandHandler = nullptr)
        : Actuator(pin, commandHandler), state(false) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    void handle(Command command) override {
        if (command.id == CMD_ALERT_ON_ID) {
            state = true;
            tone(pin, 1000); // Emits a 1000 Hz tone
            Serial.println("[BUZZER ACTUATOR] -> SONANDO (1000 Hz).");
        } else if (command.id == CMD_ALERT_OFF_ID) {
            state = false;
            noTone(pin);
            digitalWrite(pin, LOW);
            Serial.println("[BUZZER ACTUATOR] -> SILENCIADO.");
        }
        Actuator::handle(command);
    }

    bool getState() const { return state; }
};

/**
 * @brief Gas valve actuator using PWM servo control.
 */
class NexoraGasValve : public Actuator {
private:
    bool isClosed;
    void moverServo(int grados) {
        int duty = map(grados, 0, 180, 26, 123);
        ledcWrite(pin, duty);
    }

public:
    NexoraGasValve(int pin, CommandHandler* commandHandler = nullptr)
        : Actuator(pin, commandHandler), isClosed(false) {
        ledcAttach(pin, 50, 10); // Native PWM setup: 50Hz, 10-bit resolution
        moverServo(90);          // Initially open
    }

    void handle(Command command) override {
        if (command.id == CMD_CLOSE_VALVE_ID) {
            isClosed = true;
            moverServo(0); // Cut gas flow
            Serial.println("[VALVULA ACTUATOR] Fuga detectada: Servo rotado a 0 grados.");
        } else if (command.id == CMD_OPEN_VALVE_ID) {
            isClosed = false;
            moverServo(90); // Open secure gas flow
            Serial.println("[VALVULA ACTUATOR] Estado seguro: Servo rotado a 90 grados.");
        }
        Actuator::handle(command);
    }

    bool getStatus() const { return isClosed; }
};

// ============================================================================
// --- 3. CONCRETE APPLICATION TELEMETRY SCHEMA ---
// ============================================================================

/**
 * @brief Telemetry package for Nexora Safety Vessel data.
 */
class NexoraTelemetryPackage : public TelemetryPackage {
private:
    String deviceId;
    float gasPpm;
    bool smokeDetected;

public:
    NexoraTelemetryPackage(const String& id, float ppm, bool smoke)
        : deviceId(id), gasPpm(ppm), smokeDetected(smoke) {}

    virtual ~NexoraTelemetryPackage() override = default;

    void serialize(JsonDocument& serializationDestination) const override {
        serializationDestination["device_id"] = this->deviceId;
        serializationDestination["apartment_id"] = "Apt-402";
        serializationDestination["gas_ppm"] = this->gasPpm;
        serializationDestination["smoke_detected"] = this->smokeDetected;
        serializationDestination["motion_detected"] = false;
        serializationDestination["door_open"] = false;
    }
};

// ============================================================================
// --- 4. APPLICATION MEDIATOR IMPLEMENTATION ---
// ============================================================================

/**
 * @brief Orchestrates sensors and actuators for gas leakage detection and mitigation.
 */
class NexoraDevice : public Device {
private:
    NexoraAnalogReader gasSensor;
    NexoraGasValve controlValve;
    NexoraLed alertLed;
    NexoraBuzzer alertBuzzer;
    
    float currentGasPPM;
    const char* edgeUrl;
    const char* apiKey;
    const char* devId;

protected:
    /**
     * @brief Processes queued telemetry records by sending them via HTTP POST.
     */
    void processQueuedTelemetryData(const TelemetryPackage* rawQueueItemPayload) override {
        if (rawQueueItemPayload != nullptr && WiFi.status() == WL_CONNECTED) {
            JsonDocument doc;
            rawQueueItemPayload->serialize(doc);
            
            String payload;
            serializeJson(doc, payload);

            HTTPClient http;
            http.begin(edgeUrl);
            http.addHeader("Content-Type", "application/json");
            http.addHeader("X-API-Key", apiKey);
            http.addHeader("Bypass-Tunnel-Reminder", "true");

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
    }

public:
    NexoraDevice(int pinGas, int pinValve, int pinLed, int pinBuzzer, 
                 const char* url, const char* key, const char* id,
                 unsigned long samplingIntervalInMilliseconds, uint8_t timerGroupIndex = 0)
        : Device(samplingIntervalInMilliseconds, timerGroupIndex),
          gasSensor(pinGas, this), 
          controlValve(pinValve, this),
          alertLed(pinLed, this),
          alertBuzzer(pinBuzzer, this),
          currentGasPPM(0.0), edgeUrl(url), apiKey(key), devId(id)
    {
        // Set event handler
        gasSensor.setHandler(this);

        // Initialize framework's asynchronous telemetry engine with queue depth of 10
        initializeAsynchronousEngine(10);

        // Register gas sensor with the framework's scheduler
        appendSensorToScheduler(&gasSensor, Sensor::MEASURE_DATA_REQUESTED_EVENT_IDENTIFIER);
    }

    /**
     * @brief Processes sensor read events asynchronously.
     */
    void on(Event event) override {
        if (event.identifier == Sensor::DATA_READ_EVENT_IDENTIFIER) {
            currentGasPPM = gasSensor.getMappedValue();

            Serial.println("\n================================================");
            Serial.printf("[NEXORA OS] Gas Cocina Físico: %.1f PPM\n", currentGasPPM);

            // Execute local safety logic
            if (currentGasPPM >= 400.0) {
                controlValve.handle(Command(CMD_CLOSE_VALVE_ID));
                alertLed.handle(Command(CMD_ALERT_ON_ID));
                alertBuzzer.handle(Command(CMD_ALERT_ON_ID));
            } else {
                controlValve.handle(Command(CMD_OPEN_VALVE_ID));
                alertLed.handle(Command(CMD_ALERT_OFF_ID));
                alertBuzzer.handle(Command(CMD_ALERT_OFF_ID));
            }

            // Enqueue telemetry payload
            TelemetryPackage* telemetry = new NexoraTelemetryPackage(
                devId,
                currentGasPPM,
                currentGasPPM > 100.0
            );
            if (!enqueueTelemetryPayload(&telemetry)) {
                delete telemetry; // Clean up if queue is full
            }
        }
    }

    void handle(Command command) override {
        // Concrete command handling if needed
    }
};

// ============================================================================
// --- 5. TOP-LEVEL CONFIGURATIONS & ARDUINO LOOP ---
// ============================================================================

// MAPEADO DE HARDWARE ESP32
#define PIN_SENSOR_MQ2        34  // Pin analógico ADC1_CH6 (VP)
#define PIN_SERVO_VALVULA     18  // Pin PWM para el Servomotor
#define PIN_LED_ALERTA        26  // Pin digital de salida para el LED
#define PIN_BUZZER_ACTIVO     27  // Pin digital de salida para el Buzzer

// CONFIGURACIÓN DE ACCESO A RED Wi-Fi (WOKWI)
const char* wifi_ssid  = "Sunflower";
const char* wifi_pass  = "iexdnQBTHWvPjUWYmvs3";
// URL pública del túnel -> cambia según el túnel levantado
const char* target_url = "https://dirty-worlds-fly.loca.lt/api/v1/monitoring/telemetry";

// INSTANCIACIÓN DE DEVICE MEDIATOR ASÍNCRONO
static NexoraDevice* nexoraApartment = nullptr;

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

    // Inicializar dispositivo mediador asíncrono con muestreo cada 3 segundos (3000ms)
    nexoraApartment = new NexoraDevice(
        PIN_SENSOR_MQ2,
        PIN_SERVO_VALVULA,
        PIN_LED_ALERTA,
        PIN_BUZZER_ACTIVO,
        target_url,
        "test-api-key-123",
        "gas-safety-unit-apt-402",
        3000 // Intervalo de muestreo del sensor
    );

    Serial.println("[NEXORA OS] Dispositivo inicializado. Monitoreo asíncrono activo.");
}

void loop() {
    // Al ejecutarse de manera asíncrona mediante el motor interno de la biblioteca (usando FreeRTOS),
    // el loop principal queda libre y simplemente cede tiempo de CPU para evitar reinicios del WDT.
    vTaskDelay(pdMS_TO_TICKS(60000));
}
