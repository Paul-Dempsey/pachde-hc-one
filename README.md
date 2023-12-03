# HC One

HC One is a VCV Rack plugin with modules for working with any Eagan Matrix device.
These include the Haken Continuum, Continuu Mini, and Eagan Matrix Module from Haken Audio, and the Osmose from Expressive E.

If you are new to VCV Rack, make sure to get familiar with it.
See [Getting Started](https://vcvrack.com/manual/GettingStarted) in the VCV Rack manual.

[![Build Status](https://github.com/Paul-Dempsey/pachde-hc-one/actions/workflows/build-plugin.yml/badge.svg)](https://github.com/Paul-Dempsey/pachde-hc-one/actions/workflows/build-plugin.yml)

Grab a current build for the latest features and fixes: [HC-One Nightly](https://github.com/Paul-Dempsey/pachde-hc-one/releases/tag/Nightly)

The beta 5 release announced online can be downloaded here: [HC-One 2.0.0-beta5](https://github.com/Paul-Dempsey/pachde-hc-one/releases/tag/v2.0.0-beta5).

- [Documentation](doc/index.md)

- [Buy pachde a coffee](https://venmo.com/u/pcdempsey). Thank you!

  Donations help pachde's morale, and fund hardware for testing and developing this software.\
  ![Buy pachde a coffee with VenMo](doc/VenMo.png)

Pachde and HC One are not affiliated with or sponsored by Haken Audio (but we're friends).

## Modules

- **[HC-1](./doc/HC-1.md)** Fast access to favorite presets, with knobs and CV inputs for the essential controls: Macros and the Recirculator.

- **[Round](./doc/Round.md)** Control of rounding and the tuning scale, with CV and trigger.

- **[Compress](./doc/Compress.md)** EaganMatrix compressor controls, with CV.

- **[HC-2](./doc/HC-2.md)** TiltEQ, and CC maps (to be retired).

- **[HC-3](./doc/HC-3.md)** Pushbuttons to access up to 16 Favorite files.

- **[Pedal 1 & Pedal 2](./doc/Pedals.md)** Pedal control, virtual pedals, and pedal CV in/out.

## Installing

To install this plugin from the GitHub builds, you must first have VCV Rack, of course, so do that if you haven't yet.

You'll need to know the Operating System (OS), and Architecture (ARCH) of your Rack install, and where to copy the downloaded plugin package to.

- In Rack, choose **Help** / **Open user folder**.

- Inside this folder, you should see a folder named `plugins-OS-ARCH`, where "OS" is your operating system (`win`, `mac` or `lin`), and "ARCH" is the CPU architecture of your Rack (`arm64` for Mac M1 or M2, or `x64`). Keep the file browser open so you have the target folder ready. Make a note of the OS and ARCH.

- Downloading the *Release* and the *Nightly* (latest) are similar. I'll describe getting the Nightly here.

- Go to the [HC-One Nightly](https://github.com/Paul-Dempsey/pachde-hc-one/releases/tag/Nightly) release.

- Under **Assets** is a list of artifacts. Look for the `.vcvplugin` file for your OS and ARCH.

- Download and copy the `.vcvplugin` file to the plugins folder you opened previously.

- Restart Rack

At this point you should see the pachde HC One modules in your module browser.
Right click anywhere in Rack where there isn't a module to open the module browser.
Filter by Brand and check **pachde** to see just the pachde modules. This might include modules from my other VCV Rack plugins.
If you want to find other modules to use (including my other plugin **pachde One**), visit the VCV Rack library.

## Acknowledgements

Special thanks to Lippold Haken (and the Haken Audio team), for creating the Continuum.
Lippold personally has provided generous support by sharing technical details that make this software possible.

Beta testers are critical for success and covering the scenarios and hardware out there, and help me fill in the gaps on useful scenarios.

Special thanks to these early adopters:

- Richard Kram
- António Machado
- Justus Noll

Let me know how it's going for you, even if you haven't found any issues.

## Building

There's nothing special about building this plugin.
There are no external dependencies, and no submodules.
It's all basic stock VCV Rack plugin build per the Rack docs.

If you already have a Rack build environment set up:

1. Clone the repo if you just want to build.

   `git clone https://github.com/Paul-Dempsey/pachde-hc-one.git`

    Or, fork in GitHub if you want to make PRs.

1. cd to the plugin folder

1. run `make`

   Or `make plugins` from the Rack root if you have a full rack build.

No need to `make dep` or `git submodule update --init --recursive` for this module, because there are no dependencies other than Rack itself.

## Contributing code

Contributions (both code and donations) are welcome.
I'd appreciate if you're planning to add something, please open a Discussion  or contact me by email so we can discuss it.

If you've found a bug, please open an issue.
If you have a fix for the bug, then that's fantastic. Bug fix PRs are always welcome.

At this time, pull requests are accepted only from forks. GitHub makes it super easy to create a fork.

![pachde (#d) logo](doc/Logo.svg)
