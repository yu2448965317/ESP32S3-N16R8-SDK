#include "lvgl_gui.h"
#include "driver/gptimer.h"
gptimer_handle_t gptimer = NULL;
gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
};
void my_timer_init()
{
ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
}
uint64_t micros()
{
   static uint64_t count;
    gptimer_get_raw_count(gptimer, &count);
    return count;
}
