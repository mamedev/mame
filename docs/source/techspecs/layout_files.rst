.. _layfile:

MAME Layout Files
=================

.. contents:: :local:


.. _layfile-intro:

Introduction
------------

Layout files are used to tell MAME what to display when running an emulated
system, and how to arrange it.  MAME can render emulated screens, images, text,
shapes, and specialised objects for common output devices.  Elements can be
static, or dynamically update to reflect the state of inputs and outputs.
Layouts may be automatically generated based on the number/type of emulated
screens, built and linked into the MAME binary, or provided externally.  MAME
layout files are an XML application, using the ``.lay`` filename extension.


.. _layfile-concepts:

Core concepts
-------------

.. _layfile-concepts-numbers:

Numbers
~~~~~~~

There are two kinds of numbers in MAME layouts: integers and floating-point
numbers.

Integers may be supplied in decimal or hexadecimal notation.  A decimal integer
consists of an optional # (hash) prefix, an optional +/- (plus or minus) sign
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


.. _layfile-concepts-coordinates:

Coordinates
~~~~~~~~~~~

Layout coordinates are internally represented as IEEE754 32-bit binary
floating-point numbers (also known as “single precision”).  Coordinates increase
in the rightward and downward directions.  The origin (0,0) has no particular
significance, and you may freely use negative coordinates in layouts.
Coordinates are supplied as floating-point numbers.

MAME assumes that view coordinates have the same aspect ratio as pixels on the
output device (host screen or window).  Assuming square pixels and no rotation,
this means equal distances in X and Y axes correspond to equal horizontal and
vertical distances in the rendered output.

Views, groups and elements all have their own internal coordinate systems.  When
an element or group is referenced from a view or another group, its coordinates
are scaled as necessary to fit the specified bounds.

Objects are positioned and sized using ``bounds`` elements.  The horizontal
position and size may be specified in three ways: left edge and width using
``x`` and ``width`` attributes, horizontal centre and width using ``xc`` and
``width`` attributes, or left and right edges using ``left`` and ``right``
attributes.  Similarly, the vertical position and size may be specified in terms
of the top edge and height using ``y`` and ``height`` attributes, vertical
centre and height using ``yc`` and ``height`` attributes, or top and bottom
edges using ``top`` and ``bottom`` attributes.

These three ``bounds`` elements are equivalent:

.. code-block:: XML

    <bounds x="455" y="120" width="12" height="8" />
    <bounds xc="461" yc="124" width="12" height="8" />
    <bounds left="455" top="120" right="467" bottom="128" />

It’s possible to use different schemes in the horizontal and vertical
directions.  For example, these equivalent ``bounds`` elements are also valid:

.. code-block:: XML

    <bounds x="455" top="120" width="12" bottom="128" />
    <bounds left="455" yc="124" right="467" height="8" />

The ``width``/``height`` or ``right``/``bottom`` default to 1.0 if not supplied.
It is an error if ``width`` or ``height`` are negative, if ``right`` is less
than ``left``, or if ``bottom`` is less than ``top``.


.. _layfile-concepts-colours:

Colours
~~~~~~~

Colours are specified in RGBA space.  MAME is not aware of colour profiles and
gamuts, so colours will typically be interpreted as sRGB with your system’s
target gamma (usually 2.2).  Channel values are specified as floating-point
numbers.  Red, green and blue channel values range from 0.0 (off) to 1.0 (full
intensity).  Alpha ranges from 0.0 (fully transparent) to 1.0 (opaque).  Colour
channel values are not pre-multiplied by the alpha value.

Component and view item colour is specified using ``color`` elements.
Meaningful attributes are ``red``, ``green``, ``blue`` and ``alpha``.  This
example ``color`` element specifies all channel values:

.. code-block:: XML

    <color red="0.85" green="0.4" blue="0.3" alpha="1.0" />

Any omitted channel attributes default to 1.0 (full intensity or opaque).  It
is an error if any channel value falls outside the range of 0.0 to 1.0
(inclusive).


.. _layfile-concepts-params:

Parameters
~~~~~~~~~~

Parameters are named variables that can be used in most attributes.  To use
a parameter in an attribute, surround its name with tilde (~) characters.  If a
parameter is not defined, no substitution occurs.  Here is an examples showing
two instances of parameter use – the values of the ``digitno`` and ``x``
parameters will be substituted for ``~digitno~`` and ``~x~``:

.. code-block:: XML

    <element name="digit~digitno~" ref="digit">
        <bounds x="~x~" y="80" width="25" height="40" />
    </element>

A parameter name is a sequence of uppercase English letters A-Z, lowercase
English letters a-z, decimal digits 0-9, and/or underscore (_) characters.
Parameter names are case-sensitive.  When looking for a parameter, the layout
engine starts at the current, innermost scope and works outwards.  The outermost
scope level corresponds to the top-level ``mamelayout`` element.  Each
``repeat``, ``group`` or ``view`` element creates a new, nested scope level.

Internally a parameter can hold a string, integer, or floating-point number, but
this is mostly transparent.  Integers are stored as 64-bit signed
twos-complement values, and floating-point numbers are stored as IEEE754 64-bit
binary floating-point numbers (also known as “double precision”).  Integers are
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

Here’s an example assigning the value “4” to the value parameter “firstdigit”:

.. code-block:: XML

    <param name="firstdigit" value="4" />

Generator parameters are assigned using a ``param`` element with ``name`` and
``start`` attributes, and ``increment``, ``lshift`` and/or ``rshift``
attributes.  Generator parameters may only appear inside ``repeat`` elements
(see :ref:`layfile-parts-repeats` for details).  A generator parameter must not
be reassigned in the same scope (an identically named parameter may be defined
in a child scope).  Here are some example generator parameters:

.. code-block:: XML

    <param name="nybble" start="3" increment="-1" />
    <param name="switchpos" start="74" increment="156" />
    <param name="mask" start="0x0800" rshift="4" />

* The ``nybble`` parameter generates values 3, 2, 1...
* The ``switchpos`` parameter generates values 74, 230, 386...
* The ``mask`` parameter generates values 2048, 128, 8...

The ``increment`` attribute must be an integer or floating-point number to be
added to the parameter’s value.  The ``lshift`` and ``rshift`` attributes must
be non-negative integers specifying numbers of bits to shift the parameter’s
value to the left or right.  The increment and shift are applied at the end of
the repeating block before the next iteration starts.  The parameter’s value
will be interpreted as an integer or floating-point number before the increment
and/or shift are applied.  If both an increment and shift are supplied, the
increment is applied before the shift.

If the ``increment`` attribute is present and is a floating-point number, the
parameter’s value will be converted to a floating-point number if necessary
before the increment is added.  If the ``increment`` attribute is present and is
an integer while the parameter’s value is a floating-point number, the increment
will be converted to a floating-point number before the addition.

If the ``lshift`` and/or ``rshift`` attributes are present and not equal, the
parameter’s value will be converted to an integer if necessary, and shifted
accordingly.  Shifting to the left is defined as shifting towards the most
significant bit.  If both ``lshift`` and ``rshift`` are supplied, they are
netted off before being applied.  This means you cannot, for example, use equal
``lshift`` and ``rshift`` attributes to clear bits at one end of a parameter’s
value after the first iteration.

It is an error if a ``param`` element has neither ``value`` nor ``start``
attributes, and it is an error if a ``param`` element has both a ``value``
attribute and any of the ``start``, ``increment``, ``lshift``, or ``rshift``
attributes.

A ``param`` element defines a parameter or reassigns its value in the current,
innermost scope.  It is not possible to define or reassign parameters in a
containing scope.


.. _layfile-concepts-predef-params:

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
    The horizontal part of the pixel aspect ratio of the first screen’s visible
    area (if present).  The pixel aspect ratio is provided as a reduced improper
    fraction.  Note that this is the horizontal component *before* rotation is
    applied.  This parameter is an integer defined at layout (global) scope.
scr0nativeyaspect
    The vertical part of the pixel aspect ratio of the first screen’s visible
    area (if present).  The pixel aspect ratio is provided as a reduced improper
    fraction.  Note that this is the vertical component *before* rotation is
    applied.  This parameter is an integer defined at layout (global) scope.
scr0width
    The width of the first screen’s visible area (if present) in emulated
    pixels.  Note that this is the width *before* rotation is applied.  This
    parameter is an integer defined at layout (global) scope.
scr0height
    The height of the first screen’s visible area (if present) in emulated
    pixels.  Note that this is the height *before* rotation is applied.  This
    parameter is an integer defined at layout (global) scope.
scr1physicalxaspect
    The horizontal part of the physical aspect ratio of the second screen (if
    present).  This parameter is an integer defined at layout (global) scope.
scr1physicalyaspect
    The vertical part of the physical aspect ratio of the second screen (if
    present).  This parameter is an integer defined at layout (global) scope.
scr1nativexaspect
    The horizontal part of the pixel aspect ratio of the second screen’s visible
    area (if present).  This parameter is an integer defined at layout (global)
    scope.
scr1nativeyaspect
    The vertical part of the pixel aspect ratio of the second screen’s visible
    area (if present).  This parameter is an integer defined at layout (global)
    scope.
scr1width
    The width of the second screen’s visible area (if present) in emulated
    pixels.  This parameter is an integer defined at layout (global) scope.
scr1height
    The height of the second screen’s visible area (if present) in emulated
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
    screen’s visible area (if present).  This parameter is an integer defined at
    layout (global) scope.
scr\ *N*\ nativeyaspect
    The vertical part of the pixel aspect ratio of the (zero-based) *N*\ th
    screen’s visible area (if present).  This parameter is an integer defined at
    layout (global) scope.
scr\ *N*\ width
    The width of the (zero-based) *N*\ th screen’s visible area (if present) in
    emulated pixels.  This parameter is an integer defined at layout (global)
    scope.
scr\ *N*\ height
    The height of the (zero-based) *N*\ th screen’s visible area (if present) in
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


.. _layfile-parts:

Parts of a layout
-----------------

A *view* specifies an arrangement graphical object to display.  A MAME layout
file can contain multiple views.  Views are built up from *elements* and
*screens*.  To simplify complex layouts, reusable groups and repeating blocks
are supported.

The top-level element of a MAME layout file must be a ``mamelayout`` element
with a ``version`` attribute.  The ``version`` attribute must be an integer.
Currently MAME only supports version 2, and will not load any other version.
This is an example opening tag for a top-level ``mamelayout`` element:

.. code-block:: XML

    <mamelayout version="2">

In general, children of the top-level ``mamelayout`` element are processed in
reading order from top to bottom.  The exception is that, for historical
reasons, views are processed last.  This means views see the final values of all
parameters at the end of the ``mamelayout`` element, and may refer to elements
and groups that appear after them.

The following elements are allowed inside the top-level ``mamelayout`` element:

param
    Defines or reassigns a value parameter.  See :ref:`layfile-concepts-params`
    for details.
element
    Defines an element – one of the basic objects that can be arranged in a
    view.  See :ref:`layfile-parts-elements` for details.
group
    Defines a reusable group of elements/screens that may be referenced from
    views or other groups.  See :ref:`layfile-parts-groups` for details.
repeat
    A repeating group of elements – may contain ``param``, ``element``,
    ``group``, and ``repeat`` elements.  See :ref:`layfile-parts-repeats` for
    details.
view
    An arrangement of elements and/or screens that can be displayed on an output
    device (a host screen/window).  See :ref:`layfile-parts-views` for details.
script
    Allows Lua script to be supplied for enhanced interactive layouts.  See
    :ref:`layscript` for details.


.. _layfile-parts-elements:

Elements
~~~~~~~~

Elements are one of the basic visual objects that may be arranged, along with
screens, to make up a view.  Elements may be built up one or more *components*,
but an element is treated as as single surface when building the scene graph
and rendering.  An element may be used in multiple views, and may be used
multiple times within a view.

An element’s appearance depends on its *state*.  The state is an integer which
usually comes from an I/O port field or an emulated output (see
:ref:`layfile-interact-elemstate` for information on connecting an element to an
emulated I/O port or output).  Any component of an element may be restricted to
only drawing when the element’s state is a particular value.  Some components
(e.g.  multi-segment displays) use the state directly to determine their
appearance.

Each element has its own internal coordinate system.  The bounds of the
element’s coordinate system are computed as the union of the bounds of the
individual components it’s composed of.

Every element must have a ``name`` attribute specifying its name.  Elements are
referred to by name when instantiated in groups or views.  It is an error for a
layout file to contain multiple elements with identical ``name`` attributes.
Elements may optionally supply a default state value with a ``defstate``
attribute, to be used if not connected to an emulated output or I/O port.  If
present, the ``defstate`` attribute must be a non-negative integer.

Child elements of the ``element`` element instantiate components, which are
drawn into the element texture in reading order from first to last using alpha
blending (components draw over and may obscure components that come before
them).  All components support a few common features:

* Components may be conditionally drawn depending on the element’s state by
  supplying ``state`` and/or ``statemask`` attributes.  If present, these
  attributes must be non-negative integers.  If only the ``state`` attribute is
  present, the component will only be drawn when the element’s state matches its
  value.  If only the ``statemask`` attribute is present, the component will
  only be drawn when all the bits that are set in its value are set in the
  element’s state.

  If both the ``state`` and ``statemask`` attributes are present, the component
  will only be drawn when the bits in the element’s state corresponding to the
  bits that are set in the ``statemask`` attribute’s value match the value of the
  corresponding bits in the ``state`` attribute’s value.

  (The component will always be drawn if neither ``state`` nor ``statemask``
  attributes are present, or if the ``statemask`` attribute’s value is zero.)
* Each component may have a ``bounds`` child element specifying its position and
  size (see :ref:`layfile-concepts-coordinates`).  If no such element is
  present, the bounds default to a unit square (width and height of 1.0) with
  the top left corner at (0,0).

  A component’s position and/or size may be animated according to the element’s
  state by supplying multiple ``bounds`` child elements with ``state``
  attributes.  The ``state`` attribute of each ``bounds`` child element must be
  a non-negative integer.  The ``state`` attributes must not be equal for any
  two ``bounds`` elements within a component.

  If the element’s state is lower than the ``state`` value of any ``bounds``
  child element, the position/size specified by the ``bounds`` child element
  with the lowest ``state`` value will be used.  If the element’s state is
  higher than the ``state`` value of any ``bounds`` child element, the
  position/size specified by the ``bounds`` child element with the highest
  ``state`` value will be used.  If the element’s state is between the ``state``
  values of two ``bounds`` child elements, the position/size will be
  interpolated linearly.
* Each component may have a ``color`` child element specifying an RGBA colour
  (see :ref:`layfile-concepts-colours` for details).  This can be used to
  control the colour of geometric, algorithmically drawn, or textual components.
  For ``image`` components, the colour of the image pixels is multiplied by the
  specified colour.  If no such element is present, the colour defaults to
  opaque white.

  A component’s color may be animated according to the element’s state by
  supplying multiple ``color`` child elements with ``state`` attributes.  The
  ``state`` attributes must not be equal for any two ``color`` elements within a
  component.

  If the element’s state is lower than the ``state`` value of any ``color``
  child element, the colour specified by the ``color`` child element with the
  lowest ``state`` value will be used.  If the element’s state is higher than
  the ``state`` value of any ``color`` child element, the colour specified by
  the ``color`` child element with the highest ``state`` value will be used.  If
  the element’s state is between the ``state`` values of two ``color`` child
  elements, the RGBA colour components will be interpolated linearly.

The following components are supported:

rect
    Draws a uniform colour rectangle filling its bounds.
disk
    Draws a uniform colour ellipse fitted to its bounds.
image
    Draws an image loaded from a PNG, JPEG, Windows DIB (BMP) or SVG file.  The
    name of the file to load (including the file name extension) is supplied
    using the ``file`` attribute.  Additionally, an optional ``alphafile``
    attribute may be used to specify the name of a PNG file (including the file
    name extension) to load into the alpha channel of the image.

    Alternatively, image data may be supplied in the layout file itself using a
    ``data`` child element.  This can be useful for supplying simple,
    human-readable SVG graphics.  A ``file`` attribute or ``data`` child element
    must be supplied; it is an error if neither or both are supplied.

    If the ``alphafile`` attribute refers  refers to a file, it must have the
    same dimensions (in pixels) as the file referred to by the ``file``
    attribute, and must have a bit depth no greater than eight bits per channel
    per pixel.  The intensity from this image (brightness) is copied to the
    alpha channel, with full intensity (white in a greyscale image)
    corresponding to fully opaque, and black corresponding to fully transparent.
    The ``alphafile`` attribute will be ignored if the ``file`` attribute refers
    to an SVG image or the ``data`` child element contains SVG data; it is only
    used in conjunction with bitmap images.

    The image file(s) should be placed in the same directory/archive as the
    layout file.  Image file formats are detected by examining the content of
    the files, file name extensions are ignored.
text
    Draws text in using the UI font in the specified colour.  The text to draw
    must be supplied using a ``string`` attribute.  An ``align`` attribute may
    be supplied to set text alignment.  If present, the ``align`` attribute must
    be an integer, where 0 (zero) means centred, 1 (one) means left-aligned, and
    2 (two) means right-aligned.  If the ``align`` attribute is absent, the text
    will be centred.
led7seg
    Draws a standard seven-segment (plus decimal point) digital LED/fluorescent
    display in the specified colour.  The low eight bits of the element’s state
    control which segments are lit.  Starting from the least significant bit,
    the bits correspond to the top segment, the upper right-hand segment,
    continuing clockwise to the upper left segment, the middle bar, and the
    decimal point.  Unlit segments are drawn at low intensity (0x20/0xff).
led14seg
    Draws a standard fourteen-segment alphanumeric LED/fluorescent display in
    the specified colour.  The low fourteen bits of the element’s state control
    which segments are lit.  Starting from the least significant bit, the bits
    correspond to the top segment, the upper right-hand segment, continuing
    clockwise to the upper left segment, the left-hand and right-hand halves of
    the horizontal middle bar, the upper and lower halves of the vertical middle
    bar, and the diagonal bars clockwise from lower left to lower right.  Unlit
    segments are drawn at low intensity (0x20/0xff).
led14segsc
    Draws a standard fourteen-segment alphanumeric LED/fluorescent display with
    decimal point/comma in the specified colour.  The low sixteen bits of the
    element’s state control which segments are lit.  The low fourteen bits
    correspond to the same segments as in the ``led14seg`` component.  Two
    additional bits correspond to the decimal point and comma tail.  Unlit
    segments are drawn at low intensity (0x20/0xff).
led16seg
    Draws a standard sixteen-segment alphanumeric LED/fluorescent display in the
    specified colour.  The low sixteen bits of the element’s state control which
    segments are lit.  Starting from the least significant bit, the bits
    correspond to the left-hand half of the top bar, the right-hand half of the
    top bar, continuing clockwise to the upper left segment, the left-hand and
    right-hand halves of the horizontal middle bar, the upper and lower halves
    of the vertical middle bar, and the diagonal bars clockwise from lower left
    to lower right.  Unlit segments are drawn at low intensity (0x20/0xff).
led16segsc
    Draws a standard sixteen-segment alphanumeric LED/fluorescent display with
    decimal point/comma in the specified colour.  The low eighteen bits of the
    element’s state control which segments are lit.  The low sixteen bits
    correspond to the same segments as in the ``led16seg`` component.  Two
    additional bits correspond to the decimal point and comma tail.  Unlit
    segments are drawn at low intensity (0x20/0xff).
simplecounter
    Displays the numeric value of the element’s state using the system font in
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

An example element that draws a static left-aligned text string:

.. code-block:: XML

    <element name="label_reset_cpu">
        <text string="CPU" align="1"><color red="1.0" green="1.0" blue="1.0" /></text>
    </element>

An example element that displays a circular LED where the intensity depends on
the state of an active-high output:

.. code-block:: XML

    <element name="led" defstate="0">
        <disk state="0"><color red="0.43" green="0.35" blue="0.39" /></disk>
        <disk state="1"><color red="1.0" green="0.18" blue="0.20" /></disk>
    </element>

An example element for a button that gives visual feedback when clicked:

.. code-block:: XML

    <element name="btn_rst">
        <rect state="0"><bounds x="0.0" y="0.0" width="1.0" height="1.0" /><color red="0.2" green="0.2" blue="0.2" /></rect>
        <rect state="1"><bounds x="0.0" y="0.0" width="1.0" height="1.0" /><color red="0.1" green="0.1" blue="0.1" /></rect>
        <rect state="0"><bounds x="0.1" y="0.1" width="0.9" height="0.9" /><color red="0.1" green="0.1" blue="0.1" /></rect>
        <rect state="1"><bounds x="0.1" y="0.1" width="0.9" height="0.9" /><color red="0.2" green="0.2" blue="0.2" /></rect>
        <rect><bounds x="0.1" y="0.1" width="0.8" height="0.8" /><color red="0.15" green="0.15" blue="0.15" /></rect>
        <text string="RESET"><bounds x="0.1" y="0.4" width="0.8" height="0.2" /><color red="1.0" green="1.0" blue="1.0" /></text>
    </element>

An example of an element that draws a seven-segment LED display using external
segment images:

.. code-block:: XML

    <element name="digit_a" defstate="0">
        <image file="a_off.png" />
        <image file="a_a.png" statemask="0x01" />
        <image file="a_b.png" statemask="0x02" />
        <image file="a_c.png" statemask="0x04" />
        <image file="a_d.png" statemask="0x08" />
        <image file="a_e.png" statemask="0x10" />
        <image file="a_f.png" statemask="0x20" />
        <image file="a_g.png" statemask="0x40" />
        <image file="a_dp.png" statemask="0x80" />
    </element>

An example of a bar graph that grows vertically and changes colour from green,
through yellow, to red as the state increases:

.. code-block:: XML

    <element name="pedal">
        <rect>
            <bounds state="0x000" left="0.0" top="0.9" right="1.0" bottom="1.0" />
            <bounds state="0x610" left="0.0" top="0.0" right="1.0" bottom="1.0" />
            <color state="0x000" red="0.0" green="1.0" blue="0.0" />
            <color state="0x184" red="1.0" green="1.0" blue="0.0" />
            <color state="0x610" red="1.0" green="0.0" blue="0.0" />
        </rect>
    </element>

An example of a bar graph that grows horizontally to the left or right and
changes colour from green, through yellow, to red as the state changes from the
neutral position:

.. code-block:: XML

    <element name="wheel">
        <rect>
            <bounds state="0x800" left="0.475" top="0.0" right="0.525" bottom="1.0" />
            <bounds state="0x280" left="0.0" top="0.0" right="0.525" bottom="1.0" />
            <bounds state="0xd80" left="0.475" top="0.0" right="1.0" bottom="1.0" />
            <color state="0x800" red="0.0" green="1.0" blue="0.0" />
            <color state="0x3e0" red="1.0" green="1.0" blue="0.0" />
            <color state="0x280" red="1.0" green="0.0" blue="0.0" />
            <color state="0xc20" red="1.0" green="1.0" blue="0.0" />
            <color state="0xd80" red="1.0" green="0.0" blue="0.0" />
        </rect>
    </element>


.. _layfile-parts-views:

Views
~~~~~

A view defines an arrangement of elements and/or emulated screen images that can
be displayed in a window or on a screen.  Views also connect elements to
emulated I/O ports and/or outputs.  A layout file may contain multiple views.
If a view references a non-existent screen, it will be considered *unviable*.
MAME will print a warning message, skip over the unviable view, and continue to
load views from the layout file.  This is particularly useful for systems where
a screen is optional, for example computer systems with front panel controls and
an optional serial terminal.

Views are identified by name in MAME’s user interface and in command-line
options.  For layouts files associated with devices other than the root driver
device, view names are prefixed with the device’s tag (with the initial colon
omitted) – for example a view called “Keyboard LEDs” loaded for the device
``:tty:ie15`` will be called “tty:ie15 Keyboard LEDs” in MAME’s user interface.
Views are listed in the order they are loaded.  Within a layout file, views are
loaded in the order they appear, from top to bottom.

Views are created with ``view`` elements inside the top-level ``mamelayout``
element.  Each ``view`` element must have a ``name`` attribute, supplying its
human-readable name for use in the user interface and command-line options.
This is an example of a valid opening tag for a ``view`` element:

.. code-block:: XML

    <view name="Control panel">

A view creates a nested parameter scope inside the parameter scope of the
top-level ``mamelayout`` element.  For historical reasons, ``view`` elements are
processed *after* all other child elements of the top-level ``mamelayout``
element.  This means a view can reference elements and groups that appear after
it in the file, and parameters from the enclosing scope will have their final
values from the end of the ``mamelayout`` element.

The following child elements are allowed inside a ``view`` element:

bounds
    Sets the origin and size of the view’s internal coordinate system if
    present.  See :ref:`layfile-concepts-coordinates` for details.  If absent,
    the bounds of the view are computed as the union of the bounds of all
    screens and elements within the view.  It only makes sense to have one
    ``bounds`` as a direct child of a view element.  Any content outside the
    view’s bounds is cropped, and the view is scaled proportionally to fit the
    output window or screen.
param
    Defines or reassigns a value parameter in the view’s scope.  See
    :ref:`layfile-concepts-params` for details.
element
    Adds an element to the view (see :ref:`layfile-parts-elements`).  The name
    of the element to add is specified using the required ``ref`` attribute.  It
    is an error if no element with this name is defined in the layout file.
    Within a view, elements are drawn in the order they appear in the layout
    file, from front to back.  See below for more details.

    May optionally be connected to an emulated I/O port using ``inputtag`` and
    ``inputmask`` attributes, and/or an emulated output using a ``name``
    attribute.  See :ref:`layfile-interact-clickable` for details.  See
    :ref:`layfile-interact-elemstate` for details on supplying a state value to
    the instantiated element.
screen
    Adds an emulated screen image to the view.  The screen must be identified
    using either an ``index`` attribute or a ``tag`` attribute (it is an error
    for a ``screen`` element to have both ``index`` and ``tag`` attributes).
    If present, the ``index`` attribute must be a non-negative integer.  Screens
    are numbered by the order they appear in machine configuration, starting at
    zero (0).  If present, the ``tag`` attribute must be the tag path to the
    screen relative to the device that causes the layout to be loaded.  Screens
    are drawn in the order they appear in the layout file, from front to back.

    May optionally be connected to an emulated I/O port using ``inputtag`` and
    ``inputmask`` attributes, and/or an emulated output using a ``name``
    attribute.  See :ref:`layfile-interact-clickable` for details.
collection
    Adds screens and/or items in a collection that can be shown or hidden by the
    user (see :ref:`layfile-parts-collections`).  The name of the collection is
    specified using the required ``name`` attribute..  There is a limit of 32
    collections per view.
group
    Adds the content of the group to the view (see :ref:`layfile-parts-groups`).
    The name of the group to add is specified using the required ``ref``
    attribute.  It is an error if no group with this name is defined in the
    layout file.  See below for more details on positioning.
repeat
    Repeats its contents the number of times specified by the required ``count``
    attribute.  The ``count`` attribute must be a positive integer.  A
    ``repeat`` element in a view may contain ``element``, ``screen``, ``group``,
    and further ``repeat`` elements, which function the same way they do when
    placed in a view directly.  See :ref:`layfile-parts-repeats` for discussion
    on using ``repeat`` elements.

Screens (``screen`` elements) and layout elements (``element`` elements) may
have an ``id`` attribute.  If present, the ``id`` attribute must not be empty,
and must be unique within a view, including screens and elements instantiated
via reusable groups and repeating blocks.  Screens and layout elements with
``id`` attributes can be looked up by Lua scripts (see :ref:`layscript`).

Screens (``screen`` elements), layout elements (``element`` elements) and groups
(``group`` elements) may have their orientation altered using an ``orientation``
child element.  For screens, the orientation modifiers are applied in addition
to the orientation modifiers specified on the screen device and on the machine.
The ``orientation`` element supports the following attributes, all of which are
optional:

rotate
    If present, applies clockwise rotation in ninety degree increments.  Must be
    an integer equal to 0, 90, 180 or 270.
swapxy
    Allows the screen, element or group to be mirrored along a line at
    forty-five degrees to vertical from upper left to lower right.  Must be
    either ``yes`` or ``no`` if present.  Mirroring applies logically after
    rotation.
flipx
    Allows the screen, element or group to be mirrored around its vertical axis,
    from left to right.  Must be either ``yes`` or ``no`` if present.  Mirroring
    applies logically after rotation.
flipy
    Allows the screen, element or group to be mirrored around its horizontal
    axis, from top to bottom.  Must be either ``yes`` or ``no`` if present.
    Mirroring applies logically after rotation.

Screens (``screen`` elements) and layout elements (``element`` elements) may
have a ``blend`` attribute to set the blending mode.  Supported values are
``none`` (no blending), ``alpha`` (alpha blending), ``multiply`` (RGB
multiplication), and ``add`` (additive blending).  The default for screens is to
allow the driver to specify blending per layer; the default blending mode for
layout elements is alpha blending.

Screens (``screen`` elements), layout elements (``element`` elements) and groups
(``group`` elements) may be positioned and sized using a ``bounds`` child
element (see :ref:`layfile-concepts-coordinates` for details).  In the absence
of a ``bounds`` child element, screens’ and layout elements’ bounds default to a
unit square (origin at 0,0 and height and width both equal to 1).  In the
absence of a ``bounds`` child element, groups are expanded with no
translation/scaling (note that groups may position screens/elements outside
their bounds).  This example shows a view instantiating and positioning a
screen, an individual layout element, and two element groups:

.. code-block:: XML

    <view name="LED Displays, Terminal and Keypad">
        <screen index="0"><bounds x="0" y="132" width="320" height="240" /></screen>
        <element ref="beige"><bounds x="320" y="0" width="172" height="372" /></element>
        <group ref="displays"><bounds x="0" y="0" width="320" height="132" /></group>
        <group ref="keypad"><bounds x="336" y="16" width="140" height="260" /></group>
    </view>

Screens (``screen`` elements), layout elements (``element`` elements) and groups
(``group`` elements) may have a ``color`` child element (see
:ref:`layfile-concepts-colours`) specifying a modifier colour.  The component
colours of the screen or layout element(s) are multiplied by this colour.

Screens (``screen`` elements) and layout elements (``element`` elements) may
have their colour and position/size animated by supplying multiple ``color``
and/or ``bounds`` child elements with ``state`` attributes.  See
:ref:`layfile-interact-itemanim` for details.

Layout elements (``element`` elements) may be configured to show only part of
the element’s width or height using ``xscroll`` and/or ``yscroll`` child
elements.  This can be used for devices like slot machine reels.  The
``xscroll`` and ``yscroll`` elements support the same attributes:

size
    The size of the horizontal or vertical scroll window, as a proportion of the
    element’s width or height, respectively.  Must be in the range 0.01 to 1.0,
    inclusive, if present (1% of the width/height to the full width/height).  By
    default, the entire width and height of the element is shown.
wrap
    Whether the element should wrap horizontally or vertically.  Must be either
    ``yes`` or ``no`` if present.  By default, items do not wrap horizontally or
    vertically.
inputtag
    If present, the horizontal or vertical scroll position will be taken from
    the value of the corresponding I/O port.  Specifies the tag path of an I/O
    port relative to the device that caused the layout file to be loaded.  The
    raw value from the input port is used, active-low switch values are not
    normalised.
name
    If present, the horizontal or vertical scroll position will be taken from
    the correspondingly named output.
mask
    If present, the horizontal or vertical scroll position will be masked with
    the value and shifted to the right to remove trailing zeroes (for example a
    mask of 0x05 will result in no shift, while a mask of 0x68 will result in
    the value being shifted three bits to the right).  Note that this applies to
    output values (specified with the ``name`` attribute) as well as input port
    values (specified with the ``inputtag`` attribute).  Must be an integer
    value if present.  If not present, it is equivalent to all 32 bits being
    set.
min
    Minimum horizontal or vertical scroll position value.  When the horizontal
    or vertical scroll position has this value, the left or top edge or the
    scroll window will be aligned with the left or top edge of the element.
    Must be an integer value if present.  Defaults to zero.
max
    Maximum horizontal or vertical scroll position value.  Must be an integer
    value if present.  Defaults to the ``mask`` value shifted to the right to
    remove trailing zeroes.


.. _layfile-parts-collections:

Collections
~~~~~~~~~~~

Collections of screens and/or layout elements can be shown or hidden by the user
as desired.  For example, a single view could include both displays and a
clickable keypad, and allow the user to hide the keypad leaving only the
displays visible.  Collections are created using ``collection`` elements inside
``view``, ``group`` and other ``collection`` elements.

A collection element must have a ``name`` attribute providing the display name
for the collection.  Collection names must be unique within a view.  The initial
visibility of a collection may be specified by providing a ``visible``
attribute.  Set the ``visible`` attribute to ``yes`` if the collection should be
initially visible, or ``no`` if it should be initially hidden.  Collections are
initially visible by default.

Here is an example demonstrating the use of collections to allow parts of a view
to be hidden by the user:

.. code-block:: XML

    <view name="LED Displays, CRT and Keypad">
        <collection name="LED Displays">
            <group ref="displays"><bounds x="240" y="0" width="320" height="47" /></group>
        </collection>
        <collection name="Keypad">
            <group ref="keypad"><bounds x="650" y="57" width="148" height="140" /></group>
        </collection>
        <screen tag="screen"><bounds x="0" y="57" width="640" height="480" /></screen>
    </view>


A collection creates a nested parameter scope.  Any ``param`` elements inside
the collection element set parameters in the local scope for the collection.
See :ref:`layfile-concepts-params` for more detail on parameters.  (Note that
the collection’s name and default visibility are not part of its content, and
any parameter references in the ``name`` and ``visible`` attributes themselves
will be substituted using parameter values from the collection’s parent’s
scope.)


.. _layfile-parts-groups:

Reusable groups
~~~~~~~~~~~~~~~

Groups allow an arrangement of screens and/or layout elements to be used
multiple times in views or other groups.  Groups can be beneficial even if you
only use the arrangement once, as they can be used to encapsulate part of a
complex layout.  Groups are defined using ``group`` elements inside the
top-level ``mamelayout`` element, and instantiated using ``group`` elements
inside ``view`` and other ``group`` elements.

Each group definition element must have a ``name`` attribute providing a unique
identifier.  It is an error if a layout file contains multiple group definitions
with identical ``name`` attributes.  The value of the ``name`` attribute is used
when instantiating the group from a view or another group.  This is an example
opening tag for a group definition element inside the top-level ``mamelayout``
element:

.. code-block:: XML

    <group name="panel">

This group may then be instantiated in a view or another group element using a
group reference element, optionally supplying destination bounds, orientation,
and/or modifier colour.  The ``ref`` attribute identifies the group to
instantiate – in this example, destination bounds are supplied:

.. code-block:: XML

    <group ref="panel"><bounds x="87" y="58" width="23" height="23.5" /></group>

Group definition elements allow all the same child elements as views.
Positioning and orienting screens, layout elements and nested groups works the
same way as for views.  See :ref:`layfile-parts-views` for details.  A group may
instantiate other groups, but recursive loops are not permitted.  It is an error
if a group directly or indirectly instantiates itself.

Groups have their own internal coordinate systems.  If a group definition
element has no ``bounds`` element as a direct child, its bounds are computed as
the union of the bounds of all the screens, layout elements and/or nested groups
it instantiates.  A ``bounds`` child element may be used to explicitly specify
group bounds (see :ref:`layfile-concepts-coordinates` for details).  Note that
groups’ bounds are only used for the purpose of calculating the coordinate
transform when instantiating a group.  A group may position screens and/or
elements outside its bounds, and they will not be cropped.

To demonstrate how bounds calculation works, consider this example:

.. code-block:: XML

    <group name="autobounds">
        <!-- bounds automatically calculated with origin at (5,10), width 30, and height 15 -->
        <element ref="topleft"><bounds x="5" y="10" width="10" height="10" /></element>
        <element ref="bottomright"><bounds x="25" y="15" width="10" height="10" /></element>
    </group>

    <view name="Test">
        <!--
            group bounds translated and scaled to fit - 2/3 scale horizontally and double vertically
            element topleft positioned at (0,0) with width 6.67 and height 20
            element bottomright positioned at (13.33,10) with width 6.67 and height 20
            view bounds calculated with origin at (0,0), width 20, and height 30
        -->
        <group ref="autobounds"><bounds x="0" y="0" width="20" height="30" /></group>
    </view>

This is relatively straightforward, as all elements inherently fall within the
group’s automatically computed bounds.  Now consider what happens if a group
positions elements outside its explicit bounds:

.. code-block:: XML

    <group name="periphery">
        <!-- elements are above the top edge and to the right of the right edge of the bounds -->
        <bounds x="10" y="10" width="20" height="25" />
        <element ref="topleft"><bounds x="10" y="0" width="10" height="10" /></element>
        <element ref="bottomright"><bounds x="30" y="20" width="10" height="10" /></element>
    </group>

    <view name="Test">
        <!--
            group bounds translated and scaled to fit - 3/2 scale horizontally and unity vertically
            element topleft positioned at (5,-5) with width 15 and height 10
            element bottomright positioned at (35,15) with width 15 and height 10
            view bounds calculated with origin at (5,-5), width 45, and height 30
        -->
        <group ref="periphery"><bounds x="5" y="5" width="30" height="25" /></group>
    </view>

The group’s elements are translated and scaled as necessary to distort the
group’s internal bounds to the destination bounds in the view.  The group’s
content is not restricted to its bounds.  The view considers the bounds of the
actual layout elements when computing its bounds, not the destination bounds
specified for the group.

When a group is instantiated, it creates a nested parameter scope.  The logical
parent scope is the parameter scope of the view, group or repeating block where
the group is instantiated (*not* its lexical parent, the top-level
``mamelayout`` element).  Any ``param`` elements inside the group definition
element set parameters in the local scope for the group instantiation.  Local
parameters do not persist across multiple instantiations.  See
:ref:`layfile-concepts-params` for more detail on parameters.  (Note that the
group’s name is not part of its content, and any parameter references in the
``name`` attribute itself will be substituted at the point where the group
definition appears in the top-level ``mamelayout`` element’s scope.)


.. _layfile-parts-repeats:

Repeating blocks
~~~~~~~~~~~~~~~~

Repeating blocks provide a concise way to generate or arrange large numbers of
similar elements.  Repeating blocks are generally used in conjunction with
generator parameters (see :ref:`layfile-concepts-params`).  Repeating blocks may
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
  ``param``, ``element`` (reference), ``screen``, ``group`` (reference), and
  ``repeat`` elements.

A repeating block effectively repeats its contents the number of times specified
by its ``count`` attribute.  See the relevant sections for details on how the
child elements are used (:ref:`layfile-parts`, :ref:`layfile-parts-groups`, and
:ref:`layfile-parts-views`).  A repeating block creates a nested parameter scope
inside the parameter scope of its lexical (DOM) parent element.

Generating white number labels from zero to eleven named ``label_0``,
``label_1``, and so on (inside the top-level ``mamelayout`` element):

.. code-block:: XML

    <repeat count="12">
        <param name="labelnum" start="0" increment="1" />
        <element name="label_~labelnum~">
            <text string="~labelnum~"><color red="1.0" green="1.0" blue="1.0" /></text>
        </element>
    </repeat>

A horizontal row of forty digital displays, with five units space between them,
controlled by outputs ``digit0`` to ``digit39`` (inside a ``group`` or ``view``
element):

.. code-block:: XML

    <repeat count="40">
        <param name="i" start="0" increment="1" />
        <param name="x" start="5" increment="30" />
        <element name="digit~i~" ref="digit">
            <bounds x="~x~" y="5" width="25" height="50" />
        </element>
    </repeat>

Eight five-by-seven dot matrix displays in a row, with pixels controlled by
outputs ``Dot_000`` to ``Dot_764`` (inside a ``group`` or ``view`` element):

.. code-block:: XML

    <repeat count="8"> <!-- 8 digits -->
        <param name="digitno" start="1" increment="1" />
        <param name="digitx" start="0" increment="935" /> <!-- distance between digits ((111 * 5) + 380) -->
        <repeat count="7"> <!-- 7 rows in each digit -->
            <param name="rowno" start="1" increment="1" />
            <param name="rowy" start="0" increment="114" /> <!-- vertical distance between LEDs -->
            <repeat count="5"> <!-- 5 columns in each digit -->
                <param name="colno" start="1" increment="1" />
                <param name="colx" start="~digitx~" increment="111" /> <!-- horizontal distance between LEDs -->
                <element name="Dot_~digitno~~rowno~~colno~" ref="Pixel" state="0">
                    <bounds x="~colx~" y="~rowy~" width="100" height="100" /> <!-- size of each LED -->
                </element>
            </repeat>
        </repeat>
    </repeat>

Two horizontally separated, clickable, four-by-four keypads (inside a ``group``
or ``view`` element):

.. code-block:: XML

    <repeat count="2">
        <param name="group" start="0" increment="4" />
        <param name="padx" start="10" increment="530" />
        <param name="mask" start="0x01" lshift="4" />
        <repeat count="4">
            <param name="row" start="0" increment="1" />
            <param name="y" start="100" increment="110" />
            <repeat count="4">
                <param name="col" start="~group~" increment="1" />
                <param name="btnx" start="~padx~" increment="110" />
                <param name="mask" start="~mask~" lshift="1" />
                <element ref="btn~row~~col~" inputtag="row~row~" inputmask="~mask~">
                    <bounds x="~btnx~" y="~y~" width="80" height="80" />
                </element>
            </repeat>
        </repeat>
    </repeat>

The buttons are drawn using elements ``btn00`` in the top left, ``bnt07`` in the
top right, ``btn30`` in the bottom left, and ``btn37`` in the bottom right,
counting in between.  The four rows are connected to I/O ports ``row0``,
``row1``, ``row2``, and ``row3``, from top to bottom.  The columns are connected
to consecutive I/O port bits, starting with the least significant bit on the
left.   Note that the ``mask`` parameter in the innermost ``repeat`` element
takes its initial value from the correspondingly named parameter in the
enclosing scope, but does not modify it.

Generating a chequerboard pattern with alternating alpha values 0.4 and 0.2
(inside a ``group`` or ``view`` element):

.. code-block:: XML

    <repeat count="4">
        <param name="pairy" start="3" increment="20" />
        <param name="pairno" start="7" increment="-2" />
        <repeat count="2">
            <param name="rowy" start="~pairy~" increment="10" />
            <param name="rowno" start="~pairno~" increment="-1" />
            <param name="lalpha" start="0.4" increment="-0.2" />
            <param name="ralpha" start="0.2" increment="0.2" />
            <repeat count="4">
                <param name="lx" start="3" increment="20" />
                <param name="rx" start="13" increment="20" />
                <param name="lmask" start="0x01" lshift="2" />
                <param name="rmask" start="0x02" lshift="2" />
                <element ref="hl" inputtag="board:IN.~rowno~" inputmask="~lmask~">
                    <bounds x="~lx~" y="~rowy~" width="10" height="10" />
                    <color alpha="~lalpha~" />
                </element>
                <element ref="hl" inputtag="board:IN.~rowno~" inputmask="~rmask~">
                    <bounds x="~rx~" y="~rowy~" width="10" height="10" />
                    <color alpha="~ralpha~" />
                </element>
            </repeat>
        </repeat>
    </repeat>

The outermost ``repeat`` element generates a group of two rows on each
iteration; the next ``repeat`` element generates an individual row on each
iteration; the innermost ``repeat`` element produces two horizontally adjacent
tiles on each iteration.  Rows are connected to I/O ports ``board:IN.7`` at the
top to ``board.IN.0`` at the bottom.


.. _layfile-interact:

Interactivity
-------------

Interactive views are supported by allowing items to be bound to emulated
outputs and I/O ports.  Five kinds of interactivity are supported:

Clickable items
    If an item in a view is bound to an I/O port switch field, clicking the
    item will activate the emulated switch.
State-dependent components
    Some components will be drawn differently depending on the containing
    element’s state.  These include the dot matrix, multi-segment LED display
    and simple counter elements.  See :ref:`layfile-parts-elements` for details.
Conditionally-drawn components
    Components may be conditionally drawn or hidden depending on the containing
    element’s state by supplying ``state`` and/or ``statemask`` attributes.  See
    :ref:`layfile-parts-elements` for details.
Component parameter animation
    Components’ colour and position/size within their containing element may be
    animated according the element’s state by providing multiple ``color``
    and/or ``bounds`` elements with ``state`` attributes.  See
    :ref:`layfile-parts-elements` for details.
Item parameter animation
    Items’ colour and position/size within their containing view may be animated
    according to their animation state.


.. _layfile-interact-clickable:

Clickable items
~~~~~~~~~~~~~~~

If a view item (``element`` or ``screen`` element) has ``inputtag`` and
``inputmask`` attribute values that correspond to a digital switch field in the
emulated system, clicking the element will activate the switch.  The switch
will remain active as long as the mouse button is held down and the pointer is
within the item’s current bounds.  (Note that the bounds may change depending on
the item’s animation state, see :ref:`layfile-interact-itemanim`).

The ``inputtag`` attribute specifies the tag path of an I/O port relative to the
device that caused the layout file to be loaded.  The ``inputmask`` attribute
must be an integer specifying the bits of the I/O port field that the item
should activate.  This sample shows instantiation of clickable buttons:

.. code-block:: XML

    <element ref="btn_3" inputtag="X2" inputmask="0x10">
        <bounds x="2.30" y="4.325" width="1.0" height="1.0" />
    </element>
    <element ref="btn_0" inputtag="X0" inputmask="0x20">
        <bounds x="0.725" y="5.375" width="1.0" height="1.0" />
    </element>
    <element ref="btn_rst" inputtag="RESET" inputmask="0x01">
        <bounds x="1.775" y="5.375" width="1.0" height="1.0" />
    </element>

When handling mouse input, MAME treats all layout elements as being rectangular,
and only activates the first clickable item whose area includes the location of
the mouse pointer.


.. _layfile-interact-elemstate:

Element state
~~~~~~~~~~~~~

A view item that instantiates an element (``element`` element) may supply a
state value to the element from an emulated I/O port or output.  See
:ref:`layfile-parts-elements` for details on how an element’s state affects its
appearance.

If the ``element`` element has a ``name`` attribute, the element state value
will be taken from the value of the correspondingly named emulated output.  Note
that output names are global, which can become an issue when a machine uses
multiple instances of the same type of device.  This example shows how digital
displays may be connected to emulated outputs:

.. code-block:: XML

    <element name="digit6" ref="digit"><bounds x="16" y="16" width="48" height="80" /></element>
    <element name="digit5" ref="digit"><bounds x="64" y="16" width="48" height="80" /></element>
    <element name="digit4" ref="digit"><bounds x="112" y="16" width="48" height="80" /></element>
    <element name="digit3" ref="digit"><bounds x="160" y="16" width="48" height="80" /></element>
    <element name="digit2" ref="digit"><bounds x="208" y="16" width="48" height="80" /></element>
    <element name="digit1" ref="digit"><bounds x="256" y="16" width="48" height="80" /></element>

If the ``element`` element has ``inputtag`` and ``inputmask`` attributes but
lacks a ``name`` attribute, the element state value will be taken from the value
of the corresponding I/O port, masked with the ``inputmask`` value.  The
``inputtag`` attribute specifies the tag path of an I/O port relative to the
device that caused the layout file to be loaded.  The ``inputmask`` attribute
must be an integer specifying the bits of the I/O port field to use.

If the ``element`` element has no ``inputraw`` attribute, or if the value of the
``inputraw`` attribute is ``no``, the I/O port’s value is masked with the
``inputmask`` value and XORed with the I/O port default field value.  If the
result is non-zero, the element state is 1, otherwise it’s 0.  This is often
used or provide visual feedback for clickable buttons, as values for active-high
and active-low switches are normalised.

If the ``element`` element has an ``inputraw`` attribute with the value ``yes``,
the element state will be taken from the I/O port’s value masked with the
``inputmask`` value and shifted to the right to remove trailing zeroes (for
example a mask of 0x05 will result in no shift, while a mask of 0xb0 will result
in the value being shifted four bits to the right).  This is useful for
obtaining the value of analog or positional inputs.


.. _layfile-interact-itemanim:

View item animation
~~~~~~~~~~~~~~~~~~~

Items’ colour and position/size within their containing view may be animated.
This is achieved by supplying multiple ``color`` and/or ``bounds`` child
elements with ``state`` attributes.  The ``state`` attribute of each ``color``
or ``bounds`` child element must be a non-negative integer.  Withing a view
item, no two ``color`` elements may have equal state ``state`` attributes, and
no two ``bounds`` elements may have equal ``state`` attributes.

If the item’s animation state is lower than the ``state`` value of any
``bounds`` child element, the position/size specified by the ``bounds`` child
element with the lowest ``state`` value will be used.  If the item’s
animation state is higher than the ``state`` value of any ``bounds`` child
element, the position/size specified by the ``bounds`` child element with the
highest ``state`` value will be used.  If the item’s animation state is between
the ``state`` values of two ``bounds`` child elements, the position/size will be
interpolated linearly.

If the item’s animation state is lower than the ``state`` value of any ``color``
child element, the colour specified by the ``color`` child element with the
lowest ``state`` value will be used.  If the item’s animation state is higher
than the ``state`` value of any ``color`` child element, the colour specified by
the ``color`` child element with the highest ``state`` value will be used.  If
the item’s animation state is between the ``state`` values of two ``color``
child elements, the RGBA colour components will be interpolated linearly.

An item’s animation state may be bound to an emulated output or input port by
supplying an ``animate`` child element.  If present, the ``animate`` element
must have either an ``inputtag`` attribute or a ``name`` attribute (but not
both).  If the ``animate`` child element is not present, the item’s animation
state is the same as its element state (see :ref:`layfile-interact-elemstate`).

If the ``animate`` child element is present and has an ``inputtag``
attribute, the item’s animation state will be taken from the value of the
corresponding I/O port.  The ``inputtag`` attribute specifies the tag path of an
I/O port relative to the device that caused the layout file to be loaded.  The
raw value from the input port is used, active-low switch values are not
normalised.

If the ``animate`` child element is present and has a ``name`` attribute, the
item’s animation state will be taken from the value of the correspondingly named
emulated output.  Note that output names are global, which can become an issue
when a machine uses multiple instances of the same type of device.

If the ``animate`` child element has a ``mask`` attribute, the item’s animation
state will be masked with the ``mask`` value and shifted to the right to remove
trailing zeroes (for example a mask of 0x05 will result in no shift, while a
mask of 0xb0 will result in the value being shifted four bits to the right).
Note that the ``mask`` attribute applies to output values (specified with the
``name`` attribute) as well as input port values (specified with the
``inputtag`` attribute).  If the ``mask`` attribute is present, it must be an
integer value.  If the ``mask`` attribute is not present, it is equivalent to
all 32 bits being set.

This example shows elements with independent element state and animation state,
using the animation state taken from emulated outputs to control their
position:

.. code-block:: XML

    <repeat count="5">
        <param name="x" start="10" increment="9" />
        <param name="i" start="0" increment="1" />
        <param name="mask" start="0x01" lshift="1" />

        <element name="cg_sol~i~" ref="cosmo">
            <animate name="cg_count~i~" />
            <bounds state="0" x="~x~" y="10" width="6" height="7" />
            <bounds state="255" x="~x~" y="48.5" width="6" height="7" />
        </element>

        <element ref="nothing" inputtag="FAKE1" inputmask="~mask~">
            <animate name="cg_count~i~" />
            <bounds state="0" x="~x~" y="10" width="6" height="7" />
            <bounds state="255" x="~x~" y="48.5" width="6" height="7" />
        </element>
    </repeat>

This example shows elements with independent element state and animation state,
using the animation state taken from an emulated positional input to control
their positions:

.. code-block:: XML

        <repeat count="4">
            <param name="y" start="1" increment="3" />
            <param name="n" start="0" increment="1" />
            <element ref="ledr" name="~n~.7">
                <animate inputtag="IN.1" mask="0x0f" />
                <bounds state="0" x="0" y="~y~" width="1" height="1" />
                <bounds state="11" x="16.5" y="~y~" width="1" height="1" />
            </element>
        </repeat>


.. _layfile-errors:

Error handling
--------------

* For internal (developer-supplied) layout files, errors detected by the
  ``complay.py`` script result in a build failure.
* MAME will stop loading a layout file if a syntax error is encountered.  No
  views from the layout will be available.  Examples of syntax errors include
  undefined element or group references, invalid bounds, invalid colours,
  recursively nested groups, and redefined generator parameters.
* When loading a layout file, if a view references a non-existent screen, MAME
  will print a warning message and continue.  Views referencing non-existent
  screens are considered unviable and not available to the user.


.. _layfile-autogen:

Automatically-generated views
-----------------------------

After loading internal (developer-supplied) and external (user-supplied)
layouts, MAME automatically generates views based on the machine configuration.
The following views will be automatically generated:

* If the system has no screens and no viable views were found in the internal
  and external layouts, MAME will load a view that shows the message “No screens
  attached to the system”.
* For each emulated screen, MAME will generate a view showing the screen at its
  physical aspect ratio with rotation applied.
* For each emulated screen where the configured pixel aspect ratio doesn’t match
  the physical aspect ratio, MAME will generate a view showing the screen at an
  aspect ratio that produces square pixels, with rotation applied.
* If the system has a single emulated screen, MAME will generate a view showing
  two copies of the screen image above each other with a small gap between them.
  The upper copy will be rotated by 180 degrees.  This view can be used in a
  “cocktail table” cabinet for simultaneous two-player games, or alternating
  play games that don’t automatically rotate the display for the second player.
  The screen will be displayed at its physical aspect ratio, with rotation
  applied.
* If the system has exactly two emulated screens, MAME will generate a view
  showing the second screen above the first screen with a small gap between
  them.  The second screen will be rotated by 180 degrees.  This view can be
  used to play a dual-screen two-player game on a “cocktail table” cabinet with
  a single screen.  The screens will be displayed at their physical aspect
  ratios, with rotation applied.
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


.. _layfile-complay:

Using complay.py
----------------

The MAME source contains a Python script called ``complay.py``, found in the
``scripts/build`` subdirectory.  This script is used as part of MAME’s build
process to reduce the size of data for internal layouts and convert it to a form
that can be built into the executable.  However, it can also detect many common
layout file format errors, and generally provides better error messages than
MAME does when loading a layout file.  Note that it doesn’t actually run the
whole layout engine, so it can’t detect errors like undefined element references
when parameters are used, or recursively nested groups.  The ``complay.py``
script is compatible with both Python 2.7 and Python 3 interpreters.

The ``complay.py`` script takes three parameters – an input file name, an output
file name, and a base name for variables in the output:

    **python scripts/build/complay.py** *<input>* [*<output>* [*<varname>*]]

The input file name is required.  If no output file name is supplied,
``complay.py`` will parse and check the input, reporting any errors found,
without producing output.  If no base variable name is provided, ``complay.py``
will generate one based on the input file name.  This is not guaranteed to
produce valid identifiers.  The exit status is 0 (zero) on success, 1 on an
error in the command invocation, 2 if error are found in the input file, or 3
in case of an I/O error.  If an output file name is specified, the file will be
created/overwritten on success or removed on failure.

To check a layout file for common errors, run the script with the path to the
file to check and no output file name or base variable name.  For example:

    **python scripts/build/complay.py artwork/dino/default.lay**


.. _layfile-examples:

Example layout files
--------------------

These layout files demonstrate various artwork system features.  They are all
internal layouts included in MAME.

`sstrangr.lay <https://git.redump.net/mame/tree/src/mame/layout/sstrangr.lay?h=mame0235>`_
    A simple case of using translucent colour overlays to visually separate and
    highlight elements on a black and white screen.
`seawolf.lay <https://git.redump.net/mame/tree/src/mame/layout/seawolf.lay?h=mame0235>`_
    This system uses lamps for key gameplay elements.  Blending modes are used
    for the translucent colour overlay placed over the monitor, and the lamps
    reflected in front of the monitor.  Also uses collections to allow parts of
    the layout to be disabled selectively.
`armora.lay <https://git.redump.net/mame/tree/src/mame/layout/armora.lay?h=mame0235>`_
    This game’s monitor is viewed directly through a translucent colour overlay
    rather than being reflected from inside the cabinet.  This means the overlay
    reflects ambient light as well as affecting the colour of the video image.
    The shapes on the overlay are drawn using embedded SVG images.
`tranz330.lay <https://git.redump.net/mame/tree/src/mame/layout/tranz330.lay?h=mame0235>`_
    A multi-segment alphanumeric display and keypad.  The keys are clickable,
    and provide visual feedback when pressed.
`esq2by16.lay <https://git.redump.net/mame/tree/src/mame/layout/esq2by16.lay?h=mame0235>`_
    Builds up a multi-line dot matrix character display.  Repeats are used to
    avoid repetition for the rows in a character, characters in a line, and
    lines in a page.  Group colors allow a single element to be used for all
    four display colours.
`cgang.lay <https://git.redump.net/mame/tree/src/mame/layout/cgang.lay?h=mame0235>`_
    Animates the position of element items to simulate an electromechanical
    shooting gallery game.  Also demonstrates effective use of components to
    build up complex graphics.
`unkeinv.lay <https://git.redump.net/mame/tree/src/mame/layout/unkeinv.lay?h=mame0235>`_
    Shows the position of a slider control with LEDs on it.
`md6802.lay <https://git.redump.net/mame/tree/src/mame/layout/md6802.lay?h=mame0235>`_
    Effectively using groups as a procedural programming language to build up an
    image of a trainer board.
