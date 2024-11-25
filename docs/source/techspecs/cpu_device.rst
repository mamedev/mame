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
            call appropriate restarted instruction handler
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

3.4 Bus contention cpu_device interface
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The main way to setup bus contention is through the memory maps.
Lower-level access can be obtained through some methods on cpu_device
though.

.. code-block:: C++

    bool cpu_device::access_before_time(u64 access_time, u64 current_time) noexcept;

The method ``access_before_time`` allows to try to run an access at a
given time in cpu cycles.  It takes the current time
(``total_cycles()``) and the expected time for the access.  If there
aren't enough cycles to reach that time the remaining cycles are eaten
and the method returns true to tell not to do the access and call the
method again eventually.  Otherwise enough cycles are eaten to reach
the access time and false is returned to tell to do the access.


.. code-block:: C++

    bool cpu_device::access_before_delay(u32 cycles, const void *tag) noexcept;

The method ``access_before_delay`` allows to try to run an access
after a given delay.  The tag is an opaque, non-nullptr value used to
characterize the source of the delay, so that the delay is not applied
multiple times.  Similarly to the previous method cycles are eaten and
true is returned to abort the access, false to execute it.

.. code-block:: C++

    void cpu_device::access_after_delay(u32 cycles) noexcept;

The method ``access_after_delay`` allows to add a delay after an
access is done.  There is no abort possible, hence no return boolean.

.. code-block:: C++

    void cpu_device::defer_access() noexcept;

The method ``defer_access`` tells the cpu that we need to wait for an
external event.  It marks the access as to be redone, and eats all the
remaining cycles of the timeslice.  The idea is then that the access
will be retried after time advances up to the next global system
synchronisation event (sync, timer timeout or set_input_line).  This
is the method to use when for instance waiting on a magic latch for
data expected from scsi transfers, which happen on timer timeouts.

.. code-block:: C++

    void cpu_device::retry_access() noexcept;

The method ``retry_access`` tells the cpu that the access will need to
be retried, and nothing else.  This can easily reach a situation of
livelock, so be careful.  It is used for instance to simulate a wait
line (for the z80 for instance) which is controlled through
set_input_line.  The idea is that the device setting wait does the
set_input_line and a retry_access.  The cpu core, as long as the wait
line is set just eats cycles.  Then, when the line is cleared the core
will retry the access.


3.5 Interaction with DRC
~~~~~~~~~~~~~~~~~~~~~~~~

At this point, interruptibility and DRC are entirely incompatible.  We
do not have a method to quit the generated code before or after an
access.  It's theorically possible but definitely non-trivial.

