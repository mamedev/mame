.. _debugger-registerpoints-list:

Registerpoint Debugger Commands
================================

:ref:`debugger-command-rpset`
    sets a registerpoint to trigger on a condition
:ref:`debugger-command-rpclear`
    clears registerpoints
:ref:`debugger-command-rpdisable`
    disables a registerpoint
:ref:`debugger-command-rpenable`
    enables registerpoints
:ref:`debugger-command-rplist`
    lists registerpoints

Registerpoints evaluate an expression each time a CPU executes an
instruction and halt execution and activate the debugger if the result
is true (non-zero).


.. _debugger-command-rpset:

rpset
-----

**rp[set] <condition>[,<action>]**

Sets a new registerpoint which will be triggered when the expression
supplied as the **<condition>** evaluates to true (non-zero).  Note that
the condition may need to be surrounded with braces ``{ }`` to prevent
it from being interpreted as an assignment.  The optional **<action>**
parameter provides a command to be executed whenever the registerpoint
is triggered.  Note that you may need to surround the action with braces
``{ }`` to ensure commas and semicolons within the command are not
interpreted in the context of the ``rpset`` command itself.

Each registerpoint that is set is assigned a numeric index which can be
used to refer to it in other registerpoint commands.  Registerpoint
indices are unique throughout a session.

Examples:

``rp {PC==150}``
    Set a registerpoint that will halt execution whenever the **PC**
    register equals 150.
``temp0=0; rp {PC==150},{temp0++; g}``
    Set a registerpoint that will increment the variable **temp0**
    whenever the **PC** register equals 150.
``rp {temp0==5}``
    Set a registerpoint that will halt execution whenever the **temp0**
    variable equals 5.

Back to :ref:`debugger-registerpoints-list`


.. _debugger-command-rpclear:

rpclear
-------

**rpclear [<rpnum>,[,…]]**

Clears registerpoints.  If **<rpnum>** is specified, the registerpoints
referred to will be cleared.  If **<rpnum>** is not specified, all
registerpoints will be cleared.

Examples:

``rpclear 3``
    Clear the registerpoint with index 3.
``rpclear``
    Clear all registerpoints.

Back to :ref:`debugger-registerpoints-list`


.. _debugger-command-rpdisable:

rpdisable
---------

**rpdisable [<rpnum>[,…]]**

Disables registerpoints.  If **<rpnum>** is specified, the
registerpoints referred to will be disabled.  If **<rpnum>** is not
specified, all registerpoints will be disabled.

Note that disabling a registerpoint does not delete it, it just
temporarily marks the registerpoint as inactive.  Disabled
registerpoints will not cause execution to halt, their condition
expressions will not be evaluated, and their associated commands will
not be executed.

Examples:

``rpdisable 3``
    Disable the registerpoint with index 3.
``rpdisable``
    Disable all registerpoints.

Back to :ref:`debugger-registerpoints-list`


.. _debugger-command-rpenable:

rpenable
--------

**rpenable [<rpnum>[,…]]**

Enables registerpoints.  If **<rpnum>** is specified, the registerpoints
referred to will be enabled.  If **<rpnum>** is not specified, all
registerpoints will be enabled.

Examples:

``rpenable 3``
    Enable the registerpoint with index 3.
``rpenable``
    Enable all registerpoints.

Back to :ref:`debugger-registerpoints-list`


.. _debugger-command-rplist:

rplist
------

**rplist [<CPU>]**

List current registerpoints, along with their indices and conditions,
and any associated actions actions.  If no **<CPU>** is specified,
registerpoints for all CPUs in the system will be listed; if a **<CPU>**
is specified, only registerpoints for that CPU will be listed.  The
**<CPU>** can be specified by tag or by debugger CPU number (see
:ref:`debugger-devicespec` for details).

Examples:

``rplist``
    List all registerpoints.
``rplist .``
    List all registerpoints for the visible CPU.
``rplist maincpu``
    List all registerpoints for the CPU with the absolute tag path
    ``:maincpu``.

Back to :ref:`debugger-registerpoints-list`
