#include "PWR_Key.h"
#include "esp_log.h"
#include "esp_timer.h"
const int inputPin = 40; // Define GPIO40 as input
const int outputPin = 41; // Define GPIO41 as output
bool buttonState = false;
bool lastButtonState = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long lastClickTime = 0;
unsigned long clickInterval = 500; // Double-click interval
unsigned long longPressDuration = 1000; // Long press duration
bool longPressDetected = false;
static const char* TAG = "PWR_KEY";

void PWR_Loop(TFT_t dev)
{
    int reading = gpio_get_level(inputPin);
    // Debounce processing
    if (reading != lastButtonState) {
        lastDebounceTime = esp_timer_get_time();
    }
    if ((esp_timer_get_time() - lastDebounceTime) >= longPressDuration) {
        if (reading != buttonState) {
            buttonState = reading;
            {
                // Operation when button is released
                if (longPressDetected) {

                    // Set GPIO35 output to low level after long pressing the button and releasing it
                    ESP_LOGI(TAG, "SHUTDOWN!");
                    longPressDetected = false; // Reset long press detection flag
                    Shutdown(dev);
                }
            }
        }
    }

    // Check if the button is in long press state
    if (buttonState == 0 && (esp_timer_get_time() - lastDebounceTime >= longPressDuration)) {
        // Long press button
        ESP_LOGI(TAG, "Long Press");
        longPressDetected = true; // Set long press detection flag
    }
    lastButtonState = reading;
}
void Fall_Asleep(void)
{
}
void Restart(void)
{
}
void Shutdown(TFT_t dev)
{
    gpio_set_level(outputPin, false);
    lcdBacklightOff(&dev);
}
static void configure_GPIO(int pin, gpio_mode_t Mode)
{
    gpio_reset_pin(pin);
    gpio_set_direction(pin, Mode);
}
void PWR_Init(void)
{
    configure_GPIO(inputPin, GPIO_MODE_INPUT);
    configure_GPIO(outputPin, GPIO_MODE_OUTPUT);
    gpio_set_level(outputPin, false);
    ESP_LOGI(TAG, "PWR Key Init");
}
