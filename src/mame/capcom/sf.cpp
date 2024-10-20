// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    Street Fighter

    driver by Olivier Galibert

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "machine/gen_latch.h"
#include "screen.h"
#include "sound/msm5205.h"
#include "sound/ymopm.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class sf_state : public driver_device
{
public:
	sf_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_protcpu(*this, "protcpu"),
		m_msm(*this, "msm%u", 1U),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_objectram(*this, "objectram"),
		m_tilerom(*this, "tilerom"),
		m_audiobank(*this, "audiobank")
	{ }

	void sfp(machine_config &config);
	void sfjp(machine_config &config);
	void sfus(machine_config &config);
	void sfan(machine_config &config);

private:
	/* devices */
	required_device<m68000_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	optional_device<i8751_device> m_protcpu;
	required_device_array<msm5205_device, 2> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_objectram;
	required_region_ptr<uint8_t> m_tilerom;

	required_memory_bank m_audiobank;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;
	int m_active;
	bool m_prot_t0;
	uint16_t m_bgscroll;
	uint16_t m_fgscroll;

	void coin_w(u8 data);
	void soundcmd_w(u8 data);
	void protection_w(u16);
	void sound2_bank_w(u8 data);
	void videoram_w(offs_t offset, u16 data, u16 mem_mask);
	void bg_scroll_w(offs_t, u16 data, u16 mem_mask);
	void fg_scroll_w(offs_t, u16 data, u16 mem_mask);
	void gfxctrl_w(offs_t, u16 data, u16 mem_mask);
	template<int Chip> void msm_w(u8 data);
	void prot_p3_w(u8 data);
	void prot_ram_w(offs_t offset, u8 data);
	u8 prot_ram_r(offs_t offset);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline int invert( int nb );
	void draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect );
	void write_dword( address_space &space, offs_t offset, uint32_t data );

	void sfan_map(address_map &map) ATTR_COLD;
	void sfjp_map(address_map &map) ATTR_COLD;
	void sfus_map(address_map &map) ATTR_COLD;
	void sound2_io_map(address_map &map) ATTR_COLD;
	void sound2_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void prot_map(address_map &map) ATTR_COLD;
};

void sf_state::coin_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0,  data & 0x01);
	machine().bookkeeping().coin_counter_w(1,  data & 0x02);
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x10);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x20);
	machine().bookkeeping().coin_lockout_w(2, ~data & 0x40); /* is there a third coin input? */
}

void sf_state::soundcmd_w(u8 data)
{
	m_soundlatch->write(data & 0xff);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void sf_state::sound2_bank_w(u8 data)
{
	m_audiobank->set_entry(data);
}

template<int Chip>
void sf_state::msm_w(u8 data)
{
	m_msm[Chip]->reset_w(BIT(data, 7));
	/* ?? bit 6?? */
	m_msm[Chip]->data_w(data);
	m_msm[Chip]->vclk_w(1);
	m_msm[Chip]->vclk_w(0);
}

void sf_state::sfan_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x04ffff).rom();
	map(0x800000, 0x800fff).ram().w(FUNC(sf_state::videoram_w)).share("videoram");
	map(0xb00000, 0xb007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xc00000, 0xc00001).portr("IN0");
	map(0xc00002, 0xc00003).portr("IN1");
	map(0xc00004, 0xc00005).portr("PUNCH");
	map(0xc00006, 0xc00007).portr("KICK");
	map(0xc00008, 0xc00009).portr("DSW1");
	map(0xc0000a, 0xc0000b).portr("DSW2");
	map(0xc0000c, 0xc0000d).portr("SYSTEM");
	map(0xc0000e, 0xc0000f).nopr();
	map(0xc00011, 0xc00011).w(FUNC(sf_state::coin_w));
	map(0xc00014, 0xc00015).w(FUNC(sf_state::fg_scroll_w));
	map(0xc00018, 0xc00019).w(FUNC(sf_state::bg_scroll_w));
	map(0xc0001a, 0xc0001b).w(FUNC(sf_state::gfxctrl_w));
	map(0xc0001d, 0xc0001d).w(FUNC(sf_state::soundcmd_w));
	map(0xff8000, 0xffdfff).ram();
	map(0xffe000, 0xffffff).ram().share("objectram");
}

void sf_state::sfus_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x04ffff).rom();
	map(0x800000, 0x800fff).ram().w(FUNC(sf_state::videoram_w)).share("videoram");
	map(0xb00000, 0xb007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xc00000, 0xc00001).portr("IN0");
	map(0xc00002, 0xc00003).portr("IN1");
	map(0xc00004, 0xc00005).nopr();
	map(0xc00006, 0xc00007).nopr();
	map(0xc00008, 0xc00009).portr("DSW1");
	map(0xc0000a, 0xc0000b).portr("DSW2");
	map(0xc0000c, 0xc0000d).portr("SYSTEM");
	map(0xc0000e, 0xc0000f).nopr();
	map(0xc00011, 0xc00011).w(FUNC(sf_state::coin_w));
	map(0xc00014, 0xc00015).w(FUNC(sf_state::fg_scroll_w));
	map(0xc00018, 0xc00019).w(FUNC(sf_state::bg_scroll_w));
	map(0xc0001a, 0xc0001b).w(FUNC(sf_state::gfxctrl_w));
	map(0xc0001d, 0xc0001d).w(FUNC(sf_state::soundcmd_w));
	map(0xff8000, 0xffdfff).ram();
	map(0xffe000, 0xffffff).ram().share("objectram");
}

void sf_state::sfjp_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x04ffff).rom();
	map(0x800000, 0x800fff).ram().w(FUNC(sf_state::videoram_w)).share("videoram");
	map(0xb00000, 0xb007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xc00000, 0xc00001).portr("IN0");
	map(0xc00002, 0xc00003).portr("IN1");
	map(0xc00004, 0xc00005).portr("IN2");
	map(0xc00006, 0xc00007).nopr();
	map(0xc00008, 0xc00009).portr("DSW1");
	map(0xc0000a, 0xc0000b).portr("DSW2");
	map(0xc0000c, 0xc0000d).portr("SYSTEM");
	map(0xc0000e, 0xc0000f).nopr();
	map(0xc00011, 0xc00011).w(FUNC(sf_state::coin_w));
	map(0xc00014, 0xc00015).w(FUNC(sf_state::fg_scroll_w));
	map(0xc00018, 0xc00019).w(FUNC(sf_state::bg_scroll_w));
	map(0xc0001a, 0xc0001b).w(FUNC(sf_state::gfxctrl_w));
	map(0xc0001d, 0xc0001d).w(FUNC(sf_state::soundcmd_w));
	map(0xc0001e, 0xc0001f).w(FUNC(sf_state::protection_w));
	map(0xff8000, 0xffdfff).ram();
	map(0xffe000, 0xffffff).ram().share("objectram");
}

void sf_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
}

/* Yes, _no_ ram */
void sf_state::sound2_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xffff).bankr("audiobank");
	map(0x0000, 0xffff).nopw(); /* avoid cluttering up error.log */
}

void sf_state::sound2_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(sf_state::msm_w<0>));
	map(0x01, 0x01).w(FUNC(sf_state::msm_w<1>));
	map(0x01, 0x01).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x02, 0x02).w(FUNC(sf_state::sound2_bank_w));
}

void sf_state::prot_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(sf_state::prot_ram_r), FUNC(sf_state::prot_ram_w));
}

TILE_GET_INFO_MEMBER(sf_state::get_bg_tile_info)
{
	uint8_t *base = &m_tilerom[2 * tile_index];
	int attr = base[0x10000];
	int color = base[0];
	int code = (base[0x10000 + 1] << 8) | base[1];
	tileinfo.set(0, code, color, TILE_FLIPYX(attr & 3));
}

TILE_GET_INFO_MEMBER(sf_state::get_fg_tile_info)
{
	uint8_t *base = &m_tilerom[0x20000 + 2 * tile_index];
	int attr = base[0x10000];
	int color = base[0];
	int code = (base[0x10000 + 1] << 8) | base[1];
	tileinfo.set(1, code, color, TILE_FLIPYX(attr & 3));
}

TILE_GET_INFO_MEMBER(sf_state::get_tx_tile_info)
{
	int code = m_videoram[tile_index];
	tileinfo.set(3, code & 0x3ff, code>>12, TILE_FLIPYX((code & 0xc00)>>10));
}

void sf_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sf_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 2048, 16);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sf_state::get_fg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 2048, 16);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sf_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS,  8,  8,   64, 32);

	m_fg_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_transparent_pen(3);
}

void sf_state::protection_w(u16)
{
	m_protcpu->set_input_line(MCS51_INT0_LINE, ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void sf_state::prot_p3_w(u8 data)
{
	m_prot_t0 = data & 0x10;

	// Dunno if it's using HALT or DTACK, not really important though
	if(!(data & 0x02)) {
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_protcpu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);
	}
}

void sf_state::prot_ram_w(offs_t offset, u8 data)
{
	offset = offset * 2 + m_prot_t0;
	if(offset & 0x8000)
		offset = 0xff8000 | offset;
	else
		offset = 0xc00000 | (offset & 0x7fff);

	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

u8 sf_state::prot_ram_r(offs_t offset)
{
	offset = offset * 2 + m_prot_t0;
	if(offset & 0x8000)
		offset = 0xff8000 | offset;
	else
		offset = 0xc00000 | (offset & 0x7fff);

	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void sf_state::videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

void sf_state::bg_scroll_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bgscroll);
	m_bg_tilemap->set_scrollx(0, m_bgscroll);
}

void sf_state::fg_scroll_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fgscroll);
	m_fg_tilemap->set_scrollx(0, m_fgscroll);
}

void sf_state::gfxctrl_w(offs_t, u16 data, u16 mem_mask)
{
	/* b0 = reset, or maybe "set anyway" */
	/* b1 = pulsed when control6.b6==0 until it's 1 */
	/* b2 = active when dip 8 (flip) on */
	/* b3 = active character plane */
	/* b4 = unused */
	/* b5 = active background plane */
	/* b6 = active middle plane */
	/* b7 = active sprites */

	if(ACCESSING_BITS_0_7) {
		m_active = data & 0xff;
		flip_screen_set(data & 0x04);
		m_tx_tilemap->enable(data & 0x08);
		m_bg_tilemap->enable(data & 0x20);
		m_fg_tilemap->enable(data & 0x40);
	}
}


inline int sf_state::invert( int nb )
{
	static const int delta[4] = {0x00, 0x18, 0x18, 0x00};
	return nb ^ delta[(nb >> 3) & 3];
}

void sf_state::draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	int offs;

	for (offs = 0x1000 - 0x20; offs >= 0; offs -= 0x20)
	{
		int c = m_objectram[offs];
		int attr = m_objectram[offs + 1];
		int sy = m_objectram[offs + 2];
		int sx = m_objectram[offs + 3];
		int color = attr & 0x000f;
		int flipx = attr & 0x0100;
		int flipy = attr & 0x0200;

		if (attr & 0x400)   /* large sprite */
		{
			int c1, c2, c3, c4, t;

			if (flip_screen())
			{
				sx = 480 - sx;
				sy = 224 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			c1 = c;
			c2 = c + 1;
			c3 = c + 16;
			c4 = c + 17;

			if (flipx)
			{
				t = c1; c1 = c2; c2 = t;
				t = c3; c3 = c4; c4 = t;
			}
			if (flipy)
			{
				t = c1; c1 = c3; c3 = t;
				t = c2; c2 = c4; c4 = t;
			}

			m_gfxdecode->gfx(2)->transpen(bitmap,
					cliprect,
					invert(c1),
					color,
					flipx,flipy,
					sx,sy, 15);
			m_gfxdecode->gfx(2)->transpen(bitmap,
					cliprect,
					invert(c2),
					color,
					flipx,flipy,
					sx+16,sy, 15);
			m_gfxdecode->gfx(2)->transpen(bitmap,
					cliprect,
					invert(c3),
					color,
					flipx,flipy,
					sx,sy+16, 15);
			m_gfxdecode->gfx(2)->transpen(bitmap,
					cliprect,
					invert(c4),
					color,
					flipx,flipy,
					sx+16,sy+16, 15);
		}
		else
		{
			if (flip_screen())
			{
				sx = 496 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(2)->transpen(bitmap,
					cliprect,
					invert(c),
					color,
					flipx,flipy,
					sx,sy, 15);
		}
	}
}


uint32_t sf_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if(m_active & 0x20)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(0, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if(m_active & 0x80)
		draw_sprites(bitmap, cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( common )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("DSW1.7E:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("DSW1.7E:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "DSW1.7E:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "DSW1.7E:8" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("DSW2.13E:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Attract Music" )                 PORT_DIPLOCATION("DSW2.13E:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "DSW2.13E:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "DSW2.13E:4" )
	PORT_DIPNAME( 0x1000, 0x1000, "Speed" )                         PORT_DIPLOCATION("DSW2.13E:5")
	PORT_DIPSETTING(      0x0000, "Slow" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("DSW2.13E:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Freeze" )                        PORT_DIPLOCATION("DSW2.13E:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "DSW2.13E:8" ) // Self-Test Mode

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0007, "Game Continuation" )             PORT_DIPLOCATION("DSW3.6E:1,2,3")
	PORT_DIPSETTING(      0x0007, "5th Stage Maximum" )
	PORT_DIPSETTING(      0x0006, "4th Stage Maximum" )
	PORT_DIPSETTING(      0x0005, "3rd Stage Maximum" )
	PORT_DIPSETTING(      0x0004, "2nd Stage Maximum" )
	PORT_DIPSETTING(      0x0003, "1st Stage Maximum" )
	PORT_DIPSETTING(      0x0002, DEF_STR( None ) )
	PORT_DIPNAME( 0x0018, 0x0018, "Round Time Count" )              PORT_DIPLOCATION("DSW3.6E:4,5")
	PORT_DIPSETTING(      0x0018, "100" )
	PORT_DIPSETTING(      0x0010, "150" )
	PORT_DIPSETTING(      0x0008, "200" )
	PORT_DIPSETTING(      0x0000, "250" )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("DSW3.6E:6,7")
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Difficult ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x0380, 0x0380, "Buy-In Feature" )                PORT_DIPLOCATION("DSW3.6E:8,DSW4.11E:1,2")
	PORT_DIPSETTING(      0x0380, "5th Stage Maximum" )
	PORT_DIPSETTING(      0x0300, "4th Stage Maximum" )
	PORT_DIPSETTING(      0x0280, "3rd Stage Maximum" )
	PORT_DIPSETTING(      0x0200, "2nd Stage Maximum" )
	PORT_DIPSETTING(      0x0180, "1st Stage Maximum" )
	PORT_DIPSETTING(      0x0080, DEF_STR( None ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Number of Countries Selected" )  PORT_DIPLOCATION("DSW4.11E:3")
	PORT_DIPSETTING(      0x0400, "2" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "DSW4.11E:4" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "DSW4.11E:5" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "DSW4.11E:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "DSW4.11E:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "DSW4.11E:8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Freezes the game ? */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( sfan )
	PORT_INCLUDE( common )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "DSW2.13E:1" ) // Flip Screen not available

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0400, 0x0400, "Number of Countries Selected" )  PORT_DIPLOCATION("DSW4.11E:3")
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0000, "2" )

	// 4 pneumatic buttons. When their pressure starts decreasing, the game will latch
	// the highest measured value and respond with a low/mid/strong attack: approx.
	// 0x40 for low, 0xe0 for mid, 0xfe for strong.
	// NOTE: Timing is a matter of tenth-seconds. Tapping the button too lightly/quickly,
	// will not trigger an attack, same as on the original cab. Similarly, holding the
	// button for too long won't register either, analogous to the original cab by pushing
	// the button down slowly instead of hammering it.
	PORT_START("PUNCH")
	PORT_BIT( 0x00ff, 0x0000, IPT_PEDAL1 ) PORT_PLAYER(1) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_NAME("P1 Punch")
	PORT_BIT( 0xff00, 0x0000, IPT_PEDAL1 ) PORT_PLAYER(2) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_NAME("P2 Punch")

	PORT_START("KICK")
	PORT_BIT( 0x00ff, 0x0000, IPT_PEDAL2 ) PORT_PLAYER(1) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_NAME("P1 Kick")
	PORT_BIT( 0xff00, 0x0000, IPT_PEDAL2 ) PORT_PLAYER(2) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_NAME("P2 Kick")
INPUT_PORTS_END

static INPUT_PORTS_START( sfus )
	PORT_INCLUDE( common )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( sfjp )
	PORT_INCLUDE( common )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/***************************************************************************

  Machine Configs

***************************************************************************/

static const gfx_layout char_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ STEP4(0,1), STEP4(4*2,1) },
	{ STEP8(0,1*16) },
	16*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2) },
	{ STEP4(0,1), STEP4(4*2,1), STEP4(4*2*2*16,1), STEP4(4*2*2*16+8,1) },
	{ STEP16(0,1*16) },
	64*8
};


static GFXDECODE_START( gfx_sf )
	GFXDECODE_ENTRY( "gfx1", 0, sprite_layout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, sprite_layout, 512, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, char_layout,   768, 16 )
GFXDECODE_END


void sf_state::machine_start()
{
	save_item(NAME(m_active));
	save_item(NAME(m_bgscroll));
	save_item(NAME(m_fgscroll));
	save_item(NAME(m_prot_t0));

	m_audiobank->configure_entries(0, 256, memregion("audio2")->base() + 0x8000, 0x8000);
}

void sf_state::machine_reset()
{
	m_active = 0;
	m_bgscroll = 0;
	m_fgscroll = 0;
	m_prot_t0 = 0;
}

void sf_state::sfan(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(8'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &sf_state::sfan_map);
	m_maincpu->set_vblank_int("screen", FUNC(sf_state::irq1_line_hold));

	Z80(config, m_audiocpu, XTAL(3'579'545));   /* ? xtal is 3.579545MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &sf_state::sound_map);

	z80_device &audio2(Z80(config, "audio2", XTAL(3'579'545))); /* ? xtal is 3.579545MHz */
	audio2.set_addrmap(AS_PROGRAM, &sf_state::sound2_map);
	audio2.set_addrmap(AS_IO, &sf_state::sound2_io_map);
	audio2.set_periodic_int(FUNC(sf_state::irq0_line_hold), attotime::from_hz(8000)); // ?

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, (64-8)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(sf_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sf);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 1024);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(3'579'545)));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.60);
	ymsnd.add_route(1, "rspeaker", 0.60);

	MSM5205(config, m_msm[0], 384000);
	m_msm[0]->set_prescaler_selector(msm5205_device::SEX_4B);   /* 8KHz playback ? */
	m_msm[0]->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_msm[0]->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	MSM5205(config, m_msm[1], 384000);
	m_msm[1]->set_prescaler_selector(msm5205_device::SEX_4B);   /* 8KHz playback ? */
	m_msm[1]->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_msm[1]->add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}

void sf_state::sfus(machine_config &config)
{
	sfan(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &sf_state::sfus_map);
}

void sf_state::sfjp(machine_config &config)
{
	sfan(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &sf_state::sfjp_map);

	I8751(config, m_protcpu, XTAL(8'000'000)); // Clock unknown, but shares the bus with the 68k, so could be similar
	m_protcpu->set_addrmap(AS_IO, &sf_state::prot_map);
	m_protcpu->port_out_cb<3>().set(FUNC(sf_state::prot_p3_w));
}

void sf_state::sfp(machine_config &config)
{
	sfan(config);
	m_maincpu->set_vblank_int("screen", FUNC(sf_state::irq6_line_hold));
}



/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( sf )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE("sfd-19.2a", 0x00000, 0x10000, CRC(faaf6255) SHA1(f6d0186c6109780839576c141fc6b557c170c182) )
	ROM_LOAD16_BYTE("sfd-22.2c", 0x00001, 0x10000, CRC(e1fe3519) SHA1(5c59343a8acaaa4f36636d8e28a4ca7854110dad) )
	ROM_LOAD16_BYTE("sfd-20.3a", 0x20000, 0x10000, CRC(44b915bd) SHA1(85772fb89712f97bb0489a7e43f8b1a5037c8081) )
	ROM_LOAD16_BYTE("sfd-23.3c", 0x20001, 0x10000, CRC(79c43ff8) SHA1(450fb75b6f36e08788d7a806122e4e1b0a87746c) )
	ROM_LOAD16_BYTE("sfd-21.4a", 0x40000, 0x10000, CRC(e8db799b) SHA1(8443ba6a9b9ad29d5985d434658e685fd46d8f1e) )
	ROM_LOAD16_BYTE("sfd-24.4c", 0x40001, 0x10000, CRC(466a3440) SHA1(689823763bfdbc12ac11ff176acfd22f352e2658) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the music CPU */
	ROM_LOAD( "sf-02.7k", 0x0000, 0x8000, CRC(4a9ac534) SHA1(933645f8db4756aa2a35a843c3ac6f93cb8d565d) )

	ROM_REGION( 0x40000, "audio2", 0 )  /* 256k for the samples CPU */
	ROM_LOAD( "sfu-00.1h",0x00000, 0x20000, CRC(a7cce903) SHA1(76f521c9a00abd95a3491ab95e8eccd0fc7ea0e5) )
	ROM_LOAD( "sf-01.1k", 0x20000, 0x20000, CRC(86e0f0d5) SHA1(7cef8056f83dac15f1b47d7be705d26170858337) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "sf-39.2k", 0x000000, 0x020000, CRC(cee3d292) SHA1(a8c22f1dc81976e8dd5d6c70361c61fa3f9f89d6) ) /* Background b planes 0-1*/
	ROM_LOAD( "sf-38.1k", 0x020000, 0x020000, CRC(2ea99676) SHA1(5f3eb77e75f0ee27fb8fc7bab2819b3fdd480206) )
	ROM_LOAD( "sf-41.4k", 0x040000, 0x020000, CRC(e0280495) SHA1(e52c79feed590535b9a0b71ccadd0ed27d04ff45) ) /* planes 2-3 */
	ROM_LOAD( "sf-40.3k", 0x060000, 0x020000, CRC(c70b30de) SHA1(26112ee1720b6ad0e2e29e2d25ee2ec76fca0e3a) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "sf-25.1d", 0x000000, 0x020000, CRC(7f23042e) SHA1(a355fd7047fb1a71ab5cd08e1afd82c2558494c1) ) /* Background m planes 0-1 */
	ROM_LOAD( "sf-28.1e", 0x020000, 0x020000, CRC(92f8b91c) SHA1(6d958bc45131810d7b0af02be939ce37a39c35e8) )
	ROM_LOAD( "sf-30.1g", 0x040000, 0x020000, CRC(b1399856) SHA1(7c956d49b2e73291182ea1ec4cebd3411d1322a1) )
	ROM_LOAD( "sf-34.1h", 0x060000, 0x020000, CRC(96b6ae2e) SHA1(700e050463b7a29a1eb08007a2add045afdcd8a0) )
	ROM_LOAD( "sf-26.2d", 0x080000, 0x020000, CRC(54ede9f5) SHA1(c2cb354a6b32047759945fa3ecafc70ba7d1dda1) ) /* planes 2-3 */
	ROM_LOAD( "sf-29.2e", 0x0a0000, 0x020000, CRC(f0649a67) SHA1(eeda256527f7a2ee2d5e0688c505a01de548bc54) )
	ROM_LOAD( "sf-31.2g", 0x0c0000, 0x020000, CRC(8f4dd71a) SHA1(28b82c540df04c91a2dd6cbbc9a95bbebda6643b) )
	ROM_LOAD( "sf-35.2h", 0x0e0000, 0x020000, CRC(70c00fb4) SHA1(7c5504a5aedd3be7b663c5090eb22243e3fa669b) )

	ROM_REGION( 0x1c0000, "gfx3", 0 )
	/* Sprites planes 1-2 */
	ROM_LOAD( "sf-15.1m", 0x000000, 0x020000, CRC(fc0113db) SHA1(7c19603129be5f6e1ccd07fd8b7ee1cbf86468db) )
	ROM_LOAD( "sf-16.2m", 0x020000, 0x020000, CRC(82e4a6d3) SHA1(5ec519c2740c66f5da27ced1db99e19fe38fdad7) )
	ROM_LOAD( "sf-11.1k", 0x040000, 0x020000, CRC(e112df1b) SHA1(3f9856f69b457d79fe085bf51dfb2efcd98f883d) )
	ROM_LOAD( "sf-12.2k", 0x060000, 0x020000, CRC(42d52299) SHA1(6560c38f5fd5a47db7728cc7df83d2169157174f) )
	ROM_LOAD( "sf-07.1h", 0x080000, 0x020000, CRC(49f340d9) SHA1(65822efefa198791a632ef851a5ce06a71b4ed0f) )
	ROM_LOAD( "sf-08.2h", 0x0a0000, 0x020000, CRC(95ece9b1) SHA1(f0a15fce5cd9617fa5d4dd43bd5b6ea190dace85) )
	ROM_LOAD( "sf-03.1f", 0x0c0000, 0x020000, CRC(5ca05781) SHA1(004f5ad34798471b39bd4612c797f0913ed0fb4a) )
	/* Sprites planes 2-3 */
	ROM_LOAD( "sf-17.3m", 0x0e0000, 0x020000, CRC(69fac48e) SHA1(c9272217256c73cb8ddb4fbbfb5905ce1122c746) )
	ROM_LOAD( "sf-18.4m", 0x100000, 0x020000, CRC(71cfd18d) SHA1(4c17e2124f3456d6b13ede8ad3ae916b53f9bb7e) )
	ROM_LOAD( "sf-13.3k", 0x120000, 0x020000, CRC(fa2eb24b) SHA1(96f3bd54c340771577cc232ebde93965421f2557) )
	ROM_LOAD( "sf-14.4k", 0x140000, 0x020000, CRC(ad955c95) SHA1(549d6a5125432aa45d03f15e76f6c2c8ab2e05a3) )
	ROM_LOAD( "sf-09.3h", 0x160000, 0x020000, CRC(41b73a31) SHA1(aaa7a53e29fe23a1ca8ec4430f7efcbd774a8cbf) )
	ROM_LOAD( "sf-10.4h", 0x180000, 0x020000, CRC(91c41c50) SHA1(b03fb9b3c553fb4aae45ad6997eeb7bb95fdcce3) )
	ROM_LOAD( "sf-05.3f", 0x1a0000, 0x020000, CRC(538c7cbe) SHA1(f030a9562fbb93d1534b91343ca3f429cdbd0136) )

	ROM_REGION( 0x004000, "gfx4", 0 )
	ROM_LOAD( "sf-27.4d", 0x000000, 0x004000, CRC(2b09b36d) SHA1(9fe1dd3a9396fbb06f30247cfe526653553beca1) ) /* Characters planes 1-2 */

	ROM_REGION( 0x40000, "tilerom", 0 )    /* background tilemaps */
	ROM_LOAD( "sf-37.4h", 0x000000, 0x010000, CRC(23d09d3d) SHA1(a0c71abc49c5fe59487a63b502e3d03021bfef13) )
	ROM_LOAD( "sf-36.3h", 0x010000, 0x010000, CRC(ea16df6c) SHA1(68709a314b775c500817fc17d40a80204b2ae06c) )
	ROM_LOAD( "sf-32.3g", 0x020000, 0x010000, CRC(72df2bd9) SHA1(9a0da618139673738b6b3302207255e44c5491a2) )
	ROM_LOAD( "sf-33.4g", 0x030000, 0x010000, CRC(3e99d3d5) SHA1(9168a977e80f8c23c6126b9e64eb176290cf941a) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "mb7114h.12k",  0x0000, 0x0100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) ) /* unknown */
	ROM_LOAD( "mb7114h.11h",  0x0100, 0x0100, CRC(c0e56586) SHA1(2abf93aef48af34f869b30f63c130513a97f86a3) ) /* unknown */
	ROM_LOAD( "mb7114h.12j",  0x0200, 0x0100, CRC(4c734b64) SHA1(7a122b643bad3e3586821980efff023a63e5a029) ) /* unknown */
	ROM_LOAD( "mmi-7603.13h", 0x0300, 0x0020, CRC(06bcda53) SHA1(fa69b77697bb12aa6012d82ef5b504d3a1d20232) ) /* unknown */
ROM_END


ROM_START( sfua )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE("sfs19u.1a", 0x00000, 0x10000, CRC(c8e41c49) SHA1(01f023864a662fa451901b8689341b00a36973c1) )
	ROM_LOAD16_BYTE("sfs22u.1b", 0x00001, 0x10000, CRC(667e9309) SHA1(3d895874cf86470e4f2041d21e751fac3170b4c5) )
	ROM_LOAD16_BYTE("sfs20u.2a", 0x20000, 0x10000, CRC(303065bf) SHA1(152bb707cd71a8614f6d17cf9a145c8a8184ded7) )
	ROM_LOAD16_BYTE("sfs23u.2b", 0x20001, 0x10000, CRC(de6927a3) SHA1(862a62b71fbd2049f05968a238b97344d3b7404e) )
	ROM_LOAD16_BYTE("sfs21u.3a", 0x40000, 0x10000, CRC(004a418b) SHA1(1048afe2e0dbc22969d79a031394f3c8ab4c8901) )
	ROM_LOAD16_BYTE("sfs24u.3b", 0x40001, 0x10000, CRC(2b4545ff) SHA1(19bdae7947d13b861ace25b96e46f199ee9a6eb2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the music CPU */
	ROM_LOAD( "sf-02.7k", 0x0000, 0x8000, CRC(4a9ac534) SHA1(933645f8db4756aa2a35a843c3ac6f93cb8d565d) )

	ROM_REGION( 0x1000, "protcpu", 0 )
	ROM_LOAD( "sf_s.id8751h-8.14f", 0x0000, 0x1000, CRC(6588891f) SHA1(699a96c682dd527dc77aa5cb2c2655136d2bfc90) ) // is this mcu label right for the US set?

	ROM_REGION( 0x40000, "audio2", 0 )  /* 256k for the samples CPU */
	ROM_LOAD( "sfu-00.1h",0x00000, 0x20000, CRC(a7cce903) SHA1(76f521c9a00abd95a3491ab95e8eccd0fc7ea0e5) )
	ROM_LOAD( "sf-01.1k", 0x20000, 0x20000, CRC(86e0f0d5) SHA1(7cef8056f83dac15f1b47d7be705d26170858337) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "sf-39.2k", 0x000000, 0x020000, CRC(cee3d292) SHA1(a8c22f1dc81976e8dd5d6c70361c61fa3f9f89d6) ) /* Background b planes 0-1*/
	ROM_LOAD( "sf-38.1k", 0x020000, 0x020000, CRC(2ea99676) SHA1(5f3eb77e75f0ee27fb8fc7bab2819b3fdd480206) )
	ROM_LOAD( "sf-41.4k", 0x040000, 0x020000, CRC(e0280495) SHA1(e52c79feed590535b9a0b71ccadd0ed27d04ff45) ) /* planes 2-3 */
	ROM_LOAD( "sf-40.3k", 0x060000, 0x020000, CRC(c70b30de) SHA1(26112ee1720b6ad0e2e29e2d25ee2ec76fca0e3a) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "sf-25.1d", 0x000000, 0x020000, CRC(7f23042e) SHA1(a355fd7047fb1a71ab5cd08e1afd82c2558494c1) ) /* Background m planes 0-1 */
	ROM_LOAD( "sf-28.1e", 0x020000, 0x020000, CRC(92f8b91c) SHA1(6d958bc45131810d7b0af02be939ce37a39c35e8) )
	ROM_LOAD( "sf-30.1g", 0x040000, 0x020000, CRC(b1399856) SHA1(7c956d49b2e73291182ea1ec4cebd3411d1322a1) )
	ROM_LOAD( "sf-34.1h", 0x060000, 0x020000, CRC(96b6ae2e) SHA1(700e050463b7a29a1eb08007a2add045afdcd8a0) )
	ROM_LOAD( "sf-26.2d", 0x080000, 0x020000, CRC(54ede9f5) SHA1(c2cb354a6b32047759945fa3ecafc70ba7d1dda1) ) /* planes 2-3 */
	ROM_LOAD( "sf-29.2e", 0x0a0000, 0x020000, CRC(f0649a67) SHA1(eeda256527f7a2ee2d5e0688c505a01de548bc54) )
	ROM_LOAD( "sf-31.2g", 0x0c0000, 0x020000, CRC(8f4dd71a) SHA1(28b82c540df04c91a2dd6cbbc9a95bbebda6643b) )
	ROM_LOAD( "sf-35.2h", 0x0e0000, 0x020000, CRC(70c00fb4) SHA1(7c5504a5aedd3be7b663c5090eb22243e3fa669b) )

	ROM_REGION( 0x1c0000, "gfx3", 0 )
	/* Sprites planes 1-2 */
	ROM_LOAD( "sf-15.1m", 0x000000, 0x020000, CRC(fc0113db) SHA1(7c19603129be5f6e1ccd07fd8b7ee1cbf86468db) )
	ROM_LOAD( "sf-16.2m", 0x020000, 0x020000, CRC(82e4a6d3) SHA1(5ec519c2740c66f5da27ced1db99e19fe38fdad7) )
	ROM_LOAD( "sf-11.1k", 0x040000, 0x020000, CRC(e112df1b) SHA1(3f9856f69b457d79fe085bf51dfb2efcd98f883d) )
	ROM_LOAD( "sf-12.2k", 0x060000, 0x020000, CRC(42d52299) SHA1(6560c38f5fd5a47db7728cc7df83d2169157174f) )
	ROM_LOAD( "sf-07.1h", 0x080000, 0x020000, CRC(49f340d9) SHA1(65822efefa198791a632ef851a5ce06a71b4ed0f) )
	ROM_LOAD( "sf-08.2h", 0x0a0000, 0x020000, CRC(95ece9b1) SHA1(f0a15fce5cd9617fa5d4dd43bd5b6ea190dace85) )
	ROM_LOAD( "sf-03.1f", 0x0c0000, 0x020000, CRC(5ca05781) SHA1(004f5ad34798471b39bd4612c797f0913ed0fb4a) )
	/* Sprites planes 2-3 */
	ROM_LOAD( "sf-17.3m", 0x0e0000, 0x020000, CRC(69fac48e) SHA1(c9272217256c73cb8ddb4fbbfb5905ce1122c746) )
	ROM_LOAD( "sf-18.4m", 0x100000, 0x020000, CRC(71cfd18d) SHA1(4c17e2124f3456d6b13ede8ad3ae916b53f9bb7e) )
	ROM_LOAD( "sf-13.3k", 0x120000, 0x020000, CRC(fa2eb24b) SHA1(96f3bd54c340771577cc232ebde93965421f2557) )
	ROM_LOAD( "sf-14.4k", 0x140000, 0x020000, CRC(ad955c95) SHA1(549d6a5125432aa45d03f15e76f6c2c8ab2e05a3) )
	ROM_LOAD( "sf-09.3h", 0x160000, 0x020000, CRC(41b73a31) SHA1(aaa7a53e29fe23a1ca8ec4430f7efcbd774a8cbf) )
	ROM_LOAD( "sf-10.4h", 0x180000, 0x020000, CRC(91c41c50) SHA1(b03fb9b3c553fb4aae45ad6997eeb7bb95fdcce3) )
	ROM_LOAD( "sf-05.3f", 0x1a0000, 0x020000, CRC(538c7cbe) SHA1(f030a9562fbb93d1534b91343ca3f429cdbd0136) )

	ROM_REGION( 0x004000, "gfx4", 0 )
	ROM_LOAD( "sf-27.4d", 0x000000, 0x004000, CRC(2b09b36d) SHA1(9fe1dd3a9396fbb06f30247cfe526653553beca1) ) /* Characters planes 1-2 */

	ROM_REGION( 0x40000, "tilerom", 0 )    /* background tilemaps */
	ROM_LOAD( "sf-37.4h", 0x000000, 0x010000, CRC(23d09d3d) SHA1(a0c71abc49c5fe59487a63b502e3d03021bfef13) )
	ROM_LOAD( "sf-36.3h", 0x010000, 0x010000, CRC(ea16df6c) SHA1(68709a314b775c500817fc17d40a80204b2ae06c) )
	ROM_LOAD( "sf-32.3g", 0x020000, 0x010000, CRC(72df2bd9) SHA1(9a0da618139673738b6b3302207255e44c5491a2) )
	ROM_LOAD( "sf-33.4g", 0x030000, 0x010000, CRC(3e99d3d5) SHA1(9168a977e80f8c23c6126b9e64eb176290cf941a) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "sfb05.bin",    0x0000, 0x0100, CRC(864199ad) SHA1(b777df20b19fa7b7536120191df1875101e9d7af) ) /* unknown */
	ROM_LOAD( "sfb00.bin",    0x0100, 0x0100, CRC(bd3f8c5d) SHA1(c31ee9f466f05a21612f5ea29fb8c7c25dc9e011) ) /* unknown */
	ROM_LOAD( "mb7114h.12j",  0x0200, 0x0100, CRC(4c734b64) SHA1(7a122b643bad3e3586821980efff023a63e5a029) ) /* unknown */
	ROM_LOAD( "mmi-7603.13h", 0x0300, 0x0020, CRC(06bcda53) SHA1(fa69b77697bb12aa6012d82ef5b504d3a1d20232) ) /* unknown */
ROM_END


ROM_START( sfj )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE("sf-19.2a", 0x00000, 0x10000, CRC(116027d7) SHA1(6bcb117ee415aff4d8ea962d4eff4088ca94c251) )
	ROM_LOAD16_BYTE("sf-22.2c", 0x00001, 0x10000, CRC(d3cbd09e) SHA1(7274c603100132102de09e10d2129cfeb6c06369) )
	ROM_LOAD16_BYTE("sf-20.3a", 0x20000, 0x10000, CRC(fe07e83f) SHA1(252dd592c31e594103ac1eabd734d10748655701) )
	ROM_LOAD16_BYTE("sf-23.3c", 0x20001, 0x10000, CRC(1e435d33) SHA1(2022a4368aa63cb036e77cb5739810030db469ff) )
	ROM_LOAD16_BYTE("sf-21.4a", 0x40000, 0x10000, CRC(e086bc4c) SHA1(782134978ff0a7133768d9cc8050bc3b5016580b) )
	ROM_LOAD16_BYTE("sf-24.4c", 0x40001, 0x10000, CRC(13a6696b) SHA1(c01f9b700928e427bc9914c61beeaa6bcbde4546) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the music CPU */
	ROM_LOAD( "sf-02.7k", 0x0000, 0x8000, CRC(4a9ac534) SHA1(933645f8db4756aa2a35a843c3ac6f93cb8d565d) )

	ROM_REGION( 0x1000, "protcpu", 0 )
	ROM_LOAD( "sf_s.id8751h-8.14f", 0x0000, 0x1000, CRC(6588891f) SHA1(699a96c682dd527dc77aa5cb2c2655136d2bfc90) )

	ROM_REGION( 0x40000, "audio2", 0 )  /* 256k for the samples CPU */
	ROM_LOAD( "sf-00.1h", 0x00000, 0x20000, CRC(4b733845) SHA1(f7ff46e02f8ce6682d6e573588271bae2edfa90f) )
	ROM_LOAD( "sf-01.1k", 0x20000, 0x20000, CRC(86e0f0d5) SHA1(7cef8056f83dac15f1b47d7be705d26170858337) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "sf-39.2k", 0x000000, 0x020000, CRC(cee3d292) SHA1(a8c22f1dc81976e8dd5d6c70361c61fa3f9f89d6) ) /* Background b planes 0-1*/
	ROM_LOAD( "sf-38.1k", 0x020000, 0x020000, CRC(2ea99676) SHA1(5f3eb77e75f0ee27fb8fc7bab2819b3fdd480206) )
	ROM_LOAD( "sf-41.4k", 0x040000, 0x020000, CRC(e0280495) SHA1(e52c79feed590535b9a0b71ccadd0ed27d04ff45) ) /* planes 2-3 */
	ROM_LOAD( "sf-40.3k", 0x060000, 0x020000, CRC(c70b30de) SHA1(26112ee1720b6ad0e2e29e2d25ee2ec76fca0e3a) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "sf-25.1d", 0x000000, 0x020000, CRC(7f23042e) SHA1(a355fd7047fb1a71ab5cd08e1afd82c2558494c1) ) /* Background m planes 0-1 */
	ROM_LOAD( "sf-28.1e", 0x020000, 0x020000, CRC(92f8b91c) SHA1(6d958bc45131810d7b0af02be939ce37a39c35e8) )
	ROM_LOAD( "sf-30.1g", 0x040000, 0x020000, CRC(b1399856) SHA1(7c956d49b2e73291182ea1ec4cebd3411d1322a1) )
	ROM_LOAD( "sf-34.1h", 0x060000, 0x020000, CRC(96b6ae2e) SHA1(700e050463b7a29a1eb08007a2add045afdcd8a0) )
	ROM_LOAD( "sf-26.2d", 0x080000, 0x020000, CRC(54ede9f5) SHA1(c2cb354a6b32047759945fa3ecafc70ba7d1dda1) ) /* planes 2-3 */
	ROM_LOAD( "sf-29.2e", 0x0a0000, 0x020000, CRC(f0649a67) SHA1(eeda256527f7a2ee2d5e0688c505a01de548bc54) )
	ROM_LOAD( "sf-31.2g", 0x0c0000, 0x020000, CRC(8f4dd71a) SHA1(28b82c540df04c91a2dd6cbbc9a95bbebda6643b) )
	ROM_LOAD( "sf-35.2h", 0x0e0000, 0x020000, CRC(70c00fb4) SHA1(7c5504a5aedd3be7b663c5090eb22243e3fa669b) )

	ROM_REGION( 0x1c0000, "gfx3", 0 )
	/* Sprites planes 1-2 */
	ROM_LOAD( "sf-15.1m", 0x000000, 0x020000, CRC(fc0113db) SHA1(7c19603129be5f6e1ccd07fd8b7ee1cbf86468db) )
	ROM_LOAD( "sf-16.2m", 0x020000, 0x020000, CRC(82e4a6d3) SHA1(5ec519c2740c66f5da27ced1db99e19fe38fdad7) )
	ROM_LOAD( "sf-11.1k", 0x040000, 0x020000, CRC(e112df1b) SHA1(3f9856f69b457d79fe085bf51dfb2efcd98f883d) )
	ROM_LOAD( "sf-12.2k", 0x060000, 0x020000, CRC(42d52299) SHA1(6560c38f5fd5a47db7728cc7df83d2169157174f) )
	ROM_LOAD( "sf-07.1h", 0x080000, 0x020000, CRC(49f340d9) SHA1(65822efefa198791a632ef851a5ce06a71b4ed0f) )
	ROM_LOAD( "sf-08.2h", 0x0a0000, 0x020000, CRC(95ece9b1) SHA1(f0a15fce5cd9617fa5d4dd43bd5b6ea190dace85) )
	ROM_LOAD( "sf-03.1f", 0x0c0000, 0x020000, CRC(5ca05781) SHA1(004f5ad34798471b39bd4612c797f0913ed0fb4a) )
	/* Sprites planes 2-3 */
	ROM_LOAD( "sf-17.3m", 0x0e0000, 0x020000, CRC(69fac48e) SHA1(c9272217256c73cb8ddb4fbbfb5905ce1122c746) )
	ROM_LOAD( "sf-18.4m", 0x100000, 0x020000, CRC(71cfd18d) SHA1(4c17e2124f3456d6b13ede8ad3ae916b53f9bb7e) )
	ROM_LOAD( "sf-13.3k", 0x120000, 0x020000, CRC(fa2eb24b) SHA1(96f3bd54c340771577cc232ebde93965421f2557) )
	ROM_LOAD( "sf-14.4k", 0x140000, 0x020000, CRC(ad955c95) SHA1(549d6a5125432aa45d03f15e76f6c2c8ab2e05a3) )
	ROM_LOAD( "sf-09.3h", 0x160000, 0x020000, CRC(41b73a31) SHA1(aaa7a53e29fe23a1ca8ec4430f7efcbd774a8cbf) )
	ROM_LOAD( "sf-10.4h", 0x180000, 0x020000, CRC(91c41c50) SHA1(b03fb9b3c553fb4aae45ad6997eeb7bb95fdcce3) )
	ROM_LOAD( "sf-05.3f", 0x1a0000, 0x020000, CRC(538c7cbe) SHA1(f030a9562fbb93d1534b91343ca3f429cdbd0136) )

	ROM_REGION( 0x004000, "gfx4", 0 )
	ROM_LOAD( "sf-27.4d", 0x000000, 0x004000, CRC(2b09b36d) SHA1(9fe1dd3a9396fbb06f30247cfe526653553beca1) ) /* Characters planes 1-2 */

	ROM_REGION( 0x40000, "tilerom", 0 )    /* background tilemaps */
	ROM_LOAD( "sf-37.4h", 0x000000, 0x010000, CRC(23d09d3d) SHA1(a0c71abc49c5fe59487a63b502e3d03021bfef13) )
	ROM_LOAD( "sf-36.3h", 0x010000, 0x010000, CRC(ea16df6c) SHA1(68709a314b775c500817fc17d40a80204b2ae06c) )
	ROM_LOAD( "sf-32.3g", 0x020000, 0x010000, CRC(72df2bd9) SHA1(9a0da618139673738b6b3302207255e44c5491a2) )
	ROM_LOAD( "sf-33.4g", 0x030000, 0x010000, CRC(3e99d3d5) SHA1(9168a977e80f8c23c6126b9e64eb176290cf941a) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "sfb05.bin",    0x0000, 0x0100, CRC(864199ad) SHA1(b777df20b19fa7b7536120191df1875101e9d7af) ) /* unknown */
	ROM_LOAD( "sfb00.bin",    0x0100, 0x0100, CRC(bd3f8c5d) SHA1(c31ee9f466f05a21612f5ea29fb8c7c25dc9e011) ) /* unknown */
	ROM_LOAD( "mb7114h.12j",  0x0200, 0x0100, CRC(4c734b64) SHA1(7a122b643bad3e3586821980efff023a63e5a029) ) /* unknown */
	ROM_LOAD( "mmi-7603.13h", 0x0300, 0x0020, CRC(06bcda53) SHA1(fa69b77697bb12aa6012d82ef5b504d3a1d20232) ) /* unknown */
ROM_END


ROM_START( sfjbl ) // main PCB is marked '17-51-1' on component side
	ROM_REGION( 0x60000, "maincpu", 0 ) // only range 0x3c200-0x3c23f has minor differences from the original, to skip the disclaimer screen
	ROM_LOAD16_BYTE("a-34.34.4a", 0x00000, 0x10000, CRC(116027d7) SHA1(6bcb117ee415aff4d8ea962d4eff4088ca94c251) )
	ROM_LOAD16_BYTE("a-37.37.4c", 0x00001, 0x10000, CRC(d3cbd09e) SHA1(7274c603100132102de09e10d2129cfeb6c06369) )
	ROM_LOAD16_BYTE("a-35.35.5a", 0x20000, 0x10000, CRC(5c31ea04) SHA1(6ae07c668ad97356b1b7163dbca9518e37dba42a) )
	ROM_LOAD16_BYTE("a-38.38.5c", 0x20001, 0x10000, CRC(908d9e98) SHA1(bd2acd424864a79fb225ae5fa5bcbe3b9fdfe3a6) )
	ROM_LOAD16_BYTE("a-36.36.6a", 0x40000, 0x08000, CRC(5ea75033) SHA1(c79d5eac8787a326546ae4d2506a8f8bcd72611f) )
	ROM_LOAD16_BYTE("a-39.39.6c", 0x40001, 0x08000, CRC(4d0606cb) SHA1(27042925b94cd827d800f658062a914d7425c407) )
	// original has 0xff filled ROM at 0x50000-0x5ffff

	ROM_REGION( 0x10000, "audiocpu", 0 ) // identical to the original
	ROM_LOAD( "a-6.6.8f", 0x0000, 0x8000, CRC(4a9ac534) SHA1(933645f8db4756aa2a35a843c3ac6f93cb8d565d) )

	ROM_REGION( 0x1000, "protcpu", 0 )
	ROM_LOAD( "c8751h-88.5o", 0x0000, 0x1000, CRC(6588891f) SHA1(699a96c682dd527dc77aa5cb2c2655136d2bfc90) ) // decapped, matches other dump from unprotected 8751

	ROM_REGION( 0x40000, "audio2", 0 )  // samples CPU, identical to the original, but with half size ROMs
	ROM_LOAD( "a-2.2.6b", 0x00000, 0x10000, CRC(a3212da3) SHA1(4f95eb1dedbcca05016d92f62968ffce8173defe) )
	ROM_LOAD( "a-1.1.6a", 0x10000, 0x10000, CRC(707d5f67) SHA1(355cecb1f45ada332b834bff824b84b0b3fb60b5) )
	ROM_LOAD( "a-5.5.8a", 0x20000, 0x10000, CRC(e2574554) SHA1(9519d29c815b267145844524aad8c7b962450e8d) )
	ROM_LOAD( "a-4.4.7a", 0x30000, 0x10000, CRC(af7ae326) SHA1(655f4eb2585e39ab3cf8c148ea48b8ebf41ec049) )

	ROM_REGION( 0x080000, "gfx1", 0 ) // identical to the original, but with half size ROMs
	ROM_LOAD( "k-5.55.4g",  0x000000, 0x010000, CRC(7c90a53e) SHA1(a97507e17d61fdcd6af447cd6d1d7472d4606bba) ) // Background b planes 0-1
	ROM_LOAD( "a-43.43.1h", 0x010000, 0x010000, CRC(0cfe888c) SHA1(a059f6bd9c4a6ffd5eb0f7628651ef7bf66c46e9) )
	ROM_LOAD( "k-2.52.4e",  0x020000, 0x010000, CRC(b5263e4b) SHA1(d48af16273605ccbcb6ebc6f5004a221b47a5ad7) )
	ROM_LOAD( "a-42.42.1g", 0x030000, 0x010000, CRC(d401b93e) SHA1(84a6ff40d4cd365303fd03794d264c09e4144b4c) )
	ROM_LOAD( "k-6.56.5g",  0x040000, 0x010000, CRC(91f77551) SHA1(52570eeeee0f39a3583dcc2fafc6908178c686ce) ) // planes 2-3
	ROM_LOAD( "a-41.41.1f", 0x050000, 0x010000, CRC(90c0115c) SHA1(a064f58d0a235453897ec71877e715d2a6a585b5) )
	ROM_LOAD( "k-3.53.5e",  0x060000, 0x010000, CRC(c9dae6b9) SHA1(95fab99a5f3d6e610c632b3c9692030af3fd17d3) )
	ROM_LOAD( "a-40.40.1e", 0x070000, 0x010000, CRC(c4e6a3b1) SHA1(5eb3a4ceee58567e0a2ec35f080dce1ae0e79361) )

	ROM_REGION( 0x100000, "gfx2", 0 ) // identical to the original, but with half size ROMs
	ROM_LOAD( "k-8.58.4m",  0x000000, 0x010000, CRC(365c87bf) SHA1(bf2da41193ce08d048612fb5291e96eab91460e1) ) // Background m planes 0-1
	ROM_LOAD( "a-46.46.1l", 0x010000, 0x010000, CRC(2559af76) SHA1(0450b82a59782639c55bbf1bab73148a3e698481) )
	ROM_LOAD( "k-9.59.5m",  0x020000, 0x010000, CRC(0a14421a) SHA1(aa13ee4738b774d7cfa8c0b35dc51a27e964018d) )
	ROM_LOAD( "a-47.47.1m", 0x030000, 0x010000, CRC(e97f19fa) SHA1(849da1dbcbbcfafb66ed80eab60a2a2e7f2d8ac7) )
	ROM_LOAD( "k-10.60.6m", 0x040000, 0x010000, CRC(96e34f7d) SHA1(373bae1fcba249b4f23290304ba549c25b858d45) )
	ROM_LOAD( "a-45.45.1k", 0x050000, 0x010000, CRC(bd5700b4) SHA1(9077e6783fe5e923574a6d0999e49e2a884b4e95) )
	ROM_LOAD( "k-11.61.7m", 0x060000, 0x010000, CRC(22746035) SHA1(55efd91d88ab63e425961cb6eed23e3401060463) )
	// original has 0xff filled ROM at 0x70000-0x7ffff
	ROM_LOAD( "k-12.62.4r", 0x080000, 0x010000, CRC(643660c0) SHA1(4f57cafd36977733e95b45920f1390dded047c11) ) // planes 2-3
	ROM_LOAD( "a-49.49.1q", 0x090000, 0x010000, CRC(63f7f181) SHA1(4c5ac7e4272f1a9c62791260c02ad29c6435b42d) )
	ROM_LOAD( "k-13.63.5r", 0x0a0000, 0x010000, CRC(e8e113b1) SHA1(42b52ff462ad6f0429086f62bcd690d9e3a09a30) )
	ROM_LOAD( "a-48.48.1p", 0x0b0000, 0x010000, CRC(8da0a11c) SHA1(51e25c6481410524e1be9b5f4cdebdf9d1db5c0f) )
	ROM_LOAD( "k-14.64.6r", 0x0c0000, 0x010000, CRC(5990fda8) SHA1(c8539a59b57df648d219c4003cf4001b3d7bf7ec) )
	ROM_LOAD( "a-50.50.1r", 0x0d0000, 0x010000, CRC(29ef5174) SHA1(e610850f69574adb8b0b1713715bec7dbc04a0c6) )
	ROM_LOAD( "k-15.65.7r", 0x0e0000, 0x010000, CRC(301b4bcc) SHA1(8a3aed7838baa1dd3085fdbc963f5338e99234aa) )
	// original has 0xff filled ROM at 0xf0000-0xfffff

	ROM_REGION( 0x1c0000, "gfx3", 0 ) // identical to the original, but with half size ROMs
	// Sprites planes 1-2
	ROM_LOAD( "a-7.7.1b",    0x000000, 0x010000, CRC(2063e23c) SHA1(b856ab4ac4ca414e19ead4a60c81050db57b7d7b) )
	ROM_LOAD( "a-8.8.1c",    0x010000, 0x010000, CRC(00af40c4) SHA1(554966188713cb80fce1089636cfbe3ea6b6b243) )
	ROM_LOAD( "a-9.9.1d",    0x020000, 0x010000, CRC(92b791d8) SHA1(9eb382398db0dc7260348c6fb69aabcae6e8a9c0) )
	ROM_LOAD( "a-10.10.1de", 0x030000, 0x010000, CRC(f4a8e86b) SHA1(b3039aac1eb0c52e7409b766872796c1df96327f) )
	ROM_LOAD( "a-11.11.1ef", 0x040000, 0x010000, CRC(b2af5bfd) SHA1(ef9bcbd43d5fe08686e193f175d117a49b394caf) )
	ROM_LOAD( "a-12.12.1f",  0x050000, 0x010000, CRC(a50bf0b9) SHA1(cf67fb1ff13c61fb5ad1cfb4736bd1a67d7a456f) )
	ROM_LOAD( "a-13.13.1g",  0x060000, 0x010000, CRC(e7d297e3) SHA1(3a6c53a460f1f637a58e3e831f725b28fb981c43) )
	ROM_LOAD( "a-14.14.1gh", 0x070000, 0x010000, CRC(02c037a9) SHA1(5de414aa5171bc84e8fc8ae82d85b761542c96bc) )
	ROM_LOAD( "a-15.15.1h",  0x080000, 0x010000, CRC(222dbc94) SHA1(06d32ef02dd921e66fbd35f93a69f4d32bb98a03) )
	ROM_LOAD( "a-16.16.1j",  0x090000, 0x010000, CRC(7c6481e0) SHA1(50b00478cf16757669928b13ca336bfd01e86722) )
	ROM_LOAD( "a-17.17.1jk", 0x0a0000, 0x010000, CRC(6c19b324) SHA1(ac2dd74ec743b554b179d589d04c4a15b6cf75c8) )
	ROM_LOAD( "a-18.18.1kl", 0x0b0000, 0x010000, CRC(38f6e7d8) SHA1(d193dd7e48502d4c80e97eacfd5c976110b93486) )
	ROM_LOAD( "a-19.19.1l",  0x0c0000, 0x010000, CRC(6e521aea) SHA1(092e117fda00070d6361a72016a83afd5db5f3b6) )
	// original has 0xff filled ROM at 0xd0000-0xdffff
	// Sprites planes 2-3
	ROM_LOAD( "a-21.21.3f",  0x0e0000, 0x010000, CRC(ee694c18) SHA1(97bf0fe4f5b70cae94ac8b0713dcbd6e5b253064) )
	ROM_LOAD( "a-22.22.4f",  0x0f0000, 0x010000, CRC(2dc438a8) SHA1(fb402fecd5723df797ccbc8a3f5501619ae01fc7) )
	ROM_LOAD( "a-25.25.3h",  0x100000, 0x010000, CRC(cc6cd84a) SHA1(e8073fdf385f8e8240473b36d5011ebc87405fa7) )
	ROM_LOAD( "a-26.26.4h",  0x110000, 0x010000, CRC(be2eaac7) SHA1(42696e83be38790a65a2200aa88c9376f2e3fbf9) )
	ROM_LOAD( "a-29.29.3k",  0x120000, 0x010000, CRC(7d90034d) SHA1(10b24598db634bcb4e349f84ad74cf36c5f8d9d6) )
	ROM_LOAD( "a-30.30.4k",  0x130000, 0x010000, CRC(48ea345f) SHA1(c1beee4d6db95345e97eb57573c1a5b9ddd5cc41) )
	ROM_LOAD( "a-32.32.3l",  0x140000, 0x010000, CRC(b9563911) SHA1(329e485dc580f8cbd0550e17817af151ef58f933) )
	ROM_LOAD( "a-33.33.4l",  0x150000, 0x010000, CRC(61a9d0ea) SHA1(086d129e91ef13f7e001bd93e3862756b8febedf) )
	ROM_LOAD( "a-24.24.6f",  0x160000, 0x010000, CRC(a21e27ff) SHA1(bddee3a925bc5bd000b70ab6a024e1a7fd419da9) )
	ROM_LOAD( "a-23.23.5f",  0x170000, 0x010000, CRC(e77e3d75) SHA1(47397d04d1709f6aaf36b8f204560d7b7e28f3b4) )
	ROM_LOAD( "a-28.28.6h",  0x180000, 0x010000, CRC(f08c974e) SHA1(a1050241892f613c79c4ea87304545d973f89807) )
	ROM_LOAD( "a-27.27.5h",  0x190000, 0x010000, CRC(263f93b0) SHA1(6019b7a4f47da1f74cc76debe69ead975064817e) )
	ROM_LOAD( "a-31.31.6k",  0x1a0000, 0x010000, CRC(eb8db0bf) SHA1(ccef46e4c9034f9729712826f3a0a48c15034d7c) )
	// original has 0xff filled ROM at 0x1a0000-0x1affff

	ROM_REGION( 0x004000, "gfx4", 0 )
	ROM_LOAD( "a-3.3.6e", 0x000000, 0x004000, CRC(59416e03) SHA1(028e74e0b23c5063883d19f94da35719be4feada) ) // Characters planes 1-2, identical to the original
	ROM_CONTINUE(         0x000000, 0x004000) // 0xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x40000, "tilerom", 0 )    // background tilemaps, identical to the original
	ROM_LOAD( "k-7.57.7g",   0x000000, 0x010000, CRC(23d09d3d) SHA1(a0c71abc49c5fe59487a63b502e3d03021bfef13) )
	ROM_LOAD( "k-4.54.7e",   0x010000, 0x010000, CRC(ea16df6c) SHA1(68709a314b775c500817fc17d40a80204b2ae06c) )
	ROM_LOAD( "k-17.67.10r", 0x020000, 0x010000, CRC(72df2bd9) SHA1(9a0da618139673738b6b3302207255e44c5491a2) )
	ROM_LOAD( "k-16.66.10m", 0x030000, 0x010000, CRC(3e99d3d5) SHA1(9168a977e80f8c23c6126b9e64eb176290cf941a) )

	ROM_REGION( 0x0320, "proms", 0 ) // not used by the emulation, 13g doesn't match the original
	ROM_LOAD( "n82s129n.8m",  0x0000, 0x0100, CRC(864199ad) SHA1(b777df20b19fa7b7536120191df1875101e9d7af) )
	ROM_LOAD( "n82s129n.13g", 0x0100, 0x0100, CRC(c0e56586) SHA1(2abf93aef48af34f869b30f63c130513a97f86a3) )
	ROM_LOAD( "n82s129n.7m",  0x0200, 0x0100, CRC(4c734b64) SHA1(7a122b643bad3e3586821980efff023a63e5a029) )
	ROM_LOAD( "n82s123n.9p",  0x0300, 0x0020, CRC(06bcda53) SHA1(fa69b77697bb12aa6012d82ef5b504d3a1d20232) )

	ROM_REGION( 0x1a00, "plds", 0 )
	ROM_LOAD( "tibpal16l8-25cn.10g", 0x0000, 0x0104, CRC(b95bc9bc) SHA1(75c671dd0bef906bdec1f76fa8867d945502d3af) )
	ROM_LOAD( "tibpal16l8-25cn.10j", 0x0200, 0x0104, CRC(d5e7fb3c) SHA1(4be2fa491e3b2cfd68917bf335bfecf4375e633f) )
	ROM_LOAD( "tibpal16l8-25cn.11b", 0x0400, 0x0104, CRC(1db26b84) SHA1(1164d8ae135cad28067f8a711580a51be38724fb) )
	ROM_LOAD( "tibpal16l8-25cn.11e", 0x0600, 0x0104, CRC(e419a33f) SHA1(81431743df314d5ff06b44ba203a7a3113becbab) )
	ROM_LOAD( "tibpal16l8-25cn.12e", 0x0800, 0x0104, CRC(4803a097) SHA1(4264d98bbb8ea476e4fb6351fe0f08ca0fadaffb) )
	ROM_LOAD( "tibpal16l8-25cn.12f", 0x0a00, 0x0104, CRC(40689e0e) SHA1(a30e1bf2f9d38a2be25ef3850089770d403f219b) )
	ROM_LOAD( "tibpal16l8-25cn.16e", 0x0c00, 0x0104, CRC(9daca4f6) SHA1(7323c120a8d47304ccac509b671daa7565f6e601) )
	ROM_LOAD( "tibpal16l8-25cn.3q",  0x0e00, 0x0104, CRC(e7bb2b87) SHA1(76cfbbf5af06a70d44230993447b2616babe16e4) )
	ROM_LOAD( "tibpal16l8-25cn.6m",  0x1000, 0x0104, CRC(6675dcc7) SHA1(06b8e01a458e2bf4e24bbd6b0a0d0863406fb44a) )
	ROM_LOAD( "tibpal16l8-25cn.7c",  0x1200, 0x0104, CRC(c8a1458f) SHA1(ca48afa619bc62191c1c690ea90a01b0fa2ce6d8) )
	ROM_LOAD( "tibpal16l8-25cn.7e",  0x1400, 0x0104, CRC(858b1c21) SHA1(7672729dff67cf804300a6ad3bc802aa9f5081e6) )
	ROM_LOAD( "tibpal16r4-25cn.7a",  0x1600, 0x0104, CRC(b3c22357) SHA1(aa0005471eb08cc4ba34acb35cde7a82712409e4) )
	ROM_LOAD( "tibpal16r4-25cn.7d",  0x1800, 0x0104, CRC(29cdd190) SHA1(1b7383cda958f0cea53231c3b52d9040faee677d) )
ROM_END


ROM_START( sfan )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE("sfe-19.2a", 0x00000, 0x10000, CRC(8346c3ca) SHA1(404e26d210e453ef0f03b092d70c770106eed1d1) )
	ROM_LOAD16_BYTE("sfe-22.2c", 0x00001, 0x10000, CRC(3a4bfaa8) SHA1(6a6fc8d967838eca7d2973de987bb350c25628d5) )
	ROM_LOAD16_BYTE("sfe-20.3a", 0x20000, 0x10000, CRC(b40e67ee) SHA1(394987dc4c306351b1657d10528ecb665700c4db) )
	ROM_LOAD16_BYTE("sfe-23.3c", 0x20001, 0x10000, CRC(477c3d5b) SHA1(6443334b3546550e5d97cf4057b279ec7b3cd758) )
	ROM_LOAD16_BYTE("sfe-21.4a", 0x40000, 0x10000, CRC(2547192b) SHA1(aaf07c613a6c42ec1dc82ffa86d00044b4ea27fc) )
	ROM_LOAD16_BYTE("sfe-24.4c", 0x40001, 0x10000, CRC(79680f4e) SHA1(df596fa5b49a336fe462c2be7b454e695f5382db) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the music CPU */
	ROM_LOAD( "sf-02.7k", 0x0000, 0x8000, CRC(4a9ac534) SHA1(933645f8db4756aa2a35a843c3ac6f93cb8d565d) )

	ROM_REGION( 0x40000, "audio2", 0 )  /* 256k for the samples CPU */
	ROM_LOAD( "sfu-00.1h",0x00000, 0x20000, CRC(a7cce903) SHA1(76f521c9a00abd95a3491ab95e8eccd0fc7ea0e5) )
	ROM_LOAD( "sf-01.2k", 0x20000, 0x20000, CRC(86e0f0d5) SHA1(7cef8056f83dac15f1b47d7be705d26170858337) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "sf-39.2k", 0x000000, 0x020000, CRC(cee3d292) SHA1(a8c22f1dc81976e8dd5d6c70361c61fa3f9f89d6) ) /* Background b planes 0-1*/
	ROM_LOAD( "sf-38.1k", 0x020000, 0x020000, CRC(2ea99676) SHA1(5f3eb77e75f0ee27fb8fc7bab2819b3fdd480206) )
	ROM_LOAD( "sf-41.4k", 0x040000, 0x020000, CRC(e0280495) SHA1(e52c79feed590535b9a0b71ccadd0ed27d04ff45) ) /* planes 2-3 */
	ROM_LOAD( "sf-40.3k", 0x060000, 0x020000, CRC(c70b30de) SHA1(26112ee1720b6ad0e2e29e2d25ee2ec76fca0e3a) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "sf-25.1d", 0x000000, 0x020000, CRC(7f23042e) SHA1(a355fd7047fb1a71ab5cd08e1afd82c2558494c1) ) /* Background m planes 0-1 */
	ROM_LOAD( "sf-28.1e", 0x020000, 0x020000, CRC(92f8b91c) SHA1(6d958bc45131810d7b0af02be939ce37a39c35e8) )
	ROM_LOAD( "sf-30.1g", 0x040000, 0x020000, CRC(b1399856) SHA1(7c956d49b2e73291182ea1ec4cebd3411d1322a1) )
	ROM_LOAD( "sf-34.1h", 0x060000, 0x020000, CRC(96b6ae2e) SHA1(700e050463b7a29a1eb08007a2add045afdcd8a0) )
	ROM_LOAD( "sf-26.2d", 0x080000, 0x020000, CRC(54ede9f5) SHA1(c2cb354a6b32047759945fa3ecafc70ba7d1dda1) ) /* planes 2-3 */
	ROM_LOAD( "sf-29.2e", 0x0a0000, 0x020000, CRC(f0649a67) SHA1(eeda256527f7a2ee2d5e0688c505a01de548bc54) )
	ROM_LOAD( "sf-31.2g", 0x0c0000, 0x020000, CRC(8f4dd71a) SHA1(28b82c540df04c91a2dd6cbbc9a95bbebda6643b) )
	ROM_LOAD( "sf-35.2h", 0x0e0000, 0x020000, CRC(70c00fb4) SHA1(7c5504a5aedd3be7b663c5090eb22243e3fa669b) )

	ROM_REGION( 0x1c0000, "gfx3", 0 )
	/* Sprites planes 1-2 */
	ROM_LOAD( "sf-15.1m", 0x000000, 0x020000, CRC(fc0113db) SHA1(7c19603129be5f6e1ccd07fd8b7ee1cbf86468db) )
	ROM_LOAD( "sf-16.2m", 0x020000, 0x020000, CRC(82e4a6d3) SHA1(5ec519c2740c66f5da27ced1db99e19fe38fdad7) )
	ROM_LOAD( "sf-11.1k", 0x040000, 0x020000, CRC(e112df1b) SHA1(3f9856f69b457d79fe085bf51dfb2efcd98f883d) )
	ROM_LOAD( "sf-12.2k", 0x060000, 0x020000, CRC(42d52299) SHA1(6560c38f5fd5a47db7728cc7df83d2169157174f) )
	ROM_LOAD( "sf-07.1h", 0x080000, 0x020000, CRC(49f340d9) SHA1(65822efefa198791a632ef851a5ce06a71b4ed0f) )
	ROM_LOAD( "sf-08.2h", 0x0a0000, 0x020000, CRC(95ece9b1) SHA1(f0a15fce5cd9617fa5d4dd43bd5b6ea190dace85) )
	ROM_LOAD( "sf-03.1f", 0x0c0000, 0x020000, CRC(5ca05781) SHA1(004f5ad34798471b39bd4612c797f0913ed0fb4a) )
	/* Sprites planes 2-3 */
	ROM_LOAD( "sf-17.3m", 0x0e0000, 0x020000, CRC(69fac48e) SHA1(c9272217256c73cb8ddb4fbbfb5905ce1122c746) )
	ROM_LOAD( "sf-18.4m", 0x100000, 0x020000, CRC(71cfd18d) SHA1(4c17e2124f3456d6b13ede8ad3ae916b53f9bb7e) )
	ROM_LOAD( "sf-13.3k", 0x120000, 0x020000, CRC(fa2eb24b) SHA1(96f3bd54c340771577cc232ebde93965421f2557) )
	ROM_LOAD( "sf-14.4k", 0x140000, 0x020000, CRC(ad955c95) SHA1(549d6a5125432aa45d03f15e76f6c2c8ab2e05a3) )
	ROM_LOAD( "sf-09.3h", 0x160000, 0x020000, CRC(41b73a31) SHA1(aaa7a53e29fe23a1ca8ec4430f7efcbd774a8cbf) )
	ROM_LOAD( "sf-10.4h", 0x180000, 0x020000, CRC(91c41c50) SHA1(b03fb9b3c553fb4aae45ad6997eeb7bb95fdcce3) )
	ROM_LOAD( "sf-05.3f", 0x1a0000, 0x020000, CRC(538c7cbe) SHA1(f030a9562fbb93d1534b91343ca3f429cdbd0136) )

	ROM_REGION( 0x004000, "gfx4", 0 )
	ROM_LOAD( "sf-27.4d", 0x000000, 0x004000, CRC(2b09b36d) SHA1(9fe1dd3a9396fbb06f30247cfe526653553beca1) ) /* Characters planes 1-2 */

	ROM_REGION( 0x40000, "tilerom", 0 )    /* background tilemaps */
	ROM_LOAD( "sf-37.4h", 0x000000, 0x010000, CRC(23d09d3d) SHA1(a0c71abc49c5fe59487a63b502e3d03021bfef13) )
	ROM_LOAD( "sf-36.3h", 0x010000, 0x010000, CRC(ea16df6c) SHA1(68709a314b775c500817fc17d40a80204b2ae06c) )
	ROM_LOAD( "sf-32.3g", 0x020000, 0x010000, CRC(72df2bd9) SHA1(9a0da618139673738b6b3302207255e44c5491a2) )
	ROM_LOAD( "sf-33.4g", 0x030000, 0x010000, CRC(3e99d3d5) SHA1(9168a977e80f8c23c6126b9e64eb176290cf941a) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "sfb05.mb7114h.12k", 0x0000, 0x0100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) ) /* MB7114H */
	ROM_LOAD( "sfb10.mb7114h.11h", 0x0100, 0x0100, CRC(c0e56586) SHA1(2abf93aef48af34f869b30f63c130513a97f86a3) ) /* MB7114H */
	ROM_LOAD( "sfb04.mb7114h.12j", 0x0200, 0x0100, CRC(4c734b64) SHA1(7a122b643bad3e3586821980efff023a63e5a029) ) /* MB7114H */
	ROM_LOAD( "sfb00.mb7051.13h",  0x0300, 0x0020, CRC(06bcda53) SHA1(fa69b77697bb12aa6012d82ef5b504d3a1d20232) ) /* MMI-7603 or MB7051 (equiv to 82s123 32x8 TS) */
ROM_END


ROM_START( sfjan )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sf_n_19a.2a.27c512", 0x000000, 0x010000, CRC(8346c3ca) SHA1(404e26d210e453ef0f03b092d70c770106eed1d1) )
	ROM_LOAD16_BYTE( "sf_n_22a.2c.27c512", 0x000001, 0x010000, CRC(3a4bfaa8) SHA1(6a6fc8d967838eca7d2973de987bb350c25628d5) )
	ROM_LOAD16_BYTE( "sf_n_20a.3a.27c512", 0x020000, 0x010000, CRC(7e00b1dd) SHA1(cae5e7e3ee8876d7a67f4afe4e5ddb75b90d1ded) )
	ROM_LOAD16_BYTE( "sf_n_23a.3c.27c512", 0x020001, 0x010000, CRC(1cf3c108) SHA1(b94eac4fb2dc2a1ba2ae4079299a546fe471913e) )
	ROM_LOAD16_BYTE( "sf_n_21a.4a.27c512", 0x040000, 0x010000, CRC(2547192b) SHA1(aaf07c613a6c42ec1dc82ffa86d00044b4ea27fc) )
	ROM_LOAD16_BYTE( "sf_n_24a.4c.27c512", 0x040001, 0x010000, CRC(79680f4e) SHA1(df596fa5b49a336fe462c2be7b454e695f5382db) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the music CPU */
	ROM_LOAD( "sf-02.bin", 0x0000, 0x8000, CRC(4a9ac534) SHA1(933645f8db4756aa2a35a843c3ac6f93cb8d565d) ) // sf_02a.7k.27256

	ROM_REGION( 0x40000, "audio2", 0 )  /* 256k for the samples CPU */
	ROM_LOAD( "sf-00.bin", 0x00000, 0x20000, CRC(4b733845) SHA1(f7ff46e02f8ce6682d6e573588271bae2edfa90f) ) // sf_n_00.1h.27c1000
	ROM_LOAD( "sf-01.bin", 0x20000, 0x20000, CRC(86e0f0d5) SHA1(7cef8056f83dac15f1b47d7be705d26170858337) ) // sf_n_01.1k.27c1000

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "sf-39.2k", 0x000000, 0x020000, CRC(cee3d292) SHA1(a8c22f1dc81976e8dd5d6c70361c61fa3f9f89d6) ) /* Background b planes 0-1*/
	ROM_LOAD( "sf-38.1k", 0x020000, 0x020000, CRC(2ea99676) SHA1(5f3eb77e75f0ee27fb8fc7bab2819b3fdd480206) )
	ROM_LOAD( "sf-41.4k", 0x040000, 0x020000, CRC(e0280495) SHA1(e52c79feed590535b9a0b71ccadd0ed27d04ff45) ) /* planes 2-3 */
	ROM_LOAD( "sf-40.3k", 0x060000, 0x020000, CRC(c70b30de) SHA1(26112ee1720b6ad0e2e29e2d25ee2ec76fca0e3a) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "sf-25.1d", 0x000000, 0x020000, CRC(7f23042e) SHA1(a355fd7047fb1a71ab5cd08e1afd82c2558494c1) ) /* Background m planes 0-1 */
	ROM_LOAD( "sf-28.1e", 0x020000, 0x020000, CRC(92f8b91c) SHA1(6d958bc45131810d7b0af02be939ce37a39c35e8) )
	ROM_LOAD( "sf-30.1g", 0x040000, 0x020000, CRC(b1399856) SHA1(7c956d49b2e73291182ea1ec4cebd3411d1322a1) )
	ROM_LOAD( "sf-34.1h", 0x060000, 0x020000, CRC(96b6ae2e) SHA1(700e050463b7a29a1eb08007a2add045afdcd8a0) )
	ROM_LOAD( "sf-26.2d", 0x080000, 0x020000, CRC(54ede9f5) SHA1(c2cb354a6b32047759945fa3ecafc70ba7d1dda1) ) /* planes 2-3 */
	ROM_LOAD( "sf-29.2e", 0x0a0000, 0x020000, CRC(f0649a67) SHA1(eeda256527f7a2ee2d5e0688c505a01de548bc54) )
	ROM_LOAD( "sf-31.2g", 0x0c0000, 0x020000, CRC(8f4dd71a) SHA1(28b82c540df04c91a2dd6cbbc9a95bbebda6643b) )
	ROM_LOAD( "sf-35.2h", 0x0e0000, 0x020000, CRC(70c00fb4) SHA1(7c5504a5aedd3be7b663c5090eb22243e3fa669b) )

	ROM_REGION( 0x1c0000, "gfx3", 0 )
	/* Sprites planes 1-2 */
	ROM_LOAD( "sf-15.1m", 0x000000, 0x020000, CRC(fc0113db) SHA1(7c19603129be5f6e1ccd07fd8b7ee1cbf86468db) )
	ROM_LOAD( "sf-16.2m", 0x020000, 0x020000, CRC(82e4a6d3) SHA1(5ec519c2740c66f5da27ced1db99e19fe38fdad7) )
	ROM_LOAD( "sf-11.1k", 0x040000, 0x020000, CRC(e112df1b) SHA1(3f9856f69b457d79fe085bf51dfb2efcd98f883d) )
	ROM_LOAD( "sf-12.2k", 0x060000, 0x020000, CRC(42d52299) SHA1(6560c38f5fd5a47db7728cc7df83d2169157174f) )
	ROM_LOAD( "sf-07.1h", 0x080000, 0x020000, CRC(49f340d9) SHA1(65822efefa198791a632ef851a5ce06a71b4ed0f) )
	ROM_LOAD( "sf-08.2h", 0x0a0000, 0x020000, CRC(95ece9b1) SHA1(f0a15fce5cd9617fa5d4dd43bd5b6ea190dace85) )
	ROM_LOAD( "sf-03.1f", 0x0c0000, 0x020000, CRC(5ca05781) SHA1(004f5ad34798471b39bd4612c797f0913ed0fb4a) )
	/* Sprites planes 2-3 */
	ROM_LOAD( "sf-17.3m", 0x0e0000, 0x020000, CRC(69fac48e) SHA1(c9272217256c73cb8ddb4fbbfb5905ce1122c746) )
	ROM_LOAD( "sf-18.4m", 0x100000, 0x020000, CRC(71cfd18d) SHA1(4c17e2124f3456d6b13ede8ad3ae916b53f9bb7e) )
	ROM_LOAD( "sf-13.3k", 0x120000, 0x020000, CRC(fa2eb24b) SHA1(96f3bd54c340771577cc232ebde93965421f2557) )
	ROM_LOAD( "sf-14.4k", 0x140000, 0x020000, CRC(ad955c95) SHA1(549d6a5125432aa45d03f15e76f6c2c8ab2e05a3) )
	ROM_LOAD( "sf-09.3h", 0x160000, 0x020000, CRC(41b73a31) SHA1(aaa7a53e29fe23a1ca8ec4430f7efcbd774a8cbf) )
	ROM_LOAD( "sf-10.4h", 0x180000, 0x020000, CRC(91c41c50) SHA1(b03fb9b3c553fb4aae45ad6997eeb7bb95fdcce3) )
	ROM_LOAD( "sf-05.3f", 0x1a0000, 0x020000, CRC(538c7cbe) SHA1(f030a9562fbb93d1534b91343ca3f429cdbd0136) )

	ROM_REGION( 0x004000, "gfx4", 0 )
	ROM_LOAD( "sf_27.4d.27256", 0x000000, 0x004000, CRC(59416e03) SHA1(028e74e0b23c5063883d19f94da35719be4feada) ) //  First half FF, second half matches known
	ROM_CONTINUE( 0, 0x004000 )

	ROM_REGION( 0x40000, "tilerom", 0 )    /* background tilemaps */
	ROM_LOAD( "sf-37.4h", 0x000000, 0x010000, CRC(23d09d3d) SHA1(a0c71abc49c5fe59487a63b502e3d03021bfef13) ) // sf_37.4h.27c512
	ROM_LOAD( "sf-36.3h", 0x010000, 0x010000, CRC(ea16df6c) SHA1(68709a314b775c500817fc17d40a80204b2ae06c) ) // sf_36.3h.27c512
	ROM_LOAD( "sf-32.3g", 0x020000, 0x010000, CRC(72df2bd9) SHA1(9a0da618139673738b6b3302207255e44c5491a2) ) // sf_32.3g.27c512
	ROM_LOAD( "sf-33.4g", 0x030000, 0x010000, CRC(3e99d3d5) SHA1(9168a977e80f8c23c6126b9e64eb176290cf941a) ) // sf_33.4g.27c512

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "sfb05.12k",    0x0000, 0x0100, CRC(864199ad) SHA1(b777df20b19fa7b7536120191df1875101e9d7af) ) /* unknown */
	ROM_LOAD( "sfb00.13h",    0x0100, 0x0100, CRC(bd3f8c5d) SHA1(c31ee9f466f05a21612f5ea29fb8c7c25dc9e011) ) /* unknown */
	ROM_LOAD( "sfb04.12j",    0x0200, 0x0100, CRC(4c734b64) SHA1(7a122b643bad3e3586821980efff023a63e5a029) ) /* unknown */
	ROM_LOAD( "mmi-7603.13h", 0x0300, 0x0020, CRC(06bcda53) SHA1(fa69b77697bb12aa6012d82ef5b504d3a1d20232) ) /* unknown */
ROM_END

ROM_START( sfw )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE("sfs19e.2a", 0x00000, 0x10000, CRC(56eabd3a) SHA1(fe6edb15e12af2d1c39c4f0224444509b940aba3) )
	ROM_LOAD16_BYTE("sfs22e.2c", 0x00001, 0x10000, CRC(34541285) SHA1(f8c673e5ee7647b64ada284c4d0ee6cb0f48fd92) )
	ROM_LOAD16_BYTE("sfs20e.3a", 0x20000, 0x10000, CRC(ea8d3a70) SHA1(bdc90569573f0877b839008fe671411c93bc47d8) )
	ROM_LOAD16_BYTE("sfs23e.3c", 0x20001, 0x10000, CRC(21e7d1c7) SHA1(c8fd36ef62ee8be6a5a0532f5a18d6a784b6abf8) )
	ROM_LOAD16_BYTE("sfs21e.4a", 0x40000, 0x10000, CRC(e0d3f410) SHA1(442841df8c29a43ad8322b2f91c615f98ded3950) )
	ROM_LOAD16_BYTE("sfs24e.4c", 0x40001, 0x10000, CRC(bcd20105) SHA1(ce0383e839c7c08534dfbe885f709c727352c3f0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the music CPU */
	ROM_LOAD( "sf-02.7k", 0x0000, 0x8000, CRC(4a9ac534) SHA1(933645f8db4756aa2a35a843c3ac6f93cb8d565d) )

	ROM_REGION( 0x1000, "protcpu", 0 )
	ROM_LOAD( "sf.14e", 0x0000, 0x1000, CRC(6588891f) SHA1(699a96c682dd527dc77aa5cb2c2655136d2bfc90) )

	ROM_REGION( 0x40000, "audio2", 0 )  /* 256k for the samples CPU */
	ROM_LOAD( "sfu-00.1h",0x00000, 0x20000, CRC(a7cce903) SHA1(76f521c9a00abd95a3491ab95e8eccd0fc7ea0e5) )
	ROM_LOAD( "sf-01.2k", 0x20000, 0x20000, CRC(86e0f0d5) SHA1(7cef8056f83dac15f1b47d7be705d26170858337) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "sf-39.2k", 0x000000, 0x020000, CRC(cee3d292) SHA1(a8c22f1dc81976e8dd5d6c70361c61fa3f9f89d6) ) /* Background b planes 0-1*/
	ROM_LOAD( "sf-38.1k", 0x020000, 0x020000, CRC(2ea99676) SHA1(5f3eb77e75f0ee27fb8fc7bab2819b3fdd480206) )
	ROM_LOAD( "sf-41.4k", 0x040000, 0x020000, CRC(e0280495) SHA1(e52c79feed590535b9a0b71ccadd0ed27d04ff45) ) /* planes 2-3 */
	ROM_LOAD( "sf-40.3k", 0x060000, 0x020000, CRC(c70b30de) SHA1(26112ee1720b6ad0e2e29e2d25ee2ec76fca0e3a) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "sf-25.1d", 0x000000, 0x020000, CRC(7f23042e) SHA1(a355fd7047fb1a71ab5cd08e1afd82c2558494c1) ) /* Background m planes 0-1 */
	ROM_LOAD( "sf-28.1e", 0x020000, 0x020000, CRC(92f8b91c) SHA1(6d958bc45131810d7b0af02be939ce37a39c35e8) )
	ROM_LOAD( "sf-30.1g", 0x040000, 0x020000, CRC(b1399856) SHA1(7c956d49b2e73291182ea1ec4cebd3411d1322a1) )
	ROM_LOAD( "sf-34.1h", 0x060000, 0x020000, CRC(96b6ae2e) SHA1(700e050463b7a29a1eb08007a2add045afdcd8a0) )
	ROM_LOAD( "sf-26.2d", 0x080000, 0x020000, CRC(54ede9f5) SHA1(c2cb354a6b32047759945fa3ecafc70ba7d1dda1) ) /* planes 2-3 */
	ROM_LOAD( "sf-29.2e", 0x0a0000, 0x020000, CRC(f0649a67) SHA1(eeda256527f7a2ee2d5e0688c505a01de548bc54) )
	ROM_LOAD( "sf-31.2g", 0x0c0000, 0x020000, CRC(8f4dd71a) SHA1(28b82c540df04c91a2dd6cbbc9a95bbebda6643b) )
	ROM_LOAD( "sf-35.2h", 0x0e0000, 0x020000, CRC(70c00fb4) SHA1(7c5504a5aedd3be7b663c5090eb22243e3fa669b) )

	ROM_REGION( 0x1c0000, "gfx3", 0 )
	/* Sprites planes 1-2 */
	ROM_LOAD( "sf-15.1m", 0x000000, 0x020000, CRC(fc0113db) SHA1(7c19603129be5f6e1ccd07fd8b7ee1cbf86468db) )
	ROM_LOAD( "sf-16.2m", 0x020000, 0x020000, CRC(82e4a6d3) SHA1(5ec519c2740c66f5da27ced1db99e19fe38fdad7) )
	ROM_LOAD( "sf-11.1k", 0x040000, 0x020000, CRC(e112df1b) SHA1(3f9856f69b457d79fe085bf51dfb2efcd98f883d) )
	ROM_LOAD( "sf-12.2k", 0x060000, 0x020000, CRC(42d52299) SHA1(6560c38f5fd5a47db7728cc7df83d2169157174f) )
	ROM_LOAD( "sf-07.1h", 0x080000, 0x020000, CRC(49f340d9) SHA1(65822efefa198791a632ef851a5ce06a71b4ed0f) )
	ROM_LOAD( "sf-08.2h", 0x0a0000, 0x020000, CRC(95ece9b1) SHA1(f0a15fce5cd9617fa5d4dd43bd5b6ea190dace85) )
	ROM_LOAD( "sf-03.1f", 0x0c0000, 0x020000, CRC(5ca05781) SHA1(004f5ad34798471b39bd4612c797f0913ed0fb4a) )
	/* Sprites planes 2-3 */
	ROM_LOAD( "sf-17.3m", 0x0e0000, 0x020000, CRC(69fac48e) SHA1(c9272217256c73cb8ddb4fbbfb5905ce1122c746) )
	ROM_LOAD( "sf-18.4m", 0x100000, 0x020000, CRC(71cfd18d) SHA1(4c17e2124f3456d6b13ede8ad3ae916b53f9bb7e) )
	ROM_LOAD( "sf-13.3k", 0x120000, 0x020000, CRC(fa2eb24b) SHA1(96f3bd54c340771577cc232ebde93965421f2557) )
	ROM_LOAD( "sf-14.4k", 0x140000, 0x020000, CRC(ad955c95) SHA1(549d6a5125432aa45d03f15e76f6c2c8ab2e05a3) )
	ROM_LOAD( "sf-09.3h", 0x160000, 0x020000, CRC(41b73a31) SHA1(aaa7a53e29fe23a1ca8ec4430f7efcbd774a8cbf) )
	ROM_LOAD( "sf-10.4h", 0x180000, 0x020000, CRC(91c41c50) SHA1(b03fb9b3c553fb4aae45ad6997eeb7bb95fdcce3) )
	ROM_LOAD( "sf-05.3f", 0x1a0000, 0x020000, CRC(538c7cbe) SHA1(f030a9562fbb93d1534b91343ca3f429cdbd0136) )

	ROM_REGION( 0x004000, "gfx4", 0 )
	ROM_LOAD( "sf-27.4d", 0x000000, 0x004000, CRC(2b09b36d) SHA1(9fe1dd3a9396fbb06f30247cfe526653553beca1) ) /* Characters planes 1-2 */

	ROM_REGION( 0x40000, "tilerom", 0 )    /* background tilemaps */
	ROM_LOAD( "sf-37.4h", 0x000000, 0x010000, CRC(23d09d3d) SHA1(a0c71abc49c5fe59487a63b502e3d03021bfef13) )
	ROM_LOAD( "sf-36.3h", 0x010000, 0x010000, CRC(ea16df6c) SHA1(68709a314b775c500817fc17d40a80204b2ae06c) )
	ROM_LOAD( "sf-32.3g", 0x020000, 0x010000, CRC(72df2bd9) SHA1(9a0da618139673738b6b3302207255e44c5491a2) )
	ROM_LOAD( "sf-33.4g", 0x030000, 0x010000, CRC(3e99d3d5) SHA1(9168a977e80f8c23c6126b9e64eb176290cf941a) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "sfb05.mb7114h.12k", 0x0000, 0x0100, CRC(75af3553) SHA1(14da009592877a6097b34ea844fa897ceda7465e) ) /* MB7114H */
	ROM_LOAD( "sfb10.mb7114h.11h", 0x0100, 0x0100, CRC(c0e56586) SHA1(2abf93aef48af34f869b30f63c130513a97f86a3) ) /* MB7114H */
	ROM_LOAD( "sfb04.mb7114h.12j", 0x0200, 0x0100, CRC(4c734b64) SHA1(7a122b643bad3e3586821980efff023a63e5a029) ) /* MB7114H */
	ROM_LOAD( "sfb00.mb7051.13h",  0x0300, 0x0020, CRC(06bcda53) SHA1(fa69b77697bb12aa6012d82ef5b504d3a1d20232) ) /* MMI-7603 or MB7051 (equiv to 82s123 32x8 TS) */
ROM_END

ROM_START( sfp )
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE("prg8.2a", 0x00000, 0x20000, CRC(d48d06a3) SHA1(d899771c66c1e7a5caa11f67a1122adb6f0f4d28) )
	ROM_LOAD16_BYTE("prg0.2c", 0x00001, 0x20000, CRC(e8606c1a) SHA1(be94203cba733e337993e6f386ff5ce1e76d8913) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the music CPU */
	ROM_LOAD( "sound.9j", 0x0000, 0x8000, CRC(43cd32ae) SHA1(42e59becde5761eb5d5bc310d2bc690f6f16882a) )

	ROM_REGION( 0x10000, "audio2", 0 )  /* 256k for the samples CPU */
	ROM_LOAD( "voice.1g", 0x00000, 0x10000, CRC(3f23c180) SHA1(fb4e3bb835d94a733eacc0b1df9fe19fa1120997) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "bkchr.2k", 0x000000, 0x020000, CRC(e4d47aca) SHA1(597ed03e5c8328ec7209282247080c171eaedf86) ) /* Background b planes 0-1*/
	ROM_LOAD( "bkchr.1k", 0x020000, 0x020000, CRC(5a1cbc1b) SHA1(ad7bf117a7d1c0ef2aa47e133b0889092a009ae5) )
	ROM_LOAD( "bkchr.4k", 0x040000, 0x020000, CRC(c351bd48) SHA1(58131974d378a91f03f8c0bbd2ea384bd4fe501a) ) /* planes 2-3 */
	ROM_LOAD( "bkchr.3k", 0x060000, 0x020000, CRC(6bb2b050) SHA1(d36419dabdc0a90b76e295b746928d9e1e69674a) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "mchr.1d", 0x000000, 0x020000, CRC(ab06a60b) SHA1(44febaa2ac8f060ed297b69af1fd258164ff565d) ) /* Background m planes 0-1 */
	ROM_LOAD( "mchr.1e", 0x020000, 0x020000, CRC(d221387d) SHA1(012dc8c646a6a4b8bf905d859e3465b4bcaaed67) )
	ROM_LOAD( "mchr.1g", 0x040000, 0x020000, CRC(1e4c1712) SHA1(543b47a865d11dd91331c0236c5578dbe7549881) )
	ROM_LOAD( "mchr.1h", 0x060000, 0x020000, CRC(a381f529) SHA1(7e427894f8440c23c92ce5d1f118b7a1d70b0282) )
	ROM_LOAD( "mchr.2d", 0x080000, 0x020000, CRC(e52303c4) SHA1(1ae4979c53e589d9a5e7c0dbbf33b980d10274ac) ) /* planes 2-3 */
	ROM_LOAD( "mchr.2e", 0x0a0000, 0x020000, CRC(23b9a6a1) SHA1(bf7f67d97cfaa1f4c78f290c7c18e099566709c7) )
	ROM_LOAD( "mchr.2g", 0x0c0000, 0x020000, CRC(1283ac09) SHA1(229a507e0a1c46b451d8879e690e8557d21d588d) )
	ROM_LOAD( "mchr.2h", 0x0e0000, 0x020000, CRC(cc6bf05c) SHA1(4e83dd55c88d5b539ab1dcae5bfd16195bcd2565) )

	/* these graphic roms seem mismatched with this version of the prototype, they don't contain the graphics needed
	   for the bonus round, or have complete tile sets graphic set for 2 of the characters which are used by the prototype
	   (all of Joe is missing, many of Mike's poses are missing) If you use the original ROMs instead the graphics are
	   correct, so the prototype is clearly already referencing the final tile arrangement for them.  The glitches
	   therefore are not emulation bugs, if the PCB contained the same mismatched ROMs it would exhibit the same glitches. */
	ROM_REGION( 0x1c0000, "gfx3", ROMREGION_ERASE00 )
	/* Sprites planes 1-2 */
	ROM_LOAD( "b1m.bin", 0x000000, 0x010000, CRC(64758232) SHA1(20d21677b791a7f96afed54b286ee92adb80456d) )
	ROM_LOAD( "b2m.bin", 0x010000, 0x010000, CRC(d958f5ad) SHA1(0e5c98a24814f5e1e6346dba4cfbd3a3a72ed724) )
	ROM_LOAD( "b1k.bin", 0x020000, 0x010000, CRC(e766f5fe) SHA1(ad48a543507a981d844f0e2d5cceb689775b9ad6) )
	ROM_LOAD( "b2k.bin", 0x030000, 0x010000, CRC(e71572d3) SHA1(752540bbabf56c883208b132e285b485d4b5b4ee) )
	ROM_LOAD( "b1h.bin", 0x040000, 0x010000, CRC(8494f38c) SHA1(8d99ae088bd5b479f10e69b0a960f07d10adc23b) )
	ROM_LOAD( "b2h.bin", 0x050000, 0x010000, CRC(1fc5f049) SHA1(bb6d5622247ec32ad044cde856cf67dddc3c732f) )
	/* Sprites planes 2-3 */
	ROM_LOAD( "b3m.bin", 0x0e0000, 0x010000, CRC(d136802e) SHA1(84c2a6b2a8bad7e9249b6dce9cbf5301526aa6af) )
	ROM_LOAD( "b4m.bin", 0x0f0000, 0x010000, CRC(b4fa85d3) SHA1(c15e36000bf68a838eb34c3872e342acbb9c140a) )
	ROM_LOAD( "b3k.bin", 0x100000, 0x010000, CRC(40e11cc8) SHA1(ed469a8629080da88ce6faeb232633f94e2816c3) )
	ROM_LOAD( "b4k.bin", 0x110000, 0x010000, CRC(5ca9716e) SHA1(87620083aa6a7697f6faf742ac0e47115af3e0f3) )
	ROM_LOAD( "b3h.bin", 0x120000, 0x010000, CRC(8c3d9173) SHA1(08df92d962852f88b42e76dfaf6bb23a80d84657) )
	ROM_LOAD( "b4h.bin", 0x130000, 0x010000, CRC(a2df66f8) SHA1(9349704fdb7b0919813cb48d4deacdbbdebb2fee) )

	ROM_REGION( 0x004000, "gfx4", 0 )
	ROM_LOAD( "vram.4d", 0x000000, 0x004000, CRC(bfadfb32) SHA1(8443ad9f02da5fb032017fc0c657b1bdc15e4f27) ) /* Characters planes 1-2 */

	ROM_REGION( 0x40000, "tilerom", 0 )    /* background tilemaps */
	ROM_LOAD( "bks1j10.5h", 0x000000, 0x010000, CRC(4934aacd) SHA1(15274ae8b26799e15c7a66ff89ffd386de1659d3) )
	ROM_LOAD( "bks1j18.3h", 0x010000, 0x010000, CRC(551ffc88) SHA1(4f9213f4e80033f910dd8aae44b2c6d9ba760d61) )
	ROM_LOAD( "ms1j10.3g",  0x020000, 0x010000, CRC(f92958b8) SHA1(da8fa64ea9ad27c737225681c49f7c57cc7afeed) )
	ROM_LOAD( "ms1j18.5g",  0x030000, 0x010000, CRC(89e35dc1) SHA1(368d0cce3bc39b3762d79df0c023242018fbbcb8) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "sfb05.bin",    0x0000, 0x0100, CRC(864199ad) SHA1(b777df20b19fa7b7536120191df1875101e9d7af) ) /* unknown */
	ROM_LOAD( "sfb00.bin",    0x0100, 0x0100, CRC(bd3f8c5d) SHA1(c31ee9f466f05a21612f5ea29fb8c7c25dc9e011) ) /* unknown */
	ROM_LOAD( "mb7114h.12j",  0x0200, 0x0100, CRC(4c734b64) SHA1(7a122b643bad3e3586821980efff023a63e5a029) ) /* unknown */
	ROM_LOAD( "mmi-7603.13h", 0x0300, 0x0020, CRC(06bcda53) SHA1(fa69b77697bb12aa6012d82ef5b504d3a1d20232) ) /* unknown */
ROM_END

} // anonymous namespace


GAME( 1987, sf,     0, sfus, sfus, sf_state, empty_init, ROT0, "Capcom",  "Street Fighter (US, set 1)", MACHINE_SUPPORTS_SAVE ) // Shows Capcom copyright
GAME( 1987, sfua,  sf, sfjp, sfjp, sf_state, empty_init, ROT0, "Capcom",  "Street Fighter (US, set 2) (protected)", MACHINE_SUPPORTS_SAVE ) // Shows Capcom USA copyright
GAME( 1987, sfj,   sf, sfjp, sfjp, sf_state, empty_init, ROT0, "Capcom",  "Street Fighter (Japan) (protected)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sfjan, sf, sfan, sfan, sf_state, empty_init, ROT0, "Capcom",  "Street Fighter (Japan, pneumatic buttons)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sfjbl, sf, sfjp, sfjp, sf_state, empty_init, ROT0, "bootleg", "Street Fighter (Japan, bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sfan,  sf, sfan, sfan, sf_state, empty_init, ROT0, "Capcom",  "Street Fighter (World, pneumatic buttons)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sfp,   sf, sfp,  sfan, sf_state, empty_init, ROT0, "Capcom",  "Street Fighter (prototype)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sfw,   sf, sfjp, sfjp, sf_state, empty_init, ROT0, "Capcom",  "Street Fighter (World) (protected)", MACHINE_SUPPORTS_SAVE )  // Shows Capcom copyright
