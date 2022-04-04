# openfdd - Fancy Devices Drivers for Linux

This is a personal project to learn more about USB and protocol reversing.

openfdd is a collection of open source userspace drivers for Fancy Devices™,
like gaming mice and keyboards with macros, RGB lighting, and lots of 
features that are usually configured from the manufacturer's proprietary
software.

# Building and running

## Dependecies

This project uses *libusb-1.0*. Make sure to install it from your distro's
repositories.

nlohmann's json library is also used, but it's bundled in the repo, so no
need to download anything extra.

## Building

Once you cloned the repo, you can build a binary with:

```console
 $ ./build.sh
```

Note that this make a dev build, with debugging symbols and sanitizers
enabled, this isn't great for day-to-day use :/  
... But this project isn't yet ready for day-to-day use anyways!

## Runing

Now run it with:

```console
 $ sudo ./openfdd
```

This will list all the connected devices that have a driver, and the actions
you can run on them.

The driver id is next to the name of the device, all the entries bellow
are actions, starting with their name. To run an action, use the following
syntax:

```console
 $ sudo ./openfdd [driver_id] [action_name] [optional_parameters]
```

# Supported devices

* SteelSeries Apex 100
  * Missing button remapping (though it may not be a driver feature :/)
* SteelSeries Rival 3 Wireless
  * Missing RGB configurations
  * Missing button remapping

And others coming! There isn't much stuff yet here, because I don't have
many Fancy Devices™... I do have a SteelSeries Rival 100 & 110, and just
bought a Aerox 3 Wireless, so I will be making drivers for those too,
but that's about it for now.  
If you have Fancy Devices™ of your own, I will be making a guide soon
on how to write a driver for it. My goal is to lay the groundwork to make
it as easy as possible for new devices to be ported!

# Want to help?

PRs are welcome! You can try porting your device if it doesn't work already,
but there are other things to do!

Overall, there is a lack of consistency in naming/strings, fixing those
shouldn't be too hard, but time consuming, and I'm focusing on the underlying
functionality for now.

You can also look for TODOs using:

```console
 $ grep "TODO" -R .
```

# Long term goals

* [ ] Configuration saving and loading
  * [ ] Multiple configurations for each device
* [ ] Make the driver a daemon that can be interacted with using a socket
  * [ ] Nicer (GUI?) front end
* [ ] Easy to use API for writing drivers, and interacting with them
* [ ] More device support

