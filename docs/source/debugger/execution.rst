.. _debugger-execution-list:

Execution Debugger Commands
===========================

:ref:`debugger-command-step`
    single step for <count> instructions (F11, when disassembly view is active)
:ref:`debugger-command-steps`
    single step one source line (F11, when source view is active)
:ref:`debugger-command-over`
    single step over <count> instructions (F10, when disassembly view is active)
:ref:`debugger-command-overs`
    single step over one source line (F10, when source view is active)
:ref:`debugger-command-out`
    single step until the current subroutine/exception handler returns
    (Shift-F11)
:ref:`debugger-command-go`
    resume execution (F5)
:ref:`debugger-command-gbt`
    resume execution until next true branch is executed
:ref:`debugger-command-gbf`
    resume execution until next false branch is executed
:ref:`debugger-command-gex`
    resume execution until exception is raised
:ref:`debugger-command-gint`
    resume execution until interrupt is taken (F7)
:ref:`debugger-command-gni`
    resume execution until next further instruction
:ref:`debugger-command-gtime`
    resume execution until the given delay has elapsed
:ref:`debugger-command-gvblank`
    resume execution until next vertical blanking interval (F8)
:ref:`debugger-command-next`
    resume execution until the next CPU switch (F6)
:ref:`debugger-command-focus`
    focus debugger only on <CPU>
:ref:`debugger-command-ignore`
    stop debugging on <CPU>
:ref:`debugger-command-observe`
    resume debugging on <CPU>
:ref:`debugger-command-trace`
    trace the specified CPU to a file
:ref:`debugger-command-traceover`
    trace the specified CPU to a file skipping subroutines
:ref:`debugger-command-traceflush`
    flush all open trace files.


.. _debugger-command-step:

step
----

**s[tep] [<count>]**

Single steps one or more instructions on the currently executing CPU.
Executes one instruction if **<count>** is omitted, or steps **<count>**
instructions if it is supplied.

Examples:

``s``
    Steps forward one instruction on the current CPU.
``step 4``
    Steps forward four instructions on the current CPU.

Back to :ref:`debugger-execution-list`


.. _debugger-command-steps:

steps
-----

**steps**

When :ref:`source-level debugging <srcdbg>` is enabled, this single-steps
one *source* line on the currently executing CPU.  When the original
source is assembly language, ``step`` and ``steps`` generally behave the same.
But when the original source is in a higher level language like C or BASIC,
``steps`` results in executing the remainder of a block of instructions
associated with the current source line.

If the current source line is a
call instruction, ``steps`` stops at the first source line in the called
function, but only if the called function has source associated with it.
If neither the called function nor any of its callees have source associated
with them, ``steps`` will continue execution until the call is complete, and
the first instruction with source associated with it is encountered.

If the current line returns from a recursive function, ``steps`` will stop
at the same line, but in the prior call frame.  It will appear as if
no stepping occurred, but the stack register will indicate a return
has occurred.  Note that this logic can be fooled when stepping into a function
without associated source, which makes further calls without proper
matching returns (using direct manipulation of the stack pointer instead).



Examples:

``steps``
    Steps forward to the next source line on the current CPU.
``sts``
    Steps forward to the next source line on the current CPU.

Back to :ref:`debugger-execution-list`


.. _debugger-command-over:

over
----

**o[ver] [<count>]**

The over command single steps “over” one or more instructions on the
currently executing CPU, stepping over subroutine calls and exception
handler traps and counting them as a single instruction.  Note that when
stepping over a subroutine call, code may execute on other CPUs before
the subroutine returns.

Steps over one instruction if **<count>** is omitted, or steps over
**<count>** instructions if it is supplied.

Note that the step over functionality may not be implemented for all CPU
types.  If it is not implemented, then ``over`` will behave exactly like
:ref:`debugger-command-step`.

Examples:

``o``
    Steps forward over one instruction on the current CPU.
``over 4``
    Steps forward over four instructions on the current CPU.

Back to :ref:`debugger-execution-list`


.. _debugger-command-overs:

overs
-----

**overs**

When :ref:`source-level debugging <srcdbg>` is enabled, this steps forward
over one *source* line on the currently executing CPU.  When the original
source is assembly language, ``over`` and ``overs`` generally behave the same.
But when the original source is in a higher level language like C or BASIC,
``overs`` results in executing the remainder of a block of instructions
associated with the current source line.

Examples:

``overs``
    Steps forward over the next source line on the current CPU.
``os``
    Steps forward over the next source line on the current CPU.

Back to :ref:`debugger-execution-list`


.. _debugger-command-out:

out
---

**out**

Single steps until a return from subroutine or return from exception
instruction is encountered.  Note that because it detects return from
exception conditions, if you attempt to step out of a subroutine and an
interrupt/exception occurs before the subroutine completes, execution
may stop prematurely at the end of the exception handler.

Note that the step out functionality may not be implemented for all CPU
types.  If it is not implemented, then ``out`` will behave exactly like
:ref:`debugger-command-step`.

Example:

``out``
    Steps until a subroutine or exception handler returns.

Back to :ref:`debugger-execution-list`


.. _debugger-command-outs:

outs
----

**outs**

When :ref:`source-level debugging <srcdbg>` is enabled, this single-steps
until the next return from subroutine or return from exception instruction
lands the currently executing CPU to a *source* line.  When the original
source is assembly language and source-level debugging information is provided
for the current and calling subroutine, ``out`` and ``outs`` generally
behave the same.  But when layers of functions or subroutines *without*
source-level information exist between the current instruction and the
most recent calling instruction *with* source-level information, ``outs``
will skip over those "non source-level information" layers until it lands on
calling code with source-level information.

Examples:

``outs``
    Steps until a subroutine or exception handler returns to code with
    source-level information.

Back to :ref:`debugger-execution-list`


.. _debugger-command-go:

go
--

**g[o] [<address>]**

Resumes execution.  Control will not be returned to the debugger until a
breakpoint or watchpoint is triggered, or a debugger break is manually
requested.  If the optional **<address>** is supplied, a temporary
unconditional breakpoint will be set for the visible CPU at the
specified address.  It will be cleared automatically when triggered.

Examples:

``g``
    Resume execution until a breakpoint/watchpoint is triggered, or a
    break is manually requested.
``g 1234``
    Resume execution, stopping at address 1234, unless another condition
    causes execution to stop before then.

Back to :ref:`debugger-execution-list`


.. _debugger-command-gbf:

gbf
---

**gbf [<condition>]**

Resumes execution.  Control will not be returned to the debugger until
a breakpoint or watchpoint is triggered, or until a conditional branch
or skip instruction is not taken, following any delay slots.

The optional **<condition>** parameter lets you specify an expression
that will be evaluated each time a conditional branch is encountered.
If the result of the expression is true (non-zero), execution will be
halted after the branch if it is not taken; otherwise, execution will
continue with no notification.

Examples:

``gbf``
    Resume execution until a breakpoint/watchpoint is triggered, or
    until the next false branch.
``gbf {pc != 1234}``
    Resume execution until the next false branch, disregarding the
    instruction at address 1234.

Back to :ref:`debugger-execution-list`


.. _debugger-command-gbt:

gbt
---

**gbt [<condition>]**

Resumes execution.  Control will not be returned to the debugger until
a breakpoint or watchpoint is triggered, or until a conditional branch
or skip instruction is taken, following any delay slots.

The optional **<condition>** parameter lets you specify an expression
that will be evaluated each time a conditional branch is encountered.
If the result of the expression is true (non-zero), execution will be
halted after the branch if it is taken; otherwise, execution will
continue with no notification.

Examples:

``gbt``
    Resume execution until a breakpoint/watchpoint is triggered, or
    until the next true branch.
``gbt {pc != 1234}``
    Resume execution until the next true branch, disregarding the
    instruction at address 1234.

Back to :ref:`debugger-execution-list`


.. _debugger-command-gex:

gex
---

**ge[x] [<exception>,[<condition>]]**

Resumes execution.  Control will not be returned to the debugger until
a breakpoint or watchpoint is triggered, or until an exception condition
is raised on the current CPU.  Use the optional **<exception>**
parameter to stop execution only for a specific exception condition.  If
**<exception>** is omitted, execution will stop for any exception
condition.

The optional **<condition>** parameter lets you specify an expression
that will be evaluated each time the specified exception condition
is raised.  If the result of the expression is true (non-zero), the
exception will halt execution; otherwise, execution will continue with
no notification.

Examples:

``gex``
    Resume execution until a breakpoint/watchpoint is triggered, or
    until any exception condition is raised on the current CPU.
``ge 2``
    Resume execution until a breakpoint/watchpoint is triggered, or
    until exception condition 2 is raised on the current CPU.

Back to :ref:`debugger-execution-list`


.. _debugger-command-gint:

gint
----

**gi[nt] [<irqline>]**

Resumes execution.  Control will not be returned to the debugger until a
breakpoint or watchpoint is triggered, or until an interrupt is asserted
and acknowledged on the current CPU.  Use the optional **<irqline>**
parameter to stop execution only for a specific interrupt line being
asserted and acknowledged.  If **<irqline>** is omitted, execution will
stop when any interrupt is acknowledged.

Examples:

``gi``
    Resume execution until a breakpoint/watchpoint is triggered, or
    any interrupt is asserted and acknowledged on the current CPU.
``gint 4``
    Resume execution until a breakpoint/watchpoint is triggered, or
    interrupt request line 4 is asserted and acknowledged on the current
    CPU.

Back to :ref:`debugger-execution-list`


.. _debugger-command-gni:

gni
---

**gni [<count>]**

Resumes execution.  Control will not be returned to the debugger until a
breakpoint or watchpoint is triggered.  A temporary unconditional breakpoint
is set at the program address **<count>** instructions sequentially past the
current one.  When this breakpoint is hit, it is automatically removed.

The **<count>** parameter is optional and defaults to 1 if omitted.  If
**<count>** is specified as zero, the command does nothing.  **<count>** is
not permitted to exceed 512 decimal.

Examples:

``gni``
    Resume execution until a breakpoint/watchpoint is triggered, including
    the temporary breakpoint set at the address of the following instruction.
``gni 2``
    Resume execution until a breakpoint/watchpoint is triggered.  A temporary
    breakpoint is set two instructions past the current one.

Back to :ref:`debugger-execution-list`


.. _debugger-command-gtime:

gtime
-----

**gt[ime] <milliseconds>**

Resumes execution.  Control will not be returned to the debugger until a
specified interval of emulated time has elapsed.  The interval is
specified in milliseconds.

Example:

``gtime #10000``
    Resume execution for ten seconds of emulated time.

Back to :ref:`debugger-execution-list`


.. _debugger-command-gvblank:

gvblank
-------

**gv[blank]**

Resumes execution.  Control will not be returned to the debugger until a
breakpoint or watchpoint is triggered, or until the beginning of the
vertical blanking interval for an emulated screen.

Example:

``gv``
    Resume execution until a breakpoint/watchpoint is triggered, or a
    vertical blanking interval starts.

Back to :ref:`debugger-execution-list`


.. _debugger-command-next:

next
----

**n[ext]**

Resumes execution until a different CPU is scheduled.  Execution will
not stop when a CPU is scheduled if it is ignored due to the use of
:ref:`debugger-command-ignore` or :ref:`debugger-command-focus`.

Example:

``n``
    Resume execution, stopping when a different CPU that is not ignored
    is scheduled.

Back to :ref:`debugger-execution-list`


.. _debugger-command-focus:

focus
-----

**focus <CPU>**

Focus exclusively on to the specified **<CPU>**, ignoring all other
CPUs.  The **<CPU>** argument can be a device tag or debugger CPU number
(see :ref:`debugger-devicespec` for details).  This is equivalent to
using the :ref:`debugger-command-ignore` command to ignore all CPUs
besides the specified CPU.

Examples:

``focus 1``
    Focus exclusively on the second CPU in the system (zero-based
    index), ignoring all other CPUs.
``focus audiopcb:melodycpu``
    Focus exclusively on the CPU with the absolute tag path
    ``:audiopcb:melodycpu``.

Back to :ref:`debugger-execution-list`


.. _debugger-command-ignore:

ignore
------

**ignore [<CPU>[,<CPU>[,…]]]**

Ignores the specified CPUs in the debugger.  CPUs can be specified by
tag or debugger CPU number (see :ref:`debugger-devicespec` for details).
The debugger never shows execution for ignored CPUs, and breakpoints or
watchpoints on ignored CPUs have no effect.  If no CPUs are specified,
currently ignored CPUs will be listed.  Use the
:ref:`debugger-command-observe` command to stop ignoring a CPU.

Note that you cannot ignore all CPUs; at least CPU must be observed at
all times.

Examples:

``ignore audiocpu``
    Ignore the CPU with the absolute tag path ``:audiocpu`` when using
    the debugger.
``ignore 2,3,4``
    Ignore the third, fourth and fifth CPUs in the system (zero-based
    indices) when using the debugger.
``ignore``
    List the CPUs that are currently being ignored by the debugger.

Back to :ref:`debugger-execution-list`


.. _debugger-command-observe:

observe
-------

**observe [<CPU>[,<CPU>[,…]]]**

Allow interaction with the specified CPUs in the debugger.  CPUs can be
specified by tag or debugger CPU number (see :ref:`debugger-devicespec`
for details).  This command reverses the effects of the
:ref:`debugger-command-ignore` command.  If no CPUs are specified,
currently observed CPUs will be listed.

Examples:

``observe audiocpu``
    Stop ignoring the CPU with the absolute tag path ``:audiocpu`` when
    using the debugger.
``observe 2,3,4``
    Stop ignoring the third, fourth and fifth CPUs in the system
    (zero-based indices) when using the debugger.
``observe``
    List the CPUs that are currently being observed by the debugger.

Back to :ref:`debugger-execution-list`


.. _debugger-command-trace:

trace
-----

**trace {<filename>|off}[,<CPU>[,[noloop|logerror][,<action>]]]**

Starts or stops tracing for execution of the specified **<CPU>**, or the
currently visible CPU if no CPU is specified.  To enable tracing,
specify the trace log file name in the **<filename>** parameter.  To
disable tracing, use the keyword ``off`` for the **<filename>**
parameter.  If the **<filename>** argument begins with two right angle
brackets (**>>**), it is treated as a directive to open the file for
appending rather than overwriting.

The optional third parameter is a flags field.  The supported flags are
``noloop`` and ``logerror``.  Multiple flags must be separated by ``|``
(pipe) characters.  By default, loops are detected and condensed to a
single line.  If the ``noloop`` flag is specified, loops will not be
detected and every instruction will be logged as executed.  If the
``logerror`` flag is specified, error log output will be included in the
trace log.

The optional **<action>** parameter is a debugger command to execute
before each trace message is logged.  Generally, this will include a
:ref:`debugger-command-tracelog` or :ref:`debugger-command-tracesym`
command to include additional information in the trace log.  Note that
you may need to surround the action within braces ``{ }`` to ensure
commas and semicolons within the command are not interpreted in the
context of the ``trace`` command itself.

Examples:

``trace joust.tr``
    Begin tracing the execution of the currently visible CPU, logging
    output to the file **joust.tr**.
``trace dribling.tr,maincpu``
    Begin tracing the execution of the CPU with the absolute tag path
    ``:maincpu:``, logging output to the file **dribling.tr**.
``trace starswep.tr,,noloop``
    Begin tracing the execution of the currently visible CPU, logging
    output to the file **starswep.tr**, with loop detection disabled.
``trace starswep.tr,1,logerror``
    Begin tracing the execution of the second CPU in the system
    (zero-based index), logging output along with error log output to
    the file **starswep.tr**.
``trace starswep.tr,0,logerror|noloop``
    Begin tracing the execution of the first CPU in the system
    (zero-based index), logging output along with error log output to
    the file **starswep.tr**, with loop detection disabled.
``trace >>pigskin.tr``
    Begin tracing execution of the currently visible CPU, appending log
    output to the file **pigskin.tr**.
``trace off,0``
    Turn off tracing for the first CPU in the system (zero-based index).
``trace asteroid.tr,,,{tracelog "A=%02X ",a}``
    Begin tracing the execution of the currently visible CPU, logging
    output to the file **asteroid.tr**.  Before each line, output
    **A=<aval>** to the trace log.

Back to :ref:`debugger-execution-list`


.. _debugger-command-traceover:

traceover
---------

**traceover {<filename>|off}[,<CPU>[,[noloop|logerror][,<action>]]]**

Starts or stops tracing for execution of the specified **<CPU>**, or the
currently visible CPU if no CPU is specified.  When a subroutine call is
encountered, tracing will skip over the subroutine.  The same algorithm
is used as is used in the :ref:`step over <debugger-command-over>`
command.  It will not work properly with recursive functions, or if the
return address does not immediately follow the call instruction.

This command accepts the same parameters as the
:ref:`debugger-command-trace` command.  Please refer to the
corresponding section for a detailed description of options and more
examples.

Examples:

``traceover joust.tr``
    Begin tracing the execution of the currently visible CPU, logging
    output to the file **joust.tr**.
``traceover dribling.tr,maincpu``
    Begin tracing the execution of the CPU with the absolute tag path
    ``:maincpu:``, logging output to the file **dribling.tr**.
``traceover starswep.tr,,noloop``
    Begin tracing the execution of the currently visible CPU, logging
    output to the file **starswep.tr**, with loop detection disabled.
``traceover off,0``
    Turn off tracing for the first CPU in the system (zero-based index).
``traceover asteroid.tr,,,{tracelog "A=%02X ",a}``
    Begin tracing the execution of the currently visible CPU, logging
    output to the file **asteroid.tr**.  Before each line, output
    **A=<aval>** to the trace log.

Back to :ref:`debugger-execution-list`


.. _debugger-command-traceflush:

traceflush
----------

**traceflush**

Flushes all open trace log files to disk.

Example:

``traceflush``
    Flush trace log files.

Back to :ref:`debugger-execution-list`
