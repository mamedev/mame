.. _debugger-registerpoints-list:

Registerpoints Debugger Commands
================================


You can also type **help <command>** for further details on each command in the MAME Debugger interface.

| :ref:`debugger-command-rpset` -- sets a registerpoint to trigger on <condition>
| :ref:`debugger-command-rpclear` -- clears a given registerpoint or all if no <rpnum> specified
| :ref:`debugger-command-rpdisable` -- disabled a given registerpoint or all if no <rpnum> specified
| :ref:`debugger-command-rpenable` -- enables a given registerpoint or all if no <rpnum> specified
| :ref:`debugger-command-rplist` -- lists all the registerpoints



 .. _debugger-command-rpset:

rpset
-----

|  **rp[set] {<condition>}[,<action>]]**
|
| Sets a new registerpoint which will be triggered when <condition> is met. The condition must be specified between curly braces to prevent the condition from being evaluated as an assignment.
| The optional <action> parameter provides a command that is executed whenever the registerpoint is hit. Note that you may need to embed the action within braces { } in order to prevent commas and semicolons from being interpreted as applying to the rpset command itself.
| Each registerpoint that is set is assigned an index which can be used in other registerpoint commands to reference this registerpoint.
|
| Examples:
|
|  rp {PC==0150}
|
| Set a registerpoint that will halt execution whenever the PC register equals 0x150.
|
|  temp0=0; rp {PC==0150},{temp0++; g}
|
| Set a registerpoint that will increment the variable temp0 whenever the PC register equals 0x0150.
|
|  rp {temp0==5}
|
| Set a registerpoint that will halt execution whenever the temp0 variable equals 5.
|
| Back to :ref:`debugger-registerpoints-list`


 .. _debugger-command-rpclear:

rpclear
-------

|  **rpclear [<rpnum>]**
|
| The rpclear command clears a registerpoint. If <rpnum> is specified, only the requested registerpoint is cleared, otherwise all registerpoints are cleared.
|
| Examples:
|
|  rpclear 3
|
| Clear registerpoint index 3.
|
|  rpclear
|
| Clear all registerpoints.
|
| Back to :ref:`debugger-registerpoints-list`


 .. _debugger-command-rpdisable:

rpdisable
---------

|  **rpdisable [<rpnum>]**
|
| The rpdisable command disables a registerpoint. If <rpnum> is specified, only the requested registerpoint is disabled, otherwise all registerpoints are disabled. Note that disabling a registerpoint does not delete it, it just temporarily marks the registerpoint as inactive.
|
| Examples:
|
|  rpdisable 3
|
| Disable registerpoint index 3.
|
|  rpdisable
|
| Disable all registerpoints.
|
| Back to :ref:`debugger-registerpoints-list`


 .. _debugger-command-rpenable:

rpenable
--------

|  **rpenable [<rpnum>]**
|
| The rpenable command enables a registerpoint. If <rpnum> is specified, only the requested registerpoint is enabled, otherwise all registerpoints are enabled.
|
| Examples:
|
|  rpenable 3
|
| Enable registerpoint index 3.
|
|  rpenable
|
| Enable all registerpoints.
|
| Back to :ref:`debugger-registerpoints-list`


 .. _debugger-command-rplist:

rplist
------

|  **rplist**
|
| The rplist command lists all the current registerpoints, along with their index and any actions attached to them.
|
| Back to :ref:`debugger-registerpoints-list`
