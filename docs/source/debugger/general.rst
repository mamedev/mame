.. _debugger-general-list:

General Debugger Commands
=========================


You can also type **help <command>** for further details on each command in the MAME Debugger interface.

| :ref:`debugger-command-do` -- evaluates the given expression
| :ref:`debugger-command-symlist` -- lists registered symbols
| :ref:`debugger-command-softreset` -- executes a soft reset
| :ref:`debugger-command-hardreset` -- executes a hard reset
| :ref:`debugger-command-print` -- prints one or more <item>s to the console
| :ref:`debugger-command-printf` -- prints one or more <item>s to the console using <format>
| :ref:`debugger-command-logerror` -- outputs one or more <item>s to the error.log
| :ref:`debugger-command-tracelog` -- outputs one or more <item>s to the trace file using <format>
| :ref:`debugger-command-tracesym` -- outputs one or more <item>s to the trace file
| history -- outputs a brief history of visited opcodes (**to fix: help missing for this command**)
| :ref:`debugger-command-trackpc` -- visually track visited opcodes [boolean to turn on and off, for the given CPU, clear]
| :ref:`debugger-command-trackmem` -- record which PC writes to each memory address [boolean to turn on and off, clear]
| :ref:`debugger-command-pcatmem` -- query which PC wrote to a given memory address for the current CPU
| :ref:`debugger-command-rewind` -- go back in time by loading the most recent rewind state
| :ref:`debugger-command-statesave` -- save a state file for the current driver
| :ref:`debugger-command-stateload` -- load a state file for the current driver
| :ref:`debugger-command-snap` -- save a screen snapshot.
| :ref:`debugger-command-source` -- reads commands from <filename> and executes them one by one
| :ref:`debugger-command-quit` -- exits MAME and the debugger


 .. _debugger-command-do:

do
--

|  **do <expression>**
|
| The do command simply evaluates the given <expression>. This is typically used to set or modify variables.
|
| Examples:
|
|   do pc = 0
|
| Sets the register 'pc' to 0.
|
| Back to :ref:`debugger-general-list`

 .. _debugger-command-symlist:

symlist
-------

|  **symlist [<cpu>]**
|
| Lists registered symbols. If <cpu> is not specified, then symbols in the global symbol table are displayed; otherwise, the symbols for <cpu>'s specific CPU are displayed. Symbols are listed alphabetically. Read-only symbols are flagged with an asterisk.
|
| Examples:
|
|  symlist
|
| Displays the global symbol table.
|
|  symlist 2
|
| Displays the symbols specific to CPU #2.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-softreset:

softreset
---------

|  **softreset**
|
| Executes a soft reset.
|
| Examples:
|
| softreset
|
| Executes a soft reset.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-hardreset:

hardreset
---------

|  **hardreset**
|
| Executes a hard reset.
|
| Examples:
|
| hardreset
|
| Executes a hard reset.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-print:

print
-----

|  **print <item>[,...]**
|
| The print command prints the results of one or more expressions to the debugger console as hexadecimal values.
|
| Examples:
|
|  print pc
|
| Prints the value of 'pc' to the console as a hex number.
|
|  print a,b,a+b
|
| Prints a, b, and the value of a+b to the console as hex numbers.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-printf:

printf
------

|  **printf <format>[,<item>[,...]]**
|
| The printf command performs a C-style printf to the debugger console. Only a very limited set of formatting options are available:
|
|  %[0][<n>]d -- prints <item> as a decimal value with optional digit count and zero-fill
|  %[0][<n>]x -- prints <item> as a hexadecimal value with optional digit count and zero-fill
|
| All remaining formatting options are ignored. Use %% together to output a % character. Multiple lines can be printed by embedding a \\n in the text.
|
| Examples:
|
|  printf "PC=%04X",pc
|
| Prints PC=<pcval> where <pcval> is displayed in hexadecimal with 4 digits with zero-fill.
|
|  printf "A=%d, B=%d\\nC=%d",a,b,a+b
|
| Prints A=<aval>, B=<bval> on one line, and C=<a+bval> on a second line.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-logerror:

logerror
--------

|  **logerror <format>[,<item>[,...]]**
|
| The logerror command performs a C-style printf to the error log. Only a very limited set of formatting options are available:
|
|  %[0][<n>]d -- logs <item> as a decimal value with optional digit count and zero-fill
|  %[0][<n>]x -- logs <item> as a hexadecimal value with optional digit count and zero-fill
|
| All remaining formatting options are ignored. Use %% together to output a % character. Multiple lines can be printed by embedding a \\n in the text.
|
| Examples:
|
|  logerror "PC=%04X",pc
|
| Logs PC=<pcval> where <pcval> is displayed in hexadecimal with 4 digits with zero-fill.
|
|  logerror "A=%d, B=%d\\nC=%d",a,b,a+b
|
| Logs A=<aval>, B=<bval> on one line, and C=<a+bval> on a second line.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-tracelog:

tracelog
--------

|  **tracelog <format>[,<item>[,...]]**
|
| The tracelog command performs a C-style printf and routes the output to the currently open trace file (see the 'trace' command for details). If no file is currently open, tracelog does nothing. Only a very limited set of formatting options are available. See the :ref:`debugger-command-printf` help for details.
|
| Examples:
|
|  tracelog "PC=%04X",pc
|
| Outputs PC=<pcval> where <pcval> is displayed in hexadecimal with 4 digits with zero-fill.
|
|  printf "A=%d, B=%d\\nC=%d",a,b,a+b
|
| Outputs A=<aval>, B=<bval> on one line, and C=<a+bval> on a second line.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-tracesym:

tracesym
--------

|  **tracesym <item>[,...]**
|
| The tracesym command prints the specified symbols and routes the output to the currently open trace file (see the 'trace' command for details). If no file is currently open, tracesym does nothing.
|
| Examples:
|
|  tracelog pc
|
| Outputs PC=<pcval> where <pcval> is displayed in the default format.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-trackpc:

trackpc
-------

|  **trackpc [<bool>,<cpu>,<bool>]**
|
| The trackpc command displays which program counters have already been visited in all disassembler windows. The first boolean argument toggles the process on and off. The second argument is a CPU selector; if no CPU is specified, the current CPU is automatically selected. The third argument is a boolean denoting if the existing data should be cleared or not.
|
| Examples:
|
|  trackpc 1
|
| Begin tracking the current CPU's pc.
|
|  trackpc 1, 0, 1
|
| Continue tracking pc on CPU 0, but clear existing track info.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-trackmem:

trackmem
--------

|  **trackmem [<bool>,<cpu>,<bool>]**
|
| The trackmem command logs the PC at each time a memory address is written to. The first boolean argument toggles the process on and off. The second argument is a CPU selector; if no CPU is specified, the current CPU is automatically selected. The third argument is a boolean denoting if the existing data should be cleared or not. Please refer to the pcatmem command for information on how to retrieve this data. Also, right clicking in a memory window will display the logged PC for the given address.
|
| Examples:
|
|  trackmem
|
| Begin tracking the current CPU's pc.
|
|  trackmem 1, 0, 1
|
| Continue tracking memory writes on CPU 0, but clear existing track info.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-pcatmem:

pcatmem
-------

|  **pcatmem(p/d/i) <address>[,<cpu>]**
|
| **pcatmemp <address>[,<cpu>]** -- query which PC wrote to a given program memory address for the current CPU
| **pcatmemd <address>[,<cpu>]** -- query which PC wrote to a given data memory address for the current CPU
| **pcatmemi <address>[,<cpu>]** -- query which PC wrote to a given I/O memory address for the current CPU (you can also query this info by right clicking in a memory window)
|
| The pcatmem command returns which PC wrote to a given memory address for the current CPU. The first argument is the requested address. The second argument is a CPU selector; if no CPU is specified, the current CPU is automatically selected. Right clicking in a memory window will also display the logged PC for the given address.
|
| Examples:
|
|  pcatmem 400000
|
| Print which PC wrote this CPU's memory location 0x400000.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-rewind:

rewind
------

|  **rewind[rw]**
|
| The rewind command loads the most recent RAM-based state. Rewind states, when enabled, are saved when "step", "over", or "out" command gets executed, storing the machine state as of the moment before actually stepping. Consecutively loading rewind states can work like reverse execution. Depending on which steps forward were taken previously, the behavior can be similar to GDB's "reverse-stepi" or "reverse-next". All output for this command is currently echoed into the running machine window. Previous memory and PC tracking statistics are cleared, actual reverse execution does not occur.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-statesave:

statesave
---------

|  **statesave[ss] <filename>**
|
| The statesave command creates a save state at this exact moment in time. The given state file gets written to the standard state directory (sta), and gets .sta added to it - no file extension necessary. All output for this command is currently echoed into the running machine window.
|
| Examples:
|
|  statesave foo
|
| Writes file 'foo.sta' in the default state save directory.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-stateload:

stateload
---------

|  **stateload[sl] <filename>**
|
| The stateload command retrieves a save state from disk. The given state file gets read from the standard state directory (sta), and gets .sta added to it - no file extension necessary. All output for this command is currently echoed into the running machine window. Previous memory and PC tracking statistics are cleared.
|
| Examples:
|
|  stateload foo
|
| Reads file 'foo.sta' from the default state save directory.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-snap:

snap
----

|  **snap [[<filename>], <scrnum>]**
|
| The snap command takes a snapshot of the current video display and saves it to the configured snapshot directory. If <filename> is specified explicitly, a single screenshot for <scrnum> is saved under the requested filename. If <filename> is omitted, all screens are saved using the same default rules as the "save snapshot" key in MAME proper.
|
| Examples:
|
|  snap
|
| Takes a snapshot of the current video screen and saves to the next non-conflicting filename in the configured snapshot directory.
|
|  snap shinobi
|
| Takes a snapshot of the current video screen and saves it as 'shinobi.png' in the configured snapshot directory.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-source:

source
------

|  **source <filename>**
|
| The source command reads in a set of debugger commands from a file and executes them one by one, similar to a batch file.
|
| Examples:
|
|  source break_and_trace.cmd
|
| Reads in debugger commands from break_and_trace.cmd and executes them.
|
| Back to :ref:`debugger-general-list`


 .. _debugger-command-quit:

quit
----

|  **quit**
|
| The quit command exits MAME immediately.
|
| Back to :ref:`debugger-general-list`

