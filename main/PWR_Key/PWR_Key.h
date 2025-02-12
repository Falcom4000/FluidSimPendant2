#ifdef __cplusplus
extern "C" {
#endif
#pragma once
#include <stdint.h>
#include "driver/gpio.h"
#include "st7789.h"
#include "esp_log.h"
#define PWR_KEY_Input_PIN   6
#define PWR_Control_PIN     7
 
#define Device_Sleep_Time    10
#define Device_Restart_Time  15
#define Device_Shutdown_Time 20

void Fall_Asleep(void);
void Shutdown(TFT_t dev);
void Restart(void);

void PWR_Init(void);
void PWR_Loop(TFT_t dev);
#ifdef __cplusplus
}
#endif