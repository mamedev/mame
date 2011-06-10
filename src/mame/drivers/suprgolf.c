/************************************************************************************

 Super Crowns Golf (c) 1989 Nasco Japan

 driver by Tomasz Slanina & Angelo Salese

 TODO:
 - remove the patch and understand what needs to be modified for the gfxs, game
   doesn't crash anymore (note: I suspect it's actually a ppi port C bug);
 - Some weird framebuffer vertical gaps with some object, namely the green and the
   trees (zooming?)
 - not sure if the analog inputs are handled correctly;
 - Fix the framebuffer display in cocktail mode;
 - Albatross: bad graphics, caused by missing rom(s).

 Notes:
 - The game uses special control panel with 1 golf club shaped device to select shot
   strength (0,1,2,3), and 6 buttons (direction L&R, select angle of club head, club
   select, shot, and power to use items). -YO

************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "machine/i8255.h"

class suprgolf_state : public driver_device
{
public:
	suprgolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	tilemap_t *m_tilemap;
	UINT8 *m_videoram;
	UINT8 *m_paletteram;
	UINT8 *m_bg_vram;
	UINT16 *m_bg_fb;
	UINT16 *m_fg_fb;
	UINT8 m_rom_bank;
	UINT8 m_bg_bank;
	UINT8 m_vreg_bank;
	UINT8 m_msm5205next;
	UINT8 m_msm_nmi_mask;
	UINT8 m_vreg_pen;
	UINT8 m_palette_switch;
	UINT8 m_bg_vreg_test;
	UINT8 m_toggle;
};

static TILE_GET_INFO( get_tile_info )
{
	suprgolf_state *state = machine.driver_data<suprgolf_state>();
	int code = state->m_videoram[tile_index*2]+256*(state->m_videoram[tile_index*2+1]);
	int color = state->m_videoram[tile_index*2+0x800] & 0x7f;

	SET_TILE_INFO(
		0,
		code,
		color,
		0);
}

static VIDEO_START( suprgolf )
{
	suprgolf_state *state = machine.driver_data<suprgolf_state>();

	state->m_tilemap = tilemap_create( machine, get_tile_info,tilemap_scan_rows,8,8,32,32 );
	state->m_paletteram = auto_alloc_array(machine, UINT8, 0x1000);
	state->m_bg_vram = auto_alloc_array(machine, UINT8, 0x2000*0x20);
	state->m_bg_fb = auto_alloc_array(machine, UINT16, 0x2000*0x20);
	state->m_fg_fb = auto_alloc_array(machine, UINT16, 0x2000*0x20);

	tilemap_set_transparent_pen(state->m_tilemap,15);
}

static SCREEN_UPDATE( suprgolf )
{
	suprgolf_state *state = screen->machine().driver_data<suprgolf_state>();
	int x,y,count,color;
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));

	{
		count = 0;

		for(y=0;y<256;y++)
		{
			for(x=0;x<512;x++)
			{
				color = state->m_bg_fb[count];

				if(x <= cliprect->max_x && y <= cliprect->max_y)
					*BITMAP_ADDR16(bitmap, y, x) = screen->machine().pens[(color & 0x7ff)];

				count++;
			}
		}
	}

	{
		count = 0;

		for(y=0;y<256;y++)
		{
			for(x=0;x<512;x++)
			{
				color = state->m_fg_fb[count];

				if(((state->m_fg_fb[count] & 0x0f) != 0x0f) && (x <= cliprect->max_x && y <= cliprect->max_y))
					*BITMAP_ADDR16(bitmap, y, x) = screen->machine().pens[(color & 0x7ff)];

				count++;
			}
		}
	}

	tilemap_draw(bitmap,cliprect,state->m_tilemap,0,0);

	return 0;
}

static READ8_HANDLER( suprgolf_videoram_r )
{
	suprgolf_state *state = space->machine().driver_data<suprgolf_state>();

	if (state->m_palette_switch)
		return state->m_paletteram[offset];
	else
		return state->m_videoram[offset];
}

static WRITE8_HANDLER( suprgolf_videoram_w )
{
	suprgolf_state *state = space->machine().driver_data<suprgolf_state>();

	if(state->m_palette_switch)
	{
		int r,g,b,datax;
		state->m_paletteram[offset] = data;
		offset>>=1;
		datax = state->m_paletteram[offset*2] + 256*state->m_paletteram[offset*2 + 1];

		b = (datax & 0x8000) ? 0 : ((datax)&0x001f)>>0;
		g = (datax & 0x8000) ? 0 : ((datax)&0x03e0)>>5;
		r = (datax & 0x8000) ? 0 : ((datax)&0x7c00)>>10;

		palette_set_color_rgb(space->machine(), offset, pal5bit(r), pal5bit(g), pal5bit(b));
	}
	else
	{
		state->m_videoram[offset] = data;
		tilemap_mark_tile_dirty(state->m_tilemap, (offset & 0x7fe) >> 1);
	}
}

static READ8_DEVICE_HANDLER( suprgolf_vregs_r )
{
	suprgolf_state *state = device->machine().driver_data<suprgolf_state>();

	return state->m_vreg_bank;
}

static WRITE8_DEVICE_HANDLER( suprgolf_vregs_w )
{
	suprgolf_state *state = device->machine().driver_data<suprgolf_state>();

	//printf("%02x\n",data);

	//bits 0,1,2 and probably 3 controls the background vram banking
	state->m_vreg_bank = data;
	state->m_palette_switch = (data & 0x80);
	state->m_bg_bank = (data & 0x1f);

	state->m_bg_vreg_test = data & 0x20;

	//if(data & 0x60)
	//  printf("Video regs with data %02x activated\n",data);
}

static READ8_HANDLER( suprgolf_bg_vram_r )
{
	suprgolf_state *state = space->machine().driver_data<suprgolf_state>();

	return state->m_bg_vram[offset+state->m_bg_bank*0x2000];
}

static WRITE8_HANDLER( suprgolf_bg_vram_w )
{
	suprgolf_state *state = space->machine().driver_data<suprgolf_state>();
	UINT8 hi_nibble,lo_nibble;
	UINT8 hi_dirty_dot,lo_dirty_dot; // helpers

	hi_nibble = data & 0xf0;
	lo_nibble = data & 0x0f;
	hi_dirty_dot = 1;
	lo_dirty_dot = 1;

	if(hi_nibble == 0xf0)
	{
		hi_nibble = state->m_bg_vram[offset+state->m_bg_bank*0x2000] & 0xf0;
		if(!(state->m_vreg_pen & 0x80) && (!(state->m_bg_bank & 0x10)))
			hi_dirty_dot = 0;
	}

	if(lo_nibble == 0x0f)
	{
		lo_nibble = state->m_bg_vram[offset+state->m_bg_bank*0x2000] & 0x0f;
		if(!(state->m_vreg_pen & 0x80) && (!(state->m_bg_bank & 0x10)))
			lo_dirty_dot = 0;
	}

	if(state->m_vreg_pen & 0x80 || state->m_bg_bank & 0x10)
		state->m_bg_vram[offset+state->m_bg_bank*0x2000] = data;
	else
		state->m_bg_vram[offset+state->m_bg_bank*0x2000] = hi_nibble|lo_nibble;

	if(state->m_bg_bank & 0x10)
	{
		if(hi_dirty_dot)
			state->m_fg_fb[(offset+(state->m_bg_bank & 0x0f)*0x2000)*2+1] = (state->m_vreg_pen & 0x7f)<<4 | ((state->m_bg_vram[offset+state->m_bg_bank*0x2000] & 0xf0)>>4);
		if(lo_dirty_dot)
			state->m_fg_fb[(offset+(state->m_bg_bank & 0x0f)*0x2000)*2+0] = (state->m_vreg_pen & 0x7f)<<4 | ((state->m_bg_vram[offset+state->m_bg_bank*0x2000] & 0x0f)>>0);
	}
	else
	{
		if(hi_dirty_dot)
			state->m_bg_fb[(offset+(state->m_bg_bank & 0x0f)*0x2000)*2+1] = (state->m_vreg_pen & 0x7f)<<4 | ((state->m_bg_vram[offset+state->m_bg_bank*0x2000] & 0xf0)>>4);
		if(lo_dirty_dot)
			state->m_bg_fb[(offset+(state->m_bg_bank & 0x0f)*0x2000)*2+0] = (state->m_vreg_pen & 0x7f)<<4 | ((state->m_bg_vram[offset+state->m_bg_bank*0x2000] & 0x0f)>>0);
	}
}

static WRITE8_HANDLER( suprgolf_pen_w )
{
	suprgolf_state *state = space->machine().driver_data<suprgolf_state>();

	state->m_vreg_pen = data;
}

static WRITE8_HANDLER( adpcm_data_w )
{
	suprgolf_state *state = space->machine().driver_data<suprgolf_state>();

	state->m_msm5205next = data;
}

static READ8_DEVICE_HANDLER( rom_bank_select_r )
{
	suprgolf_state *state = device->machine().driver_data<suprgolf_state>();

	return state->m_rom_bank;
}

static WRITE8_DEVICE_HANDLER( rom_bank_select_w )
{
	suprgolf_state *state = device->machine().driver_data<suprgolf_state>();
	UINT8 *region_base = device->machine().region("user1")->base();

	state->m_rom_bank = data;

	//popmessage("%08x %02x",((data & 0x3f) * 0x4000),data);

//  mame_printf_debug("ROM_BANK 0x8000 - %X @%X\n",data,cpu_get_previouspc(&space->device()));
	memory_set_bankptr(device->machine(), "bank2", region_base + (data&0x3f ) * 0x4000);

	state->m_msm_nmi_mask = data & 0x40;
	flip_screen_set(device->machine(), data & 0x80);
}

static WRITE8_HANDLER( rom2_bank_select_w )
{
	UINT8 *region_base = space->machine().region("user2")->base();
//  mame_printf_debug("ROM_BANK 0x4000 - %X @%X\n",data,cpu_get_previouspc(&space->device()));

	memory_set_bankptr(space->machine(), "bank1", region_base + (data&0x0f) * 0x4000);

	if(data & 0xf0)
		printf("Rom bank select 2 with data %02x activated\n",data);
}

static READ8_DEVICE_HANDLER( pedal_extra_bits_r )
{
	UINT8 p1_sht_sw,p2_sht_sw;

	p1_sht_sw = (input_port_read(device->machine(), "P1_RELEASE") & 0x80)>>7;
	p2_sht_sw = (input_port_read(device->machine(), "P2_RELEASE") & 0x80)>>6;

	return p1_sht_sw | p2_sht_sw;
}

static READ8_DEVICE_HANDLER( p1_r )
{
	return (input_port_read(device->machine(), "P1") & 0xf0) | ((input_port_read(device->machine(), "P1_ANALOG") & 0xf));
}

static READ8_DEVICE_HANDLER( p2_r )
{
	return (input_port_read(device->machine(), "P2") & 0xf0) | ((input_port_read(device->machine(), "P2_ANALOG") & 0xf));
}

static ADDRESS_MAP_START( suprgolf_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x4000) AM_WRITE( rom2_bank_select_w )
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank2")
	AM_RANGE(0xc000, 0xdfff) AM_READWRITE( suprgolf_bg_vram_r, suprgolf_bg_vram_w ) // banked background vram
	AM_RANGE(0xe000, 0xefff) AM_READWRITE( suprgolf_videoram_r, suprgolf_videoram_w ) AM_BASE_MEMBER(suprgolf_state,m_videoram) //foreground vram + paletteram
	AM_RANGE(0xf000, 0xf000) AM_WRITE( suprgolf_pen_w )
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE_MODERN("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE_MODERN("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x08, 0x09) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(adpcm_data_w)
 ADDRESS_MAP_END

static INPUT_PORTS_START( suprgolf )
	PORT_START("P1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_SPECIAL ) /* low port of P1 Pedal */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)	    /* D.L */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)	    /* D.R */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)			/* CNT - shot switch */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)			/* SEL */

	PORT_START("P1_ANALOG")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(5) PORT_KEYDELTA(5) PORT_PLAYER(1)

	/* simulate spring throttle with the following button */
	PORT_START("P1_RELEASE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)	/* release power? */

	PORT_START("P2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_SPECIAL ) /* low port of P2 Pedal */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)	    /* D.L */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)	    /* D.R */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)			/* CNT - shot switch */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)			/* SEL */

	PORT_START("P2_ANALOG")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(5) PORT_KEYDELTA(5) PORT_PLAYER(2)

	/* simulate spring throttle with the following button */
	PORT_START("P2_RELEASE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)	/* release power? */

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )             			/* 1P */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)			/* POW */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) 	                	/* 1P */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)			/* POW */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "TST" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	/* According to the manual, 4 and 5 are for Indoor Practice tries, but doesn't suit well...different version? */
	PORT_DIPNAME( 0x08, 0x08, "Indoor Practice" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Number of Balls" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	/* According to the manual, Allow Continue should be dip-sw 2:5 */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPNAME( 0x06, 0x00, "Percentage of wind over 10m/s" ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, "30%" )
	PORT_DIPSETTING(    0x04, "40%" )
	PORT_DIPSETTING(    0x02, "50%" )
	PORT_DIPSETTING(    0x06, "60%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:8" )
INPUT_PORTS_END

static WRITE8_DEVICE_HANDLER( suprgolf_writeA )
{
	mame_printf_debug("ymwA\n");
}

static WRITE8_DEVICE_HANDLER( suprgolf_writeB )
{
	mame_printf_debug("ymwA\n");
}

static void irqhandler(device_t *device, int irq)
{
	//cputag_set_input_line(device->machine(), "maincpu", INPUT_LINE_NMI, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_INPUT_PORT("DSW0"),
		DEVCB_INPUT_PORT("DSW1"),
		DEVCB_HANDLER(suprgolf_writeA),
		DEVCB_HANDLER(suprgolf_writeB),
	},
	irqhandler
};

static void adpcm_int(device_t *device)
{
	suprgolf_state *state = device->machine().driver_data<suprgolf_state>();

	{
		msm5205_reset_w(device,0);
		state->m_toggle ^= 1;
		if(state->m_toggle)
		{
			msm5205_data_w(device, (state->m_msm5205next & 0xf0) >> 4);
			if(state->m_msm_nmi_mask) { cputag_set_input_line(device->machine(), "maincpu", INPUT_LINE_NMI, PULSE_LINE); }
		}
		else
		{
			msm5205_data_w(device, (state->m_msm5205next & 0x0f) >> 0);
		}
	}
}

static const msm5205_interface msm5205_config =
{
	adpcm_int,		/* interrupt function */
	MSM5205_S48_4B	/* 4KHz 4-bit */
};

static const gfx_layout gfxlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};

static GFXDECODE_START( suprgolf )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout,   0, 0x80 )
GFXDECODE_END

static MACHINE_RESET( suprgolf )
{
	suprgolf_state *state = machine.driver_data<suprgolf_state>();

	state->m_msm_nmi_mask = 0;
}

static I8255A_INTERFACE( ppi8255_intf_0 )
{
	DEVCB_HANDLER(p1_r),						/* Port A read */
	DEVCB_NULL,									/* Port A write */
	DEVCB_HANDLER(p2_r),						/* Port B read */
	DEVCB_NULL,									/* Port B write */
	DEVCB_HANDLER(pedal_extra_bits_r),			/* Port C read */
	DEVCB_NULL									/* Port C write */
};

static I8255A_INTERFACE( ppi8255_intf_1 )
{
	DEVCB_INPUT_PORT("SYSTEM"),					/* Port A read */
	DEVCB_NULL,									/* Port A write */
	DEVCB_HANDLER(rom_bank_select_r),			/* Port B read */
	DEVCB_HANDLER(rom_bank_select_w),			/* Port B write */
	DEVCB_HANDLER(suprgolf_vregs_r),			/* Port C read */
	DEVCB_HANDLER(suprgolf_vregs_w)				/* Port C write */
};


#define MASTER_CLOCK XTAL_12MHz

static MACHINE_CONFIG_START( suprgolf, suprgolf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,MASTER_CLOCK/2) /* guess */
	MCFG_CPU_PROGRAM_MAP(suprgolf_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_VIDEO_START(suprgolf)

	MCFG_MACHINE_RESET(suprgolf)

	MCFG_I8255A_ADD( "ppi8255_0", ppi8255_intf_0 )
	MCFG_I8255A_ADD( "ppi8255_1", ppi8255_intf_1 )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 0, 191)
	MCFG_SCREEN_UPDATE(suprgolf)

	MCFG_GFXDECODE(suprgolf)
	MCFG_PALETTE_LENGTH(0x800)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, MASTER_CLOCK/4) /* guess */
	MCFG_SOUND_CONFIG(ym2203_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_SOUND_ADD("msm", MSM5205, XTAL_384kHz) /* guess */
	MCFG_SOUND_CONFIG(msm5205_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/*
----------------------
CG24     6K        CONN BD
CG1      6J         "
CG2      6G         "
CG3      6F         "
CG4      6D         "
CG5      6C         "
CG6      6A         "
CG7      5J         "
CG8      5G         "
CG9      5F         "
CG10     5D         "
CG11     5A         "
CG12     6K         "
CG13     6J         "
CG14     5K         "
CG15     5J         "
CG16     5G         "
CG17     5F         "

CG18     3K        DAUGHTER BOARD
CG20     7K         "
CG21     7J         "
CG22     7G         "
CG23     7F         "
*/

ROM_START( suprgolf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cg24.6k",0x000000, 0x08000, CRC(de548044) SHA1(f96b4cfcfca4dffabfaf205eb903cbc70972626b) )

	ROM_REGION( 0x100000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "cg1.6j", 0x000000, 0x10000, CRC(ee545c71) SHA1(8ee459a85e52257d3f9a2aa7263b641aad87bafd) )
	ROM_LOAD( "cg2.6g", 0x010000, 0x10000, CRC(a2ed2159) SHA1(5e13b6c4eaba8146a4c6c2ff24197f3ffca29b92) )
	ROM_LOAD( "cg3.6f", 0x020000, 0x10000, CRC(4543334d) SHA1(7ee268ed6d02c78db8c222418313593df37cde4b) )
	ROM_LOAD( "cg4.6d", 0x030000, 0x10000, CRC(85ace664) SHA1(5267406c98e2d124a4985816f8e2e32e74e09614) )
	ROM_LOAD( "cg5.6c", 0x040000, 0x10000, CRC(609d5b37) SHA1(60640a9bd0883bf4dc999077d89ef793e827ac23) )
	ROM_LOAD( "cg6.6a", 0x050000, 0x10000, CRC(5e4a8ddb) SHA1(0c71c7eba9fe79187c4214eb639a481305070dcc) )
	ROM_LOAD( "cg7.5j", 0x060000, 0x10000, CRC(90ac6734) SHA1(2656397fca6dceabf8e35c093c0ba25e08d2ad1e) )
	ROM_LOAD( "cg8.5g", 0x070000, 0x10000, CRC(2e9edece) SHA1(a0961bb23f312ed137134746d2d3d438fe098085) )
	ROM_LOAD( "cg9.5f", 0x080000, 0x10000, CRC(139d71f1) SHA1(756ed068e1e2b76a9d1df95b432976e632edfa77) )
	ROM_LOAD( "cg10.5d",0x090000, 0x10000, CRC(c069e75e) SHA1(77f1b7571e677aef601b8b1c481b352ca6e485d6) )
	/* no 5c? */
	ROM_LOAD( "cg11.5a",0x0b0000, 0x10000, CRC(cfec1a0f) SHA1(c09ece059cb3c456b66c016c6fab3139d3f61c6a) )

	ROM_REGION( 0x100000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD( "cg20.7k",0x000000, 0x10000, CRC(1e3fa2fd) SHA1(4771b90e40ebfbae4a98ff7ce6db50f635232597) )
	ROM_LOAD( "cg21.7j",0x010000, 0x10000, CRC(0323a2cd) SHA1(d7d4b35ad451acb2fa3d117bb0ae2f8fbd883f17) )
	ROM_LOAD( "cg22.7g",0x020000, 0x10000, CRC(83bcbefd) SHA1(77f29cfd1583d2506e95b8513cb9f87569c31821) )
	ROM_LOAD( "cg23.7f",0x030000, 0x10000, CRC(50191b4d) SHA1(8f74cba2a2b5fd2a03eaf13a6d6b39af8833a4ab) )

	ROM_REGION( 0x70000, "gfx1", 0 )
	ROM_LOAD( "cg12.6k",0x00000, 0x10000, CRC(5707b3d5) SHA1(9102a40fefb6426f2cd9d92d66fdc77e078e3f4c) )
	ROM_LOAD( "cg13.6j",0x10000, 0x10000, CRC(02ff0187) SHA1(aeeb3b2d15c3c8ff4695ecf6cfc0c385295ecce6) )
	ROM_LOAD( "cg14.5k",0x20000, 0x10000, CRC(ca12e01d) SHA1(9c627fb527c8966e16dc6bdb99ec0b9728b5c5f9) )
	ROM_LOAD( "cg15.5j",0x30000, 0x10000, CRC(0fb88270) SHA1(d85a7f1bc5b3c4b13bbd887cea4c055541cbb737) )
	ROM_LOAD( "cg16.5g",0x40000, 0x10000, CRC(0498aa2e) SHA1(988965c3a584dac17ad8c7e504fa1f1e49775611) )
	ROM_LOAD( "cg17.5f",0x50000, 0x10000, CRC(d27f87b5) SHA1(5b2927e89615589540e3853593aeff517584b6a0) )
	ROM_LOAD( "cg18.3k",0x60000, 0x10000, CRC(36edd88e) SHA1(374c95721198a88831d6f7e0b71d05e2f8465271) )
ROM_END

ROM_START( albatross )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.6k",         0x000000, 0x008000, CRC(6f934951) SHA1(b7217a4e509e452f15f414ce7e23c724ecac6184) )

	ROM_REGION( 0x100000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "at1.6h",       0x000000, 0x010000, CRC(ee545c71) SHA1(8ee459a85e52257d3f9a2aa7263b641aad87bafd) )
	ROM_LOAD( "at2.6g",       0x010000, 0x010000, CRC(a2ed2159) SHA1(5e13b6c4eaba8146a4c6c2ff24197f3ffca29b92) )
	ROM_LOAD( "at3.6e",       0x020000, 0x010000, CRC(4543334d) SHA1(7ee268ed6d02c78db8c222418313593df37cde4b) )
	ROM_LOAD( "at4.6d",       0x030000, 0x010000, CRC(85ace664) SHA1(5267406c98e2d124a4985816f8e2e32e74e09614) )
	ROM_LOAD( "at5.6c",       0x040000, 0x010000, CRC(609d5b37) SHA1(60640a9bd0883bf4dc999077d89ef793e827ac23) )
	ROM_LOAD( "at6.6a",       0x050000, 0x010000, CRC(5e4a8ddb) SHA1(0c71c7eba9fe79187c4214eb639a481305070dcc) )
	ROM_LOAD( "at7.4h",       0x060000, 0x010000, CRC(90ac6734) SHA1(2656397fca6dceabf8e35c093c0ba25e08d2ad1e) )
	ROM_LOAD( "at8.4g",       0x070000, 0x010000, CRC(2e9edece) SHA1(a0961bb23f312ed137134746d2d3d438fe098085) )
	ROM_LOAD( "kage.4e",      0x080000, 0x010000, CRC(139d71f1) SHA1(756ed068e1e2b76a9d1df95b432976e632edfa77) )
	ROM_LOAD( "at10.4d",      0x090000, 0x010000, CRC(c4d5617c) SHA1(5f2d66f827d8d7437fde84ffa17db105a5352f06) )
	/* 4c is connected below */
	ROM_LOAD( "map.4a",       0x0b0000, 0x010000, CRC(cfec1a0f) SHA1(c09ece059cb3c456b66c016c6fab3139d3f61c6a) )

	ROM_REGION( 0x100000, "user2", ROMREGION_ERASEFF )
	ROM_LOAD( "cg20.7k",0x000000, 0x10000, BAD_DUMP CRC(1e3fa2fd) SHA1(4771b90e40ebfbae4a98ff7ce6db50f635232597) ) // - empty sockets on PCB :/ (temps from Super Crowns Golf)
	ROM_LOAD( "cg21.7j",0x010000, 0x10000, BAD_DUMP CRC(0323a2cd) SHA1(d7d4b35ad451acb2fa3d117bb0ae2f8fbd883f17) ) // /
	ROM_LOAD( "2.4c",   0x020000, 0x20000, CRC(08d4363b) SHA1(60c5543c35f44af2f4a8f7ca4bc10633f5fa67fb) )

	ROM_REGION( 0x70000, "gfx1", 0 )
	ROM_LOAD( "chr1.3h",      0x000000, 0x020000, CRC(e62d2bb4) SHA1(f931699114a99b7eb25f8bb841d85de0d6a106a5) )
	ROM_LOAD( "chr2.3g",      0x020000, 0x020000, CRC(808c15e6) SHA1(d7d1ac7456f492dfcc1c1b501f8dde86e405fd7b) )
	ROM_LOAD( "chr3.3e",      0x040000, 0x020000, CRC(9a60193d) SHA1(d22c958b5bd82626fcfc94f7ad16d8cd4bacdda2) )
	ROM_LOAD( "chr4.3d",      0x060000, 0x010000, CRC(0fb88270) SHA1(d85a7f1bc5b3c4b13bbd887cea4c055541cbb737) )
ROM_END




static DRIVER_INIT( suprgolf )
{
	UINT8 *ROM = machine.region("user2")->base();

	ROM[0x74f4-0x4000] = 0x00;
	ROM[0x74f5-0x4000] = 0x00;
	ROM[0x6d72+(0x4000*3)-0x4000] = 0x20; //patch ROM check
}

GAME( 1989, suprgolf,  0,         suprgolf,  suprgolf,  suprgolf, ROT0, "Nasco", "Super Crowns Golf (Japan)", GAME_IMPERFECT_GRAPHICS | GAME_NO_COCKTAIL )
GAME( 1989, albatross, suprgolf,  suprgolf,  suprgolf,  0,        ROT0, "Nasco", "Albatross (US Prototype?)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_NO_COCKTAIL )
