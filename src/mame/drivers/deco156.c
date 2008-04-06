/*
    (Some) Data East 32 bit 156 CPU ARM based games:

    Heavy Smash
    World Cup Volleyball 95

    See also deco32.c, deco_mlc.c, backfire.c

    Todo:
        complete co-processor emulation for wcvol95

    Emulation by Bryan McPhail, mish@tendril.co.uk
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

VIDEO_START(hvysmsh);
VIDEO_UPDATE(hvysmsh);

static int simpl156_bank_callback(const int bank)
{
	return ((bank>>4)&0x7) * 0x1000;
}


static VIDEO_START( wcvol95 )
{
	/* allocate the ram as 16-bit (we do it here because the CPU is 32-bit) */
	deco16_pf1_data = auto_malloc(0x2000);
	deco16_pf2_data = auto_malloc(0x2000);
	deco16_pf1_rowscroll = auto_malloc(0x800);
	deco16_pf2_rowscroll = auto_malloc(0x800);
	deco16_pf12_control = auto_malloc(0x10);
	paletteram16 =  auto_malloc(0x1000);

	/* and register the allocated ram so that save states still work */
	state_save_register_global_pointer(deco16_pf1_data, 0x2000/2);
	state_save_register_global_pointer(deco16_pf2_data, 0x2000/2);
	state_save_register_global_pointer(deco16_pf1_rowscroll, 0x800/2);
	state_save_register_global_pointer(deco16_pf2_rowscroll, 0x800/2);
	state_save_register_global_pointer(deco16_pf12_control, 0x10/2);
	state_save_register_global_pointer(paletteram16, 0x1000/2);

	deco16_1_video_init();

	deco16_set_tilemap_bank_callback(0, simpl156_bank_callback);
	deco16_set_tilemap_bank_callback(1, simpl156_bank_callback);
}

/* spriteram is really 16-bit.. this can be changed to use 16-bit ram like the tilemaps
 its the same sprite chip Data East used on many, many 16-bit era titles */
static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	int offs;

	flip_screen_set_no_update(1);

	for (offs = (0x1400/4)-4;offs >= 0;offs -= 4) // 0x1400 for charlien
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult, pri;

		sprite = spriteram32[offs+1]&0xffff;

		y = spriteram32[offs]&0xffff;
		flash=y&0x1000;
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1)) continue;

		x = spriteram32[offs+2]&0xffff;
		colour = (x >>9) & 0x1f;

		pri = (x&0xc000); // 2 bits or 1?

		switch (pri&0xc000) {
		case 0x0000: pri=0; break;
		case 0x4000: pri=0xf0; break;
		case 0x8000: pri=0xf0|0xcc; break;
		case 0xc000: pri=0xf0|0xcc; break; /*  or 0xf0|0xcc|0xaa ? */
		}

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

		if (flip_screen_x_get())
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
			pdrawgfx(bitmap,machine->gfx[2],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					cliprect,TRANSPARENCY_PEN,0,pri);

			multi--;
		}
	}
}



static VIDEO_UPDATE( wcvol95 )
{
	fillbitmap(priority_bitmap,0,NULL);
	fillbitmap(bitmap,0,NULL);

	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);

	deco16_tilemap_2_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);
	draw_sprites(screen->machine,bitmap,cliprect);
	deco16_tilemap_1_draw(bitmap,cliprect,0,0);
	return 0;
}

/***************************************************************************/

static READ32_HANDLER(hvysmsh_eeprom_r)
{
	return (EEPROM_read_bit()<<24) | input_port_read_indexed(machine, 0) | (input_port_read_indexed(machine, 1)<<16);
}

static WRITE32_HANDLER(hvysmsh_eeprom_w)
{
	if (ACCESSING_BITS_0_7) {

		OKIM6295_set_bank_base(1, 0x40000 * (data & 0x7) );

		EEPROM_set_clock_line((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
		EEPROM_write_bit(data & 0x10);
		EEPROM_set_cs_line((data & 0x40) ? CLEAR_LINE : ASSERT_LINE);
	}
}

static WRITE32_HANDLER( hvysmsh_oki_0_bank_w )
{
	OKIM6295_set_bank_base(0, (data & 1) * 0x40000);
}

static READ32_HANDLER(hvysmsh_oki_0_r)
{
	return OKIM6295_status_0_r(machine, 0);
}

static WRITE32_HANDLER(hvysmsh_oki_0_w)
{
//  data & 0xff00 is written sometimes too. game bug or needed data?
	OKIM6295_data_0_w(machine,0,data&0xff);
}

static READ32_HANDLER(hvysmsh_oki_1_r)
{
	return OKIM6295_status_1_r(machine, 0);
}

static WRITE32_HANDLER(hvysmsh_oki_1_w)
{
	OKIM6295_data_1_w(machine,0,data&0xff);
}

static READ32_HANDLER(wcvol95_eeprom_r)
{
	return (EEPROM_read_bit()<<24) | input_port_read_indexed(machine, 0) | ((input_port_read_indexed(machine, 1)&0xff)<<16);
}

static WRITE32_HANDLER(wcvol95_eeprom_w)
{
	if (ACCESSING_BITS_0_7) {
		EEPROM_set_clock_line((data & 0x2) ? ASSERT_LINE : CLEAR_LINE);
		EEPROM_write_bit(data & 0x1);
		EEPROM_set_cs_line((data & 0x4) ? CLEAR_LINE : ASSERT_LINE);
	}
}

static WRITE32_HANDLER(wcvol95_nonbuffered_palette_w)
{
	COMBINE_DATA(&paletteram32[offset]);
	palette_set_color_rgb(machine,offset,pal5bit(paletteram32[offset] >> 0),pal5bit(paletteram32[offset] >> 5),pal5bit(paletteram32[offset] >> 10));
}


static READ32_HANDLER( deco156_snd_r )
{
	return YMZ280B_status_0_r(machine, 0);
}

static WRITE32_HANDLER( deco156_snd_w )
{
	if (offset)
		YMZ280B_data_0_w(machine, 0, data);
	else
		YMZ280B_register_0_w(machine, 0, data);
}

/***************************************************************************/
static READ32_HANDLER( wcvol95_pf1_rowscroll_r ) { return deco16_pf1_rowscroll[offset]^0xffff0000; }
static READ32_HANDLER( wcvol95_pf2_rowscroll_r ) { return deco16_pf2_rowscroll[offset]^0xffff0000; }
static WRITE32_HANDLER( wcvol95_pf1_rowscroll_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; COMBINE_DATA(&deco16_pf1_rowscroll[offset]); }
static WRITE32_HANDLER( wcvol95_pf2_rowscroll_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; COMBINE_DATA(&deco16_pf2_rowscroll[offset]); }
static READ32_HANDLER ( wcvol95_pf12_control_r ) { return deco16_pf12_control[offset]^0xffff0000; }
static WRITE32_HANDLER( wcvol95_pf12_control_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; COMBINE_DATA(&deco16_pf12_control[offset]); }
static READ32_HANDLER( wcvol95_pf1_data_r ) {	return deco16_pf1_data[offset]^0xffff0000; }
static READ32_HANDLER( wcvol95_pf2_data_r ) {	return deco16_pf2_data[offset]^0xffff0000; }
static WRITE32_HANDLER( wcvol95_pf1_data_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; deco16_pf1_data_w(machine,offset,data,mem_mask); }
static WRITE32_HANDLER( wcvol95_pf2_data_w ) { data &=0x0000ffff; mem_mask &=0x0000ffff; deco16_pf2_data_w(machine,offset,data,mem_mask); }


static ADDRESS_MAP_START( hvysmsh_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x107fff) AM_RAM
	AM_RANGE(0x120000, 0x120003) AM_READ(hvysmsh_eeprom_r)
	AM_RANGE(0x120000, 0x120003) AM_WRITE(SMH_NOP) // Volume control in low byte
	AM_RANGE(0x120004, 0x120007) AM_WRITE(hvysmsh_eeprom_w)
	AM_RANGE(0x120008, 0x12000b) AM_WRITE(SMH_NOP) // IRQ ack?
	AM_RANGE(0x12000c, 0x12000f) AM_WRITE(hvysmsh_oki_0_bank_w)
	AM_RANGE(0x140000, 0x140003) AM_READ(hvysmsh_oki_0_r) AM_WRITE(hvysmsh_oki_0_w)
	AM_RANGE(0x160000, 0x160003) AM_READ(hvysmsh_oki_1_r) AM_WRITE(hvysmsh_oki_1_w)
	AM_RANGE(0x180000, 0x18001f) AM_READWRITE( wcvol95_pf12_control_r, wcvol95_pf12_control_w )
	AM_RANGE(0x190000, 0x191fff) AM_READWRITE( wcvol95_pf1_data_r, wcvol95_pf1_data_w )
	AM_RANGE(0x194000, 0x195fff) AM_READWRITE( wcvol95_pf2_data_r, wcvol95_pf2_data_w )
	AM_RANGE(0x1a0000, 0x1a0fff) AM_READWRITE( wcvol95_pf1_rowscroll_r, wcvol95_pf1_rowscroll_w )
	AM_RANGE(0x1a4000, 0x1a4fff) AM_READWRITE( wcvol95_pf2_rowscroll_r, wcvol95_pf2_rowscroll_w )
	AM_RANGE(0x1c0000, 0x1c0fff) AM_READ(SMH_RAM) AM_WRITE(deco32_nonbuffered_palette_w) AM_BASE(&paletteram32)
	AM_RANGE(0x1d0010, 0x1d002f) AM_READ(SMH_NOP) // Check for DMA complete?
	AM_RANGE(0x1e0000, 0x1e1fff) AM_RAM AM_BASE(&spriteram32) AM_SIZE(&spriteram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( wcvol95_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10001f) AM_READWRITE( wcvol95_pf12_control_r, wcvol95_pf12_control_w )
	AM_RANGE(0x110000, 0x111fff) AM_READWRITE( wcvol95_pf1_data_r, wcvol95_pf1_data_w )
	AM_RANGE(0x114000, 0x115fff) AM_READWRITE( wcvol95_pf2_data_r, wcvol95_pf2_data_w )
	AM_RANGE(0x120000, 0x120fff) AM_READWRITE( wcvol95_pf1_rowscroll_r, wcvol95_pf1_rowscroll_w )
	AM_RANGE(0x124000, 0x124fff) AM_READWRITE( wcvol95_pf2_rowscroll_r, wcvol95_pf2_rowscroll_w )
	AM_RANGE(0x130000, 0x137fff) AM_RAM
	AM_RANGE(0x140000, 0x140003) AM_READ(wcvol95_eeprom_r)
	AM_RANGE(0x150000, 0x150003) AM_WRITE(wcvol95_eeprom_w)
	AM_RANGE(0x160000, 0x161fff) AM_RAM AM_BASE(&spriteram32) AM_SIZE(&spriteram_size)
	AM_RANGE(0x170000, 0x170003) AM_NOP // Irq ack?
	AM_RANGE(0x180000, 0x180fff) AM_READ(SMH_RAM) AM_WRITE(wcvol95_nonbuffered_palette_w) AM_BASE(&paletteram32)
	AM_RANGE(0x1a0000, 0x1a0007) AM_READ(deco156_snd_r) AM_WRITE(deco156_snd_w)
ADDRESS_MAP_END


/***************************************************************************/

static INPUT_PORTS_START( hvysmsh )
	PORT_START
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

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( wcvol95 )
	PORT_START
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

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED ) /* 'soundmask' */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/**********************************************************************************/

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

static GFXDECODE_START( hvysmsh )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,          0, 32 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( REGION_GFX1, 0, tilelayout,          0, 32 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( REGION_GFX2, 0, spritelayout,      512, 32 )	/* Sprites 16x16 */
GFXDECODE_END


/**********************************************************************************/

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

static MACHINE_DRIVER_START( hvysmsh )

	/* basic machine hardware */
	MDRV_CPU_ADD(ARM, 28000000) /* Unconfirmed */
	MDRV_CPU_PROGRAM_MAP(hvysmsh_map,0)
	MDRV_CPU_VBLANK_INT("main", deco32_vbl_interrupt)

	MDRV_NVRAM_HANDLER(93C46)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM )

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(hvysmsh)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(wcvol95)
	MDRV_VIDEO_UPDATE(wcvol95)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(OKIM6295, 28000000/28)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)

	MDRV_SOUND_ADD(OKIM6295, 28000000/14)
	MDRV_SOUND_CONFIG(okim6295_interface_region_2_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.35)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.35)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( wcvol95 )

	/* basic machine hardware */
	MDRV_CPU_ADD(ARM, 28000000) /* Unconfirmed */
	MDRV_CPU_PROGRAM_MAP(wcvol95_map,0)
	MDRV_CPU_VBLANK_INT("main", deco32_vbl_interrupt)

	MDRV_NVRAM_HANDLER(93C46)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM )

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(hvysmsh)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(wcvol95)
	MDRV_VIDEO_UPDATE(wcvol95)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YMZ280B, 28000000 / 2)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END


/**********************************************************************************/

/*

Heavy Smash
Data East, 1993

PCB Layout

DE-0385-2  DEC-22VO
|----------------------------------------------|
|                      28MHz  DE52             |
|           MBG-04.13J               MBG-02.11A|
|  M6295(1) MBG-03.10K  VL-02        MBG-01.10A|
|  M6295(2)                   DE153  MBG-00.9A |
|                                              |
|                                              |
|J                                             |
|A               93C46.8K                      |
|M                                             |
|M                            DE141            |
|A                                VL-01 VL-00  |
|                  6264                        |
|      DE153       6264                        |
|                                              |
|                                              |
|TEST_SW                                 DE156 |
|           LP01-2.3J  6264   6264             |
|           LP00-2.2J  6264   6264             |
|                                              |
|----------------------------------------------|

Notes:
      - CPU is unknown. It's chip DE156. The clock input is 7.000MHz on pin 90
        It's thought to be a custom-made encrypted ARM7-based CPU.
        The package is a Quad Flat Pack and has 100 pins.
      - OKI M6295(1) clock: 1.000MHz (28 / 28), sample rate = 1000000 / 132
      - OKI M6295(2) clock: 2.000MHz (28 / 14), sample rate = 2000000 / 132
      - VSync: 58Hz
      - VL-00 (PAL16R8), VL-01 (PAL16L8), VL-02 (PAL16R6)
      - On the Data East boards of this type (using DE156) that use an EEPROM, the EEPROM contains the
        country/region code also. It has been proven by comparing the dumps of Osman and Cannon Dancer....
        they were identical and there are no region jumper pads on the PCB. Therefore the EEPROM must
        hold the region code.

      ROMs
      ----
      - MBG-00, MBG-01, MBG-02  - 16M MASK  Graphics
      - LP00, LP01              - 27C4096   Main program
      - MBG-03                  - 4M MASK   Sound (samples, linked to M6295(1)
      - MBG-04                  - 16M MASK  Sound (samples, linked to M6295(2)
      - 93C46                   - 128 bytes EEPROM (Note! this chip has identical halves and fixed
                                                    bits, but the dump is correct!)

*/

ROM_START( hvysmsh )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* DE156 code (encrypted) */
	ROM_LOAD32_WORD( "lp00-2.2j",    0x000002, 0x080000, CRC(3f8fd724) SHA1(8efb27b96dbdc58715eb44c7846f30d485e1ded4) )
	ROM_LOAD32_WORD( "lp01-2.3j",    0x000000, 0x080000, CRC(a6fe282a) SHA1(10295b740ced35b3bb1f48ca3af2e985912405ec) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "mbg-00.9a",    0x000000, 0x080000, CRC(7d94eb16) SHA1(31cf5302eba37e935865822aebd76c700bc51eaf) )
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x080000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)

	ROM_REGION( 0x800000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD16_BYTE( "mbg-01.10a",    0x000000, 0x200000, CRC(bcd7fb29) SHA1(a54a813b5adcb4df0bfdd58285b1f8e17fbbb7a2) )
	ROM_LOAD16_BYTE( "mbg-02.11a",    0x000001, 0x200000, CRC(0cc16440) SHA1(1cbf620a9d875ec87dd28a97a256584b6ef277cd) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* Oki samples */
	ROM_LOAD( "mbg-03.10k",    0x00000, 0x80000,  CRC(4b809420) SHA1(ad0278745002320804a31af0b772f9ab5f075027) )

	ROM_REGION( 0x200000, REGION_SOUND2, 0 ) /* Oki samples */
	ROM_LOAD( "mbg-04.11k",    0x00000, 0x200000, CRC(2281c758) SHA1(934691b4002ecd6bc9a09b8970ff18a09451d492) )

//  ROM_REGION( 0x80, REGION_USER1, 0 ) /* eeprom */
//  ROM_LOAD( "93c46.8k",    0x00, 0x80, CRC(d31fbd5b) SHA1(bf044408c637f6b39afd30ccb86af183ec0acc02) )
ROM_END

ROM_START( hvysmsha )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* DE156 code (encrypted) */
	ROM_LOAD32_WORD( "2j.bin",    0x000002, 0x080000, CRC(333a92c1) SHA1(b7e174ea081febb765298aa1c6533b2f9f162bce) )
	ROM_LOAD32_WORD( "3j.bin",    0x000000, 0x080000, CRC(8c24c5ed) SHA1(ab9689530f4f4a6015ce0a6f8e0d796b0618cd79) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "mbg-00.9a",    0x000000, 0x080000, CRC(7d94eb16) SHA1(31cf5302eba37e935865822aebd76c700bc51eaf) )
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x080000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)

	ROM_REGION( 0x800000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD16_BYTE( "mbg-01.10a",    0x000000, 0x200000, CRC(bcd7fb29) SHA1(a54a813b5adcb4df0bfdd58285b1f8e17fbbb7a2) )
	ROM_LOAD16_BYTE( "mbg-02.11a",    0x000001, 0x200000, CRC(0cc16440) SHA1(1cbf620a9d875ec87dd28a97a256584b6ef277cd) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 ) /* Oki samples */
	ROM_LOAD( "mbg-03.10k",    0x00000, 0x80000,  CRC(4b809420) SHA1(ad0278745002320804a31af0b772f9ab5f075027) )

	ROM_REGION( 0x200000, REGION_SOUND2, 0 ) /* Oki samples */
	ROM_LOAD( "mbg-04.11k",    0x00000, 0x200000, CRC(2281c758) SHA1(934691b4002ecd6bc9a09b8970ff18a09451d492) )

//  ROM_REGION( 0x80, REGION_USER1, 0 ) /* eeprom */
//  ROM_LOAD( "93c46.8k",    0x00, 0x80, CRC(d31fbd5b) SHA1(bf044408c637f6b39afd30ccb86af183ec0acc02) )
ROM_END

/*
World Cup Volley '95
Data East, 1995

PCB Layout

DE-0430-2
|----------------------------------------------|
|          MBX-03.13J                MBX-02.13A|
|       LC7881         28MHz   DE52            |
|             YMZ280B-F              MBX-01.12A|
|                         CY7C185              |
|                         CY7C185    MBX-00.9A |
|             WE-02                            |
|J                                             |
|A                                             |
|M                 6264                        |
|M                             DE141           |
|A     DE223       6264                        |
|                                              |
|                                       WE-00  |
|                              WE-01           |
|                                              |
|TEST_SW           PN01-0.4F   6264            |
|         93C46.3K             6264            |
|                  PN00-0.2F   6264     DE156  |
|                              6264            |
|----------------------------------------------|

Notes:
      - CPU is unknown. It's chip DE156. The clock input is 7.000MHz on pin 90
        It's thought to be a custom-made encrypted ARM7-based CPU.
        The package is a Quad Flat Pack and has 100 pins.
      - YMZ280B-F clock: 14.000MHz (28 / 2)
        SANYO LC7881 clock: 2.3333MHz (28 / 12)
      - VSync: 58Hz
      - WE-00, WE-01 and WE-02 are PALs type GAL16V8

*/

ROM_START( wcvol95 )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* DE156 code (encrypted) */
	ROM_LOAD32_WORD( "pn00-0.2f",    0x000002, 0x080000, CRC(c9ed2006) SHA1(cee93eafc42c4de7a1453c85e7d6bca8d62cdc7b) )
	ROM_LOAD32_WORD( "pn01-0.4f",    0x000000, 0x080000, CRC(1c3641c3) SHA1(60dddc3585e4dedb485f7505fee03495f615c0c0) )

	ROM_REGION( 0x080000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "mbx-00.9a",    0x000000, 0x080000, CRC(a0b24204) SHA1(cec8089c6c635f23b3a4aeeef2c43f519568ad70) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD16_BYTE( "mbx-01.12a",    0x000000, 0x100000, CRC(73deb3f1) SHA1(c0cabecfd88695afe0f27c5bb115b4973907207d) )
	ROM_LOAD16_BYTE( "mbx-02.13a",    0x000001, 0x100000, CRC(3204d324) SHA1(44102f71bae44bf3a9bd2de7e5791d959a2c9bdd) )

	ROM_REGION( 0x200000, REGION_SOUND1, 0 ) /* YMZ280B-F samples */
	ROM_LOAD( "mbx-03.13j",    0x00000, 0x200000,  CRC(061632bc) SHA1(7900ac56e59f4a4e5768ce72f4a4b7c5875f5ae8) )

//  ROM_REGION( 0x80, REGION_USER1, 0 ) /* eeprom */
//  ROM_LOAD( "93c46.3k",    0x00, 0x80, CRC(88f8e270) SHA1(cb82203ad38e0c12ea998562b7b785979726afe5) )
ROM_END

/**********************************************************************************/

static void descramble_sound( int region )
{
	UINT8 *rom = memory_region(region);
	int length = memory_region_length(region);
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

static DRIVER_INIT( hvysmsh )
{
	deco56_decrypt(REGION_GFX1); /* 141 */
	decrypt156();
	descramble_sound(REGION_SOUND2);
}

static DRIVER_INIT( wcvol95 )
{
	deco56_decrypt(REGION_GFX1); /* 141 */
	decrypt156();
	descramble_sound(REGION_SOUND1);
}


/**********************************************************************************/

GAME( 1993, hvysmsh,  0,        hvysmsh,       hvysmsh,  hvysmsh,  ROT0, "Data East Corporation", "Heavy Smash (Japan version -2)", 0)
GAME( 1993, hvysmsha,  hvysmsh,        hvysmsh,       hvysmsh,  hvysmsh,  ROT0, "Data East Corporation", "Heavy Smash (Asia version -4)", 0)
GAME( 1995, wcvol95,  0,        wcvol95,       wcvol95,  wcvol95,  ROT0, "Data East Corporation", "World Cup Volley '95 (Japan v1.0)",0 )
