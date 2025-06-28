// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Megadrive / Genesis support

this could probably do with a complete rewrite at this point to take into
account all new information discovered since this was created, it's looking
rather old now.



Cleanup / Rewrite notes:



Known Non-Issues (confirmed on Real Genesis)
    Castlevania - Bloodlines (U) [!] - Pause text is missing on upside down level
    Blood Shot (E) (M4) [!] - corrupt texture in level 1 is correct...



*/


#include "emu.h"
#include "megadriv.h"

#include "machine/input_merger.h"

#include "speaker.h"

#define LOG_AUDIOBANK   (1U << 1) // z80 to 68k space window access at $8000-$ffff
#define LOG_AUDIOBUS    (1U << 2) // z80 bus grants
#define LOG_AUDIORESET  (1U << 3) // z80 reset line

#define VERBOSE (0)

#include "logmacro.h"

#define LOGAUDIOBANK(...)    LOGMASKED(LOG_AUDIOBANK, __VA_ARGS__)
#define LOGAUDIOBUS(...)     LOGMASKED(LOG_AUDIOBUS, __VA_ARGS__)
#define LOGAUDIORESET(...)   LOGMASKED(LOG_AUDIORESET, __VA_ARGS__)


void md_base_state::megadriv_z80_bank_w(uint16_t data)
{
	// TODO: menghu crashes here
	// Tries to setup a bank of 0xff0000 from z80 side (PC=1131) after you talk with the cashier twice.
	// Without a guard over it game will trash 68k memory causing a crash, works on real HW with everdrive
	// so not coming from a cart copy protection.
	// Update: it breaks cfodder BGM on character select at least, therefore we current don't guard against it
	// Apparently reading 68k RAM from z80 is not recommended by Sega, so *writing* isn't possible lacking bus grant?
	m_genz80.z80_bank_addr = ((m_genz80.z80_bank_addr >> 1) | (data << 23)) & 0xff8000;
}

void md_base_state::megadriv_68k_z80_bank_write(uint16_t data)
{
	megadriv_z80_bank_w(data & 0x01);
}

void md_base_state::megadriv_z80_z80_bank_w(uint8_t data)
{
	LOGAUDIOBANK("%s: port $6000 write 0x%02x ", machine().describe_context(), data);
	megadriv_z80_bank_w(data & 0x01);
	LOGAUDIOBANK("Current bank %08x\n", m_genz80.z80_bank_addr);
}

uint8_t md_base_state::megadriv_68k_YM2612_read(offs_t offset, uint8_t mem_mask)
{
	//osd_printf_debug("megadriv_68k_YM2612_read %02x %04x\n",offset,mem_mask);
	if ((m_genz80.z80_has_bus == 0) && (m_genz80.z80_is_reset == 0))
	{
		return m_ymsnd->read(offset);
	}
	else
	{
		LOG("%s: 68000 attempting to access YM2612 (read) without bus\n", machine().describe_context());
		return 0;
	}
}


void md_base_state::megadriv_68k_YM2612_write(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	//osd_printf_debug("megadriv_68k_YM2612_write %02x %04x %04x\n",offset,data,mem_mask);
	if ((m_genz80.z80_has_bus == 0) && (m_genz80.z80_is_reset == 0))
	{
		m_ymsnd->write(offset, data);
	}
	else
	{
		LOG("%s: 68000 attempting to access YM2612 (write) without bus\n", machine().describe_context());
	}
}

// this is used by 6 button pads and gets installed in machine_start for drivers requiring it
TIMER_CALLBACK_MEMBER(md_ctrl_state::ioport_timeout)
{
	m_ioport_phase[param] = 0;
}


/*

    A10001h = A0         Version register

    A10003h = 7F         Data register for port A
    A10005h = 7F         Data register for port B
    A10007h = 7F         Data register for port C

    A10009h = 00         Ctrl register for port A
    A1000Bh = 00         Ctrl register for port B
    A1000Dh = 00         Ctrl register for port C

    A1000Fh = FF         TxData register for port A
    A10011h = 00         RxData register for port A
    A10013h = 00         S-Ctrl register for port A

    A10015h = FF         TxData register for port B
    A10017h = 00         RxData register for port B
    A10019h = 00         S-Ctrl register for port B

    A1001Bh = FF         TxData register for port C
    A1001Dh = 00         RxData register for port C
    A1001Fh = 00         S-Ctrl register for port C




 Bit 7 - (Not connected)
 Bit 6 - TH
 Bit 5 - TL
 Bit 4 - TR
 Bit 3 - RIGHT
 Bit 2 - LEFT
 Bit 1 - DOWN
 Bit 0 - UP


*/

INPUT_PORTS_START( md_common )
	PORT_START("PAD1")      // Joypad 1 (3 button + start)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("%p B")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("%p C")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("%p A")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START )   PORT_PLAYER(1)
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED ) // extra buttons on 6-button pad

	PORT_START("PAD2")      // Joypad 2 (3 button + start)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("%p B")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("%p C")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("%p A")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START )   PORT_PLAYER(2)
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED ) // extra buttons on 6-button pad
INPUT_PORTS_END


INPUT_PORTS_START( megadriv )
	PORT_INCLUDE( md_common )

	PORT_START("RESET")     /* Buttons on Genesis Console */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Reset Button") PORT_IMPULSE(1) // reset, resets 68k (and..?)
INPUT_PORTS_END


template <unsigned N>
uint8_t md_ctrl_state::ioport_in_3button()
{
	ioport_value const pad = m_io_pad[N]->read();
	if (m_ioport_th[N])
		return BIT(pad, 0, 6);
	else
		return (BIT(pad, 6, 2) << 4) | BIT(pad, 0, 2);
}

template <unsigned N>
uint8_t md_ctrl_state::ioport_in_6button()
{
	ioport_value const pad = m_io_pad[N]->read();
	switch (m_ioport_phase[N])
	{
	default:
	case 0:
	case 1:
		if (m_ioport_th[N])
			return BIT(pad, 0, 6);
		else
			return (BIT(pad, 6, 2) << 4) | BIT(pad, 0, 2);
	case 2:
		if (m_ioport_th[N])
			return BIT(pad, 0, 6);
		else
			return BIT(pad, 6, 2) << 4;
	case 3:
		if (m_ioport_th[N])
			return (BIT(pad, 4, 2) << 4) | BIT(pad, 8, 4);
		else
			return (BIT(pad, 6, 2) << 4) | 0x0f;
	}
}

template <unsigned N>
void md_ctrl_state::ioport_out_3button(uint8_t data, uint8_t mem_mask)
{
	m_ioport_th[N] = BIT(data, 6);
}

template <unsigned N>
void md_ctrl_state::ioport_out_6button(uint8_t data, uint8_t mem_mask)
{
	uint8_t const th = BIT(data, 6);
	if (!th)
	{
		m_ioport_idle[N]->reset();
	}
	else if (!m_ioport_th[N])
	{
		m_ioport_idle[N]->adjust(attotime::from_usec(1500), N);
		m_ioport_phase[N] = (m_ioport_phase[N] + 1) & 0x03;
	}
	m_ioport_th[N] = th;
}


uint16_t md_base_state::m68k_version_read()
{
	/* Charles MacDonald ( http://cgfm2.emuviews.com/ )
	  D7 : Console is 1= Export (USA, Europe, etc.) 0= Domestic (Japan)
	  D6 : Video type is 1= PAL, 0= NTSC
	  D5 : Sega CD unit is 1= not present, 0= connected.
	  D4 : Unused (always returns zero)
	  D3 : Bit 3 of version number
	  D2 : Bit 2 of version number
	  D1 : Bit 1 of version number
	  D0 : Bit 0 of version number
	*/
	LOG("%s: read version register\n", machine().describe_context());
	// Version number contained in bits 3-0
	// TODO: non-TMSS BIOSes must return 0 here
	uint16_t const retdata = m_version_hi_nibble | 0x01;

	return retdata | (retdata << 8);
}

uint16_t md_base_state::m68k_ioport_data_read(offs_t offset)
{
	uint16_t retdata = m_ioports[offset]->data_r();
	return retdata | (retdata << 8);
}

uint16_t md_base_state::m68k_ioport_ctrl_read(offs_t offset)
{
	uint16_t retdata = m_ioports[offset]->ctrl_r();
	return retdata | (retdata << 8);
}

template <unsigned N>
uint16_t md_base_state::m68k_ioport_txdata_read()
{
	uint16_t retdata = m_ioports[N]->txdata_r();
	return retdata | (retdata << 8);
}

template <unsigned N>
uint16_t md_base_state::m68k_ioport_rxdata_read()
{
	uint16_t retdata = m_ioports[N]->rxdata_r();
	return retdata | (retdata << 8);
}

template <unsigned N>
uint16_t md_base_state::m68k_ioport_s_ctrl_read()
{
	uint16_t retdata = m_ioports[N]->s_ctrl_r();
	return retdata | (retdata << 8);
}

void md_base_state::m68k_ioport_data_write(offs_t offset, uint16_t data)
{
	m_ioports[offset]->data_w(uint8_t(data));
}

void md_base_state::m68k_ioport_ctrl_write(offs_t offset, uint16_t data)
{
	m_ioports[offset]->ctrl_w(uint8_t(data));
}

template <unsigned N>
void md_base_state::m68k_ioport_txdata_write(uint16_t data)
{
	m_ioports[N]->txdata_w(uint8_t(data));
}

template <unsigned N>
void md_base_state::m68k_ioport_s_ctrl_write(uint16_t data)
{
	m_ioports[N]->s_ctrl_w(uint8_t(data));
}



void md_base_state::megadriv_68k_base_map(address_map &map)
{
	map(0xa00000, 0xa01fff).rw(FUNC(md_base_state::megadriv_68k_read_z80_ram), FUNC(md_base_state::megadriv_68k_write_z80_ram));
	map(0xa02000, 0xa03fff).w(FUNC(md_base_state::megadriv_68k_write_z80_ram));
	map(0xa04000, 0xa04003).rw(FUNC(md_base_state::megadriv_68k_YM2612_read), FUNC(md_base_state::megadriv_68k_YM2612_write));

	map(0xa06000, 0xa06001).w(FUNC(md_base_state::megadriv_68k_z80_bank_write));

	map(0xa10000, 0xa10001).r(FUNC(md_base_state::m68k_version_read));
	map(0xa10002, 0xa10007).rw(FUNC(md_base_state::m68k_ioport_data_read), FUNC(md_base_state::m68k_ioport_data_write));
	map(0xa10008, 0xa1000d).rw(FUNC(md_base_state::m68k_ioport_ctrl_read), FUNC(md_base_state::m68k_ioport_ctrl_write));
	map(0xa1000e, 0xa1000f).rw(FUNC(md_base_state::m68k_ioport_txdata_read<0>), FUNC(md_base_state::m68k_ioport_txdata_write<0>));
	map(0xa10010, 0xa10011).r(FUNC(md_base_state::m68k_ioport_rxdata_read<0>));
	map(0xa10012, 0xa10013).rw(FUNC(md_base_state::m68k_ioport_s_ctrl_read<0>), FUNC(md_base_state::m68k_ioport_s_ctrl_write<0>));
	map(0xa10014, 0xa10015).rw(FUNC(md_base_state::m68k_ioport_txdata_read<1>), FUNC(md_base_state::m68k_ioport_txdata_write<1>));
	map(0xa10016, 0xa10017).r(FUNC(md_base_state::m68k_ioport_rxdata_read<1>));
	map(0xa10018, 0xa10019).rw(FUNC(md_base_state::m68k_ioport_s_ctrl_read<1>), FUNC(md_base_state::m68k_ioport_s_ctrl_write<1>));
	map(0xa1001a, 0xa1001b).rw(FUNC(md_base_state::m68k_ioport_txdata_read<2>), FUNC(md_base_state::m68k_ioport_txdata_write<2>));
	map(0xa1001c, 0xa1001d).r(FUNC(md_base_state::m68k_ioport_rxdata_read<2>));
	map(0xa1001e, 0xa1001f).rw(FUNC(md_base_state::m68k_ioport_s_ctrl_read<2>), FUNC(md_base_state::m68k_ioport_s_ctrl_write<2>));

	map(0xa11100, 0xa11101).rw(FUNC(md_base_state::megadriv_68k_check_z80_bus), FUNC(md_base_state::megadriv_68k_req_z80_bus));
	map(0xa11200, 0xa11201).w(FUNC(md_base_state::megadriv_68k_req_z80_reset));

	map(0xc00000, 0xc0001f).rw(m_vdp, FUNC(sega315_5313_device::vdp_r), FUNC(sega315_5313_device::vdp_w));
	map(0xd00000, 0xd0001f).rw(m_vdp, FUNC(sega315_5313_device::vdp_r), FUNC(sega315_5313_device::vdp_w)); // the earth defend
	map(0xe00000, 0xe0ffff).ram().mirror(0x1f0000).share("megadrive_ram");
//  map(0xff0000, 0xffffff).readonly();
	/*       0xe00000 - 0xffffff) == MAIN RAM (64kb, Mirrored, most games use ff0000 - ffffff) */
}

void md_base_state::megadriv_68k_map(address_map &map)
{
	megadriv_68k_base_map(map);

	map(0x000000, 0x3fffff).rom();
	/*      (0x000000 - 0x3fffff) == GAME ROM (4Meg Max, Some games have special banking too) */
}


/* z80 sounds/sub CPU */


uint16_t md_base_state::megadriv_68k_read_z80_ram(offs_t offset, uint16_t mem_mask)
{
	//osd_printf_debug("read z80 ram %04x\n",mem_mask);

	if ((m_genz80.z80_has_bus == 0) && (m_genz80.z80_is_reset == 0))
	{
		return m_genz80.z80_prgram[(offset<<1)^1] | (m_genz80.z80_prgram[(offset<<1)]<<8);
	}
	else
	{
		LOG("%06x: 68000 attempting to access Z80 (read) address space without bus\n", m_maincpu->pc());
		return machine().rand();
	}
}

void md_base_state::megadriv_68k_write_z80_ram(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if ((m_genz80.z80_has_bus == 0) && (m_genz80.z80_is_reset == 0))
	{
		if (!ACCESSING_BITS_0_7) // byte (MSB) access
		{
			m_genz80.z80_prgram[(offset<<1)] = (data & 0xff00) >> 8;
		}
		else if (!ACCESSING_BITS_8_15)
		{
			m_genz80.z80_prgram[(offset<<1)^1] = (data & 0x00ff);
		}
		else
		{
			// for WORD access only the MSB is used, LSB is ignored
			m_genz80.z80_prgram[(offset<<1)] = (data & 0xff00) >> 8;
		}
	}
	else
	{
		LOG("%06x: 68000 attempting to access Z80 (write) address space without bus\n", m_maincpu->pc());
	}
}

/*
 * ddragon, beast, superoff, and timekill have buggy sound programs.
 * They request the bus, then have a loop which waits for the bus
 * to be unavailable, checking for a 0 value due to bad coding.  The real hardware
 * appears to return bits of the next instruction in the unused bits, thus meaning
 * the value is never zero.  Time Killers is the most fussy, and doesn't like the
 * read_next_instruction function from system16, so I just return a random value
 * in the unused bits
 */
uint16_t md_base_state::megadriv_68k_check_z80_bus(offs_t offset, uint16_t mem_mask)
{
	uint16_t retvalue;


	uint16_t nextvalue = machine().rand(); //read_next_instruction(space)&0xff00;


	/* Check if the 68k has the z80 bus */
	if (!ACCESSING_BITS_0_7) // byte (MSB) access
	{
		if (m_genz80.z80_has_bus || m_genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

		LOGAUDIOBUS("%06x: 68000 check z80 Bus (byte MSB access) returning %04x mask %04x\n", m_maincpu->pc(),retvalue, mem_mask);
		return retvalue;

	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		LOGAUDIOBUS("%06x: 68000 check z80 Bus (byte LSB access) %04x\n", m_maincpu->pc(), mem_mask);
		if (m_genz80.z80_has_bus || m_genz80.z80_is_reset) retvalue = 0x0001;
		else retvalue = 0x0000;

		return retvalue;
	}
	else
	{
		LOGAUDIOBUS("%06x: 68000 check z80 Bus (word access) %04x\n", m_maincpu->pc(),mem_mask);
		if (m_genz80.z80_has_bus || m_genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

	//  osd_printf_debug("%06x: 68000 check z80 Bus (word access) %04x %04x\n", m_maincpu->pc(),mem_mask, retvalue);
		return retvalue;
	}
}


TIMER_CALLBACK_MEMBER(md_base_state::megadriv_z80_run_state)
{
	/* Is the z80 RESET line pulled? */
	if (m_genz80.z80_is_reset)
	{
		m_z80snd->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_ymsnd->reset();
	}
	else
	{
		m_z80snd->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

		/* Check if z80 has the bus */
		m_z80snd->set_input_line(Z80_INPUT_LINE_BUSRQ, m_genz80.z80_has_bus ? CLEAR_LINE : ASSERT_LINE);
	}
}


void md_base_state::megadriv_68k_req_z80_bus(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* Request the Z80 bus, allows 68k to read/write Z80 address space */
	if (!ACCESSING_BITS_0_7) // byte access
	{
		if (data & 0x0100)
		{
			LOGAUDIOBUS("%06x: 68000 request z80 Bus (byte MSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_has_bus = 0;
		}
		else
		{
			LOGAUDIOBUS("%06x: 68000 return z80 Bus (byte MSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_has_bus = 1;
		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			LOGAUDIOBUS("%06x: 68000 request z80 Bus (byte LSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_has_bus = 0;
		}
		else
		{
			LOGAUDIOBUS("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_has_bus = 1;
		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			LOGAUDIOBUS("%06x: 68000 request z80 Bus (word access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_has_bus = 0;
		}
		else
		{
			LOGAUDIOBUS("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_has_bus = 1;
		}
	}

	/* If the z80 is running, sync the z80 execution state */
	if (!m_genz80.z80_is_reset)
		m_genz80.z80_run_timer->adjust(attotime::zero);
}

void md_base_state::megadriv_68k_req_z80_reset(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!ACCESSING_BITS_0_7) // byte access
	{
		if (data & 0x0100)
		{
			LOGAUDIORESET("%06x: 68000 clear z80 reset (byte MSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_is_reset = 0;
		}
		else
		{
			LOGAUDIORESET("%06x: 68000 start z80 reset (byte MSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_is_reset = 1;
		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			LOGAUDIORESET("%06x: 68000 clear z80 reset (byte LSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_is_reset = 0;
		}
		else
		{
			LOGAUDIORESET("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_is_reset = 1;
		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			LOGAUDIORESET("%06x: 68000 clear z80 reset (word access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_is_reset = 0;
		}
		else
		{
			LOGAUDIORESET("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_is_reset = 1;
		}
	}
	m_genz80.z80_run_timer->adjust(attotime::zero);
}


// just directly access the 68k space, this makes it easier to deal with
// add-on hardware which changes the cpu mapping like the 32x and SegaCD.
// - we might need to add exceptions for example, z80 reading / writing the
//   z80 area of the 68k if games misbehave
uint8_t md_base_state::z80_read_68k_banked_data(offs_t offset)
{
	address_space &space68k = m_maincpu->space();
	uint8_t ret = space68k.read_byte(m_genz80.z80_bank_addr+offset);
	return ret;
}

void md_base_state::z80_write_68k_banked_data(offs_t offset, uint8_t data)
{
	address_space &space68k = m_maincpu->space();
	space68k.write_byte(m_genz80.z80_bank_addr+offset,data);
}

void md_base_state::megadriv_z80_vdp_write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x11:
		case 0x13:
		case 0x15:
		case 0x17:
			// accessed by either segapsg_device or sn76496_device
			m_vdp->vdp_w(offset >> 1, data, 0x00ff);
			break;

		default:
			osd_printf_debug("unhandled z80 vdp write %02x %02x\n",offset,data);
	}
}


uint8_t md_base_state::megadriv_z80_vdp_read(offs_t offset)
{
	u8 ret = 0;
	u8 shift = ((~offset & 1) << 3);
	switch (offset & ~1)
	{
		case 0x04: // ctrl_port_r
		case 0x06:
		case 0x08: // H/V counter
		case 0x0a:
		case 0x0c:
		case 0x0e:
			ret = m_vdp->vdp_r(offset >> 1, 0xff << shift) >> shift;
			break;

		default:
			if (!machine().side_effects_disabled())
				osd_printf_debug("unhandled z80 vdp read %02x\n",offset);
			ret = machine().rand();
			break;
	}
	return ret;
}

uint8_t md_base_state::megadriv_z80_unmapped_read()
{
	return 0xff;
}

void md_base_state::megadriv_z80_map(address_map &map)
{
	map(0x0000, 0x1fff).bankrw("bank1").mirror(0x2000); // RAM can be accessed by the 68k
	map(0x4000, 0x4003).rw(m_ymsnd, FUNC(ym_generic_device::read), FUNC(ym_generic_device::write));

	map(0x6000, 0x6000).w(FUNC(md_base_state::megadriv_z80_z80_bank_w));
	map(0x6001, 0x6001).w(FUNC(md_base_state::megadriv_z80_z80_bank_w)); // wacky races uses this address

	map(0x6100, 0x7eff).r(FUNC(md_base_state::megadriv_z80_unmapped_read));

	map(0x7f00, 0x7fff).rw(FUNC(md_base_state::megadriv_z80_vdp_read), FUNC(md_base_state::megadriv_z80_vdp_write));

	map(0x8000, 0xffff).rw(FUNC(md_base_state::z80_read_68k_banked_data), FUNC(md_base_state::z80_write_68k_banked_data)); // The Z80 can read the 68k address space this way
}

void md_base_state::megadriv_z80_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).noprw();
}

uint32_t md_core_state::screen_update_megadriv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// Copy our screen buffer here
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *const desty = &bitmap.pix(y, 0);
		uint32_t const *srcy;

		if (!m_vdp->m_use_alt_timing)
			srcy = &m_vdp->m_render_bitmap->pix(y, 0);
		else
			srcy = m_vdp->m_render_line.get();

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			desty[x] = srcy[x];
		}
	}

	return 0;
}



/*****************************************************************************************/

void md_core_state::machine_reset()
{
	if (!m_vdp->m_use_alt_timing)
	{
		m_vdp->m_megadriv_scanline_timer = m_scan_timer;
		m_vdp->m_megadriv_scanline_timer->adjust(attotime::zero);
	}

	m_vdp->device_reset_old();
}


void md_base_state::machine_start()
{
	md_core_state::machine_start();

	m_genz80.z80_run_timer = timer_alloc(FUNC(md_base_state::megadriv_z80_run_state), this);
}

void md_base_state::machine_reset()
{
	md_core_state::machine_reset();

	// default state of z80 = reset, with bus
	osd_printf_debug("Resetting Megadrive / Genesis\n");

	m_genz80.z80_is_reset = 1;
	m_genz80.z80_has_bus = 1;
	m_genz80.z80_bank_addr = 0;
	m_vdp->set_scanline_counter(-1);
	m_genz80.z80_run_timer->adjust(attotime::zero);

	if (m_megadrive_ram)
		memset(m_megadrive_ram, 0x00, 0x10000);
}


void md_ctrl_state::machine_start()
{
	md_base_state::machine_start();

	m_ioport_idle[0] = timer_alloc(FUNC(md_ctrl_state::ioport_timeout), this);
	m_ioport_idle[1] = timer_alloc(FUNC(md_ctrl_state::ioport_timeout), this);

	std::fill(std::begin(m_ioport_th), std::end(m_ioport_th), 1);
	std::fill(std::begin(m_ioport_phase), std::end(m_ioport_phase), 0);

	save_item(NAME(m_ioport_th));
	save_item(NAME(m_ioport_phase));
}


void md_base_state::megadriv_stop_scanline_timer()
{
	if (!m_vdp->m_use_alt_timing)
		m_vdp->m_megadriv_scanline_timer->reset();
}



// this comes from the VDP on lines 240 (on) 241 (off) and is connected to the z80 irq 0
void md_base_state::vdp_sndirqline_callback_genesis_z80(int state)
{
	if (state == ASSERT_LINE)
	{
		if ((m_genz80.z80_has_bus == 1) && (m_genz80.z80_is_reset == 0))
			m_z80snd->set_input_line(0, HOLD_LINE);
	}
	else if (state == CLEAR_LINE)
	{
		m_z80snd->set_input_line(0, CLEAR_LINE);
	}
}

// this comes from the vdp, and is connected to 68k irq level 6 (main vbl interrupt)
void md_core_state::vdp_lv6irqline_callback_genesis_68k(int state)
{
	if (state == ASSERT_LINE)
		m_maincpu->set_input_line(6, HOLD_LINE);
	else
		m_maincpu->set_input_line(6, CLEAR_LINE);
}

// this comes from the vdp, and is connected to 68k irq level 4 (raster interrupt)
void md_core_state::vdp_lv4irqline_callback_genesis_68k(int state)
{
	if (state == ASSERT_LINE)
		m_maincpu->set_input_line(4, HOLD_LINE);
	else
		m_maincpu->set_input_line(4, CLEAR_LINE);
}

/* Callback when the 68k takes an IRQ */
IRQ_CALLBACK_MEMBER(md_core_state::genesis_int_callback)
{
	if (irqline==4)
	{
		m_vdp->vdp_clear_irq4_pending();
	}

	if (irqline==6)
	{
		m_vdp->vdp_clear_irq6_pending();
	}

	return (0x60+irqline*4)/4; // vector address
}


void md_core_state::megadriv_timers(machine_config &config)
{
	TIMER(config, m_scan_timer).configure_generic(m_vdp, FUNC(sega315_5313_device::megadriv_scanline_timer_callback));
}

void md_core_state::md_core_ntsc(machine_config &config)
{
	M68000(config, m_maincpu, MASTER_CLOCK_NTSC / 7); // 7.67 MHz
	m_maincpu->set_irq_acknowledge_callback(FUNC(md_core_state::genesis_int_callback));
	// IRQs are handled via the timers

	megadriv_timers(config);

	SEGA315_5313(config, m_vdp, MASTER_CLOCK_NTSC, m_maincpu);
	m_vdp->set_is_pal(false);
	m_vdp->lv6_irq().set(FUNC(md_core_state::vdp_lv6irqline_callback_genesis_68k));
	m_vdp->lv4_irq().set(FUNC(md_core_state::vdp_lv4irqline_callback_genesis_68k));
	m_vdp->set_screen("megadriv");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(MASTER_CLOCK_NTSC / 10 / 262 / 342); // same as SMS?
//  m_screen->set_refresh_hz(double(MASTER_CLOCK_NTSC) / 8 / 262 / 427); // or 427 Htotal?
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0)); // Vblank handled manually.
	m_screen->set_size(64*8, 620);
	m_screen->set_visarea(0, 32*8-1, 0, 28*8-1);
	m_screen->set_screen_update(FUNC(md_core_state::screen_update_megadriv)); /* Copies a bitmap */
	m_screen->screen_vblank().set(FUNC(md_core_state::screen_vblank_megadriv)); /* Used to Sync the timing */
}

void md_core_state::md_core_pal(machine_config &config)
{
	M68000(config, m_maincpu, MASTER_CLOCK_PAL / 7); // 7.67 MHz
	m_maincpu->set_irq_acknowledge_callback(FUNC(md_core_state::genesis_int_callback));
	// IRQs are handled via the timers

	megadriv_timers(config);

	SEGA315_5313(config, m_vdp, MASTER_CLOCK_PAL, m_maincpu);
	m_vdp->set_is_pal(true);
	m_vdp->lv6_irq().set(FUNC(md_core_state::vdp_lv6irqline_callback_genesis_68k));
	m_vdp->lv4_irq().set(FUNC(md_core_state::vdp_lv4irqline_callback_genesis_68k));
	m_vdp->set_screen("megadriv");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(MASTER_CLOCK_PAL / 10 / 313 / 342); // same as SMS?
//  m_screen->set_refresh_hz(MASTER_CLOCK_PAL / 8 / 313 / 423); // or 423 Htotal?
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0)); // Vblank handled manually.
	m_screen->set_size(64*8, 620);
	m_screen->set_visarea(0, 32*8-1, 0, 28*8-1);
	m_screen->set_screen_update(FUNC(md_core_state::screen_update_megadriv)); /* Copies a bitmap */
	m_screen->screen_vblank().set(FUNC(md_core_state::screen_vblank_megadriv)); /* Used to Sync the timing */
}


void md_base_state::megadriv_ioports(machine_config &config)
{
	// TODO: this latches video counters as well as setting interrupt level 2
	auto &hl(INPUT_MERGER_ANY_HIGH(config, "hl"));
	hl.output_handler().set_inputline(m_maincpu, 2);

	MEGADRIVE_IO_PORT(config, m_ioports[0], 0);
	m_ioports[0]->hl_handler().set("hl", FUNC(input_merger_device::in_w<0>));

	MEGADRIVE_IO_PORT(config, m_ioports[1], 0);
	m_ioports[1]->hl_handler().set("hl", FUNC(input_merger_device::in_w<1>));

	MEGADRIVE_IO_PORT(config, m_ioports[2], 0);
	m_ioports[2]->hl_handler().set("hl", FUNC(input_merger_device::in_w<2>));
}


void md_ctrl_state::ctrl1_3button(machine_config &config)
{
	m_ioports[0]->set_in_handler(FUNC(md_ctrl_state::ioport_in_3button<0>));
	m_ioports[0]->set_out_handler(FUNC(md_ctrl_state::ioport_out_3button<0>));
}

void md_ctrl_state::ctrl2_3button(machine_config &config)
{
	m_ioports[1]->set_in_handler(FUNC(md_ctrl_state::ioport_in_3button<1>));
	m_ioports[1]->set_out_handler(FUNC(md_ctrl_state::ioport_out_3button<1>));
}

void md_ctrl_state::ctrl1_6button(machine_config &config)
{
	m_ioports[0]->set_in_handler(FUNC(md_ctrl_state::ioport_in_6button<0>));
	m_ioports[0]->set_out_handler(FUNC(md_ctrl_state::ioport_out_6button<0>));
}

void md_ctrl_state::ctrl2_6button(machine_config &config)
{
	m_ioports[1]->set_in_handler(FUNC(md_ctrl_state::ioport_in_6button<1>));
	m_ioports[1]->set_out_handler(FUNC(md_ctrl_state::ioport_out_6button<1>));
}


void md_base_state::md_ntsc(machine_config &config)
{
	md_core_ntsc(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &md_base_state::megadriv_68k_map);

	Z80(config, m_z80snd, MASTER_CLOCK_NTSC / 15); // 3.58 MHz
	m_z80snd->set_addrmap(AS_PROGRAM, &md_base_state::megadriv_z80_map);
	m_z80snd->set_addrmap(AS_IO, &md_base_state::megadriv_z80_io_map);
	// IRQ handled via the timers

	// I/O port controllers
	megadriv_ioports(config);

	m_vdp->snd_irq().set(FUNC(md_base_state::vdp_sndirqline_callback_genesis_z80));
	m_vdp->add_route(ALL_OUTPUTS, "speaker", 0.50, 0);
	m_vdp->add_route(ALL_OUTPUTS, "speaker", 0.50, 1);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2612(config, m_ymsnd, MASTER_CLOCK_NTSC / 7); // 7.67 MHz
	m_ymsnd->add_route(0, "speaker", 0.50, 0);
	m_ymsnd->add_route(1, "speaker", 0.50, 1);
}

void md_base_state::md2_ntsc(machine_config &config)
{
	md_ntsc(config);

	// Internalized YM3438 in VDP ASIC
	YM3438(config.replace(), m_ymsnd, MASTER_CLOCK_NTSC / 7); // 7.67 MHz
	m_ymsnd->add_route(0, "speaker", 0.50, 0);
	m_ymsnd->add_route(1, "speaker", 0.50, 1);
}

/************ PAL hardware has a different master clock *************/

void md_base_state::md_pal(machine_config &config)
{
	md_core_pal(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &md_base_state::megadriv_68k_map);

	Z80(config, m_z80snd, MASTER_CLOCK_PAL / 15); // 3.58 MHz
	m_z80snd->set_addrmap(AS_PROGRAM, &md_base_state::megadriv_z80_map);
	m_z80snd->set_addrmap(AS_IO, &md_base_state::megadriv_z80_io_map);
	// IRQ handled via the timers

	// I/O port controllers
	megadriv_ioports(config);

	m_vdp->snd_irq().set(FUNC(md_base_state::vdp_sndirqline_callback_genesis_z80));
	m_vdp->add_route(ALL_OUTPUTS, "speaker", 0.50, 0);
	m_vdp->add_route(ALL_OUTPUTS, "speaker", 0.50, 1);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2612(config, m_ymsnd, MASTER_CLOCK_PAL / 7); // 7.67 MHz
	m_ymsnd->add_route(0, "speaker", 0.50, 0);
	m_ymsnd->add_route(1, "speaker", 0.50, 1);
}

void md_base_state::md2_pal(machine_config &config)
{
	md_pal(config);

	// Internalized YM3438 in VDP ASIC
	YM3438(config.replace(), m_ymsnd, MASTER_CLOCK_PAL / 7); /* 7.67 MHz */
	m_ymsnd->add_route(0, "speaker", 0.50, 0);
	m_ymsnd->add_route(1, "speaker", 0.50, 1);
}


void md_core_state::megadriv_tas_callback(offs_t offset, uint8_t data)
{
	// writeback not allowed
}

void md_base_state::megadriv_init_common()
{
	// This system has the standard Sound Z80
	m_genz80.z80_prgram = std::make_unique<uint8_t[]>(0x2000);
	membank("bank1")->set_base(m_genz80.z80_prgram.get());
	save_item(NAME(m_genz80.z80_is_reset));
	save_item(NAME(m_genz80.z80_has_bus));
	save_item(NAME(m_genz80.z80_bank_addr));
	save_pointer(NAME(m_genz80.z80_prgram), 0x2000);

	m_maincpu->set_tas_write_callback(*this, FUNC(md_base_state::megadriv_tas_callback));
}

void md_base_state::init_megadriv()
{
	megadriv_init_common();

	// TODO: move this to the device interface?
	m_vdp->set_use_cram(1);
	m_vdp->set_vdp_pal(false);
	m_vdp->set_framerate(60);
	m_vdp->set_total_scanlines(262);

	m_version_hi_nibble = 0xa0; // Export NTSC no-SCD
}

void md_base_state::init_megadrij()
{
	megadriv_init_common();

	// TODO: move this to the device interface?
	m_vdp->set_use_cram(1);
	m_vdp->set_vdp_pal(false);
	m_vdp->set_framerate(60);
	m_vdp->set_total_scanlines(262);

	m_version_hi_nibble = 0x20; // JPN NTSC no-SCD
}

void md_base_state::init_megadrie()
{
	megadriv_init_common();

	// TODO: move this to the device interface?
	m_vdp->set_use_cram(1);
	m_vdp->set_vdp_pal(true);
	m_vdp->set_framerate(50);
	m_vdp->set_total_scanlines(313);

	m_version_hi_nibble = 0xe0; // Export PAL no-SCD
}

void md_core_state::screen_vblank_megadriv(int state)
{
	if (m_io_reset.read_safe(0) & 0x01)
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

	// rising edge
	if (state)
	{
		if (!m_vdp->m_use_alt_timing)
		{
			m_vdp->vdp_handle_eof();
			m_vdp->m_megadriv_scanline_timer->adjust(attotime::zero);
		}
	}
}
