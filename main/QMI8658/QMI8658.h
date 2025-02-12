#pragma once

#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "freertos/queue.h"
#include "driver/i2c.h"
#include "SensorLib.h"
#include "SensorQMI8658.hpp"

// I2C configuration
#define I2C_MASTER_SCL 10
#define I2C_MASTER_SDA 11
#define I2C_MASTER_NUM I2C_NUM_0
#define QMI8658_ADDRESS 0x6B

// Data structure for IMU readings


// Function declarations
void i2c_master_init();
void setup_sensor();
void read_sensor_data();

// External declarations
extern SensorQMI8658 qmi;
extern IMUdata acc;
extern IMUdata gyr;


