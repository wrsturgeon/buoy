#ifndef BUOY_INCLUDE_GRAPHICS_H
#define BUOY_INCLUDE_GRAPHICS_H

#include <stdint.h>

void init_graphics(void);

uint8_t is_heartbeat(uint16_t v);

void update_bpm(uint16_t /* just in a hell of a case */);

#endif // BUOY_INCLUDE_GRAPHICS_H
