.. _debugger-execution-list:

Execution Debugger Commands
===========================


You can also type **help <command>** for further details on each command in the MAME Debugger interface.

| :ref:`debugger-command-step` -- single steps for <count> instructions (F11)
| :ref:`debugger-command-over` -- single steps over <count> instructions (F10)
| :ref:`debugger-command-out` -- single steps until the current subroutine/exception handler is exited (Shift-F11)
| :ref:`debugger-command-go` -- resumes execution, sets temp breakpoint at <address> (F5)
| :ref:`debugger-command-gint` -- resumes execution, setting temp breakpoint if <irqline> is taken (F7)
| :ref:`debugger-command-gtime` -- resumes execution until the given delay has elapsed
| :ref:`debugger-command-gvblank` -- resumes execution, setting temp breakpoint on the next VBLANK (F8)
| :ref:`debugger-command-next` -- executes until the next CPU switch (F6)
| :ref:`debugger-command-focus` -- focuses debugger only on <cpu>
| :ref:`debugger-command-ignore` -- stops debugging on <cpu>
| :ref:`debugger-command-observe` -- resumes debugging on <cpu>
| :ref:`debugger-command-trace` -- trace the given CPU to a file (defaults to active CPU)
| :ref:`debugger-command-traceover` -- trace the given CPU to a file, but skip subroutines (defaults to active CPU)
| :ref:`debugger-command-traceflush` -- flushes all open trace files.


 .. _debugger-command-step:

step
----

|  **s[tep] [<count>=1]**
|
| The step command single steps one or more instructions in the currently executing CPU. By default, step executes one instruction each time it is issued. You can also tell step to step multiple instructions by including the optional <count> parameter.
|
| Examples:
|
|  s
|
| Steps forward one instruction on the current CPU.
|
|  step 4
|
| Steps forward four instructions on the current CPU.
|
| Back to :ref:`debugger-execution-list`


 .. _debugger-command-over:

over
----

|  **o[ver] [<count>=1]**
|
| The over command single steps "over" one or more instructions in the currently executing CPU, stepping over subroutine calls and exception handler traps and counting them as a single instruction. Note that when stepping over a subroutine call, code may execute on other CPUs before the subroutine call completes. By default, over executes one instruction each time it is issued. You can also tell step to step multiple instructions by including the optional <count> parameter.
|
| Note that the step over functionality may not be implemented on all CPU types. If it is not implemented, then 'over' will behave exactly like 'step'.
|
| Examples:
|
|  o
|
| Steps forward over one instruction on the current CPU.
|
|  over 4
|
| Steps forward over four instructions on the current CPU.
|
| Back to :ref:`debugger-execution-list`


 .. _debugger-command-out:

out
---

|  **out**
|
| The out command single steps until it encounters a return from subroutine or return from exception instruction. Note that because it detects return from exception conditions, if you attempt to step out of a subroutine and an interrupt/exception occurs before you hit the end, then you may stop prematurely at the end of the exception handler.
|
| Note that the step out functionality may not be implemented on all CPU types. If it is not implemented, then 'out' will behave exactly like 'step'.
|
| Examples:
|
|  out
|
| Steps until the current subroutine or exception handler returns.
|
| Back to :ref:`debugger-execution-list`


 .. _debugger-command-go:

go
--

|  **g[o] [<address>]**
|
| The go command resumes execution of the current code. Control will not be returned to the debugger until a breakpoint or watchpoint is hit, or until you manually break in using the assigned key. The go command takes an optional <address> parameter which is a temporary unconditional breakpoint that is set before executing, and automatically removed when hit.
|
| Examples:
|
|  g
|
| Resume execution until the next break/watchpoint or until a manual break.
|
|  g 1234
|
| Resume execution, stopping at address 1234 unless something else stops us first.
|
| Back to :ref:`debugger-execution-list`


 .. _debugger-command-gvblank:

gvblank
-------

|  **gv[blank]**
|
| The gvblank command resumes execution of the current code. Control will not be returned to the debugger until a breakpoint or watchpoint is hit, or until the next VBLANK occurs in the emulator.
|
| Examples:
|
|  gv
|
| Resume execution until the next break/watchpoint or until the next VBLANK.
|
| Back to :ref:`debugger-execution-list`


 .. _debugger-command-gint:

gint
----

|  **gi[nt] [<irqline>]**
|
| The gint command resumes execution of the current code. Control will not be returned to the debugger until a breakpoint or watchpoint is hit, or until an IRQ is asserted and acknowledged on the current CPU. You can specify <irqline> if you wish to stop execution only on a particular IRQ line being asserted and acknowledged. If <irqline> is omitted, then any IRQ line will stop execution.
|
| Examples:
|
|  gi
|
| Resume execution until the next break/watchpoint or until any IRQ is asserted and acknowledged on the current CPU.
|
|  gint 4
|
| Resume execution until the next break/watchpoint or until IRQ line 4 is asserted and acknowledged on the current CPU.
|
| Back to :ref:`debugger-execution-list`


 .. _debugger-command-gtime:

gtime
-----

|  **gt[ime] <milliseconds>**
|
| The gtime command resumes execution of the current code. Control will not be returned to the debugger until a specified delay has elapsed. The delay is in milliseconds.
|
| Example:
|
|  gtime #10000
|
| Resume execution for ten seconds
|
| Back to :ref:`debugger-execution-list`


 .. _debugger-command-next:

next
----

|  **n[ext]**
|
| The next command resumes execution and continues executing until the next time a different CPU is scheduled. Note that if you have used 'ignore' to ignore certain CPUs, you will not stop until a non-'ignore'd CPU is scheduled.
|
| Back to :ref:`debugger-execution-list`


 .. _debugger-command-focus:

focus
-----

|  **focus <cpu>**
|
| Sets the debugger focus exclusively to the given <cpu>. This is equivalent to specifying 'ignore' on all other CPUs.
|
| Example:
|
|  focus 1
|
| Focus exclusively CPU #1 while ignoring all other CPUs when using the debugger.
|
| Back to :ref:`debugger-execution-list`


 .. _debugger-command-ignore:

ignore
------

|  **ignore [<cpu>[,<cpu>[,...]]]**
|
| Ignores the specified <cpu> in the debugger. This means that you won't ever see execution on that CPU, nor will you be able to set breakpoints on that CPU. To undo this change use the 'observe' command. You can specify multiple <cpu>s in a single command. Note also that you are not permitted to ignore all CPUs; at least one must be active at all times.
|
| Examples:
|
|  ignore 1
|
| Ignore CPU #1 when using the debugger.
|
|  ignore 2,3,4
|
| Ignore CPU #2, #3 and #4 when using the debugger.
|
|  ignore
|
| List the CPUs that are currently ignored.
|
| Back to :ref:`debugger-execution-list`


 .. _debugger-command-observe:

observe
-------

|  **observe [<cpu>[,<cpu>[,...]]]**
|
| Re-enables interaction with the specified <cpu> in the debugger. This command undoes the effects of the 'ignore' command. You can specify multiple <cpu>s in a single command.
|
| Examples:
|
|  observe 1
|
| Stop ignoring CPU #1 when using the debugger.
|
|  observe 2,3,4
|
| Stop ignoring CPU #2, #3 and #4 when using the debugger.
|
|  observe
|
| List the CPUs that are currently observed.
|
| Back to :ref:`debugger-execution-list`


 .. _debugger-command-trace:

trace
-----

|  **trace {<filename>|OFF}[,<cpu>[,[noloop|logerror][,<action>]]]**
|
| Starts or stops tracing of the execution of the specified <cpu>. If <cpu> is omitted, the currently active CPU is specified.
|
| When enabling tracing, specify the filename in the <filename> parameter. To disable tracing, substitute the keyword 'off' for <filename>.
|
| <detectloops> should be either true or false.
|
| If 'noloop' is omitted, the trace will have loops detected and condensed to a single line. If 'noloop' is specified, the trace will contain every opcode as it is executed.
|
| If 'logerror' is specified, logerror output will augment the trace. If you wish to log additional information on each trace, you can append an <action> parameter which is a command that is executed before each trace is logged. Generally, this is used to include a 'tracelog' command. Note that you may need to embed the action within braces { } in order to prevent commas and semicolons from being interpreted as applying to the trace command itself.
|
|
| Examples:
|
|  trace joust.tr
|
| Begin tracing the currently active CPU, logging output to joust.tr.
|
|  trace dribling.tr,0
|
| Begin tracing the execution of CPU #0, logging output to dribling.tr.
|
|  trace starswep.tr,0,noloop
|
| Begin tracing the execution of CPU #0, logging output to starswep.tr, with loop detection disabled.
|
|  trace starswep.tr,0,logerror
|
| Begin tracing the execution of CPU #0, logging output (along with logerror output) to starswep.tr.
|
|  trace starswep.tr,0,logerror|noloop
|
| Begin tracing the execution of CPU #0, logging output (along with logerror output) to starswep.tr, with loop detection disabled.
|
|  trace >>pigskin.tr
|
| Begin tracing the currently active CPU, appending log output to pigskin.tr.
|
|  trace off,0
|
| Turn off tracing on CPU #0.
|
|  trace asteroid.tr,0,,{tracelog "A=%02X ",a}
|
| Begin tracing the execution of CPU #0, logging output to asteroid.tr. Before each line, output A=<aval> to the tracelog.
|
| Back to :ref:`debugger-execution-list`


 .. _debugger-command-traceover:

traceover
---------

|  **traceover {<filename>|OFF}[,<cpu>[,<detectloops>[,<action>]]]**
|
| Starts or stops tracing of the execution of the specified <cpu>.
|
| When tracing reaches a subroutine or call, tracing will skip over the subroutine. The same algorithm is used as is used in the step over command. This means that traceover will not work properly when calls are recursive or the return address is not immediately following the call instruction.
|
| <detectloops> should be either true or false. If <detectloops> is *true or omitted*, the trace will have loops detected and condensed to a single line. If it is false, the trace will contain every opcode as it is executed.
| If <cpu> is omitted, the currently active CPU is specified.
| When enabling tracing, specify the filename in the <filename> parameter.
| To disable tracing, substitute the keyword 'off' for <filename>.
| If you wish to log additional information on each trace, you can append an <action> parameter which is a command that is executed before each trace is logged. Generally, this is used to include a 'tracelog' command. Note that you may need to embed the action within braces { } in order to prevent commas and semicolons from being interpreted as applying to the trace command itself.
|
|
| Examples:
|
|  traceover joust.tr
|
| Begin tracing the currently active CPU, logging output to joust.tr.
|
|  traceover dribling.tr,0
|
| Begin tracing the execution of CPU #0, logging output to dribling.tr.
|
|  traceover starswep.tr,0,false
|
| Begin tracing the execution of CPU #0, logging output to starswep.tr, with loop detection disabled.
|
|  traceover off,0
|
| Turn off tracing on CPU #0.
|
|  traceover asteroid.tr,0,true,{tracelog "A=%02X ",a}
|
| Begin tracing the execution of CPU #0, logging output to asteroid.tr. Before each line, output A=<aval> to the tracelog.
|
| Back to :ref:`debugger-execution-list`


 .. _debugger-command-traceflush:

traceflush
----------

|  **traceflush**
|
| Flushes all open trace files.
|
| Back to :ref:`debugger-execution-list`
