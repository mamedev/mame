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

High score data is saved in the **hi** folder in the working directory.  A file
with a name corresponding the system short name (or ROM set name) with the
extension ``.hi``.  For example, high scores for the game Moon Cresta will be
saved in the file **mooncrst.hi** in the **hi** folder.
