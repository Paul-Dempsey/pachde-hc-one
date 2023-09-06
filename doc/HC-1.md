# HC-1

HC-1 supports the Haken Continuum and other Haken Audio Eagan Matrix-based devices like the Eagan Matrix Eurorack module (EMM) and the Osmose from Expressive E.
It is focused on having quick and easy access to presets on the device, favorites, plus CV control of the most essential performance settings, on a compact panel.

HC-1 is not a replacement for the Haken Editor.

**Note** — This documentation uses the terms *Eagan Matrix*, *EM*, and *device* interchangeably.
They all refer to whatever Eagan-Matrix-based device you are controlling with HC-1.

HC-1 was designed for Eagan Matrix firmware version 10.09.
Earlier or later firmware versions may require changes to HC-1.
If you have an issue, please open one in the Github Issues or send me an email.

![The HC-1 mdoule](HC-1.png)

When first loaded, there is a long initialization time to read all user and system presets from the device.
The Eagan Matrix devices require a high-quality MIDI connection.
This process can sometimes fail, especially when you have a poor MIDI connection.
When this happens, you may need to reboot HC-1 from the menu, unplug and re-plug the MIDI connection, or power cycle the device.
When possible, make sure you're using a connection directly to the computer, and not through a USB hub.
Even then, you may need to use a different USB port on your computer.
Be sure to use the MIDI cable that came with your device or another high-quality USB cable.

## A tour of the user interface

There's a lot of UI packed into this module, so let's take a tour.

From right to left, top to bottom:

- Device status indicator light. This shows the same thing the status light in the Haken Editor does.

- Module title **HC-1**

- Current preset name in Gold.
  This space also carries status messages in gray while the module is initializing and processing actions on the device other than playing, such as getting the definition of a new preset selected on the device.

- The pink heart is the Favorite button/indicator.
  The heart is filled when the selected preset has been favorited.
  To add a preset to your favorites list, click the outlined heart.
  Clicking a filled heart removes the preset from the favorites list.

  There are menu options to save and load favorites, so you can keep as many set lists as you like.

- Tabbed preset listing. Click on **User**, **Favorite**, or **System** to see the corresponding list of presets.

  User presets are always in the order set on the device.
  Favorites are shown in the order that they are added.
  At this writing, you can't change the order of favorites in HC-1, but it is possible to manually edit the saved Favorites file in a text editor.
  System presets can be sorted Alphabetically (the default), by Category, and by System order.
  The options for this are in the menu.

- Paging is to the right of the tabs.
  Current page number is show above the Up and Down buttons.
  Roll the mouse wheel while over the tabs or page buttons to quickly scroll the list.

- Click on a preset to select it. The selected preset is outlined in gold.
  If the preset is a User preset or a Favorite, the appropriate symbols are shown in the lower left.
  The two-letter shorthand for the preset's category is shown in the lower right.
  Hovering over the preset shows a tooltip with more details about the preset.

- At the bottom of the preset tabs, you can see two tiny lights that move from left to right as MDIDI communications travel between the module and the Egagan Matrix.
  The purple light shows progress of MIDI messages **sent**, and the green light shows the progress of MIDI Messages **received**.
  These allow you to see when the module loses MIDI communication with the device, which unfortunately happens from time to time through no fault of HC-1.

- Below the preset listing are the macro and recirculator controls.
  When a preset is selected, the preset metadata is parsed, and if it contains Macro labels, these are read and shown.
  If no label is defined for a macro, you see the default Roman numerals.
  Some presets make use of a macro control, but do not provide a label.
  Similarly, there are occasionally labels defined for macros that are not actually used by the preset.

- To the right of the first row is a small graph showing the DSP usage of the preset.

  When this is gray, the DSP usage is not available.
  DSP usage is reported by the device heartbeat, which you can turn off in the menu.
  Why turn it off? Becuase (unfortunately) the heartbeat can cause glitches in the audio output from the EM.

- Each Macro, Recirculator, and Volume control can operate independently (with no cable connected to the input jack), with absolute CV control, or combined Knob + CV Relative control.
  Click the **Relative mode** button to toggle between Absolute and Relative mode.
  When a cable is connected, the value of the parameter is either the input value (Absolute mode), or the sum of the Knob position and the CV value (Relative mode).
  The Relative mode indicator light is lit when the control is in relative mode.

  ![Anatomy of HC-1 controls](knob-anatomy.svg)

  There are menu options under **Knob control** to set all controls to Absolute or Relative mode, and to set all knobs to zero or middle position at once.

- The boxed controls are the Eagan Matrix Recirculator.
  The labels change to reflect the current preset's Recirculator type and the appropriate R1-R4 and Recirculator Mix (R5).
  The indicator light button reflects the use of Extended (half-sample rate) computation. You can toggle extended computation on and off by clicking the light button.

  You can temporarily change the Recirculator type in the **Recirculator** menu.
  Changes in recirculator type and extended mode are not saved.
  When you refresh or return to the preset they will reflect the preset definition.

- The Volume knob and Mute button controls the Eagan Matrix Master Level.

- The bottom row shows the following, from left to right:

  - Midi device input and output selectors. If you have only one Eagan Matrix device, you should never need to use these, becuase HC-1 generally finds the connected Eagan Matrix device.

  - Test note button. Click to send a Middle C (Note 60) *Note on* MIDI message, and Ctrl+Click (Cmd on Mac) to send the corresponding *Note off*.
    You'll see the little circle next to the button light up when a note is on, either through the Test note button, or when playing the device.

  - The row of indicators show different parts of the state of the module/device connection and initialization process.
    When everything is conencted and working properly, you should see a row of blue indicators.
    When an indicator is orange, it means the corresponding item is in-progress.

    If the module gets stuck, an indicator may remain orange, and you may see the MIDI communication lights stop moving.
    If you see this happen you may need to reboot the module from the menu, or unplug/power cycle your device.

    These indicators are:

    1. Note
    1. MIDI output device connection
    1. MIDI input device connection
    1. Connected device is a recognizxed Eagan Matrix
    1. System presets initialized
    1. User presets initialized
    1. Current preset initialized
    1. Saved preset from previous session available
    1. Module has requested updates from the device
    1. Device heartbeat status

  - In the middle is the pachde (#d) logo
  - The name of the currently connected EM device
  - The device firmware version.

## Menu options

| Menu | Option | Description |
| -- | -- | -- |
| **Knob control** | **Center knobs** | Set all knobs to center position. |
|  | **Zero knobs** | Set all knobs (except Volume) to zero position. |
|  | **Absolute CV** | Set all controls to Absolute CV mode |
|  | **Relative CV** | Set all controls to Relative CV mode |
| **Recirculator** | **Reverb** | Use Reverb |
|  | **Mod delay** | Use the Mod delay |
|  | **Swept Echo** | Use Swept echo |
|  | **Analog Echo** | Use Analog Echo. Be careful switching to this one. If Feedback or R Mix is high, damaging high volumes can result. |
|  | **Delay LPF** | Use delay LPF (low-pass filter) |
|  | **Delay HPF** | Use Delay HPF (high-pass filter) |
| **Module** | **Reboot HC-1** | Re-initialize HC1. May be needed if MIDI communication isn't working. This always re-scans all User and System presets from the device. |
|  | **Suppress heartbeat handshake** | Do not periodically poll the device. You may want to do this because the heartbeat can sometimes cause glitches in the audio output. Without the heartbeat, the DSP usage indicator doesn't work. |
|  | **One handshake** | Send one heartbeat handshake. When heartbeat is suppressed, use this to take a snapshot of the DSP usage. |
|  | **Request config** | Request the current preset (edit slot) configuration from the device. |
|  | **Reset MIDI I/O** | Re-initialize MIDI I/O. |
| **Favorites** | **Clear favorites** | Empty the Favorites list. |
| | **Open favorites...** | Load favorites from a file (`.fav` or `.json`) |
| | **Save favorites as...** | Save current favorites list to a `.fav` or `.json` file. |
| **Presets** | **Restore last preset at startup** | Enable to select the last preset used in HC1 at startup. Disable to use whatever preset is current on the device. |
|  | **Use saved presets** | Turn on to use a cached list of user and system presets instead of (slowly) re-querying the device at startup. |
|  | **Save presets** | Force-save the preset lists. |
|  | **Refresh User presets** | Refresh the list of User presets from the device. Do this after adding or removing new user presets using the Haken Editor, and you have **Use saved presets** turned on. |
| **Sort System presets** | **Alphabetically** | (default) Show system presets alphabetically by name. |
|  | **by Category** | Sort first by Category, then alphabetically within the category. |
|  | **in System order** | Show system presets in internal System order. This is the numerical order in the Haken Editor "File 2" list, which is mostly but not entirely alphabetical. |

## Notes

- **File usage:** After using this module, you can find a folder of data files that the module saves.
The folder _Rack user folder_`/pachde-hc-one` contains per-EM-device Favorites, User, and System files in JSON format.
It is also the default folder presented for saving and loading favorites files.
You can, of course, save favorites anywhere you like, but this is the default, per VCV rack convention.

- **Preset `.mid`:** At this time, this plugin has no support for managing preset .mid files.
I do have plans to build such functionality, but it may not be in the Rack plugin format.

- **Multiple EM Devices:** The USB plug on my ContinuuMini is broken, so I haven't been able to test the multiple-EM device scenario.
That's where *you* come in!
If you own multiple EM-based devices, you should be able to use an instance of HC-1 for each device in the same Rack patch to control multiple devices.
The automatic discovery will always choose the first EM device it finds, but you can choose another from the MIDI controls in the bottom left:
Pick the _output_ device from the right-hand MIDI selector and HC-1 will automatically find the corresponding input.

- **Factory presets:** For beta, there are no Rack module factory presets.
Please let me know if you have something useful that comes from using Rack module presets.

- **HC-2:** The HC-2 module is present in the plugin.
It is currently used for module prototyping, diagnostics and debugging.
It's possible we may turn to it for support purposes, but HC-1 has most of what we need.
HC-2 is really not useful to users at this time and may occasionally cause VCV Rack to deadlock.
The CV map display on HC-2 can be somewhat interesting if you are familiar with the EM usage of Channel 1 and Channel 16 CCs.

- **More functionality:** I plan to add extender modules for controlling most other things on the device.
I am *not* planning to create a complete preset editor.
That's not really suitable for a VCV rack module and it would require additional proprietary internal information from Haken Audio.

  I am very interesting to hear from you what will be most important controls to include in future HC modules.

  - I have some ideas for other potential useful features, such as:

    - Save and load profiles. These would contain a profile of settings including pedal assignments, rounding, MIDI routing, and so on.
    These could be independent and also optionally associated with a specific preset (or presets), so that when you load a preset of any kind (user favorite, or system), the associated profile is applied.

    - User preset archive and restore. This possibly in a separate program rather than VCV Rack modules.

![pachde (#d) logo](Logo.svg)
