// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

マイコン麻雀 - Micom Mahjong, mail-order Mahjong console.

Two models are known, a grey one and a white one, hardware is presumed to be
the same. It's not known who developed/manufactured it. Probably not the
mail order house, but they can be considered the publisher.

The options at the start of the game:
Option 1: 1: 3 minutes play time, 2: 5 minutes
Option 2: 1: advanced difficulty, 2: beginner

Hardware notes:
- PCB label: IFVC-3224A
- Zilog Z8400A or Sharp LH0080A, 11.0592MHz XTAL
- 16KB ROM (4*2732), 1KB RAM (2*MSM2114L)
- 2KB ROM (2716) for tiles, 1KB VRAM (2*MSM2114L), 1bpp video
- 1-bit sound

TODO:
- video timing, maybe 11059200 / 2 / (262*352)?

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class mmahjong_state : public driver_device
{
public:
	mmahjong_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vram(*this, "vram"),
		m_tilemap(*this, "tilemap"),
		m_screen(*this, "screen"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void mmahjong(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_vram;
	required_device<tilemap_device> m_tilemap;
	required_device<screen_device> m_screen;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<3> m_inputs;

	u8 m_inp_matrix = 0;

	TILE_GET_INFO_MEMBER(get_tile_info) { tileinfo.set(0, m_vram[tile_index], 0, 0); }
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;

	void vram_w(offs_t offset, u8 data);
	void input_w(u8 data);
	u8 input_r();
	void sound_w(u8 data);
};

void mmahjong_state::machine_start()
{
	save_item(NAME(m_inp_matrix));
}



/*******************************************************************************
    Video
*******************************************************************************/

u32 mmahjong_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect);
	return 0;
}

static GFXDECODE_START( gfx_mmahjong )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x1, 0, 1 )
GFXDECODE_END



/*******************************************************************************
    I/O
*******************************************************************************/

void mmahjong_state::vram_w(offs_t offset, u8 data)
{
	m_vram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void mmahjong_state::input_w(u8 data)
{
	// d0-d2: input matrix
	// d3 is also written, but unused
	m_inp_matrix = data;
}

u8 mmahjong_state::input_r()
{
	u8 data = 0xff;

	// read keypad
	for (int i = 0; i < 3; i++)
		if (!BIT(m_inp_matrix, i))
			data &= m_inputs[i]->read();

	return data;
}

void mmahjong_state::sound_w(u8 data)
{
	// d0: speaker out
	m_dac->write(data & 1);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void mmahjong_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x5000, 0x53ff).ram();
	map(0x6000, 0x63ff).w(FUNC(mmahjong_state::vram_w)).share(m_vram);
	map(0x7001, 0x7001).r(FUNC(mmahjong_state::input_r));
	map(0x7002, 0x7002).w(FUNC(mmahjong_state::input_w));
	map(0x7004, 0x7004).w(FUNC(mmahjong_state::sound_w));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( mmahjong )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A) // 1
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B) // 2
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C) // 3
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D) // 4
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E) // 5
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_F) // 6
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_G) // 7
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H) // 8
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I) // 9
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J) // 10
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K) // 11
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_L) // 12
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M) // 13
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N) // 0 (Tsumo)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_MAHJONG_RON)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void mmahjong_state::mmahjong(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 11.0592_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmahjong_state::main_map);

	// video hardware
	GFXDECODE(config, "gfxdecode", "palette", gfx_mmahjong);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	TILEMAP(config, m_tilemap, "gfxdecode", 0, 8, 8, TILEMAP_SCAN_ROWS, 32, 32).set_info_callback(FUNC(mmahjong_state::get_tile_info));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(8*32, 8*32);
	m_screen->set_visarea(0, 8*32-1, 0, 8*24-1);
	m_screen->set_screen_update(FUNC(mmahjong_state::screen_update));
	m_screen->set_palette("palette");

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( mmahjong )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "ms_1.10", 0x0000, 0x1000, CRC(a1607ac8) SHA1(4e0d7a0482c7619ef25b12a7e02f5d03bea8ce6f) )
	ROM_LOAD( "ms_2.9",  0x1000, 0x1000, CRC(cb1cab68) SHA1(88dc4808126528a269edf742062fa12e902be324) )
	ROM_LOAD( "ms_3.8",  0x2000, 0x1000, CRC(2fdd4f55) SHA1(a9246239144c41fd38bd42015552b5afab40e55a) )
	ROM_LOAD( "ms_4.7",  0x3000, 0x1000, CRC(cc550e36) SHA1(d66750ce6ddf6e4db4e5bd46a639494d8335a590) )

	ROM_REGION( 0x800, "tiles", 0 )
	ROM_LOAD( "ms_a.2", 0x000, 0x800, CRC(d1dfe5c1) SHA1(5042b89555867db418f4aeef6b520619d8f533f2) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1982, mmahjong, 0,      0,      mmahjong, mmahjong, mmahjong_state, empty_init, "Nippon Mail Service", "Micom Mahjong", MACHINE_SUPPORTS_SAVE )
