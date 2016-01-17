// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************

    PINBALL
    Atari Generation/System 1

    Schematics and PinMAME used as references.

    Dipswitch vs address:
    SW1 Toggle 1 = 200B
    SW1 Toggle 2 = 200A
    SW1 Toggle 3 = 2009
    SW1 Toggle 4 = 2008
    SW1 Toggle 5 = 200F
    SW1 Toggle 6 = 200E
    SW1 Toggle 7 = 200D
    SW1 Toggle 8 = 200C
    SW2 Toggle 1 = 2003
    SW2 Toggle 2 = 2002
    SW2 Toggle 3 = 2001
    SW2 Toggle 4 = 2000
    SW2 Toggle 5 = 2007
    SW2 Toggle 6 = 2006
    SW2 Toggle 7 = 2005
    SW2 Toggle 8 = 2004

No NVRAM; coin counters are mechanical.

Until game-specific switches are set up, here is the outhole for each game:
- spcrider: O
- atarians: W
- time2000: Y
- aavenger: X

ToDo:
- Inputs per game; the ones there are for Airborne Avenger
- Link up switches where 2 or more act together
- Sound
- Lamps, solenoids
- Middle Earth is a disaster area


************************************************************************************/

#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "sound/dac.h"
#include "atari_s1.lh"


#define MASTER_CLK XTAL_4MHz / 4
#define DMA_CLK MASTER_CLK / 2
#define AUDIO_CLK DMA_CLK / 4
#define DMA_INT DMA_CLK / 128
#define NMI_INT DMA_INT / 16
//#define BIT6_CLK NMI_INT / 4

class atari_s1_state : public genpin_class
{
public:
	atari_s1_state(const machine_config &mconfig, device_type type, std::string tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "ram")
		, m_dac(*this, "dac")
		, m_switch(*this, "SWITCH")
	{ }

	DECLARE_READ8_MEMBER(m1080_r);
	DECLARE_WRITE8_MEMBER(m1080_w);
	DECLARE_READ8_MEMBER(m1084_r);
	DECLARE_WRITE8_MEMBER(m1084_w);
	DECLARE_READ8_MEMBER(m1088_r);
	DECLARE_WRITE8_MEMBER(m1088_w);
	DECLARE_READ8_MEMBER(m108c_r);
	DECLARE_WRITE8_MEMBER(m108c_w);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(meter_w);
	DECLARE_WRITE8_MEMBER(audioen_w);
	DECLARE_WRITE8_MEMBER(audiores_w);
	DECLARE_WRITE8_MEMBER(midearth_w);
	TIMER_DEVICE_CALLBACK_MEMBER(nmi);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_s);
private:
	bool m_audiores;
	UINT8 m_timer_s[3];
	UINT8 m_vol;
	UINT8 m_1080;
	UINT8 m_1084;
	UINT8 m_1088;
	UINT8 m_108c;
	UINT8 m_bit6;
	UINT8 m_out_offs;
	UINT8 m_t_c;
	UINT8 *m_p_prom;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_p_ram;
	required_device<dac_device> m_dac;
	required_ioport_array<10> m_switch;
};

static ADDRESS_MAP_START( atari_s1_map, AS_PROGRAM, 8, atari_s1_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x00ff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x1080, 0x1083) AM_READWRITE(m1080_r,m1080_w)
	AM_RANGE(0x1084, 0x1087) AM_READWRITE(m1084_r,m1084_w)
	AM_RANGE(0x1088, 0x108b) AM_READWRITE(m1088_r,m1088_w)
	AM_RANGE(0x108c, 0x108f) AM_READWRITE(m108c_r,m108c_w)
	AM_RANGE(0x2000, 0x204f) AM_MIRROR(0x0F80) AM_READ(switch_r) AM_WRITENOP // aavenger ROL 200B causes a spurious write
	AM_RANGE(0x3000, 0x3fff) AM_WRITE(audioen_w) // audio enable
	AM_RANGE(0x4000, 0x4fff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5080, 0x508c) AM_MIRROR(3) AM_WRITE(meter_w) // time2000 only
	AM_RANGE(0x6000, 0x6fff) AM_WRITE(audiores_w) // audio reset
	AM_RANGE(0x7000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( atarians_map, AS_PROGRAM, 8, atari_s1_state ) // more ram
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x01ff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x1080, 0x1083) AM_READWRITE(m1080_r,m1080_w)
	AM_RANGE(0x1084, 0x1087) AM_READWRITE(m1084_r,m1084_w)
	AM_RANGE(0x1088, 0x108b) AM_READWRITE(m1088_r,m1088_w)
	AM_RANGE(0x108c, 0x108f) AM_READWRITE(m108c_r,m108c_w)
	AM_RANGE(0x2000, 0x204f) AM_MIRROR(0x0F80) AM_READ(switch_r)
	AM_RANGE(0x3000, 0x3fff) AM_WRITE(audioen_w) // audio enable
	AM_RANGE(0x4000, 0x4fff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x6000, 0x6fff) AM_WRITE(audiores_w) // audio reset
	AM_RANGE(0x7000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( midearth_map, AS_PROGRAM, 8, atari_s1_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x01ff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x1000, 0x11ff) AM_WRITE(midearth_w)
	AM_RANGE(0x2000, 0x204f) AM_MIRROR(0x0F80) AM_READ(switch_r)
	AM_RANGE(0x3000, 0x3fff) AM_WRITE(audioen_w) // audio enable
	AM_RANGE(0x4000, 0x4fff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x6000, 0x6fff) AM_WRITE(audiores_w) // audio reset
	AM_RANGE(0x7000, 0x7fff) AM_ROM AM_WRITENOP // writes to FFFF due to poor coding at 7FF5
ADDRESS_MAP_END

static INPUT_PORTS_START( atari_s1 )
	PORT_START("SWITCH.0") // 2000-2007
	PORT_DIPNAME( 0xc3, 0x00, DEF_STR( Coinage ) ) // left chute; right chute
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )   // 1C_1C ; 1C_1C
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )   // 1C_2C ; 1C_2C
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )   // 1C_3C ; 1C_3C
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )   // 1C_4C ; 1C_4C
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )   // 2C_1C ; 2C_1C
	PORT_DIPSETTING(    0x82, DEF_STR( 2C_3C ) )   // 2C_3C ; 2C_3C
	PORT_DIPSETTING(    0x81, DEF_STR( 2C_5C ) )   // 2C_5C ; 2C_5C
	// these work but they confuse the dipswitch menu
	//PORT_DIPSETTING(    0x83, DEF_STR( 2C_1C ) )   // 2C_1C ; 1C_1C
	//PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )   // 1C_1C ; 1C_2C
	//PORT_DIPSETTING(    0x42, DEF_STR( 1C_2C ) )   // 1C_2C ; 1C_4C
	//PORT_DIPSETTING(    0x41, DEF_STR( 1C_3C ) )   // 1C_3C ; 1C_6C
	//PORT_DIPSETTING(    0x43, DEF_STR( 2C_3C ) )   // 2C_3C ; 2C_7C
	//PORT_DIPSETTING(    0xc0, DEF_STR( 2C_5C ) )   // 2C_5C ; 1C_5C
	//PORT_DIPSETTING(    0xc2, DEF_STR( 2C_7C ) )   // 2C_7C ; 1C_7C
	//PORT_DIPSETTING(    0xc1, DEF_STR( 3C_1C ) )   // 3C_1C ; 1C_1C
	//PORT_DIPSETTING(    0xc3, DEF_STR( 1C_1C ) )   // 1C_1C ; 1C_3C
	PORT_DIPNAME( 0x04, 0x04, "Match" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Balls" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x30, 0x20, "Special" )
	PORT_DIPSETTING(    0x00, "Extra Ball" )
	PORT_DIPSETTING(    0x20, "Free Game" )
	PORT_DIPSETTING(    0x10, "20000 points" ) // same as 0x30

	PORT_START("SWITCH.1") // 2008-200F
	// This switch together with 204C thru 204F, sets the scores at which a replay is awarded
	PORT_DIPNAME( 0x01, 0x00, "Replay score" )
	PORT_DIPSETTING(    0x01, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x02, 0x02, "Last Ball double bonus" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Spelling Award" )
	PORT_DIPSETTING(    0x00, "Extra Ball" )
	PORT_DIPSETTING(    0x04, "20,000 points" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Test") PORT_CODE(KEYCODE_0)
	PORT_DIPNAME( 0x30, 0x10, "Exceed replay score" )
	PORT_DIPSETTING(    0x00, "Nothing" )
	PORT_DIPSETTING(    0x20, "Extra Ball" )
	PORT_DIPSETTING(    0x10, "Replay" ) // same as 0x30
	PORT_DIPNAME( 0xc0, 0x00, "Max Credits" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x80, "12" )
	PORT_DIPSETTING(    0x40, "15" )
	PORT_DIPSETTING(    0xc0, "20" )

	PORT_START("SWITCH.2") // 2010-2017
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )

	PORT_START("SWITCH.3") // 2018-201F
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )

	PORT_START("SWITCH.4") // 2020-2027
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Cabinet Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT ) PORT_NAME("Pendulum Tilt")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left Flipper") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right Flipper") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Target 4")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Target 3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Target 2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Target 1")

	PORT_START("SWITCH.5") // 2028-202F
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )

	PORT_START("SWITCH.6") // 2030-2037
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 3") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 2") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 1") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 8") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 7") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 6") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 5") PORT_CODE(KEYCODE_I)

	PORT_START("SWITCH.7") // 2038-203F
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L Hole") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bumper 1") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bumper 2") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bumper 3") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Spinner") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R Pocket") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L Pocket") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R Hole") PORT_CODE(KEYCODE_D)

	PORT_START("SWITCH.8") // 2040-2047
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L Triangle") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("10 points") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("50 points") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("OutHole") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R Triangle") PORT_CODE(KEYCODE_J)

	PORT_START("SWITCH.9") // 2048-204F
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("50 points and adv letter") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Captive ball rollovers") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 'B' centre") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Roll thru upper right") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
INPUT_PORTS_END

READ8_MEMBER( atari_s1_state::m1080_r )
{
	return m_1080 & 0xf0;
}

WRITE8_MEMBER( atari_s1_state::m1080_w )
{
	m_1080 = data;
}

READ8_MEMBER( atari_s1_state::m1084_r )
{
	return m_1084 & 0xf0;
}

WRITE8_MEMBER( atari_s1_state::m1084_w )
{
	m_1084 = data;

	data &= 15;

	if (data != m_vol)
	{
		m_vol = data;
		float vol = m_vol/16.666+0.1;
		m_dac->set_output_gain(0, vol);
	}
}

READ8_MEMBER( atari_s1_state::m1088_r )
{
	return m_1088 & 0xf0;
}

WRITE8_MEMBER( atari_s1_state::m1088_w )
{
	m_1088 = data;
}

READ8_MEMBER( atari_s1_state::m108c_r )
{
	return m_108c;
}

WRITE8_MEMBER( atari_s1_state::m108c_w )
{
	m_108c = data;
}

WRITE8_MEMBER( atari_s1_state::meter_w )
{
// time2000 has optional coin counters etc
}

// midearth has a ram mirror that goes on top of the output ports
WRITE8_MEMBER( atari_s1_state::midearth_w )
{
	m_p_ram[offset] = data;

	switch (offset)
	{
		case 0x80:
			m1080_w(space, 0, data);
			break;
		case 0x84:
			m1084_w(space, 0, data);
			break;
		case 0x88:
			m1088_w(space, 0, data);
			break;
		case 0x8c:
			m108c_w(space, 0, data);
			break;
	}
}

READ8_MEMBER( atari_s1_state::switch_r )
{
	return (BIT(m_switch[offset>>3]->read(), offset&7 ) << 7) | (BIT(m_bit6, 1) << 6); // switch bit | BIT6_CLK
}

TIMER_DEVICE_CALLBACK_MEMBER( atari_s1_state::nmi )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // 4511
	m_bit6++;
	if (m_t_c > 0x40)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	else
		m_t_c++;

	m_out_offs++;
	m_out_offs &= 0x1f;
	if ((m_out_offs & 3) == 3)
	{
		// Player number
		char wordnum[8];
		sprintf(wordnum,"text%d",m_out_offs>>2);
		output().set_value(wordnum, !BIT(patterns[m_p_ram[m_out_offs]&15], 6)); // uses 'g' segment
	}
	else
	{
		// Digits
		output().set_digit_value(m_out_offs << 1, patterns[m_p_ram[m_out_offs]>>4]);
		output().set_digit_value((m_out_offs << 1)+1, patterns[m_p_ram[m_out_offs]&15]);
	}
}

// Sound
// Presettable 74LS161 binary divider controlled by 1088:d0-3
// Then a pair of 7493 to generate 5 address lines, enabled by audiores
// The address lines are merged with 1080:d0-3 to form a lookup on the prom
// Output of prom goes to a 4-bit DAC
// Volume is controlled by 1084:d0-3
// Variables:
// m_timer_s[1] count in 74LS161
// m_timer_s[2] count in 7493s
TIMER_DEVICE_CALLBACK_MEMBER( atari_s1_state::timer_s )
{
	m_timer_s[1]++;

	if (m_timer_s[1] > 15)
	{
		m_timer_s[1] = m_1088 & 15; // set to preset value
		if (!m_audiores)
		{
			m_timer_s[2]++;
			offs_t offs = (m_timer_s[2] & 31) | ((m_1080 & 15) << 5);
			m_dac->write_unsigned8(m_p_prom[offs]<< 4);
		}
		else
			m_timer_s[2] = 0;
	}
}

WRITE8_MEMBER( atari_s1_state::audioen_w )
{
}

WRITE8_MEMBER( atari_s1_state::audiores_w )
{
	if (data==0x5b) data=0; // spcrider
	m_audiores = (data) ? 0 : 1;
}


void atari_s1_state::machine_reset()
{
	m_p_prom = memregion("proms")->base();
	m_vol = 0;
	m_t_c = 0;
	m_audiores = 0;
}

static MACHINE_CONFIG_START( atari_s1, atari_s1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, MASTER_CLK)
	MCFG_CPU_PROGRAM_MAP(atari_s1_map)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_atari_s1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmi", atari_s1_state, nmi, attotime::from_hz(NMI_INT))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_s", atari_s1_state, timer_s, attotime::from_hz(AUDIO_CLK))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( atarians, atari_s1 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(atarians_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( midearth, atari_s1 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(midearth_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ The Atarians (11/1976)
/-------------------------------------------------------------------*/
ROM_START(atarians)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("atarian.e00", 0x7000, 0x0800, CRC(6066bd63) SHA1(e993497d0ca9f056e18838494089def8bdc265c9))
	ROM_LOAD("atarian.e0", 0x7800, 0x0800, CRC(45cb0427) SHA1(e286930ca36bdd0f79acefd142d2a5431fa8005b))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

/*-------------------------------------------------------------------
/ Time 2000 (06/1977)
/-------------------------------------------------------------------*/
ROM_START(time2000)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("time.e00", 0x7000, 0x0800, CRC(e380f35c) SHA1(f2b4c508c8b7a2ce9924da97c05fb31d5115f36f))
	ROM_LOAD("time.e0", 0x7800, 0x0800, CRC(1e79c133) SHA1(54ce5d59a00334fcec8b12c077d70e3629549af0))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

/*-------------------------------------------------------------------
/ Airborne Avenger (09/1977)
/-------------------------------------------------------------------*/
ROM_START(aavenger)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("airborne.e00", 0x7000, 0x0800, CRC(05ac26b8) SHA1(114d587923ade9370d606e428af02a407d272c85))
	ROM_LOAD("airborne.e0", 0x7800, 0x0800, CRC(44e67c54) SHA1(7f94189c12e322c41908d651cf6a3b6061426959))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

/*-------------------------------------------------------------------
/ Middle Earth (02/1978)
/-------------------------------------------------------------------*/
ROM_START(midearth)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("609.bin", 0x7000, 0x0800, CRC(589df745) SHA1(4bd3e4f177e8d86bab41f3a14c169b936eeb480a))
	ROM_LOAD("608.bin", 0x7800, 0x0800, CRC(28b92faf) SHA1(8585770f4059049f1dcbc0c6ef5718b6ff1a5431))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

/*-------------------------------------------------------------------
/ Space Riders (09/1978)
/-------------------------------------------------------------------*/
ROM_START(spcrider)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("spacer.bin", 0x7000, 0x0800, CRC(3cf1cd73) SHA1(c46044fb815b439f12fb3e21c470c8b93ebdfd55))
	ROM_LOAD("spacel.bin", 0x7800, 0x0800, CRC(66ffb04e) SHA1(42d8b7fb7206b30478f631d0e947c0908dcf5419))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END


GAME( 1976, atarians, 0,         atarians, atari_s1, driver_device, 0, ROT0, "Atari", "The Atarians", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND)
GAME( 1977, time2000, 0,         atari_s1, atari_s1, driver_device, 0, ROT0, "Atari", "Time 2000", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND)
GAME( 1977, aavenger, 0,         atari_s1, atari_s1, driver_device, 0, ROT0, "Atari", "Airborne Avenger", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND)
GAME( 1978, midearth, 0,         midearth, atari_s1, driver_device, 0, ROT0, "Atari", "Middle Earth", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1978, spcrider, 0,         atari_s1, atari_s1, driver_device, 0, ROT0, "Atari", "Space Riders", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND)
