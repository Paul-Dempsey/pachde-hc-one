# HC-One

[Repo](https://github.com/Paul-Dempsey/pachde-hc-one)

HC-One is a plugin with modules for working with Haken Audio's Eagan Matrix devices.
These include the Haken Continuum, Continuu Mini, and Eagan Matrix Module, and Expressive E's Osmose.

The beta release can be downloaded here: [HC-One 2.0.0-beta](https://github.com/Paul-Dempsey/pachde-hc-one/releases/edit/v2.0.0-beta).
If you're adventurous, drill down in Actions and grab a more recent build.

Pachde and HC-One are not affiliated with or sponsored by Haken Audio.

- [Documentation](doc/index.md)

- [Buy me a coffee](https://venmo.com/u/pcdempsey). Thank you!

  Donations go to keeping me caffeinated, or help fund getting hardware for testing and development. (I have a wish list :-).\
  ![Buy pachde a coffee with VenMo](doc/VenMo.png)

## Acknowledgements

Special thanks to Lippold Haken (and all of the Haken Audio team), for creating the Continuum.
Lippold has provided generous support by sharing technical details that make this software possible.

## Modules

- **[HC-1](HC-1.md)** Fast access to favorite presets, with knobs and CV for the essential controls: Macros and the Recirculator.

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

## Contributing

Contributions are generally welcome.
I'd appreciate if you're planning to add something, please open a Discussion so we can discuss it.

If you've found a bug, please open an issue.
If you have a fix for the bug, then that's fantastic. Bug fix PRs are always welcome.

At this time, pull requests are accepted only from forks. GitHub makes it super easy to create a fork.

![pachde (#d) logo](doc/Logo.svg)
