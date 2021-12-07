// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Human Designed Systems HDS200

    ANSI/DEC-compatbile terminal

    Hardware:
    - Z80A (Z8400APS)
    - Z80A DMA (Z8410APS)
    - 2x SCN2681A
    - SCN2674B
    - 2x TMM2016BP-90 (2k)
    - 1x TMM2016AP-10 (2k)
    - MK48Z02B-25 (2k)
	- XTAL 3.6864 MHz (next go DUARTs)
	- XTAL 8 MHz (CPU)
	- XTAL 22.680 MHz and 35.640 MHz (video)

    TODO:
    - Everything

    Notes:
    - 

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/mc68681.h"
#include "machine/z80dma.h"
#include "machine/nvram.h"
#include "video/scn2674.h"
#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class hds200_state : public driver_device
{
public:
	hds200_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_avdc(*this, "avdc")
	{ }

	void hds200(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<z80_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<scn2674_device> m_avdc;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void hds200_state::mem_map(address_map &map)
{
	map(0x0000, 0x5fff).rom().region("maincpu", 0);
	map(0x6000, 0x7fff).ram();
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).ram();
}

void hds200_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x60, 0x67).rw(m_avdc, FUNC(scn2674_device::read), FUNC(scn2674_device::write));
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

static const gfx_layout char_layout =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void hds200_state::machine_start()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void hds200_state::hds200(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &hds200_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &hds200_state::io_map);

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	Z80DMA(config, "z80dma", 8_MHz_XTAL / 2); // divider not verified

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::amber());
	m_screen->set_raw(22.680_MHz_XTAL, 1008, 0, 720, 375, 0, 350); // 80-column mode
	m_screen->set_screen_update(m_avdc, FUNC(scn2674_device::screen_update));

	PALETTE(config, "palette", palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", "palette", chars);

	SCN2674(config, m_avdc, 22.680_MHz_XTAL / 9);
	m_avdc->set_character_width(9);
	m_avdc->set_screen("screen");

	SCN2681(config, "duart1", 3.6864_MHz_XTAL);

	SCN2681(config, "duart2", 3.6864_MHz_XTAL);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( hds200 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("u78.bin", 0x0000, 0x2000, CRC(518cfeb7) SHA1(3c214dede545a2a991fdd77311b3474b01b2123f))
	ROM_LOAD("u79.bin", 0x2000, 0x2000, CRC(3a765c8a) SHA1(8ffb5fb07b086ac725f22c2643ecd2e061130b57))
	ROM_LOAD("u80.bin", 0x4000, 0x2000, CRC(f72dfeeb) SHA1(7e09b8f0df8384f6b5c4d29cd59fa31f743de8b8))
	ROM_LOAD("u81.bin", 0x6000, 0x2000, CRC(b3f430be) SHA1(dd5503de46c7f00f2e376104dff13224026f5870))

	ROM_REGION(0x2000, "chargen", ROMREGION_INVERT)
	ROM_LOAD("u56.bin", 0x0000, 0x2000, CRC(cd268bff) SHA1(42f2aa3f51ae53e5cbcb57f974e99b24bca5f56f))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT   COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY         FULLNAME            FLAGS
COMP( 198?, hds200, 0,       0,      hds200,  0,     hds200_state, empty_init, "Human Designed Systems", "HDS200", MACHINE_IS_SKELETON )
