.. _contributing-cxx:

C++ Coding Guidelines
=====================

.. contents:: :local:


.. _contributing-cxx-intro:

Introduction
------------

**In terms of coding conventions, the style present within an existing
source file should be favoured over the standards found below.**

When a new source file is being created, the following coding
conventions should be observed if creating a new file within the MAME
core (``src/emu`` and ``src/lib``).  If the source file is outside the
core, deference can be given to a contributor’s preferred style,
although it is strongly encouraged to code with the understanding that
the file may need to be comprehensible by more than one person as time
marches forward.


.. _contributing-cxx-definitions:

Definitions
-----------

Snake case
    All lowercase letters with words separated by underscores:
    ``this_is_snake_case``
Screaming snake case
    All uppercase letters with words separated by underscores:
    ``SCREAMING_SNAKE_CASE``
Camel case:
    Lowercase initial letter, first letter of each subsequent word
    capitalised, with no separators between words: ``exampleCamelCase``
Llama case:
    Uppercase initial letter, first letter of each subsequent word
    capitalised, with no separators between words: ``LlamaCaseSample``


.. _contributing-cxx-fileformat:

Source file format
------------------

MAME C++ source files are encoded as UTF-8 text, assuming fixed-width
characters, with tab stops at four-space intervals.  Source files should
end with a terminating end-of-line.  Any valid printable Unicode text is
permitted in comments.  Outside comments and strings, only the printable
ASCII subset of Unicode is permitted.

The ``srcclean`` tool is used to enforce file format rules before each
release.  You can build this tool and apply it to the files you modify
before opening a pull request to avoid conflicts or surprising changes
later.


.. _contributing-cxx-naming:

Naming conventions
------------------

Preprocessor macros
    Macro names should use screaming snake case.  Macros are always
    global and name conflicts can cause confusing errors – think
    carefully about what macros really need to be in headers and name
    them carefully.
Include guards
    Include guard macros should begin with ``MAME_``, and end with a
    capitalised version of the file name, with separators replaced by
    underscores.
Constants
    Constants should use screaming snake case, whether they are constant
    globals, constant data members, enumerators or preprocessor
    constants.
Functions
    Free functions names should use snake case.  (There are some utility
    function that were previously implemented as preprocessor macros
    that still use screaming snake case.)
Classes
    Class names should use snake case.  Abstract class names should end
    in ``_base``.  Public member functions (including static member
    functions) should use snake case.
Device classes
    Concrete driver ``driver_device`` implementation names
    conventionally end in ``_state``, while other concrete device class
    names end in ``_device``.  Concrete ``device_interface`` names
    conventionally begin with ``device_`` and end with ``_interface``.
Device types
    Device types should use screaming snake case.  Remember that device
    types are names in the global namespace, so choose explicit,
    unambiguous names.
Enumerations
    The enumeration name should use snake case.  The enumerators should
    use screaming snake case.
Template parameters
    Template parameters should use llama case (both type and value
    parameters).

Identifiers containing two consecutive underscores or starting with an
underscore followed by an uppercase letter are always reserved and
should not be used.

Type names and other identifiers with a leading underscore should be
avoided within the global namespace, as they are explicitly reserved
according to the C++ standard.  Additionally, identifiers suffixed with
``_t`` should be avoided within the global namespace, as they are also
reserved according to POSIX standards.  While MAME violates this policy
occasionally – most notably with ``device_t`` – it’s considered to be an
unfortunate legacy decision that should be avoided in any new code.


.. _contributing-cxx-literals:

Variables and literals
----------------------

Octal literals are discouraged from use outside of specific cases.  They
lack the obvious letter-based prefixes found in hexadecimal and binary
literals, and therefore can be difficult to distinguish at a glance from
a decimal literal to coders who are unfamiliar with octal notation.

Lower-case hexadecimal literals are preferred, e.g. ``0xbadc0de`` rather
than ``0xBADC0DE``.  For clarity, try not to exceed the bit width of the
variable which will be used to store it.

Binary literals have rarely been used in the MAME source code due to the
``0b`` prefix not being standardised until C++14, but there is no policy
to avoid their use.

Integer suffix notation should be used when specifying 64-bit literals,
but is not strictly required in other cases.  It can, however, clarify
the intended use of a given literal at a glance.  Uppercase long integer
literal suffixes should be used to avoid confusion with the digit 1,
e.g.  ``7LL`` rather than ``7ll``.

Digit grouping should be used for longer numeric literals, as it aids in
recognising order of magnitude or bit field positions at a glance.
Decimal literals should use groups of three digits, and hexadecimal
literals should use groups of four digits, outside of specific
situations where different grouping would be easier to understand, e.g.
``4'433'619`` or ``0xfff8'1fff``.

Types that do not have a specifically defined size should be avoided if
they are to be registered with MAME’s save-state system, as it harms
portability.  In general, this means avoiding the use of ``int`` for
these members.

It's encouraged, but not required, for class data members to be prefixed
with ``m_`` for non-static instance members and ``s_`` for static
members.  This does not apply to nested classes or structs.


.. _contributing-cxx-braceindent:

Bracing and indentation
-----------------------

Tabs are used for initial indentation of lines, with one tab used per
nested scope level.  Statements split across multiple lines should be
indented by two tabs.  Spaces are used for alignment at other places
within a line.

Either K&R or Allman-style bracing is preferred.  There is no specific
preference for bracing on single-line statements, although bracing
should be consistent for a given ``if/else`` block, as shown:

.. code-block:: C++

    if (x == 0)
    {
        return;
    }
    else
    {
        call_some_function();
        x--;
    }

When using a series of ``if``/``else`` or ``if``/``else if``/``else``
blocks with comments at the top indentation level, avoid extraneous
newlines.  The use of additional newlines may lead to ``else if`` or
``else`` blocks being missed due to the newlines pushing the blocks
outside the visible editor height:

.. code-block:: C++

    // Early-out if our hypothetical counter has run out.
    if (x == 0)
    {
        return;
    }
    // We should do something if the counter is running.
    else
    {
        call_some_function();
        x--;
    }

Indentation for ``case`` statements inside a ``switch`` body can either
be on the same level as the ``switch`` statement or inward by one level.
There is no specific style which is used across all core files, although
indenting by one level appears to be used most often.


.. _contributing-cxx-spacing:

Spacing
-------

Consistent single-spacing between binary operators, variables, and
literals is strongly preferred.  The following examples exhibit
reasonably consistent spacing:

.. code-block:: C++

    uint8_t foo = (((bar + baz) + 3) & 7) << 1;
    uint8_t foo = ((bar << 1) + baz) & 0x0e;
    uint8_t foo = bar ? baz : 5;

The following examples exhibit extremes in either direction, although
having extra spaces is less difficult to read than having too few:

.. code-block:: C++

    uint8_t foo = ( ( ( bar + baz ) + 3 ) & 7 ) << 1;
    uint8_t foo = ((bar<<1)+baz)&0x0e;
    uint8_t foo = (bar?baz:5);

A space should be used between a fundamental C++ statement and its
opening parenthesis, e.g.:

.. code-block:: C++

    switch (value) ...
    if (a != b) ...
    for (int i = 0; i < foo; i++) ...


.. _contributing-cxx-scoping:

Scoping
-------

Variables should be scoped as narrowly as is reasonably possible.  There
are many instances of C89-style local variable declaration in the MAME
codebase, but this is largely a hold-over from MAME’s early days, which
pre-date the C99 specification.

The following two snippets exhibit the legacy style of local variable
declaration, followed by the more modern and preferred style:

.. code-block:: C++

    void example_device::some_function()
    {
        int i;
        uint8_t data;

        for (i = 0; i < std::size(m_buffer); i++)
        {
            data = m_buffer[i];
            if (data)
            {
                some_other_function(data);
            }
        }
    }

.. code-block:: C++

    void example_device::some_function()
    {
        for (int i = 0; i < std::size(m_buffer); i++)
        {
            const uint8_t data = m_buffer[i];
            if (data)
            {
                some_other_function(data);
            }
        }
    }

Enumerated values, structs, and classes used only by one specific device
should be declared within the device's class itself.  This avoids
pollution of the global namespace and makes the device-specific use of
them more obvious at a glance.


.. _contributing-cxx-const:

Const Correctness
-----------------

Const-correctness has not historically been a strict requirement of code
that goes into MAME, but there’s increasing value in it as the amount of
code refactoring increases and technical debt decreases.

When writing new code, it’s worth taking the time to determine if a
local variable can be declared ``const``.  Similarly, it's encouraged to
consider which member functions of a new class can be ``const``
qualified.

In a similar vein, arrays of constants should be declared ``constexpr``
and should use screaming snake case, as outlined towards the top of this
document.  Lastly, arrays of C-style strings should be declared as both
a const array of const strings, as so:

.. code-block:: C++

    static const char *const EXAMPLE_NAMES[4] =
    {
        "1-bit",
        "2-bit",
        "4-bit",
        "Invalid"
    };


.. _contributing-cxx-comments:

Comments
--------

While ``/* ANSI C comments */`` are often found in the codebase, there
has been a gradual shift towards ``// C++-style comments`` for
single-line comments.  This is very much a guideline, and coders are
encouraged to use whichever style is most comfortable.

Unless specifically quoting content from a machine or ancillary
materials, comments should be in English so as to match the predominant
language that the MAME team shares worldwide.

Commented-out code should typically be removed prior to authoring a pull
request, as it has a tendency to rot due to the fast-moving nature of
MAME’s core API.  If there is a desire known beforehand for the code to
eventually be included, it should be bookended in ``if (0)`` or
``if (false)``, as code removed through a preprocessor macro will rot at
the same rate.


.. _contributing-cxx-helpers:

MAME-Specific Helpers
---------------------

When at all possible, use helper functions and macros for bit
manipulation operations.

The ``BIT(value, bit)`` helper can be used to extract the state of a bit
at a given position from an integer value.  The resulting value will be
aligned to the least significant bit position, i.e. will be either 0 or
1.

An overload of the same function, ``BIT(value, bit, width)`` can be used
to extract a bit field of a specified width from an integer value,
starting at the specified bit position.  The result will also be
right-justified and will be of the same type as the incoming value.

There are, additionally, a number of helpers for functionality such as
counting leading zeroes/ones, population count, and signed/unsigned
integer multiplication and division for both 32-bit and 64-bit results.
Not all of these helpers have wide use in the MAME codebase, but using
them in new code is strongly preferred when that code is performance-
critical, as they utilise inline assembly or compiler intrinsics per-
platform when available.

``count_leading_zeros_32/64(T value)``
    Accepts an unsigned 32/64-bit value and returns an unsigned 8-bit
    value containing the number of consecutive zeros starting from the
    most significant bit.
``count_leading_ones_32/64(T value)``
    Same functionality as above, but examining consecutive one-bits.
``population_count_32/64(T value)``
    Accepts an unsigned 32/64-bit value and returns the number of
    one-bits found, i.e. the Hamming weight of the value.
``rotl_32/64(T value, int shift)``
    Performs a circular/barrel left shift of an unsigned 32/64-bit value
    with the specified shift value. The shift value will be masked to
    the valid bit range for a 32-bit or 64-bit value.
``rotr_32/64(T value, int shift)``
    Same functionality as above, but with a right shift.

For documentation on helpers related to multiplication and division,
refer to ``src/osd/eminline.h``.


.. _contributing-cxx-logging:

Logging
-------

MAME has multiple logging function for different purposes.  Two of the
most frequently used logging functions are ``logerror`` and
``osd_printf_verbose``:

* Devices inherit a ``logerror`` member function.  This automatically
  includes the fully-qualified tag of the invoking device in log
  messages.  Output is sent to MAME’s debugger’s rotating log buffer if
  the debugger is enabled.  If the
  :ref:`-log option <mame-commandline-log>` is enabled, it’s also
  written to the file ``error.log`` in the working directory.  If the
  :ref:`-oslog option <mame-commandline-oslog>` is enabled, it’s
  additionally sent to the OS diagnostic output (the host debugger
  diagnostic log on Windows if a host debugger is attached, or standard
  error otherwise).
* The output of the ``osd_printf_verbose`` function is sent to standard
  error if the :ref:`-verbose option <mame-commandline-verbose>` is
  enabled.

The ``osd_printf_verbose`` function should be used for logging that is
useful for diagnosing user issues, while ``logerror`` should be used for
messages more relevant to developers (either developing MAME itself, or
developing software for emulated systems using MAME’s debugger).

For debug logging, a channel-based logging system exists via the header
``logmacro.h``.  It can be used as a generic logging system as follows,
without needing to make use of its ability to mask out specific
channels:

.. code-block:: C++

    // All other headers in the .cpp file should be above this line.
    #define VERBOSE (1)
    #include "logmacro.h"
    ...
    void some_device::some_reg_write(u8 data)
    {
        LOG("%s: some_reg_write: %02x\n", machine().describe_context(), data);
    }

The above example also makes use of a helper function which is available
in all derivatives of ``device_t``: ``machine().describe_context()``.
This function will return a string that describes the emulation context
in which the function is being run.  This includes the fully-qualified
tag of the currently executing device (if any).  If the relevant device
implements ``device_state_interface``, it will also include the current
program-counter value reported by the device.

For more fine-grained control, specific bit masks can be defined and
used via the ``LOGMASKED`` macro:

.. code-block:: C++

    // All other headers in the .cpp file should be above this line.
    #define LOG_FOO (1 << 1U)
    #define LOG_BAR (1 << 2U)

    #define VERBOSE (LOG_FOO | LOG_BAR)
    #include "logmacro.h"
    ...
    void some_device::some_reg_write(u8 data)
    {
        LOGMASKED(LOG_FOO, "some_reg_write: %02x\n", data);
    }

    void some_device::another_reg_write(u8 data)
    {
        LOGMASKED(LOG_BAR, "another_reg_write: %02x\n", data);
    }

Note that the least significant bit position for user-supplied masks is
1, as bit position 0 is reserved for ``LOG_GENERAL``.

By default, ``LOG`` and ``LOGMASKED`` will use the device-supplied
``logerror`` function. However, this can be redirected as desired.  The
most common use case would be to direct output to the standard output
instead, which can be accomplished by explicitly defining
``LOG_OUTPUT_FUNC`` as so:

.. code-block:: C++

    #define LOG_OUTPUT_FUNC osd_printf_info

A developer should always ensure that ``VERBOSE`` is set to 0 and that
any definition of ``LOG_OUTPUT_FUNC`` is commented out prior to opening
a pull request.


.. _contributing-cxx-structure:

Structural organization
-----------------------

All C++ source files must begin with a two comments listing the
distribution license and copyright holders in a standard format.
Licenses are specified by their SPDX short identifier if available.
Here is an example of the standard format:

.. code-block:: C++

    // license:BSD-3-Clause
    // copyright-holders:David Haywood, Tomasz Slanina

Header includes should generally be grouped from most-dependent to
least-dependent, and sorted alphabetically within said groups:

* The project prefix header, ``emu.h``, must be the first thing in a
  translation unit
* Local project headers (i.e. headers found in the same source
  directory)
* Headers in ``src/devices``
* Headers in ``src/emu``
* Headers in ``src/lib/formats``
* Headers in ``src/lib/util``
* Headers from the OSD layer
* C++ standard library headers
* C standard library headers
* OS-specific headers
* Layout headers

Finally, task-specific headers such as ``logmacro.h`` - described in the
previous section - should be included last.  A practical example
follows:

.. code-block:: C++

    #include "emu.h"

    #include "cpu/m68000/m68000.h"
    #include "machine/mc68328.h"
    #include "machine/ram.h"
    #include "sound/dac.h"
    #include "video/mc68328lcd.h"
    #include "video/sed1375.h"

    #include "emupal.h"
    #include "screen.h"
    #include "speaker.h"

    #include "pilot1k.lh"

    #define VERBOSE (0)
    #include "logmacro.h"

In most cases, the class declaration for a system driver should be
within the corresponding source file along with the implementation.  In
such cases, the class declaration and all contents of the source file,
excluding the ``GAME``, ``COMP``, or ``CONS`` macro, should be enclosed
in an anonymous namespace (this produces better compiler diagnostics,
allows more aggressive optimisation, reduces the chance of duplicate
symbols, and reduces linking time).

Within a class declaration, there should be one section for each member
access level (``public``, ``protected`` and ``private``) if practical.
This may not be possible in cases where private constants and/or types
need to be declared before public members.  Members should use the least
public access level necessary.  Overridden virtual member functions
should generally use the same access level as the corresponding member
function in the base class.

Class member declarations should be grouped to aid understanding:

* Within a member access level section, constants, types, data members,
  instance member functions and static member functions should be
  grouped.
* In device classes, configuration member functions should be grouped
  separately from live signal member functions.
* Overridden virtual member functions should be grouped according to the
  base classes they are inherited from.

For classes with multiple overloaded constructors, constructor
delegation should be used where possible to avoid repeated member
initialiser lists.

Constants which are used by a device or machine driver should be in the
form of explicitly-sized enumerated values within the class declaration,
or be relegated to ``#define`` macros within the source file.  This
helps avoid polluting the preprocessor.
