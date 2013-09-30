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
      |                          +--------| DMUX |
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
: device_t(mconfig, DATAMUX, "Databus multiplexer", tag, owner, clock, "ti99_datamux", __FILE__)
{
}

#define VERBOSE 1
#define LOG logerror

/***************************************************************************
    DEVICE ACCESSOR FUNCTIONS
***************************************************************************/

void ti99_datamux_device::read_all(address_space& space, UINT16 addr, UINT8 *target)
{
	attached_device *dev = m_devices.first();

	// Reading the odd address first (addr+1)
	while (dev != NULL)
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
	while (dev != NULL)
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
	while (dev != NULL)
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
    Read access. We are using two loops because the delay between both
    accesses must not occur within the loop. So we have one access on the bus,
    a delay, and then the second access (each one with possibly many attached
    devices)
*/
READ16_MEMBER( ti99_datamux_device::read )
{
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
		if (VERBOSE>3) LOG("datamux: read even byte from address %04x -> %02x\n",  m_addr_buf, hbyte);

		return ((hbyte<<8) | m_latch) & mem_mask;
	}
}

/*
    Write access.
*/
WRITE16_MEMBER( ti99_datamux_device::write )
{
	// Although MESS allows for using mem_mask to address parts of the
	// data bus, this is not used in the TMS implementation. In fact, all
	// accesses are true 16-bit accesses. We check for mem_mask always
	// being ffff.

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
		if (VERBOSE>3) LOG("datamux: write odd byte to address %04x <- %02x\n",  m_addr_buf+1, data & 0xff);
		write_all(space, m_addr_buf+1, data & 0xff);
	}
}

/*
    Called when the memory access starts by setting the address bus. From that
    point on, we suspend the CPU until all operations are done.
*/
SETOFFSET_MEMBER( ti99_datamux_device::setoffset )
{
	if (VERBOSE>6) LOG("datamux: set address %04x\n", offset << 1);
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
		m_ready(CLEAR_LINE);
	}
	else m_waitcount = 0;
}

/*
    The datamux is connected to the clock line in order to operate
    the wait state counter and to read/write the bytes.
*/
void ti99_datamux_device::clock_in(int clock)
{
	// return immediately if the datamux is currently inactive
	if (m_waitcount>0)
	{
		if (VERBOSE>6) LOG("datamux: wait count %d\n", m_waitcount);
		if (m_read_mode)
		{
			// Reading
			if (clock==ASSERT_LINE)
			{   // raising edge
				m_waitcount--;
				if (m_waitcount==0) m_ready(ASSERT_LINE);
				if (m_waitcount==2)
				{
					// read odd byte
					read_all(*m_spacep, m_addr_buf+1, &m_latch);
					if (VERBOSE>3) LOG("datamux: read odd byte from address %04x -> %02x\n",  m_addr_buf+1, m_latch);
					// do the setaddress for the even address
					setaddress_all(*m_spacep, m_addr_buf);
				}
			}
		}
		else
		{
			if (clock==ASSERT_LINE)
			{   // raising edge
				m_waitcount--;
				if (m_waitcount==0) m_ready(ASSERT_LINE);
			}
			else
			{   // falling edge
				if (m_waitcount==2)
				{
					// do the setaddress for the even address
					setaddress_all(*m_spacep, m_addr_buf);
					// write even byte
					if (VERBOSE>3) LOG("datamux: write even byte to address %04x <- %02x\n",  m_addr_buf, m_latch);
					write_all(*m_spacep, m_addr_buf, m_latch);
				}
			}
		}
	}
}

void ti99_datamux_device::dbin_in(int state)
{
	m_read_mode = (state==ASSERT_LINE);
	if (VERBOSE>6) LOG("datamux: data bus in = %d\n", m_read_mode? 1:0 );
}

/***************************************************************************
    DEVICE LIFECYCLE FUNCTIONS
***************************************************************************/

void ti99_datamux_device::device_start(void)
{
	m_ram16b = NULL;
}

void ti99_datamux_device::device_stop(void)
{
	if (m_ram16b) free(m_ram16b);
}

void ti99_datamux_device::device_reset(void)
{
	const datamux_config *conf = reinterpret_cast<const datamux_config *>(static_config());

	const dmux_device_list_entry *list = conf->devlist;
	m_ready.resolve(conf->ready, *this);

	m_cpu = machine().device("maincpu");
	// m_space = &m_cpu->memory().space(AS_PROGRAM);

	m_devices.reset(); // clear the list
	m_use32k = (ioport("RAM")->read()==1);

	// better use a region?
	if (m_ram16b==NULL)
	{
		m_ram16b = (UINT16*)malloc(32768);
		memset(m_ram16b, 0, 32768);
	}

	// Now building the list of active devices at this databus multiplex.
	// We allow for turning off devices according to configuration switch settings.
	// In particular, the HSGPL card cannot function unless the console GROMs are
	// removed.
	if ( list != NULL )
	{
		bool done = false;
		for (int i=0; !done; i++)
		{
			if (list[i].name == NULL)
			{
				done = true;
			}
			else
			{
				UINT32 set = 0;
				bool active_device = true;
				if (list[i].setting!=NULL)
				{
					set = ioport(list[i].setting)->read();
					active_device = ((set & list[i].set)==list[i].set) && ((set & list[i].unset)==0);
				}
				if (active_device)
				{
					device_t *dev = machine().device(list[i].name);
					if (dev != NULL)
					{
						attached_device *ad = new attached_device(dev, list[i]);
						m_devices.append(*ad);
						if (VERBOSE>8) LOG("datamux: Device %s mounted at index %d.\n", list[i].name, i);
					}
					else
					{
						if (VERBOSE>8) LOG("datamux: Device %s not found.\n", list[i].name);
					}
				}
				else
				{
					if (VERBOSE>8) LOG("datamux: Device %s not mounted due to configuration setting %s.\n", list[i].name, list[i].setting);
				}
			}
		}
	}
	if (VERBOSE>8) LOG("datamux: Device count = %d\n", m_devices.count());
	m_ready(ASSERT_LINE);

	m_waitcount = 0;
	m_latch = 0;

	m_read_mode = true;
}

INPUT_PORTS_START( datamux )
	PORT_START( "RAM" ) /* config */
	PORT_CONFNAME( 0x01, 0x01, "Console 32 KiB RAM upgrade (16 bit)" )
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
