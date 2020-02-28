The device_memory_interface
===========================

1. Capabilities
---------------

The device memory interface provides devices with the capability of
creating address spaces, to which address maps can be associated.
It's used for any device that provides a (logically) address/data bus
other devices can be connected to.  It's mainly, but not only, cpus.

The interface allows for an unlimited set of address spaces, numbered
with small positive values.  The IDs should stay small because they
index vectors to keep the lookup fast.  Spaces number 0-3 have an
associated constant name:

+----+---------------+
| ID | Name          |
+====+===============+
| 0  | AS_PROGRAM    |
+----+---------------+
| 1  | AS_DATA       |
+----+---------------+
| 2  | AS_IO         |
+----+---------------+
| 3  | AS_OPCODES    |
+----+---------------+

Spaces 0 and 3, e.g. AS_PROGRAM and AS_OPCODE, are special for the
debugger and some CPUs.  AS_PROGRAM is use by the debugger and the
cpus as the space from with the cpu reads its instructions for the
disassembler.  When present, AS_OPCODE is used by the debugger and
some cpus to read the opcode part of the instruction.  What opcode
means is device-dependant, for instance for the z80 it's the initial
byte(s) which are read with the M1 signal asserted.  For the 68000 is
means every instruction word plus the PC-relative accesses.  The main,
but not only, use of AS_OPCODE is to implement hardware decrypting
instructions separately from the data.

2. Setup
--------

| std::vector<std::pair<int, const address_space_config \*>>\ **memory_space_config**\ (int spacenum) const

The device must override that method to provide a vector of pairs
comprising of a space number and its associated
**address_space_config** describing its configuration.  Some examples
to look up when needed:

* Standard two-space vector: v60_device
* Conditional AS_OPCODE: z80_device
* Inherit config and add a space: m6801_device
* Inherit config and patch a space: tmpz84c011_device


| bool **has_configured_map**\ () const
| bool **has_configured_map**\ (int index) const

The **has_configured_map** method allows to test in the
**memory_space_config** method whether an **address_map** has been
associated with a given space.  That allows to implement optional
memory spaces, such as AS_OPCODES in certain cpu cores.  The
parameterless version tests for space 0.

3. Associating maps to spaces
-----------------------------
Associating maps to spaces is done at the machine config level, after the device declaration.

| **MCFG_DEVICE_ADDRESS_MAP**\ (_space, _map)
| **MCFG_DEVICE_PROGRAM_MAP**\ (_map)
| **MCFG_DEVICE_DATA_MAP**\ (_map)
| **MCFG_DEVICE_IO_MAP**\ (_map)
| **MCFG_DEVICE_OPCODES_MAP**\ (_map)

The generic macro and the four specific ones associate a map to a
given space. Address maps associated to non-existing spaces are
ignored (no warning given).  devcpu.h defines MCFG_CPU_*_MAP aliases
to the specific macros.

| **MCFG_DEVICE_REMOVE_ADDRESS_MAP**\ (_space)

That macro removes a memory map associated to a given space.  Useful
when removing a map for an optional space in a machine config
derivative.


4. Accessing the spaces
-----------------------

| address_space &\ **space**\ () const
| address_space &\ **space**\ (int index) const

Returns a given address space post-initialization.  The parameterless
version tests for AS_PROGRAM/AS_0.  Aborts if the space doesn't exist.

| bool **has_space**\ () const
| bool **has_space**\ (int index) const

Indicates whether a given space actually exists. The parameterless
version tests for AS_PROGRAM/AS_0.


5. MMU support for disassembler
-------------------------------

| bool **translate**\ (int spacenum, int intention, offs_t &address)

Does a logical to physical address translation through the device's
MMU.  spacenum gives the space number, intention the type of the
future access (TRANSLATE_(READ\|WRITE\|FETCH)(\|_USER\|_DEBUG)) and
address is an inout parameter with the address to translate and its
translated version.  Should return true if the translation went
correctly, false if the address is unmapped.

Note that for some historical reason the device itself must override
the virtual method **memory_translate** with the same signature.
