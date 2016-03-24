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
: device_t(mconfig, DATAMUX, "Databus multiplexer", tag, owner, clock, "ti99_datamux", __FILE__), m_spacep(nullptr),
	m_ready(*this), m_muxready(), m_sysready(), m_addr_buf(0), m_read_mode(false), m_latch(0), m_waitcount(0), m_ram16b(nullptr), m_use32k(false), m_base32k(0), m_cpu(nullptr)
{ }

#define TRACE_READY 0
#define TRACE_ACCESS 0
#define TRACE_ADDRESS 0
#define TRACE_WAITCOUNT 0
#define TRACE_SETUP 0

/***************************************************************************
    DEVICE ACCESSOR FUNCTIONS
***************************************************************************/

void ti99_datamux_device::read_all(address_space& space, UINT16 addr, UINT8 *target)
{
	attached_device *dev = m_devices.first();

	// Reading the odd address first (addr+1)
	while (dev != nullptr)
	{
		if (dev->m_config->write_select != 0xffff) // write-only
		{
			if ((addr & dev->m_config->address_mask)==dev->m_config->select)
			{
				// Cast to the bus8z_device (see ti99defs.h)
				bus8z_device *devz = static_cast<bus8z_device *>(dev->m_device);
				devz->readz(space, addr, target);
			}
			// hope we don't have two devices answering...
			// consider something like a logical OR and maybe some artificial smoke
		}
		dev = dev->m_next;
	}
}

void ti99_datamux_device::write_all(address_space& space, UINT16 addr, UINT8 value)
{
	attached_device *dev = m_devices.first();
	while (dev != nullptr)
	{
		if ((addr & dev->m_config->address_mask)==(dev->m_config->select | dev->m_config->write_select))
		{
			bus8z_device *devz = static_cast<bus8z_device *>(dev->m_device);
			devz->write(space, addr, value);
		}
		dev = dev->m_next;
	}
}

void ti99_datamux_device::setaddress_all(address_space& space, UINT16 addr)
{
	attached_device *dev = m_devices.first();
	while (dev != nullptr)
	{
		if ((addr & dev->m_config->address_mask)==(dev->m_config->select | dev->m_config->write_select))
		{
			bus8z_device *devz = static_cast<bus8z_device *>(dev->m_device);
			devz->setaddress_dbin(space, addr, m_read_mode? ASSERT_LINE : CLEAR_LINE);
		}
		dev = dev->m_next;
	}
}

/*
    Special debugger access; these routines have no influence on the wait
    state generation.
*/
UINT16 ti99_datamux_device::debugger_read(address_space& space, UINT16 addr)
{
	UINT16 base32k = 0;
	UINT8 lval, hval;

	UINT16 addrb = addr << 1;
	if (m_use32k)
	{
		if ((addrb & 0xe000)==0x2000) base32k = 0x1000;
		if (((addrb & 0xe000)==0xa000) || ((addrb & 0xc000)==0xc000)) base32k = 0x4000;
	}
	if (base32k != 0)
	{
		return m_ram16b[addr - base32k];
	}
	else
	{
		lval = hval = 0;
		read_all(space, addrb+1, &lval);
		read_all(space, addrb, &hval);
		return ((hval << 8)&0xff00) | (lval & 0xff);
	}
}

void ti99_datamux_device::debugger_write(address_space& space, UINT16 addr, UINT16 data)
{
	UINT16 base32k = 0;

	UINT16 addrb = addr << 1;
	if (m_use32k)
	{
		if ((addrb & 0xe000)==0x2000) base32k = 0x1000;
		if (((addrb & 0xe000)==0xa000) || ((addrb & 0xc000)==0xc000)) base32k = 0x4000;
	}
	if (base32k != 0)
	{
		m_ram16b[addr - base32k] = data;
	}
	else
	{
		write_all(space, addrb+1, data & 0xff);
		write_all(space, addrb, (data >> 8) & 0xff);
	}
}

/*
    Read access. We are using two loops because the delay between both
    accesses must not occur within the loop. So we have one access on the bus,
    a delay, and then the second access (each one with possibly many attached
    devices)
*/
READ16_MEMBER( ti99_datamux_device::read )
{
	// Care for debugger
	if (space.debugger_access())
	{
		return debugger_read(space, offset);
	}

	// Looks ugly, but this is close to the real thing. If the 16bit
	// memory expansion is installed in the console, and the access hits its
	// space, just respond to the memory access and don't bother the
	// datamux in any way. In particular, do not make the datamux insert wait
	// states.

	if (m_base32k != 0)
	{
		UINT16 reply = m_ram16b[offset-m_base32k];
		return reply & mem_mask;
	}
	else
	{
		// The byte from the odd address has already been read into the latch
		// Reading the even address now (addr)
		UINT8 hbyte = 0;
		read_all(space, m_addr_buf, &hbyte);
		if (TRACE_ACCESS) logerror("datamux: read even byte from address %04x -> %02x\n",  m_addr_buf, hbyte);

		return ((hbyte<<8) | m_latch) & mem_mask;
	}
}

/*
    Write access.
*/
WRITE16_MEMBER( ti99_datamux_device::write )
{
	// Addresses below 0x2000 are ROM and should be handled in the address map
	// by the ROM entry, but as the write handler for ROM is not mapped, we end up
	// here when there are invalid accesses, and this will mess up everything.
	if (offset < 0x1000) return;

	if (space.debugger_access())
	{
		debugger_write(space, offset, data);
		return;
	}

	// Handle the internal 32K expansion
	if (m_base32k != 0)
	{
		m_ram16b[offset-m_base32k] = data;
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
	if (TRACE_ADDRESS) logerror("datamux: set address %04x\n", offset << 1);
	// Initialize counter
	// 1 cycle for loading into the datamux
	// 2 subsequent wait states (LSB)
	// 2 subsequent wait states (MSB)
	// clock cycle 6 is the nominal follower of the last wait state
	m_waitcount = 5;
	m_addr_buf = offset << 1;
	m_spacep = &space;

	m_base32k = 0;
	if (m_use32k)
	{
		if ((m_addr_buf & 0xe000)==0x2000) m_base32k = 0x1000;
		if (((m_addr_buf & 0xe000)==0xa000) || ((m_addr_buf & 0xc000)==0xc000)) m_base32k = 0x4000;
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
		if (m_read_mode)
		{
			// Reading
			if (state==ASSERT_LINE)
			{   // raising edge
				m_waitcount--;
				if (m_waitcount==0)
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
				m_waitcount--;
				if (m_waitcount==0)
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
	m_read_mode = (state==ASSERT_LINE);
	if (TRACE_ADDRESS) logerror("datamux: data bus in = %d\n", m_read_mode? 1:0 );
}

WRITE_LINE_MEMBER( ti99_datamux_device::ready_line )
{
	if (TRACE_READY)
	{
		if (state != m_sysready) logerror("datamux: READY line from PBox = %d\n", state);
	}
	m_sysready = (line_state)state;
	// Also propagate to CPU via driver
	ready_join();
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
	const datamux_config *conf = reinterpret_cast<const datamux_config *>(static_config());

	const dmux_device_list_entry *list = conf->devlist;

	m_cpu = machine().device("maincpu");
	// m_space = &m_cpu->memory().space(AS_PROGRAM);

	m_devices.reset(); // clear the list
	m_use32k = (ioport("RAM")->read()==1);

	// better use a region?
	if (m_ram16b==nullptr)
	{
		m_ram16b = make_unique_clear<UINT16[]>(32768/2);
	}

	// Now building the list of active devices at this databus multiplex.
	// We allow for turning off devices according to configuration switch settings.
	// In particular, the HSGPL card cannot function unless the console GROMs are
	// removed.
	if ( list != nullptr )
	{
		bool done = false;
		for (int i=0; !done; i++)
		{
			if (list[i].name == nullptr)
			{
				done = true;
			}
			else
			{
				UINT32 set;
				bool active_device = true;
				if (list[i].setting!=nullptr)
				{
					set = ioport(list[i].setting)->read();
					active_device = ((set & list[i].set)==list[i].set) && ((set & list[i].unset)==0);
				}
				if (active_device)
				{
					device_t *dev = machine().device(list[i].name);
					if (dev != nullptr)
					{
						auto ad = new attached_device(dev, list[i]);
						m_devices.append(*ad);
						if (TRACE_SETUP) logerror("datamux: Device %s mounted at index %d.\n", list[i].name, i);
					}
					else
					{
						if (TRACE_SETUP) logerror("datamux: Device %s not found.\n", list[i].name);
					}
				}
				else
				{
					if (TRACE_SETUP) logerror("datamux: Device %s not mounted due to configuration setting %s.\n", list[i].name, list[i].setting);
				}
			}
		}
	}
	if (TRACE_SETUP) logerror("datamux: Device count = %d\n", m_devices.count());

	m_sysready = ASSERT_LINE;
	m_muxready = ASSERT_LINE;
	ready_join();

	m_waitcount = 0;
	m_latch = 0;

	m_read_mode = true;
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
