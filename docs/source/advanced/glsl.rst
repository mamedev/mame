GLSL Effects for \*nix, OS X, and Windows
=========================================

By default, MAME outputs an idealized version of the video as it would be on the way to the arcade cabinet's monitor, with minimal modification of the output (primarily to stretch the game image back to the aspect ratio the monitor would traditionally have, usually 4:3) -- this works well, but misses some of the nostalgia factor. Arcade monitors were never ideal, even in perfect condition, and the nature of a CRT display distorts that image in ways that change the appearance significantly.

Modern LCD monitors simply do not look the same, and even computer CRT monitors cannot match the look of an arcade monitor without help.

That's where GLSL comes into the picture.

GLSL simulates most of the effects that a CRT arcade monitor has on the video, making the result look a lot more authentic. However, GLSL requires some effort on the user's part: the settings you use are going to be tailored to your PC's system specs, and especially the monitor you're using. Additionally, there were hundreds of thousands of monitors out there in arcades. Each was tuned and maintained differently, meaning there is no one correct appearance to judge by either. Basic guidelines will be provided here to help you, but you may also wish to ask for opinions on popular MAME-centric forums.


Resolution and Aspect Ratio
---------------------------

Resolution is a very important subject for GLSL settings. You will want MAME to be using the native resolution of your monitor to avoid additional distortion and lag created by your monitor upscaling the display image.

While most arcade machines used a 4:3 ratio display (or 3:4 for vertically oriented monitors like Pac-Man), it's difficult to find a consumer display that is 4:3 at this point. The good news is that that extra space on the sides isn't wasted. Many arcade cabinets used bezel artwork around the main display, and should you have the necessary artwork files, MAME will display that artwork. Turn the artwork view to Cropped for best results.

Some older LCD displays used a native resolution of 1280x1024, which is a 5:4 aspect ratio. There's not enough extra space to display artwork, and you'll end up with some very slight pillarboxing, but the results will be on-par with a 4:3 monitor.


Getting Started with GLSL
-------------------------

You will need to have followed the initial MAME setup instructions elsewhere in this manual before beginning. Official MAME distributions include GLSL support by default, but do NOT include the GLSL shader files. You will need to obtain the shader files from third party online sources.

Open your ``mame.ini`` in your text editor of choice (e.g. Notepad), and make sure the following options are set correctly:

* ``video opengl``
* ``filter 0``

The former is required because GLSL requires OpenGL support. The latter turns off extra filtering that interferes with GLSL output.

Lastly, one more edit will turn GLSL on:

* ``gl_glsl 1``

Save the .INI file and you're ready to begin.


Tweaking GLSL Settings inside MAME
----------------------------------

For multiple, complicated to explain reasons, GLSL settings are no longer saved when you exit MAME. This means that while tweaking settings is a little more work on your part, the results will always come out as expected.

Start by loading MAME with the game of your choice (e.g. **mame pacman**)

The tilde key (**~**) brings up the on-screen display options. Use up and down to go through the various settings, while left and right will allow you to change that setting. Results will be shown in real time as you're changing these settings.

Once you've found settings you like, write the numbers down on a notepad and exit MAME.


Configuration Editing
---------------------

As referenced in :ref:`advanced-multi-CFG`, MAME has a order in which it processes INI files. The GLSL settings can be edited in ``mame.ini``, but to take full advantage of the power of MAME's config files, you'll want to copy the GLSL settings from mame.ini to one of the other config files and make changes there.

For instance, once you've found GLSL settings you think are appropriate for Neo Geo games, you can put those settings into neogeo.ini so that all Neo-Geo games will be able to take advantage of those settings without needing to add it to every game INI manually.


Configuration Settings
----------------------

| **gl_glsl**
|
| 	Enables GLSL when set to 1, disabled if set to 0. Defaults to *0*.
|
| **gl_glsl_filter**
|
| 	Enables filtering to GLSL output. Reduces jagginess at the cost of blurriness.
|
| **glsl_shader_mame0**
|         ...
| **glsl_shader_mame9**
|
| 	Specifies the shaders to run, in the order from 0 to 9. See your shader pack author for details on which to run in which order for best effect.
|
| **glsl_shader_screen0**
|         ...
| **glsl_shader_screen9**
|
| 	Specifies screen to apply the shaders on.
|
