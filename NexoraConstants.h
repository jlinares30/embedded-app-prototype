#ifndef NEXORA_CONSTANTS_H
#define NEXORA_CONSTANTS_H

// IDs Únicos para los Eventos (Sensores -> Device)
#define EVENT_GAS_READ_ID         10

// IDs Únicos para los Comandos (Device -> Actuadores / Red)
#define CMD_CLOSE_VALVE_ID        20
#define CMD_OPEN_VALVE_ID         21
#define CMD_ALERT_ON_ID           30  // Comando para encender LED y Buzzer
#define CMD_ALERT_OFF_ID          31  // Comando para apagar LED y Buzzer
#define CMD_DISPATCH_EDGE_ID      40

#endif // NEXORA_CONSTANTS_H