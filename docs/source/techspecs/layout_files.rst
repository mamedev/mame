MAME Layout Files
=================

.. contents:: :local:


.. _layout-intro:

Introduction
------------

Layout files are used to tell MAME what to display when running an emulated
system, and how to arrange it.  MAME can render emulated screens, images, text,
shapes, and specialised objects for common output devices.  Elements can be
static, or dynamically update to reflect the state of inputs and outputs.
Layouts may be automatically generated based on the number/type of emulated
screens, built and linked into the MAME binary, or provided externally.  MAME
layout files are an XML application, using the ``.lay`` filename extension.


.. _layout-concepts:

Core concepts
-------------

.. _layout-concepts-numbers:

Numbers
~~~~~~~

There are two kinds of numbers in MAME layouts: integers and floating-point
numbers.

Integers may be supplied in decimal or hexadecimal notation.  A decimal integer
consists of and optional # (hash) prefix, an optional +/- (plus or minus) sign
character, and a sequence of digits 0-9.  A hexadecimal number consists of one
of the prefixes $ (dollar sign) or 0x (zero ex) followed by a sequence of
hexadecimal digits 0-9 and A-F.  Hexadecimal numbers are case-insensitive for
both the prefix and digits.

Floating-point numbers may be supplied in decimal fixed-point or scientific
notation.  Note that integer prefixes and hexadecimal values are *not*
accepted where a floating-point number is expected.

For a few attributes, both integers and floating-point numbers are allowed.  In
these cases, the presence of a # (hash), $ (dollar sign) or 0x (zero ex) prefix
causes the value to be interpreted as an integer.  If no recognised integer
prefix is found and the value contains a decimal point or the letter E
(uppercase or lowercase) introducing an exponent, it is interpreted as a
floating-point number.  If no integer prefix, decimal point or letter E is
found, the number will be interpreted as an integer.

Numbers are parsed using the "C" locale for portability.


.. _layout-concepts-coordinates:

Coordinates
~~~~~~~~~~~

Layout coordinates are internally represented as IEEE754 32-bit binary
floating-point numbers (also known as "single precision").  Coordinates increase
in the rightward and downward directions.  The origin (0,0) has no particular
significance, and you may freely use negative coordinates in layouts.
Coordinates are supplied as floating-point numbers.

MAME assumes that view coordinates have the same aspect ratio as pixel on the
output device (host screen or window).  Assuming square pixels and no rotation,
this means equal distances in X and Y axes correspond to equal horizontal and
vertical distances in the rendered output.

Views, groups and elements all have their own internal coordinate systems.  When
an element or group is referenced from a view or another group, its coordinates
are scaled as necessary to fit the specified bounds.

Objects are positioned and sized using ``bounds`` elements.  A bounds element
may specify the position of the top left corner and the size using ``x``, ``y``,
``width`` and ``height`` attributes, or it may specify the coordinates of the
edges with the ``left``, ``top``, ``right`` and ``bottom`` attributes.  These
two ``bounds`` elements are equivalent::

    <bounds x="455" y="120" width="11" height="7" />
    <bounds left="455" top="120" right="466" bottom="127" />

Either the ``x`` or ``left`` attribute must be present to distinguish between
the two schemes.  The ``width`` and ``height`` or ``right`` and ``bottom``
default to 1.0 if not supplied.  It is an error if ``width`` or ``height`` are
negative, if ``right`` is less than ``left``, or if ``bottom`` is less than
``top``.


.. _layout-concepts-colours:

Colours
~~~~~~~

Colours are specified in RGBA space.  MAME is not aware of colour profiles and
gamuts, so colours will typically be interpreted as sRGB with your system's
target gamma (usually 2.2).  Channel values are specified as floating-point
numbers.  Red, green and blue channel values range from 0.0 (off) to 1.0 (full
intensity).  Alpha ranges from 0.0 (fully transparent) to 1.0 (opaque).  Colour
channels values are not pre-multiplied by the alpha value.

Component and view item colour is specified using ``color`` elements.
Meaningful attributes are ``red``, ``green``, ``blue`` and ``alpha``.  This
example ``color`` element specifies all channel values::

    <color red="0.85" green="0.4" blue="0.3" alpha="1.0" />

Any omitted channel attributes default to 1.0 (full intensity or opaque).  It
is an error if any channel value falls outside the range of 0.0 to 1.0
(inclusive).


.. _layout-concepts-params:

Parameters
~~~~~~~~~~

Parameters are named variables that can be used in most attributes.  To use
a parameter in an attribute, surround its name with tilde (~) characters.  If a
parameter is not defined, no substitution occurs.  Here is an examples showing
two instances of parameter use -- the values of the ``digitno`` and ``x``
parameters will be substituted for ``~digitno~`` and ``~x~``::

    <bezel name="digit~digitno~" element="digit">
        <bounds x="~x~" y="80" width="25" height="40" />
    </bezel>

A parameter name is a sequence of uppercase English letters A-Z, lowercase
English letters a-z, decimal digits 0-9, and/or underscore (_) characters.
Parameter names are case-sensitive.  When looking for a parameter, the layout
engine starts at the current, innermost scope and works outwards.  The outermost
scope level corresponds to the top-level ``mamelayout`` element.  Each
``repeat``, ``group`` or ``view`` element creates a new, nested scope level.

Internally a parameter can hold a string, integer, or floating-point number, but
this is mostly transparent.  Integers are stored as 64-bit signed
twos-complement values, and floating-point numbers are stored as IEEE754 64-bit
binary floating-point numbers (also known as "double precision").  Integers are
substituted in decimal notation, and floating point numbers are substituted in
default format, which may be decimal fixed-point or scientific notation
depending on the value).  There is no way to override the default formatting of
integer and floating-point number parameters.

There are two kinds of parameters: *value parameters* and *generator
parameters*.  Value parameters keep their assigned value until reassigned.
Generator parameters have a starting value and an increment and/or shift to be
applied for each iteration.

Value parameters are assigned using a ``param`` element with ``name`` and
``value`` attributes.  Value parameters may appear inside the top-level
``mamelayout`` element, inside ``repeat``, and ``view`` elements, and inside
``group`` definition elements (that is, ``group`` elements in the top-level
``mamelayout`` element, as opposed to ``group`` reference elements inside
``view`` elements other ``group`` definition elements).  A value parameter may
be reassigned at any point.

Here's an example assigning the value "4" to the value parameter "firstdigit"::

    <param name="firstdigit" value="4" />

Generator parameters are assigned using a ``param`` element with ``name`` and
``start`` attributes, and ``increment``, ``lshift`` and/or ``rshift``
attributes.  Generator parameters may only appear inside ``repeat`` elements
(see :ref:`layout-parts-repeats` for details).  A generator parameter must not
be reassigned in the same scope (an identically named parameter may be defined
in a child scope).  Here are some example generator parameters::

    <param name="nybble" start="3" increment="-1" />
    <param name="switchpos" start="74" increment="156" />
    <param name="mask" start="0x0800" rshift="4" />

* The ``nybble`` parameter generates values 3, 2, 1...
* The ``switchpos`` parameter generates values 74, 230, 386...
* The ``mask`` parameter generates values 2048, 128, 8...

The ``increment`` attribute must be an integer or floating-point number to be
added to the parameter's value.  The ``lshift`` and ``rshift`` attributes must
be non-negative integers specifying numbers of bits to shift the parameter's
value to the left or right.  The increment and shift are applied at the end of
the repeating block before the next iteration starts.  If both an increment and
shift are supplied, the increment is applied before the shift.

If the ``increment`` attribute is present and is a floating-point number, the
parameter's value will be interpreted as an integer or floating-point number and
converted to a floating-point number before the increment is added.  If the
``increment`` attribute is present and is an integer, the parameter's value will
be interpreted as an integer or floating number before the increment is added.
The increment will be converted to a floating-point number before the addition
if the parameter's value is a floating-point number.

If the ``lshift`` and/or ``rshift`` attributes are present and not equal, the
parameter's value will be interpreted as an integer or floating-point number,
converted to an integer as necessary, and shifted accordingly.  Shifting to the
left is defined as shifting towards the most significant bit.  If both
``lshift`` and ``rshift`` are supplied, they are netted off before being
applied.  This means you cannot, for example, use equal ``lshift`` and
``rshift`` attributes to clear bits at one end of a parameter's value after the
first iteration.

It is an error if a ``param`` element has neither ``value`` nor ``start``
attributes, and it is an error if a ``param`` element has both a ``value``
attribute and any of the ``start``, ``increment``, ``lshift``, or ``rshift``
attributes.

A ``param`` element defines a parameter or reassigns its value in the current,
innermost scope.  It is not possible to define or reassign parameters in a
containing scope.


.. _layout-concepts-predef-params:

Pre-defined parameters
~~~~~~~~~~~~~~~~~~~~~~

A number of pre-defined value parameters are available providing information
about the running machine:

devicetag
    The full tag path of the device that caused the layout to be loaded, for
    example ``:`` for the root driver device, or ``:tty:ie15`` for a terminal
    connected to a port.  This parameter is a string defined at layout (global)
    scope.
devicebasetag
    The base tag of the device that caused the layout to be loaded, for example
    ``root`` for the root driver device, or ``ie15`` for a terminal connected to
    a port.  This parameter is a string defined at layout (global) scope.
devicename
    The full name (description) of the device that caused the layout to be
    loaded, for example ``AIM-65/40`` or ``IE15 Terminal``.  This parameter is a
    string defined at layout (global) scope.
deviceshortname
    The short name of the device that caused the layout to be loaded, for
    example ``aim65_40`` or ``ie15_terminal``.  This parameter is a string
    defined at layout (global) scope.
scr0physicalxaspect
    The horizontal part of the physical aspect ratio of the first screen (if
    present).  The physical aspect ratio is provided as a reduced improper
    fraction.  Note that this is the horizontal component *before* rotation is
    applied.  This parameter is an integer defined at layout (global) scope.
scr0physicalyaspect
    The vertical part of the physical aspect ratio of the first screen (if
    present).  The physical aspect ratio is provided as a reduced improper
    fraction.  Note that this is the vertical component *before* rotation is
    applied.  This parameter is an integer defined at layout (global) scope.
scr0nativexaspect
    The horizontal part of the pixel aspect ratio of the first screen's visible
    area (if present).  The pixel aspect ratio is provided as a reduced improper
    fraction.  Note that this is the horizontal component *before* rotation is
    applied.  This parameter is an integer defined at layout (global) scope.
scr0nativeyaspect
    The vertical part of the pixel aspect ratio of the first screen's visible
    area (if present).  The pixel aspect ratio is provided as a reduced improper
    fraction.  Note that this is the vertical component *before* rotation is
    applied.  This parameter is an integer defined at layout (global) scope.
scr0width
    The width of the first screen's visible area (if present) in emulated
    pixels.  Note that this is the width *before* rotation is applied.  This
    parameter is an integer defined at layout (global) scope.
scr0height
    The height of the first screen's visible area (if present) in emulated
    pixels.  Note that this is the height *before* rotation is applied.  This
    parameter is an integer defined at layout (global) scope.
scr1physicalxaspect
    The horizontal part of the physical aspect ratio of the second screen (if
    present).  This parameter is an integer defined at layout (global) scope.
scr1physicalyaspect
    The vertical part of the physical aspect ratio of the second screen (if
    present).  This parameter is an integer defined at layout (global) scope.
scr1nativexaspect
    The horizontal part of the pixel aspect ratio of the second screen's visible
    area (if present).  This parameter is an integer defined at layout (global)
    scope.
scr1nativeyaspect
    The vertical part of the pixel aspect ratio of the second screen's visible
    area (if present).  This parameter is an integer defined at layout (global)
    scope.
scr1width
    The width of the second screen's visible area (if present) in emulated
    pixels.  This parameter is an integer defined at layout (global) scope.
scr1height
    The height of the second screen's visible area (if present) in emulated
    pixels.  This parameter is an integer defined at layout (global) scope.
scr\ *N*\ physicalxaspect
    The horizontal part of the physical aspect ratio of the (zero-based) *N*\ th
    screen (if present).  This parameter is an integer defined at layout
    (global) scope.
scr\ *N*\ physicalyaspect
    The vertical part of the physical aspect ratio of the (zero-based) *N*\ th
    screen (if present).  This parameter is an integer defined at layout
    (global) scope.
scr\ *N*\ nativexaspect
    The horizontal part of the pixel aspect ratio of the (zero-based) *N*\ th
    screen's visible area (if present).  This parameter is an integer defined at
    layout (global) scope.
scr\ *N*\ nativeyaspect
    The vertical part of the pixel aspect ratio of the (zero-based) *N*\ th
    screen's visible area (if present).  This parameter is an integer defined at
    layout (global) scope.
scr\ *N*\ width
    The width of the (zero-based) *N*\ th screen's visible area (if present) in
    emulated pixels.  This parameter is an integer defined at layout (global)
    scope.
scr\ *N*\ height
    The height of the (zero-based) *N*\ th screen's visible area (if present) in
    emulated pixels.  This parameter is an integer defined at layout (global)
    scope.
viewname
    The name of the current view.  This parameter is a string defined at view
    scope.  It is not defined outside a view.

For screen-related parameters, screens are numbered from zero in the order they
appear in machine configuration, and all screens are included (not just
subdevices of the device that caused the layout to be loaded).  X/width and
Y/height refer to the horizontal and vertical dimensions of the screen *before*
rotation is applied.  Values based on the visible area are calculated at the
end of configuration.  Values are not updated and layouts are not recomputed if
the system reconfigures the screen while running.


.. _layout-concepts-layers:

Layers
~~~~~~

Views are rendered as a stack of layers, named after parts of an arcade cabinet.
The layout supplies elements to be drawn in all layers besides the screen layer,
which is reserved for emulated screens.  With the exception of the screen layer,
users can enable or disable layers using the in-emulation menu or command-line
options.

The following layers are available:

backdrop
    Intended for use in situations were the screen image is projected over a
    backdrop using a semi-reflective mirror (Pepper's ghost).  This arrangement
    is famously used in the Space Invaders deluxe cabinet.
screen
    This layer is reserved for emulated screen images, and cannot be disabled by
    the user.  It is drawn using additive blending.
overlay
    This layer is intended for use translucent overlays used to add colour to
    games using monochrome monitors like Circus, Gee Bee, and of course Space
    Invaders.  It is drawn using RGB multiplication.
bezel
    This layer is for elements that surround and potentially obscure the screen
    image.  It is drawn with standard alpha blending.
cpanel
    This layer is intended for displaying controls/input devices (control
    panels).  It is drawn using standard alpha blending.
marquee
    This layer is intended for displaying arcade cabinet marquee images.  It is
    drawn using standard alpha blending.

By default, layers are drawn in this order (from back to front):

* screen (add)
* overlay (multiply)
* backdrop (add)
* bezel (alpha)
* cpanel (alpha)
* marquee (alpha)

If a view has multiple backdrop elements and no overlay elements, a different
order is used (from back to front):

* backdrop (alpha)
* screen (add)
* bezel (alpha)
* cpanel (alpha)
* marquee (alpha)

The alternate drawing order makes it simpler to build a backdrop from multiple
scanned/traced pieces of art, as they can have opaque parts.  It can't be used
with overlay elements because colour overlays are conventionally placed between
the screen and mirror, and as such do not affect the backdrop.


.. _layout-parts:

Parts of a layout
-----------------

A *view* specifies an arrangement graphical object to display.  A MAME layout
file can contain multiple views.  Views are built up from *elements* and
*screens*.  To simplify complex layouts, reusable groups and repeating blocks
are supported.

The top-level element of a MAME layout file must be a ``mamelayout`` element
with a ``version`` attribute.  The ``version`` attribute must be an integer.
Currently MAME only supports version 2, and will not load any other version.
This is an example opening tag for a top-level ``mamelayout`` element::

    <mamelayout version="2">

In general, children of the top-level ``mamelayout`` element are processed in
reading order from top to bottom.  The exception is that, for historical
reasons, views are processed last.  This means views see the final values of all
parameters at the end of the ``mamelayout`` element, and may refer to elements
and groups that appear after them.

The following elements are allowed inside the top-level ``mamelayout`` element:

param
    Defines or reassigns a value parameter.  See :ref:`layout-concepts-params`
    for details.
element
    Defines an element -- one of the basic objects that can be arranged in a
    view.  See :ref:`layout-parts-elements` for details.
group
    Defines a reusable group of elements/screens that may be referenced from
    views or other groups.  See :ref:`layout-parts-groups` for details.
repeat
    A repeating group of elements -- may contain ``param``, ``element``,
    ``group``, and ``repeat`` elements.  See :ref:`layout-parts-repeats` for
    details.
view
    An arrangement of elements and/or screens that can be displayed on an output
    device (a host screen/window).  See :ref:`layout-parts-views` for details.
script
    Allows lua script to be supplied for enhanced interactive layouts.


.. _layout-parts-elements:

Elements
~~~~~~~~

Elements are one of the basic visual objects that may be arranged, along with
screens, to make up a view.  Elements may be built up one or more *components*,
but an element is treated as as single surface when building the scene graph
and rendering.  An element may be used in multiple views, and may be used
multiple times within a view.

An element's appearance depends on its *state*.  The state is an integer which
usually comes from an I/O port field or an emulated output (see the discussion
of :ref:`layout-parts-views` for information on connecting an element to an I/O
port or output).  Any component of an element may be restricted to only drawing
when the element's state is a particular value.  Some components (e.g.
multi-segment displays and reels) use the state directly to determine their
appearance.

Each element has its own internal coordinate system.  The bounds of the
element's coordinate system are computed as the union of the bounds of the
individual components it's composed of.

Every element must have a ``name`` attribute specifying its name.  Elements are
referred to by name when instantiated in groups or views.  Elements may
optionally supply a default state value with a ``defstate`` attribute, to be
used if not connected to an emulated output or I/O port.  If present, the
``defstate`` attribute must be a non-negative integer.

Child elements of the ``element`` element instantiate components, which are
drawn in reading order from first to last (components draw on top of components
that come before them).  All components support a few common features:

* Each component may have a ``state`` attribute.  If present, the component will
  only be drawn when the element's state matches its value (if absent, the
  component will always be drawn).  If present, the ``state`` attribute must be
  a non-negative integer.
* Each component may have a ``bounds`` child element specifying its position and
  size (see :ref:`layout-concepts-coordinates`).  If no such element is present,
  the bounds default to a unit square (width and height of 1.0) with the top
  left corner at (0,0).
* Each component may have a ``color`` child element specifying an RGBA colour
  (see :ref:`layout-concepts-colours` for details).  This can be used to control
  the colour of geometric, algorithmically drawn, or textual components.  It is
  ignored for ``image`` components.  If no such element is present, the colour
  defaults to opaque white.

The following components are supported:

rect
    Draws a uniform colour rectangle filling its bounds.
disk
    Draws a uniform colour ellipse fitted to its bounds.
image
    Draws an image loaded from a PNG or JPEG file.  The name of the file to load
    (including the file name extension) is supplied with the required ``file``
    attribute.  Additionally, an optional ``alphafile`` attribute may be used to
    specify the name of a PNG file (including the file name extension) to load
    into the alpha channel of the image.  The image file(s) should be placed in
    the same directory/archive as the layout file.  If the ``alphafile``
    attribute refers  refers to a file, it must have the same dimensions as the
    file referred to by the ``file`` attribute, and must have a bit depth no
    greater than eight bits per channel per pixel.  The intensity from this
    image (brightness) is copied to the alpha channel, with full intensity (white
    in a greyscale image) corresponding to fully opaque, and black corresponding
    to fully transparent.
text
    Draws text in using the UI font in the specified colour.  The text to draw
    must be supplied using a ``string`` attribute.  An ``align`` attribute may
    be supplied to set text alignment.  If present, the ``align`` attribute must
    be an integer, where 0 (zero) means centred, 1 (one) means left-aligned, and
    2 (two) means right-aligned.  If the ``align`` attribute is absent, the text
    will be centred.
dotmatrix
    Draws an eight-pixel horizontal segment of a dot matrix display, using
    circular pixels in the specified colour.  The bits of the element's state
    determine which pixels are lit, with the least significant bit corresponding
    to the leftmost pixel.  Unlit pixels are drawn at low intensity (0x20/0xff).
dotmatrix5dot
    Draws a five-pixel horizontal segment of a dot matrix display, using
    circular pixels in the specified colour.  The bits of the element's state
    determine which pixels are lit, with the least significant bit corresponding
    to the leftmost pixel.  Unlit pixels are drawn at low intensity (0x20/0xff).
dotmatrixdot
    Draws a single element of a dot matrix display as a circular pixels in the
    specified colour.  The least significant bit of the element's state
    determines whether the pixel is lit.  An unlit pixel is drawn at low
    intensity (0x20/0xff).
led7seg
    Draws a standard seven-segment (plus decimal point) digital LED/fluorescent
    display in the specified colour.  The low eight bits of the element's state
    control which segments are lit.  Starting from the least significant bit,
    the bits correspond to the top segment, the upper right-hand segment,
    continuing clockwise to the upper left segment, the middle bar, and the
    decimal point.  Unlit segments are drawn at low intensity (0x20/0xff).
led8seg_gts1
    Draws an eight-segment digital fluorescent display of the type used in
    Gottlieb System 1 pinball machines (actually a Futaba part).  Compared to
    standard seven-segment displays, these displays have no decimal point, the
    horizontal middle bar is broken in the centre, and there is a broken
    vertical middle bar controlled by the bit that would control the decimal
    point in a standard seven-segment display.  Unlit segments are drawn at low
    intensity (0x20/0xff).
led14seg
    Draws a standard fourteen-segment alphanumeric LED/fluorescent display in
    the specified colour.  The low fourteen bits of the element's state control
    which segments are lit.  Starting from the least significant bit, the bits
    correspond to the top segment, the upper right-hand segment, continuing
    clockwise to the upper left segment, the left-hand and right-hand halves of
    the horizontal middle bar, the upper and lower halves of the vertical middle
    bar, and the diagonal bars clockwise from lower left to lower right.  Unlit
    segments are drawn at low intensity (0x20/0xff).
led14segsc
    Draws a standard fourteen-segment alphanumeric LED/fluorescent display with
    decimal point/comma in the specified colour.  The low sixteen bits of the
    element's state control which segments are lit.  The low fourteen bits
    correspond to the same segments as in the ``led14seg`` component.  Two
    additional bits correspond to the decimal point and comma tail.  Unlit
    segments are drawn at low intensity (0x20/0xff).
led16seg
    Draws a standard sixteen-segment alphanumeric LED/fluorescent display in the
    specified colour.  The low sixteen bits of the element's state control which
    segments are lit.  Starting from the least significant bit, the bits
    correspond to the left-hand half of the top bar, the right-hand half of the
    top bar, continuing clockwise to the upper left segment, the left-hand and
    right-hand halves of the horizontal middle bar, the upper and lower halves
    of the vertical middle bar, and the diagonal bars clockwise from lower left
    to lower right.  Unlit segments are drawn at low intensity (0x20/0xff).
led16segsc
    Draws a standard sixteen-segment alphanumeric LED/fluorescent display with
    decimal point/comma in the specified colour.  The low eighteen bits of the
    element's state control which segments are lit.  The low sixteen bits
    correspond to the same segments as in the ``led16seg`` component.  Two
    additional bits correspond to the decimal point and comma tail.  Unlit
    segments are drawn at low intensity (0x20/0xff).
simplecounter
    Displays the numeric value of the element's state using the system font in
    the specified colour.  The value is formatted in decimal notation.  A
    ``digits`` attribute may be supplied to specify the minimum number of digits
    to display.  If present, the ``digits`` attribute must be a positive
    integer; if absent, a minimum of two digits will be displayed.  A
    ``maxstate`` attribute may be supplied to specify the maximum state value to
    display.  If present, the ``maxstate`` attribute must be a non-negative
    number; if absent it defaults to 999.  An ``align`` attribute may be supplied
    to set text alignment.  If present, the ``align`` attribute must be an
    integer, where 0 (zero) means centred, 1 (one) means left-aligned, and 2
    (two) means right-aligned; if absent, the text will be centred.
reel
    Used for drawing slot machine reels.  Supported attributes include
    ``symbollist``, ``stateoffset``, ``numsymbolsvisible``, ``reelreversed``,
    and ``beltreel``.

An example element that draws a static left-aligned text string::

    <element name="label_reset_cpu">
        <text string="CPU" align="1"><color red="1.0" green="1.0" blue="1.0" /></text>
    </element>


An example element that displays a circular LED where the intensity depends on
the state of an active-high output::

    <element name="led" defstate="0">
        <rect state="0"><color red="0.43" green="0.35" blue="0.39" /></rect>
        <rect state="1"><color red="1.0" green="0.18" blue="0.20" /></rect>
    </element>

An example element for a button that gives visual feedback when clicked::

    <element name="btn_rst">
        <rect state="0"><bounds x="0.0" y="0.0" width="1.0" height="1.0" /><color red="0.2" green="0.2" blue="0.2" /></rect>
        <rect state="1"><bounds x="0.0" y="0.0" width="1.0" height="1.0" /><color red="0.1" green="0.1" blue="0.1" /></rect>
        <rect state="0"><bounds x="0.1" y="0.1" width="0.9" height="0.9" /><color red="0.1" green="0.1" blue="0.1" /></rect>
        <rect state="1"><bounds x="0.1" y="0.1" width="0.9" height="0.9" /><color red="0.2" green="0.2" blue="0.2" /></rect>
        <rect><bounds x="0.1" y="0.1" width="0.8" height="0.8" /><color red="0.15" green="0.15" blue="0.15" /></rect>
        <text string="RESET"><bounds x="0.1" y="0.4" width="0.8" height="0.2" /><color red="1.0" green="1.0" blue="1.0" /></text>
    </element>


.. _layout-parts-repeats:

Repeating blocks
~~~~~~~~~~~~~~~~

Repeating blocks provide a concise way to generate or arrange large numbers of
similar elements.  Repeating blocks are generally used in conjunction with
generator parameters (see :ref:`layout-concepts-params`).  Repeating blocks may
be nested for more complex arrangements.

Repeating blocks are created with ``repeat`` elements.  Each ``repeat`` element
requires a ``count`` attribute specifying the number of iterations to generate.
The ``count`` attribute must be a positive integer.  Repeating blocks are
allowed inside the top-level ``mamelayout`` element, inside ``group`` and
``view`` elements, and insider other ``repeat`` elements.  The exact child
elements allowed inside a ``repeat`` element depend on where it appears:

* A repeating block inside the top-level ``mamelayout`` element may contain
  ``param``, ``element``, ``group`` (definition), and ``repeat`` elements.
* A repeating block inside a ``group`` or ``view`` element may contain
  ``param``, ``backdrop``, ``screen``, ``overlay``, ``bezel``, ``cpanel``,
  ``marquee``, ``group`` (reference), and ``repeat`` elements.

A repeating block effectively repeats its contents the number of times specified
by its ``count`` attribute.  See the relevant sections for details on how the
child elements are used (:ref:`layout-parts`, :ref:`layout-parts-groups`, and
:ref:`layout-parts-views`).  A repeating block creates a nested parameter scope
inside the parameter scope of its lexical (DOM) parent element.

Generating white number labels from zero to eleven named ``label_0``,
``label_1``, and so on (inside the top-level ``mamelayout`` element)::

    <repeat count="12">
        <param name="labelnum" start="0" increment="1" />
        <element name="label_~labelnum~">
            <text string="~labelnum~"><color red="1.0" green="1.0" blue="1.0" /></text>
        </element>
    </repeat>

A horizontal row of forty digital displays, with five units space between them,
controlled by outputs ``digit0`` to ``digit39`` (inside a ``group`` or ``view``
element)::

    <repeat count="40">
        <param name="i" start="0" increment="1" />
        <param name="x" start="5" increment="30" />
        <bezel name="digit~i~" element="digit">
            <bounds x="~x~" y="5" width="25" height="50" />
        </bezel>
    </repeat>

Eight five-by-seven dot matrix displays in a row, with pixels controlled by
outputs ``Dot_000`` to ``Dot_764`` (inside a ``group`` or ``view`` element)::

    <repeat count="8"> <!-- 8 digits -->
        <param name="digitno" start="1" increment="1" />
        <param name="digitx" start="0" increment="935" /> <!-- distance between digits ((111 * 5) + 380) -->
        <repeat count="7"> <!-- 7 rows in each digit -->
            <param name="rowno" start="1" increment="1" />
            <param name="rowy" start="0" increment="114" /> <!-- vertical distance between LEDs -->
            <repeat count="5"> <!-- 5 columns in each digit -->
                <param name="colno" start="1" increment="1" />
                <param name="colx" start="~digitx~" increment="111" /> <!-- horizontal distance between LEDs -->
                <bezel name="Dot_~digitno~~rowno~~colno~" element="Pixel" state="0">
                    <bounds x="~colx~" y="~rowy~" width="100" height="100" /> <!-- size of each LED -->
                </bezel>
            </repeat>
        </repeat>
    </repeat>

Two horizontally separated, clickable, four-by-four keypads (inside a ``group``
or ``view`` element)::

    <repeat count="2">
        <param name="col" start="0" increment="4" />
        <param name="x" start="10" increment="530" />
        <param name="mask" start="0x01" lshift="4" />
        <repeat count="4">
            <param name="row" start="0" increment="1" />
            <param name="y" start="100" increment="110" />
            <repeat count="4">
                <param name="col" start="~col~" increment="1" />
                <param name="x" start="~x~" increment="110" />
                <param name="mask" start="~mask~" lshift="1" />
                <bezel element="btn~row~~col~" inputtag="row~row~" inputmask="~mask~">
                    <bounds x="~x~" y="~y~" width="80" height="80" />
                </bezel>
            </repeat>
        </repeat>
    </repeat>

The buttons are drawn using elements ``btn00`` in the top left, ``bnt07`` in the
top right, ``btn30`` in the bottom left, and ``btn37`` in the bottom right,
counting in between.  The four rows are connected to I/O ports ``row0``,
``row1``, ``row2``, and ``row3``, from top to bottom.  The columns are connected
to consecutive I/O port bits, starting with the least significant bit on the
left.   Note that the ``col``, ``x`` and ``mask`` parameters in the innermost
``repeat`` element take their initial values from the correspondingly named
parameters in the enclosing scope, but do not modify them.

Generating a chequerboard pattern with alternating alpha values 0.4 and 0.2
(inside a ``group`` or ``view`` element)::

    <repeat count="4">
        <param name="ipty" start="3" increment="20" />
        <param name="iptno" start="7" increment="-2" />
        <repeat count="2">
            <param name="ipty" start="~ipty~" increment="10" />
            <param name="iptno" start="~iptno~" increment="-1" />
            <param name="lalpha" start="0.4" increment="-0.2" />
            <param name="ralpha" start="0.2" increment="0.2" />
            <repeat count="4">
                <param name="lx" start="3" increment="20" />
                <param name="rx" start="13" increment="20" />
                <param name="lmask" start="0x01" lshift="2" />
                <param name="rmask" start="0x02" lshift="2" />
                <bezel element="hl" inputtag="board:IN.~iptno~" inputmask="~lmask~">
                    <bounds x="~lx~" y="~ipty~" width="10" height="10" />
                    <color alpha="~lalpha~" />
                </bezel>
                <bezel element="hl" inputtag="board:IN.~iptno~" inputmask="~rmask~">
                    <bounds x="~rx~" y="~ipty~" width="10" height="10" />
                    <color alpha="~ralpha~" />
                </bezel>
            </repeat>
        </repeat>
    </repeat>

Rows are connected to I/O ports ``board:IN.7`` at the top to ``board.IN.0`` at
the bottom.


.. _layout-autogen:

Automatically-generated views
-----------------------------

After loading internal (developer-supplied) and external (user-supplied)
layouts, MAME automatically generates views based on the machine configuration.
The following views will be automatically generated:

* If the system has no screens and no viable views were found in the internal
  and external layouts, MAME will load a view that shows the message "No screens
  attached to the system".
* For each emulated screen, MAME will generate a view showing the screen at its
  physical aspect ratio with rotation applied.
* For each emulated screen where the configured pixel aspect ratio doesn't match
  the physical aspect ratio, MAME will generate a view showing the screen at an
  aspect ratio that produces square pixels, with rotation applied.
* If the system has a single emulated screen, MAME will generate a view showing
  two copies of the screen image above each other with a small gap between them.
  The upper copy will be rotated by 180 degrees.  This view can be used in a
  "cocktail table" cabinet for simultaneous two-player games, or alternating
  play games that don't automatically rotate the display for the second player.
  The screen will be displayed at its physical aspect ratio, with rotation
  applied.
* If the system has exactly two emulated screens and no view in the internal or
  external layouts shows all screens, or if the system has more than two
  emulated screens, MAME will generate views with the screens arranged
  horizontally from left to right and vertically from top to bottom, both with
  and without small gaps between them.  The screens will be displayed at
  physical aspect ratio, with rotation applied.
* If the system has three or more emulated screens, MAME will generate views
  tiling the screens in grid patterns, in both row-major (left-to-right then
  top-to-bottom) and column-major (top-to-bottom then left-to-right) order.
  Views are generated with and without gaps between the screens.  The screens
  will be displayed at physical aspect ratio, with rotation applied.


.. _layout-complay:

Using complay.py
----------------

The MAME source contains a Python script called ``complay.py``, found in the
``scripts/build`` subdirectory.  This script is used as part of MAME's build
process to reduce the size of data for internal layouts and convert it to a form
that can be built into the executable.  However, it can also detect many common
layout file format errors, and generally provides better error messages than
MAME does when loading a layout file.  Note that it doesn't actually run the
whole layout engine, so it can't detect errors like undefined element references
when parameters are used, or recursively nested groups.  The ``complay.py``
script is compatible with both Python 2.7 and Python 3 interpreters.

The ``complay.py`` script takes three parameters -- an input file name, an
output file name, and a base name for variables in the output::

    python scripts/build/complay.py input [output [varname]]

The input file name is required.  If no output file name is supplied,
``complay.py`` will parse and check the input, reporting any errors found,
without producing output.  If no base variable name is provided, ``complay.py``
will generate one based on the input file name.  This is not guaranteed to
produce valid identifiers.  The exit status is 0 (zero) on success, 1 on an
error in the command invocation, 2 if error are found in the input file, or 3
in case of an I/O error.  If an output file name is specified, the file will be
created/overwritten on success or removed on failure.

To check a layout file for common errors, run the script with the path to the
file no check and no output file name or base variable name.  For example::

    python scripts/build/complay.py artwork/dino/default.lay
