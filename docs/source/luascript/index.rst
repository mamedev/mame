.. _luascript:

Lua Scripting Interface
=======================

.. contents:: :local:


.. _luascript-intro:

Introduction
------------

MAME provides Lua script bindings for a useful set of core functionality.  This
feature first appeared in version 0.148, when a minimal Lua interface was
implemented.  Today, the Lua interface is rich enough to let you inspect and
manipulate device state, access CPU registers, read and write memory, and draw
custom graphical overlays.

There are three ways to use MAME’s Lua scripting capabilities:

* Using the :ref:`interactive Lua console <luascript-console>`, enabled by the
  :ref:`console option <mame-commandline-console>`.
* By providing a script file to run using the :ref:`-autoboot_script option
  <mame-commandline-autobootscript>`.  The :ref:`-autoboot_delay option
  <mame-commandline-autobootdelay>` controls how long MAME waits after starting
  the emulated system before running the script.
* By writing :ref:`Lua plugins <plugins>`.  Several plugins are included with
  MAME.

Internally, MAME makes extensive use of `Sol3 <https://github.com/ThePhD/sol2>`_
to implement Lua bindings.

The Lua API is not yet declared stable and may suddenly change without prior
notice.  However, we expose methods to let you know at runtime which API version
you are running against, and most objects support some level of runtime
introspection.


.. _luascript-features:

Features
--------

The API is not yet complete, but this is a partial list of capabilities exposed
to Lua scripts:

*  Session information (application version, current emulated system)
*  Session control (starting, pausing, resetting, stopping)
*  Event hooks (on frame painting and on user events)
*  Device introspection (device tree listing, memory and register enumeration)
*  Screen introspection (screens listing, screen details, frame counting)
*  Screen overlay drawing (text, lines, boxes on multiple screens)
*  Memory read/write (8/16/32/64 bits, signed and unsigned)
*  Register and state control (state enumeration, get and set)


.. _luascript-api:

API reference
-------------

.. toctree::
    :maxdepth: 2

    ref-common
    ref-core
    ref-devices
    ref-mem
    ref-input
    ref-render
    ref-debugger


.. _luascript-console:

Interactive Lua console tutorial
--------------------------------

First run an arcade game in MAME at the command prompt with the ``-console``
and ``-window`` options to enable the Lua console:

::

    $ mame -console -window YOUR_SYSTEM
           /|  /|    /|     /|  /|    _______
          / | / |   / |    / | / |   /      /
         /  |/  |  /  |   /  |/  |  /  ____/
        /       | /   |  /       | /  /_
       /        |/    | /        |/  __/
      /  /|  /|    /| |/  /|  /|    /____
     /  / | / |   / |    / | / |        /
    / _/  |/  /  /  |___/  |/  /_______/
             /  /
            / _/

    mame 0.255
    Copyright (C) Nicola Salmoria and the MAME team

    Lua 5.4
    Copyright (C) Lua.org, PUC-Rio

    [MAME]>

At this point, your game is probably running in attract mode.  Let’s pause it:

::

    [MAME]> emu.pause()
    [MAME]>

Even without textual feedback on the console, you’ll notice the game is now
paused.  In general, commands are quiet and only print error messages.

You can check the version of MAME you are running with:

::

    [MAME]> print(emu.app_name() .. " " .. emu.app_version())
    mame 0.255

Let’s examine the emulated screens.  First, enumerate the :ref:`screen devices
<luascript-ref-screendev>` in the system:

::

    [MAME]> for tag, screen in pairs(manager.machine.screens) do print(tag) end
    :screen

``manager.machine`` is the :ref:`running machine <luascript-ref-machine>` object
for the current emulation session.  We will be using this frequently.
``screens`` is a :ref:`device enumerator <luascript-ref-devenum>` that yields
all emulated screens in the system.  Most arcade games only have one main
screen.  In our case, the main and only screen has the absolute tag ``:screen``.
We can examine it further:

::

    [MAME]> -- keep a reference to the main screen in a variable
    [MAME]> s = manager.machine.screens[':screen']
    [MAME]> print(s.width .. 'x' .. s.height)
    320x224

Several methods are available for drawing an overlay on the screen using lines,
rectangles and text:

::

    [MAME]> -- define a function for drawing an overlay and call it
    [MAME]> function draw_overlay()
    [MAME]>> s:draw_text(40, 40, 'foo') -- (x0, y0, msg)
    [MAME]>> s:draw_box(20, 20, 80, 80, 0xff00ffff, 0) -- (x0, y0, x1, y1, line-color, fill-color)
    [MAME]>> s:draw_line(20, 20, 80, 80, 0xff00ffff) -- (x0, y0, x1, y1, line-color)
    [MAME]>> end
    [MAME]> draw_overlay()

This will draw some useless lines and text over the screen.  However, when the
emulated system is resumed, your overlay needs to be refreshed or it will just
disappear.  In order to do this, you have to register your function to be called
on every video update:

::

    [MAME]> emu.register_frame_done(draw_overlay, 'frame')

All colors are specified in ARGB format (eight bits per channel).  The
coordinate origin (0,0) normally corresponds to the top-left corner of the
screen.

As with screens, you can examine all the emulated devices in the running system:

::

    [MAME]> for tag, device in pairs(manager.machine.devices) do print(tag) end
    :audiocpu
    :maincpu
    :saveram
    :screen
    :palette
    [...]

For some of them, you can also inspect and manipulate memory and state:

::

    [MAME]> cpu = manager.machine.devices[':maincpu']
    [MAME]> -- enumerate, read and write register state
    [MAME]> for k, v in pairs(cpu.state) do print(k) end
    CURPC
    rPC
    IR
    CURFLAGS
    SSR
    D0
    [...]
    [MAME]> print(cpu.state["D0"].value)
    303
    [MAME]> cpu.state['D0'].value = 255
    [MAME]> print(cpu.state['D0'].value)
    255

::

    [MAME]> -- inspect memory
    [MAME]> for name, space in pairs(cpu.spaces) do print(name) end
    program
    cpu_space
    [MAME]> mem = cpu.spaces['program']
    [MAME]> print(mem:read_i8(0xc000))
    41

Note that many objects support symbol completion if you type part of a method or
property name and press the Tab key:

::

    [MAME]>print(mem:read_<TAB>
    read_direct_i8
    read_u16
    read_range
    read_direct_u16
    read_direct_i64
    read_i64
    read_i32
    read_direct_u64
    read_i8
    read_u32
    read_u8
    read_u64
    read_direct_u32
    read_direct_i16
    read_direct_i32
    read_direct_u8
    read_i16
    [MAME]>print(mem:read_direct_i8
