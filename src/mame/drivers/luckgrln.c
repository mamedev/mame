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

static UINT8 *luck_vram1,*luck_vram2,*luck_vram3;
static UINT8 nmi_enable;
static tilemap_t *reel1_tilemap, *reel2_tilemap, *reel3_tilemap, *reel4_tilemap;
static UINT8* reel1_ram;
static UINT8* reel2_ram;
static UINT8* reel3_ram;
static UINT8* reel4_ram;
static UINT8* reel1_scroll;
static UINT8* reel2_scroll;
static UINT8* reel3_scroll;
static UINT8* reel4_scroll;
static UINT8* reel1_attr;
static UINT8* reel2_attr;
static UINT8* reel3_attr;
static UINT8* reel4_attr;



static WRITE8_HANDLER( luckgrln_reel1_ram_w )
{
	reel1_ram[offset] = data;
	tilemap_mark_tile_dirty(reel1_tilemap,offset);
}

static WRITE8_HANDLER( luckgrln_reel1_attr_w )
{
	reel1_attr[offset] = data;
	tilemap_mark_tile_dirty(reel1_tilemap,offset);
}



static TILE_GET_INFO( get_luckgrln_reel1_tile_info )
{
	int code = reel1_ram[tile_index];
	int attr = reel1_attr[tile_index];
	int col = (attr & 0x03)<<1;
	col |= (attr & 0x4)>>2;

	code |= (attr & 0xe0)<<3;


	SET_TILE_INFO(
			1,
			code,
			col,
			0);
}


static WRITE8_HANDLER( luckgrln_reel2_ram_w )
{
	reel2_ram[offset] = data;
	tilemap_mark_tile_dirty(reel2_tilemap,offset);
}

static WRITE8_HANDLER( luckgrln_reel2_attr_w )
{
	reel2_attr[offset] = data;
	tilemap_mark_tile_dirty(reel2_tilemap,offset);
}


static TILE_GET_INFO( get_luckgrln_reel2_tile_info )
{
	int code = reel2_ram[tile_index];
	int attr = reel2_attr[tile_index];
	int col = (attr & 0x03)<<1;
	col |= (attr & 0x4)>>2;

	code |= (attr & 0xe0)<<3;


	SET_TILE_INFO(
			1,
			code,
			col,
			0);
}

static WRITE8_HANDLER( luckgrln_reel3_ram_w )
{
	reel3_ram[offset] = data;
	tilemap_mark_tile_dirty(reel3_tilemap,offset);
}

static WRITE8_HANDLER( luckgrln_reel3_attr_w )
{
	reel3_attr[offset] = data;
	tilemap_mark_tile_dirty(reel3_tilemap,offset);
}


static TILE_GET_INFO( get_luckgrln_reel3_tile_info )
{
	int code = reel3_ram[tile_index];
	int attr = reel3_attr[tile_index];
	int col = (attr & 0x03)<<1;
	col |= (attr & 0x4)>>2;

	code |= (attr & 0xe0)<<3;

	SET_TILE_INFO(
			1,
			code,
			col,
			0);
}

static WRITE8_HANDLER( luckgrln_reel4_ram_w )
{
	reel4_ram[offset] = data;
	tilemap_mark_tile_dirty(reel4_tilemap,offset);
}

static WRITE8_HANDLER( luckgrln_reel4_attr_w )
{
	reel4_attr[offset] = data;
	tilemap_mark_tile_dirty(reel4_tilemap,offset);
}


static TILE_GET_INFO( get_luckgrln_reel4_tile_info )
{
	int code = reel4_ram[tile_index];
	int attr = reel4_attr[tile_index];
	int col = (attr & 0x03)<<1;
	col |= (attr & 0x4)>>2;

	code |= (attr & 0xe0)<<3;

	SET_TILE_INFO(
			1,
			code,
			col,
			0);
}

static VIDEO_START(luckgrln)
{
	reel1_tilemap = tilemap_create(machine,get_luckgrln_reel1_tile_info,tilemap_scan_rows, 8, 32, 64, 8);
	reel2_tilemap = tilemap_create(machine,get_luckgrln_reel2_tile_info,tilemap_scan_rows, 8, 32, 64, 8);
	reel3_tilemap = tilemap_create(machine,get_luckgrln_reel3_tile_info,tilemap_scan_rows, 8, 32, 64, 8);
	reel4_tilemap = tilemap_create(machine,get_luckgrln_reel4_tile_info,tilemap_scan_rows, 8, 32, 64, 8);

	//machine->gfx[0]->color_granularity = 16;

}

static VIDEO_UPDATE(luckgrln)
{
	int y,x;
	int count = 0;
	const rectangle *visarea = video_screen_get_visible_area(screen);
	int i;

	rectangle clip;

	bitmap_fill(bitmap, cliprect, 0x3f); /* is there a register controling the colour?, we currently use the last colour of the tx palette */

	clip.min_x = visarea->min_x;
	clip.max_x = visarea->max_x;
	clip.min_y = visarea->min_y;
	clip.max_y = visarea->max_y;

	bitmap_fill(bitmap, cliprect, 0);

	for (i= 0;i < 64;i++)
	{
		tilemap_set_scrolly(reel1_tilemap, i, reel1_scroll[i]);
		tilemap_set_scrolly(reel2_tilemap, i, reel2_scroll[i]);
		tilemap_set_scrolly(reel3_tilemap, i, reel3_scroll[i]);
		tilemap_set_scrolly(reel4_tilemap, i, reel4_scroll[i]);
	}


	for (y=0;y<32;y++)
	{
		clip.min_y = y*8;
		clip.max_y = y*8+8;

		if (clip.min_y<visarea->min_y) clip.min_y = visarea->min_y;
		if (clip.max_y>visarea->max_y) clip.max_y = visarea->max_y;

		for (x=0;x<64;x++)
		{
			UINT16 tile = (luck_vram1[count] & 0xff);
			UINT16 tile_high = (luck_vram2[count]);
			UINT16 tileattr = (luck_vram3[count]);
			UINT8 col = 0;
			UINT8 region = 0;
			UINT8 bgenable;

			clip.min_x = x*8;
			clip.max_x = x*8+8;

			if (clip.min_x<visarea->min_x) clip.min_x = visarea->min_x;
			if (clip.max_x>visarea->max_x) clip.max_x = visarea->max_x;

			/*
			  luck_vram1  tttt tttt   (t = low tile bits)
			  luck_vram2  tttt ppp?   (t = high tile bits) (p = pal select)?
              luck_vram3  --ee --t-   (t = tile bank) (e = select which of the reel tiles to display here too?)


			 */

			tile |= (tile_high & 0xf0) << 4;
			if (tileattr & 0x02) tile |= 0x1000;

			// ?? low bit is used too
			col = (tile_high>>1)&0x7;
			col += 8;

			// other bits are used.. do they affect palette of reels, or..?
			bgenable = (tileattr &0x30)>>4;


			if (bgenable==0) tilemap_draw(bitmap, &clip, reel1_tilemap, 0, 0);
			if (bgenable==1) tilemap_draw(bitmap, &clip, reel2_tilemap, 0, 0);
			if (bgenable==2) tilemap_draw(bitmap, &clip, reel3_tilemap, 0, 0);
			if (bgenable==3) tilemap_draw(bitmap, &clip, reel4_tilemap, 0, 0);

			drawgfx_transpen(bitmap,&clip,screen->machine->gfx[region],tile,col,0,0,x*8,y*8, 0);

			count++;
		}
	}
	return 0;
}

static ADDRESS_MAP_START( mainmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x03fff) AM_ROM
	AM_RANGE(0x10000, 0x1ffff) AM_ROM AM_REGION("rom_data",0x10000)
	AM_RANGE(0x20000, 0x2ffff) AM_ROM AM_REGION("rom_data",0x00000)

	AM_RANGE(0x0c000, 0x0c1ff) AM_RAM_WRITE(luckgrln_reel1_ram_w)  AM_BASE(&reel1_ram) // only written to half way
	AM_RANGE(0x0c800, 0x0c9ff) AM_RAM_WRITE(luckgrln_reel1_attr_w) AM_BASE(&reel1_attr)
	AM_RANGE(0x0d000, 0x0d03f) AM_RAM AM_BASE(&reel1_scroll) AM_MIRROR(0x000c0)

	AM_RANGE(0x0c200, 0x0c3ff) AM_RAM_WRITE(luckgrln_reel2_ram_w)  AM_BASE(&reel2_ram)
	AM_RANGE(0x0ca00, 0x0cbff) AM_RAM_WRITE(luckgrln_reel2_attr_w) AM_BASE(&reel2_attr)
	AM_RANGE(0x0d200, 0x0d23f) AM_RAM AM_BASE(&reel2_scroll) AM_MIRROR(0x000c0)

	AM_RANGE(0x0c400, 0x0c5ff) AM_RAM_WRITE(luckgrln_reel3_ram_w ) AM_BASE(&reel3_ram)
	AM_RANGE(0x0cc00, 0x0cdff) AM_RAM_WRITE(luckgrln_reel3_attr_w) AM_BASE(&reel3_attr)
	AM_RANGE(0x0d400, 0x0d43f) AM_RAM AM_BASE(&reel3_scroll) AM_MIRROR(0x000c0)

	AM_RANGE(0x0c600, 0x0c7ff) AM_RAM_WRITE(luckgrln_reel4_ram_w ) AM_BASE(&reel4_ram)
	AM_RANGE(0x0ce00, 0x0cfff) AM_RAM_WRITE(luckgrln_reel4_attr_w) AM_BASE(&reel4_attr)
	AM_RANGE(0x0d600, 0x0d63f) AM_RAM AM_BASE(&reel4_scroll)

//	AM_RANGE(0x0d200, 0x0d2ff) AM_RAM


	AM_RANGE(0x0d800, 0x0dfff) AM_RAM // nvram

	AM_RANGE(0x0e000, 0x0e7ff) AM_RAM AM_BASE(&luck_vram1)
	AM_RANGE(0x0e800, 0x0efff) AM_RAM AM_BASE(&luck_vram2)
	AM_RANGE(0x0f000, 0x0f7ff) AM_RAM AM_BASE(&luck_vram3)


	AM_RANGE(0x0f800, 0x0ffff) AM_RAM
	AM_RANGE(0xf0000, 0xfffff) AM_RAM
ADDRESS_MAP_END

#if 1
static WRITE8_HANDLER( output_w )
{
	/* correct? */
	if (data==0x84)
		nmi_enable = 0;
	else if (data==0x85)
		nmi_enable = 1;
	else
		printf("output_w unk data %02x\n",data);

	// other values unknown
//  printf("%02x\n",data);
}
#endif

static int palette_count = 0;
static UINT8 palette_ram[0x10000];

static WRITE8_HANDLER( palette_offset_low_w )
{
	palette_count = data;
}
static WRITE8_HANDLER( palette_offset_high_w )
{
	palette_count = palette_count | data<<8;
}


static WRITE8_HANDLER( palette_w )
{
	palette_ram[palette_count] = data;


	{
		int r,g,b;
		int offs;
		UINT16 dat;
		offs = palette_count&~0x1;
		dat = palette_ram[offs] | palette_ram[offs+1]<<8;

		r = (dat >> 0) & 0x1f;
		g = (dat >> 5) & 0x1f;
		b = (dat >> 10) & 0x1f;

		palette_set_color_rgb(space->machine, offs/2, pal5bit(r), pal5bit(g), pal5bit(b));

	}

	palette_count++;

}

// Oki M62X428 is a 4-bit RTC, doesn't seem to be millennium bug proof ...
static READ8_HANDLER( rtc_r )
{
	mame_system_time systime;
	mame_get_base_datetime(space->machine, &systime);

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

/* are some of these reads / writes mirrored? there seem to be far too many */
static ADDRESS_MAP_START( portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff) // i think
	AM_RANGE(0x0000, 0x003f) AM_RAM // Z180 internal regs
	AM_RANGE(0x0060, 0x0060) AM_WRITE(output_w)

	AM_RANGE(0x0090, 0x009f) AM_READ(rtc_r) AM_WRITENOP

	AM_RANGE(0x00a0, 0x00a0) AM_WRITE(palette_offset_low_w)
	AM_RANGE(0x00a1, 0x00a1) AM_WRITE(palette_offset_high_w)
	AM_RANGE(0x00a2, 0x00a2) AM_WRITE(palette_w)

	AM_RANGE(0x00b0, 0x00b0) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x00b1, 0x00b1) AM_DEVWRITE("crtc", mc6845_register_w)

	AM_RANGE(0x00b8, 0x00b8) AM_READ_PORT("IN0")
	AM_RANGE(0x00b9, 0x00b9) AM_READ_PORT("IN1") AM_WRITENOP // coin counters are here
	AM_RANGE(0x00ba, 0x00ba) AM_READ_PORT("IN2")
	AM_RANGE(0x00bb, 0x00bb) AM_READ_PORT("IN3") AM_WRITENOP // coin lockouts?
	AM_RANGE(0x00bc, 0x00bc) AM_READ_PORT("IN4")

	AM_RANGE(0x00c3, 0x00c3) AM_WRITENOP
	AM_RANGE(0x00c7, 0x00c7) AM_WRITENOP
	AM_RANGE(0x00cb, 0x00cb) AM_WRITENOP
	AM_RANGE(0x00cf, 0x00cf) AM_WRITENOP

	AM_RANGE(0x00d3, 0x00d3) AM_WRITENOP
	AM_RANGE(0x00d7, 0x00d7) AM_WRITENOP
	AM_RANGE(0x00db, 0x00db) AM_WRITENOP
	AM_RANGE(0x00df, 0x00df) AM_WRITENOP

	AM_RANGE(0x00e4, 0x00e7) AM_WRITENOP

	AM_RANGE(0x00f3, 0x00f3) AM_WRITENOP
	AM_RANGE(0x00f7, 0x00f7) AM_WRITENOP

	AM_RANGE(0x00f8, 0x00f8) AM_READ_PORT("DSW0")
	AM_RANGE(0x00f9, 0x00f9) AM_READ_PORT("DSW1")
	AM_RANGE(0x00fa, 0x00fa) AM_READ_PORT("DSW2")
	AM_RANGE(0x00fb, 0x00fb) AM_READ_PORT("DSW3") AM_WRITENOP
	AM_RANGE(0x00fc, 0x00fc) AM_WRITENOP
	AM_RANGE(0x00fd, 0x00fd) AM_WRITENOP
	AM_RANGE(0x00fe, 0x00fe) AM_WRITENOP
	AM_RANGE(0x00ff, 0x00ff) AM_WRITENOP


ADDRESS_MAP_END


static INPUT_PORTS_START( luckgrln )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_CODE(KEYCODE_1) PORT_NAME("Start")
	PORT_DIPNAME( 0x02, 0x02, "IN0" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Reset")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME( 0x04, 0x04, "IN3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Credit Clear")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Books SW")

	PORT_START("IN4") //DIP SW 1
	PORT_DIPNAME( 0x01, 0x01, "IN4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) //adult content
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
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) //causes POST errors otherwise

	PORT_START("DSW0") //DIP SW 2
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED ) //causes POST errors otherwise
	PORT_DIPNAME( 0x10, 0x10, "DSW0" )
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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "IN6" )
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "IN7" )
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

	PORT_START("DSW3") //DIP SW 5
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED ) //causes POST errors otherwise
	PORT_DIPNAME( 0x04, 0x04, "DSW3" )
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
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0x100, 64 )
	GFXDECODE_ENTRY( "reels", 0, tiles8x32_layout, 0, 64 )
GFXDECODE_END

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};

static INTERRUPT_GEN( luckgrln_irq )
{
	#if 1
	//nmi_enable = 1;
	if(nmi_enable)
		cputag_set_input_line(device->machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE);
	#endif
}

static MACHINE_DRIVER_START( luckgrln )
	MDRV_CPU_ADD("maincpu", Z180,8000000)
	MDRV_CPU_PROGRAM_MAP(mainmap)
	MDRV_CPU_IO_MAP(portmap)
	MDRV_CPU_VBLANK_INT("screen", luckgrln_irq)

	MDRV_MC6845_ADD("crtc", H46505, 6000000/4, mc6845_intf)	/* unknown clock, hand tuned to get ~60 fps */

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)

	MDRV_GFXDECODE(luckgrln)
	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_VIDEO_START(luckgrln)
	MDRV_VIDEO_UPDATE(luckgrln)

	MDRV_SPEAKER_STANDARD_MONO("mono")

MACHINE_DRIVER_END

static DRIVER_INIT( luckgrln )
{
	int i;
	UINT8 x,v;
	UINT8* rom = memory_region(machine,"rom_data");

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
		sprintf(filename,"decrypted_%s", machine->gamedrv->name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(rom, 0x20000, 1, fp);
			fclose(fp);
		}
	}
	#endif

	// ??
//  memory_set_bankptr(machine, "bank1",&rom[0x010000]);
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

GAME( 1991, luckgrln,  0,    luckgrln, luckgrln,  luckgrln, ROT0, "Wing Co., Ltd.", "Lucky Girl (newer Z180 based hardware)", GAME_IMPERFECT_GRAPHICS|GAME_NO_SOUND )

