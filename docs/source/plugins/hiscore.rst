.. _plugins-hiscore:

Hiscore Support Plugin
======================

The hiscore support plugin saves and restores high scores for games that did not
originally save high scores in non-volatile memory.  Note that this plugin
modifies the contents of memory directly with no coordination with the emulated
software, and hence changes behaviour.  This may have undesirable effects,
including broken gameplay or causing the emulated software to crash.

The plugin includes a **hiscore.dat** file that contains the information on how
to save and restore high scores for supported systems.  This file must be kept
up-to-date when system definitions change in MAME.

High scores can be saved automatically either on exit, or a few seconds after
they’re updated in memory.  To change the setting, activate the main menu (press
**Tab** during emulation by default), select **Plugin Options**, and then select
**Hiscore Support**.  Change the **Save scores** option by highlighting it and
using the UI Left/Right keys, or clicking the arrows.

High score data is saved in the **hiscore** folder in the plugin data folder
(see the :ref:`homepath option <mame-commandline-homepath>`).  A file with a
name corresponding the system short name (or ROM set name) with the extension
``.hi``.  For example, high scores for the game Moon Cresta will be saved in the
file **mooncrst.hi** in the **hiscore** folder in your plugin data folder.  The
settings for the hiscore support plugin are stored in the file **plugin.cfg** in
the **hiscore** folder in the plugin data folder (this file is in JSON format).

The hiscore support plugin can be disabled on a game-by-game basis by toggling the 
**Enable Hiscore Support for this game** option within the plugin's menu.  By
default, the plugin is enabled for all games.  Games that have been excluded are
tracked in a json file **exclude_games.json** that is stored in the 
**plugins/hiscore** folder.
