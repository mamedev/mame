.. _debugger-exceptionpoint-list:

Exceptionpoint Debugger Commands
================================

:ref:`debugger-command-epset`
    sets a new exceptionpoint
:ref:`debugger-command-epclear`
    clears a specific exceptionpoint or all exceptionpoints
:ref:`debugger-command-epdisable`
    disables a specific exceptionpoint or all exceptionpoints
:ref:`debugger-command-epenable`
    enables a specific exceptionpoint or all exceptionpoints
:ref:`debugger-command-eplist`
    lists exceptionpoints

Exceptionpoints halt execution and activate the debugger when a CPU
raises a particular exception number.


.. _debugger-command-epset:

epset
-----

**ep[set] <type>[,<condition>[,<action>]]**

Sets a new exceptionpoint for exceptions of type **<type>**.  The
optional **<condition>** parameter lets you specify an expression that
will be evaluated each time the exceptionpoint is hit.  If the result
of the expression is true (non-zero), the exceptionpoint will actually
halt execution at the start of the exception handler; otherwise,
execution will continue with no notification.  The optional **<action>**
parameter provides a command that is executed whenever the
exceptionpoint is hit and the **<condition>** is true.  Note that you
may need to embed the action within braces ``{ }`` in order to prevent
commas and semicolons from being interpreted as applying to the
``epset`` command itself.

The numbering of exceptions depends upon the CPU type.  Causes of
exceptions may include internally or externally vectored interrupts,
errors occurring within instructions and system calls.

Each exceptionpoint that is set is assigned an index which can be used
in other exceptionpoint commands to reference this exceptionpoint.

Examples:

``ep 2``
  Set an exception that will halt execution whenever the visible CPU
  raises exception number 2.

Back to :ref:`debugger-exceptionpoint-list`


.. _debugger-command-epclear:

epclear
-------

**epclear [<epnum>[,…]]**

The epclear command clears exceptionpoints.  If **<epnum>** is
specified, only the requested exceptionpoints are cleared, otherwise
all exceptionpoints are cleared.

Examples:

``epclear 3``
  Clear exceptionpoint index 3.

``epclear``
  Clear all exceptionpoints.

Back to :ref:`debugger-exceptionpoint-list`


.. _debugger-command-epdisable:

epdisable
---------

**epdisable [<epnum>[,…]]**

The epdisable command disables exceptionpoints.  If **<epnum>** is
specified, only the requested exceptionpoints are disabled, otherwise
all exceptionpoints are disabled.  Note that disabling an
exceptionpoint does not delete it, it just temporarily marks the
exceptionpoint as inactive.

Examples:

``epdisable 3``
  Disable exceptionpoint index 3.

``epdisable``
  Disable all exceptionpoints.

Back to :ref:`debugger-exceptionpoint-list`


.. _debugger-command-epenable:

epenable
--------

**epenable [<epnum>[,…]]**

The epenable command enables exceptionpoints.  If **<epnum>** is
specified, only the requested exceptionpoints are enabled, otherwise
all exceptionpoints are enabled.

Examples:

``epenable 3``
  Enable exceptionpoint index 3.

``epenable``
  Enable all exceptionpoints.

Back to :ref:`debugger-exceptionpoint-list`


.. _debugger-command-eplist:

eplist
------

**eplist**

The eplist command lists all the current exceptionpoints, along with
their index and any conditions or actions attached to them.

Back to :ref:`debugger-exceptionpoint-list`
