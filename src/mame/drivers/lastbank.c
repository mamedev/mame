/***************************************************************************

    Last Bank skeleton driver

    Uses a TC0091LVC, a variant of the one used on Taito L HW

    TODO:
    - somebody should port CPU core contents in a shared file;
    - Currently resets half-way through bootstrap routine, check why;

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"

#define MASTER_CLOCK XTAL_14_31818MHz

class lastbank_state : public driver_device
{
public:
	lastbank_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
		{ }

	required_device<cpu_device> m_maincpu;

	UINT8 m_rom_bank;
	UINT8 m_irq_vector[3];
	UINT8 m_irq_enable;

	DECLARE_READ8_MEMBER(lastbank_rom_r);
	DECLARE_READ8_MEMBER(lastbank_rom_bank_r);
	DECLARE_WRITE8_MEMBER(lastbank_rom_bank_w);
	DECLARE_READ8_MEMBER(lastbank_irq_vector_r);
	DECLARE_WRITE8_MEMBER(lastbank_irq_vector_w);
	DECLARE_READ8_MEMBER(lastbank_irq_enable_r);
	DECLARE_WRITE8_MEMBER(lastbank_irq_enable_w);
};


static VIDEO_START( lastbank )
{

}

static SCREEN_UPDATE_IND16( lastbank )
{
	return 0;
}


READ8_MEMBER(lastbank_state::lastbank_rom_r)
{
	UINT8 *ROM = memregion("maincpu")->base();

	return ROM[offset + 0x10000 + m_rom_bank * 0x2000];
}

READ8_MEMBER(lastbank_state::lastbank_rom_bank_r)
{
	return m_rom_bank;
}

WRITE8_MEMBER(lastbank_state::lastbank_rom_bank_w)
{
	m_rom_bank = data;
}

READ8_MEMBER(lastbank_state::lastbank_irq_vector_r)
{
	return m_irq_vector[offset];
}

WRITE8_MEMBER(lastbank_state::lastbank_irq_vector_w)
{
	m_irq_vector[offset] = data;
}

READ8_MEMBER(lastbank_state::lastbank_irq_enable_r)
{
	return m_irq_enable;
}

WRITE8_MEMBER(lastbank_state::lastbank_irq_enable_w)
{
	m_irq_enable = data;
}

static READ8_HANDLER( test_r )
{
	return -1;
}

static ADDRESS_MAP_START( lastbank_map, AS_PROGRAM, 8, lastbank_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x7fff) AM_READ(lastbank_rom_r)

	AM_RANGE(0x8000, 0x9fff) AM_RAM

	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("IN0")
	AM_RANGE(0xa801, 0xa801) AM_READ_PORT("IN1")
	AM_RANGE(0xa802, 0xa802) AM_READ_PORT("IN2")
	AM_RANGE(0xa803, 0xa803) AM_READ_PORT("IN3") AM_WRITENOP // mux for $a808 / $a80c
	AM_RANGE(0xa804, 0xa804) AM_READ_PORT("IN4")
	AM_RANGE(0xa808, 0xa808) AM_READNOP
	AM_RANGE(0xa80c, 0xa80c) AM_READNOP
	AM_RANGE(0xa800, 0xa81f) AM_READ_LEGACY(test_r)
	/* TODO: RAM banks! */
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xfdff) AM_RAM

	//AM_RANGE(0xfe00, 0xfe03) AM_READWRITE_LEGACY(taitol_bankc_r, taitol_bankc_w)
	//AM_RANGE(0xfe04, 0xfe04) AM_READWRITE_LEGACY(taitol_control_r, taitol_control_w)
	AM_RANGE(0xfe00, 0xfeff) AM_RAM

	AM_RANGE(0xff00, 0xff02) AM_READWRITE(lastbank_irq_vector_r, lastbank_irq_vector_w)
	AM_RANGE(0xff03, 0xff03) AM_READWRITE(lastbank_irq_enable_r, lastbank_irq_enable_w)
	//AM_RANGE(0xff04, 0xff07) AM_READWRITE_LEGACY(rambankswitch_r, rambankswitch_w)
	AM_RANGE(0xff08, 0xff08) AM_READWRITE(lastbank_rom_bank_r, lastbank_rom_bank_w)

ADDRESS_MAP_END

static ADDRESS_MAP_START( lastbank_io, AS_IO, 8, lastbank_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)

ADDRESS_MAP_END


static INPUT_PORTS_START( lastbank )
	PORT_START("IN0")
	PORT_START("IN1")
	PORT_START("IN2")
	PORT_START("IN3")

	PORT_START("IN4")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) //?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

INPUT_PORTS_END

static const gfx_layout bg2_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};

#define O 8*8*4
#define O2 2*O
static const gfx_layout sp2_layout =
{
	16, 16,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16, O+3, O+2, O+1, O+0, O+19, O+18, O+17, O+16 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, O2+0*32, O2+1*32, O2+2*32, O2+3*32, O2+4*32, O2+5*32, O2+6*32, O2+7*32 },
	8*8*4*4
};
#undef O
#undef O2

static const gfx_layout char_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};

static GFXDECODE_START( lastbank )
	GFXDECODE_ENTRY( "gfx1", 0, bg2_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, sp2_layout, 0, 16 )
	GFXDECODE_ENTRY( "maincpu",           0, char_layout,  0, 16 )  // Ram-based
GFXDECODE_END

static TIMER_DEVICE_CALLBACK( lastbank_irq_scanline )
{
	lastbank_state *state = timer.machine().driver_data<lastbank_state>();
	int scanline = param;

	if (scanline == 240 && (state->m_irq_enable & 4))
	{
		device_set_input_line_and_vector(state->m_maincpu, 0, HOLD_LINE, state->m_irq_vector[2]);
	}

	if (scanline == 0 && (state->m_irq_enable & 2))
	{
		device_set_input_line_and_vector(state->m_maincpu, 0, HOLD_LINE, state->m_irq_vector[1]);
	}
}

static MACHINE_CONFIG_START( lastbank, lastbank_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MASTER_CLOCK/4) //!!! TC0091LVC !!!
	MCFG_CPU_PROGRAM_MAP(lastbank_map)
	MCFG_CPU_IO_MAP(lastbank_io)
	MCFG_TIMER_ADD_SCANLINE("scantimer", lastbank_irq_scanline, "screen", 0, 1)

//  MCFG_CPU_ADD("audiocpu",Z80,MASTER_CLOCK/4)

	//MCFG_MACHINE_START(lastbank)
	//MCFG_MACHINE_RESET(lastbank)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(lastbank)


	MCFG_GFXDECODE( lastbank )
	MCFG_PALETTE_LENGTH(16)

	MCFG_VIDEO_START(lastbank)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	// es8712
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( lastbank )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "3.u9", 0x00000, 0x40000, CRC(f430e1f0) SHA1(dd5b697f5c2250d98911f4c7d3e7d4cc16b0b40f) )
	ROM_RELOAD(              0x10000, 0x40000 )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "8.u48", 0x00000, 0x10000, CRC(3a7bfe10) SHA1(7dc543e11d3c0b9872fcc622339ade25383a1eb3) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "5.u10", 0x00000, 0x20000, CRC(51f3c5a7) SHA1(73d4c8817fe96d75be32c43e816e93c52b5d2b27) )

	ROM_REGION( 0x200000, "essnd", 0 ) /* Samples */
	ROM_LOAD( "6.u55", 0x00000, 0x40000, CRC(9e78e234) SHA1(031f93e4bc338d0257fa673da7ce656bb1cda5fb) )
	ROM_LOAD( "7.u60", 0x40000, 0x80000, CRC(41be7146) SHA1(00f1c0d5809efccf888e27518a2a5876c4b633d8) )
ROM_END

GAME( 1994, lastbank,  0,   lastbank, lastbank,  0, ROT0, "Excellent Systems", "Last Bank", GAME_IS_SKELETON )
