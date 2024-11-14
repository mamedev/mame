CPU devices
===========

.. contents:: :local:


1. Overview
-----------

CPU devices derivatives are used, unsurprisingly, to implement the
emulation of CPUs, MCUs and SOCs.  A CPU device is first a combination
of ``device_execute_interface``, ``device_memory_interface``,
``device_state_interface`` and ``device_disasm_interface``.  Refer to
the associated documentations when they exist.

Two more functionalities are specific to CPU devices which are the DRC
and the interruptibility support.


2. DRC
------

TODO.


3. Interruptibility
-------------------

3.1 Definition
~~~~~~~~~~~~~~

An interruptible CPU is defined as a core which is able to suspend the
execution of one instruction at any time, exit execute_run, then at
the next call of ``execute_run`` keep going from where it was.  This
includes being able to abort an issued memory access, quit
execute_run, then upon the next call of execute_run reissue the exact
same access.


3.2 Implementation requirements
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Memory accesses must be done with ``read_interruptible`` or
``write_interruptible`` on a ``memory_access_specific`` or a
``memory_access_cache``.  The access must be done as bus width and bus
alignment.

After each access the core must test whether ``icount <= 0``.  This
test should be done after ``icount`` is decremented of the time taken
by the access itself, to limit the number of tests.  When ``icount``
reaches 0 or less it means that the instruction emulation needs to be
suspended.

To know whether the access needs to be re-issued,
``access_to_be_redone()`` needs to be called.  If it returns true then
the time taken by the access needs to be credited back, since it
hasn't yet happened, and the access will need to be re-issued.  The
call to ``access_to_be_redone()`` clears the reissue flag.  If you
need to check the flag without clearing it use
``access_to_be_redone_noclear()``.

The core needs to do enough bookkeeping to eventually restart the
instruction execution just before the access or just after the test,
depending on the need of reissue.

Finally, to indicate to the rest of the infrastructure the support, it
must override cpu_is_interruptible() to return true.


3.3 Example implementation with generators
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To ensure decent performance, the current implementations (h8, 6502
and 68000) use a python generator to generate two versions of each
instruction interpreter, one for the normal emulation, and one for
restarting the instruction.

The restarted version looks like that (for a 4-cycles per access cpu):

.. code-block:: C++

    void device::execute_inst_restarted()
    {
        switch(m_inst_substate) {
        case 0:
            [...]

            m_address = [...];
            m_mask = [...];
            [[fallthrough]];
        case 42:
            m_result = specific.read_interruptible(m_address, m_mask);
            m_icount -= 4;
            if(m_icount <= 0) {
                if(access_to_be_redone()) {
                    m_icount += 4;
                    m_inst_substate = 42;
                } else
                    m_inst_substate = 43;
                return;
            }
            [[fallthrough]];
        case 43:
            [...] = m_result;
            [...]
        }
        m_inst_substate = 0;
        return;
    }

The non-restarted version is the same thing with the switch and the
final ``m_inst_substate`` clearing removed.

.. code-block:: C++

    void device::execute_inst_non_restarted()
    {
        [...]
        m_address = [...];
        m_mask = [...];
        m_result = specific.read_interruptible(m_address, m_mask);
        m_icount -= 4;
        if(m_icount <= 0) {
            if(access_to_be_redone()) {
                m_icount += 4;
                m_inst_substate = 42;
            } else
                m_inst_substate = 43;
            return;
        }
        [...] = m_result;
        [...]
        return;
    }

The main loop then looks like this:

.. code-block:: C++

    void device::execute_run()
    {
        if(m_inst_substate)
            call appropriate restarted instrution handler
        while(m_icount > 0) {
            debugger_instruction_hook(m_pc);
            call appropriate non-restarted instruction handler
        }
    }

The idea is thus that ``m_inst_substate`` indicates where in an
instruction one is, but only when an interruption happens.  It
otherwise stays at 0 and is essentially never looked at.  Having two
versions of the interpretation allows to remove the overhead of the
switch and the end-of-instruction substate clearing.

It is not a requirement to use a generator-based that method, but a
different one which does not have unacceptable performance
implications has not yet been found.


3.4 Interaction with DRC
~~~~~~~~~~~~~~~~~~~~~~~~

At this point, interruptibility and DRC are entirely incompatible.  We
do not have a method to quit the generated code before or after an
access.  It's theorically possible but definitely non-trivial.

