Windows-Specific Commandline Options
====================================

This section contains configuration options that are specific to the native (non-SDL) Windows version of MAME.



Performance options
-------------------

.. _mame-wcommandline-priority:

**-priority** *<priority>*

	Sets the thread priority for the MAME threads. By default the priority is left alone to guarantee proper cooperation with other applications. The valid range is -15 to 1, with 1 being the highest priority. The default is *0* (*NORMAL priority*).

.. _mame-wcommandline-profile:

**-profile** *[n]*

    Enables profiling, specifying the stack depth of *[n]* to track.


Full screen options
-------------------

.. _mame-wcommandline-triplebuffer:

**-[no]triplebuffer** / **-[no]tb**

	Enables or disables "triple buffering". Normally, MAME just draws directly to the screen, without any fancy buffering. But with this option enabled, MAME creates three buffers to draw to, and cycles between them in order. It attempts to keep things flowing such that one buffer is currently displayed, the second buffer is waiting to be displayed, and the third buffer is being drawn to. **-triplebuffer** will override **-waitvsync**, if the buffer is successfully created. This option does not work with **-video gdi**. The default is OFF (**-notriplebuffer**).

.. _mame-wcommandline-fullscreenbrightness:

**-full_screen_brightness** *<value>* / **-fsb** *<value>*

	Controls the brightness, or black level, of the entire display. The standard value is 1.0. Selecting lower values (down to 0.1) will produce a darkened display, while selecting higher values (up to 2.0) will give a brighter display. Note that not all video cards have hardware to support this option. This option does not work with **-video gdi**. The default is ``1.0``.

.. _mame-wcommandline-fullscreencontrast:

**-full_screen_contrast** *<value>* / **-fsc** *<value>*

	Controls the contrast, or white level, of the entire display. The standard value is 1.0. Selecting lower values (down to 0.1) will produce a dimmer display, while selecting higher values (up to 2.0) will give a more saturated display. Note that not all video cards have hardware to support this option. This option does not work with **-video gdi**. The default is ``1.0``.

.. _mame-wcommandline-fullscreengamma:

**-full_screen_gamma** *<value>* / **-fsg** *<value>*

	Controls the gamma, which produces a potentially nonlinear black to white ramp, for the entire display. The standard value is 1.0, which gives a linear ramp from black to white. Selecting lower values (down to 0.1) will increase the nonlinearity toward black, while selecting higher values (up to 3.0) will push the nonlinearity toward white. Note that not all video cards have hardware to support this option. This option does not work with **-video gdi**. The default is ``1.0``.



Input device options
--------------------

.. _mame-wcommandline-duallightgun:

**-[no]dual_lightgun** / **-[no]dual**

	Controls whether or not MAME attempts to track two lightguns connected at the same time. This option requires -lightgun. This option is a hack for supporting certain older dual lightgun setups. If you have multiple lightguns connected, you will probably just need to enable -mouse and configure each lightgun independently. The default is *OFF* (**-nodual_lightgun**).
