Linux Lightguns
===============

Getting Lightguns on Linux working correctly can be a complicated process. There are several methods to achieve this, however the most commonly described method - allowing MAME to see the lightgun as a mouse device - results in a poor user experience, as the tracking between the hidden+accurate mouse pointer and "crosshair" is way off, with per-game configuration required to attempt to align these.

Another method is available which dramatically increases accuracy.

This document will describe how to configure the X11 Lightgun provider in MAME, and provide sample udev and Xorg.conf configurations. We will assume Ubuntu, and an Ultimarc Aimtrak lightgun. Changes will be necessary for other distros/lightgun brands, however the general patterns should be identical.

Configure udev rules
--------------------

For the AimTrack, each Lightgun exposes many USB devices once connected - 2 mouse like devices, and 1 joystick device. We would like to instruct libinput, via udev, to ignore all but the correct mouse device. This prevents each lighgun from producing multiple mouse devices, which would result in non-determinsitic seclection between the "good" and "bad" mouse device by Xorg.

Create a new file named `/etc/udev/rules.d/65-aimtrak.rules` and place the following contents into it:

|        # Set mode (0666) & disable libinput handling to avoid X11 picking up the wrong
|        # interfaces/devices.
|        SUBSYSTEMS=="usb", ATTRS{idVendor}=="d209", ATTRS{idProduct}=="160*",
|           MODE="0666", ENV{ID_INPUT}="", ENV{LIBINPUT_IGNORE_DEVICE}="1"
|
|        # For ID_USB_INTERFACE_NUM==2, re-enable libinput handling.
|        SUBSYSTEMS=="usb", ATTRS{idVendor}=="d209", ATTRS{idProduct}=="160*",
|            ENV{ID_USB_INTERFACE_NUM}=="02", ENV{ID_INPUT}="1",
|            ENV{LIBINPUT_IGNORE_DEVICE}="0"

This configuration will be correct for the AimTrak lightguns, however each brand of lightgun will require their own settings.

Configure Xorg inputs
---------------------

Next, we'll configure Xorg to treat the Lightguns as a "Floating" device. This is important for multiple lightguns to work correctly, and ensures each gun's "mouse pointer" is NOT merged with the main system mouse pointer.

/etc/X11/xorg.conf.d/60-aimtrak.conf:

| Section "InputClass"
| 	Identifier "AimTrak Guns"
| 	MatchDevicePath "/dev/input/event*"
| 	MatchUSBID "d209:160*"
| 	Driver "libinput"
| 	Option "Floating" "yes"
| 	Option "AutoServerLayout" "no"
| EndSection

Configure MAME
--------------

Next, we'll configure MAME to use the X11 `lightgun_provider`, as well as all the other related settings:

| lightgun                  1                          # Enable Lightgun support
| lightgun_device           lightgun                   # Use the lighgun subsystem for lightguns (instead of Keyboard/Mouse)
| lightgunprovider          x11                        # Use the x11 lightgun driver
| lightgun_index1           "Ultimarc ATRAK Device #1" # Name our devices for consistent detection
| lightgun_index2           "Ultimarc ATRAK Device #2"
| lightgun_index3           "Ultimarc ATRAK Device #3"
| lightgun_index4           "Ultimarc ATRAK Device #4"
| offscreen_reload          1                          # Enable offscreen reload, required for most games.
