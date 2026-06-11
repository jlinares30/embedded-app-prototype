#include "Buzzer.h"
#include "NexoraConstants.h"
#include <Arduino.h>

Buzzer::Buzzer(int pin, CommandHandler* commandHandler)
    : Actuator(pin, commandHandler), state(false) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void Buzzer::handle(Command command) {
    if (command.id == CMD_ALERT_ON_ID) {
        state = true;
        // buzzer activo
        tone(pin, 1000); // Emite un tono de 1000 Hz
        Serial.println("[BUZZER ACTUATOR] -> SONANDO (1000 Hz).");
    } else if (command.id == CMD_ALERT_OFF_ID) {
        state = false;
        noTone(pin); // Detiene el tono
        digitalWrite(pin, LOW); // asegurar que quede en LOW
        Serial.println("[BUZZER ACTUATOR] -> SILENCIADO.");
    }
    Actuator::handle(command);
}

bool Buzzer::getState() const {
    return state;
}