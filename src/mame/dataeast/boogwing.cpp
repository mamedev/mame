// license:BSD-3-Clause
// copyright-holders: Bryan McPhail, David Haywood

/*
    Boogie Wings (aka The Great Ragtime Show)
    Data East, 1992

    PCB No: DE-0379-1

    CPU: DE102
    Sound: HuC6280A, YM2151, YM3012, OKI M6295 (x2)
    OSC: 32.220MHz, 28.000MHz
    DIPs: 8 position (x2)
    PALs: (not dumped) VF-00 (PAL16L8)
                       VF-01 (PAL16L8)
                       VF-02 (22CV10P)
                       VF-03 (PAL16L8)
                       VF-04 (PAL16L8)
                       VF-05 (PAL16L8)

    PROM: MB7122 (compatible to 82S137, located near MBD-09)

    RAM: 62256 (x2), 6264 (x5)

    Data East Chips:   52 (x2)
                    141 (x2)
                    102
                    104
                    113
                    99
                    71 (x2)
                    200

    ROMs:
    kn00-2.2b   27c2001   \
    kn01-2.4b   27c2001    |  Main program
    kn02-2.2e   27c2001    |
    kn03-2.4e   27c2001   /
    kn04.8e     27c512    \
    kn05.9e     27c512    /   near 141's and MBD-00, MBD-01 and MBD-02
    kn06.18p    27c512        Sound Program
    mbd-00.8b   16M
    mbd-01.9b   16M
    mbd-02.10e  4M
    mbd-03.13b  16M
    mbd-04.14b  16M
    mbd-05.16b  16M
    mbd-06.17b  16M
    mbd-07.18b  16M
    mbd-08.19b  16M
    mbd-09.16p  4M         Oki Samples
    mbd-10.17p  4M         Oki Samples


    Driver by Bryan McPhail and David Haywood.

    DECO 99 "ACE" Chip hooked up by cam900.

    TODO:
        * Sprite priorities aren't verified to be 100% accurate.
          (Addendum - all known issues seem to be correct - see Sprite Priority Notes below).
        * There may be some kind of fullscreen palette effect (controlled by bit 3 in priority
          word - used at end of each level, and on final boss).
        * ACE chip isn't fully emulated.

    Sprite Priority Notes:
        * On the Imperial Science Museum level at the beginning, you fly behind a wall, but your
          shots go in front of it.  This is verified to be correct behavior by Guru.
        * There's a level where the player passes through several columns in a building and the
          player goes behind every 2nd one. That is correct as well.
        * There is a fire hydrant with an odd-looking spray of water on various levels.
          If you drop the hydrant in front of an enemy (a tank, for example), the priorities are
          wrong.  This is actual PCB behavior.  Also, the strange-looking water spray is correct.

    Alpha Blend Note:
        * There are semi-transparent round spots around your plane while fighting the final boss.
          These are correct.
        * The final boss shoots a blue beam downwards during the battle.  This should have alpha.
          It fades in to blue, then fades out to nothing again after a few seconds (Guru).
          (Potentially related to note above about bit 3 in priority word)

    2008-07
    Dip Locations added according to the manual of the JPN version
*/

#include "emu.h"


#include "deco102.h"
#include "deco104.h"
#include "deco16ic.h"
#include "deco_ace.h"
#include "decocrpt.h"
#include "decospr.h"


#include "cpu/h6280/h6280.h"
#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "video/bufsprite.h"

#include "screen.h"
#include "speaker.h"


namespace {

class boogwing_state : public driver_device
{
public:
	boogwing_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_deco104(*this, "ioprot")
		, m_deco_ace(*this, "deco_ace")
		, m_screen(*this, "screen")
		, m_deco_tilegen(*this, "tilegen%u", 1)
		, m_oki(*this, "oki%u", 1)
		, m_sprgen(*this, "spritegen%u", 1)
		, m_spriteram(*this, "spriteram%u", 1)
		, m_pf_rowscroll(*this, "pf%u_rowscroll", 1)
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void init_boogwing() ATTR_COLD;

	void boogwing(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device<deco104_device> m_deco104;
	required_device<deco_ace_device> m_deco_ace;
	required_device<screen_device> m_screen;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device_array<okim6295_device, 2> m_oki;
	required_device_array<decospr_device, 2> m_sprgen;
	required_device_array<buffered_spriteram16_device, 2> m_spriteram;

	required_shared_ptr_array<u16, 4> m_pf_rowscroll;
	required_shared_ptr<u16> m_decrypted_opcodes;

	u16 m_priority = 0U;
	bitmap_ind16 m_temp_bitmap = 0;
	bitmap_ind16 m_alpha_tmap_bitmap = 0;

	void sound_bankswitch_w(u8 data);
	void priority_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mix(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u16 ioprot_r(offs_t offset);
	void ioprot_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECO16IC_BANK_CB_MEMBER(bank_callback2);

	void audio_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
};


void boogwing_state::video_start()
{
	m_priority = 0;
	m_sprgen[0]->alloc_sprite_bitmap();
	m_sprgen[1]->alloc_sprite_bitmap();
	m_screen->register_screen_bitmap(m_temp_bitmap);
	m_screen->register_screen_bitmap(m_alpha_tmap_bitmap);
	save_item(NAME(m_priority));
}

constexpr u32 sub_blend_r32(u32 d, u32 s, u8 level)
{
	// stage 1 boss for ragtime, inverts source layer for a shadow effect,
	// >> 9 instead of >> 8 is a guess
	s ^= 0xffffff;
	return ((((s & 0x0000ff) * level + (d & 0x0000ff) * int(256 - level)) >> 9)) |
			((((s & 0x00ff00) * level + (d & 0x00ff00) * int(256 - level)) >> 9) & 0x00ff00) |
			((((s & 0xff0000) * level + (d & 0xff0000) * int(256 - level)) >> 9) & 0xff0000);
}

/* Mix the 2 sprite planes with the already rendered tilemaps..
 note, if we implement tilemap blending etc. too we'll probably have to mix those in here as well..

 this is just a reimplementation of the old priority system used before conversion but to work with
 the bitmaps.  It could probably be simplified / improved greatly, along with the long-standing bugs
 fixed, with manual mixing you have full control.

 apparently priority is based on a PROM, that should be used if possible.

 Reference video : https://www.youtube.com/watch?v=mRdIlP_erBM (Live stream)
*/
void boogwing_state::mix(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const paldata = &m_deco_ace->pen(0);

	u16 const priority = m_priority & 0x7;

	bitmap_ind16 const *sprite_bitmap1 = &m_sprgen[0]->get_sprite_temp_bitmap();
	bitmap_ind16 const *sprite_bitmap2 = &m_sprgen[1]->get_sprite_temp_bitmap();
	bitmap_ind16 const *alpha_tmap_bitmap = &m_alpha_tmap_bitmap;
	bitmap_ind8 const *priority_bitmap = &screen.priority();

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u16 const *srcline1 = &sprite_bitmap1->pix(y, 0);
		u16 const *srcline2 = &sprite_bitmap2->pix(y, 0);
		u16 const *srcline3 = &alpha_tmap_bitmap->pix(y, 0);
		u16 const *tmapline = &m_temp_bitmap.pix(y, 0);
		u8 const *srcpriline = &priority_bitmap->pix(y, 0);

		u32 *dstline = &bitmap.pix(y, 0);

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			u16 const pix1 = srcline1[x];
			u16 const pix2 = srcline2[x];
			u16 const pix3 = srcline3[x];
			u16 const tmappix = tmapline[x];

			/* Here we have
			 pix1 - raw pixel / colour / priority data from first 1st chip
			 pix2 - raw pixel / colour / priority data from first 2nd chip
			 pix3 - raw pixel data from alpha blended tilemap
			*/

			int pri1, pri2, pri3 = 0;
			int spri1, spri2;
			int alpha2 = m_deco_ace->get_alpha((pix2 >> 4) & 0xf);
			int const alpha3 = m_deco_ace->get_alpha(0x1f);

			// pix1 sprite vs pix2 sprite
			switch (priority)
			{
				// TODO - check only in pri mode 0/2??
				case 0x00:
					{
						if ((pix1 & 0x600) == 0x600)
							spri1 = 2;
						else if ((pix1 & 0x600) == 0x400)
							spri1 = 8;
						else
							spri1 = 32;
					}
					break;
				default:
					{
						if (pix1 & 0x400)
							spri1 = 8;
						else
							spri1 = 32;
					}
					break;
			}

			// pix1 sprite vs playfield
			switch (priority)
			{
				case 0x01:
					{
						if ((pix1 & 0x600))
							pri1 = 16;
						else
							pri1 = 64;
					}
					break;

				case 0x00:
					{
						if ((pix1 & 0x400) == 0x400)
							pri1 = 16;
						else
							pri1 = 64;
					}
					break;

				default:
					{
						if ((pix1 & 0x600) == 0x600)
							pri1 = 4;
						else if ((pix1 & 0x600) == 0x400)
							pri1 = 16;
						else
							pri1 = 64;
					}
					break;
			}

			// pix2 sprite vs pix1 sprite
			if ((pix2 & 0x600) == 0x600)
				spri2 = 4;
			else if ((pix2 & 0x600))
				spri2 = 16;
			else
				spri2 = 64;

			// Transparency
			if (pix2 & 0x100)
			{
				if (pix2 & 0x800) // Use LUT, ex : Explosions
					alpha2 = (pix2 & 8) ? 0xff : m_deco_ace->get_alpha(0x14 + ((pix2 - 1) & 0x7));
				else
					alpha2 = m_deco_ace->get_alpha(0x10 + ((pix2 & 0x80) >> 7));
			}
			else if (pix2 & 0x800)
				alpha2 = m_deco_ace->get_alpha(0x12 + ((pix2 & 0x80) >> 7));

			// pix2 sprite vs playfield
			switch (priority)
			{
				case 0x02:
					// Sprite vs playfield
					if ((pix2 & 0x600) == 0x600)
						pri2 = 4;
					else if ((pix2 & 0x600) == 0x400)
						pri2 = 16;
					else
						pri2 = 64;
					break;

				case 0x03:
					pri3 = 32;
					[[fallthrough]];
				default:
					if ((pix2 & 0x400) == 0x400)
						pri2 = 16;
					else
						pri2 = 64;
					break;
			}

			u8 bgpri = srcpriline[x];
			/* once we get here we have

			pri1 - 4/16/64 (sprite chip 1 pixel priority relative to bg)
			pri2 - 4/16/64 (sprite chip 2 pixel priority relative to bg)
			spri1 - 2/8/32 (priority of sprite chip 1 relative to other sprite chip)
			spri2 - 4/16/64 (priority of sprite chip 2 relative to other sprite chip)
			alpha2 - 0x80/0xff alpha level of sprite chip 2 pixels (0x80 if enabled, 0xff if not)
			alpha3 - alpha level of alpha-blended playfield pixels

			bgpri - 0 / 8 / 32 (from drawing tilemaps earlier, to compare above pri1/pri2 priorities against)
			pix1 - same as before (ready to extract just colour data from)
			pix2 - same as before  ^^
			*/

			const u16 calculated_coloffs = (m_priority & 8) ? 0x800 : 0;
			int drawnpixe1 = 0;
			if (pix1 & 0xf)
			{
				if (pri1 > bgpri)
				{
					dstline[x] = paldata[calculated_coloffs | ((pix1 & 0x1ff) + 0x500)];
					drawnpixe1 |= 1;
				}
			}

			if (pix2 & 0xf)
			{
				if ((drawnpixe1 == 0) && (tmappix != 0xffff))
					dstline[x] = paldata[calculated_coloffs | tmappix];

				if (pri2 > bgpri)
				{
					if ((!drawnpixe1) || (spri2 > spri1))
					{
						if (alpha2 >= 0xff)
						{
							dstline[x] = paldata[calculated_coloffs | ((pix2 & 0xff) + 0x700)];
						}
						else
						{
							u32 base = dstline[x];
							dstline[x] = alpha_blend_r32(base, paldata[calculated_coloffs | ((pix2 & 0xff) + 0x700)], alpha2);
						}
						drawnpixe1 |= 2;
					}
				}
			}

			if ((drawnpixe1 == 0) && (tmappix != 0xffff))
				dstline[x] = paldata[tmappix];

			// alpha blended tilemap handling
			if (pix3 & 0xf)
			{
				// TODO : sprite vs playfield priority, actual behavior of shadowing
				if (priority == 0x3)
				{
					bool const bg2_drawed = (bgpri == 8) && (!drawnpixe1);
					bool const sprite1_drawed = (drawnpixe1 & 1) && (pri1 <= pri3);
					bool const sprite2_drawed = (drawnpixe1 & 2) && (pri2 <= pri3);
					if ((bg2_drawed) || ((sprite1_drawed && (~drawnpixe1 & 2)) || (sprite2_drawed && (~drawnpixe1 & 1)) || (sprite1_drawed && sprite2_drawed)))
					{
						if (((pix2 & 0x900) != 0x900) || ((spri2 <= spri1) && sprite1_drawed))
						{
							// TODO: make it functional, check out modes 0x21 and 0x1000.
							dstline[x] = (m_deco_ace->get_aceram(0x1f) == 0x22) ?
								sub_blend_r32(dstline[x], paldata[((drawnpixe1 & 3) ? calculated_coloffs : 0) | pix3], alpha3) :
								alpha_blend_r32(dstline[x], paldata[((drawnpixe1 & 3) ? calculated_coloffs : 0) | pix3], alpha3);
						}
					}
				}
			}
		}
	}
}

u32 boogwing_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u16 const flip = m_deco_tilegen[0]->pf_control_r(0);
	u16 const priority = m_priority;

	// sprites are flipped relative to tilemaps
	flip_screen_set(BIT(flip, 7));
	m_sprgen[0]->set_flip_screen(!BIT(flip, 7));
	m_sprgen[1]->set_flip_screen(!BIT(flip, 7));

	// Draw sprite planes to bitmaps for later mixing
	m_sprgen[1]->draw_sprites(bitmap, cliprect, m_spriteram[1]->buffer(), 0x400);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram[0]->buffer(), 0x400);

	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2], m_pf_rowscroll[3]);

	// Draw playfields
	bitmap.fill(m_deco_ace->pen(0x400), cliprect); // pen not confirmed
	m_temp_bitmap.fill(0xffff, cliprect);
	screen.priority().fill(0);

	// bit&0x8 is definitely some kind of palette effect
	// bit&0x4 combines playfields
	if ((priority & 0x7) == 0x5)
	{
		m_deco_tilegen[0]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[1]->tilemap_12_combine_draw(screen, m_temp_bitmap, cliprect, 0, 32);
	}
	else if ((priority & 0x7) == 0x4)
	{
		m_deco_tilegen[1]->tilemap_12_combine_draw(screen, m_temp_bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[0]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, 0, 32);
	}
	else if ((priority & 0x7) == 0x1 || (priority & 0x7) == 0x2)
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[0]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, 0, 8);
		m_deco_tilegen[1]->tilemap_1_draw(screen, m_temp_bitmap, cliprect, 0, 32);
	}
	else if ((priority & 0x7) == 0x3)
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[0]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, 0, 8);

		// This mode uses playfield 3 to shadow sprites & playfield 2 (instead of
		// regular alpha-blending, the destination is inverted).  Not yet implemented.
		m_alpha_tmap_bitmap.fill(0, cliprect);
		m_deco_tilegen[1]->tilemap_1_draw(screen, m_alpha_tmap_bitmap, cliprect, 0, 0); // 32
	}
	else
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[1]->tilemap_1_draw(screen, m_temp_bitmap, cliprect, 0, 8);
		m_deco_tilegen[0]->tilemap_2_draw(screen, m_temp_bitmap, cliprect, 0, 32);
	}

	mix(screen,bitmap,cliprect);

	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


u16 boogwing_state::ioprot_r(offs_t offset)
{
	int const real_address = 0 + (offset * 2);
	int const deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	u8 cs = 0;
	u16 const data = m_deco104->read_data(deco146_addr, cs);
	return data;
}

void boogwing_state::ioprot_w(offs_t offset, u16 data, u16 mem_mask)
{
	int const real_address = 0 + (offset * 2);
	int const deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	u8 cs = 0;
	m_deco104->write_data(deco146_addr, data, mem_mask, cs);
}

void boogwing_state::priority_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_priority);
}


void boogwing_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x20ffff).ram();

	map(0x220000, 0x220001).w(FUNC(boogwing_state::priority_w));
	map(0x220002, 0x22000f).noprw();

	map(0x240000, 0x240001).w(m_spriteram[0], FUNC(buffered_spriteram16_device::write));
	map(0x242000, 0x2427ff).ram().share("spriteram1");
	map(0x244000, 0x244001).w(m_spriteram[1], FUNC(buffered_spriteram16_device::write));
	map(0x246000, 0x2467ff).ram().share("spriteram2");

//  map(0x24e6c0, 0x24e6c1).portr("DSW");
//  map(0x24e138, 0x24e139).portr("SYSTEM");
//  map(0x24e344, 0x24e345).portr("INPUTS");
	map(0x24e000, 0x24efff).rw(FUNC(boogwing_state::ioprot_r), FUNC(boogwing_state::ioprot_w)).share("prot16ram"); // Protection device

	map(0x260000, 0x26000f).w(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_w));
	map(0x264000, 0x265fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x266000, 0x267fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x268000, 0x268fff).ram().share(m_pf_rowscroll[0]);
	map(0x26a000, 0x26afff).ram().share(m_pf_rowscroll[1]);

	map(0x270000, 0x27000f).w(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_w));
	map(0x274000, 0x275fff).ram().w(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_w));
	map(0x276000, 0x277fff).ram().w(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_w));
	map(0x278000, 0x278fff).ram().share(m_pf_rowscroll[2]);
	map(0x27a000, 0x27afff).ram().share(m_pf_rowscroll[3]);

	map(0x280000, 0x28000f).noprw(); // ?
	map(0x282000, 0x282001).noprw(); // Palette setup?
	map(0x282008, 0x282009).w(m_deco_ace, FUNC(deco_ace_device::palette_dma_w));
	map(0x284000, 0x285fff).rw(m_deco_ace, FUNC(deco_ace_device::buffered_palette16_r), FUNC(deco_ace_device::buffered_palette16_w));

	map(0x3c0000, 0x3c004f).rw(m_deco_ace, FUNC(deco_ace_device::ace_r), FUNC(deco_ace_device::ace_w));
}

void boogwing_state::decrypted_opcodes_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().share(m_decrypted_opcodes);
}

void boogwing_state::audio_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x100000, 0x100001).noprw();
	map(0x110000, 0x110001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140000, 0x140000).r(m_deco104, FUNC(deco104_device::soundlatch_r));
	map(0x1f0000, 0x1f1fff).ram();
}


/**********************************************************************************/

static INPUT_PORTS_START( boogwing )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Continue Coin" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, "Normal Coin Credit" )
	PORT_DIPSETTING(      0x0000, "2 Start/1 Continue" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "2" )
	PORT_DIPSETTING(      0x0200, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Coin Slots" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, "Common" )
	PORT_DIPSETTING(      0x0000, "Individual" )
	PORT_DIPNAME( 0x2000, 0x2000, "Stage Reset" ) PORT_DIPLOCATION("SW2:6") // At loss of life
	PORT_DIPSETTING(      0x2000, "Point of Termination" )
	PORT_DIPSETTING(      0x0000, "Beginning of Stage" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7") // Manual shows as OFF and states "Don't Change"
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

/**********************************************************************************/

static const gfx_layout tile_8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	8*16
};

static const gfx_layout tile_16x16_layout_5bpp =
{
	16,16,
	RGN_FRAC(1,3),
	5,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	32*16
};

static const gfx_layout tile_16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	32*16
};


static GFXDECODE_START( gfx_boogwing )
	GFXDECODE_ENTRY( "tiles1",   0, tile_8x8_layout,        0x800, 16 )
	GFXDECODE_ENTRY( "tiles2",   0, tile_16x16_layout_5bpp, 0x100, 16 )
	GFXDECODE_ENTRY( "tiles3",   0, tile_16x16_layout,      0x300, 32 )
GFXDECODE_END

static GFXDECODE_START( gfx_boogwing_spr1 )
	GFXDECODE_ENTRY( "sprites1", 0, tile_16x16_layout,      0x500, 32 )
GFXDECODE_END

static GFXDECODE_START( gfx_boogwing_spr2 )
	GFXDECODE_ENTRY( "sprites2", 0, tile_16x16_layout,      0x700, 16 )
GFXDECODE_END

/**********************************************************************************/

void boogwing_state::machine_reset()
{
	m_priority = 0;
}

void boogwing_state::sound_bankswitch_w(u8 data)
{
	m_oki[1]->set_rom_bank((data & 2) >> 1);
	m_oki[0]->set_rom_bank(data & 1);
}


DECO16IC_BANK_CB_MEMBER(boogwing_state::bank_callback)
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

DECO16IC_BANK_CB_MEMBER(boogwing_state::bank_callback2)
{
	int offset = ((bank >> 4) & 0x7) * 0x1000;
	if ((bank & 0xf) == 0xa)
		offset += 0x800; // strange - transporter level

	return offset;
}

void boogwing_state::boogwing(machine_config &config)
{
	constexpr XTAL MAIN_XTAL = XTAL(28'000'000);
	constexpr XTAL SOUND_XTAL = XTAL(32'220'000);

	// basic machine hardware
	M68000(config, m_maincpu, MAIN_XTAL / 2);   // DE102
	m_maincpu->set_addrmap(AS_PROGRAM, &boogwing_state::main_map);
	m_maincpu->set_addrmap(AS_OPCODES, &boogwing_state::decrypted_opcodes_map);
	m_maincpu->set_vblank_int("screen", FUNC(boogwing_state::irq6_line_hold));

	H6280(config, m_audiocpu, SOUND_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &boogwing_state::audio_map);
	m_audiocpu->add_route(ALL_OUTPUTS, "speaker", 0, 0); // internal sound unused
	m_audiocpu->add_route(ALL_OUTPUTS, "speaker", 0, 1);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MAIN_XTAL / 4, 442, 0, 320, 274, 8, 248); // same as robocop2(cninja.cpp)? verify this from real PCB.
	m_screen->set_screen_update(FUNC(boogwing_state::screen_update));

	GFXDECODE(config, "gfxdecode", m_deco_ace, gfx_boogwing);

	BUFFERED_SPRITERAM16(config, m_spriteram[0]);
	BUFFERED_SPRITERAM16(config, m_spriteram[1]);

	DECO_ACE(config, m_deco_ace, 0);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0);
	m_deco_tilegen[0]->set_pf2_col_bank(0);   // pf2 is non default
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	// no bank1 callback
	m_deco_tilegen[0]->set_bank2_callback(FUNC(boogwing_state::bank_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag("gfxdecode");

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0);
	m_deco_tilegen[1]->set_pf2_col_bank(16);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(boogwing_state::bank_callback2));
	m_deco_tilegen[1]->set_bank2_callback(FUNC(boogwing_state::bank_callback2));
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag("gfxdecode");

	DECO_SPRITE(config, m_sprgen[0], 0, m_deco_ace, gfx_boogwing_spr1);
	DECO_SPRITE(config, m_sprgen[1], 0, m_deco_ace, gfx_boogwing_spr2);

	DECO104PROT(config, m_deco104, 0);
	m_deco104->port_a_cb().set_ioport("INPUTS");
	m_deco104->port_b_cb().set_ioport("SYSTEM");
	m_deco104->port_c_cb().set_ioport("DSW");
	m_deco104->soundlatch_irq_cb().set_inputline(m_audiocpu, 0);
	m_deco104->set_interface_scramble_reverse();
	m_deco104->set_use_magic_read_address_xor(true);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", SOUND_XTAL / 9));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 1); // IRQ2
	ymsnd.port_write_handler().set(FUNC(boogwing_state::sound_bankswitch_w));
	ymsnd.add_route(0, "speaker", 0.32, 0);
	ymsnd.add_route(1, "speaker", 0.32, 1);

	OKIM6295(config, m_oki[0], SOUND_XTAL / 32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.56, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.56, 1);

	OKIM6295(config, m_oki[1], SOUND_XTAL / 16, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.12, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.12, 1);
}

/**********************************************************************************/

ROM_START( boogwing ) // VER 1.5 EUR 92.12.07
	ROM_REGION( 0x100000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_BYTE( "kn_00-2.2b",    0x000000, 0x040000, CRC(e38892b9) SHA1(49b5637965a43e0378e1258c5f0a780926f1f283) )
	ROM_LOAD16_BYTE( "kn_02-2.2e",    0x000001, 0x040000, CRC(8426efef) SHA1(2ea33cbd58b638053d75668a484648dbf67dabb8) )
	ROM_LOAD16_BYTE( "kn_01-2.4b",    0x080000, 0x040000, CRC(3ad4b54c) SHA1(5141001768266995078407851b445378b21453de) )
	ROM_LOAD16_BYTE( "kn_03-2.4e",    0x080001, 0x040000, CRC(10b61f4a) SHA1(41d7f670defbd7dae89afafac9839a9e237814d5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "km06.18p",    0x00000, 0x10000, CRC(3e8bc4e1) SHA1(7e4c357afefa47b8f101727e06485eb9ebae635d) )

	ROM_REGION( 0x20000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "km05.9e",   0x00000, 0x010000, CRC(d10aef95) SHA1(a611a35ab312caee19c31da079c647679d31673d) )
	ROM_LOAD16_BYTE( "km04.8e",   0x00001, 0x010000, CRC(329323a8) SHA1(e2ec7b059301c0a2e052dfc683e044c808ad9b33) )

	ROM_REGION( 0x300000, "tiles2", 0 )
	ROM_LOAD( "mbd-01.9b", 0x000000, 0x100000, CRC(d7de4f4b) SHA1(4747f8795e277ed8106667b6f68e1176d95db684) )
	ROM_LOAD( "mbd-00.8b", 0x100000, 0x100000, CRC(adb20ba9) SHA1(2ffa1dd19a438a4d2f5743b1050a8037183a3e7d) )
	// 0x100000 bytes expanded from mbd-02.10e copied here later

	ROM_REGION( 0x200000, "tiles3", 0 )
	ROM_LOAD( "mbd-03.13b",   0x000000, 0x100000, CRC(cf798f2c) SHA1(f484a22679d6a4d4b0dcac820de3f1a37cbc478f) )
	ROM_LOAD( "mbd-04.14b",   0x100000, 0x100000, CRC(d9764d0b) SHA1(74d6f09d65d073606a6e10556cedf740aa50ff08) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "mbd-05.16b",    0x200000, 0x200000, CRC(1768c66a) SHA1(06bf3bb187c65db9dcce959a43a7231e2ac45c17) )
	ROM_LOAD( "mbd-06.17b",    0x000000, 0x200000, CRC(7750847a) SHA1(358266ed68a9816094e7aab0905d958284c8ce98) )

	ROM_REGION( 0x400000, "sprites2", 0 )
	ROM_LOAD( "mbd-07.18b",    0x200000, 0x200000, CRC(241faac1) SHA1(588be0cf2647c1d185a99c987a5a20ab7ad8dea8) )
	ROM_LOAD( "mbd-08.19b",    0x000000, 0x200000, CRC(f13b1e56) SHA1(f8f5e8c4e6c159f076d4e6505bd901ade5c6a0ca) )

	ROM_REGION( 0x0100000, "tiles2_hi", 0 ) // 1bpp graphics
	ROM_LOAD16_BYTE( "mbd-02.10e",    0x000000, 0x080000, CRC(b25aa721) SHA1(efe800759080bd1dac2da93bd79062a48c5da2b2) )

	ROM_REGION( 0x80000, "oki1", 0 )
	ROM_LOAD( "mbd-10.17p",    0x000000, 0x080000, CRC(f159f76a) SHA1(0b1ea69fecdd151e2b1fa96a21eade492499691d) )

	ROM_REGION( 0x80000, "oki2", 0 )
	ROM_LOAD( "mbd-09.16p",    0x000000, 0x080000, CRC(f44f2f87) SHA1(d941520bdfc9e6d88c45462bc1f697c18f33498e) )

	ROM_REGION( 0x000400, "proms", 0 ) // Priority (not used)
	ROM_LOAD( "kj-00.15n",    0x000000, 0x00400, CRC(add4d50b) SHA1(080e5a8192a146d5141aef5c8d9996ddf8cd3ab4) )
ROM_END

ROM_START( boogwingu ) // VER 1.7 USA 92.12.14
	ROM_REGION( 0x100000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_BYTE( "kl_00.2b",    0x000000, 0x040000, CRC(4dc14798) SHA1(f991edf8e308087ed7222b3b4e3bc959980f8f66) )
	ROM_LOAD16_BYTE( "kl_02.2e",    0x000001, 0x040000, CRC(3bb3b0a0) SHA1(ba892ea52b6bb8d110050efdaa5effd8447c1b2a) )
	ROM_LOAD16_BYTE( "kl_01.4b",    0x080000, 0x040000, CRC(d109ba13) SHA1(93fcda71e260ba94141e2d4d6b248f2cb8530b61) )
	ROM_LOAD16_BYTE( "kl_03.4e",    0x080001, 0x040000, CRC(fef2a176) SHA1(b0505466237fe17b6aaa7eea47e309cd679208d1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "kl06.18p",    0x00000, 0x10000, CRC(3e8bc4e1) SHA1(7e4c357afefa47b8f101727e06485eb9ebae635d) ) // same as other sets but labeled KL

	ROM_REGION( 0x20000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "kl05.9e",   0x00000, 0x010000, CRC(d10aef95) SHA1(a611a35ab312caee19c31da079c647679d31673d) ) // same as other sets but labeled KL
	ROM_LOAD16_BYTE( "kl04.8e",   0x00001, 0x010000, CRC(329323a8) SHA1(e2ec7b059301c0a2e052dfc683e044c808ad9b33) ) // same as other sets but labeled KL

	ROM_REGION( 0x300000, "tiles2", 0 )
	ROM_LOAD( "mbd-01.9b", 0x000000, 0x100000, CRC(d7de4f4b) SHA1(4747f8795e277ed8106667b6f68e1176d95db684) )
	ROM_LOAD( "mbd-00.8b", 0x100000, 0x100000, CRC(adb20ba9) SHA1(2ffa1dd19a438a4d2f5743b1050a8037183a3e7d) )
	// 0x100000 bytes expanded from mbd-02.10e copied here later

	ROM_REGION( 0x200000, "tiles3", 0 )
	ROM_LOAD( "mbd-03.13b",   0x000000, 0x100000, CRC(cf798f2c) SHA1(f484a22679d6a4d4b0dcac820de3f1a37cbc478f) )
	ROM_LOAD( "mbd-04.14b",   0x100000, 0x100000, CRC(d9764d0b) SHA1(74d6f09d65d073606a6e10556cedf740aa50ff08) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "mbd-05.16b",    0x200000, 0x200000, CRC(1768c66a) SHA1(06bf3bb187c65db9dcce959a43a7231e2ac45c17) )
	ROM_LOAD( "mbd-06.17b",    0x000000, 0x200000, CRC(7750847a) SHA1(358266ed68a9816094e7aab0905d958284c8ce98) )

	ROM_REGION( 0x400000, "sprites2", 0 )
	ROM_LOAD( "mbd-07.18b",    0x200000, 0x200000, CRC(241faac1) SHA1(588be0cf2647c1d185a99c987a5a20ab7ad8dea8) )
	ROM_LOAD( "mbd-08.19b",    0x000000, 0x200000, CRC(f13b1e56) SHA1(f8f5e8c4e6c159f076d4e6505bd901ade5c6a0ca) )

	ROM_REGION( 0x0100000, "tiles2_hi", 0 ) // 1bpp graphics
	ROM_LOAD16_BYTE( "mbd-02.10e",    0x000000, 0x080000, CRC(b25aa721) SHA1(efe800759080bd1dac2da93bd79062a48c5da2b2) )

	ROM_REGION( 0x80000, "oki1", 0 )
	ROM_LOAD( "mbd-10.17p",    0x000000, 0x080000, CRC(f159f76a) SHA1(0b1ea69fecdd151e2b1fa96a21eade492499691d) )

	ROM_REGION( 0x80000, "oki2", 0 )
	ROM_LOAD( "mbd-09.16p",    0x000000, 0x080000, CRC(f44f2f87) SHA1(d941520bdfc9e6d88c45462bc1f697c18f33498e) )

	ROM_REGION( 0x000400, "proms", 0 ) // Priority (not used)
	ROM_LOAD( "kj-00.15n",    0x000000, 0x00400, CRC(add4d50b) SHA1(080e5a8192a146d5141aef5c8d9996ddf8cd3ab4) )
ROM_END

ROM_START( boogwinga ) // VER 1.5 ASA 92.12.07
	ROM_REGION( 0x100000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_BYTE( "km_00-2.2b",    0x000000, 0x040000, CRC(71ab71c6) SHA1(00bfd71dd9ae5f12c574ab0ecc07d85898930c4b) )
	ROM_LOAD16_BYTE( "km_02-2.2e",    0x000001, 0x040000, CRC(e90f07f9) SHA1(1e8bd3983ed875f4752cbf2ab1c7e748d3df019c) )
	ROM_LOAD16_BYTE( "km_01-2.4b",    0x080000, 0x040000, CRC(7fdce2d3) SHA1(5ce9b8ac26700f1c3bfb3ce4845f890b81241823) )
	ROM_LOAD16_BYTE( "km_03-2.4e",    0x080001, 0x040000, CRC(0b582de3) SHA1(f5c58c7e0e8a227506a81e38c266356596dcda7b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "km06.18p",    0x00000, 0x10000, CRC(3e8bc4e1) SHA1(7e4c357afefa47b8f101727e06485eb9ebae635d) )

	ROM_REGION( 0x20000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "km05.9e",   0x00000, 0x010000, CRC(d10aef95) SHA1(a611a35ab312caee19c31da079c647679d31673d) )
	ROM_LOAD16_BYTE( "km04.8e",   0x00001, 0x010000, CRC(329323a8) SHA1(e2ec7b059301c0a2e052dfc683e044c808ad9b33) )

	ROM_REGION( 0x300000, "tiles2", 0 )
	ROM_LOAD( "mbd-01.9b", 0x000000, 0x100000, CRC(d7de4f4b) SHA1(4747f8795e277ed8106667b6f68e1176d95db684) )
	ROM_LOAD( "mbd-00.8b", 0x100000, 0x100000, CRC(adb20ba9) SHA1(2ffa1dd19a438a4d2f5743b1050a8037183a3e7d) )
	// 0x100000 bytes expanded from mbd-02.10e copied here later

	ROM_REGION( 0x200000, "tiles3", 0 )
	ROM_LOAD( "mbd-03.13b",   0x000000, 0x100000, CRC(cf798f2c) SHA1(f484a22679d6a4d4b0dcac820de3f1a37cbc478f) )
	ROM_LOAD( "mbd-04.14b",   0x100000, 0x100000, CRC(d9764d0b) SHA1(74d6f09d65d073606a6e10556cedf740aa50ff08) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "mbd-05.16b",    0x200000, 0x200000, CRC(1768c66a) SHA1(06bf3bb187c65db9dcce959a43a7231e2ac45c17) )
	ROM_LOAD( "mbd-06.17b",    0x000000, 0x200000, CRC(7750847a) SHA1(358266ed68a9816094e7aab0905d958284c8ce98) )

	ROM_REGION( 0x400000, "sprites2", 0 )
	ROM_LOAD( "mbd-07.18b",    0x200000, 0x200000, CRC(241faac1) SHA1(588be0cf2647c1d185a99c987a5a20ab7ad8dea8) )
	ROM_LOAD( "mbd-08.19b",    0x000000, 0x200000, CRC(f13b1e56) SHA1(f8f5e8c4e6c159f076d4e6505bd901ade5c6a0ca) )

	ROM_REGION( 0x0100000, "tiles2_hi", 0 ) // 1bpp graphics
	ROM_LOAD16_BYTE( "mbd-02.10e",    0x000000, 0x080000, CRC(b25aa721) SHA1(efe800759080bd1dac2da93bd79062a48c5da2b2) )

	ROM_REGION( 0x80000, "oki1", 0 )
	ROM_LOAD( "mbd-10.17p",    0x000000, 0x080000, CRC(f159f76a) SHA1(0b1ea69fecdd151e2b1fa96a21eade492499691d) )

	ROM_REGION( 0x80000, "oki2", 0 )
	ROM_LOAD( "mbd-09.16p",    0x000000, 0x080000, CRC(f44f2f87) SHA1(d941520bdfc9e6d88c45462bc1f697c18f33498e) )

	ROM_REGION( 0x000400, "proms", 0 ) // Priority (not used)
	ROM_LOAD( "kj-00.15n",    0x000000, 0x00400, CRC(add4d50b) SHA1(080e5a8192a146d5141aef5c8d9996ddf8cd3ab4) )
ROM_END

ROM_START( ragtime ) // VER 1.5 JPN 92.12.07
	ROM_REGION( 0x100000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_BYTE( "kh_00-2.2b",    0x000000, 0x040000, CRC(553e179f) SHA1(ab156d9eca4a74084da944989529fd8f5a147dfc) )
	ROM_LOAD16_BYTE( "kh_02-2.2e",    0x000001, 0x040000, CRC(6c759ec0) SHA1(f503d225c31543a7cd975fc599811a31ff729251) )
	ROM_LOAD16_BYTE( "kh_01-2.4b",    0x080000, 0x040000, CRC(12dfee70) SHA1(a7c8fd118f589ef13bcb43a6aa446ff81015f5b3) )
	ROM_LOAD16_BYTE( "kh_03-2.4e",    0x080001, 0x040000, CRC(076fea18) SHA1(342ca71b6d8c8be92dbf221ada717bdbd0061226) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "km06.18p",    0x00000, 0x10000, CRC(3e8bc4e1) SHA1(7e4c357afefa47b8f101727e06485eb9ebae635d) )

	ROM_REGION( 0x20000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "km05.9e",   0x00000, 0x010000, CRC(d10aef95) SHA1(a611a35ab312caee19c31da079c647679d31673d) )
	ROM_LOAD16_BYTE( "km04.8e",   0x00001, 0x010000, CRC(329323a8) SHA1(e2ec7b059301c0a2e052dfc683e044c808ad9b33) )

	ROM_REGION( 0x300000, "tiles2", 0 )
	ROM_LOAD( "mbd-01.9b", 0x000000, 0x100000, CRC(d7de4f4b) SHA1(4747f8795e277ed8106667b6f68e1176d95db684) )
	ROM_LOAD( "mbd-00.8b", 0x100000, 0x100000, CRC(adb20ba9) SHA1(2ffa1dd19a438a4d2f5743b1050a8037183a3e7d) )
	// 0x100000 bytes expanded from mbd-02.10e copied here later

	ROM_REGION( 0x200000, "tiles3", 0 )
	ROM_LOAD( "mbd-03.13b",   0x000000, 0x100000, CRC(cf798f2c) SHA1(f484a22679d6a4d4b0dcac820de3f1a37cbc478f) )
	ROM_LOAD( "mbd-04.14b",   0x100000, 0x100000, CRC(d9764d0b) SHA1(74d6f09d65d073606a6e10556cedf740aa50ff08) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "mbd-05.16b",    0x200000, 0x200000, CRC(1768c66a) SHA1(06bf3bb187c65db9dcce959a43a7231e2ac45c17) )
	ROM_LOAD( "mbd-06.17b",    0x000000, 0x200000, CRC(7750847a) SHA1(358266ed68a9816094e7aab0905d958284c8ce98) )

	ROM_REGION( 0x400000, "sprites2", 0 )
	ROM_LOAD( "mbd-07.18b",    0x200000, 0x200000, CRC(241faac1) SHA1(588be0cf2647c1d185a99c987a5a20ab7ad8dea8) )
	ROM_LOAD( "mbd-08.19b",    0x000000, 0x200000, CRC(f13b1e56) SHA1(f8f5e8c4e6c159f076d4e6505bd901ade5c6a0ca) )

	ROM_REGION( 0x0100000, "tiles2_hi", 0 ) // 1bpp graphics
	ROM_LOAD16_BYTE( "mbd-02.10e",    0x000000, 0x080000, CRC(b25aa721) SHA1(efe800759080bd1dac2da93bd79062a48c5da2b2) )

	ROM_REGION( 0x80000, "oki1", 0 )
	ROM_LOAD( "mbd-10.17p",    0x000000, 0x080000, CRC(f159f76a) SHA1(0b1ea69fecdd151e2b1fa96a21eade492499691d) )

	ROM_REGION( 0x80000, "oki2", 0 )
	ROM_LOAD( "mbd-09.16p",    0x000000, 0x080000, CRC(f44f2f87) SHA1(d941520bdfc9e6d88c45462bc1f697c18f33498e) )

	ROM_REGION( 0x000400, "proms", 0 ) // Priority (not used)
	ROM_LOAD( "kj-00.15n",    0x000000, 0x00400, CRC(add4d50b) SHA1(080e5a8192a146d5141aef5c8d9996ddf8cd3ab4) )
ROM_END

ROM_START( ragtimea ) // VER 1.3 JPN 92.11.26
	ROM_REGION( 0x100000, "maincpu", 0 ) // DE102 code (encrypted)
	ROM_LOAD16_BYTE( "kh_00-1.2b",    0x000000, 0x040000, CRC(88f0155a) SHA1(6f11cc91e36cd68b7143e3326d92b258f051012e) )
	ROM_LOAD16_BYTE( "kh_02-1.2e",    0x000001, 0x040000, CRC(8811b41b) SHA1(d395338bcd812add0de3d1554d1dc3e048d0e4c9) )
	ROM_LOAD16_BYTE( "kh_01-1.4b",    0x080000, 0x040000, CRC(4dab63ad) SHA1(8c6f6e8382bcbba6e1a7ced504397181e7d6e1d1) )
	ROM_LOAD16_BYTE( "kh_03-1.4e",    0x080001, 0x040000, CRC(8a4cbb18) SHA1(272c8e2b20b0a38ce37552be00130c4117533ea9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "km06.18p",    0x00000, 0x10000, CRC(3e8bc4e1) SHA1(7e4c357afefa47b8f101727e06485eb9ebae635d) )

	ROM_REGION( 0x20000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "km05.9e",   0x00000, 0x010000, CRC(d10aef95) SHA1(a611a35ab312caee19c31da079c647679d31673d) )
	ROM_LOAD16_BYTE( "km04.8e",   0x00001, 0x010000, CRC(329323a8) SHA1(e2ec7b059301c0a2e052dfc683e044c808ad9b33) )

	ROM_REGION( 0x300000, "tiles2", 0 )
	ROM_LOAD( "mbd-01.9b", 0x000000, 0x100000, CRC(d7de4f4b) SHA1(4747f8795e277ed8106667b6f68e1176d95db684) )
	ROM_LOAD( "mbd-00.8b", 0x100000, 0x100000, CRC(adb20ba9) SHA1(2ffa1dd19a438a4d2f5743b1050a8037183a3e7d) )
	// 0x100000 bytes expanded from mbd-02.10e copied here later

	ROM_REGION( 0x200000, "tiles3", 0 )
	ROM_LOAD( "mbd-03.13b",   0x000000, 0x100000, CRC(cf798f2c) SHA1(f484a22679d6a4d4b0dcac820de3f1a37cbc478f) )
	ROM_LOAD( "mbd-04.14b",   0x100000, 0x100000, CRC(d9764d0b) SHA1(74d6f09d65d073606a6e10556cedf740aa50ff08) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "mbd-05.16b",    0x200000, 0x200000, CRC(1768c66a) SHA1(06bf3bb187c65db9dcce959a43a7231e2ac45c17) )
	ROM_LOAD( "mbd-06.17b",    0x000000, 0x200000, CRC(7750847a) SHA1(358266ed68a9816094e7aab0905d958284c8ce98) )

	ROM_REGION( 0x400000, "sprites2", 0 )
	ROM_LOAD( "mbd-07.18b",    0x200000, 0x200000, CRC(241faac1) SHA1(588be0cf2647c1d185a99c987a5a20ab7ad8dea8) )
	ROM_LOAD( "mbd-08.19b",    0x000000, 0x200000, CRC(f13b1e56) SHA1(f8f5e8c4e6c159f076d4e6505bd901ade5c6a0ca) )

	ROM_REGION( 0x0100000, "tiles2_hi", 0 ) // 1bpp graphics
	ROM_LOAD16_BYTE( "mbd-02.10e",    0x000000, 0x080000, CRC(b25aa721) SHA1(efe800759080bd1dac2da93bd79062a48c5da2b2) )

	ROM_REGION( 0x80000, "oki1", 0 )
	ROM_LOAD( "mbd-10.17p",    0x000000, 0x080000, CRC(f159f76a) SHA1(0b1ea69fecdd151e2b1fa96a21eade492499691d) )

	ROM_REGION( 0x80000, "oki2", 0 )
	ROM_LOAD( "mbd-09.16p",    0x000000, 0x080000, CRC(f44f2f87) SHA1(d941520bdfc9e6d88c45462bc1f697c18f33498e) )

	ROM_REGION( 0x000400, "proms", 0 ) // Priority (not used)
	ROM_LOAD( "kj-00.15n",    0x000000, 0x00400, CRC(add4d50b) SHA1(080e5a8192a146d5141aef5c8d9996ddf8cd3ab4) )
ROM_END

void boogwing_state::init_boogwing()
{
	const u8 *src = memregion("tiles2_hi")->base();
	u8 *dst = memregion("tiles2")->base() + 0x200000;

	deco56_decrypt_gfx(machine(), "tiles1");
	deco56_decrypt_gfx(machine(), "tiles2");
	deco56_decrypt_gfx(machine(), "tiles3");
	deco56_remap_gfx(machine(), "tiles2_hi");
	deco102_decrypt_cpu((u16 *)memregion("maincpu")->base(), m_decrypted_opcodes, 0x100000, 0x42ba, 0x00, 0x18);
	memcpy(dst, src, 0x100000);
}

} // anonymous namespace


GAME( 1992, boogwing,  0,        boogwing, boogwing, boogwing_state, init_boogwing, ROT0, "Data East Corporation", "Boogie Wings (Europe v1.5, 92.12.07)",          MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1992, boogwingu, boogwing, boogwing, boogwing, boogwing_state, init_boogwing, ROT0, "Data East Corporation", "Boogie Wings (USA v1.7, 92.12.14)",             MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1992, boogwinga, boogwing, boogwing, boogwing, boogwing_state, init_boogwing, ROT0, "Data East Corporation", "Boogie Wings (Asia v1.5, 92.12.07)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1992, ragtime,   boogwing, boogwing, boogwing, boogwing_state, init_boogwing, ROT0, "Data East Corporation", "The Great Ragtime Show (Japan v1.5, 92.12.07)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1992, ragtimea,  boogwing, boogwing, boogwing, boogwing_state, init_boogwing, ROT0, "Data East Corporation", "The Great Ragtime Show (Japan v1.3, 92.11.26)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
