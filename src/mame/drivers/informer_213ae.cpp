// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Informer 213 AE

    VT-100 compatible terminal

    Hardware:
    - EF68B09EP
	- 2x TC5564PL-15 + 1x TC5565APL
	- Z0853006PSC SCC
	- ASIC
	- 18.432 MHz XTAL

    TODO:
	- Figure out the ASIC and how it's connected

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/z80scc.h"
#include "emupal.h"
#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class informer_213ae_state : public driver_device
{
public:
	informer_213ae_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_scc(*this, "scc"),
		m_vram(*this, "vram"),
		m_aram(*this, "aram"),
		m_chargen(*this, "chargen")
	{ }

	void informer_213ae(machine_config &config);

protected:
	void machine_start() override;
	void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<scc8530_device> m_scc;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_aram;
	required_region_ptr<uint8_t> m_chargen;

	void mem_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void informer_213ae_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x6000, 0x6fff).ram().share("vram");
	map(0x7000, 0x7fff).ram().share("aram");
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( informer_213ae )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint32_t informer_213ae_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 51; y++)
	{
		for (int x = 0; x < 80; x++)
		{
			uint8_t code = m_vram[y * 80 + x];
//			uint8_t attr = m_aram[y * 80 + x];

			// draw 9 lines
			for (int i = 0; i < 9; i++)
			{
				uint8_t data = m_chargen[0x2000 | ((code << 4) + i)];

				// 8 pixels of the character
				bitmap.pix32(y * 9 + i, x * 8 + 0) = BIT(data, 7) ? rgb_t::white() : rgb_t::black();
				bitmap.pix32(y * 9 + i, x * 8 + 1) = BIT(data, 6) ? rgb_t::white() : rgb_t::black();
				bitmap.pix32(y * 9 + i, x * 8 + 2) = BIT(data, 5) ? rgb_t::white() : rgb_t::black();
				bitmap.pix32(y * 9 + i, x * 8 + 3) = BIT(data, 4) ? rgb_t::white() : rgb_t::black();
				bitmap.pix32(y * 9 + i, x * 8 + 4) = BIT(data, 3) ? rgb_t::white() : rgb_t::black();
				bitmap.pix32(y * 9 + i, x * 8 + 5) = BIT(data, 2) ? rgb_t::white() : rgb_t::black();
				bitmap.pix32(y * 9 + i, x * 8 + 6) = BIT(data, 1) ? rgb_t::white() : rgb_t::black();
				bitmap.pix32(y * 9 + i, x * 8 + 7) = BIT(data, 0) ? rgb_t::white() : rgb_t::black();
			}
		}
	}

	return 0;
}

static const gfx_layout char_layout =
{
	8,9,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8 },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void informer_213ae_state::machine_start()
{
}

void informer_213ae_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void informer_213ae_state::informer_213ae(machine_config &config)
{
	MC6809(config, m_maincpu, 18.432_MHz_XTAL / 4); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &informer_213ae_state::mem_map);

	SCC8530N(config, m_scc, 0); // unknown clock

	// video
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_size(640, 480);
	m_screen->set_visarea_full();
	m_screen->set_refresh_hz(60);
//	m_screen->set_raw(18.432_MHz_XTAL, 0, 0, 0, 0, 0, 0);
	m_screen->set_screen_update(FUNC(informer_213ae_state::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", m_palette, chars);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( in213ae )
	ROM_REGION(0x8000, "maincpu", 0)
	// 79750-304-213AE_V1.6_CK-B1B8
	ROM_LOAD("79750-304.bin", 0x0000, 0x8000, CRC(82ffe69e) SHA1(3803100aeb8f5e484bc9f4c533ef4f25223c9023))

	ROM_REGION(0x4000, "chargen", 0)
	// 79747-002  V.32 ME C.G.  V3.1 CK=D68C (checksum matches)
	ROM_LOAD("79747-002.bin", 0x0000, 0x4000, CRC(7425327f) SHA1(e3e67305b3b8936683724d1347a451fffe96bf0e))
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT   COMPAT  MACHINE         INPUT           CLASS                 INIT        COMPANY     FULLNAME          FLAGS
COMP( 1992, in213ae, 0,       0,      informer_213ae, informer_213ae, informer_213ae_state, empty_init, "Informer", "Informer 213 AE", MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )
