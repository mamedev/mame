.. _ui:

MAME’s User Interface
=====================

.. contents:: :local:


.. _ui-intro:

Introduction
------------

MAME provides a simple user interface for selecting a system and software to
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
    Close the menu, returning to the previous menu, or returning to the
    emulated machine for the main menu (there’s usually an item at the bottom
    of the menu for the same purpose).
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
* **UI Add Remove favorite**, **UI Export List** and **UI Audit Media** if you
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
