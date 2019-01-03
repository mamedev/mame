HLSL Effects for Windows
========================

By default, MAME outputs an idealized version of the video as it would be on the way to the arcade cabinet's monitor, with minimal modification of the output (primarily to stretch the game image back to the aspect ratio the monitor would traditionally have, usually 4:3) -- this works well, but misses some of the nostalgia factor. Arcade monitors were never ideal, even in perfect condition, and the nature of a CRT display distorts that image in ways that change the appearance significantly.

Modern LCD monitors simply do not look the same, and even computer CRT monitors cannot match the look of an arcade monitor without help.

That's where HLSL comes into the picture.

HLSL simulates most of the effects that a CRT arcade monitor has on the video, making the result look a lot more authentic. However, HLSL requires some effort on the user's part: the settings you use are going to be tailored to your PC's system specs, and especially the monitor you're using. Additionally, there were hundreds of thousands of monitors out there in arcades. Each was tuned and maintained differently, meaning there is no one correct appearance to judge by either. Basic guidelines will be provided here to help you, but you may also wish to ask for opinions on popular MAME-centric forums.


Resolution and Aspect Ratio
---------------------------

Resolution is a very important subject for HLSL settings. You will want MAME to be using the native resolution of your monitor to avoid additional distortion and lag created by your monitor upscaling the display image.

While most arcade machines used a 4:3 ratio display (or 3:4 for vertically oriented monitors like Pac-Man), it's difficult to find a consumer display that is 4:3 at this point. The good news is that that extra space on the sides isn't wasted. Many arcade cabinets used bezel artwork around the main display, and should you have the necessary artwork files, MAME will display that artwork. Turn the artwork view to Cropped for best results.

Some older LCD displays used a native resolution of 1280x1024 and were a 5:4 aspect ratio. There's not enough extra space to display artwork, and you'll end up with some very slight pillarboxing, but the results will be still be good and on-par with a 4:3 monitor.


Getting Started with HLSL
-------------------------

You will need to have followed the initial MAME setup instructions elsewhere in this manual before beginning. Official MAME distributions include HLSL by default, so you don't need to download any additional files.

Open your ``mame.ini`` in your text editor of choice (e.g. Notepad), and make sure the following options are set correctly:

* **video d3d**
* **filter 0**

The former is required because HLSL requires Direct3D support. The latter turns off extra filtering that interferes with HLSL output.

Lastly, one more edit will turn HLSL on:

* **hlsl_enable 1**

Save the .INI file and you're ready to begin.

Several presets have been included in the INI folder with MAME, allowing for good quick starting points for Nintendo Game Boy, Nintendo Game Boy Advance, Raster, and Vector monitor settings.


Tweaking HLSL Settings inside MAME
----------------------------------

For multiple, complicated to explain reasons, HLSL settings are no longer saved when you exit MAME. This means that while tweaking settings is a little more work on your part, the results will always come out as expected.

Start by loading MAME with the game of your choice (e.g. **mame pacman**)

The tilde key (**~**) brings up the on-screen display options. Use up and down to go through the various settings, while left and right will allow you to change that setting. Results will be shown in real time as you're changing these settings.

Once you've found settings you like, write the numbers down on a notepad and exit MAME.


Configuration Editing
---------------------

As referenced in :ref:`advanced-multi-CFG`, MAME has a order in which it processes INI files. The HLSL settings can be edited in ``mame.ini``, but to take full advantage of the power of MAME's config files, you'll want to copy the HLSL settings from mame.ini to one of the other config files and make changes there.

For instance, once you've found HLSL settings you think are appropriate for Neo Geo games, you can put those settings into ``neogeo.ini`` so that all Neo-Geo games will be able to take advantage of those settings without needing to add it to every game INI manually.


Configuration Settings
----------------------

| **hlslpath**
|
| 	This is where your HLSL files are stored. By default, this will be the HLSL folder in your MAME installation.
|
| **hlsl_snap_width**
| **hlsl_snap_height**
|
| 	Sets the resolution that Alt+F12 HLSL screenshots are output at.
|
| **shadow_mask_alpha** (*Shadow Mask Amount*)
|
| 	This defines how strong the effect of the shadowmask is. Acceptable range is from 0 to 1, where 0 will show no shadowmask effect, 1 will be a completely opaque shadowmask, and 0.5 will be 50% transparent.
|
| **shadow_mask_tile_mode** (*Shadow Mask Tile Mode*)
|
| 	This defines whether the shadowmask should be tiled based on the screen resolution of your monitor or based on the source resolution of the emulated system. Valid values are 0 for *Screen* mode and 1 for *Source* mode.
|
| **shadow_mask_texture**
| **shadow_mask_x_count** (*Shadow Mask Pixel X Count*)
| **shadow_mask_y_count** (*Shadow Mask Pixel Y Count*)
| **shadow_mask_usize** (*Shadow Mask U Size*)
| **shadow_mask_vsize** (*Shadow Mask V Size*)
| **shadow_mask_x_count** (*Shadow Mask U Offset*)
| **shadow_mask_y_count** (*Shadow Mask V Offset*)
|
| 	These settings need to be set in unison with one another. In particular, **shadow_mask_texture** sets rules for how you need to set the other options.
|
| 	**shadow_mask_texture** sets the texture of the shadowmask effect. Three shadowmasks are included with MAME: *aperture-grille.png*, *shadow-mask.png*, and *slot-mask.png*
|
| 	**shadow_mask_usize** and **shadow_mask_vsize** define the used size of the shadow_mask_texture in percentage, staring at the top-left corner. The means for a texture with the actual size of 24x24 pixel and an u/v size of 0.5,0.5 the top-left 12x12 pixel will be used. Keep in mind to define an u/v size that makes is possible to tile the texture without gaps or glitches. 0.5,0.5 is fine for any shadowmask texture that is included with MAME.
|
| 	**shadow_mask_x_count** and **shadow_mask_y_count** define how many screen pixel should be used to display the u/v sized texture. e.g. if you use the example from above and define a x/y count of 12,12 every pixel of the texture will be displayed 1:1 on the screen, if you define a x/y count of 24,24 the texture will be displayed twice as large.
|
| example settings for **shadow_mask.png**:
|
| 	shadow_mask_texture shadow-mask.png
| 	shadow_mask_x_count 12
| 	shadow_mask_y_count 6 or 12
| 	shadow_mask_usize 0.5
| 	shadow_mask_vsize 0.5
|
| example settings for **slot-mask.png**:
|
| 	shadow_mask_texture slot-mask.png
| 	shadow_mask_x_count 12
| 	shadow_mask_y_count 8 or 16
| 	shadow_mask_usize 0.5
| 	shadow_mask_vsize 0.5
|
| example settings for **aperture-grille**:
|
| 	shadow_mask_texture aperture-grille.png
| 	shadow_mask_x_count 12
| 	shadow_mask_y_count 12 or any
| 	shadow_mask_usize 0.5
| 	shadow_mask_vsize 0.5
|
| 	**shadow_mask_uoffset** and **shadow_mask_voffset** can be used to tweak the alignment of the final shadowmask in subpixel range. Range is from -1.00 to 1.00, where 0.5 moves the shadowmask by 50 percent of the u/v sized texture.
|
| **distortion** (*Quadric Distortion Amount*)
|
| 	This setting determines strength of the quadric distortion of the screen image.
|
| **cubic_distortion** (*Cubic Distortion Amount*)
|
| 	This setting determines strength of the qubic distortion of the screen image.
|
|   Both distortion factors can be negative to compensate each other. e.g. distortion 0.5 and cubic_distortion -0.5
|
| **distort_corner** (*Distorted Corner Amount*)
|
| 	This setting determines strength of distortion of the screen corners, which does not affect the distortion of screen image itself.
|
| **round_corner** (*Rounded Corner Amount*)
|
| 	The corners of the display can be rounded off through the use of this setting.
|
| **smooth_border** (*Smooth Border Amount*)
|
| 	Sets a smoothened/blurred border around the edges of the screen.
|
| **reflection** (*Reflection Amount*)
|
| 	If set above 0, this creates a white reflective blotch on the display. By default, this is put in the upper right corner of the display. By editing the POST.FX file's GetSpotAddend section, you can change the location. Range is from 0.00 to 1.00.
|
| **vignetting** (*Vignetting Amount*)
|
| 	When set above 0, will increasingly darken the outer edges of the display in a pseudo-3D effect. Range is from 0.00 to 1.00.
|
| **scanline_alpha** (*Scanline Amount*)
|
| 	This defines how strong the effect of the scanlines are. Acceptable range is from 0 to 1, where 0 will show no scanline effect, 1 will be a completely black line, and 0.5 will be 50% transparent. Note that arcade monitors did not have completely black scanlines.
|
| **scanline_size** (*Overall Scanline Scale*)
|
| 	The overall spacing of the scanlines is set with this option. Setting it at 1 represents consistent alternating spacing between display lines and scanlines.
|
| **scanline_height** (*Individual Scanline Scale*)
|
| 	This determines the overall size of each scanline. Setting lower than 1 makes them thinner, larger than 1 makes them thicker.
|
| **scanline_variation** (*Scanline Variation*)
|
|	This affects the size of each scanline depending on its brightness. Brighter scanlines will be thicker than darker scanline. Acceptable range is from 0 to 2.0, with the default being 1.0. At 0.0 all scanlines will have the same size independent of their brightness.
|
| **scanline_bright_scale** (*Scanline Brightness Scale*)
|
| 	Specifies how bright the scanlines are. Larger than 1 will make them brighter, lower will make them dimmer. Setting to 0 will make scanlines disappear entirely.
|
| **scanline_bright_offset** (*Scanline Brightness Offset*)
|
| 	This will give scanlines a glow/overdrive effect, softening and smoothing the top and bottom of each scanline.
|
| **scanline_jitter** (*Scanline Jitter Amount*)
|
| 	Specifies the wobble or jitter of the scanlines, causing them to jitter on the monitor. Warning: Higher settings may hurt your eyes.
|
| **hum_bar_alpha** (*Hum Bar Amount*)
|
| 	Defines the strength of the hum bar effect.
|
| **defocus** (*Defocus*)
|
| 	This option will defocus the display, blurring individual pixels like an extremely badly maintained monitor. Specify as X,Y values (e.g. **defocus 1,1**)
|
| **converge_x** (*Linear Convergence X, RGB*)
| **converge_y** (*Linear Convergence Y, RGB*)
| **radial_converge_x** (*Radial Convergence X, RGB*)
| **radial_converge_y** (*Radial Convergence Y, RGB*)
|
| 	Adjust the convergence of the red, green, and blue channels in a given direction. Many badly maintained monitors with bad convergence would bleed colored ghosting off-center of a sprite, and this simulates that.
|
| **red_ratio** (*Red Output from RGB*)
| **grn_ratio** (*Green Output from RGB*)
| **blu_ratio** (*Blue Output from RGB*)
|
| 	Defines a 3x3 matrix that is multiplied with the RGB signals to simulate color channel interference. For instance, a green channel of (0.100, 1.000, 0.250) is weakened 10% by the red channel and strengthened 25% through the blue channel.
|
| **offset** (*Signal Offset*)
|
| 	Strengthen or weakens the current color value of a given channel. For instance, a red signal of 0.5 with an offset of 0.2 will be raised to 0.7
|
| **scale** (*Signal Scale*)
|
| 	Applies scaling to the current color value of the channel. For instance, a red signal of 0.5 with a scale of 1.1 will result in a red signal of 0.55
|
| **power** (*Signal Exponent, RGB*)
|
| 	Exponentiate the current color value of the channel, also called gamma. For instance, a red signal of 0.5 with red power of 2 will result in a red signal of 0.25
|
| 	This setting also can be used to adjust line thickness in vector games.
|
| **floor** (*Signal Floor, RGB*)
|
| 	Sets the absolute minimum color value of a channel. For instance, a red signal of 0.0 (total absence of red) with a red floor of 0.2 will result in a red signal of 0.2
|
| 	Typically used in conjunction with artwork turned on to make the screen have a dim raster glow.
|
| **phosphor_life** (*Phosphor Persistence, RGB*)
|
| 	How long the color channel stays on the screen, also called phosphor ghosting. 0 gives absolutely no ghost effect, and 1 will leave a contrail behind that is only overwritten by a higher color value.
|
| 	This also affects vector games quite a bit.
|
| **saturation** (*Color Saturation*)
|
| 	Color saturation can be adjusted here.
|
| **bloom_blend_mode** (*Bloom Blend Mode*)
|
| 	Determines the mode of the bloom effect. Valid values are 0 for *Brighten* mode and 1 for *Darken* mode, last is only useful for systems with STN LCD.
|
| **bloom_scale** (*Bloom Scale*)
|
| 	Determines the intensity of bloom effect. Arcade CRT displays had a tendency towards bloom, where bright colors could bleed out into neighboring pixels. This effect is extremely graphics card intensive, and can be turned completely off to save GPU power by setting it to 0
|
| **bloom_overdrive** (*Bloom Overdrive, RGB*)
|
| 	Sets a RGB color, separated by commas, that has reached the brightest possible color and will be overdriven to white. This is only useful on color raster, color LCD, or color vector games.
|
| **bloom_lvl0_weight** (*Bloom Level 0 Scale*)
| **bloom_lvl1_weight** (*Bloom Level 1 Scale*)
|      .  .  .  .
| **bloom_lvl7_weight** (*Bloom Level 7 Scale*)
| **bloom_lvl8_weight** (*Bloom Level 8 Scale*)
|
| 	These define the bloom effect. Range is from 0.00 to 1.00. If used carefully in conjunction with phosphor_life, glowing/ghosting for moving objects can be achieved.
|
| **hlsl_write**
|
| 	Enables writing of an uncompressed AVI video with the HLSL effects included with set to *1*. This uses a massive amount of disk space very quickly, so a large HD with fast write speeds is highly recommended. Default is *0*, which is off.
|

| Suggested defaults for raster-based games:
|

+-------------------------------+-------------------------+------------------------------------+
| | bloom_lvl0_weight     1.00  | | Bloom level 0 weight  | | Full-size target.                |
| | bloom_lvl1_weight     0.64  | | Bloom level 1 weight  | | 1/4 smaller that level 0 target  |
| | bloom_lvl2_weight     0.32  | | Bloom level 2 weight  | | 1/4 smaller that level 1 target  |
| | bloom_lvl3_weight     0.16  | | Bloom level 3 weight  | | 1/4 smaller that level 2 target  |
| | bloom_lvl4_weight     0.08  | | Bloom level 4 weight  | | 1/4 smaller that level 3 target  |
| | bloom_lvl5_weight     0.06  | | Bloom level 5 weight  | | 1/4 smaller that level 4 target  |
| | bloom_lvl6_weight     0.04  | | Bloom level 6 weight  | | 1/4 smaller that level 5 target  |
| | bloom_lvl7_weight     0.02  | | Bloom level 7 weight  | | 1/4 smaller that level 6 target  |
| | bloom_lvl8_weight     0.01  | | Bloom level 8 weight  | | 1/4 smaller that level 7 target  |
+-------------------------------+-------------------------+------------------------------------+

Vector Games
------------

HLSL effects can also be used with vector games. Due to a wide variance of vector settings to optimize for each individual game, it is heavily suggested you add these to per-game INI files (e.g. tempest.ini)

Shadowmasks were only present on color vector games, and should not be used on monochrome vector games. Additionally, vector games did not use scanlines, so that should also be turned off.

Open your INI file in your text editor of choice (e.g. Notepad), and make sure the following options are set correctly:

* **video d3d**
* **filter 0**
* **hlsl_enable 1**

In the Core Vector Options section:

* **beam_width_min 1.0** (*Beam Width Minimum*)
* **beam_width_max 1.0** (*Beam Width Maximum*)
* **beam_intensity_weight 0.0** (*Beam Intensity Weight*)
* **flicker 0.0** (*Vector Flicker*)

In the Vector Post-Processing Options section:

* **vector_beam_smooth 0.0** (*Vector Beam Smooth Amount*)
* **vector_length_scale 0.5** (*Vector Attenuation Maximum*)
* **vector_length_ratio 0.5** (*Vector Attenuation Length Minimum*)

Suggested settings for vector games:

* **bloom_scale** should typically be set higher for vector games than raster games. Try between 0.4 and 1.0 for best effect.
* **bloom_overdrive** should only be used with color vector games.

* **bloom_lvl_weights** should be set as follows:

+-------------------------------+-------------------------+------------------------------------+
| | bloom_lvl0_weight     1.00  | | Bloom level 0 weight  | | Full-size target.                |
| | bloom_lvl1_weight     0.48  | | Bloom level 1 weight  | | 1/4 smaller that level 0 target  |
| | bloom_lvl2_weight     0.32  | | Bloom level 2 weight  | | 1/4 smaller that level 1 target  |
| | bloom_lvl3_weight     0.24  | | Bloom level 3 weight  | | 1/4 smaller that level 2 target  |
| | bloom_lvl4_weight     0.16  | | Bloom level 4 weight  | | 1/4 smaller that level 3 target  |
| | bloom_lvl5_weight     0.24  | | Bloom level 5 weight  | | 1/4 smaller that level 4 target  |
| | bloom_lvl6_weight     0.32  | | Bloom level 6 weight  | | 1/4 smaller that level 5 target  |
| | bloom_lvl7_weight     0.48  | | Bloom level 7 weight  | | 1/4 smaller that level 6 target  |
| | bloom_lvl8_weight     0.64  | | Bloom level 8 weight  | | 1/4 smaller that level 7 target  |
+-------------------------------+-------------------------+------------------------------------+
