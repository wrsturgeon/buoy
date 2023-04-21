#ifndef NOT_BARE_METAL_TASKS_H
#define NOT_BARE_METAL_TASKS_H

#include "not-bare-metal/wifi.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

__attribute__((always_inline)) inline static void start_freertos_tasks(TaskFunction_t bare_metal_task) {
  BaseType_t return_code;

  // Start non-WiFi-enabled tasks (bare-metal)
  switch (return_code = xTaskCreatePinnedToCore(bare_metal_task, "Non-WiFi tasks", CONFIG_ESP_MAIN_TASK_STACK_SIZE, 0, 2, 0, APP_CPU_NUM)) {
  case pdPASS: break;
  default: // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "`xTaskCreatePinnedToCore(non_wifi_tasks, ...` failed with code %d\r\n", return_code);
    return;
  }

  // Start WiFi-enabled tasks (not bare-metal)
  switch (return_code = xTaskCreatePinnedToCore(wifi_tasks, "WiFi tasks", CONFIG_ESP_MAIN_TASK_STACK_SIZE, 0, 2, 0, PRO_CPU_NUM)) {
  case pdPASS: break;
  default: // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "`xTaskCreatePinnedToCore(wifi_tasks, ...` failed with code %d\r\n", return_code);
    return;
  }
}

#endif // NOT_BARE_METAL_TASKS_H
