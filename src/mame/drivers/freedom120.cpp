// license: BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Liberty Electronics Freedom 120/Aspect 100

    Serial terminal

    Hardware:
    - Z0840006PSC Z80 CPU
    - XTAL 32 Mhz and 48 MHz
    - MC2674B4P
    - 27C256 labeled "G212011 6A22"
    - M27C512 labeled "M2ASP11 C368"
    - KM6264BL-7L x2 (next to ROM)
    - MS6264L-70PC
    - V61C16P70L x2
    - SCN2681AC1N40
    - XTAL 3.6864 MHz
    - PAL labeled "10A8"
    - PAL labeled "PL10251"
    - PAL labeled "PL10151"
    - Motorola IC "LS38BC712PP01"
    - Battery

    External:
    - Serial port labeled "Main Port"
    - Parallel port labeled "Parallel Port"
    - Keyboard port

    Status: Skeleton driver

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class freedom120_state : public driver_device
{
public:
	freedom120_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void freedom120(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80_device> m_maincpu;

	void mem_map(address_map &map);
	void io_map(address_map &map);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void freedom120_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void freedom120_state::io_map(address_map &map)
{
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

static const gfx_layout char_layout =
{
	8, 16,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8 * 16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void freedom120_state::machine_start()
{
}

void freedom120_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void freedom120_state::freedom120(machine_config &config)
{
	Z80(config, m_maincpu, 48_MHz_XTAL / 8); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &freedom120_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &freedom120_state::io_map);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", "palette", chars);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( free120 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("m2asp11.u241", 0x0000, 0x10000, CRC(c079f471) SHA1(4864660c9a4470a4a444943fe89d91dc44297c39))

	ROM_REGION(0x8000, "chargen", 0)
	ROM_LOAD("g212011.u242", 0x0000, 0x8000, CRC(18e6700f) SHA1(fdf22d11468f978661005be115ec0bcc043519fc))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE     INPUT  CLASS             INIT        COMPANY                FULLNAME                  FLAGS
COMP( 1993, free120, 0,      0,      freedom120, 0,     freedom120_state, empty_init, "Liberty Electronics", "Freedom 120/Aspect 100", MACHINE_IS_SKELETON )
