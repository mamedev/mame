// license: BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Micro-Term 440

    VT240 compatible serial terminal

    Hardware for the terminal PCB:
    - Z8400APS Z80A CPU
    - 27256 EPROM "1965 M.P. R4.0 27256 U6"
    - 2764 EPROM "1965 M.P. R4.0 2764 U7"
    - D4364C-20L (work RAM?)
    - 2732 EPROM "1965 M.P. R4.0 2732 U9"
    - 2x NMC 9345N EEPROM
    - Z8430AB1 Z80ACTC
    - 2x Z8470AB1 Z80ADART
    - SCN2674B AVDC
    - 4x TMM2016BP-10 (VRAM?)
    - 1x M58725P
    - 2732 EPROM "MT425 CG REV 2.1"
    - 2716 EPROM "MT425 ATTR U25 REV1.1"
    - 4 MHz XTAL, 9.87768 MHz XTAL, 15.30072 MHz XTAL

    Hardware for the graphics PCB:
    - D8088-2 CPU
    - IM6402-1IJL
    - 27256 EPROM "1955 8088 R4.0 27256 U 58"
    - CDM6264E3
    - 2732 EPROM "1959 BLNKG R1.0 2732 U56"
    - 2732 EPROM "1959 CNTRL R1.0 2732 U75"
    - 24 MHz XTAL

    External:
    - RCA connector "Video Out"
    - DB9 connector "Printer"
    - DB25 connector "I/O"
    - RJ? connector "Keyboard"

    TODO:
    - Everything

    Notes:
    - The graphics PCB appears to be serially connected to the terminal PCB

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

#include "emupal.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mt440_state : public driver_device
{
public:
	mt440_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void mt440(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void mt440_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
}

void mt440_state::io_map(address_map &map)
{
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( mt440 )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

static const gfx_layout char_layout =
{
	8, 10,
	128,
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ 0*0x80*8, 1*0x80*8, 2*0x80*8, 3*0x80*8, 4*0x80*8, 5*0x80*8, 6*0x80*8, 7*0x80*8, 8*0x80*8, 9*0x80*8 },
	8
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("cg", 0x000, char_layout, 0, 1)
	GFXDECODE_ENTRY("cg", 0x800, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void mt440_state::machine_start()
{
}

void mt440_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void mt440_state::mt440(machine_config &config)
{
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mt440_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mt440_state::io_map);

	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", "palette", chars);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( mt440 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("1965_mp_r40.u6", 0x0000, 0x8000, CRC(cecda080) SHA1(0b40709a5f729c7d39a216fccee4b1ac7a59ad60))
	ROM_LOAD("1965_mp_r40.u7", 0x8000, 0x2000, CRC(798cceef) SHA1(6de7b536d756666c058662e067bb718629bccb39))
	ROM_LOAD("1965_mp_r40.u9", 0xa000, 0x1000, CRC(e39d145c) SHA1(79c4a8dd62398b6f2f6e7e8004ec42b2511016de))

	ROM_REGION(0x1000, "cg", 0)
	// bad? gfxdecode shows missing pixels
	ROM_LOAD("mt425_cg_rev21.u37", 0x0000, 0x1000, CRC(3ba34cf4) SHA1(dddddf69d736f04a17aae2019f47257484e88377))

	ROM_REGION(0x800, "attr", 0)
	ROM_LOAD("mt425_attr_rev11.u25", 0x000, 0x800, CRC(128a461a) SHA1(716cf85680b7cec8732a934c6f9eebd68c47c8d6))

	ROM_REGION(0x8000, "gfxcpu", 0)
	ROM_LOAD("1955_8088_r40.u58", 0x0000, 0x8000, CRC(836445cf) SHA1(540fcb2daeb9aebf474c213025641d3adb480697))

	ROM_REGION(0x2000, "gfxextra", 0)
	ROM_LOAD("1959_blnkg_r10.u56", 0x0000, 0x1000, CRC(a8f04ba4) SHA1(c37cc33eb426201d53e1c3b4e237a1dfd94f0f40))
	ROM_LOAD("1959_cntrl_r10.u75", 0x1000, 0x1000, CRC(098c8a37) SHA1(4dd3612d576809acfae1e62996dbc6ae4ece1ada))

	ROM_REGION(0x20, "gfxpal", 0)
	ROM_LOAD("1959_13.u3", 0x00, 0x20, CRC(ebeb6d51) SHA1(faa06769938d6832baeb904775162f8436eeba59))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY       FULLNAME          FLAGS
COMP( 1986, mt440, 0,      0,      mt440,   mt440, mt440_state, empty_init, "Micro-Term", "Micro-Term 440", MACHINE_IS_SKELETON )
