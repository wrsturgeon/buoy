# Mostafa Afr & Will Sturgeon
## The _Heart_-Throbs ❤️

Notable features:
- Scrolling display, corrected _before_ displaying to pin the moving average, plus a real-time running peak (surprisingly difficult!)
- Circular buffers (fully-featured potential standalone library) used across the codebase, e.g. heart rate calculation over a time window
- Displaying integers uses an array of function pointers, choosing an integer in O(1)
- Custom font by hand! Writes sideways as well!
- Hardware filtering (cred to Mostafa)
