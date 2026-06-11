#ifndef DIGITAL_ACTUATOR_H
#define DIGITAL_ACTUATOR_H

#include "Actuator.h"

class DigitalActuator : public Actuator {
private:
    bool state;
    const char* label;

public:
    DigitalActuator(int pin, const char* label, CommandHandler* commandHandler = nullptr);
    void handle(Command command) override;
    bool getState() const;
};

#endif // DIGITAL_ACTUATOR_H