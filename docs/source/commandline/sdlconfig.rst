SDL-Specific Commandline Options
================================


This section contains configuration options that are specific to any build supported by SDL (including Windows where compiled as SDL instead of native).



Performance Options
-------------------

.. _mame-scommandline-sdlvideofps:

**-[no]sdlvideofps**

	Enable output of benchmark data on the SDL video subsystem, including your system's video driver, X server (if applicable), and OpenGL stack in **-video opengl** mode.


Video Options
-------------

.. _mame-scommandline-centerh:

**-[no]centerh**

	Center horizontally within the view area. Default is ON (**-centerh**).

.. _mame-scommandline-centerv:

**-[no]centerv**

	Center vertically within the view area. Default is ON (**-centerv**).


Video Soft-Specific Options
---------------------------

.. _mame-scommandline-scalemode:

**-scalemode**

	Scale mode: none, async, yv12, yuy2, yv12x2, yuy2x2 (**-video soft** only). Default is *none*.


SDL Keyboard Mapping
--------------------

.. _mame-scommandline-keymap:

**-keymap**

	Enable keymap. Default is OFF (**-nokeymap**)

.. _mame-scommandline-keymapfile:

**-keymap_file** *<file>*

	Keymap Filename. Default is ``keymap.dat``.


SDL Joystick Mapping
--------------------

.. _mame-scommandline-joyidx:

|
| **-joy_idx1** *<name>*
| **-joy_idx2** *<name>*
| ...
| **-joy_idx8** *<name>*
|

Name of joystick mapped to a given joystick slot, default is *auto*.


.. _mame-scommandline-sixaxis:

**-sixaxis**

	Use special handling for PS3 SixAxis controllers. Default is OFF (**-nosixaxis**)


SDL Low-level Driver Options
~---------------------------

.. _mame-scommandline-videodriver:

**-videodriver** *<driver>*

	SDL video driver to use ('x11', 'directfb', ... or '*auto*' for SDL default)

.. _mame-scommandline-audiodriver:

**-audiodriver** *<driver>*

	SDL audio driver to use ('alsa', 'arts', ... or '*auto*' for SDL default)

.. _mame-scommandline-gllib:

**-gl_lib** *<driver>*

	Alternative **libGL.so** to use; '*auto*' for system default
