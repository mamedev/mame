.. _umlarch:

UML Architecture
================

.. contents::
    :local:
    :depth: 2


.. _umlarch-intro:

Introduction
------------

MAME provides a framework to assist in developing recompilers for CPUs,
DSPs and other emulated devices.  A recompiler uses several components:

* The *front-end* analyses the emulated instruction stream and produces
  sequences of instruction descriptions.  Recompiler front-ends derive
  from the ``drc_frontend_base`` class template.
* The *recompiler* translates the instruction descriptions to
  intermediate UML instructions.  The recompiler derives from the
  ``cpu_device`` class for the common cases of emulating CPUs and DSPs.
* The *back-end* translates the UML instructions to a form suitable for
  execution, and provides the means to execute the result.  While the
  front-end and recompiler are specific to the emulated device, the
  back-end is independent of the emulated device but may be specific to
  the host system.


.. _umlarch-vm:

The UML virtual machine
-----------------------

UML instructions run on a register-based virtual machine.

The virtual machine has the following architectural registers:

Integer registers
    Ten general-purpose integer registers, named ``I0`` to ``I9``.  Each
    register holds a 64-bit binary value.  Values are interpreted as
    unsigned integers, or two’s complement signed integers.  When used
    as the destination for operations that produce 32-bit results, the
    most significant 32 bits of the register are cleared (set to zero).
Floating point registers
    Ten general-purpose floating point registers, named ``F0`` to
    ``F9``.  Each register may hold a 32-bit value or a 64-bit value,
    interpreted as IEEE 754 32-bit binary format (single precision) or
    IEEE 754 64-bit binary format (double precision), respectively.
    The result of using a register containing a 32-bit value as an input
    for an operation expecting a 64-bit value or using a register
    containing a 64-bit value as an input for an operation expecting a
    32-bit value is undefined.
Condition code register
    The condition code register consists of five flags: carry (C),
    overflow (V), zero (Z), sign (S) and unordered (U).
Exception register
    A special-function register, named ``EXP``, which holds a 32-bit
    binary value.  UML instructions are provided to set the value of
    this register when calling a subroutine (:ref:`EXH <umlinst-exh>`)
    and to retrieve its value (:ref:`GETEXP <umlinst-getexp>`).
Floating point mode register
    A special-function register that controls the rounding mode for
    floating point arithmetic.  Four IEEE 754 rounding modes are
    supported: to nearest (ties to even), toward zero, toward positive
    infinity and toward negative infinity.

The condition code flags have the following definitions:

Carry (C)
    Set to indicate carry out of the most significant bit of an integer
    arithmetic operation, the last bit shifted out for an integer shift
    or rotate operation, or a less than result for floating point
    comparison.  The value is inverted for integer subtraction,
    indicating arithmetic borrow.
Overflow (V)
    Set to indicate two’s complement signed overflow after an integer
    arithmetic operation.
Zero (Z)
    Set to indicate the result of an integer operation is zero, or an
    equal result for a floating point comparison.
Sign (S)
    Set to the value of the most significant bit of the result of
    integer arithmetic, logical and shift operations.  This indicates
    when a two’s complement signed value is negative.
Unordered (U)
    Set to indicate an unordered result for a floating point comparison
    (at least one of the inputs is not a number).

The condition codes are tested by conditional jump, subroutine
call/return, C function call, and integer/floating point copy
instructions (:ref:`JMP <umlinst-jmp>`, :ref:`CALLH <umlinst-callh>`,
:ref:`EXH <umlinst-exh>`, :ref:`RET <umlinst-ret>`,
:ref:`CALLC <umlinst-callc>`, :ref:`SET <umlinst-set>`,
:ref:`MOV <umlinst-mov>` and :ref:`FMOV <umlinst-fmov>`).
:ref:`Sixteen conditions <umlinst-conditions>` are supported.

A subroutine call stack is provided, with support for at least sixteen
levels of nested subroutine calls.

General-purpose floating point registers may be used to hold arbitrary
binary values.  Instructions that copy, load or store data to or from
floating point registers make no changes to the binary data.

Note that the general-purpose integer and floating point registers may
not all have the same performance characteristics.  When this is the
case, the lowest-numbered registers will have the highest performance.
Back-ends may provide some number of “fast” integer and floating point
registers, typically by assigning them directly to host CPU registers.
A recompiler may determine the number of “fast” registers available by
calling ``get_backend_info``.  If, for example, the back-end reports
four “fast” integer registers and six “fast” floating point registers,
the registers ``I0`` to ``I3`` and ``F0`` to ``F5`` are “fast”
registers, and all higher-numbered registers should be expected to have
poorer relative performance.


.. _umlarch-blocks:

Code blocks
-----------

Recompilers must generate code in *code blocks*.  There are two kinds of
code blocks: *invariant blocks* and *transient blocks*.  Invariant
blocks, created by calling ``begin_invariant_block``, persist for the
lifetime of the recompiler; transient blocks, created by calling
``begin_block``, persist until the next time the transient portion of
the recompiler cache is cleared (by calling ``reset``).  After
generating a transient block, it is not possible to generate additional
invariant blocks until the transient portion of the recompiler cache is
cleared.

Invariant blocks should be used for code that only depends on static
configuration and won’t need to change for the lifetime of the
recompiler (and its associated emulated device).  Transient blocks
should be used for translating emulated code.

Code blocks require explicit entry points to be defined using
:ref:`HANDLE <umlinst-label>` or :ref:`HASH <umlinst-hash>`
instructions, and execution must not be allowed to reach the end of a
code block (a :ref:`HASHJMP <umlinst-hashjmp>`, :ref:`RET <umlinst-ret>`
or :ref:`EXIT <umlinst-exit>` instruction will be used to transfer
control at the end of a block of translated code or subroutine).  See
the section on :ref:`flow control <umlarch-flow>` for more details.


.. _umlarch-flow:

UML flow control
----------------

UML instructions provide four kinds of flow control operations:

* Jumping to a label within the same generated code block.
* Calling and returning from subroutines.
* Jumping to the location associated with a mode and emulated program
  counter value.
* Exiting from the generated code and returning to the caller.

The initial entry point to the generated code must also be a subroutine
entry point.

Labels
~~~~~~

Labels are identified using 32-bit unsigned integer values.  Label
numbers must be unique within a generated code block.  Locations are
marked as potential targets for intra-block jumps using the
:ref:`LABEL <umlinst-label>` instruction.  The :ref:`JMP <umlinst-jmp>`
instruction is used to transfer control to the location associated with
a label within the same generated code block.  It is not possible to
jump to a location in a different code block in this way.

Subroutines
~~~~~~~~~~~

Recompilers refer to generated subroutine entry points using *code
handles*, represented by the ``uml::code_handle`` class and allocated
by calling ``handle_alloc``.  Code handles are allocated from the near
cache.  The recompiler must not free any code handles, they remain valid
for the lifetime of the recompiler.

A subroutine entry point is created and associated with a code handle
using the :ref:`HANDLE <umlinst-handle>` instruction.  Subroutine entry
points may be created in both invariant and transient blocks.  Handles
associated with function entry points in transient blocks are
automatically reset when the transient portion of the recompiler cache
is cleared.  Note that a code handle that is currently associated with a
subroutine entry point cannot be associated with a different subroutine
entry point.

Subroutines are called using the :ref:`CALLH <umlinst-callh>` and
:ref:`EXH <umlinst-exh>` instructions.  The
:ref:`HASHJMP <umlinst-hashjmp>` instruction will also call a subroutine
if no location is associated with the specified mode and emulated
program pointer value.  The :ref:`RET <umlinst-ret>` instruction returns
from a subroutine.

Subroutines may be called from within the same code block of from a
different code block.  A handle must be allocated before code to call it
is generated.  However, the handle does not need to be associated with a
subroutine entry point until the subroutine call is actually executed.
This allows for creation of mutually recursive subroutines.  Note that
it may be possible to generate more efficient subroutine call code if
the handle is associated with a subroutine entry point before the code
to call it is generated.

Exiting from generated code
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :ref:`EXIT <umlinst-exit>` exits from the generated code, returning
control to the caller.  It may be used from within any level of nested
subroutine calls in the generated code.  The call stack is automatically
unwound.
