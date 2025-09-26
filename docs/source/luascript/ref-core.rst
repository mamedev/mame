.. _luascript-ref-core:

Lua Core Classes
================

Many of MAME’s core classes used to implement an emulation session are available
to Lua scripts.

.. contents::
    :local:
    :depth: 1


.. _luascript-ref-notifiersub:

Notifier subscription
---------------------

Wraps MAME’s ``util::notifier_subscription`` class, which manages a subscription
to a broadcast notification.

Methods
~~~~~~~

subscription:unsubscribe()
    Unsubscribes from notifications.  The subscription will become inactive and
    no future notifications will be received.

Properties
~~~~~~~~~~

subscription.is_active (read-only)
    A Boolean indicating whether the subscription is active.  A subscription
    becomes inactive after explicitly unsubscribing or if the underlying
    notifier is destroyed.


.. _luascript-ref-attotime:

Attotime
--------

Wraps MAME’s ``attotime`` class, which represents a high-precision time
interval.  Attotime values support addition and subtraction with other attotime
values, and multiplication and division by integers.

Instantiation
~~~~~~~~~~~~~

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
~~~~~~~

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
~~~~~~~~~~

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


.. _luascript-ref-mameman:

MAME machine manager
--------------------

Wraps MAME’s ``mame_machine_manager`` class, which holds the running machine, UI
manager, and other global components.

Instantiation
~~~~~~~~~~~~~

manager
    The MAME machine manager is available as a global variable in the Lua
    environment.

Properties
~~~~~~~~~~

manager.machine (read-only)
    The :ref:`running machine <luascript-ref-machine>` for the current emulation
    session.
manager.ui (read-only)
    The :ref:`UI manager <luascript-ref-uiman>` for the current session.
manager.options (read-only)
    The :ref:`emulation options <luascript-ref-emuopts>` for the current
    session.
manager.plugins[] (read-only)
    Gets information about the :ref:`Lua plugins <luascript-ref-plugin>` that
    are present, indexed by name.  The index get, ``at`` and ``index_of``
    methods have O(n) complexity.


.. _luascript-ref-machine:

Running machine
---------------

Wraps MAME’s ``running_machine`` class, which represents an emulation session.
It provides access to the other core objects that implement an emulation session
as well as the emulated device tree.

Instantiation
~~~~~~~~~~~~~

manager.machine
    Gets the running machine instance for the current emulation session.

Methods
~~~~~~~

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
~~~~~~~~~~

machine.time (read-only)
    The elapsed emulated time for the current session as an
    :ref:`attotime <luascript-ref-attotime>`.
machine.system (read-only)
    The :ref:`driver metadata <luascript-ref-driver>` for the current
    system.
machine.parameters (read-only)
    The :ref:`parameters manager <luascript-ref-paramman>` for the current
    emulation session.
machine.video (read-only)
    The :ref:`video manager <luascript-ref-videoman>` for the current emulation
    session.
machine.sound (read-only)
    The :ref:`sound manager <luascript-ref-soundman>` for the current emulation
    session.
machine.output (read-only)
    The :ref:`output manager <luascript-ref-outputman>` for the current
    emulation session.
machine.memory (read-only)
    The :ref:`emulated memory manager <luascript-ref-memman>` for the current
    session.
machine.ioport (read-only)
    The :ref:`I/O port manager <luascript-ref-ioportman>` for the current
    emulation session.
machine.input (read-only)
    The :ref:`input manager <luascript-ref-inputman>` for the current emulation
    session.
machine.natkeyboard (read-only)
    Gets the :ref:`natural keyboard manager <luascript-ref-natkbdman>`, used for
    controlling keyboard and keypad input to the emulated system.
machine.uiinput (read-only)
    The :ref:`UI input manager <luascript-ref-uiinputman>` for the current
    emulation session.
machine.render (read-only)
    The :ref:`render manager <luascript-ref-renderman>` for the current
    emulation session.
machine.debugger (read-only)
    The :ref:`debugger manager <luascript-ref-debugman>` for the current
    emulation session, or ``nil`` if the debugger is not enabled.
machine.options (read-only)
    The user-specified :ref:`options <luascript-ref-emuopts>` for the current
    emulation session.
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
    A :ref:`device enumerator <luascript-ref-devenum>` that yields all
    :ref:`devices <luascript-ref-device>` in the emulated system.
machine.palettes (read-only)
    A :ref:`device enumerator <luascript-ref-devenum>` that yields all
    :ref:`palette devices <luascript-ref-dipalette>` in the emulated system.
machine.screens (read-only)
    A :ref:`device enumerator <luascript-ref-devenum>` that yields all
    :ref:`screen devices <luascript-ref-screendev>` in the emulated system.
machine.cassettes (read-only)
    A :ref:`device enumerator <luascript-ref-devenum>` that yields all
    :ref:`cassette image devices <luascript-ref-cassdev>` in the emulated
    system.
machine.images (read-only)
    A :ref:`device enumerator <luascript-ref-devenum>` that yields all
    :ref:`media image devices <luascript-ref-diimage>` in the emulated system.
machine.slots (read-only)
    A :ref:`device enumerator <luascript-ref-devenum>` that yields all
    :ref:`slot devices <luascript-ref-dislot>` in the emulated system.


.. _luascript-ref-videoman:

Video manager
-------------

Wraps MAME’s ``video_manager`` class, which is responsible for coordinating
emulated video drawing, speed throttling, and reading host inputs.

Instantiation
~~~~~~~~~~~~~

manager.machine.video
    Gets the video manager for the current emulation session.

Methods
~~~~~~~

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
~~~~~~~~~~

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
    The :ref:`render target <luascript-ref-rendertarget>` used to produce
    snapshots and video recordings.


.. _luascript-ref-soundman:

Sound manager
-------------

Wraps MAME’s ``sound_manager`` class, which manages the emulated sound stream
graph and coordinates sound output.

Instantiation
~~~~~~~~~~~~~

manager.machine.sound
    Gets the sound manager for the current emulation session.

Methods
~~~~~~~

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
~~~~~~~~~~

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
sound.volume (read/write)
    The output volume in decibels.  Should generally be a negative or zero.
sound.recording (read-only)
    A Boolean indicating whether sound output is currently being recorded to a
    WAV file.


.. _luascript-ref-outputman:

Output manager
--------------

Wraps MAME’s ``output_manager`` class, providing access to system outputs that
can be used for interactive artwork or consumed by external programs.

Instantiation
~~~~~~~~~~~~~

manager.machine.output
    Gets the output manager for the current emulation session.

Methods
~~~~~~~

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


.. _luascript-ref-paramman:

Parameters manager
------------------

Wraps MAME’s ``parameters_manager`` class, which provides a simple key-value
store for metadata from system ROM definitions.

Instantiation
~~~~~~~~~~~~~

manager.machine.parameters
    Gets the parameters manager for the current emulation session.

Methods
~~~~~~~

parameters:lookup(tag)
    Gets the value for the specified parameter if it is set, or an empty string
    if it is not set.
parameters:add(tag, value)
    Sets the specified parameter if it is not set.  Has no effect if the
    specified parameter is already set.


.. _luascript-ref-uiman:

UI manager
----------

Wraps MAME’s ``mame_ui_manager`` class, which handles menus and other user
interface functionality.

Instantiation
~~~~~~~~~~~~~

manager.ui
    Gets the UI manager for the current session.

Methods
~~~~~~~

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
    :ref:`input sequence <luascript-ref-inputseq>` for the specified input type
    and player suitable for using in prompts.  The input type is an enumerated
    value.  The player number is a zero-based index.  If the player number is
    not supplied, it is assumed to be zero.

Properties
~~~~~~~~~~

ui.options (read-only)
    The UI :ref:`options <luascript-ref-coreopts>` for the current session.
ui.line_height (read-only)
    The configured UI text line height as a proportion of the height of the UI
    container.
ui.menu_active (read-only)
    A Boolean indicating whether an interactive UI element is currently active.
    Examples include menus and slider controls.
ui.ui_active (read/write)
    A Boolean indicating whether UI control inputs are currently enabled.
ui.single_step (read/write)
    A Boolean controlling whether the emulated system should be automatically
    paused when the next frame is drawn.  This property is automatically reset
    when the automatic pause happens.
ui.show_fps (read/write)
    A Boolean controlling whether the current emulation speed and frame skipping
    settings should be displayed.
ui.show_profiler (read/write)
    A Boolean controlling whether profiling statistics should be displayed.


.. _luascript-ref-driver:

System driver metadata
----------------------

Provides some metadata for an emulated system.

Instantiation
~~~~~~~~~~~~~

emu.driver_find(name)
    Gets the driver metadata for the system with the specified short name, or
    ``nil`` if no such system exists.
manager.machine.system
    Gets the driver metadata for the current system.

Properties
~~~~~~~~~~

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


.. _luascript-ref-plugin:

Lua plugin
----------

Provides a description of an available Lua plugin.

Instantiation
~~~~~~~~~~~~~

manager.plugins[name]
    Gets the description of the Lua plugin with the specified name, or ``nil``
    if no such plugin is available

Properties
~~~~~~~~~~

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
