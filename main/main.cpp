#include "PWR_Key.h"
#include "QMI8658.h"
#include "Scene.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "st7789.h"
#include <rom/ets_sys.h>
static const char* TAG1 = "FLUID_SIM";
static Scene scene;
static TFT_t dev;
void Driver_Loop(void* parameter)
{
    while (1) {
        PWR_Loop(dev);
        read_sensor_data();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(NULL);
}
void setupBoot()
{
    // Configure BOOT button (GPIO0)
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_0); // GPIO0
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
}

// 添加按键消抖函数
bool isButtonPressed() {
    static uint32_t last_press_time = 0;
    const uint32_t debounce_delay = 200; // 消抖延时200ms
    
    if (gpio_get_level(GPIO_NUM_0) == 0) { // Boot键按下为低电平
        uint32_t now = esp_timer_get_time() / 1000; // 转换为毫秒
        if ((now - last_press_time) > debounce_delay) {
            last_press_time = now;
            return true;
        }
    }
    return false;
}

void Driver_Init(void)
{
    PWR_Init();
    setup_sensor();
    setupBoot();
    xTaskCreatePinnedToCore(
        Driver_Loop,
        "Other Driver task",
        4096,
        NULL,
        11,
        NULL,
        tskNO_AFFINITY);
}

void LCD_Init(void)
{
    spi_clock_speed(80000000); // 80MHz
    spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_BL_GPIO);
    lcdInit(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);
    ByteBlueInit();
    ByteWhiteInit();
    lcdFillScreen(&dev, WHITE);
}
void FluidSimLoop(void* parameter)
{
    int64_t start_time = esp_timer_get_time();
    for (int i = 1;; ++i) {
        scene.update(scene.getdt(), Vector3(0 - acc.y, acc.x, 0));
        scene.render(&dev);
        if (i % 1000 == 0) {
            ESP_LOGI(TAG1, "Finished %i FluidSimLoop, duration: %lld ms", i, (esp_timer_get_time() - start_time) / 1000);
            start_time = esp_timer_get_time();
        }
    }
    vTaskDelete(NULL);
}
void AddFluidLoop(void* parameter)
{
    int AddNum = 100;
    
    for (;;) {
        if (isButtonPressed()) {
            scene.add_object(Vec(0.5, 0.3), 100);
            ESP_LOGI(TAG1, "Added %i Fluid, Now we have %i Particles.", AddNum, scene.getNumParticles());
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(NULL);
}

extern "C" void app_main();
void app_main(void)
{
    Driver_Init();
    LCD_Init();
    scene.init(Vec(0.5, 0.3));
    ESP_LOGI(TAG1, "Finished init.");
    xTaskCreatePinnedToCore(
        FluidSimLoop,
        "FluidSim",
        10240,
        NULL,
        10,
        NULL,
        tskNO_AFFINITY);

    xTaskCreatePinnedToCore(
        AddFluidLoop,
        "AddFluidLoop",
        10240,
        NULL,
        10,
        NULL,
        tskNO_AFFINITY);
}
