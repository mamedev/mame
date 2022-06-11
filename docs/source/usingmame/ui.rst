.. _ui:

MAME’s User Interface
=====================

.. contents:: :local:


.. _ui-intro:

Introduction
------------

MAME provides a simple user interface for selecting the system and software to
run and changing settings while running an emulated system.  MAME’s user
interface is designed to be usable with a keyboard, game controller, or pointing
device, but will require a keyboard for initial configuration.

The default settings for the most important controls to know when running an
emulated system, and the settings they correspond to in case you want to change
them, are as follows:

Scroll Lock, or Forward Delete on macOS (UI Toggle)
    For emulated systems with keyboard inputs, enable or disable UI controls.
    (MAME starts with UI controls disabled for systems with keyboard inputs
    unless the :ref:`ui_active option <mame-commandline-uiactive>` is on.)
Tab (Config Menu)
    Show or hide the menu during emulation.
Escape (UI Cancel)
    Return to the system selection menu, or exit if MAME was started with a
    system specified (from the command line or using an
    :ref:`external front-end <frontends>`).


.. _ui-menus:

Navigating menus
----------------

By default, MAME menus can be navigated using the keyboard cursor keys.  All
the UI controls can be changed by going to the **General Inputs** menu and then
selecting **User Interface**.  The default keyboard controls on a US ANSI QWERTY
layout keyboard, and the settings they correspond to, are as follows:

Up Arrow (UI Up)
    Highlight the previous menu item, or the last item if the first item is
    highlighted.
Down Arrow (UI Down)
    Highlight the next menu item, or the first item if the last item is
    highlighted.
Left Arrow (UI Left)
    For menu items that are adjustable settings, reduce the value or select the
    previous setting (these menu items show left- and right-facing triangles
    beside the value).
Right Arrow (UI Left)
    For menu items that are adjustable settings, increase the value or select
    the next setting (these menu items show left- and right-facing triangles
    beside the value).
Return/Enter keypad Enter (UI Select)
    Select the highlighted menu item.
Forward Delete, or Fn+Delete on some compact keyboards (UI Clear)
    Clear setting or reset to default value.
Escape (UI Cancel)
    Clear the search if searching the menu, otherwise close the menu, returning
    to the previous menu, or returning to the emulated system in the case of the
    main menu (there’s usually an item at the bottom of the menu for the same
    purpose).
Home (UI Home)
    Highlight the first menu item and scroll to the top of the menu.
End (UI End)
    Highlight the last menu item and scroll to the bottom of the menu.
Page Up (UI Page Up)
    Scroll the menu up by one screen.
Page Down (UI Page Down)
    Scroll the menu down by one screen.
[ (UI Previous Group)
    Move to the previous group of items (not used by all menus).
] (UI Next Group)
    Move to the next group of items (not used by all menus).


.. _ui-menus-gamectrl:

Using a game controller
~~~~~~~~~~~~~~~~~~~~~~~

MAME supports navigating menus with a game controller or joystick, but only the
most important UI controls have joystick assignments by default:

* Move the first joystick up or down in the Y axis to highlight the previous or
  next menu item.
* Move the first joystick left or right in the X axis to adjust settings.
* Press the first button on the first joystick to select the highlighted menu
  item.

For gamepad-style controllers, the left analog thumb stick usually controls UI
navigation.  You may find it convenient to assign directional pad controls to UI
navigation in addition to or in place of the left thumb stick.

If you want to be able to use MAME with a game controller without needing a
keyboard, you’ll need to assign joystick buttons (or combinations of buttons) to
these controls as well:

* **Config Menu** to show or dismiss the menu during emulation
* **UI Cancel** to close menus, return to the system selection menu, or exit
  MAME
* **UI Clear** isn’t essential for basic emulation, but it’s used to clear or
  reset some settings to defaults
* **UI Home**, **UI End**, **UI Page Up**, **UI Page Down**, **UI Previous
  Group** and **UI Next Group** are not essential, but make navigating some
  menus easier

If you’re not using an external front-end to launch systems in MAME, you should
assign joystick buttons (or combinations of buttons) to these controls to make
full use of the system and software selection menus:

* **UI Focus Next**/**UI Focus Previous** to navigate between panes
* **UI Add/Remove favorite**, **UI Export List** and **UI Audit Media** if you
  want access to these features without using a keyboard or pointing device


.. _ui-menus-mouse:

Using a mouse or trackball
~~~~~~~~~~~~~~~~~~~~~~~~~~

MAME supports navigating menus using a mouse or trackball that works as a system
pointing device:

* Click menu items to highlight them.
* Double-click menu items to select them.
* Click the left- or right-pointing triangle to adjust settings.
* For menus with too many items to fit on the screen, click the upward- or
  downward-pointing triangle at the top or bottom to scroll up or down by one
  screen at a time.
* Use vertical scrolling gestures to scroll menus or text boxes with too many
  items or lines to fit on the screen.
* Click toolbar items to select them, or hover over them to see a description.

If you have enough additional mouse buttons, you may want to assign button
combinations to the **Config Menu**, **Pause** and/or **UI Cancel** inputs to
make it possible to use MAME without a keyboard.


.. _ui-inptcfg:

Configuring inputs
------------------

MAME needs a flexible input system to support the control schemes of the vast
array of systems it emulates.  In MAME, inputs that only have two distinct
states, on and off or active and inactive, are called *digital inputs*, and all
other inputs are called *analog inputs*, even if this is not strictly true (for
example multi-position switches are called analog inputs in MAME).

To assign MAME’s user interface controls or the default inputs for all systems,
select **Input Settings** from the main menu during emulation and then select
**Input Assignments (general)** from the Input Settings menu, or select
**General Settings** from the system selection menu and then select **Input
Assignments** from the General Settings menu.  From there, select a category.

To assign inputs for the currently running system, select **Input Settings**
from the main menu during emulation and then select **Input Assignments (this
system)** from the Input Settings menu.  Inputs are grouped by device and sorted
by type.  You can move between devices with the next group and previous group
keys/buttons (opening/closing brackets **[** and **]** on the keyboard by
default).

The input assignment menus show the name of the emulated input or user interface
control on the left, and the controls (or combination of controls) assigned to
it on the right.

To adjust the sensitivity, auto-centre speed and inversion settings, or to see
how emulated analog controls react to your inputs, select **Input Settings**
from the main menu during emulation, and then select **Analog Input
Adjustments** from the Input Settings Menu (this item only appears on the Input
Settings menu for systems with analog controls).


.. _ui-inptcfg-digital:

Digital input settings
~~~~~~~~~~~~~~~~~~~~~~

Each emulated digital input has a single assignment setting.  For flexibility,
MAME can combine controls (keys, buttons and joystick axes) using logical
**and**, **not** and **or** operations.  This is best illustrated with some
examples:

Kbd 1
    In this simple case, pressing the **1** key on the keyboard activates the
    emulated input or user interface control.
Kbd Down or Joy 1 Down
    Pressing the down arrow on the keyboard or moving the first joystick down
    activates the emulated input or user interface control.
Kbd P not Kbd Shift not Kbd Right Shift
    Pressing the **P** key on the keyboard while not pressing either **Shift**
    key activates the emulated input or user interface control.  MAME does not
    show the implicit **and** operations.
Kbd P Kbd Shift or Kbd P Kbd Right Shift
    Pressing the **P** key while also pressing either of the **Shift** keys
    activates the emulated input or user interface control.  Once again, the
    implicit **and** operations are not shown.

(In technical terms, MAME uses Boolean sum of products logic to combine inputs.)

When a digital input setting is highlighted, the prompt below the menu shows
whether selecting it will replace the current assignment or append an **or**
operation to it.  Press **UI Left/Right** before selecting the setting to switch
between replacing the assignment or appending an **or** operation to it.  Press
**UI Clear** (**Delete** or **Forward Delete** by default) to clear the
highlighted setting, or restore the default assignment if it is currently
cleared.

When you select a digital input setting, MAME will wait for you to enter an
input or a combination of inputs for a logical **and** operation:

* Press a key or button or move an analog control once to add it to the **and**
  operation.
* Press a key or button or move an analog control twice to add a **not** item to
  the **and** operation.  Pressing the same key or button or moving the same
  analog control additional times toggles the **not** on and off.
* Pressing **UI Cancel** (**Escape** by default) *before* activating any other
  controls clears the setting or restores the default assignment.
* Press **UI Cancel** *after* activating another control to leave the setting
  unchanged.
* The new setting is shown below the menu.  Wait one second after activating an
  input to accept the new setting.

Here’s how to produce some example settings:

Kbd 1
    Press the **1** key on the keyboard once, then wait one second to accept the
    setting.
Kbd F12 Kbd Shift Keyboard Alt
    Press the **F12** key on the keyboard once, press the left **Shift** key
    once, press the left **Alt** key once, then wait one second to accept the
    setting.
Kbd P not Kbd Shift not Kbd Right Shift
    Press the **P** key on the keyboard once, press the left **Shift** key
    twice, press the right **Shift** key twice, then wait one second to accept
    the setting.


.. _ui-inptcfg-analog:

Analog input settings
~~~~~~~~~~~~~~~~~~~~~

Each emulated analog input has three assignment settings:

* Use the *axis setting* to assign an analog axis to control the emulated analog
  input.  The axis setting uses the name of the input with the suffix “Analog”.
  For example the axis setting for the steering wheel in Ridge Racer is called
  **Steering Wheel Analog**.
* Use the *increment setting* assign a control (or combination of controls) to
  increase the value of the emulated analog input.  The increment setting uses
  the name of the input with the suffix “Analog Inc”.  For example the increment
  setting for the steering wheel in Ridge Racer is called **Steering Wheel
  Analog Inc**.  This is a digital input setting – if an analog axis is
  assigned to it, MAME will not increase the emulated input value at a
  proportional speed.
* Use the *decrement setting* assign a control (or combination of controls) to
  decrease the value of the emulated analog input.  The decrement setting uses
  the name of the input with the suffix “Analog Dec”.  For example the decrement
  setting for the steering wheel in Ridge Racer is called **Steering Wheel
  Analog Dec**.  This is a digital input setting – if an analog axis is
  assigned to it, MAME will not decrease the emulated input value at a
  proportional speed.

The increment and decrement settings are most useful for controlling an emulated
analog input using digital controls (for example keyboard keys, joystick
buttons, or a directional pad).  They are configured in the same way as emulated
digital inputs (:ref:`see above <ui-inptcfg-digital>`).  **It’s important that
you don’t assign the same control to the axis setting as well as the increment
and/or decrement settings for the same emulated input at the same time.**  For
example if you assign Ridge Racer’s **Steering Wheel Analog** setting to the X
axis of the left analog stick on your controller, you *should not* assign either
the **Steering Wheel Analog Inc** or **Steering Wheel Analog Dec** setting to
the X axis of the same analog stick.

You can assign one or more analog axes to the axis setting for an emulated
analog input.  When multiple axes are assigned to an axis setting, they will be
added together, but absolute position controls will override relative position
controls.  For example suppose for Arkanoid you assign the **Dial Analog** axis
setting to **Mouse X or Joy 1 LSX or Joy 1 RSX** on a mouse and Xbox-style
controller.  You will be able to control the paddle with the mouse or either
analog stick, but the mouse will only take effect if both analog sticks are in
the neutral position (centred) on the X axis.  If either analog stick is *not*
centred on the X axis, the mouse will have no effect, because a mouse is a
relative position control while joysticks are absolute position controls.

For absolute position controls like joysticks and pedals, MAME allows you to
assign either the full range of an axis or the range on one side of the neutral
position (a *half axis*) to an axis setting.  Assigning a half axis is usually
used for pedals or other absolute inputs where the neutral position is at one
end of the input range.  For example suppose for **Ridge Racer** you assign the
**Brake Pedal Analog** setting to the portion of a vertical joystick axis below
the neutral position.  If the joystick is at or above the neutral position
vertically, the brake pedal will be released; if the joystick is below the
neutral position vertically, the brake pedal will be applied proportionally.
Half axes are displayed as the name of the axis followed by a plus or minus sign
(**+** or **-**).  Plus refers to the portion of the axis below or to the right
of the neutral position; minus refers to the portion of the axis above or to the
left of the neutral position.  For pedal or analog trigger controls, the active
range is treated as being above the neutral position (the half axis indicated by
a minus sign).

When keys or buttons are assigned to an axis setting, they conditionally enable
analog controls assigned to the setting.  This can be used in conjunction with
an absolute position control to create a “sticky” control.

Here are some examples of some possible axis setting assignments, assuming an
Xbox-style controller and a mouse are used:

Joy 1 RSY
    Use vertical movement of the right analog stick to control the emulated
    input.
Mouse X or Joy 1 LT or Joy 1 RT Reverse
    Use horizontal mouse movement, or the left and right triggers to control the
    emulated input.  The right trigger is reversed so it acts in the opposite
    direction to the left trigger.
Joy 1 LB Joy 1 LSX
    Use horizontal movement of the left analog stick to control the emulated
    input, but *only* while holding the left shoulder button.  If the left
    shoulder button is released while the left analog stick is not centred
    horizontally, the emulated input will hold its value until the left shoulder
    button is pressed again (a “sticky” control).
not Joy 1 RB Joy 1 RSX or Joy 1 RB Joy 1 RSX Reverse
    Use horizontal movement of the right analog stick to control the emulated
    input, but invert the control if the right shoulder button is held.

When you select an axis setting, MAME will wait for you to enter an input:

* Move an analog control to assign it to the axis setting.
* Press a key or button (or a combination of keys or buttons) *before* moving an
  analog control to conditionally enable the analog control.
* When appending to a setting, if the last assigned control is an absolute
  position control, move the same control again to cycle between the full range
  of the axis, the portion of the axis on either side of the neutral position,
  and the full range of the axis reversed.
* When appending to a setting, if the last assigned control is a relative
  position control, move the same control again to toggle reversing the
  direction of the control on or off.
* When appending to a setting, move an analog control other than the last
  assigned control or press a key or button to add an **or** operation.
* Pressing **UI Cancel** (**Escape** by default) *before* activating any other
  controls clears the setting or restores the default assignment.
* Pressing **UI Cancel** *after* activating another control leaves the setting
  unchanged.
* The new setting is shown below the menu.  Wait one second after moving an
  analog control to accept the new setting.

To adjust sensitivity, auto-centring speed and inversion settings for emulated
analog inputs, or to see how they respond to controls with your settings, select
**Input Settings** from the main menu during emulation, and then select **Analog
Input Adjustments** from the Input Settings Menu.  Settings for emulated analog
inputs are grouped by device and sorted by type.  You can move between devices
with the next group and previous group keys/buttons (opening/closing brackets
**[** and **]** on the keyboard by default).  The state of the emulated analog
inputs is shown below the menu, and reacts in real time.  Press the **On Screen
Display** key or button (the backtick/tilde key by default on a US ANSI QWERTY
keyboard) to hide the menu to make it easier to test without changing settings.
Press the same key or button to show the menu again.

Each emulated input has four settings on the **Analog Controls** menu:

* The *increment/decrement speed* setting controls how fast the input value
  increases or decreases in response to the controls assigned to the
  increment/decrement settings.
* The *auto-centering speed* setting controls how fast the input value returns
  to the neutral state when the controls assigned to the increment/decrement
  settings are released.  Setting it to zero (**0**) will result in the value
  not automatically returning to the neutral position.
* The *reverse* setting allows the direction of the emulated input’s response
  to controls to be inverted.  This applies to controls assigned to the axis
  setting *and* the increment/decrement settings.
* The *sensitivity* setting adjusts the input value’s response to the control
  assigned to the axis setting.


Use the UI left/right keys or buttons to adjust the highlighted setting.
Selecting a setting or pressing the UI clear key/button (**Forward Delete** by
default) restores its default value.

The units for the increment/decrement speed, auto-centering speed and
sensitivity settings are tied to the driver/device implementation.  The
increment/decrement speed and auto-centering speed settings are also tied to the
frame rate of the first emulated screen in the system.  The response to controls
assigned to the increment/decrement settings will change if the system changes
the frame rate of this screen.


.. _ui-selmenu:

The system and software selection menus
---------------------------------------

If you start MAME without specifying a system on the command line, the system
selection menu will be shown (assuming the
:ref:`ui option <mame-commandline-ui>` is set to **cabinet**).  The system
selection menu is also shown if you select **Select New System** from the main
menu during emulation.  Selecting a system that uses software lists shows the
similar software selection menu.

The system and software selection menus have the following parts:

* The heading area at the top, showing the emulator name and version, the number
  of systems or software items in the menu, and the current search text.  The
  software selection menu also shows the name of the selected system.
* The toolbar immediately below the heading area.  The exact toolbar buttons
  shown depend on the menu.  Hover the mouse pointer over a button to see a
  description.  Click a button to select it.

  Toolbar buttons are add/remove highlighted system/software from favourites
  (star), export displayed list to file (diskette), audit media (magnifying
  glass), show info viewer (“i” emblazoned on blue circle), return to previous
  menu (bent arrow on blue), and exit (cross on red).
* The list of systems or software in the centre.  For the system selection menu,
  there are configuration options below the list of systems.  Clones are shown
  with a different text colour (grey by default).  You can right-click a system
  name as a shortcut to show the System Settings menu for the system.

  Systems or software items are sorted by full name or description, keeping
  clones immediately below their parents.  This may appear confusing if your
  filter settings cause a parent system or software item to be hidden while one
  or more of its clones are visible.
* The info panel at the bottom, showing summary information about the
  highlighted system or software.  The background colour changes depending on
  the emulation status: green for working, amber for imperfectly emulated
  features or known issues, or red for more serious issues.

  A yellow star is show at the top left of the info panel if the highlighted
  system or software is in your favourites list.
* The collapsible list of filter options on the left.  Click a filter to apply
  it to the list of systems/software.  Some filters show a menu with additional
  options (e.g. specifying the manufacturer for the **Manufacturer** filter, or
  specifying a file and group for the **Category** filter).

  Click **Unfiltered** to display all items.  Click **Custom Filter** to combine
  multiple filters.  Click the strip between the list of filters and the list of
  systems/software to show or hide the list of filters.  Be aware that filters
  still apply when the list of filters is hidden.
* The collapsible info viewer on the right.  This has two tabs for showing
  images and information.  Click a tab to switch tabs; click the left- or
  right-facing triangles next to the image/info title to switch between images
  or information sources.

  Emulation information is automatically shown for systems, and information from
  the software list is shown for software items.  Additional information from
  external files can be shown using the :ref:`Data plugin <plugins-data>`.

You can type to search the displayed list of systems or software.  Systems are
searched by full name, manufacturer and full name, and short name.  If you are
using localised system names, phonetic names will also be searched if present.
Software items are searched by description, alternate titles (``alt_title``
info elements in the software lists), and short name.  **UI Cancel** (Escape by
default) will clear the search if currently searching.


.. _ui-selmenu-nav:

Navigation controls
~~~~~~~~~~~~~~~~~~~

In addition to the usual :ref:`menu navigation controls <ui-menus>`, the system
and software selection menus have additional configurable controls for
navigating the multi-pane layout, and providing alternatives to toolbar buttons
if you don’t want to use a pointing device.  The default additional controls
(with a US ANSI QWERTY keyboard), and the settings they correspond to, are:

Tab (UI Focus Next)
    Move focus to the next area.  The order is system/software list,
    configuration options (if visible), filter list (if visible), info/image
    tabs (if visible), info/image source (if visible).
Shift+Tab (UI Focus Previous)
    Move focus to the previous area.
Alt+D (UI External DAT View)
    Show the full-size info viewer.
Alt+F (UI Add/Remove favorite)
    Add or remove the highlighted system or software item from the favourites
    list.
F1 (UI Audit Media)
    Audit ROMs and/or disk images for systems.  The results are saved for use
    with the **Available** and **Unavailable** filters.

When focus is on the filter list, you can use the menu navigation controls (up,
down, home and end) to highlight a filter, and **UI Select** (Return/Enter by
default) apply it.

When focus is on any area besides the info/image tabs, you can change the image
or info source with left/right.  When focus is on the info/image tabs,
left/right switch between tabs.  When focus is on the image/info tabs or source,
you can scroll the info using up, down, page up, page down, home and end.


.. _ui-simpleselmenu:

The simple system selection menu
--------------------------------

If you start MAME without specifying a system on the command line (or choose
**Select New System** from the main menu during emulation) with the
:ref:`ui option <mame-commandline-ui>` set to **simple**, the simple system
selection menu will be shown.  The simple system selection menu shows fifteen
randomly selected systems that have ROM sets present in your configured
:ref:`ROM folder(s) <mame-commandline-rompath>`.  You can type to search for a
system.  Clearing the search causes fifteen systems to be randomly selected
again.

The info panel below the menu shows summary information about the highlighted
system.  The background colour changes depending on the emulation status: green
for working, amber for imperfectly emulated features or known issues, or red for
more serious issues.
