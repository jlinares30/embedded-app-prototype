#include "DigitalActuator.h"
#include "NexoraConstants.h"
#include <Arduino.h>

DigitalActuator::DigitalActuator(int pin, const char* label, CommandHandler* commandHandler)
    : Actuator(pin, commandHandler), state(false), label(label) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW); // Apagado por defecto al iniciar
}

void DigitalActuator::handle(Command command) {
    if (command.id == CMD_ALERT_ON_ID) {
        state = true;
        digitalWrite(pin, HIGH); // Enciende físicamente (LED prende rojo / Buzzer suena)
        Serial.printf("[%s ACTUATOR] -> ENCENDIDO (HIGH).\n", label);
    } else if (command.id == CMD_ALERT_OFF_ID) {
        state = false;
        digitalWrite(pin, LOW);  // Apaga físicamente el componente
        Serial.printf("[%s ACTUATOR] -> APAGADO (LOW).\n", label);
    }
    Actuator::handle(command); // Propaga la confirmación al Device
}

bool DigitalActuator::getState() const {
    return state;
}