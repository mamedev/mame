// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "screen.h"
#include "speaker.h"

#include "cpu/nec/nec.h"

class chesskng_state : public driver_device
{
public:
	chesskng_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_mainram(*this, "mainram")
	{ }

	void chesskng(machine_config &config);

private:
	// Devices
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_mainram;

	void chesskng_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

void chesskng_state::chesskng_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram().share(m_mainram); // 2x SRM20256 RAM
	map(0xc0000, 0xfffff).rom().region("maincpu", 0x00000);
}

uint32_t chesskng_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// quickly draw from memory (should be handled by LCDC?)
	int count = 0;

	for (int y = 0; y < 512; y++)
	{
		uint32_t *dst = &bitmap.pix(y);
		for (int x = 0; x < 256 / 8; x++)
		{
			// seem to be 2 256x256 images, one at c000, one at d000, maybe 2bpp graphics?
			uint8_t data = m_mainram[0xc000 + count];	

			for (int xx = 0; xx < 8; xx++)
			{
				uint8_t pix = (data >> (7 - xx)) & 1;

				dst[x * 8 + xx] = pix ? rgb_t::white() : rgb_t::black();
			}

			count++;
		}
	}
	return 0;
}

static INPUT_PORTS_START( chesskng )
	// UP,DOWN,LEFT,RIGHT,A,B,START,SELECT
INPUT_PORTS_END

void chesskng_state::chesskng(machine_config &config)
{
	// Basic machine hardware
	V20(config, m_maincpu, 9600000); // D70108HG-10 V20, Unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &chesskng_state::chesskng_map);

	// Video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(256, 512); // unknown resolution
	m_screen->set_visarea(0, 256-1, 0, 512-1);
	m_screen->set_screen_update(FUNC(chesskng_state::screen_update));

	// There are 2x HD66204F (LCDC)
	// and 1x HD66205F (LCDC)
	// then another with a sticker over the part (probably another HD620xF?)
	// 16160
	// S2RB
	// 94.10

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	// Has a cartridge slot
}

ROM_START( chesskng )
	ROM_REGION( 0x040000, "maincpu", 0 )
	ROM_LOAD( "chess_king_etmate-cch.bin", 0x000000, 0x040000, CRC(a4d1764b) SHA1(ccfae1e985f6ad316ff192206fbc0f8bcd4e44d5) )

	// there is also a CCH01 ET-MATE F3X0 713 near the CPU, what is it?
ROM_END

CONS( 1994, chesskng,         0, 0, chesskng, chesskng, chesskng_state, empty_init, "I-Star Co.,Ltd", "Chess King (Model ET-6)", MACHINE_IS_SKELETON )
