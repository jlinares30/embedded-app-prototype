#ifndef LED_H
#define LED_H

#include "Actuator.h"

class Led : public Actuator {
private:
    bool state; ///< Current state of the LED (true = ON, false = OFF).

public:
    /**
     * @brief Constructs an Led actuator.
     * @param pin The GPIO pin for the LED (configured as OUTPUT).
     * @param initialState Initial state of the LED (default: false/OFF).
     * @param commandHandler Optional handler to receive commands (default: nullptr).
     */
    Led(int pin, bool initialState = false, CommandHandler* commandHandler = nullptr);

    /**
     * @brief Handles commands to control the LED state.
     * @param command The command to execute (e.g., CMD_ALERT_ON_ID).
     */
    void handle(Command command) override;

    /**
     * @brief Gets the current state of the LED.
     * @return True if the LED is ON, false if OFF.
     */
    bool getState() const;

    /**
     * @brief Sets the LED state directly.
     * @param newState The new state (true = ON, false = OFF).
     */
    void setState(bool newState);
};

#endif // LED_H