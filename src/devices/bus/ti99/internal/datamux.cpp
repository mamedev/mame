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
#include "cpu/tms9900/tms99com.h"

#define LOG_WARN        (1U<<1)   // Warnings
#define LOG_READY       (1U<<2)   // READY line
#define LOG_ACCESS      (1U<<3)   // Access to this GROM
#define LOG_ADDRESS     (1U<<4)   // Address register
#define LOG_WAITCOUNT   (1U<<5)   // Wait state counter

#define VERBOSE ( LOG_GENERAL | LOG_WARN )

#include "logmacro.h"

DEFINE_DEVICE_TYPE_NS(TI99_DATAMUX, bus::ti99::internal, datamux_device, "ti99_datamux", "TI-99 Databus multiplexer")

namespace bus { namespace ti99 { namespace internal {

/*
    Constructor
*/
datamux_device::datamux_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TI99_DATAMUX, tag, owner, clock),
	m_video(*owner, TI_VDP_TAG),
	m_sound(*owner, TI_SOUNDCHIP_TAG),
	m_ioport(*owner, TI99_IOPORT_TAG),
	m_gromport(*owner, TI99_GROMPORT_TAG),
	m_ram16b(*owner, TI99_EXPRAM_TAG),
	m_padram(*owner, TI99_PADRAM_TAG),
	m_cpu(*owner, "maincpu"),
	m_grom0(*owner, TI99_GROM0_TAG),
	m_grom1(*owner, TI99_GROM1_TAG),
	m_grom2(*owner, TI99_GROM2_TAG),
	m_ready(*this),
	m_addr_buf(0),
	m_dbin(CLEAR_LINE),
	m_muxready(CLEAR_LINE),
	m_sysready(CLEAR_LINE),
	m_latch(0),
	m_waitcount(0),
	m_romgq_state(CLEAR_LINE),
	m_memen_state(CLEAR_LINE),
	m_use32k(false),
	m_base32k(0),
	m_console_groms_present(false),
	m_grom_idle(true)
{
}

/***************************************************************************
    DEVICE ACCESSOR FUNCTIONS
***************************************************************************/

void datamux_device::read_all(uint16_t addr, uint8_t *value)
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
				m_grom0->readz(value);
				m_grom1->readz(value);
				m_grom2->readz(value);
			}
			// GROMport (GROMs)
			m_gromport->readz(addr, value);
			m_grom_idle = false;
		}

		// Video
		if ((addr & 0xf801)==0x8800)
		{
			// Forward to VDP unless we have an EVPC
			if (m_video != nullptr) *value = m_video->read(addr>>1); // A14 determines data or register read
		}
	}

	// GROMport (ROMs)
	if ((addr & 0xe000)==0x6000) m_gromport->readz(addr, value);

	// I/O port gets all accesses
	m_ioport->readz(addr, value);
	m_ioport->memen_in(CLEAR_LINE);
	m_memen_state = CLEAR_LINE;
}

void datamux_device::write_all(uint16_t addr, uint8_t value)
{
	// GROM access
	if ((addr & 0xf801)==0x9800)
	{
		if (m_console_groms_present)
		{
			m_grom0->write(value);
			m_grom1->write(value);
			m_grom2->write(value);
		}
		// GROMport
		m_gromport->write(addr, value);
		m_grom_idle = false;
	}

	// Cartridge port and sound
	if ((addr & 0xe000)==0x6000) m_gromport->write(addr, value);

	// Only if the sound chip has not been removed
	if ((addr & 0xfc01)==0x8400)
	{
		if (m_sound != nullptr) m_sound->write(value);
	}

	// Video
	if ((addr & 0xf801)==0x8800)
	{
		// Forward to VDP unless we have an EVPC
		if (m_video != nullptr) m_video->write(addr>>1, value);   // A14 determines data or register write
	}

	// I/O port gets all accesses
	m_ioport->write(addr, value);
	m_ioport->memen_in(CLEAR_LINE);
	m_memen_state = CLEAR_LINE;
}

void datamux_device::setaddress_all(uint16_t addr)
{
	line_state a14 = ((addr & 2)!=0)? ASSERT_LINE : CLEAR_LINE;

	// Valid access = not(DBIN and A5)
	bool validaccess = (m_dbin==CLEAR_LINE || (addr & 0x0400)==0);

	// GROM access
	bool isgrom = ((addr & 0xf801)==0x9800) && validaccess;

	// Cartridge ROM
	bool iscartrom = ((addr & 0xe000)==0x6000);

	// Always deliver to GROM so that the select line may be cleared
	line_state gsq = isgrom? ASSERT_LINE : CLEAR_LINE;
	if (isgrom) m_grom_idle = false;

	if (m_console_groms_present)
	{
		m_grom0->set_lines((line_state)m_dbin, a14, gsq);
		m_grom1->set_lines((line_state)m_dbin, a14, gsq);
		m_grom2->set_lines((line_state)m_dbin, a14, gsq);
	}

	// GROMport (GROMs)
	m_gromport->set_gromlines((line_state)m_dbin, a14, gsq);

	// Sound chip and video chip do not require the address to be set before access

	// GROMport (ROMs)
	m_romgq_state = iscartrom? ASSERT_LINE : CLEAR_LINE;
	m_gromport->romgq_line(m_romgq_state);

	// I/O port gets all accesses
	m_memen_state = ASSERT_LINE;
	m_ioport->memen_in(m_memen_state);
	m_ioport->setaddress_dbin(addr, m_dbin);
}

/*
    Special debugger access. The access is similar to the normal access,
    but it bypasses the wait state circuitry. Also, access ports of memory-
    mapped devices are excluded because their state would be changed
    unpredictably by the debugger access.
*/
uint16_t datamux_device::debugger_read(uint16_t addr)
{
	uint16_t addrb = addr << 1;
	uint16_t value = 0;

	if ((addrb & 0xe000)==0x0000) value = m_consolerom[(addrb & 0x1fff)>>1];
	else
	{
		if ((addrb & 0xfc00)==0x8000)
			value = (m_padram->pointer()[addrb & 0x00ff] << 8) | m_padram->pointer()[(addrb & 0x00ff)+1];
		else
		{
			int base32k = 0;
			if (m_use32k)
			{
				if ((addrb & 0xe000)==0x2000) base32k = 0x2000;
				if (((addrb & 0xe000)==0xa000) || ((addrb & 0xc000)==0xc000)) base32k = 0x8000;
			}

			if (base32k != 0)
			{
				value = (m_ram16b->pointer()[addrb-base32k] << 8) | m_ram16b->pointer()[addrb-base32k+1];
			}
			else
			{
				uint8_t lval = 0;
				uint8_t hval = 0;

				if ((addrb & 0xe000)==0x6000)
				{
					m_gromport->romgq_line(ASSERT_LINE);
					m_gromport->readz(addrb+1, &lval);
					m_gromport->readz(addrb, &hval);
					m_gromport->romgq_line(m_romgq_state);  // reset to previous state
				}
				m_ioport->memen_in(ASSERT_LINE);
				m_ioport->readz(addrb+1, &lval);
				m_ioport->readz(addrb, &hval);
				m_ioport->memen_in(m_memen_state);   // reset to previous state
				value = ((hval << 8)&0xff00) | (lval & 0xff);
			}
		}
	}
	return value;
}

void datamux_device::debugger_write(uint16_t addr, uint16_t data)
{
	uint16_t addrb = addr << 1;

	if ((addrb & 0xe000)==0x0000) return;

	if ((addrb & 0xfc00)==0x8000)
	{
		m_padram->pointer()[addrb & 0x00ff] = data >> 8;
		m_padram->pointer()[(addrb & 0x00ff)+1] = data & 0xff;
	}
	else
	{
		int base32k = 0;
		if (m_use32k)
		{
			if ((addrb & 0xe000)==0x2000) base32k = 0x2000;
			if (((addrb & 0xe000)==0xa000) || ((addrb & 0xc000)==0xc000)) base32k = 0x8000;
		}

		if (base32k != 0)
		{
			m_ram16b->pointer()[addrb-base32k] = data >> 8;
			m_ram16b->pointer()[(addrb-base32k)+1] = data & 0xff;
		}
		else
		{
			if ((addrb & 0xe000)==0x6000)
			{
				m_gromport->romgq_line(ASSERT_LINE);
				m_gromport->write(addr+1, data & 0xff);
				m_gromport->write(addr, (data>>8) & 0xff);
				m_gromport->romgq_line(m_romgq_state);  // reset to previous state
			}

			m_ioport->memen_in(ASSERT_LINE);
			m_ioport->write(addr+1, data & 0xff);
			m_ioport->write(addr,  (data>>8) & 0xff);
			m_ioport->memen_in(m_memen_state);   // reset to previous state
		}
	}
}

/*
    Read access. We are using two loops because the delay between both
    accesses must not occur within the loop. So we have one access on the bus,
    a delay, and then the second access.

    mem_mask is irrelevant for TMS processors (cannot control bus width)
*/
uint16_t datamux_device::read(offs_t offset)
{
	uint16_t value = 0;

	// Care for debugger
	if (machine().side_effects_disabled())
	{
		return debugger_read(offset);
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
			value = (m_padram->pointer()[m_addr_buf & 0x00ff] << 8) | m_padram->pointer()[(m_addr_buf & 0x00ff)+1];
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
				value = (m_ram16b->pointer()[m_addr_buf-m_base32k] << 8) | m_ram16b->pointer()[(m_addr_buf-m_base32k)+1];
			}
			else
			{
				// The byte from the odd address has already been read into the latch
				// Reading the even address now (addr)
				uint8_t hbyte = 0;
				read_all(m_addr_buf, &hbyte);
				LOGMASKED(LOG_ACCESS, "Read even byte from address %04x -> %02x\n",  m_addr_buf, hbyte);

				value = (hbyte<<8) | m_latch;
			}
		}
	}
	return value;
}

/*
    Write access.
*/
void datamux_device::write(offs_t offset, uint16_t data)
{
	if (machine().side_effects_disabled())
	{
		debugger_write(offset, data);
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
		m_padram->pointer()[(m_addr_buf & 0x00ff)] = data >> 8;
		m_padram->pointer()[(m_addr_buf & 0x00ff)+1] = data & 0xff;
		return;
	}

	// Handle the internal 32K expansion
	if (m_base32k != 0)
	{
		m_ram16b->pointer()[(m_addr_buf-m_base32k)] = data >> 8;
		m_ram16b->pointer()[(m_addr_buf-m_base32k)+1] = data & 0xff;
	}
	else
	{
		// Otherwise the datamux is in normal operation which means it puts
		// the even value into the latch and outputs the odd value now.
		m_latch = (data >> 8) & 0xff;

		// write odd byte
		LOGMASKED(LOG_ACCESS, "Write odd byte to address %04x <- %02x\n",  m_addr_buf+1, data & 0xff);
		write_all(m_addr_buf+1, data & 0xff);
	}
}

/*
    Called when the memory access starts by setting the address bus. From that
    point on, we suspend the CPU until all operations are done.
*/
void datamux_device::setaddress(offs_t offset, uint16_t busctrl)
{
	m_addr_buf = offset << 1;
	m_waitcount = 0;
	m_dbin = ((busctrl & TMS99xx_BUS_DBIN)!=0);

	LOGMASKED(LOG_ADDRESS, "Set address %04x\n", m_addr_buf);

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
		setaddress_all(m_addr_buf+1);
		m_muxready = CLEAR_LINE;
		ready_join();
	}
	else m_waitcount = 0;
}

/*
    The datamux is connected to the clock line in order to operate
    the wait state counter and to read/write the bytes.
*/
WRITE_LINE_MEMBER( datamux_device::clock_in )
{
	// return immediately if the datamux is currently inactive
	if (m_waitcount>0)
	{
		LOGMASKED(LOG_WAITCOUNT, "Wait count %d\n", m_waitcount);
		if (m_sysready==CLEAR_LINE)
		{
			LOGMASKED(LOG_READY, "Stalled due to external READY=0\n");
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
					read_all(m_addr_buf+1, &m_latch);
					LOGMASKED(LOG_ACCESS, "Read odd byte from address %04x -> %02x\n",  m_addr_buf+1, m_latch);
					// do the setaddress for the even address
					setaddress_all(m_addr_buf);
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
					setaddress_all(m_addr_buf);
					// write even byte
					LOGMASKED(LOG_ACCESS, "Write even byte to address %04x <- %02x\n",  m_addr_buf, m_latch);
					write_all(m_addr_buf, m_latch);
				}
			}
		}
	}
}

/*
    Combine the external (sysready) and the own (muxready) READY states.
*/
void datamux_device::ready_join()
{
	m_ready((m_sysready==CLEAR_LINE || m_muxready==CLEAR_LINE)? CLEAR_LINE : ASSERT_LINE);
}

WRITE_LINE_MEMBER( datamux_device::ready_line )
{
	if (state != m_sysready) LOGMASKED(LOG_READY, "READY line from PBox = %d\n", state);
	m_sysready = (line_state)state;
	// Also propagate to CPU via driver
	ready_join();
}

/* Called from VDP via console. */
WRITE_LINE_MEMBER( datamux_device::gromclk_in )
{
	// Don't propagate the clock in idle phase
	if (m_grom_idle) return;

	// Propagate to the GROMs
	if (m_console_groms_present)
	{
		m_grom0->gclock_in(state);
		m_grom1->gclock_in(state);
		m_grom2->gclock_in(state);
		m_grom_idle = m_grom0->idle();
	}
	m_gromport->gclock_in(state);

	// Only ask the gromport when we don't have GROMs in the console
	if (!m_console_groms_present)
		m_grom_idle = m_gromport->is_grom_idle();
}

/***************************************************************************
    DEVICE LIFECYCLE FUNCTIONS
***************************************************************************/

void datamux_device::device_start(void)
{
	m_muxready = ASSERT_LINE;
	m_ready.resolve();

	// Register persistable state variables
	save_item(NAME(m_addr_buf));
	save_item(NAME(m_dbin));
	save_item(NAME(m_muxready));
	save_item(NAME(m_sysready));
	save_item(NAME(m_latch));
	save_item(NAME(m_waitcount));
	save_item(NAME(m_use32k));
	save_item(NAME(m_base32k));
	save_item(NAME(m_console_groms_present));
	save_item(NAME(m_grom_idle));
}

void datamux_device::device_stop(void)
{
}

void datamux_device::device_reset(void)
{
	m_consolerom = (uint16_t*)owner()->memregion(TI99_CONSOLEROM)->base();
	m_use32k = (ioport("RAM")->read()==1);
	m_console_groms_present = (ioport("GROMENA")->read()==1);

	m_sysready = ASSERT_LINE;
	m_muxready = ASSERT_LINE;
	ready_join();

	m_waitcount = 0;
	m_latch = 0;

	m_dbin = CLEAR_LINE;
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

ioport_constructor datamux_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(datamux);
}

} } } // end namespace bus::ti99::internal

