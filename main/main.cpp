#include "esp_timer.h"
#include "PWR_Key.h"
#include "QMI8658.h"
#include "st7789.h"
#include "Scene.h"
#include "esp_log.h"
static const char* TAG1 = "FLUID_SIM";
static Scene scene;
static TFT_t dev;
void Driver_Loop(void* parameter)
{
    // Wireless_Init();
    while (1) {
        PWR_Loop(dev);
        read_sensor_data();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}
void Driver_Init(void)
{
    PWR_Init();
    setup_sensor();
    xTaskCreatePinnedToCore(
        Driver_Loop,
        "Other Driver task",
        4096,
        NULL,
        11,
        NULL,
        tskNO_AFFINITY);
}

void LCD_Init(void){
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
    for (int i = 1;; ++i) {
        if (i% 10 == 0)
        {
            scene.add_object(Vec(0.5, 0.3), 100);
            ESP_LOGI(TAG1, "Added %i Fluid, Now we have %i Particles.", AddNum, scene.getNumParticles());
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
        
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
