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

#include "emu.h"
#include "machine/rescap.h"
#include "machine/6821pia.h"
#include "machine/74123.h"
#include "video/mc6845.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"


#define NUM_PENS	(8)

#define LOG_AUDIO_COMM	(0)

#define MAIN_CPU_MASTER_CLOCK	(11200000)
#define PIXEL_CLOCK				(MAIN_CPU_MASTER_CLOCK / 2)
#define CRTC_CLOCK				(MAIN_CPU_MASTER_CLOCK / 16)

class r2dtank_state : public driver_device
{
public:
	r2dtank_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 m_flipscreen;
	UINT32 m_ttl74123_output;
	UINT8 m_AY8910_selected;
	pen_t m_pens[NUM_PENS];
	DECLARE_READ8_MEMBER(audio_command_r);
	DECLARE_WRITE8_MEMBER(audio_command_w);
	DECLARE_READ8_MEMBER(audio_answer_r);
	DECLARE_WRITE8_MEMBER(audio_answer_w);
};



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( flipscreen_w );



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( main_cpu_irq )
{
	pia6821_device *pia0 = device->machine().device<pia6821_device>("pia_main");
	pia6821_device *pia1 = device->machine().device<pia6821_device>("pia_audio");
	int combined_state = pia0->irq_a_state() | pia0->irq_b_state() |
						 pia1->irq_a_state() | pia1->irq_b_state();

	cputag_set_input_line(device->machine(), "maincpu", M6809_IRQ_LINE,  combined_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Audio system - CPU 1
 *
 *************************************/

READ8_MEMBER(r2dtank_state::audio_command_r)
{
	UINT8 ret = soundlatch_r(space, 0);

if (LOG_AUDIO_COMM) logerror("%08X  CPU#1  Audio Command Read: %x\n", cpu_get_pc(&space.device()), ret);

	return ret;
}


WRITE8_MEMBER(r2dtank_state::audio_command_w)
{
	soundlatch_w(space, 0, ~data);
	cputag_set_input_line(machine(), "audiocpu", M6800_IRQ_LINE, HOLD_LINE);

if (LOG_AUDIO_COMM) logerror("%08X   CPU#0  Audio Command Write: %x\n", cpu_get_pc(&space.device()), data^0xff);
}


READ8_MEMBER(r2dtank_state::audio_answer_r)
{
	UINT8 ret = soundlatch2_r(space, 0);
if (LOG_AUDIO_COMM) logerror("%08X  CPU#0  Audio Answer Read: %x\n", cpu_get_pc(&space.device()), ret);

	return ret;
}


WRITE8_MEMBER(r2dtank_state::audio_answer_w)
{
	/* HACK - prevents lock-up, but causes game to end some in-between sreens prematurely */
	if (cpu_get_pc(&space.device()) == 0xfb12)
		data = 0x00;

	soundlatch2_w(space, 0, data);
	cputag_set_input_line(machine(), "maincpu", M6809_IRQ_LINE, HOLD_LINE);

if (LOG_AUDIO_COMM) logerror("%08X  CPU#1  Audio Answer Write: %x\n", cpu_get_pc(&space.device()), data);
}


static WRITE8_DEVICE_HANDLER( AY8910_select_w )
{
	r2dtank_state *state = device->machine().driver_data<r2dtank_state>();
	/* not sure what all the bits mean:
       D0 - ????? definetely used
       D1 - not used?
       D2 - selects ay8910 control or port
       D3 - selects ay8910 #0
       D4 - selects ay8910 #1
       D5-D7 - not used */
	state->m_AY8910_selected = data;

if (LOG_AUDIO_COMM) logerror("%s:  CPU#1  AY8910_select_w: %x\n", device->machine().describe_context(), data);
}


static READ8_DEVICE_HANDLER( AY8910_port_r )
{
	r2dtank_state *state = device->machine().driver_data<r2dtank_state>();
	UINT8 ret = 0;

	if (state->m_AY8910_selected & 0x08)
		ret = ay8910_r(device->machine().device("ay1"), 0);

	if (state->m_AY8910_selected & 0x10)
		ret = ay8910_r(device->machine().device("ay2"), 0);

	return ret;
}


static WRITE8_DEVICE_HANDLER( AY8910_port_w )
{
	r2dtank_state *state = device->machine().driver_data<r2dtank_state>();
	if (state->m_AY8910_selected & 0x08)
		ay8910_data_address_w(device->machine().device("ay1"), state->m_AY8910_selected >> 2, data);

	if (state->m_AY8910_selected & 0x10)
		ay8910_data_address_w(device->machine().device("ay2"), state->m_AY8910_selected >> 2, data);
}


static const ay8910_interface ay8910_1_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSWB"),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


static const ay8910_interface ay8910_2_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("IN1"),
	DEVCB_INPUT_PORT("DSWA"),
	DEVCB_NULL,
	DEVCB_NULL
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

static WRITE8_DEVICE_HANDLER( ttl74123_output_changed )
{
	r2dtank_state *state = device->machine().driver_data<r2dtank_state>();
	pia6821_device *pia = device->machine().device<pia6821_device>("pia_main");
	pia->ca1_w(data);
	state->m_ttl74123_output = data;
}


static CUSTOM_INPUT( get_ttl74123_output )
{
	r2dtank_state *state = field.machine().driver_data<r2dtank_state>();
	return state->m_ttl74123_output;
}


static const ttl74123_interface ttl74123_intf =
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
	DEVCB_INPUT_PORT("IN0"),		/* port A in */
	DEVCB_INPUT_PORT("IN1"),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_LINE(flipscreen_w),		/* port CB2 out */
	DEVCB_LINE(main_cpu_irq),		/* IRQA */
	DEVCB_LINE(main_cpu_irq)		/* IRQB */
};


static const pia6821_interface pia_audio_intf =
{
	DEVCB_HANDLER(AY8910_port_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(AY8910_port_w),		/* port A out */
	DEVCB_HANDLER(AY8910_select_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_LINE(main_cpu_irq),		/* IRQA */
	DEVCB_LINE(main_cpu_irq)		/* IRQB */
};


static MACHINE_START( r2dtank )
{
	r2dtank_state *state = machine.driver_data<r2dtank_state>();
	/* setup for save states */
	state_save_register_global(machine, state->m_flipscreen);
	state_save_register_global(machine, state->m_ttl74123_output);
	state_save_register_global(machine, state->m_AY8910_selected);
}



/*************************************
 *
 *  Video system
 *
 *************************************/


static WRITE_LINE_DEVICE_HANDLER( flipscreen_w )
{
	r2dtank_state *drvstate = device->machine().driver_data<r2dtank_state>();
	drvstate->m_flipscreen = !state;
}


static MC6845_BEGIN_UPDATE( begin_update )
{
	r2dtank_state *state = device->machine().driver_data<r2dtank_state>();
	/* create the pens */
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		state->m_pens[i] = MAKE_RGB(pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
	}

	return state->m_pens;
}


static MC6845_UPDATE_ROW( update_row )
{
	r2dtank_state *state = device->machine().driver_data<r2dtank_state>();
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

		if (state->m_flipscreen)
			offs = offs ^ 0x1fff;

		data = state->m_videoram[offs];
		fore_color = (state->m_colorram[offs] >> 5) & 0x07;

		for (i = 0; i < 8; i++)
		{
			UINT8 bit, color;

			if (state->m_flipscreen)
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
			bitmap.pix32(y, x) = pens[color];

			x = x + 1;
		}

		ma = ma + 1;
	}
}


static WRITE_LINE_DEVICE_HANDLER( display_enable_changed )
{
	ttl74123_a_w(device->machine().device("74123"), 0, state);
}


static const mc6845_interface mc6845_intf =
{
	"screen",				/* screen we are acting on */
	8,						/* number of pixels per video memory address */
	begin_update,			/* before pixel update callback */
	update_row,				/* row update callback */
	NULL,					/* after pixel update callback */
	DEVCB_LINE(display_enable_changed),	/* callback for display state changes */
	DEVCB_NULL,				/* callback for cursor state changes */
	DEVCB_NULL,				/* HSYNC callback */
	DEVCB_NULL,				/* VSYNC callback */
	NULL					/* update address callback */
};



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( pia_comp_w )
{
	downcast<pia6821_device *>(device)->write(*device->machine().memory().first_space(), offset, ~data);
}


static ADDRESS_MAP_START( r2dtank_main_map, AS_PROGRAM, 8, r2dtank_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_BASE(m_videoram)
	AM_RANGE(0x2000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_BASE(m_colorram)
	AM_RANGE(0x6000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x8003) AM_DEVREAD("pia_main", pia6821_device, read) AM_DEVWRITE_LEGACY("pia_main", pia_comp_w)
	AM_RANGE(0x8004, 0x8004) AM_READWRITE(audio_answer_r, audio_command_w)
	AM_RANGE(0xb000, 0xb000) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xb001, 0xb001) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xc000, 0xc007) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xc800, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( r2dtank_audio_map, AS_PROGRAM, 8, r2dtank_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM		/* internal RAM */
	AM_RANGE(0xd000, 0xd003) AM_DEVREADWRITE("pia_audio", pia6821_device, read, write)
	AM_RANGE(0xf000, 0xf000) AM_READWRITE(audio_command_r, audio_answer_w)
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( r2dtank )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_ttl74123_output, NULL)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )

	PORT_START("DSWA")
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

	PORT_START("DSWB")
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

static MACHINE_CONFIG_START( r2dtank, r2dtank_state )
	MCFG_CPU_ADD("maincpu", M6809,3000000)		 /* ?? too fast ? */
	MCFG_CPU_PROGRAM_MAP(r2dtank_main_map)

	MCFG_CPU_ADD("audiocpu", M6802,3000000)			/* ?? */
	MCFG_CPU_PROGRAM_MAP(r2dtank_audio_map)

	MCFG_MACHINE_START(r2dtank)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 256, 0, 256, 256, 0, 256)	/* temporary, CRTC will configure screen */
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)

	MCFG_MC6845_ADD("crtc", MC6845, CRTC_CLOCK, mc6845_intf)

	/* 74LS123 */

	MCFG_TTL74123_ADD("74123", ttl74123_intf)

	MCFG_PIA6821_ADD("pia_main", pia_main_intf)
	MCFG_PIA6821_ADD("pia_audio", pia_audio_intf)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, (4000000 / 4))
	MCFG_SOUND_CONFIG(ay8910_1_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, (4000000 / 4))
	MCFG_SOUND_CONFIG(ay8910_2_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( r2dtank )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r2d1.1c",      0xc800, 0x0800, CRC(20606a0f) SHA1(9a55e595c7ea332bdc89142338947be8a28a92a3) )
	ROM_LOAD( "r2d2.1a",      0xd000, 0x1000, CRC(7561c67f) SHA1(cccc7bbd7975db340fe571a4c31c25b41b2563b8) )
	ROM_LOAD( "r2d3.2c",      0xe000, 0x1000, CRC(fc53c538) SHA1(8f9a2edcf7a2cb2a8ddd084828b52f1bf45f434a) )
	ROM_LOAD( "r2d4.2a",      0xf000, 0x1000, CRC(56636225) SHA1(dcfc6e29b4c51a45cfbecf6790b7d88b89af433b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "r2d5.7l",      0xf800, 0x0800, CRC(c49bed15) SHA1(ffa635a65c024c532bb13fb91bbd3e54923e81bf) )
ROM_END



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1980, r2dtank, 0, r2dtank, r2dtank, 0, ROT270, "Sigma Enterprises Inc.", "R2D Tank", GAME_SUPPORTS_SAVE)
