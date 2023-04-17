# The _Heart_-Throbs ❤️
## UPenn ESE 350 Final Project

### Instructions
Just run `make`. (This should install dependencies as well).

### Notable features:
- Bare-metal register-level C on the ESP32 with a manual build script & flashing routine
  - It's difficult to articulate how much of a pain in the ass this was
  - E.g., some essential registers (like `SENS_SAR_SLAVE_ADDR1`—I dare you to look it up) are just entirely absent from the manual
  - Yet at the same time the official library (ESP-IDF) is undocumented and, in the name of safety, ~20x slower than this repo (no shade tho!)
- Scrolling display with interpolated lines, corrected _before_ displaying to pin the moving average, plus a real-time running peak (surprisingly difficult!), all fast enough to display in real time
- Fixed-point division, aligned with real-world seconds (by some fucking crazy unit math), without overflow on the full range of possible human heart rates
- Custom font by hand! Writes sideways as well!
- Displaying integers uses an array of function pointers, choosing an integer in O(1)
- Hardware filtering (cred to Mostafa)
- Automatically write VSCode JSON when compiling for mouse-over config info (see `esp32/Makefile`)
