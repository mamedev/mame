The device_memory_interface
===========================

1. Capabilities
---------------

The device memory interface provides devices with the capability of
creating address spaces, to which address maps can be associated.
It's used for any device that provides a (logically) address/data bus
other devices can be connected to.  It's mainly, but not only, cpus.

The interface allows for up to four address spaces, numbered 0-3, with
symbolic names associated to them in emumem.h for historical reasons.

+------------+-------------+----------------------+
| Numeric ID | Symbolic ID | Symbolic name        |
+============+=============+======================+
| 0          | AS_0        | AS_PROGRAM           |
+------------+-------------+----------------------+
| 1          | AS_1        | AS_DATA              |
+------------+-------------+----------------------+
| 2          | AS_2        | AS_IO                |
+------------+-------------+----------------------+
| 3          | AS_3        | AS_DECRYPTED_OPCODES |
+------------+-------------+----------------------+

2. Setup
--------

| const address_space_config *\ **memory_space_config**\ (address_spacenum spacenum) const

The device must override that method to provide, for each of the four
address spaces, either an **address_space_config** describing the
space's configucation or **nullptr** if the space is not to be
created.

| bool **has_configured_map**\ () const
| bool **has_configured_map**\ (int index) const
| bool **has_configured_map**\ (address_spacenum index) const

The **has_configured_map** method allows to test in the
**memory_space_config** method whether an **address_map** has been
associated with a given space.  That allows to implement optional
memory spaces, such as AS_DECRYPTED_OPCODES in certain cpu cores.  The
parameterless version tests for AS_PROGRAM/AS_0.

3. Associating maps to spaces
-----------------------------
Associating maps to spaces is done at the machine config level, after the device declaration.

| **MCFG_DEVICE_ADDRESS_MAP**\ (_space, _map)
| **MCFG_DEVICE_PROGRAM_MAP**\ (_map)
| **MCFG_DEVICE_DATA_MAP**\ (_map)
| **MCFG_DEVICE_IO_MAP**\ (_map)
| **MCFG_DEVICE_DECRYPTED_OPCODES_MAP**\ (_map)

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
| address_space &\ **space**\ (address_spacenum index) const

Returns a given address space post-initialization.  The parameterless
version tests for AS_PROGRAM/AS_0.  Aborts if the space doesn't exist.

| bool **has_space**\ () const
| bool **has_space**\ (int index) const
| bool **has_space**\ (address_spacenum index) const

Indicates whether a given space actually exists. The parameterless
version tests for AS_PROGRAM/AS_0.


5. Weird/to deprecate stuff
---------------------------

| bool **translate**\ (address_spacenum spacenum, int intention, offs_t &address)
| bool **read**\ (address_spacenum spacenum, offs_t offset, int size, UINT64 &value)
| bool **write**\ (address_spacenum spacenum, offs_t offset, int size, UINT64 value)
| bool **readop**\ (offs_t offset, int size, UINT64 &value)

These methods override how the debugger accesses memory for a cpu.
Avoid them if you can. Otherwise, prepare for heavy-duty spelunking in
complicated code.

If really required, should probably be part of cpu_device directly.
