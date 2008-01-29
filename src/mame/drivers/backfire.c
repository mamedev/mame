/* Data East Backfire!

    Backfire!

    inputs are incomplete (p2 side.., alt control modes etc.)

    there may still be some problems with the 156 co-processor, but it seems to be mostly correct

    set 2 defaults to wheel controls, so until they're mapped you must change back to joystick in test mode

*/

#define DE156CPU ARM
extern void decrypt156(void);

#include "driver.h"
#include "deprecat.h"
#include "decocrpt.h"
#include "deco32.h"
#include "machine/eeprom.h"
#include "sound/okim6295.h"
#include "sound/ymz280b.h"
#include "cpu/arm/arm.h"
#include "deco16ic.h"
#include "rendlay.h"

static UINT32 *backfire_spriteram32_1;
static UINT32 *backfire_spriteram32_2;
static UINT32 *backfire_mainram;
static mame_bitmap * backfire_left;
static mame_bitmap * backfire_right;

//UINT32 *backfire_180010, *backfire_188010;
static UINT32 *backfire_left_priority, *backfire_right_priority;

/* I'm using the functions in deco16ic.c ... same chips, why duplicate code? */

static int backfire_bank_callback(int bank)
{
//  mame_printf_debug("bank callback %04x\n",bank); // bit 1 gets set too?

	bank = bank >> 4;

	bank = (bank & 1) |  ( (bank & 4) >> 1 ) | ((bank & 2) << 1);

	return bank * 0x1000;
}



static VIDEO_START(backfire)
{
	/* allocate the ram as 16-bit (we do it here because the CPU is 32-bit) */
	deco16_pf1_data = auto_malloc(0x2000);
	deco16_pf2_data = auto_malloc(0x2000);
	deco16_pf3_data = auto_malloc(0x2000);
	deco16_pf4_data = auto_malloc(0x2000);
	deco16_pf1_rowscroll = auto_malloc(0x0800);
	deco16_pf2_rowscroll = auto_malloc(0x0800);
	deco16_pf3_rowscroll = auto_malloc(0x0800);
	deco16_pf4_rowscroll = auto_malloc(0x0800);
	deco16_pf12_control = auto_malloc(0x10);
	deco16_pf34_control = auto_malloc(0x10);

	/* and register the allocated ram so that save states still work */
	state_save_register_global_pointer(deco16_pf1_data, 0x2000/2);
	state_save_register_global_pointer(deco16_pf2_data, 0x2000/2);
	state_save_register_global_pointer(deco16_pf3_data, 0x2000/2);
	state_save_register_global_pointer(deco16_pf4_data, 0x2000/2);
	state_save_register_global_pointer(deco16_pf1_rowscroll, 0x800/2);
	state_save_register_global_pointer(deco16_pf2_rowscroll, 0x800/2);
	state_save_register_global_pointer(deco16_pf3_rowscroll, 0x800/2);
	state_save_register_global_pointer(deco16_pf4_rowscroll, 0x800/2);
	state_save_register_global_pointer(deco16_pf12_control, 0x10/2);
	state_save_register_global_pointer(deco16_pf34_control, 0x10/2);

	deco16_2_video_init(0);

	deco16_pf1_colour_bank = 0x00;
	deco16_pf2_colour_bank = 0x40;
	deco16_pf3_colour_bank = 0x10;
	deco16_pf4_colour_bank = 0x50;

	deco16_set_tilemap_bank_callback(0, backfire_bank_callback);
	deco16_set_tilemap_bank_callback(1, backfire_bank_callback);
	deco16_set_tilemap_bank_callback(2, backfire_bank_callback);
	deco16_set_tilemap_bank_callback(3, backfire_bank_callback);

	backfire_left =  auto_bitmap_alloc(80*8, 32*8, BITMAP_FORMAT_INDEXED16);
	backfire_right = auto_bitmap_alloc(80*8, 32*8, BITMAP_FORMAT_INDEXED16);
}

static void draw_sprites(running_machine *machine,mame_bitmap *bitmap,const rectangle *cliprect, UINT32 *backfire_spriteram32, int region)
{
	int offs;

	flip_screen = 1;

	for (offs = (0x1400/4)-4;offs >= 0;offs -= 4) // 0x1400 for charlien
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult, pri;

		sprite = backfire_spriteram32[offs+1]&0xffff;

		y = backfire_spriteram32[offs]&0xffff;
		flash=y&0x1000;
		if (flash && (cpu_getcurrentframe() & 1)) continue;

		x = backfire_spriteram32[offs+2]&0xffff;
		colour = (x >>9) & 0x1f;

		pri = (x&0xc000); // 2 bits or 1?

		switch (pri&0xc000) {
			case 0x0000: pri=0;   break; // numbers, people, cars when in the air, status display..
			case 0x4000: pri=0xf0;break; // cars most of the time
			case 0x8000: pri=0;   break; // car wheels during jump?
			case 0xc000: pri=0xf0;break; /* car wheels in race? */
		}

		// pri 0 = ontop of everything//

//      pri = 0;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
        x = 304 - x;

		if (x>320) continue;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flip_screen)
		{
			y=240-y;
			x=304-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			pdrawgfx(bitmap,machine->gfx[region],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					cliprect,TRANSPARENCY_PEN,0,pri);

			multi--;
		}
	}
}



static VIDEO_UPDATE(backfire)
{
	/* screen 1 uses pf1 as the forground and pf3 as the background */
	/* screen 2 uses pf2 as the foreground and pf4 as the background */

	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);
	deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);

	if (screen==0)
	{

		fillbitmap(priority_bitmap,0,NULL);
		fillbitmap(bitmap,machine->pens[0x100],cliprect);

		if (backfire_left_priority[0] == 0)
		{
			deco16_tilemap_3_draw(bitmap,cliprect,0,1);
			deco16_tilemap_1_draw(bitmap,cliprect,0,2);
			draw_sprites(machine,bitmap,cliprect,backfire_spriteram32_1,3);
		}
		else if (backfire_left_priority[0] == 2)
		{
			deco16_tilemap_1_draw(bitmap,cliprect,0,2);
			deco16_tilemap_3_draw(bitmap,cliprect,0,4);
			draw_sprites(machine,bitmap,cliprect,backfire_spriteram32_1,3);
		}
		else
		{
			popmessage( "unknown left priority %08x", backfire_left_priority[0] );
		}
	}
	else if (screen==1)
	{
		fillbitmap(priority_bitmap,0,NULL);
		fillbitmap(bitmap,machine->pens[0x500],cliprect);

		if (backfire_right_priority[0] == 0)
		{
			deco16_tilemap_4_draw(bitmap,cliprect,0,1);
			deco16_tilemap_2_draw(bitmap,cliprect,0,2);
			draw_sprites(machine,bitmap,cliprect,backfire_spriteram32_2,4);
		}
		else if (backfire_right_priority[0] == 2)
		{
			deco16_tilemap_2_draw(bitmap,cliprect,0,2);
			deco16_tilemap_4_draw(bitmap,cliprect,0,4);
			draw_sprites(machine,bitmap,cliprect,backfire_spriteram32_2,4);
		}
		else
		{
			popmessage( "unknown right priority %08x", backfire_right_priority[0] );
		}
	}
	return 0;
}



static READ32_HANDLER(backfire_eeprom_r)
{
	/* some kind of screen indicator?  checked by backfira set before it will boot */
	int backfire_screen = mame_rand(Machine)&1;
	return ((EEPROM_read_bit()<<24) | readinputport(0) | (readinputport(3)<<16)) ^  (backfire_screen << 26) ;
}

static READ32_HANDLER(backfire_control2_r)
{
//  logerror("%08x:Read eprom %08x (%08x)\n",activecpu_get_pc(),offset<<1,mem_mask);
	return (EEPROM_read_bit()<<24) | readinputport(1) | (readinputport(1)<<16);
}

#ifdef UNUSED_FUNCTION
static READ32_HANDLER(backfire_control3_r)
{
//  logerror("%08x:Read eprom %08x (%08x)\n",activecpu_get_pc(),offset<<1,mem_mask);
	return (EEPROM_read_bit()<<24) | readinputport(2) | (readinputport(2)<<16);
}
#endif


static WRITE32_HANDLER(backfire_eeprom_w)
{
	logerror("%08x:write eprom %08x (%08x) %08x\n",activecpu_get_pc(),offset<<1,mem_mask,data);
	if (ACCESSING_LSB32) {
		EEPROM_set_clock_line((data & 0x2) ? ASSERT_LINE : CLEAR_LINE);
		EEPROM_write_bit(data & 0x1);
		EEPROM_set_cs_line((data & 0x4) ? CLEAR_LINE : ASSERT_LINE);
	}
}


static WRITE32_HANDLER(wcvol95_nonbuffered_palette_w)
{
	COMBINE_DATA(&paletteram32[offset]);
	palette_set_color_rgb(Machine,offset,pal5bit(paletteram32[offset] >> 0),pal5bit(paletteram32[offset] >> 5),pal5bit(paletteram32[offset] >> 10));
}


static READ32_HANDLER( deco156_snd_r )
{
	return YMZ280B_status_0_r(0);
}

static WRITE32_HANDLER( deco156_snd_w )
{
	if (offset)
		YMZ280B_data_0_w(0, data);
	else
		YMZ280B_register_0_w(0, data);
}

/* map 32-bit writes to 16-bit */

static READ32_HANDLER( backfire_pf1_rowscroll_r ) { return deco16_pf1_rowscroll[offset]^0xffff0000; }
static READ32_HANDLER( backfire_pf2_rowscroll_r ) { return deco16_pf2_rowscroll[offset]^0xffff0000; }
static READ32_HANDLER( backfire_pf3_rowscroll_r ) { return deco16_pf3_rowscroll[offset]^0xffff0000; }
static READ32_HANDLER( backfire_pf4_rowscroll_r ) { return deco16_pf4_rowscroll[offset]^0xffff0000; }
static WRITE32_HANDLER( backfire_pf1_rowscroll_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; COMBINE_DATA(&deco16_pf1_rowscroll[offset]); }
static WRITE32_HANDLER( backfire_pf2_rowscroll_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; COMBINE_DATA(&deco16_pf2_rowscroll[offset]); }
static WRITE32_HANDLER( backfire_pf3_rowscroll_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; COMBINE_DATA(&deco16_pf3_rowscroll[offset]); }
static WRITE32_HANDLER( backfire_pf4_rowscroll_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; COMBINE_DATA(&deco16_pf4_rowscroll[offset]); }
static READ32_HANDLER ( backfire_pf12_control_r ) { return deco16_pf12_control[offset]^0xffff0000; }
static READ32_HANDLER ( backfire_pf34_control_r ) { return deco16_pf34_control[offset]^0xffff0000; }
static WRITE32_HANDLER( backfire_pf12_control_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; COMBINE_DATA(&deco16_pf12_control[offset]); }
static WRITE32_HANDLER( backfire_pf34_control_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; COMBINE_DATA(&deco16_pf34_control[offset]); }
static READ32_HANDLER( backfire_pf1_data_r ) {	return deco16_pf1_data[offset]^0xffff0000; }
static READ32_HANDLER( backfire_pf2_data_r ) {	return deco16_pf2_data[offset]^0xffff0000; }
static READ32_HANDLER( backfire_pf3_data_r ) {	return deco16_pf3_data[offset]^0xffff0000; }
static READ32_HANDLER( backfire_pf4_data_r ) {	return deco16_pf4_data[offset]^0xffff0000; }
static WRITE32_HANDLER( backfire_pf1_data_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; deco16_pf1_data_w(offset,data,mem_mask); }
static WRITE32_HANDLER( backfire_pf2_data_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; deco16_pf2_data_w(offset,data,mem_mask); }
static WRITE32_HANDLER( backfire_pf3_data_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; deco16_pf3_data_w(offset,data,mem_mask); }
static WRITE32_HANDLER( backfire_pf4_data_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; deco16_pf4_data_w(offset,data,mem_mask); }

#ifdef UNUSED_FUNCTION
READ32_HANDLER( backfire_unknown_wheel_r )
{
	return readinputport(4);
}

READ32_HANDLER( backfire_wheel1_r )
{
	return mame_rand(Machine);
}

READ32_HANDLER( backfire_wheel2_r )
{
	return mame_rand(Machine);
}
#endif


static ADDRESS_MAP_START( backfire_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10001f) AM_READWRITE( backfire_pf12_control_r, backfire_pf12_control_w)
	AM_RANGE(0x110000, 0x111fff) AM_READWRITE( backfire_pf1_data_r, backfire_pf1_data_w)
	AM_RANGE(0x114000, 0x115fff) AM_READWRITE( backfire_pf2_data_r, backfire_pf2_data_w)
	AM_RANGE(0x120000, 0x120fff) AM_READWRITE( backfire_pf1_rowscroll_r, backfire_pf1_rowscroll_w)
	AM_RANGE(0x124000, 0x124fff) AM_READWRITE( backfire_pf2_rowscroll_r, backfire_pf2_rowscroll_w)
	AM_RANGE(0x130000, 0x13001f) AM_READWRITE( backfire_pf34_control_r, backfire_pf34_control_w)
	AM_RANGE(0x140000, 0x141fff) AM_READWRITE( backfire_pf3_data_r, backfire_pf3_data_w)
	AM_RANGE(0x144000, 0x145fff) AM_READWRITE( backfire_pf4_data_r, backfire_pf4_data_w)
	AM_RANGE(0x150000, 0x150fff) AM_READWRITE( backfire_pf3_rowscroll_r, backfire_pf3_rowscroll_w)
	AM_RANGE(0x154000, 0x154fff) AM_READWRITE( backfire_pf4_rowscroll_r, backfire_pf4_rowscroll_w)
	AM_RANGE(0x160000, 0x161fff) AM_WRITE(wcvol95_nonbuffered_palette_w) AM_BASE(&paletteram32)
	AM_RANGE(0x170000, 0x177fff) AM_RAM AM_BASE( &backfire_mainram )// main ram

//  AM_RANGE(0x180010, 0x180013) AM_RAM AM_BASE(&backfire_180010) // always 180010 ?
//  AM_RANGE(0x188010, 0x188013) AM_RAM AM_BASE(&backfire_188010) // always 188010 ?

	AM_RANGE(0x184000, 0x185fff) AM_RAM AM_BASE( &backfire_spriteram32_1 )
	AM_RANGE(0x18c000, 0x18dfff) AM_RAM AM_BASE( &backfire_spriteram32_2 )
	AM_RANGE(0x190000, 0x190003) AM_READ(backfire_eeprom_r)
	AM_RANGE(0x194000, 0x194003) AM_READ(backfire_control2_r)
	AM_RANGE(0x1a4000, 0x1a4003) AM_WRITE(backfire_eeprom_w)

	AM_RANGE(0x1a8000, 0x1a8003) AM_RAM AM_BASE(&backfire_left_priority)
	AM_RANGE(0x1ac000, 0x1ac003) AM_RAM AM_BASE(&backfire_right_priority)
//  AM_RANGE(0x1b0000, 0x1b0003) AM_WRITE(MWA32_NOP) // always 1b0000

	/* when set to pentometer in test mode */
//  AM_RANGE(0x1e4000, 0x1e4003) AM_READ(backfire_unknown_wheel_r)
//  AM_RANGE(0x1e8000, 0x1e8003) AM_READ(backfire_wheel1_r)
//  AM_RANGE(0x1e8004, 0x1e8007) AM_READ(backfire_wheel2_r)

	AM_RANGE(0x1c0000, 0x1c0007) AM_READ(deco156_snd_r) AM_WRITE(deco156_snd_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( backfire )
	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED ) /* 'soundmask' */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED ) /* 'soundmask' */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT ( 0x00ff, 0x0080, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(1)

	PORT_START
	PORT_BIT ( 0x00ff, 0x0080, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(1)

	PORT_START
	/* ?? */
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8	/* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 16, 0, 24, 8 },
	{ 64*8+0, 64*8+1, 64*8+2, 64*8+3, 64*8+4, 64*8+5, 64*8+6, 64*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};


static GFXDECODE_START( backfire )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,      0, 128 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( REGION_GFX1, 0, tilelayout,      0, 128 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( REGION_GFX2, 0, tilelayout,      0, 128 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( REGION_GFX3, 0, spritelayout,    0x200, 32 )	/* Sprites 16x16 (screen 1) */
	GFXDECODE_ENTRY( REGION_GFX4, 0, spritelayout,    0x600, 32 )	/* Sprites 16x16 (screen 2) */
GFXDECODE_END


static void sound_irq_gen(int state)
{
	logerror("sound irq\n");
}

static const struct YMZ280Binterface ymz280b_intf =
{
	REGION_SOUND1,
	sound_irq_gen
};

static INTERRUPT_GEN( deco32_vbl_interrupt )
{
	cpunum_set_input_line(machine, 0, ARM_IRQ_LINE, HOLD_LINE);
}



static MACHINE_DRIVER_START( backfire )

	/* basic machine hardware */
	MDRV_CPU_ADD(ARM, 28000000/4) /* Unconfirmed */
	MDRV_CPU_PROGRAM_MAP(backfire_map,0)
	MDRV_CPU_VBLANK_INT(deco32_vbl_interrupt,1)

	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MDRV_NVRAM_HANDLER(93C46)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_PALETTE_LENGTH(2048)
	MDRV_GFXDECODE(backfire)
	MDRV_DEFAULT_LAYOUT(layout_dualhsxs)

	MDRV_SCREEN_ADD("left", 0x000)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_SCREEN_ADD("right", 0x000)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_VIDEO_START(backfire)
	MDRV_VIDEO_UPDATE(backfire)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YMZ280B, 28000000 / 2)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END


/*

Backfire!
Data East, 1995

This game is similar to World Rally, Blomby Car, Drift Out'94 etc


PCB Layout
----------


DE-0432-2
---------------------------------------------------------------------
|              MBZ-06.19L     28.000MHz                MBZ-04.19A * |
|                                           52                      |
|                              153                     MBZ-03.18A + |
|              MBZ-05.17L                                           |
|                                                                   |
--|        LC7881  YMZ280B-F   153          52         MBZ-04.16A * |
  |                                                                 |
--|                                                    MBZ-03.15A + |
|                     CY7C185 (x2)                                  |
|J                                     141                          |
|                                                      MBZ-02.12A   |
|A                                                                  |
|                                                      MBZ-01.10A   |
|M       223                                                        |
|                                                      MBZ-00.9A    |
|M         93C45.8M   CY7C185 (x2)     141                          |
|                                                                   |
|A                                                                  |
|                                                                   |
--|                                                                 |
  |                                                                 |
--|        TSW1                                                     |
|                                          CY7C185 (x4)             |
|                                                           156     |
|                 ADC0808       RA01-0.3J                           |
|                               RA00-0.2J                           |
|CONN2      CONN1    D4701                                          |
|                                                                   |
---------------------------------------------------------------------


Notes:
CONN1 & CONN2: For connection of potentiometer or opto steering wheel.
               Joystick (via JAMMA) can also be used for controls.
TSW1: Push Button TEST switch to access options menu (coins/lives etc).
*   : These ROMs have identical contents AND identical halves.
+   : These ROMs have identical contents AND identical halves.

*/

ROM_START( backfire )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* DE156 code (encrypted) */
	ROM_LOAD32_WORD( "ra00-0.2j",    0x000002, 0x080000, CRC(790da069) SHA1(84fd90fb1833b97459cb337fdb92f7b6e93b5936) )
	ROM_LOAD32_WORD( "ra01-0.3j",    0x000000, 0x080000, CRC(447cb57b) SHA1(1d503b9cf1cadd3fdd7c9d6d59d4c40a59fa25ab))

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE ) /* Tiles 1 */
	ROM_LOAD( "mbz-00.9a",    0x000000, 0x080000, CRC(1098d504) SHA1(1fecd26b92faffce0b59a8a9646bfd457c17c87c) )
	ROM_CONTINUE( 0x200000, 0x080000)
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x300000, 0x080000)
	ROM_LOAD( "mbz-01.10a",    0x080000, 0x080000, CRC(19b81e5c) SHA1(4c8204a6a4ad30b23fbfdd79c6e39581e23de6ae) )
	ROM_CONTINUE( 0x280000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)
	ROM_CONTINUE( 0x380000, 0x080000)

	ROM_REGION( 0x100000, REGION_GFX2, ROMREGION_DISPOSE ) /* Tiles 2 */
	ROM_LOAD( "mbz-02.12a",    0x000000, 0x100000, CRC(2bd2b0a1) SHA1(8fcb37728f3248ad55e48f2d398b014b36c9ec05) )

	ROM_REGION( 0x400000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprites 1 */
	ROM_LOAD16_BYTE( "mbz-03.15a",    0x000001, 0x200000, CRC(2e818569) SHA1(457c1cad25d9b21459262be8b5788969f566a996) )
	ROM_LOAD16_BYTE( "mbz-04.16a",    0x000000, 0x200000, CRC(67bdafb1) SHA1(9729c18f3153e4bba703a6f46ad0b886c52d84e2) )

	ROM_REGION( 0x400000, REGION_GFX4, ROMREGION_DISPOSE ) /* Sprites 2 */
	ROM_LOAD16_BYTE( "mbz-03.18a",    0x000001, 0x200000, CRC(2e818569) SHA1(457c1cad25d9b21459262be8b5788969f566a996) )
	ROM_LOAD16_BYTE( "mbz-04.19a",    0x000000, 0x200000, CRC(67bdafb1) SHA1(9729c18f3153e4bba703a6f46ad0b886c52d84e2) )

	ROM_REGION( 0x400000, REGION_SOUND1, ROMREGION_ERASEFF ) /* samples */
	ROM_LOAD( "mbz-05.17l",    0x000000, 0x200000,  CRC(947c1da6) SHA1(ac36006e04dc5e3990f76539763cc76facd08376) )
	ROM_LOAD( "mbz-06.19l",    0x200000, 0x080000,  CRC(4a38c635) SHA1(7f0fb6a7a4aa6774c04fa38e53ceff8744fe1e9f) )

	ROM_REGION( 0x0600, REGION_PLDS, ROMREGION_DISPOSE )
	ROM_LOAD( "gal16v8b.6b",  0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8b.6d",  0x0200, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8b.12n", 0x0400, 0x0117, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( backfira )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* DE156 code (encrypted) */
	ROM_LOAD32_WORD( "rb-00h.h2",    0x000002, 0x080000, CRC(60973046) SHA1(e70d9be9cb172920da2a2ac9d317768b1438c59d) )
	ROM_LOAD32_WORD( "rb-01l.h3",    0x000000, 0x080000, CRC(27472f60) SHA1(d73b1e68dc51e28b1148db39ce22bd2e93f6fd0a) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE ) /* Tiles 1 */
	ROM_LOAD( "mbz-00.9a",    0x000000, 0x080000, CRC(1098d504) SHA1(1fecd26b92faffce0b59a8a9646bfd457c17c87c) )
	ROM_CONTINUE( 0x200000, 0x080000)
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x300000, 0x080000)
	ROM_LOAD( "mbz-01.10a",    0x080000, 0x080000, CRC(19b81e5c) SHA1(4c8204a6a4ad30b23fbfdd79c6e39581e23de6ae) )
	ROM_CONTINUE( 0x280000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)
	ROM_CONTINUE( 0x380000, 0x080000)

	ROM_REGION( 0x100000, REGION_GFX2, ROMREGION_DISPOSE ) /* Tiles 2 */
	ROM_LOAD( "mbz-02.12a",    0x000000, 0x100000, CRC(2bd2b0a1) SHA1(8fcb37728f3248ad55e48f2d398b014b36c9ec05) )

	ROM_REGION( 0x400000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprites 1 */
	ROM_LOAD16_BYTE( "mbz-03.15a",    0x000001, 0x200000, CRC(2e818569) SHA1(457c1cad25d9b21459262be8b5788969f566a996) )
	ROM_LOAD16_BYTE( "mbz-04.16a",    0x000000, 0x200000, CRC(67bdafb1) SHA1(9729c18f3153e4bba703a6f46ad0b886c52d84e2) )

	ROM_REGION( 0x400000, REGION_GFX4, ROMREGION_DISPOSE ) /* Sprites 2 */
	ROM_LOAD16_BYTE( "mbz-03.18a",    0x000001, 0x200000, CRC(2e818569) SHA1(457c1cad25d9b21459262be8b5788969f566a996) )
	ROM_LOAD16_BYTE( "mbz-04.19a",    0x000000, 0x200000, CRC(67bdafb1) SHA1(9729c18f3153e4bba703a6f46ad0b886c52d84e2) )

	ROM_REGION( 0x400000, REGION_SOUND1, ROMREGION_ERASEFF ) /* samples */
	ROM_LOAD( "mbz-05.17l",    0x000000, 0x200000,  CRC(947c1da6) SHA1(ac36006e04dc5e3990f76539763cc76facd08376) )
	ROM_LOAD( "mbz-06.19l",    0x200000, 0x080000,  CRC(4a38c635) SHA1(7f0fb6a7a4aa6774c04fa38e53ceff8744fe1e9f) )
ROM_END

static void descramble_sound( void )
{
	UINT8 *rom = memory_region(REGION_SOUND1);
	int length = 0x200000; // only the first rom is swapped on backfire!
	UINT8 *buf1 = malloc_or_die(length);
	UINT32 x;

	for (x=0;x<length;x++)
	{
		UINT32 addr;

		addr = BITSWAP24 (x,23,22,21,0, 20,
		                    19,18,17,16,
		                    15,14,13,12,
		                    11,10,9, 8,
		                    7, 6, 5, 4,
		                    3, 2, 1 );

		buf1[addr] = rom[x];
	}

	memcpy(rom,buf1,length);

	free (buf1);
}

static READ32_HANDLER( backfire_speedup_r )
{
//  mame_printf_debug( "%08x\n",activecpu_get_pc());

	if (activecpu_get_pc()==0xce44)  cpu_spinuntil_time(ATTOTIME_IN_USEC(400)); // backfire
	if (activecpu_get_pc()==0xcee4)  cpu_spinuntil_time(ATTOTIME_IN_USEC(400)); // backfira

	return backfire_mainram[0x18/4];
}


static DRIVER_INIT( backfire )
{
	deco56_decrypt(REGION_GFX1); /* 141 */
	deco56_decrypt(REGION_GFX2); /* 141 */
	decrypt156();
	cpunum_set_clockscale(machine, 0, 4.0f); /* core timings aren't accurate */
	descramble_sound();
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x0170018, 0x017001b, 0, 0, backfire_speedup_r );
}

GAME( 1995, backfire, 0,        backfire,      backfire, backfire, ROT0, "Data East Corporation", "Backfire! (set 1)", 0 )
GAME( 1995, backfira, backfire, backfire,      backfire, backfire, ROT0, "Data East Corporation", "Backfire! (set 2)", 0 ) // defaults to wheel controls, must change to joystick to play
