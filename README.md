# The _Heart_-Throbs ❤️
## UPenn ESE 350 Final Project

### Instructions
Just run `make`. (This should install dependencies as well).

### Notable features:
- Bare-metal C on the ESP32, manual build script & flashing routine instead of the Arduino IDE
- Scrolling display with interpolated lines, corrected _before_ displaying to pin the moving average, plus a real-time running peak (surprisingly difficult!), all fast enough to display in real time
- Fixed-point division, aligned with real-world seconds (by some fucking crazy unit math), without overflow on the full range of possible human heart rates
- Circular buffers (fully-featured potential standalone library) used across the codebase, e.g. heart rate calculation over a time window
- Custom font by hand! Writes sideways as well!
- Displaying integers uses an array of function pointers, choosing an integer in O(1)
- Hardware filtering (cred to Mostafa)
- Automatically write VSCode JSON when compiling for mouse-over config info (see `esp32/Makefile`)
