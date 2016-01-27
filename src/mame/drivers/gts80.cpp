// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************************************

  PINBALL
  Gottlieb System 80


Status:
- Games boot up, score alternates between 0 and high score (currently blank as there is no high score yet)
- Dips are read and stored to ram
- Switches are not being read, therefore unable to play, or enter setup/test menu
- Original sound card added, it continuously produces weird noises
- 2nd version of sound card not coded yet


ToDO:
- Switches
- Outputs
- Mechanical sounds
- Sound


************************************************************************************************************/


#include "machine/genpin.h"
#include "audio/gottlieb.h"
#include "gts80.lh"

class gts80_state : public genpin_class
{
public:
	gts80_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_r0_sound(*this, "r0sound")
		, m_r1_sound(*this, "r1sound")
	{ }

	DECLARE_DRIVER_INIT(gts80);
	DECLARE_READ8_MEMBER(port1a_r);
	DECLARE_READ8_MEMBER(port2a_r);
	DECLARE_WRITE8_MEMBER(port1b_w);
	DECLARE_WRITE8_MEMBER(port2a_w);
	DECLARE_WRITE8_MEMBER(port2b_w);
	DECLARE_WRITE8_MEMBER(port3a_w);
	DECLARE_WRITE8_MEMBER(port3b_w);
private:
	UINT8 m_port2;
	UINT8 m_segment;
	UINT8 m_lamprow;
	UINT8 m_swrow;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	optional_device<gottlieb_sound_r0_device> m_r0_sound;
	optional_device<gottlieb_sound_r1_device> m_r1_sound;
};

static ADDRESS_MAP_START( gts80_map, AS_PROGRAM, 8, gts80_state )
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

static INPUT_PORTS_START( gts80 )
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

READ8_MEMBER( gts80_state::port1a_r )
{
	char kbdrow[8];
	UINT8 data = 0;
	if ((m_lamprow < 4) && (m_segment==0x80))
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

READ8_MEMBER( gts80_state::port2a_r )
{
	return m_port2 | 0x80; // slam tilt off
}

// sw strobes
WRITE8_MEMBER( gts80_state::port1b_w )
{
	m_swrow = data;
}

// schematic and pinmame say '1' is indicated by m_segment !bits 4,5,6, but it is !bit 7
WRITE8_MEMBER( gts80_state::port2a_w )
{
	m_port2 = data;
	static const UINT8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0 }; // 7448
	UINT16 seg1 = (UINT16)patterns[m_segment & 15];
	UINT16 seg2 = BITSWAP16(seg1, 8, 8, 8, 8, 8, 8, 7, 7, 6, 6, 5, 4, 3, 2, 1, 0);
	switch (data & 0x70)
	{
		case 0x10: // player 1&2
			if (!BIT(m_segment, 7)) seg2 |= 0x300; // put '1' in the middle
			output().set_digit_value(data & 15, seg2);
			break;
		case 0x20: // player 3&4
			if (!BIT(m_segment, 7)) seg2 |= 0x300; // put '1' in the middle
			output().set_digit_value((data & 15)+20, seg2);
			break;
		case 0x40: // credits & balls
			if (!BIT(m_segment, 7)) m_segment = 1; // turn '1' back to normal
			output().set_digit_value((data & 15)+40, patterns[m_segment & 15]);
			break;
	}
}

//d0-3 bcd data; d4-6 = centre segment; d7 = dipsw enable
WRITE8_MEMBER( gts80_state::port2b_w )
{
	m_segment = data;//printf("%s:%X ",machine().describe_context(),data);
}

// solenoids
WRITE8_MEMBER( gts80_state::port3a_w )
{
}

//pb0-3 = sound; pb4-7 = lamprow
WRITE8_MEMBER( gts80_state::port3b_w )
{
	UINT8 sndcmd = data & 15;
	m_lamprow = data >> 4;
	if (m_r0_sound)
		m_r0_sound->write(space, offset, sndcmd);
	if (m_r1_sound)
		m_r1_sound->write(space, offset, sndcmd);
}

void gts80_state::machine_reset()
{
}

DRIVER_INIT_MEMBER( gts80_state, gts80 )
{
}

/* with Sound Board */
static MACHINE_CONFIG_START( gts80, gts80_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, XTAL_3_579545MHz/4)
	MCFG_CPU_PROGRAM_MAP(gts80_map)

	MCFG_NVRAM_ADD_1FILL("nvram") // must be 1

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_gts80)

	/* Devices */
	MCFG_DEVICE_ADD("riot1", RIOT6532, XTAL_3_579545MHz/4)
	MCFG_RIOT6532_IN_PA_CB(READ8(gts80_state, port1a_r)) // sw_r
	//MCFG_RIOT6532_OUT_PA_CB(WRITE8(gts80_state, port1a_w))
	//MCFG_RIOT6532_IN_PB_CB(READ8(gts80_state, port1b_r))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(gts80_state, port1b_w)) // sw_w
	MCFG_RIOT6532_IRQ_CB(INPUTLINE("maincpu", M6502_IRQ_LINE))
	MCFG_DEVICE_ADD("riot2", RIOT6532, XTAL_3_579545MHz/4)
	MCFG_RIOT6532_IN_PA_CB(READ8(gts80_state, port2a_r)) // pa7 - slam tilt
	MCFG_RIOT6532_OUT_PA_CB(WRITE8(gts80_state, port2a_w)) // digit select
	//MCFG_RIOT6532_IN_PB_CB(READ8(gts80_state, port2b_r))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(gts80_state, port2b_w)) // seg
	MCFG_RIOT6532_IRQ_CB(INPUTLINE("maincpu", M6502_IRQ_LINE))
	MCFG_DEVICE_ADD("riot3", RIOT6532, XTAL_3_579545MHz/4)
	//MCFG_RIOT6532_IN_PA_CB(READ8(gts80_state, port3a_r))
	MCFG_RIOT6532_OUT_PA_CB(WRITE8(gts80_state, port3a_w)) // sol, snd
	//MCFG_RIOT6532_IN_PB_CB(READ8(gts80_state, port3b_r))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(gts80_state, port3b_w)) // lamps
	MCFG_RIOT6532_IRQ_CB(INPUTLINE("maincpu", M6502_IRQ_LINE))

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gts80_s, gts80 )
	MCFG_GOTTLIEB_SOUND_R0_ADD("r0sound")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gts80_hh, gts80 )
	MCFG_GOTTLIEB_SOUND_R1_ADD("r1sound")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gts80_ss, gts80 )
	MCFG_GOTTLIEB_SOUND_R1_ADD("r1sound")
	//MCFG_GOTTLIEB_SOUND_R1_ADD_VOTRAX("r1sound")  // votrax crashes
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Black Hole #668
/-------------------------------------------------------------------*/
ROM_START(blckhole)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("668-4.cpu", 0x1000, 0x0800, CRC(01b53045) SHA1(72d73bbb09358b331696cd1cc44fc4958feffbe2))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("668-s1.snd", 0x7000, 0x0800, CRC(23d5045d) SHA1(a20bf02ece97e8238d1dbe8d35ca63d82b62431e))
	ROM_LOAD("668-s2.snd", 0x7800, 0x0800, CRC(d63da498) SHA1(84dd87783f47fbf64b1830284c168501f9b455e2))
ROM_END

ROM_START(blckhole2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("668-2.cpu", 0x1000, 0x0800, CRC(df03ffea) SHA1(7ca8fc321f74b9193104c282c7b4b92af93694c9))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("668-s1.snd", 0x7000, 0x0800, CRC(23d5045d) SHA1(a20bf02ece97e8238d1dbe8d35ca63d82b62431e))
	ROM_LOAD("668-s2.snd", 0x7800, 0x0800, CRC(d63da498) SHA1(84dd87783f47fbf64b1830284c168501f9b455e2))
ROM_END

ROM_START(blckhols)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("668-a2.cpu", 0x1000, 0x0800, CRC(df56f896) SHA1(1ec945a7ed8d25064476791adab2b554371dadbe))

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("668-a-s.snd", 0x0400, 0x0400, CRC(5175f307) SHA1(97be8f2bbc393cc45a07fa43daec4bbba2336af8))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Circus #654
/-------------------------------------------------------------------*/
ROM_START(circusp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("654-1.cpu", 0x1000, 0x0200, CRC(0eeb2731) SHA1(087cd6400bf0775bda0264422b3f790a77852bc4))
	ROM_RELOAD(0x1400, 0x0200)
	ROM_LOAD("654-2.cpu", 0x1200, 0x0200, CRC(01e23569) SHA1(47088421254e487aa1d1e87ea911dc1634e7d9ad))
	ROM_RELOAD(0x1600, 0x0200)

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("654.snd", 0x0400, 0x0400, CRC(75c3ad67) SHA1(4f59c451b8659d964d5242728814c2d97f68445b))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Counterforce #656
/-------------------------------------------------------------------*/
ROM_START(cntforce)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("656-1.cpu", 0x1000, 0x0200, CRC(42baf51d) SHA1(6c7947df6e4d7ed2fd48410705018bde91db3356))
	ROM_RELOAD(0x1400, 0x0200)
	ROM_LOAD("656-2.cpu", 0x1200, 0x0200, CRC(0e185c30) SHA1(01d9fb5d335c24bed9f747d6e23f57adb6ef09a5))
	ROM_RELOAD(0x1600, 0x0200)

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("656.snd", 0x0400, 0x0400, CRC(0be2cbe9) SHA1(306a3e7d93733562360285de35b331b5daae7250))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Critical Mass
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Eclipse #671
/-------------------------------------------------------------------*/
ROM_START(eclipse)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("671-a.cpu", 0x1000, 0x0800, CRC(efad7312) SHA1(fcfd5e5c7924d65ac42561994797156a80018667))

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("671-a-s.snd", 0x0400, 0x0400, CRC(5175f307) SHA1(97be8f2bbc393cc45a07fa43daec4bbba2336af8))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Force II #661
/-------------------------------------------------------------------*/
ROM_START(forceii)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("661-2.cpu", 0x1000, 0x0800, CRC(a4fa42a4) SHA1(c17af4f0da6d5630e43db44655bece0e26b0112a))

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("661.snd", 0x0400, 0x0400, CRC(650158a7) SHA1(c7a9d521d1e7de1e00e7abc3a97aaaee04f8052e))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Haunted House #669, since serial no. 5000
/-------------------------------------------------------------------*/
ROM_START(hh)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("669-2.cpu", 0x1000, 0x0800, CRC(f3085f77) SHA1(ebd43588401a735d9c941d06d67ac90183139e90))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("669-s1.snd", 0x7000, 0x0800, CRC(52ec7335) SHA1(2b08dd8a89057c9c8c184d5b723ecad01572129f))
	ROM_LOAD("669-s2.snd", 0x7800, 0x0800, CRC(a3317b4b) SHA1(c3b14aa58fd4588c8b8fa3540ea6331a9ee40f1f))
ROM_END

ROM_START(hh_1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("669-1.cpu", 0x1000, 0x0800, CRC(96e72b93) SHA1(3eb3d3e064ba2fe637bba2a93ffd07f00edfa0f2))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("669-s1.snd", 0x7000, 0x0800, CRC(52ec7335) SHA1(2b08dd8a89057c9c8c184d5b723ecad01572129f))
	ROM_LOAD("669-s2.snd", 0x7800, 0x0800, CRC(a3317b4b) SHA1(c3b14aa58fd4588c8b8fa3540ea6331a9ee40f1f))
ROM_END

/*-------------------------------------------------------------------
/ James Bond #658
/-------------------------------------------------------------------*/
ROM_START(jamesb)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("658-1.cpu", 0x1000, 0x0800, CRC(b841ad7a) SHA1(3396e82351c975781cac9112bfa341a3b799f296))

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("658.snd", 0x0400, 0x0400, CRC(962c03df) SHA1(e8ff5d502a038531a921380b75c27ef79b6feac8))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

ROM_START(jamesb2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("658-x.cpu", 0x1000, 0x0800, CRC(e7e0febf) SHA1(2c101a88b61229f30ed15d38f395bc538999d766))

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("658.snd", 0x0400, 0x0400, CRC(962c03df) SHA1(e8ff5d502a038531a921380b75c27ef79b6feac8))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Mars - God of War #666
/-------------------------------------------------------------------*/
ROM_START(marsp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("666-1.cpu", 0x1000, 0x0800, CRC(bb7d476a) SHA1(22d5d7f0e52c5180f73a1ca0b3c6bd4b7d0843d6))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("666-s1.snd", 0x7000, 0x0800, CRC(d33dc8a5) SHA1(8d071c392996a74c3cdc2cf5ea3be3c86553ce89))
	ROM_LOAD("666-s2.snd", 0x7800, 0x0800, CRC(e5616f3e) SHA1(a6b5ebd0b456a555db0889cd63ce79aafc64dbe5))
ROM_END

/*-------------------------------------------------------------------
/ Panthera #652
/-------------------------------------------------------------------*/
ROM_START(panthera)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("652.cpu", 0x1000, 0x0800, CRC(5386e5fb) SHA1(822f47951b702f9c6a1ce674baaab0a596f34413))

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("652.snd", 0x0400, 0x0400, CRC(4d0cf2c0) SHA1(0da5d118ffd19b1e78dfaaee3e31c43750d45c8d))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

#ifdef MISSING_GAME // all missing games are using a similar set of ROMs.
ROM_START(panther7)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2g807dc.bin", 0x2000, 0x1000, CRC(f8a687b3) SHA1(ba7747c04a5967df760ace102e47c91d42e07a12))
	ROM_LOAD("u3g807dc.bin", 0x3000, 0x1000, CRC(6e31242e) SHA1(14e371a0352a6068dec20af1f2b344e34a5b9011))
	ROM_LOAD("652.cpu", 0x1000, 0x0800, CRC(5386e5fb) SHA1(822f47951b702f9c6a1ce674baaab0a596f34413))

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("652.snd", 0x0400, 0x0400, CRC(4d0cf2c0) SHA1(0da5d118ffd19b1e78dfaaee3e31c43750d45c8d))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END
#endif

/*-------------------------------------------------------------------
/ Pink Panther #664
/-------------------------------------------------------------------*/
ROM_START(pnkpnthr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("664-1.cpu", 0x1000, 0x0800, CRC(a0d3e69a) SHA1(590e68dc28067e61832927cd4b3eefcc066f0a92))

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("664.snd", 0x0400, 0x0400, CRC(18f4abfd) SHA1(9e85eb7e9b1e2fe71be828ff1b5752424ed42588))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

#ifdef MISSING_GAME // all missing games are using a similar set of ROMs.
ROM_START(pnkpntr7)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2g807dc.bin", 0x2000, 0x1000, CRC(f8a687b3) SHA1(ba7747c04a5967df760ace102e47c91d42e07a12))
	ROM_LOAD("u3g807dc.bin", 0x3000, 0x1000, CRC(6e31242e) SHA1(14e371a0352a6068dec20af1f2b344e34a5b9011))
	ROM_LOAD("664-1.cpu", 0x1000, 0x0800, CRC(a0d3e69a) SHA1(590e68dc28067e61832927cd4b3eefcc066f0a92))

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("664.snd", 0x0400, 0x0400, CRC(18f4abfd) SHA1(9e85eb7e9b1e2fe71be828ff1b5752424ed42588))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END
#endif

/*-------------------------------------------------------------------
/ Star Race #657
/-------------------------------------------------------------------*/
ROM_START(starrace)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("657-1.cpu", 0x1000, 0x0200, CRC(27081372) SHA1(2d9cd81ffa44c389c4895043fa1e93b899544158))
	ROM_RELOAD(0x1400, 0x0200)
	ROM_LOAD("657-2.cpu", 0x1200, 0x0200, CRC(c56e31c8) SHA1(1e129fb6309e015a16f2bdb1e389cbc85d1919a7))
	ROM_RELOAD(0x1600, 0x0200)

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("657.snd", 0x0400, 0x0400, CRC(3a1d3995) SHA1(6f0bdb34c4fa11d5f8ecbb98ae55bafeb5d62c9e))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

#ifdef MISSING_GAME // all missing games are using a similar set of ROMs.
ROM_START(starrac7)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2g807dc.bin", 0x2000, 0x1000, CRC(f8a687b3) SHA1(ba7747c04a5967df760ace102e47c91d42e07a12))
	ROM_LOAD("u3g807dc.bin", 0x3000, 0x1000, CRC(6e31242e) SHA1(14e371a0352a6068dec20af1f2b344e34a5b9011))
	ROM_LOAD("657-1.cpu", 0x1000, 0x0200, CRC(27081372) SHA1(2d9cd81ffa44c389c4895043fa1e93b899544158))
	ROM_RELOAD(0x1400, 0x0200)
	ROM_LOAD("657-2.cpu", 0x1200, 0x0200, CRC(c56e31c8) SHA1(1e129fb6309e015a16f2bdb1e389cbc85d1919a7))
	ROM_RELOAD(0x1600, 0x0200)

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("657.snd", 0x0400, 0x0400, CRC(3a1d3995) SHA1(6f0bdb34c4fa11d5f8ecbb98ae55bafeb5d62c9e))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END
#endif

/*-------------------------------------------------------------------
/ The Amazing Spider-Man #653
/-------------------------------------------------------------------*/
ROM_START(spidermn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("653-1.cpu", 0x1000, 0x0200, CRC(674ddc58) SHA1(c9be45391b8dd58a0836801807d593d4c7da9904))
	ROM_RELOAD(0x1400, 0x0200)
	ROM_LOAD("653-2.cpu", 0x1200, 0x0200, CRC(ff1ddfd7) SHA1(dd7b98e491045916153b760f36432506277a4093))
	ROM_RELOAD(0x1600, 0x0200)

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("653.snd", 0x0400, 0x0400, CRC(f5650c46) SHA1(2d0e50fa2f4b3d633daeaa7454630e3444453cb2))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ The Incredible Hulk
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Time Line #659
/-------------------------------------------------------------------*/
ROM_START(timeline)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("659.cpu", 0x1000, 0x0800, CRC(d6950e3b) SHA1(939b45a9ee4bb122fbea534ad728ec6b85120416))

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("659.snd", 0x0400, 0x0400, CRC(28185568) SHA1(2fd26e7e0a8f050d67159f17634df2b1fc47cbd3))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ Volcano (Sound and Speech) #667
/-------------------------------------------------------------------*/
ROM_START(vlcno_ax)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("667-a-x.cpu", 0x1000, 0x0800, CRC(1f51c351) SHA1(8e1850808faab843ac324040ca665a83809cdc7b))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("667-s1.snd", 0x7000, 0x0800, CRC(ba9d40b7) SHA1(3d6640b259cd8ae87b998cbf1ae2dc13a2913e4f))
	ROM_LOAD("667-s2.snd", 0x7800, 0x0800, CRC(b54bd123) SHA1(3522ccdcb28bfacff2287f5537d52f22879249ab))
ROM_END

ROM_START(vlcno_1b)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("667-1b.cpu", 0x1000, 0x0800, CRC(a422d862) SHA1(2785388eb43c08405774a9413ffa52c1591a84f2))

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("667-a-s.snd", 0x0400, 0x0400, CRC(894b4e2e) SHA1(d888f8e00b2b50cef5cc916d46e4c5e6699914a1))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

ROM_START(vlcno_1a)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("667-1a.cpu", 0x1000, 0x0800, CRC(5931c6f7) SHA1(e104a6c3ca2175bb49199e06963e26185dd563d2))

	ROM_REGION(0x1000, "r0sound:audiocpu", 0)
	ROM_LOAD("667-a-s.snd", 0x0400, 0x0400, CRC(894b4e2e) SHA1(d888f8e00b2b50cef5cc916d46e4c5e6699914a1))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
ROM_END

/*-------------------------------------------------------------------
/ System 80 Test Fixture
/-------------------------------------------------------------------*/
ROM_START(s80tst)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80.bin", 0x2000, 0x1000, CRC(4f0bc7b1) SHA1(612cbacdca5cfa6ad23940796df3b7c385be79fe))
	ROM_LOAD("u3_80.bin", 0x3000, 0x1000, CRC(1e69f9d0) SHA1(ad738cac2555830257b531e5e533b15362f624b9))
	ROM_LOAD("80tst.cpu", 0x1000, 0x0800, CRC(a0f9e56b) SHA1(5146745ab61fea4b3070c6cf4324a9e77a7cee36))

	ROM_REGION(0x10000, "r1sound:audiocpu", 0)
	ROM_LOAD("80tst-s1.snd", 0x7000, 0x0800, CRC(b9dbdd21) SHA1(dfe42c9e6e02f82ffd0cafe164df3211cdc2d966))
	ROM_LOAD("80tst-s2.snd", 0x7800, 0x0800, CRC(1a4b1e9d) SHA1(18e7ffbdbdaf83ab1c8daa5fa5201d9f54390758))
ROM_END

/* disp1 */GAME(1981, s80tst,     0,        gts80_ss,   gts80, gts80_state, gts80,  ROT0, "Gottlieb", "System 80 Test", MACHINE_IS_SKELETON_MECHANICAL)

/* disp1 */GAME(1980, panthera,   0,        gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Panthera",             MACHINE_IS_SKELETON_MECHANICAL)
/* disp1 */GAME(1980, spidermn,   0,        gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "The Amazing Spider-Man", MACHINE_IS_SKELETON_MECHANICAL)
/* disp1 */GAME(1980, circusp,    0,        gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Circus",               MACHINE_IS_SKELETON_MECHANICAL)
/* disp1 */GAME(1980, cntforce,   0,        gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Counterforce",         MACHINE_IS_SKELETON_MECHANICAL)
/* disp1 */GAME(1980, starrace,   0,        gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Star Race",            MACHINE_IS_SKELETON_MECHANICAL)
/* disp2 */GAME(1980, jamesb,     0,        gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "James Bond (Timed Play)", MACHINE_IS_SKELETON_MECHANICAL)
/* disp2 */GAME(1980, jamesb2,    jamesb,   gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "James Bond (3/5-Ball)",   MACHINE_IS_SKELETON_MECHANICAL)
/* cust  */GAME(1980, timeline,   0,        gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Time Line",            MACHINE_IS_SKELETON_MECHANICAL)
/* disp1 */GAME(1981, forceii,    0,        gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Force II",             MACHINE_IS_SKELETON_MECHANICAL)
/* cust  */GAME(1981, pnkpnthr,   0,        gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Pink Panther",         MACHINE_IS_SKELETON_MECHANICAL)
/* disp1 */GAME(1981, marsp,      0,        gts80_ss,   gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Mars - God of War",    MACHINE_IS_SKELETON_MECHANICAL)
/* disp1 */GAME(1981, vlcno_ax,   0,        gts80_ss,   gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Volcano",              MACHINE_IS_SKELETON_MECHANICAL)
/* disp1 */GAME(1981, vlcno_1b,   0,        gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Volcano (Sound Only set 1)", MACHINE_IS_SKELETON_MECHANICAL)
/* disp1 */GAME(1981, vlcno_1a,   vlcno_1b, gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Volcano (Sound Only set 2)", MACHINE_IS_SKELETON_MECHANICAL)
/* disp2 */GAME(1981, blckhole,   0,        gts80_ss,   gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Black Hole (Rev. 4)",      MACHINE_IS_SKELETON_MECHANICAL)
/* disp2 */GAME(1981, blckhole2,  blckhole, gts80_ss,   gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Black Hole (Rev. 2)",      MACHINE_IS_SKELETON_MECHANICAL)
/* disp2 */GAME(1981, blckhols,   0,        gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Black Hole (Sound Only)",  MACHINE_IS_SKELETON_MECHANICAL)
/* disp2 */GAME(1982, hh,         0,        gts80_hh,   gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Haunted House (Rev. 2)",   MACHINE_IS_SKELETON_MECHANICAL)
/* disp2 */GAME(1982, hh_1,       hh,       gts80_hh,   gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Haunted House (Rev. 1)",   MACHINE_IS_SKELETON_MECHANICAL)
/* disp2 */GAME(1981, eclipse,    0,        gts80_s,    gts80, gts80_state, gts80,  ROT0, "Gottlieb", "Eclipse",              MACHINE_IS_SKELETON_MECHANICAL)
