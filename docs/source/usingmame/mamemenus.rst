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
    Shows the :ref:`Audio Mixer <menus-audiomixer>` menu, where you
    decide how to connect your system audio inputs and outputs to the
    emulated system's microphones and speakers.
Audio Effects
    Shows the :ref:`Audio Effects <menus-audioeffects>` menu, which
    allows to configure the audio effects applied between the emulated
    system's speakers and the actual system audio outputs.
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

The Audio Mixer menu allows to establish connections between emulated
speakers and microphones, and system audio inputs and outputs.  It
uses the standard up/down arrows to select a device and/or current
mapping, left/right arrows to change a value (system audio port,
level, channel...) and [ ] to change column.  In addition the (by
default) F key adds a full mapping, C a channel mapping, and Delete
clears a mapping.

A full mapping sends all channels of a speaker to the appropriate(s)
channel(s) of the system output, and similarly retrieves all channels
of a microphone from the appropriate(s) input(s) of a system input.
For instance a mono speaker will send audio to both channels of a
stereo system output.

A channel mapping maps between one channel of speaker or a microphone
and one channel of a system input or output.  It can be a little
tedious, but it allows for instance to take two mono speakers and turn
it into the left and right channels of a system output, whcih is
useful for some cabinets.

Every mapping has a configurable volume associated.

The mapping configuration is saved in the system cfg file.

Some OSes propose an external interface to change mappings and volumes
dynamically, for instance pipewire on linux.  Mame does its best to
follow that and keep the information in the cfg file for future runs.


.. _menus-audioeffects:

Audio Effects menu
------------------

This menu allows to configure the audio effects that are applied to
the speaker outputs between the speaker device and the audio mixer.
In other words, the output channels as seen in the audio mixer are the
outputs of the effect chains.  Each speaker has an independant effect
chain applied.

The chain itself is not configurable it is always in order:

* Filter
* Compressor
* Reverb
* EQ

The parameters of each are fully configurable though.  A configured
parameter shows as white, a default as grey, and Clear allows to go
back to the default value.  The default parameters for the chain of a
given speaker are the parameters of the Default chain, and the default
parameters of the Default chain are fixed.  The default chain allows
to create a global setup that one likes and have it applied everywhere
by default.

Filter effect
~~~~~~~~~~~~~

This effect proposes an order-2 high-pass and order-2 low-pass filter.
The high-pass filter allows to remove the DC offset some emulated
hardware has which can create saturation when not needed.  The
low-pass filter (defaulting to off) allows to reproduce how muffled
the sound of a number of cabinets and TVs were.

The Q factor defines how sharp the transition is, the higher the
sharper.  Over 0.7 the filter starts amplifying the frequencies arount
the cutoff though, which can be surprising.


Compression effect
~~~~~~~~~~~~~~~~~~

Not implemented yet.


Reverb effect
~~~~~~~~~~~~~

Not implemented yet.


EQ effect
~~~~~~~~~

The 5-band parametric equalizer allows to amplify or reduce certains
bands of frequency in the spectrum.  The three middle filters, and
also the extreme ones if configured as "Peak", change frequencies
around the cutoff.  The Q factor selects the sharpness of the peak,
the higher the sharper.  The extreme filters in "Shelf" mode move all
the frequencies under (or over) the cutoff frequency.

