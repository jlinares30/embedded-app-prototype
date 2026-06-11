#ifndef NEXORA_DEVICE_H
#define NEXORA_DEVICE_H

#include "Device.h"
#include "AnalogReader.h"
#include "GasValve.h"
#include "Led.h"
#include "Buzzer.h"

class NexoraDevice : public Device {
private:
    AnalogReader gasSensor;
    GasValve controlValve;
    Led alertLed;
    Buzzer alertBuzzer;
    
    float currentGasPPM;

    const char* edgeUrl;
    const char* apiKey;
    const char* devId;

    void dispatchToEdgeService();

public:
    NexoraDevice(int pinGas, int pinValve, int pinLed, int pinBuzzer, 
                 const char* url, const char* key, const char* id);
                 
    void on(Event event) override;
    void handle(Command command) override;
    void update();
};

#endif // NEXORA_DEVICE_H