.. _inputsystem:

Input System
============

.. contents::
   :local:
   :depth: 2


.. _inputsystem-intro:

Introduction
------------

The variety of systems MAME emulates, as well as the variation in host
systems and peripherals, necessitates a flexible, configurable input
system.

Note that the input system is concerned with low-level user input.
High-level user interaction, involving things like text input and
pointing devices, is handled separately.


.. _inputsystem-components:

Components
----------

From the emulated system’s point of view, the input system has the
following conceptual components.

Input device
~~~~~~~~~~~~

Input devices supply input values.  An input device typically
corresponds to a physical device in the host system, for example a
keyboard, mouse or game controller.  However, there isn’t always a
one-to-one correspondence between input devices and physical devices.
For example the SDL keyboard provider module aggregates all keyboards
into a single input device, and the Win32 lightgun provider module can
present two input devices using input from a single mouse.

Input devices are identified by their device class (keyboard, mouse,
joystick or lightgun) and device number within the class.  Input
provider modules can also supply an implementation-dependent identifier
to allow the user to configure stable device numbering.

Note that input devices are unrelated to emulated devices (``device_t``
implementations) despite the similar name.

Input device item
~~~~~~~~~~~~~~~~~

Also known as a **control**, and input device item corresponds to a
input source that produces a single value.  This usually corresponds to
a physical control or sensor, for example a joystick axis, a button or
an accelerometer.

MAME supports three kinds of controls: **switches**, **absolute axes**
and **relative axes**:

* Switches produce the value 0 when inactive (released or off) or 1 when
  active (pressed or on).
* Absolute axes produce a value normalised to the range -65,536 to
  65,536 with zero corresponding to the neutral position.
* Relative axes produce a value corresponding to the movement since the
  previous input update.  Mouse-like devices scale values to
  approximately 512 per nominal 100 DPI pixel.

Negative axis values should correspond to directions up, to the left,
away from the player, or anti-clockwise.  For single-ended axes (e.g.
pedals or displacement-sensitive triggers and buttons), only zero and
the negative portion of the range should be used.

Switches are used to represent controls that naturally have two distinct
states, like buttons and toggle switches.

Absolute axes are used to represent controls with a definite range
and/or neutral position.  Examples include steering wheels with limit
stops, joystick axes, and displacement-sensitive triggers.

Relative axes are used to represent controls with an effectively
infinite range.  Examples include mouse/trackball axes, incremental
encoder dials, and gyroscopes.

Accelerometers and force sensing joystick axes should be represented as
absolute axes, even though the range is theoretically open-ended.  In
practice, there is a limit to the range the transducers can report,
which is usually substantially larger than needed for normal operation.

Input device items are identified by their associated device’s class and
device number along with an **input item ID**.  MAME supplies item IDs
for common types of controls.  Additional controls or controls that do
not correspond to a common type are dynamically assigned item IDs.  MAME
supports hundreds to items per input device.

I/O port field
~~~~~~~~~~~~~~

An I/O port field represents an input source in an emulated device or
system.  Most types of I/O port fields can be assigned one or more
combinations of controls, allowing the user to control the input to
the emulated system.

Similarly to input device items, there are multiple types of I/O port
fields:

* **Digital fields** function as switches that produce one of two
  distinct values.  They are used for keyboard keys, eight-way joystick
  direction switches, toggle switches, photointerruptors and other
  emulated inputs that function as two-position switches.
* **Absolute analog fields** have a range with defined minimum, maximum
  and neutral positions.  They are used for analog joystick axes,
  displacement-sensitive pedals, paddle knobs, and other emulated inputs
  with a defined range.
* **Relative analog fields** have a range with defined minimum, maximum
  and starting positions.  On each update, the value accumulates and
  wraps when it passes either end of the range.  Functionally, this is
  like the output of an up/down counter connected to an incremental
  encoder.  They are used for mouse/trackball axes, steering wheels
  without limit stops, and other emulated inputs that have no range
  limits.
* DIP switch, configuration and adjuster fields allow the user to set
  the value through MAME’s user interface.
* Additional special field types are used to produce fixed or
  programmatically generated values.

A digital field appears to the user as a single assignable input, which
accepts switch values.

An analog field appears to the user as three assignable inputs: an
**axis input**, which accepts axis values; and an **increment input**
and a **decrement input** which accept switch values.

Input manager
~~~~~~~~~~~~~

The input manager has several responsibilities:

* Tracking the available input devices in the system.
* Reading input values.
* Converting between internal identifier values, configuration token
  strings and display strings.

In practice, emulated devices and systems rarely interact with the input
manager directly.  The most common reason to access the input manager is
implementing special debug controls, which should be disabled in release
builds.  Plugins that respond to input need to call the input manager to
read inputs.

I/O port manager
~~~~~~~~~~~~~~~~

The I/O port manager’s primary responsibilities include:

* Managing assignments of controls to I/O port fields and user interface
  actions.
* Reading input values via the input manager and updating I/O port field
  values.

Like the input manager, the I/O port manager is largely transparent to
emulated devices and systems.  You just need to set up your I/O ports
and fields, and the I/O port manager handles the rest.


.. _inputsystem-structures:

Structures and data types
-------------------------

The following data types are used for dealing with input.

Input code
~~~~~~~~~~

An input code specifies an input device item and how it should be
interpreted.  It is a tuple consisting of the following values: **device
class**, **device number**, **item class**, **item modifier** and **item
ID**:

* The device class, device number and item ID together identify the
  input device item to read.
* The item class specifies the type of output value desired: switch,
  absolute axis or relative axis.  Axis values can be converted to
  switch values by specifying an appropriate modifier.
* The modifier specifies how a value should be interpreted.  Valid
  options depend on the type of input device item and the specified
  item class.

If the specified input item is a switch, it can only be read using the
switch class, and no modifiers are supported.  Attempting to read a
switch as an absolute or relative axis always returns zero.

If the specified input item is an absolute axis, it can be read as an
absolute axis or as a switch:

* Reading an absolute axis item as an absolute axis returns the current
  state of the control, potentially transformed if a modifier is
  specified.  Supported modifiers are **reverse** to reverse the range
  of the control, **positive** to map the positive range of the control
  onto the output (zero corresponding to -65,536 and 65,536
  corresponding to 65,536), and **negative** to map the negative range
  of the control onto the output (zero corresponding to -65,536 and
  -65,536 corresponding to 65,536).
* Reading an absolute axis item as a switch returns zero or 1 depending
  on whether the control is past a threshold in the direction specified
  by the modifier.  Use the **negative** modifier to return 1 when the
  control is beyond the threshold in the negative direction (up or
  left), or the **positive** modifier to return 1 when the control is
  beyond the threshold in the positive direction (down or right).  There
  are two special pairs of modifiers, **left**/**right** and
  **up**/**down** that are only applicable to the primary X/Y axes of
  joystick devices.  The user can specify a *joystick map* to control
  how these modifiers interpret joystick movement.
* Attempting to read an absolute axis item as a relative axis always
  returns zero.

If the specified input item is a relative axis, it can be read as a
relative axis or as a switch:

* Reading a relative axis item as a relative axis returns the change in
  value since the last input update.  The only supported modifier is
  **reverse**, which negates the value, reversing the direction.
* Reading a relative axis as a switch returns 1 if the control moved in
  the direction specified by the modifier since the last input update.
  Use the **negative**/**left**/**up** modifiers to return 1 when the
  control has been moved in the negative direction (up or left), or the
  **positive**/**right**/**down** modifiers to return 1 when the control
  has moved in the positive direction (down or right).
* Attempting to read a relative axis item as an absolute axis always
  returns zero.

There are also special input codes used for specifying how multiple
controls are to be combined in an input sequence.

The most common place you’ll encounter input codes in device and system
driver code is when specifying initial assignments for I/O port fields
that don’t have default assignments supplied by the core.  The
``PORT_CODE`` macro is used for this purpose.

MAME provides macros and helper functions for producing commonly used
input codes, including standard keyboard keys and
mouse/joystick/lightgun axes and buttons.

Input sequence
~~~~~~~~~~~~~~

An input sequence specifies a combination controls that can be assigned
to an input.  The name refers to the fact that it is implemented as a
sequence container with input codes as elements.  It is somewhat
misleading, as input sequences are interpreted using instantaneous
control values.  Input sequences are interpreted differently for switch
and axis input.

Input sequences for switch input must only contain input codes with the
item class set to switch along with the special **or** and **not** input
codes.  The input sequence is interpreted using sum-of-products logic.
A **not** code causes the value returned by the immediately following
code to be inverted.  The conjunction of values returned by successive
codes is evaluated until an **or** code is encountered.  If the current
value is 1 when an **or** code is encountered it is returned, otherwise
evaluation continues.

Input sequences for axis input can contain input codes with the item
class set to switch, absolute axis or relative axis along with the
special **or** and **not** codes.  It’s helpful to think of the input
sequence as containing one or more groups of input codes separated by
**or** codes:

* A **not** code causes the value returned by an immediately following
  switch code to be inverted.  It has no effect on absolute or relative
  axis codes.
* Within a group, the conjunction of the values returned by switch codes
  is evaluated.  If it is zero, the group is ignored.
* Within a group, multiple axis values of the same type are summed.
  Values returned by absolute axis codes are summed, and values returned
  by relative axis codes are summed.
* If any absolute axis code in a group returns a non-zero value, the sum
  of relative axes in the group is ignored.  Any non-zero absolute axis
  value takes precedence over relative axis values.
* The same logic is applied when combining group values: group values
  produced from the same axis type are summed, and values produced from
  absolute axes take precedence over values produced from relative axes.
* After the group values are summed, if the value was produced from
  absolute axes it is clamped to the range -65,536 to 65,536 (values
  produced from relative axes are not clamped).

Emulation code rarely needs to deal with input sequences directly, as
they’re handled internally between the I/O port manager and input
manager.  The input manager also converts input sequences to and from
the token strings stored in configuration files and produces text for
displaying input sequences to users.

Plugins with controls or hotkeys need to use input sequences to allow
configuration.  Utility classes are provided to allow input sequences to
be entered by the user in a consistent way, and the input manager can be
used for conversions to and from configuration and display strings.  It
is very rare to need to directly manipulate input sequences.


.. _inputsystem-providermodules:

Input provider modules
----------------------

Input provider modules are part of the OS-dependent layer (OSD), and are
not directly exposed to emulation and user interface code.  Input
provider modules are responsible for detecting available host input
devices, setting up input devices for the input manager, and providing
callbacks to read the current state of input device items.  Input
provider modules may also provide additional default input assignments
suitable for host input devices that are present.

The user is given a choice of input modules to use.  One input provider
module is used for each of the four input device classes (keyboard,
mouse, joystick and lightgun).  The available modules depend on the host
operating system and OSD implementation.  Different modules may use
different APIs, support different kinds of devices, or present devices
in different ways.


.. _inputsystem-playerpositions:

Player positions
----------------

MAME uses a concept called *player positions* to help manage input
assignments.  The number of player positions supported depends on the
I/O port field type:

* Ten player positions are supported for common game inputs, including
  joystick, pedal, paddle, dial, trackball, lightgun and mouse.
* Four player positions are supported for mahjong and hanafuda inputs.
* One player position is supported for gambling system inputs.
* Other inputs do not use player positions.  This includes coin slots,
  arcade start buttons, tilt switches, service switches and
  keyboard/keypad keys.

The user can configure default input assignments per player position for
supported I/O port field types which are saved in the file
**default.cfg**.  These assignments are used for all systems unless the
device/system driver supplies its own default assignments, or the user
configures system-specific input assignments.

In order to facilitate development of reusable emulated devices with
inputs, particularly slot devices, the I/O port manager automatically
renumbers player positions when setting up the emulated system:

* The I/O port manager starts at player position 1 and begins
  iterating the emulated device tree in depth first order, starting from
  the root device.
* If a device has I/O port fields that support player positions, they
  are renumbered to start from the I/O port manager’s current player
  position.
* Before advancing to the next device, the I/O port manager sets its
  current player position to the last seen player position plus one.

For a simple example, consider what happens when you run a Sega Mega
Drive console with two game pads connected:

* The I/O port manager starts at player position 1 at the root device.
* The first device encountered with I/O port fields that support player
  positions is the first game pad.  The inputs are renumbered to start
  at player position 1.  This has no visible effect, as the I/O port
  fields are initially numbered starting at player position 1.
* Before moving to the next device, the I/O port manager sets its
  current player position to 2 (the last player position seen plus one).
* The next device encountered with I/O port fields that support player
  positions is the second game pad.  The inputs are renumbered to start
  at player position 2.  This avoids I/O port field type conflicts with
  the first game pad.
* Before moving to the next device, the I/O port manager sets its
  current player position to 3 (the last player position seen plus one).
* No more devices with I/O port fields that support player positions are
  encountered.


.. _inputsystem-updatingfields:

Updating I/O port fields
------------------------

The I/O port manager updates I/O port fields once for each video frame
produced by the first emulated screen in the system.  How a field is
updated depends on whether it is a digital or analog field.

Updating digital fields
~~~~~~~~~~~~~~~~~~~~~~~

Updating digital I/O port fields is simple:

* The I/O port manager reads the current value for the field’s assigned
  input sequence (via the input manager).
* If the value is zero, the field’s default value is set.
* If the value is non-zero, the binary complement of the field’s default
  value is set.

Updating absolute analog fields
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Updating absolute analog I/O port fields is more complex due to the need
to support a variety of control setups:

* The I/O port manager reads the current value for the field’s assigned
  axis input sequence (via the input manager).
* If the current value changed since the last update and the input
  device item that produced the current value was an absolute axis, the
  field’s value is set to the current value scaled to the correct range,
  and no further processing is performed.
* If the current value is non-zero and the input device item that
  produced the current value was a relative axis, the current value is
  added to the field’s value, scaled by the field’s sensitivity setting.
* The I/O port manager reads the current value for the field’s assigned
  increment input sequence (via the input manager); if this value is
  non-zero, the field’s increment/decrement speed setting value is added
  to its value, scaled by its sensitivity setting.
* The I/O port manager reads the current value for the field’s assigned
  decrement input sequence (via the input manager); if this value is
  non-zero, the field’s increment/decrement speed setting value is
  subtracted from its value, scaled by its sensitivity setting.
* If the current axis input, increment input and decrement input values
  are all zero, but either or both of the increment input and decrement
  input values were non-zero the last time the field’s value changed in
  response to user input, the field’s auto-centring speed setting value
  is added to or subtracted from its value to move it toward its default
  value.

Note that the sensitivity setting value for absolute analog fields
affects the response to relative axis input device items and
increment/decrement inputs, but it does not affect the response to
absolute axis input device items or the auto-centring speed.

Updating relative analog fields
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Relative analog I/O port fields also need special handling to cater for
multiple control setups, but they are a little simpler than absolute
analog fields:

* The I/O port manager reads the current value for the field’s assigned
  axis input sequence (via the input manager).
* If the current value is non-zero and the input device item that
  produced the current value was an absolute axis, the current value is
  added to the field’s value, scaled by the field’s sensitivity setting,
  and no further processing is performed.
* If the current value is non-zero and the input device item that
  produced the current value was a relative axis, the current value is
  added to the field’s value, scaled by the field’s sensitivity setting.
* The I/O port manager reads the current value for the field’s assigned
  increment input sequence (via the input manager); if this value is
  non-zero, the field’s increment/decrement speed setting value is added
  to its value, scaled by its sensitivity setting.
* The I/O port manager reads the current value for the field’s assigned
  decrement input sequence (via the input manager); if this value is
  non-zero, the field’s increment/decrement speed setting value is
  subtracted from its value, scaled by its sensitivity setting.

Note that the sensitivity setting value for relative analog fields
affects the response to all user input.
