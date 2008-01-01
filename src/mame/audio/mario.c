#include "driver.h"
#include "cpu/i8039/i8039.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "sound/samples.h"

#include "includes/mario.h"

/****************************************************************
 *
 * Defines and Macros
 *
 ****************************************************************/

#define ACTIVELOW_PORT_BIT(P,A,D)   ((P & (~(1 << A))) | ((D ^ 1) << A))
#define ACTIVEHIGH_PORT_BIT(P,A,D)   ((P & (~(1 << A))) | (D << A))

/*************************************
 *
 *  statics
 *
 *************************************/

static UINT8 p[8] = { 0,0xf0,0,0,0,0,0,0 };
static UINT8 t[2] = { 0,0 };

static UINT8 last;

/****************************************************************
 *
 * Initialization
 *
 ****************************************************************/

static SOUND_START( mario )
{
	p[1] = 0xf0;
	state_save_register_global_array(p);
	state_save_register_global_array(t);
	state_save_register_global(last);
}

/****************************************************************
 *
 * I/O Handlers - static
 *
 ****************************************************************/

static READ8_HANDLER( mario_sh_p1_r )
{
	return p[1];
}

static READ8_HANDLER( mario_sh_p2_r )
{
	return p[2];
}

static READ8_HANDLER( mario_sh_t0_r )
{
	return t[0];
}

static READ8_HANDLER( mario_sh_t1_r )
{
	return t[1];
}

static READ8_HANDLER( mario_sh_tune_r )
{
	return soundlatch_r(offset);
}

static WRITE8_HANDLER( mario_sh_sound_w )
{
	DAC_data_w(0,data);
}

static WRITE8_HANDLER( mario_sh_p1_w )
{
	p[1] = data;
}

static WRITE8_HANDLER( mario_sh_p2_w )
{
	p[2] = data;
}

/****************************************************************
 *
 * I/O Handlers - global
 *
 ****************************************************************/

WRITE8_HANDLER( mario_sh_getcoin_w )
{
	t[0] = data;
}

WRITE8_HANDLER( mario_sh_crab_w )
{
	p[1] = ACTIVEHIGH_PORT_BIT(p[1],0,data);
}

WRITE8_HANDLER( mario_sh_turtle_w )
{
	p[1] = ACTIVEHIGH_PORT_BIT(p[1],1,data);
}

WRITE8_HANDLER( mario_sh_fly_w )
{
	p[1] = ACTIVEHIGH_PORT_BIT(p[1],2,data);
}

WRITE8_HANDLER( masao_sh_irqtrigger_w )
{
	if (last == 1 && data == 0)
	{
		/* setting bit 0 high then low triggers IRQ on the sound CPU */
		cpunum_set_input_line_and_vector(1,0,HOLD_LINE,0xff);
	}

	last = data;
}

WRITE8_HANDLER( mario_sh_tuneselect_w )
{
	soundlatch_w(offset,data);
}

WRITE8_HANDLER( mario_sh_w )
{
	if (data)
		cpunum_set_input_line(1,0,ASSERT_LINE);
	else
		cpunum_set_input_line(1,0,CLEAR_LINE);
}


/* Mario running sample */
WRITE8_HANDLER( mario_sh1_w )
{
	static int last;

	if (last!= data)
	{
		last = data;
                if (data && sample_playing(0) == 0) sample_start (0, 3, 0);
	}
}

/* Luigi running sample */
WRITE8_HANDLER( mario_sh2_w )
{
	static int last;

	if (last!= data)
	{
		last = data;
                if (data && sample_playing(1) == 0) sample_start (1, 4, 0);
	}
}

/* Misc samples */
WRITE8_HANDLER( mario_sh3_w )
{
	static int state[8];

	/* Don't trigger the sample if it's still playing */
	if (state[offset] == data) return;

	state[offset] = data;
	if (data)
	{
		switch (offset)
		{
			case 2: /* ice */
				sample_start (2, 0, 0);
				break;
			case 6: /* coin */
				sample_start (2, 1, 0);
				break;
			case 7: /* skid */
				sample_start (2, 2, 0);
				break;
		}
	}
}

/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( mario_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mario_sound_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READWRITE(mario_sh_tune_r, mario_sh_sound_w)
	AM_RANGE(I8039_p1, I8039_p1) AM_READWRITE(mario_sh_p1_r, mario_sh_p1_w)
	AM_RANGE(I8039_p2, I8039_p2) AM_READWRITE(mario_sh_p2_r, mario_sh_p2_w)
	AM_RANGE(I8039_t0, I8039_t0) AM_READ(mario_sh_t0_r)
	AM_RANGE(I8039_t1, I8039_t1) AM_READ(mario_sh_t1_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( masao_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_READWRITE(AY8910_read_port_0_r, AY8910_write_port_0_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(AY8910_control_port_0_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Sound Interfaces
 *
 *************************************/

static const char *const mario_sample_names[] =
{
	"*mario",

	/* 7f01 - 7f07 sounds */
	"ice.wav",    /* 0x02 ice appears (formerly effect0.wav) */
	"coin.wav",   /* 0x06 coin appears (formerly effect1.wav) */
	"skid.wav",   /* 0x07 skid */

	/* 7c00 */
	"run.wav",        /* 03, 02, 01 - 0x1b */

	/* 7c80 */
	"luigirun.wav",   /* 03, 02, 01 - 0x1c */

    0	/* end of array */
};

static const struct Samplesinterface samples_interface =
{
	3,	/* 3 channels */
	mario_sample_names
};

static const struct AY8910interface ay8910_interface =
{
	soundlatch_r
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_DRIVER_START( mario_audio )

	MDRV_CPU_ADD(I8039, I8035_CLOCK)  /* audio CPU */         /* 730 kHz */
	MDRV_CPU_PROGRAM_MAP(mario_sound_map, 0)
	MDRV_CPU_IO_MAP(mario_sound_io_map, 0)
	
	MDRV_SOUND_START(mario)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	
	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	
	MDRV_SOUND_ADD(SAMPLES, 0)
	MDRV_SOUND_CONFIG(samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

MACHINE_DRIVER_END

MACHINE_DRIVER_START( masao_audio )

	MDRV_CPU_ADD(Z80,24576000/16) /* audio CPU */	/* ???? */
	MDRV_CPU_PROGRAM_MAP(masao_sound_map,0)
	
	MDRV_SOUND_START(mario)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	
	MDRV_SOUND_ADD(AY8910, 14318000/6)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_DRIVER_END

