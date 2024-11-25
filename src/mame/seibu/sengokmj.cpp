// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli
/******************************************************************************************

Sengoku Mahjong (c) 1991 Sigma

driver by Angelo Salese & Pierpaolo Prazzoli

Uses the same Seibu custom chips of the D-Con HW.

TODO:
- Find what the remaining video C.R.T. registers do;
- Fix sprites bugs at a start of a play;
- Check NVRAM boundaries;
- How does the "SW Service Mode" (press F2 during gameplay) really work (inputs etc)? Nothing mapped works with it...

Notes:
- Some strings written in the sound ROM:
  "SENGOKU-MAHJONG Z80 PROGRAM ROM VERSION 1.00 WRITTEN BY K.SAEKI" at location 0x00c0-0x00ff.
  "Copyright 1990/1991 Sigma" at location 0x770-0x789.
- To bypass the startup message, toggle "Reset" dip-switch or reset with F3.
- If the Work RAM is not hooked-up (areas $67xx), a sound sample is played. I can't understand what it says though,
  appears to be the Japanese words for "RAM failed".
- Snippets of a non-BET Version are scattered thru the code (for example a credit display).
  Might be either undumped revision or selectable somehow.

CPU:    uPD70116C-8 (V30)
Sound:  Z80-A
        YM3812
        M6295
OSC:    14.31818MHz
        16.000MHz
Chips:  SEI0100 (YM3931, main/sub cpu interface)
        SEI0160
        SEI0200 (tilemap chip)
        SEI0210
        SEI0220 (sprite chip)


MAH1-1-1.915  samples

MAH1-2-1.013  sound prg. (ic1013:27c512)

MM01-1-1.21   main prg.
MM01-2-1.24

RS006.89      video timing?

RSSENGO0.64   chr.
RSSENGO1.68

RSSENGO2.72   chr.

*******************************************************************************************/

#include "emu.h"

#include "sei021x_sei0220_spr.h"

#include "seibusound.h"

#include "cpu/nec/nec.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "seibu_crtc.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class sengokmj_state : public driver_device, public seibu_sound_common
{
public:
	sengokmj_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritegen(*this, "spritegen"),
		m_sc0_vram(*this, "sc0_vram"),
		m_sc1_vram(*this, "sc1_vram"),
		m_sc2_vram(*this, "sc2_vram"),
		m_sc3_vram(*this, "sc3_vram"),
		m_spriteram16(*this, "sprite_ram")
	{ }

	void sengokmj(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<sei0210_device> m_spritegen;

	required_shared_ptr<uint16_t> m_sc0_vram;
	required_shared_ptr<uint16_t> m_sc1_vram;
	required_shared_ptr<uint16_t> m_sc2_vram;
	required_shared_ptr<uint16_t> m_sc3_vram;
	required_shared_ptr<uint16_t> m_spriteram16;

	tilemap_t *m_sc0_tilemap = nullptr;
	tilemap_t *m_sc1_tilemap = nullptr;
	tilemap_t *m_sc2_tilemap = nullptr;
	tilemap_t *m_sc3_tilemap = nullptr;

	uint16_t m_mux_data = 0;
	uint8_t m_hopper_io = 0;
	uint16_t m_layer_en = 0;
	uint16_t m_scrollram[6]{};

	uint16_t mahjong_panel_r();
	void mahjong_panel_w(uint16_t data);
	void out_w(uint16_t data);
	uint16_t system_r();
	void seibucrtc_sc0vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void seibucrtc_sc1vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void seibucrtc_sc2vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void seibucrtc_sc3vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void layer_en_w(uint16_t data);
	void layer_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(seibucrtc_sc0_tile_info);
	TILE_GET_INFO_MEMBER(seibucrtc_sc1_tile_info);
	TILE_GET_INFO_MEMBER(seibucrtc_sc2_tile_info);
	TILE_GET_INFO_MEMBER(seibucrtc_sc3_tile_info);

	void vblank_irq(int state);

	uint32_t pri_cb(uint8_t pri, uint8_t ext);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sengokmj_io_map(address_map &map) ATTR_COLD;
	void sengokmj_map(address_map &map) ATTR_COLD;
};


/*******************************
*
* Macros for the video registers
*
*******************************/

#define SEIBU_CRTC_ENABLE_SC0   (!(m_layer_en & 0x0001))
#define SEIBU_CRTC_ENABLE_SC2   (!(m_layer_en & 0x0002))
#define SEIBU_CRTC_ENABLE_SC1   (!(m_layer_en & 0x0004))
#define SEIBU_CRTC_ENABLE_SC3   (!(m_layer_en & 0x0008))
#define SEIBU_CRTC_ENABLE_SPR   (!(m_layer_en & 0x0010))

/************************************
* 0x20 - Screen 0 (BG) scroll x
************************************/
#define SEIBU_CRTC_SC0_SX   (m_scrollram[0])

/************************************
* 0x22 - Screen 0 (BG) scroll y
************************************/
#define SEIBU_CRTC_SC0_SY   (m_scrollram[1])

/************************************
* 0x24 - Screen 1 (FG) scroll x
************************************/
#define SEIBU_CRTC_SC1_SX   (m_scrollram[4])

/************************************
* 0x26 - Screen 1 (FG) scroll y
************************************/
#define SEIBU_CRTC_SC1_SY   (m_scrollram[5])

/************************************
* 0x28 - Screen 2 (MD) scroll x
************************************/
#define SEIBU_CRTC_SC2_SX   (m_scrollram[2])

/************************************
* 0x2a - Screen 2 (MD) scroll y
************************************/
#define SEIBU_CRTC_SC2_SY   (m_scrollram[3])

#if 0
/*******************************
* 0x1a - Layer Dynamic Paging?
*******************************/
#define SEIBU_CRTC_DYN_PAGING   (m_seibucrtc_vregs[0x001a/2])
#define SEIBU_CRTC_SC3_PAGE_SEL (SEIBU_CRTC_DYN_PAGING & 0x0002)

/*******************************
* 0x1c - Layer Enable
*******************************/



/************************************
* 0x2c - Fix screen scroll x (global)
************************************/
#define SEIBU_CRTC_FIX_SX   (m_seibucrtc_vregs[0x002c/2])

/************************************
* 0x2e - Fix screen scroll y (global)
************************************/
#define SEIBU_CRTC_FIX_SY   (m_seibucrtc_vregs[0x002e/2])
#endif

/*******************************
*
* Write RAM accesses
*
*******************************/

void sengokmj_state::seibucrtc_sc0vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sc0_vram[offset]);
	m_sc0_tilemap->mark_tile_dirty(offset);
}

void sengokmj_state::seibucrtc_sc2vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sc2_vram[offset]);
	m_sc2_tilemap->mark_tile_dirty(offset);
}

void sengokmj_state::seibucrtc_sc1vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sc1_vram[offset]);
	m_sc1_tilemap->mark_tile_dirty(offset);
}

void sengokmj_state::seibucrtc_sc3vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sc3_vram[offset]);
	m_sc3_tilemap->mark_tile_dirty(offset);
}

/*******************************
*
* Tilemap info accesses
*
*******************************/

TILE_GET_INFO_MEMBER( sengokmj_state::seibucrtc_sc0_tile_info )
{
	int tile = m_sc0_vram[tile_index] & 0xfff;
	int color = (m_sc0_vram[tile_index] >> 12) & 0x0f;
//  tile+=(m_seibucrtc_sc0bank<<12);
	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER( sengokmj_state::seibucrtc_sc2_tile_info )
{
	int tile = m_sc2_vram[tile_index] & 0xfff;
	int color = (m_sc2_vram[tile_index] >> 12) & 0x0f;
	tileinfo.set(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER( sengokmj_state::seibucrtc_sc1_tile_info )
{
	int tile = m_sc1_vram[tile_index] & 0xfff;
	int color = (m_sc1_vram[tile_index] >> 12) & 0x0f;
	tileinfo.set(2, tile, color, 0);
}

TILE_GET_INFO_MEMBER( sengokmj_state::seibucrtc_sc3_tile_info )
{
	int tile = m_sc3_vram[tile_index] & 0xfff;
	int color = (m_sc3_vram[tile_index] >> 12) & 0x0f;
	tileinfo.set(3, tile, color, 0);
}

uint32_t sengokmj_state::pri_cb(uint8_t pri, uint8_t ext)
{
	switch (pri)
	{
		case 0: return GFX_PMASK_8;
		case 1: return GFX_PMASK_8 | GFX_PMASK_4;
		case 2: return GFX_PMASK_8 | GFX_PMASK_4 | GFX_PMASK_2;
		case 3:
		default: return 0;
	}
}

void sengokmj_state::video_start()
{
	m_sc0_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sengokmj_state::seibucrtc_sc0_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_sc2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sengokmj_state::seibucrtc_sc2_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_sc1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sengokmj_state::seibucrtc_sc1_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_sc3_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sengokmj_state::seibucrtc_sc3_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_sc2_tilemap->set_transparent_pen(15);
	m_sc1_tilemap->set_transparent_pen(15);
	m_sc3_tilemap->set_transparent_pen(15);

	save_item(NAME(m_layer_en));
	save_item(NAME(m_scrollram));
}

uint32_t sengokmj_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(0x7ff), cliprect); //black pen

	/* TODO: offsetted? */
	m_sc0_tilemap->set_scrollx(0, (SEIBU_CRTC_SC0_SX + 128) & 0x1ff );
	m_sc0_tilemap->set_scrolly(0, (SEIBU_CRTC_SC0_SY) & 0x1ff );
	m_sc2_tilemap->set_scrollx(0, (SEIBU_CRTC_SC2_SX + 128) & 0x1ff );
	m_sc2_tilemap->set_scrolly(0, (SEIBU_CRTC_SC2_SY) & 0x1ff );
	m_sc1_tilemap->set_scrollx(0, (SEIBU_CRTC_SC1_SX + 128) & 0x1ff );
	m_sc1_tilemap->set_scrolly(0, (SEIBU_CRTC_SC1_SY) & 0x1ff );
	m_sc3_tilemap->set_scrollx(0, (128) & 0x1ff );
	m_sc3_tilemap->set_scrolly(0, (0) & 0x1ff );

	if(SEIBU_CRTC_ENABLE_SC0) { m_sc0_tilemap->draw(screen, bitmap, cliprect, 0, 1); }
	if(SEIBU_CRTC_ENABLE_SC2) { m_sc2_tilemap->draw(screen, bitmap, cliprect, 0, 2); }
	if(SEIBU_CRTC_ENABLE_SC1) { m_sc1_tilemap->draw(screen, bitmap, cliprect, 0, 4); }
	if(SEIBU_CRTC_ENABLE_SC3) { m_sc3_tilemap->draw(screen, bitmap, cliprect, 0, 8); }
	if(SEIBU_CRTC_ENABLE_SPR) { m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram16, m_spriteram16.bytes()); }

	return 0;
}


void sengokmj_state::machine_start()
{
	save_item(NAME(m_mux_data));
	save_item(NAME(m_hopper_io));
}


/* Multiplexer device for the mahjong panel */
uint16_t sengokmj_state::mahjong_panel_r()
{
	const char *const mpnames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5" };
	int i;
	uint16_t res = 0xffff;

	for(i=0;i<5;i++)
	{
		if(m_mux_data & 1 << i)
			res = ioport(mpnames[i])->read();
	}

	return res;
}

void sengokmj_state::mahjong_panel_w(uint16_t data)
{
	m_mux_data = (data & 0x3f00) >> 8;

	if(data & 0xc0ff)
		logerror("Write to mux %04x\n",data);
}

void sengokmj_state::out_w(uint16_t data)
{
	/* ---- ---- ---x ---- J.P. Signal (?)*/
	/* ---- ---- ---- -x-- Coin counter (done AFTER you press start)*/
	/* ---- ---- ---- --x- Cash enable (lockout)*/
	/* ---- ---- ---- ---x Hopper 10 */
	machine().bookkeeping().coin_lockout_w(0,~data & 2);
	machine().bookkeeping().coin_lockout_w(1,~data & 2);
	machine().bookkeeping().coin_counter_w(0,data & 4);
	m_hopper_io = ((data & 1)<<6);
//  popmessage("%02x",m_hopper_io);
}

uint16_t sengokmj_state::system_r()
{
	return (ioport("SYSTEM")->read() & 0xffbf) | m_hopper_io;
}

void sengokmj_state::sengokmj_map(address_map &map)
{
	map(0x00000, 0x07fff).ram();
	map(0x08000, 0x09fff).ram().share("nvram");
	map(0x0c000, 0x0c7ff).ram().w(FUNC(sengokmj_state::seibucrtc_sc0vram_w)).share("sc0_vram");
	map(0x0c800, 0x0cfff).ram().w(FUNC(sengokmj_state::seibucrtc_sc1vram_w)).share("sc1_vram");
	map(0x0d000, 0x0d7ff).ram().w(FUNC(sengokmj_state::seibucrtc_sc2vram_w)).share("sc2_vram");
	map(0x0d800, 0x0e7ff).ram().w(FUNC(sengokmj_state::seibucrtc_sc3vram_w)).share("sc3_vram");
	map(0x0e800, 0x0f7ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0f800, 0x0ffff).ram().share("sprite_ram");
	map(0xc0000, 0xfffff).rom();
}

void sengokmj_state::sengokmj_io_map(address_map &map)
{
	map(0x4000, 0x400f).rw("seibu_sound", FUNC(seibu_sound_device::main_r), FUNC(seibu_sound_device::main_w)).umask16(0x00ff);
	/*Areas from 8000-804f are for the custom Seibu CRTC.*/
	map(0x8000, 0x804f).rw("crtc", FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));

//  map(0x8080, 0x8081) CRTC extra register?
//  map(0x80c0, 0x80c1) CRTC extra register?
//  map(0x8100, 0x8101).nopw(); // always 0
	map(0x8180, 0x8181).w(FUNC(sengokmj_state::out_w));
	map(0x8140, 0x8141).w(FUNC(sengokmj_state::mahjong_panel_w));
	map(0xc000, 0xc001).portr("DSW");
	map(0xc002, 0xc003).r(FUNC(sengokmj_state::mahjong_panel_r));
	map(0xc004, 0xc005).r(FUNC(sengokmj_state::system_r)); //switches
}


static INPUT_PORTS_START( sengokmj )
	SEIBU_COIN_INPUTS   /* coin inputs read through sound cpu */

	PORT_START("DSW") // Names and locations from service mode
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Re-start" )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Double G" )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Double L" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Kamon" )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" )
	PORT_DIPNAME( 0x0040, 0x0040, "Out Sw" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) ) // One of these probably selects coins
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) ) // The other probably selects tickets
	PORT_DIPNAME( 0x0080, 0x0000, "Hopper" ) PORT_DIPLOCATION("SW1:8") //game gives hopper error with this off.
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0200, 0x0200, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A ) // Internal code 0F0h
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E ) // Internal code 0F4h
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I ) // Internal code 0F8h
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M ) // Internal code 0FCh
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) // Internal code 3h
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 ) // Internal code 0Ah
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Bet 2") PORT_CODE(KEYCODE_4) // Internal code 0FFFFh; ignored in service mode but probably does something
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B ) // Internal code 0F1h
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F ) // Internal code 0F5h
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J ) // Internal code 0F9h
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N ) // Internal code 0FBh
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) // Internal code 0Bh
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) // Internal code 9h
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C ) // Internal code 0F2h
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G ) // Internal code 0F6h
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K ) // Internal code 0FAh
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) // Internal code 1h
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) // Internal code 0Ch
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D ) // Internal code 0F3h
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H ) // Internal code 0F7h
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L ) // Internal code 0FBh
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) // Internal code 2h
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Internal code 6h; not shown in service mode and probably does nothing
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) // Internal code 4h
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) // Internal code 8h
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) // Internal code 7h
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Show Checksum") PORT_CODE(KEYCODE_X) // Internal code 5h; not shown in service mode but certainly does something
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) // Only used in service mode?
	PORT_SERVICE_NO_TOGGLE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Opt. 1st") // Only used in service mode?
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Cash") // Only used in service mode?
//  0x40 Hopper
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Meter")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 sprites  */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0,
				3+32*16, 2+32*16, 1+32*16, 0+32*16, 16+3+32*16, 16+2+32*16, 16+1+32*16, 16+0+32*16 },
	{ 0*16, 2*16, 4*16, 6*16, 8*16, 10*16, 12*16, 14*16,
			16*16, 18*16, 20*16, 22*16, 24*16, 26*16, 28*16, 30*16 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0 },
	{ 0*16, 2*16, 4*16, 6*16, 8*16, 10*16, 12*16, 14*16 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( gfx_sengokmj )
	GFXDECODE_ENTRY( "bg_gfx", 0, tilelayout, 0x400, 0x10 ) /* Tiles */
	GFXDECODE_ENTRY( "md_gfx", 0, tilelayout, 0x500, 0x10 ) /* Tiles */
	GFXDECODE_ENTRY( "fg_gfx", 0, tilelayout, 0x600, 0x10 ) /* Tiles */
	GFXDECODE_ENTRY( "tx_gfx", 0, charlayout, 0x700, 0x10 ) /* Text */
GFXDECODE_END

static GFXDECODE_START( gfx_sengokmj_spr )
	GFXDECODE_ENTRY( "spr_gfx",0, tilelayout, 0x000, 0x40 ) /* Sprites */
GFXDECODE_END

void sengokmj_state::vblank_irq(int state)
{
	if (state)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xc8/4); // V30
}

void sengokmj_state::layer_en_w(uint16_t data)
{
	m_layer_en = data;
}

void sengokmj_state::layer_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scrollram[offset]);
}


void sengokmj_state::sengokmj(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, 16000000/2); /* V30-8 */
	m_maincpu->set_addrmap(AS_PROGRAM, &sengokmj_state::sengokmj_map);
	m_maincpu->set_addrmap(AS_IO, &sengokmj_state::sengokmj_io_map);

	z80_device &audiocpu(Z80(config, "audiocpu", 14318180/4));
	audiocpu.set_addrmap(AS_PROGRAM, &sengokmj_state::seibu_sound_map);
	audiocpu.set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 320-1, 16, 256-1); //TODO: dynamic resolution
	screen.set_screen_update(FUNC(sengokmj_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(sengokmj_state::vblank_irq));

	seibu_crtc_device &crtc(SEIBU_CRTC(config, "crtc", 0));
	crtc.layer_en_callback().set(FUNC(sengokmj_state::layer_en_w));
	crtc.layer_scroll_callback().set(FUNC(sengokmj_state::layer_scroll_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sengokmj);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x800);

	SEI0210(config, m_spritegen, XTAL(14'318'181), m_palette, gfx_sengokmj_spr);
	m_spritegen->set_pri_callback(FUNC(sengokmj_state::pri_cb));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 14318180/4));
	ymsnd.irq_handler().set("seibu_sound", FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", 1320000, okim6295_device::PIN7_LOW));
	oki.add_route(ALL_OUTPUTS, "mono", 0.40);

	seibu_sound_device &seibu_sound(SEIBU_SOUND(config, "seibu_sound", 0));
	seibu_sound.int_callback().set_inputline("audiocpu", 0);
	seibu_sound.set_rom_tag("audiocpu");
	seibu_sound.set_rombank_tag("seibu_bank1");
	seibu_sound.ym_read_callback().set("ymsnd", FUNC(ym3812_device::read));
	seibu_sound.ym_write_callback().set("ymsnd", FUNC(ym3812_device::write));
}


ROM_START( sengokmj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 code */
	ROM_LOAD16_BYTE( "mm01-1-1.21",  0xc0000, 0x20000, CRC(74076b46) SHA1(64b0ed5a8c32e21157ae12fe40519e4c605b329c) )
	ROM_LOAD16_BYTE( "mm01-2-1.24",  0xc0001, 0x20000, CRC(f1a7c131) SHA1(d0fbbdedbff8f05da0e0296baa41369bc41a67e4) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "mah1-2-1.013", 0x000000, 0x08000, CRC(6a4f31b8) SHA1(5e1d7ed299c1fd65c7a43faa02831220f4251733) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x000000,     0x018000, 0x08000 )

	ROM_REGION( 0x100000, "spr_gfx", 0 ) /*Sprites gfx rom*/
	ROM_LOAD( "rssengo2.72", 0x00000, 0x100000, CRC(fb215ff8) SHA1(f98c0a53ad9b97d209dd1f85c994fc17ec585bd7) )

	ROM_REGION( 0x200000, "gfx_tiles", 0 ) /*Tiles data,to be reloaded*/
	ROM_LOAD( "rssengo0.64", 0x000000, 0x100000, CRC(36924b71) SHA1(814b2c69ab9876ccc57774e5718c05059ea23150) )
	ROM_LOAD( "rssengo1.68", 0x100000, 0x100000, CRC(1bbd00e5) SHA1(86391323b8e0d3b7e09a5914d87fb2adc48e5af4) )

	ROM_REGION( 0x080000, "bg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x000000, 0x00000, 0x080000)

	ROM_REGION( 0x080000, "md_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x080000, 0x00000, 0x080000)

	ROM_REGION( 0x080000, "fg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x100000, 0x00000, 0x080000)

	ROM_REGION( 0x080000, "tx_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x180000, 0x00000, 0x080000)

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "mah1-1-1.915", 0x00000, 0x20000, CRC(d4612e95) SHA1(937c5dbd25c89d4f4178b0bed510307020c5f40e) )

	ROM_REGION( 0x200, "user1", 0 ) /* not used */
	ROM_LOAD( "rs006.89", 0x000, 0x200, CRC(96f7646e) SHA1(400a831b83d6ac4d2a46ef95b97b1ee237099e44) ) /* Priority */
ROM_END

} // anonymous namespace

GAME( 1991, sengokmj, 0, sengokmj, sengokmj, sengokmj_state, empty_init, ROT0, "Sigma", "Sengoku Mahjong (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
/*Non-Bet Version?*/
