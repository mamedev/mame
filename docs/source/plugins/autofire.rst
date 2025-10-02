.. _plugins-autofire:

Autofire Plugin
===============

.. contents:: :local:


.. _plugins-autofire-intro:

Introduction
------------

The autofire plugin allows you to simulate repeatedly pressing an emulated
button by holding down a key or button combination.  This can help people with
certain disabilities or injuries play shooting games, and may help reduce the
risk of repetitive strain injuries (or keyboard damage).

To configure the autofire plugin, activate the main menu (press **Tab** during
emulation by default), select **Plugin Options**, and then select **Autofire**.
Configured autofire buttons for the current system are listed, along with their
repetition rates and activation hotkeys (initially there will be no autofire
buttons configured).  Select an autofire button to change settings, or choose
**Add autofire button** to set up a new autofire button.  See
:ref:`plugins-autofire-settings` for details on setting up an autofire button.
You can delete an autofire button by highlighting it in the menu and pressing
the UI Clear key (Del/Delete/Forward Delete on the keyboard by default).

Autofire settings are saved in the **autofire** folder in the plugin data
folder (see the :ref:`homepath option <mame-commandline-homepath>`).  A file is
created for each system with autofire buttons configured, named according to the
system’s short name (or ROM set name), with the extension ``.cfg``.  For
example, autofire settings for Super-X will be saved in the file **superx.cfg**
in the **autofire** folder in your plugin data folder.  The autofire settings
are stored in JSON format.


.. _plugins-autofire-settings:

Autofire buttons settings
-------------------------

The options for adding a new autofire button or modifying an existing autofire
button are the same.

Select **Input** to set the emulated button that you want to simulate pressing
repeatedly.  Only digital inputs are supported.  Typically you’ll set this to
the primary fire button for shooting games.  This is most often *P1 Button 1*
or the equivalent for another player, but it might have a different name.  On
Konami’s Gradius games, *P1 Button 2* is the primary fire button.

Select **Hotkey** to set the control (or combination of controls) you’ll use to
activate the autofire button.  This can be any combination that MAME supports
for activating a digital input.

**On frames** and **Off frames** are the number of consecutive emulated video
frames that the emulated button will be held and released for, respectively.
Adjust the value with the UI Left/Right keys, or click the arrows.  Press the UI
Clear key to reset the values to one frame.  Lower values correspond to pressing
the button at a faster rate.  Depending on how fast the system reads inputs, you
may need higher numbers than 1 for the system to recognise the button being
released and pressed again (e.g. 2 on frames and 2 off frames works for Alcon).
Experiment with different values to get the best effect.

When adding a new autofire button, there is a **Cancel** option that changes to
**Create** after you set the input and hotkey.  Select **Create** to finish
creating the autofire button and return to the list of autofire buttons.  The
new autofire button will be added at the end of the list.  Press the UI Back key
(Escape/Esc on the keyboard by default), or select **Cancel** before setting the
input/hotkey, to return to the previous menu without creating the new autofire
button.

When modifying an existing autofire button, select **Done** or press the UI
Cancel key to return to the list of autofire buttons.  Changes take effect
immediately.

When modifying an existing autofire button, select **Delete** to delete the
autofire button.  The autofire button will be deleted immediately when you
select **Delete** without requiring additional confirmation.  You can also
delete an autofire button by highlighting it in the list of autofire buttons
and pressing the UI Clear key (Del/Delete/Forward Delete on the keyboard by
default).


.. _plugins-autofire-notes:

Notes and potential pitfalls
----------------------------

Autofire buttons act as if they’re wired in parallel with MAME’s regular
controls.  This means that if you set the activation hotkey for an autofire
button to a button or key that’s also assigned to one of the emulated inputs
directly, you may get unexpected results.  Using Gradius as an example:

* Suppose you set button 1 on your controller to fire, and set an autofire
  hotkey to button 1 as well.  Holding the button down to shoot will not trigger
  the autofire effect: the button will never be released as you’re holding the
  non-autofire button 1 down.  This will also happen if you set a different
  button as autofire (say, button 3 in this case), and hold button 1 down while
  also pressing button 3.
* If you set button 3 on your controller to autofire and assign button 3 to
  powerup as well, you will trigger the powerup action every time you grab a
  powerup because the powerup button is also being held down along with the
  autofire button.

It is recommended that you choose control combinations for autofire hotkeys that
are not assigned to any other emulated inputs in the system.

Autofire is not necessarily desirable in all situations.  For example using
autofire in Super-X with the blue “lightning” weapon equipped at high power
levels will only produce a single beam, greatly reducing the weapon’s
effectiveness.  The fire button must be held down to produce all beams.  Some
shooting games (e.g. Raiden Fighters) require the primary fire button to be held
down for a charged special attack.  This means it’s often necessary to have a
non-autofire input for the primary fire button assigned to play effectively.
