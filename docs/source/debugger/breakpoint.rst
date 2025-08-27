.. _debugger-breakpoint-list:

Breakpoint Debugger Commands
============================

:ref:`debugger-command-bpset`
    sets a breakpoint at <address>
:ref:`debugger-command-bpclear`
    clears a specific breakpoint or all breakpoints
:ref:`debugger-command-bpdisable`
    disables a specific breakpoint or all breakpoints
:ref:`debugger-command-bpenable`
    enables a specific breakpoint or all breakpoints
:ref:`debugger-command-bplist`
    lists breakpoints

Breakpoints halt execution and activate the debugger before a CPU
executes an instruction at a particular address.


.. _debugger-command-bpset:

bpset
-----

**bp[set] <address>[:<CPU>][,<condition>[,<action>]]**

Sets a new execution breakpoint at the specified **<address>**.  The
**<address>** may optionally be followed by a colon and a tag or
debugger CPU number to set a breakpoint for a specific CPU.  If no CPU
is specified, the breakpoint will be set for the CPU currently visible
in the debugger.  When :ref:`source-level debugging <srcdbg>` is
enabled, :ref:`alternatives <srcdbg_bp>` to specifying **<address>**
are allowed.

The optional **<condition>** parameter lets you specify an expression
that will be evaluated each time the breakpoint address is hit.  If the
result of the expression is true (non-zero), the breakpoint will halt
execution; otherwise, execution will continue with no notification.  The
optional **<action>** parameter provides a command to be executed
whenever the breakpoint is hit and the **<condition>** is true.  Note
that you may need to surround the action with braces ``{ }`` to ensure
commas and semicolons within the command are not interpreted in the
context of the ``bpset`` command itself.

Each breakpoint that is set is assigned a numeric index which can be
used to refer to it in other breakpoint commands.  Breakpoint indices
are unique throughout a session.

Examples:

``bp 1234``
    Set a breakpoint for the visible CPU that will halt execution
    whenever the PC is equal to 1234.
``bp 23456,a0 == 0 && a1 == 0``
    Set a breakpoint for the visible CPU that will halt execution
    whenever the PC is equal to 23456 *and* the expression
    ``a0 == 0 && a1 == 0`` is true.
``bp 3456:audiocpu,1,{ printf "A0=%08X\n",a0 ; g }``
    Set a breakpoint for the CPU with the absolute tag path
    ``:audiocpu`` that will halt execution whenever the PC is equal to
    3456.  When this happens, print **A0=<a0val>** to the debugger
    console and resume execution.
``bp 45678:2,a0==100,{ a0 = ff ; g }``
    Set a breakpoint on the third CPU in the system (zero-based index)
    that will halt execution whenever the PC is equal to 45678 and the
    expression ``a0 == 100`` is true.  When that happens, set **a0** to
    ff and resume execution.
``temp0 = 0 ; bp 567890,++temp0 >= 10``
    Set a breakpoint for the visible CPU that will halt execution
    whenever the PC is equal to 567890 and the expression
    ``++temp0 >= 10`` is true.  This effectively breaks only after the
    breakpoint has been hit sixteen times.

Back to :ref:`debugger-breakpoint-list`


.. _debugger-command-bpclear:

bpclear
-------

**bpclear [<bpnum>[,…]]**

Clear breakpoints.  If **<bpnum>** is specified, the breakpoints
referred to will be cleared.  If **<bpnum>** is not specified, all
breakpoints will be cleared.

Examples:

``bpclear 3``
    Clear the breakpoint with index 3.
``bpclear``
    Clear all breakpoints.

Back to :ref:`debugger-breakpoint-list`


.. _debugger-command-bpdisable:

bpdisable
---------

**bpdisable [<bpnum>[,…]]**

Disable breakpoints.  If **<bpnum>** is specified, the breakpoints
referred to will be disabled.  If **<bpnum>** is not specified, all
breakpoints will be disabled.

Note that disabling a breakpoint does not delete it, it just temporarily
marks the breakpoint as inactive.  Disabled breakpoints will not cause
execution to halt, their associated condition expressions will not be
evaluated, and their associated commands will not be executed.

Examples:

``bpdisable 3``
    Disable the breakpoint with index 3.
``bpdisable``
    Disable all breakpoints.

Back to :ref:`debugger-breakpoint-list`


.. _debugger-command-bpenable:

bpenable
--------

**bpenable [<bpnum>[,…]]**

Enable breakpoints.  If **<bpnum>** is specified, the breakpoint
referred to will be enabled.  If **<bpnum>** is not specified, all
breakpoints will be enabled.

Examples:

``bpenable 3``
    Enable the breakpoint with index 3.
``bpenable``
    Enable all breakpoints.

Back to :ref:`debugger-breakpoint-list`


.. _debugger-command-bplist:

bplist
------

**bplist [<CPU>]**

List current breakpoints, along with their indices and any associated
conditions or actions.  If no **<CPU>** is specified, breakpoints for
all CPUs in the system will be listed; if a **<CPU>** is specified, only
breakpoints for that CPU will be listed.  The **<CPU>** can be specified
by tag or by debugger CPU number (see :ref:`debugger-devicespec` for
details).

Examples:

``bplist``
    List all breakpoints.
``bplist .``
    List all breakpoints for the visible CPU.
``bplist maincpu``
    List all breakpoints for the CPU with the absolute tag path
    ``:maincpu``.

Back to :ref:`debugger-breakpoint-list`
