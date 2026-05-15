#include "app_main.h"

void app_main(void)
{
    const uint8_t tick = '*';
    while (1) {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        HAL_UART_Transmit(&huart1, &tick, 1, HAL_MAX_DELAY);
        HAL_Delay(100);
    };
}
