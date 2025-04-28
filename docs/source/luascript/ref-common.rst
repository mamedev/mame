.. _luascript-ref-common:

Lua Common Types and Globals
============================

.. contents::
    :local:
    :depth: 1


.. _luascript-ref-containers:

Containers
----------

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
    Iterate over container by key and value.  The key is what you would pass to
    the index operator or the ``get`` method to get the value.
ipairs(c)
    Iterate over container by index and value.  The index is what you would pass
    to the ``at`` method to get the value (this may be the same as the key for
    some containers).
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


.. _luascript-ref-emu:

Emulator interface
------------------

The emulator interface ``emu`` provides access to core functionality.  Many
classes are also available as properties of the emulator interface.

Methods
~~~~~~~

emu.wait(duration, …)
    Yields for the specified duration in terms of emulated time.  The duration
    may be specified as an :ref:`attotime <luascript-ref-attotime>` or a number
    in seconds.  Any additional arguments are returned from the coroutine.
    Returns a Boolean indicating whether the duration expired normally.

    All outstanding calls to ``emu.wait`` will return ``false`` immediately if a
    saved state is loaded or the emulation session ends.  Calling this function
    from callbacks that are not run as coroutines will raise an error.
emu.wait_next_update(…)
    Yields until the next video/UI update.  Any arguments are returned from the
    coroutine.  Calling this function from callbacks that are not run as coroutines
    will raise an error.
emu.wait_next_frame(…)
    Yields until the next emulated frame completes.  Any arguments are returned
    from the coroutine.  Calling this function from callbacks that are not run as
    coroutines will raise an error.
emu.add_machine_reset_notifier(callback)
    Add a callback to receive notifications when the emulated system is reset.
    Returns a :ref:`notifier subscription <luascript-ref-notifiersub>`.
emu.add_machine_stop_notifier(callback)
    Add a callback to receive notifications when the emulated system is stopped.
    Returns a :ref:`notifier subscription <luascript-ref-notifiersub>`.
emu.add_machine_pause_notifier(callback)
    Add a callback to receive notifications when the emulated system is paused.
    Returns a :ref:`notifier subscription <luascript-ref-notifiersub>`.
emu.add_machine_resume_notifier(callback)
    Add a callback to receive notifications when the emulated system is resumed
    after being paused.  Returns a
    :ref:`notifier subscription <luascript-ref-notifiersub>`.
emu.add_machine_frame_notifier(callback)
    Add a callback to receive notifications when an emulated frame completes.
    Returns a :ref:`notifier subscription <luascript-ref-notifiersub>`.
emu.add_machine_pre_save_notifier(callback)
    Add a callback to receive notification before the emulated system state is
    saved.  Returns a
    :ref:`notifier subscription <luascript-ref-notifiersub>`.
emu.add_machine_post_load_notifier(callback)
    Add a callback to receive notification after the emulated system is restored
    to a previously saved state.  Returns a
    :ref:`notifier subscription <luascript-ref-notifiersub>`.
emu.register_sound_update(callback)
    Add a callback to receive new samples that have been created.  The samples
    are coming from the sound devices for which the hook property has been set
    to true.  The callback gets one parameter which is a hash with device tag
    as key and a (channel-sized) vector of (buffer-sized) vector of samples
    in the -1..1 range.
emu.print_error(message)
    Print an error message.
emu.print_warning(message)
    Print a warning message.
emu.print_info(message)
    Print an informational message.
emu.print_verbose(message)
    Print a verbose diagnostic message (disabled by default).
emu.print_debug(message)
    Print a debug message (only enabled for debug builds by default).
emu.lang_translate([context], message)
    Look up a message with optional context in the current localised message
    catalog.  Returns the message unchanged if no corresponding localised
    message is found.
emu.subst_env(string)
    Substitute environment variables in a string.  The syntax is dependent on
    the host operating system.
