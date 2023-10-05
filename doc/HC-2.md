# Module HC-2

HC-2 is a companion to HC-1, offering control of Rounding (including tuning/scale quantization), Pedal assignments, and a live map of MIDI channel 1 and MIDI channel 16 control codes.

Let me know if the HC-2 pedal controls are useful. They are redundant given the complete support for pedals present in the Pedal 1 and Pedal 2 modules.

For more detailed pedal control, see [Pedal 1 and Pedal 2](Pedals.md).

![HC-2 module](HC-2.png)

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
