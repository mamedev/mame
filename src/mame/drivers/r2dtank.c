/*******************************************************************
R2D Tank (c) 1980 Sigma Ent. Inc.

driver by: David Haywood & Pierpaolo Prazzoli


from the readme
----------------------------------------------------
Orca board number OVG-17A

r2d1.1c is ROM #1 at board position 1C, and so on.

1 = 2716
2 = 2732
3 = 2732
4 = 2732
5 = 2716 Sound

CPU = 6809
other = HD46505SP (6845) (CRT controller)
other = MB14282(x2)
other = HD468458SP
other = MB14282
other = MB14368
other = HD6821 (x2) (PIA)
other = HD46802
other = M5L8226 (x2)
RAM = 4116 (x11)

----------------------------------------------------
********************************************************************/

#include "driver.h"
#include "rescap.h"
#include "machine/6821pia.h"
#include "machine/74123.h"
#include "video/crtc6845.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"


#define LOG_AUDIO_COMM	(0)

#define MAIN_CPU_MASTER_CLOCK	(11200000)
#define PIXEL_CLOCK				(MAIN_CPU_MASTER_CLOCK / 2)
#define CRTC_CLOCK				(MAIN_CPU_MASTER_CLOCK / 16)


static UINT8 *r2dtank_videoram;
static UINT8 *r2dtank_colorram;
static UINT8 flipscreen;
static UINT32 ttl74123_output;
static UINT8 AY8910_selected;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static WRITE8_HANDLER( flipscreen_w );



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static void main_cpu_irq(int state)
{
	cpunum_set_input_line(0, M6809_IRQ_LINE,  state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Audio system - CPU 1
 *
 *************************************/

static READ8_HANDLER( audio_command_r )
{
	UINT8 ret = soundlatch_r(0);

if (LOG_AUDIO_COMM) logerror("%08X  CPU#1  Audio Command Read: %x\n", safe_activecpu_get_pc(), ret);

	return ret;
}


static WRITE8_HANDLER( audio_command_w )
{
	soundlatch_w(0, ~data);
	cpunum_set_input_line(1, M6802_IRQ_LINE, HOLD_LINE);

if (LOG_AUDIO_COMM) logerror("%08X   CPU#0  Audio Command Write: %x\n", safe_activecpu_get_pc(), data^0xff);
}


static READ8_HANDLER( audio_answer_r )
{
	UINT8 ret = soundlatch2_r(0);
if (LOG_AUDIO_COMM) logerror("%08X  CPU#0  Audio Answer Read: %x\n", safe_activecpu_get_pc(), ret);

	return ret;
}


static WRITE8_HANDLER( audio_answer_w )
{
	/* HACK - prevents lock-up, but causes game to end some in-between sreens prematurely */
	if (safe_activecpu_get_pc() == 0xfb12)
		data = 0x00;

	soundlatch2_w(0, data);
	cpunum_set_input_line(0, M6809_IRQ_LINE, HOLD_LINE);

if (LOG_AUDIO_COMM) logerror("%08X  CPU#1  Audio Answer Write: %x\n", safe_activecpu_get_pc(), data);
}


static WRITE8_HANDLER( AY8910_select_w )
{
	/* not sure what all the bits mean:
       D0 - ????? definetely used
       D1 - not used?
       D2 - selects ay8910 control or port
       D3 - selects ay8910 #0
       D4 - selects ay8910 #1
       D5-D7 - not used */
	AY8910_selected = data;

if (LOG_AUDIO_COMM) logerror("%08X  CPU#1  AY8910_select_w: %x\n", safe_activecpu_get_pc(), data);
}


static READ8_HANDLER( AY8910_port_r )
{
	UINT8 ret = 0;

	if (AY8910_selected & 0x08)
{
		ret = AY8910_read_port_0_r(offset);
if (LOG_AUDIO_COMM) logerror("%08X  CPU#1  AY8910 #0 Port Read: %x\n", safe_activecpu_get_pc(), ret);}

	if (AY8910_selected & 0x10)
{
		ret = AY8910_read_port_1_r(offset);
if (LOG_AUDIO_COMM) logerror("%08X  CPU#1  AY8910 #1 Port Read: %x\n", safe_activecpu_get_pc(), ret);}

	return ret;
}


static WRITE8_HANDLER( AY8910_port_w )
{
	if (AY8910_selected & 0x04)
	{
		if (AY8910_selected & 0x08)
{if (LOG_AUDIO_COMM) logerror("%08X  CPU#1  AY8910 #0 Control Write: %x\n", safe_activecpu_get_pc(), data);
			AY8910_control_port_0_w(offset, data);
}
		if (AY8910_selected & 0x10)
{if (LOG_AUDIO_COMM) logerror("%08X  CPU#1  AY8910 #1 Control Write: %x\n", safe_activecpu_get_pc(), data);
			AY8910_control_port_1_w(offset, data);
}
	}
	else
	{
		if (AY8910_selected & 0x08)
{if (LOG_AUDIO_COMM) logerror("%08X  CPU#1  AY8910 #0 Port Write: %x\n", safe_activecpu_get_pc(), data);
			AY8910_write_port_0_w(offset, data);
}

		if (AY8910_selected & 0x10)
{if (LOG_AUDIO_COMM) logerror("%08X  CPU#1  AY8910 #1 Port Write: %x\n", safe_activecpu_get_pc(), data);
			AY8910_write_port_1_w(offset, data);
}
	}
}


static struct AY8910interface ay8910_1_interface =
{
	input_port_3_r,
	0
};


static struct AY8910interface ay8910_2_interface =
{
	input_port_1_r,
	input_port_2_r
};



/*************************************
 *
 *  74123
 *
 *  This timer is responsible for
 *  delaying the PIA1's port input.
 *  This delay ensures that
 *  CA1 is only changed in the VBLANK
 *  region, but not in HBLANK
 *
 *************************************/

static void ttl74123_output_changed(int output)
{
	pia_0_ca1_w(0, output);
	ttl74123_output = output;
}


static UINT32 get_ttl74123_output(void *param)
{
	return ttl74123_output;
}


static TTL74123_interface ttl74123_intf =
{
	TTL74123_GROUNDED,	/* the hook up type */
	RES_K(22),			/* resistor connected to RCext */
	CAP_U(0.01),		/* capacitor connected to Cext and RCext */
	1,					/* A pin - driven by the CRTC */
	1,					/* B pin - pulled high */
	1,					/* Clear pin - pulled high */
	ttl74123_output_changed
};



/*************************************
 *
 *  Machine start
 *
 *************************************/

static const pia6821_interface pia_main_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_0_r, input_port_1_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, 0, flipscreen_w,
	/*irqs   : A/B             */ main_cpu_irq, main_cpu_irq
};


static const pia6821_interface pia_audio_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ AY8910_port_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ AY8910_port_w, AY8910_select_w, 0, 0,
	/*irqs   : A/B             */ main_cpu_irq, main_cpu_irq
};


static MACHINE_START( r2dtank )
{
	pia_config(0, &pia_main_intf);
	pia_config(1, &pia_audio_intf);

	TTL74123_config(0, &ttl74123_intf);

	/* setup for save states */
	state_save_register_global(flipscreen);
	state_save_register_global(ttl74123_output);
	state_save_register_global(AY8910_selected);
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

static MACHINE_RESET( r2dtank )
{
	pia_reset();
	TTL74123_reset(0);
}



/*************************************
 *
 *  Video system
 *
 *************************************/

#define NUM_PENS	(8)


static WRITE8_HANDLER( flipscreen_w )
{
	flipscreen = !data;
}


static void *begin_update(running_machine *machine, int screen,
						  mame_bitmap *bitmap, const rectangle *cliprect)
{
	/* create the pens */
	offs_t i;
	static pen_t pens[NUM_PENS];

	for (i = 0; i < NUM_PENS; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
	}

	return pens;
}


static void update_row(mame_bitmap *bitmap, const rectangle *cliprect,
					   UINT16 ma, UINT8 ra, UINT16 y, UINT8 x_count, void *param)
{
	UINT8 cx;

	pen_t *pens = (pen_t *)param;
	UINT8 x = 0;

	for (cx = 0; cx < x_count; cx++)
	{
		int i;
		UINT8 data, fore_color;

		/* the memory is hooked up to the MA, RA lines this way */
		offs_t offs = ((ma << 3) & 0x1f00) |
					  ((ra << 5) & 0x00e0) |
					  ((ma << 0) & 0x001f);

		if (flipscreen)
			offs = offs ^ 0x1fff;

		data = r2dtank_videoram[offs];
		fore_color = (r2dtank_colorram[offs] >> 5) & 0x07;

		for (i = 0; i < 8; i++)
		{
			UINT8 bit, color;

			if (flipscreen)
			{
				bit = data & 0x01;
				data = data >> 1;
			}
			else
			{
				bit = data & 0x80;
				data = data << 1;
			}

			color = bit ? fore_color : RGB_BLACK;
			*BITMAP_ADDR32(bitmap, y, x) = pens[color];

			x = x + 1;
		}

		ma = ma + 1;
	}
}


static void display_enable_changed(int display_enabled)
{
	TTL74123_A_w(0, display_enabled);
}


static const crtc6845_interface crtc6845_intf =
{
	0,						/* screen we are acting on */
	CRTC_CLOCK, 			/* the clock (pin 21) of the chip */
	8,						/* number of pixels per video memory address */
	begin_update,			/* before pixel update callback */
	update_row,				/* row update callback */
	0,						/* after pixel update callback */
	display_enable_changed	/* call back for display state changes */
};


static VIDEO_START( r2dtank )
{
	/* configure the CRT controller */
	crtc6845_config(0, &crtc6845_intf);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static WRITE8_HANDLER( pia_comp_0_w )
{
	pia_0_w(offset, ~data);
}


static ADDRESS_MAP_START( r2dtank_main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_BASE(&r2dtank_videoram)
	AM_RANGE(0x2000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_BASE(&r2dtank_colorram)
	AM_RANGE(0x6000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x8003) AM_READWRITE(pia_0_r, pia_comp_0_w)
	AM_RANGE(0x8004, 0x8004) AM_READWRITE(audio_answer_r, audio_command_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(crtc6845_address_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(crtc6845_register_w)
	AM_RANGE(0xc000, 0xc007) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0xc800, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( r2dtank_audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007f) AM_RAM		/* internal RAM */
	AM_RANGE(0xd000, 0xd003) AM_READWRITE(pia_1_r, pia_1_w)
	AM_RANGE(0xf000, 0xf000) AM_READWRITE(audio_command_r, audio_answer_w)
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( r2dtank )

	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_ttl74123_output, 0)

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )

	PORT_START_TAG("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START_TAG("DSWB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( r2dtank )
	MDRV_CPU_ADD(M6809,3000000)		 /* ?? too fast ? */
	MDRV_CPU_PROGRAM_MAP(r2dtank_main_map,0)

	/* audio CPU */
	MDRV_CPU_ADD(M6802,3000000/4)			/* ?? */
	MDRV_CPU_PROGRAM_MAP(r2dtank_audio_map,0)

	MDRV_MACHINE_START(r2dtank)
	MDRV_MACHINE_RESET(r2dtank)
	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_VIDEO_START(r2dtank)
	MDRV_VIDEO_UPDATE(crtc6845)

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 256, 0, 256, 256, 0, 256)	/* temporary, CRTC will configure screen */

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, (4000000 / 4))
	MDRV_SOUND_CONFIG(ay8910_1_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, (4000000 / 4))
	MDRV_SOUND_CONFIG(ay8910_2_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( r2dtank )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "r2d1.1c",      0xc800, 0x0800, CRC(20606a0f) SHA1(9a55e595c7ea332bdc89142338947be8a28a92a3) )
	ROM_LOAD( "r2d2.1a",      0xd000, 0x1000, CRC(7561c67f) SHA1(cccc7bbd7975db340fe571a4c31c25b41b2563b8) )
	ROM_LOAD( "r2d3.2c",      0xe000, 0x1000, CRC(fc53c538) SHA1(8f9a2edcf7a2cb2a8ddd084828b52f1bf45f434a) )
	ROM_LOAD( "r2d4.2a",      0xf000, 0x1000, CRC(56636225) SHA1(dcfc6e29b4c51a45cfbecf6790b7d88b89af433b) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "r2d5.7l",      0xf800, 0x0800, CRC(c49bed15) SHA1(ffa635a65c024c532bb13fb91bbd3e54923e81bf) )
ROM_END



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1980, r2dtank, 0, r2dtank, r2dtank, 0, ROT270, "Sigma Enterprises Inc.", "R2D Tank", GAME_SUPPORTS_SAVE)
