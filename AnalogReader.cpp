#include "AnalogReader.h"
#include <Arduino.h>

AnalogReader::AnalogReader(int pin, int assignedEventId, EventHandler* eventHandler)
    : Sensor(pin, eventHandler), eventId(assignedEventId) {
    pinMode(pin, INPUT);
}

float AnalogReader::readValue(int mapMin, int mapMax) {
    int raw = analogRead(pin);
    // Mapea la lectura cruda de la ESP32 (0 a 4095) al rango del negocio
    float mappedValue = map(raw, 0, 4095, mapMin, mapMax);
    
    // Propaga la señal del evento hacia la interfaz on() del Device
    on(Event(eventId)); 
    return mappedValue;
}