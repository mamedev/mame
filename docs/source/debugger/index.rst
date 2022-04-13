.. _debugger:

MAME Debugger
=============

.. contents:: :local:


.. _debugger-intro:

Introduction
------------

MAME includes an interactive low-level debugger that targets the
emulated system.  This can be a useful tool for diagnosing emulation
issues, developing software to run on vintage systems, creating cheats,
ROM hacking, or just investigating how software works.

Use the ``-debug`` command line option to start MAME with the debugger
activated.  By default, pressing the backtick/tilde (**~**) during
emulation breaks into the debugger (this can by changed by reassigning
the **Break in Debugger** input).

The exact appearance of the debugger depends on your operating system
and the options MAME was built with.  All variants of the debugger
provide a multi-window interface for viewing the contents of memory and
disassembled code.

The debugger console window is a special window that shows the contents
of CPU registers and disassembled code around the current program
counter address, and provides a command-line interface to most of the
debugging functionality.


.. _debugger-sections-list:

Debugger commands
-----------------

Debugger commands are described in the sections below.  You can also
type **help <topic>** in the debugger console, where **<topic>** is the
name of a command, to see documentation directly in MAME.

.. toctree::
    :titlesonly:

    general
    memory
    execution
    breakpoint
    watchpoint
    registerpoints
    annotation
    cheats
    image


.. _debugger-devicespec:

Specifying devices and address spaces
-------------------------------------

Many debugger commands accept parameters specifying which device to
operate on.  If a device is not specified explicitly, the CPU currently
visible in the debugger is used.  Devices can be specified by tag, or by
CPU number:

* Tags are the colon-separated paths that MAME uses to identify devices
  within a system.  You see them in options for configuring slot
  devices, in debugger disassembly and memory viewer source lists, and
  various other places within MAME’s UI.
* CPU numbers are monotonically incrementing numbers that the debugger
  assigns to CPU-like devices within a system, starting at zero.  The
  **cpunum** symbol holds the CPU number for the currently visible CPU
  in the debugger (you can see it by entering the command
  **print cpunum** in the debugger console).

If a tag starts with a caret (**^**) or dot (**.**), it is interpreted
relative to the CPU currently visible in the debugger, otherwise it is
interpreted relative to the root machine device.  If a device argument
is ambiguously valid as both a tag and a CPU number, it will be
interpreted as a tag.

Examples:

``maincpu``
    The device with the absolute tag ``:maincpu``.
``^melodypsg``
    The sibling device of the visible CPU with the tag ``melodypsg``.
``.:adc``
    The child device of the visible CPU with the tag ``adc``.
``2``
    The third CPU-like device in the system (zero-based index).

Commands that operate on memory extend this by allowing the device tag
or CPU number to be optionally followed by an address space identifier.
Address space identifiers are tag-like strings.  You can see them in
debugger memory viewer source lists.  If the address space identifier is
omitted, a default address space will be used.  Usually, this is the
address space that appears first for the device.  Many commands have
variants with **d**, **i** and **o** (data, I/O and opcodes) suffixes
that default to the address spaces at indices 1, 2 and 3, respectively,
as these have special significance for CPU-like devices.

In ambiguous cases, the default address space of a child device will be
used rather than a specific address space.

Examples:

``ram``
    The default address space of the device with the absolute tag
    ``:ram``, or the ``ram`` space of the visible CPU.
``.:io``
    The default address space of the child device of the visible CPU
    with the tag ``io``, or the ``io`` space of the visible CPU.
``:program``
    The default address space of the device with the absolute tag
    ``:program``, or the ``program`` space of the root machine device.
``^vdp``
    The default address space of the sibling device of the visible CPU
    with the tag ``vdp``.
``^:data``
    The default address space of the sibling device of the visible CPU
    with the tag ``data``, or the ``data`` space of the parent device
    of the visible CPU.
``1:rom``
    The default address space of the child device of the second CPU in
    the system (zero-based index) with the tag ``rom``, or the ``rom``
    space of the second CPU in the system.
``2``
    The default address space of the third CPU-like device in the system
    (zero-based index).

If a command takes an emulated memory address as a parameter, the
address may optionally be followed by an address space specification, as
described above.

Examples:

``0220``
    Address 0220 in the default address space for the visible CPU.
``0378:io``
    Address 0378 in the default address space of the device with the
    absolute tag ``:io``, or the ``io`` space of the visible CPU.
``1234:.:rom``
    Address 1234 in the default address space of the child device of the
    visible CPU with the tag ``:rom``, or the ``rom`` space of the
    visible CPU.
``1260:^vdp``
    Address 1260 in the default address space of the sibling device of
    the visible CPU with the tag ``vdp``.
``8008:^:data``
    Address 8008 in the default address space of the sibling device of
    the visible CPU with the tag ``data``, or the ``data`` space of the
    parent device of the visible CPU.
``9660::ram``
    Address 9660 in the default address space of the device with the
    absolute tag ``:ram``, or the ``ram`` space of the root machine
    device.

The examples here include a lot of corner cases, but in general the
debugger should take the most likely meaning for a device or address
space specification.


.. _debugger-express:

Debugger expression syntax
--------------------------

Expressions can be used anywhere a numeric or Boolean parameter is
expected.  The syntax for expressions is similar to a subset of C-style
expression syntax, with full operator precedence and parentheses.  There
are a few operators missing (notably the ternary conditional operator),
and a few new ones (memory accessors).

The table below lists all the operators, ordered from highest to lowest
precedence:

``(`` ``)``
    Standard parentheses
``++`` ``--``
    Postfix increment/decrement
``++`` ``--`` ``~`` ``!`` ``-`` ``+`` ``b@`` ``w@`` ``d@`` ``q@`` ``b!`` ``w!`` ``d!`` ``q!``
    Prefix increment/decrement, binary complement, logical complement,
    unary identity/negation, memory access
``*`` ``/`` ``%``
    Multiplication, division, modulo
``+`` ``-``
    Addition, subtraction
``<<`` ``>>``
    Bitwise left/right shift
``< ``<=`` ``>`` ``>=``
    Less than, less than or equal, greater than, greater than or equal
``==`` ``!=``
    Equal, not equal
``&``
    Bitwise intersection (and)
``^``
    Bitwise exclusive or
``|``
    Bitwise union (or)
``&&``
    Logical conjunction (and)
``||``
    Logical disjunction (or)
``=`` ``*=`` ``/=`` ``%=`` ``+=`` ``-=`` ``<<=`` ``>>=`` ``&=`` ``|=`` ``^=``
    Assignment and modifying assignment
``,``
    Separate terms, function parameters

Major differences from C expression semantics:

* All numbers are unsigned 64-bit values.  In particular, this means
  negative numbers are not possible.
* The logical conjunction and disjunction operators ``&&`` and ``||`` do
  not have short-circuit properties – both sides of the expression are
  always evaluated.


.. _debugger-express-num:

Numbers
~~~~~~~

Literal numbers are prefixed according to their bases:

* Hexadecimal (base-16) with ``$`` or ``0x``
* Decimal (base-10) with ``#``
* Octal (base-8) with ``0o``
* Binary (base-2) with ``0b``
* Unprefixed numbers are hexadecimal (base-16).

Examples:

* ``123`` is 123 hexadecimal (291 decimal)
* ``$123`` is 123 hexadecimal (291 decimal)
* ``0x123`` is 123 hexadecimal (291 decimal)
* ``#123`` is 123 decimal
* ``0o123`` is 123 octal (83 decimal)
* ``0b1001`` is is 1001 binary (9 decimal)
* ``0b123`` is invalid


.. _debugger-express-bool:

Boolean values
~~~~~~~~~~~~~~

Any expression that evaluates to a number can be used where a Boolean
value is required.  Zero is treated as false, and all non-zero values
are treated as true.  Additionally, the string ``true`` is treated as
true, and the string ``false`` is treated as false.

An empty string may be supplied as an argument for Boolean parameters to
debugger commands to use the default value, even when subsequent
parameters are specified.


.. _debugger-express-mem:

Memory accesses
~~~~~~~~~~~~~~~

The memory access prefix operators allow reading from and writing to
emulated address spaces.  The memory prefix operators specify the
access size and whether side effects are disabled, and may optionally be
preceded by an address space specification.  The supported access sizes
and side effect modes are as follows:

* ``b`` specifies an 8-bit access (byte)
* ``w`` specifies a 16-bit access (word)
* ``d`` specifies a 32-bit access (double word)
* ``q`` specifies a 64-bit access (quadruple word)
* ``@`` suppress side effects
* ``!`` do not suppress side effects

Suppressing side effects of a read access yields the value reading from
address would, with no further effects.  For example reading a mailbox
with side effects disabled will not clear the pending flag, and reading
a FIFO with side effects disabled will not remove an item.

For write accesses, suppressing side effects doesn’t change behaviour in
most cases – you want to see the effects of writing to a location.
However, there are some exceptions where it is useful to separate
multiple effects of a write access.  For example:

* Some registers need to be written in sequence to avoid race
  conditions.  The debugger can issue multiple writes at the same point
  in emulated time, so these race conditions can be avoided trivially.
  For example writing to the MC68HC05 output compare register high byte
  (OCRH) inhibits compare until the output compare register low byte
  (OCRL) is written to prevent race conditions.  Since the debugger can
  write to both locations at the same instant from the emulated
  machine’s point of view, the race condition is not usually relevant.
  It’s more error-prone if you can accidentally set hidden state when
  all you really want to do is change the value, so writing to OCRH with
  side effects suppressed does not inhibit compare, it just changes the
  value in the output compare register.
* Writing to some registers has multiple effects that may be useful to
  separate for debugging purposes.  Using the MC68HC05 as an example
  again, writing to OCRL changes the value in the output compare
  register, and also clears the output compare flag (OCF) and enables
  compare if it was previously inhibited by writing to OCRH.  Writing to
  OCRL with side effects disabled just changes the value in the register
  without clearing OCF or enabling compare, since it’s useful for
  debugging purposes.  Writing to OCRL with side effects enabled has the
  additional effects.

The size may optionally be preceded by an access type specification:

* ``p`` or ``lp`` specifies a logical address defaulting to space 0
  (program)
* ``d`` or ``ld`` specifies a logical address defaulting to space 1
  (data)
* ``i`` or ``li`` specifies a logical address defaulting to space 2
  (I/O)
* ``3`` or ``l3`` specifies a logical address defaulting to space 3
  (opcodes)
* ``pp`` specifies a physical address defaulting to space 0 (program)
* ``pd`` specifies a physical address defaulting to space 1 (data)
* ``pi`` specifies a physical address defaulting to space 2 (I/O)
* ``p3`` specifies a physical address defaulting to space 3 (opcodes)
* ``r`` specifies direct read/write pointer access defaulting to space 0
  (program)
* ``o`` specifies direct read/write pointer access defaulting to space 3
  (opcodes)
* ``m`` specifies a memory region

Finally, this may be preceded by a tag and/or address space name
followed by a dot (``.``).

That may seem like a lot to digest, so let’s look at the simplest
examples:

``b@<addr>``
    Refers to the byte at **<addr>** in the program space of the current
    CPU while suppressing side effects
``b!<addr>``
    Refers to the byte at **<addr>** in the program space of the current
    CPU, *not* suppressing side effects such as reading a mailbox
    clearing the pending flag, or reading a FIFO removing an item
``w@<addr>`` and ``w!<addr>``
    Refer to the word at **<addr>** in the program space of the current
    CPU, suppressing or not suppressing side effects, respectively.
``d@<addr>`` and ``d!<addr>``
    Refer to the double word at **<addr>** in the program space of the
    current CPU, suppressing or not suppressing side effects,
    respectively.
``q@<addr>`` and ``q!<addr>``
    Refer to the quadruple word at **<addr>** in the program space of
    the current CPU, suppressing or not suppressing side effects,
    respectively.

Adding access types gives additional possibilities:

``dw@300``
    Refers to the word at 300 in the data space of the current CPU while
    suppressing side effects
``id@400``
    Refers to the double word at 400 in the I/O space of the current CPU
    CPU while suppressing side effects
``ppd!<addr>``
    Refers to the double word at physical address **<addr>** in the
    program space of the current CPU while not suppressing side effects
``rw@<addr>``
    Refers to the word at address **<addr>** in the program space of the
    current CPU using direct read/write pointer access

If we want to access an address space of a device other than the current
CPU, an address space beyond the first four indices, or a memory region,
we need to include a tag or name:

``ramport.b@<addr>``
    Refers to the byte at address **<addr>** in the ``ramport`` space of
    the current CPU
``audiocpu.dw@<addr>``
    Refers to the word at address **<addr>** in the data space of the
    CPU with absolute tag ``:audiocpu``
``maincpu:status.b@<addr>``
    Refers to the byte at address **<addr>** in the ``status`` space of
    the CPU with the absolute tag ``:maincpu``
``monitor.mb@78``
    Refers to the byte at 78 in the memory region with the absolute tag
    ``:monitor``
``..md@202``
    Refers to the double word at address 202 in the memory region with
    the same tag path as the current CPU.

Some combinations are not useful.  For example physical and logical
addresses are equivalent for some CPUs, and direct read/write pointer
accesses never have side effects.  Accessing a memory region (``m``
access type) requires a tag to be specified.

Memory accesses can be used as both lvalues and rvalues, so you can write
``b@100 = ff`` to store a byte in memory.


.. _debugger-express-func:

Functions
~~~~~~~~~

The debugger supports a number of useful utility functions in expressions.

min(<a>, <b>)
    Returns the lesser of the two arguments.
max(<a>, <b>)
    Returns the greater of the two arguments.
if(<cond>, <trueval>, <falseval>)
    Returns **<trueval>** if **<cond>** is true (non-zero), or
    **<falseval>** otherwise.  Note that the expressions for
    **<trueval>** and **<falseval>** are both evaluated irrespective of
    whether **<cond>** is true or false.
abs(<x>)
    Reinterprets the argument as a 64-bit signed integer and returns the
    absolute value.
bit(<x>, <n>[, <w>])
    Extracts and right-aligns a bit field **<w>** bits wide from **<x>**
    with least significant bit position position **<n>**, counting from
    the least significant bit.  If **<w>** is omitted, a single bit is
    extracted.
s8(<x>)
    Sign-extends the argument from 8 bits to 64 bits (overwrites bits 8
    through 63, inclusive, with the value of bit 7, counting from the
    least significant bit).
s16(<x>)
    Sign-extends the argument from 16 bits to 64 bits (overwrites bits
    16 through 63, inclusive, with the value of bit 15, counting from
    the least significant bit).
s32(<x>)
    Sign-extends the argument from 32 bits to 64 bits (overwrites bits
    32 through 63, inclusive, with the value of bit 31, counting from
    the least significant bit).
