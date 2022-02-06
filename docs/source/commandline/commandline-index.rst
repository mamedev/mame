.. _index-commandline:

Commandline Index
=================

This is a complete index of all commandline options and commands for MAME, suitable for quickly finding a given command.

Universal Commandline Options
-----------------------------

This section contains configuration options that are applicable to *all* MAME sub-builds (both SDL and Windows native).


Core Verbs
~~~~~~~~~~

| :ref:`help <mame-commandline-help>`
| :ref:`validate <mame-commandline-validate>`


Configuration Verbs
~~~~~~~~~~~~~~~~~~~

| :ref:`createconfig <mame-commandline-createconfig>`
| :ref:`showconfig <mame-commandline-showconfig>`
| :ref:`showusage <mame-commandline-showusage>`


Frontend Verbs
~~~~~~~~~~~~~~

| :ref:`listxml <mame-commandline-listxml>`
| :ref:`listfull <mame-commandline-listfull>`
| :ref:`listsource <mame-commandline-listsource>`
| :ref:`listclones <mame-commandline-listclones>`
| :ref:`listbrothers <mame-commandline-listbrothers>`
| :ref:`listcrc <mame-commandline-listcrc>`
| :ref:`listroms <mame-commandline-listroms>`
| :ref:`listsamples <mame-commandline-listsamples>`
| :ref:`verifyroms <mame-commandline-verifyroms>`
| :ref:`verifysamples <mame-commandline-verifysamples>`
| :ref:`romident <mame-commandline-romident>`
| :ref:`listdevices <mame-commandline-listdevices>`
| :ref:`listslots <mame-commandline-listslots>`
| :ref:`listmedia <mame-commandline-listmedia>`
| :ref:`listsoftware <mame-commandline-listsoftware>`
| :ref:`verifysoftware <mame-commandline-verifysoftware>`
| :ref:`getsoftlist <mame-commandline-getsoftlist>`
| :ref:`verifysoftlist <mame-commandline-verifysoftlist>`


OSD-related Options
~~~~~~~~~~~~~~~~~~~

| :ref:`uimodekey <mame-commandline-uimodekey>`
| :ref:`uifontprovider <mame-commandline-uifontprovider>`
| :ref:`keyboardprovider <mame-commandline-keyboardprovider>`
| :ref:`mouseprovider <mame-commandline-mouseprovider>`
| :ref:`lightgunprovider <mame-commandline-lightgunprovider>`
| :ref:`joystickprovider <mame-commandline-joystickprovider>`


OSD CLI Verbs
~~~~~~~~~~~~~

| :ref:`listmidi <mame-commandline-listmidi>`
| :ref:`listnetwork <mame-commandline-listnetwork>`


OSD Output Options
~~~~~~~~~~~~~~~~~~

| :ref:`output <mame-commandline-output>`


Configuration Options
~~~~~~~~~~~~~~~~~~~~~

| :ref:`noreadconfig <mame-commandline-noreadconfig>`


Core Search Path Options
~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`homepath <mame-commandline-homepath>`
| :ref:`rompath <mame-commandline-rompath>`
| :ref:`hashpath <mame-commandline-hashpath>`
| :ref:`samplepath <mame-commandline-samplepath>`
| :ref:`artpath <mame-commandline-artpath>`
| :ref:`ctrlrpath <mame-commandline-ctrlrpath>`
| :ref:`inipath <mame-commandline-inipath>`
| :ref:`fontpath <mame-commandline-fontpath>`
| :ref:`cheatpath <mame-commandline-cheatpath>`
| :ref:`crosshairpath <mame-commandline-crosshairpath>`
| :ref:`pluginspath <mame-commandline-pluginspath>`
| :ref:`languagepath <mame-commandline-languagepath>`
| :ref:`swpath <mame-commandline-swpath>`


Core Output Directory Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`cfg_directory <mame-commandline-cfgdirectory>`
| :ref:`nvram_directory <mame-commandline-nvramdirectory>`
| :ref:`input_directory <mame-commandline-inputdirectory>`
| :ref:`state_directory <mame-commandline-statedirectory>`
| :ref:`snapshot_directory <mame-commandline-snapshotdirectory>`
| :ref:`diff_directory <mame-commandline-diffdirectory>`
| :ref:`comment_directory <mame-commandline-commentdirectory>`


Core State/Playback Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`[no]rewind / rewind<mame-commandline-norewind>`
| :ref:`rewind_capacity <mame-commandline-rewindcapacity>`
| :ref:`state <mame-commandline-state>`
| :ref:`[no]autosave <mame-commandline-noautosave>`
| :ref:`playback <mame-commandline-playback>`
| :ref:`[no]exit_after_playback <mame-commandline-exitafterplayback>`
| :ref:`record <mame-commandline-record>`
| :ref:`mngwrite <mame-commandline-mngwrite>`
| :ref:`aviwrite <mame-commandline-aviwrite>`
| :ref:`wavwrite <mame-commandline-wavwrite>`
| :ref:`snapname <mame-commandline-snapname>`
| :ref:`snapsize <mame-commandline-snapsize>`
| :ref:`snapview <mame-commandline-snapview>`
| :ref:`[no]snapbilinear <mame-commandline-nosnapbilinear>`
| :ref:`statename <mame-commandline-statename>`
| :ref:`[no]burnin <mame-commandline-noburnin>`


Core Performance Options
~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`[no]autoframeskip <mame-commandline-noautoframeskip>`
| :ref:`frameskip <mame-commandline-frameskip>`
| :ref:`seconds_to_run <mame-commandline-secondstorun>`
| :ref:`[no]throttle <mame-commandline-nothrottle>`
| :ref:`[no]sleep <mame-commandline-nosleep>`
| :ref:`speed <mame-commandline-speed>`
| :ref:`[no]refreshspeed <mame-commandline-norefreshspeed>`
| :ref:`numprocessors <mame-commandline-numprocessors>`
| :ref:`bench <mame-commandline-bench>`
| :ref:`[no]lowlatency <mame-commandline-lowlatency>`


Core Rotation Options
~~~~~~~~~~~~~~~~~~~~~

| :ref:`[no]rotate <mame-commandline-norotate>`
| :ref:`[no]ror <mame-commandline-noror>`
| :ref:`[no]rol <mame-commandline-norol>`
| :ref:`[no]autoror <mame-commandline-noautoror>`
| :ref:`[no]autorol <mame-commandline-noautorol>`
| :ref:`[no]flipx <mame-commandline-noflipx>`
| :ref:`[no]flipy <mame-commandline-noflipy>`


Core Video Options
~~~~~~~~~~~~~~~~~~

| :ref:`video <mame-commandline-video>`
| :ref:`numscreens <mame-commandline-numscreens>`
| :ref:`[no]window <mame-commandline-window>`
| :ref:`[no]maximize <mame-commandline-maximize>`
| :ref:`[no]keepaspect <mame-commandline-keepaspect>`
| :ref:`[no]waitvsync <mame-commandline-waitvsync>`
| :ref:`[no]syncrefresh <mame-commandline-syncrefresh>`
| :ref:`prescale <mame-commandline-prescale>`
| :ref:`[no]filter <mame-commandline-filter>`
| :ref:`[no]unevenstretch <mame-commandline-unevenstretch>`


Core Full Screen Options
~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`[no]switchres <mame-commandline-switchres>`


Core Per-Window Video Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`screen <mame-commandline-screen>`
| :ref:`aspect <mame-commandline-aspect>`
| :ref:`resolution <mame-commandline-resolution>`
| :ref:`view <mame-commandline-view>`


Core Artwork Options
~~~~~~~~~~~~~~~~~~~~

| :ref:`[no]artwork_crop <mame-commandline-noartworkcrop>`
| :ref:`fallback_artwork <mame-commandline-fallbackartwork>`
| :ref:`override_artwork <mame-commandline-overrideartwork>`


Core Screen Options
~~~~~~~~~~~~~~~~~~~

| :ref:`brightness <mame-commandline-brightness>`
| :ref:`contrast <mame-commandline-contrast>`
| :ref:`gamma <mame-commandline-gamma>`
| :ref:`pause_brightness <mame-commandline-pausebrightness>`
| :ref:`effect <mame-commandline-effect>`


Core Vector Options
~~~~~~~~~~~~~~~~~~~

| :ref:`beam_width_min <mame-commandline-beamwidthmin>`
| :ref:`beam_width_max <mame-commandline-beamwidthmax>`
| :ref:`beam_intensity_weight <mame-commandline-beamintensityweight>`
| :ref:`beam_dot_size <mame-commandline-beamdotsize>`
| :ref:`flicker <mame-commandline-flicker>`


Core Video OpenGL Debugging Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`[no]gl_forcepow2texture <mame-commandline-glforcepow2texture>`
| :ref:`[no]gl_notexturerect <mame-commandline-glnotexturerect>`
| :ref:`[no]gl_vbo <mame-commandline-glvbo>`
| :ref:`[no]gl_pbo <mame-commandline-glpbo>`


Core Video OpenGL GLSL Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`[no]gl_glsl <mame-commandline-glglsl>`
| :ref:`gl_glsl_filter <mame-commandline-glglslfilter>`
| :ref:`glsl_shader_mame[0-9] <mame-commandline-glslshadermame>`
| :ref:`glsl_shader_screen[0-9] <mame-commandline-glslshaderscreen>`


Core Sound Options
~~~~~~~~~~~~~~~~~~

| :ref:`samplerate <mame-commandline-samplerate>`
| :ref:`[no]samples <mame-commandline-nosamples>`
| :ref:`[no]compressor <mame-commandline-nocompressor>`
| :ref:`volume <mame-commandline-volume>`
| :ref:`sound <mame-commandline-sound>`
| :ref:`audio_latency <mame-commandline-audiolatency>`


Core Input Options
~~~~~~~~~~~~~~~~~~

| :ref:`[no]coin_lockout <mame-commandline-nocoinlockout>`
| :ref:`ctrlr <mame-commandline-ctrlr>`
| :ref:`[no]mouse <mame-commandline-nomouse>`
| :ref:`[no]joystick <mame-commandline-nojoystick>`
| :ref:`[no]lightgun <mame-commandline-nolightgun>`
| :ref:`[no]multikeyboard <mame-commandline-nomultikeyboard>`
| :ref:`[no]multimouse <mame-commandline-nomultimouse>`
| :ref:`[no]steadykey <mame-commandline-nosteadykey>`
| :ref:`[no]ui_active <mame-commandline-uiactive>`
| :ref:`[no]offscreen_reload <mame-commandline-nooffscreenreload>`
| :ref:`joystick_map <mame-commandline-joystickmap>`
| :ref:`joystick_deadzone <mame-commandline-joystickdeadzone>`
| :ref:`joystick_saturation <mame-commandline-joysticksaturation>`
| :ref:`[no]natural <mame-commandline-natural>`
| :ref:`[no]joystick_contradictory <mame-commandline-joystickcontradictory>`
| :ref:`coin_impulse <mame-commandline-coinimpulse>`


Core Input Automatic Enable Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`paddle_device <mame-commandline-paddledevice>`
| :ref:`adstick_device <mame-commandline-adstickdevice>`
| :ref:`pedal_device <mame-commandline-pedaldevice>`
| :ref:`dial_device <mame-commandline-dialdevice>`
| :ref:`trackball_device <mame-commandline-trackballdevice>`
| :ref:`lightgun_device <mame-commandline-lightgundevice>`
| :ref:`positional_device <mame-commandline-positionaldevice>`
| :ref:`mouse_device <mame-commandline-mousedevice>`


Core Debugging Options
~~~~~~~~~~~~~~~~~~~~~~

| :ref:`[no]verbose <mame-commandline-verbose>`
| :ref:`[no]oslog <mame-commandline-oslog>`
| :ref:`[no]log <mame-commandline-log>`
| :ref:`[no]debug <mame-commandline-debug>`
| :ref:`debugger <mame-commandline-debugger>`
| :ref:`debugscript <mame-commandline-debugscript>`
| :ref:`[no]update_in_pause <mame-commandline-updateinpause>`
| :ref:`watchdog <mame-commandline-watchdog>`
| :ref:`debugger_port <mame-commandline-debuggerport>`
| :ref:`debugger_font <mame-commandline-debuggerfont>`
| :ref:`debugger_font_size <mame-commandline-debuggerfontsize>`


Core Communication Options
~~~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`comm_localhost <mame-commandline-commlocalhost>`
| :ref:`comm_localport <mame-commandline-commlocalport>`
| :ref:`comm_remotehost <mame-commandline-commremotehost>`
| :ref:`comm_remoteport <mame-commandline-commremoteport>`
| :ref:`[no]comm_framesync <mame-commandline-commframesync>`


Core Misc Options
~~~~~~~~~~~~~~~~~

| :ref:`[no]drc <mame-commandline-drc>`
| :ref:`[no]drc_use_c <mame-commandline-drcusec>`
| :ref:`[no]drc_log_uml <mame-commandline-drcloguml>`
| :ref:`[no]drc_log_native <mame-commandline-drclognative>`
| :ref:`bios <mame-commandline-bios>`
| :ref:`[no]cheat <mame-commandline-cheat>`
| :ref:`[no]skip_gameinfo <mame-commandline-skipgameinfo>`
| :ref:`uifont <mame-commandline-uifont>`
| :ref:`ui <mame-commandline-ui>`
| :ref:`ramsize <mame-commandline-ramsize>`
| :ref:`[no]confirm_quit <mame-commandline-confirmquit>`
| :ref:`[no]ui_mouse <mame-commandline-uimouse>`
| :ref:`language <mame-commandline-language>`
| :ref:`[no]nvram_save <mame-commandline-nvramsave>`


Scripting Options
~~~~~~~~~~~~~~~~~

| :ref:`autoboot_command <mame-commandline-autobootcommand>`
| :ref:`autoboot_delay <mame-commandline-autobootdelay>`
| :ref:`autoboot_script <mame-commandline-autobootscript>`
| :ref:`[no]console <mame-commandline-console>`
| :ref:`[no]plugins <mame-commandline-plugins>`
| :ref:`plugin <mame-commandline-plugin>`
| :ref:`noplugin <mame-commandline-noplugin>`


HTTP Server Options
~~~~~~~~~~~~~~~~~~~

| :ref:`http <mame-commandline-http>`
| :ref:`http_port <mame-commandline-httpport>`
| :ref:`http_root <mame-commandline-httproot>`


PortAudio Options
~~~~~~~~~~~~~~~~~

| :ref:`pa_api <mame-commandline-paapi>`
| :ref:`pa_device <mame-commandline-padevice>`
| :ref:`pa_latency <mame-commandline-palatency>`


Windows-Specific Commandline Options
------------------------------------


Windows Performance Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`priority <mame-wcommandline-priority>`
| :ref:`profile <mame-wcommandline-profile>`


Windows Full Screen Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`[no]triplebuffer <mame-wcommandline-triplebuffer>`
| :ref:`full_screen_brightness <mame-wcommandline-fullscreenbrightness>`
| :ref:`full_screen_contrast <mame-wcommandline-fullscreencontrast>`
| :ref:`full_screen_gamma <mame-wcommandline-fullscreengamma>`


Windows Input Device Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`[no]dual_lightgun <mame-wcommandline-duallightgun>`


SDL-Specific Commandline Options
--------------------------------

This section contains configuration options that are specific to any build supported by SDL (including Windows where compiled as SDL instead of native).


SDL Performance Options
~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`[no]sdlvideofps <mame-scommandline-sdlvideofps>`


SDL Video Options
~~~~~~~~~~~~~~~~~

| :ref:`[no]centerh <mame-scommandline-centerh>`
| :ref:`[no]centerv <mame-scommandline-centerv>`


SDL Video Soft-Specific Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`scalemode <mame-scommandline-scalemode>`


SDL Keyboard Mapping
~~~~~~~~~~~~~~~~~~~~

| :ref:`keymap <mame-scommandline-keymap>`
| :ref:`keymap_file <mame-scommandline-keymapfile>`


SDL Joystick Mapping
~~~~~~~~~~~~~~~~~~~~

| :ref:`joyidx <mame-scommandline-joyidx>`
| :ref:`sixaxis <mame-scommandline-sixaxis>`


SDL Low-level Driver Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

| :ref:`videodriver <mame-scommandline-videodriver>`
| :ref:`audiodriver <mame-scommandline-audiodriver>`
| :ref:`gl_lib <mame-scommandline-gllib>`

