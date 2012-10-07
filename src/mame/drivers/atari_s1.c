/***********************************************************************************

    Pinball
    Atari Generation/System 1
    These were the first widebody pinball machines.
    Unfortunately they tended to be boring.

    The design of having the circuit boards under the playfield wasn't such a good
    idea, because it allowed debris and loose screws that could fall off the
    playfield to cause havoc among the electronics.

    Schematics and PinMAME used as references.

ToDo:
- Inputs per game; the ones there are for Airborne Avenger
- Link up switches where 2 or more act together
- Sound
- Lamps, solenoids


************************************************************************************/

#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "atari_s1.lh"

// looking at chip '29 74', this could be /8
#define MASTER_CLK XTAL_4MHz / 4
#define DMA_CLK MASTER_CLK / 2
//#define AUDIO_CLK DMA_CLK / 4
#define DMA_INT DMA_CLK / 128
#define NMI_INT DMA_INT / 16
//#define BIT6_CLK NMI_INT / 4

class atari_s1_state : public driver_device
{
public:
	atari_s1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_p_ram(*this, "ram"),
	m_samples(*this, "samples")
	{ }

	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_READ8_MEMBER(latch_r);
	DECLARE_WRITE8_MEMBER(latch_w);

	TIMER_DEVICE_CALLBACK_MEMBER(nmi);

protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_p_ram;
	required_device<samples_device> m_samples;

	// driver_device overrides
	virtual void machine_reset();
private:
	UINT8 m_bit6;
	UINT8 m_out_offs;
};

static ADDRESS_MAP_START( atari_s1_map, AS_PROGRAM, 8, atari_s1_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x00ff) AM_RAM AM_SHARE("ram")
	//AM_RANGE(0x1000, 0x100f) AM_MIRROR(0x0F70) AM_WRITE(latch1000_w) // lamps
	AM_RANGE(0x1080, 0x108f) AM_MIRROR(0x0F70) AM_READWRITE(latch_r,latch_w)
	//AM_RANGE(0x2000, 0x200f) AM_WRITE(dipg1_w)
	AM_RANGE(0x2000, 0x204f) AM_MIRROR(0x0F80) AM_READ(switch_r)
	//AM_RANGE(0x3000, 0x3fff) AM_WRITE(soundg1_w) // audio enable
	AM_RANGE(0x4000, 0x4fff) AM_WRITE(watchdog_reset_w)
	//AM_RANGE(0x508c, 0x508c) AM_WRITE(latch508c_w)
	//AM_RANGE(0x6000, 0x6fff) AM_WRITE(audiog1_w) // audio reset
	AM_RANGE(0x7000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( atari_s1 )
	// need to fix these, the manual has a lot of mistakes
	PORT_START("INP00")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test") PORT_CODE(KEYCODE_0)
	PORT_START("INP01")
	PORT_DIPNAME( 0x80, 0x80, "Spelling Award" )
	PORT_DIPSETTING(    0x80, "Extra Ball" )
	PORT_DIPSETTING(    0x00, "20,000 points" )
	PORT_START("INP02")
	PORT_DIPNAME( 0x80, 0x80, "Last Ball double bonus" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("INP03")
	// This switch together with INP4C thru 4F, sets the scores at which a replay is awarded
	PORT_DIPNAME( 0x80, 0x80, "Replay score" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// These 2 dips control max number of credits (both off = 8; then 12, 15, both on=20)
	PORT_START("INP04")
	PORT_DIPNAME( 0x80, 0x80, "Max Credits-a" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("INP05")
	PORT_DIPNAME( 0x80, 0x80, "Max Credits-b" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("INP06")
	PORT_DIPNAME( 0x80, 0x80, "Exceed replay score" )
	PORT_DIPSETTING(    0x80, "Nothing" )
	PORT_DIPSETTING(    0x00, "Extra Ball" )
	PORT_START("INP07")
	PORT_DIPNAME( 0x80, 0x80, "Replay when Exceed replay score" ) // overrides INP06 if on
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("INP08")
	PORT_DIPNAME( 0x80, 0x80, "Balls" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_START("INP09")
	PORT_DIPNAME( 0x80, 0x80, "Match" )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	// next 4 dips control coins and credits. Leave them all off for 1coin=1credit.
	PORT_START("INP0A")
	PORT_DIPNAME( 0x80, 0x80, "Coins-a" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("INP0B")
	PORT_DIPNAME( 0x80, 0x80, "Coins-b" ) // this is actually the test button
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("INP0C")
	PORT_DIPNAME( 0x80, 0x80, "Coins-c" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("INP0D")
	PORT_DIPNAME( 0x80, 0x80, "Coins-d" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("INP0E")
	PORT_DIPNAME( 0x80, 0x80, "Special" )
	PORT_DIPSETTING(    0x80, "Extra Ball" )
	PORT_DIPSETTING(    0x00, "Free Game" )
	PORT_START("INP0F")
	PORT_DIPNAME( 0x80, 0x80, "20000 points for special" ) // overrides INP0E if on
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("INP10")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_START("INP11")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_START("INP12")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START )
	PORT_START("INP13")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Slam Tilt")
	PORT_START("INP14")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP15")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP16")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP17")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP18")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP19")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP1A")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP1B")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP1C")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP1D")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP1E")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP1F")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP20")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Cabinet Tilt")
	PORT_START("INP21")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT ) PORT_NAME("Pendulum Tilt")
	PORT_START("INP22")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left Flipper") PORT_CODE(KEYCODE_LSHIFT)
	PORT_START("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right Flipper") PORT_CODE(KEYCODE_RSHIFT)
	PORT_START("INP24")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Target 4")
	PORT_START("INP25")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Target 3")
	PORT_START("INP26")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Target 2")
	PORT_START("INP27")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Target 1")
	PORT_START("INP28")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP29")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP2A")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP2B")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP2C")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP2D")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP2E")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP2F")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP30")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 3") PORT_CODE(KEYCODE_Q)
	PORT_START("INP32")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 2") PORT_CODE(KEYCODE_W)
	PORT_START("INP33")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 1") PORT_CODE(KEYCODE_E)
	PORT_START("INP34")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 8") PORT_CODE(KEYCODE_R)
	PORT_START("INP35")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 7") PORT_CODE(KEYCODE_Y)
	PORT_START("INP36")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 6") PORT_CODE(KEYCODE_U)
	PORT_START("INP37")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 5") PORT_CODE(KEYCODE_I)
	PORT_START("INP38")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L Hole") PORT_CODE(KEYCODE_O)
	PORT_START("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bumper 1") PORT_CODE(KEYCODE_V)
	PORT_START("INP3A")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bumper 2") PORT_CODE(KEYCODE_B)
	PORT_START("INP3B")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bumper 3") PORT_CODE(KEYCODE_N)
	PORT_START("INP3C")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Spinner") PORT_CODE(KEYCODE_M)
	PORT_START("INP3D")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R Pocket") PORT_CODE(KEYCODE_A)
	PORT_START("INP3E")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L Pocket") PORT_CODE(KEYCODE_S)
	PORT_START("INP3F")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R Hole") PORT_CODE(KEYCODE_D)
	PORT_START("INP40")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L Triangle") PORT_CODE(KEYCODE_F)
	PORT_START("INP41")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("10 points") PORT_CODE(KEYCODE_G)
	PORT_START("INP42")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("50 points") PORT_CODE(KEYCODE_H)
	PORT_START("INP43")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("OutHole") PORT_CODE(KEYCODE_X)
	PORT_START("INP44")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP45")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP46")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R Triangle") PORT_CODE(KEYCODE_J)
	PORT_START("INP48")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("50 points and adv letter") PORT_CODE(KEYCODE_K)
	PORT_START("INP49")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Captive ball rollovers") PORT_CODE(KEYCODE_L)
	PORT_START("INP4A")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rollover 'B' centre") PORT_CODE(KEYCODE_Z)
	PORT_START("INP4B")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Roll thru upper right") PORT_CODE(KEYCODE_C)
	PORT_START("INP4C")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP4D")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP4E")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_START("INP4F")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )
INPUT_PORTS_END

void atari_s1_state::machine_reset()
{
}

READ8_MEMBER( atari_s1_state::latch_r )
{
	return 0;
}

WRITE8_MEMBER( atari_s1_state::latch_w )
{
}

READ8_MEMBER( atari_s1_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"INP%02X",offset);
	return ioport(kbdrow)->read() | (BIT(m_bit6, 1) << 6);
}

TIMER_DEVICE_CALLBACK_MEMBER( atari_s1_state::nmi )
{
	m_bit6++;
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);

	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0, 0, 0, 0, 0, 0 };
	m_out_offs++;
	m_out_offs &= 0x1f;
	output_set_digit_value(m_out_offs << 1, patterns[m_p_ram[m_out_offs]>>4]);
	output_set_digit_value((m_out_offs << 1)+1, patterns[m_p_ram[m_out_offs]&15]);
}

static MACHINE_CONFIG_START( atari_s1, atari_s1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, MASTER_CLK)
	MCFG_CPU_PROGRAM_MAP(atari_s1_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmi", atari_s1_state, nmi, attotime::from_hz(NMI_INT))

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_atari_s1)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ The Atarians (11/1976)
/-------------------------------------------------------------------*/
ROM_START(atarians)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("atarian.e00", 0x7000, 0x0800, CRC(6066bd63) SHA1(e993497d0ca9f056e18838494089def8bdc265c9))
	ROM_LOAD("atarian.e0", 0x7800, 0x0800, CRC(45cb0427) SHA1(e286930ca36bdd0f79acefd142d2a5431fa8005b))
ROM_END

/*-------------------------------------------------------------------
/ The Atarians (working bootleg)
/-------------------------------------------------------------------*/
//ROM_START(atarianb)
//  ROM_REGION(0x10000, "maincpu", 0)
//  ROM_LOAD("atarianb.e00", 0x7000, 0x0800, CRC(74fc86e4) SHA1(135d75e5c03feae0929fa84caa3c802353cdd94e))
//  ROM_LOAD("atarian.e0", 0x7800, 0x0800, CRC(45cb0427) SHA1(e286930ca36bdd0f79acefd142d2a5431fa8005b))
//ROM_END

/*-------------------------------------------------------------------
/ Time 2000 (06/1977)
/-------------------------------------------------------------------*/
ROM_START(time2000)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("time.e00", 0x7000, 0x0800, CRC(e380f35c) SHA1(f2b4c508c8b7a2ce9924da97c05fb31d5115f36f))
	ROM_LOAD("time.e0", 0x7800, 0x0800, CRC(1e79c133) SHA1(54ce5d59a00334fcec8b12c077d70e3629549af0))
ROM_END

/*-------------------------------------------------------------------
/ Airborne Avenger (09/1977)
/-------------------------------------------------------------------*/
ROM_START(aavenger)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("airborne.e00", 0x7000, 0x0800, CRC(05ac26b8) SHA1(114d587923ade9370d606e428af02a407d272c85))
	ROM_LOAD("airborne.e0", 0x7800, 0x0800, CRC(44e67c54) SHA1(7f94189c12e322c41908d651cf6a3b6061426959))
ROM_END

/*-------------------------------------------------------------------
/ Middle Earth (02/1978)
/-------------------------------------------------------------------*/
ROM_START(midearth)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("609.bin", 0x7000, 0x0800, CRC(589df745) SHA1(4bd3e4f177e8d86bab41f3a14c169b936eeb480a))
	ROM_LOAD("608.bin", 0x7800, 0x0800, CRC(28b92faf) SHA1(8585770f4059049f1dcbc0c6ef5718b6ff1a5431))
ROM_END

/*-------------------------------------------------------------------
/ Space Riders (09/1978)
/-------------------------------------------------------------------*/
ROM_START(spcrider)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("spacer.bin", 0x7000, 0x0800, CRC(3cf1cd73) SHA1(c46044fb815b439f12fb3e21c470c8b93ebdfd55))
	ROM_LOAD("spacel.bin", 0x7800, 0x0800, CRC(66ffb04e) SHA1(42d8b7fb7206b30478f631d0e947c0908dcf5419))
ROM_END


GAME( 1976, atarians, 0,		atari_s1, atari_s1, driver_device, 0, ROT0, "Atari","The Atarians", 	GAME_IS_SKELETON_MECHANICAL)
//GAME( 2002, atarianb, atarians,   atari_s1, atari_s1, driver_device, 0, ROT0, "Atari / Gaston","The Atarians (working bootleg)",  GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, time2000, 0,		atari_s1, atari_s1, driver_device, 0, ROT0, "Atari","Time 2000",		GAME_IS_SKELETON_MECHANICAL)
GAME( 1977, aavenger, 0,		atari_s1, atari_s1, driver_device, 0, ROT0, "Atari","Airborne Avenger", GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, midearth, 0,		atari_s1, atari_s1, driver_device, 0, ROT0, "Atari","Middle Earth", 	GAME_IS_SKELETON_MECHANICAL)
GAME( 1978, spcrider, 0,		atari_s1, atari_s1, driver_device, 0, ROT0, "Atari","Space Riders",		GAME_IS_SKELETON_MECHANICAL)
