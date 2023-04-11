// license: BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Liberty Electronics Freedom 220 Video Display Terminal

    VT220 compatible serial terminal

    Hardware:
    - Z8400A Z80A
    - 4 MHz XTAL (next to CPU)
    - 3x 2764 (next to CPU)
    - 6x TMM2016AP-10 or D446C-2 (2k)
    - SCN2674B
    - SCB2675B
    - 2x 2732A (next to CRT controller)
    - 2x S68B10P (128 byte SRAM)
    - M5L8253P-5
    - 3x D8251AC
    - 18.432 MHz XTAL

    Keyboard:
    - SCN8050 (8039)
    - 2716 labeled "121"
    - UA555TC
    - Speaker

    External:
    - DB25 connector "Main Port"
    - DB25 connector "Auxialiary Port"
    - Keyboard connector
    - Expansion slot

    TODO:
    - Everything

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class freedom220_state : public driver_device
{
public:
	freedom220_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void freedom220(machine_config &config);

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

void freedom220_state::mem_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
}

void freedom220_state::io_map(address_map &map)
{
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

static const gfx_layout char_layout =
{
	8, 12,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8 * 16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void freedom220_state::machine_start()
{
}

void freedom220_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void freedom220_state::freedom220(machine_config &config)
{
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &freedom220_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &freedom220_state::io_map);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", "palette", chars);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( free220 )
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("m122010__8cdd.ic213", 0x0000, 0x2000, CRC(a1181809) SHA1(0ec0fd30c8a55f0bb9e1c6453120ab9a696f9041))
	ROM_LOAD("m222010__04c8.ic212", 0x2000, 0x2000, CRC(ddd1e5eb) SHA1(3e3998035721050cd2019474343f072dade6589d))
	ROM_LOAD("m322010__8121.ic214", 0x4000, 0x2000, CRC(eeaa4b44) SHA1(93402e00205d7220f5e248a902ed92de4bbe6dd8))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("g022010__d64e.bin", 0x0000, 0x1000, CRC(a4482adc) SHA1(98479f6396743da6cf23909ff5a0097e9f021e3b))

	ROM_REGION(0x1000, "unk", 0)
	ROM_LOAD("t022010__61f0.bin", 0x0000, 0x1000, CRC(00461116) SHA1(79a53a557ea4386b3e85a312731c6c0763ab46cc))

	ROM_REGION(0x800, "keyboard", 0)
	ROM_LOAD("121.bin", 0x000, 0x800, CRC(ee491f39) SHA1(477eb9f3d3abc89cfc9b5f9a924a794ca48750c4))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE     INPUT  CLASS             INIT        COMPANY                FULLNAME       FLAGS
COMP( 1984, free220, 0,      0,      freedom220, 0,     freedom220_state, empty_init, "Liberty Electronics", "Freedom 220", MACHINE_IS_SKELETON )
