The device_disasm_interface and the disassemblers
=================================================

1. Capabilities
---------------

The disassemblers are classes that provide disassembly and opcode
meta-information for the cpu cores and **unidasm**.  The
**device_disasm_interface** connects a cpu core with its disassembler.

2. The disassemblers
--------------------

2.1. Definition
~~~~~~~~~~~~~~~

A disassembler is a class that derives from
**util::disasm_interface**.  It then has two required methods to
implement, **opcode_alignment** and **disassemble**, and 6 optional,
**interface_flags**, **page_address_bits**, **pc_linear_to_real**,
**pc_real_to_linear**, and one with four possible variants,
**decrypt8/16/32/64**.


2.2. opcode_alignment
~~~~~~~~~~~~~~~~~~~~~

| u32 \ **opcode_alignment**\ () const

Returns the required alignment of opcodes by the cpu, in PC-units.  In
other words, the required alignment for the PC register of the cpu.
Tends to be 1 (almost everything), 2 (68000...), 4 (mips, ppc...),
which an exceptional 8 (tms 32082 parallel processor) and 16
(tms32010, instructions are 16-bits aligned and the PC targets bits).
It must be a power-of-two or things will break.

Note that processors like the tms32031 which have 32-bits instructions
but where the PC targets 32-bits values have an alignment of 1.

2.3. disassemble
~~~~~~~~~~~~~~~~

| offs_t \ **disassemble**\ (std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)

This is the method where the real work is done.  This method must
disassemble the instruction at address *pc* and write the result to
*stream*.  The values to decode are retrieved from the *opcode*
buffer.  A **data_buffer** object offers four accessor methods:

| u8  util::disasm_interface::data_buffer::\ **r8**\  (offs_t pc) const
| u16 util::disasm_interface::data_buffer::\ **r16**\ (offs_t pc) const
| u32 util::disasm_interface::data_buffer::\ **r32**\ (offs_t pc) const
| u64 util::disasm_interface::data_buffer::\ **r64**\ (offs_t pc) const

They read the data at a given address and take endianness and
nonlinear PCs for larger-than-bus-width accesses.  The debugger
variant also caches the read data in one block, so for that reason one
should not read data too far from the base pc (e.g. stay within 16K or
so, careful when trying to follow indirect accesses).

A number of CPUs have an external signal that splits fetches into an
opcode part and a parameter part.  This is for instance the M1 signal
of the z80 or the SYNC signal of the 6502.  Some systems present
different values to the cpu depending on whether that signal is
active, usually for protection purposes.  On these cpus the opcode
part should be read from the *opcode* buffer, and the parameter part
from the *params* buffer.  They will or will not be the same buffer
depending on the system itself.

The method returns the size of the instruction in PC units, with a
maximum of 65535.  In addition, if possible, the disassembler should
give some meta-information about the opcode by OR-ing in into the
result:

* **STEP_OVER** for subroutine calls or auto-decrementing loops.  If there is some delay slots, also OR with **step_over_extra**\ (n) where n is the number of instruction slots.
* **STEP_OUT** for the return-from-subroutine instructions

In addition, to indicated that these flags are supported, OR the
result with **SUPPORTED**\ .  An annoying number of disassemblers lies
about that support (e.g. they do a or with **SUPPORTED** without even
generating the **STEP_OVER** or **STEP_OUT** information).  Don't do
that, it breaks the step over/step out functionality of the debugger.

2.4. interface_flags
~~~~~~~~~~~~~~~~~~~~

| u32 **interface_flags**\ () const

That optional method indicates specifics of the disassembler.  Default
of zero is correct most of the time.  Possible flags, which need to be
OR-ed together, are:

* **NONLINEAR_PC**\ : stepping to the next opcode or the next byte of the opcode is not adding one to pc.  Used for old LFSR-based PCs.
* **PAGED**\ : PC wraps at a page boundary
* **PAGED2LEVEL**\ : not only PC wraps at some kind of page boundary, but there are two levels of paging
* **INTERNAL_DECRYPTION**\ : there is some decryption tucked between reading from AS_PROGRAM and the actual disassembler
* **SPLIT_DECRYPTION**\ : there is some decryption tucked between reading from AS_PROGRAM and the actual disassembler, and that decryption is different for opcodes and parameters

Note that in practice non-linear pc systems are also paged, that
**PAGED2LEVEL** implies **PAGED**, and that **SPLIT_DECRYPTION**
implies **DECRYPTION**.


2.5. pc_linear_to_real and pc_real_to_linear
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

| offs_t **pc_linear_to_real**\ (offs_t pc) const
| offs_t **pc_real_to_linear**\ (offs_t pc) const

These methods should be present only when **NONLINEAR_PC** is set in
the interface flags.  They must convert pc to and from a value to a
linear domain where the instruction parameters and next instruction
are reached by incrementing the value.  **pc_real_to_linear** converts
to that domain, **pc_linear_to_real** converts back from that domain.


2.6. page_address_bits
~~~~~~~~~~~~~~~~~~~~~~

| u32 **page_address_bits**\ () const

Present on when **PAGED** or **PAGED2LEVEL** is set, gives the number
of address bits in the lowest page.


2.7. page2_address_bits
~~~~~~~~~~~~~~~~~~~~~~~

| u32 **page2_address_bits**\ () const

Present on when **PAGED2LEVEL** is set, gives the number
of address bits in the upper page.

2.8. decryptnn
~~~~~~~~~~~~~~

| u8  **decrypt8**\  (u8  value, offs_t pc, bool opcode) const
| u16 **decrypt16**\ (u16 value, offs_t pc, bool opcode) const
| u32 **decrypt32**\ (u32 value, offs_t pc, bool opcode) const
| u64 **decrypt64**\ (u64 value, offs_t pc, bool opcode) const

One of these must be defined when **INTERNAL_DECRYPTION** or
**SPLIT_DECRYPTION** is set.  The chosen one is the one which takes
what **opcode_alignment** represents in bytes.

That method decrypts a given value read from address pc (from
AS_PROGRAM) and gives the result which will be passed to the
disassembler.  In the split decryption case, opcode indicates whether
we're in the opcode (true) or parameter (false) part of the
instruction.


3. Disassembler interface, device_disasm_interface
--------------------------------------------------

3.1. Definition
~~~~~~~~~~~~~~~

A CPU core derives from **device_disasm_interface** through
**cpu_device**\ .  One method has to be implemented,
**create_disassembler**\ .

3.2. create_disassembler
~~~~~~~~~~~~~~~~~~~~~~~~

| util::disasm_interface \*\ **create_disassembler**\ ()

That method must return a pointer to a newly allocated disassembler
object.  The caller takes ownership and handles the lifetime.

THis method will be called at most one in the lifetime of the cpu
object.

4. Disassembler configuration and communication
-----------------------------------------------

Some disassemblers need to be configured.  Configuration can be
unchanging (static) for the duration of the run (cpu model type for
instance) or dynamic (state of a flag or a user preference).  Static
configuration can be done through either (a) parameter(s) to the
disassembler constructor, or through deriving a main disassembler
class.  If the information is short and its semantics obvious (like a
model name), feel free to use a parameter.  Otherwise derive the
class.

Dynamic configuration must be done by first defining a nested public
struct called "config" in the disassembler, with virtual destructor
and pure virtual methods to pull the required information.  A pointer
to that struct should be passed to the disassembler constructor.  The
cpu core should then add a derivation from that config struct and
implement the methods.  Unidasm will have to derive a small class from
the config class to give the information.

5. Missing stuff
----------------

There currently is no way for the debugger GUI to add per-core
configuration.  It is needed for in particular the s2650 and the
saturn cores.  It should go through the cpu core class itself, since
it's pulled from the config struct.

There is support missing in unidasm for per-cpu configuration.  That's
needed for a lot of things, see the unidasm source code for the
current list ("Configuration missing" comments).
