// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Aero Fighters (newer hardware type)

***************************************************************************/

#include "emu.h"

#include "vs9209.h"
#include "vsystem_spr.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/mb3773.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class aerofgt_state : public driver_device
{
public:
	aerofgt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_soundlatch(*this, "soundlatch")
		, m_spr(*this, "vsystem_spr")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_vram(*this, "vram.%u", 0)
		, m_rasterram(*this, "rasterram")
		, m_sprlookupram(*this, "sprlookupram")
		, m_spriteram(*this, "spriteram")
		, m_soundbank(*this, "soundbank")
	{ }

	void aerofgt(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// handlers
	template <int Layer> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <int Layer> void scrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void setbank(int layer, int num, int bank);
	void gfxbank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t tile_callback(uint32_t code);
	void soundlatch_pending_w(int state);
	void sh_bankswitch_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;

	// devices referenced above
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<vsystem_spr_device> m_spr;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr_array<uint16_t, 2> m_vram;
	required_shared_ptr<uint16_t> m_rasterram;
	required_shared_ptr<uint16_t> m_sprlookupram;
	required_shared_ptr<uint16_t> m_spriteram;

	required_memory_bank m_soundbank;

	// video-related
	tilemap_t   *m_tilemap[2]{};
	uint8_t     m_gfxbank[8]{};
	uint16_t    m_bank[4]{};
	uint16_t    m_scrolly[2]{};
};


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

template <int Layer>
TILE_GET_INFO_MEMBER(aerofgt_state::get_tile_info)
{
	const uint16_t code = m_vram[Layer][tile_index];
	const int bank = (Layer << 2) | (code & 0x1800) >> 11;
	tileinfo.set(Layer,
			(code & 0x07ff) | (m_gfxbank[bank] << 11),
			(code & 0xe000) >> 13,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/


void aerofgt_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aerofgt_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aerofgt_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap[1]->set_transparent_pen(15);

	save_item(NAME(m_gfxbank));
	save_item(NAME(m_bank));
}

uint32_t aerofgt_state::tile_callback(uint32_t code)
{
	return m_sprlookupram[code&0x7fff];
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void aerofgt_state::setbank(int layer, int num, int bank)
{
	if (m_gfxbank[num] != bank)
	{
		m_gfxbank[num] = bank;
		m_tilemap[layer]->mark_all_dirty();
	}
}

void aerofgt_state::gfxbank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data = COMBINE_DATA(&m_bank[offset]);

	setbank(offset >> 1, 2 * offset + 0, (data >> 8) & 0xff);
	setbank(offset >> 1, 2 * offset + 1, (data >> 0) & 0xff);
}


/***************************************************************************

  Display refresh

***************************************************************************/


uint32_t aerofgt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->set_scrollx(0, m_rasterram[0x0000] - 18);
	m_tilemap[0]->set_scrolly(0, m_scrolly[0]);
	m_tilemap[1]->set_scrollx(0, m_rasterram[0x0200] - 20);
	m_tilemap[1]->set_scrolly(0, m_scrolly[1]);

	screen.priority().fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);

	m_spr->draw_sprites(m_spriteram, m_spriteram.bytes(), screen, bitmap, cliprect, 0x03, 0x00);
	m_spr->draw_sprites(m_spriteram, m_spriteram.bytes(), screen, bitmap, cliprect, 0x03, 0x01);

	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	m_spr->draw_sprites(m_spriteram, m_spriteram.bytes(), screen, bitmap, cliprect, 0x03, 0x02);
	m_spr->draw_sprites(m_spriteram, m_spriteram.bytes(), screen, bitmap, cliprect, 0x03, 0x03);

	return 0;
}


void aerofgt_state::soundlatch_pending_w(int state)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);

	// NMI routine is very short, so briefly set perfect_quantum to make sure that the timing is right
	if (state)
		machine().scheduler().perfect_quantum(attotime::from_usec(100));
}

void aerofgt_state::sh_bankswitch_w(uint8_t data)
{
	m_soundbank->set_entry(data & 0x03);
}

template <int Layer>
void aerofgt_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	m_tilemap[Layer]->mark_tile_dirty(offset);
}

template <int Layer>
void aerofgt_state::scrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scrolly[Layer]);
}

void aerofgt_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x1a0000, 0x1a07ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x1b0000, 0x1b07ff).ram().share(m_rasterram);   // used only for the scroll registers
	map(0x1b0800, 0x1b0801).ram(); // tracks watchdog state
	map(0x1b0ff0, 0x1b0fff).ram(); // stack area during boot
	map(0x1b2000, 0x1b3fff).ram().w(FUNC(aerofgt_state::vram_w<0>)).share("vram.0");
	map(0x1b4000, 0x1b5fff).ram().w(FUNC(aerofgt_state::vram_w<1>)).share("vram.1");
	map(0x1c0000, 0x1c7fff).ram().share(m_sprlookupram);
	map(0x1d0000, 0x1d1fff).ram().share(m_spriteram);
	map(0xfef000, 0xffefff).ram(); // work RAM
	map(0xffff80, 0xffff87).w(FUNC(aerofgt_state::gfxbank_w));
	map(0xffff88, 0xffff89).w(FUNC(aerofgt_state::scrolly_w<0>)); // + something else in the top byte
	map(0xffff90, 0xffff91).w(FUNC(aerofgt_state::scrolly_w<1>)); // + something else in the top byte
	map(0xffffa0, 0xffffbf).rw("io", FUNC(vs9209_device::read), FUNC(vs9209_device::write)).umask16(0x00ff);
	map(0xffffc1, 0xffffc1).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}

void aerofgt_state::sound_map(address_map &map)
{
	map(0x0000, 0x77ff).rom().region("audiocpu", 0);
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_soundbank);
}

void aerofgt_state::sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0x04, 0x04).w(FUNC(aerofgt_state::sh_bankswitch_w));
	map(0x08, 0x08).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
	map(0x0c, 0x0c).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

static INPUT_PORTS_START( aerofgt )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	// "Free Play mode: Have SW1:1-8 ON."
	PORT_DIPNAME( 0x01, 0x01, "Coin Slot" )                 PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Same" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x80, 0x80, "Continue Coin" )             PORT_DIPLOCATION("SW1:8") // "When ON, SW1:2-7 are disabled."
	PORT_DIPSETTING(    0x80, "Start 1 Coin/Continue 1 Coin" )
	PORT_DIPSETTING(    0x00, "Start 2 Coin/Continue 1 Coin" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "200000" )
	PORT_DIPSETTING(    0x00, "300000" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )

	// Jumpers not documented in the Aero Fighters manual
	PORT_START("JP1")
	PORT_DIPNAME( 0xf, 0x0, DEF_STR( Region ) )
	PORT_DIPSETTING(   0x0, "Any" )
	PORT_DIPSETTING(   0xf, "USA/Canada" )
	PORT_DIPSETTING(   0xe, DEF_STR( Korea ) )
	PORT_DIPSETTING(   0xd, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(   0xb, DEF_STR( Taiwan ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_aerofgt )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_msb,     0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_msb,   256, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_16x16x4_packed_msb, 512, 32 )
GFXDECODE_END


void aerofgt_state::machine_start()
{
	m_soundbank->configure_entries(0, 4, memregion("soundbank")->base(), 0x8000);
}

void aerofgt_state::machine_reset()
{
	m_soundbank->set_entry(0); // needed by spinlbrk
}

void aerofgt_state::aerofgt(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(20'000'000)/2); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &aerofgt_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(aerofgt_state::irq1_line_hold)); // all irq vectors are the same

	Z80(config, m_audiocpu, XTAL(20'000'000)/4); // 5 MHz verified on pcb
	m_audiocpu->set_addrmap(AS_PROGRAM, &aerofgt_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &aerofgt_state::sound_portmap); // IRQs are triggered by the YM2610

	vs9209_device &io(VS9209(config, "io", 0));
	io.porta_input_cb().set_ioport("P1");
	io.portb_input_cb().set_ioport("P2");
	io.portc_input_cb().set_ioport("SYSTEM");
	io.portd_input_cb().set_ioport("DSW1");
	io.porte_input_cb().set_ioport("DSW2");
	io.portg_input_cb().set(m_soundlatch, FUNC(generic_latch_8_device::pending_r)).lshift(0);
	io.portg_output_cb().set("watchdog", FUNC(mb3773_device::write_line_ck)).bit(7);
	io.porth_input_cb().set_ioport("JP1");

	MB3773(config, "watchdog", 0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(61.31);  // verified on pcb
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(400)); // wrong but improves sprite-background synchronization
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(aerofgt_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_aerofgt);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 1024);

	VSYSTEM_SPR(config, m_spr, 0);
	m_spr->set_tile_indirect_cb(FUNC(aerofgt_state::tile_callback));
	m_spr->set_gfx_region(2);
	m_spr->set_gfxdecode_tag(m_gfxdecode);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(FUNC(aerofgt_state::soundlatch_pending_w));
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", XTAL(8'000'000)));  // verified on pcb
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( aerofgt )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "1.u4",         0x00000, 0x80000, CRC(6fdff0a2) SHA1(7cc9529b426091027aa3e23586cb7d162376c0ff) )

	ROM_REGION( 0x20000, "soundbank", 0 )    // 128k for the audio CPU + banks
	ROM_LOAD( "2.153",        0x00000, 0x20000, CRC(a1ef64ec) SHA1(fa3e434738bf4e742ad68882c1e914100ce0f761) )

	ROM_REGION( 0x08000, "audiocpu", 0 )    // 32k for the audio CPU
	ROM_COPY( "soundbank", 0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_WORD_SWAP( "538a54.124",   0x000000, 0x80000, CRC(4d2c4df2) SHA1(f51c2b3135f0a921ac1a79e63d6878c03cb6254b) )
	ROM_LOAD16_WORD_SWAP( "1538a54.124",  0x080000, 0x80000, CRC(286d109e) SHA1(3a5f3d2d89cf58f6ef15e4bd3f570b84e8e695b2) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_WORD_SWAP( "538a53.u9",    0x000000, 0x100000, CRC(630d8e0b) SHA1(5a0c252ccd53c5199a695909d25ecb4e53dc15b9) )
	ROM_LOAD16_WORD_SWAP( "534g8f.u18",   0x200000, 0x080000, CRC(76ce0926) SHA1(5ef4cec215d4dd600d8fcd1bd9a4c09081d59e33) )

	ROM_REGION( 0x40000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "it-19-01",     0x00000, 0x40000, CRC(6d42723d) SHA1(57c59234e9925430a4c687733682efed06d7eed1) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "it-19-06",     0x000000, 0x100000, CRC(cdbbdb1d) SHA1(067c816545f246ff1fd4c821d70df1e7eb47938c) )
ROM_END

} // anonymous namespace

GAME( 1992, aerofgt, 0, aerofgt, aerofgt, aerofgt_state, empty_init, ROT270, "Video System Co.", "Aero Fighters (World / USA + Canada / Korea / Hong Kong / Taiwan) (newer hardware)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
// All clones run on older type hardware and are in pspikes.cpp
