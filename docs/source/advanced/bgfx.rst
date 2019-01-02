BGFX Effects for (nearly) Everyone
==================================

By default, MAME outputs an idealized version of the video as it would be on the way to the arcade cabinet's monitor, with minimal modification of the output (primarily to stretch the game image back to the aspect ratio the monitor would traditionally have, usually 4:3) -- this works well, but misses some of the nostalgia factor. Arcade monitors were never ideal, even in perfect condition, and the nature of a CRT display distorts that image in ways that change the appearance significantly.

Modern LCD monitors simply do not look the same, and even computer CRT monitors cannot match the look of an arcade monitor without help.

That's where the new BGFX renderer with HLSL comes into the picture.

HLSL simulates most of the effects that a CRT arcade monitor has on the video, making the result look a lot more authentic. However, HLSL requires some effort on the user's part: the settings you use are going to be tailored to your PC's system specs, and especially the monitor you're using. Additionally, there were hundreds of thousands of monitors out there in arcades. Each was tuned and maintained differently, meaning there is no one correct appearance to judge by either. Basic guidelines will be provided here to help you, but you may also wish to ask for opinions on popular MAME-centric forums.


Resolution and Aspect Ratio
---------------------------

Resolution is a very important subject for HLSL settings. You will want MAME to be using the native resolution of your monitor to avoid additional distortion and lag created by your monitor upscaling the display image.

While most arcade machines used a 4:3 ratio display (or 3:4 for vertically oriented monitors like Pac-Man), it's difficult to find a consumer display that is 4:3 at this point. The good news is that that extra space on the sides isn't wasted. Many arcade cabinets used bezel artwork around the main display, and should you have the necessary artwork files, MAME will display that artwork. Turn the artwork view to Cropped for best results.

Some older LCD displays used a native resolution of 1280x1024 and were a 5:4 aspect ratio. There's not enough extra space to display artwork, and you'll end up with some very slight pillarboxing, but the results will be still be good and on-par with a 4:3 monitor.


Getting Started with BGFX
-------------------------

You will need to have followed the initial MAME setup instructions elsewhere in this manual before beginning. Official MAME distributions include BGFX as of MAME 0.172, so you don't need to download any additional files.

Open your ``mame.ini`` in your text editor of choice (e.g. Notepad), and make sure the following options are set correctly:

* ``video bgfx``

Now, you may want to take a moment to look below at the Configuration Settings section to see how to set up these next options.

As referenced in :ref:`advanced-multi-CFG`, MAME has a order in which it processes INI files. The BGFX settings can be edited in ``mame.ini``, but to take full advantage of the power of MAME's config files, you'll want to copy the BGFX settings from ``mame.ini`` to one of the other config files and make changes there.)

In particular, you will want the ``bgfx_screen_chains`` to be specific to each game.

Save your .INI file(s) and you're ready to begin.

Configuration Settings
----------------------

| **bgfx_path**
|
| 	This is where your BGFX shader files are stored. By default, this will be the BGFX folder in your MAME installation.
|
| **bgfx_backend**
|
|	Selects a rendering backend for BGFX to use. Possible choices include ``d3d9``, ``d3d11``, ``opengl``, and ``metal``. The default is ``**auto**``, which will let MAME choose the best selection for you.
|
|	``d3d9`` -- Direct3D 9.0 Renderer (Requires Windows XP or higher)
|	``d3d11`` -- Direct3D 11.0 Renderer (Requires Windows Vista with D3D11 update or Windows 7 or higher)
|	``opengl`` -- OpenGL Renderer (Requires OpenGL drivers, may work better on some poorly designed video cards, supported on Linux/Mac OS X)
|	``metal`` -- Metal Apple Graphics API (Requires OS X 10.11 El Capitan or newer)
|
| **bgfx_debug**
|
|	Enables BGFX debugging features. Most users will not need to use this.
|
| **bgfx_screen_chains**
|
|	This dictates how to handle BGFX rendering on a per-display basis. Possible choices include ``hlsl``, ``unfiltered``, and ``default``.
|
|	``default`` -- **default** bilinear filterered output
|	``unfiltered`` -- nearest neighbor unfiltered output
|	``hlsl`` -- HLSL display simulation through shaders
|
|	We make a distinction between emulated device screens (which we'll call a **screen**) and physical displays (which we'll call a **window**, set by ``-numscreens``) here. We use colons (:) to seperate windows, and commas (,) to seperate screens. Commas always go on the outside of the chain (see House Mannequin example)
|
|	On a combination of a single window, single screen case, such as Pac-Man on one physical PC monitor, you can specify one entry like:
|
|		**bgfx_screen_chains hlsl**
|
|	Things get only slightly more complicated when we get to multiple windows and multiple screens.
|
|	On a single window, multiple screen game, such as Darius on one physical PC monitor, specify multiple entries (one per window) like:
|
|		bgfx_screen_chains hlsl,hlsl,hlsl
|
|	This also works with single screen games where you are mirroring the output to more than one physical display. For instance, you could set up Pac-Man to have one unfiltered output for use with video broadcasting while a second display is set up HLSL for playing on.
|
|	On a mulitple window, multiple screen game, such as Darius on three physical PC monitors, specify multiple entries (one per window) like:
|
|		``bgfx_screen_chains hlsl:hlsl:hlsl``
|
|	Another example game would be Taisen Hot Gimmick, which used two CRTs to show individual player hands to just that player. If using two windows (two physical displays):
|
|		``bgfx_screen_chains hlsl:hlsl``
|
|	One more special case is that Nichibutsu had a special cocktail mahjongg cabinet that used a CRT in the middle along with two LCD displays to show each player their hand. We would want the LCDs to be unfiltered and untouched as they were, while the CRT would be improved through HLSL. Since we want to give each player their own full screen display (two physical monitors) along with the LCD, we'll go with:
|
|		**-numscreens 2 -view0 "Player 1" -view1 "Player 2" -video bgfx -bgfx_screen_chains hlsl,unfiltered,unfiltered:hlsl,unfiltered,unfiltered**
|
|	This sets up the view for each display respectively, keeping HLSL effect on the CRT for each window (physical display) while going unfiltered for the LCD screens.
|
|	If using only one window (one display), keep in mind the game still has three screens, so we would use:
|
|		``bgfx_screen_chains hlsl,unfiltered,unfiltered``
|
|
|	Note that the commas are on the outside edges, and any colons are in the middle.
|
| ``bgfx_shadow_mask``
|
|	This specifies the shadow mask effect PNG file. By default this is ``**slot-mask.png**``.
|
|


Tweaking BGFX HLSL Settings inside MAME
---------------------------------------

*Warning: Currently BGFX HLSL settings are not saved or loaded from any configuration files. This is expected to change in the future.*

Start by loading MAME with the game of your choice (e.g. **mame pacman**)

The tilde key (**~**) brings up the on-screen display options. Use up and down to go through the various settings, while left and right will allow you to change that setting. Results will be shown in real time as you're changing these settings.

Note that settings are individually changable on a per-screen basis.
