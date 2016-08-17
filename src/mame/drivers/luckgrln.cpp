// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca, Angelo Salese
/*

 Lucky Girl (newer 1991 version on different hardware?)
  -- there is an early 'Lucky Girl' which appears to be running on Nichibutsu like hardware.

 The program rom extracted from the Z180 also refers to this as Lucky 74..

 TODO:
 - inputs / port mapping
 - sound (what's the sound chip?)
 - reel scroll / reel enable / reel colours
 - are the colours correct even on the text layer? they look odd in places, and there are unused bits
 - dunno where / how is the service mode bit is connected (I've accessed it once, but dunno how I've did it)
 - graphics in 7smash are quite off (encrypted ROM?)

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
 I/O conenctors.

 Reset switch cuts power supply going to Video/Work RAM.


*/


#include "emu.h"
#include "cpu/z180/z180.h"
#include "video/mc6845.h"
#include "luckgrln.lh"


class luckgrln_state : public driver_device
{
public:
	luckgrln_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_reel1_ram(*this, "reel1_ram"),
		m_reel1_attr(*this, "reel1_attr"),
		m_reel1_scroll(*this, "reel1_scroll"),
		m_reel2_ram(*this, "reel2_ram"),
		m_reel2_attr(*this, "reel2_attr"),
		m_reel2_scroll(*this, "reel2_scroll"),
		m_reel3_ram(*this, "reel3_ram"),
		m_reel3_attr(*this, "reel3_attr"),
		m_reel3_scroll(*this, "reel3_scroll"),
		m_reel4_ram(*this, "reel4_ram"),
		m_reel4_attr(*this, "reel4_attr"),
		m_reel4_scroll(*this, "reel4_scroll"),
		m_luck_vram1(*this, "luck_vram1"),
		m_luck_vram2(*this, "luck_vram2"),
		m_luck_vram3(*this, "luck_vram3"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	UINT8 m_nmi_enable;
	tilemap_t *m_reel1_tilemap;
	tilemap_t *m_reel2_tilemap;
	tilemap_t *m_reel3_tilemap;
	tilemap_t *m_reel4_tilemap;
	required_shared_ptr<UINT8> m_reel1_ram;
	required_shared_ptr<UINT8> m_reel1_attr;
	required_shared_ptr<UINT8> m_reel1_scroll;
	required_shared_ptr<UINT8> m_reel2_ram;
	required_shared_ptr<UINT8> m_reel2_attr;
	required_shared_ptr<UINT8> m_reel2_scroll;
	required_shared_ptr<UINT8> m_reel3_ram;
	required_shared_ptr<UINT8> m_reel3_attr;
	required_shared_ptr<UINT8> m_reel3_scroll;
	required_shared_ptr<UINT8> m_reel4_ram;
	required_shared_ptr<UINT8> m_reel4_attr;
	required_shared_ptr<UINT8> m_reel4_scroll;
	required_shared_ptr<UINT8> m_luck_vram1;
	required_shared_ptr<UINT8> m_luck_vram2;
	required_shared_ptr<UINT8> m_luck_vram3;

	int m_palette_count;
	UINT8 m_palette_ram[0x10000];
	DECLARE_WRITE8_MEMBER(luckgrln_reel1_ram_w);
	DECLARE_WRITE8_MEMBER(luckgrln_reel1_attr_w);
	DECLARE_WRITE8_MEMBER(luckgrln_reel2_ram_w);
	DECLARE_WRITE8_MEMBER(luckgrln_reel2_attr_w);
	DECLARE_WRITE8_MEMBER(luckgrln_reel3_ram_w);
	DECLARE_WRITE8_MEMBER(luckgrln_reel3_attr_w);
	DECLARE_WRITE8_MEMBER(luckgrln_reel4_ram_w);
	DECLARE_WRITE8_MEMBER(luckgrln_reel4_attr_w);
	DECLARE_WRITE8_MEMBER(output_w);
	DECLARE_WRITE8_MEMBER(palette_offset_low_w);
	DECLARE_WRITE8_MEMBER(palette_offset_high_w);
	DECLARE_WRITE8_MEMBER(palette_w);
	DECLARE_READ8_MEMBER(rtc_r);
	DECLARE_WRITE8_MEMBER(lamps_a_w);
	DECLARE_WRITE8_MEMBER(lamps_b_w);
	DECLARE_WRITE8_MEMBER(counters_w);
	DECLARE_READ8_MEMBER(test_r);
	DECLARE_DRIVER_INIT(luckgrln);
	TILE_GET_INFO_MEMBER(get_luckgrln_reel1_tile_info);
	TILE_GET_INFO_MEMBER(get_luckgrln_reel2_tile_info);
	TILE_GET_INFO_MEMBER(get_luckgrln_reel3_tile_info);
	TILE_GET_INFO_MEMBER(get_luckgrln_reel4_tile_info);
	virtual void video_start() override;
	UINT32 screen_update_luckgrln(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(luckgrln_irq);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


WRITE8_MEMBER(luckgrln_state::luckgrln_reel1_ram_w)
{
	m_reel1_ram[offset] = data;
	m_reel1_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(luckgrln_state::luckgrln_reel1_attr_w)
{
	m_reel1_attr[offset] = data;
	m_reel1_tilemap->mark_tile_dirty(offset);
}



TILE_GET_INFO_MEMBER(luckgrln_state::get_luckgrln_reel1_tile_info)
{
	int code = m_reel1_ram[tile_index];
	int attr = m_reel1_attr[tile_index];
	int col = (attr & 0x1f);

	code |= (attr & 0xe0)<<3;


	SET_TILE_INFO_MEMBER(1,
			code,
			col,
			0);
}


WRITE8_MEMBER(luckgrln_state::luckgrln_reel2_ram_w)
{
	m_reel2_ram[offset] = data;
	m_reel2_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(luckgrln_state::luckgrln_reel2_attr_w)
{
	m_reel2_attr[offset] = data;
	m_reel2_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(luckgrln_state::get_luckgrln_reel2_tile_info)
{
	int code = m_reel2_ram[tile_index];
	int attr = m_reel2_attr[tile_index];
	int col = (attr & 0x1f);

	code |= (attr & 0xe0)<<3;


	SET_TILE_INFO_MEMBER(1,
			code,
			col,
			0);
}

WRITE8_MEMBER(luckgrln_state::luckgrln_reel3_ram_w)
{
	m_reel3_ram[offset] = data;
	m_reel3_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(luckgrln_state::luckgrln_reel3_attr_w)
{
	m_reel3_attr[offset] = data;
	m_reel3_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(luckgrln_state::get_luckgrln_reel3_tile_info)
{
	int code = m_reel3_ram[tile_index];
	int attr = m_reel3_attr[tile_index];
	int col = (attr & 0x1f);

	code |= (attr & 0xe0)<<3;

	SET_TILE_INFO_MEMBER(1,
			code,
			col,
			0);
}

WRITE8_MEMBER(luckgrln_state::luckgrln_reel4_ram_w)
{
	m_reel4_ram[offset] = data;
	m_reel4_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(luckgrln_state::luckgrln_reel4_attr_w)
{
	m_reel4_attr[offset] = data;
	m_reel4_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(luckgrln_state::get_luckgrln_reel4_tile_info)
{
	int code = m_reel4_ram[tile_index];
	int attr = m_reel4_attr[tile_index];
	int col = (attr & 0x1f);

	code |= (attr & 0xe0)<<3;

	SET_TILE_INFO_MEMBER(1,
			code,
			col,
			0);
}

void luckgrln_state::video_start()
{
	m_reel1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(luckgrln_state::get_luckgrln_reel1_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(luckgrln_state::get_luckgrln_reel2_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(luckgrln_state::get_luckgrln_reel3_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel4_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(luckgrln_state::get_luckgrln_reel4_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_reel1_tilemap->set_scroll_cols(64);
	m_reel2_tilemap->set_scroll_cols(64);
	m_reel3_tilemap->set_scroll_cols(64);
	m_reel4_tilemap->set_scroll_cols(64);

	m_reel1_tilemap->set_transparent_pen(0 );
	m_reel2_tilemap->set_transparent_pen(0 );
	m_reel3_tilemap->set_transparent_pen(0 );
	m_reel4_tilemap->set_transparent_pen(0 );
}

UINT32 luckgrln_state::screen_update_luckgrln(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y,x;
	int count = 0;
	const rectangle &visarea = screen.visible_area();
	int i;

	rectangle clip = visarea;

	bitmap.fill(0, cliprect);

	for (i= 0;i < 64;i++)
	{
		m_reel1_tilemap->set_scrolly(i, m_reel1_scroll[i]);
		m_reel2_tilemap->set_scrolly(i, m_reel2_scroll[i]);
		m_reel3_tilemap->set_scrolly(i, m_reel3_scroll[i]);
		m_reel4_tilemap->set_scrolly(i, m_reel4_scroll[i]);
	}


	for (y=0;y<32;y++)
	{
		clip.min_y = y*8;
		clip.max_y = y*8+8;

		if (clip.min_y<visarea.min_y) clip.min_y = visarea.min_y;
		if (clip.max_y>visarea.max_y) clip.max_y = visarea.max_y;

		for (x=0;x<64;x++)
		{
			UINT16 tile = (m_luck_vram1[count] & 0xff);
			UINT16 tile_high = (m_luck_vram2[count]);
			UINT16 tileattr = (m_luck_vram3[count]);
			UINT8 col = 0;
			UINT8 region = 0;
			UINT8 bgenable;

			clip.min_x = x*8;
			clip.max_x = x*8+8;

			if (clip.min_x<visarea.min_x) clip.min_x = visarea.min_x;
			if (clip.max_x>visarea.max_x) clip.max_x = visarea.max_x;

			/*
			  m_luck_vram1  tttt tttt   (t = low tile bits)
			  m_luck_vram2  tttt ppp?   (t = high tile bits) (p = pal select)?


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
				if (bgenable==0) m_reel1_tilemap->draw(screen, bitmap, clip, 0, 0);
				if (bgenable==1) m_reel2_tilemap->draw(screen, bitmap, clip, 0, 0);
				if (bgenable==2) m_reel3_tilemap->draw(screen, bitmap, clip, 0, 0);
				if (bgenable==3) m_reel4_tilemap->draw(screen, bitmap, clip, 0, 0);
			}

			if (tileattr&0x08) m_gfxdecode->gfx(region)->transpen(bitmap,clip,tile,col,0,0,x*8,y*8, 0);

#else // treat it as priority flag instead (looks better in non-adult title screen - needs verifying)
			if (!(tileattr&0x08)) m_gfxdecode->gfx(region)->transpen(bitmap,clip,tile,col,0,0,x*8,y*8, 0);

			if (tileattr&0x04)
			{
				if (bgenable==0) m_reel1_tilemap->draw(screen, bitmap, clip, 0, 0);
				if (bgenable==1) m_reel2_tilemap->draw(screen, bitmap, clip, 0, 0);
				if (bgenable==2) m_reel3_tilemap->draw(screen, bitmap, clip, 0, 0);
				if (bgenable==3) m_reel4_tilemap->draw(screen, bitmap, clip, 0, 0);
			}

			if ((tileattr&0x08)) m_gfxdecode->gfx(region)->transpen(bitmap,clip,tile,col,0,0,x*8,y*8, 0);
#endif

			count++;
		}
	}
	return 0;
}

static ADDRESS_MAP_START( mainmap, AS_PROGRAM, 8, luckgrln_state )
	AM_RANGE(0x00000, 0x03fff) AM_ROM
	AM_RANGE(0x10000, 0x1ffff) AM_ROM AM_REGION("rom_data",0x10000)
	AM_RANGE(0x20000, 0x2ffff) AM_ROM AM_REGION("rom_data",0x00000)

	AM_RANGE(0x0c000, 0x0c1ff) AM_RAM_WRITE(luckgrln_reel1_ram_w)  AM_SHARE("reel1_ram") // only written to half way
	AM_RANGE(0x0c800, 0x0c9ff) AM_RAM_WRITE(luckgrln_reel1_attr_w) AM_SHARE("reel1_attr")
	AM_RANGE(0x0d000, 0x0d03f) AM_RAM AM_SHARE("reel1_scroll") AM_MIRROR(0x000c0)

	AM_RANGE(0x0c200, 0x0c3ff) AM_RAM_WRITE(luckgrln_reel2_ram_w)  AM_SHARE("reel2_ram")
	AM_RANGE(0x0ca00, 0x0cbff) AM_RAM_WRITE(luckgrln_reel2_attr_w) AM_SHARE("reel2_attr")
	AM_RANGE(0x0d200, 0x0d23f) AM_RAM AM_SHARE("reel2_scroll") AM_MIRROR(0x000c0)

	AM_RANGE(0x0c400, 0x0c5ff) AM_RAM_WRITE(luckgrln_reel3_ram_w ) AM_SHARE("reel3_ram")
	AM_RANGE(0x0cc00, 0x0cdff) AM_RAM_WRITE(luckgrln_reel3_attr_w) AM_SHARE("reel3_attr")
	AM_RANGE(0x0d400, 0x0d43f) AM_RAM AM_SHARE("reel3_scroll") AM_MIRROR(0x000c0)

	AM_RANGE(0x0c600, 0x0c7ff) AM_RAM_WRITE(luckgrln_reel4_ram_w ) AM_SHARE("reel4_ram")
	AM_RANGE(0x0ce00, 0x0cfff) AM_RAM_WRITE(luckgrln_reel4_attr_w) AM_SHARE("reel4_attr")
	AM_RANGE(0x0d600, 0x0d63f) AM_RAM AM_SHARE("reel4_scroll")

//  AM_RANGE(0x0d200, 0x0d2ff) AM_RAM


	AM_RANGE(0x0d800, 0x0dfff) AM_RAM // nvram

	AM_RANGE(0x0e000, 0x0e7ff) AM_RAM AM_SHARE("luck_vram1")
	AM_RANGE(0x0e800, 0x0efff) AM_RAM AM_SHARE("luck_vram2")
	AM_RANGE(0x0f000, 0x0f7ff) AM_RAM AM_SHARE("luck_vram3")


	AM_RANGE(0x0f800, 0x0ffff) AM_RAM
	AM_RANGE(0xf0000, 0xfffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( _7smash_map, AS_PROGRAM, 8, luckgrln_state )
	AM_RANGE(0x00000, 0x0bfff) AM_ROM
	AM_IMPORT_FROM( mainmap )
	AM_RANGE(0x10000, 0x2ffff) AM_UNMAP
	AM_RANGE(0xf0000, 0xfffff) AM_UNMAP
ADDRESS_MAP_END

WRITE8_MEMBER(luckgrln_state::output_w)
{
	/* correct? */
	if (data==0x84)
		m_nmi_enable = 0;
	else if (data==0x85)
		m_nmi_enable = 1;
	else
		printf("output_w unk data %02x\n",data);
}



WRITE8_MEMBER(luckgrln_state::palette_offset_low_w)
{
	m_palette_count = data<<1;
}
WRITE8_MEMBER(luckgrln_state::palette_offset_high_w)
{
	m_palette_count = m_palette_count | data<<9;
}


WRITE8_MEMBER(luckgrln_state::palette_w)
{
	m_palette_ram[m_palette_count] = data;


	{
		int r,g,b;
		int offs;
		UINT16 dat;
		offs = m_palette_count&~0x1;
		dat = m_palette_ram[offs] | m_palette_ram[offs+1]<<8;

		r = (dat >> 0) & 0x1f;
		g = (dat >> 5) & 0x1f;
		b = (dat >> 10) & 0x1f;

		m_palette->set_pen_color(offs/2, pal5bit(r), pal5bit(g), pal5bit(b));

	}

	m_palette_count++;

}

// Oki M62X428 is a 4-bit RTC, doesn't seem to be millennium bug proof ...
READ8_MEMBER(luckgrln_state::rtc_r)
{
	system_time systime;
	machine().base_datetime(systime);

	switch(offset)
	{
		case 0x00: return (systime.local_time.second % 10);
		case 0x01: return (systime.local_time.second / 10);
		case 0x02: return (systime.local_time.minute % 10);
		case 0x03: return (systime.local_time.minute / 10);
		case 0x04: return (systime.local_time.hour % 10);
		case 0x05: return (systime.local_time.hour / 10);
		case 0x06: return (systime.local_time.mday % 10);
		case 0x07: return (systime.local_time.mday / 10);
		case 0x08: return ((systime.local_time.month+1) % 10);
		case 0x09: return ((systime.local_time.month+1) / 10);
		case 0x0a: return (systime.local_time.year%10);
		case 0x0b: return ((systime.local_time.year%100)/10);

		case 0x0d: return 0xff; // bit 1: reset switch for the RTC?
	}

	return 0;
}

/* Analizing the lamps, the game should has a 12-buttons control layout */
WRITE8_MEMBER(luckgrln_state::lamps_a_w)
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
	output().set_lamp_value(0, (data >> 0) & 1);      /* HOLD1 */
	output().set_lamp_value(1, (data >> 1) & 1);      /* HOLD2 */
	output().set_lamp_value(2, (data >> 2) & 1);      /* HOLD3 */
	output().set_lamp_value(3, (data >> 3) & 1);      /* HOLD4 */
	output().set_lamp_value(4, (data >> 4) & 1);      /* HOLD5 */
	output().set_lamp_value(5, (data >> 5) & 1);      /* START */
	output().set_lamp_value(6, (data >> 6) & 1);      /* BET */
	output().set_lamp_value(7, (data >> 7) & 1);      /* TAKE */
}

WRITE8_MEMBER(luckgrln_state::lamps_b_w)
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
	output().set_lamp_value(8, (data >> 0) & 1);      /* D-UP */
	output().set_lamp_value(9, (data >> 1) & 1);      /* HIGH */
	output().set_lamp_value(10, (data >> 2) & 1);     /* LOW */
	output().set_lamp_value(11, (data >> 3) & 1);     /* CANCEL */
}

WRITE8_MEMBER(luckgrln_state::counters_w)
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
static ADDRESS_MAP_START( portmap, AS_IO, 8, luckgrln_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0000, 0x003f) AM_RAM // Z180 internal regs
	AM_RANGE(0x0060, 0x0060) AM_WRITE(output_w)

	AM_RANGE(0x0090, 0x009f) AM_READ(rtc_r) //AM_WRITENOP

	AM_RANGE(0x00a0, 0x00a0) AM_WRITE(palette_offset_low_w)
	AM_RANGE(0x00a1, 0x00a1) AM_WRITE(palette_offset_high_w)
	AM_RANGE(0x00a2, 0x00a2) AM_WRITE(palette_w)

	AM_RANGE(0x00b0, 0x00b0) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x00b1, 0x00b1) AM_DEVWRITE("crtc", mc6845_device, register_w)

	AM_RANGE(0x00b8, 0x00b8) AM_READ_PORT("IN0")
	AM_RANGE(0x00b9, 0x00b9) AM_READ_PORT("IN1") AM_WRITE(counters_w)
	AM_RANGE(0x00ba, 0x00ba) AM_READ_PORT("IN2") AM_WRITE(lamps_a_w)
	AM_RANGE(0x00bb, 0x00bb) AM_READ_PORT("IN3") AM_WRITE(lamps_b_w)
	AM_RANGE(0x00bc, 0x00bc) AM_READ_PORT("DSW1")

	AM_RANGE(0x00c0, 0x00c3) AM_WRITENOP
	AM_RANGE(0x00c4, 0x00c7) AM_WRITENOP
	AM_RANGE(0x00c8, 0x00cb) AM_WRITENOP
	AM_RANGE(0x00cc, 0x00cf) AM_WRITENOP

	AM_RANGE(0x00d0, 0x00d3) AM_WRITENOP
	AM_RANGE(0x00d4, 0x00d7) AM_WRITENOP
	AM_RANGE(0x00d8, 0x00db) AM_WRITENOP
	AM_RANGE(0x00dc, 0x00df) AM_WRITENOP

	AM_RANGE(0x00e4, 0x00e7) AM_WRITENOP

	AM_RANGE(0x00f3, 0x00f3) AM_WRITENOP
	AM_RANGE(0x00f7, 0x00f7) AM_WRITENOP

	AM_RANGE(0x00f8, 0x00f8) AM_READ_PORT("DSW2")
	AM_RANGE(0x00f9, 0x00f9) AM_READ_PORT("DSW3")
	AM_RANGE(0x00fa, 0x00fa) AM_READ_PORT("DSW4")
	AM_RANGE(0x00fb, 0x00fb) AM_READ_PORT("DSW5") //AM_WRITENOP
	AM_RANGE(0x00fc, 0x00fc) AM_WRITENOP
	AM_RANGE(0x00fd, 0x00fd) AM_WRITENOP
	AM_RANGE(0x00fe, 0x00fe) AM_WRITENOP
	AM_RANGE(0x00ff, 0x00ff) AM_WRITENOP

/*

  C0-C3 seems to be a 4-bytes port device (maybe sound), where is written --> data, control (or channel), unknown, volume.
  If in fact it's a sound device, the last parameter (volume) is decreasing once the sound event was triggered.
  Seems to be more than one, and these devices seems mirrored along the rest of memory.

*/

ADDRESS_MAP_END

/* reads a bit 1 status there after every round played */
READ8_MEMBER(luckgrln_state::test_r)
{
	return 0xff;
}

static ADDRESS_MAP_START( _7smash_io, AS_IO, 8, luckgrln_state )
	AM_RANGE(0x66, 0x66) AM_READ(test_r)
	AM_IMPORT_FROM( portmap )
ADDRESS_MAP_END

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
	PORT_DIPNAME( 0x01, 0x01, "Auto Hold" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Game Type" )
	PORT_DIPSETTING(    0x02, "Hold Game" )
	PORT_DIPSETTING(    0x00, "Discard Game" )
	PORT_DIPNAME( 0x04, 0x04, "Adult Content" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW1-08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Minimal Winning Hand" )
	PORT_DIPSETTING(    0x10, "Jacks or Better" )
	PORT_DIPSETTING(    0x00, "2 Pairs" )
	PORT_DIPNAME( 0x20, 0x20, "Minimum Bet" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, "DSW1-40 (Do Not Use)" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW1-80 (Do Not Use)" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2") //DIP SW 2
	PORT_DIPNAME( 0x01, 0x01, "DSW2-01 (Do Not Use)" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW2-02 (Do Not Use)" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW2-04 (Do Not Use)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW2-08 (Do Not Use)" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW2-10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW2-20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW2-40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW2-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x04, "Key In" )
	PORT_DIPSETTING(    0x07, "1 Pulse / 5 Credits" )
	PORT_DIPSETTING(    0x06, "1 Pulse / 10 Credits" )
	PORT_DIPSETTING(    0x05, "1 Pulse / 50 Credits" )
	PORT_DIPSETTING(    0x04, "1 Pulse / 100 Credits" )
	PORT_DIPSETTING(    0x03, "1 Pulse / 200 Credits" )
	PORT_DIPSETTING(    0x02, "1 Pulse / 300 Credits" )
	PORT_DIPSETTING(    0x01, "1 Pulse / 500 Credits" )
	PORT_DIPSETTING(    0x00, "1 Pulse / 1000 Credits" )
	PORT_DIPNAME( 0x38, 0x10, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x10, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin / 50 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "DSW3-40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW3-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0f, 0x04, "Coin C" )
	PORT_DIPSETTING(    0x0f, "10 Coins / 1 Credit" )
	PORT_DIPSETTING(    0x0e, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0d, "5 Coins / 2 Credits" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x04, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x70, 0x10, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x70, "10 Coins / 1 Credit" )
	PORT_DIPSETTING(    0x60, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "1 Coin / 200 Credits" )
	PORT_DIPNAME( 0x80, 0x80, "DSW4-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5") //DIP SW 5
	PORT_DIPNAME( 0x01, 0x01, "DSW5-01 (Do Not Use)" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW5-02 (Do Not Use)" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW5-04" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Double-Up" )
	PORT_DIPSETTING(    0x18, "Double-Up (Normal)" )
	PORT_DIPSETTING(    0x08, "Double-Up Poker" )
	PORT_DIPSETTING(    0x10, "Double-Up Bingo" )
	PORT_DIPSETTING(    0x00, "No Double-Up" )
	PORT_DIPNAME( 0x20, 0x20, "DSW5-20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW5-40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW5-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( _7smash )
	PORT_START("DSW1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
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

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
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

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
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

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_UNUSED  )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SLOT_STOP3 ) PORT_NAME("Slot 3 / Odds")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SLOT_STOP2 ) PORT_NAME("Slot 2 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SLOT_STOP1 ) PORT_NAME("Slot 1")
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
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
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

static GFXDECODE_START( luckgrln )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0x400, 64 )
	GFXDECODE_ENTRY( "reels", 0, tiles8x32_layout, 0, 64 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(luckgrln_state::luckgrln_irq)
{
	if(m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( luckgrln, luckgrln_state )
	MCFG_CPU_ADD("maincpu", Z180,8000000)
	MCFG_CPU_PROGRAM_MAP(mainmap)
	MCFG_CPU_IO_MAP(portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", luckgrln_state,  luckgrln_irq)

	MCFG_MC6845_ADD("crtc", H46505, "screen", 6000000/4) /* unknown clock, hand tuned to get ~60 fps */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(luckgrln_state, screen_update_luckgrln)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", luckgrln)
	MCFG_PALETTE_ADD("palette", 0x8000)

	MCFG_SPEAKER_STANDARD_MONO("mono")

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( _7smash, luckgrln )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(_7smash_map)
	MCFG_CPU_IO_MAP(_7smash_io)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(luckgrln_state,luckgrln)
{
	int i;
	UINT8 x,v;
	UINT8* rom = memregion("rom_data")->base();

	for (i=0;i<0x20000;i++)
	{
		x = rom[i];
		v = 0xfe + (i & 0xf)*0x3b + ((i >> 4) & 0xf)*0x9c + ((i >> 8) & 0xf)*0xe1 + ((i >> 12) & 0x7)*0x10;
		v += ((((i >> 4) & 0xf) + ((i >> 2) & 3)) >> 2) * 0x50;
		x ^= ~v;
		x = (x << (i & 7)) | (x >> (8-(i & 7)));
		rom[i] = x;
	}

	#if 0
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"decrypted_%s", machine().system().name);
		fp=fopen(filename, "w+b");
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


/*********************************************
*                Game Drivers                *
**********************************************

       YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT      ROT    COMPANY           FULLNAME                                 FLAGS                                                       LAYOUT  */
GAMEL( 1991, luckgrln, 0,      luckgrln, luckgrln, luckgrln_state, luckgrln, ROT0, "Wing Co., Ltd.", "Lucky Girl (newer Z180 based hardware)", MACHINE_NO_SOUND,                                              layout_luckgrln )
GAMEL( 1993, 7smash,   0,      _7smash,  _7smash,  driver_device,  0,        ROT0, "Sovic",          "7 Smash",                                MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_luckgrln )
