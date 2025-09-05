.. _umlinst:

UML Instruction Reference
=========================

.. contents::
    :local:
    :depth: 2


.. _umlinst-intro:

Introduction
------------

UML is the instruction set used by MAME’s recompiler framework.
Front-ends translate code running on the guest CPUs to UML instructions,
and back-ends convert the UML instructions to a form that can be
executed or interpreted on the host system.

Many UML instruction have multiple instruction sizes.  Integer instructions
default to 32-bit size.  Adding a ``D`` or ``d`` prefix to the mnemonic changes
to 64-bit size (double word).  Floating point instructions use the mnemonic
prefix/suffix ``FS`` or ``fs`` for IEEE 754 32-bit format (single precision) or
or the prefix/suffix ``FD`` or ``fd`` for IEEE 754 64-bit format (double
precision).


.. _umlinst-flow:

Flow control
------------

.. _umlinst-comment:

COMMENT
~~~~~~~

Insert a comment into logged UML code.

+--------------------+---------------------------------+
| Disassembly        | Usage                           |
+====================+=================================+
| .. code-block::    | .. code-block:: C++             |
|                    |                                 |
|     comment string |     UML_COMMENT(block, string); |
+--------------------+---------------------------------+

Operands
^^^^^^^^

string
    The comment text as a pointer to a NUL-terminated string.  This must
    remain valid until code is generated for the block.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-nop:

NOP
~~~

No operation.

+-----------------+---------------------+
| Disassembly     | Usage               |
+=================+=====================+
| .. code-block:: | .. code-block:: C++ |
|                 |                     |
|     nop         |     UML_NOP(block); |
+-----------------+---------------------+

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-label:

LABEL
~~~~~

Associate a location with a label number local to the current generated
code block.  Label numbers must not be reused within a generated code
block.  The :ref:`JMP <umlinst-jmp>` instruction may be used to transfer
control to the location associated with a label number.

+-------------------+------------------------------+
| Disassembly       | Usage                        |
+===================+==============================+
| .. code-block::   | .. code-block:: C++          |
|                   |                              |
|     label   label |     UML_LABEL(block, label); |
+-------------------+------------------------------+

Operands
^^^^^^^^

label (label number)
    The label number to associate with the current location.  A label
    number must not be used more than once within a generated code
    block.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-handle:

HANDLE
~~~~~~

Mark a location as an entry point of a subroutine.  Subroutines may be
called using the :ref:`CALLH <umlinst-callh>` and :ref:`EXH
<umlinst-exh>` instructions, and also by the `HASHJMP <umlinst-hashjmp>`
if no location is associated with the specified mode and emulated
program counter.

+--------------------+--------------------------------+
| Disassembly        | Usage                          |
+====================+================================+
| .. code-block::    | .. code-block:: C++            |
|                    |                                |
|     handle  handle |     UML_HANDLE(block, handle); |
+--------------------+--------------------------------+

Operands
^^^^^^^^

handle (code handle)
    The code handle to bind to the current location.  The handle must
    already be allocated, and must not have been bound since the last
    generated code reset (all handles are implicitly unbound when
    resetting the generated code cache).

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-hash:

HASH
~~~~

Associate a location with the specified mode and emulated program
counter values.  The :ref:`HASHJMP <umlinst-hashjmp>` instruction may be
used to transfer control to the location associated with a mode and
emulated program counter value.

This is usually used to mark the location of the generated code for an
emulated instruction or sequence of instructions.

+---------------------+------------------------------+
| Disassembly         | Usage                        |
+=====================+==============================+
| .. code-block::     | .. code-block:: C++          |
|                     |                              |
|     hash    mode,pc |   UML_HASH(block, mode, pc); |
+---------------------+------------------------------+

Operands
^^^^^^^^

mode (32-bit – immediate, map variable)
    The mode to associate with the current location in the generated
    code.  Must be greater than or equal to zero and less than the
    number of modes specified when creating the recompiler context.
pc (32-bit – immediate, map variable)
    The emulated program counter value to associate with the current
    location in the generated code.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-jmp:

JMP
~~~

Jump to the location associated with a label number within the current
block.

+------------------------+-----------------------------------+
| Disassembly            | Usage                             |
+========================+===================================+
| .. code-block::        | .. code-block:: C++               |
|                        |                                   |
|     jmp     label      |     UML_JMP(block, label);        |
|     jmp     label,cond |     UML_JMPc(block, cond, label); |
+------------------------+-----------------------------------+

Operands
^^^^^^^^

label (label number)
    The label number associated with the location to jump to in the
    current generated code block.  The label number must be associated
    with a location in the generated code block before the block is
    finalised.
cond (condition)
    If supplied, a condition that must be met to jump to the specified
    label.  If the condition is not met, execution will continue with
    the following instruction.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-callh:

CALLH
~~~~~

Call the subroutine beginning at the specified code handle.

+-------------------------+--------------------------------------+
| Disassembly             | Usage                                |
+=========================+======================================+
| .. code-block::         | .. code-block:: C++                  |
|                         |                                      |
|     callh   handle      |     UML_CALLH(block, handle);        |
|     callh   handle,cond |     UML_CALLHc(block, handle, cond); |
+-------------------------+--------------------------------------+

Operands
^^^^^^^^

handle (code handle)
    Handle located at the entry point of the subroutine to call.  The
    handle must already be allocated but does not need to be bound until
    the instruction is executed.  Calling a handle that was unbound at
    code generation time may produce less efficient code than calling a
    handle that was already bound.
cond (condition)
    If supplied, a condition that must be met for the subroutine to be
    called.  If the condition is not met, the subroutine will not be
    called.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-exh:

EXH
~~~

Set the ``EXP`` register and call the subroutine beginning at the
specified code handle.  The ``EXP`` register is a 32-bit special
function register that may be retrieved with the :ref:`GETEXP
<umlinst-getexp>` instruction.

+-----------------------------+-----------------------------------------+
| Disassembly                 | Usage                                   |
+=============================+=========================================+
| .. code-block::             | .. code-block:: C++                     |
|                             |                                         |
|     exh     handle,arg      |     UML_EXH(block, handle, arg);        |
|     exh     handle,arg,cond |     UML_EXHc(block, handle, arg, cond); |
+-----------------------------+-----------------------------------------+

Operands
^^^^^^^^

handle (code handle)
    Handle located at the entry point of the subroutine to call.  The
    handle must already be allocated but does not need to be bound until
    the instruction is executed.  Calling a handle that was unbound at
    code generation time may produce less efficient code than calling a
    handle that was already bound.
arg (32-bit – memory, integer register, immediate, map variable)
    Value to store in the ``EXP`` register.
cond (condition)
    If supplied, a condition that must be met for the subroutine to be
    called.  If the condition is not met, the subroutine will not be
    called and the ``EXP`` register will not be modified.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Immediate values for the ``arg`` operand are truncated to 32 bits.

.. _umlinst-ret:

RET
~~~

Return from a subroutine, transferring control to the instruction
following the :ref:`CALLH <umlinst-callh>` or :ref:`EXH <umlinst-exh>`
instruction used to call the subroutine.  This instruction must only be
used within generated code subroutines.  The :ref:`EXIT <umlinst-exit>`
instruction must be used to exit from the generated code.

+------------------+----------------------------+
| Disassembly      | Usage                      |
+==================+============================+
| .. code-block::  | .. code-block:: C++        |
|                  |                            |
|     ret          |     UML_RET(block);        |
|     ret     cond |     UML_RETc(block, cond); |
+------------------+----------------------------+

Operands
^^^^^^^^

cond (condition)
    If supplied, a condition that must be met to return from the
    subroutine.  If the condition is not met, execution will continue
    with the following instruction.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-hashjmp:

HASHJMP
~~~~~~~

Unwind all nested generated code subroutine frames and transfer control
to the location associated with the specified mode and emulated program
counter values.  If no location is associated with the specified mode
and program counter values, call the subroutine beginning at the
specified code handle.  Note that all nested generated code subroutine
frames are unwound in either case.

This is usually used to jump to the generated code corresponding to the
emulated code at a particular address when it is not known to be in the
current generated code block or when the mode changes.

+----------------------------+-----------------------------------------+
| Disassembly                | Usage                                   |
+============================+=========================================+
| .. code-block::            | .. code-block:: C++                     |
|                            |                                         |
|     hashjmp mode,pc,handle |   UML_HASHJMP(block, mode, pc, handle); |
+----------------------------+-----------------------------------------+

Operands
^^^^^^^^

mode (32-bit – memory, integer register, immediate, map variable)
    The mode associated with the location in the generated code to
    transfer control to.  Must be greater than or equal to zero and less
    than the number of modes specified when creating the recompiler
    context.
pc (32-bit – memory, integer register, immediate, map variable)
    The emulated program counter value associated with the location in
    the generated code to transfer control to.
handle (code handle)
    Handle located at the entry point of the subroutine to call if no
    location in the generated code is associated with the specified mode
    and emulated program counter values.  The handle must already be
    allocated but does not need to be bound until the instruction is
    executed.  Calling a handle that was unbound at code generation time
    may produce less efficient code than calling a handle that was
    already bound.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-exit:

EXIT
~~~~

Exit from the generated code, returning control to the caller.  May be
used from within any level of nested subroutine calls in the generated
code.

+-----------------------+----------------------------------+
| Disassembly           | Usage                            |
+=======================+==================================+
| .. code-block::       | .. code-block:: C++              |
|                       |                                  |
|     exit    arg,      |     UML_EXIT(block, arg);        |
|     exit    arg,,cond |     UML_EXITc(block, arg, cond); |
+-----------------------+----------------------------------+

Operands
^^^^^^^^

arg (32-bit – memory, integer register, immediate, map variable)
    The value to return to the caller.
cond (condition)
    If supplied, a condition that must be met to exit from the generated
    code.  If the condition is not met, execution will continue with the
    following instruction.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Immediate values for the ``arg`` operand are truncated to 32 bits.

.. _umlinst-callc:

CALLC
~~~~~

Call a C function with the signature ``void (*)(void *)``.

+---------------------------+-----------------------------------------+
| Disassembly               | Usage                                   |
+===========================+=========================================+
| .. code-block::           | .. code-block:: C++                     |
|                           |                                         |
|     callc   func,arg      |     UML_CALLC(block, func, arg);        |
|     callc   func,arg,cond |     UML_CALLCc(block, func, arg, cond); |
+---------------------------+-----------------------------------------+

Operands
^^^^^^^^

func (C function)
    Function pointer to the function to call.
arg (memory)
    Argument to pass to the function.
cond (condition)
    If supplied, a condition that must be met for the function to be
    called.  If the condition is not met, the function will not be
    called.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

.. _umlinst-debug:

DEBUG
~~~~~

Call the debugger instruction hook function if appropriate.

If the debugger is active, this should be executed before each emulated
instruction.  Any emulated CPU state kept in UML registers should be
flushed to memory before executing this instruction and reloaded
afterwards to ensure the debugger can display and modify values
correctly.

+-----------------+---------------------------+
| Disassembly     | Usage                     |
+=================+===========================+
| .. code-block:: | .. code-block:: C++       |
|                 |                           |
|     debug   pc  |     UML_DEBUG(block, pc); |
+-----------------+---------------------------+

Operands
^^^^^^^^

pc (32-bit – memory, integer register, immediate, map variable)
    The emulated program counter value to supply to the debugger
    instruction hook function.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Immediate values for the ``pc`` operand are truncated to 32 bits.

.. _umlinst-break:

BREAK
~~~~~

Break into the host debugger if attached.  Has no effect or crashes if
no host debugger is attached depending on the host system and
configuration.  This is intended as a developer aid and should not be
left in final code.

+-----------------+-----------------------+
| Disassembly     | Usage                 |
+=================+=======================+
| .. code-block:: | .. code-block:: C++   |
|                 |                       |
|     break       |     UML_BREAK(block); |
+-----------------+-----------------------+

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.


.. _umlinst-datamove:

Data movement
-------------

.. _umlinst-mov:

MOV
~~~

Copy an integer value.

+--------------------------+---------------------------------------+
| Disassembly              | Usage                                 |
+==========================+=======================================+
| .. code-block::          | .. code-block:: C++                   |
|                          |                                       |
|     mov     dst,src      |     UML_MOV(block, dst, src);         |
|     mov     dst,src,cond |     UML_MOVc(block, cond, dst, src);  |
|     dmov    dst,src      |     UML_DMOV(block, dst, src);        |
|     dmov    dst,src,cond |     UML_DMOVc(block, cond, dst, src); |
+--------------------------+---------------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the value will be copied to.
src (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The source value to copy.
cond (condition)
    If supplied, a condition that must be met to copy the value.  If the
    condition is not met, the instruction will have no effect.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Immediate values for the ``src`` operand are truncated to the
  instruction size.
* Converted to :ref:`NOP <umlinst-nop>` if the ``src`` and ``dst``
  operands refer to the same memory location or register and the
  instruction size is no larger than the destination size.

.. _umlinst-fmov:

FMOV
~~~~

Copy a floating point value.  The binary value will be preserved even if
it is not a valid representation of a floating point number.

+--------------------------+----------------------------------------+
| Disassembly              | Usage                                  |
+==========================+========================================+
| .. code-block::          | .. code-block:: C++                    |
|                          |                                        |
|     fsmov   dst,src      |     UML_FSMOV(block, dst, src);        |
|     fsmov   dst,src,cond |     UML_FSMOVc(block, cond, dst, src); |
|     fdmov   dst,src      |     UML_FDMOV(block, dst, src);        |
|     fdmov   dst,src,cond |     UML_FDMOVc(block, cond, dst, src); |
+--------------------------+----------------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, floating point register)
    The destination where the value will be copied to.
src (32-bit or 64-bit – memory, floating point register)
    The source value to copy.
cond (condition)
    If supplied, a condition that must be met to copy the value.  If the
    condition is not met, the instruction will have no effect.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Converted to :ref:`NOP <umlinst-nop>` if the ``src`` and ``dst``
  operands refer to the same memory location or register.

.. _umlinst-fcopyi:

FCOPYI
~~~~~~

Reinterpret an integer value as a floating point value.  The binary
value will be preserved even if it is not a valid representation of a
floating point number.

+---------------------+-----------------------------------+
| Disassembly         | Usage                             |
+=====================+===================================+
| .. code-block::     | .. code-block:: C++               |
|                     |                                   |
|     fscopyi dst,src |     UML_FSCOPYI(block, dst, src); |
|     fdcopyi dst,src |     UML_FDCOPYI(block, dst, src); |
+---------------------+-----------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, floating point register)
    The destination where the value will be copied to.
src (32-bit or 64-bit – memory, integer register)
    The source value to copy.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-icopyf:

ICOPYF
~~~~~~

Reinterpret a floating point value as an integer value.  The binary
value will be preserved even if it is not a valid representation of a
floating point number.

+---------------------+-----------------------------------+
| Disassembly         | Usage                             |
+=====================+===================================+
| .. code-block::     | .. code-block:: C++               |
|                     |                                   |
|     icopyfs dst,src |     UML_ICOPYFS(block, dst, src); |
|     icopyfd dst,src |     UML_ICOPYFD(block, dst, src); |
+---------------------+-----------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the value will be copied to.
src (32-bit or 64-bit – memory, floating point register)
    The source value to copy.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-load:

LOAD
~~~~

Load an unsigned integer value from a memory location with variable
displacement.  The value is zero-extended to the size of the
destination.  Host system rules for integer alignment must be followed.

+---------------------------------------+------------------------------------------------------+
| Disassembly                           | Usage                                                |
+=======================================+======================================================+
| .. code-block::                       | .. code-block:: C++                                  |
|                                       |                                                      |
|     load    dst,base,index,size_scale |     UML_LOAD(block, dst, base, index, size, scale);  |
|     dload   dst,base,index,size_scale |     UML_DLOAD(block, dst, base, index, size, scale); |
+---------------------------------------+------------------------------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the value read from memory will be stored.
base (memory)
    The base address of the area of memory to read from.
index (32-bit – memory, integer register, immediate, map variable)
    The displacement value added to the base address to calculate the
    address to read from.  This value may be scaled by a factor of 1, 2,
    4 or 8 depending on the ``scale`` operand.  Note that this is always
    a 32-bit operand interpreted as a signed integer, irrespective of
    the instruction size.
size (access size)
    The size of the value to read.  Must be ``SIZE_BYTE`` (8-bit),
    ``SIZE_WORD`` (16-bit), ``SIZE_DWORD`` (32-bit) or ``SIZE_QWORD``
    (64-bit).  Note that this operand controls the size of the value
    read from memory while the instruction size sets the size of the
    ``dst`` operand.
scale (index scale)
    The scale factor to apply to the ``index`` operand.  Must be
    ``SCALE_x1``, ``SCALE_x2``, ``SCALE_x4`` or ``SCALE_x8`` to multiply
    by 1, 2, 4 or 8, respectively (shift left by 0, 1, 2 or 3 bits).

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-loads:

LOADS
~~~~~

Load a signed integer value from a memory location with variable
displacement.  The value is sign-extended to the size of the
destination.  Host system rules for integer alignment must be followed.

+---------------------------------------+-------------------------------------------------------+
| Disassembly                           | Usage                                                 |
+=======================================+=======================================================+
| .. code-block::                       | .. code-block:: C++                                   |
|                                       |                                                       |
|     loads   dst,base,index,size_scale |     UML_LOADS(block, dst, base, index, size, scale);  |
|     dloads  dst,base,index,size_scale |     UML_DLOADS(block, dst, base, index, size, scale); |
+---------------------------------------+-------------------------------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the value read from memory will be stored.
base (memory)
    The base address of the area of memory to read from.
index (32-bit – memory, integer register, immediate, map variable)
    The displacement value added to the base address to calculate the
    address to read from.  This value may be scaled by a factor of 1, 2,
    4 or 8 depending on the ``scale`` operand.  Note that this is always
    a 32-bit operand interpreted as a signed integer, irrespective of
    the instruction size.
size (access size)
    The size of the value to read.  Must be ``SIZE_BYTE`` (8-bit),
    ``SIZE_WORD`` (16-bit), ``SIZE_DWORD`` (32-bit) or ``SIZE_QWORD``
    (64-bit).  Note that this operand controls the size of the value
    read from memory while the instruction size sets the size of the
    ``dst`` operand.
scale (index scale)
    The scale factor to apply to the ``index`` operand.  Must be
    ``SCALE_x1``, ``SCALE_x2``, ``SCALE_x4`` or ``SCALE_x8`` to multiply
    by 1, 2, 4 or 8, respectively (shift left by 0, 1, 2 or 3 bits).

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-store:

STORE
~~~~~

Store an integer value to a location in memory with variable
displacement.  Host system rules for integer alignment must be followed.

+---------------------------------------+-------------------------------------------------------+
| Disassembly                           | Usage                                                 |
+=======================================+=======================================================+
| .. code-block::                       | .. code-block:: C++                                   |
|                                       |                                                       |
|     store   base,index,src,size_scale |     UML_STORE(block, base, index, src, size, scale);  |
|     dstore  base,index,src,size_scale |     UML_DSTORE(block, base, index, src, size, scale); |
+---------------------------------------+-------------------------------------------------------+

Operands
^^^^^^^^

base (memory)
    The base address of the area of memory to write to.
index (32-bit – memory, integer register, immediate, map variable)
    The displacement value added to the base address to calculate the
    address to write to.  This value may be scaled by a factor of 1, 2,
    4 or 8 depending on the ``scale`` operand.  Note that this is always
    a 32-bit operand interpreted as a signed integer, irrespective of
    the instruction size.
src (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The value to write to memory.
size (access size)
    The size of the value to write.  Must be ``SIZE_BYTE`` (8-bit),
    ``SIZE_WORD`` (16-bit), ``SIZE_DWORD`` (32-bit) or ``SIZE_QWORD``
    (64-bit).  Note that this operand controls the size of the value
    written to memory while the instruction size sets the size of the
    ``src`` operand.
scale (index scale)
    The scale factor to apply to the ``index`` operand.  Must be
    ``SCALE_x1``, ``SCALE_x2``, ``SCALE_x4`` or ``SCALE_x8`` to multiply
    by 1, 2, 4 or 8, respectively (shift left by 0, 1, 2 or 3 bits).

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-fload:

FLOAD
~~~~~

Load a floating point value from a memory location with variable
displacement.  The binary value will be preserved even if it is not a
valid representation of a floating point number.  Host system rules for
memory access alignment must be followed.

+----------------------------+------------------------------------------+
| Disassembly                | Usage                                    |
+============================+==========================================+
| .. code-block::            | .. code-block:: C++                      |
|                            |                                          |
|     fsload  dst,base,index |     UML_FSLOAD(block, dst, base, index); |
|     fdload  dst,base,index |     UML_FDLOAD(block, dst, base, index); |
+----------------------------+------------------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, floating point register)
    The destination where the value read from memory will be stored.
base (memory)
    The base address of the area of memory to read from.
index (32-bit – memory, integer register, immediate, map variable)
    The displacement value added to the base address to calculate the
    address to read from.  This value will be scaled by the instruction
    size (multiplied by 4 or 8).  Note that this is always a 32-bit
    operand interpreted as a signed integer, irrespective of the
    instruction size.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-fstore:

FSTORE
~~~~~~

Store a floating point value to a memory location with variable
displacement.  The binary value will be preserved even if it is not a
valid representation of a floating point number.  Host system rules for
memory access alignment must be followed.

+----------------------------+-------------------------------------------+
| Disassembly                | Usage                                     |
+============================+===========================================+
| .. code-block::            | .. code-block:: C++                       |
|                            |                                           |
|     fsstore base,index,src |     UML_FSSTORE(block, base, index, src); |
|     fdstore base,index,src |     UML_FDSTORE(block, base, index, src); |
+----------------------------+-------------------------------------------+

Operands
^^^^^^^^

base (memory)
    The base address of the area of memory to write to.
index (32-bit – memory, integer register, immediate, map variable)
    The displacement value added to the base address to calculate the
    address to write to.  This value will be scaled by the instruction
    size (multiplied by 4 or 8).  Note that this is always a 32-bit
    operand interpreted as a signed integer, irrespective of the
    instruction size.
src (32-bit or 64-bit – memory, floating point register)
    The value to write to memory.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-getexp:

GETEXP
~~~~~~

Copy the value of the ``EXP`` register.  The ``EXP`` register can be set
using the :ref:`EXH <umlinst-exh>` instruction.

+-----------------+-----------------------------+
| Disassembly     | Usage                       |
+=================+=============================+
| .. code-block:: | .. code-block:: C++         |
|                 |                             |
|     getexp  dst |     UML_GETEXP(block, dst); |
+-----------------+-----------------------------+

Operands
^^^^^^^^

dst (32-bit – memory, integer register)
    The destination to copy the value of the ``EXP`` register to.  Note
    that the ``EXP`` register can only hold a 32-bit value.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-mapvar:

MAPVAR
~~~~~~

Set the value of a map variable starting at the current location in the
current generated code block.

+--------------------------+---------------------------------------+
| Disassembly              | Usage                                 |
+==========================+=======================================+
| .. code-block::          | .. code-block:: C++                   |
|                          |                                       |
|     mapvar  mapvar,value |     UML_MAPVAR(block, mapvar, value); |
+--------------------------+---------------------------------------+

Operands
^^^^^^^^

mapvar (map variable)
    The map variable to set the value of.
value (32-bit – immediate, map variable)
    The value to set the map variable to.  Note that map variables can
    only hold 32-bit values.

Flags
^^^^^

carry (C)
    Unchanged.
overflow (V)
    Unchanged.
zero (Z)
    Unchanged.
sign (S)
    Unchanged.
unordered (U)
    Unchanged.

.. _umlinst-recover:

RECOVER
~~~~~~~

Retrieve the value of a map variable at the location of the call
instruction in the outermost generated code frame.  This instruction
should only be used from within a generated code subroutine.  Results
are undefined if this instruction is executed from outside any
generated code subroutines.

+------------------------+--------------------------------------+
| Disassembly            | Usage                                |
+========================+======================================+
| .. code-block::        | .. code-block:: C++                  |
|                        |                                      |
|     recover dst,mapvar |     UML_RECOVER(block, dst, mapvar); |
+------------------------+--------------------------------------+

Operands
^^^^^^^^

dst (32-bit – memory, integer register)
    The destination to copy the value of the map variable to.  Note that
    map variables can only hold 32-bit values.
mapvar (map variable)
    The map variable to retrieve the value of from the outermost
    generated code frame.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.


.. _umlinst-memaccess:

Emulated memory access
----------------------

.. _umlinst-read:

READ
~~~~

Read from an emulated address space.  The access mask is implied to have
all bits set.

+---------------------------------+-----------------------------------------------+
| Disassembly                     | Usage                                         |
+=================================+===============================================+
| .. code-block::                 | .. code-block:: C++                           |
|                                 |                                               |
|     read    dst,addr,space_size |     UML_READ(block, dst, addr, size, space);  |
|     dread   dst,addr,space_size |     UML_DREAD(block, dst, addr, size, space); |
+---------------------------------+-----------------------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the value read from the emulated address space
    will be stored.
addr (32-bit – memory, integer register, immediate, map variable)
    The address to read from in the emulated address space.  Note that
    this is always a 32-bit operand, irrespective of the instruction
    size.
size (access size)
    The size of the emulated memory access.  Must be ``SIZE_BYTE``
    (8-bit), ``SIZE_WORD`` (16-bit), ``SIZE_DWORD`` (32-bit) or
    ``SIZE_QWORD`` (64-bit).  Note that this operand controls the size
    of the emulated memory access while the instruction size sets the
    size of the ``dst`` operand.
space (address space number)
    An integer identifying the address space to read from.  May be
    ``SPACE_PROGRAM``, ``SPACE_DATA``, ``SPACE_IO`` or ``SPACE_OPCODES``
    for one of the common CPU address spaces, or a non-negative integer
    cast to ``memory_space``.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Immediate values for the ``addr`` operand are truncated to 32 bits.

.. _umlinst-readm:

READM
~~~~~

Read from an emulated address space with access mask specified.

+--------------------------------------+------------------------------------------------------+
| Disassembly                          | Usage                                                |
+======================================+======================================================+
| .. code-block::                      | .. code-block:: C++                                  |
|                                      |                                                      |
|     readm   dst,addr,mask,space_size |     UML_READM(block, dst, addr, mask, size, space);  |
|     dreadm  dst,addr,mask,space_size |     UML_DREADM(block, dst, addr, mask, size, space); |
+--------------------------------------+------------------------------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the value read from the emulated address space
    will be stored.
addr (32-bit – memory, integer register, immediate, map variable)
    The address to read from in the emulated address space.  Note that
    this is always a 32-bit operand, irrespective of the instruction
    size.
mask (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The access mask for the emulated memory access.
size (access size)
    The size of the emulated memory access.  Must be ``SIZE_BYTE``
    (8-bit), ``SIZE_WORD`` (16-bit), ``SIZE_DWORD`` (32-bit) or
    ``SIZE_QWORD`` (64-bit).  Note that this operand controls the size
    of the emulated memory access while the instruction size sets the
    size of the ``dst`` and ``mask`` operands.
space (address space number)
    An integer identifying the address space to read from.  May be
    ``SPACE_PROGRAM``, ``SPACE_DATA``, ``SPACE_IO`` or ``SPACE_OPCODES``
    for one of the common CPU address spaces, or a non-negative integer
    cast to ``memory_space``.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Immediate values for the ``addr`` operand are truncated to 32 bits.
* Immediate values for the ``mask`` operand are truncated to the access
  size.
* Converted to :ref:`READ <umlinst-read>` if the ``mask`` operand is an
  immediate value with all bits set.

.. _umlinst-write:

WRITE
~~~~~

Write to an emulated address space.  The access mask is implied to have
all bits set.

+---------------------------------+------------------------------------------------+
| Disassembly                     | Usage                                          |
+=================================+================================================+
| .. code-block::                 | .. code-block:: C++                            |
|                                 |                                                |
|     write   addr,src,space_size |     UML_WRITE(block, addr, src, size, space);  |
|     dwrite  addr,src,space_size |     UML_DWRITE(block, addr, src, size, space); |
+---------------------------------+------------------------------------------------+

Operands
^^^^^^^^

addr (32-bit – memory, integer register, immediate, map variable)
    The address to write to in the emulated address space.  Note that
    this is always a 32-bit operand, irrespective of the instruction
    size.
src (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The value to write to the emulated address space.
size (access size)
    The size of the emulated memory access.  Must be ``SIZE_BYTE``
    (8-bit), ``SIZE_WORD`` (16-bit), ``SIZE_DWORD`` (32-bit) or
    ``SIZE_QWORD`` (64-bit).  Note that this operand controls the size
    of the emulated memory access while the instruction size sets the
    size of the ``src`` operand.
space (address space number)
    An integer identifying the address space to read from.  May be
    ``SPACE_PROGRAM``, ``SPACE_DATA``, ``SPACE_IO`` or ``SPACE_OPCODES``
    for one of the common CPU address spaces, or a non-negative integer
    cast to ``memory_space``.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Immediate values for the ``addr`` operand are truncated to 32 bits.
* Immediate values for the ``src`` operand are truncated to the access
  size.

.. _umlinst-writem:

WRITEM
~~~~~~

Write to an emulated address space with access mask specified.

+--------------------------------------+-------------------------------------------------------+
| Disassembly                          | Usage                                                 |
+======================================+=======================================================+
| .. code-block::                      | .. code-block:: C++                                   |
|                                      |                                                       |
|     writem  addr,src,mask,space_size |     UML_WRITEM(block, addr, src, mask, size, space);  |
|     dwritem addr,src,mask,space_size |     UML_DWRITEM(block, addr, src, mask, size, space); |
+--------------------------------------+-------------------------------------------------------+

Operands
^^^^^^^^

addr (32-bit – memory, integer register, immediate, map variable)
    The address to write to in the emulated address space.  Note that
    this is always a 32-bit operand, irrespective of the instruction
    size.
src (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The value to write to the emulated address space.
mask (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The access mask for the emulated memory access.
size (access size)
    The size of the emulated memory access.  Must be ``SIZE_BYTE``
    (8-bit), ``SIZE_WORD`` (16-bit), ``SIZE_DWORD`` (32-bit) or
    ``SIZE_QWORD`` (64-bit).  Note that this operand controls the size
    of the emulated memory access while the instruction size sets the
    size of the ``src`` and ``mask`` operands.
space (address space number)
    An integer identifying the address space to read from.  May be
    ``SPACE_PROGRAM``, ``SPACE_DATA``, ``SPACE_IO`` or ``SPACE_OPCODES``
    for one of the common CPU address spaces, or a non-negative integer
    cast to ``memory_space``.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Immediate values for the ``addr`` operand are truncated to 32 bits.
* Immediate values for the ``src`` and ``mask`` operands are truncated
  to the access size.
* Converted to :ref:`WRITE <umlinst-read>` if the ``mask`` operand is an
  immediate value with all bits set.

.. _umlinst-fread:

FREAD
~~~~~

Read a floating point value from an emulated address space.  The binary
value will be preserved even if it is not a valid representation of a
floating point number.  The access mask is implied to have all bits set.

+---------------------------------+------------------------------------------+
| Disassembly                     | Usage                                    |
+=================================+==========================================+
| .. code-block::                 | .. code-block:: C++                      |
|                                 |                                          |
|     fsread  dst,addr,space_size |     UML_FSREAD(block, dst, addr, space); |
|     fdread  dst,addr,space_size |     UML_FDREAD(block, dst, addr, space); |
+---------------------------------+------------------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, floating point register)
    The destination where the value read from the emulated address space
    will be stored.
addr (32-bit – memory, integer register, immediate, map variable)
    The address to read from in the emulated address space.  Note that
    this is always a 32-bit operand, irrespective of the instruction
    size.
space (address space number)
    An integer identifying the address space to read from.  May be
    ``SPACE_PROGRAM``, ``SPACE_DATA``, ``SPACE_IO`` or ``SPACE_OPCODES``
    for one of the common CPU address spaces, or a non-negative integer
    cast to ``memory_space``.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Immediate values for the ``addr`` operand are truncated to 32 bits.

.. _umlinst-fwrite:

FWRITE
~~~~~~

Write a floating point value to an emulated address space.  The binary
value will be preserved even if it is not a valid representation of a
floating point number.  The access mask is implied to have all bits set.

+---------------------------------+-------------------------------------------+
| Disassembly                     | Usage                                     |
+=================================+===========================================+
| .. code-block::                 | .. code-block:: C++                       |
|                                 |                                           |
|     fswrite addr,src,space_size |     UML_FSWRITE(block, addr, src, space); |
|     fdwrite addr,src,space_size |     UML_FDWRITE(block, addr, src, space); |
+---------------------------------+-------------------------------------------+

Operands
^^^^^^^^

addr (32-bit – memory, integer register, immediate, map variable)
    The address to write to in the emulated address space.  Note that
    this is always a 32-bit operand, irrespective of the instruction
    size.
src (32-bit or 64-bit – memory, floating point register)
    The value to write to the emulated address space.
    will be stored.
space (address space number)
    An integer identifying the address space to read from.  May be
    ``SPACE_PROGRAM``, ``SPACE_DATA``, ``SPACE_IO`` or ``SPACE_OPCODES``
    for one of the common CPU address spaces, or a non-negative integer
    cast to ``memory_space``.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Immediate values for the ``addr`` operand are truncated to 32 bits.


.. _umlinst-intarith:

Integer arithmetic and logic
----------------------------

.. _umlinst-add:

ADD
~~~

Add two integers.

+---------------------------+---------------------------------------+
| Disassembly               | Usage                                 |
+===========================+=======================================+
| .. code-block::           | .. code-block:: C++                   |
|                           |                                       |
|     add     dst,src1,src2 |     UML_ADD(block, dst, src1, src2);  |
|     dadd    dst,src1,src2 |     UML_DADD(block, dst, src1, src2); |
+---------------------------+---------------------------------------+

Calculates ``dst = src1 + src2``.

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the sum will be stored.
src1 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The first addend.
src2 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The second addend.

Flags
^^^^^

carry (C)
    Set in the case of arithmetic carry out of the most significant bit, or
    cleared otherwise (unsigned overflow).
overflow (V)
    Set in the case of signed two’s complement overflow, or cleared otherwise.
zero (Z)
    Set if the result is zero, or cleared otherwise.
sign (S)
    Set to the value of the most significant bit of the result (set if the
    result is a negative signed integer, or cleared otherwise).
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Converted to :ref:`MOV <umlinst-mov>`, :ref:`AND <umlinst-and>` or :ref:`OR
  <umlinst-or>` if the ``src1`` and ``src2`` operands are both immediate values
  and the carry and overflow flags are not required.
* Converted to :ref:`MOV <umlinst-mov>` or :ref:`AND <umlinst-and>` if the
  ``src1`` operand or ``src2`` operand is the immediate value zero and the carry
  and overflow flags are not required.
* Immediate values for the ``src1`` and ``src2`` operands are truncated to the
  instruction size.
* If the ``src2`` and ``dst`` operands refer to the same register or memory
  location, the ``src1`` and ``src2`` operands are exchanged.
* If the ``src1`` operand is an immediate value and the ``src2`` operand is not
  an immediate value, the ``src1`` and ``src2`` operands are exchanged.

.. _umlinst-addc:

ADDC
~~~~

Add two integers and the carry flag.

+---------------------------+----------------------------------------+
| Disassembly               | Usage                                  |
+===========================+========================================+
| .. code-block::           | .. code-block:: C++                    |
|                           |                                        |
|     addc    dst,src1,src2 |     UML_ADDC(block, dst, src1, src2);  |
|     daddc   dst,src1,src2 |     UML_DADDC(block, dst, src1, src2); |
+---------------------------+----------------------------------------+

Calculates ``dst = src1 + src2 + C``.

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the sum will be stored.
src1 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The first addend.
src2 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The second addend.

Flags
^^^^^

carry (C)
    Set in the case of arithmetic carry out of the most significant bit, or
    cleared otherwise (unsigned overflow).
overflow (V)
    Set in the case of signed two’s complement overflow, or cleared otherwise.
zero (Z)
    Set if the result is zero, or cleared otherwise.
sign (S)
    Set to the value of the most significant bit of the result (set if the
    result is a negative signed integer, or cleared otherwise).
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Immediate values for the ``src1`` and ``src2`` operands are truncated to the
  instruction size.
* If the ``src2`` and ``dst`` operands refer to the same register or memory
  location, the ``src1`` and ``src2`` operands are exchanged.
* If the ``src1`` operand is an immediate value and the ``src2`` operand is not
  an immediate value, the ``src1`` and ``src2`` operands are exchanged.

.. _umlinst-sub:

SUB
~~~

Subtract an integer from another integer.

+---------------------------+---------------------------------------+
| Disassembly               | Usage                                 |
+===========================+=======================================+
| .. code-block::           | .. code-block:: C++                   |
|                           |                                       |
|     sub     dst,src1,src2 |     UML_SUB(block, dst, src1, src2);  |
|     dsub    dst,src1,src2 |     UML_DSUB(block, dst, src1, src2); |
+---------------------------+---------------------------------------+

Calculates ``dst = src1 - src2``.

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the difference will be stored.
src1 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The minuend (the value to subtract from).
src2 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The subtrahend (the value to subtract).

Flags
^^^^^

carry (C)
    Set if the subtrahend is a larger unsigned value than the minuend, or
    cleared otherwise (unsigned overflow, or arithmetic borrow).
overflow (V)
    Set in the case of signed two’s complement overflow, or cleared otherwise.
zero (Z)
    Set if the result is zero, or cleared otherwise (set if the minuend and
    subtrahend are equal, or cleared otherwise).
sign (S)
    Set to the value of the most significant bit of the result (set if the
    result is a negative signed integer, or cleared otherwise).
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Immediate values for the ``src1`` and ``src2`` operands are truncated to the
  instruction size.
* Converted to :ref:`MOV <umlinst-mov>`, :ref:`AND <umlinst-and>` or :ref:`OR
  <umlinst-or>` if the ``src1`` and ``src2`` operands are both immediate values
  and the carry and overflow flags are not required.
* Converted to :ref:`MOV <umlinst-mov>` or :ref:`AND <umlinst-and>` if the
  ``src2`` operand is the immediate value zero and the carry and overflow flags
  are not required.

.. _umlinst-subb:

SUBB
~~~~

Subtract an integer and the carry flag from another integer.

+---------------------------+----------------------------------------+
| Disassembly               | Usage                                  |
+===========================+========================================+
| .. code-block::           | .. code-block:: C++                    |
|                           |                                        |
|     subb    dst,src1,src2 |     UML_SUBB(block, dst, src1, src2);  |
|     dsubb   dst,src1,src2 |     UML_DSUBB(block, dst, src1, src2); |
+---------------------------+----------------------------------------+

Calculates ``dst = src1 - src2 - C``.

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the difference will be stored.
src1 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The minuend (the value to subtract from).
src2 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The subtrahend (the value to subtract).

Flags
^^^^^

carry (C)
    Set if the subtrahend plus the carry flag is a larger unsigned value than
    the minuend, or cleared otherwise (unsigned overflow, or arithmetic borrow).
overflow (V)
    Set in the case of signed two’s complement overflow, or cleared otherwise.
zero (Z)
    Set if the result is zero, or cleared otherwise (set if the minuend is equal
    to the subtrahend plus the carry flag, or cleared otherwise).
sign (S)
    Set to the value of the most significant bit of the result (set if the
    result is a negative signed integer, or cleared otherwise).
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Immediate values for the ``src1`` and ``src2`` operands are truncated to the
  instruction size.

.. _umlinst-cmp:

CMP
~~~

Compare two integers and set the flags as though they were subtracted.

+-----------------------+----------------------------------+
| Disassembly           | Usage                            |
+=======================+==================================+
| .. code-block::       | .. code-block:: C++              |
|                       |                                  |
|     cmp     src1,src2 |     UML_CMP(block, src1, src2);  |
|     dcmp    src1,src2 |     UML_DCMP(block, src1, src2); |
+-----------------------+----------------------------------+

Sets the flags based on calculating ``src1 - src2`` but discards the result of
the subtraction.

Operands
^^^^^^^^

src1 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The left-hand side value to compare, or the minuend (the value to subtract
    from).
src2 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The right-hand side value to compare, or the subtrahend (the value to
    subtract).

Flags
^^^^^

carry (C)
    Set if the unsigned value of the ``src1`` operand is smaller than the
    unsigned value of the ``src2`` operand, or cleared otherwise.
overflow (V)
    Set if subtracting the value of the ``src2`` operand from the value of the
    ``src1`` operand would result in two’s complement overflow, or cleared
    otherwise.
zero (Z)
    Set if the values of the ``src1`` and ``src2`` operands are equal, or
    cleared otherwise.
sign (S)
    Set to the value of the most significant bit of the result of subtracting
    the value of the ``src2`` operand from the value of the ``src1`` operand
    (set if the result would be a negative signed integer, or cleared
    otherwise).
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Converted to :ref:`NOP <umlinst-nop>` if no flags are required.
* Immediate values for the ``src1`` and ``src2`` operands are truncated to the
  instruction size.

.. _umlinst-and:

AND
~~~

Calculate the bitwise logical conjunction of two integers (result bits will be
set if the corresponding bits are set in both inputs).

+---------------------------+---------------------------------------+
| Disassembly               | Usage                                 |
+===========================+=======================================+
| .. code-block::           | .. code-block:: C++                   |
|                           |                                       |
|     and     dst,src1,src2 |     UML_AND(block, dst, src1, src2);  |
|     dand    dst,src1,src2 |     UML_DAND(block, dst, src1, src2); |
+---------------------------+---------------------------------------+

Calculates ``dst = src1 & src2``.

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the logical conjunction will be stored.
src1 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The first input.
src2 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The second input.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Set if the result is zero, or cleared otherwise.
sign (S)
    Set to the value of the most significant bit of the result (set if the
    result is a negative signed integer, or cleared otherwise).
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Converted to :ref:`MOV <umlinst-mov>` if the ``src1`` and ``src2`` operands
  refer to the same memory location or register, the ``src1`` and ``src2``
  operands are both immediate values or one of them is an immediate value with
  all bits set or no bits set and flags are not required.
* Converted to :ref:`OR <umlinst-or>` if the ``src1`` and ``src2`` operands are
  both immediate values with all bits set and flags are required.
* Converted to :ref:`TEST <umlinst-test>` if the instruction size is 64 bits or
  the ``dst`` operand refers to a memory location, one of the ``src1`` and
  ``src2`` operands refer to the same memory location or register as ``dst``,
  the other source operand refers to the same memory location or register or is
  an immediate value with all bits set, and flags are required.
* If the ``src1`` and ``src2`` operands are both immediate values, the
  conjunction is not zero and flags are required, ``src1`` is replaced with the
  conjunction and ``src2`` is set to an immediate value with all bits set.
* If the ``src1`` and ``src2`` operands are both immediate values and the
  conjunction is zero or either the ``src1`` or ``src2`` operand is the
  immediate value zero and flags are required, ``src1`` is set to refer to the
  same memory location or register as ``dst`` and ``src2`` is set to the
  immediate value zero.
* Immediate values for the ``src1`` and ``src2`` operands are truncated to the
  instruction size.
* If the ``src2`` and ``dst`` operands refer to the same register or memory
  location, the ``src1`` and ``src2`` operands are exchanged.
* If the ``src1`` operand is an immediate value and the ``src2`` operand is not
  an immediate value, the ``src1`` and ``src2`` operands are exchanged.

.. _umlinst-test:

TEST
~~~~

Set the flags based on the bitwise logical conjunction of two integers.

+-----------------------+-----------------------------------+
| Disassembly           | Usage                             |
+=======================+===================================+
| .. code-block::       | .. code-block:: C++               |
|                       |                                   |
|     test    src1,src2 |     UML_TEST(block, src1, src2);  |
|     dtest   src1,src2 |     UML_DTEST(block, src1, src2); |
+-----------------------+-----------------------------------+

Sets the flags based on calculating ``src1 & src2`` but discards the result of
the conjunction.

Operands
^^^^^^^^

src1 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The first input.
src2 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The second input.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Set if the result of the conjunction is zero, or cleared otherwise.
sign (S)
    Set if the most significant bit is set in both inputs, or cleared otherwise
    (set if the both inputs are negative signed integers, or cleared otherwise).
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Converted to :ref:`NOP <umlinst-nop>` if flags are not required.
* If the ``src1`` and ``src2`` operands are both immediate values and the
  bitwise logical conjunction is not zero, the ``src1`` operand is set to the
  conjunction and the ``src2`` operand is set to an immediate value with all
  bits set.
* If either of the ``src1`` and ``src2`` operands is the immediate value zero or
  the ``src1`` and ``src2`` operands are both immediate values and the bitwise
  logical conjunction is zero, the ``src1`` and ``src2`` operands are both set
  to the immediate value zero.
* If the ``src1`` and ``src2`` operands refer to the same memory location or
  register, the ``src2`` operand is set to an immediate value with all bits set.
* Immediate values for the ``src1`` and ``src2`` operands are truncated to the
  instruction size.
* If the ``src1`` operand is an immediate value and the ``src2`` operand is not
  an immediate value, the ``src1`` and ``src2`` operands are exchanged.

.. _umlinst-or:

OR
~~

Calculate the bitwise logical inclusive disjunction of two integers (result bits
will be set if the corresponding bits are set in either input).

+---------------------------+--------------------------------------+
| Disassembly               | Usage                                |
+===========================+======================================+
| .. code-block::           | .. code-block:: C++                  |
|                           |                                      |
|     or      dst,src1,src2 |     UML_OR(block, dst, src1, src2);  |
|     dor     dst,src1,src2 |     UML_DOR(block, dst, src1, src2); |
+---------------------------+--------------------------------------+

Calculates ``dst = src1 | src2``.

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the logical inclusive disjunction will be stored.
src1 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The first input.
src2 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The second input.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Set if the result is zero, or cleared otherwise.
sign (S)
    Set to the value of the most significant bit of the result (set if the
    result is a negative signed integer, or cleared otherwise).
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Converted to :ref:`MOV <umlinst-mov>` if the ``src1`` and ``src2`` operands
  are both immediate values or one of the ``src1`` or ``src2`` operands is an
  immediate value with all bits set and flags are not required.
* Converted to :ref:`AND <umlinst-and>` if the ``src1`` and ``src2`` operands
  are both immediate values and the inclusive disjunction does not have all
  bits set and flags are required.
* Converted to :ref:`MOV <umlinst-mov>`, :ref:`AND <umlinst-and>` or :ref:`TEST
  <umlinst-test>` if the ``src1`` and ``src2`` operands refer to the same memory
  location or register or if one of the ``src1`` and ``src2`` operands is the
  immediate value zero.
* If one of the ``src1`` and ``src2`` operands is an immediate value with all
  bits set or the ``src1`` and ``src2`` operands are both immediate values and
  the inclusive disjunction has all bits set and flags are required, ``src1`` is
  set to refer to the same memory location or register as ``dst`` and ``src2``
  is set to an immediate value with all bits set.
* Immediate values for the ``src1`` and ``src2`` operands are truncated to the
  instruction size.
* If the ``src2`` and ``dst`` operands refer to the same register or memory
  location, the ``src1`` and ``src2`` operands are exchanged.
* If the ``src1`` operand is an immediate value and the ``src2`` operand is not
  an immediate value, the ``src1`` and ``src2`` operands are exchanged.

.. _umlinst-xor:

XOR
~~~

Calculate the bitwise logical exclusive disjunction of two integers (result bits
will be set if the corresponding bit is set in one input and unset in the other
input).

+---------------------------+---------------------------------------+
| Disassembly               | Usage                                 |
+===========================+=======================================+
| .. code-block::           | .. code-block:: C++                   |
|                           |                                       |
|     xor     dst,src1,src2 |     UML_XOR(block, dst, src1, src2);  |
|     dxor    dst,src1,src2 |     UML_DXOR(block, dst, src1, src2); |
+---------------------------+---------------------------------------+

Calculates ``dst = src1 ^ src2``.

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the logical exclusive disjunction will be stored.
src1 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The first input.
src2 (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The second input.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Set if the result is zero, or cleared otherwise.
sign (S)
    Set to the value of the most significant bit of the result (set if the
    result is a negative signed integer, or cleared otherwise).
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Converted to :ref:`MOV <umlinst-mov>`, :ref:`AND <umlinst-and>`, :ref:`TEST
  <umlinst-test>` or :ref:`OR <umlinst-or>` if the ``src1`` and ``src2``
  operands are both immediate values, if one of the ``src1`` and ``src2``
  operands is the immediate value zero or if the ``src1`` and ``src2`` operands
  refer to the same memory location or register.

.. _umlinst-lzcnt:

LZCNT
~~~~~

Count the number of contiguous left-aligned zero bits in an integer (count
leading zeroes).

+---------------------+----------------------------------+
| Disassembly         | Usage                            |
+=====================+==================================+
| .. code-block::     | .. code-block:: C++              |
|                     |                                  |
|     lzcnt   dst,src |     UML_LZCNT(block, dst, src);  |
|     dlzcnt  dst,src |     UML_DLZCNT(block, dst, src); |
+---------------------+----------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the result will be stored
src (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The input value in which to count left-aligned zero bits.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Set if the result is zero, or cleared otherwise (set to the most significant
    bit of the input).
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Converted to :ref:`MOV <umlinst-mov>` or :ref:`AND <umlinst-and>` if the
  ``src`` operand is an immediate value.

.. _umlinst-tzcnt:

TZCNT
~~~~~

Count the number of contiguous right-aligned zero bits in an integer (count
trailing zeroes).

+---------------------+----------------------------------+
| Disassembly         | Usage                            |
+=====================+==================================+
| .. code-block::     | .. code-block:: C++              |
|                     |                                  |
|     tzcnt   dst,src |     UML_TZCNT(block, dst, src);  |
|     dtzcnt  dst,src |     UML_DTZCNT(block, dst, src); |
+---------------------+----------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, integer register)
    The destination where the result will be stored
src (32-bit or 64-bit – memory, integer register, immediate, map variable)
    The input value in which to count right-aligned zero bits.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Set if the result is zero, or cleared otherwise (set to the least
    significant bit of the input).
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

* Converted to :ref:`MOV <umlinst-mov>` or :ref:`AND <umlinst-and>` if the
  ``src`` operand is an immediate value.


.. _umlinst-fparith:

Floating point arithmetic
-------------------------

.. _umlinst-fadd:

FADD
~~~~

Add two floating point numbers.

+---------------------------+----------------------------------------+
| Disassembly               | Usage                                  |
+===========================+========================================+
| .. code-block::           | .. code-block:: C++                    |
|                           |                                        |
|     fsadd   dst,src1,src2 |     UML_FSADD(block, dst, src1, src2); |
|     fdadd   dst,src1,src2 |     UML_FDADD(block, dst, src1, src2); |
+---------------------------+----------------------------------------+

Calculates ``dst = src1 + src2``.

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, floating point register)
    The destination where the sum will be stored.
src1 (32-bit or 64-bit – memory, floating point register)
    The first addend.
src2 (32-bit or 64-bit – memory, floating point register)
    The second addend.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

No simplifications are applied to this instruction.

.. _umlinst-fsub:

FSUB
~~~~

Subtract a floating point number from another floating point number.

+---------------------------+----------------------------------------+
| Disassembly               | Usage                                  |
+===========================+========================================+
| .. code-block::           | .. code-block:: C++                    |
|                           |                                        |
|     fssub   dst,src1,src2 |     UML_FSSUB(block, dst, src1, src2); |
|     fdsub   dst,src1,src2 |     UML_FDSUB(block, dst, src1, src2); |
+---------------------------+----------------------------------------+

Calculates ``dst = src1 - src2``.

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, floating point register)
    The destination where the difference will be stored.
src1 (32-bit or 64-bit – memory, floating point register)
    The minuend (the value to subtract from).
src2 (32-bit or 64-bit – memory, floating point register)
    The subtrahend (the value to subtract).

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

No simplifications are applied to this instruction.

.. _umlinst-fcmp:

FCMP
~~~~

Compare two floating-point numbers and set the carry, zero and unordered flags.

+-----------------------+-----------------------------------+
| Disassembly           | Usage                             |
+=======================+===================================+
| .. code-block::       | .. code-block:: C++               |
|                       |                                   |
|     fscmp   src1,src2 |     UML_FSCMP(block, src1, src2); |
|     fdcmp   src1,src2 |     UML_FDCMP(block, src1, src2); |
+-----------------------+-----------------------------------+

Operands
^^^^^^^^

src1 (32-bit or 64-bit – memory, floating point register)
    The left-hand side value to compare.
src2 (32-bit or 64-bit – memory, floating point register)
    The right-hand side value to compare.

Flags
^^^^^

carry (C)
    Set if the value of ``src1`` is less than the value of ``src2``, or cleared
    otherwise.
overflow (V)
    Undefined.
zero (Z)
    Set if the values of ``src1`` and ``src2`` are equal, or cleared otherwise.
sign (S)
    Undefined.
unordered (U)
    Set if either ``src1`` or ``src2`` is not a number (NaN), or cleared
    otherwise.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

No simplifications are applied to this instruction.

.. _umlinst-fmul:

FMUL
~~~~

Multiply two floating point numbers.

+---------------------------+----------------------------------------+
| Disassembly               | Usage                                  |
+===========================+========================================+
| .. code-block::           | .. code-block:: C++                    |
|                           |                                        |
|     fsmul   dst,src1,src2 |     UML_FSMUL(block, dst, src1, src2); |
|     fdmul   dst,src1,src2 |     UML_FDMUL(block, dst, src1, src2); |
+---------------------------+----------------------------------------+

Calculates ``dst = src1 * src2``.

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, floating point register)
    The destination where the product will be stored.
src1 (32-bit or 64-bit – memory, floating point register)
    The multiplicand (the value to multiply).
src2 (32-bit or 64-bit – memory, floating point register)
    The multiplier (the value to multiply by).

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

No simplifications are applied to this instruction.

.. _umlinst-fdiv:

FDIV
~~~~

Divide a floating point number by another floating point number.

+---------------------------+----------------------------------------+
| Disassembly               | Usage                                  |
+===========================+========================================+
| .. code-block::           | .. code-block:: C++                    |
|                           |                                        |
|     fsdiv   dst,src1,src2 |     UML_FSDIV(block, dst, src1, src2); |
|     fddiv   dst,src1,src2 |     UML_FDDIV(block, dst, src1, src2); |
+---------------------------+----------------------------------------+

Calculates ``dst = src1 / src2``.

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, floating point register)
    The destination where the quotient will be stored.
src1 (32-bit or 64-bit – memory, floating point register)
    The dividend (the value to divide).
src2 (32-bit or 64-bit – memory, floating point register)
    The divisor (the value to divide by).

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

No simplifications are applied to this instruction.

.. _umlinst-fneg:

FNEG
~~~~

Negate a floating point number.

+---------------------+---------------------------------+
| Disassembly         | Usage                           |
+=====================+=================================+
| .. code-block::     | .. code-block:: C++             |
|                     |                                 |
|     fsneg   dst,src |     UML_FSNEG(block, dst, src); |
|     fdneg   dst,src |     UML_FDNEG(block, dst, src); |
+---------------------+---------------------------------+

Calculates ``dst = -src``.

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, floating point register)
    The destination where the result will be stored.
src (32-bit or 64-bit – memory, floating point register)
    The value to be negated.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

No simplifications are applied to this instruction.

.. _umlinst-fabs:

FABS
~~~~

Calculate the absolute value of a floating point number.

+---------------------+---------------------------------+
| Disassembly         | Usage                           |
+=====================+=================================+
| .. code-block::     | .. code-block:: C++             |
|                     |                                 |
|     fsabs   dst,src |     UML_FSABS(block, dst, src); |
|     fdabs   dst,src |     UML_FDABS(block, dst, src); |
+---------------------+---------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, floating point register)
    The destination where the result will be stored.
src (32-bit or 64-bit – memory, floating point register)
    The value to calculate the absolute value of.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

No simplifications are applied to this instruction.

.. _umlinst-fsqrt:

FSQRT
~~~~~

Calculate the square root of a floating point number.

+---------------------+----------------------------------+
| Disassembly         | Usage                            |
+=====================+==================================+
| .. code-block::     | .. code-block:: C++              |
|                     |                                  |
|     fssqrt  dst,src |     UML_FSSQRT(block, dst, src); |
|     fdsqrt  dst,src |     UML_FDSQRT(block, dst, src); |
+---------------------+----------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, floating point register)
    The destination where the square root will be stored.
src (32-bit or 64-bit – memory, floating point register)
    The value to calculate the square root of.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

No simplifications are applied to this instruction.

.. _umlinst-frecip:

FRECIP
~~~~~~

Calculate an approximate reciprocal value of a floating point number.  The
algorithm used, precision and nature of inaccuracies in the approximation are
undefined.

+---------------------+---------------------------------+
| Disassembly         | Usage                           |
+=====================+=================================+
| .. code-block::     | .. code-block:: C++             |
|                     |                                 |
|     fsabs   dst,src |     UML_FSABS(block, dst, src); |
|     fdabs   dst,src |     UML_FDABS(block, dst, src); |
+---------------------+---------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, floating point register)
    The destination where the result will be stored.
src (32-bit or 64-bit – memory, floating point register)
    The value to approximate the reciprocal of.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

No simplifications are applied to this instruction.

.. _umlinst-frsqrt:

FRSQRT
~~~~~~

Calculate an approximate reciprocal value of the square root of a floating point
number.  The algorithm used, precision and nature of inaccuracies in the
approximation are undefined.

+---------------------+-----------------------------------+
| Disassembly         | Usage                             |
+=====================+===================================+
| .. code-block::     | .. code-block:: C++               |
|                     |                                   |
|     fsrsqrt dst,src |     UML_FSRSQRT(block, dst, src); |
|     fdrsqrt dst,src |     UML_FDRSQRT(block, dst, src); |
+---------------------+-----------------------------------+

Operands
^^^^^^^^

dst (32-bit or 64-bit – memory, floating point register)
    The destination where the result will be stored.
src (32-bit or 64-bit – memory, floating point register)
    The value to approximate the reciprocal of the square root of.

Flags
^^^^^

carry (C)
    Undefined.
overflow (V)
    Undefined.
zero (Z)
    Undefined.
sign (S)
    Undefined.
unordered (U)
    Undefined.

Simplification rules
^^^^^^^^^^^^^^^^^^^^

No simplifications are applied to this instruction.
