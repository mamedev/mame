// license:BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Termtek TK-635 terminal

    Hardware:
    - AMD N80C188-25
    - TERMTEK TKA-200
    - 128k + 32k RAM
    - 256 KB flash memory

    Features:
    - 31.5khz or 48.1khz horizontal
    - 70/72 hz vertical
    - 16 background/foreground/border colors
    - 24x80/132, 25x80/132, 42x80/132, 43x80/132
    - Standard PC/AT keyboard

    Notes:
    - Identical to the Qume QVT-72 Plus?

    TODO:
    - Everything

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "emupal.h"
#include "screen.h"


namespace {

class tk635_state : public driver_device
{
public:
	tk635_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_charram(*this, "charram"),
		m_nmi_enable(false),
		m_voffset(0)
	{ }

	void tk635(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_ram;
	required_shared_ptr<u8> m_charram;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);

	uint8_t unk_00_r();
	void voffset_lsb_w(uint8_t data);
	void voffset_msb_w(uint8_t data);
	uint8_t unk_11_r();
	uint8_t unk_19_r();
	void unk_f0_w(uint8_t data);

	bool m_nmi_enable;
	uint16_t m_voffset;
};

void tk635_state::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram().share("ram");
	map(0x20000, 0x23fff).ram().share("charram");
	map(0xc0000, 0xfffff).rom().region("maincpu", 0);
}

void tk635_state::io_map(address_map &map)
{
	map(0x00, 0x00).r(FUNC(tk635_state::unk_00_r));
	map(0x04, 0x04).w(FUNC(tk635_state::voffset_lsb_w));
	map(0x05, 0x05).w(FUNC(tk635_state::voffset_msb_w));
	map(0x11, 0x11).r(FUNC(tk635_state::unk_11_r));
//  map(0x13, 0x13).ram(); // host port
	map(0x19, 0x19).r(FUNC(tk635_state::unk_19_r));
//  map(0x1b, 0x1b).ram(); // aux port
	map(0xf0, 0xf0).w(FUNC(tk635_state::unk_f0_w));
}

static INPUT_PORTS_START( tk635 )
INPUT_PORTS_END

static const gfx_layout char_layout_8x16 =
{
	8, 16,
	512,
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	8*32
};

static GFXDECODE_START(chars)
	GFXDECODE_RAM("charram", 0, char_layout_8x16, 0, 1)
GFXDECODE_END

uint32_t tk635_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 26; y++)
	{
		for (int x = 0; x < 132; x++)
		{
			uint8_t code = m_ram[(0x10000 | m_voffset) + (y * 132) + x];

			// draw 16 lines
			for (int i = 0; i < 16; i++)
			{
				uint8_t data = m_charram[(code << 5) + i];

				// 8 pixels of the character
				for (int p = 0; p < 8; p++)
					bitmap.pix(y * 16 + i, x * 9 + p) = BIT(data, 7 - p) ? rgb_t::white() : rgb_t::black();

				// 9th pixel empty
				bitmap.pix(y * 16 + i, x * 9 + 8) = rgb_t::black();
			}
		}
	}

	return 0;
}

void tk635_state::vblank_w(int state)
{
	if (state == 1 && m_nmi_enable)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

uint8_t tk635_state::unk_00_r()
{
	logerror("unk_00_r: %02x\n", 0x10);
	return 0x10;
}

void tk635_state::voffset_lsb_w(uint8_t data)
{
	m_voffset = (m_voffset & 0xff00) | data;
}

void tk635_state::voffset_msb_w(uint8_t data)
{
	m_voffset = (data << 8) | (m_voffset & 0x00ff);
}

uint8_t tk635_state::unk_11_r()
{
	logerror("unk_11_r: %02x\n", 0x09);
	return 0x09;
}

uint8_t tk635_state::unk_19_r()
{
	logerror("unk_19_r: %02x\n", 0x09);
	return 0x09;
}

void tk635_state::unk_f0_w(uint8_t data)
{
	logerror("unk_f0_w: %02x\n", data);
	m_nmi_enable = true;
}

void tk635_state::machine_start()
{
	// register for save states
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_voffset));
}

void tk635_state::machine_reset()
{
	m_nmi_enable = false;
}

void tk635_state::tk635(machine_config &config)
{
	I80188(config, m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &tk635_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &tk635_state::io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(70);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_screen_update(FUNC(tk635_state::screen_update));
	screen.set_size(1188, 416);
	screen.set_visarea(0, 1188-1, 0, 416-1);
	screen.screen_vblank().set(FUNC(tk635_state::vblank_w));

	PALETTE(config, "palette").set_entries(16);

	GFXDECODE(config, "gfxdecode", "palette", chars);
}

ROM_START( tk635 )
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("fw_v0_23.bin", 0x00000, 0x40000, CRC(bec6fdae) SHA1(37dc46f6b761d874bd1627a1137bc4082e364698))
ROM_END

} // anonymous namespace


COMP( 199?, tk635, 0, 0, tk635, tk635, tk635_state, empty_init, "Termtek", "TK-635", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
