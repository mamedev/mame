SDL-Specific Command-line Options
=================================

This section contains configuration options that are specific to any build
supported by SDL (including Windows when built with SDL instead of native).



Performance Options
-------------------

.. _mame-scommandline-sdlvideofps:

**-[no]sdlvideofps**

    Enable output of benchmark data on the SDL video subsystem, including your
    system’s video driver, X server (if applicable), and OpenGL stack in
    **-video opengl** mode.


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

    Scale mode: none, async, yv12, yuy2, yv12x2, yuy2x2 (**-video soft** only).
    Default is *none*.


SDL Keyboard Mapping
--------------------

.. _mame-scommandline-keymap:

**-keymap**

    Enable keymap.  Default is OFF (**-nokeymap**)

.. _mame-scommandline-keymapfile:

**-keymap_file** *<file>*

    Keymap file name.  Default is ``keymap.dat``.


SDL Input Options
--------------------

.. _mame-scommandline-enabletouch:

**-enable_touch**

    Enable support for touch input.  If this option is switched off, mouse input
    simulated from touch devices will be used instead.  Default is OFF
    (**-noenable_touch**)

.. _mame-scommandline-sixaxis:

**-sixaxis**

    Use special handling for PlayStation 3 SixAxis controllers.  May cause
    undesirable behaviour with other controllers.  Only affects the ``sdljoy``
    joystick provider.  Default is OFF (**-nosixaxis**)


SDL Lightgun Mapping
--------------------

.. _mame-scommandline-lightgunindex:

|
| **-lightgun_index1** *<name>*
| **-lightgun_index2** *<name>*
| ...
| **-lightgun_index8** *<name>*
|

Device name or ID mapped to a given lightgun slot.


SDL Low-level Driver Options
----------------------------

.. _mame-scommandline-videodriver:

**-videodriver** *<driver>*

    SDL video driver to use ('x11', 'directfb', ... or '*auto*' for SDL default)

.. _mame-scommandline-audiodriver:

**-audiodriver** *<driver>*

    SDL audio driver to use ('alsa', 'arts', ... or '*auto*' for SDL default)

.. _mame-scommandline-gllib:

**-gl_lib** *<driver>*

    Alternative **libGL.so** to use; '*auto*' for system default
