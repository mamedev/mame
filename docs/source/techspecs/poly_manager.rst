Software 3D Rendering in MAME
=============================

.. contents:: :local:


Background
----------

Beginning in the late 1980s, many arcade games began incorporating hardware-rendered
3D graphics into their video. These 3D graphics are typically rendered from low-level
primitives into a frame buffer (usually double- or triple-buffered), then perhaps
combined with traditional tilemaps or sprites, before being presented to the player.

When it comes to emulating 3D games, there are two general approaches. The first
approach is to leverage modern 3D hardware by mapping the low-level primitives onto
modern equivalents. For a cross-platform emulator like MAME, this requires having an
API that is flexible enough to describe the primitives and all their associated
behaviors with high accuracy. It also requires the emulator to be able to read back
from the rendered frame buffer (since many games do this) and combine it with other
elements, in a way that is properly synchronized with background rendering.

The alternative approach is to render the low-level primitives directly in software.
This has the advantage of being able to achieve pretty much any behavior exhibited by
the original hardware, but at the cost of speed. In MAME, since all emulation happens
on one thread, this is particularly painful. However, just as with the 3D hardware
approach, in theory a software-based approach could be spun off to other threads to
handle the work, as long as mechanisms were present to synchronize when necessary,
for example, when reading/writing directly to/from the frame buffer.

For the time being, MAME has opted for the second approach, leveraging a templated
helper class called **poly_manager** to handle common situations.


Concepts
--------

At its core, **poly_manager** is a mechanism to support multi-threaded rendering of
low-level 3D primitives. Callers provide **poly_manager** with a set of *vertices* for a
primitive plus a *render callback*. **poly_manager** breaks the primitive into
clipped scanline *extents* and distributes the work among a pool of *worker
threads*. The render callback is then called on the worker thread for each extent,
where game-specific logic can do whatever needs to happen to render the data.

One key responsibility that **poly_manager** takes care of is ensuring order. Given a
pool of threads and a number of work items to complete, it is important that—at least
within a given scanline—all work is performed serially in order. The basic approach is
to assign each extent to a *bucket* based on the Y coordinate. **poly_manager** then ensures
that only one worker thread at a time is responsible for processing work in a given bucket.

Vertices in **poly_manager** consist of simple 2D X and Y *coordinates*, plus zero or
more additional *iterated parameters*. These iterated parameters can be anything: intensity
values for lighting; RGB(A) colors for Gouraud shading; normalized U, V coordinates for
texture mapping; 1/Z values for Z buffering; etc. Iterated parameters, regardless of what
they represent, are interpolated linearly across the primitive in screen space and provided
as part of the extent to the render callback.


ObjectType
~~~~~~~~~~

When creating a **poly_manager** class, you must provide it a special type that you define,
known as **ObjectType**.

Because rendering happens asynchronously on worker threads, the idea is that the
**ObjectType** class will hold a snapshot of all the relevant data needed for rendering.
This allows the main thread to proceed—potentially modifying some of the relevant state—while
rendering happens elsewhere.

In theory, we could allocate a new **ObjectType** class for each primitive rendered;
however, that would be rather inefficient. It is quite common to set up the rendering
state and then render several primitives using the same state.

For this reason, **poly_manager** maintains an internal array of **ObjectType** objects and
keeps a copy of the last **ObjectType** used. Before submitting a new primitive, callers
can see if the rendering state has changed. If it has, it can ask **poly_manager** to allocate
a new **ObjectType** class and fill it in. When the primitive is submitted for rendering, the
most recently allocated **ObjectType** instance is implicitly captured and provided to the
render callbacks.

For more complex scenarios, where data might change even more infrequently, there is a
**poly_array** template, which can be used to manage data in a similar way. In fact,
internally **poly_manager** uses the **poly_array** class to manage its **ObjectType**
allocations. More information on the **poly_array** class is provided later.



Primitives
~~~~~~~~~~

**poly_manager** supports several different types of primitives:

* The most commonly-used primitive in **poly_manager** is the *triangle*, which has the
  nice property that iterated parameters have constant deltas across the full surface.
  Arbitrary-length *triangle fans* and *triangle strips* are also supported.

* In addition to triangles, **poly_manager** also supports *polygons* with an arbitrary
  number of vertices. The list of vertices is expected to be in either clockwise or
  anticlockwise order. **poly_manager** will walk the edges to compute deltas across
  each extent.

* As a special case, **poly_manager** supports a *tile* primitive, which is a simple quad
  defined by two vertices, a top-left vertex and a bottom-right vertex. Like triangles,
  tiles have constant iterated parameter deltas across their surface.

* Finally, **poly_manager** supports a fully custom mechanism where the caller provides
  a list of extents that are more or less fed directly to the worker threads.
  This is useful if emulating a system that has unusual primitives or requires highly
  specific behaviors for its edges.


Synchronization
~~~~~~~~~~~~~~~

One of the key requirements of providing an asynchronous rendering mechanism is
synchronization. Synchronization in **poly_manager** is super simple: just
call the ``wait()`` function.

There are several common reasons for issuing a wait:

* At display time, the pixel data must be copied to the screen. If any primitives were
  queued which touch the portion of the display that is going to be shown, you need to
  wait for rendering to be complete before copying. Note that this wait may not be
  strictly necessary in some situations (for example, a triple-buffered system).

* If the emulated system has a mechanism to read back from the framebuffer after
  rendering, then a wait must be issued prior to the read in order to ensure that
  asynchronous rendering is complete.

* If the emulated system modifies any state that is not cached in the **ObjectType**
  or elsewhere (for example, texture memory), then a wait must be issued to ensure
  that pending primitives which might consume that state have finished their work.

* If the emulated system can use a previous render target as, say, the texture source
  for a new primitive, then submitting the second primitive must wait until the first
  completes. **poly_manager** provides no internal mechanism to help detect this, so it
  is on the caller to determine when or if this is necessary.

Because the wait operation knows after it is done that all rendering is complete,
**poly_manager** also takes this opportunity to reclaim all memory allocated for its
internal structures, as well as memory allocated for **ObjectType** structures. Thus it is
important that you don’t hang onto any **ObjectType** pointers after a wait is called.


The poly_manager class
----------------------

In most applications, **poly_manager** is not used directly, but rather serves as
the base class for a more complete rendering class. The **poly_manager** class
itself is a template::

    template<typename BaseType, class ObjectType, int MaxParams, u8 Flags = 0>
    class poly_manager;

and the template parameters are:

* **BaseType** is the type used internally for coordinates and iterated parameters, and
  should generally be either ``float`` or ``double``. In theory, a fixed-point integral
  type could also be used, though the math logic has not been designed for that, so you
  may encounter problems.

* **ObjectType** is the user-defined per-object data structure described above.
  Internally, **poly_manager** will manage a **poly_array** of these, and a pointer to
  the most-recently allocated one at the time a primitive is submitted will be implicitly
  passed to the render callback for each corresponding extent.

* **MaxParams** is the maximum number of iterated parameters that may be specified in a
  vertex. Iterated parameters are generic and treated identically, so the mapping of
  parameter indices is completely up to the contract between the caller and the render
  callback. It is permitted for **MaxParams** to be 0.

* **Flags** is zero or more of the following flags:

  - POLY_FLAG_NO_WORK_QUEUE — specify this flag to disable asynchronous rendering; this
    can be useful for debugging. When this option is enabled, all primitives are queued
    and then processed in order on the calling thread when ``wait()`` is called on the
    **poly_manager** class.

  - POLY_FLAG_NO_CLIPPING — specify this if you want **poly_manager** to skip its
    internal clipping. Use this if your render callbacks do their own clipping, or if
    the caller always handles clipping prior to submitting primitives.


Types & Constants
~~~~~~~~~~~~~~~~~

vertex_t
++++++++

Within the **poly_manager** class, you’ll find a **vertex_t** type that describes a
single vertex. All primitive drawing methods accept 2 or more of these **vertex_t**
objects. The **vertex_t** includes the X and Y coordinates along with an array of
iterated parameter values at that vertex::

    struct vertex_t
    {
        vertex_t() { }
        vertex_t(BaseType _x, BaseType _y) { x = _x; y = _y; }

        BaseType x, y;                          // X, Y coordinates
        std::array<BaseType, MaxParams> p;      // iterated parameters
    };

Note that **vertex_t** itself is defined in terms of the **BaseType** and **MaxParams**
template values of the owning **poly_manager** class.

All of **poly_manager**’s primitives operate in screen space, where (0,0) represents the
top-left corner of the top-left pixel, and (0.5,0.5) represents the center of that pixel.
Left and top pixel values are inclusive, while right and bottom pixel values are exclusive.

Thus, a *tile* rendered from (2,2)-(4,3) will completely cover 2 pixels: (2,2) and (3,2).

When calling a primitive drawing method, the iterated parameter array **p** need not be
completely filled out. The number of valid iterated parameter values is specified as a
template parameter to the primitive drawing methods, so only that many parameters need to
actually be populated in the **vertex_t** structures that are passed in.


extent_t
++++++++

**poly_manager** breaks primitives into extents, which are contiguous horizontal spans
contained within a single scanline. These extents are then distributed to worker threads,
who will call the render callback with information on how to render each extent. The
**extent_t** type describes one such extent, providing the bounding X coordinates along with
an array of iterated parameter start values and deltas across the span::

    struct extent_t
    {
        struct param_t
        {
            BaseType start;                     // parameter value at start
            BaseType dpdx;                      // dp/dx relative to start
        };
        int16_t startx, stopx;                  // starting (inclusive)/ending (exclusive) endpoints
        std::array<param_t, MaxParams> param;   // array of parameter start/deltas
        void *userdata;                         // custom per-span data
    };

For each iterated parameter, the **start** value contains the value at the left side of
the span. The **dpdx** value contains the change of the parameter’s value per X coordinate.

There is also a **userdata** field in the **extent_t** structure, which is not normally used,
except when performing custom rendering.


render_delegate
+++++++++++++++

When rendering a primitive, in addition to the vertices, you must also provide a
**render_delegate** callback of the form::

  void render(int32_t y, extent_t const &extent, ObjectType const &object, int threadid)

This callback is responsible for the actual rendering. It will be called at a later time,
likely on a different thread, for each extent. The parameters passed are:

* **y** is the Y coordinate (scanline) of the current extent.

* **extent** is a reference to a **extent_t** structure, described above, which specifies for
  this extent the start/stop X values along with the start/delta values for each iterated
  parameter.

* **object** is a reference to the most recently allocated **ObjectType** at the time the
  primitive was submitted for rendering; in theory it should contain most of not all of the
  necessary data to perform rendering.

* **threadid** is a unique ID indicating the index of the thread you’re running on; this value
  is useful if you are keeping any kind of statistics and don’t want to add contention over
  shared values. In this situation, you can allocate **WORK_MAX_THREADS** instances of your
  data and update the instance for the **threadid** you are passed. When you want to display
  the statistics, the main thread can accumulate and reset the data from all threads when it’s
  safe to do so (e.g., after a wait).


Methods
~~~~~~~

poly_manager
++++++++++++
::

    poly_manager(running_machine &machine);

The **poly_manager** constructor takes just one parameter, a reference to the
**running_machine**. This grants **poly_manager** access to the work queues needed for
multithreaded running.

wait
++++
::

    void wait(char const *debug_reason = "general");

Calling ``wait()`` stalls the calling thread until all outstanding rendering is complete:

* **debug_reason** is an optional parameter specifying the reason for the wait. It is
  useful if the compile-time constant **TRACK_POLY_WAITS** is enabled, as it will print a
  summary of wait times and reasons at the end of execution.

**Return value:** none.

object_data
+++++++++++
::

    objectdata_array &object_data();

This method just returns a reference to the internally-maintained **poly_array** of the
**ObjectType** you specified when creating **poly_manager**. For most applications, the
only interesting thing to do with this object is call the ``next()`` method to allocate
a new object to fill out.

**Return value:** reference to a **poly_array** of **ObjectType**.

register_poly_array
+++++++++++++++++++
::

    void register_poly_array(poly_array_base &array);

For advanced applications, you may choose to create your own **poly_array** objects to
manage large chunks of infrequently-changed data, such a palettes. After each ``wait()``,
**poly_manager** resets all the **poly_array** objects it knows about in order to reclaim all
outstanding allocated memory. By registering your **poly_array** objects here, you can ensure
that your arrays will also be reset after an ``wait()`` call.

**Return value:** none.

render_tile
+++++++++++
::

    template<int ParamCount>
    uint32_t render_tile(rectangle const &cliprect, render_delegate callback,
                         vertex_t const &v1, vertex_t const &v2);

This method enqueues a single *tile* primitive for rendering:

* **ParamCount** is the number of live values in the iterated parameter array within each
  **vertex_t** provided; it must be no greater than the **MaxParams** value specified in the
  **poly_manager** template instantiation.

* **cliprect** is a reference to a clipping rectangle. All pixels and parameter values are
  clipped to stay within these bounds before being added to the work queues for rendering,
  unless **POLY_FLAG_NO_CLIPPING** was specified as a flag parameter to **poly_manager**.

* **callback** is the render callback delegate that will be called to render each extent.

* **v1** contains the coordinates and iterated parameters for the top-left corner of the tile.

* **v2** contains the coordinates and iterated parameters for the bottom-right corner of the tile.

**Return value:** the total number of clipped pixels represented by the enqueued extents.

render_triangle
+++++++++++++++
::

    template<int ParamCount>
    uint32_t render_triangle(rectangle const &cliprect, render_delegate callback,
                             vertex_t const &v1, vertex_t const &v2, vertex_t const &v3);

This method enqueues a single *triangle* primitive for rendering:

* **ParamCount** is the number of live values in the iterated parameter array within each
  **vertex_t** provided; it must be no greater than the **MaxParams** value specified in the
  **poly_manager** template instantiation.

* **cliprect** is a reference to a clipping rectangle. All pixels and parameter values are
  clipped to stay within these bounds before being added to the work queues for rendering,
  unless **POLY_FLAG_NO_CLIPPING** was specified as a flag parameter to **poly_manager**.

* **callback** is the render callback delegate that will be called to render each extent.

* **v1**, **v2**, **v3** contain the coordinates and iterated parameters for each vertex
  of the triangle.

**Return value:** the total number of clipped pixels represented by the enqueued extents.

render_triangle_fan
+++++++++++++++++++
::

    template<int ParamCount>
    uint32_t render_triangle_fan(rectangle const &cliprect, render_delegate callback,
                                 int numverts, vertex_t const *v);

This method enqueues one or more *triangle* primitives for rendering, specified in fan order:

* **ParamCount** is the number of live values in the iterated parameter array within each
  **vertex_t** provided; it must be no greater than the **MaxParams** value specified in the
  **poly_manager** template instantiation.

* **cliprect** is a reference to a clipping rectangle. All pixels and parameter values are
  clipped to stay within these bounds before being added to the work queues for rendering,
  unless **POLY_FLAG_NO_CLIPPING** was specified as a flag parameter to **poly_manager**.

* **callback** is the render callback delegate that will be called to render each extent.

* **numverts** is the total number of vertices provided; it must be at least 3.

* **v** is a pointer to an array of **vertex_t** objects containing the coordinates and iterated
  parameters for all the triangles, in fan order. This means that the first vertex is fixed.
  So if 5 vertices are provided, indicating 3 triangles, the vertices used will be:
  (0,1,2) (0,2,3) (0,3,4)

**Return value:** the total number of clipped pixels represented by the enqueued extents.

render_triangle_strip
+++++++++++++++++++++
::

    template<int ParamCount>
    uint32_t render_triangle_strip(rectangle const &cliprect, render_delegate callback,
                                   int numverts, vertex_t const *v);

This method enqueues one or more *triangle* primitives for rendering, specified in strip order:

* **ParamCount** is the number of live values in the iterated parameter array within each
  **vertex_t** provided; it must be no greater than the **MaxParams** value specified in the
  **poly_manager** template instantiation.

* **cliprect** is a reference to a clipping rectangle. All pixels and parameter values are
  clipped to stay within these bounds before being added to the work queues for rendering,
  unless **POLY_FLAG_NO_CLIPPING** was specified as a flag parameter to **poly_manager**.

* **callback** is the render callback delegate that will be called to render each extent.

* **numverts** is the total number of vertices provided; it must be at least 3.

* **v** is a pointer to an array of **vertex_t** objects containing the coordinates and iterated
  parameters for all the triangles, in strip order.
  So if 5 vertices are provided, indicating 3 triangles, the vertices used will be:
  (0,1,2) (1,2,3) (2,3,4)

**Return value:** the total number of clipped pixels represented by the enqueued extents.

render_polygon
++++++++++++++
::

    template<int NumVerts, int ParamCount>
    uint32_t render_polygon(rectangle const &cliprect, render_delegate callback, vertex_t const *v);

This method enqueues a single *polygon* primitive for rendering:

* **NumVerts** is the number of vertices in the polygon.

* **ParamCount** is the number of live values in the iterated parameter array within each
  **vertex_t** provided; it must be no greater than the **MaxParams** value specified in the
  **poly_manager** template instantiation.

* **cliprect** is a reference to a clipping rectangle. All pixels and parameter values are
  clipped to stay within these bounds before being added to the work queues for rendering,
  unless **POLY_FLAG_NO_CLIPPING** was specified as a flag parameter to **poly_manager**.

* **callback** is the render callback delegate that will be called to render each extent.

* **v** is a pointer to an array of **vertex_t** objects containing the coordinates and iterated
  parameters for the polygon. Vertices are assumed to be in either clockwise or anticlockwise
  order.

**Return value:** the total number of clipped pixels represented by the enqueued extents.

render_extents
++++++++++++++
::

    template<int ParamCount>
    uint32_t render_extents(rectangle const &cliprect, render_delegate callback,
                            int startscanline, int numscanlines, extent_t const *extents);

This method enqueues custom extents directly:

* **ParamCount** is the number of live values in the iterated parameter array within each
  **vertex_t** provided; it must be no greater than the **MaxParams** value specified in the
  **poly_manager** template instantiation.

* **cliprect** is a reference to a clipping rectangle. All pixels and parameter values are
  clipped to stay within these bounds before being added to the work queues for rendering,
  unless **POLY_FLAG_NO_CLIPPING** was specified as a flag parameter to **poly_manager**.

* **callback** is the render callback delegate that will be called to render each extent.

* **startscanline** is the Y coordinate of the first extent provided.

* **numscanlines** is the number of extents provided.

* **extents** is a pointer to an array of **extent_t** objects containing the start/stop
  X coordinates and iterated parameters. The **userdata** field of the source extents is
  copied to the target as well (this field is otherwise unused for all other types of
  rendering).

**Return value:** the total number of clipped pixels represented by the enqueued extents.

zclip_if_less
+++++++++++++
::

    template<int ParamCount>
    int zclip_if_less(int numverts, vertex_t const *v, vertex_t *outv, BaseType clipval);

This method is a helper method to clip a polygon against a provided Z value. It assumes
that the first iterated parameter in **vertex_t** represents the Z coordinate. If any edge
crosses the Z plane represented by **clipval** that edge is clipped.

* **ParamCount** is the number of live values in the iterated parameter array within each
  **vertex_t** provided; it must be no greater than the **MaxParams** value specified in the
  **poly_manager** template instantiation.

* **numverts** is the number of vertices in the input array.

* **v** is a pointer to the input array of **vertex_t** objects.

* **outv** is a pointer to the output array of **vertex_t** objects. **v** and **outv**
  cannot overlap or point to the same memory.

* **clipval** is the value to compare parameter 0 against for clipping.

**Return value:** the number of output vertices written to **outv**.
Note that by design it is possible for this method to produce more vertices than the
input array, so callers should ensure there is enough room in the output buffer to
accommodate this.


Example Renderer
----------------

Here is a complete example of how to create a software 3D renderer using **poly_manager**.
Our example renderer will only handle flat and Gouraud-shaded triangles with depth (Z)
buffering.


Types
~~~~~

The first thing we need to define is our *externally-visible* vertex format, which is distinct
from the internal **vertex_t** that **poly_manager** will define. In theory you could
use **vertex_t** directly, but the generic nature of **poly_manager**’s iterated parameters
make it awkward::

    struct example_vertex
    {
        float x, y, z;      // X,Y,Z coordinates
        rgb_t color;        // color at this vertex
    };

Next we define the **ObjectType** needed by **poly_manager**. For our simple case, we
define an **example_object_data** struct that consists of pointers to our rendering buffers,
plus a couple of fixed values that are consumed in some cases. More complex renderers would
typically have many more object-wide parameters defined here::

    struct example_object_data
    {
        bitmap_rgb32 *dest;    // pointer to the rendering bitmap
        bitmap_ind16 *depth;   // pointer to the depth bitmap
        rgb_t color;           // overall color (for clearing and flat shaded case)
        uint16_t depthval;     // fixed depth value (for clearing)
    };

Now it’s time to define our renderer class, which we derive from **poly_manager**. As
template parameters we specify ``float`` as the base type for our data, since that will
be enough accuracy for this example, and we also provide our **example_object_data** as
the **ObjectType** class, plus the maximum number of iterated parameters our renderer
will ever need (4 in this case)::

    class example_renderer : public poly_manager<float, example_object_data, 4>
    {
    public:
        example_renderer(running_machine &machine, uint32_t width, uint32_t height);

        bitmap_rgb32 *swap_buffers();

        void clear_buffers(rgb_t color, uint16_t depthval);
        void draw_triangle(example_vertex const *verts);

    private:
        static uint16_t ooz_to_depthval(float ooz);

        void draw_triangle_flat(example_vertex const *verts);
        void draw_triangle_gouraud(example_vertex const *verts);

        void render_clear(int32_t y, extent_t const &extent, example_object_data const &object, int threadid);
        void render_flat(int32_t y, extent_t const &extent, example_object_data const &object, int threadid);
        void render_gouraud(int32_t y, extent_t const &extent, example_object_data const &object, int threadid);

        int m_draw_buffer;
        bitmap_rgb32 m_display[2];
        bitmap_ind16 m_depth;
    };


Constructor
~~~~~~~~~~~

The constructor for our example renderer just initializes **poly_manager** and allocates
the rendering and depth buffers::

    example_renderer::example_renderer(running_machine &machine, uint32_t width, uint32_t height) :
        poly_manager(machine),
        m_draw_buffer(0)
    {
        // allocate two display buffers and a depth buffer
        m_display[0].allocate(width, height);
        m_display[1].allocate(width, height);
        m_depth.allocate(width, height);
    }


swap_buffers
~~~~~~~~~~~~

The first interesting method in our renderer is ``swap_buffers()``, which returns a pointer to
the buffer we’ve been drawing to, and sets up the other buffer as the new drawing target. The
idea is that the display update handler will call this method to get ahold of the bitmap to
display to the user::

    bitmap_rgb32 *example_renderer::swap_buffers()
    {
        // wait for any rendering to complete before returning the buffer
        wait("swap_buffers");

        // return the current draw buffer and then switch to the other
        // for future drawing
        bitmap_rgb32 *result = &m_display[m_draw_buffer];
        m_draw_buffer ^= 1;
        return result;
    }

The most important thing here to note here is the call to **poly_manager**’s ``wait()``, which
will block the current thread until all rendering is complete. This is important because
otherwise the caller may receive a bitmap that is still being drawn to, leading to torn
or corrupt visuals.


clear_buffers
~~~~~~~~~~~~~

One of the most common operations to perform when doing 3D rendering is to initialize or
clear the display and depth buffers to a known value. This method below leverages
the *tile* primitive to render a rectangle over the screen by passing in (0,0) and (width,height)
for the two vertices.

Because the color and depth values to clear the buffer to are constant, they are stored in
a freshly-allocated **example_object_data** object, along with a pointer to the buffers in
question. The ``render_tile()`` call is made with a ``<0>`` suffix indicating that there are
no iterated parameters to worry about::

    void example_renderer::clear_buffers(rgb_t color, uint16_t depthval)
    {
        // allocate object data and populate it with information needed
        example_object_data &object = object_data().next();
        object.dest = &m_display[m_draw_buffer];
        object.depth = &m_depth;
        object.color = color;
        object.depthval = depthval;

        // top,left coordinate is always (0,0)
        vertex_t topleft;
        topleft.x = 0;
        topleft.y = 0;

        // bottom,right coordinate is (width,height)
        vertex_t botright;
        botright.x = m_display[0].width();
        botright.y = m_display[0].height();

        // render as a tile with 0 iterated parameters
        render_tile<0>(m_display[0].cliprect(),
                       render_delegate(&example_renderer::render_clear, this),
                       topleft, botright);
    }

The render callback provided to ``render_tile()`` is also defined (privately) in our class,
and handles a single span. Note how the rendering parameters are extracted from the
**example_object_data** struct provided::

    void example_renderer::render_clear(int32_t y, extent_t const &extent, example_object_data const &object, int threadid)
    {
        // get pointers to the start of the depth buffer and destination scanlines
        uint16_t *depth = &object.depth->pix(y);
        uint32_t *dest = &object.dest->pix(y);

        // loop over the full extent and just store the constant values from the object
        for (int x = extent.startx; x < extent.stopx; x++)
        {
            dest[x] = object.color;
            depth[x] = object.depthval;
        }
    }

Another important point to make is that the X coordinates provided by extent struct are
inclusive of startx but exclusive of stopx. Clipping is performed ahead of time so that
the render callback can focus on laying down pixels as quickly as possible with minimal
overhead.


draw_triangle
~~~~~~~~~~~~~

Next up, we have our actual triangle rendering function, which will draw a single triangle
given an array of three vertices provided in the external **example_vertex** format::

    void example_renderer::draw_triangle(example_vertex const *verts)
    {
        // flat shaded case
        if (verts[0].color == verts[1].color && verts[0].color == verts[2].color)
            draw_triangle_flat(verts);
        else
            draw_triangle_gouraud(verts);
    }

Because it is simpler and faster to render a flat shaded triangle, the code checks to see
if the colors are the same on all three vertices. If they are, we call through to a special
flat-shaded case, otherwise we process it as a full Gouraud-shaded triangle.

This is a common technique to optimize rendering performance: identify special cases that
reduce the per-pixel work, and route them to separate render callbacks that are optimized
for that special case.


draw_triangle_flat
~~~~~~~~~~~~~~~~~~

Here’s the setup code for rendering a flat-shaded triangle::

    void example_renderer::draw_triangle_flat(example_vertex const *verts)
    {
        // allocate object data and populate it with information needed
        example_object_data &object = object_data().next();
        object.dest = &m_display[m_draw_buffer];
        object.depth = &m_depth;

        // in this case the color is constant and specified in the object data
        object.color = verts[0].color;

        // copy X, Y, and 1/Z into poly_manager vertices
        vertex_t v[3];
        for (int vertnum = 0; vertnum < 3; vertnum++)
        {
            v[vertnum].x = verts[vertnum].x;
            v[vertnum].y = verts[vertnum].y;
            v[vertnum].p[0] = 1.0f / verts[vertnum].z;
        }

        // render the triangle with 1 iterated parameter (1/Z)
        render_triangle<1>(m_display[0].cliprect(),
                            render_delegate(&example_renderer::render_flat, this),
                            v[0], v[1], v[2]);
    }

First, we put the fixed color into the **example_object_data** directly, and then fill
out three **vertex_t** objects with the X and Y coordinates in the usual spot, and 1/Z
as our one and only iterated parameter. (We use 1/Z here because iterated parameters are
interpolated linearly in screen space. Z is not linear in screen space, but 1/Z is due to
perspective correction.)

Our flat-shaded case then calls ``render_trangle`` specifying ``<1>`` iterated parameter to
interpolate, and pointing to a special-case flat render callback::

    void example_renderer::render_flat(int32_t y, extent_t const &extent, example_object_data const &object, int threadid)
    {
        // get pointers to the start of the depth buffer and destination scanlines
        uint16_t *depth = &object.depth->pix(y);
        uint32_t *dest = &object.dest->pix(y);

        // get the starting 1/Z value and the delta per X
        float ooz = extent.param[0].start;
        float doozdx = extent.param[0].dpdx;

        // iterate over the extent
        for (int x = extent.startx; x < extent.stopx; x++)
        {
            // convert the 1/Z value into an integral depth value
            uint16_t depthval = ooz_to_depthval(ooz);

            // if closer than the current pixel, copy the color and depth value
            if (depthval < depth[x])
            {
                dest[x] = object.color;
                depth[x] = depthval;
            }

            // regardless, update the 1/Z value for the next pixel
            ooz += doozdx;
        }
    }

This render callback is a bit more involved than the clearing case.

First, we have an iterated parameter (1/Z) to deal with, whose starting and X-delta
values we extract from the extent before the start of the inner loop.

Second, we perform depth buffer testing, using ``ooz_to_depthval()`` as a helper
to transform the floating-point 1/Z value into a 16-bit integer. We compare this value against
the current depth buffer value, and only store the pixel/depth value if it’s less.

At the end of each iteration, we advance the 1/Z value by the X-delta in preparation for the
next pixel.


draw_triangle_gouraud
~~~~~~~~~~~~~~~~~~~~~

Finally we get to the code for the full-on Gouraud-shaded case::

    void example_renderer::draw_triangle_gouraud(example_vertex const *verts)
    {
        // allocate object data and populate it with information needed
        example_object_data &object = object_data().next();
        object.dest = &m_display[m_draw_buffer];
        object.depth = &m_depth;

        // copy X, Y, 1/Z, and R,G,B into poly_manager vertices
        vertex_t v[3];
        for (int vertnum = 0; vertnum < 3; vertnum++)
        {
            v[vertnum].x = verts[vertnum].x;
            v[vertnum].y = verts[vertnum].y;
            v[vertnum].p[0] = 1.0f / verts[vertnum].z;
            v[vertnum].p[1] = verts[vertnum].color.r();
            v[vertnum].p[2] = verts[vertnum].color.g();
            v[vertnum].p[3] = verts[vertnum].color.b();
        }

        // render the triangle with 4 iterated parameters (1/Z, R, G, B)
        render_triangle<4>(m_display[0].cliprect(),
                            render_delegate(&example_renderer::render_gouraud, this),
                            v[0], v[1], v[2]);
    }

Here we have 4 iterated parameters: the 1/Z depth value, plus red, green, and blue,
stored as floating point values. We call ``render_triangle()`` with ``<4>`` as the
number of iterated parameters to process, and point to the full Gouraud render callback::

    void example_renderer::render_gouraud(int32_t y, extent_t const &extent, example_object_data const &object, int threadid)
    {
        // get pointers to the start of the depth buffer and destination scanlines
        uint16_t *depth = &object.depth->pix(y);
        uint32_t *dest = &object.dest->pix(y);

        // get the starting 1/Z value and the delta per X
        float ooz = extent.param[0].start;
        float doozdx = extent.param[0].dpdx;

        // get the starting R,G,B values and the delta per X as 8.24 fixed-point values
        uint32_t r = uint32_t(extent.param[1].start * float(1 << 24));
        uint32_t drdx = uint32_t(extent.param[1].dpdx * float(1 << 24));
        uint32_t g = uint32_t(extent.param[2].start * float(1 << 24));
        uint32_t dgdx = uint32_t(extent.param[2].dpdx * float(1 << 24));
        uint32_t b = uint32_t(extent.param[3].start * float(1 << 24));
        uint32_t dbdx = uint32_t(extent.param[3].dpdx * float(1 << 24));

        // iterate over the extent
        for (int x = extent.startx; x < extent.stopx; x++)
        {
            // convert the 1/Z value into an integral depth value
            uint16_t depthval = ooz_to_depthval(ooz);

            // if closer than the current pixel, assemble the color
            if (depthval < depth[x])
            {
                dest[x] = rgb_t(r >> 24, g >> 24, b >> 24);
                depth[x] = depthval;
            }

            // regardless, update the 1/Z and R,G,B values for the next pixel
            ooz += doozdx;
            r += drdx;
            g += dgdx;
            b += dbdx;
        }
    }

This follows the same pattern as the flat-shaded callback, except we have 4 iterated parameters
to step through.

Note that even though the iterated parameters are of ``float`` type, we convert the
color values to fixed-point integers when iterating over them. This saves us doing 3
float-to-int conversions each pixel. The original RGB values were 0-255, so interpolation
can only produce values in the 0-255 range. Thus we can use 24 bits of a 32-bit integer as
the fraction, which is plenty accurate for this case.


Advanced Topic: the poly_array class
------------------------------------

**poly_array** is a template class that is used to manage a dynamically-sized vector of
objects whose lifetime starts at allocation and ends when ``reset()`` is called. The
**poly_manager** class uses several **poly_array** objects internally, including one for
allocated **ObjectType** data, one for each primitive rendered, and one for holding all
allocated extents.

**poly_array** has an additional property where after a reset it retains a copy of the most
recently allocated object. This ensures that callers can always call ``last()`` and get
a valid object, even immediately after a reset.

The **poly_array** class requires two template parameters::

    template<class ArrayType, int TrackingCount>
    class poly_array;

These parameters are:

* **ArrayType** is the type of object you wish to allocate and manage.

* **TrackingCount** is the number of objects you wish to preserve after a reset. Typically
  this value is either 0 (you don’t care to track any objects) or 1 (you only need one
  object); however, if you are using **poly_array** to manage a shared collection of
  objects across several independent consumers, it can be higher. See below for an example
  where this might be handy.

Note that objects allocated by **poly_array** are owned by **poly_array** and will be
automatically freed upon exit.

**poly_array** is optimized for use in high frequency multi-threaded systems. Therefore,
one added feature of the class is that it rounds the allocation size of **ArrayType** to
the nearest cache line boundary, on the assumption that neighboring entries could be
accessed by different cores simultaneously. Keeping each **ArrayType** object in its
own cache line ensures no false sharing performance impacts.

Currently, **poly_array** has no mechanism to determine cache line size at runtime, so
it presumes that 64 bytes is a typical cache line size, which is true for most x64 and ARM
chips as of 2021. This value can be altered by changing the **CACHE_LINE_SHIFT** constant
defined at the top of the class.

Objects allocated by **poly_array** are created in 64k chunks. At construction time, one
chunk’s worth of objects is allocated up front. The chunk size is controlled by the
**CHUNK_GRANULARITY** constant defined at the top of the class.

As more objects are allocated, if **poly_array** runs out of space, it will dynamically
allocate more. This will produce discontiguous chunks of objects until the next ``reset()``
call, at which point **poly_array** will reallocate all the objects into a contiguous
vector once again.

For the case where **poly_array** is used to manage a shared pool of objects, it can be
configured to retain multiple most recently allocated items by using a **TrackingCount**
greater than 1. For example, if **poly_array** is managing objects for two texture units,
then it can set **TrackingCount** equal to 2, and pass the index of the texture unit in
calls to ``next()`` and ``last()``. After a reset, **poly_array** will remember the most
recently allocated object for each of the units independently.


Methods
~~~~~~~

poly_array
++++++++++
::

    poly_array();

The **poly_array** constructor requires no parameters and simply pre-allocates one
chunk of objects in preparation for future allocations.

count
+++++
::

	u32 count() const;

**Return value:** the number of objects currently allocated.

max
+++
::

	u32 max() const;

**Return value:** the maximum number of objects ever allocated at one time.

itemsize
++++++++
::

	size_t itemsize() const;

**Return value:** the size of an object, rounded up to the nearest cache line boundary.

allocated
+++++++++
::

	u32 allocated() const;

**Return value:** the number of objects that fit within what’s currently been allocated.

byindex
+++++++
::

	ArrayType &byindex(u32 index);

Returns a reference to an object in the array by index. Equivalent to [**index**] on a
normal array:

* **index** is the index of the item you wish to reference.

**Return value:** a reference to the object in question. Since a reference is returned,
it is your responsibility to ensure that **index** is less than ``count()`` as there
is no mechanism to return an invalid result.

contiguous
++++++++++
::

	ArrayType *contiguous(u32 index, u32 count, u32 &chunk);

Returns a pointer to the base of a contiguous section of **count** items starting at
**index**. Because **poly_array** dynamically resizes, it may not be possible to access
all **count** objects contiguously, so the number of actually contiguous items is
returned in **chunk**:

* **index** is the index of the first item you wish to access contiguously.

* **count** is the number of items you wish to access contiguously.

* **chunk** is a reference to a variable that will be set to the actual number of
  contiguous items available starting at **index**. If **chunk** is less than **count**,
  then the caller should process the **chunk** items returned, then call ``contiguous()``
  again at (**index** + **chunk**) to access the rest.

**Return value:** a pointer to the first item in the contiguous chunk. No range checking
is performed, so it is your responsibility to ensure that **index** + **count** is less
than or equal to ``count()``.

indexof
+++++++
::

	int indexof(ArrayType &item) const;

Returns the index within the array of the given item:

* **item** is a reference to an item in the array.

**Return value:** the index of the item. It should always be the case that::

    array.indexof(array.byindex(index)) == index

reset
+++++
::

	void reset();

Resets the **poly_array** by semantically deallocating all objects. If previous allocations
created a discontiguous array, a fresh vector is allocated at this time so that future
allocations up to the same level will remain contiguous.

Note that the **ArrayType** destructor is *not* called on objects as they are deallocated.

**Return value:** none.

next
++++
::

	ArrayType &next(int tracking_index = 0);

Allocates a new object and returns a reference to it. If there is not enough space for
a new object in the current array, a new discontiguous array is created to hold it:

* **tracking_index** is the tracking index you wish to assign the new item to. In the
  common case this is 0, but could be non-zero if using a **TrackingCount** greater than 1.

**Return value:** a reference to the object. Note that the placement new operator is
called on this object, so the default **ArrayType** constructor will be invoked here.

last
++++
::

	ArrayType &last(int tracking_index = 0) const;

Returns a reference to the last object allocated:

* **tracking_index** is the tracking index whose object you want. In the
  common case this is 0, but could be non-zero if using a **TrackingCount** greater than 1.
  **poly_array** remembers the most recently allocated object independently for each
  **tracking_index**.

**Return value:** a reference to the last allocated object.
