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
emulation breaks into the debugger (this can be changed by reassigning
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
    exceptionpoint
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
``<`` ``<=`` ``>`` ``>=``
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
* ``0b1001`` is 1001 binary (9 decimal)
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
    while suppressing side effects
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
    with least significant bit position **<n>**, counting from the
    least significant bit.  If **<w>** is omitted, a single bit is
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


.. _srcdbg:

Source-level debugging
----------------------

Source-level debugging allows you to step through and reference symbols
from the original source of a program, rather than the disassembly.
This feature is intended for use
when emulating a vintage microcomputer and running software on that
emulated machine for which you have access to the original source code.
For example, if you are developing new software for a vintage machine,
source-level debugging allows you to view your own source code while
debugging.


.. _srcdbg_enable:

Enabling source-level debugging
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You will need to generate a :ref:`MAME Debugging Information File <srcdbg_mdi>`.
You can then enable source-level debugging by launching MAME with
the :ref:`-src_debug_info <mame-commandline-srcdbginfo>`
command-line option.  You may also want
to specify :ref:`-src_debug_search_path <mame-commandline-srcdbgsearchpath>`
and / or :ref:`-src_debug_prefix_map <mame-commandline-srcdbgprefixmap>`.

Once source-level debugging is enabled, you will then be able to
access the Options menu, Show Source command from
the main debugger window.  You may switch back and forth between source
and disassembly view, and open separate disassembly windows even
while the main window shows source.

The source view has a drop-list at the top for selecting which source
file to view.  The list is populated with source file paths from the
:ref:`MAME Debugging Information File <srcdbg_mdi>`.  When the debugger is paused,
you can change the selection to view the file you wish.  When stepping,
the selection is automatically updated to show the file associated
with the current PC address.


.. _srcdbg_bp:

Source-level breakpoints
~~~~~~~~~~~~~~~~~~~~~~~~

The expression evaluator includes syntax for specifying a source
file and line number.  This can be used in conjunction with the
:ref:`bpset <debugger-command-bpset>` command to set
a breakpoint on a particular line number,
rather than specifying the address manually.  Source-level symbols
evaluate to addresses, and so may also be used in ``bpset`` commands.

When specifying a
source file, you may specify either the full path to the source file
as it existed on the machine that built the binary, the full path
to the source file as it exists on the host running MAME, or any
non-ambiguous substring at the end of the full path (such as
just the filename).  The filename or path is surrounded by single
backticks `````, with the line number immediately following
the closing backtick.

Examples:

``bpset `c:\full\path\to\file.c`3``
    Set a breakpoint on the first instruction associated with file.c, line 3
``bpset `file.c`3``
    Set a breakpoint on the first instruction associated with file.c, line 3.
    Note the use of just the filename instead of a full path.  This is fine
    so long as it is unambiguous relative to the other paths present in the
    MAME Debugging Information File.
``print `to\file.c`3``
    Print the address of the first instruction associated with file.c, line 3.
    Note the use of the path *ending* rather than the full path.  This is fine
    so long as it is unambiguous.
``bpset Main``
    Set a breakpoint on the label named ``Main``

When the main window is showing source, you may also click on any source line
and hit F9 to set a breakpoint on that line.


.. _srcdbg_stepping:

Source-level stepping
~~~~~~~~~~~~~~~~~~~~~

The stepping commands ``step``, ``over``, and ``out`` have source-level debugging
versions :ref:`steps <debugger-command-steps>`,
:ref:`overs <debugger-command-overs>`, and
:ref:`outs <debugger-command-outs>`, respectively, which operate
at the source level rather than at the disassembly level.
To take ``step`` as an example, if
the PC points to an address associated with line 4, ``step`` (with
no parameters) will advance
to the next instruction, whereas ``steps`` will advance to the next instruction
that is associated with a source line other than 4.  When the original
source is assembly language, ``step`` and ``steps`` generally behave the same.
But when the original source is in a higher-level language like C or BASIC, ``steps``
executes the remainder of a block of instructions associated with
the current source line.

When executing Step Into, Step Over, and Step Out from the menu or keyboard
shortcuts, the behavior depends on what the main window is showing.
If the main window shows disassembly, then ``step``, ``over``, or ``out``
would be invoked.  If the main window shows source, 
``steps``, ``overs``, or ``outs``  would be invoked.


.. _srcdbg_symbols:

Source-level symbol evaluation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

MAME defines its own built-in symbols based on the current
CPU (e.g., register names) and from its global symbol table
(e.g., ``beamx``, ``frame``, names of built-in :ref:`functions <debugger-express-func>`, etc.).  
When source-level debugging is enabled, symbols from the source code will
be imported from the MAME Debugging Information File and added
to the list of symbols recognized by MAME's expression evaluator.

A symbol from the MAME Debugging Information File representing a source-code
variable evaluates to the variable's *address*, not its *value*.  So,
for example, a 16-bit C compiler that compiles ``int harry = 4;`` will
create a symbol named ``harry`` whose value is the address at which
the variable is stored.  To view the *value* you would issue a command
to the debugger console such as ``print w@harry``.

* **Symbol collisions and priority**: It is possible that source-level symbols
  present in the MAME Debugging Information File will conflict with built-in symbols.
  When source-level debugging is enabled, source-level symbols take precedence.  You
  may always force a reference to the built-in symbol by prefixing the symbol with ``ns\``
  ("not source").  For example, suppose the MAME Debugging Information File includes the
  source-level symbol ``y`` which conflicts with the symbol for the register ``y``.
  References to ``y`` will be interpreted as the source-level symbol ``y``.
  References to ``ns\y``, such as:

  * ``print ns\y``
  * ``print b@ns\y``
  * ``wpset ns\y``

  will be interpeted as the register ``y``.   
* **Case sensitivity**: When the debugger evaluates expressions, symbols
  are generally interpreted case-insensitively when source-level debugging
  is not active.  Many programming languages treat symbols
  case-sensitively.  This can lead to source-level symbols like ``Foo`` and ``foo``
  being present in the MAME Debugging Information File simultaneously.
  When source-level debugging is active and MAME's expression evaluator
  encounters a symbol, it will give precendence to
  a case-sensitive match.  If no such match is found, it will look for a
  case-insensitive match.


.. _srcdbg_offsets:

Address offsets
~~~~~~~~~~~~~~~

In some cases, the assembler or compiler does not know where the generated
code will be loaded at run-time.  For example, the program might be written
in position-independent code to allow an operating system to decide
where to load the code at run-time.  In such cases, the assembler or compiler
will be unable to provide the correct addresses in the MAME Debugging Information
File.  A build tool that supports run-time relocation could then start numbering
its addresses at 0, assuming that the operating system will apply the appropriate
offset when the binary is loaded.  Similarly, the MAME debugger may also
apply an offset to the addresses from the debugging info so that the
resulting addresses match where the code is loaded at run-time.  You can do this
in two ways:

* On the command-line, specify :ref:`-src_debug_offset <mame-commandline-srcdbgoffset>`
  with the offset to apply.
  This approach makes sense if the code to be debugged is reliably loaded
  at an offset you can predict.
* At any time during the debugging of the program, use the command :ref:`debugger-command-sdoffset`
  from the debugger console with the offset to apply.  This approach can be used by
  users or LUA scripts that need to inspect memory to determine where the program was
  loaded.  Any breakpoints set before ``sdoffset`` was executed will need to be removed
  and re-added so the new offset can be applied.  Any source-level symbols loaded
  from the MAME Debugging Information File will automatically evaluate to 
  values with the new offset the next time a command references them.


.. _srcdbg_mdi:

Generating MAME Debugging Information Files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

MAME Debugging Information Files (or ``.mdi`` files) are generated by
assemblers or compilers that target machines emulated by MAME.  Currently
``.mdi`` files adhere to a single binary format called "Simple".  In the
future, new formats may be created as the need arises.  The Simple format includes:

* Full or relative paths to the source files input to the build tool
* Mappings from source file and line numbers to blocks of 16-bit addresses where the
  corresponding instructions reside
* Mappings from symbol names to 16-bit addresses

    * These symbols can either be global or scoped
    * Scoped symbols can either have fixed values or values
      dependent on register values (e.g., stack local variables)

The recommended way for build tools to generate ``.mdi`` files is to use
the small static library ``libmame_srcdbg_static.a``.  The source code resides
in the MAME tree at ``src/lib/srcdbg``, and the library gets built to
a configuration-specific subdirectory, such as
``build/linux_gcc/bin/x64/Release/libmame_srcdbg_static.a``.  The header
file ``srcdbg_api.h`` includes declarations and documentation for the C
functions comprising the API, along with comments that describe how to use
them.

.. DANGER::
	**Tools must not rely on any functionality     
	other than that declared in** ``srcdbg_api.h`` 
	**and** ``srcdbg_format.h``. **Other files will   
	change without warning.**

Tools written in **C++** can include  ``srcdbg_api.h`` and link to
``libmame_srcdbg_static.a`` without any further makefile changes.  Since
the library's API is pure C, tools
written in **C** can also include ``srcdbg_api.h`` and link to
``libmame_srcdbg_static.a``, but will need to add the C++ standard library
to the link line (as the library's *implementation* is C++).  For example,
``cc -m64 -o mytool mytool.c -L/path/to/lib/dir -lmame_srcdbg_static -lstdc++``.
Note that ``-lstdc++`` must appear at the *end*.

Tools *not* written in C or C++ may be able to use the shared library
``libmame_srcdbg_shared.so`` or ``mame_srcdbg_shared.dll``, assuming the tool
is written in a language that supports interfacing with shared libraries.

.. admonition:: Linux shared library versioning

	On Linux the shared library follows the recommended versioning names, with
	the initial version of the library's *real name* being
	``libmame_srcdbg_shared.so.1.0`` and *soname* being ``libmame_srcdbg_shared.so.1``.
	MAME does not have a setup program, so the responsibility is on tools
	redistributing the shared library to perform the usual shared library installation
	steps.  For example, a tool might want to insulate itself from the machine's
	environment, and tuck its own copy of the shared library into a tool-specific
	folder, and use the ``-rpath`` linker option to declare where the tool can
	find ``libmame_srcdbg_shared.so`` at run-time.  Alternatively, a tool could
	install ``libmame_srcdbg_shared.so`` into a machine-wide folder like
	``/usr/lib`` and run ``sudo ldconfig`` to register the library and create
	a symbolic link from the soname to the real name.  In any case, the tool
	will need to manually create a symbolic link from the *linker name*
	``libmame_srcdbg_shared.so`` to either the soname
	(if ``ldconfig`` was run) or directly to the real
	name to ensure that the build-time linker can find ``libmame_srcdbg_shared.so``.
	Before proceeding with any of these options, it's recommended that you read up
	on Linux shared library versioning, for example:
	https://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html

If consuming neither the static nor shared version of the MAME srcdbg library is
feasible, tools may also manually generate the binary format directly.  The format
is defined in ``src/lib/srcdbg/srcdbg_format.h``.  Because this is error-prone,
tools should prefer using the static or shared library over generating the binary
format directly.


.. _srcdbg_dump:

Viewing MAME Debugging Information Files with srcdbgdump
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A small console executable, :ref:`srcdbgdump <othertools_srcdbgdump>` is built
along with other MAME tools.  It may be used to view the contents
of MAME Debugging Information files.  
