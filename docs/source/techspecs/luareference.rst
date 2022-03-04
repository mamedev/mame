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
    Returns the key for item ``v``, or ``nil`` if it is not in the container.
    The key is what you would pass to the index operator to get the value.
c:index_of(v)
    Returns the 1-based index for item ``v``, or ``nil`` if it is not in the
    container.  The index is what you would pass to the ``at`` method to get the
    value.


.. _luareference-core:

Core classes
------------

Many of MAME’s core classes used to implement an emulation session are available
to Lua scripts.

.. _luareference-core-notifiersub:

Notifier subscription
~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``util::notifier_subscription`` class, which manages a subscription
to a broadcast notification.

Methods
^^^^^^^

subscription:unsubscribe()
    Unsubscribes from notifications.  The subscription will become inactive and
    no future notifications will be received.

Properties
^^^^^^^^^^

subscription.is_active (read-only)
    A Boolean indicating whether the subscription is active.  A subscription
    becomes inactive after explicitly unsubscribing or if the underlying
    notifier is destroyed.

.. _luareference-core-attotime:

Attotime
~~~~~~~~

Wraps MAME’s ``attotime`` class, which represents a high-precision time
interval.  Attotime values support addition and subtraction with other attotime
values, and multiplication and division by integers.

Instantiation
^^^^^^^^^^^^^

emu.attotime()
    Creates an attotime value representing zero (i.e. no elapsed time).
emu.attotime(seconds, attoseconds)
    Creates an attotime with the specified whole and fractional parts.
emu.attotime(attotime)
    Creates a copy of an existing attotime value.
emu.attotime.from_double(seconds)
    Creates an attotime value representing the specified number of seconds.
emu.attotime.from_ticks(periods, frequency)
    Creates an attotime representing the specified number of periods of the
    specified frequency in Hertz.
emu.attotime.from_seconds(seconds)
    Creates an attotime value representing the specified whole number of
    seconds.
emu.attotime.from_msec(milliseconds)
    Creates an attotime value representing the specified whole number of
    milliseconds.
emu.attotime.from_usec(microseconds)
    Creates an attotime value representing the specified whole number of
    microseconds.
emu.attotime.from_nsec(nanoseconds)
    Creates an attotime value representing the specified whole number of
    nanoseconds.

Methods
^^^^^^^

t:as_double()
    Returns the time interval in seconds as a floating-point value.
t:as_hz()
    Interprets the interval as a period and returns the corresponding frequency
    in Hertz as a floating-point value.  Returns zero if ``t.is_never`` is true.
    The interval must not be zero.
t:as_khz()
    Interprets the interval as a period and returns the corresponding frequency
    kilohertz as a floating-point value.  Returns zero if ``t.is_never`` is
    true.  The interval must not be zero.
t:as_mhz()
    Interprets the interval as a period and returns the corresponding frequency
    megahertz as a floating-point value.  Returns zero if ``t.is_never`` is
    true.  The interval must not be zero.
t:as_ticks(frequency)
    Returns the interval as a whole number of periods at the specified
    frequency.  The frequency is specified in Hertz.

Properties
^^^^^^^^^^

t.is_zero (read-only)
    A Boolean indicating whether the value represents no elapsed time.
t.is_never (read-only)
    A Boolean indicating whether the value is greater than the maximum number of
    whole seconds that can be represented (treated as an unreachable time in the
    future or overflow).
t.attoseconds (read-only)
    The fraction seconds portion of the interval in attoseconds.
t.seconds (read-only)
    The number of whole seconds in the interval.
t.msec (read-only)
    The number of whole milliseconds in the fractional seconds portion of the
    interval.
t.usec (read-only)
    The number of whole microseconds in the fractional seconds portion of the
    interval.
t.nsec (read-only)
    The number of whole nanoseconds in the fractional seconds portion of the
    interval.

.. _luareference-core-mameman:

MAME machine manager
~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``mame_machine_manager`` class, which holds the running machine, UI
manager, and other global components.

Instantiation
^^^^^^^^^^^^^

manager
    The MAME machine manager is available as a global variable in the Lua
    environment.

Properties
^^^^^^^^^^

manager.machine (read-only)
    The :ref:`running machine <luareference-core-machine>` for the current
    emulation session.
manager.ui (read-only)
    The :ref:`UI manager <luareference-core-uiman>` for the current session.
manager.options (read-only)
    The :ref:`emulation options <luareference-core-emuopts>` for the current
    session.
manager.plugins[] (read-only)
    Gets information about the :ref:`Lua plugins <luareference-core-plugin>`
    that are present, indexed by name.  The index get, ``at`` and ``index_of``
    methods have O(n) complexity.

.. _luareference-core-machine:

Running machine
~~~~~~~~~~~~~~~

Wraps MAME’s ``running_machine`` class, which represents an emulation session.
It provides access to the other core objects that implement an emulation session
as well as the emulated device tree.

Instantiation
^^^^^^^^^^^^^

manager.machine
    Gets the running machine instance for the current emulation session.

Methods
^^^^^^^

machine:exit()
    Schedules an exit from the current emulation session.  This will either
    return to the system selection menu or exit the application, depending on
    how it was started.  This method returns immediately, before the scheduled
    exit takes place.
machine:hard_reset()
    Schedules a hard reset.  This is implemented by tearing down the emulation
    session and starting another emulation session for the same system.  This
    method returns immediately, before the scheduled reset takes place.
machine:soft_reset()
    Schedules a soft reset.  This is implemented by calling the reset method of
    the root device, which is propagated down the device tree.  This method
    returns immediately, before the scheduled reset takes place.
machine:save(filename)
    Schedules saving machine state to the specified file.  If the file name is a
    relative path, it is considered to be relative to the first configured save
    state directory.  This method returns immediately, before the machine state
    is saved.  If this method is called when a save or load operation is already
    pending, the previously pending operation will be cancelled.
machine:load(filename)
    Schedules loading machine state from the specified file.  If the file name
    is a relative path, the configured save state directories will be searched.
    This method returns immediately, before the machine state is saved.  If this
    method is called when a save or load operation is already pending, the
    previously pending operation will be cancelled.
machine:popmessage([msg])
    Displays a pop-up message to the user.  If the message is not provided, the
    currently displayed pop-up message (if any) will be hidden.
machine:logerror(msg)
    Writes the message to the machine error log.  This may be displayed in a
    debugger window, written to a file, or written to the standard error output.

Properties
^^^^^^^^^^

machine.time (read-only)
    The elapsed emulated time for the current session as an
    :ref:`attotime <luareference-core-attotime>`.
machine.system (read-only)
    The :ref:`driver metadata <luareference-core-driver>` for the current
    system.
machine.parameters (read-only)
    The :ref:`parameters manager <luareference-core-paramman>` for the current
    emulation session.
machine.video (read-only)
    The :ref:`video manager <luareference-core-videoman>` for the current
    emulation session.
machine.sound (read-only)
    The :ref:`sound manager <luareference-core-soundman>` for the current
    emulation session.
machine.output (read-only)
    The :ref:`output manager <luareference-core-outputman>` for the current
    emulation session.
machine.memory (read-only)
    The :ref:`emulated memory manager <luareference-mem-manager>` for the
    current emulation session.
machine.ioport (read-only)
    The :ref:`I/O port manager <luareference-input-ioportman>` for the current
    emulation session.
machine.input (read-only)
    The :ref:`input manager <luareference-input-inputman>` for the current
    emulation session.
machine.natkeyboard (read-only)
    Gets the :ref:`natural keyboard manager <luareference-input-natkbd>`, used
    for controlling keyboard and keypad input to the emulated system.
machine.uiinput (read-only)
    The :ref:`UI input manager <luareference-input-uiinput>` for the current
    emulation session.
machine.render (read-only)
    The :ref:`render manager <luareference-render-manager>` for the current
    emulation session.
machine.debugger (read-only)
    The :ref:`debugger manager <luareference-debug-manager>` for the current
    emulation session, or ``nil`` if the debugger is not enabled.
machine.options (read-only)
    The user-specified :ref:`options <luareference-core-emuopts>` for the
    current emulation session.
machine.samplerate (read-only)
    The output audio sample rate in Hertz.
machine.paused (read-only)
    A Boolean indicating whether emulation is not currently running, usually
    because the session has been paused or the emulated system has not completed
    starting.
machine.exit_pending (read-only)
    A Boolean indicating whether the emulation session is scheduled to exit.
machine.hard_reset_pending (read-only)
    A Boolean indicating whether a hard reset of the emulated system is pending.
machine.devices (read-only)
    A :ref:`device enumerator <luareference-dev-enum>` that yields all
    :ref:`devices <luareference-dev-device>` in the emulated system.
machine.screens (read-only)
    A :ref:`device enumerator <luareference-dev-enum>` that yields all
    :ref:`screen devices <luareference-dev-screen>` in the emulated system.
machine.cassettes (read-only)
    A :ref:`device enumerator <luareference-dev-enum>` that yields all
    :ref:`cassette image devices <luareference-dev-cass>` in the emulated
    system.
machine.images (read-only)
    A :ref:`device enumerator <luareference-dev-enum>` that yields all
    :ref:`media image devices <luareference-dev-diimage>` in the emulated system.
machine.slots (read-only)
    A :ref:`device enumerator <luareference-dev-enum>` that yields all
    :ref:`slot devices <luareference-dev-dislot>` in the emulated system.

.. _luareference-core-videoman:

Video manager
~~~~~~~~~~~~~

Wraps MAME’s ``video_manager`` class, which is responsible for coordinating
emulated video drawing, speed throttling, and reading host inputs.

Instantiation
^^^^^^^^^^^^^

manager.machine.video
    Gets the video manager for the current emulation session.

Methods
^^^^^^^

video:frame_update()
    Updates emulated screens, reads host inputs, and updates video output.
video:snapshot()
    Saves snapshot files according to the current configuration.  If MAME is
    configured to take native emulated screen snapshots, one snapshot will be
    saved for each emulated screen that is visible in a host window/screen with
    the current view configuration.  If MAME is not configured to use take
    native emulated screen snapshots or if the system has no emulated screens, a
    single snapshot will be saved using the currently selected snapshot view.
video:begin_recording([filename], [format])
    Stops any video recordings currently in progress and starts recording either
    the visible emulated screens or the current snapshot view, depending on
    whether MAME is configured to take native emulated screen snapshots.

    If the file name is not supplied, the configured snapshot file name is used.
    If the file name is a relative path, it is interpreted relative to the first
    configured snapshot directory.  If the format is supplied, it must be
    ``"avi"`` or ``"mng"``.  If the format is not supplied, it defaults to AVI.
video:end_recording()
    Stops any video recordings that are in progress.
video:snapshot_size()
    Returns the width and height in pixels of snapshots created with the current
    snapshot target configuration and emulated screen state.  This may be
    configured explicitly by the user, or calculated based on the selected
    snapshot view and the resolution of any visible emulated screens.
video:snapshot_pixels()
    Returns the pixels of a snapshot created using the current snapshot target
    configuration as 32-bit integers packed into a binary string in host Endian
    order.  Pixels are organised in row-major order, from left to right then top
    to bottom.  Pixel values are colours in RGB format packed into 32-bit
    integers.

Properties
^^^^^^^^^^

video.speed_factor (read-only)
    Configured emulation speed adjustment in per mille (i.e. the ratio to normal
    speed multiplied by 1,000).
video.throttled (read/write)
    A Boolean indicating whether MAME should wait before video updates to avoid
    running faster than the target speed.
video.throttle_rate (read/write)
    The target emulation speed as a ratio of full speed adjusted by the speed
    factor (i.e. 1 is normal speed adjusted by the speed factor, larger numbers
    are faster, and smaller numbers are slower).
video.frameskip (read/write)
    The number of emulated video frames to skip drawing out of every twelve, or
    -1 to automatically adjust the number of frames to skip to maintain the
    target emulation speed.
video.speed_percent (read-only)
    The current emulated speed as a percentage of the full speed adjusted by the
    speed factor.
video.effective_frameskip (read-only)
    The number of emulated frames that are skipped out of every twelve.
video.skip_this_frame (read-only)
    A Boolean indicating whether the video manager will skip drawing emulated
    screens for the current frame.
video.snap_native (read-only)
    A Boolean indicating whether the video manager will take native emulated
    screen snapshots.  In addition to the relevant configuration setting, the
    emulated system must have at least one emulated screen.
video.is_recording (read-only)
    A Boolean indicating whether any video recordings are currently in progress.
video.snapshot_target (read-only)
    The :ref:`render target <luareference-render-target>` used to produce
    snapshots and video recordings.

.. _luareference-core-soundman:

Sound manager
~~~~~~~~~~~~~

Wraps MAME’s ``sound_manager`` class, which manages the emulated sound stream
graph and coordinates sound output.

Instantiation
^^^^^^^^^^^^^

manager.machine.sound
    Gets the sound manager for the current emulation session.

Methods
^^^^^^^

sound:start_recording([filename])
    Starts recording to a WAV file.  Has no effect if currently recording.  If
    the file name is not supplied, uses the configured WAV file name (from
    command line or INI file), or has no effect if no WAV file name is
    configured.  Returns ``true`` if recording started, or ``false`` if
    recording is already in progress, opening the output file failed, or no file
    name was supplied or configured.
sound:stop_recording()
    Stops recording and closes the file if currently recording to a WAV file.
sound:get_samples()
    Returns the current contents of the output sample buffer as a binary string.
    Samples are 16-bit integers in host byte order.  Samples for left and right
    stereo channels are interleaved.

Properties
^^^^^^^^^^

sound.muted (read-only)
    A Boolean indicating whether sound output is muted for any reason.
sound.ui_mute (read/write)
    A Boolean indicating whether sound output is muted at the request of the
    user.
sound.debugger_mute (read/write)
    A Boolean indicating whether sound output is muted at the request of the
    debugger.
sound.system_mute (read/write)
    A Boolean indicating whether sound output is muted at the request of the
    emulated system.
sound.attenuation (read/write)
    The output volume attenuation in decibels.  Should generally be a negative
    integer or zero.
sound.recording (read-only)
    A Boolean indicating whether sound output is currently being recorded to a
    WAV file.

.. _luareference-core-outputman:

Output manager
~~~~~~~~~~~~~~

Wraps MAME’s ``output_manager`` class, providing access to system outputs that
can be used for interactive artwork or consumed by external programs.

Instantiation
^^^^^^^^^^^^^

manager.machine.output
    Gets the output manager for the current emulation session.

Methods
^^^^^^^

output:set_value(name, val)
    Sets the specified output value.  The value must be an integer.  The output
    will be created if it does not already exist.
output:set_indexed_value(prefix, index, val)
    Appends the index (formatted as a decimal integer) to the prefix and sets
    the value of the corresponding output.  The value must be an integer.  The
    output will be created if it does not already exist.
output:get_value(name)
    Returns the value of the specified output, or zero if it doesn’t exist.
output:get_indexed_value(prefix, index)
    Appends the index (formatted as a decimal integer) to the prefix and returns
    the value of the corresponding output, or zero if it doesn’t exist.
output:name_to_id(name)
    Gets the per-session unique integer ID for the specified output, or zero if
    it doesn’t exist.
output:id_to_name(id)
    Gets the name for the output with the specified per-session unique ID, or
    ``nil`` if it doesn’t exist.  This method has O(n) complexity, so avoid
    calling it when performance is important.

.. _luareference-core-paramman:

Parameters manager
~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``parameters_manager`` class, which provides a simple key-value
store for metadata from system ROM definitions.

Instantiation
^^^^^^^^^^^^^

manager.machine.parameters
    Gets the parameters manager for the current emulation session.

Methods
^^^^^^^

parameters:lookup(tag)
    Gets the value for the specified parameter if it is set, or an empty string
    if it is not set.
parameters:add(tag, value)
    Sets the specified parameter if it is not set.  Has no effect if the
    specified parameter is already set.

.. _luareference-core-uiman:

UI manager
~~~~~~~~~~

Wraps MAME’s ``mame_ui_manager`` class, which handles menus and other user
interface functionality.

Instantiation
^^^^^^^^^^^^^

manager.ui
    Gets the UI manager for the current session.

Methods
^^^^^^^

ui:get_char_width(ch)
    Gets the width of a Unicode character as a proportion of the width of the UI
    container in the current font at the configured UI line height.
ui:get_string_width(str)
    Gets the width of a string as a proportion of the width of the UI container
    in the current font at the configured UI line height.
ui:set_aggressive_input_focus(enable)
    On some platforms, this controls whether MAME should accept input focus in
    more situations than when its windows have UI focus.
ui:get_general_input_setting(type, [player])
    Gets a description of the configured
    :ref:`input sequence <luareference-input-iptseq>` for the specified input
    type and player suitable for using in prompts.  The input type is an
    enumerated value.  The player number is a zero-based index.  If the player
    number is not supplied, it is assumed to be zero.

Properties
^^^^^^^^^^

ui.options (read-only)
    The UI :ref:`options <luareference-core-coreopts>` for the current session.
ui.line_height (read-only)
    The configured UI text line height as a proportion of the height of the UI
    container.
ui.menu_active (read-only)
    A Boolean indicating whether an interactive UI element is currently active.
    Examples include menus and slider controls.
ui.single_step (read/write)
    A Boolean controlling whether the emulated system should be automatically
    paused when the next frame is drawn.  This property is automatically reset
    when the automatic pause happens.
ui.show_fps (read/write)
    A Boolean controlling whether the current emulation speed and frame skipping
    settings should be displayed.
ui.show_profiler (read/write)
    A Boolean controlling whether profiling statistics should be displayed.

.. _luareference-core-driver:

System driver metadata
~~~~~~~~~~~~~~~~~~~~~~

Provides some metadata for an emulated system.

Instantiation
^^^^^^^^^^^^^

emu.driver_find(name)
    Gets the driver metadata for the system with the specified short name, or
    ``nil`` if no such system exists.
manager.machine.system
    Gets the driver metadata for the current system.

Properties
^^^^^^^^^^

driver.name (read-only)
    The short name of the system, as used on the command line, in configuration
    files, and when searching for resources.
driver.description (read-only)
    The full display name for the system.
driver.year (read-only)
    The release year for the system.  May contain question marks if not known
    definitively.
driver.manufacturer (read-only)
    The manufacturer, developer or distributor of the system.
driver.parent (read-only)
    The short name of parent system for organisation purposes, or ``"0"`` if the
    system has no parent.
driver.compatible_with (read-only)
    The short name of a system that this system is compatible with software for,
    or ``nil`` if the system is not listed as compatible with another system.
driver.source_file (read-only)
    The source file where this system driver is defined.  The path format
    depends on the toolchain the emulator was built with.
driver.rotation (read-only)
    A string indicating the rotation applied to all screens in the system after
    the screen orientation specified in the machine configuration is applied.
    Will be one of ``"rot0"``, ``"rot90"``, ``"rot180"`` or ``"rot270"``.
driver.type (read-only)
    A string providing a system type.  Will be one of ``"arcade"``,
    ``"console"``, ``"computer"`` or ``"other"``.  This is for informational
    purposes only, and may not be supported in the future.
driver.not_working (read-only)
    A Boolean indicating whether the system is marked as not working.
driver.supports_save (read-only)
    A Boolean indicating whether the system supports save states.
driver.no_cocktail (read-only)
    A Boolean indicating whether screen flipping in cocktail mode is
    unsupported.
driver.is_bios_root (read-only)
    A Boolean indicating whether this system represents a system that runs
    software from removable media without media present.
driver.requires_artwork (read-only)
    A Boolean indicating whether the system requires external artwork to be
    usable.
driver.clickable_artwork (read-only)
    A Boolean indicating whether the system requires clickable artwork features
    to be usable.
driver.unofficial (read-only)
    A Boolean indicating whether this is an unofficial but common user
    modification to a system.
driver.no_sound_hw (read-only)
    A Boolean indicating whether the system has no sound output hardware.
driver.mechanical (read-only)
    A Boolean indicating whether the system depends on mechanical features that
    cannot be properly simulated.
driver.is_incomplete (read-only)
    A Boolean indicating whether the system is a prototype with incomplete
    functionality.

.. _luareference-core-plugin:

Lua plugin
~~~~~~~~~~

Provides a description of an available Lua plugin.

Instantiation
^^^^^^^^^^^^^

manager.plugins[name]
    Gets the description of the Lua plugin with the specified name, or ``nil``
    if no such plugin is available

Properties
^^^^^^^^^^

plugin.name (read-only)
    The short name of the plugin, used in configuration and when accessing the
    plugin programmatically.
plugin.description (read-only)
    The display name for the plugin.
plugin.type (read-only)
    The plugin type.  May be ``"plugin"`` for user-loadable plugins, or
    ``"library"`` for libraries providing common functionality to multiple
    plugins.
plugin.directory (read-only)
    The path to the directory containing the plugin’s files.
plugin.start (read-only)
    A Boolean indicating whether the plugin enabled.


.. _luareference-dev:

Devices
-------

Several device classes and device mix-ins classes are exposed to Lua.  Devices
can be looked up by tag or enumerated.

.. _luareference-dev-enum:

Device enumerators
~~~~~~~~~~~~~~~~~~

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
^^^^^^^^^^^^^

manager.machine.devices
    Returns a device enumerator that will iterate over
    :ref:`devices <luareference-dev-device>` in the system.
manager.machine.screens
    Returns a device enumerator that will iterate over
    :ref:`screen devices <luareference-dev-screen>` in the system.
manager.machine.cassettes
    Returns a device enumerator that will iterate over
    :ref:`cassette image devices <luareference-dev-cass>` in the system.
manager.machine.images
    Returns a device enumerator that will iterate over
    :ref:`media image devices <luareference-dev-diimage>` in the system.
manager.machine.slots
    Returns a device enumerator that will iterate over
    :ref:`slot devices <luareference-dev-dislot>` in the system.
emu.device_enumerator(device, [depth])
    Returns a device enumerator that will iterate over
    :ref:`devices <luareference-dev-device>` in the sub-tree starting at the
    specified device.  The specified device will be included.  If the depth is
    provided, it must be an integer specifying the maximum number of levels to
    iterate below the specified device (i.e. 1 will limit iteration to the
    device and its immediate children).
emu.screen_enumerator(device, [depth])
    Returns a device enumerator that will iterate over
    :ref:`screen devices <luareference-dev-screen>` in the sub-tree starting at
    the specified device.  The specified device will be included if it is a
    screen device.  If the depth is provided, it must be an integer specifying
    the maximum number of levels to iterate below the specified device (i.e. 1
    will limit iteration to the device and its immediate children).
emu.cassette_enumerator(device, [depth])
    Returns a device enumerator that will iterate over
    :ref:`cassette image devices <luareference-dev-cass>` in the sub-tree
    starting at the specified device.  The specified device will be included if
    it is a cassette image device.  If the depth is provided, it must be an
    integer specifying the maximum number of levels to iterate below the
    specified device (i.e. 1 will limit iteration to the device and its
    immediate children).
emu.image_enumerator(device, [depth])
    Returns a device enumerator that will iterate over
    :ref:`media image devices <luareference-dev-diimage>` in the sub-tree
    starting at the specified device.  The specified device will be included if
    it is a media image device.  If the depth is provided, it must be an integer
    specifying the maximum number of levels to iterate below the specified
    device (i.e. 1 will limit iteration to the device and its immediate
    children).
emu.slot_enumerator(device, [depth])
    Returns a device enumerator that will iterate over
    :ref:`slot devices <luareference-dev-dislot>` in the sub-tree starting at
    the specified device.  The specified device will be included if it is a
    slot device.  If the depth is provided, it must be an integer specifying the
    maximum number of levels to iterate below the specified device (i.e. 1 will
    limit iteration to the device and its immediate children).

.. _luareference-dev-device:

Device
~~~~~~

Wraps MAME’s ``device_t`` class, which is a base of all device classes.

Instantiation
^^^^^^^^^^^^^

manager.machine.devices[tag]
    Gets a device by tag relative to the root machine device, or ``nil`` if no
    such device exists.
manager.machine.devices[tag]:subdevice(tag)
    Gets a device by tag relative to another arbitrary device, or ``nil`` if no
    such device exists.

Methods
^^^^^^^

device:subtag(tag)
    Converts a tag relative to the device to an absolute tag.
device:siblingtag(tag)
    Converts a tag relative to the device’s parent device to an absolute tag.
device:memshare(tag)
    Gets a :ref:`memory share <luareference-mem-share>` by tag relative to the
    device, or ``nil`` if no such memory share exists.
device:membank(tag)
    Gets a :ref:`memory bank <luareference-mem-bank>` by tag relative to the
    device, or ``nil`` if no such memory bank exists.
device:memregion(tag)
    Gets a :ref:`memory region <luareference-mem-region>` by tag relative to the
    device, or ``nil`` if no such memory region exists.
device:ioport(tag)
    Gets an :ref:`I/O port <luareference-input-ioport>` by tag relative to the
    device, or ``nil`` if no such I/O port exists.
device:subdevice(tag)
    Gets a device by tag relative to the device.
device:siblingdevice(tag)
    Gets a device by tag relative to the device’s parent.
device:parameter(tag)
    Gets a parameter value by tag relative to the device, or an empty string if
    the parameter is not set.

Properties
^^^^^^^^^^

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
    The :ref:`debugger interface <luareference-debug-devdebug>` to the device if
    it is a CPU device, or ``nil`` if it is not a CPU device or the debugger is
    not enabled.
device.spaces[] (read-only)
    A table of the device’s :ref:`address spaces <luareference-mem-space>`,
    indexed by name.  Only valid for devices that implement the memory
    interface.  Note that the names are specific to the device type and have no
    special significance.

.. _luareference-dev-screen:

Screen device
~~~~~~~~~~~~~

Wraps MAME’s ``screen_device`` class, which represents an emulated video output.

Instantiation
^^^^^^^^^^^^^

manager.machine.screens[tag]
    Gets a screen device by tag relative to the root machine device, or ``nil``
    if no such device exists or it is not a screen device.

Base classes
^^^^^^^^^^^^

* :ref:`luareference-dev-device`

Methods
^^^^^^^

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
    Returns all visible pixels as 32-bit integers packed into a binary string in
    host Endian order.  Pixels are organised in row-major order, from left to
    right then top to bottom.  Pixels values are either palette indices or
    colours in RGB format packed into 32-bit integers.
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
screen:draw_line(x1, y1, x2, y2, [color])
    Draws a line from (x1, y1) to (x2, y2).

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
^^^^^^^^^^

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
    The :ref:`render container <luareference-render-container>` used to draw the
    screen.

.. _luareference-dev-cass:

Cassette image device
~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``cassette_image_device`` class, representing a compact cassette
mechanism typically used by a home computer for program storage.

Instantiation
^^^^^^^^^^^^^

manager.machine.cassettes[tag]
    Gets a cassette image device by tag relative to the root machine device, or
    ``nil`` if no such device exists or it is not a cassette image device.

Base classes
^^^^^^^^^^^^

* :ref:`luareference-dev-device`
* :ref:`luareference-dev-diimage`

Methods
^^^^^^^

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
^^^^^^^^^^

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

.. _luareference-dev-diimage:

Image device interface
~~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``device_image_interface`` class which is a mix-in implemented by
devices that can load media image files.

Instantiation
^^^^^^^^^^^^^

manager.machine.images[tag]
    Gets an image device by tag relative to the root machine device, or ``nil``
    if no such device exists or it is not a media image device.

Methods
^^^^^^^

image:load(filename)
    Loads the specified file as a media image.  Returns ``"pass"`` or
    ``"fail"``.
image:load_software(name)
    Loads a media image described in a software list.  Returns ``"pass"`` or
    ``"fail"``.
image:unload()
    Unloads the mounted image.
image:create(filename)
    Creates and mounts a media image file with the specified name.  Returns
    ``"pass"`` or ``"fail"``.
image:display()
    Returns a “front panel display” string for the device, if supported.  This
    can be used to show status information, like the current head position or
    motor state.

Properties
^^^^^^^^^^

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
    The :ref:`media image formats <luareference-dev-imagefmt>` supported by the
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
    The underlying :ref:`device <luareference-dev-device>`.

.. _luareference-dev-dislot:

Slot device interface
~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``device_slot_interface`` class which is a mix-in implemented by
devices that instantiate a user-specified child device.

Instantiation
^^^^^^^^^^^^^

manager.machine.slots[tag]
    Gets an slot device by tag relative to the root machine device, or ``nil``
    if no such device exists or it is not a slot device.

Properties
^^^^^^^^^^

slot.fixed (read-only)
    A Boolean indicating whether this is a slot with a card specified in machine
    configuration that cannot be changed by the user.
slot.has_selectable_options (read-only)
    A Boolean indicating whether the slot has any user-selectable options (as
    opposed to options that can only be selected programmatically, typically for
    fixed slots or to load media images).
slot.options[] (read-only)
    The :ref:`slot options <luareference-dev-slotopt>` describing the child
    devices that can be instantiated by the slot, indexed by option value.  The
    ``at`` and ``index_of`` methods have O(n) complexity; all other supported
    operations have O(1) complexity.
slot.device (read-only)
    The underlying :ref:`device <luareference-dev-device>`.

.. _luareference-dev-imagefmt:

Media image format
~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``image_device_format`` class, which describes a media file format
supported by a :ref:`media image device <luareference-dev-diimage>`.

Instantiation
^^^^^^^^^^^^^

manager.machine.images[tag].formatlist[name]
    Gets a media image format supported by a given device by name.

Properties
^^^^^^^^^^

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

.. _luareference-dev-slotopt:

Slot option
~~~~~~~~~~~

Wraps MAME’s ``device_slot_interface::slot_option`` class, which represents a
child device that a :ref:`slot device <luareference-dev-dislot>` can be
configured to instantiate.

Instantiation
^^^^^^^^^^^^^

manager.machine.slots[tag].options[name]
    Gets a slot option for a given :ref:`slot device <luareference-dev-dislot>`
    by name (i.e. the value used to select the option).

Properties
^^^^^^^^^^

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

manager.machine.memory
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

manager.machine.devices[tag].spaces[name]
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
space:add_change_notifier(callback)
    Add a callback to receive notifications for handler changes in address
    space.  The callback function is passed a single string as an argument,
    either ``r`` if read handlers have potentially changed, ``w`` if write
    handlers have potentially changed, or ``rw`` if both read and write handlers
    have potentially changed.

    Returns a :ref:`notifier subscription <luareference-core-notifiersub>`.
space:install_read_tap(start, end, name, callback)
    Installs a :ref:`pass-through handler <luareference-mem-tap>` that will
    receive notifications on reads from the specified range of addresses in the
    address space.  The start and end addresses are inclusive.  The name must be
    a string, and the callback must be a function.

    The callback is passed three arguments for the access offset, the data read,
    and the memory access mask.  To modify the data being read, return the
    modified value from the callback function as an integer.  If the callback
    does not return an integer, the data will not be modified.
space:install_write_tap(start, end, name, callback)
    Installs a :ref:`pass-through handler <luareference-mem-tap>` that will
    receive notifications on write to the specified range of addresses in the
    address space.  The start and end addresses are inclusive.  The name must be
    a string, and the callback must be a function.

    The callback is passed three arguments for the access offset, the data
    written, and the memory access mask.  To modify the data being written,
    return the modified value from the callback function as an integer.  If the
    callback does not return an integer, the data will not be modified.

Properties
^^^^^^^^^^

space.name (read-only)
    The display name for the address space.
space.shift (read-only)
    The address granularity for the address space specified as the shift
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

.. _luareference-mem-tap:

Pass-through handler
~~~~~~~~~~~~~~~~~~~~

Tracks a pass-through handler installed in an
:ref:`address space <luareference-mem-space>`.  A memory pass-through handler
receives notifications on accesses to a specified range of addresses, and can
modify the data that is read or written if desired.

Instantiation
^^^^^^^^^^^^^

manager.machine.devices[tag].spaces[name]:install_read_tap(start, end, name, callback)
    Installs a pass-through handler that will receive notifications on reads
    from the specified range of addresses in an
    :ref:`address space <luareference-mem-space>`.
manager.machine.devices[tag].spaces[name]:install_write_tap(start, end, name, callback)
    Installs a pass-through handler that will receive notifications on writes to
    the specified range of addresses in an
    :ref:`address space <luareference-mem-space>`.

Methods
^^^^^^^

passthrough:reinstall()
    Reinstalls the pass-through handler in the address space.  May be necessary
    if the handler is removed due to other changes to handlers in the address
    space.
passthrough:remove()
    Removes the pass-through handler from the address space.  The associated
    callback will not be called in response to future memory accesses.

Properties
^^^^^^^^^^

passthrough.addrstart (read-only)
    The inclusive start address of the address range monitored by the
    pass-through handler (i.e. the lowest address that the handler will be
    notified for).
passthrough.addrend (read-only)
    The inclusive end address of the address range monitored by the pass-through
    handler (i.e. the highest address that the handler will be notified for).
passthrough.name (read-only)
    The display name for the pass-through handler.

.. _luareference-mem-map:

Address map
~~~~~~~~~~~

Wraps MAME’s ``address_map`` class, used to configure handlers for an address
space.

Instantiation
^^^^^^^^^^^^^

manager.machine.devices[tag].spaces[name].map
    Gets the configured address map for an address space, or ``nil`` if no map
    is configured.

Properties
^^^^^^^^^^

map.spacenum (read-only)
    The address space number of the address space the map is associated with.
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

manager.machine.devices[tag].spaces[name].map.entries[index]
    Gets an entry from the configured map for an address space.

Properties
^^^^^^^^^^

entry.address_start (read-only)
    Start address of the entry’s range.
entry.address_end (read-only)
    End address of the entry’s range (inclusive).
entry.address_mirror (read-only)
    Address mirror bits.
entry.address_mask (read-only)
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
entry.region (read-only)
    Explicit memory region tag for ROM entries, or ``nil``.  For ROM entries,
    ``nil`` infers the region from the device tag.
entry.region_offset (read-only)
    Starting offset in memory region for ROM entries.

.. _luareference-memory-handlerdata:

Address map handler data
~~~~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``map_handler_data`` class, which provides configuration data to
handlers in address maps.

Instantiation
^^^^^^^^^^^^^

manager.machine.devices[tag].spaces[name].map.entries[index].read
    Gets the read handler data for an address map entry.
manager.machine.devices[tag].spaces[name].map.entries[index].write
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

manager.machine.memory.shares[tag]
    Gets a memory share by absolute tag, or ``nil`` if no such memory share
    exists.
manager.machine.devices[tag]:memshare(tag)
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

manager.machine.memory.banks[tag]
    Gets a memory region by absolute tag, or ``nil`` if no such memory bank
    exists.
manager.machine.devices[tag]:membank(tag)
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

manager.machine.memory.regions[tag]
    Gets a memory region by absolute tag, or ``nil`` if no such memory region
    exists.
manager.machine.devices[tag]:memregion(tag)
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

manager.machine.ioport
    Gets the global I/O port manager instance for the emulated machine.

Methods
^^^^^^^

ioport:count_players()
    Returns the number of player controllers in the system.
ioport:type_pressed(type, [player])
    Returns a Boolean indicating whether the specified input is currently
    pressed.  The input type may be an enumerated value or an
    :ref:`input type <luareference-input-inputtype>` entry.  If the input type
    is an enumerated value, the player number may be supplied as a zero-based
    index; if the player number is not supplied, it is assumed to be zero.  If
    the input type is an input type entry, the player number may not be supplied
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
    Get the configured :ref:`input sequence <luareference-input-iptseq>` for the
    specified input type, player number and sequence type.  The input type may
    be an enumerated value or an
    :ref:`input type <luareference-input-inputtype>` entry.  If the input type
    is an enumerated value, the player number may be supplied as a zero-based
    index; if the player number is not supplied, it is assumed to be zero.  If
    the input type is an input type entry, the player number may not be supplied
    separately.  If the sequence type is supplied, it must be ``"standard"``,
    ``"increment"`` or ``"decrement"``; if it is not supplied, it is assumed to
    be ``"standard"``.

    This provides access to general input configuration.
ioport:set_type_seq(type, [player], seqtype, seq)
    Set the configured :ref:`input sequence <luareference-input-iptseq>` for the
    specified input type, player number and sequence type.  The input type may
    be an enumerated value or an
    :ref:`input type <luareference-input-inputtype>` entry.  If the input type
    is an enumerated value, the player number must be supplied as a zero-based
    index.  If the input type is an input type entry, the player number may not
    be supplied separately.  The sequence type must be ``"standard"``,
    ``"increment"`` or ``"decrement"``.

    This allows general input configuration to be set.
ioport:token_to_input_type(string)
    Returns the input type and player number for the specified input type token
    string.
ioport:input_type_to_token(type, [player])
    Returns the token string for the specified input type and player number.  If
    the player number is not supplied, it assumed to be zero.

Properties
^^^^^^^^^^

ioport.types[] (read-only)
    Gets the supported :ref:`input types <luareference-input-inputtype>`.  Keys
    are arbitrary indices.  All supported operations have O(1) complexity.
ioport.ports[]
    Gets the emulated :ref:`I/O ports <luareference-input-ioport>` in the
    system.  Keys are absolute tags.  The ``at`` and ``index_of`` methods have
    O(n) complexity; all other supported operations have O(1) complexity.

.. _luareference-input-natkbd:

Natural keyboard manager
~~~~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``natural_keyboard`` class, which manages emulated keyboard and
keypad inputs.

Instantiation
^^^^^^^^^^^^^

manager.machine.natkeyboard
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

manager.machine.natkeyboard.keyboards[tag]
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

manager.machine.ioport.ports[tag]
    Gets an emulated I/O port by absolute tag, or ``nil`` if the tag does not
    correspond to an I/O port.
manager.machine.devices[devtag]:ioport(porttag)
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

manager.machine.ioport.ports[tag]:field(mask)
    Gets a field for the given port by bit mask.
manager.machine.ioport.ports[tag].fields[name]
    Gets a field for the given port by display name.

Methods
^^^^^^^

field:set_value(value)
    Set the value of the I/O port field.  For digital fields, the value is
    compared to zero to determine whether the field should be active; for
    analog fields, the value must be right-aligned and in the correct range.
field:clear_value()
    Clear programmatically overridden value and restore the field’s regular
    behaviour.
field:set_input_seq(seqtype, seq)
    Set the :ref:`input sequence <luareference-input-iptseq>` for the
    specified sequence type.  This is used to configure per-machine input
    settings.  The sequence type must be ``"standard"``, ``"increment"`` or
    ``"decrement"``.
field:input_seq(seq_type)
    Get the configured :ref:`input sequence <luareference-input-iptseq>` for the
    specified sequence type.  This gets per-machine input assignments.  The
    sequence type must be ``"standard"``, ``"increment"`` or ``"decrement"``.
field:set_default_input_seq(seq_type, seq)
    Set the default :ref:`input sequence <luareference-input-iptseq>` for the
    specified sequence type.  This overrides the default input assignment for a
    specific input.  The sequence type must be ``"standard"``, ``"increment"``
    or ``"decrement"``.
field:default_input_seq(seq_type)
    Gets the default :ref:`input sequence <luareference-input-iptseq>` for the
    specified sequence type.  If the default assignment is not overridden, this
    gets the general input assignment.  The sequence type must be
    ``"standard"``, ``"increment"`` or ``"decrement"``.
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

.. _luareference-input-fieldlive:

Live I/O port field state
~~~~~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``ioport_field_live`` class, representing the live state of an I/O
port field.

Instantiation
^^^^^^^^^^^^^

manager.machine.ioport.ports[tag]:field(mask).live
    Gets the live state for an I/O port field.

Properties
^^^^^^^^^^

live.name
    Display name for the field.

.. _luareference-input-inputtype:

Input type
~~~~~~~~~~

Wraps MAME’s ``input_type_entry`` class, representing an emulated input type or
emulator UI input type.  Input types are uniquely identified by the combination
of their enumerated type value and player index.

Instantiation
^^^^^^^^^^^^^

manager.machine.ioport.types[index]
    Gets a supported input type.

Properties
^^^^^^^^^^

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

.. _luareference-input-inputman:

Input manager
~~~~~~~~~~~~~

Wraps MAME’s ``input_manager`` class, which reads host input devices and checks
whether configured inputs are active.

Instantiation
^^^^^^^^^^^^^

manager.machine.input
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
    Returns a Boolean indicating whether the supplied
    :ref:`input sequence <luareference-input-iptseq>` is currently pressed.
input:seq_clean(seq)
    Remove invalid elements from the supplied
    :ref:`input sequence <luareference-input-iptseq>`.  Returns the new, cleaned
    input sequence.
input:seq_name(seq)
    Get display text for an :ref:`input sequence <luareference-input-iptseq>`.
input:seq_to_tokens(seq)
    Convert an :ref:`input sequence <luareference-input-iptseq>` to a token
    string.  This should be used when saving configuration.
input:seq_from_tokens(tokens)
    Convert a token string to an
    :ref:`input sequence <luareference-input-iptseq>`.  This should be used when
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
    obtaining an :ref:`input sequence <luareference-input-iptseq>` for
    configuring an analog input.
input:axis_sequence_poller()
    Returns an :ref:`input sequence poller <luareference-input-seqpoll>` for
    obtaining an :ref:`input sequence <luareference-input-iptseq>` for
    configuring a digital input.

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

manager.machine.input:axis_code_poller()
    Returns an input code poller that polls for analog inputs being activated.
manager.machine.input:switch_code_poller()
    Returns an input code poller that polls for host switch inputs being
    activated.
manager.machine.input:keyboard_code_poller()
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

manager.machine.input:axis_sequence_poller()
    Returns an input sequence poller for assigning host inputs to an analog
    input.
manager.machine.input:switch_sequence_poller()
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
    The current :ref:`input sequence <luareference-input-iptseq>`.  This is
    updated while polling.  It is possible for the sequence to become invalid.
poller.valid (read-only)
    A Boolean indicating whether the current input sequence is valid.
poller.modified (read-only)
    A Boolean indicating whether the sequence was changed by any user input
    since starting polling.

.. _luareference-input-iptseq:

Input sequence
~~~~~~~~~~~~~~

Wraps MAME’s ``input_seq`` class, representing a combination of host inputs that
can be read or assigned to an emulated input.  Input sequences can be
manipulated using :ref:`input manager <luareference-input-inputman>` methods.
Use an :ref:`input sequence poller <luareference-input-seqpoll>` to obtain an
input sequence from the user.

Instantiation
^^^^^^^^^^^^^

emu.input_seq()
    Creates an empty input sequence.
emu.input_seq(seq)
    Creates a copy of an existing input sequence.

Methods
^^^^^^^

seq:reset()
    Clears the input sequence, removing all items.
seq:set_default()
    Sets the input sequence to a single item containing the metavalue specifying
    that the default setting should be used.

Properties
^^^^^^^^^^

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

.. _luareference-input-devclass:

Host input device class
~~~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``input_class`` class, representing a category of host input
devices (e.g. keyboards or joysticks).

Instantiation
^^^^^^^^^^^^^

manager.machine.input.device_classes[name]
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

manager.machine.input.device_classes[name].devices[index]
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

manager.machine.input.device_classes[name].devices[index].items[id]
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

manager.machine.uiinput
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

manager.machine.render
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

manager.machine.render.targets[index]
    Gets a render target by index.
manager.machine.render.ui_target
    Gets the render target used to display the user interface (including menus,
    sliders and pop-up messages).  This is usually the first host window or
    screen.
manager.machine.video.snapshot_target
    Gets the render target used to produce snapshots and video recordings.

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

manager.machine.render.ui_container
    Gets the render container used to draw the user interface, including menus,
    sliders and pop-up messages.
manager.machine.screens[tag].container
    Gets the render container used to draw a given screen.

Methods
^^^^^^^

container:draw_box(left, top, right, bottom, [line], [fill])
    Draws an outlined rectangle with edges at the specified positions.

    Coordinates are floating-point numbers in the range of 0 (zero) to 1 (one),
    with (0, 0) at the top left and (1, 1) at the bottom right of the window or
    screen that showss the user interface.  Note that the aspect ratio is
    usually not square.  Coordinates are limited to the window or screen area.

    The fill and line colours are in alpha/red/green/blue (ARGB) format.
    Channel values are in the range 0 (transparent or off) to 255 (opaque or
    full intensity), inclusive.  Colour channel values are not pre-multiplied by
    the alpha value.  The channel values must be packed into the bytes of a
    32-bit unsigned integer, in the order alpha, red, green, blue from
    most-significant to least-significant byte.  If the line colour is not
    provided, the UI text colour is used; if the fill colour is not provided,
    the UI background colour is used.
container:draw_line(x1, y1, x2, y2, [color])
    Draws a line from (x1, y1) to (x2, y2).

    Coordinates are floating-point numbers in the range of 0 (zero) to 1 (one),
    with (0, 0) at the top left and (1, 1) at the bottom right of the window or
    screen that showss the user interface.  Note that the aspect ratio is
    usually not square.  Coordinates are limited to the window or screen area.

    Coordinates are floating-point numbers in units of screen pixels, with the
    origin at (0, 0).  Note that screen pixels often aren’t square.  The
    coordinate system is rotated if the screen is rotated, which is usually the
    case for vertical-format screens.  Before rotation, the origin is at the top
    left, and coordinates increase to the right and downwards.  Coordinates are
    limited to the screen area.

    The line colour is in alpha/red/green/blue (ARGB) format.  Channel values
    are in the range 0 (transparent or off) to 255 (opaque or full intensity),
    inclusive.  Colour channel values are not pre-multiplied by the alpha value.
    The channel values must be packed into the bytes of a 32-bit unsigned
    integer, in the order alpha, red, green, blue from most-significant to
    least-significant byte.  If the line colour is not provided, the UI text
    colour is used.
container:draw_text(x|justify, y, text, [foreground], [background])
    Draws text at the specified position.  If the screen is rotated the text
    will be rotated.

    If the first argument is a number, the text will be left-aligned at this X
    coordinate.  If the first argument is a string, it must be ``"left"``,
    ``"center"`` or ``"right"`` to draw the text left-aligned at the
    left edge of the window or screen, horizontally centred in the window or
    screen, or right-aligned at the right edge of the window or screen,
    respectively.  The second argument specifies the Y coordinate of the maximum
    ascent of the text.

    Coordinates are floating-point numbers in the range of 0 (zero) to 1 (one),
    with (0, 0) at the top left and (1, 1) at the bottom right of the window or
    screen that showss the user interface.  Note that the aspect ratio is
    usually not square.  Coordinates are limited to the window or screen area.

    The foreground and background colours are in alpha/red/green/blue (ARGB)
    format.  Channel values are in the range 0 (transparent or off) to 255
    (opaque or full intensity), inclusive.  Colour channel values are not
    pre-multiplied by the alpha value.  The channel values must be packed into
    the bytes of a 32-bit unsigned integer, in the order alpha, red, green, blue
    from most-significant to least-significant byte.  If the foreground colour
    is not provided, the UI text colour is used; if the background colour is not
    provided, it is fully transparent.

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

manager.machine.screens[tag].container
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

manager.machine.render.targets[index].current_view
    Gets the currently selected view for a given render target.
file.views[name]
    Gets the view with the specified name from a
    :ref:`layout file <luareference-render-layfile>`.  This is how layout
    scripts generally obtain views.

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

Wraps MAME’s ``layout_view_item`` class, representing an item in a view.  An
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
item:set_element_state_callback(cb)
    Set a function to call to obtain the element state for the item.  The
    function must accept no arguments and return an integer.  Call with ``nil``
    to restore the default element state callback (based on bindings in the XML
    layout file).

    Note that the function must not access the item’s ``element_state``
    property, as this will result in infinite recursion.

    This callback will not be used to obtain the animation state for the item,
    even if the item lacks explicit animation state bindings in the XML layout
    file.
item:set_animation_state_callback(cb)
    Set a function to call to obtain the animation state for the item.  The
    function must accept no arguments and return an integer.  Call with ``nil``
    to restore the default animation state callback (based on bindings in the
    XML layout file).

    Note that the function must not access the item’s ``animation_state``
    property, as this will result in infinite recursion.
item:set_bounds_callback(cb)
    Set a function to call to obtain the bounds for the item.  The function must
    accept no arguments and return a
    :ref:`render bounds <luareference-render-bounds>` object in render target
    coordinates.  Call with ``nil`` to restore the default bounds callback
    (based on the item’s animation state and ``bounds`` child elements in the
    XML layout file).

    Note that the function must not access the item’s ``bounds`` property, as
    this will result in infinite recursion.
item:set_color_callback(cb)
    Set a function to call to obtain the multiplier colour for the item.  The
    function must accept no arguments and return a
    :ref:`render colour <luareference-render-color>` object.  Call with ``nil``
    to restore the default colour callback (based on the item’s animation state
    and ``color`` child elements in the XML layout file).

    Note that the function must not access the item’s ``color`` property, as
    this will result in infinite recursion.
item:set_scroll_size_x_callback(cb)
    Set a function to call to obtain the size of the horizontal scroll window as
    a proportion of the associated element’s width.  The function must accept no
    arguments and return a floating-point value.  Call with ``nil`` to restore
    the default horizontal scroll window size callback (based on the ``xscroll``
    child element in the XML layout file).

    Note that the function must not access the item’s ``scroll_size_x``
    property, as this will result in infinite recursion.
item:set_scroll_size_y_callback(cb)
    Set a function to call to obtain the size of the vertical scroll window as a
    proportion of the associated element’s height.  The function must accept no
    arguments and return a floating-point value.  Call with ``nil`` to restore
    the default vertical scroll window size callback (based on the ``yscroll``
    child element in the XML layout file).

    Note that the function must not access the item’s ``scroll_size_y``
    property, as this will result in infinite recursion.
item:set_scroll_pos_x_callback(cb)
    Set a function to call to obtain the horizontal scroll position.  A value of
    zero places the horizontal scroll window at the left edge of the associated
    element.  If the item does not wrap horizontally, a value of 1.0 places the
    horizontal scroll window at the right edge of the associated element; if the
    item wraps horizontally, a value of 1.0 corresponds to wrapping back to the
    left edge of the associated element.  The function must accept no arguments
    and return a floating-point value.  Call with ``nil`` to restore the default
    horizontal scroll position callback (based on bindings in the ``xscroll``
    child element in the XML layout file).

    Note that the function must not access the item’s ``scroll_pos_x`` property,
    as this will result in infinite recursion.
item:set_scroll_pos_y_callback(cb)
    Set a function to call to obtain the vertical scroll position.  A value of
    zero places the vertical scroll window at the top edge of the associated
    element.  If the item does not wrap vertically, a value of 1.0 places the
    vertical scroll window at the bottom edge of the associated element; if the
    item wraps vertically, a value of 1.0 corresponds to wrapping back to the
    left edge of the associated element.  The function must accept no arguments
    and return a floating-point value.  Call with ``nil`` to restore the default
    vertical scroll position callback (based on bindings in the ``yscroll``
    child element in the XML layout file).

    Note that the function must not access the item’s ``scroll_pos_y`` property,
    as this will result in infinite recursion.

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
item.scroll_wrap_x (read-only)
    A Boolean indicating whether the item wraps horizontally.
item.scroll_wrap_y (read-only)
    A Boolean indicating whether the item wraps vertically.
item.scroll_size_x (read/write)
    Get the item’s horizontal scroll window size for the current state, or set
    the horizontal scroll window size to use in the absence of bindings.  This
    is a floating-point value representing a proportion of the associated
    element’s width.
item.scroll_size_y (read/write)
    Get the item’s vertical scroll window size for the current state, or set the
    vertical scroll window size to use in the absence of bindings.  This is a
    floating-point value representing a proportion of the associated element’s
    height.
item.scroll_pos_x (read/write)
    Get the item’s horizontal scroll position for the current state, or set the
    horizontal scroll position size to use in the absence of bindings.  This is
    a floating-point value.
item.scroll_pos_y (read/write)
    Get the item’s vertical scroll position for the current state, or set the
    vertical position size to use in the absence of bindings.  This is a
    floating-point value.
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
debugger must be enabled to use the debugger features (usually by passing
``-debug`` on the command line).

.. _luareference-debug-symtable:

Symbol table
~~~~~~~~~~~~

Wrap’s MAME’s ``symbol_table`` class, providing named symbols that can be used
in expressions.  Note that symbol tables can be created and used even when the
debugger is not enabled.

Instantiation
^^^^^^^^^^^^^

emu.symbol_table(machine)
    Creates a new symbol table in the context of the specified machine,
emu.symbol_table(parent, [device])
    Creates a new symbol table with the specified parent symbol table.  If a
    device is specified and it implements ``device_memory_interface``, it will
    be used as the base for looking up address spaces and memory regions.  Note
    that if a device that does not implement ``device_memory_interface`` is
    supplied, it will not be used (address spaces and memory regions will be
    looked up relative to the root device).
emu.symbol_table(device)
    Creates a new symbol table in the context of the specified device.  If the
    device implements ``device_memory_interface``, it will be used as the base
    for looking up address spaces and memory regions.  Note that if a device
    that does not implement ``device_memory_interface`` is supplied, it will
    only be used to determine the machine context (address spaces and memory
    regions will be looked up relative to the root device).

Methods
^^^^^^^

symbols:set_memory_modified_func(cb)
    Set a function to call when memory is modified via the symbol table.  No
    arguments are passed to the function and any return values are ignored.
    Call with ``nil`` to remove the callback.
symbols:add(name, [value])
    Adds a named integer symbol.  The name must be a string.  If a value is
    supplied, it must be an integer.  If a value is supplied, a read-only symbol
    is added with the supplied value.  If no value is supplied, a read/write
    symbol is created with and initial value of zero.  If a symbol entry with
    the specified name already exists in the symbol table, it will be replaced.

    Returns the new :ref:`symbol entry <luareference-debug-symentry>`.
symbols:add(name, getter, [setter], [format])
    Adds a named integer symbol using getter and optional setter callbacks.  The
    name must be a string.  The getter must be a function returning an integer
    for the symbol value.  If supplied, the setter must be a function that
    accepts a single integer argument for the new value of the symbol.  A format
    string for displaying the symbol value may optionally be supplied.  If a
    symbol entry with the specified name already exists in the symbol table, it
    will be replaced.

    Returns the new :ref:`symbol entry <luareference-debug-symentry>`.
symbols:add(name, minparams, maxparams, execute)
    Adds a named function symbol.  The name must be a string.  The minimum and
    maximum numbers of parameters must be integers.  If a symbol entry with the
    specified name already exists in the symbol table, it will be replaced.

    Returns the new :ref:`symbol entry <luareference-debug-symentry>`.
symbols:find(name)
    Returns the :ref:`symbol entry <luareference-debug-symentry>` with the
    specified name, or ``nil`` if there is no symbol with the specified name in
    the symbol table.
symbols:find_deep(name)
    Returns the :ref:`symbol entry <luareference-debug-symentry>` with the
    specified name, or ``nil`` if there is no symbol with the specified name in
    the symbol table or any of its parent symbol tables.
symbols:value(name)
    Returns the integer value of the symbol with the specified name, or zero if
    there is no symbol with the specified name in the symbol table or any of its
    parent symbol tables.  Raises an error if the symbol with specified name is
    a function symbol.
symbols:set_value(name, value)
    Sets the value of the symbol with the specified name.  Raises an error if
    the symbol with the specified name is a read-only integer symbol or if it is
    a function symbol.  Has no effect if there is no symbol with the specified
    name in the symbol table or any of its parent symbol tables.
symbols:memory_value(name, space, offset, size, disable_se)
    Read a value from memory.  Supply the name or tag of the address space or
    memory region to read from, or ``nil`` to use the address space or memory
    region implied by the ``space`` argument.  See
    :ref:`memory accesses in debugger expressions <debugger-express-mem>` for
    access type specifications that can be used for the ``space`` argument.
    The access size is specified in bytes, and must be 1, 2, 4 or 8.  The
    ``disable_se`` argument specifies whether memory access side effects should
    be disabled.
symbols:set_memory_value(name, space, offset, value, size, disable_se)
    Write a value to memory.  Supply the name or tag of the address space or
    memory region to write to, or ``nil`` to use the address space or memory
    region implied by the ``space`` argument.  See
    :ref:`memory accesses in debugger expressions <debugger-express-mem>` for
    access type specifications that can be used for the ``space`` argument.
    The access size is specified in bytes, and must be 1, 2, 4 or 8.  The
    ``disable_se`` argument specifies whether memory access side effects should
    be disabled.

Properties
^^^^^^^^^^

symbols.entries[]
    The :ref:`symbol entries <luareference-debug-symentry>` in the symbol table,
    indexed by name.  The ``at`` and ``index_of`` methods have O(n) complexity;
    all other supported operations have O(1) complexity.
symbols.parent (read-only)
    The parent symbol table, or ``nil`` if the symbol table has no parent.

.. _luareference-debug-expression:

Parsed expression
~~~~~~~~~~~~~~~~~

Wraps MAME’s ``parsed_expression`` class, which represents a tokenised debugger
expression.  Note that parsed expressions can be created and used even when the
debugger is not enabled.

Instantiation
^^^^^^^^^^^^^

emu.parsed_expression(symbols)
    Creates an empty expression that will use the supplied
    :ref:`symbol table <luareference-debug-symtable>` to look up symbols.
emu.parsed_expression(symbols, string, [default_base])
    Creates an expression by parsing the supplied string, looking up symbols in
    the supplied :ref:`symbol table <luareference-debug-symtable>`.  If the
    default base for interpreting integer literals is not supplied, 16 is used
    (hexadecimal).  Raises an error if the string contains syntax errors or uses
    undefined symbols.

Methods
^^^^^^^

expression:set_default_base(base)
    Set the default base for interpreting numeric literals.  The base must be a
    positive integer.
expression:parse(string)
    Parse a debugger expression string.  Replaces the current contents of the
    expression if it is not empty.  Raises an error if the string contains
    syntax errors or uses undefined symbols.  The previous content of the
    expression is not preserved when attempting to parse an invalid expression
    string.
expression:execute()
    Evaluates the expression, returning an unsigned integer result.  Raises an
    error if the expression cannot be evaluated (e.g. calling a function with an
    invalid number of arguments).

Properties
^^^^^^^^^^

expression.is_empty (read-only)
    A Boolean indicating whether the expression contains no tokens.
expression.original_string (read-only)
    The original string that was parsed to create the expression.
expression.symbols (read/write)
    The :ref:`symbol table <luareference-debug-symtable>` used for to look up
    symbols in the expression.

.. _luareference-debug-symentry:

Symbol entry
~~~~~~~~~~~~

Wraps MAME’s ``symbol_entry`` class, which represents an entry in a
:ref:`symbol table <luareference-debug-symtable>`.  Note that symbol entries
must not be used after the symbol table they belong to is destroyed.

Instantiation
^^^^^^^^^^^^^

symbols:add(name, [value])
    Adds an integer symbol to a
    :ref:`symbol table <luareference-debug-symtable>`, returning the new symbol
    entry.
symbols:add(name, getter, [setter], [format])
    Adds an integer symbol to a
    :ref:`symbol table <luareference-debug-symtable>`, returning the new symbol
    entry.
symbols:add(name, minparams, maxparams, execute)
    Adds function symbol to a
    :ref:`symbol table <luareference-debug-symtable>`, returning the new symbol
    entry.

Properties
^^^^^^^^^^

entry.name (read-only)
    The name of the symbol entry.
entry.format (read-only)
    The format string used to convert the symbol entry to text for display.
entry.is_function (read-only)
    A Boolean indicating whether the symbol entry is a callable function.
entry.is_lval (read-only)
    A Boolean indicating whether the symbol entry is an integer symbol that can
    be set (i.e. whether it can be used on the left-hand side of assignment
    expressions).
entry.value (read/write)
    The integer value of the symbol entry.  Attempting to set the value raises
    an error if the symbol entry is read-only.  Attempting to get or set the
    value of a function symbol raises an error.

.. _luareference-debug-manager:

Debugger manager
~~~~~~~~~~~~~~~~

Wraps MAME’s ``debugger_manager`` class, providing the main interface to control
the debugger.

Instantiation
^^^^^^^^^^^^^

manager.machine.debugger
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

manager.machine.devices[tag].debug
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
    if the expression evaluates to a non-zero value.  The variable 'wpaddr' is
    set to the address that actually triggered the watchpoint, the variable
    'wpdata' is set to the data that is being read or written, and the variable
    'wpsize' is set to the size of the data in bytes.  If the condition is not
    specified, it defaults to always active.
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

manager.machine.devices[tag].debug:bplist()[bp]
    Gets the specified breakpoint for an emulated CPU device, or ``nil`` if no
    breakpoint corresponds to the specified index.

Properties
^^^^^^^^^^

breakpoint.index (read-only)
    The breakpoint’s index.  The can be used to enable, disable or clear the
    breakpoint via the
    :ref:`CPU debugger interface <luareference-debug-devdebug>`.
breakpoint.enabled (read/write)
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

manager.machine.devices[tag].debug:wplist(space)[wp]
    Gets the specified watchpoint for an address space of an emulated CPU
    device, or ``nil`` if no watchpoint in the address space corresponds to the
    specified index.

Properties
^^^^^^^^^^

watchpoint.index (read-only)
    The watchpoint’s index.  The can be used to enable, disable or clear the
    watchpoint via the
    :ref:`CPU debugger interface <luareference-debug-devdebug>`.
watchpoint.enabled (read/write)
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
