.. _luascript-ref-input:

Lua Input System Classes
========================

Allows scripts to get input from the user, and access I/O ports in the emulated
system.

.. contents::
    :local:
    :depth: 1


.. _luascript-ref-ioportman:

I/O port manager
----------------

Wraps MAME’s ``ioport_manager`` class, which provides access to emulated I/O
ports and handles input configuration.

Instantiation
~~~~~~~~~~~~~

manager.machine.ioport
    Gets the global I/O port manager instance for the emulated machine.

Methods
~~~~~~~

ioport:count_players()
    Returns the number of player controllers in the system.
ioport:type_pressed(type, [player])
    Returns a Boolean indicating whether the specified input is currently
    pressed.  The input type may be an enumerated value or an
    :ref:`input type <luascript-ref-inputtype>` entry.  If the input type is an
    enumerated value, the player number may be supplied as a zero-based index;
    if the player number is not supplied, it is assumed to be zero.  If the
    input type is an input type entry, the player number may not be supplied
    separately.
ioport:type_name(type, [player])
    Returns the display name for the specified input type and player number.
    The input type is an enumerated value.  The player number is a zero-based
    index.  If the player number is not supplied, it is assumed to be zero.
ioport:type_group(type, player)
    Returns the input group for the specified input type and player number.  The
    input type is an enumerated value.  The player number is a zero-based index.
    Returns an integer giving the grouping for the input.  If the player number
    is not supplied, it is assumed to be zero.

    This should be called with values obtained from I/O port fields to provide
    canonical grouping in an input configuration UI.
ioport:type_seq(type, [player], [seqtype])
    Get the configured :ref:`input sequence <luascript-ref-inputseq>` for the
    specified input type, player number and sequence type.  The input type may
    be an enumerated value or an
    :ref:`input type <luascript-ref-inputtype>` entry.  If the input type is an
    enumerated value, the player number may be supplied as a zero-based index;
    if the player number is not supplied, it is assumed to be zero.  If the
    input type is an input type entry, the player number may not be supplied
    separately.  If the sequence type is supplied, it must be ``"standard"``,
    ``"increment"`` or ``"decrement"``; if it is not supplied, it is assumed to
    be ``"standard"``.

    This provides access to general input assignments.
ioport:set_type_seq(type, [player], seqtype, seq)
    Set the configured :ref:`input sequence <luascript-ref-inputseq>` for the
    specified input type, player number and sequence type.  The input type may
    be an enumerated value or an
    :ref:`input type <luascript-ref-inputtype>` entry.  If the input type is an
    enumerated value, the player number must be supplied as a zero-based index.
    If the input type is an input type entry, the player number may not be
    supplied separately.  The sequence type must be ``"standard"``,
    ``"increment"`` or ``"decrement"``.

    This allows general input assignments to be set.
ioport:token_to_input_type(string)
    Returns the input type and player number for the specified input type token
    string.
ioport:input_type_to_token(type, [player])
    Returns the token string for the specified input type and player number.  If
    the player number is not supplied, it assumed to be zero.

Properties
~~~~~~~~~~

ioport.types[] (read-only)
    Gets the supported :ref:`input types <luascript-ref-inputtype>`.  Keys are
    arbitrary indices.  All supported operations have O(1) complexity.
ioport.ports[]
    Gets the emulated :ref:`I/O ports <luascript-ref-ioport>` in the system.
    Keys are absolute tags.  The ``at`` and ``index_of`` methods have O(n)
    complexity; all other supported operations have O(1) complexity.


.. _luascript-ref-natkbdman:

Natural keyboard manager
------------------------

Wraps MAME’s ``natural_keyboard`` class, which manages emulated keyboard and
keypad inputs.

Instantiation
~~~~~~~~~~~~~

manager.machine.natkeyboard
    Gets the global natural keyboard manager instance for the emulated machine.

Methods
~~~~~~~

natkeyboard:post(text)
    Post literal text to the emulated machine.  The machine must have keyboard
    inputs with character bindings, and the correct keyboard input device must
    be enabled.
natkeyboard:post_coded(text)
    Post text to the emulated machine.  Brace-enclosed codes are interpreted in
    the text.  The machine must have keyboard inputs with character bindings,
    and the correct keyboard input device must be enabled.

    The recognised codes are ``{BACKSPACE}``, ``{BS}``, ``{BKSP}``, ``{DEL}``,
    ``{DELETE}``, ``{END}``, ``{ENTER}``, ``{ESC}``, ``{HOME}``, ``{INS}``,
    ``{INSERT}``, ``{PGDN}``, ``{PGUP}``, ``{SPACE}``, ``{TAB}``, ``{F1}``,
    ``{F2}``, ``{F3}``, ``{F4}``, ``{F5}``, ``{F6}``, ``{F7}``, ``{F8}``,
    ``{F9}``, ``{F10}``, ``{F11}``, ``{F12}``, and ``{QUOTE}``.
natkeyboard:paste()
    Post the contents of the host clipboard to the emulated machine.  The
    machine must have keyboard inputs with character bindings, and the correct
    keyboard input device must be enabled.
natkeyboard:dump()
    Returns a string with a human-readable description of the keyboard and
    keypad input devices in the system, whether they are enabled, and their
    character bindings.

Properties
~~~~~~~~~~

natkeyboard.empty (read-only)
    A Boolean indicating whether the natural keyboard manager’s input buffer is
    empty.
natkeyboard.full (read-only)
    A Boolean indicating whether the natural keyboard manager’s input buffer is
    full.
natkeyboard.can_post (read-only)
    A Boolean indicating whether the emulated system supports posting character
    data via the natural keyboard manager.
natkeyboard.is_posting (read-only)
    A Boolean indicating whether posted character data is currently being
    delivered to the emulated system.
natkeyboard.in_use (read/write)
    A Boolean indicating whether “natural keyboard” mode is enabled.  When
    “natural keyboard” mode is enabled, the natural keyboard manager translates
    host character input to emulated system keystrokes.
natkeyboard.keyboards[]
    Gets the :ref:`keyboard/keypad input devices <luascript-ref-natkbddev>` in
    the emulated system, indexed by absolute device tag.  Index get has O(n)
    complexity; all other supported operations have O(1) complexity.


.. _luascript-ref-natkbddev:

Keyboard input device
---------------------

Represents a keyboard or keypad input device managed by the
:ref:`natural keyboard manager <luascript-ref-natkbdman>`.  Note that this is
not a :ref:`device <luascript-ref-device>` class.

Instantiation
~~~~~~~~~~~~~

manager.machine.natkeyboard.keyboards[tag]
    Gets the keyboard input device with the specified tag, or ``nil`` if the tag
    does not correspond to a keyboard input device.

Properties
~~~~~~~~~~

keyboard.device (read-only)
    The underlying :ref:`device <luascript-ref-device>`.
keyboard.tag (read-only)
    The absolute tag of the underlying device.
keyboard.basetag (read-only)
    The last component of the tag of the underlying device, or ``"root"`` for
    the root machine device.
keyboard.name (read-only)
    The human-readable description of the underlying device type.
keyboard.shortname (read-only)
    The identifier for the underlying device type.
keyboard.is_keypad (read-only)
    A Boolean indicating whether the underlying device has keypad inputs but no
    keyboard inputs.  This is used when determining which keyboard input devices
    should be enabled by default.
keyboard.enabled (read/write)
    A Boolean indicating whether the device’s keyboard and/or keypad inputs are
    enabled.


.. _luascript-ref-ioport:

I/O port
--------

Wraps MAME’s ``ioport_port`` class, representing an emulated I/O port.

Instantiation
~~~~~~~~~~~~~

manager.machine.ioport.ports[tag]
    Gets an emulated I/O port by absolute tag, or ``nil`` if the tag does not
    correspond to an I/O port.
manager.machine.devices[devtag]:ioport(porttag)
    Gets an emulated I/O port by tag relative to a device, or ``nil`` if no such
    I/O port exists.

Methods
~~~~~~~

port:read()
    Read the current input value.  Returns a 32-bit integer.
port:write(value, mask)
    Write to the I/O port output fields that are set in the specified mask.  The
    value and mask must be 32-bit integers.  Note that this does not set values
    for input fields.
port:field(mask)
    Get the first :ref:`I/O port field <luascript-ref-ioportfield>`
    corresponding to the bits that are set in the specified mask, or ``nil`` if
    there is no corresponding field.

Properties
~~~~~~~~~~

port.device (read-only)
    The device that owns the I/O port.
port.tag (read-only)
    The absolute tag of the I/O port
port.active (read-only)
    A mask indicating which bits of the I/O port correspond to active fields
    (i.e. not unused or unassigned bits).
port.live (read-only)
    The live state of the I/O port.
port.fields[] (read-only)
    Gets a table of :ref:`fields <luascript-ref-ioportfield>` indexed by name.


.. _luascript-ref-ioportfield:

I/O port field
--------------

Wraps MAME’s ``ioport_field`` class, representing a field within an I/O port.

Instantiation
~~~~~~~~~~~~~

manager.machine.ioport.ports[tag]:field(mask)
    Gets a field for the given port by bit mask.
manager.machine.ioport.ports[tag].fields[name]
    Gets a field for the given port by display name.

Methods
~~~~~~~

field:set_value(value)
    Set the value of the I/O port field.  For digital fields, the value is
    compared to zero to determine whether the field should be active; for
    analog fields, the value must be right-aligned and in the correct range.
field:clear_value()
    Clear programmatically overridden value and restore the field’s regular
    behaviour.
field:set_input_seq(seqtype, seq)
    Set the :ref:`input sequence <luascript-ref-inputseq>` for the specified
    sequence type.  This is used to configure per-machine input settings.  The
    sequence type must be ``"standard"``, ``"increment"`` or ``"decrement"``.
field:input_seq(seq_type)
    Get the configured :ref:`input sequence <luascript-ref-inputseq>` for the
    specified sequence type.  This gets per-machine input assignments.  The
    sequence type must be ``"standard"``, ``"increment"`` or ``"decrement"``.
field:set_default_input_seq(seq_type, seq)
    Set the default :ref:`input sequence <luascript-ref-inputseq>` for the
    specified sequence type.  This overrides the default input assignment for a
    specific input.  The sequence type must be ``"standard"``, ``"increment"``
    or ``"decrement"``.
field:default_input_seq(seq_type)
    Gets the default :ref:`input sequence <luascript-ref-inputseq>` for the
    specified sequence type.  If the default assignment is not overridden, this
    returns the general input assignment for the field’s input type.  The
    sequence type must be ``"standard"``, ``"increment"`` or ``"decrement"``.
field:keyboard_codes(shift)
    Gets a table of characters corresponding to the field for the specified
    shift state.  The shift state is a bit mask of active shift keys.

Properties
~~~~~~~~~~

field.device (read-only)
    The device that owns the port that the field belongs to.
field.port (read-only)
    The :ref:`I/O port <luascript-ref-ioport>` that the field belongs to.
field.live (read-only)
    The :ref:`live state <luascript-ref-ioportfieldlive>` of the field.
field.type (read-only)
    The input type of the field.  This is an enumerated value.
field.name (read-only)
    The display name for the field.
field.default_name (read-only)
    The name for the field from the emulated system’s configuration (cannot be
    overridden by scripts or plugins).
field.player (read-only)
    Zero-based player number for the field.
field.mask (read-only)
    Bits in the I/O port corresponding to this field.
field.defvalue (read-only)
    The field’s default value.
field.minvalue (read-only)
    The minimum allowed value for analog fields, or ``nil`` for digital fields.
field.maxvalue (read-only)
    The maximum allowed value for analog fields, or ``nil`` for digital fields.
field.sensitivity (read-only)
    The sensitivity or gain for analog fields, or ``nil`` for digital fields.
field.way (read-only)
    The number of directions allowed by the restrictor plate/gate for a digital
    joystick, or zero (0) for other inputs.
field.type_class (read-only)
    The type class for the input field – one of ``"keyboard"``,
    ``"controller"``, ``"config"``, ``"dipswitch"`` or ``"misc"``.
field.is_analog (read-only)
    A Boolean indicating whether the field is an analog axis or positional
    control.
field.is_digital_joystick (read-only)
    A Boolean indicating whether the field corresponds to a digital joystick
    switch.
field.enabled (read-only)
    A Boolean indicating whether the field is enabled.
field.optional (read-only)
    A Boolean indicating whether the field is optional and not required to use
    the emulated system.
field.cocktail (read-only)
    A Boolean indicating whether the field is only used when the system is
    configured for a cocktail table cabinet.
field.toggle (read-only)
    A Boolean indicating whether the field corresponds to a hardware toggle
    switch or push-on, push-off button.
field.rotated (read-only)
    A Boolean indicating whether the field corresponds to a control that is
    rotated relative its standard orientation.
field.analog_reverse (read-only)
    A Boolean indicating whether the field corresponds to an analog control that
    increases in the opposite direction to the convention (e.g. larger values
    when a pedal is released or a joystick is moved to the left).
field.analog_reset (read-only)
    A Boolean indicating whether the field corresponds to an incremental
    position input (e.g. a dial or trackball axis) that should be reset to zero
    for every video frame.
field.analog_wraps (read-only)
    A Boolean indicating whether the field corresponds to an analog input that
    wraps from one end of its range to the other (e.g. an incremental position
    input like a dial or trackball axis).
field.analog_invert (read-only)
    A Boolean indicating whether the field corresponds to an analog input that
    has its value ones-complemented.
field.impulse (read-only)
    A Boolean indicating whether the field corresponds to a digital input that
    activates for a fixed amount of time.
field.crosshair_scale (read-only)
    The scale factor for translating the field’s range to crosshair position.  A
    value of one (1) translates the field’s full range to the full width or
    height the screen.
field.crosshair_offset (read-only)
    The offset for translating the field’s range to crosshair position.
field.user_value (read/write)
    The value for DIP switch or configuration settings.
field.settings[] (read-only)
    Gets a table of the currently enabled settings for a DIP switch or
    configuration field, indexed by value.


.. _luascript-ref-ioportfieldlive:

Live I/O port field state
-------------------------

Wraps MAME’s ``ioport_field_live`` class, representing the live state of an I/O
port field.

Instantiation
~~~~~~~~~~~~~

manager.machine.ioport.ports[tag]:field(mask).live
    Gets the live state for an I/O port field.

Properties
~~~~~~~~~~

live.name
    Display name for the field.


.. _luascript-ref-inputtype:

Input type
----------

Wraps MAME’s ``input_type_entry`` class, representing an emulated input type or
emulator UI input type.  Input types are uniquely identified by the combination
of their enumerated type value and player index.

Instantiation
~~~~~~~~~~~~~

manager.machine.ioport.types[index]
    Gets a supported input type.

Properties
~~~~~~~~~~

type.type (read-only)
    An enumerated value representing the type of input.
type.group (read-only)
    An integer giving the grouping for the input type.  Should be used to
    provide canonical grouping in an input configuration UI.
type.player (read-only)
    The zero-based player number, or zero for non-player controls.
type.token (read-only)
    The token string for the input type, used in configuration files.
type.name (read-only)
    The display name for the input type.
type.is_analog (read-only)
    A Boolean indicating whether the input type is analog or digital.  Inputs
    that only have on and off states are considered digital, while all other
    inputs are considered analog, even if they can only represent discrete
    values or positions.


.. _luascript-ref-inputman:

Input manager
-------------

Wraps MAME’s ``input_manager`` class, which reads host input devices and checks
whether configured inputs are active.

Instantiation
~~~~~~~~~~~~~

manager.machine.input
    Gets the global input manager instance for the emulated system.

Methods
~~~~~~~

input:code_value(code)
    Gets the current value for the host input corresponding to the specified
    code.  Returns a signed integer value, where zero is the neutral position.
input:code_pressed(code)
    Returns a Boolean indicating whether the host input corresponding to the
    specified code has a non-zero value (i.e. it is not in the neutral
    position).
input:code_pressed_once(code)
    Returns a Boolean indicating whether the host input corresponding to the
    specified code has moved away from the neutral position since the last time
    it was checked using this function.  The input manager can track a limited
    number of inputs this way.
input:code_name(code)
    Get display name for an input code.
input:code_to_token(code)
    Get token string for an input code.  This should be used when saving
    configuration.
input:code_from_token(token)
    Convert a token string to an input code.  Returns the invalid input code if
    the token is not valid or belongs to an input device that is not present.
input:seq_pressed(seq)
    Returns a Boolean indicating whether the supplied
    :ref:`input sequence <luascript-ref-inputseq>` is currently pressed.
input:seq_clean(seq)
    Remove invalid elements from the supplied
    :ref:`input sequence <luascript-ref-inputseq>`.  Returns the new, cleaned
    input sequence.
input:seq_name(seq)
    Get display text for an :ref:`input sequence <luascript-ref-inputseq>`.
input:seq_to_tokens(seq)
    Convert an :ref:`input sequence <luascript-ref-inputseq>` to a token string.
    This should be used when saving configuration.
input:seq_from_tokens(tokens)
    Convert a token string to an
    :ref:`input sequence <luascript-ref-inputseq>`.  This should be used when
    loading configuration.
input:axis_code_poller()
    Returns an :ref:`input code poller <luascript-ref-inputcodepoll>` for
    obtaining an analog host input code.
input:switch_code_poller()
    Returns an :ref:`input code poller <luascript-ref-inputcodepoll>` for
    obtaining a host switch input code.
input:keyboard_code_poller()
    Returns an :ref:`input code poller <luascript-ref-inputcodepoll>` for
    obtaining a host switch input code that only considers keyboard input
    devices.
input:axis_sequence_poller()
    Returns an :ref:`input sequence poller <luascript-ref-inputseqpoll>` for
    obtaining an :ref:`input sequence <luascript-ref-inputseq>` for configuring
    an analog input assignment.
input:axis_sequence_poller()
    Returns an :ref:`input sequence poller <luascript-ref-inputseqpoll>` for
    obtaining an :ref:`input sequence <luascript-ref-inputseq>` for configuring
    a digital input assignment.

Properties
~~~~~~~~~~

input.device_classes[] (read-only)
    Gets a table of host
    :ref:`input device classes <luascript-ref-inputdevclass>` indexed by name.


.. _luascript-ref-inputcodepoll:

Input code poller
-----------------

Wraps MAME’s ``input_code_poller`` class, used to poll for host inputs being
activated.

Instantiation
~~~~~~~~~~~~~

manager.machine.input:axis_code_poller()
    Returns an input code poller that polls for analog inputs being activated.
manager.machine.input:switch_code_poller()
    Returns an input code poller that polls for host switch inputs being
    activated.
manager.machine.input:keyboard_code_poller()
    Returns an input code poller that polls for host switch inputs being
    activated, only considering keyboard input devices.

Methods
~~~~~~~

poller:reset()
    Resets the polling logic.  Active switch inputs are cleared and initial
    analog input positions are set.
poller:poll()
    Returns an input code corresponding to the first relevant host input that
    has been activated since the last time the method was called.  Returns the
    invalid input code if no relevant input has been activated.


.. _luascript-ref-inputseqpoll:

Input sequence poller
---------------------

Wraps MAME’s ``input_sequence_poller`` poller class, which allows users to
assign host input combinations to emulated inputs and other actions.

Instantiation
~~~~~~~~~~~~~

manager.machine.input:axis_sequence_poller()
    Returns an input sequence poller for assigning host inputs to an analog
    input.
manager.machine.input:switch_sequence_poller()
    Returns an input sequence poller for assigning host inputs to a switch
    input.

Methods
~~~~~~~

poller:start([seq])
    Start polling.  If a sequence is supplied, it is used as a starting
    sequence: for analog inputs, the user can cycle between the full range, and
    the positive and negative portions of an axis; for switch inputs, an “or”
    code is appended and the user can add an alternate host input combination.
poller:poll()
    Polls for user input and updates the sequence if appropriate.  Returns a
    Boolean indicating whether sequence input is complete.  If this method
    returns false, you should continue polling.

Properties
~~~~~~~~~~

poller.sequence (read-only)
    The current :ref:`input sequence <luascript-ref-inputseq>`.  This is updated
    while polling.  It is possible for the sequence to become invalid.
poller.valid (read-only)
    A Boolean indicating whether the current input sequence is valid.
poller.modified (read-only)
    A Boolean indicating whether the sequence was changed by any user input
    since starting polling.


.. _luascript-ref-inputseq:

Input sequence
--------------

Wraps MAME’s ``input_seq`` class, representing a combination of host inputs that
can be read or assigned to an emulated input.  Input sequences can be
manipulated using :ref:`input manager <luascript-ref-inputman>` methods.  Use an
:ref:`input sequence poller <luascript-ref-inputseqpoll>` to obtain an input
sequence from the user.

Instantiation
~~~~~~~~~~~~~

emu.input_seq()
    Creates an empty input sequence.
emu.input_seq(seq)
    Creates a copy of an existing input sequence.

Methods
~~~~~~~

seq:reset()
    Clears the input sequence, removing all items.
seq:set_default()
    Sets the input sequence to a single item containing the metavalue specifying
    that the default setting should be used.

Properties
~~~~~~~~~~

seq.empty (read-only)
    A Boolean indicating whether the input sequence is empty (contains no items,
    indicating an unassigned input).
seq.length (read-only)
    The number of items in the input sequence.
seq.is_valid (read-only)
    A Boolean indicating whether the input sequence is a valid.  To be valid, it
    must contain at least one item, all items must be valid codes, all product
    groups must contain at least one item that is not negated, and items
    referring to absolute and relative axes must not be mixed within a product
    group.
seq.is_default (read-only)
    A Boolean indicating whether the input sequence specifies that the default
    setting should be used.


.. _luascript-ref-inputdevclass:

Host input device class
-----------------------

Wraps MAME’s ``input_class`` class, representing a category of host input
devices (e.g. keyboards or joysticks).

Instantiation
~~~~~~~~~~~~~

manager.machine.input.device_classes[name]
    Gets an input device class by name.

Properties
~~~~~~~~~~

devclass.name (read-only)
    The device class name.
devclass.enabled (read-only)
    A Boolean indicating whether the device class is enabled.
devclass.multi (read-only)
    A Boolean indicating whether the device class supports multiple devices, or
    inputs from all devices in the class are combined and treated as a single
    device.
devclass.devices[] (read-only)
    Gets a table of :ref:`host input devices <luascript-ref-inputdev>` in the
    class.  Keys are one-based indices.


.. _luascript-ref-inputdev:

Host input device
-----------------

Wraps MAME’s ``input_device`` class, representing a host input device.

Instantiation
~~~~~~~~~~~~~

manager.machine.input.device_classes[name].devices[index]
    Gets a specific host input device.

Properties
~~~~~~~~~~

inputdev.name (read-only)
    Display name for the device.  This is not guaranteed to be unique.
inputdev.id (read-only)
    Unique identifier string for the device.  This may not be human-readable.
inputdev.devindex (read-only)
    Device index within the device class.  This is not necessarily the same as
    the index in the ``devices`` property of the device class – the ``devindex``
    indices may not be contiguous.
inputdev.items (read-only)
    Gets a table of :ref:`input items <luascript-ref-inputdevitem>`, indexed
    by item ID.  The item ID is an enumerated value.


.. _luascript-ref-inputdevitem:

Host input device item
----------------------

Wraps MAME’s ``input_device_item`` class, representing a single host input (e.g.
a key, button, or axis).

Instantiation
~~~~~~~~~~~~~

manager.machine.input.device_classes[name].devices[index].items[id]
    Gets an individual host input item.  The item ID is an enumerated value.

Properties
~~~~~~~~~~

item.name (read-only)
    The display name of the input item.  Note that this is just the name of the
    item itself, and does not include the device name.  The full display name
    for the item can be obtained by calling the ``code_name`` method on the
    :ref:`input manager <luascript-ref-inputman>` with the item’s code.
item.code (read-only)
    The input item’s identification code.  This is used by several
    :ref:`input manager <luascript-ref-inputman>` methods.
item.token (read-only)
    The input item’s token string.  Note that this is a token fragment for the
    item itself, and does not include the device portion.  The full token for
    the item can be obtained by calling the ``code_to_token`` method on the
    :ref:`input manager <luascript-ref-inputman>` with the item’s code.
item.current (read-only)
    The item’s current value.  This is a signed integer where zero is the
    neutral position.


.. _luascript-ref-uiinputman:

UI input manager
----------------

Wraps MAME’s ``ui_input_manager`` class, which is used for high-level input.

Instantiation
~~~~~~~~~~~~~

manager.machine.uiinput
    Gets the global UI input manager instance for the machine.

Methods
~~~~~~~

uiinput:reset()
    Clears pending events and UI input states.  Should be called when leaving a
    modal state where input is handled directly (e.g. configuring an input
    combination).
uiinput:pressed(type)
    Returns a Boolean indicating whether the specified UI input has been
    pressed.  The input type is an enumerated value.
uiinput:pressed_repeat(type, speed)
    Returns a Boolean indicating whether the specified UI input has been
    pressed or auto-repeat has been triggered at the specified speed.  The input
    type is an enumerated value; the speed is an interval in sixtieths of a
    second.

Properties
~~~~~~~~~~

uiinput.presses_enabled (read/write)
    Whether the UI input manager will check for UI inputs frame updates.
