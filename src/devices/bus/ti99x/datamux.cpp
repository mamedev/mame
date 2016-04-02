// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    TI-99/4(A) databus multiplexer circuit

    The DMUX is used to convert the 16-bit databus of the TMS9900 into
    an 8-bit databus. The processor writes a 16 bit word which is split
    by this circuit into two bytes that are sent subsequently over the 8-bit bus.
    In the opposite direction, one 16-bit read request from the CPU is
    translated into two 8-bit read requests (odd address / even address) from
    this datamux. Its 8-bit latch (LS373) holds the first (odd address) byte,
    while the datamux puts the CPU on hold, gets the second byte,
    and routes that second byte to the D0-D7 lines, while the latch now puts
    the first byte on D8-D15. Since we get two memory accesses each time,
    there are twice as many wait states than for a direct 16-bit access
    (order LSB, MSB).

    In addition, since the TMS 9900 also supports byte operations, all write
    operations are automatically preceded by a read operation, so this adds even
    more delays.

    Within the TI-99/4(A) console, only the internal ROM and the small internal
    RAM ("scratch pad RAM") are directly connected to the 16-bit bus. All other
    devices (video, audio, speech, GROM, and the complete P-Box system are
    connected to the datamux.

    The TMS9995 which is used in the Geneve has an internal multiplex, and
    the byte order is reversed: MSB, LSB

    ROM = 4K * 16 bit (8 KiB) system ROM (kind of BIOS, plus the GPL interpreter)
    RAM = 128 * 16 bit (256 byte) system RAM ("scratch pad")

    Many users (me too) used to solder a 16K * 16 bit (32 KiB) SRAM circuit into
    the console, before the datamux, decoded to 0x2000-0x3fff and 0xa000-0xffff.
    (This expansion was also called 0-waitstate, since it could be accessed
    with the full databus width, and the datamux did not create waitstates.)

    +---+                                                   +-------+
    |   |===##========##== D0-D7 ==========##===============|TMS9918| Video
    |   |   ||        ||                   ||               +-------+
    | T |   +-----+  +-----+      LS245  +----+
    | M |   | ROM |  | RAM |             +----+
    | S |   +-----+  +-----+               || |                     :
    |   |---||-||-----||-||----------------||-|---------------------:
    | 9 |   ||        ||    A0 - A14       || |                A0   : Sound
    | 9 |---||--------||-------------------||-|----------+    -A15  : GROM
    | 0 |   ||        ||      LS373  +-+   || | +----A15-+----------: Cartridges
    | 0 |   ||        ||   ##========|<|===## | |                   : Speech
    |   |   ||        ||   ||  +-+   +-+   || | |                   : Expansion
    |   |===## D8-D15 ##===##==|>|=====|===##=|=|=========== D0-D7 =: cards
    +---+                      +-+     |      | |                   :
      ^                     LS244|     |      | |
      |                          |     +--+---+-++
      |                          +--------| DMUX |---------------<--: READY
      +--- READY -------------------------+------+

         Databus width
        :------------- 16 bit ---------------|---------- 8 bit -----:

    A0=MSB; A15=LSB
    D0=MSB; D15=LSB

    We integrate the 16 bit memory expansion in this datamux component
    (pretending that the memory expansion was soldered on top of the datamux)

    January 2012: Rewritten as class

***************************************************************************/

#include "emu.h"
#include "datamux.h"

/*
    Constructor
*/
ti99_datamux_device::ti99_datamux_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DATAMUX, "Databus multiplexer", tag, owner, clock, "ti99_datamux", __FILE__),
	m_spacep(nullptr),
	m_ready(*this),
	m_addr_buf(0),
	m_dbin(CLEAR_LINE),
	m_muxready(CLEAR_LINE),
	m_sysready(CLEAR_LINE),
	m_latch(0),
	m_waitcount(0),
	m_ram16b(nullptr),
	m_use32k(false),
	m_base32k(0),
	m_console_groms_present(false)
	{ }

#define TRACE_READY 0
#define TRACE_ACCESS 0
#define TRACE_ADDRESS 0
#define TRACE_WAITCOUNT 0
#define TRACE_SETUP 0

/***************************************************************************
    DEVICE ACCESSOR FUNCTIONS
***************************************************************************/

void ti99_datamux_device::read_all(address_space& space, UINT16 addr, UINT8 *value)
{
	// Valid access
	bool validaccess = ((addr & 0x0400)==0);

	if (validaccess)
	{
		// GROM access
		if ((addr & 0xf801)==0x9800)
		{
			if (m_console_groms_present)
			{
				for (int i=0; i < 3; i++)
				{
					m_grom[i]->readz(space, addr, value);
				}
			}
			// GROMport (GROMs)
			m_gromport->readz(space, addr, value);
		}

		// Video
		if ((addr & 0xf801)==0x8800)
		{
			m_video->readz(space, addr, value);
		}
	}

	// GROMport (ROMs)
	if ((addr & 0xe000)==0x6000) m_gromport->readz(space, addr, value);

	// PEB gets all accesses
	m_peb->readz(space, addr, value);
	m_peb->memen_in(CLEAR_LINE);
}

void ti99_datamux_device::write_all(address_space& space, UINT16 addr, UINT8 value)
{
	// GROM access
	if ((addr & 0xf801)==0x9800)
	{
		if (m_console_groms_present)
		{
			for (int i=0; i < 3; i++)
				m_grom[i]->write(space, addr, value);
		}
		// GROMport
		m_gromport->write(space, addr, value);
	}

	// Cartridge port and sound
	if ((addr & 0xe000)==0x6000) m_gromport->write(space, addr, value);
	if ((addr & 0xfc01)==0x8400) m_sound->write(space, 0, value);

	// Video
	if ((addr & 0xf801)==0x8800)
	{
		m_video->write(space, addr, value);
	}

	// PEB gets all accesses
	m_peb->write(space, addr, value);
	m_peb->memen_in(CLEAR_LINE);
}

void ti99_datamux_device::setaddress_all(address_space& space, UINT16 addr)
{
	line_state a14 = ((addr & 2)!=0)? ASSERT_LINE : CLEAR_LINE;

	// Valid access = not(DBIN and A5)
	bool validaccess = (m_dbin==CLEAR_LINE || (addr & 0x0400)==0);

	// GROM access
	bool isgrom = ((addr & 0xf801)==0x9800) && validaccess;

	// Cartridge ROM
	bool iscartrom = ((addr & 0xe000)==0x6000);

	// Always deliver to GROM so that the select line may be cleared
	int lines = (m_dbin==ASSERT_LINE)? 1 : 0;
	if (a14==ASSERT_LINE) lines |= 2;
	line_state select = isgrom? ASSERT_LINE : CLEAR_LINE;

	if (m_console_groms_present)
		for (int i=0; i < 3; i++)
			m_grom[i]->set_lines(space, lines, select);

	// GROMport (GROMs)
	m_gromport->set_gromlines(space, lines, select);

	// Sound chip and video chip do not require the address to be set before access

	// GROMport (ROMs)
	m_gromport->romgq_line(iscartrom? ASSERT_LINE : CLEAR_LINE);

	// PEB gets all accesses
	m_peb->memen_in(ASSERT_LINE);
	m_peb->setaddress_dbin(space, addr, m_dbin);
}

/*
    Special debugger access. The access is similar to the normal access,
    but it bypasses the wait state circuitry. Also, access ports of memory-
    mapped devices are excluded because their state would be changed
    unpredictably by the debugger access.
*/
UINT16 ti99_datamux_device::debugger_read(address_space& space, UINT16 addr)
{
	UINT16 addrb = addr << 1;
	UINT16 value = 0;

	if ((addrb & 0xe000)==0x0000) value = m_consolerom[(addrb & 0x1fff)>>1];
	else
	{
		if ((addrb & 0xfc00)==0x8000) value = m_padram[(addrb & 0x00ff)>>1];
		else
		{
			int base32k = 0;
			if (m_use32k)
			{
				if ((addrb & 0xe000)==0x2000) base32k = 0x2000;
				if (((addrb & 0xe000)==0xa000) || ((addrb & 0xc000)==0xc000)) base32k = 0x8000;
			}

			if (base32k != 0) value = m_ram16b[(addrb-base32k)>>1];
			else
			{
				UINT8 lval = 0;
				UINT8 hval = 0;

				if ((addr & 0xe000)==0x6000)
				{
					m_gromport->readz(space, addrb+1, &lval);
					m_gromport->readz(space, addrb, &hval);
				}
				m_peb->memen_in(ASSERT_LINE);
				m_peb->readz(space, addrb+1, &lval);
				m_peb->readz(space, addrb, &hval);
				m_peb->memen_in(CLEAR_LINE);
				value = ((hval << 8)&0xff00) | (lval & 0xff);
			}
		}
	}
	return value;
}

void ti99_datamux_device::debugger_write(address_space& space, UINT16 addr, UINT16 data)
{
	UINT16 addrb = addr << 1;

	if ((addrb & 0xe000)==0x0000) return;

	if ((addrb & 0xfc00)==0x8000) m_padram[(addrb & 0x00ff)>>1] = data;
	else
	{
		int base32k = 0;
		if (m_use32k)
		{
			if ((addrb & 0xe000)==0x2000) base32k = 0x2000;
			if (((addrb & 0xe000)==0xa000) || ((addrb & 0xc000)==0xc000)) base32k = 0x8000;
		}

		if (base32k != 0) m_ram16b[(addrb-base32k)>>1] = data;
		else
		{
			if ((addr & 0xe000)==0x6000)
			{
				m_gromport->write(space, addr+1, data & 0xff);
				m_gromport->write(space, addr, (data>>8) & 0xff);
			}

			m_peb->memen_in(ASSERT_LINE);
			m_peb->write(space, addr+1, data & 0xff);
			m_peb->write(space, addr,  (data>>8) & 0xff);
			m_peb->memen_in(CLEAR_LINE);
		}
	}
}

/*
    Read access. We are using two loops because the delay between both
    accesses must not occur within the loop. So we have one access on the bus,
    a delay, and then the second access.

    mem_mask is always ffff on TMS processors (cannot control bus width)
*/
READ16_MEMBER( ti99_datamux_device::read )
{
	UINT16 value = 0;

	// Care for debugger
	if (space.debugger_access())
	{
		return debugger_read(space, offset);
	}

	// Addresses below 0x2000 are ROM (no wait states)
	if ((m_addr_buf & 0xe000)==0x0000)
	{
		value = m_consolerom[(m_addr_buf & 0x1fff)>>1];
	}
	else
	{
		// Addresses from 8300-83ff (mirrors at 8000, 8100, 8200) are console RAM  (no wait states)
		if ((m_addr_buf & 0xfc00)==0x8000)
		{
			value = m_padram[(m_addr_buf & 0x00ff)>>1];
		}
		else
		{
			// Looks ugly, but this is close to the real thing. If the 16bit
			// memory expansion is installed in the console, and the access hits its
			// space, just respond to the memory access and don't bother the
			// datamux in any way. In particular, do not make the datamux insert wait
			// states.

			if (m_base32k != 0)
			{
				value = m_ram16b[(m_addr_buf-m_base32k)>>1];
			}
			else
			{
				// The byte from the odd address has already been read into the latch
				// Reading the even address now (addr)
				UINT8 hbyte = 0;
				read_all(space, m_addr_buf, &hbyte);
				if (TRACE_ACCESS) logerror("Read even byte from address %04x -> %02x\n",  m_addr_buf, hbyte);

				value = (hbyte<<8) | m_latch;
			}
		}
	}
	return value;
}

/*
    Write access.
*/
WRITE16_MEMBER( ti99_datamux_device::write )
{
	if (space.debugger_access())
	{
		debugger_write(space, offset, data);
		return;
	}

	// Addresses below 0x2000 are ROM
	if ((m_addr_buf & 0xe000)==0x0000)
	{
		return;
	}

	// Addresses from 8300-83ff (mirrors at 8000, 8100, 8200) are console RAM
	if ((m_addr_buf & 0xfc00)==0x8000)
	{
		m_padram[(m_addr_buf & 0x00ff)>>1] = data;
		return;
	}

	// Handle the internal 32K expansion
	if (m_base32k != 0)
	{
		m_ram16b[(m_addr_buf-m_base32k)>>1] = data;
	}
	else
	{
		// Otherwise the datamux is in normal operation which means it puts
		// the even value into the latch and outputs the odd value now.
		m_latch = (data >> 8) & 0xff;

		// write odd byte
		if (TRACE_ACCESS) logerror("datamux: write odd byte to address %04x <- %02x\n",  m_addr_buf+1, data & 0xff);
		write_all(space, m_addr_buf+1, data & 0xff);
	}
}

/*
    Called when the memory access starts by setting the address bus. From that
    point on, we suspend the CPU until all operations are done.
*/
SETOFFSET_MEMBER( ti99_datamux_device::setoffset )
{
	m_addr_buf = offset << 1;
	m_waitcount = 0;

	if (TRACE_ADDRESS) logerror("set address %04x\n", m_addr_buf);

	if ((m_addr_buf & 0xe000) == 0x0000)
	{
		return; // console ROM
	}

	if ((m_addr_buf & 0xfc00) == 0x8000)
	{
		return; // console RAM
	}

	// Initialize counter
	// 1 cycle for loading into the datamux
	// 2 subsequent wait states (LSB)
	// 2 subsequent wait states (MSB)
	// clock cycle 6 is the nominal follower of the last wait state
	m_waitcount = 5;
	m_spacep = &space;

	m_base32k = 0;
	if (m_use32k)
	{
		if ((m_addr_buf & 0xe000)==0x2000) m_base32k = 0x2000;
		if (((m_addr_buf & 0xe000)==0xa000) || ((m_addr_buf & 0xc000)==0xc000)) m_base32k = 0x8000;
	}

	// Suspend the CPU if not using the 32K
	if (m_base32k == 0)
	{
		// propagate the setaddress operation
		// First the odd address
		setaddress_all(space, m_addr_buf+1);
		m_muxready = CLEAR_LINE;
		ready_join();
	}
	else m_waitcount = 0;
}

/*
    The datamux is connected to the clock line in order to operate
    the wait state counter and to read/write the bytes.
*/
WRITE_LINE_MEMBER( ti99_datamux_device::clock_in )
{
	// return immediately if the datamux is currently inactive
	if (m_waitcount>0)
	{
		if (TRACE_WAITCOUNT) logerror("datamux: wait count %d\n", m_waitcount);
		if (m_sysready==CLEAR_LINE)
		{
			if (TRACE_READY) logerror("datamux: stalled due to external READY=0\n");
			return;
		}

		if (m_dbin==ASSERT_LINE)
		{
			// Reading
			if (state==ASSERT_LINE)
			{   // raising edge
				if (--m_waitcount==0)
				{
					m_muxready = ASSERT_LINE;
					ready_join();
				}
				if (m_waitcount==2)
				{
					// read odd byte
					read_all(*m_spacep, m_addr_buf+1, &m_latch);
					if (TRACE_ACCESS) logerror("datamux: read odd byte from address %04x -> %02x\n",  m_addr_buf+1, m_latch);
					// do the setaddress for the even address
					setaddress_all(*m_spacep, m_addr_buf);
				}
			}
		}
		else
		{
			if (state==ASSERT_LINE)
			{   // raising edge
				if (--m_waitcount==0)
				{
					m_muxready = ASSERT_LINE;
					ready_join();
				}
			}
			else
			{   // falling edge
				if (m_waitcount==2)
				{
					// do the setaddress for the even address
					setaddress_all(*m_spacep, m_addr_buf);
					// write even byte
					if (TRACE_ACCESS) logerror("datamux: write even byte to address %04x <- %02x\n",  m_addr_buf, m_latch);
					write_all(*m_spacep, m_addr_buf, m_latch);
				}
			}
		}
	}
}

/*
    Combine the external (sysready) and the own (muxready) READY states.
*/
void ti99_datamux_device::ready_join()
{
	m_ready((m_sysready==CLEAR_LINE || m_muxready==CLEAR_LINE)? CLEAR_LINE : ASSERT_LINE);
}

WRITE_LINE_MEMBER( ti99_datamux_device::dbin_in )
{
	m_dbin = (line_state)state;
	if (TRACE_ADDRESS) logerror("data bus in = %d\n", (m_dbin==ASSERT_LINE)? 1:0 );
}

WRITE_LINE_MEMBER( ti99_datamux_device::ready_line )
{
	if (TRACE_READY)
	{
		if (state != m_sysready) logerror("READY line from PBox = %d\n", state);
	}
	m_sysready = (line_state)state;
	// Also propagate to CPU via driver
	ready_join();
}

WRITE_LINE_MEMBER( ti99_datamux_device::gromclk_in )
{
	// Propagate to the GROMs
	if (m_console_groms_present)
	{
		for (int i=0; i < 3; i++) m_grom[i]->gclock_in(state);
	}
	m_gromport->gclock_in(state);
}

/***************************************************************************
    DEVICE LIFECYCLE FUNCTIONS
***************************************************************************/

void ti99_datamux_device::device_start(void)
{
	m_ram16b = nullptr;
	m_muxready = ASSERT_LINE;
	m_ready.resolve();
}

void ti99_datamux_device::device_stop(void)
{
	m_ram16b = nullptr;
}

void ti99_datamux_device::device_reset(void)
{
	m_consolerom = (UINT16*)owner()->memregion(CONSOLEROM)->base();
	m_use32k = (ioport("RAM")->read()==1);
	m_console_groms_present = (ioport("GROMENA")->read()==1);

	// better use a region?
	if (m_ram16b==nullptr)
	{
		m_ram16b = make_unique_clear<UINT16[]>(32768/2);
	}

	m_sysready = ASSERT_LINE;
	m_muxready = ASSERT_LINE;
	ready_join();

	m_waitcount = 0;
	m_latch = 0;

	m_dbin = CLEAR_LINE;
}

void ti99_datamux_device::device_config_complete()
{
	m_video = downcast<bus8z_device*>(owner()->subdevice(VIDEO_SYSTEM_TAG));
	m_sound = downcast<sn76496_base_device*>(owner()->subdevice(TISOUNDCHIP_TAG));
	m_gromport = downcast<gromport_device*>(owner()->subdevice(GROMPORT_TAG));
	m_peb = downcast<peribox_device*>(owner()->subdevice(PERIBOX_TAG));
	m_grom[0] = downcast<tmc0430_device*>(owner()->subdevice(GROM0_TAG));
	m_grom[1] = downcast<tmc0430_device*>(owner()->subdevice(GROM1_TAG));
	m_grom[2] = downcast<tmc0430_device*>(owner()->subdevice(GROM2_TAG));
	m_padram = make_unique_clear<UINT16[]>(256/2);
}


INPUT_PORTS_START( datamux )
	PORT_START( "RAM" ) /* config */
	PORT_CONFNAME( 0x01, 0x00, "Console 32 KiB RAM upgrade (16 bit)" )
		PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
		PORT_CONFSETTING(    0x01, DEF_STR( On ) )

	PORT_START( "GROMENA" )
	PORT_CONFNAME( 0x01, 0x01, "Console GROMs" )
		PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
		PORT_CONFSETTING(    0x01, DEF_STR( On ) )

INPUT_PORTS_END

ioport_constructor ti99_datamux_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(datamux);
}

const device_type DATAMUX = &device_creator<ti99_datamux_device>;
