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
bookkeeping data.  This example shows the overall structure of a controller
configuration file:

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
``system`` element contains an ``input`` element which holds the actual
``remap`` and ``port`` configuration elements, which will be described later.

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
that use MAME’s default assignment for the control type.  That is, they only
apply to default assignments for control types set in the “Inputs (general)”
menu.  They *do not* apply to default input assignments set in driver/device I/O
port definitions (using the ``PORT_CODE`` macro).

MAME applies ``remap`` elements found inside any applicable ``system`` element.


.. _ctrlrcfg-typeoverride:

Overriding defaults by control type
-----------------------------------

Use ``port`` elements with ``type`` attributes but without ``tag`` attributes to
override the default host input assignments for a controls:

.. code-block:: XML

    <input>
        <port type="UI_CONFIGURE">
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

Config Menu (User Interface)
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
the control type.  That is, ``port`` elements without ``tag`` attributes only
override default assignments for control types set in the “Inputs (general)”
menu.  They *do not* override default input assignments set in driver/device I/O
port definitions (using the ``PORT_CODE`` macro).

MAME applies ``port`` elements without ``tag`` attributes found inside any
applicable ``system`` element.


.. _ctrlrcfg-ctrloverride:

Overriding defaults for specific controls
-----------------------------------------

Use ``port`` elements with ``tag``, ``type``, ``mask`` and ``defvalue``
attributes to override defaults for specific controls.  These ``port`` elements
should only occur inside ``system`` elements that apply to particular systems or
source files (i.e. they should not occur inside ``system`` elements where the
``name`` attribute has the value ``default``).  The default host input
assignments can be overridden, as well as the toggle setting for digital
controls.

The ``tag``, ``type``, ``mask`` and ``defvalue`` are used to identify the
affected input.  You can find out the values to use for a particular input by
changing its assigned host input, exiting MAME, and checking the values in the
system configuration file.  Note that these values are not guaranteed to be
stable, and may change between MAME versions.

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

This sets the host inputs to steer left and right to the K and J keys,
respectively, and disables the toggle setting for the gear shift input.
