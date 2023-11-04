# ptpview
Some PTP extensions for Magic Lantern:
- Adds HD liveview with overlays
- Adds CHDK opcode extension

![demo](https://eggnog.danielc.dev/f/75-53fwdwdn24lhaixlywg9h03z54pmf9.png)

The goal is to achieve near HDMI speeds, so we can have liveview without a capture card or HDMI cable.

# v1
- Heavy CPU usage for data processing
- slows down overlays
- Downscaled (320x200)
- 7fps

# v2
- Overlays work
- Sends raw data to viewer
- Full HD resolution
- 15fps

# v3
- Fix bugs
- Ready for public use

# Pros over HDMI
- Allows normal PTP operations to be run while running liveview, from one USB cable
- No external capture device needed or HDMI cable
