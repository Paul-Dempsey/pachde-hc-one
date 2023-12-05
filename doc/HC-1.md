# Module HC-1

HC-1 is the core module for the pachde (#d) HC One plugin.
All other modules in the plugin are companions for HC-1, and don't do anything without HC-1 in the patch.

HC-1 supports the Haken Continuum and other Haken Audio Eagan Matrix-based devices like the Eagan Matrix Eurorack module (EMM) and the Osmose from Expressive E.
It is focused on having quick and easy access to presets on the device, favorites, plus CV control of the most essential performance settings, on a compact panel.

If you're new to VCV Rack, make sure to get familiar with it. See [Getting Started](https://vcvrack.com/manual/GettingStarted) in the VCV Rack manual.

HC-1 is not a replacement for the Haken Editor.
The Haken Editor is still required for managing and editing user presets.

**Note** — This documentation uses the terms *Eagan Matrix*, *EM*, and *device* interchangeably.
They all refer to whatever Eagan-Matrix-based device you are controlling with HC-1.

HC-1 was designed for Eagan Matrix firmware version 10.09.
Earlier or later firmware versions may require changes to HC-1.
If you have an issue, please open one in the Github Issues or send me an email.

To install the plugin from Github, please see **Install** in the [README](https://github.com/Paul-Dempsey/pachde-hc-one#installing).

Here is the HC-1 module:

![The HC-1 module](HC-1.png)

When first loaded, there is a long initialization time (a minute or two) to find the EM device, initialize the MIDI ports, and read all user and system presets from the device.
By default, the list of system presets are saved, so the next time you open the patch, it skips loading the list from the device and you should be up and running quickly.

You can watch the prograss of the multi-step initialization process with the row of dots on the bottom.
When everything is connected and working as expected, all the dots are blue, and you can begin playing.

## Troubleshooting

If you are running on Windows, remember that no other software can open the same MIDI device, unless you have third-party virtual MIDI software or drivers installed.
So, make sure that you are not running the Haken Editor, a DAW, or other software that has the device open.

If HC-1 is unable to successfully connect and initialize fully, check that the Haken Editor can connect to the device.
If the Haken Editor cannot connect, then neither can HC-1.

Eagan Matrix devices require a high-quality MIDI connection.
The HC-1 initialization process can sometimes fail, especially when you have a poor MIDI connection.
When this happens, you may need to reboot HC-1 from the menu, unplug and re-plug the MIDI connection, or power cycle the device.
In some cases on Windows, you may even need to reboot your computer.

When possible, make sure you're using a connection directly to the computer, and not through a USB hub.
Even then, you may need to use a different USB port on your computer.
Be sure to use the MIDI cable that came with your device or another high-quality USB cable.

The Haken Editor has a MIDI test feature that you can use to troubleshoot your MIDI connection.
If the Haken editor doesn't report a reliable connection, you can try different ports and USB cables until it reports a quality connection.

The little green dot that travels across the bottom of the preset list indicates the progress of MIDI messages received from the device.
This should move rapidly while initializing the device, and while playing the device.
If this stops moving while loading or playing the device, then the MIDI connection has been lost, and you must take steps to recover the connection.

If you still have trouble with the first-time initialization process, you may need to configure the startup process.
See [Configuring startup parameters](#configuring-startup-parameters) below.

## A tour of the user interface

There's a lot packed into HC-1, so let's take a tour.

From right to left, top to bottom:

- Device status indicator light. This shows the same thing the status light in the Haken Editor does.

- Module title **HC-1**

- Current preset name in Gold.
  This space also carries status messages in gray while the module is initializing and processing actions on the device other than playing, such as getting the definition of a new preset selected on the device.

- The pink heart is the Favorite button/indicator.
  The heart is filled when the selected preset has been favorited.
  To add a preset to your favorites list, click the outlined heart.
  Clicking a filled heart removes the preset from the favorites list.

  There are menu options to save and load Favorites, so you can keep as many set lists as you like.

- Tabbed preset listing. Click on **User**, **Favorite**, or **System** to see the corresponding list of presets.

  Right click a preset for Favoriting and ordering options if the preset is a favorite. You also favorite and un-favorite the current preset with the heart button in the upper right.

  - **User**
    User presets are always in the order set on the device.

  - **Favorite** Favorites are shown in the order that they are added. Favoriting a preset always adds it to the end of the list.
  Right click a preset for options to change it's order in the list, or to un-Favorite it.
  Right click the **Favorite** tab for sorting and Favorites file operations.
  Sorting from the **Favorite menu** is a one-time operation, unlike the System tab where the selected sort is persistent.
  If you load and save presets in the Haken Editor by Groups, you can import an HE group list (`.txt`) file into Favorites.

  See the companion **[HC-3](HC-3.md)** module for more options for working with favorites files.

  - **System** System presets can be sorted Alphabetically (the default), by Category, and by internal device System order.
  The options for this are in the **System** tab right click menu.

- Paging is to the right of the tabs.
  Current page number is show above the Page up and Page down buttons.
  Roll the mouse wheel while over the tabs or page buttons to quickly scroll the list.

- Click on a preset to select it. The selected preset is outlined in gold.
  If the preset is a User preset or a Favorite, the appropriate symbols are shown in the lower left.
  The two-letter shorthand for the preset's category is shown in the lower right.
  Hovering over the preset shows a tooltip with more details about the preset.

  Right click a preset for a menu of options for that preset.

- Under the paging buttons is a small graph showing the DSP usage of the preset.

  When this is gray, the DSP usage is not available.
  DSP usage is reported by the device heartbeat, which you can turn off in the menu.
  Why turn it off? Becuase (unfortunately) the heartbeat can cause glitches in the audio output from the EM.

- The row of 4 Rounding Status LEDs under the DSP graph show the same information that a Continuum shows in the lower right of its display.

- Below the Rounding Status LEDs, are preset navigation buttons similar to the buttons on the Continuum.
  Clicking the - or + button selects previous or next preset in the list.
  Shift+Click moves by 10 presets.
  Ctrl+Click (Cmd on Mac) and Shift+Ctrl+Click moves in device order, independent of the sorting of the current tab.
  This is the same ordering that the buttons on the device use.

- At the bottom of the preset tabs, you can see two tiny lights that move from left to right as MIDI communications travel between the module and the Eagan Matrix.
  The purple light shows progress of MIDI messages **sent**, and the green light shows the progress of MIDI Messages **received**.
  These allow you to see when the module loses MIDI communication with the device, which unfortunately can happen.

- The preset's Pedal assignments are shown to the bottom right of the preset list.
  If a pedal is assigned to a macro or recirculator, you will see a **`1`** or **`2`**
next to the knob, indicating which pedal is assigned to that knob.

- Below the preset listings are the macro and recirculator controls.
  When a preset is selected, the preset metadata is parsed, and if it contains Macro labels, these are read and shown.
  If no label is defined for a macro, you see the default Roman numerals.
  Some presets use a macro controller, but do not provide a label.
  Similarly, there are occasionally labels for macros that are not used.

- Each Macro, Recirculator, and Volume control can operate independently (with no cable connected to the input jack), with absolute CV control, or combined Knob + CV Relative control.
  Click the **Relative mode** button to toggle between Absolute and Relative mode.
  When a cable is connected, the value of the parameter is either the input value (Absolute mode), or the sum of the Knob position and the CV value (Relative mode).
  Absolute mode takes Unipolar 0-10v signals and Relative mode takes Bipolar or Unipolar signals.
  The Relative mode indicator light is lit when the control is in relative mode,
and the track around the knob becomes gold, with an indicator dot showing the effective value.

  ![Anatomy of HC-1 controls](knob-anatomy.svg)

  There are menu options under **Knob control** to set all controls to Absolute or Relative mode, and to set all knobs to zero or middle position at once.

- The **Ready** output jack produces a trigger when the EM device is fully initialized and ready to work with.
  This is the only CV output produced by HC-1.
  CV inputs are processed only after the device has reached "Ready".

- The boxed controls are the Eagan Matrix Recirculator.
  The labels on the knobs change to reflect the current preset's Recirculator type and the appropriate R1-R4 and Recirculator Mix (R5) assignments.
  When you see a number next a knob, it means that the corresponding pedal is assigned to that parameter. (You can change pedal assignments using the **Pedal 1** and **Pedal 2** modules.)

  The left light button enables and disables the recirculator. When lit the recirculator is active.
  Recirculator on/off is global and doesn't change between presets.
  The right light button reflects the use of Extended (half-sample rate) computation.
  You can toggle extended computation on and off by clicking the light button.


  You can temporarily change the Recirculator type in the **Recirculator** menu.
  Changes in recirculator type and extended mode are not saved.
  When you refresh or return to the preset they will reflect the preset definition.

- The Volume knob and Mute button controls the Eagan Matrix Post Level.

- The bottom row shows the following, from left to right:

  - EM Midi device selector. If you have only one Eagan Matrix device, you should never need to use this. HC-1 finds a connected EM device automatically.

  - Test note button. Click to send a Middle C *Note on* MIDI message (Note 60, velocity 64), and Ctrl+Click (Cmd on Mac) to send the corresponding *Note off*.
    You'll see the little circle next to the button light up when a note is on, either through the Test note button, or when playing the device.
    This indicator is not completely reliable, but can be useful to get a sense of MIDI activity.

  - The row of indicators show different parts of the state of the module/device connection and initialization process.
    When everything is conencted and working properly, you should see a row of blue indicators.
    When an indicator is orange, it means the corresponding item is in-progress.

    Hover over an indicator to see what item it reflects and its current status.

    If the module gets stuck, an indicator may remain orange and you may see the MIDI communication lights stop moving.
    If you see this happen you may need to reset MIDI I/O, reboot the module from the menu, or unplug the Midi or USB or even power cycle your EM device.

  - In the middle is the pachde (#d) logo
  - The name of the currently connected EM device, if any
  - The device firmware version.

## Module Menu

| Menu | Option | Description |
| -- | -- | -- |
| **Presets** | **Restore last preset at startup** | Enable to select the last preset selected in HC-1 at startup. Disable to use whatever preset is current on the device. |
|  | **Use saved system presets** | Turn on to use a cached list of system presets instead of (slowly) re-querying the device at startup. |
|  | **Use saved user presets** | Turn on to use a cached list of system presets instead of (slowly) re-querying the device at startup. |
|  | **Refresh current preset** | Request the current preset (edit slot) configuration from the device. |
|  | **Refresh User presets** | Refresh the list of User presets from the device. Do this after adding or removing new user presets using the Haken Editor, and you have **Use saved presets** turned on. |
|  | **Refresh System presets** | Refresh the list of System presets from the device. |
| **Knob control** | **Center knobs** | Set all knobs to center position. |
|  | **Center Macro knobs** | Center only Macro knobs to center position. |
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
|  | **Send one handshake (ping)** | Send one heartbeat handshake to ping the device. Use this to take a snapshot of the DSP usage. |
|  | **Remake QSPI data** | Re-initialize QSPI (additive) data. Same functions as in the Haken Editor. |

## Favorite tab menu

Right click on the **Favorite** tab to see the **Favorite menu**.

| Menu | Option | Description |
| -- | -- | -- |
| **Sort** | **Alphabetically** | Sort favorites alphabetically by name. |
|  | **by Category** | Sort favorites first by Category, then alphabetically within the category. |
| **Open...** | | Load favorites from a file (`.fav` or `.json`) |
| **Add from...** | | Append to the current list from a favorites file. |
| **Import Haken Editor group list** | | Create Favorites from a Haken editor group listing. Presets named in the group file are first matched by name from user presets, then from system presets. The first matching name is used. If a name is not found, it is silently omitted. |
| **Forget file and clear** | | Forget the current favorites file (if any) and empty the list. |
| File _name_ | | Shows current favorites file name or `(none)` if no favorites file is open. |
| **Save as...** | | Save current favorites to a `.fav` or `.json` file, and switch to that file. |
| **Save copy as...** | | Save current favorites to a new `.fav` or `.json` file, but remain in the current favorites file. Use this to save a snapshot of the currrent favorites file. |
| **Clear** | | Empty the current Favorites file.  |

## System tab menu

Right click on the **System** tab to see the **System menu** for navigation and sorting System presets.

| Menu | Option | Description |
| -- | -- | -- |
| **Go to category** | **Strings (ST)</br>Winds (WI)</br>Vocal (VO)</br>Keyboard (KY)</br>Classic (CL)</br>Other (OT)</br>Percussion (PE)</br>Tuned (PT)</br>Processor (PR)</br>Drone (DO)</br>Midi (MD)</br>Control (CV)</br>Utility (UT)** | Moves to the **System** tab with **Sort by Category** and selects the first preset in the category, or the current preset if it is in that category. |
| **Sort System presets** | **Alphabetically** | (default) Sort system presets alphabetically by name. |
|  | **by Category** | Sort first by Category, then alphabetically within the category. This is the order that presets are shown on a Continuum. |
|  | **in System order** | Show system presets in internal System order. This is the numerical order in the Haken Editor "File 2" list, which is mostly but not entirely alphabetical. |

## Configuring startup parameters

On some commputers, operating systems, and particular EM devices, you may need to configure startup parameters in order to reliably and successfully initialize HC-1.

If you're starting up reliably, you may be able to speed up the startup process by allowing full MIDI transmission rate and shortening delays between phases.

This is a manual process of editing the `startup-config.json` file created in the user folder for the plugin.
The plugin user folder is the same folder where cached presets are stored, and it is the default folder for Favorites files.

To find the plugin's user folder:

1. Go to the Rack **Help** menu, and choose **Open user folder**.

   Your operating system's file browser will be opened showing the Rack User folder.

1. Open the `pachde-hc-one` folder.

Once you've found the plugin user folder, open the `startup-config.json` file in a plain-text editor.

The default `startup-config.json` looks like this:

```json
{
  "heartbeat_period": 4.0,
  "phase-device-output": {
    "delay": 4.0,
    "midi_rate": 0,
    "budget": 0
  },
  "phase-device-input": {
    "delay": 4.0,
    "midi_rate": 0,
    "budget": 0
  },
  "phase-device-hello": {
    "delay": 1.0,
    "midi_rate": 1,
    "budget": 2
  },
  "phase-device-config": {
    "delay": 1.0,
    "midi_rate": 1,
    "budget": 4
  },
  "phase-cached-presets": {
    "delay": 0.0,
    "midi_rate": 0,
    "budget": 0
  },
  "phase-user-presets": {
    "delay": 1.0,
    "midi_rate": 1,
    "budget": 12
  },
  "phase-system-presets": {
    "delay": 1.0,
    "midi_rate": 1,
    "budget": 25
  },
  "phase-favorites": {
    "delay": 0.0,
    "midi_rate": 0,
    "budget": 0
  },
  "phase-saved_preset": {
    "delay": 1.0,
    "midi_rate": 0,
    "budget": 4
  },
  "phase-preset-config": {
    "delay": 1.0,
    "midi_rate": 0,
    "budget": 4
  },
  "phase-request-updates": {
    "delay": 0.0,
    "midi_rate": 0,
    "budget": 0
  },
  "phase-heartbeat": {
    "delay": 2.0,
    "midi_rate": 0,
    "budget": 2
  },
  "phase-done": {
    "delay": 0.0,
    "midi_rate": 0,
    "budget": 0
  }
}
```

This configure the initialization process, and the midi transmission rate for the queries that HC-1 sends to the device.
In between these requests and when HC-One is fully initialized, the full transmission rate is used.

For each initialization phase, there are three options:

| Option | Value | Description |
| -- | -- | -- |
| `delay` | float | Seconds to pause after the phase before starting the next phase. |
| `midi_rate` | 0-2 | Sets the Midi transmission rate for the phase. **0** = full (fastest), **1** = 1/3 full rate, **2** = 1/20 full rate (meant for bluetooth MIDI). |
| `budget` | float | Seconds allowed for a phase to complete. If the phase is not complete in the alloted time it is retried with a lower MIDI transmission rate. If it fails after a maximum of three attempts, HC-1 is rebooted. |

The delays allow time for both the device and HC-1 to settle and be ready for the next phase.

## Technical Information

Input CV is assumed to be Unipolar 0-10v. When a knob is set to relative mode, then the voltage is assumed to be bipolar.
The voltage is converted to MIDI data to send to the device according to the range supported by the particular control.
The Macro section (**i** through **vi**) and Volume (EM Post level) support 14-bit MIDI, and the recirculator section is 7-bit MIDI (0-127).

## Notes

- **File usage:** After using this module, you can find a folder of data files that the module saves.
The folder _Rack user folder_ / `pachde-hc-one` contains `startup-config.json`, per-EM-device Favorites, User, and System files in JSON format.
It is also the default folder presented for saving and loading favorites files.
You can, of course, save favorites anywhere you like, but this is the default per VCV rack conventions.

  To find the Rack user folder, choose **Help** / **Open user folder** in VCV Rack.

- **Preset `.mid`:** At this time, this plugin has no support for managing preset .mid files.
I do have plans to build such functionality, but it may not be in the Rack plugin format.

- **Multiple EM Devices:** 

  If you own multiple EM-based devices, you can use an instance of HC-1 for each device in the same Rack patch to control multiple devices.
  The automatic discovery chooses a second (or third) device if there is already an HC-1 bound to the first device.
  You can choose another device from the **EM Device** button in the bottom left.

- **Factory presets:** For beta, there are no Rack module factory presets.
Please let me know if you have something useful that comes from using Rack module presets.
Module presets are important to use for HC-3 to save the list for use in other patches.

- **More functionality:** I plan to add expander modules for controlling most other things on the device.
I am *not* planning to create a preset (matrix) editor.
That's not really suitable for a VCV rack module and it would require additional proprietary internal information from Haken Audio.

  I am interested to hear from you what will be most important controls to include in future HC modules.

  - I have some ideas for other potential useful features, such as:

    - Save and load profiles. These would contain a profile of settings including pedal assignments, rounding, MIDI routing, and so on.
    These could be independent and also optionally associated with a specific preset (or presets), so that when you load a preset of any kind (user, favorite, or system), the associated profile is applied. This won't come until I've covered the basic controls.

    - User preset save, load, archive-all, and restore-all. This possibly in a separate program rather than VCV Rack modules.

![pachde (#d) logo](Logo.svg)
