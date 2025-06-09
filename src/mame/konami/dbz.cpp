// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont, Pierpaolo Prazzoli
/*
  Dragon Ball Z                  (c) 1993 Banpresto
  Dragon Ball Z 2 - Super Battle (c) 1994 Banpresto

  Driver by David Haywood, R. Belmont and Pierpaolo Prazzoli

  MC68000 + Konami Xexex-era video hardware and system controller ICs
  Z80 + YM2151 + OKIM6295 for sound

  Note: game has an extremely complete test mode, it's beautiful for emulation.
        flip the DIP and check it out!

  TODO:
    - Self Test Fails
    - Banpresto logo in DBZ has bad colors after 1 run of the attract mode because
      it's associated to the wrong logical tilemap and the same happens in DBZ2
      test mode. It should be a bug in K056832 emulation.

PCB Layout:

BP924-1  PWB250248D (note PCB is identical to DBZ2 also)
|-------------------------------------------------------|
| YM3014  Z80    32MHz  053252       222A05   222A07    |
|   YM2151 5168                      222A04   222A06    |
| 1.056kHz 5168                                         |
|  M6295   222A10                           5864        |
|   222A03     68000                 2018   5864 053246A|
|J          222A11  222A12           2018               |
|A 5864        *       *                                |
|M 5864     62256   62256                               |
|M                                               053247A|
|A                                                      |
|                                053936 053936     2018 |
|    053251  053251                                2018 |
|                                        CY7C128        |
|       054157  054156  5864     CY7C128 CY7C128 CY7C128|
|       222A01  222A02  5864     CY7C128                |
| DSW2  DSW1            5864     CY7C128 222A08  222A09 |
|                                           *       *   |
|-------------------------------------------------------|

Notes:
      68k clock: 16.000MHz
      Z80 clock: 4.000MHz
   YM2151 clock: 4.000MHz
    M6295 clock: 1.056MHz (sample rate = /132)
          Vsync: 55Hz
          Hsync: 15.36kHz
              *: unpopulated ROM positions on DBZ

*/

#include "emu.h"

#include "k053246_k053247_k055673.h"
#include "k053251.h"
#include "k054156_k054157_k056832.h"
#include "konami_helper.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/k053252.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "video/k053936.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class dbz_state : public driver_device
{
public:
	dbz_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg1_videoram(*this, "bg1_videoram"),
		m_bg2_videoram(*this, "bg2_videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_k053252(*this, "k053252"),
		m_k056832(*this, "k056832"),
		m_k053936_1(*this, "k053936_1"),
		m_k053936_2(*this, "k053936_2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_dsw2(*this, "DSW2")
	{ }

	void dbz(machine_config &config);
	void dbz2bl(machine_config &config);

	void init_dbza();
	void init_dbz();
	void init_dbz2();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_bg1_videoram;
	required_shared_ptr<uint16_t> m_bg2_videoram;

	/* video-related */
	tilemap_t    *m_bg1_tilemap = nullptr;
	tilemap_t    *m_bg2_tilemap = nullptr;
	int          m_layer_colorbase[6]{};
	int          m_layerpri[5]{};
	int          m_sprite_colorbase = 0;

	/* misc */
	int           m_control = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	required_device<k053252_device> m_k053252;
	required_device<k056832_device> m_k056832;
	required_device<k053936_device> m_k053936_1;
	required_device<k053936_device> m_k053936_2;
	required_device<gfxdecode_device> m_gfxdecode;

	required_ioport m_dsw2;

	void dbzcontrol_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dbz_sound_cause_nmi(uint16_t data);
	void dbz_bg2_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dbz_bg1_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dbz_irq2_ack_w(int state);
	TILE_GET_INFO_MEMBER(get_dbz_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_dbz_bg1_tile_info);
	uint32_t screen_update_dbz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(dbz_scanline);
	K056832_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);
	void dbz_map(address_map &map) ATTR_COLD;
	void dbz2bl_map(address_map &map) ATTR_COLD;
	void dbz_sound_io_map(address_map &map) ATTR_COLD;
	void dbz_sound_map(address_map &map) ATTR_COLD;
};


K056832_CB_MEMBER(dbz_state::tile_callback)
{
	*color = (m_layer_colorbase[layer] << 1) + ((*color & 0x3c) >> 2);
}

K053246_CB_MEMBER(dbz_state::sprite_callback)
{
	int pri = (*color & 0x3c0) >> 5;

	if (pri <= m_layerpri[3])
		*priority_mask = 0xff00;
	else if (pri > m_layerpri[3] && pri <= m_layerpri[2])
		*priority_mask = 0xfff0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority_mask = 0xfffc;
	else
		*priority_mask = 0xfffe;

	*color = (m_sprite_colorbase << 1) + (*color & 0x1f);
}

/* Background Tilemaps */

void dbz_state::dbz_bg2_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg2_videoram[offset]);
	m_bg2_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(dbz_state::get_dbz_bg2_tile_info)
{
	int tileno, colour, flag;

	tileno = m_bg2_videoram[tile_index * 2 + 1] & 0x7fff;
	colour = (m_bg2_videoram[tile_index * 2] & 0x000f);
	flag = (m_bg2_videoram[tile_index * 2] & 0x0080) ? TILE_FLIPX : 0;

	tileinfo.set(0, tileno, colour + (m_layer_colorbase[5] << 1), flag);
}

void dbz_state::dbz_bg1_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg1_videoram[offset]);
	m_bg1_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(dbz_state::get_dbz_bg1_tile_info)
{
	int tileno, colour, flag;

	tileno = m_bg1_videoram[tile_index * 2 + 1] & 0x7fff;
	colour = (m_bg1_videoram[tile_index * 2] & 0x000f);
	flag = (m_bg1_videoram[tile_index * 2] & 0x0080) ? TILE_FLIPX : 0;

	tileinfo.set(1, tileno, colour + (m_layer_colorbase[4] << 1), flag);
}

void dbz_state::video_start()
{
	m_bg1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dbz_state::get_dbz_bg1_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_bg2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dbz_state::get_dbz_bg2_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);

	m_bg1_tilemap->set_transparent_pen(0);
	m_bg2_tilemap->set_transparent_pen(0);

	if (!strcmp(machine().system().name, "dbz"))
		m_k056832->set_layer_offs(0, -34, -16);
	else
		m_k056832->set_layer_offs(0, -35, -16);

	m_k056832->set_layer_offs(1, -31, -16);
	m_k056832->set_layer_offs(3, -31, -16); //?
}

uint32_t dbz_state::screen_update_dbz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const int K053251_CI[6] = { k053251_device::CI3, k053251_device::CI4, k053251_device::CI4, k053251_device::CI4, k053251_device::CI2, k053251_device::CI1 };
	int layer[5], plane, new_colorbase;

	m_sprite_colorbase = m_k053251->get_palette_index(k053251_device::CI0);

	for (plane = 0; plane < 6; plane++)
	{
		new_colorbase = m_k053251->get_palette_index(K053251_CI[plane]);
		if (m_layer_colorbase[plane] != new_colorbase)
		{
			m_layer_colorbase[plane] = new_colorbase;
			if (plane <= 3)
				m_k056832->mark_plane_dirty( plane);
			else if (plane == 4)
				m_bg1_tilemap->mark_all_dirty();
			else if (plane == 5)
				m_bg2_tilemap->mark_all_dirty();
		}
	}

	//layers priority

	layer[0] = 0;
	m_layerpri[0] = m_k053251->get_priority(k053251_device::CI3);
	layer[1] = 1;
	m_layerpri[1] = m_k053251->get_priority(k053251_device::CI4);
	layer[2] = 3;
	m_layerpri[2] = m_k053251->get_priority(k053251_device::CI0);
	layer[3] = 4;
	m_layerpri[3] = m_k053251->get_priority(k053251_device::CI2);
	layer[4] = 5;
	m_layerpri[4] = m_k053251->get_priority(k053251_device::CI1);

	konami_sortlayers5(layer, m_layerpri);

	screen.priority().fill(0, cliprect);

	for (plane = 0; plane < 5; plane++)
	{
		int flag, pri;

		if (plane == 0)
		{
			flag = TILEMAP_DRAW_OPAQUE;
			pri = 0;
		}
		else
		{
			flag = 0;
			pri = 1 << (plane - 1);
		}

		if(layer[plane] == 4)
			m_k053936_2->zoom_draw(screen, bitmap, cliprect, m_bg1_tilemap, flag, pri, 1);
		else if(layer[plane] == 5)
			m_k053936_1->zoom_draw(screen, bitmap, cliprect, m_bg2_tilemap, flag, pri, 1);
		else
			m_k056832->tilemap_draw(screen, bitmap, cliprect, layer[plane], flag, pri);
	}

	m_k053246->k053247_sprites_draw( bitmap, cliprect);
	return 0;
}


TIMER_DEVICE_CALLBACK_MEMBER(dbz_state::dbz_scanline)
{
	int scanline = param;

	if(scanline == 256) // vblank-out irq
		m_maincpu->set_input_line(M68K_IRQ_2, ASSERT_LINE);

	if(scanline == 0 && m_k053246->k053246_is_irq_enabled()) // vblank-in irq
		m_maincpu->set_input_line(M68K_IRQ_4, HOLD_LINE); //auto-acks apparently
}

#if 0
uint16_t dbz_state::dbzcontrol_r()
{
	return m_control;
}
#endif

void dbz_state::dbzcontrol_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* bit 10 = enable '246 readback */

	COMBINE_DATA(&m_control);

	if (data & 0x400)
		m_k053246->k053246_set_objcha_line( ASSERT_LINE);
	else
		m_k053246->k053246_set_objcha_line( CLEAR_LINE);

	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
}

void dbz_state::dbz_sound_cause_nmi(uint16_t data)
{
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void dbz_state::dbz_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x480000, 0x48ffff).ram();
	map(0x490000, 0x491fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));  // '157 RAM is mirrored twice
	map(0x492000, 0x493fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));
	map(0x498000, 0x49ffff).r(m_k056832, FUNC(k056832_device::rom_word_8000_r));  // code near a60 in dbz2, subroutine at 730 in dbz
	map(0x4a0000, 0x4a0fff).rw(m_k053246, FUNC(k053247_device::k053247_word_r), FUNC(k053247_device::k053247_word_w));
	map(0x4a1000, 0x4a3fff).ram();
	map(0x4a8000, 0x4abfff).ram().w("palette", FUNC(palette_device::write16)).share("palette"); // palette
	map(0x4c0000, 0x4c0001).r(m_k053246, FUNC(k053247_device::k053246_r));
	map(0x4c0000, 0x4c0007).w(m_k053246, FUNC(k053247_device::k053246_w));
	map(0x4c4000, 0x4c4007).w(m_k053246, FUNC(k053247_device::k053246_w));
	map(0x4c8000, 0x4c8007).w(m_k056832, FUNC(k056832_device::b_word_w));
	map(0x4cc000, 0x4cc03f).w(m_k056832, FUNC(k056832_device::word_w));
	map(0x4d0000, 0x4d001f).w(m_k053936_1, FUNC(k053936_device::ctrl_w));
	map(0x4d4000, 0x4d401f).w(m_k053936_2, FUNC(k053936_device::ctrl_w));
	map(0x4e0000, 0x4e0001).portr("P1_P2");
	map(0x4e0002, 0x4e0003).portr("SYSTEM_DSW1");
	map(0x4e4000, 0x4e4001).lr8(NAME([this]() { return uint8_t(m_dsw2->read()); }));
	map(0x4e8000, 0x4e8001).noprw();
	map(0x4ec000, 0x4ec001).w(FUNC(dbz_state::dbzcontrol_w));
	map(0x4f0000, 0x4f0000).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x4f4000, 0x4f4001).w(FUNC(dbz_state::dbz_sound_cause_nmi));
	map(0x4f8000, 0x4f801f).rw(m_k053252, FUNC(k053252_device::read), FUNC(k053252_device::write)).umask16(0xff00);      // 252
	map(0x4fc000, 0x4fc01f).w(m_k053251, FUNC(k053251_device::write)).umask16(0x00ff);   // 251

	map(0x500000, 0x501fff).ram().w(FUNC(dbz_state::dbz_bg2_videoram_w)).share("bg2_videoram");
	map(0x508000, 0x509fff).ram().w(FUNC(dbz_state::dbz_bg1_videoram_w)).share("bg1_videoram");
	map(0x510000, 0x513fff).rw(m_k053936_1, FUNC(k053936_device::linectrl_r), FUNC(k053936_device::linectrl_w)); // ?? guess, it might not be
	map(0x518000, 0x51bfff).rw(m_k053936_2, FUNC(k053936_device::linectrl_r), FUNC(k053936_device::linectrl_w)); // ?? guess, it might not be
	map(0x600000, 0x6fffff).nopr();             // PSAC 1 ROM readback window
	map(0x700000, 0x7fffff).nopr();             // PSAC 2 ROM readback window
}

void dbz_state::dbz2bl_map(address_map &map)
{
	dbz_map(map);

	map(0x4cc000, 0x4cc03f).unmapw();
	map(0x4ccf00, 0x4ccf3f).w(m_k056832, FUNC(k056832_device::word_w));

	map(0x4d4000, 0x4d401f).unmapw();
	map(0x4d4f00, 0x4d4f1f).w(m_k053936_2, FUNC(k053936_device::ctrl_w));

	map(0x4e4000, 0x4e4001).unmapr();
	map(0x4e0004, 0x4e0005).lr8(NAME([this]() { return uint8_t(m_dsw2->read()); }));

	map(0x4e3011, 0x4e3011).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x4f0000, 0x4f0001).unmapw();
	map(0x4f4000, 0x4f4001).unmapw(); // the bootleg nops this, so sound doesn't work with current emulation
}

/* dbz sound */
/* IRQ: from YM2151.  NMI: from 68000.  Port 0: write to ack NMI */

void dbz_state::dbz_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).ram();
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xd000, 0xd002).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe000, 0xe000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

void dbz_state::dbz_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).nopw();
}

/**********************************************************************************/


static INPUT_PORTS_START( dbz )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM_DSW1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2") // I think this is right, but can't stomach the game long enough to check
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x2000, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW1:7" )                       // seems unused
	PORT_DIPNAME( 0x8000, 0x0000, "Mask ROM Test" )     PORT_DIPLOCATION("SW1:8")           //NOP'd
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "No Coin B" )
	/* "No Coin B" = coins produce sound, but no effect on coin counter */
INPUT_PORTS_END

static INPUT_PORTS_START( dbza )
	PORT_INCLUDE( dbz )

	PORT_MODIFY("SYSTEM_DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW1:8" )                       // tests are always performed at start
INPUT_PORTS_END

static INPUT_PORTS_START( dbz2 )
	PORT_INCLUDE( dbz )

	PORT_MODIFY("SYSTEM_DSW1")
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Level_Select ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Japanese ) )
INPUT_PORTS_END

/**********************************************************************************/

static GFXDECODE_START( gfx_dbz )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_16x16x4_packed_msb, 0, 512 )
	GFXDECODE_ENTRY( "gfx4", 0, gfx_16x16x4_packed_msb, 0, 512 )
GFXDECODE_END

/**********************************************************************************/

void dbz_state::dbz_irq2_ack_w(int state)
{
	m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
}

void dbz_state::machine_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_layerpri));
	save_item(NAME(m_layer_colorbase));
}

void dbz_state::machine_reset()
{
	int i;

	for (i = 0; i < 5; i++)
		m_layerpri[i] = 0;

	for (i = 0; i < 6; i++)
		m_layer_colorbase[i] = 0;

	m_sprite_colorbase = 0;
	m_control = 0;
}

void dbz_state::dbz(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &dbz_state::dbz_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(dbz_state::dbz_scanline), "screen", 0, 1);

	Z80(config, m_audiocpu, 4000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &dbz_state::dbz_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &dbz_state::dbz_sound_io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(55);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 40*8);
	screen.set_visarea(0, 48*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(dbz_state::screen_update_dbz));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_dbz);

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x4000/2).enable_shadows();

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(dbz_state::tile_callback));
	m_k056832->set_config(K056832_BPP_4, 1, 1);
	m_k056832->set_palette("palette");

	K053246(config, m_k053246, 0);
	m_k053246->set_sprite_callback(FUNC(dbz_state::sprite_callback));
	m_k053246->set_config(NORMAL_PLANE_ORDER, -87, 32); // or -52, 16?
	m_k053246->set_palette("palette");

	K053251(config, m_k053251, 0);

	K053936(config, m_k053936_1, 0);
	m_k053936_1->set_wrap(1);
	m_k053936_1->set_offsets(-46, -16);

	K053936(config, m_k053936_2, 0);
	m_k053936_2->set_wrap(1);
	m_k053936_2->set_offsets(-46, -16);

	K053252(config, m_k053252, 16000000/2);
	m_k053252->int1_ack().set(FUNC(dbz_state::dbz_irq2_ack_w));

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 4000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "speaker", 1.0, 0);
	ymsnd.add_route(1, "speaker", 1.0, 1);

	okim6295_device &oki(OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	oki.add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}

void dbz_state::dbz2bl(machine_config &config)
{
	dbz(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &dbz_state::dbz2bl_map);
}
/**********************************************************************************/

ROM_START( dbz )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0)
	ROM_LOAD16_BYTE( "222b11.9e", 0x000000, 0x80000, CRC(4c6b75e9) SHA1(8b1d67f8c8b64bb38f824506eca4c6966215f233) )
	ROM_LOAD16_BYTE( "222b12.9f", 0x000001, 0x80000, CRC(48637fce) SHA1(d3db0d56b70b9a4b20c645dda15327ec60e69d81) )

	/* sound program */
	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD("222a10.5e", 0x000000, 0x08000, CRC(1c93e30a) SHA1(8545a0ac5126b3c855e1901b186f57820699895d) )

	/* tiles */
	ROM_REGION( 0x400000, "k056832", 0)
	ROM_LOAD32_WORD( "222a01.27c", 0x000000, 0x200000, CRC(9fce4ed4) SHA1(81e19375b351ee247f066434dd595149333d73c5) )
	ROM_LOAD32_WORD( "222a02.27e", 0x000002, 0x200000, CRC(651acaa5) SHA1(33942a90fb294b5da6a48e5bfb741b31babca188) )

	/* sprites */
	ROM_REGION( 0x800000, "k053246", 0)
	ROM_LOAD64_WORD( "222a04.3j", 0x000000, 0x200000, CRC(2533b95a) SHA1(35910836b6030130d742eae6c4bf1cdf1ff43fa4) )
	ROM_LOAD64_WORD( "222a05.1j", 0x000002, 0x200000, CRC(731b7f93) SHA1(b676fff2ede5aa72c49fe12736cd60766462fe0b) )
	ROM_LOAD64_WORD( "222a06.3l", 0x000004, 0x200000, CRC(97b767d3) SHA1(3d879c431586da2f88c632ab1a531b4a5ec96939) )
	ROM_LOAD64_WORD( "222a07.1l", 0x000006, 0x200000, CRC(430bc873) SHA1(ea483195bb7f20ef3df7cfba153e5f6f8d53e5f9) )

	/* K053536 PSAC-2 #1 */
	ROM_REGION( 0x200000, "gfx3", 0)
	ROM_LOAD( "222a08.25k", 0x000000, 0x200000, CRC(6410ee1b) SHA1(2296aafd3ba25f63a12130f7b58de53e88f14e92) )

	/* K053536 PSAC-2 #2 */
	ROM_REGION( 0x200000, "gfx4", 0)
	ROM_LOAD( "222a09.25l", 0x000000, 0x200000, CRC(f7b3f070) SHA1(50ebd8cfcda292a3df5664de50f9212108d58923) )

	/* sound data */
	ROM_REGION( 0x40000, "oki", 0)
	ROM_LOAD( "222a03.7c", 0x000000, 0x40000, CRC(1924467b) SHA1(57922090509bcc63b4783e8f2c5e95afd2090b87) )
ROM_END

ROM_START( dbza )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0)
	ROM_LOAD16_BYTE( "222a11.9e", 0x000000, 0x80000, CRC(60c7d9b2) SHA1(718ef89e89b3943845e91bedfc5c1d26229f9fe5) )
	ROM_LOAD16_BYTE( "222a12.9f", 0x000001, 0x80000, CRC(6ebc6853) SHA1(e9b2068246228968cc6b8554215563cacaa5ba9f) )

	/* sound program */
	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD("222a10.5e", 0x000000, 0x08000, CRC(1c93e30a) SHA1(8545a0ac5126b3c855e1901b186f57820699895d) )

	/* tiles */
	ROM_REGION( 0x400000, "k056832", 0)
	ROM_LOAD32_WORD( "222a01.27c", 0x000000, 0x200000, CRC(9fce4ed4) SHA1(81e19375b351ee247f066434dd595149333d73c5) )
	ROM_LOAD32_WORD( "222a02.27e", 0x000002, 0x200000, CRC(651acaa5) SHA1(33942a90fb294b5da6a48e5bfb741b31babca188) )

	/* sprites */
	ROM_REGION( 0x800000, "k053246", 0)
	ROM_LOAD64_WORD( "222a04.3j", 0x000000, 0x200000, CRC(2533b95a) SHA1(35910836b6030130d742eae6c4bf1cdf1ff43fa4) )
	ROM_LOAD64_WORD( "222a05.1j", 0x000002, 0x200000, CRC(731b7f93) SHA1(b676fff2ede5aa72c49fe12736cd60766462fe0b) )
	ROM_LOAD64_WORD( "222a06.3l", 0x000004, 0x200000, CRC(97b767d3) SHA1(3d879c431586da2f88c632ab1a531b4a5ec96939) )
	ROM_LOAD64_WORD( "222a07.1l", 0x000006, 0x200000, CRC(430bc873) SHA1(ea483195bb7f20ef3df7cfba153e5f6f8d53e5f9) )

	/* K053536 PSAC-2 #1 */
	ROM_REGION( 0x200000, "gfx3", 0)
	ROM_LOAD( "222a08.25k", 0x000000, 0x200000, CRC(6410ee1b) SHA1(2296aafd3ba25f63a12130f7b58de53e88f14e92) )

	/* K053536 PSAC-2 #2 */
	ROM_REGION( 0x200000, "gfx4", 0)
	ROM_LOAD( "222a09.25l", 0x000000, 0x200000, CRC(f7b3f070) SHA1(50ebd8cfcda292a3df5664de50f9212108d58923) )

	/* sound data */
	ROM_REGION( 0x40000, "oki", 0)
	ROM_LOAD( "222a03.7c", 0x000000, 0x40000, CRC(1924467b) SHA1(57922090509bcc63b4783e8f2c5e95afd2090b87) )
ROM_END

ROM_START( dbz2 )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0)
	ROM_LOAD16_BYTE( "a9e.9e", 0x000000, 0x80000, CRC(e6a142c9) SHA1(7951c8f7036a67a0cd3260f434654820bf3e603f) )
	ROM_LOAD16_BYTE( "a9f.9f", 0x000001, 0x80000, CRC(76cac399) SHA1(af6daa1f8b87c861dc62adef5ca029190c3cb9ae) )

	/* sound program */
	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD("s-001.5e", 0x000000, 0x08000, CRC(154e6d03) SHA1(db15c20982692271f40a733dfc3f2486221cd604) )

	/* tiles */
	ROM_REGION( 0x400000, "k056832", 0)
	ROM_LOAD32_WORD( "ds-b01.27c", 0x000000, 0x200000, CRC(8dc39972) SHA1(c6e3d4e0ff069e08bdb68e2b0ad24cc7314e4e93) )
	ROM_LOAD32_WORD( "ds-b02.27e", 0x000002, 0x200000, CRC(7552f8cd) SHA1(1f3beffe9733b1a18d44b5e8880ff1cc97e7a8ab) )

	/* sprites */
	ROM_REGION( 0x800000, "k053246", 0)
	ROM_LOAD64_WORD( "ds-o01.3j", 0x000000, 0x200000, CRC(d018531f) SHA1(d4082fe28e9f1f3f35aa75b4be650cadf1cef192) )
	ROM_LOAD64_WORD( "ds-o02.1j", 0x000002, 0x200000, CRC(5a0f1ebe) SHA1(3bb9e1389299dc046a24740ef1a1c543e44b5c37) )
	ROM_LOAD64_WORD( "ds-o03.3l", 0x000004, 0x200000, CRC(ddc3bef1) SHA1(69638ef53f627a238a12b6c206d57faadf894893) )
	ROM_LOAD64_WORD( "ds-o04.1l", 0x000006, 0x200000, CRC(b5df6676) SHA1(194cfce460ccd29e2cceec577aae4ec936ae88e5) )

	/* K053536 PSAC-2 #1 */
	ROM_REGION( 0x400000, "gfx3", 0)
	ROM_LOAD( "ds-p01.25k", 0x000000, 0x200000, CRC(1c7aad68) SHA1(a5296cf12cec262eede55397ea929965576fea81) )
	ROM_LOAD( "ds-p02.27k", 0x200000, 0x200000, CRC(e4c3a43b) SHA1(f327f75fe82f8aafd2cfe6bdd3a426418615974b) )

	/* K053536 PSAC-2 #2 */
	ROM_REGION( 0x400000, "gfx4", 0)
	ROM_LOAD( "ds-p03.25l", 0x000000, 0x200000, CRC(1eaa671b) SHA1(1875eefc6f2c3fc8feada56bfa6701144e8ef64b) )
	ROM_LOAD( "ds-p04.27l", 0x200000, 0x200000, CRC(5845ff98) SHA1(73b4c3f439321ce9c462119fe933e7cbda8cd498) )

	/* sound data */
	ROM_REGION( 0x40000, "oki", 0)
	ROM_LOAD( "pcm.7c", 0x000000, 0x40000, CRC(b58c884a) SHA1(0e2a7267e9dff29c9af25558081ec9d56629bc43) )
ROM_END

// This PCB doesn't have Konami's custom chips.
// Only the program /audio ROMs were dumped, all the mask ROMs were Banpresto original with same ROM number, so they're supposed to match, but marked as BAD_DUMP as precaution
ROM_START( dbz2bl )
	ROM_REGION( 0x400000, "maincpu", 0)
	ROM_LOAD16_BYTE( "374.bin", 0x000000, 0x80000, CRC(cb0b2fdc) SHA1(c791e788a8c2ab402afed215e70de1a66ab0e2a2) )
	ROM_LOAD16_BYTE( "273.bin", 0x000001, 0x80000, CRC(55889c38) SHA1(9fa96e9c96abe42221ca3f383b8a2cc4bf6af979) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD("s-001.5e", 0x000000, 0x08000, CRC(154e6d03) SHA1(db15c20982692271f40a733dfc3f2486221cd604) )

	// tiles
	ROM_REGION( 0x400000, "k056832", 0)
	ROM_LOAD32_WORD( "ds-b01.27c", 0x000000, 0x200000, BAD_DUMP CRC(8dc39972) SHA1(c6e3d4e0ff069e08bdb68e2b0ad24cc7314e4e93) )
	ROM_LOAD32_WORD( "ds-b02.27e", 0x000002, 0x200000, BAD_DUMP CRC(7552f8cd) SHA1(1f3beffe9733b1a18d44b5e8880ff1cc97e7a8ab) )

	// sprites
	ROM_REGION( 0x800000, "k053246", 0)
	ROM_LOAD64_WORD( "ds-o01.3j", 0x000000, 0x200000, BAD_DUMP CRC(d018531f) SHA1(d4082fe28e9f1f3f35aa75b4be650cadf1cef192) )
	ROM_LOAD64_WORD( "ds-o02.1j", 0x000002, 0x200000, BAD_DUMP CRC(5a0f1ebe) SHA1(3bb9e1389299dc046a24740ef1a1c543e44b5c37) )
	ROM_LOAD64_WORD( "ds-o03.3l", 0x000004, 0x200000, BAD_DUMP CRC(ddc3bef1) SHA1(69638ef53f627a238a12b6c206d57faadf894893) )
	ROM_LOAD64_WORD( "ds-o04.1l", 0x000006, 0x200000, BAD_DUMP CRC(b5df6676) SHA1(194cfce460ccd29e2cceec577aae4ec936ae88e5) )

	// K053536 PSAC-2 #1 equivalent
	ROM_REGION( 0x400000, "gfx3", 0)
	ROM_LOAD( "ds-p01.25k", 0x000000, 0x200000, BAD_DUMP CRC(1c7aad68) SHA1(a5296cf12cec262eede55397ea929965576fea81) )
	ROM_LOAD( "ds-p02.27k", 0x200000, 0x200000, BAD_DUMP CRC(e4c3a43b) SHA1(f327f75fe82f8aafd2cfe6bdd3a426418615974b) )

	// K053536 PSAC-2 #2 equivalent
	ROM_REGION( 0x400000, "gfx4", 0)
	ROM_LOAD( "ds-p03.25l", 0x000000, 0x200000, BAD_DUMP CRC(1eaa671b) SHA1(1875eefc6f2c3fc8feada56bfa6701144e8ef64b) )
	ROM_LOAD( "ds-p04.27l", 0x200000, 0x200000, BAD_DUMP CRC(5845ff98) SHA1(73b4c3f439321ce9c462119fe933e7cbda8cd498) )

	ROM_REGION( 0x40000, "oki", 0)
	ROM_LOAD( "pcm.7c", 0x000000, 0x40000, CRC(b58c884a) SHA1(0e2a7267e9dff29c9af25558081ec9d56629bc43) )
ROM_END

void dbz_state::init_dbz()
{
	uint16_t *ROM = (uint16_t *)memregion("maincpu")->base();

	// to avoid crash during loop at 0x00076e after D4 > 0x80 (reading tiles region out of bounds)
	ROM[0x76c/2] = 0x007f;    /* 0x00ff */
	// nop out dbz1's mask rom test
	// tile ROM test
	ROM[0x7b0/2] = 0x4e71;    /* 0x0c43 - cmpi.w  #-$1e0d, D3 */
	ROM[0x7b2/2] = 0x4e71;    /* 0xe1f3 */
	ROM[0x7b4/2] = 0x4e71;    /* 0x6600 - bne     $7d6 */
	ROM[0x7b6/2] = 0x4e71;    /* 0x0020 */
	ROM[0x7c0/2] = 0x4e71;    /* 0x0c45 - cmpi.w  #-$7aad, D5 */
	ROM[0x7c2/2] = 0x4e71;    /* 0x8553 */
	ROM[0x7c4/2] = 0x4e71;    /* 0x6600 - bne     $7d6 */
	ROM[0x7c6/2] = 0x4e71;    /* 0x0010 */
	// PSAC2 ROM test (A and B)
	ROM[0x9a8/2] = 0x4e71;    /* 0x0c43 - cmpi.w  #$43c0, D3 */
	ROM[0x9aa/2] = 0x4e71;    /* 0x43c0 */
	ROM[0x9ac/2] = 0x4e71;    /* 0x6600 - bne     $a00 */
	ROM[0x9ae/2] = 0x4e71;    /* 0x0052 */
	ROM[0x9ea/2] = 0x4e71;    /* 0x0c44 - cmpi.w  #-$13de, D4 */
	ROM[0x9ec/2] = 0x4e71;    /* 0xec22 */
	ROM[0x9ee/2] = 0x4e71;    /* 0x6600 - bne     $a00 */
	ROM[0x9f0/2] = 0x4e71;    /* 0x0010 */
	// prog ROM test
	ROM[0x80c/2] = 0x4e71;    /* 0xb650 - cmp.w   (A0), D3 */
	ROM[0x80e/2] = 0x4e71;    /* 0x6600 - bne     $820 */
	ROM[0x810/2] = 0x4e71;    /* 0x005e */
}

void dbz_state::init_dbza()
{
	uint16_t *ROM = (uint16_t *)memregion("maincpu")->base();

	// nop out dbz1's mask rom test
	// tile ROM test
	ROM[0x78c/2] = 0x4e71;    /* 0x0c43 - cmpi.w  #-$1236, D3 */
	ROM[0x78e/2] = 0x4e71;    /* 0x0010 */
	ROM[0x790/2] = 0x4e71;    /* 0x6600 - bne     $7a2 */
	ROM[0x792/2] = 0x4e71;    /* 0x0010 */
	// PSAC2 ROM test
	ROM[0x982/2] = 0x4e71;    /* 0x0c43 - cmpi.w  #$437e, D3 */
	ROM[0x984/2] = 0x4e71;    /* 0x437e */
	ROM[0x986/2] = 0x4e71;    /* 0x6600 - bne     $9a0 */
	ROM[0x988/2] = 0x4e71;    /* 0x0018 */
	ROM[0x98a/2] = 0x4e71;    /* 0x0c44 - cmpi.w  #$65e8, D4 */
	ROM[0x98c/2] = 0x4e71;    /* 0x65e8 */
	ROM[0x98e/2] = 0x4e71;    /* 0x6600 - bne     $9a0 */
	ROM[0x990/2] = 0x4e71;    /* 0x0010 */
}

void dbz_state::init_dbz2()
{
	uint16_t *ROM = (uint16_t *)memregion("maincpu")->base();

	// to avoid crash during loop at 0x000a4a after D4 > 0x80 (reading tiles region out of bounds)
	ROM[0xa48/2] = 0x007f;    /* 0x00ff */
	// nop out dbz1's mask rom test
	// tile ROM test
	ROM[0xa88/2] = 0x4e71;    /* 0x0c43 - cmpi.w  #$e58, D3 */
	ROM[0xa8a/2] = 0x4e71;    /* 0x0e58 */
	ROM[0xa8c/2] = 0x4e71;    /* 0x6600 - bne     $aae */
	ROM[0xa8e/2] = 0x4e71;    /* 0x0020 */
	ROM[0xa98/2] = 0x4e71;    /* 0x0c45 - cmpi.w  #-$3d20, D5 */
	ROM[0xa9a/2] = 0x4e71;    /* 0xc2e0 */
	ROM[0xa9c/2] = 0x4e71;    /* 0x6600 - bne     $aae */
	ROM[0xa9e/2] = 0x4e71;    /* 0x0010 */
	// PSAC2 ROM test (0 to 3)
	ROM[0xc66/2] = 0x4e71;    /* 0xb549 - cmpm.w  (A1)+, (A2)+ */
	ROM[0xc68/2] = 0x4e71;    /* 0x6600 - bne     $cc8 */
	ROM[0xc6a/2] = 0x4e71;    /* 0x005e */
	ROM[0xc7c/2] = 0x4e71;    /* 0xb549 - cmpm.w  (A1)+, (A2)+ */
	ROM[0xc7e/2] = 0x4e71;    /* 0x6600 - bne     $cc8 */
	ROM[0xc80/2] = 0x4e71;    /* 0x0048 */
	ROM[0xc9e/2] = 0x4e71;    /* 0xb549 - cmpm.w  (A1)+, (A2)+ */
	ROM[0xca0/2] = 0x4e71;    /* 0x6600 - bne     $cc8 */
	ROM[0xca2/2] = 0x4e71;    /* 0x0026 */
	ROM[0xcb4/2] = 0x4e71;    /* 0xb549 - cmpm.w  (A1)+, (A2)+ */
	ROM[0xcb6/2] = 0x4e71;    /* 0x6600 - bne     $cc8 */
	ROM[0xcb8/2] = 0x4e71;    /* 0x0010 */
	// prog ROM test
	ROM[0xae4/2] = 0x4e71;    /* 0xb650 - cmp.w   (A0), D3 */
	ROM[0xae6/2] = 0x4e71;    /* 0x6600 - bne     $af8 */
	ROM[0xae8/2] = 0x4e71;    /* 0x005e */
}

} // anonymous namespace

GAME( 1993, dbz,    0,    dbz,    dbz,  dbz_state, init_dbz,  ROT0, "Banpresto", "Dragon Ball Z (rev B)",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // crashes MAME in tile/PSAC2 ROM test
GAME( 1993, dbza,   dbz,  dbz,    dbza, dbz_state, init_dbza, ROT0, "Banpresto", "Dragon Ball Z (rev A)",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, dbz2,   0,    dbz,    dbz2, dbz_state, init_dbz2, ROT0, "Banpresto", "Dragon Ball Z 2 - Super Battle",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // crashes MAME in tile/PSAC2 ROM test
GAME( 1994, dbz2bl, dbz2, dbz2bl, dbz2, dbz_state, init_dbz2, ROT0, "bootleg",   "Dragon Ball Z 2 - Super Battle (bootleg)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // heavy priority / GFX issues, no sound
