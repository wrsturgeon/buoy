#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <esp_task_wdt.h>
#include <rtc_wdt.h>

__attribute__((always_inline)) inline static void feed_watchdog(void) {
  rtc_wdt_feed();
}

#endif // WATCHDOG_H
