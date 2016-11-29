The device_rom_interface
========================

1. Capabilities
---------------

This interface is designed for devices which expect to have a rom
connected to them on a dedicated bus.  It's mostly designed for sound
chips.  Other devices types may be interested but other considerations
may make it impratical (graphics decode caching for instance).  The
interface provides the capability of either connecting a ROM_REGION,
connecting an ADDRESS_MAP or dynamically setting up a block of memory
as rom.  In the region/block cases, banking is automatically handled.

2. Setup
--------

| **device_rom_interface**\ (const machine_config &mconfig, device_t &device, UINT8 addrwidth, endianness_t endian = ENDIANNESS_LITTLE, UINT8 datawidth = 8)

The constructor of the interface wants, in addition to the standard
parameters, the address bus width of the dedicated bus.  In addition
the endianness (if not little endian or byte-sized bus) and data bus
width (if not byte) can be provided.

| **MCFG_DEVICE_ADDRESS_MAP**\ (AS_0, map)

Use that method at machine config time to provide an address map for
the bus to connect to.  It has priority over a rom region if one is
also present.

| **MCFG_DEVICE_ROM**\ (tag)

Used to select a rom region to use if a device address map is not
given.  Defaults to DEVICE_SELF, e.g. the device tag.

| **ROM_REGION**\ (length, tag, flags)

If a rom region with a tag as given with **MCFG_DEVICE_ROM** if
present, or identical to the device tag otherwise, is provided in the
rom description for the system, it will be automatically picked up as
the connected rom.  An address map has priority over the region if
present in the machine config.

| void **set_rom**\ (const void \*base, UINT32 size);

At any time post- **interface_pre_start**, a memory block can be
setup as the connected rom with that method.  It overrides any
previous setup that may have been provided.  It can be done multiple
times.

3. Rom access
-------------

| UINT8 **read_byte**\ (offs_t byteaddress)
| UINT16 **read_word**\ (offs_t byteaddress)
| UINT32 **read_dword**\ (offs_t byteaddress)
| UINT64 **read_qword**\ (offs_t byteaddress)

These methods provide read access to the connected rom.  Out-of-bounds
access results in standard unmapped read logerror messages.

4. Rom banking
--------------

If the rom region or the memory block in set_rom is larger than the
address bus, banking is automatically setup.

| void **set_rom_bank**\ (int bank)

That method selects the current bank number.

5. Caveats
----------

Using that interface makes the device derive from
**device_memory_interface**. If the device wants to actually use the
memory interface for itself, remember that AS_0/AS_PROGRAM is used by
the rom interface, and don't forget to upcall **memory_space_config**.
