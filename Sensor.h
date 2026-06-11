#ifndef SENSOR_H
#define SENSOR_H

#include "EventHandler.h"

class Sensor : public EventHandler {
protected:
    int pin; ///< GPIO pin assigned to the sensor.
    EventHandler* handler; ///< Optional handler to receive propagated events.

public:
    /**
     * @brief Constructs a Sensor with a pin and optional event handler.
     * @param pin The GPIO pin for the sensor.
     * @param eventHandler Pointer to an EventHandler to receive events (default: nullptr).
     */
    Sensor(int pin, EventHandler* eventHandler = nullptr);

    /**
     * @brief Handles an event by propagating it to the assigned handler.
     * @param event The event to handle.
     */
    void on(Event event) override;

    /**
     * @brief Sets or updates the event handler for this sensor.
     * @param eventHandler Pointer to the new EventHandler.
     */
    void setHandler(EventHandler* eventHandler);
};

#endif // SENSOR_H