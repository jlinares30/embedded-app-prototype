#ifndef BUZZER_H
#define BUZZER_H

#include "Actuator.h"

class Buzzer : public Actuator {
private:
    bool state;

public:
    Buzzer(int pin, CommandHandler* commandHandler = nullptr);
    void handle(Command command) override;
    bool getState() const;
};

#endif // BUZZER_H