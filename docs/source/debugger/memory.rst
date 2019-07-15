.. _debugger-memory-list:

Memory Debugger Commands
========================


You can also type **help <command>** for further details on each command in the MAME Debugger interface.

| :ref:`debugger-command-dasm` -- disassemble to the given file
| :ref:`debugger-command-find` -- search program memory, data memory, or I/O memory for data
| :ref:`debugger-command-dump` -- dump program memory, data memory, or I/O memory as text
| :ref:`debugger-command-save` -- save binary program, data, or I/O memory to the given file
| :ref:`debugger-command-load` -- load binary program memory, data memory, or I/O memory from the given file
| :ref:`debugger-command-map` -- map logical program, data, or I/O address to physical address and bank




 .. _debugger-command-dasm:

dasm
----

|  **dasm <filename>,<address>,<length>[,<opcodes>[,<cpu>]]**
|
| The dasm command disassembles program memory to the file specified in the <filename> parameter. <address> indicates the address of the start of disassembly, and <length> indicates how much memory to disassemble. The range <address> through <address>+<length>-1 inclusive will be output to the file. By default, the raw opcode data is output with each line. The optional <opcodes> parameter can be used to enable (1) or disable (0) this feature. Finally, you can disassemble code from another CPU by specifying the <cpu> parameter.
|
| Examples:
|
|  dasm venture.asm,0,10000
|
| Disassembles addresses 0-ffff in the current CPU, including raw opcode data, to the file 'venture.asm'.
|
|  dasm harddriv.asm,3000,1000,0,2
|
| Disassembles addresses 3000-3fff from CPU #2, with no raw opcode data, to the file 'harddriv.asm'.
|
| Back to :ref:`debugger-memory-list`


 .. _debugger-command-find:

find
----

|  **f[ind][{d|i}] <address>,<length>[,<data>[,...]]**
|
| The **find**/**findd**/**findi** commands search through memory for the specified sequence of data. 'find' will search program space memory, while 'findd' will search data space memory and 'findi' will search I/O space memory. <address> indicates the address to begin searching, and <length> indicates how much memory to search. <data> can either be a quoted string or a numeric value or expression or the wildcard character '?'. Strings by default imply a byte-sized search; non-string data is searched by default in the native word size of the CPU. To override the search size for non-strings, you can prefix the value with b. to force byte- sized search, w. for word-sized search, d. for dword-sized, and q. for qword-sized. Overrides are remembered, so if you want to search for a series of words, you need only to prefix the first value with a w. Note also that you can intermix sizes in order to perform more complex searches. The entire range <address> through <address>+<length>-1 inclusive will be searched for the sequence, and all occurrences will be displayed.
|
| Examples:
|
|  find 0,10000,"HIGH SCORE",0
|
| Searches the address range 0-ffff in the current CPU for the string "HIGH SCORE" followed by a 0 byte.
|
|  findd 3000,1000,w.abcd,4567
|
| Searches the data memory address range 3000-3fff for the word-sized value abcd followed by the word-sized value 4567.
|
|  find 0,8000,"AAR",d.0,"BEN",w.0
|
| Searches the address range 0000-7fff for the string "AAR" followed by a dword-sized 0 followed by the string "BEN", followed by a word-sized 0.
|
| Back to :ref:`debugger-memory-list`


 .. _debugger-command-dump:

dump
----

|  **dump[{d|i}] <filename>,<address>,<length>[,<size>[,<ascii>[,<cpu>]]]**
|
| The **dump**/**dumpd**/**dumpi** commands dump memory to the text file specified in the <filename> parameter.
| 'dump' will dump program space memory, while 'dumpd' will dump data space memory and 'dumpi' will dump I/O space memory.
| <address> indicates the address of the start of dumping, and <length> indicates how much memory to dump. The range <address> through <address>+<length>-1 inclusive will be output to the file.
| By default, the data will be output in byte format, unless the underlying address space is word/dword/qword-only. You can override this by specifying the <size> parameter, which can be used to group the data in 1, 2, 4 or 8-byte chunks.
| The optional <ascii> parameter can be used to enable (1) or disable (0) the output of ASCII characters to the right of each line; by default, this is enabled.
| Finally, you can dump memory from another CPU by specifying the <cpu> parameter.
|
|
| Examples:
|
|  dump venture.dmp,0,10000
|
| Dumps addresses 0-ffff in the current CPU in 1-byte chunks, including ASCII data, to the file 'venture.dmp'.
|
|  dumpd harddriv.dmp,3000,1000,4,0,3
|
| Dumps data memory addresses 3000-3fff from CPU #3 in 4-byte chunks, with no ASCII data, to the file 'harddriv.dmp'.
|
| Back to :ref:`debugger-memory-list`


 .. _debugger-command-save:

save
----

|  **save[{d|i}] <filename>,<address>,<length>[,<cpu>]**
|
| The **save**/**saved**/**savei** commands save raw memory to the binary file specified in the <filename> parameter.
| 'save' will save program space memory, while 'saved' will save data space memory and 'savei' will save I/O space memory.
| <address> indicates the address of the start of saving, and <length> indicates how much memory to save. The range <address> through <address>+<length>-1 inclusive will be output to the file.
| You can also save memory from another CPU by specifying the <cpu> parameter.
|
|
| Examples:
|
|  save venture.bin,0,10000
|
| Saves addresses 0-ffff in the current CPU to the binary file 'venture.bin'.
|
|  saved harddriv.bin,3000,1000,3
|
| Saves data memory addresses 3000-3fff from CPU #3 to the binary file 'harddriv.bin'.
|
| Back to :ref:`debugger-memory-list`


 .. _debugger-command-load:

load
----

|  **load[{d|i}] <filename>,<address>[,<length>,<cpu>]**
|
| The **load**/**loadd**/**loadi** commands load raw memory from the binary file specified in the <filename> parameter.
| 'load' will load program space memory, while 'loadd' will load data space memory and 'loadi' will load I/O space memory.
| <address> indicates the address of the start of saving, and <length> indicates how much memory to load. The range <address> through <address>+<length>-1 inclusive will be read in from the file.
| If you specify <length> = 0 or a length greater than the total length of the file it will load the entire contents of the file and no more.
| You can also load memory from another CPU by specifying the <cpu> parameter.
|
| NOTE: This will only actually write memory that is possible to overwrite in the Memory Window
|
|
| Examples:
|
|  load venture.bin,0,10000
|
| Loads addresses 0-ffff in the current CPU from the binary file 'venture.bin'.
|
|  loadd harddriv.bin,3000,1000,3
|
| Loads data memory addresses 3000-3fff from CPU #3 from the binary file 'harddriv.bin'.
|
| Back to :ref:`debugger-memory-list`


 .. _debugger-command-map:

map
---

|  **map[{d|i}] <address>**
|
| The map/mapd/mapi commands map a logical address in memory to the correct physical address, as well as specifying the bank.
| 'map' will map program space memory, while 'mapd' will map data space memory and 'mapi' will map I/O space memory.
|
| Example:
|
|  map 152d0
|
| Gives physical address and bank for logical address 152d0 in program memory
|
| Back to :ref:`debugger-memory-list`

