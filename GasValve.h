#ifndef GAS_VALVE_H
#define GAS_VALVE_H

#include "Actuator.h"

class GasValve : public Actuator {
private:
    bool isClosed;
    void moverServo(int grados);

public:
    GasValve(int pin, CommandHandler* commandHandler = nullptr);
    void handle(Command command) override;
    bool getStatus() const;
};

#endif // GAS_VALVE_H