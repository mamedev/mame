/*
"CGA" Amcoe HW (c) 1999-2004 Amcoe

Notes:
- Some games requires a password, it's 123456 for Robin's Adventure, might be the same for the others.
*/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"

static tilemap *sfbonus_tilemap;
static tilemap *sfbonus_reel_tilemap;
static tilemap *sfbonus_reel2_tilemap;
static tilemap *sfbonus_reel3_tilemap;
static tilemap *sfbonus_reel4_tilemap;
static UINT8 *sfbonus_tilemap_ram;
static UINT8 *sfbonus_reel_ram;
static UINT8 *sfbonus_reel2_ram;
static UINT8 *sfbonus_reel3_ram;
static UINT8 *sfbonus_reel4_ram;
static UINT8* sfbonus_videoram;
static UINT8 *sfbonus_vregs;

static TILE_GET_INFO( get_sfbonus_tile_info )
{
	int code = sfbonus_tilemap_ram[(tile_index*2)+0] | (sfbonus_tilemap_ram[(tile_index*2)+1]<<8);
	int flipx = (sfbonus_tilemap_ram[(tile_index*2)+1] & 0x80)>>7;
	// bit 6 might be flipy

	SET_TILE_INFO(
			0,
			code,
			0,
			TILE_FLIPYX(flipx));
}

static TILE_GET_INFO( get_sfbonus_reel_tile_info )
{
	int code = sfbonus_reel_ram[(tile_index*2)+0] | (sfbonus_reel_ram[(tile_index*2)+1]<<8);
	int flipx = (sfbonus_reel_ram[(tile_index*2)+1] & 0x80)>>7;

	SET_TILE_INFO(
			1,
			code,
			0,
			TILE_FLIPYX(flipx));
}

static TILE_GET_INFO( get_sfbonus_reel2_tile_info )
{
	int code = sfbonus_reel2_ram[(tile_index*2)+0] | (sfbonus_reel2_ram[(tile_index*2)+1]<<8);
	int flipx = (sfbonus_reel2_ram[(tile_index*2)+1] & 0x80)>>7;

	SET_TILE_INFO(
			1,
			code,
			0,
			TILE_FLIPYX(flipx));
}

static TILE_GET_INFO( get_sfbonus_reel3_tile_info )
{
	int code = sfbonus_reel3_ram[(tile_index*2)+0] | (sfbonus_reel3_ram[(tile_index*2)+1]<<8);
	int flipx = (sfbonus_reel3_ram[(tile_index*2)+1] & 0x80)>>7;

	SET_TILE_INFO(
			1,
			code,
			0,
			TILE_FLIPYX(flipx));
}

static TILE_GET_INFO( get_sfbonus_reel4_tile_info )
{
	int code = sfbonus_reel4_ram[(tile_index*2)+0] | (sfbonus_reel4_ram[(tile_index*2)+1]<<8);
	int flipx = (sfbonus_reel4_ram[(tile_index*2)+1] & 0x80)>>7;

	SET_TILE_INFO(
			1,
			code,
			0,
			TILE_FLIPYX(flipx));
}


static WRITE8_HANDLER( sfbonus_videoram_w )
{
	if (offset<0x4000) /* 0x0000 - 0x3fff */
	{
		sfbonus_tilemap_ram[offset] = data;
		tilemap_mark_tile_dirty(sfbonus_tilemap,offset/2);
	}
	else if (offset<0x4800) /* 0x4000 - 0x47ff */
	{
		offset-=0x4000;

		sfbonus_reel_ram[offset] = data;
		tilemap_mark_tile_dirty(sfbonus_reel_tilemap,offset/2);
	}
	else if (offset<0x5000)  /* 0x4800 - 0x4fff */
	{
		offset-=0x4800;

		sfbonus_reel2_ram[offset] = data;
		tilemap_mark_tile_dirty(sfbonus_reel2_tilemap,offset/2);
	}
	else if (offset<0x5800) /* 0x5000 - 0x57ff */
	{
		offset-=0x5000;

		sfbonus_reel3_ram[offset] = data;
		tilemap_mark_tile_dirty(sfbonus_reel3_tilemap,offset/2);
	}
	else if (offset<0x6000) /* 0x5800 - 0x5fff */
	{
		offset-=0x5800;

		sfbonus_reel4_ram[offset] = data;
		tilemap_mark_tile_dirty(sfbonus_reel4_tilemap,offset/2);
	}
	else if (offset<0x8000)
	{
		offset -=0x6000;
		// scroll regs etc.
		//printf("access vram at %04x\n",offset);
		sfbonus_videoram[offset] = data;
	}
	else
	{
		printf("access vram at %04x\n",offset);
	}

}



VIDEO_START(sfbonus)
{
	sfbonus_tilemap = tilemap_create(machine,get_sfbonus_tile_info,tilemap_scan_rows,8,8, 128, 64);
	sfbonus_reel_tilemap = tilemap_create(machine,get_sfbonus_reel_tile_info,tilemap_scan_rows,8,32, 64, 16);
	sfbonus_reel2_tilemap = tilemap_create(machine,get_sfbonus_reel2_tile_info,tilemap_scan_rows,8,32, 64, 16);
	sfbonus_reel3_tilemap = tilemap_create(machine,get_sfbonus_reel3_tile_info,tilemap_scan_rows,8,32, 64, 16);
	sfbonus_reel4_tilemap = tilemap_create(machine,get_sfbonus_reel4_tile_info,tilemap_scan_rows,8,32, 64, 16);

	tilemap_set_transparent_pen(sfbonus_tilemap,0);
	tilemap_set_transparent_pen(sfbonus_reel_tilemap,255);
	tilemap_set_transparent_pen(sfbonus_reel2_tilemap,255);
	tilemap_set_transparent_pen(sfbonus_reel3_tilemap,255);
	tilemap_set_transparent_pen(sfbonus_reel4_tilemap,255);

	tilemap_set_scroll_cols(sfbonus_reel_tilemap, 64);
	tilemap_set_scroll_cols(sfbonus_reel2_tilemap, 64);
	tilemap_set_scroll_cols(sfbonus_reel3_tilemap, 64);
	tilemap_set_scroll_cols(sfbonus_reel4_tilemap, 64);
}


VIDEO_UPDATE(sfbonus)
{
//	int y,x;
//	int count = 0;
//	const gfx_element *gfx2 = screen->machine->gfx[1];
	tilemap_set_scrolly(sfbonus_tilemap, 0, (sfbonus_vregs[2] | sfbonus_vregs[3]<<8));
#if 0
	tilemap_set_scrolly(sfbonus_reel_tilemap, 0, (sfbonus_vregs[6] | sfbonus_vregs[7]<<8));
#endif
	bitmap_fill(bitmap,cliprect,screen->machine->pens[0]);
//	tilemap_draw(bitmap,cliprect,sfbonus_reel_tilemap,0,0);

	{
		int zz;
		int i;
		int startclipmin = 0;
		const rectangle *visarea = video_screen_get_visible_area(screen);
		UINT8* selectbase = &sfbonus_videoram[0x600];
		UINT8* bg_scroll = &sfbonus_videoram[0x000];

		for (i= 0;i < 0x80;i++)
		{
			int scroll;
			scroll = bg_scroll[(i*2)+0x000] | (bg_scroll[(i*2)+0x001]<<8);
			tilemap_set_scrolly(sfbonus_reel_tilemap, i, scroll);

			scroll = bg_scroll[(i*2)+0x080] | (bg_scroll[(i*2)+0x081]<<8);
			tilemap_set_scrolly(sfbonus_reel2_tilemap, i, scroll);

			scroll = bg_scroll[(i*2)+0x100] | (bg_scroll[(i*2)+0x101]<<8);
			tilemap_set_scrolly(sfbonus_reel3_tilemap, i, scroll);

			scroll = bg_scroll[(i*2)+0x180] | (bg_scroll[(i*2)+0x181]<<8);
			tilemap_set_scrolly(sfbonus_reel4_tilemap, i, scroll);
		}




		for (zz=0;zz<0x100;zz++) // -8 because of visible area (2*8 = 16)
		{
			rectangle clip;
			int rowenable = selectbase[zz];

			/* draw top of screen */
			clip.min_x = visarea->min_x;
			clip.max_x = visarea->max_x;
			clip.min_y = startclipmin;
			clip.max_y = startclipmin+1;

			if (rowenable==0)
			{
				tilemap_draw(bitmap,&clip,sfbonus_reel_tilemap,0,0);
			}
			else if (rowenable==0x5)
			{
				tilemap_draw(bitmap,&clip,sfbonus_reel2_tilemap,0,0);
			}
			else if (rowenable==0xa)
			{
				tilemap_draw(bitmap,&clip,sfbonus_reel3_tilemap,0,0);
			}
			else if (rowenable==0xf)
			{
				tilemap_draw(bitmap,&clip,sfbonus_reel4_tilemap,0,0);
			}
			else
			{
				bitmap_fill(bitmap,&clip,screen->machine->pens[rowenable]);
			}

			startclipmin+=1;
		}


		tilemap_draw(bitmap,cliprect,sfbonus_tilemap,0,0);
	}

//	popmessage("%02x %02x %02x %02x %02x %02x %02x %02x %d",sfbonus_vregs[0+test_vregs],sfbonus_vregs[1+test_vregs],sfbonus_vregs[2+test_vregs],
//	sfbonus_vregs[3+test_vregs],sfbonus_vregs[4+test_vregs],sfbonus_vregs[5+test_vregs],sfbonus_vregs[6+test_vregs],sfbonus_vregs[7+test_vregs],test_vregs);

	/*
	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			UINT16 tiledat = sfbonus_videoram[count] | (sfbonus_videoram[count+1]<<8);

			drawgfx(bitmap,gfx2,tiledat,0,0,0,x*8,y*32,cliprect,TRANSPARENCY_PEN,255);
			count+=2;
		}

	}
	*/
	return 0;
}


static WRITE8_HANDLER( paletteram_io_w )
{
	static int pal_offs,r,g,b,internal_pal_offs;

	switch(offset)
	{
		case 0:
			pal_offs = data;
			break;
		case 2:
			internal_pal_offs = 0;
			break;
		case 1:
			switch(internal_pal_offs)
			{
				case 0:
					r = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 1:
					g = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 2:
					b = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					palette_set_color(space->machine, pal_offs, MAKE_RGB(r, g, b));
					internal_pal_offs = 0;
					pal_offs++;
					break;
			}

			break;
	}
}



static ADDRESS_MAP_START( sfbonus_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xefff) AM_ROMBANK(1) AM_WRITE(sfbonus_videoram_w)
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static READ8_HANDLER( sfbonus_unk_r )
{
	return mame_rand(space->machine);
}

static WRITE8_HANDLER( sfbonus_bank_w )
{
	UINT8 *ROM = memory_region(space->machine, "main");
	UINT8 bank;

	bank = data & 3;

	memory_set_bankptr(space->machine, 1, &ROM[bank * 0x10000]);
}

static ADDRESS_MAP_START( sfbonus_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0400, 0x0400) AM_READ_PORT("IN0")
	AM_RANGE(0x0408, 0x0408) AM_READ_PORT("IN1")
	AM_RANGE(0x0410, 0x0410) AM_READ_PORT("IN2")

	AM_RANGE(0x0418, 0x0418) AM_READ_PORT("IN4")
	AM_RANGE(0x0420, 0x0420) AM_READ_PORT("IN5")
	AM_RANGE(0x0428, 0x0428) AM_READ_PORT("IN6")
	AM_RANGE(0x0430, 0x0430) AM_READ_PORT("IN7")

	AM_RANGE(0x0438, 0x0438) AM_READ_PORT("IN3")

	AM_RANGE(0x0800, 0x0800) AM_DEVREADWRITE(SOUND, "oki", okim6295_r, okim6295_w)

	AM_RANGE(0x0c00, 0x0c03) AM_WRITE( paletteram_io_w )

	AM_RANGE(0x2400, 0x241f) AM_RAM AM_BASE(&sfbonus_vregs)

	AM_RANGE(0x2800, 0x2800) AM_READ(sfbonus_unk_r)
	AM_RANGE(0x2801, 0x2801) AM_READ(sfbonus_unk_r)	AM_WRITE(SMH_NOP)

	AM_RANGE(0x2c00, 0x2c00) AM_READ(sfbonus_unk_r)
	AM_RANGE(0x2c01, 0x2c01) AM_READ(sfbonus_unk_r) AM_WRITE(SMH_NOP)

	AM_RANGE(0x3000, 0x3000) AM_WRITE(SMH_NOP)
	AM_RANGE(0x3400, 0x3400) AM_WRITE(sfbonus_bank_w)
	AM_RANGE(0x3800, 0x3800) AM_READ(sfbonus_unk_r) AM_WRITE(SMH_NOP)

	AM_RANGE(0x1800, 0x1800) AM_WRITE(SMH_NOP)
	AM_RANGE(0x1801, 0x1801) AM_WRITE(SMH_NOP)
	AM_RANGE(0x1802, 0x1802) AM_WRITE(SMH_NOP)
	AM_RANGE(0x1803, 0x1803) AM_WRITE(SMH_NOP)
	AM_RANGE(0x1804, 0x1804) AM_WRITE(SMH_NOP)
	AM_RANGE(0x1805, 0x1805) AM_WRITE(SMH_NOP)
	AM_RANGE(0x1806, 0x1806) AM_WRITE(SMH_NOP)

	AM_RANGE(0x3801, 0x3801) AM_WRITE(SMH_NOP)
	AM_RANGE(0x3802, 0x3802) AM_WRITE(SMH_NOP)
	AM_RANGE(0x3803, 0x3803) AM_WRITE(SMH_NOP)
	AM_RANGE(0x3806, 0x3806) AM_WRITE(SMH_NOP)
	AM_RANGE(0x3807, 0x3807) AM_WRITE(SMH_NOP)
ADDRESS_MAP_END

/* 8-liners input define */
static INPUT_PORTS_START( sfbonus )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) // credit clear
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) //causes "printer busy" msg.
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Account Test")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Port Test")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 ) //probably some kind of service button or remote
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Stop 2") PORT_CODE(KEYCODE_X)
	PORT_DIPNAME( 0x02, 0x02, "IN1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Stop 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Stop 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("All Stop") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x02, 0x02, "IN2" )
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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Help") PORT_CODE(KEYCODE_H)
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3" )
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
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Move Reel") PORT_CODE(KEYCODE_Q)
	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4" )
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
	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, "IN5" )
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
	PORT_START("IN6")
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
	PORT_START("IN7")
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
INPUT_PORTS_END

/* pokers input define */
static INPUT_PORTS_START( parrot3 )
	PORT_INCLUDE(sfbonus)
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold 4 / Small") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold 1 / Bet") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold 3 / W-Up") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Cancel All") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold 2 / Big") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout sfbonus_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64 },
	8*64
};

static const gfx_layout sfbonus32_layout =
{
	8,32,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64,
      8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64,
	  16*64,17*64,18*64,19*64,20*64,21*64,22*64,23*64,
	  24*64,25*64,26*64,27*64,28*64,29*64,30*64,31*64
	  },
	32*64
};



static GFXDECODE_START( sfbonus )
	GFXDECODE_ENTRY( "gfx1", 0, sfbonus_layout,   0x0, 2  )
	GFXDECODE_ENTRY( "gfx2", 0, sfbonus32_layout,   0x0, 2  )
GFXDECODE_END


static MACHINE_RESET( sfbonus )
{
	UINT8 *ROM = memory_region(machine, "main");

	memory_set_bankptr(machine, 1, &ROM[0]);
}

static MACHINE_DRIVER_START( sfbonus )
	MDRV_CPU_ADD("main", Z80, 16000000) // unknown CPU
	MDRV_CPU_PROGRAM_MAP(0,sfbonus_map)
	MDRV_CPU_IO_MAP(0,sfbonus_io)
	MDRV_CPU_VBLANK_INT("main",irq0_line_hold)
//	MDRV_CPU_PERIODIC_INT(nmi_line_pulse,100)

	MDRV_MACHINE_RESET( sfbonus )

	MDRV_GFXDECODE(sfbonus)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(128*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 128*8-1, 0*8, 64*8-1)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(sfbonus)
	MDRV_VIDEO_UPDATE(sfbonus)

	/* Parrot 3 seems fine at 1 Mhz, but Double Challenge isn't? */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("oki", OKIM6295, 1000000)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_DRIVER_END

// the gfx2 roms might be swapped on these sets
ROM_START( sfbonus )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80  Code */
	ROM_LOAD( "skfb16.bin", 0x00000, 0x40000, CRC(bfd53646) SHA1(bd58f8c6d5386649a6fc0f4bac46d1b6cd6248b1) )

	//ROM_REGION( 0x10000, "user1", 0 ) /* Z80  Code */

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "skfbrom2.bin", 0x00000, 0x20000, CRC(3823a36e) SHA1(4136e380b63546b9490033ad26d776f326eb9290) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "skfbrom3.bin", 0x00000, 0x80000, CRC(36119517) SHA1(241bb256ab3ba595dcb0c81fd2e60ed35dd7c197) )
	ROM_LOAD16_BYTE( "skfbrom4.bin", 0x00001, 0x80000, CRC(a655bac1) SHA1(0faea01c09409f9182f08370dcc0b466a799f17f) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "skfbrom5.bin", 0x00000, 0x80000, CRC(752e6e3b) SHA1(46c3a1bbbf1a2afe36fa5333b6e74459e17e9bae) )
	ROM_LOAD16_BYTE( "skfbrom6.bin", 0x00001, 0x80000, CRC(30df6b6a) SHA1(7a180fa8ee64b9efb0321baffad72f0a9485d568) )
ROM_END

ROM_START( sfbonusa )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80  Code */
	ROM_LOAD( "skfb17.bin", 0x00000, 0x40000, CRC(e28ede82) SHA1(f320c4c9c30ec280ee2437d1ad4d2b6270580916) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "skfbrom2.bin", 0x00000, 0x20000, CRC(3823a36e) SHA1(4136e380b63546b9490033ad26d776f326eb9290) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "skfbrom3.bin", 0x00000, 0x80000, CRC(36119517) SHA1(241bb256ab3ba595dcb0c81fd2e60ed35dd7c197) )
	ROM_LOAD16_BYTE( "skfbrom4.bin", 0x00001, 0x80000, CRC(a655bac1) SHA1(0faea01c09409f9182f08370dcc0b466a799f17f) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "skfbrom5a.bin", 0x00000, 0x80000, CRC(ed07a635) SHA1(a31ee06d8cb78c43affa9d899e9cec87a0875934) )
	ROM_LOAD16_BYTE( "skfbrom6a.bin", 0x00001, 0x80000, CRC(b9a39e5a) SHA1(cb875db25d8bccf48542bd6dbecc474b1205563b) )
ROM_END

ROM_START( parrot3 )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "p4p24.bin", 0x00000, 0x40000, CRC(356a49c8) SHA1(7e0ed7d1063675b66bfe28c427712249654be6ab) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "p4rom2.bin", 0x00000, 0x40000, CRC(d0574efc) SHA1(dd6628450883f0f723744e7caf6525bca7b18a43) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "p4rom3.bin", 0x00000, 0x80000, CRC(c5fc21cb) SHA1(b4137a97611ff688fbfa688eb3108622bed8da5b) )
	ROM_LOAD16_BYTE( "p4rom4.bin", 0x00001, 0x80000, CRC(bbe174d3) SHA1(75d964d37470843962419ead170f1db9a1dcc4c4) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "p4rom5.bin", 0x00000, 0x80000, CRC(5e184b6e) SHA1(a00eb5a62246ec00e1af6e8c0629a118f71f0c58) )
	ROM_LOAD16_BYTE( "p4rom6.bin", 0x00001, 0x80000, CRC(598d2117) SHA1(8391054aa8deb8480a69de97b8f5316e7864ed2d) )
ROM_END

ROM_START( hldspin1 )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "hs1p25t.bin", 0x00000, 0x40000, CRC(0fce5691) SHA1(4920ee490fdd690987bee92525b48596a051f83d) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "hs_2.bin", 0x00000, 0x40000, CRC(3b0f5374) SHA1(7e7b185b62d1a321e2853b4b08e8ee2aa54933f5) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "hs1_3.bin", 0x00000, 0x40000, CRC(85a016cb) SHA1(abb32c0191a531706593088b2ecfb48ceb02a127)  )
	ROM_LOAD16_BYTE( "hs1_4.bin", 0x00001, 0x40000, CRC(4313c099) SHA1(620452ac607b044ce4c8a5a7b03bc831125c81eb) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "hs_5.bin", 0x00000, 0x40000, CRC(09931910) SHA1(cac792f7c67d0ea274ecb369cef0554a033e8d88) )
	ROM_LOAD16_BYTE( "hs_6.bin", 0x00001, 0x40000, CRC(801703e4) SHA1(7da822b03a6d4f53a49bb1fedc9e1262d8a84782) )
ROM_END

ROM_START( hldspin2 )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "hs2p26.bin", 0x00000, 0x40000, CRC(35844d85) SHA1(cd9bd3a95d1aaf4171bc9c57dec45b59fcc11902) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "hs_2.bin", 0x00000, 0x40000, CRC(3b0f5374) SHA1(7e7b185b62d1a321e2853b4b08e8ee2aa54933f5) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "hs2_3.bin", 0x00000, 0x40000, CRC(b6890061) SHA1(c196f8740d8487b108cff58d77a203b2d8431a67) )
	ROM_LOAD16_BYTE( "hs2_4.bin", 0x00001, 0x40000, CRC(132a2312) SHA1(9ad5c3a3ca895d290ff584f605f05d70386cfa10) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "hs_5.bin", 0x00000, 0x40000, CRC(09931910) SHA1(cac792f7c67d0ea274ecb369cef0554a033e8d88) )
	ROM_LOAD16_BYTE( "hs_6.bin", 0x00001, 0x40000, CRC(801703e4) SHA1(7da822b03a6d4f53a49bb1fedc9e1262d8a84782) )
ROM_END



ROM_START( pickwin )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "pw25t.bin", 0x00000, 0x40000, CRC(9b6bd032) SHA1(241c772d191841c72e973d5dc494be445d6fd668) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "pw-2.bin", 0x00000, 0x40000, CRC(3b0f5374) SHA1(7e7b185b62d1a321e2853b4b08e8ee2aa54933f5) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "pw-3.bin", 0x00000, 0x40000, CRC(9a27acbd) SHA1(3a3a6d0b5f6eeeebcb2ccaff773b8971b3bfd3c7) )
	ROM_LOAD16_BYTE( "pw-4.bin", 0x00001, 0x40000, CRC(6b629619) SHA1(3842a7579f67866aa9744b0d6fa6a47c923be6fe) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "pw-5.bin", 0x00000, 0x40000, CRC(ec2ac284) SHA1(35cfab27db3740823b3cba821bd178d28e0be5f8) )
	ROM_LOAD16_BYTE( "pw-6.bin", 0x00001, 0x40000, CRC(aba36d00) SHA1(5abb1fe7d4f212fa0f7d5314f76e7c6b07e6c4bb) )
ROM_END

ROM_START( pickwina )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "pw26.bin", 0x00000, 0x40000, CRC(9bedbe5a) SHA1(fb9ee63932b5f86fe42f84a5e1b8a3c29194761b) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "pw-2.bin", 0x00000, 0x40000, CRC(3b0f5374) SHA1(7e7b185b62d1a321e2853b4b08e8ee2aa54933f5) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "pw-3.bin", 0x00000, 0x40000, CRC(9a27acbd) SHA1(3a3a6d0b5f6eeeebcb2ccaff773b8971b3bfd3c7) )
	ROM_LOAD16_BYTE( "pw-4.bin", 0x00001, 0x40000, CRC(6b629619) SHA1(3842a7579f67866aa9744b0d6fa6a47c923be6fe) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "pw-5.bin", 0x00000, 0x40000, CRC(ec2ac284) SHA1(35cfab27db3740823b3cba821bd178d28e0be5f8) )
	ROM_LOAD16_BYTE( "pw-6.bin", 0x00001, 0x40000, CRC(aba36d00) SHA1(5abb1fe7d4f212fa0f7d5314f76e7c6b07e6c4bb) )
ROM_END

ROM_START( tighook )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "thk17.bin", 0x00000, 0x40000, CRC(0e27d3dd) SHA1(c85e2e03c36e0f6ec95e15597a6bd58e8eeb6353) )
	ROM_LOAD( "thk17xt.bin", 0x00000, 0x40000, CRC(02ca5fe2) SHA1(daa66d5ef7336e311cc8bb78ec6625620b9b2800) )
	
	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "thkrom2.bin", 0x00000, 0x40000, CRC(61b61b75) SHA1(e71c5ab6aedb7ca4db32a2f4d7d2818dcdd92417) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "thkrom3.bin", 0x00000, 0x80000, CRC(ec4b4144) SHA1(36df0686b405a3c99707a6b63ad14bff1cd7b443) )
	ROM_LOAD16_BYTE( "thkrom4.bin", 0x00001, 0x80000, CRC(dbd1c526) SHA1(1f82f3e132bb5ac598e5d95254de48357130a0a0) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "thkrom5.bin", 0x00000, 0x80000, CRC(4085e345) SHA1(38c8e4727a782630527141e1586fcceff1d07d76) )
	ROM_LOAD16_BYTE( "thkrom6.bin", 0x00001, 0x80000, CRC(637695ff) SHA1(a5707b545968ac9d41c1a4ffd4de60a9df4bcbf1) )
ROM_END

ROM_START( robadv )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "ra15.bin", 0x00000, 0x40000, CRC(dd7e4ec9) SHA1(038b03855eaa8be1a97e34534822465a10886e10) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "rarom2.bin", 0x00000, 0x40000, CRC(092392cb) SHA1(fd52a0c4f46cb3242bf1b9e35ad5f41cda64010b) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "rarom3.bin", 0x00000, 0x80000, CRC(94e4cd71) SHA1(5c01e276dea3df7c367210af3d0d2399935c81c6) )
	ROM_LOAD16_BYTE( "rarom4.bin", 0x00001, 0x80000, CRC(72cfec99) SHA1(6612b8d04c0cc97dc5315fda861b606a6c158ea6) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "rarom5.bin", 0x00000, 0x80000, CRC(9bf41c2b) SHA1(8cc8ca5c2c63223e670e00ca5802b8677856bc16) )
	ROM_LOAD16_BYTE( "rarom6.bin", 0x00001, 0x80000, CRC(0fb69b4c) SHA1(8e1aaf5ade707b4045d55ff64e72cfe5db696332) )
ROM_END

ROM_START( anibonus )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "ab14.bin", 0x00000, 0x40000, CRC(d1dcb6e6) SHA1(4a95184e5d4f2e0527fdc8f29e56572cf3ba9987) )
	ROM_LOAD( "ab14a.bin", 0x00000, 0x40000, CRC(a8a0eea5) SHA1(c37a470b997ee5dbc976858c024bd67ed88061ce) )
	ROM_LOAD( "ab14xt.bin", 0x00000, 0x40000,  CRC(c6107445) SHA1(22fd3a7987219a940b965c953494939e0892661e) )
	ROM_LOAD( "ab14xta.bin", 0x00000, 0x40000,  CRC(eddf38af) SHA1(56a920ba1af213719210d25e6d8b5c7a0d513119) )
	ROM_LOAD( "ab15.bin", 0x00000, 0x40000, CRC(4640a2e7) SHA1(2659c037e88f43f89a5d8cd563eec5e4eb2025b9) )
	ROM_LOAD( "ab15xt.bin", 0x00000, 0x40000,  CRC(3aed6e7f) SHA1(51f9af92286e8b2fcfeae30913fbab4626decb99) )

	ROM_REGION( 0x040000, "oki", ROMREGION_ERASE00 ) /* Samples */
	/* None? */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "abrom3.bin", 0x00000, 0x80000, CRC(85f19e19) SHA1(2dd259af132e0cfd34974526c96f0d96ff868516) )
	ROM_LOAD16_BYTE( "abrom4.bin", 0x00001, 0x80000, CRC(c12b954c) SHA1(83556abbad0f285360da5f5e0fb93514d46b436b) )

	ROM_LOAD16_BYTE( "abrom3a.bin", 0x00000, 0x80000, CRC(85f19e19) SHA1(2dd259af132e0cfd34974526c96f0d96ff868516) )
	ROM_LOAD16_BYTE( "abrom4a.bin", 0x00001, 0x80000, CRC(c12b954c) SHA1(83556abbad0f285360da5f5e0fb93514d46b436b) )

	ROM_LOAD16_BYTE( "abrom3n.bin", 0x00000, 0x80000, CRC(aab2161a) SHA1(d472746c68720935fedfc6b2d06a4fe1152cc804) )
	ROM_LOAD16_BYTE( "abrom4n.bin", 0x00001, 0x80000, CRC(d776862c) SHA1(03b3c0e9adb11b560b8773e88ea97e712323f25e) )


	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "abrom5.bin", 0x00000, 0x80000, CRC(74b4fa88) SHA1(922d9c4f864be2b125269a69639e6206aec26d72) )
	ROM_LOAD16_BYTE( "abrom6.bin", 0x00001, 0x80000, CRC(e8f4b079) SHA1(2597fa17b6a13e634ba9fe846661d09c65fa8cf2) )

	ROM_LOAD16_BYTE( "abrom5a.bin", 0x00000, 0x80000, CRC(9810f1e2) SHA1(a10954a46d52c5a53a3b11a04e66c4ed3ce2a0f7) )
	ROM_LOAD16_BYTE( "abrom6a.bin", 0x00001, 0x80000, CRC(22d2abbe) SHA1(65d82ed0fc799c4248696f1b2ef76e7e88bf7fb7) )

	ROM_LOAD16_BYTE( "abrom5n.bin", 0x00000, 0x80000, CRC(d3db86eb) SHA1(e7e2cdfa6b4795d4021f589d2a292c67cc32f03a) )
	ROM_LOAD16_BYTE( "abrom6n.bin", 0x00001, 0x80000, CRC(0d8dcaa1) SHA1(a74c64bb89b4273e9d1e092786a5cf8ebd60477c) )
ROM_END

ROM_START( abnudge )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "ab17n.bin", 0x00000, 0x40000, CRC(aca7c2af) SHA1(8f23b4aff006fcd983769f833c2fabdbb087d36b) )
	
	ROM_REGION( 0x040000, "oki", ROMREGION_ERASE00 ) /* Samples */
	/* None? */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "abrom3n.bin", 0x00000, 0x80000, CRC(aab2161a) SHA1(d472746c68720935fedfc6b2d06a4fe1152cc804) )
	ROM_LOAD16_BYTE( "abrom4n.bin", 0x00001, 0x80000, CRC(d776862c) SHA1(03b3c0e9adb11b560b8773e88ea97e712323f25e) )


	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "abrom5n.bin", 0x00000, 0x80000, CRC(d3db86eb) SHA1(e7e2cdfa6b4795d4021f589d2a292c67cc32f03a) )
	ROM_LOAD16_BYTE( "abrom6n.bin", 0x00001, 0x80000, CRC(0d8dcaa1) SHA1(a74c64bb89b4273e9d1e092786a5cf8ebd60477c) )
ROM_END

ROM_START( anithunt )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "ath15.bin", 0x00000, 0x40000, CRC(917ae674) SHA1(67808a9d3bd48a8f7f839eb85356269a357581ad) )
	ROM_LOAD( "ath17.bin", 0x00000, 0x40000, CRC(07facf55) SHA1(2de5ca12e06a6896099672ec7383e6324d23fa12) )

	
	ROM_REGION( 0x040000, "oki", ROMREGION_ERASE00 ) /* Samples */
	/* None? */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "ath-rom3.bin", 0x00000, 0x80000, CRC(2ce266b2) SHA1(34dcc504d48a26976e17ad0b8399904e5ecc3379) )
	ROM_LOAD16_BYTE( "ath-rom4.bin", 0x00001, 0x80000, CRC(59d25672) SHA1(212ba0aa7794b7a37121896190e64069f005b1ea) )

	ROM_LOAD16_BYTE( "athrom3.bin", 0x00000, 0x80000, CRC(f784ec01) SHA1(69474fc9d10882fd9ec0c02675193df7aa31f6a7) )
	ROM_LOAD16_BYTE( "athrom4.bin", 0x00001, 0x80000, CRC(49749939) SHA1(6deb10c2b51b5718f0cba31f6bda54bcc001bc71) )


	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "ath-rom5.bin", 0x00000, 0x80000, CRC(536a7e23) SHA1(51dc6b2b022a672810b00e1006b0c7ee610a4e4f) )
	ROM_LOAD16_BYTE( "ath-rom6.bin", 0x00001, 0x80000, CRC(23bc5067) SHA1(1e279e58437b897c7a68c9cdd15277c6a906a142) )
ROM_END


ROM_START( dblchal )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "dc11.bin", 0x00000, 0x40000, CRC(05a27f07) SHA1(02b7b2731f8821bd7e0e3be005bd3024db0a7e42) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "dcrom2.bin", 0x00000, 0x20000, CRC(ce099327) SHA1(b1dc43839e9e3bf788141d58c81e1380a12d582a) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "dcrom3.bin", 0x00000, 0x40000, CRC(50b1c522) SHA1(620f8a1df6954c5db4a85448c810901d69859fec) )
	ROM_LOAD16_BYTE( "dcrom4.bin", 0x00001, 0x40000, CRC(a6f46957) SHA1(b4c0a28e428b9fc091ac5fc041a1ce01f65ff402) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "dcrom5.bin", 0x00000, 0x80000, CRC(a55f4ed3) SHA1(9f7427357af84026c056624523fd20bc556f3c22) )
	ROM_LOAD16_BYTE( "dcrom6.bin", 0x00001, 0x80000, CRC(cf783d82) SHA1(d3f8ae5cb3a5f848e2d84721a5a4ee486a52de85) )
ROM_END

ROM_START( robadv2 )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "r2a15.bin", 0x00000, 0x40000, CRC(e1932e13) SHA1(918d51e64aefaa308f92748bb5bfa92b88e00feb) )
	ROM_LOAD( "r2a15sh.bin", 0x00000, 0x40000, CRC(c53af9be) SHA1(86cb2dae1315227f01f430d23fb4e09d015f1206) )


	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "rarom2.bin", 0x00000, 0x40000, CRC(092392cb) SHA1(fd52a0c4f46cb3242bf1b9e35ad5f41cda64010b) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "ra2rom3.bin", 0x00000, 0x80000, CRC(eacd0cf7) SHA1(a04ddc339d330be4b278f12a54fe65d7eb08ffd0) )
	ROM_LOAD16_BYTE( "ra2rom4.bin", 0x00001, 0x80000, CRC(adac68d2) SHA1(6287a979a57004b1c4eea7f539550e23bac22904) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "ra2rom5.bin", 0x00000, 0x80000, CRC(ad9379a2) SHA1(93126969677cfce20e5b2e287662ac6b2ceee425) )
	ROM_LOAD16_BYTE( "ra2rom6.bin", 0x00001, 0x80000, CRC(12312874) SHA1(9d4d9d9fbec8536e8a003892643654d15f4535fa) )
ROM_END

ROM_START( pirpok2 )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "p3p20.bin", 0x00000, 0x40000, CRC(0e477094) SHA1(cd35c9ac1ed4b843886b1fc554e749f38573ca21) )
	
	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "p3rom2.bin", 0x00000, 0x20000, CRC(db6182e4) SHA1(65f05247629d5a1f37bf179f468acf8420342d2c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "p3rom3.bin", 0x00000, 0x80000, CRC(34d3e6c5) SHA1(1d89677605188f135c8dbbc2ab20510cae7548fe) )
	ROM_LOAD16_BYTE( "p3rom4.bin", 0x00001, 0x80000, CRC(3861b5fb) SHA1(72f085a1fd951919f479e5c6984304f7bbddc054) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "p3rom5.bin", 0x00000, 0x80000, CRC(c5eca135) SHA1(bdaccd32e1434016c77579bc8c4214ab2a3ae474) )
	ROM_LOAD16_BYTE( "p3rom6.bin", 0x00001, 0x80000, CRC(d990cbb8) SHA1(6f822e38bf401b2eb0b2e36f3b4fc6822fafd3fa) )
ROM_END





ROM_START( fcnudge )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "fc17n.bin", 0x00000, 0x40000, CRC(b9193d4f) SHA1(5ed77802e5a8f246eb1a559c13ad544adae35201) )

	ROM_REGION( 0x040000, "oki", ROMREGION_ERASE00 ) /* Samples */
	/* none? */
	
	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fcrom3n.bin", 0x00000, 0x80000, CRC(3d1c3d7f) SHA1(bcb20c08a0a2a36775052ae45258862afc00d61d) )
	ROM_LOAD16_BYTE( "fcrom4n.bin", 0x00001, 0x80000, CRC(a047861e) SHA1(b5d160c25945c7c103160e80d545cb3e1091e631) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "fcrom5n.bin", 0x00000, 0x80000, CRC(42955842) SHA1(383be3049da5b10ea57a278bc6578ece046058fd) )
	ROM_LOAD16_BYTE( "fcrom6n.bin", 0x00001, 0x80000, CRC(eee0f84d) SHA1(4ac096ccea258710f58c8121e7f0af28593d6368) )
ROM_END

ROM_START( sfruitb )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "sfb18xt.bin", 0x00000, 0x40000, CRC(15a7fc47) SHA1(4f1af0bab7807a69f8c67c8e83b35c8c5c2a13f1) )
	ROM_LOAD( "sfb20.bin", 0x00000, 0x40000, CRC(73a2be7f) SHA1(95b51a63ede10247fde944d980d85781947a8435) )
	ROM_LOAD( "sfb20b.bin", 0x00000, 0x40000, CRC(6fe1b8ba) SHA1(46fe3940d80578f3818702fd449fc4119ea5fc30) )

	ROM_REGION( 0x040000, "oki", ROMREGION_ERASE00 ) /* Samples */
	/* none? */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "sfbrom3.bin", 0x00000, 0x80000, CRC(b48eb491) SHA1(0369873231ffa3fb78863623209ad1e05222fc8a) )
	ROM_LOAD16_BYTE( "sfbrom4.bin", 0x00001, 0x80000, CRC(a307119c) SHA1(b45a0e73d4e2d665de634dbf0034b3dcc9152b3d) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "sfbrom5.bin", 0x00000, 0x80000, CRC(31588ff3) SHA1(6e2a65d50457ec0e93a647fd8ca5ebeeb16bdb1c) )
	ROM_LOAD16_BYTE( "sfbrom6.bin", 0x00001, 0x80000, CRC(232d6216) SHA1(ca7780adc85fa570698736785ad700797e6a98fb) )
ROM_END

ROM_START( fb2gen )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "fb2g15r.bin", 0x00000, 0x40000, CRC(a8daf67d) SHA1(6e980748ec77c4842676f14ffffe3f630879e9d9) )
	ROM_LOAD( "fb2g16xt.bin", 0x00000, 0x40000, CRC(ea525ebb) SHA1(965bba045ba69ac4316b27d0d69b130119f9ce04) )
	
	ROM_REGION( 0x040000, "oki", ROMREGION_ERASE00 ) /* Samples */
	ROM_LOAD( "fb2grom2.bin", 0x00000, 0x40000, CRC(1cbbd43a) SHA1(6e31c3bdd677d9d3cb445294cf17a0efcb16d4ed) )
	//ROM_LOAD( "fb2grom2a.bin", 0x00000, 0x40000, CRC(1cbbd43a) SHA1(6e31c3bdd677d9d3cb445294cf17a0efcb16d4ed))

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fb2grom3.bin", 0x00000, 0x80000, CRC(a4f33c67) SHA1(ec7f539725b2684add019c1dad3f230b5c798daa) )
	ROM_LOAD16_BYTE( "fb2grom4.bin", 0x00001, 0x80000, CRC(c142f2af) SHA1(3323de8cd09b64c1c8ccf51acf74444e577fdfb3) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "fb2grom5.bin", 0x00000, 0x80000, CRC(1c4172a8) SHA1(c45a57cd799681d442de02f8f07dbd9751929ca4) )
	ROM_LOAD16_BYTE( "fb2grom6.bin", 0x00001, 0x80000, CRC(953fdcc4) SHA1(c57e2b4a8273e789b96d39fe28d02bec5359b5f4) )
ROM_END

ROM_START( fb2nd )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "fb2e15.bin", 0x00000, 0x40000, CRC(40a4bc95) SHA1(f84d8615e5a247a6db7792e54d236fbd5008d794) )
	
	ROM_REGION( 0x040000, "oki", ROMREGION_ERASE00 ) /* Samples */
	ROM_LOAD( "fb2grom2.bin", 0x00000, 0x40000, CRC(1cbbd43a) SHA1(6e31c3bdd677d9d3cb445294cf17a0efcb16d4ed) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fb2erom3.bin", 0x00000, 0x80000, CRC(58201f71) SHA1(1e8d44105194a619d75d106cebcef783edc810f2) )
	ROM_LOAD16_BYTE( "fb2erom4.bin", 0x00001, 0x80000, CRC(4f8cb873) SHA1(7fae47e41abb8e3fffd584f9a5507168c06b8b6b) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "fb2erom5.bin", 0x00000, 0x80000, CRC(1bc55876) SHA1(d04ff7bf97145d45de943129bc9f3cbe27f4588e) )
	ROM_LOAD16_BYTE( "fb2erom6.bin", 0x00001, 0x80000, CRC(71b43f19) SHA1(a0b7f2b1968e6c083f9793f1249edb339422370d) )
ROM_END

ROM_START( fb4 )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "fb24k12b.bin", 0x00000, 0x40000, CRC(b238411c) SHA1(947a243141766583ce170e1f92769952281bf386) )
	ROM_LOAD( "fb24k13t.bin", 0x00000, 0x40000, CRC(ef2407cf) SHA1(4bfb8cd738d576e482828529bca3031b55cc165d) )
	
	ROM_REGION( 0x040000, "oki", ROMREGION_ERASE00 ) /* Samples */
	ROM_LOAD( "fb4rom2.bin", 0x00000, 0x3ffff, BAD_DUMP CRC(bf49ba49) SHA1(eea40e34298f7fd98771f0869ef541c5e1514f2a) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fb4rom3.bin", 0x00000, 0x80000, CRC(4176937d) SHA1(dbde944a154f648a86628a8165fa27032115c417) )
	ROM_LOAD16_BYTE( "fb4rom4.bin", 0x00001, 0x80000, CRC(f8c57041) SHA1(ca8f58e89d31563b363a78db89e2711402f3ba80) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "fb4rom5.bin", 0x00000, 0x80000, CRC(41ad506c) SHA1(19086ab859a60e5127af0e51381cbb9fda6de74a) )
	ROM_LOAD16_BYTE( "fb4rom6.bin", 0x00001, 0x80000, CRC(f6c07f3d) SHA1(709fe2a443fdd32a3f9ab9161d5321a01c0119bb) )
ROM_END

ROM_START( ch2000 )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "fc2k39.bin", 0x00000, 0x40000, CRC(77901459) SHA1(f30c416973550bf2598eb5ec388158d864ace089) )
	ROM_LOAD( "fc2k39d.bin", 0x00000, 0x40000, CRC(38fa136c) SHA1(cae17a6340829f2d1963ffcd8fde89fdf9425a6b) )
	ROM_LOAD( "fc2k39xt.bin", 0x00000, 0x40000, CRC(fa330fdc) SHA1(8bafb76762ca64d5d4e16e4542585083078ce719) )

	ROM_REGION( 0x040000, "oki", ROMREGION_ERASE00 ) /* Samples */
	ROM_LOAD( "fbrom2.bin", 0x00000, 0x40000, CRC(3b0f5374) SHA1(7e7b185b62d1a321e2853b4b08e8ee2aa54933f5) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fbrom3.bin", 0x00000, 0x40000, CRC(a712b521) SHA1(355b3bd892d5fbd360961ca4b5adb20ddf2ba553) )
	ROM_LOAD16_BYTE( "fbrom4.bin", 0x00001, 0x40000, CRC(8996d2d5) SHA1(fc3830b8126ef9d15108e0873209168ad0b608c8) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "fbrom5.bin", 0x00000, 0x40000, CRC(494fd1fa) SHA1(01ca60e35b68da398612fc7c8a7da6f8835eabd5) )
	ROM_LOAD16_BYTE( "fbrom6.bin", 0x00001, 0x40000, CRC(0ebe2ea5) SHA1(d83c1ba940e43ce1d392969055f36b3c49ac9727) )
ROM_END


/*

Action 2000 by AMCOE


Graphics - HM86171-80 ? 28DIP
Processor - Amcoe Saltire ? 208PQFP

QUARTZ OSCILLATORS 12.000, 4.9152 and 27.000

RAM

HM6264ALP-15 - 28DIP
UN61256FK-15 - 28DIP

Unknown

Chip Markings removed 44Pin PLCC

Roms

a2k1-1.2.u28 - F29C51001T
a2k-2.u11 - F29C51002T
a2k-3.u9 - F29C51004T
a2k-4.u8 - F29C51004T
a2k-5.u6 - F29C51004T
a2k-6.u4 - F29C51004T

Dumped by Dang_Spot 08/12/04
*/

ROM_START( act2000 )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "a2k1-1.2.u28", 0x00000, 0x20000, CRC(ef9d7399) SHA1(8b4b7df85c4b0a22cb591be142bf8fea37c4b211) )

	ROM_REGION( 0x040000, "oki", ROMREGION_ERASE00 ) /* Samples */
	ROM_LOAD( "a2k-2.u11", 0x00000, 0x40000, CRC(5973b644) SHA1(428e4301e495000c3903c9e942d3dfba8261d745) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "a2k-3.u9", 0x00000, 0x80000, CRC(e91c51b0) SHA1(7858f30eb698ee37d27dd61a7df092000e8f7a7c) )
	ROM_LOAD16_BYTE( "a2k-4.u8", 0x00001, 0x80000, CRC(1238f1ae) SHA1(073df71dd13a77157ae9c94204cf69fda8286e0b) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "a2k-5.u6", 0x00000, 0x80000, CRC(2b4f7af8) SHA1(6892de184f0824d7b71c48b75db4dce19d230923) )
	ROM_LOAD16_BYTE( "a2k-6.u4", 0x00001, 0x80000, CRC(1b812dd6) SHA1(55998bd26ff9795087e6e240cc202306121920e8) )
ROM_END

ROM_START( act2000a )
	ROM_REGION( 0x80000, "main", 0 ) /* Z80 Code */
	ROM_LOAD( "a2k31xt.bin", 0x00000, 0x20000, CRC(46b3b809) SHA1(cbb88dda67fca89801c6db3bf0bf3a368fe26ad1) )
	ROM_LOAD( "a2k33.bin", 0x00000, 0x20000, CRC(e096da60) SHA1(3e971ae152058c730a7ca35ce1ed3ce3896f34f5) )

	ROM_REGION( 0x040000, "oki", ROMREGION_ERASE00 ) /* Samples */
	ROM_LOAD( "a2klink2.bin", 0x00000, 0x40000, CRC(3b0f5374) SHA1(7e7b185b62d1a321e2853b4b08e8ee2aa54933f5) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "a2klink3.bin", 0x00000, 0x80000, CRC(10298268) SHA1(05b4c6ae90f069b67e7c17b7a74dc786888274a6) )
	ROM_LOAD16_BYTE( "a2klink4.bin", 0x00001, 0x80000, CRC(9c90cada) SHA1(10afbc7900ad876fddbe912c95bac1f575e0948e) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "a2klink5.bin", 0x00000, 0x80000, CRC(7083106a) SHA1(39e7da2ef91dda40b2a9d9b8d50c587d637fda54) )
	ROM_LOAD16_BYTE( "a2klink6.bin", 0x00001, 0x80000, CRC(ba0de415) SHA1(fb4d718b9ad95eaa0ca259605fc1f2916bd1b7b7) )
ROM_END


static DRIVER_INIT( sfbonus_common)
{
	sfbonus_tilemap_ram = auto_malloc(0x4000);
	state_save_register_global_pointer(machine, sfbonus_tilemap_ram , 0x4000);

	sfbonus_reel_ram = auto_malloc(0x0800);
	state_save_register_global_pointer(machine, sfbonus_reel_ram , 0x0800);

	sfbonus_reel2_ram = auto_malloc(0x0800);
	state_save_register_global_pointer(machine, sfbonus_reel2_ram , 0x0800);

	sfbonus_reel3_ram = auto_malloc(0x0800);
	state_save_register_global_pointer(machine, sfbonus_reel3_ram , 0x0800);

	sfbonus_reel4_ram = auto_malloc(0x0800);
	state_save_register_global_pointer(machine, sfbonus_reel4_ram , 0x0800);



	sfbonus_videoram = auto_malloc(0x10000);//memory_region(machine,"user1");
	state_save_register_global_pointer(machine, sfbonus_videoram, 0x10000);

	// dummy.rom helper
	{
		UINT8 *ROM = memory_region(machine, "main");
		UINT8* ROM2 = memory_region(machine,"user1");

		if (ROM2)
		{
			{

				int x;
				int y;
				for (y=0;y<0x8;y++)
				{
					printf("@Echo Off\n");
					printf("a.exe ");
					for (x=0;x<0x20*0x8;x+=0x8)
					{
						printf("%02x %02x ", ROM[x+y], ROM2[x+y]);
					}
					printf("\n");
				}

			}

			{
				UINT8 *ROM = memory_region(machine, "main");
				FILE *fp;
				char filename[256];
				sprintf(filename,"decr_%s", machine->gamedrv->name);
				fp=fopen(filename, "w+b");
				if (fp)
				{
					fwrite(ROM, 0x40000, 1, fp);
					fclose(fp);
				}
			}
		}
	}
}

static DRIVER_INIT( sfbonus )
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
			case 0: x = BITSWAP8(x^0x2a, 1,3,7,6,5,2,0,4); break;
			case 1: x = BITSWAP8(x^0xe4, 3,7,6,5,2,0,4,1); break;
			case 2: x = BITSWAP8(x^0x2d, 4,1,3,7,6,5,2,0); break;
			case 3: x = BITSWAP8(x^0xba, 4,3,0,2,1,7,6,5); break;
			case 4: x = BITSWAP8(x^0x30, 2,1,7,6,5,0,3,4); break;
			case 5: x = BITSWAP8(x^0xf1, 2,7,6,5,1,3,4,0); break;
			case 6: x = BITSWAP8(x^0x3d, 2,1,4,7,6,5,3,0); break;
			case 7: x = BITSWAP8(x^0xba, 4,3,0,1,2,7,6,5); break;
		}

		ROM[i] = x;
	}
	DRIVER_INIT_CALL(sfbonus_common);
}

DRIVER_INIT(act2000)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
			case 0: x = BITSWAP8(x^0x25, 1,2,7,6,5,4,3,0); break;
			case 1: x = BITSWAP8(x^0xE6, 1,7,6,5,4,3,0,2); break;
			case 2: x = BITSWAP8(x^0x20, 2,4,1,7,6,5,0,3); break;
			case 3: x = BITSWAP8(x^0xBF, 0,3,1,2,4,7,6,5); break;
			case 4: x = BITSWAP8(x^0x2E, 1,3,7,6,5,2,0,4); break;
			case 5: x = BITSWAP8(x^0xE0, 3,7,6,5,2,0,4,1); break;
			case 6: x = BITSWAP8(x^0x2D, 4,1,2,7,6,5,0,3); break;
			case 7: x = BITSWAP8(x^0xB2, 2,0,4,1,3,7,6,5); break;

		}

		ROM[i] = x;
	}

	DRIVER_INIT_CALL(sfbonus_common);
}

DRIVER_INIT(dblchal)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{

			case 0: x = BITSWAP8(x^0x3D, 0,3,7,6,5,2,1,4); break;
			case 1: x = BITSWAP8(x^0xF3, 3,7,6,5,1,0,4,2); break;
			case 2: x = BITSWAP8(x^0x3D, 2,0,1,7,6,5,3,4); break;
			case 3: x = BITSWAP8(x^0xA8, 3,4,2,0,1,7,6,5); break;
			case 4: x = BITSWAP8(x^0x3D, 2,3,7,6,5,1,0,4); break;
			case 5: x = BITSWAP8(x^0xEF, 2,7,6,5,1,0,3,4); break;
			case 6: x = BITSWAP8(x^0x3A, 4,2,3,7,6,5,1,0); break;
			case 7: x = BITSWAP8(x^0xBA, 2,4,1,0,3,7,6,5); break;
    		}

		ROM[i] = x;
	}


	DRIVER_INIT_CALL(sfbonus_common);
}


static DRIVER_INIT(hldspin1)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
 			case 0: x = BITSWAP8(x^0x21, 0,2,7,6,5,4,3,1); break;
			case 1: x = BITSWAP8(x^0xe1, 1,7,6,5,4,3,2,0); break;
			case 2: x = BITSWAP8(x^0x31, 1,4,3,7,6,5,2,0); break;
			case 3: x = BITSWAP8(x^0xbc, 0,3,4,2,1,7,6,5); break;
			case 4: x = BITSWAP8(x^0x24, 4,3,7,6,5,2,0,1); break;
			case 5: x = BITSWAP8(x^0xf8, 3,7,6,5,2,0,1,4); break;
			case 6: x = BITSWAP8(x^0x39, 1,4,2,7,6,5,0,3); break;
			case 7: x = BITSWAP8(x^0xaf, 0,3,2,1,4,7,6,5); break;
    	}
		ROM[i] = x;
	}


	DRIVER_INIT_CALL(sfbonus_common);
}

static DRIVER_INIT(hldspin2)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
			case 0: x = BITSWAP8(x^0x21, 1,3,7,6,5,0,4,2); break;
			case 1: x = BITSWAP8(x^0xfe, 2,7,6,5,1,0,4,3); break;
			case 2: x = BITSWAP8(x^0x33, 1,0,3,7,6,5,2,4); break;
			case 3: x = BITSWAP8(x^0xa6, 1,0,4,3,2,7,6,5); break;
			case 4: x = BITSWAP8(x^0x37, 0,1,7,6,5,3,2,4); break;
			case 5: x = BITSWAP8(x^0xfe, 2,7,6,5,1,0,4,3); break;
			case 6: x = BITSWAP8(x^0x36, 1,0,4,7,6,5,3,2); break;
			case 7: x = BITSWAP8(x^0xa2, 1,0,2,4,3,7,6,5); break;
    	}
		ROM[i] = x;
	}


	DRIVER_INIT_CALL(sfbonus_common);
}

static DRIVER_INIT(pickwin)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
 			case 0: x = BITSWAP8(x^0x20, 1,3,7,6,5,2,4,0); break;
			case 1: x = BITSWAP8(x^0xfa, 2,7,6,5,4,0,1,3); break;
			case 2: x = BITSWAP8(x^0x37, 1,0,3,7,6,5,2,4); break;
			case 3: x = BITSWAP8(x^0xb0, 4,0,1,3,2,7,6,5); break;
			case 4: x = BITSWAP8(x^0x34, 0,1,7,6,5,3,2,4); break;
			case 5: x = BITSWAP8(x^0xef, 3,7,6,5,2,0,1,4); break;
			case 6: x = BITSWAP8(x^0x27, 1,0,4,7,6,5,3,2); break;
			case 7: x = BITSWAP8(x^0xb0, 4,0,1,3,2,7,6,5); break;
    	}
		ROM[i] = x;
	}

	DRIVER_INIT_CALL(sfbonus_common);
}

static DRIVER_INIT(robadv)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
 			case 0: x = BITSWAP8(x^0x31, 0,3,7,6,5,2,1,4); break;
			case 1: x = BITSWAP8(x^0xe0, 1,7,6,5,3,2,4,0); break;
			case 2: x = BITSWAP8(x^0x2f, 4,0,2,7,6,5,3,1); break;
			case 3: x = BITSWAP8(x^0xa7, 1,0,3,4,2,7,6,5); break;
			case 4: x = BITSWAP8(x^0x33, 1,3,7,6,5,2,0,4); break;
			case 5: x = BITSWAP8(x^0xed, 2,7,6,5,1,4,3,0); break;
			case 6: x = BITSWAP8(x^0x34, 4,1,3,7,6,5,2,0); break;
			case 7: x = BITSWAP8(x^0xaf, 2,0,4,1,3,7,6,5); break;
    	}
		ROM[i] = x;
	}

	DRIVER_INIT_CALL(sfbonus_common);
}

static DRIVER_INIT(anibonus)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
 			case 0: x = BITSWAP8(x^0x33, 0,3,7,6,5,2,1,4); break;
			case 1: x = BITSWAP8(x^0xe7, 2,7,6,5,3,4,1,0); break;
			case 2: x = BITSWAP8(x^0x3a, 4,2,3,7,6,5,1,0); break;
			case 3: x = BITSWAP8(x^0xa8, 3,4,2,0,1,7,6,5); break;
			case 4: x = BITSWAP8(x^0x3d, 2,3,7,6,5,1,0,4); break;
			case 5: x = BITSWAP8(x^0xff, 3,7,6,5,1,0,2,4); break;
			case 6: x = BITSWAP8(x^0x3a, 4,2,3,7,6,5,1,0); break;
			case 7: x = BITSWAP8(x^0xbe, 3,4,1,0,2,7,6,5); break;
    	}
		ROM[i] = x;
	}

	DRIVER_INIT_CALL(sfbonus_common);
}

static DRIVER_INIT(pirpok2)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
 			case 0: x = BITSWAP8(x^0x26, 1,2,7,6,5,4,3,0); break;
			case 1: x = BITSWAP8(x^0xf6, 1,7,6,5,4,3,0,2); break;
			case 2: x = BITSWAP8(x^0x29, 4,0,1,7,6,5,2,3); break;
			case 3: x = BITSWAP8(x^0xad, 0,3,1,2,4,7,6,5); break;
			case 4: x = BITSWAP8(x^0x2e, 1,3,7,6,5,2,0,4); break;
			case 5: x = BITSWAP8(x^0xe0, 3,7,6,5,2,0,4,1); break;
			case 6: x = BITSWAP8(x^0x39, 4,1,2,7,6,5,0,3); break;
			case 7: x = BITSWAP8(x^0xb2, 2,0,4,1,3,7,6,5); break;
    	}
		ROM[i] = x;
	}

	DRIVER_INIT_CALL(sfbonus_common);
}

static DRIVER_INIT(tighook)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
			case 0: x = BITSWAP8(x^0x33, 0,1,7,6,5,2,3,4); break;
			case 1: x = BITSWAP8(x^0xf3, 3,7,6,5,1,0,4,2); break;
			case 2: x = BITSWAP8(x^0x2e, 4,0,2,7,6,5,3,1); break;
			case 3: x = BITSWAP8(x^0xa7, 1,0,4,2,3,7,6,5); break;
			case 4: x = BITSWAP8(x^0x2d, 1,2,7,6,5,3,4,0); break;
			case 5: x = BITSWAP8(x^0xff, 2,7,6,5,1,0,3,4); break;
			case 6: x = BITSWAP8(x^0x27, 1,0,2,7,6,5,3,4); break;
			case 7: x = BITSWAP8(x^0xa7, 1,0,4,2,3,7,6,5); break;
    	}      
		ROM[i] = x;
	}

	DRIVER_INIT_CALL(sfbonus_common);	
}

static DRIVER_INIT(sfruitb)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
			case 0: x = BITSWAP8(x^0x3e, 2,1,7,6,5,4,3,0); break;
			case 1: x = BITSWAP8(x^0xfd, 1,7,6,5,0,3,2,4); break;
			case 2: x = BITSWAP8(x^0x37, 4,1,3,7,6,5,2,0); break;
			case 3: x = BITSWAP8(x^0xac, 2,0,4,1,3,7,6,5); break;
			case 4: x = BITSWAP8(x^0x35, 2,3,7,6,5,1,0,4); break;
			case 5: x = BITSWAP8(x^0xf6, 3,7,6,5,2,0,1,4); break;
			case 6: x = BITSWAP8(x^0x37, 4,1,3,7,6,5,2,0); break;
			case 7: x = BITSWAP8(x^0xb9, 0,3,4,1,2,7,6,5); break;
    	}      
		ROM[i] = x;
	}

	DRIVER_INIT_CALL(sfbonus_common);	
} 

static DRIVER_INIT(fb2gen)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
 			case 0: x = BITSWAP8(x^0x35, 0,3,7,6,5,2,1,4); break;
			case 1: x = BITSWAP8(x^0xe8, 2,7,6,5,4,3,1,0); break;
			case 2: x = BITSWAP8(x^0x23, 4,3,2,7,6,5,1,0); break;
			case 3: x = BITSWAP8(x^0xb8, 2,1,4,0,3,7,6,5); break;
			case 4: x = BITSWAP8(x^0x2d, 0,1,7,6,5,4,2,3); break;
			case 5: x = BITSWAP8(x^0xf8, 2,7,6,5,1,4,3,0); break;
			case 6: x = BITSWAP8(x^0x23, 4,0,3,7,6,5,2,1); break;
			case 7: x = BITSWAP8(x^0xb8, 2,1,4,0,3,7,6,5); break;
    	}      
		ROM[i] = x;
	}

	DRIVER_INIT_CALL(sfbonus_common);	
} 

static DRIVER_INIT(fb2nd)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
 			case 0: x = BITSWAP8(x^0x2f, 0,2,7,6,5,3,4,1); break;
			case 1: x = BITSWAP8(x^0xff, 2,7,6,5,3,0,4,1); break;
			case 2: x = BITSWAP8(x^0x3e, 4,0,1,7,6,5,2,3); break;
			case 3: x = BITSWAP8(x^0xad, 3,0,4,1,2,7,6,5); break;
			case 4: x = BITSWAP8(x^0x35, 4,3,7,6,5,1,0,2); break;
			case 5: x = BITSWAP8(x^0xfd, 4,7,6,5,3,1,2,0); break;
			case 6: x = BITSWAP8(x^0x3a, 4,1,2,7,6,5,3,0); break;
			case 7: x = BITSWAP8(x^0xbd, 3,4,2,0,1,7,6,5); break;
    	}      
		ROM[i] = x;
	}

	DRIVER_INIT_CALL(sfbonus_common);	
} 

static DRIVER_INIT(fb4)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
 			case 0: x = BITSWAP8(x^0x37, 1,2,7,6,5,4,3,0); break;
			case 1: x = BITSWAP8(x^0xeb, 1,7,6,5,4,0,2,3); break;
			case 2: x = BITSWAP8(x^0x2d, 4,0,2,7,6,5,3,1); break;
			case 3: x = BITSWAP8(x^0xbd, 2,0,4,1,3,7,6,5); break;
			case 4: x = BITSWAP8(x^0x29, 4,1,7,6,5,2,3,0); break;
			case 5: x = BITSWAP8(x^0xff, 1,7,6,5,2,3,0,4); break;
			case 6: x = BITSWAP8(x^0x3f, 1,0,4,7,6,5,3,2); break;
			case 7: x = BITSWAP8(x^0xae, 2,3,0,4,1,7,6,5); break;
    	}      
		ROM[i] = x;
	}

	DRIVER_INIT_CALL(sfbonus_common);	
} 

static DRIVER_INIT(ch2000)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
 			case 0: x = BITSWAP8(x^0x29, 2,3,7,6,5,0,4,1); break;
			case 1: x = BITSWAP8(x^0xfe, 2,7,6,5,1,0,3,4); break;
			case 2: x = BITSWAP8(x^0x33, 0,1,3,7,6,5,2,4); break;
			case 3: x = BITSWAP8(x^0xa6, 1,0,3,4,2,7,6,5); break;
			case 4: x = BITSWAP8(x^0x25, 4,1,7,6,5,3,2,0); break;
			case 5: x = BITSWAP8(x^0xfe, 2,7,6,5,1,0,3,4); break;
			case 6: x = BITSWAP8(x^0x35, 0,1,4,7,6,5,3,2); break;
			case 7: x = BITSWAP8(x^0xbe, 1,0,4,2,3,7,6,5); break;
    	}      
		ROM[i] = x;
	}

	DRIVER_INIT_CALL(sfbonus_common);	
} 

static DRIVER_INIT(anithunt)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
 			case 0: x = BITSWAP8(x^0x39, 1,2,7,6,5,4,3,0); break;
			case 1: x = BITSWAP8(x^0xe7, 4,7,6,5,0,3,1,2); break;
			case 2: x = BITSWAP8(x^0x33, 0,2,1,7,6,5,4,3); break;
			case 3: x = BITSWAP8(x^0xb3, 0,3,4,2,1,7,6,5); break;
			case 4: x = BITSWAP8(x^0x2a, 1,3,7,6,5,2,0,4); break;
			case 5: x = BITSWAP8(x^0xe4, 3,7,6,5,2,0,4,1); break;
			case 6: x = BITSWAP8(x^0x2d, 4,1,3,7,6,5,2,0); break;
			case 7: x = BITSWAP8(x^0xb6, 0,3,2,1,4,7,6,5); break;
    	}      
		ROM[i] = x;
	}

	DRIVER_INIT_CALL(sfbonus_common);	
} 

static DRIVER_INIT(abnudge)
{
	int i;
	UINT8 *ROM = memory_region(machine, "main");

	for(i=0;i<0x40000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 7)
		{
 			case 0: x = BITSWAP8(x^0x33, 0,3,7,6,5,2,1,4); break;
			case 1: x = BITSWAP8(x^0xff, 3,7,6,5,1,0,4,2); break;
			case 2: x = BITSWAP8(x^0x36, 4,2,3,7,6,5,1,0); break;
			case 3: x = BITSWAP8(x^0xa8, 3,2,4,0,1,7,6,5); break;
			case 4: x = BITSWAP8(x^0x2c, 0,1,7,6,5,2,4,3); break;
			case 5: x = BITSWAP8(x^0xff, 3,7,6,5,1,0,4,2); break;
			case 6: x = BITSWAP8(x^0x26, 2,4,3,7,6,5,1,0); break;
			case 7: x = BITSWAP8(x^0xbe, 4,1,3,0,2,7,6,5); break;
    	}      
		ROM[i] = x;
	}

	DRIVER_INIT_CALL(sfbonus_common);	
} 



/*
 			case 0: x = BITSWAP8(x^0xff, 7,6,5,4,3,2,1,0); break;
			case 1: x = BITSWAP8(x^0xff, 7,6,5,4,3,2,1,0); break;
			case 2: x = BITSWAP8(x^0xff, 7,6,5,4,3,2,1,0); break;
			case 3: x = BITSWAP8(x^0xff, 7,6,5,4,3,2,1,0); break;
			case 4: x = BITSWAP8(x^0xff, 7,6,5,4,3,2,1,0); break;
			case 5: x = BITSWAP8(x^0xff, 7,6,5,4,3,2,1,0); break;
			case 6: x = BITSWAP8(x^0xff, 7,6,5,4,3,2,1,0); break;
			case 7: x = BITSWAP8(x^0xff, 7,6,5,4,3,2,1,0); break;
*/

GAME( 199?, sfbonus,     0,        sfbonus,    sfbonus,    sfbonus, ROT0,  "Amcoe", "Skill Fruit Bonus (set 1)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 199?, sfbonusa,    sfbonus,  sfbonus,    sfbonus,    sfbonus, ROT0,  "Amcoe", "Skill Fruit Bonus (set 2)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 199?, parrot3,     0,        sfbonus,    parrot3,    pirpok2, ROT0,  "Amcoe", "Parrot Poker III", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, hldspin1,    0,        sfbonus,    sfbonus,    hldspin1, ROT0,  "Amcoe", "Hold & Spin I", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, hldspin2,    0,        sfbonus,    sfbonus,    hldspin2, ROT0,  "Amcoe", "Hold & Spin II", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, fcnudge,     0,        sfbonus,    sfbonus,    abnudge, ROT0,  "Amcoe", "Fruit Carnival Nudge", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, pickwin,     0,        sfbonus,    sfbonus,    pickwin, ROT0,  "Amcoe", "Pick & Win (set 1)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, pickwina,    pickwin,  sfbonus,    sfbonus,    pickwin, ROT0,  "Amcoe", "Pick & Win (set 2)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, tighook,     0,        sfbonus,    sfbonus,    tighook, ROT0,  "Amcoe", "Tiger Hook", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, robadv,      0,        sfbonus,    sfbonus,    robadv, ROT0,  "Amcoe", "Robin Adventure", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, robadv2,     0,        sfbonus,    sfbonus,    robadv, ROT0,  "Amcoe", "Robin Adventure 2", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, pirpok2,     0,        sfbonus,    sfbonus,    pirpok2, ROT0,  "Amcoe", "Pirate Poker II", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, anibonus,    0,        sfbonus,    sfbonus,    anibonus, ROT0,  "Amcoe", "Animal Bonus", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, abnudge,     0,        sfbonus,    sfbonus,    abnudge, ROT0,  "Amcoe", "Animal Bonus Nudge", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, dblchal,     0,        sfbonus,    sfbonus,    dblchal, ROT0,  "Amcoe", "Double Challenge", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, anithunt,    0,        sfbonus,    sfbonus,    anithunt, ROT0,  "Amcoe", "Animal Treasure Hunt", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, sfruitb,     0,        sfbonus,    sfbonus,    sfruitb, ROT0,  "Amcoe", "Super Fruit Bonus", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, fb2gen,      0,        sfbonus,    sfbonus,    fb2gen, ROT0,  "Amcoe", "Fruit Bonus 2nd Generation", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, fb2nd,       0,        sfbonus,    sfbonus,    fb2nd, ROT0,  "Amcoe", "Fruit Bonus 2nd Edition", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, fb4,         0,        sfbonus,    sfbonus,    fb4, ROT0,  "Amcoe", "Fruit Bonus 4", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, act2000,     0,        sfbonus,    sfbonus,    act2000, ROT0,  "Amcoe", "Action 2000 (Version 1.2)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, act2000a,    act2000,  sfbonus,    sfbonus,    act2000, ROT0,  "Amcoe", "Action 2000 (Version 3.3)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2000, ch2000,      0,        sfbonus,    sfbonus,    ch2000, ROT0,  "Amcoe", "Fruit Bonus 2000 / New Cherry 2000", GAME_NOT_WORKING|GAME_NO_SOUND )


