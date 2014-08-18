/*******************************************************************************

  PINBALL
  LTD (Brazil)

  Not much info available for these machines.
  Used PinMAME as a reference.

ToDo:
- Everything


********************************************************************************/

#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "ltd.lh"

class ltd_state : public genpin_class
{
public:
	ltd_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "nvram")
	{ }

	DECLARE_DRIVER_INIT(ltd);
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_r);
private:
	UINT8 m_out_offs;
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_p_ram;
};


static ADDRESS_MAP_START( ltd3_map, AS_PROGRAM, 8, ltd_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM AM_SHARE("nvram") // internal to the cpu
	AM_RANGE(0x0080, 0x00ff) AM_READ(io_r)
	AM_RANGE(0x0800, 0x2fff) AM_WRITE(io_w)
	AM_RANGE(0xc000, 0xcfff) AM_ROM AM_MIRROR(0x3000) AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ltd4_map, AS_PROGRAM, 8, ltd_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	//AM_RANGE(0x0800, 0x0800) AM_WRITE(cycle_reset_w)
	//AM_RANGE(0x0c00, 0x0c00) AM_WRITE(ay8910_1_reset)
	//AM_RANGE(0x1000, 0x1000) AM_WRITE(ay8910_0_ctrl_w)
	//AM_RANGE(0x1400, 0x1400) AM_WRITE(ay8910_0_reset)
	//AM_RANGE(0x1800, 0x1800) AM_WRITE(ay8910_1_ctrl_w)
	//AM_RANGE(0x2800, 0x2800) AM_WRITE(auxlamps_w)
	//AM_RANGE(0x3000, 0x3000) AM_WRITE(ay8910_0_data_w)
	//AM_RANGE(0x3800, 0x3800) AM_WRITE(ay8910_1_data_w)
	AM_RANGE(0xc000, 0xdfff) AM_ROM AM_MIRROR(0x2000) AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ltd4_io, AS_IO, 8, ltd_state )
	//AM_RANGE(0x0100, 0x0100) AM_READWRITE
	//AM_RANGE(0x0101, 0x0101) AM_WRITE(
ADDRESS_MAP_END

static INPUT_PORTS_START( ltd )
INPUT_PORTS_END

// switches
READ8_MEMBER( ltd_state::io_r )
{
	return 0;
}

// Lamps
WRITE8_MEMBER( ltd_state::io_w )
{
	offset >>= 10; // reduces offsets to 1 per bank
}

void ltd_state::machine_reset()
{
	m_out_offs = 0;
}

DRIVER_INIT_MEMBER( ltd_state, ltd )
{
}

TIMER_DEVICE_CALLBACK_MEMBER( ltd_state::timer_r )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	m_out_offs++;

	if (m_out_offs < 0x40)
	{
		UINT8 display = (m_out_offs >> 3) & 7;
		UINT8 digit = m_out_offs & 7;
		output_set_digit_value(display * 10 + digit, patterns[m_p_ram[m_out_offs]&15]);
	}
	else
	if (m_out_offs == 0x4a) // outhole
	{
		if (BIT(m_p_ram[m_out_offs], 0))
			m_samples->start(0, 5);
	}
	else
	if (m_out_offs == 0x4b) // knocker (not strapids)
	{
		if (BIT(m_p_ram[m_out_offs], 0))
			m_samples->start(0, 6);
	}
}

static MACHINE_CONFIG_START( ltd3, ltd_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6802, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(ltd3_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_ltd)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_r", ltd_state, timer_r, attotime::from_hz(500))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ltd4, ltd_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6803, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(ltd4_map)
	MCFG_CPU_IO_MAP(ltd4_io)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_ltd)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Atlantis
/-------------------------------------------------------------------*/
ROM_START(atla_ltd)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("atlantis.bin", 0x0000, 0x0800, CRC(c61be043) SHA1(e6c4463f59a5743fa34aa55beeb6f536ad9f1b56))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Black Hole
/-------------------------------------------------------------------*/
ROM_START(bhol_ltd)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("blackhol.bin", 0x0000, 0x0800, CRC(9f6ae35e) SHA1(c17bf08a41c6cf93550671b0724c58e8ac302c33))
	ROM_RELOAD(0x0800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Zephy
/-------------------------------------------------------------------*/
ROM_START(zephy)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("zephy.l2", 0x0000, 0x1000, CRC(8dd11287) SHA1(8133d0c797eb0fdb56d83fc55da91bfc3cddc9e3))
ROM_END

/*-------------------------------------------------------------------
/ Cowboy Eight Ball
/-------------------------------------------------------------------*/
ROM_START(cowboy)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("cowboy_l.bin", 0x0000, 0x1000, CRC(87befe2a) SHA1(93fdf40b10e53d7d95e5dc72923b6be887411fc0))
	ROM_LOAD("cowboy_h.bin", 0x1000, 0x1000, CRC(105e5d7b) SHA1(75edeab8c8ba19f334479133802acbc25f405763))
ROM_END

/*-------------------------------------------------------------------
/ Mr. & Mrs. Pec-Men
/-------------------------------------------------------------------*/
ROM_START(pecmen)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("pecmen_l.bin", 0x0000, 0x1000, CRC(f86c724e) SHA1(635ec94a1c6e77800ef9774102cc639be86c4261))
	ROM_LOAD("pecmen_h.bin", 0x1000, 0x1000, CRC(013abca0) SHA1(269376af92368d214c3d09ec6d3eb653841666f3))
ROM_END

/*-------------------------------------------------------------------
/ Al Capone
/-------------------------------------------------------------------*/
ROM_START(alcapone)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("alcapo_l.bin", 0x0000, 0x1000, CRC(c4270ba8) SHA1(f3d80af9900c94df2d43f2755341a346a0b64c87))
	ROM_LOAD("alcapo_h.bin", 0x1000, 0x1000, CRC(279f766d) SHA1(453c58e44c4ef8f1f9eb752b6163c61ebed70b27))
ROM_END

/*-------------------------------------------------------------------
/ Columbia
/-------------------------------------------------------------------*/
ROM_START(columbia)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("columb-d.bin", 0x0000, 0x1000, NO_DUMP)
	ROM_LOAD("columb-e.bin", 0x1000, 0x1000, CRC(013abca0) SHA1(269376af92368d214c3d09ec6d3eb653841666f3))
ROM_END

// system 3
GAME(1981, atla_ltd, 0,  ltd3,  ltd,  ltd_state, ltd, ROT0, "LTD", "Atlantis (LTD)",     GAME_IS_SKELETON_MECHANICAL)
GAME(1981, bhol_ltd, 0,  ltd3,  ltd,  ltd_state, ltd, ROT0, "LTD", "Black Hole (LTD)",   GAME_IS_SKELETON_MECHANICAL)
GAME(1981, zephy,    0,  ltd3,  ltd,  ltd_state, ltd, ROT0, "LTD", "Zephy",              GAME_IS_SKELETON_MECHANICAL)

// system 4
GAME(1981, cowboy,   0,  ltd4,  ltd,  ltd_state, ltd, ROT0, "LTD", "Cowboy Eight Ball",  GAME_IS_SKELETON_MECHANICAL)
GAME(1981, pecmen,   0,  ltd4,  ltd,  ltd_state, ltd, ROT0, "LTD", "Mr. & Mrs. Pec-Men", GAME_IS_SKELETON_MECHANICAL)
GAME(1981, alcapone, 0,  ltd4,  ltd,  ltd_state, ltd, ROT0, "LTD", "Al Capone",          GAME_IS_SKELETON_MECHANICAL)
GAME(1982, columbia, 0,  ltd4,  ltd,  ltd_state, ltd, ROT0, "LTD", "Columbia",           GAME_IS_SKELETON_MECHANICAL)
