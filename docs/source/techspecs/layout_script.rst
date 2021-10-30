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
event-based model.  Scripts can supply functions that will be called after
certain events, or when certain data is required.

Layout scripting requires the :ref:`layout plugin <plugins-layout>` to be
enabled.  For example, to run BWB Double Take with the Lua script in the layout
enabled, you might use this command::

    mame -plugins -plugin layout v4dbltak

You may want to add the settings to enable the layout plugin to an INI file to
save having to enable it every time you start a system.  See :ref:`plugins` for
more information about using plugins with MAME.


.. _layscript-examples:

Practical examples
------------------

Before diving into the technical details of how it works, we’ll start with some
example layout files using Lua script for enhancement.  It’s assumed that you’re
familiar with MAME’s artwork system and have a basic understanding of Lua
scripting.  For details on MAME’s layout file, see :ref:`layfile`; for an
introduction to Lua scripting in MAME, see :ref:`luaengine`; for detailed
descriptions of MAME’s Lua classes, see :ref:`luareference`.

.. _layscript-examples-espial:

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

There’s no layout file syntax to build the element state using bits from
multiple I/O ports.  It’s also inconvenient if each joystick needs to be defined
as a separate element because the bits for the switches aren’t arranged the same
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
script access to its layout file.

We supply a function to be called after tags in the layout file have been
resolved.  At this point, the emulated system will have completed starting.
This function does the following tasks:

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
player inputs, and shuffles the bits into the order needed by the joystick
element.

.. _layscript-examples-starwars:

Star Wars: animation on two axes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We’ll make a layout that shows the position of the flight yoke for Atari Star
Wars.  The input ports are straightforward – each analog axis produces a value
in the range from 0x00 (0) to 0xff (255), inclusive:

.. code-block:: C++

    PORT_START("STICKY")
    PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30)

    PORT_START("STICKX")
    PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)

Here’s our layout file:

.. code-block:: XML

    <?xml version="1.0"?>
    <mamelayout version="2">

        <!-- a square with a white outline 1% of its width -->
        <element name="outline">
            <rect><bounds x="0.00" y="0.00" width="1.00" height="0.01" /></rect>
            <rect><bounds x="0.00" y="0.99" width="1.00" height="0.01" /></rect>
            <rect><bounds x="0.00" y="0.00" width="0.01" height="1.00" /></rect>
            <rect><bounds x="0.99" y="0.00" width="0.01" height="1.00" /></rect>
        </element>

        <!-- a rectangle with a vertical line 10% of its width down the middle -->
        <element name="line">
            <!-- use a transparent rectangle to force element dimensions -->
            <rect>
                <bounds x="0" y="0" width="0.1" height="1" />
                <color alpha="0" />
            </rect>
            <!-- this is the visible white line -->
            <rect><bounds x="0.045" y="0" width="0.01" height="1" /></rect>
        </element>

        <!-- an outlined square inset by 20% with lines 10% of the element width/height -->
        <element name="box">
            <!-- use a transparent rectangle to force element dimensions -->
            <rect>
                <bounds x="0" y="0" width="0.1" height="0.1" />
                <color alpha="0" />
            </rect>
            <!-- draw the outlined of a square -->
            <rect><bounds x="0.02" y="0.02" width="0.06" height="0.01" /></rect>
            <rect><bounds x="0.02" y="0.07" width="0.06" height="0.01" /></rect>
            <rect><bounds x="0.02" y="0.02" width="0.01" height="0.06" /></rect>
            <rect><bounds x="0.07" y="0.02" width="0.01" height="0.06" /></rect>
        </element>

        <!-- we'll warn the user if the layout plugin isn't enabled -->
        <!-- draw only when state is 1, and set the default state to 1 so warning is visible initially -->
        <element name="warning" defstate="1">
            <text state="1" string="This view requires the layout plugin." />
        </element>

        <!-- view showing the screen and flight yoke position -->
        <view name="Analog Control Display">
            <!-- draw the screen with correct aspect ratio -->
            <screen index="0">
                <bounds x="0" y="0" width="4" height="3" />
            </screen>

            <!-- draw the white outlined square to the right of the screen near the bottom -->
            <!-- the script uses the size of this item to determine movement ranges -->
            <element id="outline" ref="outline">
                <bounds x="4.1" y="1.9" width="1.0" height="1.0" />
            </element>

            <!-- vertical line for displaying X axis input -->
            <element id="vertical" ref="line">
                <!-- element draws a vertical line, no need to rotate it -->
                <orientation rotate="0" />
                <!-- centre it in the square horizotnally, using the full height -->
                <bounds x="4.55" y="1.9" width="0.1" height="1" />
            </element>

            <!-- horizontal line for displaying Y axis input -->
            <element id="horizontal" ref="line">
                <!-- rotate the element by 90 degrees to get a horizontal line -->
                <orientation rotate="90" />
                <!-- centre it in the square vertically, using the full width -->
                <bounds x="4.1" y="2.35" width="1" height="0.1" />
            </element>

            <!-- draw a small box at the intersection of the vertical and horiztonal lines -->
            <element id="box" ref="box">
                <bounds x="4.55" y="2.35" width="0.1" height="0.1" />
            </element>

            <!-- draw the warning text over the screen near the bottom -->
            <element id="warning" ref="warning">
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
                        -- in this case, it's the root machine driver for starwars
                        -- find the analog axis inputs
                        local x_input = file.device:ioport("STICKX")
                        local y_input = file.device:ioport("STICKY")

                        -- find the outline item
                        local outline_item = file.views["Analog Control Display"].items["outline"]

                        -- variables for keeping state across callbacks
                        local outline_bounds    -- bounds of the outlined square
                        local width, height     -- width and height for animated items
                        local x_scale, y_scale  -- ratios of axis units to render coordinates
                        local x_pos, y_pos      -- display positions for the animated items

                        -- set a function to call when view dimensions have been recalculated
                        -- this can happen when when the window is resized or scaling options are changed
                        file.views["Analog Control Display"]:set_recomputed_callback(
                                function ()
                                    -- get the bounds of the outlined square
                                    outline_bounds = outline_item.bounds
                                    -- animated items use 10% of the width/height of the square
                                    width = outline_bounds.width * 0.1
                                    height = outline_bounds.height * 0.1
                                    -- calculate ratios of axis units to render coordinates
                                    -- animated items leave 90% of the width/height for the movement range
                                    -- the end of the range of each axis is at 0xff
                                    x_scale = outline_bounds.width * 0.9 / 0xff
                                    y_scale = outline_bounds.height * 0.9 / 0xff
                                end)

                        -- set a function to call before adding the view items to the render target
                        file.views["Analog Control Display"]:set_prepare_items_callback(
                                function ()
                                    -- read analog axes, reverse Y axis as zero is at the bottom
                                    local x = x_input:read() & 0xff
                                    local y = 0xff - (y_input:read() & 0xff)
                                    -- convert the input values to layout coordinates
                                    -- use the top left corner of the outlined square as the origin
                                    x_pos = outline_bounds.x0 + (x * x_scale)
                                    y_pos = outline_bounds.y0 + (y * y_scale)
                                end)

                        -- set a function to supply the bounds for the vertical line
                        file.views["Analog Control Display"].items["vertical"]:set_bounds_callback(
                                function ()
                                    -- create a new render bounds object (starts as a unit square)
                                    local result = emu.render_bounds()
                                    -- set left, top, width and height
                                    result:set_wh(
                                            x_pos,                  -- calculated X position for animated items
                                            outline_bounds.y0,      -- top of outlined square
                                            width,                  -- 10% of width of outlined square
                                            outline_bounds.height)  -- full height of outlined square
                                    return result
                                end)

                        -- set a function to supply the bounds for the horizontal line
                        file.views["Analog Control Display"].items["horizontal"]:set_bounds_callback(
                                function ()
                                    -- create a new render bounds object (starts as a unit square)
                                    local result = emu.render_bounds()
                                    -- set left, top, width and height
                                    result:set_wh(
                                            outline_bounds.x0,      -- left of outlined square
                                            y_pos,                  -- calculated Y position for animated items
                                            outline_bounds.width,   -- full width of outlined square
                                            height)                 -- 10% of height of outlined square
                                    return result
                                end)

                        -- set a function to supply the bounds for the box at the intersection of the lines
                        file.views["Analog Control Display"].items["box"]:set_bounds_callback(
                                function ()
                                    -- create a new render bounds object (starts as a unit square)
                                    local result = emu.render_bounds()
                                    -- set left, top, width and height
                                    result:set_wh(
                                            x_pos,                  -- calculated X position for animated items
                                            y_pos,                  -- calculated Y position for animated items
                                            width,                  -- 10% of width of outlined square
                                            height)                 -- 10% of height of outlined square
                                    return result
                                end)

                        -- hide the warning, since if we got here the script is running
                        file.views["Analog Control Display"].items["warning"]:set_state(0)
                    end)
        ]]></script>

    </mamelayout>

The layout has a ``script`` element containing the Lua script, to be called as a
function by the layout plugin when the layout file is loaded.  This happens
after the layout views have been build, but before the emulated system has
finished starting.  The layout file object is supplied to the script in the
``file`` variable.

We supply a function to be called after tags in the layout file have been
resolved.  This function does the following:

* Looks up the analog axis inputs.
* Looks up the view item that draws the outline of area where the yoke position
  is displayed.
* Declares some variables to hold calculated values across function calls.
* Supplies a function to be called when the view’s dimensions have been
  recomputed.
* Supplies a function to be called before adding view items to the render
  container.
* Supplies functions that will supply the bounds for the animated items.
* Hides the warning that reminds the user to enable the layout plugin by setting
  the element state for the item to 0 (the text component is only drawn when
  the element state is 1).

The view is looked up by name (value of its ``name`` attribute), and items
within the view are looked up by ID (values of their ``id`` attributes).

Layout view dimensions are recomputed in response to several events, including
the window being resized, entering/leaving full screen mode, toggling visibility
of item collections, and changing the zoom to screen area setting.  When this
happens, we need to update our size and animation scale factors.  We get the
bounds of the square where the yoke position is displayed, calculate the size
for the animated items, and calculate the ratios of axis units to render target
coordinates in each direction.  It’s more efficient to do these calculations
only when the results may change.

Before view items are added to the render target, we read the analog axis inputs
and convert the values to coordinates positions for the animated items.  The Y
axis input uses larger values to aim higher, so we need to reverse the value by
subtracting it from 0xff (255).  We add in the coordinates of the top left
corner of the square where we’re displaying the yoke position.  We do this once
each time the layout is drawn for efficiency, since we can use the values for
all three animated items.

Finally, we supply bounds for the animated items when required.  These functions
need to return ``render_bounds`` objects giving the position and size of the
items in render target coordinates.

(Since the vertical and horizontal line elements each only move on a single
axis, it would be possible to animate them using the layout file format’s item
animation features.  Only the box at the intersection of the line actually
requires scripting.  It’s done entirely using scripting here for illustrative
purposes.)


.. _layscript-environment:

The layout script environment
-----------------------------

The Lua environment is provided by the layout plugin.  It’s fairly minimal, only
providing what’s needed:

* ``file`` giving the script’s layout file object.  Has a ``device`` property
  for obtaining the device that caused the layout file to be loaded, and a
  ``views`` property for obtaining the layout’s views (indexed by name).
* ``machine`` giving MAME’s current running machine.
* ``emu.render_bounds`` and ``emu.render_color`` functions for creating bounds
  and colour objects.
* ``emu.print_error``, ``emu.print_info`` and ``emu.print_debug`` functions for
  diagnostic output.
* Standard Lua ``pairs``, ``ipairs``, ``table.insert`` and ``table.remove``
  functions for manipulating tables and other containers.
* Standard Lua ``print`` function for text output to the console.
* Standard Lua ``string.format`` function for string formatting.


.. _layscript-events:

Layout events
-------------

MAME layout scripting uses an event-based model.  Scripts can supply functions
to be called after events occur, or when data is needed.  There are three levels
of events: layout file events, layout view events, and layout view item events.

.. _layscript-events-file:

Layout file events
~~~~~~~~~~~~~~~~~~

Layout file events apply to the file as a whole, and not to an individual view.

Resolve tags
    ``file:set_resolve_tags_callback(cb)``

    Called after the emulated system has finished starting, input and output
    tags in the layout have been resolved, and default item callbacks have been
    set up.  This is a good time to look up inputs and set up view item event
    handlers.

    The callback function has no return value and takes no parameters.  Call
    with ``nil`` as the argument to remove the event handler.

.. _layscript-events-view:

Layout view events
~~~~~~~~~~~~~~~~~~

Layout view events apply to an individual view.

Prepare items
    ``view:set_prepare_items_callback(cb)``

    Called before the view’s items are added to the render target in preparation
    for drawing a video frame.

    The callback function has no return value and takes no parameters.  Call
    with ``nil`` as the argument to remove the event handler.
Preload
    ``view:set_preload_callback(cb)``

    Called after pre-loading visible view elements.  This can happen when the
    view is selected for the first time in a session, or when the user toggles
    visibility of an element collection on.  Be aware that this can be called
    multiple times in a session and avoid repeating expensive tasks.

    The callback function has no return value and takes no parameters.  Call
    with ``nil`` as the argument to remove the event handler.
Dimensions recomputed
    ``view:set_recomputed_callback(cb)``

    Called after view dimensions are recomputed.  This happens in several
    situations, including the window being resized, entering or leaving full
    screen mode, toggling visibility of item collections, and changes to the
    rotation and zoom to screen area settings.  If you’re animating the position
    of view items, this is a good time to calculate positions and scale factors.

    The callback function has no return value and takes no parameters.  Call
    with ``nil`` as the argument to remove the event handler.

.. _layscript-events-item:

Layout view item events
~~~~~~~~~~~~~~~~~~~~~~~

Layout view item callbacks apply to individual items within a view.  They are
used to override items’ default element state, animation state, bounds and
colour behaviour.

Get element state
    ``item:set_element_state_callback(cb)``

    Set callback for getting the item’s element state.  This controls how the
    item’s element is drawn, for components that change appearance depending on
    state, conditionally-drawn components, and component bounds/colour
    animation.  Do not attempt to access the item’s ``element_state`` property
    from the callback, as it will result in infinite recursion.

    The callback function must return an integer, and takes no parameters.  Call
    with ``nil`` as the argument to restore the default element state
    handler (based on the item’s XML attributes).
Get animation state
    ``item:set_animation_state_callback(cb)``

    Set callback for getting the item’s animation state.  This is used for item
    bounds/colour animation.  Do not attempt to access the item’s
    ``animation_state`` property from the callback, as it will result in
    infinite recursion.

    The callback function must return an integer, and takes no parameters.  Call
    with ``nil`` as the argument to restore the default animation state handler
    (based on the item’s XML attributes and ``animate`` child element).
Get item bounds
    ``item:set_bounds_callback(cb)``

    Set callback for getting the item’s bounds (position and size).  Do not
    attempt to access the item’s ``bounds`` property from the callback, as it
    will result in infinite recursion.

    The callback function must return a render bounds object representing the
    item’s bounds in render target coordinates (usually created by calling
    ``emu.render_bounds``), and takes no parameters.  Call with ``nil`` as the
    argument to restore the default bounds handler (based on the item’s
    animation state and ``bounds`` child elements).
Get item colour
    ``item::set_color_callback(cb)``

    Set callback for getting the item’s colour (the element texture’s colours
    multiplied by this colour).  Do not attempt to access the item’s ``color``
    property from the callback, as it will result in infinite recursion.

    The callback function must return a render colour object representing the
    ARGB colour (usually created by calling ``emu.render_color``), and takes no
    parameters.  Call with ``nil`` as the argument to restore the default colour
    handler (based on the item’s animation state and ``color`` child elements).
