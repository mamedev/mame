The device_rom_interface
========================

.. contents:: :local:


1. Capabilities
---------------

This interface is designed for devices that expect to have a ROM
connected to them on a dedicated bus.  It’s mostly designed for sound
chips.  Other devices types may be interested but other considerations
may make it impractical (graphics decode caching, for instance).  The
interface provides the capability to connect a ROM region, connect an
address map, or dynamically set up a block of memory as ROM.  In the
region/memory block cases, banking is handled automatically.


2. Setup
--------

.. code-block:: C++

    device_rom_interface<AddrWidth, DataWidth=0, AddrShift=0, Endian=ENDIANNESS_LITTLE>

The interface is a template that takes the address width of the
dedicated bus as a parameter.  In addition the data bus width (if not
byte), address shift (if non-zero) and Endianness (if not little Endian
or byte-sized bus) can be provided.  Data bus width is 0 for byte, 1
for word, etc.

.. code-block:: C++

    void set_map(map);

Use that method at machine configuration time to provide an address map
for the bus to connect to.  It has priority over a ROM region if one is
also present.

.. code-block:: C++

    void set_device_rom_tag(tag);

Used to specify a ROM region to use if a device address map is not
given.  Defaults to ``DEVICE_SELF``, i.e. the device’s tag.

.. code-block:: C++

    ROM_REGION(length, tag, flags)

If a ROM region with the tag specified using ``set_device_rom_tag`` if
present, or identical to the device tag otherwise, is provided in the
ROM definitions for the system, it will be automatically picked up as
the connected ROM.  An address map has priority over the region if
present in the machine configuration.

.. code-block:: C++

    void override_address_width(u8 width);

This method allows the address bus width to be overridden. It must be
called from within the device before **config_complete** time.

.. code-block:: C++

    void set_rom(const void *base, u32 size);

At any time post-\ ``interface_pre_start``, a memory block can be
set up as the connected ROM with that method.  It overrides any
previous setup that may have been provided.  It can be done multiple
times.


3. ROM access
-------------

.. code-block:: C++

    u8 read_byte(offs_t addr);
    u16 read_word(offs_t addr);
    u32 read_dword(offs_t addr);
    u64 read_qword(offs_t addr);

These methods provide read access to the connected ROM.  Out-of-bounds
access results in standard unmapped read ``logerror`` messages.


4. ROM banking
--------------

If the ROM region or the memory block in ``set_rom`` is larger than the
address bus can access, banking is automatically set up.

.. code-block:: C++

    void set_rom_bank(int bank);

That method selects the current bank number.


5. Caveats
----------

Using that interface makes the device derive from
``device_memory_interface``.  If the device wants to actually use the
memory interface for itself, remember that space zero (0, or
``AS_PROGRAM``) is used by the ROM interface, and don’t forget to call
the base ``memory_space_config`` method.

For devices which have outputs that can be used to address ROMs but only
to forward the data to another device for processing, it may be helpful
to disable the interface when it is not required.  This can be done by
overriding ``memory_space_config`` to return an empty vector.
