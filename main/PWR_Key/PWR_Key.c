#include "PWR_Key.h"
#include "esp_log.h"
#include "esp_timer.h"
static const int inputPin = 40; // Define GPIO40 as input
static const int outputPin = 41; // Define GPIO41 as output
static bool buttonState = false;
static bool lastButtonState = false;
static int64_t lastDebounceTime = 0;
static int64_t longPressDuration = 2e6; // Long press duration
static bool longPressDetected = false;
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
                    ESP_LOGI(TAG, "SHUTDOWN!");
                    ESP_LOGI(TAG, "Time now: %lld", esp_timer_get_time() - lastDebounceTime);
                    longPressDetected = false; // Reset long press detection flag
                    Shutdown(dev);
                }
            }
        }
    }

    // Check if the button is in long press state
    if (buttonState == 0 && (esp_timer_get_time() - lastDebounceTime >= longPressDuration)) {
        // Long press button
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
    lcdBacklightOff(&dev);
    gpio_set_level(outputPin, false);
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
    gpio_set_level(outputPin, true);
    ESP_LOGI(TAG, "PWR Key Init");
}
