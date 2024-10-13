// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Tail to Nose / Super Formula - (c) 1989 Video System Co.

    Driver by Nicola Salmoria

    keep pressed F1 during POST to see ROM/RAM/GFX tests.

    The "Country" DIP switch is intended to select the game's title.
    However, the program code in all but one of the known sets forces it to
    one value or the other whenever it reads it outside of service mode.

***************************************************************************/

#include "emu.h"

#include "vsystem_gga.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/6850acia.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"
#include "video/k051316.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class tail2nos_state : public driver_device
{
public:
	tail2nos_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_txvideoram(*this, "txvideoram"),
		m_spriteram(*this, "spriteram"),
		m_zoomram(*this, "k051316"),
		m_soundbank(*this, "soundbank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k051316(*this, "k051316"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_acia(*this, "acia"),
		m_analog(*this, "AN%u", 0U)
	{ }

	void tail2nos(machine_config &config);

	template <int N> ioport_value analog_in_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint16_t> m_txvideoram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_zoomram;
	required_memory_bank m_soundbank;

	// video-related
	tilemap_t   *m_tx_tilemap;
	uint8_t     m_txbank;
	uint8_t     m_txpalette;
	bool        m_video_enable;
	bool        m_flip_screen;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k051316_device> m_k051316;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<acia6850_device> m_acia;
	required_ioport_array<2> m_analog;

	void txvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void zoomdata_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gfxbank_w(uint8_t data);
	void sound_bankswitch_w(uint8_t data);
	uint8_t soundlatch_pending_r();
	void soundlatch_pending_w(int state);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void postload();
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	K051316_CB_MEMBER(zoom_callback);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_port_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(tail2nos_state::get_tile_info)
{
	uint16_t code = m_txvideoram[tile_index];
	tileinfo.set(0,
			(code & 0x1fff) + (m_txbank << 13),
			((code & 0xe000) >> 13) + m_txpalette * 16,
			0);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(tail2nos_state::zoom_callback)
{
	*code |= ((*color & 0x03) << 8);
	*color = 32 + ((*color & 0x38) >> 3);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void tail2nos_state::postload()
{
	m_tx_tilemap->mark_all_dirty();

	m_k051316->gfx(0)->mark_all_dirty();
}

void tail2nos_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tail2nos_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tx_tilemap->set_transparent_pen(15);

	machine().save().register_postload(save_prepost_delegate(FUNC(tail2nos_state::postload), this));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void tail2nos_state::txvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_txvideoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

void tail2nos_state::zoomdata_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int oldword = m_zoomram[offset];
	COMBINE_DATA(&m_zoomram[offset]);
	// tell the K051316 device the data changed
	if (oldword != m_zoomram[offset])
		m_k051316->mark_gfx_dirty(offset * 2);
}

void tail2nos_state::gfxbank_w(uint8_t data)
{
	// -------- --pe-b-b
	// p = palette bank
	// b = tile bank
	// e = video enable

	// bits 0 and 2 select char bank
	int bank = 0;
	if (data & 0x04) bank |= 2;
	if (data & 0x01) bank |= 1;

	if (m_txbank != bank)
	{
		m_txbank = bank;
		m_tx_tilemap->mark_all_dirty();
	}

	// bit 5 seems to select palette bank (used on startup)
	if (data & 0x20)
		bank = 7;
	else
		bank = 3;

	if (m_txpalette != bank)
	{
		m_txpalette = bank;
		m_tx_tilemap->mark_all_dirty();
	}

	// bit 4 seems to be video enable
	m_video_enable = BIT(data, 4);

	// bit 7 is flip screen
	m_flip_screen = BIT(data, 7);
	m_tx_tilemap->set_flip(m_flip_screen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tx_tilemap->set_scrolly(m_flip_screen ? -8 : 0);
}


/***************************************************************************

    Display Refresh

***************************************************************************/

void tail2nos_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	for (int offs = 0; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		int sx = m_spriteram[offs + 1];
		if (sx >= 0x8000)
			sx -= 0x10000;
		int sy = 0x10000 - m_spriteram[offs + 0];
		if (sy >= 0x8000)
			sy -= 0x10000;
		int code = m_spriteram[offs + 2] & 0x07ff;
		int color = (m_spriteram[offs + 2] & 0xe000) >> 13;
		int flipx = m_spriteram[offs + 2] & 0x1000;
		int flipy = m_spriteram[offs + 2] & 0x0800;
		if (m_flip_screen)
		{
			flipx = !flipx;
			flipy = !flipy;
			sx = 302 - sx;
			sy = 216 - sy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, // placement relative to zoom layer verified on the real thing
				cliprect,
				code,
				40 + color,
				flipx, flipy,
				sx + 3, sy + 1, 15);
	}
}

uint32_t tail2nos_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_video_enable)
	{
		m_k051316->zoom_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect);
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(0, cliprect);

	return 0;
}


uint8_t tail2nos_state::soundlatch_pending_r()
{
	return m_soundlatch->pending_r();
}

void tail2nos_state::soundlatch_pending_w(int state)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);

	// sound comms is 2-way (see soundlatch_pending_r),
	// NMI routine is very short, so briefly set perfect_quantum to make sure that the timing is right
	if (state)
		machine().scheduler().perfect_quantum(attotime::from_usec(100));
}

void tail2nos_state::sound_bankswitch_w(uint8_t data)
{
	m_soundbank->set_entry(data & 0x01);
}

void tail2nos_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x200000, 0x27ffff).rom().region("user1", 0);    // extra ROM
	map(0x2c0000, 0x2dffff).rom().region("user2", 0);
	map(0x400000, 0x41ffff).ram().w(FUNC(tail2nos_state::zoomdata_w)).share(m_zoomram);
	map(0x500000, 0x500fff).rw(m_k051316, FUNC(k051316_device::read), FUNC(k051316_device::write)).umask16(0x00ff);
	map(0x510000, 0x51001f).w(m_k051316, FUNC(k051316_device::ctrl_w)).umask16(0x00ff);
	map(0xff8000, 0xffbfff).ram();                             // work RAM
	map(0xffc000, 0xffc2ff).ram().share(m_spriteram);
	map(0xffc300, 0xffcfff).ram();
	map(0xffd000, 0xffdfff).ram().w(FUNC(tail2nos_state::txvideoram_w)).share(m_txvideoram);
	map(0xffe000, 0xffefff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xfff000, 0xfff001).portr("IN0");
	map(0xfff001, 0xfff001).w(FUNC(tail2nos_state::gfxbank_w));
	map(0xfff002, 0xfff003).portr("IN1");
	map(0xfff004, 0xfff005).portr("DSW");
	map(0xfff009, 0xfff009).r(FUNC(tail2nos_state::soundlatch_pending_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask16(0x00ff);
	map(0xfff020, 0xfff023).w("gga", FUNC(vsystem_gga_device::write)).umask16(0x00ff);
	map(0xfff030, 0xfff033).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
}

void tail2nos_state::sound_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_soundbank);
}

void tail2nos_state::sound_port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x07, 0x07).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
	map(0x08, 0x0b).w("ymsnd", FUNC(ym2608_device::write));
#if 0
	map(0x18, 0x1b).r("ymsnd", FUNC(ym2608_device::read));
#endif
}

template <int N>
ioport_value tail2nos_state::analog_in_r()
{
	int delta = m_analog[N]->read();

	return delta >> 5;
}

static INPUT_PORTS_START( tail2nos )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CONDITION("DSW", 0x4000, EQUALS, 0x4000) PORT_NAME("Brake (standard BD)")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CONDITION("DSW", 0x4000, EQUALS, 0x4000) PORT_NAME("Accelerate (standard BD)")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CONDITION("DSW", 0x4000, EQUALS, 0x4000)
	PORT_BIT( 0x0070, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(tail2nos_state, analog_in_r<0>) PORT_CONDITION("DSW", 0x4000, NOTEQUALS, 0x4000)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Advance") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0070, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(tail2nos_state, analog_in_r<1>) PORT_CONDITION("DSW", 0x4000, NOTEQUALS, 0x4000)
	PORT_BIT( 0x0070, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW", 0x4000, EQUALS, 0x4000)
	PORT_BIT( 0xff8f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("AN0")
	PORT_BIT( 0xff, 0, IPT_AD_STICK_Z ) PORT_SENSITIVITY(10) PORT_KEYDELTA(5) PORT_NAME("Brake (original BD)") PORT_CONDITION("DSW", 0x4000, NOTEQUALS, 0x4000)
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW", 0x4000, EQUALS, 0x4000)

	PORT_START("AN1")
	PORT_BIT( 0xff, 0, IPT_AD_STICK_Z ) PORT_SENSITIVITY(10) PORT_KEYDELTA(5) PORT_NAME("Accelerate (original BD)")  PORT_CONDITION("DSW", 0x4000, NOTEQUALS, 0x4000)
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW", 0x4000, EQUALS, 0x4000)

	PORT_START("DSW")
	PORT_DIPNAME( 0x000f, 0x0000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0009, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000b, "6 Coins/4 Credits" )
	PORT_DIPSETTING(      0x000c, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000d, "5 Coins/6 Credits" )
	PORT_DIPSETTING(      0x000e, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x00f0, 0x0000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0x0090, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00b0, "6 Coins/4 Credits" )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00d0, "5 Coins/6 Credits" )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0800, IP_ACTIVE_HIGH, "SW2:4" )
	PORT_DIPNAME( 0x1000, 0x1000, "Game Mode" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Single ) )
	PORT_DIPSETTING(      0x0000, "Multiple" )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Control Panel" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Standard ) )
	PORT_DIPSETTING(      0x0000, "Original" )
	PORT_DIPNAME( 0x8000, 0x8000, "Country" ) PORT_DIPLOCATION("SW2:8") // Set to "Tail to Nose" for the 1 set that it works with
	PORT_DIPSETTING(      0x0000, "Domestic" ) // "Super Formula"
	PORT_DIPSETTING(      0x8000, "Overseas" ) // "Tail to Nose"
INPUT_PORTS_END

static INPUT_PORTS_START( sformula )
	PORT_INCLUDE(tail2nos)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x8000, 0x0000, "Country" ) PORT_DIPLOCATION("SW2:8") // Seems to be ignored by the Japanese sets?
	PORT_DIPSETTING(      0x0000, "Domestic" ) // "Super Formula"
	PORT_DIPSETTING(      0x8000, "Overseas" ) // "Tail to Nose"
INPUT_PORTS_END


static const gfx_layout tail2nos_spritelayout =
{
	16,32,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, RGN_FRAC(1,2)+1*4, RGN_FRAC(1,2)+0*4, RGN_FRAC(1,2)+3*4, RGN_FRAC(1,2)+2*4,
			5*4, 4*4, 7*4, 6*4, RGN_FRAC(1,2)+5*4, RGN_FRAC(1,2)+4*4, RGN_FRAC(1,2)+7*4, RGN_FRAC(1,2)+6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32,
			24*32, 25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32 },
	128*8
};

static GFXDECODE_START( gfx_tail2nos )
	GFXDECODE_ENTRY( "chars",   0, gfx_8x8x4_packed_lsb,  0, 128 )
	GFXDECODE_ENTRY( "sprites", 0, tail2nos_spritelayout, 0, 128 )
GFXDECODE_END


void tail2nos_state::machine_start()
{
	uint8_t *ROM = memregion("audiocpu")->base();

	m_soundbank->configure_entries(0, 2, &ROM[0x10000], 0x8000);
	m_soundbank->set_entry(0);

	m_acia->write_cts(0);
	m_acia->write_dcd(0);

	m_txbank = 0;
	m_txpalette = 0;
	m_video_enable = false;
	m_flip_screen = false;

	save_item(NAME(m_txbank));
	save_item(NAME(m_txpalette));
	save_item(NAME(m_video_enable));
	save_item(NAME(m_flip_screen));
}


void tail2nos_state::tail2nos(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(20'000'000) / 2); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &tail2nos_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(tail2nos_state::irq6_line_hold));

	Z80(config, m_audiocpu, XTAL(20'000'000) / 4); // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &tail2nos_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &tail2nos_state::sound_port_map);
								// IRQs are triggered by the YM2608

	ACIA6850(config, m_acia, 0);
	m_acia->irq_handler().set_inputline("maincpu", M68K_IRQ_3);
	//m_acia->txd_handler().set("link", FUNC(rs232_port_device::write_txd));
	//m_acia->rts_handler().set("link", FUNC(rs232_port_device::write_rts));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(tail2nos_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tail2nos);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 2048);

	K051316(config, m_k051316, 0);
	m_k051316->set_palette(m_palette);
	m_k051316->set_bpp(-4);
	m_k051316->set_offsets(-89, -14);
	m_k051316->set_wrap(1);
	m_k051316->set_zoom_callback(FUNC(tail2nos_state::zoom_callback));

	VSYSTEM_GGA(config, "gga", XTAL(14'318'181) / 2); // divider not verified

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set(FUNC(tail2nos_state::soundlatch_pending_w));
	m_soundlatch->set_separate_acknowledge(true);

	ym2608_device &ymsnd(YM2608(config, "ymsnd", XTAL(8'000'000))); // verified on PCB
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.port_b_write_callback().set(FUNC(tail2nos_state::sound_bankswitch_w));
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}



ROM_START( tail2nos )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "s2-h.ic129", 0x00000, 0x10000, CRC(567a55a4) SHA1(5cb5cce7199faf13423d7ff749774c428b56d2ec) )
	ROM_LOAD16_BYTE( "s2-l.ic130", 0x00001, 0x10000, CRC(c630f875) SHA1(1cebe3758212e1f6178fe07dea2b626c9efb86da) )
	ROM_LOAD16_BYTE( "v3.ic141",   0x20000, 0x10000, CRC(e2e0abad) SHA1(1a1054bada9654484fe81fe4b4b32af5ab7b53f0) )
	ROM_LOAD16_BYTE( "v6.ic142",   0x20001, 0x10000, CRC(069817a7) SHA1(cca382fe2a49c8c3c84b879a1c30dffff84ef406) )

	ROM_REGION16_BE( 0x80000, "user1", 0 ) // extra ROM mapped at 200000
	ROM_LOAD16_WORD_SWAP( "a23.ic96", 0x00000, 0x80000, CRC(d851cf04) SHA1(ac5b366b686c5a037b127d223dc6fe90985eb160) )
	// unpopulated 4M mask ROM socket at IC105

	ROM_REGION16_BE( 0x20000, "user2", 0 ) // extra ROM mapped at 2c0000
	ROM_LOAD16_BYTE( "v5.ic119", 0x00000, 0x10000, CRC(a9fe15a1) SHA1(d90bf40c610ea7daaa338f83f82cdffbae7da08e) )
	ROM_LOAD16_BYTE( "v8.ic120", 0x00001, 0x10000, CRC(4fb6a43e) SHA1(5cddda0029b3b141c88b0c128655d35bb12fa34d) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // 64k for the audio CPU + banks
	ROM_LOAD( "v2.ic125", 0x00000, 0x08000, CRC(920d8920) SHA1(b8d30903248fee6f985af7fafbe534cfc8c6e829) )
	ROM_LOAD( "v1.ic137", 0x10000, 0x10000, CRC(bf35c1a4) SHA1(a838740e023dc3344dc528324a8dbc48bb98b574) )

	ROM_REGION( 0x100000, "chars", 0 )
	ROM_LOAD( "a24.ic34", 0x00000, 0x80000, CRC(b1e9de43) SHA1(0144252dd9ed561fbebd4994cccf11f6c87e1825) )
	ROM_LOAD( "o1s.ic18", 0x80000, 0x40000, CRC(e27a8eb4) SHA1(4fcadabf42a1c3deeb6d74d75cdbee802cf16db5) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "oj1.ic93", 0x000000, 0x40000, CRC(39c36b35) SHA1(a97480696bf6d81bf415737e03cc5324d439ab84) )
	ROM_LOAD( "oj2.ic79", 0x040000, 0x40000, CRC(77ccaea2) SHA1(e38175859c75c6d0f2f01752fad6e167608c4662) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "osb.ic127", 0x00000, 0x20000, CRC(d49ab2f5) SHA1(92f7f6c8f35ac39910879dd88d2cfb6db7c848c9) )
ROM_END

ROM_START( tail2nosa )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "v4.ic129", 0x00000, 0x10000, CRC(1d4240c2) SHA1(db8992d8e718e20acb7b3f2f0b1f358098863145) )
	ROM_LOAD16_BYTE( "v7.ic130", 0x00001, 0x10000, CRC(0fb70066) SHA1(3d38672402d5ab70599c191cc274746a192b399b) )
	ROM_LOAD16_BYTE( "v3.ic141", 0x20000, 0x10000, CRC(e2e0abad) SHA1(1a1054bada9654484fe81fe4b4b32af5ab7b53f0) )
	ROM_LOAD16_BYTE( "v6.ic142", 0x20001, 0x10000, CRC(069817a7) SHA1(cca382fe2a49c8c3c84b879a1c30dffff84ef406) )

	ROM_REGION16_BE( 0x80000, "user1", 0 ) // extra ROM mapped at 200000
	ROM_LOAD16_WORD_SWAP( "a23.ic96", 0x00000, 0x80000, CRC(d851cf04) SHA1(ac5b366b686c5a037b127d223dc6fe90985eb160) )
	// unpopulated 4M mask ROM socket at IC105

	ROM_REGION16_BE( 0x20000, "user2", 0 ) // extra ROM mapped at 2c0000
	ROM_LOAD16_BYTE( "v5.ic119", 0x00000, 0x10000, CRC(a9fe15a1) SHA1(d90bf40c610ea7daaa338f83f82cdffbae7da08e) )
	ROM_LOAD16_BYTE( "v8.ic120", 0x00001, 0x10000, CRC(4fb6a43e) SHA1(5cddda0029b3b141c88b0c128655d35bb12fa34d) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // 64k for the audio CPU + banks
	ROM_LOAD( "v2.ic125", 0x00000, 0x08000, CRC(920d8920) SHA1(b8d30903248fee6f985af7fafbe534cfc8c6e829) )
	ROM_LOAD( "v1.ic137", 0x10000, 0x10000, CRC(bf35c1a4) SHA1(a838740e023dc3344dc528324a8dbc48bb98b574) )

	ROM_REGION( 0x100000, "chars", 0 )
	ROM_LOAD( "a24.ic34", 0x00000, 0x80000, CRC(b1e9de43) SHA1(0144252dd9ed561fbebd4994cccf11f6c87e1825) )
	ROM_LOAD( "o1s.ic18", 0x80000, 0x40000, CRC(e27a8eb4) SHA1(4fcadabf42a1c3deeb6d74d75cdbee802cf16db5) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "oj1.ic93", 0x000000, 0x40000, CRC(39c36b35) SHA1(a97480696bf6d81bf415737e03cc5324d439ab84) )
	ROM_LOAD( "oj2.ic79", 0x040000, 0x40000, CRC(77ccaea2) SHA1(e38175859c75c6d0f2f01752fad6e167608c4662) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "osb.ic127", 0x00000, 0x20000, CRC(d49ab2f5) SHA1(92f7f6c8f35ac39910879dd88d2cfb6db7c848c9) )
ROM_END

ROM_START( sformula )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "4.ic129",    0x00000, 0x10000, CRC(672bf690) SHA1(b322234b47f20a36430bc03be0b52d9b7f82967b) )
	ROM_LOAD16_BYTE( "7.ic130",    0x00001, 0x10000, CRC(73f0c91c) SHA1(faf14eb1a210c7330b47b78ca6c6563ea6482b3b) )
	ROM_LOAD16_BYTE( "v3.ic141",   0x20000, 0x10000, CRC(e2e0abad) SHA1(1a1054bada9654484fe81fe4b4b32af5ab7b53f0) )
	ROM_LOAD16_BYTE( "v6.ic142",   0x20001, 0x10000, CRC(069817a7) SHA1(cca382fe2a49c8c3c84b879a1c30dffff84ef406) )

	ROM_REGION16_BE( 0x80000, "user1", 0 ) // extra ROM mapped at 200000
	ROM_LOAD16_WORD_SWAP( "a23.ic96", 0x00000, 0x80000, CRC(d851cf04) SHA1(ac5b366b686c5a037b127d223dc6fe90985eb160) )
	// unpopulated 4M mask ROM socket at IC105

	ROM_REGION16_BE( 0x20000, "user2", 0 ) // extra ROM mapped at 2c0000
	ROM_LOAD16_BYTE( "v5.ic119", 0x00000, 0x10000, CRC(a9fe15a1) SHA1(d90bf40c610ea7daaa338f83f82cdffbae7da08e) )
	ROM_LOAD16_BYTE( "v8.ic120", 0x00001, 0x10000, CRC(4fb6a43e) SHA1(5cddda0029b3b141c88b0c128655d35bb12fa34d) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // 64k for the audio CPU + banks
	ROM_LOAD( "v2.ic125", 0x00000, 0x08000, CRC(920d8920) SHA1(b8d30903248fee6f985af7fafbe534cfc8c6e829) )
	ROM_LOAD( "v1.ic137", 0x10000, 0x10000, CRC(bf35c1a4) SHA1(a838740e023dc3344dc528324a8dbc48bb98b574) )

	ROM_REGION( 0x100000, "chars", 0 )
	ROM_LOAD( "a24.ic34", 0x00000, 0x80000, CRC(b1e9de43) SHA1(0144252dd9ed561fbebd4994cccf11f6c87e1825) )
	ROM_LOAD( "o1s.ic18", 0x80000, 0x40000, CRC(e27a8eb4) SHA1(4fcadabf42a1c3deeb6d74d75cdbee802cf16db5) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "oj1.ic93", 0x000000, 0x40000, CRC(39c36b35) SHA1(a97480696bf6d81bf415737e03cc5324d439ab84) )
	ROM_LOAD( "oj2.ic79", 0x040000, 0x40000, CRC(77ccaea2) SHA1(e38175859c75c6d0f2f01752fad6e167608c4662) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "osb.ic127", 0x00000, 0x20000, CRC(d49ab2f5) SHA1(92f7f6c8f35ac39910879dd88d2cfb6db7c848c9) )
ROM_END


ROM_START( sformulaa )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "04.ic29",  0x00000, 0x10000, CRC(f40e9c3c) SHA1(2ab45f46f92bce42748692cafe601c5893de127b) )
	ROM_LOAD16_BYTE( "07.ic30",  0x00001, 0x10000, CRC(d1cf6dca) SHA1(18228cc98722eb5907850e2d0317d1f4bf04fb8f) )
	ROM_LOAD16_BYTE( "v3.ic141", 0x20000, 0x10000, CRC(e2e0abad) SHA1(1a1054bada9654484fe81fe4b4b32af5ab7b53f0) )
	ROM_LOAD16_BYTE( "v6.ic142", 0x20001, 0x10000, CRC(069817a7) SHA1(cca382fe2a49c8c3c84b879a1c30dffff84ef406) )

	ROM_REGION16_BE( 0x80000, "user1", 0 ) // extra ROM mapped at 200000
	ROM_LOAD16_WORD_SWAP( "a23.ic96", 0x00000, 0x80000, CRC(d851cf04) SHA1(ac5b366b686c5a037b127d223dc6fe90985eb160) )
	// unpopulated 4M mask ROM socket at IC105

	ROM_REGION16_BE( 0x20000, "user2", 0 ) // extra ROM mapped at 2c0000
	ROM_LOAD16_BYTE( "v5.ic119", 0x00000, 0x10000, CRC(a9fe15a1) SHA1(d90bf40c610ea7daaa338f83f82cdffbae7da08e) )
	ROM_LOAD16_BYTE( "v8.ic120", 0x00001, 0x10000, CRC(4fb6a43e) SHA1(5cddda0029b3b141c88b0c128655d35bb12fa34d) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // 64k for the audio CPU + banks
	ROM_LOAD( "v2.ic125", 0x00000, 0x08000, CRC(920d8920) SHA1(b8d30903248fee6f985af7fafbe534cfc8c6e829) )
	ROM_LOAD( "v1.ic137", 0x10000, 0x10000, CRC(bf35c1a4) SHA1(a838740e023dc3344dc528324a8dbc48bb98b574) )

	ROM_REGION( 0x100000, "chars", 0 )
	ROM_LOAD( "a24.ic34", 0x00000, 0x80000, CRC(b1e9de43) SHA1(0144252dd9ed561fbebd4994cccf11f6c87e1825) )
	ROM_LOAD( "o1s.ic18", 0x80000, 0x40000, CRC(e27a8eb4) SHA1(4fcadabf42a1c3deeb6d74d75cdbee802cf16db5) )
	ROM_LOAD( "9.ic3",    0xc0000, 0x08000, CRC(c76edc0a) SHA1(2c6c21f8d1f3bcb0f65ba5a779fe479783271e0b) ) // present on this PCB, contains Japanese text + same font as in above ROMs, where does it map? is there another layer?

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "oj1.ic93", 0x000000, 0x40000, CRC(39c36b35) SHA1(a97480696bf6d81bf415737e03cc5324d439ab84) )
	ROM_LOAD( "oj2.ic79", 0x040000, 0x40000, CRC(77ccaea2) SHA1(e38175859c75c6d0f2f01752fad6e167608c4662) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "osb.ic127", 0x00000, 0x20000, CRC(d49ab2f5) SHA1(92f7f6c8f35ac39910879dd88d2cfb6db7c848c9) )
ROM_END

} // anonymous namespace


GAME( 1989, tail2nos,  0,        tail2nos, tail2nos, tail2nos_state, empty_init, ROT90, "V-System Co.", "Tail to Nose - Great Championship / Super Formula", MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE ) // Only set that's affected by the Country dipswitch
GAME( 1989, tail2nosa, tail2nos, tail2nos, tail2nos, tail2nos_state, empty_init, ROT90, "V-System Co.", "Tail to Nose - Great Championship",                 MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
GAME( 1989, sformula,  tail2nos, tail2nos, sformula, tail2nos_state, empty_init, ROT90, "V-System Co.", "Super Formula (Japan, set 1)",                      MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE ) // For Use in Japan... warning
GAME( 1989, sformulaa, tail2nos, tail2nos, sformula, tail2nos_state, empty_init, ROT90, "V-System Co.", "Super Formula (Japan, set 2)",                      MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE ) // No Japan warning, but Japanese version
