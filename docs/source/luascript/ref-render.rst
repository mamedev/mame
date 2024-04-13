.. _luascript-ref-render:

Lua Render System Classes
=========================

The render system is responsible for drawing what you see in MAME’s windows,
including emulated screens, artwork, and UI elements.

.. contents::
    :local:
    :depth: 1


.. _luascript-ref-renderbounds:

Render bounds
-------------

Wraps MAME’s ``render_bounds`` class, which represents a rectangle using
floating-point coordinates.

Instantiation
~~~~~~~~~~~~~

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
~~~~~~~

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
    Set the rectangle’s position and size in terms of the top left corner
    position, and the width and height.

    The arguments must all be floating-point numbers.

Properties
~~~~~~~~~~

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


.. _luascript-ref-rendercolor:

Render colour
-------------

Wraps MAME’s ``render_color`` class, which represents an ARGB (alpha, red,
green, blue) format colour.  Channels are floating-point values ranging from
zero (0, transparent alpha or colour off) to one (1, opaque or full colour
intensity).  Colour channel values are not pre-multiplied by the alpha channel
value.

Instantiation
~~~~~~~~~~~~~

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
~~~~~~~

color:set(a, r, g, b)
    Sets the colour object’s alpha, red, green and blue channel values.

    The arguments must all be floating-point numbers in the range from zero (0)
    to one (1), inclusive.

Properties
~~~~~~~~~~

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


.. _luascript-ref-palette:

Palette
-------

Wraps MAME’s ``palette_t`` class, which represents a table of colours that can
be looked up by zero-based index.  Palettes always contain additional special
entries for black and white.

Each colour has an associated contrast adjustment value.  Each adjustment group
has associated brightness and contrast adjustment values.  The palette also has
overall brightness, contrast and gamma adjustment values.

Colours are represented in alpha/red/green/blue (ARGB) format.  Channel values
range from 0 (transparent or off) to 255 (opaque or full intensity), inclusive.
Colour channel values are not pre-multiplied by the alpha value.  Channel values
are packed into the bytes of 32-bit unsigned integers, in the order alpha, red,
green, blue from most-significant to least-significant byte.

Instantiation
~~~~~~~~~~~~~

emu.palette(colors, [groups])
    Creates a palette with the specified number of colours and
    brightness/contrast adjustment groups.  The number of colour groups defaults
    to one if not specified.  Colours are initialised to black, brightness
    adjustment is initialised to 0.0, contrast adjustment initialised to 1.0,
    and gamma adjustment is initialised to 1.0.

Methods
~~~~~~~

palette:entry_color(index)
    Gets the colour at the specified zero-based index.

    Index values range from zero to the number of colours in the palette minus
    one.  Returns black if the index is greater than or equal to the number of
    colours in the palette.
palette:entry_contrast(index)
    Gets the contrast adjustment for the colour at the specified zero-based
    index.  This is a floating-point number.

    Index values range from zero to the number of colours in the palette minus
    one.  Returns 1.0 if the index is greater than or equal to the number of
    colours in the palette.
palette:entry_adjusted_color(index, [group])
    Gets a colour with brightness, contrast and gamma adjustments applied.

    If the group is specified, colour index values range from zero to the number
    of colours in the palette minus one, and group values range from zero to the
    number of adjustment groups in the palette minus one.

    If the group is not specified, index values range from zero to the number of
    colours multiplied by the number of adjustment groups plus one.  Index
    values may be calculated by multiplying the zero-based group index by the
    number of colours in the palette, and adding the zero-based colour index.
    The last two index values correspond to the special entries for black and
    white, respectively.

    Returns black if the specified combination of index and adjustment group is
    invalid.
palette:entry_set_color(index, color)
    Sets the colour at the specified zero-based index.  The colour may be
    specified as a single packed 32-bit value; or as individual red, green and
    blue channel values, in that order.

    Index values range from zero to the number of colours in the palette minus
    one.  Raises an error if the index value is invalid.
palette:entry_set_red_level(index, level)
    Sets the red channel value of the colour at the specified zero-based index.
    Other channel values are not affected.

    Index values range from zero to the number of colours in the palette minus
    one.  Raises an error if the index value is invalid.
palette:entry_set_green_level(index, level)
    Sets the green channel value of the colour at the specified zero-based
    index.  Other channel values are not affected.

    Index values range from zero to the number of colours in the palette minus
    one.  Raises an error if the index value is invalid.
palette:entry_set_blue_level(index, level)
    Sets the blue channel value of the colour at the specified zero-based index.
    Other channel values are not affected.

    Index values range from zero to the number of colours in the palette minus
    one.  Raises an error if the index value is invalid.
palette:entry_set_contrast(index, level)
    Sets the contrast adjustment value for the colour at the specified
    zero-based index.  This must be a floating-point number.

    Index values range from zero to the number of colours in the palette minus
    one.  Raises an error if the index value is invalid.
palette:group_set_brightness(group, brightness)
    Sets the brightness adjustment value for the adjustment group at the
    specified zero-based index.  This must be a floating-point number.

    Group values range from zero to the number of adjustment groups in the
    palette minus one.  Raises an error if the index value is invalid.
palette:group_set_contrast(group, contrast)
    Sets the contrast adjustment value for the adjustment group at the specified
    zero-based index.  This must be a floating-point number.

    Group values range from zero to the number of adjustment groups in the
    palette minus one.  Raises an error if the index value is invalid.

Properties
~~~~~~~~~~

palette.colors (read-only)
    The number of colour entries in each group of colours in the palette.
palette.groups (read-only)
    The number of groups of colours in the palette.
palette.max_index (read-only)
    The number of valid colour indices in the palette.
palette.black_entry (read-only)
    The index of the special entry for the colour black.
palette.white_entry (read-only)
    The index of the special entry for the colour white.
palette.brightness (write-only)
    The overall brightness adjustment for the palette.  This is a floating-point
    number.
palette.contrast (write-only)
    The overall contrast adjustment for the palette.  This is a floating-point
    number.
palette.gamma (write-only)
    The overall gamma adjustment for the palette.  This is a floating-point
    number.


.. _luascript-ref-bitmap:

Bitmap
------

Wraps implementations of MAME’s ``bitmap_t`` and ``bitmap_specific`` classes,
which represent two-dimensional bitmaps stored in row-major order.  Pixel
coordinates are zero-based, increasing to the right and down.  Several pixel
formats are supported.

Instantiation
~~~~~~~~~~~~~

emu.bitmap_ind8(palette, [width, height], [xslop, yslop])
    Creates an 8-bit indexed bitmap.  Each pixel is a zero-based, unsigned 8-bit
    index into a :ref:`palette <luascript-ref-palette>`.

    If no width and height are specified, they are assumed to be zero.  If the
    width is specified, the height must also be specified.  The X and Y slop
    values set the amount of extra storage in pixels to reserve at the
    left/right of each row and top/bottom of each column, respectively.  If an X
    slop value is specified, a Y slop value must be specified as well.  If no X
    and Y slop values are specified, they are assumed to be zero (the storage
    will be sized to fit the bitmap content).  If the width and/or height is
    less than or equal to zero, no storage will be allocated, irrespective of
    the X and Y slop values, and the width and height of the bitmap will both be
    set to zero.

    The initial clipping rectangle is set to the entirety of the bitmap.
emu.bitmap_ind16(palette, [width, height], [xslop, yslop])
    Creates a 16-bit indexed bitmap.  Each pixel is a zero-based, unsigned
    16-bit index into a :ref:`palette <luascript-ref-palette>`.

    If no width and height are specified, they are assumed to be zero.  If the
    width is specified, the height must also be specified.  The X and Y slop
    values set the amount of extra storage in pixels to reserve at the
    left/right of each row and top/bottom of each column, respectively.  If an X
    slop value is specified, a Y slop value must be specified as well.  If no X
    and Y slop values are specified, they are assumed to be zero (the storage
    will be sized to fit the bitmap content).  If the width and/or height is
    less than or equal to zero, no storage will be allocated, irrespective of
    the X and Y slop values, and the width and height of the bitmap will both be
    set to zero.

    The initial clipping rectangle is set to the entirety of the bitmap.
emu.bitmap_ind32(palette, [width, height], [xslop, yslop])
    Creates a 32-bit indexed bitmap.  Each pixel is a zero-based, unsigned
    32-bit index into a :ref:`palette <luascript-ref-palette>`.

    If no width and height are specified, they are assumed to be zero.  If the
    width is specified, the height must also be specified.  The X and Y slop
    values set the amount of extra storage in pixels to reserve at the
    left/right of each row and top/bottom of each column, respectively.  If an X
    slop value is specified, a Y slop value must be specified as well.  If no X
    and Y slop values are specified, they are assumed to be zero (the storage
    will be sized to fit the bitmap content).  If the width and/or height is
    less than or equal to zero, no storage will be allocated, irrespective of
    the X and Y slop values, and the width and height of the bitmap will both be
    set to zero.

    The initial clipping rectangle is set to the entirety of the bitmap.
emu.bitmap_ind64(palette, [width, height], [xslop, yslop])
    Creates a 64-bit indexed bitmap.  Each pixel is a zero-based, unsigned
    64-bit index into a :ref:`palette <luascript-ref-palette>`.

    If no width and height are specified, they are assumed to be zero.  If the
    width is specified, the height must also be specified.  The X and Y slop
    values set the amount of extra storage in pixels to reserve at the
    left/right of each row and top/bottom of each column, respectively.  If an X
    slop value is specified, a Y slop value must be specified as well.  If no X
    and Y slop values are specified, they are assumed to be zero (the storage
    will be sized to fit the bitmap content).  If the width and/or height is
    less than or equal to zero, no storage will be allocated, irrespective of
    the X and Y slop values, and the width and height of the bitmap will both be
    set to zero.

    The initial clipping rectangle is set to the entirety of the bitmap.
emu.bitmap_yuy16([width, height], [xslop], yslop])
    Creates a Y'CbCr format bitmap with 4:2:2 chroma subsampling (horizontal
    pairs of pixels have individual luma values but share chroma values).  Each
    pixel is a 16-bit integer value.  The most significant byte of the pixel
    value is the unsigned 8-bit Y' (luma) component of the pixel colour.  For
    each horizontal pair of pixels, the least significant byte of the first
    pixel (even zero-based X coordinate) value is the signed 8-bit Cb value for
    the pair of pixels, and the least significant byte of the second pixel (odd
    zero-based X coordinate) value is the signed 8-bit Cr value for the pair of
    pixels.

    If no width and height are specified, they are assumed to be zero.  If the
    width is specified, the height must also be specified.  The X and Y slop
    values set the amount of extra storage in pixels to reserve at the
    left/right of each row and top/bottom of each column, respectively.  If an X
    slop value is specified, a Y slop value must be specified as well.  If no X
    and Y slop values are specified, they are assumed to be zero (the storage
    will be sized to fit the bitmap content).  If the width and/or height is
    less than or equal to zero, no storage will be allocated, irrespective of
    the X and Y slop values, and the width and height of the bitmap will both be
    set to zero.

    The initial clipping rectangle is set to the entirety of the bitmap.
emu.bitmap_rgb32([width, height], [xslop, yslop])
    Creates an RGB format bitmap with no alpha (transparency) channel.  Each
    pixel is represented by a 32-bit integer value.  The most significant byte
    of the pixel value is ignored.  The remaining three bytes, from most
    significant to least significant, are the unsigned 8-bit unsigned red, green
    and blue channel values (larger values correspond to higher intensities).

    If no width and height are specified, they are assumed to be zero.  If the
    width is specified, the height must also be specified.  The X and Y slop
    values set the amount of extra storage in pixels to reserve at the
    left/right of each row and top/bottom of each column, respectively.  If an X
    slop value is specified, a Y slop value must be specified as well.  If no X
    and Y slop values are specified, they are assumed to be zero (the storage
    will be sized to fit the bitmap content).  If the width and/or height is
    less than or equal to zero, no storage will be allocated, irrespective of
    the X and Y slop values, and the width and height of the bitmap will both be
    set to zero.

    The initial clipping rectangle is set to the entirety of the bitmap.
emu.bitmap_argb32([width, height], [xslop, yslop])
    Creates an ARGB format bitmap.  Each pixel is represented by a 32-bit
    integer value.  The most significant byte of the pixel is the 8-bit unsigned
    alpha (transparency) channel value (smaller values are more transparent).
    The remaining three bytes, from most significant to least significant, are
    the unsigned 8-bit unsigned red, green and blue channel values (larger
    values correspond to higher intensities).  Colour channel values are not
    pre-multiplied by the alpha channel value.

    If no width and height are specified, they are assumed to be zero.  If the
    width is specified, the height must also be specified.  The X and Y slop
    values set the amount of extra storage in pixels to reserve at the
    left/right of each row and top/bottom of each column, respectively.  If an X
    slop value is specified, a Y slop value must be specified as well.  If no X
    and Y slop values are specified, they are assumed to be zero (the storage
    will be sized to fit the bitmap content).  If the width and/or height is
    less than or equal to zero, no storage will be allocated, irrespective of
    the X and Y slop values, and the width and height of the bitmap will both be
    set to zero.

    The initial clipping rectangle is set to the entirety of the bitmap.
emu.bitmap_ind8(source, [x0, y0, x1, y1])
    Creates an 8-bit indexed bitmap representing a view of a portion of an
    existing bitmap.  The initial clipping rectangle is set to the bounds of the
    view.  The source bitmap will be locked, preventing resizing and
    reallocation.

    If no coordinates are specified, the new bitmap will represent a view of the
    source bitmap’s current clipping rectangle.  If coordinates are specified,
    the new bitmap will represent a view of the rectangle with top left corner
    at (x0, y0) and bottom right corner at (x1, y1) in the source bitmap.
    Coordinates are in units of pixels.  The bottom right coordinates are
    inclusive.

    The source bitmap must be owned by the Lua script and must use the 8-bit
    indexed format.  Raises an error if coordinates are specified representing a
    rectangle not fully contained within the source bitmap’s clipping rectangle.
emu.bitmap_ind16(source, [x0, y0, x1, y1])
    Creates a 16-bit indexed bitmap representing a view of a portion of an
    existing bitmap.  The initial clipping rectangle is set to the bounds of the
    view.  The source bitmap will be locked, preventing resizing and
    reallocation.

    If no coordinates are specified, the new bitmap will represent a view of the
    source bitmap’s current clipping rectangle.  If coordinates are specified,
    the new bitmap will represent a view of the rectangle with top left corner
    at (x0, y0) and bottom right corner at (x1, y1) in the source bitmap.
    Coordinates are in units of pixels.  The bottom right coordinates are
    inclusive.

    The source bitmap must be owned by the Lua script and must use the 16-bit
    indexed format.  Raises an error if coordinates are specified representing a
    rectangle not fully contained within the source bitmap’s clipping rectangle.
emu.bitmap_ind32(source, [x0, y0, x1, y1])
    Creates a 32-bit indexed bitmap representing a view of a portion of an
    existing bitmap.  The initial clipping rectangle is set to the bounds of the
    view.  The source bitmap will be locked, preventing resizing and
    reallocation.

    If no coordinates are specified, the new bitmap will represent a view of the
    source bitmap’s current clipping rectangle.  If coordinates are specified,
    the new bitmap will represent a view of the rectangle with top left corner
    at (x0, y0) and bottom right corner at (x1, y1) in the source bitmap.
    Coordinates are in units of pixels.  The bottom right coordinates are
    inclusive.

    The source bitmap must be owned by the Lua script and must use the 32-bit
    indexed format.  Raises an error if coordinates are specified representing a
    rectangle not fully contained within the source bitmap’s clipping rectangle.
emu.bitmap_ind64(source, [x0, y0, x1, y1])
    Creates a 64-bit indexed bitmap representing a view of a portion of an
    existing bitmap.  The initial clipping rectangle is set to the bounds of the
    view.  The source bitmap will be locked, preventing resizing and
    reallocation.

    If no coordinates are specified, the new bitmap will represent a view of the
    source bitmap’s current clipping rectangle.  If coordinates are specified,
    the new bitmap will represent a view of the rectangle with top left corner
    at (x0, y0) and bottom right corner at (x1, y1) in the source bitmap.
    Coordinates are in units of pixels.  The bottom right coordinates are
    inclusive.

    The source bitmap must be owned by the Lua script and must use the 64-bit
    indexed format.  Raises an error if coordinates are specified representing a
    rectangle not fully contained within the source bitmap’s clipping rectangle.
emu.bitmap_yuy16(source, [x0, y0, x1, y1])
    Creates a Y'CbCr format bitmap with 4:2:2 chroma subsampling representing a
    view of a portion of an existing bitmap.  The initial clipping rectangle is
    set to the bounds of the view.  The source bitmap will be locked, preventing
    resizing and reallocation.

    If no coordinates are specified, the new bitmap will represent a view of the
    source bitmap’s current clipping rectangle.  If coordinates are specified,
    the new bitmap will represent a view of the rectangle with top left corner
    at (x0, y0) and bottom right corner at (x1, y1) in the source bitmap.
    Coordinates are in units of pixels.  The bottom right coordinates are
    inclusive.

    The source bitmap must be owned by the Lua script and must use the Y'CbCr
    format.  Raises an error if coordinates are specified representing a
    rectangle not fully contained within the source bitmap’s clipping rectangle.
emu.bitmap_rgb32(source, [x0, y0, x1, y1])
    Creates an RGB format bitmap representing a view of a portion of an existing
    bitmap.  The initial clipping rectangle is set to the bounds of the view.
    The source bitmap will be locked, preventing resizing and reallocation.

    If no coordinates are specified, the new bitmap will represent a view of the
    source bitmap’s current clipping rectangle.  If coordinates are specified,
    the new bitmap will represent a view of the rectangle with top left corner
    at (x0, y0) and bottom right corner at (x1, y1) in the source bitmap.
    Coordinates are in units of pixels.  The bottom right coordinates are
    inclusive.

    The source bitmap must be owned by the Lua script and must use the RGB
    format.  Raises an error if coordinates are specified representing a
    rectangle not fully contained within the source bitmap’s clipping rectangle.
emu.bitmap_argb32(source, [x0, y0, x1, y1])
    Creates an ARGB format bitmap representing a view of a portion of an
    existing bitmap.  The initial clipping rectangle is set to the bounds of the
    view.  The source bitmap will be locked, preventing resizing and
    reallocation.

    If no coordinates are specified, the new bitmap will represent a view of the
    source bitmap’s current clipping rectangle.  If coordinates are specified,
    the new bitmap will represent a view of the rectangle with top left corner
    at (x0, y0) and bottom right corner at (x1, y1) in the source bitmap.
    Coordinates are in units of pixels.  The bottom right coordinates are
    inclusive.

    The source bitmap must be owned by the Lua script and must use the ARGB
    format.  Raises an error if coordinates are specified representing a
    rectangle not fully contained within the source bitmap’s clipping rectangle.
emu.bitmap_argb32.load(data)
    Creates an ARGB format bitmap from data in PNG, JPEG (JFIF/EXIF) or
    Microsoft DIB (BMP) format.  Raises an error if the data invalid or not a
    supported format.

Methods
~~~~~~~

bitmap:cliprect()
    Returns the left, top, right and bottom coordinates of the bitmap’s clipping
    rectangle.  Coordinates are in units of pixels; the bottom and right
    coordinates are inclusive.
bitmap:reset()
    Sets the width and height to zero, and frees the pixel storage if the bitmap
    owns its own storage, or releases the source bitmap if the it represents a
    view of another bitmap.

    The bitmap must be owned by the Lua script.  Raises an error if the bitmap’s
    storage is referenced by another bitmap or a :ref:`texture
    <luascript-ref-rendertexture>`.
bitmap:allocate(width, height, [xslop, yslop])
    Reallocates storage for the bitmap, sets its width and height, and sets the
    clipping rectangle to the entirety of the bitmap.  If the bitmap already
    owns allocated storage, it will always be freed and reallocated; if the
    bitmap represents a view of another bitmap, the source bitmap will be
    released.  The storage will be filled with pixel value zero.

    The X and Y slop values set the amount of extra storage in pixels to reserve
    at the left/right of each row and top/bottom of each column, respectively.
    If an X slop value is specified, a Y slop value must be specified as well.
    If no X and Y slop values are specified, they are assumed to be zero (the
    storage will be sized to fit the bitmap content).  If the width and/or
    height is less than or equal to zero, no storage will be allocated,
    irrespective of the X and Y slop values, and the width and height of the
    bitmap will both be set to zero.

    The bitmap must be owned by the Lua script.  Raises an error if the bitmap’s
    storage is referenced by another bitmap or a :ref:`texture
    <luascript-ref-rendertexture>`.
bitmap:resize(width, height, [xslop, yslop])
    Changes the width and height, and sets the clipping rectangle to the
    entirety of the bitmap.

    The X and Y slop values set the amount of extra storage in pixels to reserve
    at the left/right of each row and top/bottom of each column, respectively.
    If an X slop value is specified, a Y slop value must be specified as well.
    If no X and Y slop values are specified, they are assumed to be zero (rows
    will be stored contiguously, and the top row will be placed at the beginning
    of the bitmap’s storage).

    If the bitmap already owns allocated storage and it is large enough for the
    updated size, it will be used without being freed; if it is too small for
    the updated size, it will always be freed and reallocated.  If the bitmap
    represents a view of another bitmap, the source bitmap will be released.  If
    storage is allocated, it will be filled with pixel value zero (if existing
    storage is used, its contents will not be changed).

    Raises an error if the bitmap’s storage is referenced by another bitmap or a
    :ref:`texture <luascript-ref-rendertexture>`.
bitmap:wrap(source, [x0, y0, x1, y1])
    Makes the bitmap represent a view of a portion of another bitmap and sets
    the clipping rectangle to the bounds of the view.

    If no coordinates are specified, the target bitmap will represent a view of
    the source bitmap’s current clipping rectangle.  If coordinates are
    specified, the target bitmap will represent a view of the rectangle with top
    left corner at (x0, y0) and bottom right corner at (x1, y1) in the source
    bitmap.  Coordinates are in units of pixels.  The bottom right coordinates
    are inclusive.

    The source bitmap will be locked, preventing resizing and reallocation.  If
    the target bitmap owns allocated storage, it will be freed; if it represents
    a view of another bitmap, the current source bitmap will be released.

    The source and target bitmaps must both be owned by the Lua script and must
    use the same pixel format.  Raises an error if coordinates are specified
    representing a rectangle not fully contained within the source bitmap’s
    clipping rectangle; if the bitmap’s storage is referenced by another bitmap
    or a :ref:`texture <luascript-ref-rendertexture>`; or if the source and
    target are the same bitmap.
bitmap:pix(x, y)
    Returns the colour value of the pixel at the specified location.
    Coordinates are zero-based in units of pixels.
bitmap:pixels([x0, y0, x1, y1])
    Returns the pixels, width and height of the portion of the bitmap with top
    left corner at (x0, y0) and bottom right corner at (x1, y1).  Coordinates
    are in units of pixels.  The bottom right coordinates are inclusive.  If
    coordinates are not specified, the bitmap’s clipping rectangle is used.

    Pixels are returned packed into a binary string in host Endian order.
    Pixels are organised in row-major order, from left to right then top to
    bottom.  The size and format of the pixel values depends on the format of
    the bitmap.  Raises an error if coordinates are specified representing a
    rectangle not fully contained within the bitmap’s clipping rectangle.
bitmap:fill(color, [x0, y0, x1, y1])
    Fills a portion of the bitmap with the specified colour value.  If
    coordinates are not specified, the clipping rectangle is filled; if
    coordinates are specified, the intersection of the clipping rectangle and
    the rectangle with top left corner at (x0, y0) and bottom right corner at
    (x1, y1) is filled.  Coordinates are in units of pixels.  The bottom right
    coordinates are inclusive.
bitmap:plot(x, y, color)
    Sets the colour value of the pixel at the specified location if it is within
    the clipping rectangle.  Coordinates are zero-based in units of pixels.
bitmap:plot_box(x, y, width, height, color)
    Fills the intersection of the clipping rectangle and the rectangle with top
    left (x, y) and the specified height and width with the specified colour
    value.  Coordinates and dimensions are in units of pixels.
bitmap:resample(dest, [color])
    Copies the bitmap into the destination bitmap, scaling to fill the
    destination bitmap and using a re-sampling filter.  Only ARGB format source
    and destination bitmaps are supported.  The source pixel values will be
    multiplied by the colour if it is supplied.  It must be a
    :ref:`render colour <luascript-ref-rendercolor>`.

Properties
~~~~~~~~~~

bitmap.palette (read/write)
    The :ref:`palette <luascript-ref-palette>` used to translate pixel
    values to colours.  Only applicable for bitmaps that use indexed pixel
    formats.
bitmap.width (read-only)
    Width of the bitmap in pixels.
bitmap.height (read-only)
    Height of the bitmap in pixels.
bitmap.rowpixels (read-only)
    Row stride of the bitmap’s storage in pixels.  That is, the difference in
    pixel offsets of the pixels at the same horizontal location in consecutive
    rows.  May be greater than the width.
bitmap.rowbytes (read-only)
    Row stride of the bitmap’s storage in bytes.  That is, the difference in
    byte addresses of the pixels at the same horizontal location in consecutive
    rows.
bitmap.bpp (read-only)
    Size of the type used to represent pixels in the bitmap in bits (may be
    larger than the number of significant bits).
bitmap.valid (read-only)
    A Boolean indicating whether the bitmap has storage available (may be false
    for empty bitmaps).
bitmap.locked (read-only)
    A Boolean indicating whether the bitmap’s storage is referenced by another
    bitmap or a :ref:`texture <luascript-ref-rendertexture>`.


.. _luascript-ref-rendertexture:

Render texture
--------------

Wraps MAME’s ``render_texture`` class, representing a texture that cam be drawn
in a :ref:`render container <luascript-ref-rendercontainer>`.  Render textures
must be freed before the emulation session ends.

Instantiation
~~~~~~~~~~~~~

manager.machine.render:texture_alloc(bitmap)
    Creates a render texture based on a :ref:`bitmap
    <luascript-ref-bitmap>`.  The bitmap must be owned by the Lua script, and
    must use the Y'CbCr, RGB or ARGB format.  The bitmap’s storage will be
    locked, preventing resizing and reallocation.

Methods
~~~~~~~

texture:free()
    Frees the texture.  The storage of the underlying bitmap will be released.

Properties
~~~~~~~~~~

texture.valid (read-only)
    A Boolean indicating whether the texture is valid (false if the texture has
    been freed).


.. _luascript-ref-renderman:

Render manager
--------------

Wraps MAME’s ``render_manager`` class, responsible for managing render targets
and textures.

Instantiation
~~~~~~~~~~~~~

manager.machine.render
    Gets the global render manager instance for the emulation session.

Methods
~~~~~~~

render:texture_alloc(bitmap)
    Creates a :ref:`render texture <luascript-ref-rendertexture>` based on a
    :ref:`bitmap <luascript-ref-bitmap>`.  The bitmap must be owned by the Lua
    script, and must use the Y'CbCr, RGB or ARGB pixel format.  The bitmap’s
    storage will be locked, preventing resizing and reallocation.  Render
    textures must be freed before the emulation session ends.

Properties
~~~~~~~~~~

render.max_update_rate (read-only)
    The maximum update rate in Hertz.  This is a floating-point number.
render.ui_target (read-only)
    The :ref:`render target <luascript-ref-rendertarget>` used to draw the user
    interface (including menus, sliders and pop-up messages).  This is usually
    the first host window or screen.
render.ui_container (read-only)
    The :ref:`render container <luascript-ref-rendercontainer>` used for drawing
    the user interface.
render.targets[] (read-only)
    The list of render targets, including output windows and screens, as well as
    hidden render targets used for things like rendering screenshots.  Uses
    1-based integer indices.  The index operator and the ``at`` method have O(n)
    complexity.


.. _luascript-ref-rendertarget:

Render target
-------------

Wrap’s MAME’s ``render_target`` class, which represents a video output channel.
This could be a host window or screen, or a hidden target used for rendering
screenshots.

Instantiation
~~~~~~~~~~~~~

manager.machine.render.targets[index]
    Gets a render target by index.
manager.machine.render.ui_target
    Gets the render target used to display the user interface (including menus,
    sliders and pop-up messages).  This is usually the first host window or
    screen.
manager.machine.video.snapshot_target
    Gets the render target used to produce snapshots and video recordings.

Properties
~~~~~~~~~~

target.ui_container (read-only)
    The :ref:`render container <luascript-ref-rendercontainer>` for drawing user
    interface elements over this render target, or ``nil`` for hidden render
    targets (targets that are not shown to the user directly).
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
    :ref:`layout view <luascript-ref-renderlayview>` object.
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


.. _luascript-ref-rendercontainer:

Render container
----------------

Wraps MAME’s ``render_container`` class.

Instantiation
~~~~~~~~~~~~~

manager.machine.render.ui_container
    Gets the render container used to draw the user interface, including menus,
    sliders and pop-up messages.
manager.machine.render.targets[index].ui_container
    Gets the render container used to draw user interface elements over a
    particular render target.
manager.machine.screens[tag].container
    Gets the render container used to draw a given screen.

Methods
~~~~~~~

container:draw_box(left, top, right, bottom, [line], [fill])
    Draws an outlined rectangle with edges at the specified positions.

    Coordinates are floating-point numbers in the range of 0 (zero) to 1 (one),
    with (0, 0) at the top left and (1, 1) at the bottom right of the window or
    the screen that shows the user interface.  Note that the aspect ratio is
    usually not square.  Coordinates are limited to the window or screen area.

    The fill and line colours are in alpha/red/green/blue (ARGB) format.
    Channel values are in the range 0 (transparent or off) to 255 (opaque or
    full intensity), inclusive.  Colour channel values are not pre-multiplied by
    the alpha value.  The channel values must be packed into the bytes of a
    32-bit unsigned integer, in the order alpha, red, green, blue from
    most-significant to least-significant byte.  If the line colour is not
    provided, the UI text colour is used; if the fill colour is not provided,
    the UI background colour is used.
container:draw_line(x0, y0, x1, y1, [color])
    Draws a line from (x0, y0) to (x1, y1).

    Coordinates are floating-point numbers in the range of 0 (zero) to 1 (one),
    with (0, 0) at the top left and (1, 1) at the bottom right of the window or
    the screen that shows the user interface.  Note that the aspect ratio is
    usually not square.  Coordinates are limited to the window or screen area.

    The line colour is in alpha/red/green/blue (ARGB) format.  Channel values
    are in the range 0 (transparent or off) to 255 (opaque or full intensity),
    inclusive.  Colour channel values are not pre-multiplied by the alpha value.
    The channel values must be packed into the bytes of a 32-bit unsigned
    integer, in the order alpha, red, green, blue from most-significant to
    least-significant byte.  If the line colour is not provided, the UI text
    colour is used.
container:draw_quad(texture, x0, y0, x1, y1, [color])
    Draws a textured rectangle with top left corner at (x0, y0) and bottom right
    corner at (x1, y1).  If a colour is specified, the ARGB channel values of
    the texture’s pixels are multiplied by the corresponding values of the
    specified colour.

    Coordinates are floating-point numbers in the range of 0 (zero) to 1 (one),
    with (0, 0) at the top left and (1, 1) at the bottom right of the window or
    the screen that shows the user interface.  Note that the aspect ratio is
    usually not square.  If the rectangle extends beyond the container’s bounds,
    it will be cropped.

    The colour is in alpha/red/green/blue (ARGB) format.  Channel values are in
    the range 0 (transparent or off) to 255 (opaque or full intensity),
    inclusive.  Colour channel values are not pre-multiplied by the alpha value.
    The channel values must be packed into the bytes of a 32-bit unsigned
    integer, in the order alpha, red, green, blue from most-significant to
    least-significant byte.
container:draw_text(x|justify, y, text, [foreground], [background])
    Draws text at the specified position.  If the screen is rotated the text
    will be rotated.

    If the first argument is a number, the text will be left-aligned at this X
    coordinate.  If the first argument is a string, it must be ``"left"``,
    ``"center"`` or ``"right"`` to draw the text left-aligned at the
    left edge of the window or screen, horizontally centred in the window or
    screen, or right-aligned at the right edge of the window or screen,
    respectively.  The second argument specifies the Y coordinate of the maximum
    ascent of the text.

    Coordinates are floating-point numbers in the range of 0 (zero) to 1 (one),
    with (0, 0) at the top left and (1, 1) at the bottom right of the window or
    the screen that shows the user interface.  Note that the aspect ratio is
    usually not square.  Coordinates are limited to the window or screen area.

    The foreground and background colours are in alpha/red/green/blue (ARGB)
    format.  Channel values are in the range 0 (transparent or off) to 255
    (opaque or full intensity), inclusive.  Colour channel values are not
    pre-multiplied by the alpha value.  The channel values must be packed into
    the bytes of a 32-bit unsigned integer, in the order alpha, red, green, blue
    from most-significant to least-significant byte.  If the foreground colour
    is not provided, the UI text colour is used; if the background colour is not
    provided, it is fully transparent.

Properties
~~~~~~~~~~

container.user_settings (read/write)
    The container’s :ref:`user settings <luascript-ref-rendercntnrsettings>`.
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
    corresponds to the X size of the container.
container.yoffset (read/write)
    The container’s Y offset.  This is a floating-point number where one (1)
    corresponds to the Y size of the container.
container.is_empty (read-only)
    A Boolean indicating whether the container has no items.


.. _luascript-ref-rendercntnrsettings:

Container user settings
-----------------------

Wraps MAME’s ``render_container::user_settings`` class, representing image
adjustments applied to a
:ref:`render container <luascript-ref-rendercontainer>`.

Instantiation
~~~~~~~~~~~~~

manager.machine.screens[tag].container
    Gets the current render container user settings for a given emulated screen.

Properties
~~~~~~~~~~

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


.. _luascript-ref-renderlayfile:

Layout file
-----------

Wraps MAME’s ``layout_file`` class, representing the views loaded from a layout
file for use by a render target.  Note that layout file callbacks are not run as
coroutines.

Instantiation
~~~~~~~~~~~~~

A layout file object is supplied to its layout script in the ``file`` variable.
Layout file objects are not instantiated directly from Lua scripts.

Methods
~~~~~~~

layout:set_resolve_tags_callback(cb)
    Set a function to perform additional tasks after the emulated machine has
    finished starting, tags in the layout views have been resolved, and the
    default view item handlers have been set up.  The function must accept no
    arguments.

    Call with ``nil`` to remove the callback.

Properties
~~~~~~~~~~

layout.device (read-only)
    The device that caused the layout file to be loaded.  Usually the root
    machine device for external layouts.
layout.elements[] (read-only)
    The :ref:`elements <luascript-ref-renderlayelem>` created from the layout
    file.  Elements are indexed by name (i.e. the value of the ``name``
    attribute).  The index get method has O(1) complexity, and the ``at`` and
    ``index_of`` methods have O(n) complexity.
layout.views[] (read-only)
    The :ref:`views <luascript-ref-renderlayview>` created from the layout file.
    Views are indexed by unqualified name (i.e. the value of the ``name``
    attribute).  Views are ordered how they appear in the layout file when
    iterating or using the ``at`` method.  The index get, ``at`` and
    ``index_of`` methods have O(n) complexity.

    Note that some views in the XML file may not be created.  For example views
    that reference screens provided by slot card devices will not be created if
    said slot card devices are not present in the emulated system.


.. _luascript-ref-renderlayview:

Layout view
-----------

Wraps MAME’s ``layout_view`` class, representing a view that can be displayed in
a render target.  Views are created from XML layout files, which may be loaded
from external artwork, internal to MAME, or automatically generated based on the
screens in the emulated system.  Note that layout view callbacks are not run as
coroutines.

Instantiation
~~~~~~~~~~~~~

manager.machine.render.targets[index].current_view
    Gets the currently selected view for a given render target.
file.views[name]
    Gets the view with the specified name from a
    :ref:`layout file <luascript-ref-renderlayfile>`.  This is how layout
    scripts generally obtain views.

Methods
~~~~~~~

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
view:set_pointer_updated_callback(cb)
    Set a function to receive notifications when a pointer enters, moves or
    changes button states over the view.  The function must accept nine
    arguments:

    * The pointer type (``mouse``, ``pen``, ``touch`` or ``unknown``).
    * The pointer ID (a non-negative integer that will not change for the
      lifetime of a pointer).
    * The device ID for grouping pointers to recognise multi-touch gestures
      (non-negative integer).
    * Horizontal position in layout coordinates.
    * Vertical position in layout coordinates.
    * A bit mask representing the currently pressed buttons.
    * A bit mask representing the buttons that were pressed in this update.
    * A bit mask representing the buttons that were released in this update.
    * The click count (positive for multi-click actions, or negative if a click
      is turned into a hold or drag).

    Call with ``nil`` to remove the callback.
view:set_pointer_left_callback(cb)
    Set a function to receive notifications when a pointer leaves the view
    normally.  The function must accept seven arguments:

    * The pointer type (``mouse``, ``pen``, ``touch`` or ``unknown``).
    * The pointer ID (a non-negative integer that will not change for the
      lifetime of a pointer).  The ID may be reused for a new pointer after
      receiving this notification.
    * The device ID for grouping pointers to recognise multi-touch gestures
      (non-negative integer).
    * Horizontal position in layout coordinates.
    * Vertical position in layout coordinates.
    * A bit mask representing the buttons that were released in this update.
    * The click count (positive for multi-click actions, or negative if a click
      is turned into a hold or drag).

    Call with ``nil`` to remove the callback.
view:set_pointer_aborted_callback(cb)
    Set a function to receive notifications when a pointer leaves the view
    abnormally.  The function must accept seven arguments:

    * The pointer type (``mouse``, ``pen``, ``touch`` or ``unknown``).
    * The pointer ID (a non-negative integer that will not change for the
      lifetime of a pointer).  The ID may be reused for a new pointer after
      receiving this notification.
    * The device ID for grouping pointers to recognise multi-touch gestures
      (non-negative integer).
    * Horizontal position in layout coordinates.
    * Vertical position in layout coordinates.
    * A bit mask representing the buttons that were released in this update.
    * The click count (positive for multi-click actions, or negative if a click
      is turned into a hold or drag).

    Call with ``nil`` to remove the callback.
view:set_forget_pointers_callback(cb)
    Set a function to receive notifications when the view should stop processing
    pointer input.  The function must accept no arguments.  Call with ``nil`` to
    remove the callback.

    This can happen in a number of situations, including the view configuration
    changing or a menu taking over input handling.

Properties
~~~~~~~~~~

view.items[] (read-only)
    The screen and layout element :ref:`items <luascript-ref-renderlayitem>` in
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
    A :ref:`render bounds <luascript-ref-renderbounds>` object representing the
    effective bounds of the view in its current configuration.  The coordinates
    are in view units, which are arbitrary but assumed to have square aspect
    ratio.
view.has_art (read-only)
    A Boolean indicating whether the view has any non-screen items, including
    items that are not visible because the user has hidden the item collection
    that they belong to.
view.show_pointers (read/write)
    A Boolean that sets whether mouse and pen pointers should be displayed for
    the view.
view.hide_inactive_pointers (read/write)
    A Boolean that sets whether mouse pointers for the view should be hidden
    after a period of inactivity.


.. _luascript-ref-renderlayitem:

Layout view item
----------------

Wraps MAME’s ``layout_view_item`` class, representing an item in a :ref:`layout
view <luascript-ref-renderlayview>`.  An item is drawn as a rectangular textured
surface.  The texture is supplied by an emulated screen or a layout element.
Note that layout view item callbacks are not run as coroutines.

Instantiation
~~~~~~~~~~~~~

layout.views[name].items[id]
    Get a view item by ID.  The item must have an ``id`` attribute in the XML
    layout file to be looked up by ID.

Methods
~~~~~~~

item:set_state(state)
    Set the value used as the element state and animation state in the absence
    of bindings.  The argument must be an integer.
item:set_element_state_callback(cb)
    Set a function to call to obtain the element state for the item.  The
    function must accept no arguments and return an integer.  Call with ``nil``
    to restore the default element state callback (based on bindings in the XML
    layout file).

    Note that the function must not access the item’s ``element_state``
    property, as this will result in infinite recursion.

    This callback will not be used to obtain the animation state for the item,
    even if the item lacks explicit animation state bindings in the XML layout
    file.
item:set_animation_state_callback(cb)
    Set a function to call to obtain the animation state for the item.  The
    function must accept no arguments and return an integer.  Call with ``nil``
    to restore the default animation state callback (based on bindings in the
    XML layout file).

    Note that the function must not access the item’s ``animation_state``
    property, as this will result in infinite recursion.
item:set_bounds_callback(cb)
    Set a function to call to obtain the bounds for the item.  The function must
    accept no arguments and return a
    :ref:`render bounds <luascript-ref-renderbounds>` object in render target
    coordinates.  Call with ``nil`` to restore the default bounds callback
    (based on the item’s animation state and ``bounds`` child elements in the
    XML layout file).

    Note that the function must not access the item’s ``bounds`` property, as
    this will result in infinite recursion.
item:set_color_callback(cb)
    Set a function to call to obtain the multiplier colour for the item.  The
    function must accept no arguments and return a
    :ref:`render colour <luascript-ref-rendercolor>` object.  Call with ``nil``
    to restore the default colour callback (based on the item’s animation state
    and ``color`` child elements in the XML layout file).

    Note that the function must not access the item’s ``color`` property, as
    this will result in infinite recursion.
item:set_scroll_size_x_callback(cb)
    Set a function to call to obtain the size of the horizontal scroll window as
    a proportion of the associated element’s width.  The function must accept no
    arguments and return a floating-point value.  Call with ``nil`` to restore
    the default horizontal scroll window size callback (based on the ``xscroll``
    child element in the XML layout file).

    Note that the function must not access the item’s ``scroll_size_x``
    property, as this will result in infinite recursion.
item:set_scroll_size_y_callback(cb)
    Set a function to call to obtain the size of the vertical scroll window as a
    proportion of the associated element’s height.  The function must accept no
    arguments and return a floating-point value.  Call with ``nil`` to restore
    the default vertical scroll window size callback (based on the ``yscroll``
    child element in the XML layout file).

    Note that the function must not access the item’s ``scroll_size_y``
    property, as this will result in infinite recursion.
item:set_scroll_pos_x_callback(cb)
    Set a function to call to obtain the horizontal scroll position.  A value of
    zero places the horizontal scroll window at the left edge of the associated
    element.  If the item does not wrap horizontally, a value of 1.0 places the
    horizontal scroll window at the right edge of the associated element; if the
    item wraps horizontally, a value of 1.0 corresponds to wrapping back to the
    left edge of the associated element.  The function must accept no arguments
    and return a floating-point value.  Call with ``nil`` to restore the default
    horizontal scroll position callback (based on bindings in the ``xscroll``
    child element in the XML layout file).

    Note that the function must not access the item’s ``scroll_pos_x`` property,
    as this will result in infinite recursion.
item:set_scroll_pos_y_callback(cb)
    Set a function to call to obtain the vertical scroll position.  A value of
    zero places the vertical scroll window at the top edge of the associated
    element.  If the item does not wrap vertically, a value of 1.0 places the
    vertical scroll window at the bottom edge of the associated element; if the
    item wraps vertically, a value of 1.0 corresponds to wrapping back to the
    left edge of the associated element.  The function must accept no arguments
    and return a floating-point value.  Call with ``nil`` to restore the default
    vertical scroll position callback (based on bindings in the ``yscroll``
    child element in the XML layout file).

    Note that the function must not access the item’s ``scroll_pos_y`` property,
    as this will result in infinite recursion.

Properties
~~~~~~~~~~

item.id (read-only)
    Get the optional item identifier.  This is the value of the ``id`` attribute
    in the XML layout file if present, or ``nil``.
item.element (read-only)
    The :ref:`element <luascript-ref-renderlayelem>` used to draw the item, or
    ``nil`` for screen items.
item.bounds_animated (read-only)
    A Boolean indicating whether the item’s bounds depend on its animation
    state.
item.color_animated (read-only)
    A Boolean indicating whether the item’s colour depends on its animation
    state.
item.bounds (read-only)
    The item’s bounds for the current state.  This is a
    :ref:`render bounds <luascript-ref-renderbounds>` object in render target
    coordinates.
item.color (read-only)
    The item’s colour for the current state.  The colour of the screen or
    element texture is multiplied by this colour.  This is a
    :ref:`render colour <luascript-ref-rendercolor>` object.
item.scroll_wrap_x (read-only)
    A Boolean indicating whether the item wraps horizontally.
item.scroll_wrap_y (read-only)
    A Boolean indicating whether the item wraps vertically.
item.scroll_size_x (read/write)
    Get the item’s horizontal scroll window size for the current state, or set
    the horizontal scroll window size to use in the absence of bindings.  This
    is a floating-point value representing a proportion of the associated
    element’s width.
item.scroll_size_y (read/write)
    Get the item’s vertical scroll window size for the current state, or set the
    vertical scroll window size to use in the absence of bindings.  This is a
    floating-point value representing a proportion of the associated element’s
    height.
item.scroll_pos_x (read/write)
    Get the item’s horizontal scroll position for the current state, or set the
    horizontal scroll position size to use in the absence of bindings.  This is
    a floating-point value.
item.scroll_pos_y (read/write)
    Get the item’s vertical scroll position for the current state, or set the
    vertical position size to use in the absence of bindings.  This is a
    floating-point value.
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


.. _luascript-ref-renderlayelem:

Layout element
--------------

Wraps MAME’s ``layout_element`` class, representing a visual element that can be
drawn in a :ref:`layout view <luascript-ref-renderlayview>`.  Elements are
created from XML layout files, which may be loaded from external artwork or
internal to MAME.  Note that layout element callbacks are not run as coroutines.

Instantiation
~~~~~~~~~~~~~

layout.elements[name]
    Gets a layout element by name.
layout.views[name].items[id].element
    Gets the layout element used to draw a
    :ref:`view item <luascript-ref-renderlayitem>`.

Methods
~~~~~~~

element:invalidate()
    Invalidate all cached textures for the element, ensuring it will be redrawn
    when the next video frame is drawn.
element.set_draw_callback(cb)
    Set a function to call the perform additional drawing after the element’s
    components have been drawn.  The function is passed two arguments: the
    element state (an integer) and the 32-bit ARGB
    :ref:`bitmap <luascript-ref-bitmap>` at the required size.  The function
    must not attempt to resize the bitmap.  Call with ``nil`` to remove the
    callback.

Properties
~~~~~~~~~~

element.default_state (read-only)
    The integer default state for the element if set or ``nil``.
