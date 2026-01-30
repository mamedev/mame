.. _plugins-offscreenreload:

Off-Screen Reload Helper Plugin
===============================

.. contents:: :local:


.. _plugins-offscreenreload-intro:

Introduction
------------

The off-screen reload helper plugin makes it easier to play games that require
you to aim a lightgun away from the screen to perform actions (usually
reloading).  You can use a key or button combination to simulate aiming the
lightgun away from the screen and pulling the trigger.

To configure the off-screen reload helper plugin, activate the main menu (press
**Tab** during emulation by default), select **Plugin Options**, and then select
**Off-Screen Reload Helper**.  Configured off-screen reload helpers for the
current system are listed, along with their activation sequences (initially
there will be no off-screen reload helpers configured).  Select a reload helper
to edit it, or choose **Add reload helper** to set up a new off-screen reload
helper.  See :ref:`plugins-offscreenreload-settings` for details on editing
off-screen reload helpers.  You can delete an off-screen reload helper by
highlighting it in the menu and pressing the UI Clear key (Del/Delete/Forward
Delete on the keyboard by default).  You can also delete an off-screen reload
helper by selecting it to edit it and then selecting **Delete reload helper**
from the menu.

Off-screen reload helpers are saved in the **offscreenreload** folder in the
plugin data folder (see the :ref:`homepath option <mame-commandline-homepath>`).
A file is created for each system with off-screen reload helpers configured,
named according to the system’s short name (or ROM set name), with the extension
``.cfg``.  For example, input macros for the parent version of Lethal Enforcers
will be saved in the file **lethalen.cfg** in the **offscreenreload** folder in
your plugin data folder.  The off-screen reload helpers are stored in JSON
format.


.. _plugins-offscreenreload-settings:

Editing off-screen reload helpers
---------------------------------

The options for editing off-screen reload helpers are the same whether you’re
creating a new reload helper or editing an existing one.  An off-screen reload
helper need an *axis input* (usually Y axis for the player’s lightgun), and a
*trigger input* (usually button 1 for the player, corresponding to the
lightgun’s trigger).

* Select **Reload combination** to set the control (or combination of controls)
  you want to use to activate the off-screen reload helper.  You will probably
  want to use a combination that isn’t being used for any other emulated input
  in the system.
* Select **Axis input** to set the axis input (often **Lightgun Y** for the
  first player, or **Lightgun 2 Y** for the second player).  Only non-wrapping
  analog inputs are supported.
* Select **Trigger input** to set the trigger input (often **P1 Button 1** for
  the first player, or **P2 Button 1** for the second player).  Only non-toggle
  digital inputs are supported.  The plugin attempts to guess the trigger input
  when you change the axis input.

When creating a new off-screen reload helper, there is a **Cancel** option that
changes to **Create** after you set the reload combination, axis input and
trigger input.  Select **Create** to finish creating the reload helper and
return to the list of off-screen reload helpers.  The new reload helper will be
added at the end of the list.  Press the UI Back key, or select **Cancel**
before setting the reload sequence/inputs, to return to the previous menu
without creating the new reload helper.

When editing an existing off-screen reload helper, select **Done** or press the
UI Back key to return to the list of off-screen reload helpers.  Changes take
effect immediately.

When editing an existing off-screen reload helper, select **Delete reload
helper** to delete it.  The off-screen reload helper will be deleted immediately
when you select **Delete reload helper** without requiring additional
confirmation.  You can also delete an off-screen reload helper by highlighting
it in the list of reload helpers and pressing the UI Clear key
(Del/Delete/Forward Delete on the keyboard by default).
