/********************************************************************

 Driver for common Hyperstone based games

 by bits from Angelo Salese, David Haywood,
    Pierpaolo Prazzoli and Tomasz Slanina

 Games Supported:


    Minigame Cool Collection   (c) 1999 SemiCom
    Jumping Break              (c) 1999 F2 System
    Lup Lup Puzzle             (c) 1999 Omega System       (version 3.0 and 2.9)
    Puzzle Bang Bang           (c) 1999 Omega System       (version 2.8)
    Super Lup Lup Puzzle       (c) 1999 Omega System       (version 4.0)
    Vamf 1/2                   (c) 1999 Danbi & F2 System  (Europe version)
    Vamp 1/2                   (c) 1999 Danbi & F2 System  (Korea version)
    Date Quiz Go Go Episode 2  (c) 2000 SemiCom
    Mission Craft              (c) 2000 Sun                (version 2.4)
    Final Godori               (c) 2001 SemiCom            (version 2.20.5915)
    Wyvern Wings               (c) 2001 SemiCom
    Age Of Heroes - Silkroad 2 (c) 2001 Unico              (v0.63 - 2001/02/07)

 Real games bugs:
 - dquizgo2: bugged video test

*********************************************************************/

#include "driver.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/e132xs/e132xs.h"
#include "machine/eeprom.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"

static UINT16 *tiles = NULL, *wram;
static UINT32 *tiles32 = NULL, *wram32;
static int flip_bit, flipscreen = 0;
static int palshift;

static int semicom_prot_idx;
static int semicom_prot_which = 0;
static UINT16 semicom_prot_data[2];

static UINT16 finalgdr_backupram_bank;
static UINT8 *finalgdr_backupram;

static READ16_DEVICE_HANDLER( oki_r )
{
	if(offset)
		return okim6295_r(device, 0);
	else
		return 0;
}

static WRITE16_DEVICE_HANDLER( oki_w )
{
	if(offset)
		okim6295_w(device, 0, data);
}

static READ16_DEVICE_HANDLER( eeprom_r )
{
	if(offset)
		return eeprom_read_bit(device);
	else
		return 0;
}

static READ32_DEVICE_HANDLER( eeprom32_r )
{
	return eeprom_read_bit(device);
}

static WRITE16_DEVICE_HANDLER( eeprom_w )
{
	if(offset)
	{
		eeprom_write_bit(device, data & 0x01);
		eeprom_set_cs_line(device, (data & 0x04) ? CLEAR_LINE : ASSERT_LINE );
		eeprom_set_clock_line(device, (data & 0x02) ? ASSERT_LINE : CLEAR_LINE );

		// data & 8?
	}
}

static WRITE32_DEVICE_HANDLER( eeprom32_w )
{
	eeprom_write_bit(device, data & 0x01);
	eeprom_set_cs_line(device, (data & 0x04) ? CLEAR_LINE : ASSERT_LINE );
	eeprom_set_clock_line(device, (data & 0x02) ? ASSERT_LINE : CLEAR_LINE );
}

static WRITE32_DEVICE_HANDLER( finalgdr_eeprom_w )
{
	eeprom_write_bit(device, data & 0x4000);
	eeprom_set_cs_line(device, (data & 0x1000) ? CLEAR_LINE : ASSERT_LINE );
	eeprom_set_clock_line(device, (data & 0x2000) ? ASSERT_LINE : CLEAR_LINE );
}

static WRITE16_HANDLER( flipscreen_w )
{
	if(offset)
	{
		flipscreen = data & flip_bit;
	}
}

static WRITE32_HANDLER( flipscreen32_w )
{
	flipscreen = data & flip_bit;
}


static WRITE16_HANDLER( jmpbreak_flipscreen_w )
{
	flipscreen = data & 0x8000;
}


static WRITE32_HANDLER( paletteram32_w )
{
	UINT16 paldata;

	COMBINE_DATA(&space->machine->generic.paletteram.u32[offset]);

	paldata = space->machine->generic.paletteram.u32[offset] & 0xffff;
	palette_set_color_rgb(space->machine, offset*2 + 1, pal5bit(paldata >> 10), pal5bit(paldata >> 5), pal5bit(paldata >> 0));

	paldata = (space->machine->generic.paletteram.u32[offset] >> 16) & 0xffff;
	palette_set_color_rgb(space->machine, offset*2 + 0, pal5bit(paldata >> 10), pal5bit(paldata >> 5), pal5bit(paldata >> 0));
}

static READ32_HANDLER( wyvernwg_prot_r )
{
	semicom_prot_idx--;
	return (semicom_prot_data[semicom_prot_which] & (1 << semicom_prot_idx)) >> semicom_prot_idx;
}

static WRITE32_HANDLER( wyvernwg_prot_w )
{
	semicom_prot_which = data & 1;
	semicom_prot_idx = 8;
}

static READ32_HANDLER( finalgdr_prot_r )
{
	semicom_prot_idx--;
	return (semicom_prot_data[semicom_prot_which] & (1 << semicom_prot_idx)) ? 0x8000 : 0;
}

static WRITE32_HANDLER( finalgdr_prot_w )
{
/*
41C6
967E
446B
F94B
*/
	if(data == 0x41c6 || data == 0x446b)
		semicom_prot_which = 0;
	else
		semicom_prot_which =  1;

	semicom_prot_idx = 8;
}

static WRITE32_DEVICE_HANDLER( finalgdr_oki_bank_w )
{
	okim6295_set_bank_base(device, 0x40000 * ((data & 0x300) >> 8));
}

static WRITE32_HANDLER( finalgdr_backupram_bank_w )
{
	finalgdr_backupram_bank = (data & 0xff000000) >> 24;
}

static READ32_HANDLER( finalgdr_backupram_r )
{
	return finalgdr_backupram[offset + finalgdr_backupram_bank * 0x80] << 24;
}

static WRITE32_HANDLER( finalgdr_backupram_w )
{
	finalgdr_backupram[offset + finalgdr_backupram_bank * 0x80] = data >> 24;
}

static WRITE32_HANDLER( finalgdr_prize_w )
{
	if(data & 0x1000000)
	{
		// prize 1
	}

	if(data & 0x2000000)
	{
		// prize 2
	}

	if(data & 0x4000000)
	{
		// prize 3
	}
}

static WRITE32_DEVICE_HANDLER( aoh_oki_bank_w )
{
	okim6295_set_bank_base(device, 0x40000 * (data & 0x3));
}

static ADDRESS_MAP_START( common_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_BASE(&wram)
	AM_RANGE(0x40000000, 0x4003ffff) AM_RAM AM_BASE(&tiles)
	AM_RANGE(0x80000000, 0x8000ffff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xfff00000, 0xffffffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( common_32bit_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_BASE(&wram32)
	AM_RANGE(0x40000000, 0x4003ffff) AM_RAM AM_BASE(&tiles32)
	AM_RANGE(0x80000000, 0x8000ffff) AM_RAM_WRITE(paletteram32_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xfff00000, 0xffffffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( vamphalf_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x0c0, 0x0c3) AM_DEVREADWRITE("oki", oki_r, oki_w)
	AM_RANGE(0x140, 0x143) AM_DEVWRITE8("ymsnd", ym2151_register_port_w, 0x00ff)
	AM_RANGE(0x146, 0x147) AM_DEVREADWRITE8("ymsnd", ym2151_status_port_r, ym2151_data_port_w, 0x00ff)
	AM_RANGE(0x1c0, 0x1c3) AM_DEVREAD("eeprom", eeprom_r)
	AM_RANGE(0x240, 0x243) AM_WRITE(flipscreen_w)
	AM_RANGE(0x600, 0x603) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x604, 0x607) AM_READ_PORT("P1_P2")
	AM_RANGE(0x608, 0x60b) AM_DEVWRITE("eeprom", eeprom_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( misncrft_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x100, 0x103) AM_WRITE(flipscreen_w)
	AM_RANGE(0x200, 0x203) AM_READ_PORT("P1_P2")
	AM_RANGE(0x240, 0x243) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x3c0, 0x3c3) AM_DEVWRITE("eeprom", eeprom_w)
	AM_RANGE(0x580, 0x583) AM_DEVREAD("eeprom", eeprom_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( coolmini_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x200, 0x203) AM_WRITE(flipscreen_w)
	AM_RANGE(0x300, 0x303) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x304, 0x307) AM_READ_PORT("P1_P2")
	AM_RANGE(0x308, 0x30b) AM_DEVWRITE("eeprom", eeprom_w)
	AM_RANGE(0x4c0, 0x4c3) AM_DEVREADWRITE("oki", oki_r, oki_w)
	AM_RANGE(0x540, 0x543) AM_DEVWRITE8("ymsnd", ym2151_register_port_w, 0x00ff)
	AM_RANGE(0x546, 0x547) AM_DEVREADWRITE8("ymsnd", ym2151_status_port_r, ym2151_data_port_w, 0x00ff)
	AM_RANGE(0x7c0, 0x7c3) AM_DEVREAD("eeprom", eeprom_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( suplup_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x020, 0x023) AM_DEVWRITE("eeprom", eeprom_w)
	AM_RANGE(0x040, 0x043) AM_READ_PORT("P1_P2")
	AM_RANGE(0x060, 0x063) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x080, 0x083) AM_DEVREADWRITE("oki", oki_r, oki_w)
	AM_RANGE(0x0c0, 0x0c3) AM_DEVWRITE8("ymsnd", ym2151_register_port_w, 0x00ff)
	AM_RANGE(0x0c4, 0x0c7) AM_DEVREADWRITE8("ymsnd", ym2151_status_port_r, ym2151_data_port_w, 0x00ff)
	AM_RANGE(0x100, 0x103) AM_DEVREAD("eeprom", eeprom_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( wyvernwg_io, ADDRESS_SPACE_IO, 32 )
	AM_RANGE(0x1800, 0x1803) AM_READWRITE(wyvernwg_prot_r, wyvernwg_prot_w)
	AM_RANGE(0x2000, 0x2003) AM_WRITE(flipscreen32_w)
	AM_RANGE(0x2800, 0x2803) AM_READ_PORT("P1_P2")
	AM_RANGE(0x3000, 0x3003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x5400, 0x5403) AM_WRITENOP // soundlatch
	AM_RANGE(0x7000, 0x7003) AM_DEVWRITE("eeprom", eeprom32_w)
	AM_RANGE(0x7c00, 0x7c03) AM_DEVREAD("eeprom", eeprom32_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( finalgdr_io, ADDRESS_SPACE_IO, 32 )
	AM_RANGE(0x2400, 0x2403) AM_READ(finalgdr_prot_r)
	AM_RANGE(0x2800, 0x2803) AM_WRITE(finalgdr_backupram_bank_w)
	AM_RANGE(0x2c00, 0x2dff) AM_READWRITE(finalgdr_backupram_r, finalgdr_backupram_w)
	AM_RANGE(0x3000, 0x3007) AM_DEVREADWRITE8("ymsnd", ym2151_r, ym2151_w, 0x0000ff00)
	AM_RANGE(0x3800, 0x3803) AM_READ_PORT("P1_P2")
	AM_RANGE(0x3400, 0x3403) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0x0000ff00)
	AM_RANGE(0x3c00, 0x3c03) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x4400, 0x4403) AM_DEVREAD("eeprom", eeprom32_r)
	AM_RANGE(0x6000, 0x6003) AM_READNOP //?
	AM_RANGE(0x6000, 0x6003) AM_DEVWRITE("eeprom", finalgdr_eeprom_w)
	AM_RANGE(0x6040, 0x6043) AM_WRITE(finalgdr_prot_w)
	//AM_RANGE(0x6080, 0x6083) AM_WRITE(flipscreen32_w) //?
	AM_RANGE(0x6060, 0x6063) AM_WRITE(finalgdr_prize_w)
	AM_RANGE(0x60a0, 0x60a3) AM_DEVWRITE("oki", finalgdr_oki_bank_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( jmpbreak_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x0c0, 0x0c3) AM_NOP // ?
	AM_RANGE(0x100, 0x103) AM_WRITENOP // ?
	AM_RANGE(0x240, 0x243) AM_READ_PORT("P1_P2")
	AM_RANGE(0x280, 0x283) AM_DEVWRITE("eeprom", eeprom_w)
	AM_RANGE(0x2c0, 0x2c3) AM_DEVREAD("eeprom", eeprom_r)
	AM_RANGE(0x440, 0x443) AM_DEVREADWRITE("oki", oki_r, oki_w)
	AM_RANGE(0x540, 0x543) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x680, 0x683) AM_DEVWRITE8("ymsnd", ym2151_register_port_w, 0x00ff)
	AM_RANGE(0x684, 0x687) AM_DEVREADWRITE8("ymsnd", ym2151_status_port_r, ym2151_data_port_w, 0x00ff)
ADDRESS_MAP_END


static ADDRESS_MAP_START( aoh_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_BASE(&wram32)
	AM_RANGE(0x40000000, 0x4003ffff) AM_RAM AM_BASE(&tiles32)
	AM_RANGE(0x80000000, 0x8000ffff) AM_RAM_WRITE(paletteram32_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x80210000, 0x80210003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x80220000, 0x80220003) AM_READ_PORT("P1_P2")
	AM_RANGE(0xffc00000, 0xffffffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( aoh_io, ADDRESS_SPACE_IO, 32 )
	AM_RANGE(0x0480, 0x0483) AM_DEVWRITE("eeprom", eeprom32_w)
	AM_RANGE(0x0620, 0x0623) AM_DEVREADWRITE8("oki_2", okim6295_r, okim6295_w, 0x0000ff00)
	AM_RANGE(0x0660, 0x0663) AM_DEVREADWRITE8("oki_1", okim6295_r, okim6295_w, 0x0000ff00)
	AM_RANGE(0x0640, 0x0647) AM_DEVREADWRITE8("ymsnd", ym2151_r, ym2151_w, 0x0000ff00)
	AM_RANGE(0x0680, 0x0683) AM_DEVWRITE("oki_2", aoh_oki_bank_w)
ADDRESS_MAP_END

/*
Sprite list:

Offset+0
-------- xxxxxxxx Y offs
-------x -------- Don't draw the sprite
x------- -------- Flip X
-x------ -------- Flip Y

Offset+1
xxxxxxxx xxxxxxxx Sprite number

Offset+2
-------- -xxxxxxx Color
or
-xxxxxxx -------- Color

Offset+3
-------x xxxxxxxx X offs
*/
static void draw_sprites(const device_config *screen, bitmap_t *bitmap)
{
	const gfx_element *gfx = screen->machine->gfx[0];
	UINT32 cnt;
	int block, offs;
	int code,color,x,y,fx,fy;
	rectangle clip;

	clip.min_x = video_screen_get_visible_area(screen)->min_x;
	clip.max_x = video_screen_get_visible_area(screen)->max_x;

	for (block=0; block<0x8000; block+=0x800)
	{
		if(flipscreen)
		{
			clip.min_y = 256 - (16-(block/0x800))*16;
			clip.max_y = 256 - ((16-(block/0x800))*16)+15;
		}
		else
		{
			clip.min_y = (16-(block/0x800))*16;
			clip.max_y = ((16-(block/0x800))*16)+15;
		}

		for (cnt=0; cnt<0x800; cnt+=8)
		{
			offs = (block + cnt) / 2;

			// 16bit version
			if(tiles != NULL)
			{
				if(tiles[offs] & 0x0100) continue;

				code  = tiles[offs+1];
				color = (tiles[offs+2] >> palshift) & 0x7f;

				x = tiles[offs+3] & 0x01ff;
				y = 256 - (tiles[offs] & 0x00ff);

				fx = tiles[offs] & 0x8000;
				fy = tiles[offs] & 0x4000;
			}
			// 32bit version
			else
			{
				offs /= 2;

				if(tiles32[offs] & 0x01000000) continue;

				code  = tiles32[offs] & 0xffff;
				color = ((tiles32[offs+1] >> palshift) & 0x7f0000) >> 16;

				x = tiles32[offs+1] & 0x01ff;
				y = 256 - ((tiles32[offs] & 0x00ff0000) >> 16);

				fx = tiles32[offs] & 0x80000000;
				fy = tiles32[offs] & 0x40000000;
			}

			if(flipscreen)
			{
				fx = !fx;
				fy = !fy;

				x = 366 - x;
				y = 256 - y;
			}

			drawgfx_transpen(bitmap,&clip,gfx,code,color,fx,fy,x,y,0);
		}
	}
}

static void draw_sprites_aoh(const device_config *screen, bitmap_t *bitmap)
{
	const gfx_element *gfx = screen->machine->gfx[0];
	UINT32 cnt;
	int block, offs;
	int code,color,x,y,fx,fy;
	rectangle clip;

	clip.min_x = video_screen_get_visible_area(screen)->min_x;
	clip.max_x = video_screen_get_visible_area(screen)->max_x;

	for (block=0; block<0x8000; block+=0x800)
	{
		if(flipscreen)
		{
			clip.min_y = 256 - (16-(block/0x800))*16;
			clip.max_y = 256 - ((16-(block/0x800))*16)+15;
		}
		else
		{
			clip.min_y = (16-(block/0x800))*16;
			clip.max_y = ((16-(block/0x800))*16)+15;
		}

		for (cnt=0; cnt<0x800; cnt+=8)
		{
			offs = (block + cnt) / 2;
			{
				offs /= 2;
				code  = (tiles32[offs] & 0xffff) | ((tiles32[offs] & 0x3000000) >> 8);
				color = ((tiles32[offs+1] >> palshift) & 0x7f0000) >> 16;

				x = tiles32[offs+1] & 0x01ff;
				y = 256 - ((tiles32[offs] & 0x00ff0000) >> 16);

				fx = tiles32[offs] & 0x4000000;
				fy = 0; // not used ? or it's tiles32[offs] & 0x8000000?
			}

			if(flipscreen)
			{
				fx = !fx;
				fy = !fy;

				x = 366 - x;
				y = 256 - y;
			}

			drawgfx_transpen(bitmap,&clip,gfx,code,color,fx,fy,x,y,0);
		}
	}
}


static VIDEO_UPDATE( common )
{
	bitmap_fill(bitmap,cliprect,0);
	draw_sprites(screen, bitmap);
	return 0;
}

static VIDEO_UPDATE( aoh )
{
	bitmap_fill(bitmap,cliprect,0);
	draw_sprites_aoh(screen, bitmap);
	return 0;
}


static INPUT_PORTS_START( common )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( finalgdr )
	PORT_START("P1_P2")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00800000, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( aoh )
	PORT_START("P1_P2")
	PORT_BIT( 0x000000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x000000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x000000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x000000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x000000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x000000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x000000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit) // eeprom bit
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x00100000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static const gfx_layout sprites_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,8,16,24, 32,40,48,56, 64,72,80,88 ,96,104,112,120 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128,9*128,10*128,11*128,12*128,13*128,14*128,15*128 },
	16*128,
};

static GFXDECODE_START( vamphalf )
	GFXDECODE_ENTRY( "gfx1", 0, sprites_layout, 0, 0x80 )
GFXDECODE_END


static NVRAM_HANDLER( finalgdr )
{
	if (read_or_write)
	{
		mame_fwrite(file, finalgdr_backupram, 0x80*0x100);
	}
	else
	{
		if (file)
		{
			mame_fread(file, finalgdr_backupram, 0x80*0x100);
		}
	}
}

static ADDRESS_MAP_START( qs1000_prg_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( qs1000_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( 0x0000, 0x007f) AM_RAM	// RAM?  wavetable registers?  not sure.
ADDRESS_MAP_END

static MACHINE_DRIVER_START( common )
	MDRV_CPU_ADD("maincpu", E116T, 50000000)	/* 50 MHz */
	MDRV_CPU_PROGRAM_MAP(common_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(31, 350, 16, 255)

	MDRV_PALETTE_LENGTH(0x8000)
	MDRV_GFXDECODE(vamphalf)

	MDRV_VIDEO_UPDATE(common)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( sound_ym_oki )
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 28000000/8)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_SOUND_ADD("oki", OKIM6295, 28000000/16 )
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( sound_suplup )
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 14318180/4)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_SOUND_ADD("oki", OKIM6295, 1789772.5 )
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( sound_qs1000 )
	MDRV_CPU_ADD("audiocpu", I8052, 24000000/4)	/* 6 MHz? */
	MDRV_CPU_PROGRAM_MAP(qs1000_prg_map)
	MDRV_CPU_IO_MAP( qs1000_io_map)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( vamphalf )
	MDRV_IMPORT_FROM(common)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(vamphalf_io)

	MDRV_IMPORT_FROM(sound_ym_oki)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( misncrft )
	MDRV_IMPORT_FROM(common)
	MDRV_CPU_REPLACE("maincpu", GMS30C2116, 50000000)	/* 50 MHz */
	MDRV_CPU_IO_MAP(misncrft_io)

	MDRV_IMPORT_FROM(sound_qs1000)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( coolmini )
	MDRV_IMPORT_FROM(common)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(coolmini_io)

	MDRV_IMPORT_FROM(sound_ym_oki)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( suplup )
	MDRV_IMPORT_FROM(common)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(suplup_io)

	MDRV_IMPORT_FROM(sound_suplup)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( jmpbreak )
	MDRV_IMPORT_FROM(common)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(jmpbreak_io)

	MDRV_IMPORT_FROM(sound_ym_oki)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( wyvernwg )
	MDRV_IMPORT_FROM(common)
	MDRV_CPU_REPLACE("maincpu", E132T, 50000000)	/* 50 MHz */
	MDRV_CPU_PROGRAM_MAP(common_32bit_map)
	MDRV_CPU_IO_MAP(wyvernwg_io)

	MDRV_IMPORT_FROM(sound_qs1000)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( finalgdr )
	MDRV_IMPORT_FROM(common)
	MDRV_CPU_REPLACE("maincpu", E132T, 50000000)	/* 50 MHz */
	MDRV_CPU_PROGRAM_MAP(common_32bit_map)
	MDRV_CPU_IO_MAP(finalgdr_io)

	MDRV_NVRAM_HANDLER(finalgdr)

	MDRV_IMPORT_FROM(sound_ym_oki)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( aoh )
	MDRV_CPU_ADD("maincpu", E132XN, 20000000*4)	/* 4x internal multiplier */
	MDRV_CPU_PROGRAM_MAP(aoh_map)
	MDRV_CPU_IO_MAP(aoh_io)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(59.185)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(64, 511-64, 16, 255-16)

	MDRV_PALETTE_LENGTH(0x8000)
	MDRV_GFXDECODE(vamphalf)

	MDRV_VIDEO_UPDATE(aoh)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579545)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_SOUND_ADD("oki_1", OKIM6295, 32000000/8)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_SOUND_ADD("oki_2", OKIM6295, 32000000/32)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END

/*

Vamp 1/2 (Semi Vamp)
Danbi, 1999

Official page here...
http://f2.co.kr/eng/product/intro1-17.asp


PCB Layout
----------
             KA12    VROM1.

             BS901   AD-65    ROML01.   ROMU01.
                              ROML00.   ROMU00.
                 62256
                 62256

T2316162A  E1-16T  PROM1.          QL2003-XPL84C

                 62256
                 62256       62256
                             62256
    93C46.IC3                62256
                             62256
    50.000MHz  QL2003-XPL84C
B1 B2 B3                     28.000MHz



Notes
-----
B1 B2 B3:      Push buttons for SERV, RESET, TEST
T2316162A:     Main program RAM
E1-16T:        Hyperstone E1-16T CPU
QL2003-XPL84C: QuickLogic PLCC84 PLD
AD-65:         Compatible to OKI M6295
KA12:          Compatible to Y3012
BS901          Compatible to YM2151
PROM1:         Main program
VROM1:         OKI samples
ROML* / U*:    Graphics, device is MX29F1610ML (surface mounted SOP44 MASK ROM)

*/

ROM_START( vamphalf )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "prg.rom1", 0x80000, 0x80000, CRC(9b1fc6c5) SHA1(acf10a50d2119ac893b6cbd494911982a9352350) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "eur.roml00", 0x000000, 0x200000, CRC(bdee9a46) SHA1(7e240b07377201afbe0cd0911ccee4ad52a74079) )
	ROM_LOAD32_WORD( "eur.romu00", 0x000002, 0x200000, CRC(fa79e8ea) SHA1(feaba99f0a863bc5d27ad91d206168684976b4c2) )
	ROM_LOAD32_WORD( "eur.roml01", 0x400000, 0x200000, CRC(a7995b06) SHA1(8b789b6a00bc177c3329ee4a31722fc65376b975) )
	ROM_LOAD32_WORD( "eur.romu01", 0x400002, 0x200000, CRC(e269f5fe) SHA1(70f1308f11e147dd20f8bd45b91aefc9fd653da6) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "snd.vrom1", 0x00000, 0x40000, CRC(ee9e371e) SHA1(3ead5333121a77d76e4e40a0e0bf0dbc75f261eb) )
ROM_END


ROM_START( vamphalfk )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "prom1", 0x80000, 0x80000, CRC(f05e8e96) SHA1(c860e65c811cbda2dc70300437430fb4239d3e2d) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(cc075484) SHA1(6496d94740457cbfdac3d918dce2e52957341616) )
	ROM_LOAD32_WORD( "romu00", 0x000002, 0x200000, CRC(711c8e20) SHA1(1ef7f500d6f5790f5ae4a8b58f96ee9343ef8d92) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(626c9925) SHA1(c90c72372d145165a8d3588def12e15544c6223b) )
	ROM_LOAD32_WORD( "romu01", 0x400002, 0x200000, CRC(d5be3363) SHA1(dbdd0586909064e015f190087f338f37bbf205d2) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "snd.vrom1", 0x00000, 0x40000, CRC(ee9e371e) SHA1(3ead5333121a77d76e4e40a0e0bf0dbc75f261eb) )
ROM_END

/*

Super Lup Lup Puzzle / Lup Lup Puzzle
Omega System, 1999

PCB Layout
----------

F-E1-16-001
|----------------------------------------------|
|       M6295       VROM1    N341256           |
|  YM3012                                      |
|       YM2151    |---------|N341256           |
|                 |Quicklogi|                  |
|                 |c        |N341256           |
|J                |QL2003-  |                  |
|A        N341256 |XPL84C   |N341256           |
|M                |---------|                  |
|M        N341256 |---------|N341256           |
|A                |Quicklogi|                  |
|         N341256 |c        |N341256           |
|                 |QL2003-  |                  |
|         N341256 |XPL84C   |N341256           |
|                 |---------|    ROML00  ROMU00|
|93C46            GM71C18163 N341256           |
|PAL          E1-16T             ROML01  ROMU01|
|TEST  ROM1                                    |
|SERV                                          |
|RESET ROM2   50MHz                 14.31818MHz|
|----------------------------------------------|
Notes:
      E1-16T clock : 50.000MHz
      M6295 clock  : 1.7897725MHz (14.31818/8). Sample Rate = 1789772.5 / 132
      YM2151 clock : 3.579545MHz (14.31818/4). Chip stamped 'KA51' on one PCB, BS901 on another
      VSync        : 60Hz
      N341256      : NKK N341256SJ-15 32K x8 SRAM (SOJ28)
      GM71C18163   : LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)

      ROMs:
           ROML00/01, ROMU00/01 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
           VROM1                - Macronix MX27C2000 2MBit DIP32 EPROM
           ROM1/2               - ST M27C4001 4MBit DIP32 EPROM
*/

ROM_START( suplup ) /* version 4.0 / 990518 - also has 'Puzzle Bang Bang' title but it can't be selected */
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "suplup-rom1.bin", 0x00000, 0x80000, CRC(61fb2dbe) SHA1(21cb8f571b2479de6779b877b656d1ffe5b3516f) )
	ROM_LOAD( "suplup-rom2.bin", 0x80000, 0x80000, CRC(0c176c57) SHA1(f103a1afc528c01cbc18639273ab797fb9afacb1) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "suplup-roml00.bin", 0x000000, 0x200000, CRC(7848e183) SHA1(1db8f0ea8f73f42824423d382b37b4d75fa3e54c) )
	ROM_LOAD32_WORD( "suplup-romu00.bin", 0x000002, 0x200000, CRC(13e3ab7f) SHA1(d5b6b15ca5aef2e2788d2b81e0418062f42bf2f2) )
	ROM_LOAD32_WORD( "suplup-roml01.bin", 0x400000, 0x200000, CRC(15769f55) SHA1(2c13e8da2682ccc7878218aaebe3c3c67d163fd2) )
	ROM_LOAD32_WORD( "suplup-romu01.bin", 0x400002, 0x200000, CRC(6687bc6f) SHA1(cf842dfb2bcdfda0acc0859985bdba91d4a80434) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* Default EEPROM */
	ROM_LOAD( "eeprom-suplup.bin", 0x0000, 0x0080, CRC(e60c9883) SHA1(662dd8fb85eb97a8a4d53886198b269a5f6a6268) )
ROM_END

ROM_START( luplup ) /* version 3.0 / 990128 */
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "luplup-rom1.v30", 0x00000, 0x80000, CRC(9ea67f87) SHA1(73d16c056a8d64743181069a01559a43fee529a3) )
	ROM_LOAD( "luplup-rom2.v30", 0x80000, 0x80000, CRC(99840155) SHA1(e208f8731c06b634e84fb73e04f6cdbb8b504b94) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "luplup-roml00", 0x000000, 0x200000, CRC(8e2c4453) SHA1(fbf7d72263beda2ef90bccf0369d6e93e76d45b2) )
	ROM_LOAD32_WORD( "luplup-romu00", 0x000002, 0x200000, CRC(b57f4ca5) SHA1(b968c44a0ceb3274e066fa1d057fb6b017bb3fd3) )
	ROM_LOAD32_WORD( "luplup-roml01", 0x400000, 0x200000, CRC(40e85f94) SHA1(531e67eb4eedf47b0dded52ba2f4942b12cbbe2f) )
	ROM_LOAD32_WORD( "luplup-romu01", 0x400002, 0x200000, CRC(f2645b78) SHA1(b54c3047346c0f40dba0ba23b0d607cc53384edb) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "gal22v10b.gal1", 0x0000, 0x02e5, NO_DUMP ) /* GAL is read protected */
ROM_END


ROM_START( luplup29 ) /* version 2.9 / 990108 */
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "luplup-rom1.v29", 0x00000, 0x80000, CRC(36a8b8c1) SHA1(fed3eb2d83adc1b071a12ce5d49d4cab0ca20cc7) )
	ROM_LOAD( "luplup-rom2.v29", 0x80000, 0x80000, CRC(50dac70f) SHA1(0e313114a988cb633a89508fda17eb09023827a2) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "luplup29-roml00", 0x000000, 0x200000, CRC(08b2aa75) SHA1(7577b3ab79c54980307a83186dd1500f044c1bc8) )
	ROM_LOAD32_WORD( "luplup29-romu00", 0x000002, 0x200000, CRC(b57f4ca5) SHA1(b968c44a0ceb3274e066fa1d057fb6b017bb3fd3) )
	ROM_LOAD32_WORD( "luplup29-roml01", 0x400000, 0x200000, CRC(41c7ca8c) SHA1(55704f9d54f31bbaa044cd9d10ac2d9cb5e8fb70) )
	ROM_LOAD32_WORD( "luplup29-romu01", 0x400002, 0x200000, CRC(16746158) SHA1(a5036a7aaa717fde89d62b7ff7a3fded8b7f5cda) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )
ROM_END


ROM_START( puzlbang ) /* version 2.8 / 990106 - Korea only, cannot select title or language */
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "pbb-rom1.v28", 0x00000, 0x80000, CRC(fd21c5ff) SHA1(bc6314bbb2495c140788025153c893d5fd00bdc1) )
	ROM_LOAD( "pbb-rom2.v28", 0x80000, 0x80000, CRC(490ecaeb) SHA1(2b0f25e3d681ddf95b3c65754900c046b5b50b09) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "pbbang28-roml00", 0x000000, 0x200000, CRC(08b2aa75) SHA1(7577b3ab79c54980307a83186dd1500f044c1bc8) )
	ROM_LOAD32_WORD( "pbbang28-romu00", 0x000002, 0x200000, CRC(b57f4ca5) SHA1(b968c44a0ceb3274e066fa1d057fb6b017bb3fd3) )
	ROM_LOAD32_WORD( "pbbang28-roml01", 0x400000, 0x200000, CRC(41c7ca8c) SHA1(55704f9d54f31bbaa044cd9d10ac2d9cb5e8fb70) )
	ROM_LOAD32_WORD( "pbbang28-romu01", 0x400002, 0x200000, CRC(16746158) SHA1(a5036a7aaa717fde89d62b7ff7a3fded8b7f5cda) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(34a56987) SHA1(4d8983648a7f0acf43ff4c9c8aa6c8640ee2bbfe) )
ROM_END

/*

Jumping Break
F2 System, 1999

sequel to "Die Break" from the Bestri 3 game collection (see crospang.c)

F-E1-16-002
+----------------------------------------------+
|     VR1                   M6295  VROM1 28MHz |
|                 YM3012                       |
|                 YM2151            MEM2       |
|                                   MEM3       |
|               CRAM1               MEM5       |
|               CRAM2               MEM7       |
|J                                             |
|A              MEM1U  +----------++----------+|
|M                     |          ||          ||
|M              MEM1L  |Quicklogic||Quicklogic||
|A                     | QL2003-  || QL2003-  ||
|                      | XPL84C   || XPL84C   ||
|                      |          ||          ||
|                      +----------++----------+|
|                                              |
| 93C46          DRAM1      ROM1 ROML00  ROMU00|
|P1 P2   50MHz   E1-16T     ROM2 ROML01  ROMU01|
|                                              |
+----------------------------------------------+

Notes:
CPU: Hyperstone E1-16T @ 50.000MHz

     DRAM1 - LG Semi GM71C18163 1M x16 EDO DRAM (SOJ44)
MEMx/CRAMx - NKK N341256SJ-15 32K x8 SRAM (SOJ28)

Oki M6295 rebaged as AD-65
YM3012/YM2151 rebaged as KA12/BS901

 P1 - Setup push button
 P2 - Reset push button
VR1 - Volume adjust pot

ROMs:
    ROML00/01, ROMU00/01 - Macronix MX29F1610MC-12 SOP44 16MBit FlashROM
    VROM1                - AMD 27C020 2MBit DIP32 EPROM
    ROM1/2               - TMS 27C040 4MBit DIP32 EPROM

*/

ROM_START( jmpbreak )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1.bin", 0x00000, 0x80000, CRC(7e237f7d) SHA1(042e672be34644311eefc7b998bcdf6a9ea2c28a) )
	ROM_LOAD( "rom2.bin", 0x80000, 0x80000, CRC(c722f7be) SHA1(d8b3c6b5fd0942147e0a61169c3eb6334a3b5a40) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00.bin", 0x000000, 0x200000, CRC(4b99190a) SHA1(30af068f7d9f9f349db5696c19ab53ac33304271) )
	ROM_LOAD32_WORD( "romu00.bin", 0x000002, 0x200000, CRC(e93762f8) SHA1(cc589b59e3ab7aa7092e96a1ff8a9de8a499b257) )
	ROM_LOAD32_WORD( "roml01.bin", 0x400000, 0x200000, CRC(6796a104) SHA1(3f7352cd37f78c1b01f7df45344ee7800db110f9) )
	ROM_LOAD32_WORD( "romu01.bin", 0x400002, 0x200000, CRC(0cc907c8) SHA1(86029eca0870f3b7dd4f1ee8093ccb09077cc00b) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1.bin", 0x00000, 0x40000, CRC(1b6e3671) SHA1(bd601460387b56c989785ae03d5bb3c6cdb30a50) )
ROM_END

/*

Cool Minigame Collection
SemiCom, 1999

PCB Layout
----------

F-E1-16-008
|-------------------------------------------------------|
|UPC1241            YM3012   VROM1                      |
|      LM324  LM324 YM2151                              |
|               MCM6206       M6295   ROML00    ROMU00  |
|                                                       |
|               MCM6206               ROML01    ROMU01  |
|                                                       |
|J              MCM6206               ROML02    ROMU02  |
|A                                                      |
|M              MCM6206               ROML03    ROMU03  |
|M                                                      |
|A              MCM6206                                 |
|                                                       |
|               MCM6206       QL2003    QL2003          |
|                                                28MHz  |
|               MCM6206                                 |
|                                                       |
|               MCM6206  E1-16T   GM71C1816     ROM1    |
|                                                       |
|              93C46                            ROM2    |
|RESET  TEST          50MHz              PAL            |
|-------------------------------------------------------|

*/

ROM_START( coolmini )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "cm-rom1.040", 0x00000, 0x80000, CRC(9688fa98) SHA1(d5ebeb1407980072f689c3b3a5161263c7082e9a) )
	ROM_LOAD( "cm-rom2.040", 0x80000, 0x80000, CRC(9d588fef) SHA1(7b6b0ba074c7fa0aecda2b55f411557b015522b6) )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(4b141f31) SHA1(cf4885789b0df67d00f9f3659c445248c4e72446) )
	ROM_LOAD32_WORD( "romu00", 0x000002, 0x200000, CRC(9b2fb12a) SHA1(8dce367c4c2cab6e84f586bd8dfea3ea0b6d7225) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(1e3a04bb) SHA1(9eb84b6a0172a8868f440065c30b4519e0c3fe33) )
	ROM_LOAD32_WORD( "romu01", 0x400002, 0x200000, CRC(06dd1a6c) SHA1(8c707d388848bc5826fbfc48c3035fdaf5018515) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(1e8c12cb) SHA1(f57489e81eb1e476939148cfc8d03f3df03b2a84) )
	ROM_LOAD32_WORD( "romu02", 0x800002, 0x200000, CRC(4551d4fc) SHA1(4ec102120ab99e324d9574bfce93837d8334da06) )
	ROM_LOAD32_WORD( "roml03", 0xc00000, 0x200000, CRC(231650bf) SHA1(065f742a37d5476ec6f72f0bd8ba2cfbe626b872) )
	ROM_LOAD32_WORD( "romu03", 0xc00002, 0x200000, CRC(273d5654) SHA1(0ae3d1c4c4862a8642dbebd7c955b29df29c4938) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "cm-vrom1.020", 0x00000, 0x40000, CRC(fcc28081) SHA1(44031df0ee28ca49df12bcb73c83299fac205e21) )
ROM_END

/*

Date Quiz Go Go Episode 2
SemiCom, 2000

F-E1-16-010

*/

ROM_START( dquizgo2 )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1",         0x00000, 0x080000, CRC(81eef038) SHA1(9c925d1ef261ea85069925ccd1a5aeb939f55d5a) )
	ROM_LOAD( "rom2",         0x80000, 0x080000, CRC(e8789d8a) SHA1(1ee26c26cc7024c5df9d0da630b326021ece9f41) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(de811dd7) SHA1(bf31e165440ed2e3cdddd2174521b15afd8b2e69) )
	ROM_LOAD32_WORD( "romu00", 0x000002, 0x200000, CRC(2bdbfc6b) SHA1(8e755574e3c9692bd8f82c7351fe3623a31ec136) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(f574a2a3) SHA1(c6a8aca75bd3a4e4109db5095f3a3edb9b1e6657) )
	ROM_LOAD32_WORD( "romu01", 0x400002, 0x200000, CRC(d05cf02f) SHA1(624316d4ee42c6257bc64747e4260a0d3950f9cd) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(43ca2cff) SHA1(02ad7cce42d917dbefdba2e4e8886fc883b1dc60) )
	ROM_LOAD32_WORD( "romu02", 0x800002, 0x200000, CRC(b8218222) SHA1(1e1aa60e0de9c02b841896512a1163dda280c845) )
	/* roml03 empty */
	/* romu03 empty */

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1", 0x00000, 0x40000, CRC(24d5b55f) SHA1(cb4d3a22440831e37df0a7fe5433bea708d60f31) )
ROM_END

/*

Wyvern Wings (c) 2001 SemiCom, Game Vision License

   CPU: Hyperstone E1-32T
 Video: 2 QuickLogic QL12x16B-XPL84 FPGA
 Sound: AdMOS QDSP1000 with QDSP QS1001A sample rom
   OSC: 50MHz, 28MHz & 24MHz
EEPROM: 93C46

F-E1-32-010-D
+------------------------------------------------------------------+
|    VOL    +-------+  +---------+                                 |
+-+         | QPSD  |  |  U15A   |      +---------+   +---------+  |
  |         |QS1001A|  |         |      | ROMH00  |   | ROML00  |  |
+-+         +-------+  +---------+      |         |   |         |  |
|           +-------+                   +---------+   +---------+  |
|           |QPSD   |   +----------+    +---------+   +---------+  |
|           |QS1000 |   |    U7    |    | ROMH01  |   | ROML01  |  |
|J   24MHz  +-------+   +----------+    |         |   |         |  |
|A                                      +---------+   +---------+  |
|M   50MHz           +-----+            +---------+   +---------+  |
|M                   |DRAM2|            | ROMH02  |   | ROML02  |  |
|A     +----------+  +-----+    +-----+ |         |   |         |  |
|      |          |  +-----+    |93C46| +---------+   +---------+  |
|C     |HyperStone|  |DRAM1|    +-----+ +---------+   +---------+  |
|O     |  E1-32T  |  +-----+            | ROMH03  |   | ROML03  |  |
|N     |          |              28MHz  |         |   |         |  |
|N     +----------+                     +---------+   +---------+  |
|E                                                                 |
|C           +----------+           +------------+ +------------+  |
|T           |   GAL1   |           | QuickLogic | | QuickLogic |  |
|O           +----------+           | 0048 BH    | | 0048 BH    |  |
|R           +----------+           | QL12X16B   | | QL12X16B   |  |
|            |   ROM2   |           | -XPL84C    | | -XPL84C    |  |
|            +----------+           +------------+ +------------+  |
|            +----------+            +----+                        |
|            |   ROM1   |            |MEM3|                        |
+-++--+      +----------+            +----+                        |
  ||S1|    +-----+                   |MEM2|                        |
+-++--+    |CRAM2|                   +----+                        |
|  +--+    +-----+                   |MEM7|                        |
|  |S2|    |CRAM1|                   +----+                        |
|  +--+    +-----+                   |MEM6|                        |
+------------------------------------+----+------------------------+

S1 is the setup button
S2 is the reset button

ROMH & ROML are all MX 29F1610MC-16 flash roms
u15A is a MX 29F1610MC-16 flash rom
u7 is a ST 27c1001
ROM1 & ROM2 are both ST 27c4000D

*/

ROM_START( wyvernwg )
	ROM_REGION32_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom1.bin", 0x000000, 0x080000, CRC(66bf3a5c) SHA1(037d5e7a6ef6f5b4ac08a9c811498c668a9d2522) )
	ROM_LOAD( "rom2.bin", 0x080000, 0x080000, CRC(fd9b5911) SHA1(a01e8c6e5a9009024af385268ba3ba90e1ebec50) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* QDSP ('51) Code */
	ROM_LOAD( "u7", 0x0000, 0x20000, CRC(00a3f705) SHA1(f0a6bafd16bea53d4c05c8cc108983cbd41e5757) )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(fb3541b6) SHA1(4f569ac7bde92c5febf005ab73f76552421ec223) )
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(516aca48) SHA1(42cf5678eb4c0ee7da2ab0bd66e4e34b2735c75a) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(1c764f95) SHA1(ba6ac1376e837b491bc0269f2a1d10577a3d40cb) )
	ROM_LOAD32_WORD( "romh01", 0x400002, 0x200000, CRC(fee42c63) SHA1(a27b5cbca0defa9be85fee91dde1273f445d3372) )
	ROM_LOAD32_WORD( "roml02", 0x800000, 0x200000, CRC(fc846707) SHA1(deaee15ab71927f644dcf576959e2ceaa55bfd44) )
	ROM_LOAD32_WORD( "romh02", 0x800002, 0x200000, CRC(86141c7d) SHA1(22a82cc7d44d655b03867503a83e81f7c82d6c91) )
	ROM_LOAD32_WORD( "roml03", 0xc00000, 0x200000, CRC(b10bf37c) SHA1(6af835b1e2573f0bb2c17057e016a7aecc8fcde8) )
	ROM_LOAD32_WORD( "romh03", 0xc00002, 0x200000, CRC(e01c2a92) SHA1(f53c2db92d62f595d473b1835c46d426f0dbe6b3) )

	ROM_REGION( 0x200000, "sfx", 0 ) /* Music data / QDSP samples (SFX) */
	ROM_LOAD( "romsnd.u15a",  0x000000, 0x200000, CRC(fc89eedc) SHA1(2ce28bdb773cfa5b5660e4c0a9ef454cb658f2da) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

/*

Mission Craft
Sun, 2000

PCB Layout
----------

SUN2000
|---------------------------------------------|
|       |------|  SND-ROM1     ROMH00  ROMH01 |
|       |QDSP  |                              |
|       |QS1001|                              |
|DA1311A|------|  SND-ROM2                    |
|       /------\                              |
|       |QDSP  |               ROML00  ROML01 |
|       |QS1000|                              |
|  24MHz\------/                              |
|                                 |---------| |
|                                 | ACTEL   | |
|J               62256            |A40MX04-F| |
|A  *  PRG-ROM2  62256            |PL84     | |
|M   PAL                          |         | |
|M                    62256 62256 |---------| |
|A                    62256 62256             |
|             |-------|           |---------| |
|             |GMS    |           | ACTEL   | |
|  93C46      |30C2116|           |A40MX04-F| |
|             |       | 62256     |PL84     | |
|  HY5118164C |-------| 62256     |         | |
|                                 |---------| |
|SW2                                          |
|SW1                                          |
|   50MHz                              28MHz  |
|---------------------------------------------|
Notes:
      GMS30C2116 - based on Hyperstone technology, clock running at 50.000MHz
      QS1001A    - Wavetable audio chip, 1M ROM, manufactured by AdMOS (Now LG Semi.), SOP32
      QS1000     - Wavetable audio chip manufactured by AdMOS (Now LG Semi.), QFP100
                   provides Creative Waveblaster functionality and General Midi functions
      SW1        - Used to enter test mode
      SW2        - PCB Reset
      *          - Empty socket for additional program ROM

*/

ROM_START( misncrft )
	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "prg-rom2.bin", 0x80000, 0x80000, CRC(059ae8c1) SHA1(2c72fcf560166cb17cd8ad665beae302832d551c) )

	ROM_REGION( 0x400000, "audiocpu", 0 )	/* i8052 code */
	ROM_LOAD( "snd-rom2.us1", 0x00000, 0x20000, CRC(8821e5b9) SHA1(4b8df97bc61b48aa16ed411614fcd7ed939cac33) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(748c5ae5) SHA1(28005f655920e18c82eccf05c0c449dac16ee36e) )
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(f34ae697) SHA1(2282e3ef2d100f3eea0167b25b66b35a64ddb0f8) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(e37ece7b) SHA1(744361bb73905bc0184e6938be640d3eda4b758d) )
	ROM_LOAD32_WORD( "romh01", 0x400002, 0x200000, CRC(71fe4bc3) SHA1(08110b02707e835bf428d343d5112b153441e255) )

	ROM_REGION( 0x80000, "sfx", 0 )
	ROM_LOAD( "snd-rom1.u15", 0x00000, 0x80000, CRC(fb381da9) SHA1(2b1a5447ed856ab92e44d000f27a04d981e3ac52) )

	ROM_REGION( 0x80000, "wavetable", 0 )
	ROM_LOAD( "qs1001a.u17", 0x00000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-misncrft.bin", 0x0000, 0x0080, CRC(9ad27077) SHA1(7f0e98eff9cf6e1b60c19fc1016b888e50b087e0) )
ROM_END

/*

Final Godori (c) SemiCom

SEMICOM-003a

+---------------------------------------------+
|                     +------+                |
|            YM3012   |  U7  |                |
|                     +------+                |
|            YM2151   M6295                   |
|                                             |
|            +-----+      MEM1l  +----------+ |
|            |CRAM2|             |QuickLogic| |
|            +-----+             | QL12X16B | |
|            +-----+             | XPL84C   | |
|  +-------+ |CRAM2|      MEM1U  |          | |
|J | DRAM1 | +-----+             +----------+ |
|A +-------+ +----------+ MEM3                |
|M +-------+ |          |        +----------+ |
|M | DRAM2 | |HyperStone| MEM7   |QuickLogic| |
|A +-------+ |  E1-32T  |        | QL12X16B | |
|            |          | MEM6   | XPL84C   | |
|     PAL    +----------+        |          | |
|                         MEM2   +----------+ |
|SW1 SW2       61L256S                        |
|        ROM0*  +--------+ +--------+  28MHz  |
|        ROM1   | ROML00 | | ROMH00 |  +-----+|
|               +--------+ +--------+  |93C46||
|   50MHz         ROML01*    ROMH01*   +-----+|
|                                             |
+---------------------------------------------+

ROM1 & U7 are 27C040
ROML00 & ROMH00 are MX 29F1610MC flashroms
ROM0, ROML01 & ROMH01 are unpopulated
YM2151, YM3012 & M6295 badged as BS901, BS902 U6295
CRAM are MCM6206BAEJ15
DRAM are KM416C1204AJ-6
MEM are MCM6206BAEJ15
61L256S - 32K x 8 bit High Speed CMOS SRAM (game's so called "Backup Data")

SW1 is the reset button
SW2 is the setup button

*/

ROM_START( finalgdr ) /* version 2.20.5915, Korea only */
	ROM_REGION32_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* rom0 empty */
	ROM_LOAD( "rom1", 0x080000, 0x080000, CRC(45815931) SHA1(80ba7a366994e40a1f520ea18fad82e6b068b279) )

	ROM_REGION( 0x800000, "gfx1", 0 )  /* gfx data */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(8334459d) SHA1(70ad560dada8aa8ce192e5307bd805744b82fcfe) )
	ROM_LOAD32_WORD( "romh00", 0x000002, 0x200000, CRC(f28578a5) SHA1(a5c7b17aff101f1f4f52657d0567a6c9d12a178d) )
	/* roml01 empty */
	/* romh01 empty */

	ROM_REGION( 0x080000, "user2", 0 ) /* Oki Samples */
	ROM_LOAD( "u7", 0x000000, 0x080000, CRC(080f61f8) SHA1(df3764b1b07f9fc38685e3706b0f834f62088727) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x0a0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0e0000, 0x020000)
ROM_END

/*
Age Of Heroes - Silkroad 2
Unico, 2001

PCB Layout
----------

|----------------------------------------|
|UPC1241H  BA4558     32MHz         G05  |
|TL084  YM3012  ROM3   |--------|   G06  |
|VOL ULN2003  M6295(1) |A40MX04 |   G07  |
|       YM2151  ROM4   |PL84    |   G08  |
|     93C46   M6295(2) |        |        |
|J  3.579545MHz 62256  |--------|        |
|A     HY18CV8  62256               G09  |
|M     GAL22V10           EPM7128   G10  |
|M      |--------|20MHz             G11  |
|A      |        |                  G12  |
|       |E1-32XN |                       |
|RESET  |        | 62256            62256|
|TEST   |--------| 62256            62256|
|ROM1    HY5118164                  62256|
|ROM2    HY5118164                  62256|
|----------------------------------------|
Notes:
      E1-32XN  - Hyperstone CPU, clock input 20.000MHz (QFP160)
      A40MX04  - Actel A40MX04-F FPGA (PLCC84)
      EPM7128  - Altera MAX EPM7128TC100 CPLD (TQFP100)
      YM2151   - clock 3.579545MHz (DIP24)
      M6295(1) - clock 4.000MHz [32/8] pin 7 HIGH (QFP44)
      M6295(2) - clock 1.000MHz [32/32] pin 7 HIGH (QFP44)
      YM3012   - clock 1.7897725MHz [3.579545/2] (DIP16)
      TL084    - Texas Instruments TL084 Quad JFET-Input General-Purpose Operational Amplifier (DIP8)
      BA4558   - Rohm BA4558 Dual Operational Amplifier (DIP8)
      93C46    - 128 bytes x8 EEPROM (DIP8)
      HY5118164- Hynix 1M x16 EDO DRAM (SOJ42)
      62256    - 32k x8 SRAM (DIP28)
      ROM1/2   - Main program ROMs, type MX29F1610MC-12 (SOP44)
      ROM3/4   - M6295 Sound Data ROMs, ROM3 = 27C020, ROM4 = 27C040 (both DIP32)
      G05-G12  - GFX Data ROMs, type Intel E28F640J3A120 64M x8 FlashROM (TSOP56)
      VSync    - 59.185Hz   \
      HSync    - 15.625kHz / via EL4583 & TTi PFM1300
*/

ROM_START( aoh )
	ROM_REGION32_BE( 0x400000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	ROM_LOAD16_WORD_SWAP( "rom1", 0x000000, 0x200000, CRC(2e55ff55) SHA1(b2b7605b87ee609dfbc7c21dfae0ef8d847019f0) )
	ROM_LOAD16_WORD_SWAP( "rom2", 0x200000, 0x200000, CRC(50f8a409) SHA1(a8171b7cf59dd01de1e512ab21607b4f330f40b8) )

	ROM_REGION( 0x4000000, "gfx1", 0 ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "g05", 0x0000002, 0x800000, CRC(64c8f493) SHA1(d487a74c813abbd0a612f8346eed8a7c3ff3e84e) )
	ROM_LOAD32_WORD( "g09", 0x0000000, 0x800000, CRC(c359febb) SHA1(7955385748e24dd076bc4f954b193a53c0a729c5) )
	ROM_LOAD32_WORD( "g06", 0x1000002, 0x800000, CRC(ffbc9fe5) SHA1(5e0e5cfdf6af23db0733c9fedee9c5f9ccde1109) )
	ROM_LOAD32_WORD( "g10", 0x1000000, 0x800000, CRC(08217573) SHA1(10cecdfc3a1ef835a62325b023d3bca8d0aea67d) )
	ROM_LOAD32_WORD( "g07", 0x2000002, 0x800000, CRC(5cb3c86a) SHA1(2e89f467c1a220f2510977677215e040295c3dd0) )
	ROM_LOAD32_WORD( "g11", 0x2000000, 0x800000, CRC(5f0461b8) SHA1(a0ac37d9a357e69367b8fee68bc358bfea5ecca0) )
	ROM_LOAD32_WORD( "g08", 0x3000002, 0x800000, CRC(1fd08aa0) SHA1(376a91220cd6e63418b04d590b232bb1079a40c7) )
	ROM_LOAD32_WORD( "g12", 0x3000000, 0x800000, CRC(e437b35f) SHA1(411d2926d619fba057476864f0e580f608830522) )

	ROM_REGION( 0x40000, "oki_1", 0 ) /* Oki Samples */
	ROM_LOAD( "rom3", 0x00000, 0x40000, CRC(db8cb455) SHA1(6723b4018208d554bd1bf1e0640b72d2f4f47302) )

	ROM_REGION( 0x80000, "user2", 0 ) /* Oki Samples */
	ROM_LOAD( "rom4", 0x00000, 0x80000, CRC(bba47755) SHA1(e6eeb5f64eaa88a74536119b731a76921e79f8ff) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki_2", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x0a0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0e0000, 0x020000)
ROM_END

static int irq_active(const address_space *space)
{
	UINT32 FCR = cpu_get_reg(space->cpu, 27);
	if( !(FCR&(1<<29)) ) // int 2 (irq 4)
		return 1;
	else
		return 0;
}

static READ16_HANDLER( vamphalf_speedup_r )
{
	if(cpu_get_pc(space->cpu) == 0x82de)
	{
		if(irq_active(space))
			cpu_spinuntil_int(space->cpu);
		else
			cpu_eat_cycles(space->cpu, 50);
	}

	return wram[(0x4a840/2)+offset];
}

static READ16_HANDLER( vamphafk_speedup_r )
{
	if(cpu_get_pc(space->cpu) == 0x82de)
	{
		if(irq_active(space))
			cpu_spinuntil_int(space->cpu);
		else
			cpu_eat_cycles(space->cpu, 50);
	}

	return wram[(0x4a6d0/2)+offset];
}

static READ16_HANDLER( misncrft_speedup_r )
{
	if(cpu_get_pc(space->cpu) == 0xecc8)
	{
		if(irq_active(space))
			cpu_spinuntil_int(space->cpu);
		else
			cpu_eat_cycles(space->cpu, 50);
	}

	return wram[(0x72eb4/2)+offset];
}

static READ16_HANDLER( coolmini_speedup_r )
{
	if(cpu_get_pc(space->cpu) == 0x75f7a)
	{
		if(irq_active(space))
			cpu_spinuntil_int(space->cpu);
		else
			cpu_eat_cycles(space->cpu, 50);
	}

	return wram[(0xd2e80/2)+offset];
}

static READ16_HANDLER( suplup_speedup_r )
{
	if(cpu_get_pc(space->cpu) == 0xaf18a )
	{
		if(irq_active(space))
			cpu_spinuntil_int(space->cpu);
		else
			cpu_eat_cycles(space->cpu, 50);
	}

	return wram[(0x11605c/2)+offset];
}

static READ16_HANDLER( luplup_speedup_r )
{
	if(cpu_get_pc(space->cpu) == 0xaefac )
	{
		if(irq_active(space))
			cpu_spinuntil_int(space->cpu);
		else
			cpu_eat_cycles(space->cpu, 50);
	}

	return wram[(0x115e84/2)+offset];
}

static READ16_HANDLER( luplup29_speedup_r )
{
	if(cpu_get_pc(space->cpu) == 0xae6c0 )
	{
		if(irq_active(space))
			cpu_spinuntil_int(space->cpu);
		else
			cpu_eat_cycles(space->cpu, 50);
	}

	return wram[(0x113f08/2)+offset];
}

static READ16_HANDLER( puzlbang_speedup_r )
{
	if(cpu_get_pc(space->cpu) == 0xae6d2 )
	{
		if(irq_active(space))
			cpu_spinuntil_int(space->cpu);
		else
			cpu_eat_cycles(space->cpu, 50);
	}

	return wram[(0x113ecc/2)+offset];
}

static READ32_HANDLER( wyvernwg_speedup_r )
{
	if(cpu_get_pc(space->cpu) == 0x10758 )
	{
		if(irq_active(space))
			cpu_spinuntil_int(space->cpu);
		else
			cpu_eat_cycles(space->cpu, 50);
	}

	return wram32[0x00b56fc/4];
}

static READ32_HANDLER( finalgdr_speedup_r )
{
	if(cpu_get_pc(space->cpu) == 0x1c212 )
	{
		if(irq_active(space))
			cpu_spinuntil_int(space->cpu);
		else
			cpu_eat_cycles(space->cpu, 50);
	}

	return wram32[0x005e874/4];
}

static READ16_HANDLER( dquizgo2_speedup_r )
{
	if(cpu_get_pc(space->cpu) == 0xaa622)
	{
		if(irq_active(space))
			cpu_spinuntil_int(space->cpu);
		else
			cpu_eat_cycles(space->cpu, 50);
	}

	return wram[(0xcde70/2)+offset];
}

static READ32_HANDLER( aoh_speedup_r )
{
	if(cpu_get_pc(space->cpu) == 0xb994 )
	{
		cpu_eat_cycles(space->cpu, 500);
	}
	else if (cpu_get_pc(space->cpu) == 0xba40 )
	{
		cpu_eat_cycles(space->cpu, 500);
	}


	return wram32[0x28a09c/4];
}

static READ16_HANDLER( jmpbreak_speedup_r )
{
	if(cpu_get_pc(space->cpu) == 0x983c)
	{
		if(irq_active(space))
			cpu_spinuntil_int(space->cpu);
		else
			cpu_eat_cycles(space->cpu, 50);
	}

	return wram[(0x00906fc / 2)+offset];
}

static DRIVER_INIT( vamphalf )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0004a840, 0x0004a843, 0, 0, vamphalf_speedup_r );

	palshift = 0;
	flip_bit = 0x80;
}

static DRIVER_INIT( vamphafk )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0004a6d0, 0x0004a6d3, 0, 0, vamphafk_speedup_r );

	palshift = 0;
	flip_bit = 0x80;
}

static DRIVER_INIT( misncrft )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00072eb4, 0x00072eb7, 0, 0, misncrft_speedup_r );

	palshift = 0;
	flip_bit = 1;
}

static DRIVER_INIT( coolmini )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x000d2e80, 0x000d2e83, 0, 0, coolmini_speedup_r );

	palshift = 0;
	flip_bit = 1;
}

static DRIVER_INIT( suplup )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0011605c, 0x0011605f, 0, 0, suplup_speedup_r );

	palshift = 8;
	/* no flipscreen */
}

static DRIVER_INIT( luplup )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00115e84, 0x00115e87, 0, 0, luplup_speedup_r );

	palshift = 8;
	/* no flipscreen */
}

static DRIVER_INIT( luplup29 )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00113f08, 0x00113f0b, 0, 0, luplup29_speedup_r );

	palshift = 8;
	/* no flipscreen */
}

static DRIVER_INIT( puzlbang )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00113ecc, 0x00113ecf, 0, 0, puzlbang_speedup_r );

	palshift = 8;
	/* no flipscreen */
}

static DRIVER_INIT( wyvernwg )
{
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00b56fc, 0x00b56ff, 0, 0, wyvernwg_speedup_r );

	palshift = 0;
	flip_bit = 1;

	semicom_prot_idx = 8;
	semicom_prot_data[0] = 2;
	semicom_prot_data[1] = 1;
}

static DRIVER_INIT( finalgdr )
{
	finalgdr_backupram_bank = 1;
	finalgdr_backupram = auto_alloc_array(machine, UINT8, 0x80*0x100);
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x005e874, 0x005e877, 0, 0, finalgdr_speedup_r );

	palshift = 0;
	flip_bit = 1; //?

	semicom_prot_idx = 8;
	semicom_prot_data[0] = 2;
	semicom_prot_data[1] = 3;
}

static DRIVER_INIT( dquizgo2 )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00cde70, 0x00cde73, 0, 0, dquizgo2_speedup_r );

	palshift = 0;
	flip_bit = 1;
}

static DRIVER_INIT( aoh )
{
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x028a09c, 0x028a09f, 0, 0, aoh_speedup_r );

	palshift = 0;
	/* no flipscreen */
}

static DRIVER_INIT( jmpbreak )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00906fc, 0x00906ff, 0, 0, jmpbreak_speedup_r );
	memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xe0000000, 0xe0000003, 0, 0, jmpbreak_flipscreen_w );

	palshift = 0;
}

GAME( 1999, coolmini, 0,        coolmini, common,   coolmini, ROT0,   "SemiCom",           "Cool Minigame Collection", 0 )
GAME( 1999, jmpbreak, 0,        jmpbreak, common,   jmpbreak, ROT0,   "F2 System",         "Jumping Break" , 0 )
GAME( 1999, suplup,   0,        suplup,   common,   suplup,   ROT0,   "Omega System",      "Super Lup Lup Puzzle / Zhuan Zhuan Puzzle (version 4.0 / 990518)" , 0 )
GAME( 1999, luplup,   suplup,   suplup,   common,   luplup,   ROT0,   "Omega System",      "Lup Lup Puzzle / Zhuan Zhuan Puzzle (version 3.0 / 990128)", 0 )
GAME( 1999, luplup29, suplup,   suplup,   common,   luplup29, ROT0,   "Omega System",      "Lup Lup Puzzle / Zhuan Zhuan Puzzle (version 2.9 / 990108)", 0 )
GAME( 1999, puzlbang, suplup,   suplup,   common,   puzlbang, ROT0,   "Omega System",      "Puzzle Bang Bang (Korea, version 2.8 / 990106)", 0 )
GAME( 1999, vamphalf, 0,        vamphalf, common,   vamphalf, ROT0,   "Danbi & F2 System", "Vamf x1/2 (Europe)", 0 )
GAME( 1999, vamphalfk,vamphalf, vamphalf, common,   vamphafk, ROT0,   "Danbi & F2 System", "Vamp x1/2 (Korea)", 0 )
GAME( 2000, dquizgo2, 0,        coolmini, common,   dquizgo2, ROT0,   "SemiCom",           "Date Quiz Go Go Episode 2" , 0)
GAME( 2000, misncrft, 0,        misncrft, common,   misncrft, ROT90,  "Sun",               "Mission Craft (version 2.4)", GAME_NO_SOUND )
GAME( 2001, finalgdr, 0,        finalgdr, finalgdr, finalgdr, ROT0,   "SemiCom",           "Final Godori (Korea, version 2.20.5915)", 0 )
GAME( 2001, wyvernwg, 0,        wyvernwg, common,   wyvernwg, ROT270, "SemiCom (Game Vision License)", "Wyvern Wings", GAME_NO_SOUND )
GAME( 2001, aoh,      0,        aoh,      aoh,      aoh,      ROT0,   "Unico",             "Age Of Heroes - Silkroad 2 (v0.63 - 2001/02/07)", 0 )
