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
	void chesskng_io(address_map &map);

	uint8_t unk_3f_r();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

uint8_t chesskng_state::unk_3f_r()
{
	return machine().rand();
}

void chesskng_state::chesskng_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram().share(m_mainram); // 2x SRM20256 RAM
	map(0x80000, 0x9ffff).rom().region("maincpu", 0x00000);

	// where does ROM address 0x20000 map?
	map(0xf0000, 0xfffff).rom().region("maincpu", 0x30000);
}

void chesskng_state::chesskng_io(address_map &map)
{
	map(0x3f, 0x3f).r(FUNC(chesskng_state::unk_3f_r));
}

uint32_t chesskng_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// quickly draw from memory (should be handled by LCDC?)
	int count = 0;

	for (int y = 0; y < 256; y++)
	{
		uint32_t *dst = &bitmap.pix(y);
		for (int x = 0; x < 256 / 8; x++)
		{
			// seem to be 2 256x256 images, one at c000, one at e000, maybe 2bpp graphics?
			uint8_t data = m_mainram[0xc000 + count];	
			uint8_t data2 = m_mainram[0xe000 + count];				

			for (int xx = 0; xx < 8; xx++)
			{
				uint8_t pix = (data >> (7 - xx)) & 1;
				pix |= ((data2 >> (7 - xx)) & 1)<<1;

				rgb_t pens[4] = { rgb_t::black(), rgb_t(0x40,0x40,0x40), rgb_t(0xc0,0xc0,0xc0),	rgb_t::white() };
				dst[x * 8 + xx] = pens[pix];
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
	m_maincpu->set_addrmap(AS_IO, &chesskng_state::chesskng_io);

	// Video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(256, 256); // unknown resolution
	m_screen->set_visarea(0, 256-1, 0, 256-1);
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
