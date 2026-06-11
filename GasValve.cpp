#include "GasValve.h"
#include "NexoraConstants.h"
#include <Arduino.h>

GasValve::GasValve(int pin, CommandHandler* commandHandler)
    : Actuator(pin, commandHandler), isClosed(false) {
    ledcAttach(pin, 50, 10); // Configura PWM nativo: 50Hz, 10 bits de resolución
    moverServo(90);          // Válvula abierta en estado inicial operativo
}

void GasValve::moverServo(int grados) {
    int duty = map(grados, 0, 180, 26, 123);
    ledcWrite(pin, duty);
}

void GasValve::handle(Command command) {
    if (command.id == CMD_CLOSE_VALVE_ID) {
        isClosed = true;
        moverServo(0); // 0° corta el flujo de gas
        Serial.println("[VALVULA ACTUATOR] Fuga detectada: Servo rotado a 0 grados.");
    } else if (command.id == CMD_OPEN_VALVE_ID) {
        isClosed = false;
        moverServo(90); // 90° abre el flujo seguro
        Serial.println("[VALVULA ACTUATOR] Estado seguro: Servo rotado a 90 grados.");
    }
    Actuator::handle(command); // Propaga la confirmación de regreso al Device
}

bool GasValve::getStatus() const {
    return isClosed;
}