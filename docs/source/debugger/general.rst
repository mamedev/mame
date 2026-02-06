.. _debugger-general-list:

General Debugger Commands
=========================

:ref:`debugger-command-help`
    displays built-in help in the console
:ref:`debugger-command-do`
    evaluates the given expression
:ref:`debugger-command-symlist`
    lists registered symbols
:ref:`debugger-command-softreset`
    executes a soft reset
:ref:`debugger-command-hardreset`
    executes a hard reset
:ref:`debugger-command-print`
    prints one or more <item>s to the console
:ref:`debugger-command-printf`
    prints one or more <item>s to the console using <format>
:ref:`debugger-command-logerror`
    outputs one or more <item>s to the error.log
:ref:`debugger-command-tracelog`
    outputs one or more <item>s to the trace file using <format>
:ref:`debugger-command-tracesym`
    outputs one or more <item>s to the trace file
:ref:`debugger-command-history`
    displays recently visited PC addresses and opcodes
:ref:`debugger-command-trackpc`
    visually track visited opcodes
:ref:`debugger-command-trackmem`
    record which PC writes to each memory address
:ref:`debugger-command-pcatmem`
    query which PC wrote to a given memory address
:ref:`debugger-command-rewind`
    go back in time by loading the most recent rewind state
:ref:`debugger-command-statesave`
    save a state file for the emulated system
:ref:`debugger-command-stateload`
    load a state file for the emulated system
:ref:`debugger-command-snap`
    save a screen snapshot
:ref:`debugger-command-source`
    read commands from file and executes them one by one
:ref:`debugger-command-time`
    prints the current machine time to the console
:ref:`debugger-command-sdoffset`
    sets the address offset to be used by source-level debugging
:ref:`debugger-command-quit`
    exit the debugger and end the emulation session


.. _debugger-command-help:

help
----

**help [<topic>]**

Displays built-in debugger help in the debugger console.  If no
**<topic>** is specified, top-level topics are listed.  Most debugger
commands have correspondingly named help topics.

Examples:

``help``
    Lists top-level help topics.
``help expressions``
    Displays built-in help for debugger expression syntax.
``help wpiset``
    Displays built-in help for the
    :ref:`wpiset <debugger-command-wpset>` command.

Back to :ref:`debugger-general-list`


.. _debugger-command-do:

do
--

**do <expression>**

The **do** command simply evaluates the supplied expression.  This is
often used to set or modify device state variable (e.g. CPU registers),
or to write to memory.  See :ref:`debugger-express` for details about
expression syntax.

Examples:

``do pc = 0``
    Sets the register **pc** to 0.

Back to :ref:`debugger-general-list`


.. _debugger-command-symlist:

symlist
-------

**symlist [<cpu>]**

Lists registered symbols and their values.  If **<cpu>** is not
specified, symbols in the global symbol table and the primary
CPU are displayed; otherwise,
symbols specific to the device **<cpu>** are displayed. 
If :ref:`Source-Level Debugging <srcdbg>` is enabled, and **<cpu>**
is the primary CPU (or **<cpu>** is not specified),
then :ref:`source-level symbols <srcdbg_symbols>` are listed as well.
Symbols are listed alphabetically.  Read-only symbols are noted.  See
:ref:`debugger-devicespec` for details on how to specify a CPU.

Examples:

``symlist``
    Displays the global symbol table, the primary CPU's symbol table,
    and, if enabled, source-level symbols.
``symlist 2``
    Displays the symbols for the third CPU in the system (zero-based
    index).
``symlist audiocpu``
    Displays symbols for the CPU with the absolute tag ``:audiocpu``.

Back to :ref:`debugger-general-list`


.. _debugger-command-softreset:

softreset
---------

**softreset**

Executes a soft reset.  This calls the reset member functions of all the
devices in the system (by default, pressing **F3** during emulation has
the same effect).

Examples:

``softreset``
    Executes a soft reset.

Back to :ref:`debugger-general-list`


.. _debugger-command-hardreset:

hardreset
---------

**hardreset**

Executes a hard reset.  This tears down the emulation session and starts
another session with the same system and options (by default, pressing
**Shift+F3** during emulation has the same effect).  Note that this will
lose history in the debugger console and error log.

Examples:

``hardreset``
    Executes a hard reset.

Back to :ref:`debugger-general-list`


.. _debugger-command-print:

print
-----

**print <item>[,…]**

The **print** command prints the results of one or more expressions to
the debugger console as hexadecimal numbers.

Examples:

``print pc``
    Prints the value of the **pc** register the console as a hex number.
``print a,b,a+b``
    Prints **a**, **b**, and the value of **a+b** to the console as hex
    numbers.

Back to :ref:`debugger-general-list`


.. _debugger-command-printf:

printf
------

**printf <format>[,<argument>[,…]]**

Prints a C-style formatted message to the debugger console.  Only a
very limited subset of format specifiers and escape sequences are
available:

%c
    Prints the corresponding argument as an 8-bit character.
%[-][0][<n>]d
    Prints the corresponding argument as a decimal number with optional
    left justification, zero fill and minimum field width.
%[-][0][<n>]o
    Prints the corresponding argument as an octal number with optional
    left justification, zero fill and minimum field width.
%[-][0][<n>]x
    Prints the corresponding argument as a lowercase hexadecimal number
    with optional left justification, zero fill and minimum field width.
%[-][0][<n>]X
    Prints the corresponding argument as an uppercase hexadecimal number
    with optional left justification, zero fill and minimum field width.
%[-][<n>][.[<n>]]s
    Prints a null-terminated string of 8-bit characters from the address
    and address space given by the corresponding argument, with optional
    left justification, minimum and maximum field widths.
\%%
    Prints a literal percent symbol.
\\n
    Prints a line break.
**\\\\**
    Prints a literal backslash.

All other format specifiers are ignored.

Examples:

``printf "PC=%04X",pc``
    Prints ``PC=<pcval>`` where **<pcval>** is the hexadecimal value of
    the **pc** register with a minimum of four digits and zero fill.
``printf "A=%d, B=%d\\nC=%d",a,b,a+b``
    Prints ``A=<aval>, B=<bval>`` on one line, and ``C=<a+bval>`` on a
    second line.

Back to :ref:`debugger-general-list`


.. _debugger-command-logerror:

logerror
--------

**logerror <format>[,<argument>[,…]]**

Prints a C-style formatted message to the error log.  See
:ref:`debugger-command-printf` for details about the limited set of
supported format specifiers and escape sequences.

Examples:

``logerror "PC=%04X",pc``
    Logs ``PC=<pcval>`` where **<pcval>** is the hexadecimal value of
    the **pc** register with a minimum of four digits and zero fill.
``logerror "A=%d, B=%d\\nC=%d",a,b,a+b``
    Logs ``A=<aval>, B=<bval>`` on one line, and ``C=<a+bval>`` on a
    second line.

Back to :ref:`debugger-general-list`


.. _debugger-command-tracelog:

tracelog
--------

**tracelog <format>[,<argument>[,…]]**

Prints a C-style formatted message to the currently open trace file (see
:ref:`debugger-command-trace` for more information).  If no trace file
is open, this command has no effect.  See :ref:`debugger-command-printf`
for details about the limited set of supported format specifiers and
escape sequences.

Examples:

``tracelog "PC=%04X",pc``
    Outputs ``PC=<pcval>`` where **<pcval>** is the hexadecimal value of
    the **pc** register with a minimum of four digits and zero fill if a
    trace log file is open.
``tracelog "A=%d, B=%d\\nC=%d",a,b,a+b``
    Outputs ``A=<aval>, B=<bval>`` on one line, and ``C=<a+bval>`` on a
    second line if a trace log file is open.

Back to :ref:`debugger-general-list`


.. _debugger-command-tracesym:

tracesym
--------

**tracesym <item>[,…]**

Prints the specified symbols to the currently open trace file (see
:ref:`debugger-command-trace` for more information).  If no trace file
is open, this command has no effect.

Examples:

``tracesym pc``
    Outputs ``PC=<pcval>`` where **<pcval>** is the value of the **pc**
    register in its default format if a trace log file is open.

Back to :ref:`debugger-general-list`


.. _debugger-command-history:

history
-------

**history [<CPU>[,<length>]]**

Displays recently visited PC addresses, and disassembly of the
instructions at those addresses.  If present, the first argument selects
the CPU (see :ref:`debugger-devicespec` for details); if no CPU is
specified, the visible CPU is assumed.  The second argument, if present,
limits the maximum number of addresses shown.  Addresses are shown in
order from least to most recently visited.

Examples:

``history ,5``
    Displays up to five most recently visited PC addresses and
    instructions for the visible CPU.
``history 3``
    Displays recently visited PC addresses and instructions for the
    fourth CPU in the system (zero-based index).
``history audiocpu,1``
    Displays the most recently visited PC address and instruction for
    the CPU with the absolute tag ``:audiocpu``.


.. _debugger-command-trackpc:

trackpc
-------

**trackpc [<enable>[,<CPU>[,<clear>]]]**

Turns visited PC address tracking for disassembly views on or off.
Instructions at addresses visited while tracking is on are highlighted
in debugger disassembly views.  The first argument is a Boolean
specifying whether tracking should be turned on or off (defaults to on).
The second argument specifies the CPU to enable or disable tracking for
(see :ref:`debugger-devicespec` for details); if no CPU is specified,
the visible CPU is assumed.  The third argument is a Boolean specifying
whether existing data should be cleared (defaults to false).

Examples:

``trackpc 1``
   Begin or tracking the current CPU’s PC.
``trackpc 1,0,1``
   Begin or continue tracking PC on the first CPU in the system
   (zero-based index), but clear the history tracked so far.

Back to :ref:`debugger-general-list`


.. _debugger-command-trackmem:

trackmem
--------

**trackmem [<enable>,[<CPU>,[<clear>]]]**

Enables or disables logging the PC address each time a memory address is
written to.  The first argument is a Boolean specifying whether tracking
should be enabled or disabled (defaults to enabled).  The second
argument specifies the CPU to enable or disable tracking for (see
:ref:`debugger-devicespec` for details); if no CPU is specified, the
visible CPU is assumed.  The third argument is a Boolean specifying
whether existing data should be cleared (defaults to false).

Use :ref:`debugger-command-pcatmem` to retrieve this data.
Right-clicking a debugger memory view will also display the logged PC
value for the given address in some configurations.

Examples:

``trackmem``
    Begin or continue tracking memory writes for the visible CPU.
``trackmem 1,0,1``
    Begin or continue tracking memory writes for the first CPU in the
    system (zero-based index), but clear existing tracking data.

Back to :ref:`debugger-general-list`


.. _debugger-command-pcatmem:

pcatmem
-------

**pcatmem[{d|i|o}] <address>[:<space>]**

Returns the PC value at the time the specified address was most recently
written to.  The argument is the requested address, optionally followed
by a colon and a CPU and/or address space (see
:ref:`debugger-devicespec` for details).  The optional **d**, **i** or
**o** suffix controls the default address space for the command.

Tracking must be enabled for the data this command uses to be recorded
(see :ref:`debugger-command-trackmem`).  Right-clicking a debugger
memory view will also display the logged PC value for the given address
in some configurations.

Examples:

``pcatmem 400000``
   Print the PC value when location 400000 in the visible CPU’s program
   space was most recently written.
``pcatmem 3bc:io``
    Print the PC value when location 3bc in the visible CPU’s ``io``
    space was most recently written.
``pcatmem 1400:audiocpu``
    Print the PC value when location 1400 in the CPU ``:audiocpu``’s
    program space was most recently written.

Back to :ref:`debugger-general-list`


.. _debugger-command-rewind:

rewind
------

**rewind**

Loads the most recent RAM-based saved state.  When enabled, rewind
states are saved when :ref:`debugger-command-step`,
:ref:`debugger-command-over` and :ref:`debugger-command-out` commands
are used, storing the machine state as of the moment before stepping.
May be abbreviated to ``rw``.

Consecutively loading rewind states can work like reverse execution.
Depending on which steps forward were taken previously, the behavior can
be similar to GDB's **reverse-stepi** and **reverse-next** commands.
All output for this command is currently echoed into the running machine
window.

Previous memory and PC tracking statistics are cleared.  Actual reverse
execution does not occur.

Examples:

``rewind``
    Load the previous RAM-based save state.
``rw``
    Abbreviated form of the command.

Back to :ref:`debugger-general-list`


.. _debugger-command-statesave:

statesave
---------

**statesave <filename>**

Creates a save state at the current moment in emulated time.  The state
file is written to the configured save state directory (see the
:ref:`state_directory <mame-commandline-statedirectory>` option), and
the **.sta** extension is automatically appended to the specified file
name.  May be abbreviates to ``ss``.

All output from this command is currently echoed into the running machine
window.

Examples:

``statesave foo``
   Saves the emulated machine state to the file **foo.sta** in the
   configured save state directory.
``ss bar``
    Abbreviated form of the command – saves the emulated machine state
    to **bar.sta**.

Back to :ref:`debugger-general-list`


.. _debugger-command-stateload:

stateload
---------

**stateload <filename>**

Restores a saved state file from disk.  The specified state file is read
from the configured save state directory (see the
:ref:`state_directory <mame-commandline-statedirectory>` option), and the
**.sta** extension is automatically appended to the specified file name.
May be abbreviated to ``sl``.

All output for this command is currently echoed into the running machine
window.  Previous memory and PC tracking statistics are cleared.

Examples:

``stateload foo``
    Loads state from file **foo.sta** to the configured save state
    directory.
``sl bar``
    Abbreviated form of the command – loads the file **bar.sta**.

Back to :ref:`debugger-general-list`


.. _debugger-command-snap:

snap
----

**snap [<filename>[,<scrnum>]]**

Takes a snapshot of the emulated video display and saves it to the
configured snapshot directory (see the
:ref:`snapshot_directory <mame-commandline-snapshotdirectory>` option).
If a file name is specified, a single screenshot for the specified
screen is saved using the specified filename (or the first emulated
screen in the system if a screen is not specified).  If a file name is
not specified, the configured snapshot view and file name pattern are
used (see the :ref:`snapview <mame-commandline-snapview>` and
:ref:`snapname <mame-commandline-snapname>` options).

If a file name is specified, the **.png** extension is automatically
appended.  The screen number is specified as a zero-based index, as
seen in the names of automatically-generated single-screen views in
MAME’s video options menus.

Examples:

``snap``
    Takes a snapshot using the configured snapshot view and file name
    options.
``snap shinobi``
    Takes a snapshot of the first emulated video screen and saves it as
    **shinobi.png** in the configured snapshot directory.

Back to :ref:`debugger-general-list`


.. _debugger-command-source:

source
------

**source <filename>**

Reads the specified file in text mode and executes each line as a
debugger command.  This is similar to running a shell script or batch
file.

Examples:

``source break_and_trace.cmd``
    Reads and executes debugger commands from **break_and_trace.cmd**.

Back to :ref:`debugger-general-list`


.. _debugger-command-time:

time
----

Prints the total elapsed emulated time to the debugger console.

Examples:

``time``
    Prints the elapsed emulated time.

Back to :ref:`debugger-general-list`


.. _debugger-command-sdoffset:

sdoffset
--------

Sets the :ref:`address offset <srcdbg_offsets>` to be used by
:ref:`source-level debugging <srcdbg>`

Examples:

``sdoffset E000``
    Apply an offset of $E000 to all addresses present
    in the :ref:`MAME Debugging Information File <srcdbg_mdi>`.

Back to :ref:`debugger-general-list`


.. _debugger-command-quit:

quit
----

**quit**

Closes the debugger and ends the emulation session immediately.  Either
exits MAME or returns to the system selection menu, depending on whether
the system was specified on the command line when starting MAME.

Examples:

``quit``
    Exits the emulation session immediately.

Back to :ref:`debugger-general-list`
