// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

マイコン麻雀 - Micom Mahjong, mail-order Mahjong console.

Hardware notes:
- Z80, 11.0592MHz XTAL
- 16KB ROM (4*2732), 1KB RAM
- 2KB ROM (2716) for tiles, 1KB VRAM, 1bpp video
- 1-bit sound

TODO:
- map the keypad

******************************************************************************/

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
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_vram;
	required_device<tilemap_device> m_tilemap;
	required_device<screen_device> m_screen;
	optional_device<dac_bit_interface> m_dac;
	required_ioport_array<4> m_inputs;

	TILE_GET_INFO_MEMBER(get_tile_info) { tileinfo.set(0, m_vram[tile_index], 0, 0); }
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);

	u8 input_r();
	void vram_w(offs_t offset, u8 data);

	void input_w(u8 data);
	void sound_w(u8 data);

	u8 m_inp_mux = 0;
};

void mmahjong_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}



/******************************************************************************
    I/O
******************************************************************************/

u32 mmahjong_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect);
	return 0;
}

void mmahjong_state::vram_w(offs_t offset, u8 data)
{
	m_vram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void mmahjong_state::input_w(u8 data)
{
	// d0-d3: input mux
	m_inp_mux = ~data;
}

void mmahjong_state::sound_w(u8 data)
{
	// d0: speaker out
	m_dac->write(data & 1);
}

u8 mmahjong_state::input_r()
{
	u8 data = 0;

	// d0-d7: multiplexed inputs
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return ~data;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void mmahjong_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom();
	map(0x5000, 0x53ff).ram();
	map(0x6000, 0x63ff).w(FUNC(mmahjong_state::vram_w)).share("vram");

	map(0x7001, 0x7001).r(FUNC(mmahjong_state::input_r));
	map(0x7002, 0x7002).w(FUNC(mmahjong_state::input_w));
	map(0x7004, 0x7004).w(FUNC(mmahjong_state::sound_w));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( mmahjong )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA)
INPUT_PORTS_END


static GFXDECODE_START( gfx_mmahjong )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x1, 0, 1 )
GFXDECODE_END



/******************************************************************************
    Machine Configs
******************************************************************************/

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



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( mmahjong )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "ms-1", 0x0000, 0x1000, CRC(a1607ac8) SHA1(4e0d7a0482c7619ef25b12a7e02f5d03bea8ce6f) )
	ROM_LOAD( "ms-2", 0x1000, 0x1000, CRC(cb1cab68) SHA1(88dc4808126528a269edf742062fa12e902be324) )
	ROM_LOAD( "ms-3", 0x2000, 0x1000, CRC(2fdd4f55) SHA1(a9246239144c41fd38bd42015552b5afab40e55a) )
	ROM_LOAD( "ms-4", 0x3000, 0x1000, CRC(cc550e36) SHA1(d66750ce6ddf6e4db4e5bd46a639494d8335a590) )

	ROM_REGION( 0x800, "tiles", 0 )
	ROM_LOAD( "ms-a", 0x000, 0x800, CRC(d1dfe5c1) SHA1(5042b89555867db418f4aeef6b520619d8f533f2) )
ROM_END

} // anonymous namespace

/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE  INPUT  STATE      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1982, mmahjong,      0,      0, mmahjong,     mmahjong,   mmahjong_state, empty_init, "Nippon Mail Service", "Micom Mahjong", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
