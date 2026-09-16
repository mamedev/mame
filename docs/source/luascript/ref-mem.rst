.. _luascript-ref-mem:

Lua Memory System Classes
=========================

MAME’s Lua interface exposes various memory system objects, including address
spaces, memory shares, memory banks, and memory regions.  Scripts can read from
and write to the emulated memory system.

.. contents::
    :local:
    :depth: 1


.. _luascript-ref-memman:

Memory manager
--------------

Wraps MAME’s ``memory_manager`` class, which allows the memory shares, banks and
regions in a system to be enumerated.

Instantiation
~~~~~~~~~~~~~

manager.machine.memory
    Gets the global memory manager instance for the emulated system.

Properties
~~~~~~~~~~

memory.shares[]
    The :ref:`memory shares <luascript-ref-memshare>` in the system, indexed by
    absolute tag.  The ``at`` and ``index_of`` methods have O(n) complexity; all
    other supported operations have O(1) complexity.
memory.banks[]
    The :ref:`memory banks <luascript-ref-membank>` in the system, indexed by
    absolute tag.  The ``at`` and ``index_of`` methods have O(n) complexity; all
    other supported operations have O(1) complexity.
memory.regions[]
    The :ref:`memory regions <luascript-ref-memregion>` in the system, indexed
    by absolute tag.  The ``at`` and ``index_of`` methods have O(n) complexity;
    all other supported operations have O(1) complexity.


.. _luascript-ref-addrspace:

Address space
-------------

Wraps MAME’s ``address_space`` class, which represents an address space
belonging to a device.

Instantiation
~~~~~~~~~~~~~

manager.machine.devices[tag].spaces[name]
    Gets the address space with the specified name for a given device.  Note
    that names are specific to the device type.

Methods
~~~~~~~

space:read_i{8,16,32,64}(addr)
    Reads a signed integer value of the size in bits from the specified address.
space:read_u{8,16,32,64}(addr)
    Reads an unsigned integer value of the size in bits from the specified
    address.
space:write_i{8,16,32,64}(addr, val)
    Writes a signed integer value of the size in bits to the specified address.
space:write_u{8,16,32,64}(addr, val)
    Writes an unsigned integer value of the size in bits to the specified
    address.
space:readv_i{8,16,32,64}(addr)
    Reads a signed integer value of the size in bits from the specified virtual
    address.  The address is translated with the debug read intent.  Returns
    zero if address translation fails.
space:readv_u{8,16,32,64}(addr)
    Reads an unsigned integer value of the size in bits from the specified
    virtual address.  The address is translated with the debug read intent.
    Returns zero if address translation fails.
space:writev_i{8,16,32,64}(addr, val)
    Writes a signed integer value of the size in bits to the specified virtual
    address.  The address is translated with the debug write intent.  Does not
    write if address translation fails.
space:writev_u{8,16,32,64}(addr, val)
    Writes an unsigned integer value of the size in bits to the specified
    virtual address.  The address is translated with the debug write intent.
    Does not write if address translation fails.
space:read_direct_i{8,16,32,64}(addr)
    Reads a signed integer value of the size in bits from the specified address
    one byte at a time by obtaining a read pointer for each byte address.  If
    a read pointer cannot be obtained for a byte address, the corresponding
    result byte will be zero.
space:read_direct_u{8,16,32,64}(addr)
    Reads an unsigned integer value of the size in bits from the specified
    address one byte at a time by obtaining a read pointer for each byte
    address.  If a read pointer cannot be obtained for a byte address, the
    corresponding result byte will be zero.
space:write_direct_i{8,16,32,64}(addr, val)
    Writes a signed integer value of the size in bits to the specified address
    one byte at a time by obtaining a write pointer for each byte address.  If
    a write pointer cannot be obtained for a byte address, the corresponding
    byte will not be written.
space:write_direct_u{8,16,32,64}(addr, val)
    Writes an unsigned integer value of the size in bits to the specified
    address one byte at a time by obtaining a write pointer for each byte
    address.  If a write pointer cannot be obtained for a byte address, the
    corresponding byte will not be written.
space:read_range(start, end, width, [step])
    Reads a range of addresses as a binary string.  The end address must be
    greater than or equal to the start address.  The width must be 8, 16, 30 or
    64.  If the step is provided, it must be a positive number of elements.
space:add_change_notifier(callback)
    Add a callback to receive notifications for handler changes in address
    space.  The callback function is passed a single string as an argument,
    either ``r`` if read handlers have potentially changed, ``w`` if write
    handlers have potentially changed, or ``rw`` if both read and write handlers
    have potentially changed.

    Returns a :ref:`notifier subscription <luascript-ref-notifiersub>`.
space:install_read_tap(start, end, name, callback)
    Installs a :ref:`pass-through handler <luascript-ref-addrspacetap>` that
    will receive notifications on reads from the specified range of addresses in
    the address space.  The start and end addresses are inclusive.  The name
    must be a string, and the callback must be a function.  Returns the new
    pass-through handler.

    The callback is passed three arguments for the access offset, the data read,
    and the memory access mask.  The offset is the absolute offset into the
    address space.  To modify the data being read, return the modified value
    from the callback function as an integer.  If the callback does not return
    an integer, the data will not be modified.
space:install_write_tap(start, end, name, callback)
    Installs a :ref:`pass-through handler <luascript-ref-addrspacetap>` that
    will receive notifications on write to the specified range of addresses in
    the address space.  The start and end addresses are inclusive.  The name
    must be a string, and the callback must be a function.  Returns the new
    pass-through handler.

    The callback is passed three arguments for the access offset, the data
    written, and the memory access mask.  The offset is the absolute offset into
    the address space.  To modify the data being written, return the modified
    value from the callback function as an integer.  If the callback does not
    return an integer, the data will not be modified.

Properties
~~~~~~~~~~

space.name (read-only)
    The display name for the address space.
space.shift (read-only)
    The address granularity for the address space specified as the shift
    required to translate a byte address to a native address.  Positive values
    shift towards the most significant bit (left) and negative values shift
    towards the least significant bit (right).
space.index (read-only)
    The zero-based space index.  Some space indices have special meanings for
    the debugger.
space.address_mask (read-only)
    The address mask for the space.
space.data_width (read-only)
    The data width for the space in bits.
space.endianness (read-only)
    The Endianness of the space (``"big"`` or ``"little"``).
space.map (read-only)
    The configured :ref:`address map <luascript-ref-addrmap>` for the space or
    ``nil``.


.. _luascript-ref-addrspacetap:

Pass-through handler
--------------------

Tracks a pass-through handler installed in an
:ref:`address space <luascript-ref-addrspace>`.  A memory pass-through handler
receives notifications on accesses to a specified range of addresses, and can
modify the data that is read or written if desired.  Note that pass-through handler
callbacks are not run as coroutines.

Instantiation
~~~~~~~~~~~~~

manager.machine.devices[tag].spaces[name]:install_read_tap(start, end, name, callback)
    Installs a pass-through handler that will receive notifications on reads
    from the specified range of addresses in an
    :ref:`address space <luascript-ref-addrspace>`.
manager.machine.devices[tag].spaces[name]:install_write_tap(start, end, name, callback)
    Installs a pass-through handler that will receive notifications on writes to
    the specified range of addresses in an
    :ref:`address space <luascript-ref-addrspace>`.

Methods
~~~~~~~

passthrough:reinstall()
    Reinstalls the pass-through handler in the address space.  May be necessary
    if the handler is removed due to other changes to handlers in the address
    space.
passthrough:remove()
    Removes the pass-through handler from the address space.  The associated
    callback will not be called in response to future memory accesses.

Properties
~~~~~~~~~~

passthrough.addrstart (read-only)
    The inclusive start address of the address range monitored by the
    pass-through handler (i.e. the lowest address that the handler will be
    notified for).
passthrough.addrend (read-only)
    The inclusive end address of the address range monitored by the pass-through
    handler (i.e. the highest address that the handler will be notified for).
passthrough.name (read-only)
    The display name for the pass-through handler.


.. _luascript-ref-addrmap:

Address map
-----------

Wraps MAME’s ``address_map`` class, used to configure handlers for an address
space.

Instantiation
~~~~~~~~~~~~~

manager.machine.devices[tag].spaces[name].map
    Gets the configured address map for an address space, or ``nil`` if no map
    is configured.

Properties
~~~~~~~~~~

map.spacenum (read-only)
    The address space number of the address space the map is associated with.
map.device (read-only)
    The device that owns the address space the map is associated with.
map.unmap_value (read-only)
    The constant value to return from unmapped reads.
map.global_mask (read-only)
    Global mask to be applied to all addresses when accessing the space.
map.entries[] (read-only)
    The configured :ref:`entries <luascript-ref-addrmapentry>` in the address
    map.  Uses 1-based integer indices.  The index operator and the ``at``
    method have O(n) complexity.


.. _luascript-ref-addrmapentry:

Address map entry
-----------------

Wraps MAME’s ``address_map_entry`` class, representing an entry in a configured
address map.

Instantiation
~~~~~~~~~~~~~

manager.machine.devices[tag].spaces[name].map.entries[index]
    Gets an entry from the configured map for an address space.

Properties
~~~~~~~~~~

entry.address_start (read-only)
    Start address of the entry’s range.
entry.address_end (read-only)
    End address of the entry’s range (inclusive).
entry.address_mirror (read-only)
    Address mirror bits.
entry.address_mask (read-only)
    Address mask bits.  Only valid for handlers.
entry.mask (read-only)
    Lane mask, indicating which data lines on the bus are connected to the
    handler.
entry.cswidth (read-only)
    The trigger width for a handler that isn’t connected to all the data lines.
entry.read (read-only)
    :ref:`Additional data <luascript-ref-memhandlerdata>` for the read handler.
entry.write (read-only)
    :ref:`Additional data <luascript-ref-memhandlerdata>` for the write handler.
entry.share (read-only)
    Memory share tag for making RAM entries accessible or ``nil``.
entry.region (read-only)
    Explicit memory region tag for ROM entries, or ``nil``.  For ROM entries,
    ``nil`` infers the region from the device tag.
entry.region_offset (read-only)
    Starting offset in memory region for ROM entries.


.. _luascript-ref-memhandlerdata:

Address map handler data
------------------------

Wraps MAME’s ``map_handler_data`` class, which provides configuration data to
handlers in address maps.

Instantiation
~~~~~~~~~~~~~

manager.machine.devices[tag].spaces[name].map.entries[index].read
    Gets the read handler data for an address map entry.
manager.machine.devices[tag].spaces[name].map.entries[index].write
    Gets the write handler data for an address map entry.

Properties
~~~~~~~~~~

data.handlertype (read-only)
    Handler type.  Will be one of ``"none"``, ``"ram"``, ``"rom"``, ``"nop"``,
    ``"unmap"``, ``"delegate"``, ``"port"``, ``"bank"``, ``"submap"``, or
    ``"unknown"``.  Note that multiple handler type values can yield
    ``"delegate"`` or ``"unknown"``.
data.bits (read-only)
    Data width for the handler in bits.
data.name (read-only)
    Display name for the handler, or ``nil``.
data.tag (read-only)
    Tag for I/O ports and memory banks, or ``nil``.


.. _luascript-ref-memshare:

Memory share
------------

Wraps MAME’s ``memory_share`` class, representing a named allocated memory zone.

Instantiation
~~~~~~~~~~~~~

manager.machine.memory.shares[tag]
    Gets a memory share by absolute tag, or ``nil`` if no such memory share
    exists.
manager.machine.devices[tag]:memshare(tag)
    Gets a memory share by tag relative to a device, or ``nil`` if no such
    memory share exists.

Methods
~~~~~~~

share:read_i{8,16,32,64}(offs)
    Reads a signed integer value of the size in bits from the specified offset
    in the memory share.
share:read_u{8,16,32,64}(offs)
    Reads an unsigned integer value of the size in bits from the specified
    offset in the memory share.
share:write_i{8,16,32,64}(offs, val)
    Writes a signed integer value of the size in bits to the specified offset in
    the memory share.
share:write_u{8,16,32,64}(offs, val)
    Writes an unsigned integer value of the size in bits to the specified offset
    in the memory share.

Properties
~~~~~~~~~~

share.tag (read-only)
    The absolute tag of the memory share.
share.size (read-only)
    The size of the memory share in bytes.
share.length (read-only)
    The length of the memory share in native width elements.
share.endianness (read-only)
    The Endianness of the memory share (``"big"`` or ``"little"``).
share.bitwidth (read-only)
    The native element width of the memory share in bits.
share.bytewidth (read-only)
    The native element width of the memory share in bytes.


.. _luascript-ref-membank:

Memory bank
-----------

Wraps MAME’s ``memory_bank`` class, representing a named memory zone
indirection.

Instantiation
~~~~~~~~~~~~~

manager.machine.memory.banks[tag]
    Gets a memory region by absolute tag, or ``nil`` if no such memory bank
    exists.
manager.machine.devices[tag]:membank(tag)
    Gets a memory region by tag relative to a device, or ``nil`` if no such
    memory bank exists.

Properties
~~~~~~~~~~

bank.tag (read-only)
    The absolute tag of the memory bank.
bank.entry (read/write)
    The currently selected zero-based entry number.


.. _luascript-ref-memregion:

Memory region
-------------

Wraps MAME’s ``memory_region`` class, representing a memory region used to store
read-only data like ROMs or the result of fixed decryptions.

Instantiation
~~~~~~~~~~~~~

manager.machine.memory.regions[tag]
    Gets a memory region by absolute tag, or ``nil`` if no such memory region
    exists.
manager.machine.devices[tag]:memregion(tag)
    Gets a memory region by tag relative to a device, or ``nil`` if no such
    memory region exists.

Methods
~~~~~~~

region:read(offs, len)
    Reads up to the specified length in bytes from the specified offset in the
    memory region.  The bytes read will be returned as a string.  If the
    specified length extends beyond the end of the memory region, the returned
    string will be shorter than requested.  Note that the data will be in host
    byte order.
region:read_i{8,16,32,64}(offs)
    Reads a signed integer value of the size in bits from the specified offset
    in the memory region.  The offset is specified in bytes.  Reading beyond the
    end of the memory region returns zero.
region:read_u{8,16,32,64}(offs)
    Reads an unsigned integer value of the size in bits from the specified
    offset in the memory region.  The offset is specified in bytes.  Reading
    beyond the end of the memory region returns zero.
region:write_i{8,16,32,64}(offs, val)
    Writes a signed integer value of the size in bits to the specified offset in
    the memory region.  The offset is specified in bytes.  Attempting to write
    beyond the end of the memory region has no effect.
region:write_u{8,16,32,64}(offs, val)
    Writes an unsigned integer value of the size in bits to the specified offset
    in the memory region.  The offset is specified in bytes.  Attempting to
    write beyond the end of the memory region has no effect.

Properties
~~~~~~~~~~

region.tag (read-only)
    The absolute tag of the memory region.
region.size (read-only)
    The size of the memory region in bytes.
region.length (read-only)
    The length of the memory region in native width elements.
region.endianness (read-only)
    The Endianness of the memory region (``"big"`` or ``"little"``).
region.bitwidth (read-only)
    The native element width of the memory region in bits.
region.bytewidth (read-only)
    The native element width of the memory region in bytes.
