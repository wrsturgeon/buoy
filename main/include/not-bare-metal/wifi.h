#ifndef NOT_BARE_METAL_WIFI_H
#define NOT_BARE_METAL_WIFI_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void wifi_tasks(void* /* unused */) {
  do {
    vTaskDelay(-1);
  } while (1);
}

#endif // NOT_BARE_METAL_WIFI_H
