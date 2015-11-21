// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    GROM emulation (aka TMC0430)

          +----+--+----+
      AD7 |1    G    16| Vss
      AD6 |2    R    15| GR
      AD5 |3    O    14| Vdd
      AD4 |4    M    13| GRC
      AD3 |5         12| M
      AD2 |6         11| MO
      AD1 |7         10| GS*
      AD0 |8          9| Vcc
          +------------+

    GR  = GROM Ready. Should be connected to processor's READY/HOLD*.
    GRC = GROM clock. Typically in the range of 400-500 kHz.
    M   = Direction. 1=read, 0=write
    MO  = Mode. 1=address counter access, 0=data access
    GS* = GROM select. 0=select, 1=deselect

    GROMs are slow ROM devices, which are
    interfaced via a 8-bit data bus, and include an internal address pointer
    which is incremented after each read.  This implies that accesses are
    faster when reading consecutive bytes, although the address pointer can be
    read and written at any time.

    GROMs are generally used to store programs written in GPL (Graphic Programming
    Language): a proprietary, interpreted language. The GPL interpreter takes
    most space of the TI-99/4A system ROMs.

    Both TI-99/4 and TI-99/4A include three GROMs, with some start-up code,
    system routines and TI-Basic.  TI99/4 includes an additional Equation
    Editor.  According to the preliminary schematics found on ftp.whtech.com,
    TI-99/8 includes the three standard GROMs and 16 GROMs for the UCSD
    p-system.  TI99/2 does not include GROMs at all, and was not designed to
    support any, although it should be relatively easy to create an expansion
    card with the GPL interpreter and a /4a cartridge port.

    Communication with GROM is done by writing and reading data over the
    AD0-AD7 lines. Within the TI-99 systems, the address bus is decoded for
    the M, GS*, and MO lines: Writing a byte to address 9c02 asserts the GS* and
    MO line, and clears the M line, which means the transmitted byte is put into
    the internal address register. Two bytes must be written to set up the
    complete address.

    It was obviously planned to offer GRAM circuits as well, since the
    programming manuals refer to writing to a special address, clearing the MO
    line. Although the TI-99 systems reserve a port in the memory space, no one
    has ever seen a GRAM circuit in the wild. However, third-party products like
    HSGPL or GRAM Kracker simulate GRAMs using conventional RAM with some
    addressing circuitry, usually in a custom  chip.

    Each GROM is logically 8 KiB long. Original TI-built GROM are 6 KiB long;
    the extra 2kb can be read, and follow the following formula:

        GROM[0x1800+offset] = GROM[0x0800+offset] | GROM[0x1000+offset];

    (sounds like address decoding is incomplete - we are lucky we don't burn
    any silicon when doing so... Needless to say, some hackers simulated 8kb
    GRAMs and GROMs with normal RAM/PROM chips and glue logic.)

    The address pointer is incremented after each GROM operation, but it will
    always remain within the bounds of the currently selected GROM (e.g. after
    0x3fff comes 0x2000).

    Since address are 16-bit long, you can have up to 8 GROMs.  Accordingly,
    a cartridge may include up to 5 GROMs.

    Every GROM has an internal ID which represents the high-order three
    address bits. The address counter can be set to any value from 0
    to 0xffff; the GROM will only react when selected and when the current
    address counter's high-order bits match the ID of the chip.
    Example: When the ID is 6, the GROM will react when the address
    counter contains a value from 0xc000 to 0xdfff.

    CHECK: Reading the address increases the counter only once. The first access
    returns the MSB, the second (and all following accesses) return the LSB.

    Michael Zapf, August 2010
    January 2012: rewritten as class

***************************************************************************/

#include "emu.h"
#include "grom.h"

#define TRACE_ADDRESS 0

/*
    Constructor.
*/
ti99_grom_device::ti99_grom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: bus8z_device(mconfig, GROM, "TI-99 GROM device", tag, owner, clock, "ti99_grom", __FILE__), m_writable(false), m_ident(0), m_size(0),
	m_gromready(*this), m_clockrate(0), m_address(0), m_buffer(0), m_raddr_LSB(false), m_waddr_LSB(false), m_memptr(nullptr), m_timer(nullptr)
{
}

/*
    Reading from the chip. Represents an access with M=1, GS*=0. The MO bit is
    defined by the offset (0 or 1). This is the enhanced read function with
    Z state.
*/
READ8Z_MEMBER( ti99_grom_device::readz )
{
	// Prevent debugger access
	if (space.debugger_access()) return;

	if (offset & 2)
	{
		// GROMs generally answer the address read request
		// (important if GROM simulators do not serve the request but rely on
		// the console GROMs) so we don't check the ident

		/* When reading, reset the hi/lo flag byte for writing. */
		/* TODO: Verify this with a real machine. */
		m_waddr_LSB = false;

		/* Address reading is done in two steps; first, the high byte */
		/* is transferred, then the low byte. */
		if (m_raddr_LSB)
		{
			/* second pass */
			*value = m_address & 0x00ff;
			m_raddr_LSB = false;
		}
		else
		{
			/* first pass */
			*value = (m_address & 0xff00)>>8;
			m_raddr_LSB = true;
		}
	}
	else
	{
		if (((m_address >> 13)&0x07)==m_ident)
		{
			// GROMs are buffered. Data is retrieved from a buffer,
			// while the buffer is replaced with the next cell content.
			if (TRACE_ADDRESS) if (m_ident==0) logerror("grom0: %04x = %02x\n", m_address-1, m_buffer);
			*value = m_buffer;
			// Get next value, put it in buffer. Note that the GROM
			// wraps at 8K boundaries.
			UINT16 addr = m_address-(m_ident<<13);

			if (m_size == 0x1800 && ((m_address&0x1fff)>=0x1800))
				m_buffer = m_memptr[addr-0x1000] | m_memptr[addr-0x0800];
			else
				m_buffer = m_memptr[addr];
		}
		// Note that all GROMs update their address counter.
		// TODO: Check this on a real console
		m_address = (m_address & 0xE000) | ((m_address + 1)&0x1FFF);

		// Reset the read and write address flipflops.
		m_raddr_LSB = m_waddr_LSB = false;

		// Maybe the timer is also required for address reading/setting, but
		// we don't have such technical details on GROMs.
		clear_ready();
	}
}

/*
    Writing to the chip. Represents an access with M=0, GS*=0. The MO bit is
    defined by the offset (0 or 1).
*/
WRITE8_MEMBER( ti99_grom_device::write )
{
	// Prevent debugger access
	if (space.debugger_access()) return;

	if (offset & 2)
	{
		/* write GROM address */
		/* see comments above */
		m_raddr_LSB = false;

		/* Implements the internal flipflop. */
		/* The Editor/Assembler manuals says that the current address */
		/* plus one is returned. This effect is properly emulated */
		/* by using a read-ahead buffer. */
		if (m_waddr_LSB)
		{
			/* Accept low byte (2nd write) */
			m_address = (m_address & 0xFF00) | data;
			/* Setting the address causes a new prefetch */
			if (is_selected())
			{
				m_buffer = m_memptr[m_address-(m_ident<<13)];
			}
			m_waddr_LSB = false;
			if (TRACE_ADDRESS) if (m_ident==0) logerror("grom0: %04x\n", m_address);
		}
		else
		{
			/* Accept high byte (1st write). Do not advance the address conter. */
			m_address = (data << 8) | (m_address & 0xFF);
			m_waddr_LSB = true;
			return;
		}
	}
	else
	{
		/* write GRAM data */
		if ((((m_address >> 13)&0x07)==m_ident) && m_writable)
		{
			UINT16 write_addr;
			// We need to rewind by 1 because the read address has already advanced.
			// However, do not change the address counter!
			write_addr = (m_address & 0xE000) | ((m_address - 1)&0x1FFF);

			// UINT16 addr = m_address-(m_ident<<13);
			if (m_size > 0x1800 || ((m_address&0x1fff)<0x1800))
				m_memptr[write_addr-(m_ident<<13)] = data;
		}
		m_raddr_LSB = m_waddr_LSB = false;
		clear_ready();
	}
	m_address = (m_address & 0xE000) | ((m_address + 1)&0x1FFF);
}

/*
    Timing. We assume that each data read results in READY going down for
    one cycle at the given frequency.
*/
void ti99_grom_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_gromready(ASSERT_LINE);
}

void ti99_grom_device::clear_ready()
{
	m_gromready(CLEAR_LINE);
	m_timer->adjust(attotime::from_hz(m_clockrate));
}

/***************************************************************************
    DEVICE FUNCTIONS
***************************************************************************/

void ti99_grom_device::device_start(void)
{
	const ti99grom_config *conf = reinterpret_cast<const ti99grom_config *>(static_config());

	m_memptr = owner()->memregion(conf->regionname)->base();
	assert (m_memptr!=NULL);
	m_memptr += conf->offset_reg;

	m_size = conf->size;
	m_clockrate = conf->clockrate;
	m_writable = conf->writable;
	m_ident = conf->ident;
	m_gromready.resolve_safe();

	m_timer = timer_alloc(0);
}

void ti99_grom_device::device_reset(void)
{
	m_address = 0;
	m_raddr_LSB = false;
	m_waddr_LSB = false;
	m_buffer = 0;
}

const device_type GROM = &device_creator<ti99_grom_device>;
