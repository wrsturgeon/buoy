https://user-images.githubusercontent.com/79714036/236497741-979da34e-243d-4cca-9406-edeb8d0d33e4.mov

# The _Heart_-Throbs ❤️
UPenn ESE 350 Final Project\
Will Sturgeon & Mostafa Afr

### About
This is wearable watch with a finger clip that is able to measure a patient's heart rate in real time and display the wave form and BPM on the screen of the watch!

### Instructions
Just run `make`. (This should install dependencies as well).

### Notable features:
- Bare-metal register-level C on the ESP32 with a manual build script & flashing routine
  - It's difficult to articulate how much of a pain this was to get to work the feather working
  - E.g., some essential registers (like `SENS_SAR_SLAVE_ADDR1`—I dare you to look it up) are just entirely absent from the manual
  - Yet at the same time the official library (ESP-IDF) is undocumented and, in the name of safety, ~20x slower than this repo (no shade tho!)
- Scrolling display with interpolated lines, adjusted in real time to a moving average and running peak, all fast enough to display in real time
- Fixed-point division, aligned with real-world seconds (by some crazy unit math), without overflow on the full range of possible human heart rates
- Custom font by hand! Writes sideways as well!
- Displaying integers uses an array of function pointers, choosing an integer in O(1)
- Automatically write VSCode JSON when compiling for mouse-over config info (see `esp32/Makefile`)

### Libraries
None so far.

**Disclaimer**: as noted above, the ESP32 manual is literally incomplete, and so if a module was not working after multiple hours of following the instructions and exhausting reasonable what-ifs, I would comb through an [ESP-IDF](https://github.com/espressif/esp-idf) implementation to see which registers were missing. None of their code survives in any form, and I thoroughly documented my code (unlike ESP-IDF!) to demonstrate understanding and help my future self out.

### Demonstration
Here is the current state of the project: https://youtube.com/shorts/ZsG0fzAm2Nc
We have gotten the heart beat monitor to work properly with a feather in baremetal on the LCD, and are currently working on adding a pulse oximeter as the next feature!
