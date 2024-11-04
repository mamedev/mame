// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca, Angelo Salese
/*

 Lucky Girl (newer 1991 version on different hardware?)
  -- there is an early 'Lucky Girl' which appears to be running on Nichibutsu like hardware.

 The program rom extracted from the Z180 also refers to this as Lucky 74..

 TODO:
 - sound (what's the sound chip?)
 - are the colours correct even on the text layer? they look odd in places, and there are unused bits

 Lucky Girl
 Wing 1991

 PCB labels

 "DREW'S UNDER WARRANTY IF REMOVED N3357"
 "No.A 120307 LICENSE WING SEAL"
 "EAGLE No.A 120307"

 ROMs

 1C - EAGLE 1       27C010 equiv. mask ROM
 1E - EAGLE 2       27C010 equiv. mask ROM
 1H - EAGLE 3       27C010 equiv. mask ROM
 1L - FALCON 4      27C010 equiv. mask ROM
 1N - FALCON 5      27C010 equiv. mask ROM
 1P - FALCON 6      27C010 equiv. mask ROM
 1T - FALCON 13     27C010 EPROM

 * ROMs 1-6 may need redumping, they could be half size.

 RAMs

 2x LH5186D-01L     28-pin PDIP     Video RAM and work RAM (battery backed)
 2x CY7C128A-25PC   24-pin PDIP     Probably color RAM

 Customs

 2x 06B53P          28-pin PDIP     Unknown
 1x 06B30P          40-pin PDIP     Unknown
 1x 101810P         64-pin SDIP     Unknown
 1x HG62E11B10P     64-pin SDIP     Hitachi gate array (custom)
 1x CPU module      90-pin SDIP

 Others

 1x HD6845SP        40-pin PDIP     CRTC
 1x MB3771           8-pin PDIP     Reset generator
 1x Oki M62X428     18-pin PDIP     Real-time clock

 CPU module

 Text on label:

 [2] 9015 1994.12
     LUCKY GIRL DREW'S

 Text underneath label

 WE 300B 1H2

 Looks like Hitachi part markings to me.

 Other notes

 Has some optocouplers, high voltage drivers, and what looks like additional
 I/O connectors.

 Reset switch cuts power supply going to Video/Work RAM.


*/


#include "emu.h"
#include "cpu/z180/hd647180x.h"
#include "machine/msm6242.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "luckgrln.lh"
#include "7smash.lh"


namespace {


class luckgrln_state : public driver_device
{
public:
	luckgrln_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_reel_ram(*this, "reel_ram.%u", 0U),
		m_reel_attr(*this, "reel_attr.%u", 0U),
		m_reel_scroll(*this, "reel_scroll.%u", 0U),
		m_luck_vram(*this, "luck_vram.%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_palette_ram(*this, "palette", 0x1000, ENDIANNESS_LITTLE),
		m_lamps(*this, "lamp%u", 0U) { }

	void init_luckgrln();

	void _7smash(machine_config &config);
	void luckgrln(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr_array<uint8_t, 4> m_reel_ram;
	required_shared_ptr_array<uint8_t, 4> m_reel_attr;
	required_shared_ptr_array<uint8_t, 4> m_reel_scroll;
	required_shared_ptr_array<uint8_t, 3> m_luck_vram;

	required_device<hd647180x_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	memory_share_creator<uint8_t> m_palette_ram;
	output_finder<12> m_lamps;

	uint8_t m_nmi_enable;
	tilemap_t *m_reel_tilemap[4];
	uint16_t m_palette_index;

	template<uint8_t Reel> void reel_ram_w(offs_t offset, uint8_t data);
	template<uint8_t Reel> void reel_attr_w(offs_t offset, uint8_t data);
	void output_w(uint8_t data);
	void palette_offset_low_w(uint8_t data);
	void palette_offset_high_w(uint8_t data);
	void palette_w(uint8_t data);
	void lamps_a_w(uint8_t data);
	void lamps_b_w(uint8_t data);
	void counters_w(uint8_t data);
	uint8_t test_r();
	template<uint8_t Reel> TILE_GET_INFO_MEMBER(get_reel_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(irq);

	void _7smash_io(address_map &map) ATTR_COLD;
	void _7smash_map(address_map &map) ATTR_COLD;
	void common_portmap(address_map &map) ATTR_COLD;
	void luckgrln_io(address_map &map) ATTR_COLD;
	void mainmap(address_map &map) ATTR_COLD;
};


void luckgrln_state::machine_start()
{
	m_lamps.resolve();

	save_item(NAME(m_nmi_enable));
}

template<uint8_t Reel>
void luckgrln_state::reel_ram_w(offs_t offset, uint8_t data)
{
	m_reel_ram[Reel][offset] = data;
	m_reel_tilemap[Reel]->mark_tile_dirty(offset);
}

template<uint8_t Reel>
void luckgrln_state::reel_attr_w(offs_t offset, uint8_t data)
{
	m_reel_attr[Reel][offset] = data;
	m_reel_tilemap[Reel]->mark_tile_dirty(offset);
}

template<uint8_t Reel>
TILE_GET_INFO_MEMBER(luckgrln_state::get_reel_tile_info)
{
	int code = m_reel_ram[Reel][tile_index];
	int attr = m_reel_attr[Reel][tile_index];
	int col = (attr & 0x1f);

	code |= (attr & 0xe0)<<3;


	tileinfo.set(1,
			code,
			col,
			0);
}


void luckgrln_state::video_start()
{
	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(luckgrln_state::get_reel_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(luckgrln_state::get_reel_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(luckgrln_state::get_reel_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(luckgrln_state::get_reel_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	for (uint8_t i = 0; i < 4; i++)
	{
		m_reel_tilemap[i]->set_scroll_cols(64);
		m_reel_tilemap[i]->set_transparent_pen(0);
	}

	save_item(NAME(m_palette_index));
}

uint32_t luckgrln_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int count = 0;
	const rectangle &visarea = screen.visible_area();

	rectangle clip = visarea;

	bitmap.fill(rgb_t::black(), cliprect);

	for (int i = 0; i < 64; i++)
	{
		m_reel_tilemap[0]->set_scrolly(i, m_reel_scroll[0][i]);
		m_reel_tilemap[1]->set_scrolly(i, m_reel_scroll[1][i]);
		m_reel_tilemap[2]->set_scrolly(i, m_reel_scroll[2][i]);
		m_reel_tilemap[3]->set_scrolly(i, m_reel_scroll[3][i]);
	}


	for (int y = 0; y < 32; y++)
	{
		clip.min_y = y*8;
		clip.max_y = y*8+8;

		if (clip.min_y<visarea.min_y) clip.min_y = visarea.min_y;
		if (clip.max_y>visarea.max_y) clip.max_y = visarea.max_y;

		for (int x = 0; x < 64; x++)
		{
			uint16_t tile = (m_luck_vram[0][count] & 0xff);
			uint16_t tile_high = (m_luck_vram[1][count]);
			uint16_t tileattr = (m_luck_vram[2][count]);
			uint8_t col = 0;
			uint8_t region = 0;
			uint8_t bgenable;

			clip.min_x = x*8;
			clip.max_x = x*8+8;

			if (clip.min_x<visarea.min_x) clip.min_x = visarea.min_x;
			if (clip.max_x>visarea.max_x) clip.max_x = visarea.max_x;

			/*
			  m_luck_vram[0]  tttt tttt   (t = low tile bits)
			  m_luck_vram[1]  tttt ppp?   (t = high tile bits) (p = pal select)?


			 */

			tile |= (tile_high & 0xf0) << 4;
			if (tileattr & 0x02) tile |= 0x1000;

			// ?? low bit is used too
			col = tile_high&0xf;

			// --ss fbt-   m_luck_vram3
			// - = unused?
			// s = reel layer select for this 8x8 region
			// f = fg enabled for this 8x8 region (or priority?)
			// b = reel enabled for this 8x8 region (not set on startup screens)
			// t = tile bank

			bgenable = (tileattr &0x30)>>4;

#if 0 // treat bit as fg enable
			if (tileattr&0x04)
			{
				if (bgenable==0) m_reel_tilemap[0]->draw(screen, bitmap, clip, 0, 0);
				if (bgenable==1) m_reel_tilemap[1]->draw(screen, bitmap, clip, 0, 0);
				if (bgenable==2) m_reel_tilemap[2]->draw(screen, bitmap, clip, 0, 0);
				if (bgenable==3) m_reel_tilemap[3]->draw(screen, bitmap, clip, 0, 0);
			}

			if (tileattr&0x08) m_gfxdecode->gfx(region)->transpen(bitmap,clip,tile,col,0,0,x*8,y*8, 0);

#else // treat it as priority flag instead (looks better in non-adult title screen - needs verifying)
			if (!(tileattr&0x08)) m_gfxdecode->gfx(region)->transpen(bitmap,clip,tile,col,0,0,x*8,y*8, 0);

			if (tileattr&0x04)
			{
				if (bgenable==0) m_reel_tilemap[0]->draw(screen, bitmap, clip, 0, 0);
				if (bgenable==1) m_reel_tilemap[1]->draw(screen, bitmap, clip, 0, 0);
				if (bgenable==2) m_reel_tilemap[2]->draw(screen, bitmap, clip, 0, 0);
				if (bgenable==3) m_reel_tilemap[3]->draw(screen, bitmap, clip, 0, 0);
			}

			if ((tileattr&0x08)) m_gfxdecode->gfx(region)->transpen(bitmap,clip,tile,col,0,0,x*8,y*8, 0);
#endif

			count++;
		}
	}
	return 0;
}

void luckgrln_state::mainmap(address_map &map)
{
	map(0x10000, 0x1ffff).rom().region("rom_data", 0x10000);
	map(0x20000, 0x2ffff).rom().region("rom_data", 0x00000);

	map(0x0c000, 0x0c1ff).ram().w(FUNC(luckgrln_state::reel_ram_w<0>)).share("reel_ram.0"); // only written to half way
	map(0x0c800, 0x0c9ff).ram().w(FUNC(luckgrln_state::reel_attr_w<0>)).share("reel_attr.0");
	map(0x0d000, 0x0d03f).ram().share("reel_scroll.0").mirror(0x000c0);

	map(0x0c200, 0x0c3ff).ram().w(FUNC(luckgrln_state::reel_ram_w<1>)).share("reel_ram.1");
	map(0x0ca00, 0x0cbff).ram().w(FUNC(luckgrln_state::reel_attr_w<1>)).share("reel_attr.1");
	map(0x0d200, 0x0d23f).ram().share("reel_scroll.1").mirror(0x000c0);

	map(0x0c400, 0x0c5ff).ram().w(FUNC(luckgrln_state::reel_ram_w<2>)).share("reel_ram.2");
	map(0x0cc00, 0x0cdff).ram().w(FUNC(luckgrln_state::reel_attr_w<2>)).share("reel_attr.2");
	map(0x0d400, 0x0d43f).ram().share("reel_scroll.2").mirror(0x000c0);

	map(0x0c600, 0x0c7ff).ram().w(FUNC(luckgrln_state::reel_ram_w<3>)).share("reel_ram.3");
	map(0x0ce00, 0x0cfff).ram().w(FUNC(luckgrln_state::reel_attr_w<3>)).share("reel_attr.3");
	map(0x0d600, 0x0d63f).ram().share("reel_scroll.3");

//  map(0x0d200, 0x0d2ff).ram();

	map(0x0d800, 0x0dfff).ram(); // nvram

	map(0x0e000, 0x0e7ff).ram().share("luck_vram.0");
	map(0x0e800, 0x0efff).ram().share("luck_vram.1");
	map(0x0f000, 0x0f7ff).ram().share("luck_vram.2");


	map(0x0f800, 0x0ffff).ram();
	map(0xf0000, 0xfffff).ram();
}

void luckgrln_state::_7smash_map(address_map &map)
{
	mainmap(map);
	map(0x00000, 0x0bfff).rom();
	map(0x10000, 0x2ffff).unmaprw();
	map(0xf0000, 0xfffff).unmaprw();
}

void luckgrln_state::output_w(uint8_t data)
{
	data &= 0xc7;

	/* correct? */
	if (data==0x84)
		m_nmi_enable = 0;
	else if (data==0x85)
		m_nmi_enable = 1;
	else
		printf("output_w unk data %02x\n",data);
}



void luckgrln_state::palette_offset_low_w(uint8_t data)
{
	m_palette_index = (m_palette_index & 0x0e00) | (data << 1);
}

void luckgrln_state::palette_offset_high_w(uint8_t data)
{
	m_palette_index = (data << 9) | (m_palette_index & 0x01fe);
}

void luckgrln_state::palette_w(uint8_t data)
{
	m_palette->write8(m_palette_index, data);
	m_palette_index = (m_palette_index + 1) & 0x0fff;
}

/* Analyzing the lamps, the game should have a 12-buttons control layout */
void luckgrln_state::lamps_a_w(uint8_t data)
{
/*  LAMPS A:

    7654 3210
    ---- ---x  HOLD1
    ---- --x-  HOLD2
    ---- -x--  HOLD3
    ---- x---  HOLD4
    ---x ----  HOLD5
    --x- ----  START
    -x-- ----  BET (PLAY)
    x--- ----  TAKE

*/

	for (int i = 0; i < 8; i++)
		m_lamps[i] = BIT(data, i);
}

void luckgrln_state::lamps_b_w(uint8_t data)
{
/*  LAMPS B:

    7654 3210
    ---- ---x  D-UP
    ---- --x-  HIGH
    ---- -x--  LOW
    ---- x---  CANCEL
    --xx ----  unknown (mostly on)
    xx-- ----  unused

*/
	for (int i = 0; i < 4; i++)
		m_lamps[i + 8] = BIT(data, i);
}

void luckgrln_state::counters_w(uint8_t data)
{
/*  COUNTERS:

    7654 3210
    ---- ---x  COIN1
    ---- --x-  KEYIN
    ---- -x--  COIN2
    ---- x---  COIN3
    xxxx ----  unused

*/
	machine().bookkeeping().coin_counter_w(0, data & 0x01);  /* COIN 1 */
	machine().bookkeeping().coin_counter_w(1, data & 0x04);  /* COIN 2 */
	machine().bookkeeping().coin_counter_w(2, data & 0x08);  /* COIN 3 */
	machine().bookkeeping().coin_counter_w(3, data & 0x02);  /* KEY IN */
}


/* are some of these reads / writes mirrored? there seem to be far too many */
void luckgrln_state::common_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x0000, 0x007f).noprw(); // Z180 internal regs

	map(0x00a0, 0x00a0).w(FUNC(luckgrln_state::palette_offset_low_w));
	map(0x00a1, 0x00a1).w(FUNC(luckgrln_state::palette_offset_high_w));
	map(0x00a2, 0x00a2).w(FUNC(luckgrln_state::palette_w));

	map(0x00b0, 0x00b0).w("crtc", FUNC(mc6845_device::address_w));
	map(0x00b1, 0x00b1).w("crtc", FUNC(mc6845_device::register_w));

	map(0x00b8, 0x00b8).portr("IN0");
	map(0x00b9, 0x00b9).portr("IN1").w(FUNC(luckgrln_state::counters_w));
	map(0x00ba, 0x00ba).portr("IN2").w(FUNC(luckgrln_state::lamps_a_w));
	map(0x00bb, 0x00bb).portr("IN3").w(FUNC(luckgrln_state::lamps_b_w));
	map(0x00bc, 0x00bc).portr("DSW1");

	map(0x00c0, 0x00c3).nopw();
	map(0x00c4, 0x00c7).nopw();
	map(0x00c8, 0x00cb).nopw();
	map(0x00cc, 0x00cf).nopw();

	map(0x00d0, 0x00d3).nopw();
	map(0x00d4, 0x00d7).nopw();
	map(0x00d8, 0x00db).nopw();
	map(0x00dc, 0x00df).nopw();

	map(0x00e4, 0x00e7).nopw();

	map(0x00f3, 0x00f3).nopw();
	map(0x00f7, 0x00f7).nopw();

	map(0x00f8, 0x00f8).portr("DSW2");
	map(0x00f9, 0x00f9).portr("DSW3");
	map(0x00fa, 0x00fa).portr("DSW4");
	map(0x00fb, 0x00fb).portr("DSW5"); //.nopw();
	map(0x00fc, 0x00fc).nopw();
	map(0x00fd, 0x00fd).nopw();
	map(0x00fe, 0x00fe).nopw();
	map(0x00ff, 0x00ff).nopw();

/*

  C0-C3 seems to be a 4-bytes port device (maybe sound), where is written --> data, control (or channel), unknown, volume.
  If in fact it's a sound device, the last parameter (volume) is decreasing once the sound event was triggered.
  Seems to be more than one, and these devices seems mirrored along the rest of memory.

*/

}

void luckgrln_state::luckgrln_io(address_map &map)
{
	common_portmap(map);
	map(0x0090, 0x009f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));
}

/* reads a bit 1 status there after every round played */
uint8_t luckgrln_state::test_r()
{
	return 0xff;
}

void luckgrln_state::_7smash_io(address_map &map)
{
	common_portmap(map);
}

static INPUT_PORTS_START( luckgrln )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_CODE(KEYCODE_1) PORT_NAME("Start")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_NAME("Play")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_DIPNAME( 0x80, 0x80, "IN0" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_DIPNAME( 0x20, 0x20, "IN1" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(1)
	PORT_DIPNAME( 0x10, 0x10, "IN2" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_R)   PORT_NAME("Reset")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_9)   PORT_NAME("Service In")
	PORT_DIPNAME( 0x04, 0x04, "IN3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_W)   PORT_NAME("Credit Clear")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_0)   PORT_NAME("Books SW")

	PORT_START("DSW1") //DIP SW 1
	PORT_DIPNAME( 0x01, 0x01, "Auto Hold" )            PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Game Type" )            PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "Hold Game" )
	PORT_DIPSETTING(    0x00, "Discard Game" )
	PORT_DIPNAME( 0x04, 0x04, "Adult Content" )        PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW1-08" )              PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Minimal Winning Hand" ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "Jacks or Better" )
	PORT_DIPSETTING(    0x00, "2 Pairs" )
	PORT_DIPNAME( 0x20, 0x20, "Minimum Bet" )          PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, "DSW1-40 (Do Not Use)" ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW1-80 (Do Not Use)" ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2") //DIP SW 2
	PORT_DIPNAME( 0x01, 0x01, "DSW2-01 (Do Not Use)" ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW2-02 (Do Not Use)" ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW2-04 (Do Not Use)" ) PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW2-08 (Do Not Use)" ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW2-10" )              PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW2-20" )              PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW2-40" )              PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW2-80" )              PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x04, "Key In" )               PORT_DIPLOCATION("DSW3:1,2,3")
	PORT_DIPSETTING(    0x07, "1 Pulse/5 Credits" )
	PORT_DIPSETTING(    0x06, "1 Pulse/10 Credits" )
	PORT_DIPSETTING(    0x05, "1 Pulse/50 Credits" )
	PORT_DIPSETTING(    0x04, "1 Pulse/100 Credits" )
	PORT_DIPSETTING(    0x03, "1 Pulse/200 Credits" )
	PORT_DIPSETTING(    0x02, "1 Pulse/300 Credits" )
	PORT_DIPSETTING(    0x01, "1 Pulse/500 Credits" )
	PORT_DIPSETTING(    0x00, "1 Pulse/1000 Credits" )
	PORT_DIPNAME( 0x38, 0x10, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DSW3:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/50 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "DSW3-40" )               PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW3-80" )               PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x04, "Coin C" )                PORT_DIPLOCATION("DSW4:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0e, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0d, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x04, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x70, 0x10, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("DSW4:5,6,7")
	PORT_DIPSETTING(    0x70, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x60, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/200 Credits" )
	PORT_DIPNAME( 0x80, 0x80, "DSW4-80" )               PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5") //DIP SW 5
	PORT_DIPNAME( 0x01, 0x01, "DSW5-01 (Do Not Use)" )  PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW5-02 (Do Not Use)" )  PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW5-04" )               PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Double-Up" )             PORT_DIPLOCATION("DSW5:4,5")
	PORT_DIPSETTING(    0x18, "Double-Up (Normal)" )
	PORT_DIPSETTING(    0x08, "Double-Up Poker" )
	PORT_DIPSETTING(    0x10, "Double-Up Bingo" )
	PORT_DIPSETTING(    0x00, "No Double-Up" )
	PORT_DIPNAME( 0x20, 0x20, "DSW5-20" )               PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW5-40" )               PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW5-80" )               PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( _7smash )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Reel Speed" )      PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x02, 0x02, "Renchan Pattern" ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Key In Value" )    PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(    0x00, "200" )
	PORT_DIPSETTING(    0x04, "100" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPNAME( 0x70, 0x60, "Percentage" )      PORT_DIPLOCATION("DSW1:5,6,7")
	PORT_DIPSETTING(    0x00, "-- (Invalid 1)" )
	PORT_DIPSETTING(    0x10, "-- (Invalid 2)" )
	PORT_DIPSETTING(    0x20, "-- (Invalid 3)" )
	PORT_DIPSETTING(    0x30, "80%" )
	PORT_DIPSETTING(    0x40, "85%" )
	PORT_DIPSETTING(    0x50, "90%" )
	PORT_DIPSETTING(    0x60, "95%" )
	PORT_DIPSETTING(    0x70, "105%" )
	PORT_DIPNAME( 0x80, 0x80, "Reset Mode" )      PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, "Auto" )
	PORT_DIPSETTING(    0x00, "Manual" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Payout Mode" )     PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPNAME( 0x04, 0x04, "Coin Sw Active" )  PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )    PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPNAME( 0x60, 0x60, "Panel Type" )      PORT_DIPLOCATION("DSW2:6,7")
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPSETTING(    0x20, "C" )
	PORT_DIPSETTING(    0x40, "B" )
	PORT_DIPSETTING(    0x60, "A" ) // inputs and layout are for this Panel Type only, ToDo: expand with conditional inputs
	PORT_DIPNAME( 0x80, 0x80, "Alt. Test" )       PORT_DIPLOCATION("DSW2:8") // not shown on test screen
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("DSW3:3,4")
	PORT_DIPSETTING(    0x0c, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xf0, 0x00, "Coin C" )          PORT_DIPLOCATION("DSW3:5,6,7,8")
	PORT_DIPSETTING(    0x00, "1 Coin/200 Credits" )
	PORT_DIPSETTING(    0x10, "1 Coin/100 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x30, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x40, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x50, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xd0, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0xe0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xf0, "10 Coins/1 Credit" )

	PORT_START("DSW4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_UNUSED  )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_UNUSED  )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Odds")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_NAME("Stop 1")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW,  IPT_UNUSED  )

	PORT_START("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 ) PORT_NAME("Service SW")
	PORT_BIT( 0xef, IP_ACTIVE_LOW,  IPT_UNUSED  )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN2  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN3  )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW,  IPT_UNUSED  )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SERVICE2 ) PORT_NAME("Reset SW")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNUSED  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNUSED ) //PORT_NAME("Hopper Coin SW")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNUSED ) //PORT_NAME("Hopper Coin Empty SW")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,6),
	5,
	{ RGN_FRAC(1,6),RGN_FRAC(3,6),RGN_FRAC(2,6),RGN_FRAC(5,6),RGN_FRAC(4,6) }, /* RGN_FRAC(0,6) isn't used (empty) */
	{ STEP8(0,1) },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8},
	8*8
};

static const gfx_layout tiles8x32_layout =
{
	8,32,
	RGN_FRAC(1,6),
	5,
	{ RGN_FRAC(1,6),RGN_FRAC(3,6),RGN_FRAC(2,6),RGN_FRAC(5,6),RGN_FRAC(4,6) },
	{ STEP8(0,1) },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8, 8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8,16*8,17*8,18*8,19*8,20*8,21*8,22*8,23*8,24*8,25*8,26*8,27*8,28*8,29*8,30*8,31*8},
	32*8
};

static GFXDECODE_START( gfx_luckgrln )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0x400, 64 )
	GFXDECODE_ENTRY( "reels", 0, tiles8x32_layout, 0, 64 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(luckgrln_state::irq)
{
	if(m_nmi_enable)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void luckgrln_state::luckgrln(machine_config &config)
{
	HD647180X(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &luckgrln_state::mainmap);
	m_maincpu->set_addrmap(AS_IO, &luckgrln_state::luckgrln_io);
	m_maincpu->set_vblank_int("screen", FUNC(luckgrln_state::irq));
	m_maincpu->out_pa_callback().set(FUNC(luckgrln_state::output_w));

	hd6845s_device &crtc(HD6845S(config, "crtc", 12_MHz_XTAL / 8)); /* HD6845SP; unknown clock, hand tuned to get ~60 fps */
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	MSM6242(config, "rtc", 0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(12_MHz_XTAL, 752, 0, 512, 274, 0, 256);
	m_screen->set_screen_update(FUNC(luckgrln_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_luckgrln);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);

	SPEAKER(config, "mono").front_center();
}

void luckgrln_state::_7smash(machine_config &config)
{
	luckgrln(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &luckgrln_state::_7smash_map);
	m_maincpu->set_addrmap(AS_IO, &luckgrln_state::_7smash_io);
	m_maincpu->in_pg_callback().set(FUNC(luckgrln_state::test_r));

	// slightly different timings
	m_screen->set_raw(12_MHz_XTAL, 768, 0, 512, 260, 0, 256);

	config.device_remove("rtc");
}

void luckgrln_state::init_luckgrln()
{
	uint8_t *rom = memregion("rom_data")->base();

	for (int i = 0; i < 0x20000; i++)
	{
		uint8_t x = rom[i];
		uint8_t v = 0xfe + (i & 0xf)*0x3b + ((i >> 4) & 0xf)*0x9c + ((i >> 8) & 0xf)*0xe1 + ((i >> 12) & 0x7)*0x10;
		v += ((((i >> 4) & 0xf) + ((i >> 2) & 3)) >> 2) * 0x50;
		x ^= ~v;
		x = (x << (i & 7)) | (x >> (8-(i & 7)));
		rom[i] = x;
	}

	#if 0
	{
		char filename[256];
		sprintf(filename,"decrypted_%s", machine().system().name);
		FILE *fp = fopen(filename, "w+b");
		if (fp)
		{
			fwrite(rom, 0x20000, 1, fp);
			fclose(fp);
		}
	}
	#endif

	// ??
//  membank("bank1")->set_base(&rom[0x010000]);
}


ROM_START( luckgrln )
	ROM_REGION( 0x4000, "maincpu", 0 ) // internal Z180 rom
	ROM_LOAD( "lucky74.bin",  0x00000, 0x4000, CRC(fa128e05) SHA1(97a9534b8414f984159271db48b153b0724d22f9) )

	ROM_REGION( 0x20000, "rom_data", 0 ) // external data / cpu rom
	ROM_LOAD( "falcon.13",  0x00000, 0x20000, CRC(f7a717fd) SHA1(49a39b84620876ee2faf73aaa405a1e17cab2da2) )

	ROM_REGION( 0x60000, "reels", 0 )
	ROM_LOAD( "eagle.1", 0x00000, 0x20000, CRC(37209082) SHA1(ffb30da5920886f37c6b97e03f5a8ec3b6265e68) ) // half unused, 5bpp
	ROM_LOAD( "eagle.2", 0x20000, 0x20000, CRC(bdb2d694) SHA1(3e58fe3f6b447181e3a85f0fc2a0c996231bc8e8) )
	ROM_LOAD( "eagle.3", 0x40000, 0x20000, CRC(2c765389) SHA1(d5697c73cc939aa46f36c2dd87e90bba2536e347) )

	ROM_REGION( 0x60000, "gfx2", 0 )
	ROM_LOAD( "falcon.4", 0x00000, 0x20000, CRC(369eaddf) SHA1(52387ea63e5c8fb0c27b796026152a06b68467af) ) // half unused, 5bpp
	ROM_LOAD( "falcon.5", 0x20000, 0x20000, CRC(c9ac1fe7) SHA1(fc027002754b90cc49ca74fac5240a99a194c0b3) )
	ROM_LOAD( "falcon.6", 0x40000, 0x20000, CRC(bfb02c87) SHA1(1b5ca562ed76eb3f1b4a52d379a6af07e79b6ee5) )
ROM_END

ROM_START( 7smash )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "eagle.8",      0x000000, 0x020000, CRC(b115c5d5) SHA1(3f80613886b7f8092ec914c9bfb416078aca82a3) )
	ROM_LOAD( "7smash.bin",   0x000000, 0x004000, CRC(58396efa) SHA1(b957d28e321a5c4f9a90e0a7eaf8f01450662c0e) ) // internal Z180 rom

	ROM_REGION( 0x20000, "rom_data", ROMREGION_ERASEFF ) // external data / cpu rom


	ROM_REGION( 0x60000, "reels", ROMREGION_ERASE00 ) // reel gfxs
	ROM_LOAD( "eagle.3",      0x40000, 0x020000, CRC(d75b3b2f) SHA1(1d90bc17f9e645966126fa19c42a7c4d54098776) )
	ROM_LOAD( "eagle.2",      0x20000, 0x020000, CRC(211b5acb) SHA1(e35ae6c93a1daa9d3aa46970c5c3d39788f948bb) )
	ROM_LOAD( "eagle.1",      0x00000, 0x020000, CRC(21317c37) SHA1(7706045b85f86f6e58cc67c2d7dee01d80df3422) )  // half unused, 5bpp

	ROM_REGION( 0x60000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD( "eagle.6",      0x40000, 0x20000, CRC(2c4416d4) SHA1(25d04d4d08ab491a9684b8e6f21e57479711ee87) )
	ROM_LOAD( "eagle.5",      0x20000, 0x20000, CRC(cd8bc456) SHA1(cefe211492158f445ceaaa9015e1143ea9afddbb) )
	ROM_LOAD( "eagle.4",      0x00000, 0x20000, CRC(dcf92dca) SHA1(87c7d88dc35981ad636376b53264cee87ccdaa71) )  // half unused, 5bpp
ROM_END


} // anonymous namespace


/*********************************************
*                Game Drivers                *
**********************************************/

//     YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT           ROT   COMPANY           FULL NAME                                 FLAGS                                     LAYOUT
GAMEL( 1991, luckgrln, 0,      luckgrln, luckgrln, luckgrln_state, init_luckgrln, ROT0, "Wing Co., Ltd.", "Lucky Girl (newer Z180 based hardware)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE, layout_luckgrln )
GAMEL( 1993, 7smash,   0,      _7smash,  _7smash,  luckgrln_state, empty_init,    ROT0, "Sovic",          "7 Smash",                                MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE, layout_7smash )
