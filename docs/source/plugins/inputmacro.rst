.. _plugins-inputmacro:

Input Macro Plugin
==================

.. contents:: :local:


.. _plugins-inputmacro-intro:

Introduction
------------

The input macro plugin allows you to trigger a sequence of emulated input
actions with a key or button combination.  This can help people with
disabilities or injuries that make some input sequences difficult.  It can also
be used as a way to cheat in games that require rapid sequences of inputs, like
the running events in Track & Field, or the eating minigame in Daisu-Kiss.

To configure the input macro plugin, activate the main menu (press **Tab**
during emulation by default), select **Plugin Options**, and then select **Input
Macros**.  Configured input macros for the current system are listed, along with
their activation sequences (initially there will be no input macros configured).
Select a macro to edit it, or choose **Add macro** to set up a new input macro.
See :ref:`plugins-inputmacro-settings` for details on editing input macros.  You
can delete an input macro by highlighting it in the menu and pressing the UI
Clear key (Del/Delete/Forward Delete on the keyboard by default).  You can also
delete a macro by selecting the macro to edit it and then selecting **Delete
macro** from the menu.

Input macros are saved in the **inputmacro** folder in the plugin data folder
(see the :ref:`homepath option <mame-commandline-homepath>`).  A file is created
for each system with input macros configured, named according to the system’s
short name (or ROM set name), with the extension ``.cfg``.  For example, input
macros for Daisu-Kiss will be saved in the file **daiskiss.cfg** in the
**inputmacro** folder in your plugin data folder.  The input macros are stored
in JSON format.


.. _plugins-inputmacro-settings:

Editing input macros
--------------------

The options for editing input macros are the same whether you’re creating a new
macro or editing an existing macro.  Input macros consist of a sequence of
*steps*.  Each step optionally waits for a configurable delay, then activates
one or more emulated inputs for a specified duration.  You can choose what
should happen if the activation sequence is still held when the final step of
the macro completes: the emulated inputs can be released, the final step can be
prolonged, or the macro can loop back to any step in the sequence.

The settings in first section of the macro editing menu apply to the macro as a
whole:

* The **Name** will be used in the list of input macros, so it helps to make it
  descriptive.  Press the UI Select key (Return/Enter on the keyboard or the
  first button on the first joystick by default) to edit the current name, or
  press the UI Clear key to type a new name.  Press the UI Select key before
  moving to another menu item to save the new name; press the UI Back key
  (Escape/Esc on the keyboard by default) to change discard the new name.
* Select **Activation combination** to set the control (or combination of
  controls) you want to use to activate the macro.  Keep in mind that regular
  input assignments still apply, so you will probably want to use a combination
  that isn’t being used for any other emulated input in the system.
* Set **On release** to specify what should happen if the activation sequence is
  released before the macro completes.  When set to *Stop immediately*, any
  emulated inputs activated by the macro will be released immediately, and no
  further steps will be processed; when set to *Complete macro*, the macro will
  continue to be processed until the end of the final step.
* Set **When held** to specify what should happen if the activation sequence is
  held after the final step of the macro completes.  When set to *Release*, any
  inputs activated by the macro will be released, and the macro will not be
  reactivated until the activation sequence is released and pressed again; when
  set to *Prolong step <n>* where *<n>* is the number of the final step of the
  macro, the emulated inputs activated by the final step of the macro will
  remain active until the activation sequence is released; when set to *Loop to
  step <n>* where *<n>* is a step number, macro processing will return to that
  step, including its delay, if the activation sequence is held after the final
  step completes.

Each step has delay, duration and input settings:

* Set the **Delay** to the number of emulated video frame intervals to wait
  before activating the inputs for the step.  During the delay, no emulated
  inputs will be activated by the macro.  You can reset the setting to zero by
  pressing the UI Clear key.
* Set the **Duration** to the number of emulated video frame intervals to hold
  the emulated inputs for the step active before moving to the next step (or
  completing the macro in the case of the final step).  You can reset the
  setting to one frame by pressing the UI Clear key.
* Set the **Input** settings to the emulated inputs to activate for the step.
  Only non-toggle digital inputs and non-wrapping analog inputs are supported.
  Select **Add input** to set multiple inputs for a step (this option will only
  appear after you set the first input for the initially created step when
  creating a new macro).  If the step has multiple inputs, you can highlight an
  input on the menu and press the UI Clear key to delete it (all steps must have
  at least one input, so you can’t delete the only input for a step).
* For analog inputs, you can set the desired **Value** that the input should
  have at each step.  You can adjust the value by pressing the UI Left and UI
  Right keys, or type a numeric value and press the UI Select key to accept the
  new value.  The value will be limited to the valid range for the input.
  Pressing the UI Clear key will set the default value for the input.
* If the macro has multiple steps, you can select **Delete step** to delete a
  step (this options does not appear if the macro only has a single step).
  Remember to check that the **On release** and **When held** settings are
  correct after deleting steps.

To add a step to the macro, highlight **Add step at position** (below the
existing steps), use the UI Left/Right keys or click the arrows to set the
position where you’d like to insert the new step, and then press the UI Select
key (or double-click the menu item) to add the new step.  You will be prompted
to set the first input for the new step.  Remember to check the **On release**
and **When held** settings after adding steps.  The **Add step at position**
item will only appear after you set the first input for the initially created
step when creating a new macro.

When creating a new macro, there is a **Cancel** option that changes to
**Create** after you set the activation sequence and the first input for the
initially created step.  Select **Create** to finish creating the macro and
return to the list of input macros.  The new macro will be added at the end of
the list.  Press the UI Back key, or select **Cancel** before setting the
activation sequence/input, to return to the previous menu without creating the
new macro.

When editing an existing macro, select **Done** or press the UI Back key to
return to the list of input macros.  Changes take effect immediately.

When editing an existing macro, select **Delete macro** to delete the macro.
The macro will be deleted immediately when you select **Delete macro** without
requiring additional confirmation.  You can also delete a macro by highlighting
it in the list of macros and pressing the UI Clear key (Del/Delete/Forward
Delete on the keyboard by default).


.. _plugins-inputmacro-examples:

Example macros
--------------

Raiden autofire
~~~~~~~~~~~~~~~

This provides player 1 autofire functionality using the space bar.  The same
thing could be achieved using the :ref:`plugins-autofire`, but this demonstrates
a simple looping macro:

* **Name**: P1 Autofire
* **Activation combination**: Kbd Space
* **On release**: Stop immediately
* **When held**: Loop to step 2
* **Step 1**:

  * **Delay (frames)**: 0
  * **Duration (frames)**: 2
  * **Input 1**: P1 Button 1
* **Step 2**:

  * **Delay (frames)**: 4
  * **Duration (frames)**: 2
  * **Input 1**: P1 Button 1

The first step has no delay so that firing begins as soon as the space bar is
pressed.  The second step has sufficient delay to ensure the game recognises the
button being pressed and released again.  The second step is repeated as long as
the space bar is held down.

Track & Field sprint cheat
~~~~~~~~~~~~~~~~~~~~~~~~~~

This allows you to run in Konami Track & Field by holding a single button.  This
takes most of the skill (and fun) out of the game:

* **Name**: P1 Sprint
* **Activation combination**: Kbd Shift
* **On release**: Stop immediately
* **When held**: Loop to step 2
* **Step 1**:

  * **Delay (frames)**: 0
  * **Duration (frames)**: 1
  * **Input 1**: P1 Button 1
* **Step 2**:

  * **Delay (frames)**: 1
  * **Duration (frames)**: 1
  * **Input 1**: P1 Button 3
* **Step 3**:

  * **Delay (frames)**: 1
  * **Duration (frames)**: 1
  * **Input 1**: P1 Button 1

This macro rapidly alternates pressing buttons 1 and 3 – the pattern required to
run in the game.

Street Fighter II Shoryuken
~~~~~~~~~~~~~~~~~~~~~~~~~~~

This macro allows you to perform a right-facing Shōryūken (Dragon Punch) by
pressing a single key:

* **Name**: 1P Shoryuken LP
* **Activation combination**: Kbd M
* **On release**: Complete macro
* **When held**: Prolong step 6
* **Step 1**:

  * **Delay (frames)**: 0
  * **Duration (frames)**: 1
  * **Input 1**: P1 Right
* **Step 2**:

  * **Delay (frames)**: 1
  * **Duration (frames)**: 1
  * **Input 1**: P1 Down
* **Step 3**:

  * **Delay (frames)**: 0
  * **Duration (frames)**: 1
  * **Input 1**: P1 Down
  * **Input 2**: P1 Right
* **Step 4**:

  * **Delay (frames)**: 0
  * **Duration (frames)**: 1
  * **Input 1**: P1 Right
* **Step 5**:

  * **Delay (frames)**: 0
  * **Duration (frames)**: 1
  * **Input 1**: P1 Right
  * **Input 2**: P1 Jab Punch
* **Step 6**:

  * **Delay (frames)**: 0
  * **Duration (frames)**: 1
  * **Input 1**: P1 Jab Punch

This macro involves steps that activate multiple inputs.  The macro will
complete if the activation sequence is released early, allowing you to tap the
key momentarily to perform the move.  Holding the activation sequence holds down
the attack button.

Virtua Cop Aim Off-Screen
~~~~~~~~~~~~~~~~~~~~~~~~~

This macro allows you to hold a key to simulate pointing the lightgun away from
the screen:

* **Name**: 1P Aim Off-Screen
* **Activation combination**: Kbd Alt
* **On release**: Stop immediately
* **When held**: Prolong step 1
* **Step 1**:

  * **Delay (frames)**: 0
  * **Duration (frames)**: 1
  * **Input 1**: Lightgun X
  * **Value (131-630)**: 131
  * **Input 2**: Lightgun Y
  * **Value (36-425)**: 36

This macro involves setting analog input values.
