.. _layscript:

MAME Layout Scripting
=====================

.. contents:: :local:


.. _layscript-intro:

Introduction
------------

MAME layout files can embed Lua script to provide enhanced functionality.
Although there’s a lot you can do with conditionally drawn components and
parameter animation, some things can only be done with scripting.  MAME uses an
event-based model.  Scripts can supply function that will be called after
certain events, or when certain data is required.

Layout scripting requires the layout plugin to be enabled.  For example, to run
BWB Touble Take with the Lua script in the layout enabled, you might use this
command::

    mame64 -plugins -plugin layout v4dbltak

If you may want to add the settings to enable the layout plugin to an INI file
to save having to enable it every time you start a system.


.. _layscript-examples:

Practical examples
------------------

Before diving into the technical details of how it works, we’ll start with some
complete examples using Lua script to enhance layouts.

Espial: joystick split across ports
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Take a look at the player input definitions for Espial:

.. code-block:: C++

    PORT_START("IN1")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL

    PORT_START("IN2")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY

There are two joysticks, one used for both players on an upright cabinet or the
first player on a cocktail cabinet, and one used for the second player on a
cocktail cabinet.  Notice that the switches for the first joystick are split
across the two I/O ports.

There’s no layout file syntax to build element state using bits from multiple
I/O ports.  It’s also inconvenient if each joystick needs to be defined as a
separate element because the bits for the switches aren’t arranged the same
way.

We can overcome these limitations using a script to read the player inputs and
set the items’ element state:

.. code-block:: XML

    <?xml version="1.0"?>
    <mamelayout version="2">

        <!-- element for drawing a joystick -->
        <!-- up = 1 (bit 0), down = 2 (bit 1), left = 4 (bit 2), right = 8 (bit 3) -->
        <element name="stick" defstate="0">
            <image state="0x0" file="stick_c.svg" />
            <image state="0x1" file="stick_u.svg" />
            <image state="0x9" file="stick_ur.svg" />
            <image state="0x8" file="stick_r.svg" />
            <image state="0xa" file="stick_dr.svg" />
            <image state="0x2" file="stick_d.svg" />
            <image state="0x6" file="stick_dl.svg" />
            <image state="0x4" file="stick_l.svg" />
            <image state="0x5" file="stick_ul.svg" />
        </element>

        <!-- we'll warn the user if the layout plugin isn't enabled -->
        <!-- draw only when state is 1, and set the default state to 1 so warning is visible initially -->
        <element name="warning" defstate="1">
            <text state="1" string="This view requires the layout plugin." />
        </element>

        <!-- view showing the screen and joysticks on a cocktail cabinet -->
        <view name="Joystick Display">
            <!-- draw the screen with correct aspect ratio -->
            <screen index="0">
                <bounds x="0" y="0" width="4" height="3" />
            </screen>

            <!-- first joystick, id attribute allows script to find item -->
            <!-- no bindings, state will be set by the script -->
            <element id="joy_p1" ref="stick">
                <!-- position below the screen -->
                <bounds xc="2" yc="3.35" width="0.5" height="0.5" />
            </element>

            <!-- second joystick, id attribute allows script to find item -->
            <!-- no bindings, state will be set by the script -->
            <element id="joy_p2" ref="stick">
                <!-- screen is flipped for second player, so rotate by 180 degrees -->
                <orientation rotate="180" />
                <!-- position above the screen -->
                <bounds xc="2" yc="-0.35" width="0.5" height="0.5" />
            </element>

            <!-- warning text item also has id attribute so the script can find it -->
            <element id="warning" ref="warning">
                <!-- position over the screen near the bottom -->
                <bounds x="0.2" y="2.6" width="3.6" height="0.2" />
            </element>
        </view>

        <!-- the content of the script element will be called as a function by the layout plugin -->
        <!-- use CDATA block to avoid the need to escape angle brackets and ampersands -->
        <script><![CDATA[
            -- file is the layout file object
            -- set a function to call after resolving tags
            file:set_resolve_tags_callback(
                    function ()
                        -- file.device is the device that caused the layout to be loaded
                        -- in this case, it's the root machine driver for espial
                        -- look up the two I/O ports we need to be able to read
                        local in1 = file.device:ioport("IN1")
                        local in2 = file.device:ioport("IN2")

                        -- look up the view items for showing the joystick state
                        local p1_stick = file.views["Joystick Display"].items["joy_p1"]
                        local p2_stick = file.views["Joystick Display"].items["joy_p2"]

                        -- set a function to call before adding the view items to the render target
                        file.views["Joystick Display"]:set_prepare_items_callback(
                                function ()
                                    -- read the two player input I/O ports
                                    local in1_val = in1:read()
                                    local in2_val = in2:read()

                                    -- set element state for first joystick
                                    p1_stick:set_state(
                                            ((in2_val & 0x10) >> 4) |   -- shift up from IN2 bit 4 to bit 0
                                            ((in1_val & 0x20) >> 4) |   -- shift down from IN1 bit 5 to bit 1
                                            ((in2_val & 0x80) >> 5) |   -- shift left from IN2 bit 7 to bit 2
                                            (in2_val & 0x08))           -- right is in IN2 bit 3

                                    -- set element state for second joystick
                                    p2_stick:set_state(
                                            ((in1_val & 0x10) >> 4) |   -- shift up from IN1 bit 4 to bit 0
                                            ((in1_val & 0x40) >> 5) |   -- shift down from IN1 bit 6 to bit 1
                                            (in1_val & 0x04) |          -- left is in IN1 bit 2
                                            (in1_val & 0x08))           -- right is in IN1 bit 3
                                end)

                        -- hide the warning, since if we got here the script is running
                        file.views["Joystick Display"].items["warning"]:set_state(0)
                    end)
        ]]></script>

    </mamelayout>

The layout has a ``script`` element containing the Lua script.  This is called
as a function by the layout plugin when the layout file is loaded.  The layout
views have been built at this point, but the emulated system has not finished
starting.  In particular, it’s not safe to access inputs and outputs at this
time.  The key variable in the script environment is ``file``, which gives the
script access its layout file.

We supply a function to be called after tags in the layout file have been
resolved.  At this point, the emulated system will have completed starting.
This function does the following tasks

* Looks up the two I/O ports used for player input.  I/O ports can be looked up
  by tag relative to the device that caused the layout file to be loaded.
* Looks up the two view items used to display joystick state.  Views can be
  looked up by name (i.e. value of the ``name`` attribute), and items within a
  view can be looked up by ID (i.e. the value of the ``id`` attribute).
* Supplies a function to be called before view items are added to the render
  target.
* Hides the warning that reminds the user to enable the layout plugin by setting
  the element state for the item to 0 (the text component is only drawn when
  the element state is 1).

The function called before view items are added to the render target reads the
player inputs, and shuffle the bits into the order needed by the joystick
element.
