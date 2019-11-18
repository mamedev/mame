.. _debugger-watchpoints-list:

Watchpoint Debugger Commands
============================


You can also type **help <command>** for further details on each command in the MAME Debugger interface.

| :ref:`debugger-command-wpset` -- sets program, data, or I/O space watchpoint
| :ref:`debugger-command-wpclear` -- clears a given watchpoint or all if no <wpnum> specified
| :ref:`debugger-command-wpdisable` -- disables a given watchpoint or all if no <wpnum> specified
| :ref:`debugger-command-wpenable` -- enables a given watchpoint or all if no <wpnum> specified
| :ref:`debugger-command-wplist` -- lists all the watchpoints

 .. _debugger-command-wpset:

wpset
-----

|  **wp[{d|i}][set] <address>,<length>,<type>[,<condition>[,<action>]]**
|
| Sets a new watchpoint starting at the specified <address> and extending for <length>. The inclusive range of the watchpoint is <address> through <address> + <length> - 1.
| The 'wpset' command sets a watchpoint on program memory; the 'wpdset' command sets a watchpoint on data memory; and the 'wpiset' sets a watchpoint on I/O memory.
| The <type> parameter specifies which sort of accesses to trap on. It can be one of three values: 'r' for a read watchpoint 'w' for a write watchpoint, and 'rw' for a read/write watchpoint.
|
| The optional <condition> parameter lets you specify an expression that will be evaluated each time the watchpoint is hit. If the result of the expression is true (non-zero), the watchpoint will actually halt execution; otherwise, execution will continue with no notification.
| The optional <action> parameter provides a command that is executed whenever the watchpoint is hit and the <condition> is true. Note that you may need to embed the action within braces { } in order to prevent commas and semicolons from being interpreted as applying to the wpset command itself.
| Each watchpoint that is set is assigned an index which can be used in other watchpoint commands to reference this watchpoint.
| In order to help <condition> expressions, two variables are available. For all watchpoints, the variable 'wpaddr' is set to the address that actually triggered the watchpoint. For write watchpoints, the variable 'wpdata' is set to the data that is being written.
|
| Examples:
|
|  wp 1234,6,rw
|
| Set a watchpoint that will halt execution whenever a read or write occurs in the address range 1234-1239 inclusive.
|
|  wp 23456,a,w,wpdata == 1
|
| Set a watchpoint that will halt execution whenever a write occurs in the address range 23456-2345f AND the data written is equal to 1.
|
|  wp 3456,20,r,1,{printf "Read @ %08X\\n",wpaddr; g}
|
| Set a watchpoint that will halt execution whenever a read occurs in the address range 3456-3475. When this happens, print Read @ <wpaddr> and continue executing.
|
|  temp0 = 0; wp 45678,1,w,wpdata==f0,{temp0++; g}
|
| Set a watchpoint that will halt execution whenever a write occurs to the address 45678 AND the value being written is equal to f0. When that happens, increment the variable temp0 and resume execution.
|
| Back to :ref:`debugger-watchpoints-list`


 .. _debugger-command-wpclear:

wpclear
-------

|  **wpclear [<wpnum>]**
|
| The wpclear command clears a watchpoint. If <wpnum> is specified, only the requested watchpoint is cleared, otherwise all watchpoints are cleared.
|
| Examples:
|
|  wpclear 3
|
| Clear watchpoint index 3.
|
|  wpclear
|
| Clear all watchpoints.
|
| Back to :ref:`debugger-watchpoints-list`


 .. _debugger-command-wpdisable:

wpdisable
---------

|  **wpdisable [<wpnum>]**
|
| The wpdisable command disables a watchpoint. If <wpnum> is specified, only the requested watchpoint is disabled, otherwise all watchpoints are disabled. Note that disabling a watchpoint does not delete it, it just temporarily marks the watchpoint as inactive.
|
| Examples:
|
|  wpdisable 3
|
| Disable watchpoint index 3.
|
|  wpdisable
|
| Disable all watchpoints.
|
| Back to :ref:`debugger-watchpoints-list`


 .. _debugger-command-wpenable:

wpenable
--------

|  **wpenable [<wpnum>]**
|
| The wpenable command enables a watchpoint. If <wpnum> is specified, only the requested watchpoint is enabled, otherwise all watchpoints are enabled.
|
| Examples:
|
|  wpenable 3
|
| Enable watchpoint index 3.
|
|  wpenable
|
| Enable all watchpoints.
|
| Back to :ref:`debugger-watchpoints-list`


 .. _debugger-command-wplist:

wplist
------

|  **wplist**
|
| The wplist command lists all the current watchpoints, along with their index and any conditions or actions attached to them.
|
| Back to :ref:`debugger-watchpoints-list`

