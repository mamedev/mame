Emulated system memory and address spaces management
====================================================

.. contents:: :local:


1. Overview
-----------

The memory subsystem (emumem and addrmap) combines multiple functions
useful for system emulation:

* address bus decoding and dispatching with caching
* static descriptions of an address map
* RAM allocation and registration for state saving
* interaction with memory regions to access ROM

Devices create address spaces, e.g. decodable buses, through the
``device_memory_interface``.  The machine configuration sets up address
maps to put in the address spaces, then the device can do read and
writes through the bus.

2. Basic concepts
-----------------

2.1 Address spaces
~~~~~~~~~~~~~~~~~~

An address space, implemented in the class **address_space**,
represents an addressable bus with potentially multiple sub-devices
connected requiring a decode.  It has a number of data lines (8, 16,
32 or 64) called data width, a number of address lines (1 to 32)
called address width and an Endianness.  In addition an address shift
allows for buses that have an atomic granularity different than a
byte.

Address space objects provide a series of methods for read and write
access, and a second series of methods for dynamically changing the
decode.


2.2 Address maps
~~~~~~~~~~~~~~~~

An address map is a static description of the decode expected when
using a bus.  It connects to memory, other devices and methods, and is
installed, usually at startup, in an address space.  That description
is stored in an **address_map** structure which is filled
programmatically.


2.3 Shares, banks and regions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Memory shares are allocated memory zones that can be put in multiple
places in the same or different address spaces, and can also be
directly accessed from devices.

Memory banks are zones that indirect memory access, giving the
possibility to dynamically and efficiently change where a zone
actually points to.

Memory regions are read-only memory zones in which ROMs are loaded.

All of these have names allowing to access them.

2.4 Views
~~~~~~~~~

Views are a way to multiplex different submaps over a memory range
with fast switching.  It is to be used when multiple devices map at
the same addresses and are switched in externally.  They must be
created as an object of the device and then setup either statically in
a memory map or dynamically through ``install_*`` calls.

Switchable submaps, aka variants, are named through an integer.  An
internal indirection through a map ensures that any integer value can
be used.


3. Memory objects
-----------------

3.1 Shares - memory_share
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    class memory_share {
        const std::string &name() const;
        void *ptr() const;
        size_t bytes() const;
        endianness_t endianness() const;
        u8 bitwidth() const;
        u8 bytewidth() const;
    };

A memory share is a named allocated memory zone that is automatically
saved in save states and can be mapped in address spaces.  It is the
standard container for memory that is shared between spaces, but also
shared between an emulated CPU and a driver.  As such one has easy
access to its contents from the driver class.

.. code-block:: C++

    required_shared_ptr<uNN> m_share_ptr;
    optional_shared_ptr<uNN> m_share_ptr;
    required_shared_ptr_array<uNN, count> m_share_ptr_array;
    optional_shared_ptr_array<uNN, count> m_share_ptr_array;

    [device constructor] m_share_ptr(*this, "name"),
    [device constructor] m_share_ptr_array(*this, "name%u", 0U),

At the device level, a pointer to the memory zone can easily be
retrieved by building one of these four finders.  Note that like for
every finder calling ``target()`` on the finder gives you the base
pointer of the ``memory_share`` object.

.. code-block:: C++

    memory_share_creator<uNN> m_share;

    [device constructor] m_share(*this, "name", size, endianness),

A memory share can be created if it doesn’t exist in a memory map
through that creator class.  If it already exists it is just
retrieved.  That class behaves like a pointer but also has the
``target()``, ``length()``, ``bytes()``, ``endianness()``,
``bitwidth()`` and ``bytewidth()`` methods for share information.

.. code-block:: C++

    memory_share *memshare(string tag) const;

The ``memshare`` device method retrieves a memory share by name.  Beware
that the lookup can be expensive, prefer finders instead.

3.2 Banks - memory_bank
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    class memory_bank {
        const std::string &tag() const;
        int entry() const;
        void set_entry(int entrynum);
        void configure_entry(int entrynum, void *base);
        void configure_entries(int startentry, int numentry, void *base, offs_t stride);
        void set_base(void *base);
        void *base() const;
    };

A memory bank is a named memory zone indirection that can be mapped in
address spaces.  It points to ``nullptr`` when created.
``configure_entry`` associates an entry number and a base pointer.
``configure_entries`` does the same for multiple consecutive entries
spanning a memory zone.  Alternatively ``set_base`` sets the base for
entry 0 and selects it.

``set_entry`` allows to dynamically and efficiently select the current
active entry, ``entry()`` gets that selection back, and ``base()`` gets
the associated base pointer.

.. code-block:: C++

    required_memory_bank m_bank;
    optional_memory_bank m_bank;
    required_memory_bank_array<count> m_bank_array;
    optional_memory_bank_array<count> m_bank_array;

    [device constructor] m_bank(*this, "name"),
    [device constructor] m_bank_array(*this, "name%u", 0U),

At the device level, a pointer to the memory bank object can easily be
retrieved by building one of these four finders.

.. code-block:: C++

    memory_bank_creator m_bank;

    [device constructor] m_bank(*this, "name"),

A memory share can be created if it doesn’t exist in a memory map
through that creator class.  If it already exists it is just
retrieved.

.. code-block:: C++

    memory_bank *membank(string tag) const;

The ``membank`` device method retrieves a memory bank by name.  Beware
that the lookup can be expensive, prefer finders instead.


3.3 Regions - memory_region
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    class memory_region {
        u8 *base();
        u8 *end();
        u32 bytes() const;
        const std::string &name() const;
        endianness_t endianness() const;
        u8 bitwidth() const;
        u8 bytewidth() const;
        u8 &as_u8(offs_t offset = 0);
        u16 &as_u16(offs_t offset = 0);
        u32 &as_u32(offs_t offset = 0);
        u64 &as_u64(offs_t offset = 0);
    }

A region is used to store read-only data like ROMs or the result of
fixed decryptions.  Their contents are not saved, which is why they
should not being written to from the emulated system.  They don’t
really have an intrinsic width (``base()`` returns an ``u8 *`` always),
which is historical and pretty much unfixable at this point.  The
``as_*`` methods allow for accessing them at a given width.

.. code-block:: C++

    required_memory_region m_region;
    optional_memory_region m_region;
    required_memory_region_array<count> m_region_array;
    optional_memory_region_array<count> m_region_array;

    [device constructor] m_region(*this, "name"),
    [device constructor] m_region_array(*this, "name%u", 0U),

At the device level, a pointer to the memory region object can easily be
retrieved by building one of these four finders.

.. code-block:: C++

    memory_region *memregion(string tag) const;

The ``memregion`` device method retrieves a memory region by name.
Beware that the lookup can be expensive, prefer finders instead.


3.4 Views - memory_view
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    class memory_view {
        memory_view(device_t &device, std::string name);
        memory_view_entry &operator[](int slot);

        void select(int entry);
        void disable();

        const std::string &name() const;
    }

A view allows to switch part of a memory map between multiple
possibilities, or even disable it entirely to see what was there
before.  It is created as an object of the device.

.. code-block:: C++

    memory_view m_view;

    [device constructor] m_view(*this, "name"),

It is then setup through the address map API or dynamically.  At
runtime, a numbered variant can be selected using the ``select`` method,
or the view can be disabled using the ``disable`` method.  A disabled
view can be re-enabled at any time.


4. Address maps API
-------------------

4.1 General API structure
~~~~~~~~~~~~~~~~~~~~~~~~~

An address map is a method of a device which fills an **address_map**
structure, usually called **map**, passed by reference.  The method
then can set some global configuration through specific methods and
then provide address range-oriented entries which indicate what should
happen when a specific range is accessed.

The general syntax for entries uses method chaining:

.. code-block:: C++

    map(start, end).handler(...).handler_qualifier(...).range_qualifier();

The values start and end define the range, the handler() block
determines how the access is handled, the handler_qualifier() block
specifies some aspects of the handler (memory sharing for instance) and
the range_qualifier() block refines the range (mirroring, masking, lane
selection, etc.).

The map follows a “last one wins” principle, where the handler specified
last is selected when multiple handlers match a given address.


4.2 Global configurations
~~~~~~~~~~~~~~~~~~~~~~~~~

4.2.1 Global masking
''''''''''''''''''''

.. code-block:: C++

    map.global_mask(offs_t mask);

Specifies a mask to be applied to all addresses when accessing the space
that map is installed in.


4.2.2 Returned value on unmapped/nop-ed read
''''''''''''''''''''''''''''''''''''''''''''

.. code-block:: C++

    map.unmap_value_low();
    map.unmap_value_high();
    map.unmap_value(u8 value);

Sets the value to return on reads to an unmapped or nopped-out address.
Low means 0, high ~0.


4.3 Handler setting
~~~~~~~~~~~~~~~~~~~

4.3.1 Method on the current device
''''''''''''''''''''''''''''''''''

.. code-block:: C++

    (...).r(FUNC(my_device::read_method))
    (...).w(FUNC(my_device::write_method))
    (...).rw(FUNC(my_device::read_method), FUNC(my_device::write_method))

    uNN my_device::read_method(address_space &space, offs_t offset, uNN mem_mask)
    uNN my_device::read_method(address_space &space, offs_t offset)
    uNN my_device::read_method(address_space &space)
    uNN my_device::read_method(offs_t offset, uNN mem_mask)
    uNN my_device::read_method(offs_t offset)
    uNN my_device::read_method()

    void my_device::write_method(address_space &space, offs_t offset, uNN data, uNN mem_mask)
    void my_device::write_method(address_space &space, offs_t offset, uNN data)
    void my_device::write_method(address_space &space, uNN data)
    void my_device::write_method(offs_t offset, uNN data, uNN mem_mask)
    void my_device::write_method(offs_t offset, uNN data)
    void my_device::write_method(uNN data)

Sets a method of the current device or driver to read, write or both
for the current entry.  The prototype of the method can take multiple
forms making some elements optional.  ``uNN`` represents ``u8``,
``u16``, ``u32`` or ``u64`` depending on the data width of the handler.
The handler can be narrower than the bus itself (for instance a 8-bit
device on a 32-bit bus).

The offset passed in is built from the access address.  It starts at
zero at the start of the range, and increments for each ``uNN`` unit.
An ``u8`` handler will get an offset in bytes, an ``u32`` one in double
words.  The ``mem_mask`` has its bits set where the accessors actually
drive the bit.  It’s usually built in byte units, but in some cases of
I/O chips ports with per-bit direction registers the resolution can be
at the bit level.


4.3.2 Method on a different device
''''''''''''''''''''''''''''''''''

.. code-block:: C++

    (...).r(m_other_device, FUNC(other_device::read_method))
    (...).r("other-device-tag", FUNC(other_device::read_method))
    (...).w(m_other_device, FUNC(other_device::write_method))
    (...).w("other-device-tag", FUNC(other_device::write_method))
    (...).rw(m_other_device, FUNC(other_device::read_method), FUNC(other_device::write_method))
    (...).rw("other-device-tag", FUNC(other_device::read_method), FUNC(other_device::write_method))

Sets a method of another device, designated by an object finder
(usually ``required_device`` or ``optional_device``) or its tag, to
read, write or both for the current entry.


4.3.3 Lambda function
'''''''''''''''''''''

.. code-block:: C++

    (...).lr{8,16,32,64}(NAME([...](address_space &space, offs_t offset, uNN mem_mask) -> uNN { ... }))
    (...).lr{8,16,32,64}([...](address_space &space, offs_t offset, uNN mem_mask) -> uNN { ... }, "name")
    (...).lw{8,16,32,64}(NAME([...](address_space &space, offs_t offset, uNN data, uNN mem_mask) -> void { ... }))
    (...).lw{8,16,32,64}([...](address_space &space, offs_t offset, uNN data, uNN mem_mask) -> void { ... }, "name")
    (...).lrw{8,16,32,64}(NAME(read), NAME(write))
    (...).lrw{8,16,32,64}(read, "name_r", write, "name_w")

Sets a lambda called on read, write or both.  The lambda prototype can
be any of the six available for methods.  One can either use ``NAME()``
over the whole lambda, or provide a name after the lambda definition.
The number is the data width of the access, e.g. the NN.


4.3.4 Direct memory access
''''''''''''''''''''''''''

.. code-block:: C++

    (...).rom()
    (...).writeonly()
    (...).ram()

Selects the range to access a memory zone as read-only, write-only or
read/write respectively.  Specific handler qualifiers specify the
location of this memory zone.  There are two cases when no qualifier is
acceptable:

* ``ram()`` gives an anonymous RAM zone not accessible outside of the
  address space.

* ``rom()`` when the memory map is used in an ``AS_PROGRAM``
  space of a (CPU) device which names is also the name of a region.
  Then the memory zone points to that region at the offset
  corresponding to the start of the zone.

.. code-block:: C++

    (...).rom().region("name", offset)

The ``region`` qualifier causes a read-only zone point to the contents
of a given region at a given offset.

.. code-block:: C++

    (...).rom().share("name")
    (...).writeonly.share("name")
    (...).ram().share("name")

The ``share`` qualifier causes the zone point to a shared memory region
identified by its name.  If the share is present in multiple spaces, the
size, bus width, and, if the bus is more than byte-wide, the Endianness
must match.


4.3.5 Bank access
'''''''''''''''''

.. code-block:: C++

    (...).bankr("name")
    (...).bankw("name")
    (...).bankrw("name")

Sets the range to point at the contents of a memory bank in read, write
or read/write mode.


4.3.6 Port access
'''''''''''''''''

.. code-block:: C++

    (...).portr("name")
    (...).portw("name")
    (...).portrw("name")

Sets the range to point at an I/O port.


4.3.7 Dropped access
''''''''''''''''''''

.. code-block:: C++

    (...).nopr()
    (...).nopw()
    (...).noprw()

Sets the range to drop the access without logging.  When reading, the
unmap value is returned.


4.3.8 Unmapped access
'''''''''''''''''''''

.. code-block:: C++

    (...).unmapr()
    (...).unmapw()
    (...).unmaprw()

Sets the range to drop the access with logging.  When reading, the
unmap value is returned.


4.3.9 Subdevice mapping
'''''''''''''''''''''''

.. code-block:: C++

    (...).m(m_other_device, FUNC(other_device::map_method))
    (...).m("other-device-tag", FUNC(other_device::map_method))

Includes a device-defined submap.  The start of the range indicates
where the address zero of the submap ends up, and the end of the range
clips the submap if needed.  Note that range qualifiers (defined
later) apply.

Currently, only handlers are allowed in submaps and not memory zones
or banks.


4.4 Range qualifiers
~~~~~~~~~~~~~~~~~~~~

4.4.1 Mirroring
'''''''''''''''

.. code-block:: C++

    (...).mirror(mask)

Duplicate the range on the addresses reachable by setting any of the 1
bits present in mask.  For instance, a range 0-0x1f with mirror 0x300
will be present on 0-0x1f, 0x100-0x11f, 0x200-0x21f and 0x300-0x31f.
The addresses passed in to the handler stay in the 0-0x1f range, the
mirror bits are not seen by the handler.


4.4.2 Masking
'''''''''''''

.. code-block:: C++

    (...).mask(mask)

Only valid with handlers, the address will be masked with the mask
before being passed to the handler.


4.4.3 Selection
'''''''''''''''

.. code-block:: C++

    (...).select(mask)

Only valid with handlers, the range will be mirrored as with mirror,
but the mirror address bits are preserved in the offset passed to the
handler when it is called.  This is useful for devices like sound
chips where the low bits of the address select a function and the high
bits a voice number.


4.4.4 Sub-unit selection
''''''''''''''''''''''''

.. code-block:: C++

    (...).umask16(16-bits mask)
    (...).umask32(32-bits mask)
    (...).umask64(64-bits mask)

Only valid with handlers and submaps, selects which data lines of the
bus are actually connected to the handler or the device.  The mask value
should be a multiple of a byte, e.g. the mask is a series of 00 and ff.
The offset will be adjusted accordingly, so that a difference of 1 means
the next handled unit in the access.

If the mask is narrower than the bus width, the mask is replicated in
the upper lines.


4.4.5 Chip select handling on sub-unit
''''''''''''''''''''''''''''''''''''''

.. code-block:: C++

    (...).cselect(16/32/64)

When a device is connected to part of the bus, like a byte on a
16-bits bus, the target handler is only activated when that part is
actually accessed.  In some cases, very often byte access on a 68000
16-bits bus, the actual hardware only checks the word address and not
if the correct byte is accessed.  ``cswidth`` tells the memory system to
trigger the handler if a wider part of the bus is accessed.  The
parameter is that trigger width (would be 16 in the 68000 case).


4.4.6 User flags
''''''''''''''''

.. code-block:: C++

    (...).flags(16-bits mask)

This parameter allows to set user-defined flags on the handler which
can then be retrieved by an accessing device to change their
behaviour.  An example of use the the i960 which marks burstable zones
that way (they have a specific hardware-level support).


4.5 View setup
~~~~~~~~~~~~~~

.. code-block:: C++

    map(start, end).view(m_view);
    m_view[0](start1, end1).[...];

A view is setup in a address map with the view method.  The only
qualifier accepted is mirror.  The “disabled” version of the view will
include what was in the range prior to the view setup.

The different variants are setup by indexing the view with the variant
number and setting up an entry in the usual way.  The entries within a
variant must of course stay within the range.  There are no other
additional constraints.  The contents of a variant, by default, are
what was there before, i.e. the contents of the disabled view, and
setting it up allows part or all of it to be overridden.

Variants can only be setup once the view itself has been setup with
the ``view`` method.

A view can only be put in one address map and in only one position.
If multiple views have identical or similar contents, remember that
setting up a map is nothing more than a method call, and creating a
second method to setup a view is perfectly reasonable.  A view is of
type ``memory_view`` and an indexed entry (e.g. a variant to setup) is
of type ``memory_view::memory_view_entry &``.

A view can be installed in another view, but don’t forget that a view
can be installed only once.  A view can also be part of “what was there
before”.


5. Address space dynamic mapping API
------------------------------------

5.1 General API structure
~~~~~~~~~~~~~~~~~~~~~~~~~

A series of methods allow the bus decoding of an address space to be
changed on-the-fly.  They’re powerful but have some issues:

* changing the mappings repeatedly can be slow
* the address space state is not saved in the saved states, so it has to
  be rebuilt after state load
* they can be hidden anywhere rather that be grouped in an address map,
  which can be less readable

The methods, rather than decomposing the information in handler, handler
qualifier and range qualifier, put them all together as method
parameters.  To make things a little more readable, lots of them are
optional.


5.2 Handler mapping
~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    uNN my_device::read_method(address_space &space, offs_t offset, uNN mem_mask)
    uNN my_device::read_method_m(address_space &space, offs_t offset)
    uNN my_device::read_method_mo(address_space &space)
    uNN my_device::read_method_s(offs_t offset, uNN mem_mask)
    uNN my_device::read_method_sm(offs_t offset)
    uNN my_device::read_method_smo()

    void my_device::write_method(address_space &space, offs_t offset, uNN data, uNN mem_mask)
    void my_device::write_method_m(address_space &space, offs_t offset, uNN data)
    void my_device::write_method_mo(address_space &space, uNN data)
    void my_device::write_method_s(offs_t offset, uNN data, uNN mem_mask)
    void my_device::write_method_sm(offs_t offset, uNN data)
    void my_device::write_method_smo(uNN data)

    readNN_delegate   (device, FUNC(read_method))
    readNNm_delegate  (device, FUNC(read_method_m))
    readNNmo_delegate (device, FUNC(read_method_mo))
    readNNs_delegate  (device, FUNC(read_method_s))
    readNNsm_delegate (device, FUNC(read_method_sm))
    readNNsmo_delegate(device, FUNC(read_method_smo))

    writeNN_delegate   (device, FUNC(write_method))
    writeNNm_delegate  (device, FUNC(write_method_m))
    writeNNmo_delegate (device, FUNC(write_method_mo))
    writeNNs_delegate  (device, FUNC(write_method_s))
    writeNNsm_delegate (device, FUNC(write_method_sm))
    writeNNsmo_delegate(device, FUNC(write_method_smo))

To be added to a map, a method call and the device it is called onto
have to be wrapped in the appropriate delegate type.  There are twelve
types, for read and for write and for all six possible prototypes.
Note that as all delegates, they can also wrap lambdas.

.. code-block:: C++

    space.install_read_handler(addrstart, addrend, read_delegate, unitmask, cswidth, flags)
    space.install_read_handler(addrstart, addrend, addrmask, addrmirror, addrselect, read_delegate, unitmask, cswidth, flags)
    space.install_write_handler(addrstart, addrend, write_delegate, unitmask, cswidth, flags)
    space.install_write_handler(addrstart, addrend, addrmask, addrmirror, addrselect, write_delegate, unitmask, cswidth, flags)
    space.install_readwrite_handler(addrstart, addrend, read_delegate, write_delegate, unitmask, cswidth, flags)
    space.install_readwrite_handler(addrstart, addrend, addrmask, addrmirror, addrselect, read_delegate, write_delegate, unitmask, cswidth, flags)

These six methods allow to install delegate-wrapped handlers in a live
address space. Either plain or with mask, mirror and select.  In the
read/write case both delegates must be of the same flavor (``smo``
stuff) to avoid a combinatorial explosion of method types.  The
``unitmask``, ``cswidth`` and ``flags`` arguments are optional.

5.3 Direct memory range mapping
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    space.install_rom(addrstart, addrend, void *pointer)
    space.install_rom(addrstart, addrend, addrmirror, void *pointer)
    space.install_rom(addrstart, addrend, addrmirror, flags, void *pointer)
    space.install_writeonly(addrstart, addrend, void *pointer)
    space.install_writeonly(addrstart, addrend, addrmirror, void *pointer)
    space.install_writeonly(addrstart, addrend, addrmirror, flags, void *pointer)
    space.install_ram(addrstart, addrend, void *pointer)
    space.install_ram(addrstart, addrend, addrmirror, void *pointer)
    space.install_ram(addrstart, addrend, addrmirror, flags, void *pointer)

Installs a memory block in an address space, with or without mirror
and flags.  ``_rom`` is read-only, ``_ram`` is read/write,
``_writeonly`` is write-only.  The pointer must be non-null, this
method will not allocate the memory.

5.4 Bank mapping
~~~~~~~~~~~~~~~~

.. code-block:: C++

    space.install_read_bank(addrstart, addrend, memory_bank *bank)
    space.install_read_bank(addrstart, addrend, addrmirror, memory_bank *bank)
    space.install_read_bank(addrstart, addrend, addrmirror, flags, memory_bank *bank)
    space.install_write_bank(addrstart, addrend, memory_bank *bank)
    space.install_write_bank(addrstart, addrend, addrmirror, memory_bank *bank)
    space.install_write_bank(addrstart, addrend, addrmirror, flags, memory_bank *bank)
    space.install_readwrite_bank(addrstart, addrend, memory_bank *bank)
    space.install_readwrite_bank(addrstart, addrend, addrmirror, memory_bank *bank)
    space.install_readwrite_bank(addrstart, addrend, addrmirror, flags, memory_bank *bank)

Install an existing memory bank for reading, writing or both in an
address space.

5.5 Port mapping
~~~~~~~~~~~~~~~~

.. code-block:: C++

    space.install_read_port(addrstart, addrend, const char *rtag)
    space.install_read_port(addrstart, addrend, addrmirror, const char *rtag)
    space.install_read_port(addrstart, addrend, addrmirror, flags, const char *rtag)
    space.install_write_port(addrstart, addrend, const char *wtag)
    space.install_write_port(addrstart, addrend, addrmirror, const char *wtag)
    space.install_write_port(addrstart, addrend, addrmirror, flags, const char *wtag)
    space.install_readwrite_port(addrstart, addrend, const char *rtag, const char *wtag)
    space.install_readwrite_port(addrstart, addrend, addrmirror, const char *rtag, const char *wtag)
    space.install_readwrite_port(addrstart, addrend, addrmirror, flags, const char *rtag, const char *wtag)

Install ports by name for reading, writing or both.

5.6 Dropped accesses
~~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    space.nop_read(addrstart, addrend, addrmirror, flags)
    space.nop_write(addrstart, addrend, addrmirror, flags)
    space.nop_readwrite(addrstart, addrend, addrmirror, flags)

Drops the accesses for a given range with an optional mirror and flags;

5.7 Unmapped accesses
~~~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    space.unmap_read(addrstart, addrend, addrmirror, flags)
    space.unmap_write(addrstart, addrend, addrmirror, flags)
    space.unmap_readwrite(addrstart, addrend, addrmirror, flags)

Unmaps the accesses (e.g. logs the access as unmapped) for a given range
with an optional mirror and flags.

5.8 Device map installation
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    space.install_device(addrstart, addrend, device, map, unitmask, cswidth, flags)

Install a device address with an address map in a space.  The
``unitmask``, ``cswidth`` and ``flags`` arguments are optional.

5.9 View installation
~~~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    space.install_view(addrstart, addrend, view)
    space.install_view(addrstart, addrend, addrmirror, view)

    view[0].install...

Installs a view in a space.  This can be only done once and in only
one space, and the view must not have been setup through the address
map API before.  Once the view is installed, variants can be selected
by indexing to call a dynamic mapping method on it.

A view can be installed into a variant of another view without issues,
with only the usual constraint of single installation.
