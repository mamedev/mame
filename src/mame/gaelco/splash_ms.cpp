// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Splash (Modular System)
*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"
#include "sound/msm5205.h"
#include "sound/ymopl.h"
#include "machine/gen_latch.h"
#include "machine/bankdev.h"


class splashms_state : public driver_device
{
public:
	splashms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_soundcpu(*this, "soundcpu"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_paletteram(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_bitmapram(*this, "bitmapram"),
		m_scrollregs(*this, "scrollregs"),
		m_spriteram(*this, "spriteram"),
		m_msm(*this, "msm"),
		m_bgdata(*this, "subcpu"),
		m_soundlatch(*this, "soundlatch"),
		m_subram(*this, "subrambank"),
		m_subrom(*this, "subrombank")
	{ }

	void splashms(machine_config &config);
	void init_splashms();

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_soundcpu;

	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint16_t> m_paletteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_videoram2;
	required_shared_ptr<uint8_t> m_bitmapram;
	required_shared_ptr<uint16_t> m_scrollregs;
	required_shared_ptr<uint16_t> m_spriteram;
	required_device<msm5205_device> m_msm;
	required_region_ptr<uint8_t> m_bgdata;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<address_map_bank_device> m_subram;
	required_device<address_map_bank_device> m_subrom;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void splashms_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
	void sub_portmap(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	uint16_t unknown_0x40000c_r();
	uint8_t frommain_command_r();
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t vram2_r(offs_t offset, uint16_t mem_mask);
	void vram2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_tile_info_tilemap1);
	TILE_GET_INFO_MEMBER(get_tile_info_tilemap2);
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap2 = nullptr;

	void to_subcpu_0x400004_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint8_t m_subcmd = 0;

	void sub_rambankselect_w(uint8_t data);
	void sub_rombankselect_w(uint8_t data);

	void splash_msm5205_int(int state);
	void splash_adpcm_data_w(uint8_t data);
	void splash_adpcm_control_w(uint8_t data);
	int m_adpcm_data = 0;

	void subrambank_map(address_map &map) ATTR_COLD;
	void subrombank_map(address_map &map) ATTR_COLD;

	void descramble_16x16tiles(uint8_t* src, int len);

};


uint16_t splashms_state::unknown_0x40000c_r()
{
	logerror("%06x: unknown_0x40000c_r\n", machine().describe_context());
	return machine().rand();
}


void splashms_state::sub_rambankselect_w(uint8_t data)
{
//  logerror("sub_rambankselect_w %02x\n", data);
	m_subram->set_bank(data&0x7);
}

void splashms_state::sub_rombankselect_w(uint8_t data)
{
//  logerror("sub_rombankselect_w %02x\n", data);
	m_subrom->set_bank(data & 0x7f);
}


uint8_t splashms_state::frommain_command_r()
{
	return m_subcmd;
}

void splashms_state::to_subcpu_0x400004_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// no IRQ, subcpu just polls this
	//popmessage("to_subcpu_0x400004_w %04x\n", data);
	m_subcmd = data;
}
void splashms_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset/2);
}

uint16_t splashms_state::vram2_r(offs_t offset, uint16_t mem_mask)
{
	return m_videoram2[offset];
}

void splashms_state::vram2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram2[offset]);
	m_bg_tilemap2->mark_tile_dirty(offset/2);
}

TILE_GET_INFO_MEMBER(splashms_state::get_tile_info_tilemap1)
{
	int tile = m_videoram[tile_index*2];
	int attr = m_videoram[(tile_index*2)+1] & 0x0f;
	int fx = (m_videoram[(tile_index*2)+1] & 0xc0)>>6;

	tileinfo.set(1,tile,attr,TILE_FLIPYX(fx));
}

TILE_GET_INFO_MEMBER(splashms_state::get_tile_info_tilemap2)
{
	int tile = m_videoram2[tile_index*2];

	tile &= 0x1ff;

	int attr = m_videoram2[(tile_index*2)+1] & 0x0f;
	int fx = (m_videoram2[(tile_index*2)+1] & 0xc0)>>6;

	tileinfo.set(2,tile,attr,TILE_FLIPYX(fx));
}

uint32_t splashms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  bitmap.fill(0, cliprect);

	for (int y = 0; y < 256; y++)
	{
		uint16_t *const dst = &bitmap.pix(y);

		for (int x = 0; x < 512; x++)
		{
			uint8_t const pix = m_bitmapram[(y * 512) + x];
			//if (pix)
			dst[x] = pix + 0x100;
		}
	}

	m_bg_tilemap2->set_scrollx(0, 64-(m_scrollregs[0]-0x2));
	m_bg_tilemap2->set_scrolly(0, -m_scrollregs[1]);

	m_bg_tilemap->set_scrollx(0, 64-(m_scrollregs[2]));
	m_bg_tilemap->set_scrolly(0, -m_scrollregs[3]);

	m_bg_tilemap2->draw(screen, bitmap, cliprect, 0, 0);

	// TODO, convert to device, share between Modualar System games
	const int NUM_SPRITES = 0x100;
	const int X_EXTRA_OFFSET = 64;

	for (int i = NUM_SPRITES-2; i >= 0; i-=2)
	{
		gfx_element *gfx = m_gfxdecode->gfx(0);

		uint16_t attr0 = m_spriteram[i + 0];
		uint16_t attr1 = m_spriteram[i + 1];

		uint16_t attr2 = m_spriteram[i + NUM_SPRITES];
		//uint16_t attr3 = m_spriteram[i + NUM_SPRITES+1]; // unused?

		int ypos = attr0 & 0x00ff;
		int xpos = (attr1 & 0xff00)>>8;
		xpos |= (attr2 & 0x8000) ? 0x100 : 0x000;

		ypos = (0xff - ypos);
		ypos |= (attr2 & 0x4000) ? 0x100 : 0x000; // maybe

		int tile = (attr0 & 0xff00) >> 8;
		tile |= (attr1 & 0x003f) << 8;

		int flipx = (attr1 & 0x0040);
		int flipy = (attr1 & 0x0080);

		gfx->transpen(bitmap,cliprect,tile,(attr2&0x0f00)>>8,flipx,flipy,xpos-16-X_EXTRA_OFFSET,ypos-16,15);
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void splashms_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(splashms_state::get_tile_info_tilemap1)), TILEMAP_SCAN_ROWS,  8,  8, 64, 32);
	m_bg_tilemap2 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(splashms_state::get_tile_info_tilemap2)), TILEMAP_SCAN_ROWS,  16,  16, 64, 32);

	m_bg_tilemap->set_transparent_pen(15);
	m_bg_tilemap2->set_transparent_pen(0);

}


void splashms_state::splashms_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom(); // writes to 0x030000 on startup

	map(0x080000, 0x081fff).ram().w(FUNC(splashms_state::vram_w)).share("videoram");

	map(0x08e000, 0x08ffff).rw(FUNC(splashms_state::vram2_r), FUNC(splashms_state::vram2_w)); // mirror needed for paint dripping in attract
	map(0x090000, 0x091fff).ram().rw(FUNC(splashms_state::vram2_r), FUNC(splashms_state::vram2_w)).share("videoram2");

	map(0x0a0000, 0x0a1fff).ram(); // writes unused data from vram2 area here, but isn't a mirror (eg. has large paint can from attract during gameplay) unused?

	map(0x0c0000, 0x0c000f).ram().share("scrollregs"); // scroll vals

	map(0x100000, 0x1007ff).ram().share("spriteram");

	map(0x200000, 0x200fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x400000, 0x400001).portr("IN0");
	map(0x400002, 0x400003).portr("IN1");
	map(0x400004, 0x400005).w(FUNC(splashms_state::to_subcpu_0x400004_w));
	map(0x400006, 0x400007).portr("IN3");
	map(0x400008, 0x400009).portr("IN4"); // service mode in here

	map(0x40000c, 0x40000d).r(FUNC(splashms_state::unknown_0x40000c_r));

	map(0x40000e, 0x40000e).w(m_soundlatch, FUNC(generic_latch_8_device::write));

	map(0xff0000, 0xffffff).ram();
}

void splashms_state::sub_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).w(FUNC(splashms_state::sub_rambankselect_w)); // banking for 0x4000-0x7fff RAM
	map(0x02, 0x02).w(FUNC(splashms_state::sub_rombankselect_w)); // banking for 0x8000-0xffff ROM

	map(0x03, 0x03).r(FUNC(splashms_state::frommain_command_r));
}

void splashms_state::sub_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x4000, 0x7fff).m(m_subram, FUNC(address_map_bank_device::amap8));
	map(0x8000, 0xffff).m(m_subrom, FUNC(address_map_bank_device::amap8));
}


void splashms_state::sound_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();

	map(0xe000, 0xe000).w(FUNC(splashms_state::splash_adpcm_control_w));
	map(0xe400, 0xe400).w(FUNC(splashms_state::splash_adpcm_data_w));

	map(0xe800, 0xe801).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));

	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void splashms_state::machine_start()
{
}

void splashms_state::machine_reset()
{
	m_subcmd = 0;
	m_subram->set_bank(0);
	m_subrom->set_bank(0);
}



static INPUT_PORTS_START( splashms )
	PORT_START("IN0")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN3")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0400, "1" )
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPSETTING(      0x1000, "4" )
	PORT_DIPSETTING(      0x1400, "5" )
	PORT_DIPSETTING(      0x1800, "6" )
	PORT_DIPSETTING(      0x1c00, "7" )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x2000, "1" )
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0x6000, "3" )
	PORT_DIPSETTING(      0x8000, "4" )
	PORT_DIPSETTING(      0xa000, "5" )
	PORT_DIPSETTING(      0xc000, "6" )
	PORT_DIPSETTING(      0xe000, "7" )

	PORT_START("IN4")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100,  DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0400, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x4000, "1" )
	PORT_DIPSETTING(      0x8000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
INPUT_PORTS_END

static const gfx_layout tiles16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7, 512,513,514,515,516,517,518,519 },
	{ STEP16(0,32) },
	16 * 16 * 4
};

static const gfx_layout tiles8x8x4_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7 },
	{ STEP8(0,32) },
	16 * 16
};


static GFXDECODE_START( gfx_splashms )
	GFXDECODE_ENTRY( "sprites", 0, tiles16x16x4_layout, 0x200, 16 )
	GFXDECODE_ENTRY( "fgtile", 0, tiles8x8x4_layout, 0, 16 )
	GFXDECODE_ENTRY( "bgtile", 0, tiles16x16x4_layout, 0, 16 )
GFXDECODE_END

void splashms_state::splash_adpcm_data_w(uint8_t data)
{
	m_adpcm_data = data;
}

void splashms_state::splash_adpcm_control_w(uint8_t data)
{
	m_msm->reset_w(BIT(data, 7));
}

void splashms_state::splash_msm5205_int(int state)
{
	m_msm->data_w(m_adpcm_data >> 4);
	m_adpcm_data = (m_adpcm_data << 4) & 0xf0;
}

void splashms_state::subrambank_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram().share("bitmapram");
}

void splashms_state::subrombank_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom().region("subcpu", 0x000000);
}


void splashms_state::splashms(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &splashms_state::splashms_map);
	m_maincpu->set_vblank_int("screen", FUNC(splashms_state::irq4_line_hold));

	Z80(config, m_subcpu, 12_MHz_XTAL/2);
	m_subcpu->set_addrmap(AS_PROGRAM, &splashms_state::sub_map);
	m_subcpu->set_addrmap(AS_IO, &splashms_state::sub_portmap);

	ADDRESS_MAP_BANK(config, m_subram).set_map(&splashms_state::subrambank_map).set_options(ENDIANNESS_LITTLE, 8, 17, 0x4000);
	ADDRESS_MAP_BANK(config, m_subrom).set_map(&splashms_state::subrombank_map).set_options(ENDIANNESS_LITTLE, 8, 22, 0x8000);


	Z80(config, m_soundcpu, 16_MHz_XTAL/4);
	m_soundcpu->set_addrmap(AS_PROGRAM, &splashms_state::sound_map);
	m_soundcpu->set_periodic_int(FUNC(splashms_state::nmi_line_pulse), attotime::from_hz(60*64));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(512, 256);
	m_screen->set_visarea(16, (16+368)-1, 0, 256-16-1);
	m_screen->set_screen_update(FUNC(splashms_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 0x800);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_splashms);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_IRQ0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM3812(config, "ymsnd", XTAL(16'000'000)/4).add_route(ALL_OUTPUTS, "mono", 0.80);

	MSM5205(config, m_msm, XTAL(384'000));
	m_msm->vck_legacy_callback().set(FUNC(splashms_state::splash_msm5205_int));
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.80);
}

// reorganize graphics into something we can decode with a single pass
void splashms_state::descramble_16x16tiles(uint8_t* src, int len)
{
	std::vector<uint8_t> buffer(len);
	{
		for (int i = 0; i < len; i++)
		{
			int j = bitswap<20>(i, 19,18,17,16,15,12,11,10,9,8,7,6,5,14,13,4,3,2,1,0);
			buffer[j] = src[i];
		}

		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}
}

void splashms_state::init_splashms()
{
	descramble_16x16tiles(memregion("bgtile")->base(), memregion("bgtile")->bytes());
	// fgtile is 8x8 tiles
}



ROM_START( splashms )
	ROM_REGION( 0x040000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cpu1_6-1_sp_608a.ic8",  0x000001, 0x020000, CRC(befdbaf0) SHA1(94efdeec1e1311317ffd0fe3d5fdbb02e151b985) )
	ROM_LOAD16_BYTE( "cpu1_6-1_sp_617a.ic17", 0x000000, 0x020000, CRC(080edb2b) SHA1(6104345bc72cd20051d66c04b97c9a365a88ec3f) )

	ROM_REGION( 0x400000, "subcpu", ROMREGION_ERASE00 ) // extra Z80 for backgrounds!
	ROM_LOAD( "cpu2_c_sp_c2.ic2", 0x000000, 0x080000, CRC(3a0be09f) SHA1(83abc10ff2c810c8451f583700f140f569e5b6ee) )
	ROM_LOAD( "cpu2_c_sp_c3.ic3", 0x080000, 0x080000, CRC(c3dc5e9d) SHA1(ce5fb65935cfe225132242e058cd63fa33f9da63) )
	ROM_LOAD( "cpu2_c_sp_c4.ic4", 0x100000, 0x080000, CRC(4d7b643d) SHA1(40bdcf7eedddc3244cb41530d10009b23b7ac473) )
	ROM_LOAD( "cpu2_c_sp_c5.ic5", 0x180000, 0x080000, CRC(7ba31717) SHA1(000cf6ec261ac90efc3e4f2dbf6720a54fb3bbdb) )
	ROM_LOAD( "cpu2_c_sp_c6.ic6", 0x200000, 0x080000, CRC(994e8e16) SHA1(cbe7d9e192d0390b123f1b585a0463634f33b485) )
	ROM_LOAD( "cpu2_c_sp_c7.ic7", 0x280000, 0x080000, CRC(6ea0be42) SHA1(5cc45ef1f3c8f46e1e7b3ad3313d221a65fb0025) )

	ROM_REGION( 0x010000, "soundcpu", 0 )
	ROM_LOAD( "snd_9-2_sp_916.ic6", 0x000000, 0x010000, CRC(5567fa22) SHA1(3993c733a0222ca292b60f409c78b45280a5fec6) )

	ROM_REGION( 0x40000, "fgtile", ROMREGION_ERASEFF )
	ROM_LOAD32_BYTE( "8_sp_837.ic37", 0x000000, 0x010000, CRC(3b544131) SHA1(e7fd97cb24b84739f2481efb1d232f86df4a3d8d) ) // 1xxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "8_sp_830.ic30", 0x000001, 0x010000, CRC(09bb675b) SHA1(49c41ccfce1b0077c430c6bb38bc858aeaf87fb8) ) // has some garbage in the blank space of the paired ROMs
	ROM_LOAD32_BYTE( "8_sp_822.ic22", 0x000002, 0x010000, CRC(621fcf26) SHA1(a7ff6b12fbbea1bba7c4a397a82ac2fb5c09558a) ) // 1xxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "8_sp_815.ic15", 0x000003, 0x010000, CRC(5641b621) SHA1(e71df1ab5c9b2254495d99657477b52e8843d128) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x080000, "sprites", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD32_BYTE( "5-1_sp_524.ic24", 0x000000, 0x010000, CRC(841c24c1) SHA1(70cb26033999f8184c51849e00bfcb2270f646e8) )
	ROM_LOAD32_BYTE( "5-1_sp_518.ic18", 0x000001, 0x010000, CRC(499cb813) SHA1(4d22e58530ff8a85b7ffc8ae1ab5986215986b49) )
	ROM_LOAD32_BYTE( "5-1_sp_512.ic12", 0x000002, 0x010000, CRC(8cb0b132) SHA1(894f84b6a8171ed8c22298ebf1303da020f747ee) )
	ROM_LOAD32_BYTE( "5-1_sp_503.ic3",  0x000003, 0x010000, CRC(ace09666) SHA1(d223718118b9912643d320832414df942e411e70) )
	ROM_LOAD32_BYTE( "5-1_sp_525.ic25", 0x040000, 0x010000, CRC(46bde779) SHA1(b4e1dd1952276c2d2a8f3c150d1ba2a1c2b738b7) ) // 11xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "5-1_sp_519.ic19", 0x040001, 0x010000, CRC(69cc9e06) SHA1(85e1495d01e6986f9cd88d6cdbef194c623be111) ) // 11xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "5-1_sp_513.ic13", 0x040002, 0x010000, CRC(659d206f) SHA1(fc8e9ea2d45df83509de3986763cbfc0d4745983) ) // has some garbage in the blank space of the paired ROMs
	ROM_LOAD32_BYTE( "5-1_sp_504.ic4",  0x040003, 0x010000, CRC(b6806390) SHA1(d95247bcd90bd7b7be355c267f023c19a9d60f66) ) // 11xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x080000, "bgtile", ROMREGION_ERASEFF )
	ROM_LOAD32_BYTE( "8_sp_833.ic33", 0x000000, 0x020000, CRC(8ac7cd1f) SHA1(aad88db9a82f417774f1d5eef830cc97c0d4b0de) ) // 111xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "8_sp_826.ic26", 0x000001, 0x020000, CRC(b7ec71d8) SHA1(3d4b62559c0ba688b94e605594f3e8e9f2cbefa2) ) // 111xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "8_sp_818.ic18", 0x000002, 0x020000, CRC(ae62a832) SHA1(f825a186e25a1c292aa6f880055341ec14373c0b) ) // 111xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD32_BYTE( "8_sp_811.ic11", 0x000003, 0x020000, CRC(02e474b4) SHA1(11446655cb73ec4961339fa4ee41200f8b2b81d3) ) // 111xxxxxxxxxxxxxx = 0xFF


	ROM_REGION( 0x100, "prom", ROMREGION_ERASEFF )
	ROM_LOAD( "51-3_502_82s129.ic10",      0x000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) ) // common PROM found on all? Modular System sets?

	ROM_REGION( 0x100, "protgal", 0 ) // all read protected
	ROM_LOAD( "5-1_5150_gal16v8as.ic9", 0, 1, NO_DUMP )
	ROM_LOAD( "5-1_5250_gal16v8as.ic8", 0, 1, NO_DUMP )
	ROM_LOAD( "7-4_7150_gal20v8as.ic7", 0, 1, NO_DUMP )
	ROM_LOAD( "7-4_7250_gal20v8as.ic54", 0, 1, NO_DUMP )
	ROM_LOAD( "7-4_7350_gal16v8as.ic5", 0, 1, NO_DUMP )
	ROM_LOAD( "7-4_7450_gal16v8as.ic9", 0, 1, NO_DUMP )
	ROM_LOAD( "7-4_7550_gal16v8as.ic59", 0, 1, NO_DUMP )
	ROM_LOAD( "7-4_7650_gal20v8as.ic44", 0, 1, NO_DUMP )
	ROM_LOAD( "51-3_503_gal16v8.ic46", 0, 1, NO_DUMP )
	ROM_LOAD( "cpu1_6-1_605_gal16v8as.ic13", 0, 1, NO_DUMP )
	ROM_LOAD( "cpu1_6-1_650_gal16v8as.ic7", 0, 1, NO_DUMP )
	ROM_LOAD( "cpu2_c_c50_gal16v8as.ic10", 0, 1, NO_DUMP )
	ROM_LOAD( "snd_9-2_9159_gal16v8as.ic42", 0, 1, NO_DUMP )
	ROM_LOAD( "snd_9-2_9250_gal20v8as.ic18", 0, 1, NO_DUMP )
	ROM_LOAD( "snd_9-2_9359_gal16v8as.ic10", 0, 1, NO_DUMP )
ROM_END

GAME( 1992, splashms,  splash,  splashms,  splashms,  splashms_state, init_splashms, ROT0, "Gaelco", "Splash (Modular System)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
