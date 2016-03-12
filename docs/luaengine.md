# Scripting MAME via LUA

## Introduction

It is now possible to externally drive MAME via LUA scripts. 
This feature initially appeared in version 0.148, when a minimal `luaengine`
was implemented. Nowadays, the LUA interface is rich enough
to let you inspect and manipulate devices state, access CPU 
registers, read and write memory, and draw a custom HUD on screen.

Internally, MAME makes extensive use of `luabridge` to implement
this feature: the idea is to transparently expose as many of 
the useful internals as possible.

Finally, a warning: LUA API is not yet declared stable and may 
suddenly change without prior notice.
However, we expose methods to let you know at runtime which API 
version you are running against, and you can introspect most of the
objects at runtime.

## Features

The API is not yet complete, but this is a partial list of capabilities
currently available to LUA scripts:

 * machine metadata (app version, current rom, rom details)
 * machine control (starting, pausing, resetting, stopping)
 * machine hooks (on frame painting and on user events)
 * machine options (hard reset required for options to take affect)
 * devices introspection (device tree listing, memory and register enumeration)
 * screens introspection (screens listing, screen details, frames counting)
 * screen snaps and HUD drawing (text, lines, boxes on multiple screens)
 * memory read/write (8/16/32/64 bits, signed and unsigned)
 * registers and states control (states enumeration, get and set)

## Usage

MAME supports external scripting via LUA (>= 5.3) scripts, either
written on the interactive console or loaded as a file.
To reach the console, just run MAME with `-console`; you will be
greeted by a naked `>` prompt where you can input your script.

To load a whole script at once, store it in a plaintext file and
pass it via the `-autoboot_script`. Please note that script 
loading may be delayed (few seconds by default), but you can
override the default with the `-autoboot_delay` argument.

To control the execution of your code, you can use a loop-based or
an event-based approach. The former is not encouraged as it is
resource-intensive and makes control flow unnecessarily complex.
Instead, we suggest to register custom hooks to be invoked on specific
events (eg. at each frame rendering).

## Walktrough

Let's first run MAME in a terminal to reach the LUA console:
```
$ mame -console YOUR_ROM
M.A.M.E. v0.158 (Feb  5 2015) - Multiple Arcade Machine Emulator
Copyright Nicola Salmoria and the MAME team
Lua 5.3.0  Copyright (C) 1994-2015 Lua.org, PUC-Rio

> 
```

At this point, your game is probably running in demo mode, let's pause it:
```
> emu.pause()
>
```
Even without textual feedback on the console, you'll notice the game is now paused.
In general, commands are quiet and only print back error messages.

You can check at runtime which version of MAME you are running, with:
```
> print(emu.app_name() .. " " .. emu.app_version())
mame 0.158
```

We now start exploring screen related methods. First, let's enumerate available screens:
```
> for i,v in pairs(manager:machine().screens) do print(i) end
:screen
```

`manager:machine()` is the root object of your currently running machine:
we will be using this often. `screens` is a table with all available screens;
most machines only have one main screen.
In our case, the main and only screen is tagged as `:screen`, and we can further
inspect it:
```
> -- let's define a shorthand for the main screen
> s = manager:machine().screens[":screen"]
> print(s:width() .. "x" .. s:height())
320x224
```

We have several methods to draw on the screen a HUD composed of lines, boxes and text:
```
> -- we define a HUD-drawing function, and then call it
> function draw_hud()
>> s:draw_text(40, 40, "foo"); -- (x0, y0, msg)
>> s:draw_box(20, 20, 80, 80, 0, 0xff00ffff); -- (x0, y0, x1, y1, fill-color, line-color)
>> s:draw_line(20, 20, 80, 80, 0xff00ffff); -- (x0, y0, x1, y1, line-color)
>> end
> draw_hud();
```

This will draw some useless art on the screen. However, when unpausing the game, your HUD
needs to be refreshed otherwise it will just disappear. In order to do this, you have to register
your hook to be called on every frame repaint:
```
> emu.sethook(draw_hud, "frame")
```

All colors are expected in ARGB format (32b unsigned), while screen origin (0,0)
normally corresponds to the top-left corner.

Similarly to screens, you can inspect all the devices attached to a
machine:
```
> for k,v in pairs(manager:machine().devices) do print(k) end
:audiocpu
:maincpu
:saveram
:screen
:palette
[...]
```

On some of them, you can also inspect and manipulate memory and state:
```
> cpu = manager:machine().devices[":maincpu"]
> -- enumerate, read and write state registers
> for k,v in pairs(cpu.state) do print(k) end
D5
SP
A4
A3
D0
PC
[...]
> print(cpu.state["D0"].value)
303
> cpu.state["D0"].value = 255
> print(cpu.state["D0"].value)
255
```

```
> -- inspect memory
> for k,v in pairs(cpu.spaces) do print(k) end
program
> mem = cpu.spaces["program"] 
> print(mem:read_i8(0xC000))
41
```

manager:options()
manager:machine():options()
manager:machine():ui():options()
```
> opts = manager:machine():options()
> for k, entry in pairs(opts.entries) do print(string.format("%10s: %s\n%11s %s", k, entry:value(), "", entry:description())) end
diff_directory: diff
            directory to save hard drive image differeVnce files
joystick_contradictory: false
            enable contradictory direction digital joystick input at the same time
 scalemode: none
            Scale mode: none, hwblit, hwbest, yv12, yuy2, yv12x2, yuy2x2 (-video soft only)
     oslog: false
            output error.log data to the system debugger
[...]
> print(opts.entries["sleep"]:value())
true
> print(opts.entries["sleep"]:value("invalid"))
Illegal boolean value for sleep: "invalid"; reverting to 1
true
> print(opts.entries["sleep"]:value(false))
false
```

individual screen snapshots
```
> local screen = manager:machine().screens[":screen"]
> screen:snapshot()
saved snap/gridlee/0000.png
> screen:snapshot('%g.png')
saved snap/gridlee.png
```
