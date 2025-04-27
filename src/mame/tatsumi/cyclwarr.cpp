// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese
/***************************************************************************

    Cycle Warriors                                      ABA-011
    Big Fight                                           ABA-011

    TODO:

    - Combine Big Fight & CycleWarriors video routines - currently each
      game uses different sized tilemaps - these are probably software
      controlled rather than hardwired, but I don't think either game
      changes the size at runtime.

    reference of bigfight : https://youtu.be/aUUoUCr6yhk


   Cycle Warriors Board Layout

    ABA-011


            6296             CW24A                   5864
                                CW25A                   5864
            YM2151                  50MHz

                                TZ8315                 CW26A
                                                    5864
        TC51821  TC51832                               D780C-1
        TC51821  TC51832
        TC51821  TC51832                     16MHz
        TC51821  TC51832

        CW00A   CW08A
        CW01A   CW09A
        CW02A   CW10A
        CW03A   CW11A            68000-12              81C78
        CW04A   CW12A                                  81C78
        CW05A   CW13A                CW16B  CW18B      65256
        CW06A   CW14A                CW17A  CW19A      65256
        CW07A   CW15A                       CW20A
                                            CW21       65256
                                68000-12   CW22A      65256
                                            CW23

    ABA-012

                            HD6445


                            51832
                            51832
                            51832
                            51832

                            CW28
                            CW29
                            CW30

    CW27

    Big Fight
    Tatsumi, 1992

    PCB Layout
    ----------

    ABA-011
    A-8
    |-----------------------------------------------------------------|
    |     LM324   M6295    ROM15                TC5563                |
    |LM324  VOL   KA51             50MHz        TC5563   PAL       |-||
    |    TC51832 TC51832          |--------|                       | ||
    |    TC51832 TC51832          |TATSUMI |                       | ||
    |    TC51832 TC51832          |TZB315  |           ROM20       | ||
    |    TC51832 TC51832          |        |           TMM2063     | ||
    |     ROM0    ROM8            |--------|           Z80B        | ||
    |                                                              | ||
    |J                     PAL                  16MHz              |-||
    |A    ROM2    ROM10     |--------------|                 PAL      |
    |M                      |     68000    |            TMM2088       |
    |M                      |--------------|                          |
    |A    ROM4    ROM12                                 TMM2088       |
    |                                ROM16    ROM17                |-||
    |                            PAL      PAL           TC51832    | ||
    |     ROM6    ROM14                       ROM18                | ||
    |                            EPL204   PAL           TC51832    | ||
    |                       |--------------|                       | ||
    |   CXD1095Q  CXD1095Q  |    68000     |  ROM19     TC51832    | ||
    |                       |--------------|                       | ||
    |                                                   TC51832    |-||
    |      DSW3(4) DSW2(8) DSW1(8)                                    |
    |-----------------------------------------------------------------|
    Z80 clock - 4.000MHz [16/4]
    68k clocks - 12.500MHz [50/4]
    M6295 clock - 2.000MHz [16/8]. Sample rate = 2000000/132
    YM2151 clock - 4.000MHz [16/4]

    |-------------------------|
    |       D65005(x16)       |
    |ROM21                 |-||
    |                      | ||
    |                      | ||
    |                      | ||
    |                      | ||
    |                      | ||
    |                      | ||
    |PAL                   |-||
    |     ROM24       PAL  PAL|
    |        ROM23     HD6445 |
    |           ROM22         |
    |          TC51832(x4)    |
    |      PAL             |-||
    |      PAL             | ||
    |                      | ||
    |                      | ||
    |                      | ||
    |                      | ||
    |                      | ||
    |                      |-||
    |PAL                      |
    |-------------------------|


***************************************************************************/

#include "emu.h"
#include "tatsumi.h"

#include "cpu/z80/z80.h"

#include "screen.h"
#include "speaker.h"

namespace {

class cyclwarr_state : public tatsumi_state
{
public:
	cyclwarr_state(const machine_config &mconfig, device_type type, const char *tag)
		: tatsumi_state(mconfig, type, tag)
		, m_soundlatch(*this, "soundlatch")
		, m_master_ram(*this, "master_ram")
		, m_slave_ram(*this, "slave_ram")
		, m_cyclwarr_videoram(*this, "cw_videoram%u", 0U)
		, m_cyclwarr_tileclut(*this, "cw_tileclut")
	{
	}

	void cyclwarr(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void tile_expand();

	template<int Bank> TILE_GET_INFO_MEMBER(get_tile_info_bigfight);

	tilemap_t *m_layer[4]{};
	uint16_t m_layer_page_size[4]{};
	bool m_layer1_can_be_road = false;

private:

	uint16_t cyclwarr_sprite_r(offs_t offset);
	void cyclwarr_sprite_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void video_config_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bigfight_a40000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void mixing_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cyclwarr_control_w(uint8_t data);
	void cyclwarr_sound_w(uint8_t data);
	void output_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t oki_status_xor_r();

	template<int Bank> uint16_t cyclwarr_videoram_r(offs_t offset);
	template<int Bank> void cyclwarr_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	template<int Bank> TILE_GET_INFO_MEMBER(get_tile_info_cyclwarr_road);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void common_map(address_map &map) ATTR_COLD;
	void master_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_master_ram;
	required_shared_ptr<uint16_t> m_slave_ram;
	required_shared_ptr_array<uint16_t, 2> m_cyclwarr_videoram;
	required_region_ptr<uint8_t> m_cyclwarr_tileclut;

	std::vector<uint8_t> m_mask;

	uint16_t m_video_config[4]{};
	uint16_t m_mixing_control = 0;
	uint16_t m_bigfight_a40000[2]{};
	uint16_t m_bigfight_bank = 0;
	uint16_t m_bigfight_last_bank = 0;
	uint16_t m_road_color_bank = 0, m_prev_road_bank = 0;
	std::unique_ptr<uint8_t[]> m_decoded_gfx;

	void draw_bg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *src, const uint16_t* scrollx, const uint16_t* scrolly, const uint16_t layer_page_size, bool is_road, int hi_priority);
	void draw_bg_layers(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int hi_priority);
	void apply_highlight_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &highlight_bitmap);
};

class bigfight_state : public cyclwarr_state
{
public:
	bigfight_state(const machine_config &mconfig, device_type type, const char *tag)
		: cyclwarr_state(mconfig, type, tag)
	{
	}

	void bigfight(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
};
/*
 * these video registers never changes
 *
 * Big Fight
 * 72f2 5af2 3af2 22fa
 *
 * Cycle Warriors
 * 5673 92c2 3673 267b
 *
 * Following is complete guesswork (since nothing changes it's very hard to pinpoint what these bits do :/)
 * Layer order is 3-1-2-0 ?
 * x--- -x-- ---- ---- one of these might be enable page select
 * ---- ---- x--- ---- tilemap size
 * ---x ---- ---- x--- one these might be color bank
 *
 */
void cyclwarr_state::video_config_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_config[offset]);
}

// mixing control (seems to be available only for Big Fight and Cycle Warriors)
// --x- ---- enabled in Big Fight, disabled in Cycle Warriors (unknown purpose)
// ---- -x-- enable shadow mixing
// ---- ---x if 1 invert shadows, i.e. shadows are drawn with original pen while non shadows are halved (Chen stage in Big Fight)
void cyclwarr_state::mixing_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mixing_control);
}

template<int Bank>
TILE_GET_INFO_MEMBER(cyclwarr_state::get_tile_info_bigfight)
{
	int tile = m_cyclwarr_videoram[Bank >> 1][tile_index&0x7fff];
	int bank = (m_bigfight_a40000[0] >> (((tile&0xc00)>>10)*4))&0xf;
	uint16_t tileno = (tile&0x3ff)|(bank<<10);
	// color is bits 12-13
	uint8_t color = (tile >> 12) & 0x3;

	// all layers but 0 wants this palette bank (fade in/out effects)
	// a similar result is obtainable with priority bit, but then it's wrong for
	// Big Fight CRT test (dark red background) and character name bio in attract mode (reference shows it doesn't fade in like rest of text)
	// TODO: likely an HW config sets this up
	if(Bank != 0)
		color |= 4;
	// bit 14: ignore transparency on this tile
	int opaque = ((tile >> 14) & 1) == 1;

	tileinfo.set(0, tileno, color, opaque ? TILE_FORCE_LAYER0 : 0);

	// bit 15: tile appears in front of sprites
	tileinfo.category = (tile >> 15) & 1;
	tileinfo.mask_data = &m_mask[tileno<<3];
}

// same as above but additionally apply per-scanline color banking
// TODO: split for simplicity, need to merge with above
template<int Bank>
TILE_GET_INFO_MEMBER(cyclwarr_state::get_tile_info_cyclwarr_road)
{
	int tile = m_cyclwarr_videoram[Bank >> 1][tile_index&0x7fff];
	int bank = (m_bigfight_a40000[0] >> (((tile&0xc00)>>10)*4))&0xf;
	uint16_t tileno = (tile&0x3ff)|(bank<<10);
	uint8_t color = (tile >> 12) & 0x3;
//  if(Bank != 0)
	color |= 4;
	int opaque = ((tile >> 14) & 1) == 1;

	tileinfo.set(0, tileno, color | m_road_color_bank, opaque ? TILE_FORCE_LAYER0 : 0);

	tileinfo.category = (tile >> 15) & 1;
	tileinfo.mask_data = &m_mask[((tile&0x3ff)|(bank<<10))<<3];
}

void cyclwarr_state::tile_expand()
{
	/*
	    Each tile (0x4000 of them) has a lookup table in ROM to build an individual 3-bit palette
	    from sets of 8 bit palettes!
	*/
	gfx_element *gx0 = m_gfxdecode->gfx(0);
	m_mask.resize(gx0->elements() << 3,0);
	uint8_t *dest;

	// allocate memory for the assembled data
	m_decoded_gfx = std::make_unique<uint8_t[]>(gx0->elements() * gx0->width() * gx0->height());

	// loop over elements
	dest = m_decoded_gfx.get();
	for (int c = 0; c < gx0->elements(); c++)
	{
		const uint8_t *c0base = gx0->get_data(c);

		// loop over height
		for (int y = 0; y < gx0->height(); y++)
		{
			const uint8_t *c0 = c0base;

			for (int x = 0; x < gx0->width(); x++)
			{
				uint8_t pix = (*c0++ & 7);
				uint8_t respix = m_cyclwarr_tileclut[(c << 3)|pix];
				*dest++ = respix;
				// Transparent pixels are set by both the tile pixel data==0 AND colour palette & 7 == 0
				m_mask[(c << 3) | (y & 7)] |= ((pix&0x7)!=0 || ((pix&0x7)==0 && (respix&0x7)!=0)) ? (0x80 >> (x & 7)) : 0;
			}
			c0base += gx0->rowbytes();
		}
	}

	gx0->set_raw_layout(m_decoded_gfx.get(), gx0->width(), gx0->height(), gx0->elements(), 8 * gx0->width(), 8 * gx0->width() * gx0->height());
	gx0->set_granularity(256);
}

void cyclwarr_state::video_start()
{
	tile_expand();
	m_layer[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<0>)),      TILEMAP_SCAN_ROWS, 8,8,  64,512);
	m_layer[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_cyclwarr_road<1>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);
	m_layer[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<2>)),      TILEMAP_SCAN_ROWS, 8,8,  64,512);
	m_layer[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<3>)),      TILEMAP_SCAN_ROWS, 8,8,  64,512);

	// set up scroll bases
	// TODO: more HW configs
	m_layer[3]->set_scrolldx(-8,-8);
	m_layer_page_size[3] = 0x200;
	m_layer[2]->set_scrolldx(-8,-8);
	m_layer_page_size[2] = 0x200;
	m_layer[1]->set_scrolldx(-8,-8);
	m_layer_page_size[1] = 0x200;
	m_layer[0]->set_scrolldx(-0x10,-0x10);
	m_layer_page_size[0] = 0x100;

	m_layer1_can_be_road = true;
}

// TODO: it's same video HW, we don't know how/where video registers are mapped
void bigfight_state::video_start()
{
	tile_expand();
	m_layer[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bigfight_state::get_tile_info_bigfight<0>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);
	m_layer[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bigfight_state::get_tile_info_bigfight<1>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);
	m_layer[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bigfight_state::get_tile_info_bigfight<2>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);
	m_layer[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bigfight_state::get_tile_info_bigfight<3>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);

	// set up scroll bases
	// TODO: more HW configs
	m_layer[3]->set_scrolldx(-8,-8);
	m_layer[2]->set_scrolldx(-8,-8);
	m_layer[1]->set_scrolldx(-8,-8);
	m_layer[0]->set_scrolldx(-0x10,-0x10);
	for(int i=0;i<4;i++)
		m_layer_page_size[i] = 0x200;

	m_layer1_can_be_road = false;
}


void cyclwarr_state::draw_bg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *src, const uint16_t* scrollx, const uint16_t* scrolly, const uint16_t layer_page_size, bool is_road, int hi_priority)
{
	rectangle clip;
	clip.min_x = cliprect.min_x;
	clip.max_x = cliprect.max_x;
	// TODO: both always enabled when this occurs
	bool rowscroll_enable = (scrollx[0] & 0x1000) == 0;
	bool colscroll_enable = (scrollx[0] & 0x2000) == 0;
	// this controls wraparound (tilemap can't go above a threshold)
	// TODO: Actually scrolly registers 0xf0 to 0xff are used (can split the tilemap furthermore?)
	uint16_t page_select = scrolly[0xff];

	for (int y=cliprect.min_y; y<=cliprect.max_y; y++)
	{
		clip.min_y = clip.max_y = y;
		int y_base = rowscroll_enable ? y : 0;
		int x_base = colscroll_enable ? y : 0;
		int src_y = (scrolly[y_base] & 0x7ff);
		int src_x = (scrollx[x_base] & 0x7ff);
		// apparently if this is on disables wraparound target
		int page_disable = scrolly[y_base] & 0x800;
		int cur_page = src_y + y;

		// special handling for cycle warriors road: it reads in scrolly table bits 15-13 an
		// additional tile color bank and per scanline.
		if(is_road == true)
		{
			if(scrolly[y_base] & 0x8000)
			{
				m_road_color_bank = (scrolly[y_base] >> 13) & 3;
				// road mode disables page wraparound
				page_disable = 1;
			}
			else
				m_road_color_bank = 0;

			if(m_road_color_bank != m_prev_road_bank)
			{
				m_prev_road_bank = m_road_color_bank;
				src->mark_all_dirty();
			}
		}

		// apply wraparound, if enabled tilemaps can't go above a certain threshold
		// cfr. Cycle Warriors scrolling text (ranking, ending), backgrounds when uphill,
		// Big Fight vertical scrolling in the morning Funnel stage (not the one chosen at start),
		// also Big Fight text garbage in the stage after Mevella joins you (forgot the name)
		if((cur_page - page_select) >= layer_page_size && page_disable == 0)
			src_y -= layer_page_size;

		src->set_scrollx(0,src_x);
		src->set_scrolly(0,src_y);
		src->draw(screen, bitmap, clip, TILEMAP_DRAW_CATEGORY(hi_priority), 0);
	}
}

void cyclwarr_state::draw_bg_layers(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int hi_priority)
{
	draw_bg(screen, bitmap, cliprect, m_layer[3], &m_cyclwarr_videoram[1][0x000], &m_cyclwarr_videoram[1][0x100], m_layer_page_size[3], false, hi_priority);
	draw_bg(screen, bitmap, cliprect, m_layer[2], &m_cyclwarr_videoram[1][0x200], &m_cyclwarr_videoram[1][0x300], m_layer_page_size[2],false, hi_priority);
	draw_bg(screen, bitmap, cliprect, m_layer[1], &m_cyclwarr_videoram[0][0x000], &m_cyclwarr_videoram[0][0x100], m_layer_page_size[1],m_layer1_can_be_road, hi_priority);
	draw_bg(screen, bitmap, cliprect, m_layer[0], &m_cyclwarr_videoram[0][0x200], &m_cyclwarr_videoram[0][0x300], m_layer_page_size[0], false, hi_priority);
}

uint32_t cyclwarr_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bigfight_bank=m_bigfight_a40000[0];
	if (m_bigfight_bank!=m_bigfight_last_bank)
	{
		for (int i = 0; i < 4; i++)
		{
			m_layer[i]->mark_all_dirty();
		}
		m_bigfight_last_bank=m_bigfight_bank;
	}
	m_sprites->update_cluts();

	bitmap.fill(m_palette->pen(0), cliprect);

#if 0
	popmessage("%04x %04x (%04x)|%04x %04x (%04x)|%04x %04x (%04x)|%04x %04x (%04x)"
														,m_cyclwarr_videoram[1][0x000],m_cyclwarr_videoram[1][0x100],m_cyclwarr_videoram[1][0x1ff]
														,m_cyclwarr_videoram[1][0x200],m_cyclwarr_videoram[1][0x300],m_cyclwarr_videoram[1][0x3ff]
														,m_cyclwarr_videoram[0][0x000],m_cyclwarr_videoram[0][0x100],m_cyclwarr_videoram[0][0x1ff]
														,m_cyclwarr_videoram[0][0x200],m_cyclwarr_videoram[0][0x300],m_cyclwarr_videoram[0][0x3ff]);
#endif

//  popmessage("%04x %04x %04x %04x",m_video_config[0],m_video_config[1],m_video_config[2],m_video_config[3]);

	screen.priority().fill(0, cliprect);
	m_sprites->draw_sprites(screen.priority(),cliprect,1,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0); // Alpha pass only
	draw_bg_layers(screen, bitmap, cliprect, 0);
	apply_shadow_bitmap(bitmap,cliprect,screen.priority(), m_mixing_control & 1);
	m_sprites->draw_sprites(bitmap,cliprect,0,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0);
	draw_bg_layers(screen, bitmap, cliprect, 1);
	return 0;
}

void cyclwarr_state::cyclwarr_control_w(uint8_t data)
{
	m_control_word = data;

//  if ((m_control_word&0xfe) != (m_last_control&0xfe))
//      logerror("%s  control_w %04x\n", m_maincpu->pc(), data);

/*

0x1 - watchdog
0x4 - cpu bus lock



*/

	if ((m_control_word & 4) == 4 && (m_last_control & 4) == 0)
	{
//      logerror("68k 2 halt\n");
		m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}

	if ((m_control_word & 4) == 0 && (m_last_control & 4) == 4)
	{
//      logerror("68k 2 irq go\n");
		m_subcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	}

	m_last_control = m_control_word;
}


uint8_t cyclwarr_state::oki_status_xor_r()
{
	int r = m_oki->read();

	// Cycle Warriors and Big Fight access this with reversed activeness.
	// this is particularly noticeable with the "We got em" sample played in CW at stage clear:
	// gets cut too early with the old hack below.
	// fwiw returning normal oki status doesn't work at all, both games don't make any sound.
	// TODO: verify with HW
	return (r ^ 0xff);
#if 0
	// old hack left for reference

	if (m_audiocpu->pc()==0x2b70 || m_audiocpu->pc()==0x2bb5
		|| m_audiocpu->pc()==0x2acc
		|| m_audiocpu->pc()==0x1c79 // BigFight
		|| m_audiocpu->pc()==0x1cbe // BigFight
		|| m_audiocpu->pc()==0xf9881)
		return 0xf;
	if (m_audiocpu->pc()==0x2ba3 || m_audiocpu->pc()==0x2a9b || m_audiocpu->pc()==0x2adc
		|| m_audiocpu->pc()==0x1cac) // BigFight
		return 0;
	return r;
#endif
}


void cyclwarr_state::cyclwarr_sound_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


uint16_t cyclwarr_state::cyclwarr_sprite_r(offs_t offset)
{
	return m_spriteram[offset];
}

void cyclwarr_state::cyclwarr_sprite_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_spriteram[offset]);
}

void cyclwarr_state::bigfight_a40000_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bigfight_a40000[offset]);
}

template<int Bank>
uint16_t cyclwarr_state::cyclwarr_videoram_r(offs_t offset)
{
	return m_cyclwarr_videoram[Bank][offset];
}

template<int Bank>
void cyclwarr_state::cyclwarr_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_cyclwarr_videoram[Bank][offset]);
	m_layer[(Bank<<1)|0]->mark_tile_dirty(offset);
	m_layer[(Bank<<1)|1]->mark_tile_dirty(offset);
}

void cyclwarr_state::output_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
	if(data & 0xfffc)
		logerror("output_w = %04x & %04x\n",data,mem_mask);
}


void cyclwarr_state::common_map(address_map &map)
{
	map(0x080000, 0x08ffff).rw(FUNC(cyclwarr_state::cyclwarr_videoram_r<1>), FUNC(cyclwarr_state::cyclwarr_videoram_w<1>)).share("cw_videoram1");
	map(0x090000, 0x09ffff).rw(FUNC(cyclwarr_state::cyclwarr_videoram_r<0>), FUNC(cyclwarr_state::cyclwarr_videoram_w<0>)).share("cw_videoram0");

	map(0x0a2000, 0x0a2007).w(FUNC(cyclwarr_state::video_config_w));
	map(0x0a4000, 0x0a4001).w(FUNC(cyclwarr_state::bigfight_a40000_w));
	map(0x0a6000, 0x0a6001).w(FUNC(cyclwarr_state::mixing_control_w));
	map(0x0ac000, 0x0ac003).w(FUNC(cyclwarr_state::hd6445_crt_w)).umask16(0x00ff);

	map(0x0b8000, 0x0b8001).w(FUNC(cyclwarr_state::cyclwarr_sound_w)).umask16(0xff00);
	map(0x0b9000, 0x0b900f).rw("io1", FUNC(cxd1095_device::read), FUNC(cxd1095_device::write)).umask16(0x00ff).cswidth(16);
	map(0x0ba000, 0x0ba00f).rw("io2", FUNC(cxd1095_device::read), FUNC(cxd1095_device::write)).umask16(0x00ff).cswidth(16);
	map(0x0bc000, 0x0bc001).w(FUNC(cyclwarr_state::output_w));

	map(0x0c0000, 0x0c3fff).rw(FUNC(cyclwarr_state::cyclwarr_sprite_r), FUNC(cyclwarr_state::cyclwarr_sprite_w)).share("spriteram");
	map(0x0ca000, 0x0ca1ff).rw(FUNC(cyclwarr_state::tatsumi_sprite_control_r), FUNC(cyclwarr_state::tatsumi_sprite_control_w)).share("obj_ctrl_ram");
	map(0x0d0000, 0x0d3fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	// games accesses these ranges differently, we do mirroring in rom loading to make them match.
	// address bit A19 controls if access routes to upper or lower roms
	// TODO: it's unknown what Big Fight is supposed to return for the lower roms, let's assume mirror for the time being.
	// slave ROMs
	// 0x140000 - 0x1bffff tested in Cycle Warriors
	// 0x100000 - 0x17ffff tested in Big Fight
	map(0x100000, 0x1fffff).rom().region("slave_rom",0);
	// same as above but A20 instead of A19
	// master ROMs
	// 0x2c0000 - 0x33ffff tested in Cycle Warriors
	// 0x200000 - 0x27ffff tested in Big Fight
	map(0x200000, 0x3fffff).rom().region("master_rom",0);
}

void cyclwarr_state::master_map(address_map &map)
{
	map(0x000000, 0x00ffff).ram().share("master_ram");
	map(0x03e000, 0x03efff).ram(); // RAM_A
	map(0x040000, 0x04ffff).ram().share("slave_ram");
	common_map(map);
}

void cyclwarr_state::slave_map(address_map &map)
{
	map(0x000000, 0x00ffff).ram().share("slave_ram");
	common_map(map);
}

void cyclwarr_state::sound_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xffef).ram();
	map(0xfff0, 0xfff1).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xfff4, 0xfff4).r(FUNC(cyclwarr_state::oki_status_xor_r)).w(m_oki, FUNC(okim6295_device::write));
	map(0xfffc, 0xfffc).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xfffe, 0xfffe).nopw();
}


static INPUT_PORTS_START( cyclwarr )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW3")
	PORT_DIPNAME( 0x1, 0x1, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(   0x1, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )
	PORT_DIPNAME( 0x2, 0x0, "Player Select" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(   0x2, "Coin Slot" )
	PORT_DIPSETTING(   0x0, "Select SW" )
	PORT_DIPNAME( 0x4, 0x4, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(   0x4, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )
	PORT_DIPNAME( 0x8, 0x8, "Hardware Test Mode" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(   0x8, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
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
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPNAME( 0x04, 0x04, "Ticket Dispenser" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x00, "15000" )
	PORT_DIPNAME( 0x18, 0x08, "Machine Type" ) PORT_DIPLOCATION("SW2:4,5")
//  PORT_DIPSETTING(    0x00, "2 Players" ) // same as 4 players but text layout is 2p (invalid setting)
	PORT_DIPSETTING(    0x08, "2 Players" )
	PORT_DIPSETTING(    0x10, "3 Players" )
	PORT_DIPSETTING(    0x18, "4 Players" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(3)

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(4)
INPUT_PORTS_END

static INPUT_PORTS_START( cyclwarb )
	PORT_INCLUDE(cyclwarr)

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x8, 0x8, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(   0x8, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bigfight )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW3")
	PORT_DIPNAME( 0x1, 0x1, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(   0x1, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )
	PORT_DIPNAME( 0x2, 0x0, "Player Select" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(   0x2, "Coin Slot" )
	PORT_DIPSETTING(   0x0, "Select SW" )
	PORT_DIPNAME( 0x4, 0x4, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(   0x4, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )
	PORT_DIPNAME( 0x8, 0x8, "Hardware Test Mode" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(   0x8, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
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
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Ticket Dispenser" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "100000" )
	PORT_DIPSETTING(    0x00, "150000" )
	PORT_DIPNAME( 0x08, 0x08, "Continue Coin" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Extend" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "100000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(3)

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(4)
INPUT_PORTS_END

static GFXDECODE_START( gfx_cyclwarr )
	GFXDECODE_ENTRY( "tilerom", 0, gfx_8x8x3_planar, 0,  16)
GFXDECODE_END


void cyclwarr_state::machine_reset()
{
	uint16_t *src;

	// transfer data from rom to initial vector table
	src = (uint16_t *)memregion("master_rom")->base();

	for(int i=0;i<0x100/2;i++)
		m_master_ram[i] = src[i];

	src = (uint16_t *)memregion("slave_rom")->base();

	for(int i=0;i<0x100/2;i++)
		m_slave_ram[i] = src[i];

	// reset CPUs again so that above will be notified.
	// TODO: better way?
	m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	m_subcpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

	m_last_control = 0;
	m_control_word = 0;

	m_road_color_bank = m_prev_road_bank = 0;
}

void cyclwarr_state::cyclwarr(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, cyclwarr_state::CLOCK_2 / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &cyclwarr_state::master_map);
	m_maincpu->set_vblank_int("screen", FUNC(cyclwarr_state::irq5_line_hold));

	M68000(config, m_subcpu, cyclwarr_state::CLOCK_2 / 4);
	m_subcpu->set_addrmap(AS_PROGRAM, &cyclwarr_state::slave_map);
	m_subcpu->set_vblank_int("screen", FUNC(cyclwarr_state::irq5_line_hold));

	Z80(config, m_audiocpu, cyclwarr_state::CLOCK_1 / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &cyclwarr_state::sound_map);

	// saner sync value (avoids crashing after crediting)
	config.set_maximum_quantum(attotime::from_hz(cyclwarr_state::CLOCK_2 / 1024));

	cxd1095_device &io1(CXD1095(config, "io1"));
	io1.in_portb_cb().set_ioport("SERVICE");
	io1.in_portc_cb().set_ioport("P1");
	io1.in_portd_cb().set_ioport("P2");
	io1.in_porte_cb().set_ioport("DSW3");

	cxd1095_device &io2(CXD1095(config, "io2"));
	io2.in_porta_cb().set_ioport("DSW1");
	io2.in_portb_cb().set_ioport("DSW2");
	io2.in_portc_cb().set_ioport("P3");
	io2.in_portd_cb().set_ioport("P4");
	io2.out_porte_cb().set(FUNC(cyclwarr_state::cyclwarr_control_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(cyclwarr_state::CLOCK_2 / 8, 400, 0, 320, 272, 0, 240); // TODO: Hook up CRTC
	screen.set_screen_update(FUNC(cyclwarr_state::screen_update));

	TZB315_SPRITES(config, m_sprites, 0, 0x1000);
	m_sprites->set_sprite_palette_base(4096);
	m_sprites->set_palette("sprites:palette_clut");
	m_sprites->set_basepalette(m_palette);
	m_sprites->set_spriteram(m_spriteram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cyclwarr);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 8192);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
//  m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	YM2151(config, m_ym2151, cyclwarr_state::CLOCK_1 / 4);
	m_ym2151->irq_handler().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);
	m_ym2151->add_route(0, "speaker", 0.45, 0);
	m_ym2151->add_route(1, "speaker", 0.45, 1);

	OKIM6295(config, m_oki, cyclwarr_state::CLOCK_1 / 8, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.75, 0);
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.75, 1);
}

void bigfight_state::bigfight(machine_config &config)
{
	cyclwarr(config);

	/* sound hardware */
	// TODO: 2MHz was too fast. Can the clock be software controlled?
	m_oki->set_clock(bigfight_state::CLOCK_1 / 8 / 2);
}


ROM_START( cyclwarr )
	ROM_REGION16_BE( 0x200000, "master_rom", 0 ) /* 68000 main cpu */
	ROM_LOAD16_BYTE( "cw16c.ic77", 0x000000, 0x20000, CRC(4d88892b) SHA1(dc85231a3c4f83118922c13615381f185bcee832) )
	ROM_LOAD16_BYTE( "cw18c.ic98", 0x000001, 0x20000, CRC(4ff56209) SHA1(d628dc3fdc3e9de568ba8dbabf8e13a62e20a215) )
	ROM_COPY("master_rom",         0x000000, 0x040000, 0x040000 )
	ROM_COPY("master_rom",         0x000000, 0x080000, 0x040000 )
	ROM_COPY("master_rom",         0x000000, 0x0c0000, 0x040000 )
	ROM_LOAD16_BYTE( "cw17b.ic78", 0x100000, 0x20000, CRC(da998afc) SHA1(dd9377ce079df5c66bdb29dfd333428cce817656) )
	ROM_LOAD16_BYTE( "cw19b.ic99", 0x100001, 0x20000, CRC(c15a8413) SHA1(647b2a994a4912b5d7dc71b875f5d08c14412c6a) )
	ROM_COPY("master_rom",         0x100000, 0x140000, 0x040000 )
	ROM_COPY("master_rom",         0x100000, 0x180000, 0x040000 )
	ROM_COPY("master_rom",         0x100000, 0x1c0000, 0x040000 )

	ROM_REGION16_BE( 0x100000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "cw20b.ic100", 0x000000, 0x20000, CRC(4d75292a) SHA1(71d59c1d03b323d4021209a7f0506b4a855a73af) )
	ROM_LOAD16_BYTE( "cw22b.ic102", 0x000001, 0x20000, CRC(0aec0ba4) SHA1(d559e54d303afac4a981c4a933a05278044ac068) )
	ROM_COPY("slave_rom",           0x000000, 0x040000, 0x040000 )
	ROM_LOAD16_BYTE( "cw21.ic101",  0x080000, 0x20000, CRC(ed90d956) SHA1(f533f93da31ac6eb631fb506357717e7cac8e186) )
	ROM_LOAD16_BYTE( "cw23.ic103",  0x080001, 0x20000, CRC(009cdc78) SHA1(a77933a7736546397e8c69226703d6f9be7b55e5) )
	ROM_COPY("slave_rom",           0x080000, 0x0c0000, 0x040000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "cw26a.ic91", 0x000000, 0x10000, CRC(f7a70e3a) SHA1(5581633bf1f15d7f5c1e03de897d65d60f9f1e33) )

	ROM_REGION( 0x100000, "sprites:sprites_l", 0)
	ROM_LOAD32_BYTE( "cw00a.ic26", 0x000000, 0x20000, CRC(058a77f1) SHA1(93f99fcf6ce6714d76af6f6e930115516f0379d3) )
	ROM_LOAD32_BYTE( "cw08a.ic45", 0x000001, 0x20000, CRC(f53993e7) SHA1(ef2d502ab180d2bc0bdb698c2878fdee9a2c33a8) )
	ROM_LOAD32_BYTE( "cw02a.ic28", 0x000002, 0x20000, CRC(4dadf3cb) SHA1(e42c56e295a443cb605d48eba23a16fab3c86525) )
	ROM_LOAD32_BYTE( "cw10a.ic47", 0x000003, 0x20000, CRC(3b7cd251) SHA1(52b9637404fa193421294dfb52c1a7bba0d94c9b) )
	ROM_LOAD32_BYTE( "cw01a.ic27", 0x080000, 0x20000, CRC(7c639948) SHA1(d58ff5735cd3179ffafead385a625baa7962e1d0) )
	ROM_LOAD32_BYTE( "cw09a.ic46", 0x080001, 0x20000, CRC(4ba24af5) SHA1(9203c2639e04aaa09996339f11259750ff8129b9) )
	ROM_LOAD32_BYTE( "cw03a.ic29", 0x080002, 0x20000, CRC(3ca6f98e) SHA1(8526fe38d3b4c66e09049ba18651a9e7255d85d6) )
	ROM_LOAD32_BYTE( "cw11a.ic48", 0x080003, 0x20000, CRC(5d760392) SHA1(7bbda2880af4659c267193ce10ed887a1b54a981) )

	ROM_REGION( 0x100000, "sprites:sprites_h", 0)
	ROM_LOAD32_BYTE( "cw04a.ic30", 0x000000, 0x20000, CRC(f05f594d) SHA1(80effaa517b2154c013419e0bc05fd0797b74c8d) )
	ROM_LOAD32_BYTE( "cw12a.ic49", 0x000001, 0x20000, CRC(4ac07e8b) SHA1(f9de96fba39d5752d61b8f6be87fb605694624ed) )
	ROM_LOAD32_BYTE( "cw06a.ic32", 0x000002, 0x20000, CRC(f628edc9) SHA1(473f7ec28000e6bf72782c1c3f4afb5e021bd430) )
	ROM_LOAD32_BYTE( "cw14a.ic51", 0x000003, 0x20000, CRC(a9131f5f) SHA1(3a2059946984733e6939f3298f0db676e6a3301b) )
	ROM_LOAD32_BYTE( "cw05a.ic31", 0x080000, 0x20000, CRC(c8f5faa9) SHA1(f374531ffd645597eeb1440fd2cadb426fcd3d79) )
	ROM_LOAD32_BYTE( "cw13a.ic50", 0x080001, 0x20000, CRC(8091d381) SHA1(7faf068ce20b2877559f0335df55d61be13146b4) )
	ROM_LOAD32_BYTE( "cw07a.ic33", 0x080002, 0x20000, CRC(314579b5) SHA1(3c10ec490f7821a5b5412295232bbb104d0e4b83) )
	ROM_LOAD32_BYTE( "cw15a.ic52", 0x080003, 0x20000, CRC(7ed4b721) SHA1(b87865effeff77a9ea74354ef2b5911a5102a647) )

	ROM_REGION( 0x20000, "cw_tileclut", 0 )
	ROM_LOAD( "cw27.ic128", 0x000000, 0x20000, CRC(2db48a9e) SHA1(16c307340d17cd3b5455ebcee681fbe0335dec58) )

	ROM_REGION( 0x60000, "tilerom", 0 )
	ROM_LOAD( "cw30.ic73", 0x000000, 0x20000, CRC(331d0711) SHA1(82251fe1f1d36f079080943ab1fd04a60077c353) )
	ROM_LOAD( "cw29.ic72", 0x020000, 0x20000, CRC(64dd519c) SHA1(e23611fc2be896861997063546c3eb03527eaf8e) )
	ROM_LOAD( "cw28.ic71", 0x040000, 0x20000, CRC(3fc568ed) SHA1(91125c9deddc659449ca6791a847fe908c2818b2) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "cw24a.ic39", 0x000000, 0x20000, CRC(22600cba) SHA1(a1514fbe037942f1493a17eb0b7986949470cb22) )
	ROM_LOAD( "cw25a.ic40", 0x020000, 0x20000, CRC(372c6bc8) SHA1(d4875bf3bffecf338bebba3b8d6a791585556a06) )
ROM_END

ROM_START( cyclwarra )
	ROM_REGION16_BE( 0x200000, "master_rom", 0 ) /* 68000 main cpu */
	ROM_LOAD16_BYTE( "cw16b.ic77", 0x000000, 0x20000, CRC(cb1a737a) SHA1(a603ee1256be5641d00a72f64efaaacb65ed9d7d) )
	ROM_LOAD16_BYTE( "cw18b.ic98", 0x000001, 0x20000, CRC(0633ddcb) SHA1(1196ab17065352ec5b37f2f6b383a43a2d0fa3a6) )
	ROM_COPY("master_rom",         0x000000, 0x040000, 0x040000 )
	ROM_COPY("master_rom",         0x000000, 0x080000, 0x040000 )
	ROM_COPY("master_rom",         0x000000, 0x0c0000, 0x040000 )
	ROM_LOAD16_BYTE( "cw17a.ic78", 0x100000, 0x20000, CRC(2ad6f836) SHA1(5fa4275b433013943ba1d1b64a3c725097f946f9) )
	ROM_LOAD16_BYTE( "cw19a.ic99", 0x100001, 0x20000, CRC(d3853658) SHA1(c9338083a04f55bd22285176831f4b0bdb78564f) )
	ROM_COPY("master_rom",         0x100000, 0x140000, 0x040000 )
	ROM_COPY("master_rom",         0x100000, 0x180000, 0x040000 )
	ROM_COPY("master_rom",         0x100000, 0x1c0000, 0x040000 )

	ROM_REGION16_BE( 0x100000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "cw20a.ic100", 0x000000, 0x20000, CRC(c3578ac1) SHA1(21d369da874f01922d0f0b757a42b4321df891d4) )
	ROM_LOAD16_BYTE( "cw22a.ic102", 0x000001, 0x20000, CRC(5339ed24) SHA1(5b0a54c2442dcf7373ff8b55b91af9772473ff77) )
	ROM_COPY("slave_rom",           0x000000, 0x040000, 0x040000 )
	ROM_LOAD16_BYTE( "cw21.ic101",  0x080000, 0x20000, CRC(ed90d956) SHA1(f533f93da31ac6eb631fb506357717e7cac8e186) )
	ROM_LOAD16_BYTE( "cw23.ic103",  0x080001, 0x20000, CRC(009cdc78) SHA1(a77933a7736546397e8c69226703d6f9be7b55e5) )
	ROM_COPY("slave_rom",           0x080000, 0x0c0000, 0x040000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "cw26a.ic91", 0x000000, 0x10000, CRC(f7a70e3a) SHA1(5581633bf1f15d7f5c1e03de897d65d60f9f1e33) )

	ROM_REGION( 0x100000, "sprites:sprites_l", 0)
	ROM_LOAD32_BYTE( "cw00a.ic26", 0x000000, 0x20000, CRC(058a77f1) SHA1(93f99fcf6ce6714d76af6f6e930115516f0379d3) )
	ROM_LOAD32_BYTE( "cw08a.ic45", 0x000001, 0x20000, CRC(f53993e7) SHA1(ef2d502ab180d2bc0bdb698c2878fdee9a2c33a8) )
	ROM_LOAD32_BYTE( "cw02a.ic28", 0x000002, 0x20000, CRC(4dadf3cb) SHA1(e42c56e295a443cb605d48eba23a16fab3c86525) )
	ROM_LOAD32_BYTE( "cw10a.ic47", 0x000003, 0x20000, CRC(3b7cd251) SHA1(52b9637404fa193421294dfb52c1a7bba0d94c9b) )
	ROM_LOAD32_BYTE( "cw01a.ic27", 0x080000, 0x20000, CRC(7c639948) SHA1(d58ff5735cd3179ffafead385a625baa7962e1d0) )
	ROM_LOAD32_BYTE( "cw09a.ic46", 0x080001, 0x20000, CRC(4ba24af5) SHA1(9203c2639e04aaa09996339f11259750ff8129b9) )
	ROM_LOAD32_BYTE( "cw03a.ic29", 0x080002, 0x20000, CRC(3ca6f98e) SHA1(8526fe38d3b4c66e09049ba18651a9e7255d85d6) )
	ROM_LOAD32_BYTE( "cw11a.ic48", 0x080003, 0x20000, CRC(5d760392) SHA1(7bbda2880af4659c267193ce10ed887a1b54a981) )

	ROM_REGION( 0x100000, "sprites:sprites_h", 0)
	ROM_LOAD32_BYTE( "cw04a.ic30", 0x000000, 0x20000, CRC(f05f594d) SHA1(80effaa517b2154c013419e0bc05fd0797b74c8d) )
	ROM_LOAD32_BYTE( "cw12a.ic49", 0x000001, 0x20000, CRC(4ac07e8b) SHA1(f9de96fba39d5752d61b8f6be87fb605694624ed) )
	ROM_LOAD32_BYTE( "cw06a.ic32", 0x000002, 0x20000, CRC(f628edc9) SHA1(473f7ec28000e6bf72782c1c3f4afb5e021bd430) )
	ROM_LOAD32_BYTE( "cw14a.ic51", 0x000003, 0x20000, CRC(a9131f5f) SHA1(3a2059946984733e6939f3298f0db676e6a3301b) )
	ROM_LOAD32_BYTE( "cw05a.ic31", 0x080000, 0x20000, CRC(c8f5faa9) SHA1(f374531ffd645597eeb1440fd2cadb426fcd3d79) )
	ROM_LOAD32_BYTE( "cw13a.ic50", 0x080001, 0x20000, CRC(8091d381) SHA1(7faf068ce20b2877559f0335df55d61be13146b4) )
	ROM_LOAD32_BYTE( "cw07a.ic33", 0x080002, 0x20000, CRC(314579b5) SHA1(3c10ec490f7821a5b5412295232bbb104d0e4b83) )
	ROM_LOAD32_BYTE( "cw15a.ic52", 0x080003, 0x20000, CRC(7ed4b721) SHA1(b87865effeff77a9ea74354ef2b5911a5102a647) )

	ROM_REGION( 0x20000, "cw_tileclut", 0 )
	ROM_LOAD( "cw27.ic128", 0x000000, 0x20000, CRC(2db48a9e) SHA1(16c307340d17cd3b5455ebcee681fbe0335dec58) )

	ROM_REGION( 0x60000, "tilerom", 0 )
	ROM_LOAD( "cw30.ic73", 0x000000, 0x20000, CRC(331d0711) SHA1(82251fe1f1d36f079080943ab1fd04a60077c353) )
	ROM_LOAD( "cw29.ic72", 0x020000, 0x20000, CRC(64dd519c) SHA1(e23611fc2be896861997063546c3eb03527eaf8e) )
	ROM_LOAD( "cw28.ic71", 0x040000, 0x20000, CRC(3fc568ed) SHA1(91125c9deddc659449ca6791a847fe908c2818b2) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "cw24a.ic39", 0x000000, 0x20000, CRC(22600cba) SHA1(a1514fbe037942f1493a17eb0b7986949470cb22) )
	ROM_LOAD( "cw25a.ic40", 0x020000, 0x20000, CRC(372c6bc8) SHA1(d4875bf3bffecf338bebba3b8d6a791585556a06) )
ROM_END

ROM_START( cyclwarrb )
	ROM_REGION16_BE( 0x200000, "master_rom", 0 ) /* 68000 main cpu */
	ROM_LOAD16_BYTE( "cw16.ic77", 0x000000, 0x20000, CRC(47d57cf9) SHA1(9954f3eae496d3e3552f5537d93d798fa8a397b6) )
	ROM_LOAD16_BYTE( "cw18.ic98", 0x000001, 0x20000, CRC(7d541f9e) SHA1(eab9098f08c103d6b96cb0aebe65f53a9cb361fb) )
	ROM_COPY("master_rom",        0x000000, 0x040000, 0x040000 )
	ROM_COPY("master_rom",        0x000000, 0x080000, 0x040000 )
	ROM_COPY("master_rom",        0x000000, 0x0c0000, 0x040000 )
	ROM_LOAD16_BYTE( "cw17.ic78", 0x100000, 0x20000, CRC(008bdf09) SHA1(17f739a65382caf81314840ca491f600e09c3f32) )
	ROM_LOAD16_BYTE( "cw19.ic99", 0x100001, 0x20000, CRC(e82244e0) SHA1(8887927fe74c160bc3b5c1293e0787bd9c9d2bff) )
	ROM_COPY("master_rom",        0x100000, 0x140000, 0x040000 )
	ROM_COPY("master_rom",        0x100000, 0x180000, 0x040000 )
	ROM_COPY("master_rom",        0x100000, 0x1c0000, 0x040000 )

	ROM_REGION16_BE( 0x100000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "cw20.ic100", 0x000000, 0x20000, CRC(c7a6fa85) SHA1(d696c8b9432c07abad3c4ab611d53742970c1fbc) )
	ROM_LOAD16_BYTE( "cw22.ic102", 0x000001, 0x20000, CRC(917c1a2a) SHA1(612d81b8cf68c61206e85926b95238ebcdc22ca3) )
	ROM_COPY("slave_rom",          0x000000, 0x040000, 0x040000 )
	ROM_LOAD16_BYTE( "cw21.ic101", 0x080000, 0x20000, CRC(ed90d956) SHA1(f533f93da31ac6eb631fb506357717e7cac8e186) )
	ROM_LOAD16_BYTE( "cw23.ic103", 0x080001, 0x20000, CRC(009cdc78) SHA1(a77933a7736546397e8c69226703d6f9be7b55e5) )
	ROM_COPY("slave_rom",          0x080000, 0x0c0000, 0x040000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "cw26.ic91", 0x000000, 0x10000, CRC(a6485a3a) SHA1(b4fcf541efe48b3ca32065221fe2f59476a4f96a) )

	ROM_REGION( 0x100000, "sprites:sprites_l", 0)
	ROM_LOAD32_BYTE( "cw00.ic26", 0x000000, 0x20000, CRC(ba00c582) SHA1(2cd645b828595acbe62e2f7aad037fcbdc5a543f) )
	ROM_LOAD32_BYTE( "cw08.ic45", 0x000001, 0x20000, CRC(1583e576) SHA1(646762d1d181231090a18698378f60d09f26f49f) )
	ROM_LOAD32_BYTE( "cw02.ic28", 0x000002, 0x20000, CRC(8376a744) SHA1(633d20199382f760adfb528f5b13730ddf9016e3) )
	ROM_LOAD32_BYTE( "cw10.ic47", 0x000003, 0x20000, CRC(901d849d) SHA1(601b5315717bc45b9ca3b64af019fd4437c65186) )
	ROM_LOAD32_BYTE( "cw01.ic27", 0x080000, 0x20000, CRC(35979022) SHA1(6de5c2a5edc5d76899d329f26bfa7b8b7c4f1919) )
	ROM_LOAD32_BYTE( "cw09.ic46", 0x080001, 0x20000, CRC(8114be09) SHA1(bade99653f8b4e3b5974f94d0cf0fdf2464d7dc7) )
	ROM_LOAD32_BYTE( "cw03.ic29", 0x080002, 0x20000, CRC(951ed812) SHA1(b3db6b467fd626936568773367099c9abcabfab6) )
	ROM_LOAD32_BYTE( "cw11.ic48", 0x080003, 0x20000, CRC(a7e5bf0b) SHA1(883b943d40f4516a21692beffb12514ad9301f20) )

	ROM_REGION( 0x100000, "sprites:sprites_h", 0)
	ROM_LOAD32_BYTE( "cw04.ic30", 0x000000, 0x20000, CRC(890ea7b1) SHA1(737e58800aa6863aff043ba46c9cebc8ba6c1501) )
	ROM_LOAD32_BYTE( "cw12.ic49", 0x000001, 0x20000, CRC(1587e96d) SHA1(2ffcb27d90ef29bc79d0a29f46a1d43565935a15) )
	ROM_LOAD32_BYTE( "cw06.ic32", 0x000002, 0x20000, CRC(47decb23) SHA1(4868c01035175698cb8af7aae80627b51583213f) )
	ROM_LOAD32_BYTE( "cw14.ic51", 0x000003, 0x20000, CRC(a75072a1) SHA1(a988eda496f35204bfdade8aa24441dba440618c) )
	ROM_LOAD32_BYTE( "cw05.ic31", 0x080000, 0x20000, CRC(4e49fcc5) SHA1(e4541961bd2abfb91b76ce78fa705d5dd188e118) )
	ROM_LOAD32_BYTE( "cw13.ic50", 0x080001, 0x20000, CRC(51aee710) SHA1(1d9dc575d0110bd147439c5dd87fe6b4203d125d) )
	ROM_LOAD32_BYTE( "cw07.ic33", 0x080002, 0x20000, CRC(4f6b3c72) SHA1(98ab85f2848f0a0a5f37bf2d6292ad3a039040e1) )
	ROM_LOAD32_BYTE( "cw15.ic52", 0x080003, 0x20000, CRC(9cfc3b14) SHA1(33abb0df0fc1e12e22d35a68c583d2c0a236032e) )

	ROM_REGION( 0x20000, "cw_tileclut", 0 )
	ROM_LOAD( "cw27.ic128", 0x000000, 0x20000, CRC(2db48a9e) SHA1(16c307340d17cd3b5455ebcee681fbe0335dec58) )

	ROM_REGION( 0x60000, "tilerom", 0 )
	ROM_LOAD( "cw30.ic73", 0x000000, 0x20000, CRC(331d0711) SHA1(82251fe1f1d36f079080943ab1fd04a60077c353) )
	ROM_LOAD( "cw29.ic72", 0x020000, 0x20000, CRC(64dd519c) SHA1(e23611fc2be896861997063546c3eb03527eaf8e) )
	ROM_LOAD( "cw28.ic71", 0x040000, 0x20000, CRC(3fc568ed) SHA1(91125c9deddc659449ca6791a847fe908c2818b2) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "cw24.ic39", 0x000000, 0x20000, CRC(08656756) SHA1(37352ce488c8af36a50c51fa319caed4f2391d72) )
	ROM_LOAD( "cw25.ic40", 0x020000, 0x20000, CRC(36c0b8a6) SHA1(d1519c919fa51b1fc157d5314709a3e6e0b7a5c8) )
ROM_END

ROM_START( bigfight )
	ROM_REGION16_BE( 0x200000, "master_rom", 0 ) /* 68000 main cpu */
	ROM_LOAD16_BYTE( "rom16.ic77",   0x000000, 0x40000, CRC(e7304ec8) SHA1(31a37e96bf963b349d36534bc5ebbf45e19ad00e) )
	ROM_LOAD16_BYTE( "rom17.ic98",   0x000001, 0x40000, CRC(4cf090f6) SHA1(9ae0274c890e829a90108ce316aff9665128c982) )
	ROM_COPY("master_rom",       0x000000, 0x080000, 0x080000 )
	ROM_COPY("master_rom",       0x100000, 0x080000, 0x080000 )
	ROM_COPY("master_rom",       0x180000, 0x080000, 0x080000 )

	ROM_REGION16_BE( 0x100000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "rom18.ic100",   0x000000, 0x40000, CRC(49df6207) SHA1(c4126f4542add11a3a3d236311c8787c24c98440) )
	ROM_LOAD16_BYTE( "rom19.ic102",   0x000001, 0x40000, CRC(c12aa9e9) SHA1(19cc7feaa97c6f5148ae8c0077174f96be684f05) )
	ROM_COPY("slave_rom",       0x000000, 0x080000, 0x080000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "rom20.ic91",   0x000000, 0x10000, CRC(b3add091) SHA1(8a67bfff75c13fe4d9b89d30449199200d11cea7) ) // == bf36b.ic91

	ROM_REGION( 0x200000, "sprites:sprites_l", 0 )
	ROM_LOAD32_BYTE( "rom0.ic26",   0x000000, 0x80000, CRC(a4a3c8d6) SHA1(b5365d9bc6068260c23ba9d5971c7c7d7cc07a97) )
	ROM_LOAD32_BYTE( "rom8.ic45",   0x000001, 0x80000, CRC(220956ed) SHA1(68e0ba1e850101b4cc2778819dfa76f04d88d2d6) )
	ROM_LOAD32_BYTE( "rom2.ic28",   0x000002, 0x80000, CRC(c4f6d243) SHA1(e23b241b5a40b332165a34e2f1bc4366973b2070) )
	ROM_LOAD32_BYTE( "rom10.ic47",  0x000003, 0x80000, CRC(0212d472) SHA1(5549461195fd7b6b43c0174462d7fe1a1bac24e9) )

	ROM_REGION( 0x200000, "sprites:sprites_h", 0 )
	ROM_LOAD32_BYTE( "rom4.ic30",   0x000000, 0x80000, CRC(999ff7e9) SHA1(a53b06ad084722d7a52fcf01c52967f68620e609) )
	ROM_LOAD32_BYTE( "rom12.ic49",  0x000001, 0x80000, CRC(cb4c1f0b) SHA1(32d64b78ed3d5971eb5d25be2c38e6f2c9048f74) )
	ROM_LOAD32_BYTE( "rom6.ic32",   0x000002, 0x80000, CRC(f70e2d47) SHA1(00517b5f3b2deb6f3f3bd12df421e63884c22b2e) )
	ROM_LOAD32_BYTE( "rom14.ic51",  0x000003, 0x80000, CRC(77430bc9) SHA1(0b1fd54ace84a9fb5b44d5600de8089a20bcbd47) )

	ROM_REGION( 0x20000, "cw_tileclut", 0 )
	ROM_LOAD( "rom21.ic128",   0x000000, 0x20000, CRC(da027dcf) SHA1(47d18a8a273fea72cb3ad3d58166fe38ca28a860) ) // == bf27.ic128

	ROM_REGION( 0x60000, "tilerom", 0 )
	ROM_LOAD( "rom24.ic73",   0x000000, 0x20000, CRC(c564185d) SHA1(e9b5fc10a5a5014735852c22db2a054d5787d8cb) ) // == bf30.ic73
	ROM_LOAD( "rom23.ic72",   0x020000, 0x20000, CRC(f8bb340b) SHA1(905a1ec778d6ed5c6f53d9d08cd105eed7e307ca) ) // == bf29.ic72
	ROM_LOAD( "rom22.ic71",   0x040000, 0x20000, CRC(fb505074) SHA1(b6d9b20be7c3e971e5a4392736f087e807b9c850) ) // == bf28.ic71

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "rom15.ic39",   0x000000, 0x40000, CRC(58d136e8) SHA1(4aa063c4b9b057cba4655ecbe44a87c8c411e3aa) ) // == bf24.ic39 + bf25.ic40
ROM_END

ROM_START( bigfightj ) // ABA-011 main board + ABA-012 daughter board
	ROM_REGION16_BE( 0x200000, "master_rom", 0 ) /* 68000 main cpu */
	ROM_LOAD16_BYTE( "bfj16f.ic77", 0x000000, 0x20000, CRC(9141a488) SHA1(f8b64d2ef6fcea7922f88ed75977f764e98e679b) ) // rev F
	ROM_LOAD16_BYTE( "bf18f.ic98",  0x000001, 0x20000, CRC(f23a4935) SHA1(513f2b3a83a0a7c183b2ff20b652279a5bee8863) ) // rev F
	ROM_COPY("master_rom",          0x000000, 0x040000, 0x040000 )
	ROM_COPY("master_rom",          0x000000, 0x080000, 0x040000 )
	ROM_COPY("master_rom",          0x000000, 0x0c0000, 0x040000 )
	ROM_LOAD16_BYTE( "bf17e.ic78",  0x100000, 0x20000, CRC(5e5d023d) SHA1(04f59458f15c95ad152b1b99f885f31ccb26ac40) ) // rev E
	ROM_LOAD16_BYTE( "bf19e.ic99",  0x100001, 0x20000, CRC(5329e151) SHA1(a7ce98d80379f56808291c42852b1f7173966ed7) ) // rev E
	ROM_COPY("master_rom",          0x100000, 0x140000, 0x040000 )
	ROM_COPY("master_rom",          0x100000, 0x180000, 0x040000 )
	ROM_COPY("master_rom",          0x100000, 0x1c0000, 0x040000 )

	ROM_REGION16_BE( 0x100000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "bf20.ic100",  0x000000, 0x20000, CRC(5bd44e11) SHA1(a7acb6d9f40c4b6d54bf131a2e192c16ec22b1af) )
	ROM_LOAD16_BYTE( "bf22.ic102",  0x000001, 0x20000, CRC(e52c29ab) SHA1(2b3aa55a2eeec5cf73666e1d85f65679961472e0) )
	ROM_COPY("slave_rom",           0x000000, 0x040000, 0x040000 )
	ROM_LOAD16_BYTE( "bf21d.ic101", 0x080000, 0x20000, CRC(c4c2f969) SHA1(69f453d51951c02f12f3a40ac925b48430e3f314) ) // rev D
	ROM_LOAD16_BYTE( "bf23d.ic103", 0x080001, 0x20000, CRC(c05ae1fe) SHA1(219ddc08d4b8416fb2a9f2cb14ac9c4b4421dadd) ) // rev D
	ROM_COPY("slave_rom",           0x080000, 0x0c0000, 0x040000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "bf36b.ic91", 0x000000, 0x10000, CRC(b3add091) SHA1(8a67bfff75c13fe4d9b89d30449199200d11cea7) ) // rev B

	ROM_REGION( 0x200000, "sprites:sprites_l", 0 )
	ROM_LOAD32_BYTE( "bf00d.ic26", 0x000000, 0x40000, CRC(f506d508) SHA1(86255631ac139f1b5c0f5d6e54a0858625497a1e) ) // all rev D
	ROM_LOAD32_BYTE( "bf08d.ic45", 0x000001, 0x40000, CRC(4bf948b9) SHA1(c65b95c454d04c7e4a0cf426ac2d45ddc5e8885e) )
	ROM_LOAD32_BYTE( "bf02d.ic28", 0x000002, 0x40000, CRC(af30acf7) SHA1(8f4778b33abb18b113d5d27c0e957d633557c988) )
	ROM_LOAD32_BYTE( "bf10d.ic47", 0x000003, 0x40000, CRC(ffddb7f8) SHA1(505c082adaf71bd2aa15d78d3e8968cbd1f2cc0c) )
	ROM_LOAD32_BYTE( "bf01d.ic27", 0x100000, 0x20000, CRC(ec3f5f17) SHA1(436f176155af8fc3212507448df6852a76289bef) )
	ROM_LOAD32_BYTE( "bf09d.ic46", 0x100001, 0x20000, CRC(284837ed) SHA1(b1c130b45ff0f22985962240c47b7b01df6ac636) )
	ROM_LOAD32_BYTE( "bf03d.ic29", 0x100002, 0x20000, CRC(2ba0398e) SHA1(ec9a29b661b18980c07a446afc89becb1ebddd57) )
	ROM_LOAD32_BYTE( "bf11d.ic48", 0x100003, 0x20000, CRC(3f2fa72f) SHA1(4b4821b6933ea753e092f11d80bcc7698f85ccf2) )
	ROM_COPY("sprites:sprites_l",          0x100000, 0x180000, 0x080000 )

	ROM_REGION( 0x200000, "sprites:sprites_h", 0)
	ROM_LOAD32_BYTE( "bf04d.ic30", 0x000000, 0x40000, CRC(6203d320) SHA1(d58225d8a362971a0eb63c94abc1e8c76198fd2a) ) // all rev D
	ROM_LOAD32_BYTE( "bf12d.ic49", 0x000001, 0x40000, CRC(d261dfa7) SHA1(e787901112780e9770300999722fc80aa1d7ab18) )
	ROM_LOAD32_BYTE( "bf06d.ic32", 0x000002, 0x40000, CRC(be187c3c) SHA1(46383eb40c0caeb1bc636630a4d849aa2d1a12d2) )
	ROM_LOAD32_BYTE( "bf14d.ic51", 0x000003, 0x40000, CRC(60f2ab3d) SHA1(babaef6348133b5fe34f9e044732467d8775cc3d) )
	ROM_LOAD32_BYTE( "bf05d.ic31", 0x100000, 0x20000, CRC(2229acbc) SHA1(3031fbb7b730a6d51f08d0021c5d6e91cdbdd56d) )
	ROM_LOAD32_BYTE( "bf13d.ic50", 0x100001, 0x20000, CRC(1e46cd79) SHA1(c81c96b287a6cc91d3ab4dd8043153814560be3d) )
	ROM_LOAD32_BYTE( "bf07d.ic33", 0x100002, 0x20000, CRC(4940b0bb) SHA1(762f21055921093349ca09c35ef516bde6330aa8) )
	ROM_LOAD32_BYTE( "bf15d.ic52", 0x100003, 0x20000, CRC(dab0c80a) SHA1(a172937c9599acbd77dcac02ea7e43f576d66d8c) )
	ROM_COPY("sprites:sprites_h",          0x100000, 0x180000, 0x080000 )

	ROM_REGION( 0x20000, "cw_tileclut", 0 )
	ROM_LOAD( "bf27.ic128", 0x000000, 0x20000, CRC(da027dcf) SHA1(47d18a8a273fea72cb3ad3d58166fe38ca28a860) )

	ROM_REGION( 0x60000, "tilerom", 0 )
	ROM_LOAD( "bf30.ic73", 0x000000, 0x20000, CRC(c564185d) SHA1(e9b5fc10a5a5014735852c22db2a054d5787d8cb) )
	ROM_LOAD( "bf29.ic72", 0x020000, 0x20000, CRC(f8bb340b) SHA1(905a1ec778d6ed5c6f53d9d08cd105eed7e307ca) )
	ROM_LOAD( "bf28.ic71", 0x040000, 0x20000, CRC(fb505074) SHA1(b6d9b20be7c3e971e5a4392736f087e807b9c850) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "bf24.ic39", 0x000000, 0x20000, CRC(9db80c8a) SHA1(9ce64713758ebab559a0cacc7f7501e5a1a0133a) )
	ROM_LOAD( "bf25.ic40", 0x020000, 0x20000, CRC(630154c4) SHA1(05902371b62a11c13f2582faa591945c037c6311) )
ROM_END

} // anonymous namespace

GAME( 1991, cyclwarr,  0,        cyclwarr,  cyclwarr, cyclwarr_state, init_tatsumi,  ROT0, "Tatsumi", "Cycle Warriors (rev C)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // Rev C & B CPU code
GAME( 1991, cyclwarra, cyclwarr, cyclwarr,  cyclwarb, cyclwarr_state, init_tatsumi,  ROT0, "Tatsumi", "Cycle Warriors (rev B)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // Rev B & A CPU code
GAME( 1991, cyclwarrb, cyclwarr, cyclwarr,  cyclwarb, cyclwarr_state, init_tatsumi,  ROT0, "Tatsumi", "Cycle Warriors", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // Original version with no Rev roms

GAME( 1992, bigfight,  0,        bigfight,  bigfight, bigfight_state, init_tatsumi,  ROT0, "Tatsumi", "Big Fight - Big Trouble In The Atlantic Ocean", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, bigfightj, bigfight, bigfight,  bigfight, bigfight_state, init_tatsumi,  ROT0, "Tatsumi", "Big Fight - Big Trouble In The Atlantic Ocean (Japan, rev F)", MACHINE_IMPERFECT_GRAPHICS ) // Rev D through F CPU codes
