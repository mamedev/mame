.. _luareference:

MAME Lua Class Reference
========================

.. contents::
    :local:
    :depth: 2


.. _luareference-intro:

Introduction
------------

Various aspects of MAME can be controlled using Lua scripting.  Many key classes
are exposed as Lua objects.

.. _luareference-intro-containers:

Containers
~~~~~~~~~~

Many properties yield container wrappers.  Container wrappers are cheap to
create, and provide an interface that is similar to a read-only table.  The
complexity of operations may vary.  Container wrappers usually provide most of
these operations:

#c
    Get the number of items in the container.
c[k]
    Returns the item corresponding to the key ``k``, or ``nil`` if the key is
    not present.
pairs(c)
    Iterate container by key and value.  The key is what you would pass to the
    index operator or the ``get`` method to get the value.
ipairs(c)
    Iterate container by index and value.  The index is what you would pass to
    the ``at`` method to get the value (this may be the same as the key for some
    containers).
c:empty()
    Returns a Boolean indicating whether there are no items in the container.
c:get(k)
    Returns the item corresponding to the key ``k``, or ``nil`` if the key is
    not present.  Usually equivalent to the index operator.
c:at(i)
    Returns the value at the 1-based index ``i``, or ``nil`` if it is out of
    range.
c:find(v)
    Returns the key for item ``v``, or ``nil`` if it is not in the collection.
    The key is what you would pass to the index operator to get the value.
c:find(v)
    Returns the key for item ``v``, or ``nil`` if it is not in the container.
    The key is what you would pass to the index operator to get the value.
c:index_of(v)
    Returns the 1-based index for item ``v``, or ``nil`` if it is not in the
    container.  The index is what you would pass to the ``at`` method to get the
    value.


.. _luareference-render:

Render system
-------------

The render system is responsible for drawing what you see in MAME’s windows,
including emulated screens, artwork, and UI elements.

.. _luareference-render-bounds:

Render bounds
~~~~~~~~~~~~~

Wraps MAME’s ``render_bounds`` class, which represents a rectangle using
floating-point coordinates.

Instantiation
^^^^^^^^^^^^^

emu.render_bounds()
    Creates a render bounds object representing a unit square, with top left
    corner at (0, 0) and bottom right corner at (1, 1).  Note that render
    target coordinates don’t necessarily have equal X and Y scales, so this may
    not represent a square in the final output.
emu.render_bounds(left, top, right, bottom)
    Creates a render bounds object representing a rectangle with top left
    corner at (x0, y0) and bottom right corner at (x1, y1).

    The arguments must all be floating-point numbers.

Methods
^^^^^^^

bounds:includes(x, y)
    Returns a Boolean indicating whether the specified point falls within the
    rectangle.  The rectangle must be normalised for this to work (right greater
    than left and bottom greater than top).

    The arguments must both be floating-point numbers.
bounds:set_xy(left, top, right, bottom)
    Set the rectangle’s position and size in terms of the positions of the
    edges.

    The arguments must all be floating-point numbers.
bounds:set_wh(left, top, width, height)
    Set the rectangle’s position and size in terms of the top top left corner
    position, and the width and height.

    The arguments must all be floating-point numbers.

Properties
^^^^^^^^^^

bounds.x0 (read/write)
    The leftmost coordinate in the rectangle (i.e. the X coordinate of the left
    edge or the top left corner).
bounds.x1 (read/write)
    The rightmost coordinate in the rectangle (i.e. the X coordinate of the
    right edge or the bottom right corner).
bounds.y0 (read/write)
    The topmost coordinate in the rectangle (i.e. the Y coordinate of the top
    edge or the top left corner).
bounds.y1 (read/write)
    The bottommost coordinate in the rectangle (i.e. the Y coordinate of the
    bottom edge or the bottom right corner).
bounds.width (read/write)
    The width of the rectangle.  Setting this property changes the position of
    the rightmost edge.
bounds.height (read/write)
    The height of the rectangle.  Setting this property changes the position of
    the bottommost edge.
bounds.aspect (read-only)
    The width-to-height aspect ratio of the rectangle.  Note that this is often
    in render target coordinates which don’t necessarily have equal X and Y
    scales.  A rectangle representing a square in the final output doesn’t
    necessarily have an aspect ratio of 1.

.. _luareference-render-color:

Render colour
~~~~~~~~~~~~~

Wraps MAME’s ``render_color`` class, which represents an ARGB (alpha, red,
green, blue) format colour.  Channels are floating-point values ranging from
zero (0, transparent alpha or colour off) to one (1, opaque or full colour
intensity).  Colour channel values are not pre-multiplied by the alpha channel
value.

Instantiation
^^^^^^^^^^^^^

emu.render_color()
    Creates a render colour object representing opaque white (all channels set
    to 1).  This is the identity value – ARGB multiplication by this value will
    not change a colour.
emu.render_color(a, r, g, b)
    Creates a render colour object with the specified alpha, red, green and
    blue channel values.

    The arguments must all be floating-point numbers in the range from zero (0)
    to one (1), inclusive.

Methods
^^^^^^^

color:set(a, r, g, b)
    Sets the colour object’s alpha, red, green and blue channel values.

    The arguments must all be floating-point numbers in the range from zero (0)
    to one (1), inclusive.

Properties
^^^^^^^^^^

color.a (read/write)
    Alpha value, in the range of zero (0, transparent) to one (1, opaque).
color.r (read/write)
    Red channel value, in the range of zero (0, off) to one (1, full intensity).
color.g (read/write)
    Green channel value, in the range of zero (0, off) to one (1, full
    intensity).
color.b (read/write)
    Blue channel value, in the range of zero (0, off) to one (1, full
    intensity).

.. _luareference-render-manager:

Render manager
~~~~~~~~~~~~~~

Wraps MAME’s ``render_manager`` class, responsible for managing render targets
and textures.

Instantiation
^^^^^^^^^^^^^

manager:machine():render()
    Gets the global render manager instance for the emulation session.

Properties
^^^^^^^^^^

render.max_update_rate (read-only)
    The maximum update rate in Hertz.  This is a floating-point number.
render.ui_target (read-only)
    The :ref:`render target <luareference-render-target>` used to draw the user
    interface (including menus, sliders and pop-up messages).  This is usually
    the first host window or screen.
render.ui_container (read-only)
    The :ref:`render container <luareference-render-container>` used for drawing
    the user interface.
render.targets[] (read-only)
    The list of render targets, including output windows and screens, as well as
    hidden render targets used for things like rendering screenshots.  Uses
    1-based integer indices.  The index operator and the ``at`` method have O(n)
    complexity.

.. _luareference-render-target:

Render target
~~~~~~~~~~~~~

Wrap’s MAME’s ``render_target`` class, which represents a video output channel.
This could be a host window or screen, or a hidden target used for rendering
screenshots.

Instantiation
^^^^^^^^^^^^^

manager:machine():render().targets[index]
    Get a render target by index.
manager:machine():render():ui_target()
    Get the render target used to display the user interface (including menus,
    sliders and pop-up messages).  This is usually the first host window or
    screen.

Properties
^^^^^^^^^^

target.index (read-only)
    The 1-based index of the render target.  This has O(n) complexity.
target.width (read-only)
    The width of the render target in output pixels.  This is an integer.
target.height (read-only)
    The height of the render target in output pixels.  This is an integer.
target.pixel_aspect (read-only)
    The width-to-height aspect ratio of the render target’s pixels.  This is a
    floating-point number.
target.hidden (read-only)
    A Boolean indicating whether this is an internal render target that is not
    displayed to the user directly (e.g. the render target used to draw
    screenshots).
target.is_ui_target (read-only)
    A Boolean indicating whether this is the render target used to display the
    user interface.
target.max_update_rate (read/write)
    The maximum update rate for the render target in Hertz.
target.orientation (read/write)
    The target orientation flags.  This is an integer bit mask, where bit 0
    (0x01) is set to mirror horizontally, bit 1 (0x02) is set to mirror
    vertically, and bit 2 (0x04) is set to mirror along the top left-bottom
    right diagonal.
target.view_names[]
    The names of the available views for this render target.  Uses 1-based
    integer indices.  The ``find`` and ``index_of`` methods have O(n)
    complexity; all other supported operations have O(1) complexity.
target.current_view (read-only)
    The currently selected view for the render target.  This is a
    :ref:`layout view <luareference-render-layview>` object.
target.view_index (read/write)
    The 1-based index of the selected view for this render target.
target.visibility_mask (read-only)
    An integer bit mask indicating which item collections are currently visible
    for the current view.
target.screen_overlay (read/write)
    A Boolean indicating whether screen overlays are enabled.
target.zoom_to_screen (read/write)
    A Boolean indicating whether the render target is configured to scale so
    that the emulated screen(s) fill as much of the output window/screen as
    possible.

.. _luareference-render-container:

Render container
~~~~~~~~~~~~~~~~

Wraps MAME’s ``render_container`` class.

Instantiation
^^^^^^^^^^^^^

manager:machine():render().ui_container
    Gets the render container used to draw the user interface, including menus,
    sliders and pop-up messages.
manager:machine().screens[tag].container
    Gets the render container used to draw a given screen.

Properties
^^^^^^^^^^

container.user_settings (read/write)
    The container’s :ref:`user settings <luareference-render-contsettings>`.
    This can be used to control a number of image adjustments.
container.orientation (read/write)
    The container orientation flags.  This is an integer bit mask, where bit 0
    (0x01) is set to mirror horizontally, bit 1 (0x02) is set to mirror
    vertically, and bit 2 (0x04) is set to mirror along the top left-bottom
    right diagonal.
container.xscale (read/write)
    The container’s X scale factor.  This is a floating-point number.
container.yscale (read/write)
    The container’s Y scale factor.  This is a floating-point number.
container.xoffset (read/write)
    The container’s X offset.  This is a floating-point number where one (1)
    represents the X size of the container.
container.yoffset (read/write)
    The container’s Y offset.  This is a floating-point number where one (1)
    represents the Y size of the container.
container.is_empty (read-only)
    A Boolean indicating whether the container has no items.

.. _luareference-render-contsettings:

Container user settings
~~~~~~~~~~~~~~~~~~~~~~~

Wraps MAME’s ``render_container::user_settings`` class, representing image
adjustments applied to a
:ref:`render container <luareference-render-container>`.

Instantiation
^^^^^^^^^^^^^

manager:machine().screens[tag].container
    Gets the current container user settings for a given screen.

Properties
^^^^^^^^^^

settings.orientation (read/write)
    The container orientation flags.  This is an integer bit mask, where bit 0
    (0x01) is set to mirror horizontally, bit 1 (0x02) is set to mirror
    vertically, and bit 2 (0x04) is set to mirror along the top left-bottom
    right diagonal.
settings.brightness (read/write)
    The brightness adjustment applied to the container.  This is a
    floating-point number.
settings.contrast (read/write)
    The contrast adjustment applied to the container.  This is a floating-point
    number.
settings.gamma (read/write)
    The gamma adjustment applied to the container.  This is a floating-point
    number.
settings.xscale (read/write)
    The container’s X scale factor.  This is a floating-point number.
settings.yscale (read/write)
    The container’s Y scale factor.  This is a floating-point number.
settings.xoffset (read/write)
    The container’s X offset.  This is a floating-point number where one (1)
    represents the X size of the container.
settings.yoffset (read/write)
    The container’s Y offset.  This is a floating-point number where one (1)
    represents the Y size of the container.

.. _luareference-render-layfile:

Layout file
~~~~~~~~~~~

Wraps MAME’s ``layout_file`` class, representing the views loaded from a layout
file for use by a render target.

Instantiation
^^^^^^^^^^^^^

A layout file object is supplied to its layout script in the ``file`` variable.
Layout file objects are not instantiated directly from Lua scripts.

Methods
^^^^^^^

layout:set_resolve_tags_callback(cb)
    Set a function to perform additional tasks after the emulated machine has
    finished starting, tags in the layout views have been resolved, and the
    default view item handlers have been set up.  The function must accept no
    arguments.

    Call with ``nil`` to remove the callback.

Properties
^^^^^^^^^^

layout.device (read-only)
    The device that caused the layout file to be loaded.  Usually the root
    machine device for external layouts.
layout.views[] (read-only)
    The :ref:`views <luareference-render-layview>` created from the layout file.
    Views are indexed by unqualified name (i.e. the value of the ``name``
    attribute).  Views are ordered how they appear in the layout file when
    iterating or using the ``at`` method.  The index get, ``at`` and
    ``index_of`` methods have O(n) complexity.

    Note that not all views in the XML file may be created.  For example views
    that reference screens provided by slot card devices will not be created if
    said slot card devices are not present in the system.

.. _luareference-render-layview:

Layout view
~~~~~~~~~~~

Wraps MAME’s ``layout_view`` class, representing a view that can be displayed in
a render target.  Views are created from XML layout files, which may be loaded
from external artwork, internal to MAME, or automatically generated based on the
screens in the emulated system.

Instantiation
^^^^^^^^^^^^^

Layout scripts generally

manager:machine():render().targets[index].current_view
    Gets the currently selected view for a given render target.

Methods
^^^^^^^

view:has_screen(screen)
    Returns a Boolean indicating whether the screen is present in the view.
    This is true for screens that are present but not visible because the user
    has hidden the item collection they belong to.
view:set_prepare_items_callback(cb)
    Set a function to perform additional tasks before the view items are added
    to the render target in preparation for drawing a video frame.  The function
    must accept no arguments.  Call with ``nil`` to remove the callback.
view:set_preload_callback(cb)
    Set a function to perform additional tasks after preloading visible view
    items.  The function must accept no arguments.  Call with ``nil`` to remove
    the callback.

    This function may be called when the user selects a view or makes an item
    collection visible.  It may be called multiple times for a view, so avoid
    repeating expensive tasks.
view:set_recomputed_callback(cb)
    Set a function to perform additional tasks after the view’s dimensions are
    recomputed.  The function must accept no arguments.  Call with ``nil`` to
    remove the callback.

    View coordinates are recomputed in various events, including the window
    being resized, entering or leaving full-screen mode, and changing the zoom
    to screen area setting.

Properties
^^^^^^^^^^

view.items[] (read-only)
    The screen and layout element :ref:`items <luareference-render-layitem>` in
    the view.  This container does not support iteration by key using ``pairs``;
    only iteration by index using ``ipairs`` is supported.  The key is the value
    of the ``id`` attribute if present.  Only items with ``id`` attributes can
    be looked up by key.  The index get method has O(1) complexity, and the
    ``at`` and ``index_of`` methods have O(n) complexity.
view.name (read-only)
    The display name for the view.  This may be qualified to indicate the device
    that caused the layout file to be loaded when it isn’t the root machine
    device.
view.unqualified_name (read-only)
    The unqualified name of the view, exactly as it appears in the ``name``
    attribute in the XML layout file.
view.visible_screen_count (read-only)
    The number of screens items currently enabled in the view.
view.effective_aspect (read-only)
    The effective width-to-height aspect ratio of the view in its current
    configuration.
view.bounds (read-only)
    A :ref:`render bounds <luareference-render-bounds>` object representing the
    effective bounds of the view in its current configuration.  The coordinates
    are in view units, which are arbitrary but assumed to have square aspect
    ratio.
view.has_art
    A Boolean indicating whether the view has any non-screen items, including
    items that are not visible because the user has hidden the item collection
    that they belong to.

.. _luareference-render-layitem:

Layout view item
~~~~~~~~~~~~~~~~

Wraps MAME’s ``layout_view::item`` class, representing an item in a view.  An
item is drawn as a rectangular textured surface.  The texture is supplied by an
emulated screen or a layout element.

Instantiation
^^^^^^^^^^^^^

layout.views[name].items[id]
    Get a view item by ID.  The item must have an ``id`` attribute in the XML
    layout file to be looked up by ID.

Methods
^^^^^^^

item:set_state(state)
    Set the value used as the element state and animation state in the absence
    of bindings.  The argument must be an integer.
item.set_element_state_callback(cb)
    Set a function to call to obtain the element state for the item.  The
    function must accept no arguments and return an integer.  Call with ``nil``
    to restore the default element state callback (based on bindings in the XML
    layout file).

    Note that the function must not access the item’s ``element_state``
    property, as this will result in infinite recursion.

    This callback will not be used to obtain the animation state for the item,
    even if the item lacks explicit animation state bindings in the XML layout
    file.
item.set_animation_state_callback(cb)
    Set a function to call to obtain the animation state for the item.  The
    function must accept no arguments and return an integer.  Call with ``nil``
    to restore the default animation state callback (based on bindings in the
    XML layout file).

    Note that the function must not access the item’s ``animation_state``
    property, as this will result in infinite recursion.
item.set_bounds_callback(cb)
    Set a function to call to obtain the bounds for the item.  The function must
    accept no arguments and return a
    :ref:`render bounds <luareference-render-bounds>` object in render target
    coordinates.  Call with ``nil`` to restore the default bounds callback
    (based on the item’s animation state and ``bounds`` child elements in the
    XML layout file).

    Note that the function must not access the item’s ``bounds`` property, as
    this will result in infinite recursion.
item.set_color_callback(cb)
    Set a function to call to obtain the multiplier colour for the item.  The
    function must accept no arguments and return a
    :ref:`render colour <luareference-render-color>` object.  Call with ``nil``
    to restore the default colour callback (based on the item’s animation state
    and ``color`` child elements in the XML layout file).

    Note that the function must not access the item’s ``color`` property, as
    this will result in infinite recursion.

Properties
^^^^^^^^^^

item.id (read-only)
    Get the optional item identifier.  This is the value of the ``id`` attribute
    in the XML layout file if present, or ``nil``.
item.bounds_animated (read-only)
    A Boolean indicating whether the item’s bounds depend on its animation
    state.
item.color_animated (read-only)
    A Boolean indicating whether the item’s colour depends on its animation
    state.
item.bounds (read-only)
    The item’s bounds for the current state.  This is a
    :ref:`render bounds <luareference-render-bounds>` object in render target
    coordinates.
item.color (read-only)
    The item’s colour for the current state.  The colour of the screen or
    element texture is multiplied by this colour.  This is a 
    :ref:`render colour <luareference-render-color>` object.
item.blend_mode (read-only)
    Get the item’s blend mode.  This is an integer value, where 0 means no
    blending, 1 means alpha blending, 2 means RGB multiplication, 3 means
    additive blending, and -1 allows the items within a container to specify
    their own blending modes.
item.orientation (read-only)
    Get the item orientation flags.  This is an integer bit mask, where bit 0
    (0x01) is set to mirror horizontally, bit 1 (0x02) is set to mirror
    vertically, and bit 2 (0x04) is set to mirror along the top left-bottom
    right diagonal.
item.element_state (read-only)
    Get the current element state.  This will call the element state callback
    function to handle bindings.
item.animation_state (read-only)
    Get the current animation state.  This will call the animation state
    callback function to handle bindings.
