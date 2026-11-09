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
    ``F9```.  Each register may hold a 32-bit value or a 64-bit value,
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
    this register when calling a subroutine and to retrieve its value.
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

A subroutine call stack is provided, with support for at least sixteen
nested subroutine calls.


.. _umlarch-flow:

UML flow control
----------------

UML instructions provide three kinds of flow control operations:

* Jumping to a label within the same generated code block.
* Subroutine calls.
* Jumping to the location associated with a mode and emulated program
  counter value.

Labels are identified using 32-bit unsigned integer values.  Label
numbers must be unique within a generated code block.  Locations are
marked as potential targets for intra-block jumps using the
:ref:`LABEL <umlinst-label>` instruction.  The :ref:`JMP <umlinst-jmp>`
instruction is used to transfer control to the location associated with
a label.
