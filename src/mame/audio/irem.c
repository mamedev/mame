#include "driver.h"
#include "irem.h"
#include "cpu/m6800/m6800.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"


WRITE8_HANDLER( irem_sound_cmd_w )
{
	if ((data & 0x80) == 0)
		soundlatch_w(0,data & 0x7f);
	else
		cpunum_set_input_line(1,0,HOLD_LINE);
}


static int port1,port2;

static WRITE8_HANDLER( irem_port1_w )
{
	port1 = data;
}

static WRITE8_HANDLER( irem_port2_w )
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


static READ8_HANDLER( irem_port1_r )
{
	/* PSG 0 or 1? */
	if (port2 & 0x08)
		return AY8910_read_port_0_r(0);
	if (port2 & 0x10)
		return AY8910_read_port_1_r(0);
	return 0xff;
}

static READ8_HANDLER( irem_port2_r )
{
	return 0;
}



static WRITE8_HANDLER( irem_msm5205_w )
{
	/* bits 2-4 select MSM5205 clock & 3b/4b playback mode */
	MSM5205_playmode_w(0,(data >> 2) & 7);
	MSM5205_playmode_w(1,((data >> 2) & 4) | 3);	/* always in slave mode */

	/* bits 0 and 1 reset the two chips */
	MSM5205_reset_w(0,data & 1);
	MSM5205_reset_w(1,data & 2);
}

static WRITE8_HANDLER( irem_adpcm_w )
{
	MSM5205_data_w(offset,data);
}

static void irem_adpcm_int(int data)
{
	cpunum_set_input_line(1, INPUT_LINE_NMI, PULSE_LINE);

	/* the first MSM5205 clocks the second */
	MSM5205_vclk_w(1,1);
	MSM5205_vclk_w(1,0);
}

static WRITE8_HANDLER( irem_analog_w )
{
#ifdef MAME_DEBUG
if (data&0x0f) popmessage("analog sound %x",data&0x0f);
#endif
}


static const struct AY8910interface irem_ay8910_interface_1 =
{
	soundlatch_r,
	0,
	0,
	irem_msm5205_w
};

static const struct AY8910interface irem_ay8910_interface_2 =
{
	0,
	0,
	irem_analog_w,
	0
};

static const struct MSM5205interface irem_msm5205_interface_1 =
{
	irem_adpcm_int,		/* interrupt function */
	MSM5205_S96_4B		/* default to 4KHz, but can be changed at run time */
};

static const struct MSM5205interface irem_msm5205_interface_2 =
{
	0,				/* interrupt function */
	MSM5205_SEX_4B		/* default to 4KHz, but can be changed at run time, slave */
};



static ADDRESS_MAP_START( irem_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x4000, 0xffff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( irem_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0800, 0x0800) AM_WRITE(MWA8_NOP)    /* IACK */
	AM_RANGE(0x0801, 0x0802) AM_WRITE(irem_adpcm_w)
	AM_RANGE(0x9000, 0x9000) AM_WRITE(MWA8_NOP)    /* IACK */
	AM_RANGE(0x4000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( irem_sound_readport, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(M6803_PORT1, M6803_PORT1) AM_READ(irem_port1_r)
	AM_RANGE(M6803_PORT2, M6803_PORT2) AM_READ(irem_port2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( irem_sound_writeport, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(M6803_PORT1, M6803_PORT1) AM_WRITE(irem_port1_w)
	AM_RANGE(M6803_PORT2, M6803_PORT2) AM_WRITE(irem_port2_w)
ADDRESS_MAP_END


MACHINE_DRIVER_START( irem_audio )

	/* basic machine hardware */
	MDRV_CPU_ADD(M6803, XTAL_3_579545MHz) /* verified on pcb */
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(irem_sound_readmem,irem_sound_writemem)
	MDRV_CPU_IO_MAP(irem_sound_readport,irem_sound_writeport)

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
