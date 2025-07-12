File: PortMidi Win32 Readme
Author: Belinda Thom, June 16 2002
Revised by: Roger Dannenberg, June 2002, May 2004, June 2007, 
            Umpei Kurokawa, June 2007
            Roger Dannenberg Sep 2009, May 2022

Contents:
        Using Portmidi
        To Install Portmidi
        To Compile Portmidi
        About Cmake
        Using other versions of Visual C++
        To Create Your Own Portmidi Client Application
        


=============================================================================
USING PORTMIDI:
=============================================================================

I recommend building a static library and linking with your
application. PortMidi is not large. See ../README.md for
basic compiling instructions.

The Windows version has a couple of extra switches: You can define
DEBUG and MMDEBUG for a few extra messages (see the code).

If PM_CHECK_ERRORS is defined, PortMidi reports and exits on any
error. This requires terminal output to see, and aborts your
application, so it's only intended for quick command line programs
where you do not care to check return values and handle errors
more robustly.

PortMidi is designed to run without a console and should work perfectly 
well within a graphical user interface application.

Read the portmidi.h file for PortMidi API details on using the PortMidi API.
See <...>\pm_test\testio.c and other files in pm_test for usage examples.

There are many other programs in pm_test, including a MIDI monitor.
	

============================================================================
DESIGN NOTES
============================================================================

Orderly cleanup after errors are encountered is based on a fixed order of
steps and state changes to reflect each step. Here's the order:

To open input:
    initialize return value to NULL
    - allocate the PmInternal strucure (representation of PortMidiStream)
    return value is (non-null) PmInternal structure
    - allocate midi buffer
    set buffer field of PmInternal structure
    - call system-dependent open code
        - allocate midiwinmm_type for winmm dependent data
        set descriptor field of PmInternal structure
        - open device
        set handle field of midiwinmm_type structure
        - allocate buffers
        - start device
        - return
    - return

SYSEX HANDLING

There are three cases: simple output, stream output, input
Each must deal with:
 1. Buffer Initialization (creating buffers)
 2. Buffer Allocation (finding a free buffer)
 3. Buffer Fill (putting bytes in the buffer)
 4. Buffer Preparation (midiOutPrepare, etc.)
 5. Buffer Send (to Midi device)
 6. Buffer Receive (in callback)
 7. Buffer Empty (removing bytes from buffer)
 8. Buffer Free (returning to the buffer pool)
 9. Buffer Finalization (returning to heap)

Here's how simple output handles sysex:
 1. Buffer Initialization (creating buffers)
  allocated when code tries to write first byte to a buffer
  the test is "if (!m->sysex_buffers[0]) { ... }"
  this field is initialized to NULL when device is opened
  the size is SYSEX_BYTES_PER_BUFFER
  allocate_sysex_buffers() does the initialization
  note that the actual size of the allocation includes
      additional space for a MIDIEVENT (3 longs) which are
      not used in this case
 2. Buffer Allocation (finding a free buffer)
  see get_free_sysex_buffer()
  cycle through m->sysex_buffers[] using m->next_sysex_buffer
      to determine where to look next
  if nothing is found, wait by blocking on m->sysex_buffer_signal
  this is signaled by the callback every time a message is
      received
 3. Buffer Fill (putting bytes in the buffer)
  essentially a state machine approach
  hdr->dwBytesRecorded is a position in message pointed to by m->hdr
  keep appending bytes until dwBytesRecorded >= SYSEX_BYTES_PER_BUFFER
  then send the message, reseting the state to initial values
 4. Buffer Preparation (midiOutPrepare, etc.)
  just before sending in winmm_end_sysex()
 5. Buffer Send (to Midi device)
  message is padded with zero at end (since extra space was allocated
      this is ok) -- the zero works around a bug in (an old version of)
      MIDI YOKE drivers
  dwBufferLength gets dwBytesRecorded, and dwBytesRecorded gets 0
  uses midiOutLongMsg()
 6. Buffer Receive (in callback)
 7. Buffer Empty (removing bytes from buffer)
  not applicable for output
 8. Buffer Free (returning to the buffer pool)
  unprepare message to indicate that it is free
  SetEvent on m->buffer_signal in case client is waiting
 9. Buffer Finalization (returning to heap)
  when device is closed, winmm_out_delete frees all sysex buffers

Here's how stream output handles sysex:
 1. Buffer Initialization (creating buffers)
  same code as simple output (see above)
 2. Buffer Allocation (finding a free buffer)
  same code as simple output (see above)
 3. Buffer Fill (putting bytes in the buffer)
  essentially a state machine approach
  m->dwBytesRecorded is a position in message
  keep appending bytes until buffer is full (one byte to spare)
 4. Buffer Preparation (midiOutPrepare, etc.)
  done before sending message
  dwBytesRecorded and dwBufferLength are set in winmm_end_sysex
 5. Buffer Send (to Midi device)
  uses midiStreamOutMsg()
 6. Buffer Receive (in callback)
 7. Buffer Empty (removing bytes from buffer)
  not applicable for output
 8. Buffer Free (returning to the buffer pool)
  unprepare message to indicate that it is free
  SetEvent on m->buffer_signal in case client is waiting
 9. Buffer Finalization (returning to heap)
  when device is closed, winmm_out_delete frees all sysex buffers


Here's how input handles sysex:
 1. Buffer Initialization (creating buffers)
  two buffers are allocated in winmm_in_open
 2. Buffer Allocation (finding a free buffer)
  same code as simple output (see above)
 3. Buffer Fill (putting bytes in the buffer)
  not applicable for input
 4. Buffer Preparation (midiOutPrepare, etc.)
  done before sending message -- in winmm_in_open and in callback
 5. Buffer Send (to Midi device)
  uses midiInAddbuffer in allocate_sysex_input_buffer (called from
      winmm_in_open) and callback
 6. Buffer Receive (in callback)
 7. Buffer Empty (removing bytes from buffer)
      done without pause in loop in callback
 8. Buffer Free (returning to the buffer pool)
  done by midiInAddBuffer in callback, no pointer to buffers
      is retained except by device
 9. Buffer Finalization (returning to heap)
  when device is closed, empty buffers are delivered to callback,
      which frees them

IMPORTANT: In addition to the above, PortMidi now has
"shortcuts" to optimize the transfer of sysex data. To enable
the optimization for sysex output, the system-dependent code
sets fields in the pmInternal structure: fill_base, fill_offset_ptr,
and fill_length. When fill_base is non-null, the system-independent
part of PortMidi is allowed to directly copy sysex bytes to
"fill_base[*fill_offset_ptr++]" until *fill_offset_ptr reaches
fill_length. See the code for details.


