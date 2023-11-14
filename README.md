## USB extensions for Magic Lantern
- [x] Adds HD liveview with UI overlays
- [x] Implements CHDK opcode and commands
- [ ] Complete SD card file API (upload, download, delete, rename, move)
- [ ] Download MLVs and any other file over PTP
- [ ] Update ML or add new modules over PTP

![demo](https://eggnog.danielc.dev/f/75-53fwdwdn24lhaixlywg9h03z54pmf9.png)

The goal is to achieve near HDMI speeds, so we can have liveview without a capture card or HDMI cable.

## Custom Liveview
- Overlays work, doesn't slow down UI
- Sends raw data to viewer, low CPU/RAM usage on camera
- Full HD (same as HDMI)
- 15fps

# Pros over HDMI
- Allows normal PTP operations to be run while running liveview, from one USB cable
- No external capture device needed or HDMI cable (which in my experience, are very bulky)
