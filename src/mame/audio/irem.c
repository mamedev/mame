/***************************************************************************

    Irem M52/M62 sound hardware

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "irem.h"
#include "cpu/m6800/m6800.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"

static UINT8 port1, port2;


/*************************************
 *
 *  Sound board initialization
 *
 *************************************/

static SOUND_START( irem_audio )
{
	state_save_register_global(port1);
	state_save_register_global(port2);
}



/*************************************
 *
 *  External writes to the sound 
 *	command register
 *
 *************************************/

WRITE8_HANDLER( irem_sound_cmd_w )
{
	if ((data & 0x80) == 0)
		soundlatch_w(0, data & 0x7f);
	else
		cpunum_set_input_line(Machine, 1, 0, ASSERT_LINE);
}



/*************************************
 *
 *  6803 output ports
 *
 *************************************/

static WRITE8_HANDLER( m6803_port1_w )
{
	port1 = data;
}


static WRITE8_HANDLER( m6803_port2_w )
{
	/* write latch */
	if ((port2 & 0x01) && !(data & 0x01))
	{
		/* control or data port? */
		if (port2 & 0x04)
		{
			/* PSG 0 or 1? */
			if (port2 & 0x08)
				AY8910_control_port_0_w(0,port1);
			if (port2 & 0x10)
				AY8910_control_port_1_w(0,port1);
		}
		else
		{
			/* PSG 0 or 1? */
			if (port2 & 0x08)
				AY8910_write_port_0_w(0,port1);
			if (port2 & 0x10)
				AY8910_write_port_1_w(0,port1);
		}
	}
	port2 = data;
}



/*************************************
 *
 *  6803 input ports ports
 *
 *************************************/

static READ8_HANDLER( m6803_port1_r )
{
	/* PSG 0 or 1? */
	if (port2 & 0x08)
		return AY8910_read_port_0_r(0);
	if (port2 & 0x10)
		return AY8910_read_port_1_r(0);
	return 0xff;
}


static READ8_HANDLER( m6803_port2_r )
{
	return 0;
}



/*************************************
 *
 *  AY-8910 output ports
 *
 *************************************/

static WRITE8_HANDLER( ay8910_0_portb_w )
{
	/* bits 2-4 select MSM5205 clock & 3b/4b playback mode */
	MSM5205_playmode_w(0, (data >> 2) & 7);
	MSM5205_playmode_w(1, ((data >> 2) & 4) | 3);	/* always in slave mode */

	/* bits 0 and 1 reset the two chips */
	MSM5205_reset_w(0, data & 1);
	MSM5205_reset_w(1, data & 2);
}


static WRITE8_HANDLER( ay8910_1_porta_w )
{
#ifdef MAME_DEBUG
	if (data & 0x0f) popmessage("analog sound %x",data&0x0f);
#endif
}



/*************************************
 *
 *  Memory-mapped accesses
 *
 *************************************/

static WRITE8_HANDLER( sound_irq_ack_w )
{
	cpunum_set_input_line(Machine, 1, 0, CLEAR_LINE);
}


static WRITE8_HANDLER( m52_adpcm_w )
{
	if (offset & 1)
		MSM5205_data_w(0, data);
	if (offset & 2)
		MSM5205_data_w(1, data);
}


static WRITE8_HANDLER( m62_adpcm_w )
{
	MSM5205_data_w(offset, data);
}



/*************************************
 *
 *  MSM5205 data ready signals
 *
 *************************************/

static void adpcm_int(int data)
{
	cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, PULSE_LINE);

	/* the first MSM5205 clocks the second */
	MSM5205_vclk_w(1,1);
	MSM5205_vclk_w(1,0);
}



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const struct AY8910interface irem_ay8910_interface_1 =
{
	soundlatch_r,
	0,
	0,
	ay8910_0_portb_w
};

static const struct AY8910interface irem_ay8910_interface_2 =
{
	0,
	0,
	ay8910_1_porta_w,
	0
};

static const struct MSM5205interface irem_msm5205_interface_1 =
{
	adpcm_int,			/* interrupt function */
	MSM5205_S96_4B		/* default to 4KHz, but can be changed at run time */
};

static const struct MSM5205interface irem_msm5205_interface_2 =
{
	0,				/* interrupt function */
	MSM5205_SEX_4B		/* default to 4KHz, but can be changed at run time, slave */
};



/*************************************
 *
 *  Address maps
 *
 *************************************/

/* complete address map verified from Moon Patrol/10 Yard Fight schematics */
/* large map uses 8k ROMs, small map uses 4k ROMs; this is selected via a jumper */
static ADDRESS_MAP_START( m52_small_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(15) )
	AM_RANGE(0x0000, 0x0fff) AM_WRITE(m52_adpcm_w)
	AM_RANGE(0x1000, 0x1fff) AM_WRITE(sound_irq_ack_w)
	AM_RANGE(0x2000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( m52_large_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(m52_adpcm_w)
	AM_RANGE(0x2000, 0x3fff) AM_WRITE(sound_irq_ack_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


/* complete address map verified from Kid Niki schematics */
static ADDRESS_MAP_START( m62_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0800, 0x0800) AM_MIRROR(0xf7fc) AM_WRITE(sound_irq_ack_w)
	AM_RANGE(0x0801, 0x0802) AM_MIRROR(0xf7fc) AM_WRITE(m62_adpcm_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( irem_sound_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(M6803_PORT1, M6803_PORT1) AM_READWRITE(m6803_port1_r, m6803_port1_w)
	AM_RANGE(M6803_PORT2, M6803_PORT2) AM_READWRITE(m6803_port2_r, m6803_port2_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_DRIVER_START( irem_audio_base )

	MDRV_SOUND_START(irem_audio)

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("iremsound", M6803, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_CPU_IO_MAP(irem_sound_portmap,0)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, XTAL_3_579545MHz/4) /* verified on pcb */
	MDRV_SOUND_CONFIG(irem_ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD(AY8910, XTAL_3_579545MHz/4) /* verified on pcb */
	MDRV_SOUND_CONFIG(irem_ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD(MSM5205, XTAL_384kHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(irem_msm5205_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD(MSM5205, XTAL_384kHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(irem_msm5205_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( m52_small_audio )
	MDRV_IMPORT_FROM(irem_audio_base)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("iremsound")
	MDRV_CPU_PROGRAM_MAP(m52_small_sound_map,0)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( m52_large_audio )	/* 10 yard fight */
	MDRV_IMPORT_FROM(irem_audio_base)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("iremsound")
	MDRV_CPU_PROGRAM_MAP(m52_large_sound_map,0)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( m62_audio )
	MDRV_IMPORT_FROM(irem_audio_base)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("iremsound")
	MDRV_CPU_PROGRAM_MAP(m62_sound_map,0)
MACHINE_DRIVER_END
