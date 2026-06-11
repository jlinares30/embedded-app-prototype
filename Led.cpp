#include "Led.h"
#include "NexoraConstants.h"
#include <Arduino.h>

Led::Led(int pin, bool initialState, CommandHandler* commandHandler)
    : Actuator(pin, commandHandler), state(initialState) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, state);
}

void Led::handle(Command command) {
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

bool Led::getState() const {
    return state;
}

void Led::setState(bool newState) {
    state = newState;
    digitalWrite(pin, state);
}