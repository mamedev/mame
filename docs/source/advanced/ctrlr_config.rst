.. _ctrlrcfg:

Controller Configuration Files
==============================

.. contents:: :local:


.. _ctrlrcfg-intro:

Introduction
------------

Controller configuration files can be used to modify MAME’s default input
settings.  Controller configuration files may be supplied with an input device
to provide more suitable defaults, or used as profiles that can be selected for
different situations.  MAME includes a few sample controller configuration files
in the **ctrlr** folder, designed to provide useful defaults for certain
arcade-style controllers.

Controller configuration files are an XML application, using the ``.cfg``
filename extension.  MAME searches for controller configuration files in the
directories specified using the ``ctrlrpath`` option.  A controller
configuration file is selected by setting the ``ctrlr`` option to its filename,
excluding the ``.cfg`` extension (e.g. set the ``ctrlr`` option to
``scorpionxg`` to use **scorpionxg.cfg**).  It is an error if the specified
controller configuration file does not exist, or if it contains no sections
applicable to the emulated system.

Controller configuration files use implementation-dependent input tokens.  The
values available and their precise meanings depend on the exact version of MAME
used, the input devices connected, the selected input provider modules
(``keyboardprovider``, ``mouseprovider``, ``lightgunprovider`` and
``joystickprovider`` options), and possibly other settings.


.. _ctrlrcfg-structure:

Basic structure
---------------

Controller configuration files follow a similar format to the system
configuration files that MAME uses to save things like input settings and
bookkeeping data (created in the folder specified using the
:ref:`cfg_directory option <mame-commandline-cfgdirectory>`).  This example
shows the overall structure of a controller configuration file:

.. code-block:: XML

    <?xml version="1.0"?>
    <mameconfig version="10">
        <system name="default">
            <input>
                <!-- settings affecting all emulated systems go here -->
            </input>
        </system>
        <system name="neogeo">
            <input>
                <!-- settings affecting neogeo and clones go here -->
            </input>
        </system>
        <system name="intellec4.cpp">
            <input>
                <!-- settings affecting all systems defined in intellec4.cpp go here -->
            </input>
        </system>
    </mameconfig>

The root of a controller configuration file must be a ``mameconfig`` element,
with a ``version`` attribute specifying the configuration format version
(currently ``10`` – MAME will not load a file using a different version).  The
``mameconfig`` element contains one or more ``system`` elements, each of which
has a ``name`` attribute specifying the system(s) it applies to.  Each
``system`` element may contain an ``input`` element which holds the actual
``remap`` and ``port`` configuration elements, which will be described later.
Each ``system`` element may also contain a ``pointer_input`` element to set
pointer input options for systems with interactive artwork.

When launching an emulated system, MAME will apply configuration from ``system``
elements where the value of the ``name`` attribute meets one of the following
criteria:

* If the ``name`` attribute has the value ``default``, it will always be applied
  (including for the system/software selection menus).
* If the value of the ``name`` attribute matches the system’s short name, the
  short name of its parent system, or the short name of its BIOS system (if
  applicable).
* If the value of the ``name`` attribute matches the name of the source file
  where the system is defined.

For example, for the game “DaeJeon! SanJeon SuJeon (AJTUE 990412 V1.000)”,
``system`` elements will be applied if their ``name`` attribute has the value
``default`` (applies to all systems), ``sanjeon`` (short name of the system
itself), ``sasissu`` (short name of the parent system), ``stvbios`` (short
name of the BIOS system), or ``stv.cpp`` (source file where the system is
defined).

As another example, a ``system`` element whose ``name`` attribute has the value
``zac2650.cpp`` will be applied for the systems “The Invaders”, “Super Invader
Attack (bootleg of The Invaders)”, and “Dodgem”.

Applicable ``system`` elements are applied in the order they appear in the
controller configuration file.  Settings from elements that appear later in the
file may modify or override settings from elements that appear earlier.  Within
a ``system`` element, ``remap`` elements are applied before ``port`` elements.


.. _ctrlrcfg-substitute:

Substituting default controls
-----------------------------

You can use a ``remap`` element to substitute one host input for another in
MAME’s default input configuration.  For example, this substitutes keys on the
numeric keypad for the cursor direction keys:

.. code-block:: XML

    <input>
        <remap origcode="KEYCODE_UP" newcode="KEYCODE_8PAD" />
        <remap origcode="KEYCODE_DOWN" newcode="KEYCODE_2PAD" />
        <remap origcode="KEYCODE_LEFT" newcode="KEYCODE_4PAD" />
        <remap origcode="KEYCODE_RIGHT" newcode="KEYCODE_6PAD" />
    </input>

The ``origcode`` attribute specifies the token for the host input to be
substituted, and the ``newcode`` attribute specifies the token for the
replacement host input.  In this case, assignments using the cursor up, down,
left and right arrows will be replaced with the numeric 8, 2, 4 and 6 keys on
the numeric keypad, respectively.

Note that substitutions specified using ``remap`` elements only apply to inputs
that use MAME’s default assignment for the input type.  That is, they only apply
to default assignments for control types set in the “Input Assignments
(general)” menus.  They *do not* apply to default control assignments set in
driver/device I/O port definitions (using the ``PORT_CODE`` macro).

MAME applies ``remap`` elements found inside any applicable ``system`` element.


.. _ctrlrcfg-typeoverride:

Overriding defaults by input type
---------------------------------

Use ``port`` elements with ``type`` attributes but without ``tag`` attributes to
override the default control assignments for emulated inputs by type:

.. code-block:: XML

    <input>
        <port type="UI_MENU">
            <newseq type="standard">KEYCODE_TAB OR KEYCODE_1 KEYCODE_5</newseq>
        </port>
        <port type="UI_CANCEL">
            <newseq type="standard">KEYCODE_ESC OR KEYCODE_2 KEYCODE_6</newseq>
        </port>

        <port type="P1_BUTTON1">
            <newseq type="standard">KEYCODE_C OR JOYCODE_1_BUTTON1</newseq>
        </port>
        <port type="P1_BUTTON2">
            <newseq type="standard">KEYCODE_LSHIFT OR JOYCODE_1_BUTTON2</newseq>
        </port>
        <port type="P1_BUTTON3">
            <newseq type="standard">KEYCODE_Z OR JOYCODE_1_BUTTON3</newseq>
        </port>
        <port type="P1_BUTTON4">
            <newseq type="standard">KEYCODE_X OR JOYCODE_1_BUTTON4</newseq>
        </port>
    </input>

This sets the following default input assignments:

Show/Hide Menu (User Interface)
    Tab key, or 1 and 2 keys pressed simultaneously
UI Cancel (User Interface)
    Escape key, or 2 and 6 keys pressed simultaneously
P1 Button 1 (Player 1 Controls)
    C key, or joystick 1 button 1
P1 Button 2 (Player 1 Controls)
    Left Shift key, or joystick 1 button 2
P1 Button 3 (Player 1 Controls)
    Z key, or joystick 1 button 3
P1 Button 4 (Player 1 Controls)
    X key, or joystick 1 button 4

Note that this will only apply for inputs that use MAME’s default assignment for
the input type.  That is, ``port`` elements without ``tag`` attributes only
override default assignments for control types set in the “Input Assignments
(general)” menus.  They *do not* override default control assignments set in
driver/device I/O port definitions (using the ``PORT_CODE`` macro).

MAME applies ``port`` elements without ``tag`` attributes found inside any
applicable ``system`` element.


.. _ctrlrcfg-ctrloverride:

Overriding defaults for specific inputs
---------------------------------------

Use ``port`` elements with ``tag``, ``type``, ``mask`` and ``defvalue``
attributes to override defaults for specific inputs.  These ``port`` elements
should only occur inside ``system`` elements that apply to particular systems or
source files (i.e. they should not occur inside ``system`` elements where the
``name`` attribute has the value ``default``).  The default control assignments
can be overridden, as well as the toggle setting for digital inputs.

The ``tag``, ``type``, ``mask`` and ``defvalue`` are used to identify the
affected input.  You can find out the values to use for a particular input by
changing its control assignment, exiting MAME, and checking the values in the
system configuration file (created in the folder specified using the
:ref:`cfg_directory option <mame-commandline-cfgdirectory>`).  Note that these
values are not guaranteed to be stable, and may change between MAME versions.

Here’s an example that overrides defaults for 280-ZZZAP:

.. code-block:: XML

    <system name="280zzzap">
        <input>
            <port tag=":IN0" type="P1_BUTTON2" mask="16" defvalue="0" toggle="no" />
            <port tag=":IN1" type="P1_PADDLE" mask="255" defvalue="127">
                <newseq type="increment">KEYCODE_K</newseq>
                <newseq type="decrement">KEYCODE_J</newseq>
            </port>
        </input>
    </system>

This sets the controls to steer left and right to the K and J keys,
respectively, and disables the toggle setting for the gear shift input.


.. _ctrlrcfg-mapdevice:

Assigning input device numbers
------------------------------

Use ``mapdevice`` elements with ``device`` and ``controller`` attributes to
assign stable numbers to input devices.  Note that all devices explicitly
configured in this way must be connected when MAME starts for this to work as
expected.

Set the ``device`` attribute to the device ID of the input device, and set the
``controller`` attribute to the desired input device token (device type and
number).

Here’s an example numbering two light guns and two XInput game controllers:

.. code-block:: XML

    <system name="default">
        <input>
            <mapdevice device="VID_D209&amp;PID_1601" controller="GUNCODE_1" />
            <mapdevice device="VID_D209&amp;PID_1602" controller="GUNCODE_2" />
            <mapdevice device="XInput Player 1" controller="JOYCODE_1" />
            <mapdevice device="XInput Player 2" controller="JOYCODE_2" />
        </input>
    </system>

MAME applies ``mapdevice`` elements found inside the first applicable ``system``
element only.  To avoid confusion, it’s simplest to place the ``system`` element
applying to all systems (``name`` attribute set to ``default``) first in the
file, and use it to assign input device numbers.


.. _ctrlrcfg-pointers:

Setting pointer input options
-----------------------------

A ``pointer_input`` element may contain ``target`` elements to set pointer input
options for each output screen or window.  Each ``target`` element must have an
``index`` attribute containing the zero-based index of the screen to which it
applies.

Each ``target`` element may have an ``activity_timeout`` attribute to set the
time after which a mouse pointer that has not moved and has no buttons pressed
will be considered inactive.  The value is specified in seconds, and must be in
the range of 0.1 seconds to 10 seconds, inclusive.

Each ``target`` element may have a ``hide_inactive`` element to set whether
inactive pointers may be hidden.  If the value is ``0`` (zero), inactive
pointers will not be hidden.  If the value is ``1``, inactive pointers may be
hidden, but layout views can still specify that inactive pointers should not be
hidden.

Here’s an example demonstrating the use of this feature:

.. code-block:: XML

    <system name="default">
        <pointer_input>
            <target index="0" activity_timeout="1.5" />
        </pointer_input>
    </system>
    <system name="intellec4.cpp">
        <pointer_input>
            <target index="0" hide_inactive="0" />
        </pointer_input>
    </system>

For all systems, pointers over the first output screen or window will be
considered inactive after not moving for 1.5 seconds with no buttons pressed.
For systems defined in ``intellec4.cpp``, inactive pointers over the first
window will not be hidden.
