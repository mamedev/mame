// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************************************

  PINBALL
  Gottlieb System 80B

  Same as system 80, except that the displays are 20-digit alphanumeric driven by Rockwell 10939/10941 chips.

  Nothing works.

  PinMAME used for the display character generator.

*****************************************************************************************************************/

#include "machine/genpin.h"
#include "audio/gottlieb.h"
#include "cpu/i86/i86.h"
#include "gts80b.lh"

class gts80b_state : public genpin_class
{
public:
	gts80b_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_r0_sound(*this, "r0sound")
		, m_r1_sound(*this, "r1sound")
	{ }

	DECLARE_DRIVER_INIT(gts80b);
	DECLARE_READ8_MEMBER(port1a_r);
	DECLARE_READ8_MEMBER(port2a_r);
	DECLARE_WRITE8_MEMBER(port1b_w);
	DECLARE_WRITE8_MEMBER(port2a_w);
	DECLARE_WRITE8_MEMBER(port2b_w);
	DECLARE_WRITE8_MEMBER(port3a_w);
	DECLARE_WRITE8_MEMBER(port3b_w);
private:
	UINT8 m_dispcmd;
	UINT8 m_port2a;
	UINT8 m_port2b;
	UINT8 m_lamprow;
	UINT8 m_swrow;
	bool m_in_cmd_mode[2];
	UINT8 m_digit[2];
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	optional_device<gottlieb_sound_r0_device> m_r0_sound;
	optional_device<gottlieb_sound_r1_device> m_r1_sound;
};

static ADDRESS_MAP_START( gts80b_map, AS_PROGRAM, 8, gts80b_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x017f) AM_RAM
	AM_RANGE(0x0200, 0x027f) AM_DEVREADWRITE("riot1", riot6532_device, read, write)
	AM_RANGE(0x0280, 0x02ff) AM_DEVREADWRITE("riot2", riot6532_device, read, write)
	AM_RANGE(0x0300, 0x037f) AM_DEVREADWRITE("riot3", riot6532_device, read, write)
	AM_RANGE(0x1000, 0x17ff) AM_ROM
	AM_RANGE(0x1800, 0x18ff) AM_RAM AM_SHARE("nvram") // 5101L-1 256x4
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x3fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( gts80b )
	PORT_START("DSW.0")
	PORT_DIPNAME( 0x80, 0x00, "SW 1")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x80, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x00, "SW 2")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x40, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x00, "SW 3")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x20, DEF_STR(On))
	PORT_DIPNAME( 0x10, 0x00, "SW 4")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x10, DEF_STR(On))
	PORT_DIPNAME( 0x08, 0x00, "SW 5")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x08, DEF_STR(On))
	PORT_DIPNAME( 0x04, 0x00, "SW 6")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x04, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x00, "SW 7")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x02, DEF_STR(On))
	PORT_DIPNAME( 0x01, 0x00, "SW 8")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x01, DEF_STR(On))

	PORT_START("DSW.1")
	PORT_DIPNAME( 0x80, 0x00, "SW 9")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x80, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x00, "SW 10")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x40, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x00, "SW 11")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x20, DEF_STR(On))
	PORT_DIPNAME( 0x10, 0x00, "SW 12")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x10, DEF_STR(On))
	PORT_DIPNAME( 0x08, 0x00, "SW 13")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x08, DEF_STR(On))
	PORT_DIPNAME( 0x04, 0x00, "SW 14")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x04, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x02, "SW 15")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x02, DEF_STR(On))
	PORT_DIPNAME( 0x01, 0x00, "SW 16")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x01, DEF_STR(On))

	PORT_START("DSW.2")
	PORT_DIPNAME( 0x80, 0x80, "SW 17")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x80, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, "SW 18")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x40, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x00, "SW 19")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x20, DEF_STR(On))
	PORT_DIPNAME( 0x10, 0x00, "SW 20")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x10, DEF_STR(On))
	PORT_DIPNAME( 0x08, 0x00, "SW 21")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x08, DEF_STR(On))
	PORT_DIPNAME( 0x04, 0x00, "SW 22")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x04, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x02, "SW 23")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x02, DEF_STR(On))
	PORT_DIPNAME( 0x01, 0x01, "SW 24")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x01, DEF_STR(On))

	PORT_START("DSW.3")
	PORT_DIPNAME( 0x80, 0x80, "SW 25")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x80, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, "SW 26")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x40, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x20, "SW 27")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x20, DEF_STR(On))
	PORT_DIPNAME( 0x10, 0x10, "SW 28")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x10, DEF_STR(On))
	PORT_DIPNAME( 0x08, 0x08, "SW 29")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x08, DEF_STR(On))
	PORT_DIPNAME( 0x04, 0x04, "SW 30")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x04, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x00, "SW 31")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x02, DEF_STR(On))
	PORT_DIPNAME( 0x01, 0x00, "SW 32")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x01, DEF_STR(On))

	PORT_START("X0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN3 )

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START )

	PORT_START("X20")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT ) // won't boot if closed

	PORT_START("X40")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_X)

	PORT_START("X80")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)
INPUT_PORTS_END

static const UINT16 patterns[] = {
	/* 0x00-0x07 */ 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x08-0x0f */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x10-0x17 */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x18-0x1f */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x20-0x27 */ 0x0000, 0x0309, 0x0220, 0x2A4E, 0x2A6D, 0x6E65, 0x135D, 0x0400,
	/* 0x28-0x2f */ 0x1400, 0x4100, 0x7F40, 0x2A40, 0x0000, 0x0840, 0x0000, 0x4400,
	/* 0x30-0x37 */ 0x003f, 0x2200, 0x085B, 0x084f, 0x0866, 0x086D, 0x087D, 0x0007,
	/* 0x38-0x3f */ 0x087F, 0x086F, 0x0009, 0x4001, 0x4408, 0x0848, 0x1108, 0x2803,
	/* 0x40-0x47 */ 0x205F, 0x0877, 0x2A0F, 0x0039, 0x220F, 0x0079, 0x0071, 0x083D,
	/* 0x48-0x4f */ 0x0876, 0x2209, 0x001E, 0x1470, 0x0038, 0x0536, 0x1136, 0x003f,
	/* 0x50-0x57 */ 0x0873, 0x103F, 0x1873, 0x086D, 0x2201, 0x003E, 0x4430, 0x5036,
	/* 0x58-0x5f */ 0x5500, 0x2500, 0x4409, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x60-0x67 */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x68-0x6f */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x70-0x77 */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	/* 0x78-0x7f */ 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
};

READ8_MEMBER( gts80b_state::port1a_r )
{
	char kbdrow[8];
	UINT8 data = 0;
	if ((m_lamprow < 4) && (m_port2b==0x80))
	{
		sprintf(kbdrow,"DSW.%d",m_lamprow);
		data = ioport(kbdrow)->read();
	}
	else
	{
		sprintf(kbdrow,"X%X",m_swrow);
		data = ioport(kbdrow)->read();
	}

	return data;
}

READ8_MEMBER( gts80b_state::port2a_r )
{
	return m_port2a | 0x80; // slam tilt off
}

// sw strobes
WRITE8_MEMBER( gts80b_state::port1b_w )
{
	m_swrow = data;
}

WRITE8_MEMBER( gts80b_state::port2a_w )
{
	m_port2a = data;
	if BIT(data, 4)
		m_dispcmd = (m_dispcmd & 0xf0) | m_port2b;
	if BIT(data, 5)
		m_dispcmd = (m_dispcmd & 0x0f) | (m_port2b << 4);
}

//d0-3 data; d4-5 = which display enabled; d6 = display reset; d7 = dipsw enable
WRITE8_MEMBER( gts80b_state::port2b_w )
{
	m_port2b = data & 15;
	UINT16 segment;

	// crude approximation of the Rockwell display chips
	for (UINT8 i = 0; i < 2; i++) // 2 chips
	{
		if (!BIT(data, i+4)) // are we addressing the chip?
		{
			if (m_in_cmd_mode[i]) // in command mode?
			{
				if ((m_dispcmd >= 0xc0) && (m_dispcmd < 0xd4)) // we only support one command
					m_digit[i] = data & 0x1f;
				m_in_cmd_mode[i] = false;
			}
			else
			if (m_dispcmd == 1) // 01 = enter command mode
			{
				m_in_cmd_mode[i] = true;
			}
			else
			{ // display a character
				segment = patterns[m_dispcmd & 0x7f]; // ignore blank/inverse bit
				segment = BITSWAP16(segment, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 3, 2, 1, 0, 0);
				output().set_digit_value(m_digit[i]+i*20, segment);
				m_digit[i]++; // auto-increment pointer
				if (m_digit[i] > 19) m_digit[i] = 0; // check for overflow
			}
		}
	}
}

// solenoids
WRITE8_MEMBER( gts80b_state::port3a_w )
{
}

//pb0-3 = sound; pb4-7 = lamprow
WRITE8_MEMBER( gts80b_state::port3b_w )
{
	UINT8 sndcmd = data & 15;
	m_lamprow = data >> 4;
	if (m_r0_sound)
		m_r0_sound->write(space, offset, sndcmd);
	if (m_r1_sound)
		m_r1_sound->write(space, offset, sndcmd);
}

void gts80b_state::machine_reset()
{
	m_in_cmd_mode[0] = false;
	m_in_cmd_mode[1] = false;
}

DRIVER_INIT_MEMBER( gts80b_state, gts80b )
{
}

/* with Sound Board */
static MACHINE_CONFIG_START( gts80b, gts80b_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, XTAL_3_579545MHz/4)
	MCFG_CPU_PROGRAM_MAP(gts80b_map)

	MCFG_NVRAM_ADD_1FILL("nvram") // must be 1

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_gts80b)

	/* Devices */
	MCFG_DEVICE_ADD("riot1", RIOT6532, XTAL_3_579545MHz/4)
	MCFG_RIOT6532_IN_PA_CB(READ8(gts80b_state, port1a_r)) // sw_r
	//MCFG_RIOT6532_OUT_PA_CB(WRITE8(gts80b_state, port1a_w))
	//MCFG_RIOT6532_IN_PB_CB(READ8(gts80b_state, port1b_r))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(gts80b_state, port1b_w)) // sw_w
	MCFG_RIOT6532_IRQ_CB(INPUTLINE("maincpu", M6502_IRQ_LINE))
	MCFG_DEVICE_ADD("riot2", RIOT6532, XTAL_3_579545MHz/4)
	MCFG_RIOT6532_IN_PA_CB(READ8(gts80b_state, port2a_r)) // pa7 - slam tilt
	MCFG_RIOT6532_OUT_PA_CB(WRITE8(gts80b_state, port2a_w)) // digit select
	//MCFG_RIOT6532_IN_PB_CB(READ8(gts80b_state, port2b_r))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(gts80b_state, port2b_w)) // seg
	MCFG_RIOT6532_IRQ_CB(INPUTLINE("maincpu", M6502_IRQ_LINE))
	MCFG_DEVICE_ADD("riot3", RIOT6532, XTAL_3_579545MHz/4)
	//MCFG_RIOT6532_IN_PA_CB(READ8(gts80b_state, port3a_r))
	MCFG_RIOT6532_OUT_PA_CB(WRITE8(gts80b_state, port3a_w)) // sol, snd
	//MCFG_RIOT6532_IN_PB_CB(READ8(gts80b_state, port3b_r))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(gts80b_state, port3b_w)) // lamps
	MCFG_RIOT6532_IRQ_CB(INPUTLINE("maincpu", M6502_IRQ_LINE))

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gts80b_s, gts80b )
	MCFG_GOTTLIEB_SOUND_R0_ADD("r0sound")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

//static MACHINE_CONFIG_DERIVED( gts80b_ss, gts80b )
//  MCFG_GOTTLIEB_SOUND_R1_ADD("r1sound")
//  //MCFG_GOTTLIEB_SOUND_R1_ADD_VOTRAX("r1sound")  // votrax crashes
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
//MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gts80b_s1, gts80b )

	/* related to src/mame/audio/gottlieb.c? */
//  MCFG_IMPORT_FROM(gts80s_b1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gts80b_s2, gts80b )

	/* related to src/mame/audio/gottlieb.c? */
//  MCFG_IMPORT_FROM(gts80s_b2)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gts80b_s3, gts80b )

	/* related to src/mame/audio/gottlieb.c? */
//  MCFG_IMPORT_FROM(gts80s_b3)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bonebstr, gts80b )

	/* related to src/mame/audio/gottlieb.c? */
//  MCFG_IMPORT_FROM(gts80s_b3a)
MACHINE_CONFIG_END


/*-------------------------------------------------------------------
/ Ace High (#700) 1985 (Prototype)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Amazon Hunt II 05/85
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Arena (#709)
/-------------------------------------------------------------------*/
ROM_START(arena)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(4783b689) SHA1(d10d4cbf8d00c9d0db57cdac32ef96498275eea6))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(8c9f8ee9) SHA1(840505d08e387c3f7de105305e183f8ed3a6d5c6))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(78e6cbf1) SHA1(7b66a0cb211a93cf475172aa0465a952009e1a59))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(f7a951c2) SHA1(12d7a6119d9033ae02c6312c9af888bfc7c63ad1))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(cc2aef4e) SHA1(a6e243de99f6a76eb527e879f4441c036dd379b6))
ROM_END

/*-------------------------------------------------------------------
/ Bad Girls (#717)
/-------------------------------------------------------------------*/
ROM_START(badgirls)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(583933ec) SHA1(89da6750d779d68db578715b058f9321695b79b0))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(956aeae0) SHA1(24d9d514fc83aba1ab310bfe4ed80605df399417))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(452dec20) SHA1(a9c41dfb2d83c5671ab96e946f13df774b567976))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(ab3b8e2d) SHA1(b57a0b804b42b923bb102d295e3b8a69b1033d27))
ROM_END

/*-------------------------------------------------------------------
/ Big House (#713)
/-------------------------------------------------------------------*/
ROM_START(bighouse)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(047c8ef5) SHA1(3afa2a0011b724836b69b2ef386597e0953dfadf))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(0ecef900) SHA1(78e4ed6e40fdb45dde2d0f2cf60d4c8a7ea2e39e))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(f330fd04) SHA1(1288c47f636d9d5b826a2b870b81788a630e489e))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(0b1ba1cb) SHA1(26327689992018837b1c9957c515ab67248623eb))
ROM_END

/*-------------------------------------------------------------------
/ Bone Busters Inc. (#719)
/-------------------------------------------------------------------*/
ROM_START(bonebstr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(681643df) SHA1(76af6951e4403b4951298d35a9058bcebfa6bc43))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(052f97be) SHA1(0ee108e79c4196dffedc64d7f7a576e0394427c1))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom2.snd", 0x8000, 0x8000, CRC(d147d78d) SHA1(f8f6d6a1921685b883b224a9ea85ead52a32a4c3))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(ec43f4e9) SHA1(77b0988700be7a597dca7e5f06ac5d3c6834ce21))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(a95eedfc) SHA1(5ced2d6869a9895f8ff26d830b21d3c9364b32e7))
ROM_END

ROM_START(bonebstrf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2f.cpu", 0x1000, 0x0800, CRC(73b6486e) SHA1(1baf17f31b16d564ed5e3bdf9f74b21f83ed76fa))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1f.cpu", 0x2000, 0x2000, CRC(3d334065) SHA1(6d44819cf84bee375a9f62351b00375404f6d3e3))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu4", 0)
	ROM_LOAD("drom2.snd", 0x8000, 0x8000, CRC(d147d78d) SHA1(f8f6d6a1921685b883b224a9ea85ead52a32a4c3))

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(ec43f4e9) SHA1(77b0988700be7a597dca7e5f06ac5d3c6834ce21))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(a95eedfc) SHA1(5ced2d6869a9895f8ff26d830b21d3c9364b32e7))
ROM_END

/*-------------------------------------------------------------------
/ Bounty Hunter (#694)
/-------------------------------------------------------------------*/
ROM_START(bountyh)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(e8190df7) SHA1(5304918d35e379da17ab19d8879a7ace5c864326))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("694-s.snd", 0x0800, 0x0800, CRC(a0383e41) SHA1(156514d2b52fcd89b608b85991c5066780949979))
ROM_END

/*-------------------------------------------------------------------
/ Chicago Cubs' Triple Play (#696)
/-------------------------------------------------------------------*/
ROM_START(triplay)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(42b29b01) SHA1(58145ce10939d00faff49972ada669005a223792))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("696-s.snd", 0x0800, 0x0800, CRC(deedea61) SHA1(6aec221397f250d5dd99faefa313e8028c8818f7))
ROM_END

/*-------------------------------------------------------------------
/ Diamond Lady (#711)
/-------------------------------------------------------------------*/
ROM_START(diamondp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(862951dc) SHA1(b15899ecf7ec869e3722cef3f5c16b0dadd2514e))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(7a011757) SHA1(cc49ec7451feae035670ea9d70cc8f6b32747c90))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(c216d1e4) SHA1(aa38db5ad36d1d1d35e727ab27c1f1c05a9627cd))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(0a18d626) SHA1(6b367668be55ca04c69c4c4c5a4a524ae8f790f8))
ROM_END

/*-------------------------------------------------------------------
/ Excalibur (#715)
/-------------------------------------------------------------------*/
ROM_START(excalibr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(499e2e41) SHA1(1e3fcba18882bd7df30a43843916aa5d7968eecc))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(ed1083d7) SHA1(3ff829ecfaba7d20c75268d3ee5224cb3cac3507))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(a4368cd0) SHA1(c48513e56899938dc83a3545d8ee9def3dc1491f))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(9f194744) SHA1(dbd73b546071c3d4f0dcfe21e3e646da716c5b71))
ROM_END

/*-------------------------------------------------------------------
/ Genesis (#705)
/-------------------------------------------------------------------*/
ROM_START(genesisp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(ac9f3a0f) SHA1(0e44888dc046121794e824d128628f991245c1cb))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(4a2f185c) SHA1(b45982b1ce9777292731ad523516c76cde4ddfa4))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(758e1743) SHA1(6df3011c044796afcd88e52d1ca69692cb489ff4))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(4869b0ec) SHA1(b8a56753257205af56e06105515b8a700bb1935b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0528c024) SHA1(d24ff7e088b08c1f35b54be3c806f8a8757d96c7))
ROM_END

/*-------------------------------------------------------------------
/ Gold Wings (#707)
/-------------------------------------------------------------------*/
ROM_START(goldwing)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(a5318c20) SHA1(8b4dcf45b13657ff753237a2e7d0352fda7755ef))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(bf242185) SHA1(0bf231050aa29f8bba5cb478a815b3d83bad93b3))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(892dbb21) SHA1(e24611544693e95dd2b9c0f2532c4d1f0b8ac10c))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(e17e9b1f) SHA1(ada9a6139a13ef31173801d560ec732d5a285140))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(4e482023) SHA1(62e97d229eb28ff67f0ebc4ee04c1b4918a4affe))
ROM_END

/*-------------------------------------------------------------------
/ Hollywood Heat (#703)
/-------------------------------------------------------------------*/
ROM_START(hlywoodh)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(a465e5f3) SHA1(56afa2f67aebcd17345bba76ecb814653719ee7b))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(0493e27a) SHA1(72c603cda3cc43ed0f841a9fcc6f40d020475e74))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(a698ec33) SHA1(e7c1d28279ec4f12095c3a106c6cefcc2a84b31e))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(9232591e) SHA1(72883e0c542c572226c6c654bea14749cc9e351f))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(51709c2f) SHA1(5834d7b72bd36e30c87377dc7c3ad0cf26ff303a))
ROM_END

/*-------------------------------------------------------------------
/ Hot Shots (#718)
/-------------------------------------------------------------------*/
ROM_START(hotshots)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(7695c7db) SHA1(90188ff83b888262ba849e5af9d99145c5bc1c30))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(122ff4a8) SHA1(195392b9f2050b52392a123831bb7a9428087c1b))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(42c3cc3d) SHA1(26ca7f3a71b83df18ac6be1d1eb28da20120285e))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(2933a80e) SHA1(5982b9ed361d90f8ea47047fc29770ef142acbec))
ROM_END

/*-------------------------------------------------------------------
/ Monte Carlo (#708)
/-------------------------------------------------------------------*/
ROM_START(mntecrlo)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(6860e315) SHA1(cecb1815334506dfebf29efe3e4e2a838010e8db))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(0fbf15a3) SHA1(0155b39c2c38224301857313ab784c1d39f1183b))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(1a53ac15) SHA1(f2751664a09431e908873580ddf4f44df9b4eda7))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(6e234c49) SHA1(fdb4126ecdaac378d144e9dd3c29b4e79290da2a))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(a95d1a6b) SHA1(91946ef7af0e4dd96db6d2d6f4f2e9a3a7279b81))
ROM_END

/*-------------------------------------------------------------------
/ Night Moves C-103
/-------------------------------------------------------------------*/
ROM_START(nmoves)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("nmovsp2.732", 0x1000, 0x0800, CRC(a2bc00e4) SHA1(5c3e9033f5c72b87058b2f70a0ff0811cc6770fa))
	ROM_CONTINUE(0x9000, 0x0800)
	ROM_RELOAD(0x5000, 0x0800)
	ROM_CONTINUE(0xd000, 0x0800)
	ROM_LOAD("nmovsp1.764", 0x2000, 0x2000, CRC(36837146) SHA1(88312ae1d1fe76defc4aa2d0a0570c5bb56253e9))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("nmovdrom.256", 0x8000, 0x8000, CRC(90929841) SHA1(e203ccd3552c9843c91fc49a437f60ae2dd49142))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("nmovyrom.256", 0x8000, 0x8000, CRC(cb74a687) SHA1(af8275807491eb35643cdeb6c898025fde47ceac))
ROM_END

/*-------------------------------------------------------------------
/ Raven (#702)
/-------------------------------------------------------------------*/
ROM_START(raven)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(481f3fb8) SHA1(22ffa55ed362219ebedbc40edcf866ff152a01b9))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(edc88561) SHA1(101878527307c6f04d141dd74e04102c4ea53105))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(a04bf7d0) SHA1(5be5d445b199e7dc9d42e7ee5e9b31c18dec3881))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(ee5f868b) SHA1(23ef4112b94109ad4d4a6b9bb5215acec20e5e55))
ROM_END

/*-------------------------------------------------------------------
/ Robo-War (#714)
/-------------------------------------------------------------------*/
ROM_START(robowars)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(893177ed) SHA1(791540a64d498979e5b0c8baf4ceb2fd5ff7f047))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(cd1587d8) SHA1(77e8e02dc03d052e9e4ce19c9431439e4211a29f))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(ea59b6a1) SHA1(6a4cdd37ba85f94f703afd1c5d3f102f51fedf46))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(7ecd8b67) SHA1(c5167b0acc64e535d389ba70be92a65672e119f6))
ROM_END

/*-------------------------------------------------------------------
/ Rock (#697)
/-------------------------------------------------------------------*/
ROM_START(rock)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(1146c1d3) SHA1(1e838756017cdc51239c082f8d491cd2824d273d))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(03830e81) SHA1(786f85eba5a8f5e9cc659305623e1d178b5410f6))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(effba2ad) SHA1(2288a4f655376e0aa18f8ecd9a3818ed4d6c6891))
ROM_END

/*-------------------------------------------------------------------
/ Rock Encore (#704)
/-------------------------------------------------------------------*/
ROM_START(rock_enc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(1146c1d3) SHA1(1e838756017cdc51239c082f8d491cd2824d273d))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1a.snd",0xe000,0x2000, CRC(b8aa8912) SHA1(abff690256c0030807b2d4dfa0516496516384e8))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1a.snd",0xe000,0x2000, CRC(a62e3b94) SHA1(59636c2ac7ebbd116a0eb39479c97299ba391906))
	ROM_LOAD("yrom2a.snd",0xc000,0x2000, CRC(66645a3f) SHA1(f06261af81e6b1829d639933297d2461a8c993fc))
ROM_END

/*-------------------------------------------------------------------
/ Spring Break (#706)
/-------------------------------------------------------------------*/
ROM_START(sprbreak)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(47171062) SHA1(0d2e7777f695ab22170be861019c05ddeade5f85))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(53ed608b) SHA1(555a6c02d637ea03e8265bb2b0fba95f2e2584b3))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(97d3f9ba) SHA1(1b34c7e51373c26d29d757c57a2b0333fe38d19e))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(5ea89df9) SHA1(98ce7661a4d862fd02c77e69b0f6e9372c3ade2b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0fb0128e) SHA1(3bdc5ed11b8e062f71f2a78b955830bd985e80a3))
ROM_END

ROM_START(sprbreaks)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.rv2", 0x1000, 0x0800, CRC(911cd14f) SHA1(2bc3ff6a3889da69b97f8ec318f93208e3d42cfe))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.rv2", 0x2000, 0x2000, CRC(d67d9d2f) SHA1(ebb82f0a1b7d6a2ec2607d4000e58fb6bfa73fe7))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd",0xe000,0x2000, CRC(97d3f9ba) SHA1(1b34c7e51373c26d29d757c57a2b0333fe38d19e))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd",0xe000,0x2000, CRC(5ea89df9) SHA1(98ce7661a4d862fd02c77e69b0f6e9372c3ade2b))
	ROM_LOAD("yrom2.snd",0xc000,0x2000, CRC(0fb0128e) SHA1(3bdc5ed11b8e062f71f2a78b955830bd985e80a3))
ROM_END

/*-------------------------------------------------------------------
/ Tag-Team Wrestling (#698)
/-------------------------------------------------------------------*/
ROM_START(tagteamp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(fd1615ce) SHA1(3a6c3525552286b86e5340af2bf196f12adc9b35))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(65931038) SHA1(6d2f1a9fb1b3ce4610074fd3f2ac37ad6af70a44))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("698-s.snd", 0x0800, 0x0800, CRC(9c8191b7) SHA1(12b017692f078dcdc8e4bbf1ffcea1c5d0293d06))
ROM_END

ROM_START(tagteamp2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2a.cpu", 0x1000, 0x0800, CRC(6d56b636) SHA1(8f50f2742be727835e7343307787b4b5daa1623a))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1a.cpu", 0x2000, 0x2000, CRC(92766607) SHA1(29744dd3c447cc51fb123750ae1456329122e986))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("698-s.snd", 0x0800, 0x0800, CRC(9c8191b7) SHA1(12b017692f078dcdc8e4bbf1ffcea1c5d0293d06))
ROM_END

/*-------------------------------------------------------------------
/ TX-Sector (#712)
/-------------------------------------------------------------------*/
ROM_START(txsector)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(f12514e6) SHA1(80bca17c33df99ed1a7acc21f7f70ea90e7c0463))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(e51d39da) SHA1(b6e4d573b62cc441a153cc4d8b647ee46b4dd2a7))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(61d66ca1) SHA1(59b1705b13d46b29f45257c566274f3cdce15ec2))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(469ef444) SHA1(faa16f34357a53c3fc61b59251fabdc44c605000))
ROM_END

/*-------------------------------------------------------------------
/ Victory (#710)
/-------------------------------------------------------------------*/
ROM_START(victoryp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("prom2.cpu", 0x1000, 0x0800, CRC(6a42eaf4) SHA1(3e28b01473266db463986a4283e1be85f2410fb1))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_LOAD("prom1.cpu", 0x2000, 0x2000, CRC(e724db90) SHA1(10e760e129ce89f11372c6dd3616216d45f2c926))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x10000, "cpu3", 0)
	ROM_LOAD("drom1.snd", 0x8000, 0x8000, CRC(4ab6dab7) SHA1(7e21e69029e60052112ddd5c7481582ea6684dc1))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("yrom1.snd", 0x8000, 0x8000, CRC(921a100e) SHA1(0c3c7eae4ceeb5a1a8150bac52203d3f1e8f917e))
ROM_END


GAME(1985, bountyh,   0,        gts80b_s,  gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Bounty Hunter",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985, triplay,   0,        gts80b_s,  gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Triple Play",          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985, rock,      0,        gts80b_s1, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Rock",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985, tagteamp,  0,        gts80b_s,  gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Tag-Team Wrestling",   MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985, tagteamp2, tagteamp, gts80b_s,  gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Tag-Team Wrestling (rev.2)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986, raven,     0,        gts80b_s1, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Raven",                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986, hlywoodh,  0,        gts80b_s1, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Hollywood Heat",       MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986, rock_enc,  rock,     gts80b_s1, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Rock Encore",          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986, genesisp,  0,        gts80b_s1, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Genesis",              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987, sprbreak,  0,        gts80b_s1, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Spring Break",         MACHINE_IS_SKELETON_MECHANICAL)
GAME(19??, sprbreaks, sprbreak, gts80b_s1, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Spring Break (single ball game)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986, goldwing,  0,        gts80b_s1, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Gold Wings",           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987, mntecrlo,  0,        gts80b_s1, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Monte Carlo (Pinball)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987, arena,     0,        gts80b_s1, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Arena",                MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987, victoryp,  0,        gts80b_s2, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Victory (Pinball)",    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1988, diamondp,  0,        gts80b_s2, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Diamond Lady",         MACHINE_IS_SKELETON_MECHANICAL)
GAME(1988, txsector,  0,        gts80b_s2, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "TX-Sector",            MACHINE_IS_SKELETON_MECHANICAL)
GAME(1989, bighouse,  0,        gts80b_s3, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Big House",            MACHINE_IS_SKELETON_MECHANICAL)
GAME(1988, robowars,  0,        gts80b_s2, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Robo-War",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1988, excalibr,  0,        gts80b_s3, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Excalibur",            MACHINE_IS_SKELETON_MECHANICAL)
GAME(1988, badgirls,  0,        gts80b_s3, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Bad Girls",            MACHINE_IS_SKELETON_MECHANICAL)
GAME(1989, hotshots,  0,        gts80b_s2, gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Hot Shots",            MACHINE_IS_SKELETON_MECHANICAL)
GAME(1989, bonebstr,  0,        bonebstr,  gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Bone Busters Inc.",    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1989, bonebstrf, bonebstr, bonebstr,  gts80b, gts80b_state, gts80b, ROT0, "Gottlieb",               "Bone Busters Inc. (French)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1989, nmoves,    0,        gts80b_s2, gts80b, gts80b_state, gts80b, ROT0, "International Concepts", "Night Moves",          MACHINE_IS_SKELETON_MECHANICAL)
