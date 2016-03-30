// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    Graphics Read Only Memory (GROM, aka TMC0430)

          +----+--+----+
      AD7 |1    G    16| Vss (-0.8V)
      AD6 |2    R    15| GR
      AD5 |3    O    14| Vdd (-5V)
      AD4 |4    M    13| GRC
      AD3 |5         12| M
      AD2 |6         11| MO
      AD1 |7         10| GS*
      AD0 |8          9| Vcc (+5V)
          +------------+

    GR  = GROM Ready. Should be connected to processor's READY/HOLD*.
    GRC = GROM clock. Typically in the range of 400-500 kHz.
    M   = Direction. 1=read, 0=write
    MO  = Mode. 1=address counter access, 0=data access
    GS* = GROM select. 0=select, 1=deselect

    GROMs are slow ROM devices, which are interfaced via a 8-bit data bus,
    and which include an internal address pointer which is incremented
    after each read.  This implies that accesses are faster when reading
    consecutive bytes, although the address pointer can be read and written at any time.

    GROMs are generally used to store programs written in GPL (Graphic Programming
    Language): a proprietary, interpreted language from Texas Instruments which
    is the machine language in some kind of virtual machine that is running
    inside the TI-99/4, TI-99/4A, and TI-99/8 computers.

    Communication with GROM is done by writing and reading data over the
    AD0-AD7 lines. The M line determines whether the circuit will input or
    output data over the bus. For GROMs, writing data is only done for setting
    the internal address register. The MO line must be asserted for accessing
    this address register; otherwise data from the memory banks can be read.
    Clearing MO and M means writing data, and although so-called GRAMs were
    mentioned by Texas Instruments in the specifications and manuals, they
    were never seen. GROMs ignore this setting.

    Setting the address is done by writing two bytes to the circuit with
    M=0 and MO=1. In real systems, these lines are usually controlled by
    address bus lines, which maps the circuit into the memory space at specific
    addresses.

    The GROM address counter is 13 bits long (8 KiB), and judging from its
    behavior, the memory is organized as three banks of 2 KiB each:

    00 -> Bank 0
    01 -> Bank 1
    10 -> Bank 2
    11 -> Bank 1 OR Bank 2

    The fourth 2 KiB block seems to be a logical OR of the contents of
    bank 1 and 2. This means that one GROM delivers a maximum of 6 KiB of data.
    Nevertheless, the address counter advances into the forbidden bank, and
    wraps at its end to bank 0.

    8 GROMs can be used to cover a whole 16-bit address space, but only
    48 KiB of memory can be used. Each GROM has a burnt-in 3 bit identifier
    which allows us to put 8 GROMs in parallel, each one answering only
    when its area is currently selected.

    The address that is loaded into the address register contains two parts:

    [ I I I A A A A A A A A A A A A A ]

    The I bits indicate which GROM to use. They are latched inside every GROM,
    and when the address is read from the register, they are delivered as the
    most significant three address bits.

    All GROMs are wired in parallel, and only the circuits whose ID matches the
    prefix actually delivers the data to the outside. Apart from that, all
    GROMs perform the same internal operations. This means that each one of
    them holds the same address value and delivers it on request.

    Timing. GROMs have a CPU-bound operation phase and a non-CPU-bound operation
    phase. After being selected, the READY line is immediately lowered, and
    it raises as soon as the data is ready for access. After that, a prefetch
    is done to get the next data byte from the memory banks, advancing the
    address counter. This prefetch is also done when loading the address.
    Hence, reading the address register will always deliver a value increased
    by one.

    [1] Michael L. Bunyard: Hardware Manual for the Texas Instruments 99/4A Home Computer, section 2.5

    Michael Zapf, August 2010
    January 2012: rewritten as class

***************************************************************************/

#include "emu.h"
#include "tmc0430.h"

#define TRACE_ADDRESS 0
#define TRACE_DETAIL 0
#define TRACE_CLOCK 0
#define TRACE_READY 0
#define TRACE_LINE 0

/*
    Constructor.
*/
tmc0430_device::tmc0430_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMC0430, "TMC0430 device (GROM)", tag, owner, clock, "grom", __FILE__),
	m_gromready(*this),
	m_current_clock_level(CLEAR_LINE),
	m_current_ident(0),
	m_phase(0),
	m_address_mode(false),
	m_read_mode(false),
	m_selected(false),
	m_address_lowbyte(false),
	m_regionname(nullptr),
	m_ident(0),
	m_address(0),
	m_buffer(0),
	m_memptr(nullptr)
{
}

//  ========================================================================
//    Select lines. We have three lines for the direction, the mode, and
//    the selection. You can call one function for each of them, but it is
//    recommended to use the combined set_lines function, in particular when
//    there are lots of GROMs (see, for example, TI-99/8)
//  ========================================================================

/*
    Direction. When ASSERTed, GROM is set to be read by CPU.
*/
WRITE_LINE_MEMBER( tmc0430_device::m_line )
{
	m_read_mode = (state==ASSERT_LINE);
	if (TRACE_LINE) logerror("GROM %d dir %s\n", m_ident>>13, m_read_mode? "READ" : "WRITE");
}

/*
    Mode. When ASSERTed, the address counter will be accessed (read or write).
*/
WRITE_LINE_MEMBER( tmc0430_device::mo_line )
{
	m_address_mode = (state==ASSERT_LINE);
	if (TRACE_LINE) logerror("GROM %d mode %s\n", m_ident>>13, m_address_mode? "ADDR" : "DATA");
}

/*
    Select. When ASSERTed, the read/write operation is started.
*/
WRITE_LINE_MEMBER( tmc0430_device::gsq_line )
{
	if (state==ASSERT_LINE && !m_selected)      // check for edge
	{
		if (TRACE_READY) logerror("GROM %d selected, pulling down READY\n", m_ident>>13);
		m_gromready(CLEAR_LINE);
		m_phase = 4; // set for three full GROM clock ticks (and a fraction at the start)
	}
	m_selected = (state==ASSERT_LINE);
}

/*
    Combined select lines. Avoids separate calls to the chip.
    Address:
    0 -> MO=0, M=0
    1 -> MO=0, M=1
    2 -> MO=1, M=0
    3 -> MO=1, M=1
    Data: gsq line (ASSERT, CLEAR)
*/
WRITE8_MEMBER( tmc0430_device::set_lines )
{
	m_read_mode = ((offset & GROM_M_LINE)!=0);
	m_address_mode = ((offset & GROM_MO_LINE)!=0);

	if (data!=CLEAR_LINE && !m_selected)        // check for edge
	{
		if (TRACE_READY) logerror("GROM %d selected, pulling down READY\n", m_ident>>13);
		m_gromready(CLEAR_LINE);
		m_phase = 4; // set for three full GROM clock ticks (and a fraction at the start)
	}
	m_selected = (data!=CLEAR_LINE);
}

/*
    Clock in.

    Note about the GREADY line:
    Inside the TI-99/4A console, the GREADY outputs of all GROMs are directly
    connected in parallel and pulled up. This implies that the GROMs are
    open-drain outputs pulling down. There are two options:
    - Only the currently addressed GROM pulls down the line; all others keep
    their output open.
    - All GROMs act strictly in parallel. In the case that some circuits are
    slightly out of sync, the GREADY line goes up when the last circuit releases
    the line.

    For the emulation we may assume that all GROMs at the same clock line
    raise their outputs synchronously.
*/
WRITE_LINE_MEMBER( tmc0430_device::gclock_in )
{
	int bank = 0;
	UINT16 baddr = 0;

	// Wait for rising edge
	line_state oldlevel = m_current_clock_level;
	m_current_clock_level = (line_state)state;

	if ((m_current_clock_level==CLEAR_LINE) || (oldlevel==ASSERT_LINE))
		return;

	if (TRACE_CLOCK) logerror("GROMCLK in, phase=%d, m_add=%d\n", m_phase, m_address);

	switch (m_phase)
	{
	case 0:
		break;
	case 1:
		// Get the next value into the buffer
		// 000b b000 0000 0000
		baddr = m_address & 0x07ff;
		bank = (m_address & 0x1800)>>11;

		// This is a theory how the behavior of the GROM can be explained
		// We don't have decapped GROMs (yet)
		m_buffer = 0;
		if (bank == 0) m_buffer |= m_memptr[baddr];
		if (bank & 1) m_buffer  |= m_memptr[baddr | 0x0800];
		if (bank & 2) m_buffer  |= m_memptr[baddr | 0x1000];

		if (TRACE_ADDRESS) logerror("G>%04x\n", m_address);
		if (TRACE_DETAIL) logerror("GROM %d preload %04x (bank %d) -> %02x\n", m_ident>>13, m_address, bank, m_buffer);
		m_phase = 3;
		break;
	case 2: // Do nothing if other ident
		m_phase = 3;
		break;
	case 3:
		// Increase counter
		m_address = ((m_address + 1)&0x1fff) | m_current_ident;
		if (TRACE_DETAIL) logerror("GROM %d increase address to %04x\n", m_ident>>13, m_address);

		// In read mode, READY must have already been raised; in write mode, the ready line is still low
		m_phase = m_read_mode? 0 : 7;
		break;

	case 4: // Starting here when the chip is selected
		m_phase = 5;
		break;
	case 5: // Reached only when reading (writing continues at 1 or 2)
		m_phase = 6;
		break;
	case 6:
		m_phase = 7;
		break;
	case 7:
		if (TRACE_READY) logerror("GROM %d raising READY\n", m_ident>>13);
		m_gromready(ASSERT_LINE);
		// m_address_mode = false;
		// m_read = false;
		m_phase = 0;
		break;
	}
}

/*
    Read operation. For MO=Address, delivers the address register (and destroys its contents).
    For MO=Data, delivers the byte inside the buffer and prefetches the next one.
*/
READ8Z_MEMBER( tmc0430_device::readz )
{
	if (!m_selected) return;

	if (m_address_mode)
	{
		// Address reading is destructive
		*value = (m_address & 0xff00)>>8;
		UINT8 lsb = (m_address & 0x00ff);
		m_address = (lsb << 8) | lsb;       // see [1], section 2.5.3
		if (TRACE_DETAIL) logerror("GROM %d return address %02x\n", m_ident>>13, *value);
	}
	else
	{
		// Deliver the value from the output buffer.
		if (m_current_ident == m_ident)
		{
			*value = m_buffer;
			m_phase = 1;
		}
		else
		{
			m_phase = 2;
		}
	}
	m_address_lowbyte = false;
}

/*
    Write operation. For MO=Address, shifts value in the address register
    by 8 bits and copies the new value into the low byte. After every two
    write operations, prefetches the byte from the new location. For MO=Data,
    do nothing, because GRAMs were never seen in the wild.

    This operation occurs in parallel to phase 4. The real GROM will pick up
    the value from the data bus some phases later.
*/
WRITE8_MEMBER( tmc0430_device::write )
{
	if (!m_selected) return;

	if (m_address_mode)
	{
		m_address = ((m_address << 8) | data);          // [1], section 2.5.7
		m_current_ident = m_address & 0xe000;
		if (TRACE_DETAIL) logerror("GROM %d new address %04x (%s)\n", m_ident>>13, m_address, m_address_lowbyte? "complete" : "incomplete");
		// Toggle the lowbyte flag
		m_address_lowbyte = !m_address_lowbyte;

		if (!m_address_lowbyte)
		{
			// Do a prefetch if addressed, else increase your address
			m_phase = (m_current_ident == m_ident)? 1 : 2;
		}
	}

	// TODO: Check the duration of the access. In sum, both write accesses show correct timing,
	// but in this implementation, the first one (!m_address_lowbyte) is slower (phase=4-5-6-7-0) while the
	// second is faster (phase=1-3-7-0 or 1-3-0)
}

/***************************************************************************
    DEVICE FUNCTIONS
***************************************************************************/

void tmc0430_device::device_start(void)
{
	m_gromready.resolve_safe();
}

void tmc0430_device::device_reset(void)
{
	// The memory region must be defined in the owning component
	m_memptr = owner()->memregion(m_regionname)->base() + m_offset;
	m_address = 0;
	m_buffer = 0;
	m_current_ident = 0;
	m_address_lowbyte = false;
}

int tmc0430_device::debug_get_address()
{
	return m_address;
}

const device_type TMC0430 = &device_creator<tmc0430_device>;
