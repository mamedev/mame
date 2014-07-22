/*******************************************************************************

    PINBALL
    Game Plan MPU-1
    These are "cocktail" cabinets, although there is only one seating position.

********************************************************************************/

#include "machine/genpin.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "gp_1.lh"

class gp_1_state : public genpin_class
{
public:
	gp_1_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ctc(*this, "ctc")
	{ }

	DECLARE_DRIVER_INIT(gp_1);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portc_w);
	DECLARE_READ8_MEMBER(portb_r);
	TIMER_DEVICE_CALLBACK_MEMBER(zero_timer);
private:
	UINT8 m_u14;
	UINT8 m_digit;
	UINT8 m_segment;
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
};


static ADDRESS_MAP_START( gp_1_map, AS_PROGRAM, 8, gp_1_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x8c00, 0x8cff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( gp_1_io, AS_IO, 8, gp_1_state )
	ADDRESS_MAP_GLOBAL_MASK(0x0f)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ppi", i8255_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
ADDRESS_MAP_END

static INPUT_PORTS_START( gp_1 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "S01") // S1-5: 32 combinations of coins/credits of coin slot 1.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x02, "S02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Free Play")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "S09") // S09-12 determine coinage for slot 2
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Play Tunes")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "S17") // S17-21 coins for slot 3
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x02, "S18")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S19")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S20")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S21")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S22")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S23")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "S24")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "Max number of credits")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "20")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x05, "30")
	PORT_DIPSETTING(    0x06, "35")
	PORT_DIPSETTING(    0x07, "40")
	PORT_DIPNAME( 0x08, 0x08, "Balls")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x08, "5")
	PORT_DIPNAME( 0x10, 0x10, "Award")
	PORT_DIPSETTING(    0x00, "Extra Ball")
	PORT_DIPSETTING(    0x10, "Replay")
	PORT_DIPNAME( 0x20, 0x20, "Match")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xC0, 0x80, "Credits for exceeding high score")
	PORT_DIPSETTING(    0x00, "0")
	PORT_DIPSETTING(    0x40, "1")
	PORT_DIPSETTING(    0x80, "2")
	PORT_DIPSETTING(    0xC0, "3")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("Accounting Reset")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_TILT1 ) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT )

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L and R Target") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Spinner C") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Spinner B") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R. Slingshot") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Special when lit") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L. Slingshot") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Extra when lit") PORT_CODE(KEYCODE_J)

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R. Spinner") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("1000 and advance") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance and Change") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L. Bumper") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R. Bumper") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L. Spinner") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Spinner A") PORT_CODE(KEYCODE_N)

	PORT_START("XA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("1000 Rollover") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("XB")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

READ8_MEMBER( gp_1_state::portb_r )
{
	switch (m_u14)
	{
		case 7:
			return ioport("X7")->read();
		case 8:
			return ioport("X8")->read();
		case 9:
			return ioport("X9")->read();
		case 10:
			return ioport("XA")->read();
		case 11:
			return ioport("XB")->read();
		case 12:
			return ioport("DSW0")->read();
		case 13:
			return ioport("DSW1")->read();
		case 14:
			return ioport("DSW2")->read();
		case 15:
			return ioport("DSW3")->read();
	}
	return 0xff;
}

WRITE8_MEMBER( gp_1_state::porta_w )
{
	m_u14 = data >> 4;
	/*if (m_u14 >= 8)*/ m_segment = data & 15;
	if ((data > 0x0f) && (data < 0x30))
	{
		switch (data)
		{
			case 0x10:
				break;
			case 0x11:
				m_samples->start(0, 5);
				break;
			case 0x12:
				m_samples->start(0, 6);
				break;
			case 0x13:
				m_samples->start(0, 1);
				break;
			case 0x14:
				m_samples->start(0, 2);
				break;
			case 0x15:
				m_samples->start(0, 3);
				break;
			case 0x16:
				m_samples->start(0, 4);
				break;
			case 0x17:
			case 0x18:
			case 0x19:
				m_samples->start(0, 5);
				break;
			case 0x1a:
			case 0x1b:
				m_samples->start(0, 0);
				break;
			case 0x1c:
			case 0x1d:
				m_samples->start(0, 5);
				break;
			case 0x1e:
			case 0x1f:
				break;
		}
	}
	static const UINT8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 }; // 7448
	if ((m_u14 >= 8) && (m_u14 <= 11))
	{
		output_set_digit_value(m_digit+(m_u14-8)*8, patterns[m_segment]);
	}
}

WRITE8_MEMBER( gp_1_state::portc_w )
{
	output_set_value("led0", BIT(data, 3));

	m_digit = data & 7;
	static const UINT8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 }; // 7448
	if ((m_digit) && (m_u14 >= 8) && (m_u14 <= 11))
	{
		output_set_digit_value(m_digit+(m_u14-8)*8, patterns[m_segment]);
	}
}

void gp_1_state::machine_reset()
{
	m_u14 = 0;
	m_digit = 0xff;
}

// zero-cross detection
TIMER_DEVICE_CALLBACK_MEMBER( gp_1_state::zero_timer )
{
	m_ctc->trg2(0);
	m_ctc->trg2(1);
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ NULL }
};

static MACHINE_CONFIG_START( gp_1, gp_1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 2457600)
	MCFG_CPU_PROGRAM_MAP(gp_1_map)
	MCFG_CPU_IO_MAP(gp_1_io)
	MCFG_CPU_CONFIG(daisy_chain)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_gp_1)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_DEVICE_ADD("ppi", I8255A, 0 )
	MCFG_I8255_OUT_PORTA_CB(WRITE8(gp_1_state, porta_w))
	MCFG_I8255_IN_PORTB_CB(READ8(gp_1_state, portb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(gp_1_state, portc_w))

	MCFG_DEVICE_ADD("ctc", Z80CTC, 2457600 )
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0)) // Todo: absence of ints will cause a watchdog reset
	MCFG_TIMER_DRIVER_ADD_PERIODIC("gp1", gp_1_state, zero_timer, attotime::from_hz(120)) // mains freq*2
MACHINE_CONFIG_END


ROM_START( gp_110 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "a-110.u12", 0x0000, 0x0800, CRC(ed0d518b) SHA1(8f3ca8792ad907c660d9149a1aa3a3528c7573e3))
	ROM_LOAD( "b1-110.u13", 0x0800, 0x0800, CRC(a223f2e8) SHA1(767e15e19e11399935c890c1d1034dccf1ad7f92))
ROM_END


/*-------------------------------------------------------------------
/ Black Velvet (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_blvelvet    rom_gp_110
/*-------------------------------------------------------------------
/ Camel Lights (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_camlight    rom_gp_110
/*-------------------------------------------------------------------
/ Foxy Lady (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_foxylady    rom_gp_110
/*-------------------------------------------------------------------
/ Real (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_real    rom_gp_110
/*-------------------------------------------------------------------
/ Rio (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_rio    rom_gp_110

/*-------------------------------------------------------------------
/ Chuck-A-Luck (October 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_chucklck    rom_gp_110

/*-------------------------------------------------------------------
/ Family Fun! (April 1979) - Model: Cocktail #120
/-------------------------------------------------------------------*/
ROM_START(famlyfun)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "family.u12", 0x0000, 0x0800, CRC(98f27fdf) SHA1(8bcff1e13b9b978f91110f1e83a3288723930a1d))
	ROM_LOAD( "family.u13", 0x0800, 0x0800, CRC(b941a1a8) SHA1(a43f8acadb3db3e2274162d5305e30006f912339))
ROM_END

/*-------------------------------------------------------------------
/ Star Trip (April 1979) - Model: Cocktail #120
/-------------------------------------------------------------------*/
ROM_START(startrip)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "startrip.u12", 0x0000, 0x0800, CRC(98f27fdf) SHA1(8bcff1e13b9b978f91110f1e83a3288723930a1d))
	ROM_LOAD( "startrip.u13", 0x0800, 0x0800, CRC(b941a1a8) SHA1(a43f8acadb3db3e2274162d5305e30006f912339))
ROM_END

GAME(1978, gp_110,   0,        gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Model 110",    GAME_IS_BIOS_ROOT)
GAME(1978, blvelvet, gp_110,   gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Black Velvet", GAME_IS_SKELETON_MECHANICAL)
GAME(1978, camlight, gp_110,   gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Camel Lights", GAME_IS_SKELETON_MECHANICAL)
GAME(1978, foxylady, gp_110,   gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Foxy Lady",    GAME_IS_SKELETON_MECHANICAL)
GAME(1978, real,     gp_110,   gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Real",         GAME_IS_SKELETON_MECHANICAL)
GAME(1978, rio,      gp_110,   gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Rio",          GAME_IS_SKELETON_MECHANICAL)
GAME(1978, chucklck, gp_110,   gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Chuck-A-Luck", GAME_IS_SKELETON_MECHANICAL)
GAME(1979, famlyfun, 0,        gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Family Fun!",  GAME_IS_SKELETON_MECHANICAL)
GAME(1979, startrip, 0,        gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Star Trip",    GAME_IS_SKELETON_MECHANICAL)
