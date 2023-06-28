# ptpview

ptpview is a Magic Lantern module that allows real time liveview & overlays over USB only. The goal is to achieve near HDMI quality and speed.

A basic PoC implementation is [here](https://github.com/petabyt/camlib/blob/master/test/live.c), a more polished one will be published for [CamControl](https://camcontrol.danielc.dev/).

![demo](https://eggnog.danielc.dev/f/75-53fwdwdn24lhaixlywg9h03z54pmf9.png)

# v1 (opcode 0x9997)
- Heavy CPU usage for data processing
- slows down overlays
- Downscaled (320x200)
- 7fps

# v2 (opcode 0x9996)
- Overlays work, 
- Sends raw data to viewer
- Full HD resolution
- 15fps

# Pros over HDMI
- Allows scripts to be run while running liveview, from one connection
- No external capture device needed
