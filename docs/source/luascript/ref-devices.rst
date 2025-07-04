.. _luascript-ref-dev:

Lua Device Classes
==================

Several device classes and device mix-ins classes are exposed to Lua.  Devices
can be looked up by tag or enumerated.

.. contents::
    :local:
    :depth: 1


.. _luascript-ref-devenum:

Device enumerators
------------------

Device enumerators are special containers that allow iterating over devices and
looking up devices by tag.  A device enumerator can be created to find any kind
of device, to find devices of a particular type, or to find devices that
implement a particular interface.  When iterating using ``pairs`` or ``ipairs``,
devices are returned by walking the device tree depth-first in creation order.

The index get operator looks up a device by tag.  It returns ``nil`` if no
device with the specified tag is found, or if the device with the specified tag
does not meet the type/interface requirements of the device enumerator.  The
complexity is O(1) if the result is cached, but an uncached device lookup is
expensive.  The ``at`` method has O(n) complexity.

If you create a device enumerator with a starting point other than the root
machine device, passing an absolute tag or a tag containing parent references to
the index operator may return a device that would not be discovered by
iteration.  If you create a device enumerator with restricted depth, devices
that would not be found due to being too deep in the hierarchy can still be
looked up by tag.

Creating a device enumerator with depth restricted to zero can be used to
downcast a device or test whether a device implements a certain interface.  For
example this will test whether a device implements the media image interface:

.. code-block:: Lua

    image_intf = emu.image_enumerator(device, 0):at(1)
    if image_intf then
        print(string.format("Device %s mounts images", device.tag))
    end

Instantiation
~~~~~~~~~~~~~

manager.machine.devices
    Returns a device enumerator that will iterate over
    :ref:`devices <luascript-ref-device>` in the system.
manager.machine.palettes
    Returns a device enumerator that will iterate over
    :ref:`palette devices <luascript-ref-dipalette>` in the system.
manager.machine.screens
    Returns a device enumerator that will iterate over
    :ref:`screen devices <luascript-ref-screendev>` in the system.
manager.machine.cassettes
    Returns a device enumerator that will iterate over
    :ref:`cassette image devices <luascript-ref-cassdev>` in the system.
manager.machine.images
    Returns a device enumerator that will iterate over
    :ref:`media image devices <luascript-ref-diimage>` in the system.
manager.machine.slots
    Returns a device enumerator that will iterate over
    :ref:`slot devices <luascript-ref-dislot>` in the system.
manager.machine.sounds
    Returns a device enumerator that will iterate over
    :ref:`sound devices <luascript-ref-disound>` in the system.
emu.device_enumerator(device, [depth])
    Returns a device enumerator that will iterate over
    :ref:`devices <luascript-ref-device>` in the sub-tree starting at the
    specified device.  The specified device will be included.  If the depth is
    provided, it must be an integer specifying the maximum number of levels to
    iterate below the specified device (i.e. 1 will limit iteration to the
    device and its immediate children).
emu.palette_enumerator(device, [depth])
    Returns a device enumerator that will iterate over
    :ref:`palette devices <luascript-ref-dipalette>` in the sub-tree starting at
    the specified device.  The specified device will be included if it is a
    palette device.  If the depth is provided, it must be an integer specifying
    the maximum number of levels to iterate below the specified device (i.e. 1
    will limit iteration to the device and its immediate children).
emu.screen_enumerator(device, [depth])
    Returns a device enumerator that will iterate over
    :ref:`screen devices <luascript-ref-screendev>` in the sub-tree starting at
    the specified device.  The specified device will be included if it is a
    screen device.  If the depth is provided, it must be an integer specifying
    the maximum number of levels to iterate below the specified device (i.e. 1
    will limit iteration to the device and its immediate children).
emu.cassette_enumerator(device, [depth])
    Returns a device enumerator that will iterate over
    :ref:`cassette image devices <luascript-ref-cassdev>` in the sub-tree
    starting at the specified device.  The specified device will be included if
    it is a cassette image device.  If the depth is provided, it must be an
    integer specifying the maximum number of levels to iterate below the
    specified device (i.e. 1 will limit iteration to the device and its
    immediate children).
emu.image_enumerator(device, [depth])
    Returns a device enumerator that will iterate over
    :ref:`media image devices <luascript-ref-diimage>` in the sub-tree starting
    at the specified device.  The specified device will be included if it is a
    media image device.  If the depth is provided, it must be an integer
    specifying the maximum number of levels to iterate below the specified
    device (i.e. 1 will limit iteration to the device and its immediate
    children).
emu.slot_enumerator(device, [depth])
    Returns a device enumerator that will iterate over
    :ref:`slot devices <luascript-ref-dislot>` in the sub-tree starting at the
    specified device.  The specified device will be included if it is a slot
    device.  If the depth is provided, it must be an integer specifying the
    maximum number of levels to iterate below the specified device (i.e. 1 will
    limit iteration to the device and its immediate children).


.. _luascript-ref-device:

Device
------

Wraps MAME’s ``device_t`` class, which is a base of all device classes.

Instantiation
~~~~~~~~~~~~~

manager.machine.devices[tag]
    Gets a device by tag relative to the root machine device, or ``nil`` if no
    such device exists.
manager.machine.devices[tag]:subdevice(tag)
    Gets a device by tag relative to another arbitrary device, or ``nil`` if no
    such device exists.

Methods
~~~~~~~

device:subtag(tag)
    Converts a tag relative to the device to an absolute tag.
device:siblingtag(tag)
    Converts a tag relative to the device’s parent device to an absolute tag.
device:memshare(tag)
    Gets a :ref:`memory share <luascript-ref-memshare>` by tag relative to the
    device, or ``nil`` if no such memory share exists.
device:membank(tag)
    Gets a :ref:`memory bank <luascript-ref-membank>` by tag relative to the
    device, or ``nil`` if no such memory bank exists.
device:memregion(tag)
    Gets a :ref:`memory region <luascript-ref-memregion>` by tag relative to the
    device, or ``nil`` if no such memory region exists.
device:ioport(tag)
    Gets an :ref:`I/O port <luascript-ref-ioport>` by tag relative to the
    device, or ``nil`` if no such I/O port exists.
device:subdevice(tag)
    Gets a device by tag relative to the device.
device:siblingdevice(tag)
    Gets a device by tag relative to the device’s parent.
device:parameter(tag)
    Gets a parameter value by tag relative to the device, or an empty string if
    the parameter is not set.

Properties
~~~~~~~~~~

device.tag (read-only)
    The device’s absolute tag in canonical form.
device.basetag (read-only)
    The last component of the device’s tag (i.e. its tag relative to its
    immediate parent), or ``"root"`` for the root machine device.
device.name (read-only)
    The full display name for the device’s type.
device.shortname (read-only)
    The short name of the devices type (this is used, e.g. on the command line,
    when looking for resource like ROMs or artwork, and in various data files).
device.owner (read-only)
    The device’s immediate parent in the device tree, or ``nil`` for the root
    machine device.
device.configured (read-only)
    A Boolean indicating whether the device has completed configuration.
device.started (read-only)
    A Boolean indicating whether the device has completed starting.
device.debug (read-only)
    The :ref:`debugger interface <luascript-ref-devdebug>` to the device if it
    is a CPU device, or ``nil`` if it is not a CPU device or the debugger is not
    enabled.
device.state[] (read-only)
    The :ref:`state entries <luascript-ref-distateentry>` for devices that
    expose the register state interface, indexed by symbol, or ``nil`` for other
    devices.  The index operator and ``index_of`` methods have O(n) complexity;
    all other supported operations have O(1) complexity.
device.spaces[] (read-only)
    A table of the device’s :ref:`address spaces <luascript-ref-addrspace>`,
    indexed by name.  Only valid for devices that implement the memory
    interface.  Note that the names are specific to the device type and have no
    special significance.


.. _luascript-ref-dipalette:

Palette device
--------------

Wraps MAME’s ``device_palette_interface`` class, which represents a device that
translates pen values to colours.

Colours are represented in alpha/red/green/blue (ARGB) format.  Channel values
range from 0 (transparent or off) to 255 (opaque or full intensity), inclusive.
Colour channel values are not pre-multiplied by the alpha value.  Channel values
are packed into the bytes of 32-bit unsigned integers, in the order alpha, red,
green, blue from most-significant to least-significant byte.

Instantiation
~~~~~~~~~~~~~

manager.machine.palettes[tag]
    Gets a palette device by tag relative to the root machine device, or ``nil``
    if no such device exists or it is not a palette device.

Methods
~~~~~~~

palette:pen(index)
    Gets the remapped pen number for the specified palette index.
palette:pen_color(pen)
    Gets the colour for the specified pen number.
palette:pen_contrast(pen)
    Gets the contrast value for the specified pen number.  The contrast is a
    floating-point number.
palette:pen_indirect(index)
    Gets the indirect pen index for the specified palette index.
palette:indirect_color(index)
    Gets the indirect pen colour for the specified palette index.
palette:set_pen_color(pen, color)
    Sets the colour for the specified pen number.  The colour may be specified
    as a single packed 32-bit value; or as individual red, green and blue
    channel values, in that order.
palette:set_pen_red_level(pen, level)
    Sets the red channel value of the colour for the specified pen number.
    Other channel values are not affected.
palette:set_pen_green_level(pen, level)
    Sets the green channel value of the colour for the specified pen number.
    Other channel values are not affected.
palette:set_pen_blue_level(pen, level)
    Sets the blue channel value of the colour for the specified pen number.
    Other channel values are not affected.
palette:set_pen_contrast(pen, factor)
    Sets the contrast value for the specified pen number.  The value must be a
    floating-point number.
palette:set_pen_indirect(pen, index)
    Sets the indirect pen index for the specified pen number.
palette:set_indirect_color(index, color)
    Sets the indirect pen colour for the specified palette index.  The colour
    may be specified as a single packed 32-bit value; or as individual red,
    green and blue channel values, in that order.
palette:set_shadow_factor(factor)
    Sets the contrast value for the current shadow group.  The value must be a
    floating-point number.
palette:set_highlight_factor(factor)
    Sets the contrast value for the current highlight group.  The value must be
    a floating-point number.
palette:set_shadow_mode(mode)
    Sets the shadow mode.  The value is the index of the desired shadow table.

Properties
~~~~~~~~~~

palette.palette (read-only)
    The underlying :ref:`palette <luascript-ref-palette>` managed by the
    device.
palette.entries (read-only)
    The number of colour entries in the palette.
palette.indirect_entries (read-only)
    The number of indirect pen entries in the palette.
palette.black_pen (read-only)
    The index of the fixed black pen entry.
palette.white_pen (read-only)
    The index of the fixed white pen.
palette.shadows_enabled (read-only)
    A Boolean indicating whether shadow colours are enabled.
palette.highlights_enabled (read-only)
    A Boolean indicating whether highlight colours are enabled.
palette.device (read-only)
    The underlying :ref:`device <luascript-ref-device>`.


.. _luascript-ref-screendev:

Screen device
-------------

Wraps MAME’s ``screen_device`` class, which represents an emulated video output.

Instantiation
~~~~~~~~~~~~~

manager.machine.screens[tag]
    Gets a screen device by tag relative to the root machine device, or ``nil``
    if no such device exists or it is not a screen device.

Base classes
~~~~~~~~~~~~

* :ref:`luascript-ref-device`

Methods
~~~~~~~

screen:orientation()
    Returns the rotation angle in degrees (will be one of 0, 90, 180 or 270),
    whether the screen is flipped left-to-right, and whether the screen is
    flipped top-to-bottom.  This is the final screen orientation after the
    screen orientation specified in the machine configuration and the rotation
    for the system driver are applied.
screen:time_until_pos(v, [h])
    Gets the time remaining until the raster reaches the specified position.  If
    the horizontal component of the position is not specified, it defaults to
    zero (0, i.e. the beginning of the line).  The result is a floating-point
    number in units of seconds.
screen:time_until_vblank_start()
    Gets the time remaining until the start of the vertical blanking interval.
    The result is a floating-point number in units of seconds.
screen:time_until_vblank_end()
    Gets the time remaining until the end of the vertical blanking interval.
    The result is a floating-point number in units of seconds.
screen:snapshot([filename])
    Saves a screen snapshot in PNG format.  If no filename is supplied, the
    configured snapshot path and name format will be used.  If the supplied
    filename is not an absolute path, it is interpreted relative to the first
    configured snapshot path.  The filename may contain conversion specifiers
    that will be replaced by the system name or an incrementing number.

    Returns a file error if opening the snapshot file failed, or ``nil``
    otherwise.
screen:pixel(x, y)
    Gets the pixel at the specified location.  Coordinates are in pixels, with
    the origin at the top left corner of the visible area, increasing to the
    right and down.  Returns either a palette index or a colour in RGB format
    packed into a 32-bit integer.  Returns zero (0) if the specified point is
    outside the visible area.
screen:pixels()
    Returns all visible pixels, the visible area width and visible area height.

    Pixels are returned as 32-bit integers packed into a binary string in host
    Endian order.  Pixels are organised in row-major order, from left to right
    then top to bottom.  Pixels values are either palette indices or colours in
    RGB format packed into 32-bit integers.
screen:draw_box(left, top, right, bottom, [line], [fill])
    Draws an outlined rectangle with edges at the specified positions.

    Coordinates are floating-point numbers in units of emulated screen pixels,
    with the origin at (0, 0).  Note that emulated screen pixels often aren’t
    square.  The coordinate system is rotated if the screen is rotated, which is
    usually the case for vertical-format screens.  Before rotation, the origin
    is at the top left, and coordinates increase to the right and downwards.
    Coordinates are limited to the screen area.

    The fill and line colours are in alpha/red/green/blue (ARGB) format.
    Channel values are in the range 0 (transparent or off) to 255 (opaque or
    full intensity), inclusive.  Colour channel values are not pre-multiplied by
    the alpha value.  The channel values must be packed into the bytes of a
    32-bit unsigned integer, in the order alpha, red, green, blue from
    most-significant to least-significant byte.  If the line colour is not
    provided, the UI text colour is used; if the fill colour is not provided,
    the UI background colour is used.
screen:draw_line(x0, y0, x1, y1, [color])
    Draws a line from (x0, y0) to (x1, y1).

    Coordinates are floating-point numbers in units of emulated screen pixels,
    with the origin at (0, 0).  Note that emulated screen pixels often aren’t
    square.  The coordinate system is rotated if the screen is rotated, which is
    usually the case for vertical-format screens.  Before rotation, the origin
    is at the top left, and coordinates increase to the right and downwards.
    Coordinates are limited to the screen area.

    The line colour is in alpha/red/green/blue (ARGB) format.  Channel values
    are in the range 0 (transparent or off) to 255 (opaque or full intensity),
    inclusive.  Colour channel values are not pre-multiplied by the alpha value.
    The channel values must be packed into the bytes of a 32-bit unsigned
    integer, in the order alpha, red, green, blue from most-significant to
    least-significant byte.  If the line colour is not provided, the UI text
    colour is used.
screen:draw_text(x|justify, y, text, [foreground], [background])
    Draws text at the specified position.  If the screen is rotated the text
    will be rotated.

    If the first argument is a number, the text will be left-aligned at this X
    coordinate.  If the first argument is a string, it must be ``"left"``,
    ``"center"`` or ``"right"`` to draw the text left-aligned at the
    left edge of the screen, horizontally centred on the screen, or
    right-aligned at the right edge of the screen, respectively.  The second
    argument specifies the Y coordinate of the maximum ascent of the text.

    Coordinates are floating-point numbers in units of emulated screen pixels,
    with the origin at (0, 0).  Note that emulated screen pixels often aren’t
    square.  The coordinate system is rotated if the screen is rotated, which is
    usually the case for vertical-format screens.  Before rotation, the origin
    is at the top left, and coordinates increase to the right and downwards.
    Coordinates are limited to the screen area.

    The foreground and background colours are in alpha/red/green/blue (ARGB)
    format.  Channel values are in the range 0 (transparent or off) to 255
    (opaque or full intensity), inclusive.  Colour channel values are not
    pre-multiplied by the alpha value.  The channel values must be packed into
    the bytes of a 32-bit unsigned integer, in the order alpha, red, green, blue
    from most-significant to least-significant byte.  If the foreground colour
    is not provided, the UI text colour is used; if the background colour is not
    provided, it is fully transparent.

Properties
~~~~~~~~~~

screen.width (read-only)
    The width of the bitmap produced by the emulated screen in pixels.
screen.height (read-only)
    The height of the bitmap produced by the emulated screen in pixels.
screen.refresh (read-only)
    The screen’s configured refresh rate in Hertz (this may not reflect the
    current value).
screen.refresh_attoseconds (read-only)
    The screen’s configured refresh interval in attoseconds (this may not
    reflect the current value).
screen.xoffset (read-only)
    The screen’s default X position offset.  This is a floating-point number
    where one (1) corresponds to the X size of the screen’s container.  This may
    be useful for restoring the default after adjusting the X offset via the
    screen’s container.
screen.yoffset (read-only)
    The screen’s default Y position offset.  This is a floating-point number
    where one (1) corresponds to the Y size of the screen’s container.  This may
    be useful for restoring the default after adjusting the Y offset via the
    screen’s container.
screen.xscale (read-only)
    The screen’s default X scale factor, as a floating-point number.  This may
    be useful for restoring the default after adjusting the X scale via the
    screen’s container.
screen.yscale (read-only)
    The screen’s default Y scale factor, as a floating-point number.  This may
    be useful for restoring the default after adjusting the Y scale via the
    screen’s container.
screen.pixel_period (read-only)
    The interval taken to draw a horizontal pixel, as a floating-point number in
    units of seconds.
screen.scan_period (read-only)
    The interval taken to draw a scan line (including the horizontal blanking
    interval), as a floating-point number in units of seconds.
screen.frame_period (read-only)
    The interval taken to draw a complete frame (including blanking intervals),
    as a floating-point number in units of seconds.
screen.frame_number (read-only)
    The current frame number for the screen.  This increments monotonically each
    frame interval.
screen.container (read-only)
    The :ref:`render container <luascript-ref-rendercontainer>` used to draw the
    screen.
screen.palette (read-only)
    The :ref:`palette device <luascript-ref-dipalette>` used to translate pixel
    values to colours, or ``nil`` if the screen uses a direct colour pixel
    format.


.. _luascript-ref-cassdev:

Cassette image device
---------------------

Wraps MAME’s ``cassette_image_device`` class, representing a compact cassette
mechanism typically used by a home computer for program storage.

Instantiation
~~~~~~~~~~~~~

manager.machine.cassettes[tag]
    Gets a cassette image device by tag relative to the root machine device, or
    ``nil`` if no such device exists or it is not a cassette image device.

Base classes
~~~~~~~~~~~~

* :ref:`luascript-ref-device`
* :ref:`luascript-ref-diimage`

Methods
~~~~~~~

cassette:stop()
    Disables playback.
cassette:play()
    Enables playback.  The cassette will play if the motor is enabled.
cassette:forward()
    Sets forward play direction.
cassette:reverse()
    Sets reverse play direction.
cassette:seek(time, whence)
    Jump to the specified position on the tape.  The time is a floating-point
    number in units of seconds, relative to the point specified by the whence
    argument.  The whence argument must be one of ``"set"``, ``"cur"`` or
    ``"end"`` to seek relative to the start of the tape, the current position,
    or the end of the tape, respectively.

Properties
~~~~~~~~~~

cassette.is_stopped (read-only)
    A Boolean indicating whether the cassette is stopped (i.e. not recording and
    not playing).
cassette.is_playing (read-only)
    A Boolean indicating whether playback is enabled (i.e. the cassette will
    play if the motor is enabled).
cassette.is_recording (read-only)
    A Boolean indicating whether recording is enabled (i.e. the cassette will
    record if the motor is enabled).
cassette.motor_state (read/write)
    A Boolean indicating whether the cassette motor is enabled.
cassette.speaker_state (read/write)
    A Boolean indicating whether the cassette speaker is enabled.
cassette.position (read-only)
    The current position as a floating-point number in units of seconds relative
    to the start of the tape.
cassette.length (read-only)
    The length of the tape as a floating-point number in units of seconds, or
    zero (0) if no tape image is mounted.


.. _luascript-ref-diimage:

Image device interface
----------------------

Wraps MAME’s ``device_image_interface`` class which is a mix-in implemented by
devices that can load media image files.

Instantiation
~~~~~~~~~~~~~

manager.machine.images[tag]
    Gets an image device by tag relative to the root machine device, or ``nil``
    if no such device exists or it is not a media image device.

Methods
~~~~~~~

image:load(filename)
    Loads the specified file as a media image.  Returns ``nil`` if no error
    or a string describing an error if an error occurred.
image:load_software(name)
    Loads a media image described in a software list.  Returns ``nil`` if no
    error or a string describing an error if an error occurred.
image:unload()
    Unloads the mounted image.
image:create(filename)
    Creates and mounts a media image file with the specified name.  Returns
    ``nil`` if no error or a string describing an error if an error
    occurred.
image:display()
    Returns a “front panel display” string for the device, if supported.  This
    can be used to show status information, like the current head position or
    motor state.
image:add_media_change_notifier(callback)
    Add a callback to receive notifications when a media image is loaded or
    unloaded for the device.  The callback is passed a single string argument
    which will be ``"loaded"`` if a media image has been loaded or
    ``"unloaded"`` if the previously loaded media image has been unloaded.
    Returns a :ref:`notifier subscription <luascript-ref-notifiersub>`.

Properties
~~~~~~~~~~

image.is_readable (read-only)
    A Boolean indicating whether the device supports reading.
image.is_writeable (read-only)
    A Boolean indicating whether the device supports writing.
image.must_be_loaded (read-only)
    A Boolean indicating whether the device requires a media image to be loaded
    in order to start.
image.is_reset_on_load (read-only)
    A Boolean indicating whether the device requires a hard reset to change
    media images (usually for cartridge slots that contain hardware in addition
    to memory chips).
image.image_type_name (read-only)
    A string for categorising the media device.
image.instance_name (read-only)
    The instance name of the device in the current configuration.  This is used
    for setting the media image to load on the command line or in INI files.
    This is not stable, it may have a number appended that may change depending
    on slot configuration.
image.brief_instance_name (read-only)
    The brief instance name of the device in the current configuration.  This is
    used for setting the media image to load on the command line or in INI
    files.  This is not stable, it may have a number appended that may change
    depending on slot configuration.
image.formatlist[] (read-only)
    The :ref:`media image formats <luascript-ref-imagefmt>` supported by the
    device, indexed by name.  The index operator and ``index_of`` methods have
    O(n) complexity; all other supported operations have O(1) complexity.
image.exists (read-only)
    A Boolean indicating whether a media image file is mounted.
image.readonly (read-only)
    A Boolean indicating whether a media image file is mounted in read-only
    mode.
image.filename (read-only)
    The full path to the mounted media image file, or ``nil`` if no media image
    is mounted.
image.crc (read-only)
    The 32-bit cyclic redundancy check of the content of the mounted image file
    if the mounted media image was not loaded from a software list, is mounted
    read-only and is not a CD-ROM, or zero (0) otherwise.
image.loaded_through_softlist (read-only)
    A Boolean indicating whether the mounted media image was loaded from a
    software list, or ``false`` if no media image is mounted.
image.software_list_name (read-only)
    The short name of the software list if the mounted media image was loaded
    from a software list.
image.software_longname (read-only)
    The full name of the software item if the mounted media image was loaded
    from a software list, or ``nil`` otherwise.
image.software_publisher (read-only)
    The publisher of the software item if the mounted media image was loaded
    from a software list, or ``nil`` otherwise.
image.software_year (read-only)
    The release year of the software item if the mounted media image was loaded
    from a software list, or ``nil`` otherwise.
image.software_parent (read-only)
    The short name of the parent software item if the mounted media image was
    loaded from a software list, or ``nil`` otherwise.
image.device (read-only)
    The underlying :ref:`device <luascript-ref-device>`.


.. _luascript-ref-disound:

Sound device interface
----------------------

Wraps MAME’s ``device_sound_interface`` class which is a mix-in implemented by
devices that input and/or output sound.

Instantiation
~~~~~~~~~~~~~

manager.machine.sounds[tag]
    Gets an sound device by tag relative to the root machine device, or ``nil``
    if no such device exists or it is not a slot device.

Properties
~~~~~~~~~~

sound.inputs (read-only)
    Number of sound inputs of the device.

sound.outputs (read-only)
    Number of sound outputs of the device.

sound.microphone (read-only)
    True if the device is a microphone, false otherwise

sound.speaker (read-only)
    True if the device is a speaker, false otherwise

sound.io_positions[] (read-only)
    Non-empty only for microphones and speakers, indicates the positions of
    the inputs or outputs as (x, y, z) coordinates (e.g. [-0.2, 0.0, 1.0])

sound.io_names[] (read-only)
    Non-empty only for microphones and speakers, indicates the positions of
    the inputs or outputs as strings (e.g. Front Left)

sound.hook
    A boolean indicating whether to tap the output samples of this device in
    the global sound hook.

sound.device (read-only)
    The underlying :ref:`device <luascript-ref-device>`.


.. _luascript-ref-dislot:

Slot device interface
---------------------

Wraps MAME’s ``device_slot_interface`` class which is a mix-in implemented by
devices that instantiate a user-specified child device.

Instantiation
~~~~~~~~~~~~~

manager.machine.slots[tag]
    Gets an slot device by tag relative to the root machine device, or ``nil``
    if no such device exists or it is not a slot device.

Properties
~~~~~~~~~~

slot.fixed (read-only)
    A Boolean indicating whether this is a slot with a card specified in machine
    configuration that cannot be changed by the user.
slot.has_selectable_options (read-only)
    A Boolean indicating whether the slot has any user-selectable options (as
    opposed to options that can only be selected programmatically, typically for
    fixed slots or to load media images).
slot.options[] (read-only)
    The :ref:`slot options <luascript-ref-slotopt>` describing the child devices
    that can be instantiated by the slot, indexed by option value.  The ``at``
    and ``index_of`` methods have O(n) complexity; all other supported
    operations have O(1) complexity.
slot.device (read-only)
    The underlying :ref:`device <luascript-ref-device>`.


.. _luascript-ref-distateentry:

Device state entry
------------------

Wraps MAME’s ``device_state_entry`` class, which allows access to named
registers exposed by a :ref:`device <luascript-ref-device>`.  Supports
conversion to string for display.

Instantiation
~~~~~~~~~~~~~

manager.machine.devices[tag].state[symbol]
    Gets a state entry for a given device by symbol.

Properties
~~~~~~~~~~

entry.value (read/write)
    The numeric value of the state entry, as either an integer or floating-point
    number.  Attempting to set the value of a read-only state entry raises an
    error.
entry.symbol (read-only)
    The state entry’s symbolic name.
entry.visible (read-only)
    A Boolean indicating whether the state entry should be displayed in the
    debugger register view.
entry.writeable (read-only)
    A Boolean indicating whether it is possible to modify the state entry’s
    value.
entry.is_float (read-only)
    A Boolean indicating whether the state entry’s value is a floating-point
    number.
entry.datamask (read-only)
    A bit mask of the valid bits of the value for integer state entries.
entry.datasize (read-only)
    The size of the underlying value in bytes for integer state entries.
entry.max_length (read-only)
    The maximum display string length for the state entry.


.. _luascript-ref-imagefmt:

Media image format
------------------

Wraps MAME’s ``image_device_format`` class, which describes a media file format
supported by a :ref:`media image device <luascript-ref-diimage>`.

Instantiation
~~~~~~~~~~~~~

manager.machine.images[tag].formatlist[name]
    Gets a media image format supported by a given device by name.

Properties
~~~~~~~~~~

format.name (read-only)
    An abbreviated name used to identify the format.  This often matches the
    primary filename extension used for the format.
format.description (read-only)
    The full display name of the format.
format.extensions[] (read-only)
    Yields a table of filename extensions used for the format.
format.option_spec (read-only)
    A string describing options available when creating a media image using this
    format.  The string is not intended to be human-readable.


.. _luascript-ref-slotopt:

Slot option
-----------

Wraps MAME’s ``device_slot_interface::slot_option`` class, which represents a
child device that a :ref:`slot device <luascript-ref-dislot>` can be
configured to instantiate.

Instantiation
~~~~~~~~~~~~~

manager.machine.slots[tag].options[name]
    Gets a slot option for a given :ref:`slot device <luascript-ref-dislot>` by
    name (i.e. the value used to select the option).

Properties
~~~~~~~~~~

option.name (read-only)
    The name of the slot option.  This is the value used to select this option
    on the command line or in an INI file.
option.device_fullname (read-only)
    The full display name of the device type instantiated by this option.
option.device_shortname (read-only)
    The short name of the device type instantiated by this option.
option.selectable (read-only)
    A Boolean indicating whether the option may be selected by the user (options
    that are not user-selectable are typically used for fixed slots or to load
    media images).
option.default_bios (read-only)
    The default BIOS setting for the device instantiated using this option, or
    ``nil`` if the default BIOS specified in the device’s ROM definitions will
    be used.
option.clock (read-only)
    The configured clock frequency for the device instantiated using this
    option.  This is an unsigned 32-bit integer.  If the eight most-significant
    bits are all set, it is a ratio of the parent device’s clock frequency, with
    the numerator in bits 12-23 and the denominator in bits 0-11.  If the eight
    most-significant bits are not all set, it is a frequency in Hertz.
