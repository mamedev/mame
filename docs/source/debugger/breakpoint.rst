.. _debugger-breakpoint-list:

Breakpoint Debugger Commands
============================


You can also type **help <command>** for further details on each command in the MAME Debugger interface.

| :ref:`debugger-command-bpset` -- sets breakpoint at <address>
| :ref:`debugger-command-bpclear` -- clears a given breakpoint or all if no <bpnum> specified
| :ref:`debugger-command-bpdisable` -- disables a given breakpoint or all if no <bpnum> specified
| :ref:`debugger-command-bpenable` -- enables a given breakpoint or all if no <bpnum> specified
| :ref:`debugger-command-bplist` -- lists all the breakpoints


 .. _debugger-command-bpset:

bpset
-----

|  **bp[set] <address>[,<condition>[,<action>]]**
|
| Sets a new execution breakpoint at the specified <address>.
| The optional <condition> parameter lets you specify an expression that will be evaluated each time the breakpoint is hit. If the result of the expression is true (non-zero), the breakpoint will actually halt execution; otherwise, execution will continue with no notification.
| The optional <action> parameter provides a command that is executed whenever the breakpoint is hit and the <condition> is true. Note that you may need to embed the action within braces { } in order to prevent commas and semicolons from being interpreted as applying to the bpset command itself. Each breakpoint that is set is assigned an index which can be used in other breakpoint commands to reference this breakpoint.
|
| Examples:
|
|  bp 1234
|
| Set a breakpoint that will halt execution whenever the PC is equal to 1234.
|
|  bp 23456,a0 == 0 && a1 == 0
|
| Set a breakpoint that will halt execution whenever the PC is equal to 23456 AND the expression (a0 == 0 && a1 == 0) is true.
|
|  bp 3456,1,{printf "A0=%08X\\n",a0; g}
|
| Set a breakpoint that will halt execution whenever the PC is equal to 3456. When this happens, print A0=<a0val> and continue executing.
|
|  bp 45678,a0==100,{a0 = ff; g}
|
| Set a breakpoint that will halt execution whenever the PC is equal to 45678 AND the expression (a0 == 100) is true. When that happens, set a0 to ff and resume execution.
|
|  temp0 = 0; bp 567890,++temp0 >= 10
|
| Set a breakpoint that will halt execution whenever the PC is equal to 567890 AND the expression (++temp0 >= 10) is true. This effectively breaks only after the breakpoint has been hit 16 times.
|
| Back to :ref:`debugger-breakpoint-list`


 .. _debugger-command-bpclear:

bpclear
-------

|  **bpclear [<bpnum>]**
|
| The bpclear command clears a breakpoint. If <bpnum> is specified, only the requested breakpoint is cleared, otherwise all breakpoints are cleared.
|
| Examples:
|
|  bpclear 3
|
| Clear breakpoint index 3.
|
|  bpclear
|
| Clear all breakpoints.
|
| Back to :ref:`debugger-breakpoint-list`


 .. _debugger-command-bpdisable:

bpdisable
---------

|  **bpdisable [<bpnum>]**
|
| The bpdisable command disables a breakpoint. If <bpnum> is specified, only the requested breakpoint is disabled, otherwise all breakpoints are disabled. Note that disabling a breakpoint does not delete it, it just temporarily marks the breakpoint as inactive.
|
| Examples:
|
|  bpdisable 3
|
| Disable breakpoint index 3.
|
|  bpdisable
|
| Disable all breakpoints.
|
| Back to :ref:`debugger-breakpoint-list`


 .. _debugger-command-bpenable:

bpenable
--------

|  **bpenable [<bpnum>]**
|
| The bpenable command enables a breakpoint. If <bpnum> is specified, only the requested breakpoint is enabled, otherwise all breakpoints are enabled.
|
| Examples:
|
|  bpenable 3
|
| Enable breakpoint index 3.
|
|  bpenable
|
| Enable all breakpoints.
|
| Back to :ref:`debugger-breakpoint-list`


 .. _debugger-command-bplist:

bplist
------

|  **bplist**
|
| The bplist command lists all the current breakpoints, along with their index and any conditions or actions attached to them.
|
| Back to :ref:`debugger-breakpoint-list`

