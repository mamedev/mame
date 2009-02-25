/*******************************************************************************

    Counter Steer                   (c) 1985 Data East Corporation
    Zero Target                     (c) 1985 Data East Corporation
    Gekitsui Oh                     (c) 1985 Data East Corporation

    Emulation by Bryan McPhail, mish@tendril.co.uk
    Improvements by Pierpaolo Prazzoli, David Haywood

    Various things aren't fully understood, for example the rotating BG layer

    todo:
    finish
        sprite fixes
        roz fixes
        colour fixes?
        input fixes?
        sound
        make cntsteer work
    cleanup
        split into driver/video
        add any missing sha info etc.
        remove unneeded debug code


*******************************************************************************/

#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "deprecat.h"
#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "sound/2203intf.h"

static tilemap *bg_tilemap, *fg_tilemap;

static UINT8 *videoram2;

static int flipscreen=0;
static int bg_bank = 0;

static PALETTE_INIT( zerotrgt )
{
	int i;
	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = /*255 - */(0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);
		/* green component */
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		bit2 = (color_prom[i] >> 6) & 0x01;
		g =/* 255 - */(0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);
		/* blue component */
		bit0 = (color_prom[i+256] >> 0) & 0x01;
		bit1 = (color_prom[i+256] >> 1) & 0x01;
		bit2 = (color_prom[i+256] >> 2) & 0x01;
		b = /*255 - */(0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}
static int colo=0;
static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram2[tile_index];

	SET_TILE_INFO(2, code + bg_bank, 0/*colo*/, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = videoram[tile_index];
	int attr = videoram[tile_index + 0x400];

	SET_TILE_INFO(0, code + ((attr & 0x0f) << 8), ((attr & 0x70) >> 4)+colo, 0);
}

static VIDEO_START( zerotrgt )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,       16,16,64,64);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows_flip_x,8, 8,32,32);

	tilemap_set_transparent_pen(fg_tilemap,0);

	tilemap_set_flip(bg_tilemap, TILEMAP_FLIPX|TILEMAP_FLIPY);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int pri)
{
	int offs;

	/* Sprites */
	for (offs = 0;offs < 0x200;offs += 4)
	{
		int multi,fx,fy,sx,sy,sy2,code,code2,color;

		code = spriteram[offs+3] + ( ( spriteram[offs+1] & 0xc0 ) << 2 );
//todo..

		code2 = code+1;
		sx = (241 - spriteram[offs+2]);
	//if (sx < -7) sx += 256;

		sy = 241 - spriteram[offs];
		color = (spriteram[offs+1] & 0x03) + ((spriteram[offs+1] & 0x08) >> 1);

//      if (pri==0 && color!=0) continue;
//      if (pri==1 && color==0) continue;

		fx = spriteram[offs+1] & 0x04;
		fy = spriteram[offs+1] & 0x02;  //check

		multi = spriteram[offs+1] & 0x10;



		if (flipscreen) {
			sy=240-sy;
			sx=240-sx;
			if (fx) fx=0; else fx=1;
			sy2=sy-16;
		}
		else sy2=sy+16;

		if (fy && multi) {
			sy2=sy;
			sy-=16;
			code++;
			code2--;
		}

    	drawgfx(bitmap,machine->gfx[1],
        		code,
				color,
				fx,fy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
        if (multi)
    		drawgfx(bitmap,machine->gfx[1],
				code2,
				color,
				fx,fy,
				sx,sy2,
				cliprect,TRANSPARENCY_PEN,0);
	}
}

static int scrolly = 0, scrollx = 0, scrolly_hi = 0, rotation_x = 0, rotation_y = 0, rotation_sign = 0;

static VIDEO_UPDATE( zerotrgt )
{
static int rot2=0,zoom=0;
static int scroll=0;

	if (input_code_pressed_once(KEYCODE_Q))
	{
		colo++;
		mame_printf_debug("colo = %X\n",colo);
		tilemap_mark_all_tiles_dirty(bg_tilemap);
		tilemap_mark_all_tiles_dirty(fg_tilemap);
	}

	if (input_code_pressed_once(KEYCODE_W))
	{
		colo--;
		mame_printf_debug("colo = %X\n",colo);
		tilemap_mark_all_tiles_dirty(bg_tilemap);
		tilemap_mark_all_tiles_dirty(fg_tilemap);
	}


	if (input_code_pressed_once(KEYCODE_E))
	{
		rot2++;
		mame_printf_debug("rot2 = %X\n",rot2);
	}

	if (input_code_pressed_once(KEYCODE_R))
	{
		rot2--;
		mame_printf_debug("rot2 = %X\n",rot2);
	}

	if (input_code_pressed_once(KEYCODE_T))
	{
		zoom+=0x100;
		mame_printf_debug("zoom= %X\n",zoom);
	}

	if (input_code_pressed_once(KEYCODE_Y))
	{
		zoom-=0x100;
		mame_printf_debug("zoom= %X\n",zoom);
	}

	if (input_code_pressed_once(KEYCODE_A))
	{
		scroll+=0x10;
		mame_printf_debug("scroll = %d\n",scroll);
	}

	if (input_code_pressed_once(KEYCODE_S))
	{
		scroll-=0x10;
		mame_printf_debug("scroll = %d\n",scroll);
	}


	if (input_code_pressed_once(KEYCODE_I))
	{
		scrollx+=0x10;
		mame_printf_debug("scrollx = %d\n",scrollx);
	}

	if (input_code_pressed_once(KEYCODE_O))
	{
		scrollx-=0x10;
		mame_printf_debug("scrollx = %d\n",scrollx);
	}

//  if (input_code_pressed(KEYCODE_A)) cpu_cause_interrupt(0,M6809_INT_IRQ);


//  spriteram=batwings_ram+0x100;
//  tilemap_set_scrolly( bg_tilemap, 0, scrolly | scrolly_hi);
//  tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	scrollx = 256;

	if(rotation_sign)
		rotation_x = -rotation_x;

	tilemap_draw_roz(bitmap, cliprect, bg_tilemap,
					scrollx << 16, (scrolly | scrolly_hi) << 16,
					1<<16, rotation_x << 10,
					rotation_y << 8, 1<<16,
					1,
					0, 0);

	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);

	draw_sprites(screen->machine,bitmap,cliprect,0);
	draw_sprites(screen->machine,bitmap,cliprect,1);

#if 0
{

	int i,j;
	char buf[60];
	struct osd_bitmap *mybitmap = bitmap;
	UINT8 *RAM = memory_region(machine, "maincpu");

buf[0] = 0;
for (i = 0;i < 8;i+=2)
	sprintf(&buf[strlen(buf)],"%04X\n",RAM[0x3000+i+1]+(RAM[0x3000+i]<<8));
ui_draw_text(buf,50,8*6);
}
#endif

	return 0;
}


static WRITE8_HANDLER( cntsteer_foreground_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset & 0x3ff);
}

static WRITE8_HANDLER( cntsteer_background_w )
{
	videoram2[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

#if 0
static WRITE8_HANDLER( gekitsui_int_w )
{
//  if (errorlog) fprintf(errorlog,"%04x: CPU 2 causes NMI\n",cpu_get_pc());
	cpu_set_input_line(space->machine->cpu[0], INPUT_LINE_NMI, ASSERT_LINE);
}
static WRITE8_HANDLER( gekitsui_int2_w ) // not used..
{
//  if (errorlog) fprintf(errorlog,"%04x: CPU 1 causes IRQ\n",cpu_get_pc());
	cpu_set_input_line(space->machine->cpu[1], M6809_IRQ_LINE, ASSERT_LINE);
}
#endif

static WRITE8_HANDLER( gekitsui_sub_irq_ack )
{
	cpu_set_input_line(space->machine->cpu[1], M6809_IRQ_LINE, CLEAR_LINE);
}

#if 0
static WRITE8_HANDLER( cntsteer_int_w )
{
	cpu_set_input_line(space->machine->cpu[0], M6809_IRQ_LINE, ASSERT_LINE);
}
#endif

static WRITE8_HANDLER( cntsteer_sound_w )
{
 	soundlatch_w(space,0,data);
}

static WRITE8_HANDLER( zerotrgt_ctrl_w )
{
	logerror("CTRL: %04x: %04x: %04x\n",cpu_get_pc(space->cpu),offset,data);
//  if (offset==0) cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_RESET, ASSERT_LINE);

	// Wrong - bits 0 & 1 used on this
	if (offset==1) cpu_set_input_line(space->machine->cpu[1], M6809_IRQ_LINE, ASSERT_LINE);
//  if (offset==2) cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_RESET, CLEAR_LINE);
}

#if 0
static WRITE8_HANDLER( cntsteer_halt_cpu0_w )
{
//  if (errorlog) fprintf(errorlog,"%04x: CPU halt %02x\n",cpu_get_pc(),data);
//  cpu_halt(0,0); /* Halt cpu */
}

static WRITE8_HANDLER( cntsteer_restart_cpu0_w )
{
//  if (errorlog) fprintf(errorlog,"%04x: CPU restart %02x\n",cpu_get_pc(),data);
//if (data&0x4) cpu_halt(0,1); /* Restart cpu */
}
#endif


/***************************************************************************/

#if 0
static ADDRESS_MAP_START( cntsteer_cpu1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE(1)
	AM_RANGE(0x1000, 0x11ff) AM_RAM AM_BASE(&spriteram)
	AM_RANGE(0x1200, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE(cntsteer_foreground_w) AM_BASE(&videoram)
	AM_RANGE(0x2800, 0x2fff) AM_RAM

//  AM_RANGE(0x1b00, 0x1b00) AM_READ_PORT("DSW0")
//  AM_RANGE(0x1b01, 0x1b01) AM_READ_PORT("DSW0")
//  AM_RANGE(0x3000, 0x3003) AM_WRITE(zerotrgt_ctrl_w)
//  AM_RANGE(0x3000, 0x3000) AM_WRITE(cntsteer_halt_cpu0_w)
//  AM_RANGE(0x3001, 0x3001) AM_WRITE(gekitsui_int2_w)

//  AM_RANGE(0x3003, 0x3003) AM_READ_PORT("DSW1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( cntsteer_cpu2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE(1)
	AM_RANGE(0x1000, 0x1fff) AM_RAM_WRITE(cntsteer_background_w) AM_BASE(&videoram2)
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("DSW0")
	AM_RANGE(0x3001, 0x3001) AM_READ_PORT("DSW1")
	AM_RANGE(0x3002, 0x3002) AM_READ_PORT("P1")
	AM_RANGE(0x3003, 0x3003) AM_READ_PORT("P2")
//  AM_RANGE(0x3002, 0x3002) AM_WRITE(gekitsui_int_w)
//  AM_RANGE(0x3000, 0x3003) AM_WRITE(zerotrgt_ctrl_w)
//  wrong 0 1 2 3 are scroll/rotate
//  AM_RANGE(0x3002, 0x3002) AM_WRITE(cntsteer_restart_cpu0_w)
//  AM_RANGE(0x3000, 0x3003) AM_WRITENOP
//  3007 and 3003 have values..
	AM_RANGE(0x3007, 0x3007) AM_WRITE(cntsteer_sound_w)
//  AM_RANGE(0x300a, 0x300a) AM_WRITE(cntsteer_int_w)
//  AM_RANGE(0x3004, 0x3004) AM_WRITE(cntsteer_int_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END
#endif

static ADDRESS_MAP_START( gekitsui_cpu1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE(1)
	AM_RANGE(0x1000, 0x11ff) AM_RAM AM_BASE(&spriteram)
	AM_RANGE(0x1200, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE(cntsteer_foreground_w) AM_BASE(&videoram)
	AM_RANGE(0x3000, 0x3003) AM_WRITE(zerotrgt_ctrl_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static int newdata[5];

static WRITE8_HANDLER(scrivi)
{
	if(offset == 0)
	{
		scrolly = data;
		return;

	}

	if(offset == 1)
	{
		//rotation related? zoom?
//      increment or decrement.. incyx?
		rotation_y = data;
	}

	if(offset == 4)
	{
		rotation_x = data;

		if(data != newdata[offset])
		{newdata[offset] = data;
			if(data >0x55)
				mame_printf_debug("off = %X data = %X\n",offset,data);
		}
		return;

	}

	if(offset == 2)
	{
		bg_bank = (data & 0x30) << 4;
	//  scrollx = data & 0x0f;
	//  mame_printf_debug("scrollx = %d %X\n",scrollx<<8,scrollx);
		if(data != newdata[offset])
		{newdata[offset] = data;
			if(data & ~0x30)
				mame_printf_debug("off = %X data = %X\n",offset,data&~0x30);
		}
		tilemap_mark_all_tiles_dirty(bg_tilemap);
		return;
	}

	if(offset == 3)
	{
		rotation_sign = !(data & 1);

		flip_screen_set(space->machine, !(data & 4));

		scrolly_hi = (data & 0xf0) << 4;

		if(data != newdata[offset])
		{newdata[offset] = data;
			if(data & ~0xf5)
				mame_printf_debug("off = %X data = %X\n",offset,data&~0xf5);
		}
		return;

	}

	if(data != newdata[offset] && offset !=1) {
		newdata[offset] = data;
		mame_printf_debug("off = %X data = %X\n",offset,data);
	}
}

static ADDRESS_MAP_START( gekitsui_cpu2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE(1)
	AM_RANGE(0x1000, 0x1fff) AM_RAM_WRITE(cntsteer_background_w) AM_BASE(&videoram2)
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("DSW0")
	AM_RANGE(0x3001, 0x3001) AM_READ_PORT("DSW1")
	AM_RANGE(0x3002, 0x3002) AM_READ_PORT("P1")
	AM_RANGE(0x3003, 0x3003) AM_READ_PORT("P2")
	AM_RANGE(0x3000, 0x3004) AM_WRITE(scrivi) /* Scroll, gfx? */
	AM_RANGE(0x3005, 0x3005) AM_WRITE(gekitsui_sub_irq_ack)
	AM_RANGE(0x3007, 0x3007) AM_WRITE(cntsteer_sound_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


/***************************************************************************/

static int nmimask;
static WRITE8_HANDLER( nmimask_w ) { nmimask = data & 0x80; }

static INTERRUPT_GEN ( sound_interrupt ) { if (!nmimask) cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE); }

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
//  AM_RANGE(0x1000, 0x1000) AM_WRITE(nmiack_w)
	AM_RANGE(0x2000, 0x2000) AM_DEVWRITE(SOUND, "ay1", ay8910_data_w)
	AM_RANGE(0x4000, 0x4000) AM_DEVWRITE(SOUND, "ay1", ay8910_address_w)
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE(SOUND, "ay2", ay8910_data_w)
	AM_RANGE(0x8000, 0x8000) AM_DEVWRITE(SOUND, "ay2", ay8910_address_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)
	AM_RANGE(0xd000, 0xd000) AM_WRITE(nmimask_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


/***************************************************************************/
static INPUT_PORTS_START( cntsteer )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "0" )
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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "1" )
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

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 )
//  PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_DIPNAME( 0x80, 0x80, "vblank" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


#ifdef UNUSED_DEFINITION
static INPUT_PORTS_START( zerotrgt )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "0" )
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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "1" )
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

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 )
INPUT_PORTS_END
#endif

/***************************************************************************/

/* this layout is bad and reads way beyond the end of the array */
static const gfx_layout cntsteer_charlayout =
{
	8,8,	/* 8*8 characters */
	RGN_FRAC(1,2),
	2,	/* 2 bits per pixel */
	{ 0, 4 },	/* the two bitplanes for 4 pixels are packed into one byte */
	{ 0, 1, 2, 3, 0x800*8+0, 0x800*8+1, 0x800*8+2, 0x800*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};

/* this layout is bad and reads way beyond the end of the array */
static const gfx_layout zerotrgt_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 4,0 },
	{ 0, 1, 2, 3, 1024*8*8+0, 1024*8*8+1, 1024*8*8+2, 1024*8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every tile takes 32 consecutive bytes */
};

static const gfx_layout sprites =
{
	16,16,
	RGN_FRAC(1,3),
	3,
 	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 16*8, 1+16*8, 2+16*8, 3+16*8, 4+16*8, 5+16*8, 6+16*8, 7+16*8,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	16*16
};

static const gfx_layout tilelayout =
{
	16,16,
	0x200,
	3,	/* 3 bits per pixel */
	{ RGN_FRAC(4,8)+4, 0, 4 },
	{ 3, 2, 1, 0, 11, 10, 9 , 8, 19, 18, 17,16, 27, 26, 25, 24 },
	{
		RGN_FRAC(0,8)+0*8, 	RGN_FRAC(1,8)+0*8, RGN_FRAC(2,8)+0*8, RGN_FRAC(3,8)+0*8,
		RGN_FRAC(0,8)+4*8, 	RGN_FRAC(1,8)+4*8, RGN_FRAC(2,8)+4*8, RGN_FRAC(3,8)+4*8,
		RGN_FRAC(0,8)+8*8, 	RGN_FRAC(1,8)+8*8, RGN_FRAC(2,8)+8*8, RGN_FRAC(3,8)+8*8,
		RGN_FRAC(0,8)+12*8, RGN_FRAC(1,8)+12*8,RGN_FRAC(2,8)+12*8,RGN_FRAC(3,8)+12*8
	},
	8*16
};

static GFXDECODE_START( cntsteer )
	GFXDECODE_ENTRY( "gfx1", 0x00000, cntsteer_charlayout, 0, 256 ) /* Only 1 used so far :/ */
	GFXDECODE_ENTRY( "gfx2", 0x00000, sprites,			  0, 256 )
	GFXDECODE_ENTRY( "gfx3", 0x00000, tilelayout,		  0, 256 )
GFXDECODE_END


static GFXDECODE_START( zerotrgt )
	GFXDECODE_ENTRY( "gfx1", 0x00000, zerotrgt_charlayout, 0, 256 ) /* Only 1 used so far :/ */
	GFXDECODE_ENTRY( "gfx2", 0x00000, sprites,			  0, 256 )
   	GFXDECODE_ENTRY( "gfx3", 0x00000, tilelayout,		  0, 256 )
GFXDECODE_END

/***************************************************************************/

static MACHINE_RESET( zerotrgt )
{
	nmimask = 0;
}


static MACHINE_DRIVER_START( cntsteer )
	MDRV_CPU_ADD("maincpu", M6809, 2000000)		 /* ? */
	MDRV_CPU_PROGRAM_MAP(gekitsui_cpu1_map,0)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse) /* ? */

	MDRV_CPU_ADD("sub", M6809, 2000000)		 /* ? */
	MDRV_CPU_PROGRAM_MAP(gekitsui_cpu2_map,0)
//  MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse) /* ? */

//  MDRV_CPU_ADD("audiocpu", M6502, 1500000)        /* ? */
//  MDRV_CPU_PROGRAM_MAP(sound_map,0)
//  MDRV_CPU_VBLANK_INT_HACK(nmi_line_pulse,16) /* ? */ // should be interrupt, 16?

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	// interleave 200?
	MDRV_GFXDECODE(cntsteer)
	MDRV_PALETTE_LENGTH(256)
//  MDRV_PALETTE_INIT(zerotrgt)

	MDRV_VIDEO_START(zerotrgt)
	MDRV_VIDEO_UPDATE(zerotrgt)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
//  MDRV_SOUND_ADD("ym", YM2203, ym2203_config)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( zerotrgt )
	MDRV_CPU_ADD("maincpu", M6809, 2000000)		 /* ? */
	MDRV_CPU_PROGRAM_MAP(gekitsui_cpu1_map,0)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse) /* ? */

	MDRV_CPU_ADD("sub", M6809, 2000000)		 /* ? */
	MDRV_CPU_PROGRAM_MAP(gekitsui_cpu2_map,0)
//  MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse) /* ? */

	MDRV_CPU_ADD("audiocpu", M6502, 1500000)		/* ? */
	MDRV_CPU_PROGRAM_MAP(sound_map,0)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_pulse,16) /* ? */ // should be interrupt, 16?
	MDRV_CPU_PERIODIC_INT(sound_interrupt, 1000)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MDRV_MACHINE_RESET(zerotrgt)

	// interleave 200?
	MDRV_GFXDECODE(zerotrgt)
	MDRV_PALETTE_LENGTH(256)

	MDRV_PALETTE_INIT(zerotrgt)
	MDRV_VIDEO_START(zerotrgt)
	MDRV_VIDEO_UPDATE(zerotrgt)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("ay2", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

/***************************************************************************/

ROM_START( cntsteer )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "by02", 0x8000, 0x4000, CRC(b6fdd7fd) SHA1(e54cc31628966f747f9ccbf9db1017ed1eee0d5d) )
	ROM_LOAD( "by01", 0xc000, 0x4000, CRC(932423a5) SHA1(0d8164359a79ae554328dfb4d729a8d07de7ee75) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "by12", 0x4000, 0x4000, CRC(278e7fed) SHA1(5def4c8919a507c64045c57de2da65e1d39e1185) )
	ROM_LOAD( "by11", 0x8000, 0x4000, CRC(00624e34) SHA1(27bd472e9f2feef4a2c4753d8b0da26ff30d930d) )
	ROM_LOAD( "by10", 0xc000, 0x4000, CRC(9227a9ce) SHA1(8c86f22f90a3a8853562469037ffa06693045f4c) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "by00", 0xc000, 0x2000, CRC(740e4896) SHA1(959652515188966e1c2810eabf2f428fe31a31a9) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_DISPOSE ) /* Characters */
	ROM_LOAD( "by09", 0x0000, 0x2000, CRC(273eddae) SHA1(4b5450407217d9110acb85e02ea9a6584552362e) )

	ROM_REGION( 0x18000, "gfx2", ROMREGION_DISPOSE ) /* Sprites */
	ROM_LOAD( "by03", 0x00000, 0x4000, CRC(d9537d33) SHA1(7d2af2eb0386ce695f2d9c7b71a72d2d8ef257e7) )
	ROM_LOAD( "by04", 0x04000, 0x4000, CRC(4f4e9d6f) SHA1(b590aeb5efa2afa50ef202191a88bcf6894f4b8e) )
	ROM_LOAD( "by05", 0x08000, 0x4000, CRC(592481a7) SHA1(2d412d525b04ed228a345918129b25a13286d957) )
	ROM_LOAD( "by06", 0x0c000, 0x4000, CRC(9366e9d5) SHA1(a6a137416eaee3becae657c287fff7d974bcf68f) )
	ROM_LOAD( "by07", 0x10000, 0x4000, CRC(8321e332) SHA1(a7aed12cb718526b0a1c5b4ae069c7973600204d) )
	ROM_LOAD( "by08", 0x14000, 0x4000, CRC(a24bcfef) SHA1(b4f06dfb85960668ca199cfb1b6c56ccdad9e33d) )

	ROM_REGION( 0x80000, "gfx3", ROMREGION_DISPOSE ) /* Tiles */
	ROM_LOAD( "by13", 0x00000, 0x4000, CRC(d38e94fd) SHA1(bcf61b2c509f923ef2e52051a1c0e0a63bedf7a3) )
	ROM_LOAD( "by15", 0x10000, 0x4000, CRC(b0c9de83) SHA1(b0041273fe968667a09c243d393b2b025c456c99) )
	ROM_LOAD( "by17", 0x20000, 0x4000, CRC(8aff285f) SHA1(d40332448e7fb20389ac18661569726f229bd9d6) )
	ROM_LOAD( "by19", 0x30000, 0x4000, CRC(7eff6d02) SHA1(967ab34bb969228689541c0a2eabd3e96665676d) )
	/* roms from "gfx4" are expanded here */

	ROM_REGION( 0x40000, "gfx4", ROMREGION_DISPOSE ) /* Tiles */
	ROM_LOAD( "by14", 0x00000, 0x2000, CRC(4db6c146) SHA1(93d157f4c4ffa2d7b4c0b33fedabd6d750245033) )
	ROM_LOAD( "by16", 0x10000, 0x2000, CRC(adede1e6) SHA1(87e0323b6d2f2d8a3585cd78c9dc9d384106b005) )
	ROM_LOAD( "by18", 0x20000, 0x2000, CRC(1e9ce047) SHA1(7579ba6b401eb1bfc7d2d9311ebab623bd1095a2) )
	ROM_LOAD( "by20", 0x30000, 0x2000, CRC(e2198c9e) SHA1(afea262db9154301f4b9e53e1fc91985dd934170) )

	ROM_REGION( 0x200, "proms", ROMREGION_ERASE00 )
ROM_END

ROM_START( zerotrgt )
	ROM_REGION( 0x10000, "maincpu", 0 )
 	ROM_LOAD( "ct01-s.4c", 0x8000, 0x8000, CRC(b35a16cb) SHA1(49581324c3e3d5219f0512d08a40161185368b10) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "ct08.16a",  0x4000, 0x4000,  CRC(7e8db408) SHA1(2ae407d15645753a2a0d691c9f1cf1eb383d3e8a) )
	ROM_LOAD( "cty07.14a", 0x8000, 0x4000,  CRC(119b6211) SHA1(2042f06387d34fad6b63bcb8ac6f9b06377f634d) )
	ROM_LOAD( "ct06.13a",  0xc000, 0x4000,  CRC(bce5adad) SHA1(86c4eef0d68679a24bab6460b49640a498f32ecd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ct00.1c",  0xe000, 0x2000,  CRC(ae091b6c) SHA1(8b3a1c0acbfa56f05bcf65677f85d70c8c9640d6) )

	ROM_REGION( 0x04000, "gfx1", ROMREGION_DISPOSE ) /* Characters */
	ROM_LOAD( "ct05.16h", 0x00000, 0x4000, CRC(e7a24404) SHA1(a8a33118d4f09b77cfd7e6e9f486b250078b21bc) )

	ROM_REGION( 0x18000, "gfx2", ROMREGION_DISPOSE ) /* Sprites */
	ROM_LOAD( "ct02.14c", 0x00000, 0x8000, CRC(d2a0bb72) SHA1(ee060f8db0b1fa1ba1034bf94cf44ff6820660bd) )
	ROM_LOAD( "ct03.15c", 0x08000, 0x8000, CRC(79f2be20) SHA1(62cf55d9163d522b7cb0e760f0d5662c529a22e9) )
	ROM_LOAD( "ct04.17c", 0x10000, 0x8000, CRC(1037cce8) SHA1(11e49e29f9b60fbf36a301a566f233eb6150d519) )

	ROM_REGION( 0x80000, "gfx3", ROMREGION_DISPOSE ) /* Tiles */
	ROM_LOAD( "ct09.4j",  0x00000, 0x4000, CRC(8c859d41) SHA1(8095e83de81d2c9f270a303322ddf84568e3d37a) )
	ROM_LOAD( "ct11.7j",  0x10000, 0x4000, CRC(5da2d9d8) SHA1(d2cfdbf892bce3667545568998aa03bfd03155c5) )
	ROM_LOAD( "ct13.10j", 0x20000, 0x4000, CRC(b004cedd) SHA1(2a503ea14c66805b37f25096ecfec19a07cdc387) )
	ROM_LOAD( "ct15.13j", 0x30000, 0x4000, CRC(4473fe66) SHA1(0accbcb801f58df410af305a87a960e526f8a25a) )
	/* roms from "gfx4" are expanded here */

	ROM_REGION( 0x40000, "gfx4", ROMREGION_DISPOSE ) /* Tiles */
	ROM_LOAD( "ct10.6j",  0x00000, 0x2000, CRC(16073975) SHA1(124128db649116d675503b03310ebbd919d5a837) )
	ROM_LOAD( "ct12.9j",  0x10000, 0x2000, CRC(9776974e) SHA1(7e944379c3ff3211c84bd4b48cebbd52c586ff88) )
	ROM_LOAD( "ct14.12j", 0x20000, 0x2000, CRC(5f77e84d) SHA1(ef7a53ad40ef5d3b7ceecb174099b8f2adfda92e) )
	ROM_LOAD( "ct16.15j", 0x30000, 0x2000, CRC(ebed04d3) SHA1(df5484ab44ddf91fddbb895606875b6733b03a51) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "mb7118h.7k",  0x0000, 0x100, CRC(4a7c187a) SHA1(2463ed582b77252a798b946cc831c4edd6e6b31f) )
	ROM_LOAD( "mb7052.6k",   0x0100, 0x100, CRC(cc9c7d43) SHA1(707fcc9579bae4233903142efa7dfee7d463ae9a) )

	ROM_REGION( 0x0300, "plds", ROMREGION_DISPOSE )
	ROM_LOAD( "pal10h8.12f", 0x0000, 0x002c, CRC(173f9798) SHA1(8b0b0314d25a70e098df5d93191669738d3e57af) )
	ROM_LOAD( "pal10h8.14e", 0x0100, 0x002c, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal12l6.17f", 0x0200, 0x0034, CRC(29b7e869) SHA1(85bdb6872d148c393c4cd98872b4920444394620) )
ROM_END

ROM_START( gekitsui )
	ROM_REGION( 0x10000, "maincpu", 0 )
 	ROM_LOAD( "ct01", 0x8000, 0x8000, CRC(d3d82d8d) SHA1(c175c626d4cb89a2d82740c04892092db6faf616) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "ct08.16a",  0x4000, 0x4000,  CRC(7e8db408) SHA1(2ae407d15645753a2a0d691c9f1cf1eb383d3e8a) )
	ROM_LOAD( "cty07.14a", 0x8000, 0x4000,  CRC(119b6211) SHA1(2042f06387d34fad6b63bcb8ac6f9b06377f634d) )
	ROM_LOAD( "ct06.13a",  0xc000, 0x4000,  CRC(bce5adad) SHA1(86c4eef0d68679a24bab6460b49640a498f32ecd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ct00.1c",  0xe000, 0x2000,  CRC(ae091b6c) SHA1(8b3a1c0acbfa56f05bcf65677f85d70c8c9640d6) )

	ROM_REGION( 0x04000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "ct05", 0x00000, 0x4000, CRC(b9e997a1) SHA1(5891cb0984bf4a1ccd80ef338c47e3d5705a1331) )	/* Characters */

	ROM_REGION( 0x18000, "gfx2", ROMREGION_DISPOSE ) /* Sprites */
	ROM_LOAD( "ct02.14c", 0x00000, 0x8000, CRC(d2a0bb72) SHA1(ee060f8db0b1fa1ba1034bf94cf44ff6820660bd) )
	ROM_LOAD( "ct03.15c", 0x08000, 0x8000, CRC(79f2be20) SHA1(62cf55d9163d522b7cb0e760f0d5662c529a22e9) )
	ROM_LOAD( "ct04.17c", 0x10000, 0x8000, CRC(1037cce8) SHA1(11e49e29f9b60fbf36a301a566f233eb6150d519) )

	ROM_REGION( 0x80000, "gfx3", ROMREGION_DISPOSE ) /* Tiles */
	ROM_LOAD( "ct09.4j",  0x00000, 0x4000, CRC(8c859d41) SHA1(8095e83de81d2c9f270a303322ddf84568e3d37a) )
	ROM_LOAD( "ct11.7j",  0x10000, 0x4000, CRC(5da2d9d8) SHA1(d2cfdbf892bce3667545568998aa03bfd03155c5) )
	ROM_LOAD( "ct13.10j", 0x20000, 0x4000, CRC(b004cedd) SHA1(2a503ea14c66805b37f25096ecfec19a07cdc387) )
	ROM_LOAD( "ct15.13j", 0x30000, 0x4000, CRC(4473fe66) SHA1(0accbcb801f58df410af305a87a960e526f8a25a) )
	/* roms from "gfx4" are expanded here */

	ROM_REGION( 0x40000, "gfx4", ROMREGION_DISPOSE ) /* Tiles */
	ROM_LOAD( "ct10.6j",  0x00000, 0x2000, CRC(16073975) SHA1(124128db649116d675503b03310ebbd919d5a837) )
	ROM_LOAD( "ct12.9j",  0x10000, 0x2000, CRC(9776974e) SHA1(7e944379c3ff3211c84bd4b48cebbd52c586ff88) )
	ROM_LOAD( "ct14.12j", 0x20000, 0x2000, CRC(5f77e84d) SHA1(ef7a53ad40ef5d3b7ceecb174099b8f2adfda92e) )
	ROM_LOAD( "ct16.15j", 0x30000, 0x2000, CRC(ebed04d3) SHA1(df5484ab44ddf91fddbb895606875b6733b03a51) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "mb7118h.7k",  0x0000, 0x100, CRC(4a7c187a) SHA1(2463ed582b77252a798b946cc831c4edd6e6b31f) )
	ROM_LOAD( "mb7052.6k",   0x0100, 0x100, CRC(cc9c7d43) SHA1(707fcc9579bae4233903142efa7dfee7d463ae9a) )

	ROM_REGION( 0x0300, "plds", ROMREGION_DISPOSE )
	ROM_LOAD( "pal10h8.12f", 0x0000, 0x002c, CRC(173f9798) SHA1(8b0b0314d25a70e098df5d93191669738d3e57af) )
	ROM_LOAD( "pal10h8.14e", 0x0100, 0x002c, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal12l6.17f", 0x0200, 0x0034, CRC(29b7e869) SHA1(85bdb6872d148c393c4cd98872b4920444394620) )
ROM_END

/***************************************************************************/

static void zerotrgt_rearrange_gfx(running_machine *machine, int romsize, int romarea)
{
	UINT8 *src = memory_region(machine, "gfx4");
	UINT8 *dst = memory_region(machine, "gfx3");
	int rm;
	int cnt1;

	dst += romarea * 4;

	for (rm = 0; rm < 4; rm++)
	{
		for (cnt1 = 0; cnt1 < romsize; cnt1++)
		{
			dst[rm*romarea+cnt1]         = (src[rm*romarea+cnt1] & 0x0f);
			dst[rm*romarea+cnt1+romsize] = (src[rm*romarea+cnt1] & 0xf0) >> 4;
		}
	}

}

#if 0
static void init_cntsteer(void)
{
	UINT8 *RAM = memory_region(machine, "sub");

	RAM[0xc2cf]=0x43; /* Patch out Cpu 1 ram test - it never ends..?! */
	RAM[0xc2d0]=0x43;
	RAM[0xc2f1]=0x43;
	RAM[0xc2f2]=0x43;

	zerotrgt_rearrange_gfx(machine, 0x02000, 0x10000);

}
#endif

static DRIVER_INIT( zerotrgt )
{
	int i;
	for (i=0; i<ARRAY_LENGTH(newdata); i++)
		newdata[i] = -1;

	zerotrgt_rearrange_gfx(machine, 0x02000, 0x10000);
}


/***************************************************************************/

GAME( 1985, cntsteer, 0,        cntsteer,  cntsteer, zerotrgt, ROT270, "Data East Corporation", "Counter Steer", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1985, zerotrgt, 0,        zerotrgt,  cntsteer, zerotrgt, ROT0,   "Data East Corporation", "Zero Target (World)", GAME_NO_SOUND|GAME_NOT_WORKING )
GAME( 1985, gekitsui, zerotrgt, zerotrgt,  cntsteer, zerotrgt, ROT0,   "Data East Corporation", "Gekitsui Oh (Japan)", GAME_NO_SOUND|GAME_NOT_WORKING )
