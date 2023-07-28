.. _luascript-ref-debugger:

Lua Debugger Classes
====================

Some of MAME’s core debugging features can be controlled from Lua script.  The
debugger must be enabled to use the debugger features (usually by passing
``-debug`` on the command line).

.. contents::
    :local:
    :depth: 1


.. _luascript-ref-debugsymtable:

Symbol table
------------

Wrap’s MAME’s ``symbol_table`` class, providing named symbols that can be used
in expressions.  Note that symbol tables can be created and used even when the
debugger is not enabled.

Instantiation
~~~~~~~~~~~~~

emu.symbol_table(machine)
    Creates a new symbol table in the context of the specified machine,
emu.symbol_table(parent, [device])
    Creates a new symbol table with the specified parent symbol table.  If a
    device is specified and it implements ``device_memory_interface``, it will
    be used as the base for looking up address spaces and memory regions.  Note
    that if a device that does not implement ``device_memory_interface`` is
    supplied, it will not be used (address spaces and memory regions will be
    looked up relative to the root device).
emu.symbol_table(device)
    Creates a new symbol table in the context of the specified device.  If the
    device implements ``device_memory_interface``, it will be used as the base
    for looking up address spaces and memory regions.  Note that if a device
    that does not implement ``device_memory_interface`` is supplied, it will
    only be used to determine the machine context (address spaces and memory
    regions will be looked up relative to the root device).

Methods
~~~~~~~

symbols:set_memory_modified_func(cb)
    Set a function to call when memory is modified via the symbol table.  No
    arguments are passed to the function and any return values are ignored.
    Call with ``nil`` to remove the callback.
symbols:add(name, [value])
    Adds a named integer symbol.  The name must be a string.  If a value is
    supplied, it must be an integer.  If a value is supplied, a read-only symbol
    is added with the supplied value.  If no value is supplied, a read/write
    symbol is created with and initial value of zero.  If a symbol entry with
    the specified name already exists in the symbol table, it will be replaced.

    Returns the new :ref:`symbol entry <luascript-ref-debugsymentry>`.
symbols:add(name, getter, [setter], [format])
    Adds a named integer symbol using getter and optional setter callbacks.  The
    name must be a string.  The getter must be a function returning an integer
    for the symbol value.  If supplied, the setter must be a function that
    accepts a single integer argument for the new value of the symbol.  A format
    string for displaying the symbol value may optionally be supplied.  If a
    symbol entry with the specified name already exists in the symbol table, it
    will be replaced.

    Returns the new :ref:`symbol entry <luascript-ref-debugsymentry>`.
symbols:add(name, minparams, maxparams, execute)
    Adds a named function symbol.  The name must be a string.  The minimum and
    maximum numbers of parameters must be integers.  If a symbol entry with the
    specified name already exists in the symbol table, it will be replaced.

    Returns the new :ref:`symbol entry <luascript-ref-debugsymentry>`.
symbols:find(name)
    Returns the :ref:`symbol entry <luascript-ref-debugsymentry>` with the
    specified name, or ``nil`` if there is no symbol with the specified name in
    the symbol table.
symbols:find_deep(name)
    Returns the :ref:`symbol entry <luascript-ref-debugsymentry>` with the
    specified name, or ``nil`` if there is no symbol with the specified name in
    the symbol table or any of its parent symbol tables.
symbols:value(name)
    Returns the integer value of the symbol with the specified name, or zero if
    there is no symbol with the specified name in the symbol table or any of its
    parent symbol tables.  Raises an error if the symbol with specified name is
    a function symbol.
symbols:set_value(name, value)
    Sets the value of the symbol with the specified name.  Raises an error if
    the symbol with the specified name is a read-only integer symbol or if it is
    a function symbol.  Has no effect if there is no symbol with the specified
    name in the symbol table or any of its parent symbol tables.
symbols:memory_value(name, space, offset, size, disable_se)
    Read a value from memory.  Supply the name or tag of the address space or
    memory region to read from, or ``nil`` to use the address space or memory
    region implied by the ``space`` argument.  See
    :ref:`memory accesses in debugger expressions <debugger-express-mem>` for
    access type specifications that can be used for the ``space`` argument.
    The access size is specified in bytes, and must be 1, 2, 4 or 8.  The
    ``disable_se`` argument specifies whether memory access side effects should
    be disabled.
symbols:set_memory_value(name, space, offset, value, size, disable_se)
    Write a value to memory.  Supply the name or tag of the address space or
    memory region to write to, or ``nil`` to use the address space or memory
    region implied by the ``space`` argument.  See
    :ref:`memory accesses in debugger expressions <debugger-express-mem>` for
    access type specifications that can be used for the ``space`` argument.
    The access size is specified in bytes, and must be 1, 2, 4 or 8.  The
    ``disable_se`` argument specifies whether memory access side effects should
    be disabled.
symbols:read_memory(space, address, size, apply_translation)
    Read a value from an address space.  The access size is specified in bytes,
    and must be 1, 2, 4, or 8.  If the ``apply_translation`` argument is true,
    the address will be translated with debug read intention.  Returns a value
    of the requested size with all bits set if address translation fails.
symbols:write_memory(space, address, data, size, apply_translation)
    Write a value to an address space.  The access size is specified in bytes,
    and must be 1, 2, 4, or 8.  If the ``apply_translation`` argument is true,
    the address will be translated with debug write intention.  The symbol
    table’s memory modified function will be called after the value is written.
    The value will not be written and the symbol table’s memory modified
    function will not be called if address translation fails.

Properties
~~~~~~~~~~

symbols.entries[]
    The :ref:`symbol entries <luascript-ref-debugsymentry>` in the symbol table,
    indexed by name.  The ``at`` and ``index_of`` methods have O(n) complexity;
    all other supported operations have O(1) complexity.
symbols.parent (read-only)
    The parent symbol table, or ``nil`` if the symbol table has no parent.


.. _luascript-ref-debugexpression:

Parsed expression
-----------------

Wraps MAME’s ``parsed_expression`` class, which represents a tokenised debugger
expression.  Note that parsed expressions can be created and used even when the
debugger is not enabled.

Instantiation
~~~~~~~~~~~~~

emu.parsed_expression(symbols)
    Creates an empty expression that will use the supplied
    :ref:`symbol table <luascript-ref-debugsymtable>` to look up symbols.
emu.parsed_expression(symbols, string, [default_base])
    Creates an expression by parsing the supplied string, looking up symbols in
    the supplied :ref:`symbol table <luascript-ref-debugsymtable>`.  If the
    default base for interpreting integer literals is not supplied, 16 is used
    (hexadecimal).  Raises an :ref:`expression error
    <luascript-ref-debugexpressionerror>` if the string contains syntax errors
    or uses undefined symbols.

Methods
~~~~~~~

expression:set_default_base(base)
    Set the default base for interpreting numeric literals.  The base must be a
    positive integer.
expression:parse(string)
    Parse a debugger expression string.  Replaces the current contents of the
    expression if it is not empty.  Raises an :ref:`expression error
    <luascript-ref-debugexpressionerror>` if the string contains syntax errors
    or uses undefined symbols.  The previous content of the expression is not
    preserved when attempting to parse an invalid expression string.
expression:execute()
    Evaluates the expression, returning an unsigned integer result.  Raises an
    :ref:`expression error <luascript-ref-debugexpressionerror>` if the
    expression cannot be evaluated (e.g. attempting to call a function with an
    invalid number of arguments).

Properties
~~~~~~~~~~

expression.is_empty (read-only)
    A Boolean indicating whether the expression contains no tokens.
expression.original_string (read-only)
    The original string that was parsed to create the expression.
expression.symbols (read/write)
    The :ref:`symbol table <luascript-ref-debugsymtable>` used for to look up
    symbols in the expression.


.. _luascript-ref-debugsymentry:

Symbol entry
------------

Wraps MAME’s ``symbol_entry`` class, which represents an entry in a
:ref:`symbol table <luascript-ref-debugsymtable>`.  Note that symbol entries
must not be used after the symbol table they belong to is destroyed.

Instantiation
~~~~~~~~~~~~~

symbols:add(name, [value])
    Adds an integer symbol to a
    :ref:`symbol table <luascript-ref-debugsymtable>`, returning the new symbol
    entry.
symbols:add(name, getter, [setter], [format])
    Adds an integer symbol to a
    :ref:`symbol table <luascript-ref-debugsymtable>`, returning the new symbol
    entry.
symbols:add(name, minparams, maxparams, execute)
    Adds function symbol to a
    :ref:`symbol table <luascript-ref-debugsymtable>`, returning the new symbol
    entry.

Properties
~~~~~~~~~~

entry.name (read-only)
    The name of the symbol entry.
entry.format (read-only)
    The format string used to convert the symbol entry to text for display.
entry.is_function (read-only)
    A Boolean indicating whether the symbol entry is a callable function.
entry.is_lval (read-only)
    A Boolean indicating whether the symbol entry is an integer symbol that can
    be set (i.e. whether it can be used on the left-hand side of assignment
    expressions).
entry.value (read/write)
    The integer value of the symbol entry.  Attempting to set the value raises
    an error if the symbol entry is read-only.  Attempting to get or set the
    value of a function symbol raises an error.


.. _luascript-ref-debugman:

Debugger manager
----------------

Wraps MAME’s ``debugger_manager`` class, providing the main interface to control
the debugger.

Instantiation
~~~~~~~~~~~~~

manager.machine.debugger
    Returns the global debugger manager instance, or ``nil`` if the debugger is
    not enabled.

Methods
~~~~~~~

debugger:command(str)
    Execute a debugger console command.  The argument is the command string.
    The output is sent to both the debugger console and the Lua console.

Properties
~~~~~~~~~~

debugger.consolelog[] (read-only)
    The lines in the console log (output from debugger commands).  This
    container only supports index and length operations.
debugger.errorlog[] (read-only)
    The lines in the error log (``logerror`` output).  This container only
    supports index and length operations.
debugger.visible_cpu (read/write)
    The CPU device with debugger focus.  Changes become visible in the debugger
    console after the next step.  Setting to a device that is not a CPU has no
    effect.
debugger.execution_state (read/write)
    Either ``"run"`` if the emulated system is running, or ``"stop"`` if it is
    stopped in the debugger.


.. _luascript-ref-devdebug:

Device debugger interface
-------------------------

Wraps MAME’s ``device_debug`` class, providing the debugger interface to an
emulated CPU device.

Instantiation
~~~~~~~~~~~~~

manager.machine.devices[tag].debug
    Returns the debugger interface for an emulated CPU device, or ``nil`` if the
    device is not a CPU.

Methods
~~~~~~~

debug:step([cnt])
    Step by the specified number of instructions.  If the instruction count is
    not provided, it defaults to a single instruction.
debug:go()
    Run the emulated CPU.
debug:bpset(addr, [cond], [act])
    Set a breakpoint at the specified address, with an optional condition and
    action.  If the action is not specified, it defaults to just breaking into
    the debugger.  Returns the breakpoint number for the new breakpoint.

    If specified, the condition must be a debugger expression that will be
    evaluated each time the breakpoint is hit.  Execution will only be stopped
    if the expression evaluates to a non-zero value.  If the condition is not
    specified, it defaults to always active.
debug:bpenable([bp])
    Enable the specified breakpoint, or all breakpoints for the device if no
    breakpoint number is specified.  Returns whether the specified number
    matched a breakpoint if a breakpoint number is specified, or ``nil`` if no
    breakpoint number is specified.
debug:bpdisable([bp])
    Disable the specified breakpoint, or all breakpoints for the device if no
    breakpoint number is specified.  Returns whether the specified number
    matched a breakpoint if a breakpoint number is specified, or ``nil`` if no
    breakpoint number is specified.
debug:bpclear([bp])
    Clear the specified breakpoint, or all breakpoints for the device if no
    breakpoint number is specified.  Returns whether the specified number
    matched a breakpoint if a breakpoint number is specified, or ``nil`` if no
    breakpoint number is specified.
debug:bplist()
    Returns a table of breakpoints for the device.  The keys are the breakpoint
    numbers, and the values are
    :ref:`breakpoint objects <luascript-ref-breakpoint>`.
debug:wpset(space, type, addr, len, [cond], [act])
    Set a watchpoint over the specified address range, with an optional
    condition and action.  The type must be ``"r"``, ``"w"`` or ``"rw"`` for a
    read, write or read/write breakpoint.  If the action is not specified, it
    defaults to just breaking into the debugger.  Returns the watchpoint number
    for the new watchpoint.

    If specified, the condition must be a debugger expression that will be
    evaluated each time the breakpoint is hit.  Execution will only be stopped
    if the expression evaluates to a non-zero value.  The variable 'wpaddr' is
    set to the address that actually triggered the watchpoint, the variable
    'wpdata' is set to the data that is being read or written, and the variable
    'wpsize' is set to the size of the data in bytes.  If the condition is not
    specified, it defaults to always active.
debug:wpenable([wp])
    Enable the specified watchpoint, or all watchpoints for the device if no
    watchpoint number is specified.  Returns whether the specified number
    matched a watchpoint if a watchpoint number is specified, or ``nil`` if no
    watchpoint number is specified.
debug:wpdisable([wp])
    Disable the specified watchpoint, or all watchpoints for the device if no
    watchpoint number is specified.  Returns whether the specified number
    matched a watchpoint if a watchpoint number is specified, or ``nil`` if no
    watchpoint number is specified.
debug:wpclear([wp])
    Clear the specified watchpoint, or all watchpoints for the device if no
    watchpoint number is specified.  Returns whether the specified number
    matched a watchpoint if a watchpoint number is specified, or ``nil`` if no
    watchpoint number is specified.
debug:wplist(space)
    Returns a table of watchpoints for the specified address space of the
    device.  The keys are the watchpoint numbers, and the values are
    :ref:`watchpoint objects <luascript-ref-watchpoint>`.


.. _luascript-ref-breakpoint:

Breakpoint
----------

Wraps MAME’s ``debug_breakpoint`` class, representing a breakpoint for an
emulated CPU device.

Instantiation
~~~~~~~~~~~~~

manager.machine.devices[tag].debug:bplist()[bp]
    Gets the specified breakpoint for an emulated CPU device, or ``nil`` if no
    breakpoint corresponds to the specified index.

Properties
~~~~~~~~~~

breakpoint.index (read-only)
    The breakpoint’s index.  The can be used to enable, disable or clear the
    breakpoint via the
    :ref:`CPU debugger interface <luascript-ref-devdebug>`.
breakpoint.enabled (read/write)
    A Boolean indicating whether the breakpoint is currently enabled.
breakpoint.address (read-only)
    The breakpoint’s address.
breakpoint.condition (read-only)
    A debugger expression evaluated each time the breakpoint is hit.  The action
    will only be triggered if this expression evaluates to a non-zero value.  An
    empty string if no condition was specified.
breakpoint.action (read-only)
    An action the debugger will run when the breakpoint is hit and the condition
    evaluates to a non-zero value.  An empty string if no action was specified.


.. _luascript-ref-watchpoint:

Watchpoint
----------

Wraps MAME’s ``debug_watchpoint`` class, representing a watchpoint for an
emulated CPU device.

Instantiation
~~~~~~~~~~~~~

manager.machine.devices[tag].debug:wplist(space)[wp]
    Gets the specified watchpoint for an address space of an emulated CPU
    device, or ``nil`` if no watchpoint in the address space corresponds to the
    specified index.

Properties
~~~~~~~~~~

watchpoint.index (read-only)
    The watchpoint’s index.  The can be used to enable, disable or clear the
    watchpoint via the
    :ref:`CPU debugger interface <luascript-ref-devdebug>`.
watchpoint.enabled (read/write)
    A Boolean indicating whether the watchpoint is currently enabled.
watchpoint.type (read-only)
    Either ``"r"``, ``"w"`` or ``"rw"`` for a read, write or read/write
    watchpoint.
watchpoint.address (read-only)
    The starting address of the watchpoint’s address range.
watchpoint.length (read-only)
    The length of the watchpoint’s address range.
watchpoint.condition (read-only)
    A debugger expression evaluated each time the watchpoint is hit.  The action
    will only be triggered if this expression evaluates to a non-zero value.  An
    empty string if no condition was specified.
watchpoint.action (read-only)
    An action the debugger will run when the watchpoint is hit and the condition
    evaluates to a non-zero value.  An empty string if no action was specified.


.. _luascript-ref-debugexpressionerror:

Expression error
----------------

Wraps MAME’s ``expression_error`` class, describing an error occurring while
parsing or executing a debugger expression.  Raised on errors when using
:ref:`parsed expressions <luascript-ref-debugexpression>`.  Can be converted to
a string to provide a description of the error.

Properties
~~~~~~~~~~

err.code (read-only)
    An implementation-dependent number representing the category of error.
    Should not be displayed to the user.
err.offset (read-only)
    The offset within the expression string where the error was encountered.
