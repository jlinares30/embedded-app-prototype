#ifndef ANALOG_READER_H
#define ANALOG_READER_H

#include "Sensor.h"

class AnalogReader : public Sensor {
private:
    int eventId;

public:
    AnalogReader(int pin, int assignedEventId, EventHandler* eventHandler = nullptr);
    float readValue(int mapMin, int mapMax);
};

#endif // ANALOG_READER_H