.. _debugger-watchpoints-list:

Watchpoint Debugger Commands
============================

:ref:`debugger-command-wpset`
    sets memory access watchpoints
:ref:`debugger-command-wpclear`
    clears watchpoints
:ref:`debugger-command-wpdisable`
    disables watchpoints
:ref:`debugger-command-wpenable`
    enables enables watchpoints
:ref:`debugger-command-wplist`
    lists watchpoints

Watchpoints halt execution and activate the debugger when a CPU accesses
a location in a particular memory range.


.. _debugger-command-wpset:

wpset
-----

**wp[{d|i|o}][set] <address>[:<space>],<length>,<type>[,<condition>[,<action>]]**

Sets a new watchpoint starting at the specified **<address>** and
extending for **<length>**.  The range of the watchpoint is
**<address>** through **<address>+<length>-1**, inclusive.  The
**<address>** may optionally be followed by a CPU and/or address space
(see :ref:`debugger-devicespec` for details).  If an address space is
not specified, the command suffix sets the address space: ``wpset``
defaults to the first address space exposed by the CPU, ``wpdset``
defaults to the space with index 1 (data), ``wpiset`` defaults to the
space with index 2 (I/O), and ``wposet`` defaults to the space with
index 3 (opcodes).  The **<type>** parameter specifies the access types
to trap on – it can be one of three values: ``r`` for read accesses,
``w`` for write accesses, or ``rw`` for both read and write accesses.

The optional **<condition>** parameter lets you specify an expression
that will be evaluated each time the watchpoint is triggered.  If the
result of the expression is true (non-zero), the watchpoint will halt
execution; otherwise, execution will continue with no notification.  The
optional **<action>** parameter provides a command to be executed
whenever the watchpoint is triggered and the **<condition>** is true.
Note that you may need to surround the action with braces ``{ }`` to
ensure commas and semicolons within the command are not interpreted in
the context of the ``wpset`` command itself.

Each watchpoint that is set is assigned a numeric index which can be
used to refer to it in other watchpoint commands.  Watchpoint indices
are unique throughout a session.

To make **<condition>** expressions more useful, two variables are
available: for all watchpoints, the variable **wpaddr** is set to the
access address that triggered the watchpoint; for write watchpoints, the
variable **wpdata** is set to the data being written.

Examples:

``wp 1234,6,rw``
    Set a watchpoint for the visible CPU that will halt execution
    whenever a read or write to the first address space occurs in the
    address range 1234-1239, inclusive.
``wp 23456:data,a,w,wpdata == 1``
    Set a watchpoint for the visible CPU that will halt execution
    whenever a write to the ``data`` space occurs in the address range
    23456-2345f and the data written is equal to 1.
``wp 3456:maincpu,20,r,1,{ printf "Read @ %08X\n",wpaddr ; g }``
    Set a watchpoint for the CPU with the absolute tag path ``:maincpu``
    that will halt execution whenever a read from the first address
    space occurs in the address range 3456-3475.  When this happens,
    print **Read @ <wpaddr>** to the debugger console and resume
    execution.
``temp0 = 0 ; wp 45678,1,w,wpdata==f0,{ temp0++ ; g }``
    Set a watchpoint for the visible CPU that will halt execution
    whenever a write do the first address space occurs at address 45678
    where the value being written is equal to f0.  When this happens,
    increment the variable **temp0** and resume execution.

Back to :ref:`debugger-watchpoints-list`


.. _debugger-command-wpclear:

wpclear
-------

**wpclear [<wpnum>[,…]]**

Clear watchpoints.  If **<wpnum>** is specified, the watchpoints
referred to will be cleared.  If **<wpnum>** is not specified, all
watchpoints will be cleared.

Examples:

``wpclear 3``
    Clear the watchpoint with index 3.
``wpclear``
    Clear all watchpoints.

Back to :ref:`debugger-watchpoints-list`


.. _debugger-command-wpdisable:

wpdisable
---------

**wpdisable [<wpnum>[,…]]**

Disable watchpoints.  If **<wpnum>** is specified, the watchpoints
referred to will be disabled.  If **<wpnum>** is not specified, all
watchpoints will be disabled.

Note that disabling a watchpoint does not delete it, it just temporarily
marks the watchpoint as inactive.  Disabled watchpoints will not cause
execution to halt, their associated condition expressions will not be
evaluated, and their associated commands will not be executed.

Examples:

``wpdisable 3``
    Disable the watchpoint with index 3.
``wpdisable``
    Disable all watchpoints.

Back to :ref:`debugger-watchpoints-list`


.. _debugger-command-wpenable:

wpenable
--------

**wpenable [<wpnum>[,…]]**

Enable watchpoints.  If **<wpnum>** is specified, the watchpoints
referred to will be enabled.  If **<wpnum>** is not specified, all
watchpoints will be enabled.

Examples:

``wpenable 3``
    Enable the watchpoint with index 3.
``wpenable``
    Enable all watchpoints.

Back to :ref:`debugger-watchpoints-list`


.. _debugger-command-wplist:

wplist
------

**wplist [<CPU>]**

List current watchpoints, along with their indices and any associated
conditions or actions.  If no **<CPU>** is specified, watchpoints for
all CPUs in the system will be listed; if a **<CPU>** is specified, only
watchpoints for that CPU will be listed.  The **<CPU>** can be specified
by tag or by debugger CPU number (see :ref:`debugger-devicespec` for
details).

Examples:

``wplist``
    List all watchpoints.
``wplist .``
    List all watchpoints for the visible CPU.
``wplist maincpu``
    List all watchpoints for the CPU with the absolute tag path
    ``:maincpu``.

Back to :ref:`debugger-watchpoints-list`

