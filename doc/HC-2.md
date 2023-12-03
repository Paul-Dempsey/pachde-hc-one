# Module HC-2

HC-2 is a companion to HC-1, offering control Tilt EQ, and a live map of MIDI channel 1 and MIDI channel 16 control codes.

HC-2 is in the process of being retired, and the required EM functionality is being moved to separate modules.

![HC-2 module](HC-2.png)

## Tilt EQ

Controls for the EM Tilt EQ feature. See the Continuum User Guide for more details.

Each knob has a CV input and relative button, the same as the knobs on HC-1.

- The **Tilt** is flat at 64.

- The **Frequency** knob range is from 120hz for 0%, to 15kHz at 100%.

- The **Mix** knob range is from completely dry at 0% to wet at 100%.

## CC Maps

At the bottom of the module are two live maps of the MIDI control code (cc) traffic.
The upper map is for Channel 1 (master channel) and the lower one for Channel 16 (EM control).
The maps are cleared at the start of loading a new preset.

The height of each bar relects the last received value of a particular CC.
Hover over the CC maps to see the cc, purpose, and current value of MIDI control codes with nonzero values.
Values of 127 are in red, and values of 64 are in green.
Everything else is light gray.

CC maps may not survive to release.
They have proven to be useful to the developer during the development of this plugin, which involves a fair amount of poking and prodding the device to discover how things works.
It's also a bit of eye candy, so why not, if we don't need the space for something more important?
