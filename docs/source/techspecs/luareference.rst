.. _luareference:

MAME Lua Class Reference
========================

.. contents::
    :local:
    :depth: 2


.. _luareference-intro:

Introduction
------------

Various aspects of MAME can be controlled using Lua scripting.  Many key classes
are exposed as Lua objects.

.. _luareference-intro-containers:

Containers
~~~~~~~~~~

Many properties yield container wrappers.  Container wrappers are cheap to
create, and provide an interface that is similar to a read-only table.  The
complexity of operations may vary.  Container wrappers usually provide most of
these operations:

#c
    Get the number of items in the container.
c[k]
    Returns the item corresponding to the key ``k``, or ``nil`` if the key is
    not present.
pairs(c)
    Iterate container by key and value.  The key is what you would pass to the
    index operator or the ``get`` method to get the value.
ipairs(c)
    Iterate container by index and value.  The index is what you would pass to
    the ``at`` method to get the value (this may be the same as the key for some
    containers).
c:empty()
    Returns a Boolean indicating whether there are no items in the container.
c:get(k)
    Returns the item corresponding to the key ``k``, or ``nil`` if the key is
    not present.  Usually equivalent to the index operator.
c:at(i)
    Returns the value at the 1-based index ``i``, or ``nil`` if it is out of
    range.
c:find(v)
    Returns the key for item ``v``, or ``nil`` if it is not in the collection.
    The key is what you would pass to the index operator to get the value.
c:find(v)
    Returns the key for item ``v``, or ``nil`` if it is not in the container.
    The key is what you would pass to the index operator to get the value.
c:index_of(v)
    Returns the 1-based index for item ``v``, or ``nil`` if it is not in the
    container.  The index is what you would pass to the ``at`` method to get the
    value.


.. _luareference-mem:

Memory system
-------------

MAME’s Lua interface exposes various memory system objects, including address
spaces, memory shares, memory banks, and memory regions.  Scripts can read from
and write to the emulated memory system.

.. _luareference-mem-manager:

Memory manager
~~~~~~~~~~~~~~

Wraps MAME’s ``memory_manager`` class, which allows the memory shares, banks and
regions in a system to be enumerated.

Instantiation
^^^^^^^^^^^^^

manager:machine():memory()
    Gets the global memory manager instance for the emulated system.

Properties
^^^^^^^^^^

memory.shares[]
    The :ref:`memory shares <luareference-mem-share>` in the system, indexed by
    absolute tag.  The ``at`` and ``index_of`` methods have O(n) complexity; all
    other supported operations have O(1) complexity.
memory.banks[]
    The :ref:`memory banks <luareference-mem-bank>` in the system, indexed by
    absolute tag.  The ``at`` and ``index_of`` methods have O(n) complexity; all
    other supported operations have O(1) complexity.
memory.regions[]
    The :ref:`memory regions <luareference-mem-region>` in the system, indexed
    by absolute tag.  The ``at`` and ``index_of`` methods have O(n) complexity;
    all other supported operations have O(1) complexity.

.. _luareference-mem-space:

Address space
~~~~~~~~~~~~~

Wraps MAME’s ``address_space`` class, which represent’s an address space
belonging to a device.

Instantiation
^^^^^^^^^^^^^

manager:machine().devices[tag].spaces[name]
    Gets the address space with the specified name for a given device.  Note
    that names are specific to the device type.

Methods
^^^^^^^

space:read_i{8,16,32,64}(addr)
    Reads a signed integer value of the size in bits from the specified address.
space:read_u{8,16,32,64}(addr)
    Reads an unsigned integer value of the size in bits from the specified
    address.
space:write_i{8,16,32,64}(addr, val)
    Writes a signed integer value of the size in bits to the specified address.
space:write_u{8,16,32,64}(addr, val)
    Writes an unsigned integer value of the size in bits to the specified
    address.
space:readv_i{8,16,32,64}(addr)
    Reads a signed integer value of the size in bits from the specified virtual
    address.  The address is translated with the debug read intent.  Returns
    zero if address translation fails.
space:readv_u{8,16,32,64}(addr)
    Reads an unsigned integer value of the size in bits from the specified
    virtual address.  The address is translated with the debug read intent.
    Returns zero if address translation fails.
space:writev_i{8,16,32,64}(addr, val)
    Writes a signed integer value of the size in bits to the specified virtual
    address.  The address is translated with the debug write intent.  Does not
    write if address translation fails.
space:writev_u{8,16,32,64}(addr, val)
    Writes an unsigned integer value of the size in bits to the specified
    virtual address.  The address is translated with the debug write intent.
    Does not write if address translation fails.
space:read_direct_i{8,16,32,64}(addr)
    Reads a signed integer value of the size in bits from the specified address
    one byte at a time by obtaining a read pointer for each byte address.  If
    a read pointer cannot be obtained for a byte address, the corresponding
    result byte will be zero.
space:read_direct_u{8,16,32,64}(addr)
    Reads an unsigned integer value of the size in bits from the specified
    address one byte at a time by obtaining a read pointer for each byte
    address.  If a read pointer cannot be obtained for a byte address, the
    corresponding result byte will be zero.
space:write_direct_i{8,16,32,64}(addr, val)
    Writes a signed integer value of the size in bits to the specified address
    one byte at a time by obtaining a write pointer for each byte address.  If
    a write pointer cannot be obtained for a byte address, the corresponding
    byte will not be written.
space:write_direct_u{8,16,32,64}(addr, val)
    Writes an unsigned integer value of the size in bits to the specified
    address one byte at a time by obtaining a write pointer for each byte
    address.  If a write pointer cannot be obtained for a byte address, the
    corresponding byte will not be written.
space:read_range(start, end, width, [step])
    Reads a range of addresses as a binary string.  The end address must be
    greater than or equal to the start address.  The width must be 8, 16, 30 or
    64.  If the step is provided, it must be a positive number of elements.

Properties
^^^^^^^^^^

space.name (read-only)
    The display name for the address space.
space.shift (read-only)
    The address address granularity for the address space specified as the shift
    required to translate a byte address to a native address.  Positive values
    shift towards the most significant bit (left) and negative values shift
    towards the least significant bit (right).
space.index (read-only)
    The zero-based space index.  Some space indices have special meanings for
    the debugger.
space.address_mask (read-only)
    The address mask for the space.
space.data_width (read-only)
    The data width for the space in bits.
space.endianness (read-only)
    The Endianness of the space (``"big"`` or ``"little"``).
space.map (read-only)
    The configured :ref:`address map <luareference-mem-map>` for the space or
    ``nil``.

.. _luareference-mem-map:

Address map
~~~~~~~~~~~

Wraps MAME’s ``address_map`` class, used to configure handlers for an address
space.

Instantiation
^^^^^^^^^^^^^

manager:machine().devices[tag].spaces[name].map
    Gets the configured address map for an address space, or ``nil`` if no map
    is configured.

Properties
^^^^^^^^^^

map.spacenum (read-only)
    The address space number of the address space the map is associated with
map.device (read-only)
    The device that owns the address space the map is associated with.
map.unmap_value (read-only)
    The constant value to return from unmapped reads.
map.global_mask (read-only)
    Global mask to be applied to all addresses when accessing the space.
map.entries[] (read-only)
    The configured :ref:`entries <luareference-mem-mapentry>` in the address
    map.  Uses 1-based integer indices.  The index operator and the ``at``
    method have O(n) complexity.

.. _luareference-mem-mapentry:

Address map entry
~~~~~~~~~~~~~~~~~

Wraps MAME’s ``address_map_entry`` class, representing an entry in a configured
address map.

Instantiation
^^^^^^^^^^^^^

manager:machine().devices[tag].spaces[name].map.entries[index]
    Gets an entry from the configured map for an address space.

Properties
^^^^^^^^^^

entry.address_start (read-only)
    Start address of the entry’s range.
entry.address_end (read-only)
    End address of the entry’s range (inclusive).
entry.address_mirror (read-only)
    Address mirror bits.
entry.address_end (read-only)
    Address mask bits.  Only valid for handlers.
entry.mask (read-only)
    Lane mask, indicating which data lines on the bus are connected to the
    handler.
entry.cswidth (read-only)
    The trigger width for a handler that isn’t connected to all the data lines.
entry.read (read-only)
    :ref:`Additional data <luareference-memory-handlerdata>` for the read
    handler.
entry.write (read-only)
    :ref:`Additional data <luareference-memory-handlerdata>` for the write
    handler.
entry.share (read-only)
    Memory share tag for making RAM entries accessible or ``nil``.
entry.address_end (read-only)
    Explicit memory region tag for ROM entries, or ``nil``.
entry.region_offset (read-only)
    Starting offset in memory region for ROM entries.

.. _luareference-memory-handlerdata:

Address map handler data
~~~~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``map_handler_data`` class, which provides configuration data to
handlers in address maps.

Instantiation
^^^^^^^^^^^^^

manager:machine().devices[tag].spaces[name].map.entries[index].read
    Gets the read handler data for an address map entry.
manager:machine().devices[tag].spaces[name].map.entries[index].write
    Gets the write handler data for an address map entry.

Properties
^^^^^^^^^^

data.handlertype (read-only)
    Handler type.  Will be one of ``"none"``, ``"ram"``, ``"rom"``, ``"nop"``,
    ``"unmap"``, ``"delegate"``, ``"port"``, ``"bank"``, ``"submap"``, or
    ``"unknown"``.  Note that multiple handler type values can yield
    ``"delegate"`` or ``"unknown"``.
data.bits (read-only)
    Data width for the handler in bits.
data.name (read-only)
    Display name for the handler, or ``nil``.
data.tag (read-only)
    Tag for I/O ports and memory banks, or ``nil``.

.. _luareference-mem-share:

Memory share
~~~~~~~~~~~~

Wraps MAME’s ``memory_share`` class, representing a named allocated memory zone.

Instantiation
^^^^^^^^^^^^^

manager:machine():memory().shares[tag]
    Gets a memory share by absolute tag, or ``nil`` if no such memory share
    exists.
manager:machine().devices[tag]:memshare(tag)
    Gets a memory share by tag relative to a device, or ``nil`` if no such
    memory share exists.

Methods
^^^^^^^

share:read_i{8,16,32,64}(offs)
    Reads a signed integer value of the size in bits from the specified offset
    in the memory share.
share:read_u{8,16,32,64}(offs)
    Reads an unsigned integer value of the size in bits from the specified
    offset in the memory share.
share:write_i{8,16,32,64}(offs, val)
    Writes a signed integer value of the size in bits to the specified offset in
    the memory share.
share:write_u{8,16,32,64}(offs, val)
    Writes an unsigned integer value of the size in bits to the specified offset
    in the memory share.

Properties
^^^^^^^^^^

share.tag (read-only)
    The absolute tag of the memory share.
share.size (read-only)
    The size of the memory share in bytes.
share.length (read-only)
    The length of the memory share in native width elements.
share.endianness (read-only)
    The Endianness of the memory share (``"big"`` or ``"little"``).
share.bitwidth (read-only)
    The native element width of the memory share in bits.
share.bytewidth (read-only)
    The native element width of the memory share in bytes.

.. _luareference-mem-bank:

Memory bank
~~~~~~~~~~~

Wraps MAME’s ``memory_bank`` class, representing a named memory zone
indirection.

Instantiation
^^^^^^^^^^^^^

manager:machine():memory().banks[tag]
    Gets a memory region by absolute tag, or ``nil`` if no such memory bank
    exists.
manager:machine().devices[tag]:membank(tag)
    Gets a memory region by tag relative to a device, or ``nil`` if no such
    memory bank exists.

Properties
^^^^^^^^^^

bank.tag (read-only)
    The absolute tag of the memory bank.
bank.entry (read/write)
    The currently selected zero-based entry number.

.. _luareference-mem-region:

Memory region
~~~~~~~~~~~~~

Wraps MAME’s ``memory_region`` class, representing a memory region used to store
read-only data like ROMs or the result of fixed decryptions.

Instantiation
^^^^^^^^^^^^^

manager:machine():memory().regions[tag]
    Gets a memory region by absolute tag, or ``nil`` if no such memory region
    exists.
manager:machine().devices[tag]:memregion(tag)
    Gets a memory region by tag relative to a device, or ``nil`` if no such
    memory region exists.

Methods
^^^^^^^

region:read_i{8,16,32,64}(offs)
    Reads a signed integer value of the size in bits from the specified offset
    in the memory region.
region:read_u{8,16,32,64}(offs)
    Reads an unsigned integer value of the size in bits from the specified
    offset in the memory region.
region:write_i{8,16,32,64}(offs, val)
    Writes a signed integer value of the size in bits to the specified offset in
    the memory region.
region:write_u{8,16,32,64}(offs, val)
    Writes an unsigned integer value of the size in bits to the specified offset
    in the memory region.

Properties
^^^^^^^^^^

region.tag (read-only)
    The absolute tag of the memory region.
region.size (read-only)
    The size of the memory region in bytes.
region.length (read-only)
    The length of the memory region in native width elements.
region.endianness (read-only)
    The Endianness of the memory region (``"big"`` or ``"little"``).
region.bitwidth (read-only)
    The native element width of the memory region in bits.
region.bytewidth (read-only)
    The native element width of the memory region in bytes.


.. _luareference-input:

Input system
------------

Allows scripts to get input from the user, and access I/O ports in the emulated
system.

.. _luareference-input-ioportman:

I/O port manager
~~~~~~~~~~~~~~~~

Wraps MAME’s ``ioport_manager`` class, which provides access to emulated I/O
ports and handles input configuration.

Instantiation
^^^^^^^^^^^^^

manager:machine():ioport()
    Gets the global I/O port manager instance for the emulated machine.

Methods
^^^^^^^

ioport:count_players()
    Returns the number of player controllers in the system.
ioport:type_group(type, player)
    Returns the I/O port group for the specified I/O port type and player
    number.  The I/O port type is an enumerated value.  The player number is a
    zero-based index.  Returns an integer giving the grouping for the input.

    This should be called with values obtained from I/O port fields to provide
    canonical grouping in an input configuration UI.
ioport:type_seq(type, player, seqtype)
    Get the configured input sequence for the specified input type, player and
    sequence type.  The sequence type must be ``"standard"``, ``"increment"``
    or ``"decrement"``.  This provides access to general input configuration.

Properties
^^^^^^^^^^

ioport.ports[]
    Gets the emulated :ref:`I/O ports <luareference-input-ioport>` in the
    system.  Keys are absolute tags.  The ``at`` and ``index_of`` methods have
    O(n) complexity; all other supported operations have O(1) complexity.
ioport.natkeyboard
    Gets the :ref:`natural keyboard manager <luareference-input-natkbd>`, used
    for controlling keyboard and keypad input to the emulated system.

.. _luareference-input-natkbd:

Natural keyboard manager
~~~~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``natural_keyboard`` class, which manages emulated keyboard and
keypad inputs.

Instantiation
^^^^^^^^^^^^^

manager:machine():ioport().natkeyboard
    Gets the global natural keyboard manager instance for the emulated machine.

Methods
^^^^^^^

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
^^^^^^^^^^

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
    Gets the :ref:`keyboard/keypad input devices <luareference-input-kbddev>` in
    the emulated system, indexed by absolute device tag.  Index get has O(n)
    complexity; all other supported operations have O(1) complexity.

.. _luareference-input-kbddev:

Keyboard input device
~~~~~~~~~~~~~~~~~~~~~

Represents a keyboard or keypad input device managed by the
:ref:`natural keyboard manager <luareference-input-natkbd>`.

Instantiation
^^^^^^^^^^^^^

manager:machine():ioport().natkeyboard.keyboards[tag]
    Gets the keyboard input device with the specified tag, or ``nil`` if the tag
    does not correspond to a keyboard input device.

Properties
^^^^^^^^^^

keyboard.device (read-only)
    The underlying device.
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

.. _luareference-input-ioport:

I/O port
~~~~~~~~

Wraps MAME’s ``ioport_port`` class, representing an emulated I/O port.

Instantiation
^^^^^^^^^^^^^

manager:machine():ioport().ports[tag]
    Gets an emulated I/O port by absolute tag, or ``nil`` if the tag does not
    correspond to an I/O port.
manager:machine().devices[devtag]:ioport(porttag)
    Gets an emulated I/O port by tag relative to a device, or ``nil`` if no such
    I/O port exists.

Methods
^^^^^^^

port:read()
    Read the current input value.  Returns a 32-bit integer.
port:write(value, mask)
    Write to the I/O port output fields that are set in the specified mask.  The
    value and mask must be 32-bit integers.  Note that this does not set values
    for input fields.
port:field(mask)
    Get the first :ref:`I/O port field <luareference-input-field>` corresponding
    to the bits that are set in the specified mask, or ``nil`` if there is no
    corresponding field.

Properties
^^^^^^^^^^

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
    Gets a table of :ref:`fields <luareference-input-field>` indexed by name.

.. _luareference-input-field:

I/O port field
~~~~~~~~~~~~~~

Wraps MAME’s ``ioport_field`` class, representing a field within an I/O port.

Instantiation
^^^^^^^^^^^^^

manager:machine():ioport().ports[tag]:field[mask]
    Gets a field for the given port by bit mask.
manager:machine():ioport().ports[tag].fields[name]
    Gets a field for the given port by display name.

Methods
^^^^^^^

field:set_value(value)
    Set the value of the I/O port field.  For digital fields, the value is
    compared to zero to determine whether the field should be active; for
    analog fields, the value must be right-aligned and in the correct range.
field:set_input_seq(seqtype, seq)
    Set the input sequence for the specified sequence type.  This is used to
    configure per-machine input settings.  The sequence type must be
    ``"standard"``, ``"increment"`` or ``"decrement"``.
field:input_seq(seq_type)
    Get the configured input sequence for the specified sequence type.  This
    gets per-machine input settings.  The sequence type must be ``"standard"``,
    ``"increment"`` or ``"decrement"``.
field:set_default_input_seq(seq_type, seq)
    Set the default input sequence for the specified sequence type.  This is
    used to configure general input settings.  The sequence type must be
    ``"standard"``, ``"increment"`` or ``"decrement"``.
field:default_input_seq(seq_type)
    Gets the default input sequence for the specified sequence type.  This is
    gets general input settings.  The sequence type must be ``"standard"``,
    ``"increment"`` or ``"decrement"``.
field:keyboard_codes(shift)
    Gets a table of characters corresponding to the field for the specified
    shift state.  The shift state is a bit mask of active shift keys.

Properties
^^^^^^^^^^

field.device (read-only)
    The device that owns the port that the field belongs to.
field.port (read-only)
    The :ref:`I/O port <luareference-input-ioport>` that the field belongs to.
field.live (read-only)
    The :ref:`live state <luareference-input-fieldlive>` of the field.
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
    The field’s default value
field.sensitivity (read-only)
    The sensitivity or gain for analog fields
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

.. _luareference-input-fieldlive:

Live I/O port field state
~~~~~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``ioport_field_live`` class, representing the live state of an I/O
port field.

Instantiation
^^^^^^^^^^^^^

manager:machine():ioport().ports[tag]:field(mask).live
    Gets the live state for an I/O port field.

Properties
^^^^^^^^^^

live.name
    Display name for the field.

.. _luareference-input-inputman:

Input manager
~~~~~~~~~~~~~

Wraps MAME’s ``input_manager`` class, which reads host input devices and checks
whether configured inputs are active.

Instantiation
^^^^^^^^^^^^^

manager:machine():input()
    Gets the global input manager instance for the emulated system.

Methods
^^^^^^^

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
    Returns a Boolen indicating whether the supplied input sequence is currently
    pressed.
input:seq_clean(seq)
    Remove invalid elements from the supplied input sequence.  Returns the new,
    cleaned input sequence.
input:seq_name(seq)
    Get display text for an inptu sequence.
input:seq_to_tokens(seq)
    Convert an input sequence to a token string.  This should be used when
    saving configuration.
input:seq_from_tokens(tokens)
    Convert a token string to an input sequence.  This should be used when
    loading configuration.
input:axis_code_poller()
    Returns an :ref:`input code poller <luareference-input-codepoll>` for
    obtaining an analog host input code.
input:switch_code_poller()
    Returns an :ref:`input code poller <luareference-input-codepoll>` for
    obtaining a host switch input code.
input:keyboard_code_poller()
    Returns an :ref:`input code poller <luareference-input-codepoll>` for
    obtaining a host switch input code that only considers keyboard input
    devices.
input:axis_sequence_poller()
    Returns an :ref:`input sequence poller <luareference-input-seqpoll>` for
    obtaining an input sequence for configuring an analog input.
input:axis_sequence_poller()
    Returns an :ref:`input sequence poller <luareference-input-seqpoll>` for
    obtaining an input sequence for configuring a digital input.

Properties
^^^^^^^^^^

input.device_classes[] (read-only)
    Gets a table of host
    :ref:`input device classes <luareference-input-devclass>` indexed by name.

.. _luareference-input-codepoll:

Input code poller
~~~~~~~~~~~~~~~~~

Wraps MAME’s ``input_code_poller`` class, used to poll for host inputs being
activated.

Instantiation
^^^^^^^^^^^^^

manager:machine():input():axis_code_poller()
    Returns an input code poller that polls for analog inputs being activated.
manager:machine():input():switch_code_poller()
    Returns an input code poller that polls for host switch inputs being
    activated.
manager:machine():input():keyboard_code_poller()
    Returns an input code poller that polls for host switch inputs being
    activated, only considering keyboard input devices.

Methods
^^^^^^^

poller:reset()
    Resets the polling logic.  Active switch inputs are cleared and initial
    analog input positions are set.
poller:poll()
    Returns an input code corresponding to the first relevant host input that
    has been activated since the last time the method was called.  Returns the
    invalid input code if no relevant input has been activated.

.. _luareference-input-seqpoll:

Input sequence poller
~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``input_sequence_poller`` poller class, which allows users to
assign host input combinations to emulated inputs and other actions.

Instantiation
^^^^^^^^^^^^^

manager:machine():input():axis_sequence_poller()
    Returns an input sequence poller for assigning host inputs to an analog
    input.
manager:machine():input():switch_sequence_poller()
    Returns an input sequence poller for assigning host inputs to a switch
    input.

Methods
^^^^^^^

poller:start([seq])
    Start polling.  If a sequence is supplied, it is used as a starting
    sequence: for analog inputs, the user can cycle between the full range, and
    the positive and negative portions of an axis; for switch inputs, an “or”
    code is appended and the user can add an alternate host input combination.
poller:poll()
    Polls for for user input and updates the sequence if appropriate.  Returns a
    Boolean indicating whether sequence input is complete.  If this method
    returns false, you should continue polling.

Properties
^^^^^^^^^^

poller.sequence (read-only)
    The current input sequence.  This is updated while polling.  It is possible
    for the sequence to become invalid.
poller.valid (read-only)
    A Boolean indicating whether the current input sequence is valid.
poller.modified (read-only)
    A Boolean indicating whether the sequence was changed by any user input
    since starting polling.

.. _luareference-input-devclass:

Host input device class
~~~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``input_class`` class, representing a category of host input
devices (e.g. keyboards or joysticks).

Instantiation
^^^^^^^^^^^^^

manager:machine():input().device_classes[name]
    Gets an input device class by name.

Properties
^^^^^^^^^^

devclass.name (read-only)
    The device class name.
devclass.enabled (read-only)
    A Boolean indicating whether the device class is enabled.
devclass.multi (read-only)
    A Boolean indicating whether the device class supports multiple devices, or
    inputs from all devices in the class are combined and treated as a single
    device.
devclass.devices[] (read-only)
    Gets a table of :ref:`host input devices <luareference-input-inputdev>` in
    the class.  Keys are one-based indices.

.. _luareference-input-inputdev:

Host input device
~~~~~~~~~~~~~~~~~

Wraps MAME’s ``input_device`` class, representing a host input device.

Instantiation
^^^^^^^^^^^^^

manager:machine():input().device_classes[name].devices[index]
    Gets a specific host input device.

Properties
^^^^^^^^^^

inputdev.name (read-only)
    Display name for the device.  This is not guaranteed to be unique.
inputdev.id (read-only)
    Unique identifier string for the device.  This may not be human-readable.
inputdev.devindex (read-only)
    Device index within the device class.  This is not necessarily the same as
    the index in the ``devices`` property of the device class – the ``devindex``
    indices may not be contiguous.
inputdev.items (read-only)
    Gets a table of :ref:`input items <luareference-input-inputitem>`, indexed
    by item ID.  The item ID is an enumerated value.

.. _luareference-input-inputitem:

Host input device item
~~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``input_device_item`` class, representing a single host input (e.g.
a key, button, or axis).

Instantiation
^^^^^^^^^^^^^

manager:machine():input().device_classes[name].devices[index].items[id]
    Gets an individual host input item.  The item ID is an enumerated value.

Properties
^^^^^^^^^^

item.name (read-only)
    The display name of the input item.  Note that this is just the name of the
    item itself, and does not include the device name.  The full display name
    for the item can be obtained by calling the ``code_name`` method on the
    :ref:`input manager <luareference-input-inputman>` with the item’s code.
item.code (read-only)
    The input item’s identification code.  This is used by several
    :ref:`input manager <luareference-input-inputman>` methods.
item.token (read-only)
    The input item’s token string.  Note that this is a token fragment for the
    item itself, and does not include the device portion.  The full token for
    the item can be obtained by calling the ``code_to_token`` method on the
    :ref:`input manager <luareference-input-inputman>` with the item’s code.
item.current (read-only)
    The item’s current value.  This is a signed integer where zero is the
    neutral position.

.. _luareference-input-uiinput:

UI input manager
~~~~~~~~~~~~~~~~

Wraps MAME’s ``ui_input_manager`` class, which is used for high-level input.

Instantiation
^^^^^^^^^^^^^

manager:machine():uiinput()
    Gets the global UI input manager instance for the machine.

Methods
^^^^^^^

uiinput:find_mouse()
    Returns host system mouse pointer X position, Y position, button state, and
    the :ref:`render target <luareference-render-target>` it falls in.  The
    position is in host pixels, where zero is at the top/left.  The button state
    is a Boolean indicating whether the primary mouse button is pressed.

    If the mouse pointer is not over one of MAME’s windows, this may return the
    position and render target from when the mouse pointer was most recently
    over one of MAME’s windows.  The render target may be ``nil`` if the mouse
    pointer is not over one of MAME’s windows.
uiinput:pressed(type)
    Returns a Boolean indicating whether the specified UI input has been
    pressed.  The input type is an enumerated value.
uiinput:pressed_repeat(type, speed)
    Returns a Boolean indicating whether the specified UI input has been
    pressed or auto-repeat has been triggered at the specified speed.  The input
    type is an enumerated value; the speed is an interval in sixtieths of a
    second.

Properties
^^^^^^^^^^

uiinput.presses_enabled (read/write)
    Whether the UI input manager will check for UI inputs frame updates.


.. _luareference-render:

Render system
-------------

The render system is responsible for drawing what you see in MAME’s windows,
including emulated screens, artwork, and UI elements.

.. _luareference-render-bounds:

Render bounds
~~~~~~~~~~~~~

Wraps MAME’s ``render_bounds`` class, which represents a rectangle using
floating-point coordinates.

Instantiation
^^^^^^^^^^^^^

emu.render_bounds()
    Creates a render bounds object representing a unit square, with top left
    corner at (0, 0) and bottom right corner at (1, 1).  Note that render
    target coordinates don’t necessarily have equal X and Y scales, so this may
    not represent a square in the final output.
emu.render_bounds(left, top, right, bottom)
    Creates a render bounds object representing a rectangle with top left
    corner at (x0, y0) and bottom right corner at (x1, y1).

    The arguments must all be floating-point numbers.

Methods
^^^^^^^

bounds:includes(x, y)
    Returns a Boolean indicating whether the specified point falls within the
    rectangle.  The rectangle must be normalised for this to work (right greater
    than left and bottom greater than top).

    The arguments must both be floating-point numbers.
bounds:set_xy(left, top, right, bottom)
    Set the rectangle’s position and size in terms of the positions of the
    edges.

    The arguments must all be floating-point numbers.
bounds:set_wh(left, top, width, height)
    Set the rectangle’s position and size in terms of the top top left corner
    position, and the width and height.

    The arguments must all be floating-point numbers.

Properties
^^^^^^^^^^

bounds.x0 (read/write)
    The leftmost coordinate in the rectangle (i.e. the X coordinate of the left
    edge or the top left corner).
bounds.x1 (read/write)
    The rightmost coordinate in the rectangle (i.e. the X coordinate of the
    right edge or the bottom right corner).
bounds.y0 (read/write)
    The topmost coordinate in the rectangle (i.e. the Y coordinate of the top
    edge or the top left corner).
bounds.y1 (read/write)
    The bottommost coordinate in the rectangle (i.e. the Y coordinate of the
    bottom edge or the bottom right corner).
bounds.width (read/write)
    The width of the rectangle.  Setting this property changes the position of
    the rightmost edge.
bounds.height (read/write)
    The height of the rectangle.  Setting this property changes the position of
    the bottommost edge.
bounds.aspect (read-only)
    The width-to-height aspect ratio of the rectangle.  Note that this is often
    in render target coordinates which don’t necessarily have equal X and Y
    scales.  A rectangle representing a square in the final output doesn’t
    necessarily have an aspect ratio of 1.

.. _luareference-render-color:

Render colour
~~~~~~~~~~~~~

Wraps MAME’s ``render_color`` class, which represents an ARGB (alpha, red,
green, blue) format colour.  Channels are floating-point values ranging from
zero (0, transparent alpha or colour off) to one (1, opaque or full colour
intensity).  Colour channel values are not pre-multiplied by the alpha channel
value.

Instantiation
^^^^^^^^^^^^^

emu.render_color()
    Creates a render colour object representing opaque white (all channels set
    to 1).  This is the identity value – ARGB multiplication by this value will
    not change a colour.
emu.render_color(a, r, g, b)
    Creates a render colour object with the specified alpha, red, green and
    blue channel values.

    The arguments must all be floating-point numbers in the range from zero (0)
    to one (1), inclusive.

Methods
^^^^^^^

color:set(a, r, g, b)
    Sets the colour object’s alpha, red, green and blue channel values.

    The arguments must all be floating-point numbers in the range from zero (0)
    to one (1), inclusive.

Properties
^^^^^^^^^^

color.a (read/write)
    Alpha value, in the range of zero (0, transparent) to one (1, opaque).
color.r (read/write)
    Red channel value, in the range of zero (0, off) to one (1, full intensity).
color.g (read/write)
    Green channel value, in the range of zero (0, off) to one (1, full
    intensity).
color.b (read/write)
    Blue channel value, in the range of zero (0, off) to one (1, full
    intensity).

.. _luareference-render-manager:

Render manager
~~~~~~~~~~~~~~

Wraps MAME’s ``render_manager`` class, responsible for managing render targets
and textures.

Instantiation
^^^^^^^^^^^^^

manager:machine():render()
    Gets the global render manager instance for the emulation session.

Properties
^^^^^^^^^^

render.max_update_rate (read-only)
    The maximum update rate in Hertz.  This is a floating-point number.
render.ui_target (read-only)
    The :ref:`render target <luareference-render-target>` used to draw the user
    interface (including menus, sliders and pop-up messages).  This is usually
    the first host window or screen.
render.ui_container (read-only)
    The :ref:`render container <luareference-render-container>` used for drawing
    the user interface.
render.targets[] (read-only)
    The list of render targets, including output windows and screens, as well as
    hidden render targets used for things like rendering screenshots.  Uses
    1-based integer indices.  The index operator and the ``at`` method have O(n)
    complexity.

.. _luareference-render-target:

Render target
~~~~~~~~~~~~~

Wrap’s MAME’s ``render_target`` class, which represents a video output channel.
This could be a host window or screen, or a hidden target used for rendering
screenshots.

Instantiation
^^^^^^^^^^^^^

manager:machine():render().targets[index]
    Get a render target by index.
manager:machine():render():ui_target()
    Get the render target used to display the user interface (including menus,
    sliders and pop-up messages).  This is usually the first host window or
    screen.

Properties
^^^^^^^^^^

target.index (read-only)
    The 1-based index of the render target.  This has O(n) complexity.
target.width (read-only)
    The width of the render target in output pixels.  This is an integer.
target.height (read-only)
    The height of the render target in output pixels.  This is an integer.
target.pixel_aspect (read-only)
    The width-to-height aspect ratio of the render target’s pixels.  This is a
    floating-point number.
target.hidden (read-only)
    A Boolean indicating whether this is an internal render target that is not
    displayed to the user directly (e.g. the render target used to draw
    screenshots).
target.is_ui_target (read-only)
    A Boolean indicating whether this is the render target used to display the
    user interface.
target.max_update_rate (read/write)
    The maximum update rate for the render target in Hertz.
target.orientation (read/write)
    The target orientation flags.  This is an integer bit mask, where bit 0
    (0x01) is set to mirror horizontally, bit 1 (0x02) is set to mirror
    vertically, and bit 2 (0x04) is set to mirror along the top left-bottom
    right diagonal.
target.view_names[]
    The names of the available views for this render target.  Uses 1-based
    integer indices.  The ``find`` and ``index_of`` methods have O(n)
    complexity; all other supported operations have O(1) complexity.
target.current_view (read-only)
    The currently selected view for the render target.  This is a
    :ref:`layout view <luareference-render-layview>` object.
target.view_index (read/write)
    The 1-based index of the selected view for this render target.
target.visibility_mask (read-only)
    An integer bit mask indicating which item collections are currently visible
    for the current view.
target.screen_overlay (read/write)
    A Boolean indicating whether screen overlays are enabled.
target.zoom_to_screen (read/write)
    A Boolean indicating whether the render target is configured to scale so
    that the emulated screen(s) fill as much of the output window/screen as
    possible.

.. _luareference-render-container:

Render container
~~~~~~~~~~~~~~~~

Wraps MAME’s ``render_container`` class.

Instantiation
^^^^^^^^^^^^^

manager:machine():render().ui_container
    Gets the render container used to draw the user interface, including menus,
    sliders and pop-up messages.
manager:machine().screens[tag].container
    Gets the render container used to draw a given screen.

Properties
^^^^^^^^^^

container.user_settings (read/write)
    The container’s :ref:`user settings <luareference-render-contsettings>`.
    This can be used to control a number of image adjustments.
container.orientation (read/write)
    The container orientation flags.  This is an integer bit mask, where bit 0
    (0x01) is set to mirror horizontally, bit 1 (0x02) is set to mirror
    vertically, and bit 2 (0x04) is set to mirror along the top left-bottom
    right diagonal.
container.xscale (read/write)
    The container’s X scale factor.  This is a floating-point number.
container.yscale (read/write)
    The container’s Y scale factor.  This is a floating-point number.
container.xoffset (read/write)
    The container’s X offset.  This is a floating-point number where one (1)
    corresponds to the X size of the container.
container.yoffset (read/write)
    The container’s Y offset.  This is a floating-point number where one (1)
    corresponds to the Y size of the container.
container.is_empty (read-only)
    A Boolean indicating whether the container has no items.

.. _luareference-render-contsettings:

Container user settings
~~~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``render_container::user_settings`` class, representing image
adjustments applied to a
:ref:`render container <luareference-render-container>`.

Instantiation
^^^^^^^^^^^^^

manager:machine().screens[tag].container
    Gets the current container user settings for a given screen.

Properties
^^^^^^^^^^

settings.orientation (read/write)
    The container orientation flags.  This is an integer bit mask, where bit 0
    (0x01) is set to mirror horizontally, bit 1 (0x02) is set to mirror
    vertically, and bit 2 (0x04) is set to mirror along the top left-bottom
    right diagonal.
settings.brightness (read/write)
    The brightness adjustment applied to the container.  This is a
    floating-point number.
settings.contrast (read/write)
    The contrast adjustment applied to the container.  This is a floating-point
    number.
settings.gamma (read/write)
    The gamma adjustment applied to the container.  This is a floating-point
    number.
settings.xscale (read/write)
    The container’s X scale factor.  This is a floating-point number.
settings.yscale (read/write)
    The container’s Y scale factor.  This is a floating-point number.
settings.xoffset (read/write)
    The container’s X offset.  This is a floating-point number where one (1)
    represents the X size of the container.
settings.yoffset (read/write)
    The container’s Y offset.  This is a floating-point number where one (1)
    represents the Y size of the container.

.. _luareference-render-layfile:

Layout file
~~~~~~~~~~~

Wraps MAME’s ``layout_file`` class, representing the views loaded from a layout
file for use by a render target.

Instantiation
^^^^^^^^^^^^^

A layout file object is supplied to its layout script in the ``file`` variable.
Layout file objects are not instantiated directly from Lua scripts.

Methods
^^^^^^^

layout:set_resolve_tags_callback(cb)
    Set a function to perform additional tasks after the emulated machine has
    finished starting, tags in the layout views have been resolved, and the
    default view item handlers have been set up.  The function must accept no
    arguments.

    Call with ``nil`` to remove the callback.

Properties
^^^^^^^^^^

layout.device (read-only)
    The device that caused the layout file to be loaded.  Usually the root
    machine device for external layouts.
layout.views[] (read-only)
    The :ref:`views <luareference-render-layview>` created from the layout file.
    Views are indexed by unqualified name (i.e. the value of the ``name``
    attribute).  Views are ordered how they appear in the layout file when
    iterating or using the ``at`` method.  The index get, ``at`` and
    ``index_of`` methods have O(n) complexity.

    Note that not all views in the XML file may be created.  For example views
    that reference screens provided by slot card devices will not be created if
    said slot card devices are not present in the system.

.. _luareference-render-layview:

Layout view
~~~~~~~~~~~

Wraps MAME’s ``layout_view`` class, representing a view that can be displayed in
a render target.  Views are created from XML layout files, which may be loaded
from external artwork, internal to MAME, or automatically generated based on the
screens in the emulated system.

Instantiation
^^^^^^^^^^^^^

Layout scripts generally

manager:machine():render().targets[index].current_view
    Gets the currently selected view for a given render target.

Methods
^^^^^^^

view:has_screen(screen)
    Returns a Boolean indicating whether the screen is present in the view.
    This is true for screens that are present but not visible because the user
    has hidden the item collection they belong to.
view:set_prepare_items_callback(cb)
    Set a function to perform additional tasks before the view items are added
    to the render target in preparation for drawing a video frame.  The function
    must accept no arguments.  Call with ``nil`` to remove the callback.
view:set_preload_callback(cb)
    Set a function to perform additional tasks after preloading visible view
    items.  The function must accept no arguments.  Call with ``nil`` to remove
    the callback.

    This function may be called when the user selects a view or makes an item
    collection visible.  It may be called multiple times for a view, so avoid
    repeating expensive tasks.
view:set_recomputed_callback(cb)
    Set a function to perform additional tasks after the view’s dimensions are
    recomputed.  The function must accept no arguments.  Call with ``nil`` to
    remove the callback.

    View coordinates are recomputed in various events, including the window
    being resized, entering or leaving full-screen mode, and changing the zoom
    to screen area setting.

Properties
^^^^^^^^^^

view.items[] (read-only)
    The screen and layout element :ref:`items <luareference-render-layitem>` in
    the view.  This container does not support iteration by key using ``pairs``;
    only iteration by index using ``ipairs`` is supported.  The key is the value
    of the ``id`` attribute if present.  Only items with ``id`` attributes can
    be looked up by key.  The index get method has O(1) complexity, and the
    ``at`` and ``index_of`` methods have O(n) complexity.
view.name (read-only)
    The display name for the view.  This may be qualified to indicate the device
    that caused the layout file to be loaded when it isn’t the root machine
    device.
view.unqualified_name (read-only)
    The unqualified name of the view, exactly as it appears in the ``name``
    attribute in the XML layout file.
view.visible_screen_count (read-only)
    The number of screens items currently enabled in the view.
view.effective_aspect (read-only)
    The effective width-to-height aspect ratio of the view in its current
    configuration.
view.bounds (read-only)
    A :ref:`render bounds <luareference-render-bounds>` object representing the
    effective bounds of the view in its current configuration.  The coordinates
    are in view units, which are arbitrary but assumed to have square aspect
    ratio.
view.has_art
    A Boolean indicating whether the view has any non-screen items, including
    items that are not visible because the user has hidden the item collection
    that they belong to.

.. _luareference-render-layitem:

Layout view item
~~~~~~~~~~~~~~~~

Wraps MAME’s ``layout_view::item`` class, representing an item in a view.  An
item is drawn as a rectangular textured surface.  The texture is supplied by an
emulated screen or a layout element.

Instantiation
^^^^^^^^^^^^^

layout.views[name].items[id]
    Get a view item by ID.  The item must have an ``id`` attribute in the XML
    layout file to be looked up by ID.

Methods
^^^^^^^

item:set_state(state)
    Set the value used as the element state and animation state in the absence
    of bindings.  The argument must be an integer.
item.set_element_state_callback(cb)
    Set a function to call to obtain the element state for the item.  The
    function must accept no arguments and return an integer.  Call with ``nil``
    to restore the default element state callback (based on bindings in the XML
    layout file).

    Note that the function must not access the item’s ``element_state``
    property, as this will result in infinite recursion.

    This callback will not be used to obtain the animation state for the item,
    even if the item lacks explicit animation state bindings in the XML layout
    file.
item.set_animation_state_callback(cb)
    Set a function to call to obtain the animation state for the item.  The
    function must accept no arguments and return an integer.  Call with ``nil``
    to restore the default animation state callback (based on bindings in the
    XML layout file).

    Note that the function must not access the item’s ``animation_state``
    property, as this will result in infinite recursion.
item.set_bounds_callback(cb)
    Set a function to call to obtain the bounds for the item.  The function must
    accept no arguments and return a
    :ref:`render bounds <luareference-render-bounds>` object in render target
    coordinates.  Call with ``nil`` to restore the default bounds callback
    (based on the item’s animation state and ``bounds`` child elements in the
    XML layout file).

    Note that the function must not access the item’s ``bounds`` property, as
    this will result in infinite recursion.
item.set_color_callback(cb)
    Set a function to call to obtain the multiplier colour for the item.  The
    function must accept no arguments and return a
    :ref:`render colour <luareference-render-color>` object.  Call with ``nil``
    to restore the default colour callback (based on the item’s animation state
    and ``color`` child elements in the XML layout file).

    Note that the function must not access the item’s ``color`` property, as
    this will result in infinite recursion.

Properties
^^^^^^^^^^

item.id (read-only)
    Get the optional item identifier.  This is the value of the ``id`` attribute
    in the XML layout file if present, or ``nil``.
item.bounds_animated (read-only)
    A Boolean indicating whether the item’s bounds depend on its animation
    state.
item.color_animated (read-only)
    A Boolean indicating whether the item’s colour depends on its animation
    state.
item.bounds (read-only)
    The item’s bounds for the current state.  This is a
    :ref:`render bounds <luareference-render-bounds>` object in render target
    coordinates.
item.color (read-only)
    The item’s colour for the current state.  The colour of the screen or
    element texture is multiplied by this colour.  This is a 
    :ref:`render colour <luareference-render-color>` object.
item.blend_mode (read-only)
    Get the item’s blend mode.  This is an integer value, where 0 means no
    blending, 1 means alpha blending, 2 means RGB multiplication, 3 means
    additive blending, and -1 allows the items within a container to specify
    their own blending modes.
item.orientation (read-only)
    Get the item orientation flags.  This is an integer bit mask, where bit 0
    (0x01) is set to mirror horizontally, bit 1 (0x02) is set to mirror
    vertically, and bit 2 (0x04) is set to mirror along the top left-bottom
    right diagonal.
item.element_state (read-only)
    Get the current element state.  This will call the element state callback
    function to handle bindings.
item.animation_state (read-only)
    Get the current animation state.  This will call the animation state
    callback function to handle bindings.


.. _luareference-debug:

Debugger
--------

Some of MAME’s core debugging features can be controlled from Lua script.  The
debugger must be enabled to use the debugging features (usually by passing
``-debug`` on the command line).

.. _luareference-debug-manager:

Debugger manager
~~~~~~~~~~~~~~~~

Wraps MAME’s ``debugger_manager`` class, providing the main interface to control
the debugger.

Instantiation
^^^^^^^^^^^^^

manager:machine():debugger()
    Returns the global debugger manager instance, or ``nil`` if the debugger is
    not enabled.

Methods
^^^^^^^

debugger:command(str)
    Execute a debugger console command.  The argument is the command string.
    The output is sent to both the debugger console and the Lua console.

Properties
^^^^^^^^^^

debugger.consolelog[] (read-only)
    The lines in the console log (output from debugger commands).  This
    container only supports index and length operations.
debugger.errorlog[] (read-only)
    The lines in the error log (``logerror`` output).  This container only
    supports index and length operations.
debugger.visible_cpu (read/write)
    The CPU device with debugger focus.  Changes become visible in the debugger
    console after the next step.  Setting to a device that is not a CPU has no
    effect.
debugger.execution_state (read/write)
    Either ``"run"`` if the emulated system is running, or ``"stop"`` if it is
    stopped in the debugger.

.. _luareference-debug-devdebug:

Device debugger interface
~~~~~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``device_debug`` class, providing the debugger interface to an
emulated CPU device.

Instantiation
^^^^^^^^^^^^^

manager:machine().devices[tag]:debug()
    Returns the debugger interface for an emulated CPU device, or ``nil`` if the
    device is not a CPU.

Methods
^^^^^^^

debug:step([cnt])
    Step by the specified number of instructions.  If the instruction count is
    not provided, it defaults to a single instruction.
debug:go()
    Run the emulated CPU.
debug:bpset(addr, [cond], [act])
    Set a breakpoint at the specified address, with an optional condition and
    action.  If the action is not specified, it defaults to just breaking into
    the debugger.  Returns the breakpoint number for the new breakpoint.

    If specified, the condition must be a debugger expression that will be
    evaluated each time the breakpoint is hit.  Execution will only be stopped
    if the expression evaluates to a non-zero value.  If the condition is not
    specified, it defaults to always active.
debug:bpenable([bp])
    Enable the specified breakpoint, or all breakpoints for the device if no
    breakpoint number is specified.  Returns whether the specified number
    matched a breakpoint if a breakpoint number is specified, or ``nil`` if no
    breakpoint number is specified.
debug:bpdisable([bp])
    Disable the specified breakpoint, or all breakpoints for the device if no
    breakpoint number is specified.  Returns whether the specified number
    matched a breakpoint if a breakpoint number is specified, or ``nil`` if no
    breakpoint number is specified.
debug:bpclear([bp])
    Clear the specified breakpoint, or all breakpoints for the device if no
    breakpoint number is specified.  Returns whether the specified number
    matched a breakpoint if a breakpoint number is specified, or ``nil`` if no
    breakpoint number is specified.
debug:bplist()
    Returns a table of breakpoints for the device.  The keys are the breakpoint
    numbers, and the values are
    :ref:`breakpoint objects <luareference-debug-breakpoint>`.
debug:wpset(space, type, addr, len, [cond], [act])
    Set a watchpoint over the specified address range, with an optional
    condition and action.  The type must be ``"r"``, ``"w"`` or ``"rw"`` for a
    read, write or read/write breakpoint.  If the action is not specified, it
    defaults to just breaking into the debugger.  Returns the watchpoint number
    for the new watchpoint.

    If specified, the condition must be a debugger expression that will be
    evaluated each time the breakpoint is hit.  Execution will only be stopped
    if the expression evaluates to a non-zero value.  For all watchpoints, a
    ``wpaddr`` variable is set to the address that triggered the watchpoint.
    When a watchpoint is triggered by a write, a ``wpdata`` variable is set to
    the data being written.  If the condition is not specified, it defaults to
    always active.
debug:wpenable([wp])
    Enable the specified watchpoint, or all watchpoints for the device if no
    watchpoint number is specified.  Returns whether the specified number
    matched a watchpoint if a watchpoint number is specified, or ``nil`` if no
    watchpoint number is specified.
debug:wpdisable([wp])
    Disable the specified watchpoint, or all watchpoints for the device if no
    watchpoint number is specified.  Returns whether the specified number
    matched a watchpoint if a watchpoint number is specified, or ``nil`` if no
    watchpoint number is specified.
debug:wpclear([wp])
    Clear the specified watchpoint, or all watchpoints for the device if no
    watchpoint number is specified.  Returns whether the specified number
    matched a watchpoint if a watchpoint number is specified, or ``nil`` if no
    watchpoint number is specified.
debug:wplist(space)
    Returns a table of watchpoints for the specified address space of the
    device.  The keys are the watchpoint numbers, and the values are
    :ref:`watchpoint objects <luareference-debug-watchpoint>`.

.. _luareference-debug-breakpoint:

Breakpoint
~~~~~~~~~~

Wraps MAME’s ``debug_breakpoint`` class, representing a breakpoint for an
emulated CPU device.

Instantiation
^^^^^^^^^^^^^

manager:machine().devices[tag]:debug():bplist()[bp]
    Gets the specified breakpoint for an emulated CPU device, or ``nil`` if no
    breakpoint corresponds to the specified index.

Properties
^^^^^^^^^^

breakpoint.index (read-only)
    The breakpoint’s index.  The can be used to enable, disable or clear the
    breakpoint via the
    :ref:`CPU debugger interface <luareference-debug-devdebug>`.
breakpoint.enabled (read-only)
    A Boolean indicating whether the breakpoint is currently enabled.
breakpoint.address (read-only)
    The breakpoint’s address.
breakpoint.condition (read-only)
    A debugger expression evaluated each time the breakpoint is hit.  The action
    will only be triggered if this expression evaluates to a non-zero value.  An
    empty string if no condition was specified.
breakpoint.action (read-only)
    An action the debugger will run when the breakpoint is hit and the condition
    evaluates to a non-zero value.  An empty string if no action was specified.

.. _luareference-debug-watchpoint:

Watchpoint
~~~~~~~~~~

Wraps MAME’s ``debug_watchpoint`` class, representing a watchpoint for an
emulated CPU device.

Instantiation
^^^^^^^^^^^^^

manager:machine().devices[tag]:debug():wplist(space)[wp]
    Gets the specified watchpoint for an address space of an emulated CPU
    device, or ``nil`` if no watchpoint in the address space corresponds to the
    specified index.

Properties
^^^^^^^^^^

watchpoint.index (read-only)
    The watchpoint’s index.  The can be used to enable, disable or clear the
    watchpoint via the
    :ref:`CPU debugger interface <luareference-debug-devdebug>`.
watchpoint.enabled (read-only)
    A Boolean indicating whether the watchpoint is currently enabled.
watchpoint.type (read-only)
    Either ``"r"``, ``"w"`` or ``"rw"`` for a read, write or read/write
    watchpoint.
watchpoint.address (read-only)
    The starting address of the watchpoint’s address range.
watchpoint.length (read-only)
    The length of the watchpoint’s address range.
watchpoint.condition (read-only)
    A debugger expression evaluated each time the watchpoint is hit.  The action
    will only be triggered if this expression evaluates to a non-zero value.  An
    empty string if no condition was specified.
watchpoint.action (read-only)
    An action the debugger will run when the watchpoint is hit and the condition
    evaluates to a non-zero value.  An empty string if no action was specified.
