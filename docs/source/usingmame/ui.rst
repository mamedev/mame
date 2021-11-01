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
    Clear the search if searching the menu, otherwise close the menu, returning
    to the previous menu, or returning to the emulated machine for the main menu
    (there’s usually an item at the bottom of the menu for the same purpose).
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


.. _ui-selmenu:

The system and software selection menus
---------------------------------------

If you start MAME without specifying a system on the command line, the system
selection menu will be shown (assuming the
:ref:`ui option <mame-commandline-ui>` is set to **cabinet**).  The system
selection menu is also shown if you select **Select New Machine** from the main
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
  name as a shortcut to show the machine configuration options for the system.

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
if you don’t want to use a pointing device.  The default additional controls (on
a US ANSI QWERTY keyboard), and the settings they correspond to, are:

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
**Select New Machine** from the main menu during emulation) with the
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
