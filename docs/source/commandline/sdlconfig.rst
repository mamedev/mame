SDL-Specific Commandline Options
================================


This section contains configuration options that are specific to any build supported by SDL (including Windows where compiled as SDL instead of native).



Performance Options
-------------------

.. _mame-scommandline-sdlvideofps:

**-sdlvideofps**

	Enable output of benchmark data on the SDL video subsystem, including your system's video driver, X server (if applicable), and OpenGL stack in **-video opengl** mode.


Video Options
-------------

.. _mame-scommandline-centerh:

**-[no]centerh**

	Center horizontally within the view area. Default is ON (*-centerh*).

.. _mame-scommandline-centerv:

**-[no]centerv**

	Center vertically within the view area. Default is ON (*-centerv*).


Video Soft-Specific Options
---------------------------

.. _mame-scommandline-scalemode:

**-scalemode**

	Scale mode: none, async, yv12, yuy2, yv12x2, yuy2x2 (**-video soft** only). Default is '*none*'.



Video OpenGL Debugging Options
------------------------------
	
These 4 options are for compatibility in **-video opengl**.  If you report rendering artifacts you may be asked to try messing with them by the devs, but normally they should be left at their defaults which results in the best possible video performance.	

.. _mame-scommandline-glforcepow2texture:

**-[no]gl_forcepow2texture**

	Always use only power-of-2 sized textures (default *off*)
	
.. _mame-scommandline-glnotexturerect:

**-[no]gl_notexturerect**

	Don't use OpenGL GL_ARB_texture_rectangle (default *on*)

.. _mame-scommandline-glvbo:

**-[no]gl_vbo**

    Enable OpenGL VBO,  if available (default *on*)

.. _mame-scommandline-glpbo:

**-[no]gl_pbo**

    Enable OpenGL PBO,  if available (default *on*)


Video OpenGL GLSL Options
-------------------------
	

.. _mame-scommandline-glglsl:

**-gl_glsl**

	Enable OpenGL GLSL, if available (default *off*)

.. _mame-scommandline-glglslfilter:

**-gl_glsl_filter**

	Enable OpenGL GLSL filtering instead of FF filtering -- *0-plain, 1-bilinear* (default is *1*)

.. _mame-scommandline-glslshadermame:

|
| **-glsl_shader_mame0**
| **-glsl_shader_mame1**
| ...
| **-glsl_shader_mame9**
|

	Custom OpenGL GLSL shader set MAME bitmap in the provided slot (0-9); one can be applied to each slot.

	[todo: better details on usage at some point. See http://forums.bannister.org/ubbthreads.php?ubb=showflat&Number=100988#Post100988 ]



.. _mame-scommandline-glslshaderscreen:

|
| **-glsl_shader_screen0**
| **-glsl_shader_screen1**
| ...
| **-glsl_shader_screen9**
|

	Custom OpenGL GLSL shader screen bitmap in the provided slot (0-9).

	[todo: better details on usage at some point. See http://forums.bannister.org/ubbthreads.php?ubb=showflat&Number=100988#Post100988 ]


.. _mame-scommandline-glglslvidattr:

**-gl_glsl_vid_attr**

	Enable OpenGL GLSL handling of brightness and contrast. Better RGB game performance.  Default is *on*.


SDL Keyboard Mapping
--------------------

.. _mame-scommandline-keymap:

**-keymap**

	Enable keymap. Default is OFF (*-nokeymap*)

.. _mame-scommandline-keymapfile:

**-keymap_file** *<file>*
	
	Keymap Filename. Default is '*keymap.dat*'.


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

	Use special handling for PS3 SixAxis controllers. Default is OFF (*-nosixaxis*)


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

