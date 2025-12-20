.. _debugger-memory-list:

Memory Debugger Commands
========================

:ref:`debugger-command-dasm`
    disassemble code to a file
:ref:`debugger-command-find`
    search emulated memory for data
:ref:`debugger-command-fill`
    fill emulated memory with specified pattern
:ref:`debugger-command-dump`
    dump emulated memory to a file as text
:ref:`debugger-command-strdump`
    dump delimited strings from emulated memory to a file
:ref:`debugger-command-save`
    save binary data from emulated memory to a file
:ref:`debugger-command-saver`
    save binary data from a memory region to a file
:ref:`debugger-command-load`
    load binary data from a file to emulated memory
:ref:`debugger-command-loadr`
    load binary data from a file to a memory region
:ref:`debugger-command-map`
    map a logical address to the corresponding physical address and
    handler
:ref:`debugger-command-memdump`
    dump current memory maps to a file


.. _debugger-command-dasm:

dasm
----

**dasm <filename>,<address>,<length>[,<opcodes>[,<CPU>]]**

Disassembles program memory to the file specified by the **<filename>**
parameter.  The **<address>** parameter specifies the address to start
disassembling from, and the **<length>** parameter specifies how much
memory to disassemble.  The range **<address>** through
**<address>+<length>-1**, inclusive, will be disassembled to the file.
By default, raw opcode data is output with each line.  The optional
**<opcodes>** parameter is a Boolean that enables disables this feature.
By default, program memory for the visible CPU is disassembled.  To
disassemble program memory for a different CPU, specify it using the
optional fifth parameter (see :ref:`debugger-devicespec` for details).

Examples:

``dasm venture.asm,0,10000``
    Disassembles addresses 0-ffff for the visible CPU, including raw
    opcode data, to the file **venture.asm**.
``dasm harddriv.asm,3000,1000,0,2``
    Disassembles addresses 3000-3fff for the third CPU in the system
    (zero-based index), without raw opcode data, to the file
    **harddriv.asm**.

Back to :ref:`debugger-memory-list`


.. _debugger-command-find:

find
----

**f[ind][{d|i|o}] <address>[:<space>],<length>[,<data>[,…]]**
**f[ind] <address>:<memory>.{m|s},<length>[,<data>[,…]]**

Search through memory for the specified sequence of data.  The
**<address>** is the address to begin searching from, optionally
followed by a device and/or address space (see
:ref:`debugger-devicespec` for details); the **<length>** specifies
how much memory to search.    If an address space is not specified, the
command suffix sets the address space: ``find`` defaults to the first
address space exposed by the device, ``findd`` defaults to the space
with index 1 (data), ``findi`` default to the space with index 2 (I/O),
and ``findo`` defaults to the space with index 3 (opcodes).

The **<data>** can either be a quoted string, a numeric value or
expression, or the wildcard character ``?``.  By default, strings imply
a byte-sized search; by default non-string data is searched using the
native word size of the address space. To override the search size for
non-string data, you can prefix values with ``b.`` to force byte-sized
search, ``w.`` for word-sized search, ``d.`` for double word-sized
search, and ``q.`` for quadruple word-sized search.  Overrides propagate
to subsequent values, so if you want to search for a sequence of words,
you need only prefix the first value with ``w.``.  Also note that you
can intermix sizes to perform more complex searches.

The entire range **<address>** through **<address>+<length>-1**,
inclusive, will be searched for the sequence, and all occurrences will
be displayed.

Examples:

``find 0,10000,"HIGH SCORE",0``
    Searches the address range 0-ffff in the program space of the
    visible CPU for the string “HIGH SCORE” followed by a 0 byte.
``find 300:tms9918a,100,w.abcd,4567``
    Searches the address range 300-3ff in the first address space
    exposed by the device with the absolute tag ``:tms9918a`` for the
    word-sized value abcd followed by the word-sized value 4567.
``find 0,8000,"AAR",d.0,"BEN",w.0``
    Searches the address range 0000-7fff for the string “AAR” followed
    by a dword-sized 0 followed by the string “BEN”, followed by a
    word-sized 0.

Back to :ref:`debugger-memory-list`


.. _debugger-command-fill:

fill
----

**fill[{d|i|o}] <address>[:<space>],<length>[,<data>[,…]]**
**fill <address>:<memory>.{m|s},<length>[,<data>[,…]]**

Overwrite a block of memory with copies of the supplied data sequence.
The **<address>** specifies the address to begin writing at, optionally
followed by a device and/or address space (see
:ref:`debugger-devicespec` for details); the **<length>** specifies how
much memory to fill.  If an address space is not specified, the command
suffix sets the address space: ``fill`` defaults to the first
address space exposed by the device, ``filld`` defaults to the space
with index 1 (data), ``filli`` default to the space with index 2 (I/O),
and ``fillo`` defaults to the space with index 3 (opcodes).

The **<data>** can either be a quoted string, or a numeric value or
expression.  By default, non-string data is written using the native
word size of the address space. To override the data size for non-string
data, you can prefix values with ``b.`` to force byte-sized fill, ``w.``
for word-sized fill, ``d.`` for double word-sized fill, and ``q.`` for
quadruple word-sized fill. Overrides propagate to subsequent values, so
if you want to fill with a series of words, you need only prefix the
first value with ``w.``.   Also note that you can intermix sizes to
perform more complex fills.  The fill operation may be truncated if a
page fault occurs or if part of the sequence or string would fall beyond
**<address>+<length>-1**.

Back to :ref:`debugger-memory-list`


.. _debugger-command-dump:

dump
----

**dump[{d|i|o}] <filename>,<address>[:<space>],<length>[,<group>[,<ascii>[,<rowsize>]]]**
**dump <filename>,<address>:<memory>.{m|s},<length>[,<group>[,<ascii>[,<rowsize>]]]**

Dump memory to the text file specified by the **<filename>** parameter.
The **<address>** specifies the address to start dumping from,
optionally followed by a device and/or address space (see
:ref:`debugger-devicespec` for details); the **<length>** specifies how
much memory to dump.  If an address space is not specified, the command
suffix sets the address space: ``dump`` defaults to the first
address space exposed by the device, ``dumpd`` defaults to the space
with index 1 (data), ``dumpi`` default to the space with index 2 (I/O),
and ``dumpo`` defaults to the space with index 3 (opcodes).

The range **<address>** through **<address>+<length>-1**, inclusive,
will be output to the file.  By default, the data will be output using
the native word size of the address space.  You can override this by
specifying the **<group>** parameter, which can be used to group the
data in 1-, 2-, 4- or 8-byte chunks.  The optional **<ascii>** parameter
is a Boolean value used to enable or disable output of ASCII characters
on the right of each line (enabled by default).  The optional
**<rowsize>** parameter specifies the amount of data on each line in
address units (defaults to 16 bytes).

Examples:

``dump venture.dmp,0,10000``
    Dumps addresses 0-ffff from the program space of the visible CPU in
    1-byte chunks, including ASCII data, to the file **venture.dmp**.
``dumpd harddriv.dmp,3000:3,1000,4,0``
    Dumps data memory addresses 3000-3fff from the fourth CPU in the
    system (zero-based index) in 4-byte chunks, without ASCII data, to
    the file **harddriv.dmp**.
``dump vram.dmp,0:sms_vdp:videoram,4000,1,false,8``
    Dumps ``videoram`` space addresses 0000-3fff from the device with
    the absolute tag path ``:sms_vdp`` in 1-byte chunks, without ASCII
    data, with 8 bytes per line, to the file **vram.dmp**.

Back to :ref:`debugger-memory-list`


.. _debugger-command-strdump:

strdump
-------

**strdump[{d|i|o}] <filename>,<address>[:<space>],<length>[,<term>]**
**strdump <filename>,<address>:<memory>.{m|s},<length>[,<term>]**

Dump memory to the text file specified by the **<filename>** parameter.
The **<address>** specifies the address to start dumping from,
optionally followed by a device and/or address space (see
:ref:`debugger-devicespec` for details); the **<length>** specifies how
much memory to dump.  If an address space is not specified, the command
suffix sets the address space: ``strdump`` defaults to the first
address space exposed by the device, ``strdumpd`` defaults to the space
with index 1 (data), ``strdumpi`` default to the space with index 2
(I/O), and ``strdumpo`` defaults to the space with index 3 (opcodes).

By default, the data will be interpreted as a series of NUL-terminated
(ASCIIZ) strings, the dump will have one string per line, and C-style
escapes sequences will be used for bytes that do not represent printable
ASCII characters.  The optional **<term>** parameter can be used to
specify a different string terminator character.  If **<term>** equals
-0x80, the terminator is not a separate byte but 0x80 added to the last
character of each string.


Back to :ref:`debugger-memory-list`


.. _debugger-command-save:

save
----

**save[{d|i|o}] <filename>,<address>[:<space>],<length>**
**save <filename>,<address>:<memory>.{m|s},<length>**

Save raw memory to the binary file specified by the **<filename>**
parameter.  The **<address>** specifies the address to start saving
from, optionally followed by a device and/or address space (see
:ref:`debugger-devicespec` for details); the **<length>** specifies how
much memory to save.  If an address space is not specified, the command
suffix sets the address space: ``save`` defaults to the first address
space exposed by the device, ``saved`` defaults to the space with index
1 (data), ``savei`` default to the space with index 2 (I/O), and
``saveo`` defaults to the space with index 3 (opcodes).

The range **<address>** through **<address>+<length>-1**, inclusive,
will be output to the file.

Examples:

``save venture.bin,0,10000``
    Saves addresses 0-ffff from the program space of the current CPU to
    the binary file **venture.bin**
``saved harddriv.bin,3000:3,1000``
    Saves data memory addresses 3000-3fff from the fourth CPU in the
    system (zero-based index) to the binary file **harddriv.bin**.
``save vram.bin,0:sms_vdp:videoram,4000``
    Saves ``videoram`` space addresses 0000-3fff from the device with
    the absolute tag path ``:sms_vdp`` to the binary file **vram.bin**.

Back to :ref:`debugger-memory-list`


.. _debugger-command-saver:

saver
-----

**saver <filename>,<address>,<length>,<region>**

Save raw content of memory region specified by the **<region>**
parameter to the binary file specified by the **<filename>** parameter.
Regions tags follow the same rules as device tags (see
:ref:`debugger-devicespec`).  The **<address>** specifies the address to
start saving from, and the **<length>** specifies how much memory to
save.  The range **<address>** through **<address>+<length>-1**,
inclusive, will be output to the file.

Alternetevely use :ref:`debugger-command-save` syntax:
``save <filename>,<address>:<region>.m,<length>``

Examples:

``saver data.bin,200,100,:monitor``
    Saves ``:monitor`` region addresses 200-2ff to the binary file
    **data.bin**.
``saver cpurom.bin,1000,400,.``
    Saves addresses 1000-13ff from the memory region with the same tag
    as the visible CPU to the binary file **cpurom.bin**.

Back to :ref:`debugger-memory-list`


.. _debugger-command-load:

load
----

**load[{d|i|o}] <filename>,<address>[:<space>][,<length>]**
**load <filename>,<address>:<memory>.{m|s}[,<length>]**

Load raw memory from the binary file specified by the **<filename>**
parameter.  The **<address>** specifies the address to start loading to,
optionally followed by a device and/or address space (see
:ref:`debugger-devicespec` for details); the **<length>** specifies how
much memory to load.  If an address space is not specified, the command
suffix sets the address space: ``load`` defaults to the first address
space exposed by the device, ``loadd`` defaults to the space with index
1 (data), ``loadi`` default to the space with index 2 (I/O), and
``loado`` defaults to the space with index 3 (opcodes).

The range **<address>** through **<address>+<length>-1**, inclusive,
will be read in from the file.  If the **<length>** is omitted, if it is
zero, or if it is greater than the total length of the file, the entire
contents of the file will be loaded but no more.

Note that this has the same effect as writing to the address space from
a debugger memory view, or using the ``b@``, ``w@``, ``d@`` or ``q@``
memory accessors in debugger expressions.  Read-only memory will not be
overwritten, and writing to I/O addresses may have effects beyond
setting register values.

Examples:

``load venture.bin,0,10000``
    Loads addresses 0-ffff in the program space for the visible CPU from
    the binary file **venture.bin**.
``loadd harddriv.bin,3000,1000,3``
    Loads data memory addresses 3000-3fff in the program space for the
    fourth CPU in the system (zero-based index) from the binary file
    **harddriv.bin**.
``load vram.bin,0:sms_vdp:videoram``
    Loads the ``videoram`` space for the device with the absolute tag
    path ``:sms_vdp`` starting at address 0000 with the entire content
    of the binary file **vram.bin**.

Back to :ref:`debugger-memory-list`


.. _debugger-command-loadr:

loadr
-----

**loadr <filename>,<address>,<length>,<region>**

Load memory in the memory region specified by the **<region>** with raw
data from the binary file specified by the **<filename>** parameter.
Regions tags follow the same rules as device tags (see
:ref:`debugger-devicespec`).  The **<address>** specifies the address to
start loading to, and the
**<length>** specifies how much memory to load.  The range **<address>**
through **<address>+<length>-1**, inclusive, will be read in from the
file.  If the **<length>** is zero, or is greater than the total length
of the file, the entire contents of the file will be loaded but no more.

Alternetevely use :ref:`debugger-command-load` syntax:
``load <filename>,<address>:<region>.m[,<length>]``

Examples:

``loadr data.bin,200,100,:monitor``
    Loads ``:monitor`` region addresses 200-2ff from the binary file
    **data.bin**.
``loadr cpurom.bin,1000,400,.``
    Loads addresses 1000-13ff in the memory region with the same tag as
    the visible CPU from the binary file **cpurom.bin**.

Back to :ref:`debugger-memory-list`


.. _debugger-command-map:

map
---

**map[{d|i|o}] <address>[:<space>]**

Map a logical memory address to the corresponding physical address, as
well as reporting  the handler name.  The address may optionally be
followed by a colon and device and/or address space (see
:ref:`debugger-devicespec` for details).  If an address space is not
specified, the command suffix sets the address space: ``map`` defaults
to the first address space exposed by the device, ``mapd`` defaults to
the space with index 1 (data), ``mapi`` default to the space with index
2 (I/O), and ``mapo`` defaults to the space with index 3 (opcodes).

Examples:

``map 152d0``
    Gives the physical address and handler name for logical address
    152d0 in program memory for the visible CPU.
``map 107:sms_vdp``
    Gives the physical address and handler name for logical address 107
    in the first address space for the device with the absolute tag path
    ``:sms_vdp``.

Back to :ref:`debugger-memory-list`


.. _debugger-command-memdump:

memdump
-------

**memdump [<filename>,[<device>]]**

Dumps the current memory maps to the file specified by **<filename>**,
or **memdump.log** if omitted.  If **<device>** is specified (see
:ref:`debugger-devicespec`), only memory maps for the part of the device
tree rooted at this device will be dumped.

Examples:

``memdump mylog.log``
    Dumps memory maps for all devices in the system to the file
    **mylog.log**.
``memdump``
    Dumps memory maps for all devices in the system to the file
    **memdump.log**.
``memdump audiomaps.log,audiopcb``
    Dumps memory maps for the device with the absolute tag path
    ``:audiopcb`` and all its child devices to the file
    **audiomaps.log**.
``memdump mylog.log,1``
    Dumps memory maps for the second CPU device in the system
    (zero-based index) and all its child devices to the file
    **mylog.log**.

Back to :ref:`debugger-memory-list`
