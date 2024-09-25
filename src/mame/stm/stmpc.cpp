// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    STM PC

    All-in-One IBM PC compatible portable computer

    Hardware (incomplete):
    - Intel 186 at 8 MHz
    - 256 KB RAM (expandable to 512 KB)
    - SCN2674B
    - NEC D8251AC
    - Monochrome LCD display with 80x16 lines (512x128 resolution)
    - External monitor support (RGB, composite)
    - Canon MDD413 dual 5.25" QD drive
    - SCSI interface
    - 2x RS232 port
    - Centronics port
    - Internal thermal printer
    - 83-key keyboard with RJ-11 connector
    - OS: MS-DOS 2.11

    TODO:
    - Everything

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/clock.h"
#include "video/scn2674.h"
#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class stmpc_state : public driver_device
{
public:
	stmpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_avdc(*this, "avdc")
	{ }

	void stmpc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void char_map(address_map &map) ATTR_COLD;

	required_device<i80186_cpu_device> m_maincpu;
	required_device<scn2674_device> m_avdc;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void stmpc_state::mem_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
	map(0xb8000, 0xbffff).ram();
	map(0xfc000, 0xfffff).rom().region("maincpu", 0);
}

void stmpc_state::io_map(address_map &map)
{
	map(0x0700, 0x070f).rw(m_avdc, FUNC(scn2674_device::read), FUNC(scn2674_device::write)).umask16(0x00ff);
}

void stmpc_state::char_map(address_map &map)
{
	map(0x0000, 0xffff).nopr();
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( stmpc )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

static const gfx_layout char_layout_8x8 =
{
	8, 8,
	RGN_FRAC(1, 4),
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	8 * 8
};

static const gfx_layout char_layout_8x10 =
{
	8, 10,
	RGN_FRAC(1, 4),
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ RGN_FRAC(1,4) + 0*8, RGN_FRAC(1,4) + 1*8, RGN_FRAC(1,4) + 2*8, RGN_FRAC(1,4) + 3*8,
	  RGN_FRAC(1,4) + 4*8, RGN_FRAC(1,4) + 5*8, RGN_FRAC(1,4) + 6*8, RGN_FRAC(1,4) + 7*8,
	  RGN_FRAC(3,4) + 6*8, RGN_FRAC(3,4) + 7*8 },
	8 * 8
};

static const gfx_layout char_layout_8x14 =
{
	8, 14,
	RGN_FRAC(1, 4),
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ RGN_FRAC(2,4) + 0*8, RGN_FRAC(2,4) + 1*8, RGN_FRAC(2,4) + 2*8, RGN_FRAC(2,4) + 3*8,
	  RGN_FRAC(2,4) + 4*8, RGN_FRAC(2,4) + 5*8, RGN_FRAC(2,4) + 6*8, RGN_FRAC(2,4) + 7*8,
	  RGN_FRAC(3,4) + 0*8, RGN_FRAC(3,4) + 1*8, RGN_FRAC(3,4) + 2*8, RGN_FRAC(3,4) + 3*8,
	  RGN_FRAC(3,4) + 4*8, RGN_FRAC(3,4) + 5*8 },
	8 * 8
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout_8x8, 0, 1)
	GFXDECODE_ENTRY("chargen", 0, char_layout_8x10, 0, 1)
	GFXDECODE_ENTRY("chargen", 0, char_layout_8x14, 0, 1)
GFXDECODE_END

SCN2674_DRAW_CHARACTER_MEMBER(stmpc_state::draw_character)
{
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void stmpc_state::machine_start()
{
}

void stmpc_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void stmpc_state::stmpc(machine_config &config)
{
	I80186(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &stmpc_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &stmpc_state::io_map);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", "palette", chars);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_raw(16000000, 1344, 0, 1280, 113, 0, 104);
	screen.set_screen_update(m_avdc, FUNC(scn2674_device::screen_update));

	CLOCK(config, "tmrin1", 20000).signal_handler().set(m_maincpu, FUNC(i80186_cpu_device::tmrin1_w)); // FIXME: figure out the actual source of this

	SCN2674(config, m_avdc, 16000000 / 8);
	m_avdc->set_screen("screen");
	m_avdc->set_character_width(8);
	m_avdc->set_addrmap(0, &stmpc_state::char_map);
	m_avdc->set_display_callback(FUNC(stmpc_state::draw_character));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( stmpc )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD16_BYTE("200000890_rev_2_even.u98", 0x00000, 0x2000, CRC(5e6b9212) SHA1(4790d4adee408ba85fcb6b1c52d59c2a5217a25a))
	ROM_LOAD16_BYTE("200000889_rev_2_odd.u126", 0x00001, 0x2000, CRC(90b12ca0) SHA1(19a775562c42b3e9972d4f156a3538b70e3671e8))

	ROM_REGION(0x2000, "chargen", 0)
	 // might be a BAD_DUMP, missing data compared to rev a below?
	ROM_LOAD("200000447_rev_b.u50", 0x0000, 0x2000, CRC(15f3a2d1) SHA1(cf061b596cbec96195ea6224ab6295f4df1fabeb))
ROM_END

// The dumper notes that this might be a prototype or early field trial version
ROM_START( stmpcp )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD16_BYTE("200000446_rev_e_even.u98", 0x00000, 0x2000, CRC(168cdf3e) SHA1(a2d7524ceabbaeca1f1bb17b015ddb42946bcf34))
	ROM_LOAD16_BYTE("200000445_rev_e_odd.u126", 0x00001, 0x2000, CRC(9d86f7ed) SHA1(7367697505f256cd82569865c39b208fe16a233f))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("200000447_rev_a.u50", 0x0000, 0x2000, CRC(4a6001a2) SHA1(36c10195f0655bdc8e10ceee06d176cebeb7383d))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY            FULLNAME               FLAGS
COMP( 1984, stmpc,  0,      0,      stmpc,   stmpc, stmpc_state, empty_init, "STM Electronics", "STM PC",              MACHINE_IS_SKELETON )
COMP( 1984, stmpcp, stmpc,  0,      stmpc,   stmpc, stmpc_state, empty_init, "STM Electronics", "STM PC (prototype?)", MACHINE_IS_SKELETON )
