.. _menus:

MAME Menus
==========

.. contents:: :local:


.. _menus-intro:

Introduction
------------

To show the :ref:`main menu <menus-main>` while running an emulated system in
MAME, press the **Show/Hide Menu** key or button (**Tab** by default).  If the
emulated system has keyboard inputs, you may need to press the
**Toggle UI Controls** key or button (**Scroll Lock**, or **Forward Delete** on
macOS, by default) to enable user interface controls first.  You can dismiss a
menu by pressing the **UI Back** key or button (**Escape** by default).
Dismissing a menu will return to its parent menu, or to the running system in
the case of the main menu.

You can hide a menu and return to the running system by pressing the
**Show/Hide Menu** key or button.  Pressing the **Show/Hide Menu** key or button
again will jump back to the same menu.  This is useful when testing changes to
settings.

Emulated system inputs are ignored while menus are displayed.  You can still
pause or resume the running system while most menus are displayed by pressing
the **Pause** key or button (**F5** on the keyboard by default).

If you start MAME without specifying a system on the command line, the system
selection menu will be shown (assuming the
:ref:`ui option <mame-commandline-ui>` is set to **cabinet**).  The system
selection menu is also shown if you select **Select New System** from the main
menu during emulation.

For more information on navigating menus, :ref:`see the relevant section
<ui-menus>`.


.. _menus-main:

Main menu
---------

The main menu is shown when you press the **Show/Hide Menu** key or button while
running an emulated system or while the system information screen is displayed.
It provides access to menus used to change settings, control various features,
and show information about the running system and MAME itself.

If you press the **Show/Hide Menu** key or button to show the main menu while
the system information screen is displayed, the emulated system will not start
until the main menu is dismissed (either by selecting **Start System**, pressing
the **UI Back** key or button, or pressing the **Show/Hide Menu** key or
button).  This can be useful for mounting media images or changing DIP switches
and machine configuration settings before the emulated system starts.

Input Settings
    Shows the :ref:`Input Settings <menus-inputopts>` menu, where you can assign
    controls to emulated inputs, adjust analog control settings, control toggle
    inputs, and test input devices.
DIP Switches
    Shows the DIP Switches menu, where configuration switches for the running
    system can be changed.  This item is not shown if the running system has no
    DIP switches.
Machine Configuration
    Shows the Machine Configuration menu, where various settings specific to the
    emulated system can be changed.  This item is not shown if the running
    system has no configuration settings.
Bookkeeping
    Shows uptime, coin counter and ticket dispenser statistics (if relevant) for
    the running system.
System Information
    Shows information about the running system as emulated in MAME, including
    CPU, sound and video devices.
Warning Information
    Shows information about imperfectly emulated features of the running system.
    This item is not shown if there are no relevant warnings.
Media Image Information
    Shows information about mounted media images (if any).  This item is only
    shown if the running system has one or more media devices (e.g. floppy disk
    drives or memory card slots).
File Manager
    Shows the File Manager menu, where you can mount new or existing media image
    files, or unmount currently mounted media images.  This item is only shown
    if the running system has one or more media devices (e.g. floppy disk
    drives or memory card slots).
Tape Control
    Shows the Tape Control menu, where you can control emulated cassette tape
    mechanisms.  This item is only shown for systems that use cassette tape
    media.
Pseudo Terminals
    Shows the status of any pseudo terminal devices in the running system (used
    to connect the emulated system to host pseudo terminals, for example via
    emulated serial ports).  This item is not shown if there are no pseudo
    terminal devices in the running system.
BIOS Selection
    Shows the BIOS Selection menu, where you can select the BIOS/boot
    ROM/firmware for the system and slot cards it contains.  This item is not
    shown if no BIOS options are available.
Slot Devices
    Shows the Slot Devices menu, where you can choose between emulated
    peripherals.  This item is not shown for systems that have no slot devices.
Barcode Reader
    Shows the Barcode Reader menu, where you can simulate scanning barcodes with
    emulated barcode readers.  This item is not shown if there are no barcode
    readers in the running system.
Network Devices
    Shows the Network Devices menu, where you can set up emulated network
    adapters that support bridging to a host network.  This item is not shown if
    there are no network adaptors that support bridging in the running system.
Audio Mixer
    Shows the :ref:`Audio Mixer <menus-audiomixer>` menu, where you configure
    how MAME routes sound from the emulated system to host system sound outputs,
    and from host system sound inputs to the emulated system.
Audio Effects
    Shows the :ref:`Audio Effects <menus-audioeffects>` menu, where you can
    configure audio effects applied to emulated sound output.
Slider Controls
    Shows the Slider Controls menu, where you can adjust various settings,
    including video adjustments and individual sound channel levels.
Video Options
    Shows the Video Options menu, where you can change the view for each
    screen/window, as well as for screenshots.
Crosshair Options
    Shows the Crosshair Options menu, where you can adjust the appearance of
    crosshairs used to show the location of emulated light guns and other
    absolute pointer inputs.  This item is not shown if the emulated system has
    no absolute pointer inputs.
Cheat
    Shows the Cheat menu, for controlling the built-in cheat engine.  This item
    is only shown if the built-in chat engine is enabled.  Note that the cheat
    plugin’s menu is accessed via the Plugin Options menu.
Plugin Options
    Shows the Plugin Options menu, where you can access settings for enabled
    plugins.  This item is not shown if no plugins are enabled, or if the main
    menu is shown before the emulated system starts (by pressing the Show/Hide
    Menu key/button while the system information screen is displayed).
External DAT View
    Shows the info viewer, which displays information loaded from various
    external support files.  This item is not shown if the :ref:`data plugin
    <plugins-data>` is not enabled, or if the main menu is shown before the
    emulated system starts (by pressing the Show/Hide Menu key/button while the
    system information screen is displayed).
Add To Favorites/Remove From Favorites
    Adds the running system to the favourites list, or removes it if it’s
    already in the favourites list.  The favourites list can be used as a
    filter for the system selection menu.
About MAME
    Shows the emulator version, data model, and copyright license information.
Select New System
    Shows the system selection menu, where you can select a system to start a
    new emulation session.  This item is not shown if the main menu is shown
    before the emulated system starts (by pressing the Show/Hide Menu key/button
    while the system information screen is displayed).
Close Menu/Start System
    Closes the main menu, returning control of the running system.  Shows
    **Start System** if the main menu is shown before the emulated system
    starts (by pressing the Show/Hide Menu key/button while the system
    information screen is displayed).


.. _menus-inputopts:

Input Settings menu
-------------------

The Input Settings provides options for assigning controls to emulated inputs,
adjusting analog control settings, controlling toggle inputs, and testing input
devices.  You can reach the Input Settings menu by selecting **Input Settings**
from the :ref:`main menu <menus-main>`.  The items shown on this menu depend on
available emulated inputs for the running system.  Available emulated inputs may
depend on slot options, machine configuration settings and DIP switch settings.

Input Assignments (this system)
    Lets you select assign controls to emulated inputs for the running system.
    See the section on :ref:`configuring inputs <ui-inptcfg>` for more details.
    This item is not shown if the running system has no enabled inputs that can
    be assigned controls.
Analog Input Adjustments
    Shows the Analog Input Adjustments menu, where you can adjust sensitivity,
    auto-centring speed and inversion settings for emulated analog inputs, and
    see how the emulated analog inputs respond to controls with your settings.
    For more details, see the :ref:`analog input settings <ui-inptcfg-analog>`
    section for more details.  This item is not shown if the running system has
    no enabled analog inputs.
Keyboard Selection
    Shows the :ref:`Keyboard Selection menu <menus-keyboard>`, where you can
    select between emulated and natural keyboard modes, and enable and disable
    keyboard and keypad inputs for individual emulated devices.  This item is
    not shown if the running system has no keyboard or keypad inputs.
Toggle Inputs
    Shows the :ref:`Toggle Inputs menu <menus-inputtoggle>`, where you can view
    and adjust the state of multi-position or toggle inputs.  This item is not
    shown if the running system has no enabled toggle inputs.
Input Assignments (general)
    Lets you select assign user interface controls, or assign default controls
    for all emulated systems.  See the section on :ref:`configuring inputs
    <ui-inptcfg>` for more details.
Input Devices
    Shows the :ref:`Input Devices menu <menus-inputdevices>`, which lists the
    input devices recognised by MAME.


.. _menus-inputtoggle:

Toggle Inputs menu
------------------

The Toggle Inputs menu shows the current state of multi-position or toggle
inputs.  Common examples include mechanically locking Caps Lock keys on
computers, and two-position gear shit levers on driving games.  You can reach
the Toggle Inputs menu by selecting **Toggle Inputs** from the :ref:`Input
Settings menu <menus-inputopts>`.  Note that available emulated inputs may
depend on slot options, machine configuration settings and DIP switch settings.

Inputs are grouped by the emulated device they belong to.  You can move between
devices using the **Next Group** and **Previous Group** keys or buttons.  Names
of inputs are shown on the left, and the current settings are shown on the
right.

To change the state of an input, highlight it and use the **UI Left** and **UI
Right** keys or buttons, or click the arrows beside the current setting.


.. _menus-keyboard:

Keyboard Selection menu
-----------------------

The Keyboard Selection menu lets your switch between emulated and natural
keyboard modes, and enable or disable keyboard inputs for individual emulated
devices.  You can reach the Keyboard Selection menu by selecting **Keyboard
Selection** from the :ref:`Input Settings menu <menus-inputopts>`.

In emulated keyboard mode, keyboard and keypad inputs behave like any other
digital inputs, responding to their assigned controls.  In natural keyboard
mode, MAME attempts to translate typed characters to emulated keystrokes.  The
initial keyboard mode is set using the :ref:`natural option
<mame-commandline-natural>`.

There are a number of unavoidable limitations in natural keyboard mode:

* The emulated system must to support it.
* The selected keyboard *must* match the keyboard layout selected in the
  emulated software.
* Keystrokes that don’t produce characters can’t be translated. (e.g. pressing a
  modifier key on its own, such as **Shift** or **Control**).
* Holding a key until the character repeats will cause the emulated key to be
  pressed repeatedly as opposed to being held down.
* Dead key sequences are cumbersome to use at best.
* Complex input methods will not work at all (e.g. for Chinese/Japanese/Korean).

Each emulated device in the system that has keyboard and/or keypad inputs is
listed on the menu, allowing keyboard/keypad inputs to be enabled or disabled
for individual devices.  By default, keyboard/keypad inputs are enabled for the
first device with keyboard inputs (if any), and for all other devices that have
keypad inputs but no keyboard inputs.  The enabled keyboard/keypad inputs are
automatically saved to the configuration file for the system when the emulation
session ends.


.. _menus-inputdevices:

Input Devices menu
------------------

The Input Devices menu lists input devices recognised by MAME and enabled with
your current settings.  Recognised input devices depend on the
:ref:`keyboardprovider <mame-commandline-keyboardprovider>`, :ref:`mouseprovider
<mame-commandline-mouseprovider>`, :ref:`lightgunprovider
<mame-commandline-lightgunprovider>` and :ref:`joystickprovider
<mame-commandline-joystickprovider>` options.  Classes of input devices can be
enabled or disabled using the :ref:`mouse <mame-commandline-nomouse>`,
:ref:`lightgun <mame-commandline-nolightgun>` and :ref:`joystick
<mame-commandline-nojoystick>` options.  You can reach the Input Devices menu by
selecting **Input Devices** from the :ref:`Input Settings menu
<menus-inputopts>` or the General Settings menu.

Input devices are grouped by device class (for example keyboards or light guns).
You can move between device classes using the **Next Group** and **Previous
Group** keys or buttons.  For each device, the device number (within its class)
is shown on the left, and the name is shown on the right.

Select a device to show the supported controls for the device.  The name of
each control is displayed on the left and its current state is shown on the
right.  When an analog axis control is highlighted, its state is also shown in
graphical form below the menu.  Digital control states are either zero
(inactive) or one (active).  Analog axis input states range from -65,536 to
65,536 with the neutral position at zero.  You can also select **Copy Device
ID** to copy the device’s ID to the clipboard.  This is useful for setting up
:ref:`stable controller IDs <devicemap>` in :ref:`controller configuration files
<ctrlrcfg>`.


.. _menus-audiomixer:

Audio Mixer menu
----------------

The Audio Mixer menu allows you to configure how MAME routes sound from emulated
speakers to system sound outputs, and from system sound inputs to emulated
microphones.  There are two kinds of routes: *full routes*, and *channel
routes*:

* A full output route sends sound from all channels of an emulated sound output
  device to a host sound output.  MAME automatically decides how to assign
  emulated channels (typically speakers) to output channels, based on speaker
  position information.
* Similarly, a full input route sends sound from a host sound input to all
  channels of an emulated sound input device.  MAME automatically decides how
  to assign input channels to emulated channels (typically microphones), based
  on microphone position information.
* A channel route sends sound from a single emulated sound output channel to a
  single host sound output channel, or from a single host sound input channel to
  a single emulated sound input channel.

Only one full route is allowed for each combination of an emulated sound output
or input device and host sound output or input.  Only one channel route is
allowed between an individual emulated channel and an individual host sound
channel.

Routes are grouped by emulated device.  For each device, full routes are listed
before channel routes.  For each route, you can select the system sound output
or input and adjust the volume from -96 dB (quietest) to +12 dB (loudest).  For
channel routes, you can also select the individual emulated channel and host
channel.  Select **Remove this route** to remove a route.

Select **Add new full route** to add a new full route to that group.  If
possible, it will be added and the menu highlight will move to the newly added
route.  If routes between the highlighted device and every host output/input
already exist, no route will be added.

Select **Add new channel route** to add a new channel route to that group.  If
possible, it will be added and the menu highlight will move to the newly added
route.  If routes between all channels for the highlighted device and every
host output/input channel already exist, no route will be added.

Some sound modules allow channel assignments and volumes to be controlled using
an external mixer interface (for example the PipeWire module for Linux has this
capability).  In these cases, MAME does its best to follow the changes you make
in the external mixer interface and save changes in its configuration.

The audio routes are saved in the system configuration file.


.. _menus-audioeffects:

Audio Effects menu
------------------

The Audio Effects menu allows you to configure audio effects that are applied
to emulated sound output before it’s routed to host sound outputs.  An
independent effect chain is applied for each emulated sound output device.

The effect chain itself is not configurable.  It always consists of these four
effects, in this order:

* Filters
* Compressor
* Reverb
* Equalizer

When editing parameters for an output device’s effect chain, inherited default
parameter values are showing dimmed, while parameter values set for that chain
are shown in the normal text colour.  Press the UI Clear key (Del/Delete/Forward
Delete on the keyboard by default) to reset a parameter to use the inherited
default value.

Edit the **Default** chain to set default parameter value that can be inherited
by output device chains.  When editing the **Default** chain, you can restore
the built-in default value for a parameter by pressing the UI Clear key
(Del/Delete/Forward Delete on the keyboard by default).

By default, the high-pass filter is enabled, with minimal cutoff frequency for
DC offset removal.  All other effects are bypassed (technically, the equalizer
effect is active too, but all bands are set to 0 dB so it's still turned off).

The Audio Effects menu also allows you to configure the algorithm used for audio
sample rate conversion.  The default **LoFi** algorithm has modest CPU
requirements.  The recommended **HQ** algorithm provides higher quality sample
rate conversion at the expense of requiring substantially higher CPU
performance.

The **HQ** algorithm has additional parameters.  Increasing the **HQ latency**
can improve quality.  If it's increased too much and multiple sound chips are
used, the latencies will stack up and you will end up with too much lag at the
end.  When decreasing the latency below 1 ms, the resampler will lose its
potential (in fact, it will sound similar to MAME's lower quality resampler from
before version 0.278).  Increasing the **HQ filter max size** or **HQ filter max
phases** can improve quality at the expense of higher CPU performance
requirements.


Filter effect
~~~~~~~~~~~~~

This effect implements a second-order high-pass filter and a second-order
low-pass filter.  The high-pass filter allows DC offsets to be removed.  The
low-pass filter can simulate the poor high-frequency response typical of many
arcade cabinets and television sets.

The Q factor controls how sharp the transition from the stop band to the
passband is.  Higher factors provide a sharper transition.  Values over 0.71
cause the filter to amplify frequencies close to the cutoff frequency, which
may be surprising or undesirable.


Compressor effect
~~~~~~~~~~~~~~~~~

This effect provides dynamic range compression (it is based on a
reimplementation of Alain Paul’s Versatile Compressor).  Dynamic range
compression reduces the difference in volume between the softest and loudest
sounds.  It’s useful in a variety of situations, for example it can help make
quiet sounds more audible over background noise.

The parameters are:

Threshold
    The level at which the amplification fully stops.
Ratio
    The maximum amplification.
Attack
    The reaction time to loud sounds to reduce the amplification.
Release
    The reaction time to allow the amplification to go back up.
Input gain
    The amplification level at the input.
Output gain
    The amplification level at the output.
Convexity
    The shape of the relationship between distance to the threshold and ratio
    value.  Higher values give a steeper shape.
Channel link
    At 100%, all channels of an output device are amplified identically, while
    at 0% they are fully independent.  Intermediate values give intermediate
    behaviour.
Feedback
    Allows some of the output to be fed back to the input.
Inertia
    Higher values make the ratio change more slowly.
Inertia decay
    Tweaks the impact of the Inertia.
Ceiling
    The maximum level allowed just before the output amplification.  Causes
    soft clipping at that level.

By setting **Attack** to 0 ms, **Release** to Infinite, and **Ratio** to
Infinity:1, the compressor will turn into a brickwall limiter (leave the
advanced settings to default).  If you increase **Input gain** on top of that,
with a **Threshold** of eg. -3 dB, it will act like a dynamic normalizer.


Reverb effect
~~~~~~~~~~~~~

Not documented yet.


Equalizer effect
~~~~~~~~~~~~~~~~

A five-band parametric equalizer, allowing to amplify or attenuate specific
frequency bands.

The three middle filters are bandpass/bandreject filters, meaning they amplify
or attenuate frequencies around the configured centre frequency.  The first
and last filters can also be configured as bandpass/bandreject filter by setting
the mode to **Peak**.  Setting the mode to **Shelf** causes the filter to
amplify or attenuate all frequencies below (for the first filter) or above (for
the last filter) the configured cutoff frequency.

The Q factor controls the sharpness of the peak or trough in frequency response
for bandpass/bandreject filters (the Q factor is not adjustable for **Shelf**
mode).  Higher Q factors give a sharper shape, affecting a narrower range of
frequencies.
