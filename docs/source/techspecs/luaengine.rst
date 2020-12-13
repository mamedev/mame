.. _luaengine:

Scripting MAME via Lua
======================

.. contents:: :local:


.. _luaengine-intro:

Introduction
------------

It is now possible to externally drive MAME via Lua scripts.  This feature
initially appeared in version 0.148, when a minimal Lua engine was implemented.
Today, the Lua interface is rich enough to let you inspect and manipulate
devices’ state, access CPU registers, read and write memory, and draw a custom
HUD on screen.

Internally, MAME makes extensive use of `Sol3 <https://github.com/ThePhD/sol2>`_
to implement this feature.  The idea is to transparently expose as many of the
useful internals as possible.

Finally, a warning: the Lua API is not yet declared stable and may suddenly
change without prior notice.  However, we expose methods to let you know at
runtime which API version you are running against, and most of the objects
support runtime you can introspection.


.. _luaengine-features:

Features
--------

The API is not yet complete, but this is a partial list of capabilities
currently available to Lua scripts:

-  machine metadata (app version, current emulated system, ROM details)
-  machine control (starting, pausing, resetting, stopping)
-  machine hooks (on frame painting and on user events)
-  device introspection (device tree listing, memory and register enumeration)
-  screen introspection (screens listing, screen details, frame counting)
-  screen HUD drawing (text, lines, boxes on multiple screens)
-  memory read/write (8/16/32/64 bits, signed and unsigned)
-  register and state control (state enumeration, get and set)


.. _luaengine-usage:

Usage
-----

MAME supports external scripting via Lua (>= 5.3) scripts, either written on the
interactive console or loaded as a file. To reach the console, enable the
console plugin (e.g. run MAME with ``-plugin console``) and you will be greeted
by a ``[MAME]>`` prompt where you can enter your script.

To load a whole script at once, store it in a plain text file and pass it using
``-autoboot_script``. Please note that script loading may be delayed (a few
seconds by default), but you can override the default with the
``-autoboot_delay`` option.

To control the execution of your code, you can use a loop-based or event-based
approach.  The former is not encouraged as it is resource-intensive and makes
control flow unnecessarily complex.  Instead, we suggest to register custom
hooks to be invoked on specific events (e.g. at each frame rendering).


.. _luaengine-walkthrough:

Walkthrough
-----------

Let's first run MAME in a terminal to reach the Lua console:

::

    $ mame -console YOUR_ROM
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

    mame 0.226
    Copyright (C) Nicola Salmoria and the MAME team

    Lua 5.3
    Copyright (C) Lua.org, PUC-Rio

    [MAME]>

At this point, your game is probably running in demo mode, let's pause it:

::

    [MAME]> emu.pause()
    [MAME]>

Even without textual feedback on the console, you'll notice the game is now
paused.  In general, commands are quiet and only print back error messages.

You can check at runtime which version of MAME you are running, with:

::

    [MAME]> print(emu.app_name() .. " " .. emu.app_version())
    mame 0.226

We now start exploring screen related methods.  First, let's enumerate available
screens:

::

    [MAME]> for tag, screen in pairs(manager:machine().screens) do print(tag) end
    :screen

``manager:machine()`` is the root object of your currently running machine: we
will be using this often.  ``screens`` is a table with all available screens;
most machines only have one main screen.  In our case, the main and only screen
is tagged as ``:screen``, and we can further inspect it:

::

    [MAME]> -- keep a reference to the main screen in a variable
    [MAME]> s = manager:machine().screens[":screen"]
    [MAME]> print(s:width() .. "x" .. s:height())
    320x224

We have several methods to draw a HUD on the screen composed of lines, boxes and
text:

::

    [MAME]> -- we define a HUD-drawing function, and then call it
    [MAME]> function draw_hud()
    [MAME]>> s:draw_text(40, 40, "foo"); -- (x0, y0, msg)
    [MAME]>> s:draw_box(20, 20, 80, 80, 0, 0xff00ffff); -- (x0, y0, x1, y1, fill-color, line-color)
    [MAME]>> s:draw_line(20, 20, 80, 80, 0xff00ffff); -- (x0, y0, x1, y1, line-color)
    [MAME]>> end
    [MAME]> draw_hud();

This will draw some useless art on the screen.  However, when resuming the game,
your HUD needs to be refreshed otherwise it will just disappear.  In order to do
this, you have to register your hook to be called on every frame repaint:

::

    [MAME]> emu.register_frame_done(draw_hud, "frame")

All colors are specified in ARGB format (eight bits per channel), while screen
origin (0,0) normally corresponds to the top-left corner.

Similarly to screens, you can inspect all the devices attached to a machine:

::

    [MAME]> for tag, device in pairs(manager:machine().devices) do print(tag) end
    :audiocpu
    :maincpu
    :saveram
    :screen
    :palette
    [...]

On some of them, you can also inspect and manipulate memory and state:

::

    [MAME]> cpu = manager:machine().devices[":maincpu"]
    [MAME]> -- enumerate, read and write state registers
    [MAME]> for k, v in pairs(cpu.state) do print(k) end
    D5
    SP
    A4
    A3
    D0
    PC
    [...]
    [MAME]> print(cpu.state["D0"].value)
    303
    [MAME]> cpu.state["D0"].value = 255
    [MAME]> print(cpu.state["D0"].value)
    255

::

    [MAME]> -- inspect memory
    [MAME]> for name, space in pairs(cpu.spaces) do print(name) end
    program
    [MAME]> mem = cpu.spaces["program"]
    [MAME]> print(mem:read_i8(0xc000))
    41
