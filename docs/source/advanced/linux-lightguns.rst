Linux Lightguns
===============

Many lightguns (especially the Ultimarc AimTrak) may work better in MAME under
Linux when using a slightly more complicated configuration. The instructions
here are for getting an AimTrak working on Ubuntu using udev and Xorg, but other
Linux distributions and lightguns may work with some changes to the steps.

Configure udev rules
--------------------

For the AimTrak, each lightgun exposes several USB devices once connected: 2
mouse emulation devices, and 1 joystick emulation device. We need to instruct
libinput via udev to ignore all but the correct emulated mouse device. This
prevents each lightgun from producing multiple mouse devices, which would result
in non-deterministic selection between the "good" and "bad" emulated mouse
devices by Xorg.

Create a new file named ``/etc/udev/rules.d/65-aimtrak.rules`` and place the
following contents into it::

    # Set mode (0666) & disable libinput handling to avoid X11 picking up the wrong
    # interfaces/devices.
    SUBSYSTEMS=="usb", ATTRS{idVendor}=="d209", ATTRS{idProduct}=="160*",
       MODE="0666", ENV{ID_INPUT}="", ENV{LIBINPUT_IGNORE_DEVICE}="1"

    # For ID_USB_INTERFACE_NUM==2, re-enable libinput handling.
    SUBSYSTEMS=="usb", ATTRS{idVendor}=="d209", ATTRS{idProduct}=="160*",
        ENV{ID_USB_INTERFACE_NUM}=="02", ENV{ID_INPUT}="1",
        ENV{LIBINPUT_IGNORE_DEVICE}="0"

This configuration will be correct for the AimTrak lightguns, however each brand
of lightgun will require their own settings.

Configure Xorg inputs
---------------------

Next, we'll configure Xorg to treat the lightguns as a "Floating" device. This
is important for multiple lightguns to work correctly and ensures each gun's
emulated mouse pointer is NOT merged with the main system mouse pointer.

In ``/etc/X11/xorg.conf.d/60-aimtrak.conf`` we will need::

    Section "InputClass"
        Identifier "AimTrak Guns"
        MatchDevicePath "/dev/input/event*"
        MatchUSBID "d209:160*"
        Driver "libinput"
        Option "Floating" "yes"
        Option "AutoServerLayout" "no"
    EndSection

Configure MAME
--------------

Next, we'll need to configure MAME via ``mame.ini`` to use the new lightgun
device(s)::

    lightgun 1
    lightgun_device lightgun
    lightgunprovider x11

These first three lines tell MAME to enable lightgun support, to tell MAME that
we're using a lightgun instead of a mouse, and to use the x11 provider::

    lightgun_index1 "Ultimarc ATRAK Device #1"
    lightgun_index2 "Ultimarc ATRAK Device #2"
    lightgun_index3 "Ultimarc ATRAK Device #3"
    lightgun_index4 "Ultimarc ATRAK Device #4"

Lastly, as many lightgun games require aiming away from the screen and firing to
reload, you may reloading and we're using a device that represents that as a
separate button, you will need to configure the :ref:`off-screen reload helper
plugin <plugins-offscreenreload>` or :ref:`input macro plugin
<plugins-inputmacro>` to translate this to the axis and button inputs that these
games expect.
