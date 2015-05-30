// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    Irem M52/M62 sound hardware

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "sound/discrete.h"
#include "audio/irem.h"


const device_type IREM_AUDIO = &device_creator<irem_audio_device>;

irem_audio_device::irem_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, IREM_AUDIO, "Irem Audio", tag, owner, clock, "irem_audio", __FILE__),
		device_sound_interface(mconfig, *this),
		m_port1(0),
		m_port2(0)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void irem_audio_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void irem_audio_device::device_start()
{
	m_adpcm1 = machine().device<msm5205_device>("msm1");
	m_adpcm2 = machine().device<msm5205_device>("msm2");
	m_ay1 = machine().device<ay8910_device>("ay1");
	m_ay2 = machine().device<ay8910_device>("ay2");

	save_item(NAME(m_port1));
	save_item(NAME(m_port2));
}




/*************************************
 *
 *  External writes to the sound
 *  command register
 *
 *************************************/

WRITE8_MEMBER( irem_audio_device::cmd_w )
{
	driver_device *drvstate = space.machine().driver_data<driver_device>();
	if ((data & 0x80) == 0)
		drvstate->soundlatch_byte_w(space, 0, data & 0x7f);
	else
		space.machine().device("iremsound")->execute().set_input_line(0, ASSERT_LINE);
}



/*************************************
 *
 *  6803 output ports
 *
 *************************************/

WRITE8_MEMBER( irem_audio_device::m6803_port1_w )
{
	m_port1 = data;
}


WRITE8_MEMBER( irem_audio_device::m6803_port2_w )
{
	/* write latch */
	if ((m_port2 & 0x01) && !(data & 0x01))
	{
		/* control or data port? */
		if (m_port2 & 0x04)
		{
			/* PSG 0 or 1? */
			if (m_port2 & 0x08)
				m_ay1->address_w(space, 0, m_port1);
			if (m_port2 & 0x10)
				m_ay2->address_w(space, 0, m_port1);
		}
		else
		{
			/* PSG 0 or 1? */
			if (m_port2 & 0x08)
				m_ay1->data_w(space, 0, m_port1);
			if (m_port2 & 0x10)
				m_ay2->data_w(space, 0, m_port1);
		}
	}
	m_port2 = data;
}



/*************************************
 *
 *  6803 input ports ports
 *
 *************************************/

READ8_MEMBER( irem_audio_device::m6803_port1_r )
{
	/* PSG 0 or 1? */
	if (m_port2 & 0x08)
		return m_ay1->data_r(space, 0);
	if (m_port2 & 0x10)
		return m_ay2->data_r(space, 0);
	return 0xff;
}


READ8_MEMBER( irem_audio_device::m6803_port2_r )
{
	return 0;
}



/*************************************
 *
 *  AY-8910 output ports
 *
 *************************************/

WRITE8_MEMBER( irem_audio_device::ay8910_0_portb_w )
{
	/* bits 2-4 select MSM5205 clock & 3b/4b playback mode */
	m_adpcm1->playmode_w((data >> 2) & 7);
	if (m_adpcm2 != NULL)
		m_adpcm2->playmode_w(((data >> 2) & 4) | 3); /* always in slave mode */

	/* bits 0 and 1 reset the two chips */
	m_adpcm1->reset_w(data & 1);
	if (m_adpcm2 != NULL)
		m_adpcm2->reset_w(data & 2);
}


WRITE8_MEMBER( irem_audio_device::ay8910_1_porta_w )
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

WRITE8_MEMBER( irem_audio_device::sound_irq_ack_w )
{
	space.machine().device("iremsound")->execute().set_input_line(0, CLEAR_LINE);
}


WRITE8_MEMBER( irem_audio_device::m52_adpcm_w )
{
	if (offset & 1)
	{
		m_adpcm1->data_w(data);
	}
	if (offset & 2)
	{
		if (m_adpcm2 != NULL)
			m_adpcm2->data_w(data);
	}
}


WRITE8_MEMBER( irem_audio_device::m62_adpcm_w )
{
	msm5205_device *adpcm = (offset & 1) ? m_adpcm2 : m_adpcm1;
	if (adpcm != NULL)
		adpcm->data_w(data);
}



/*************************************
 *
 *  MSM5205 data ready signals
 *
 *************************************/

void irem_audio_device::adpcm_int(int st)
{
	machine().device("iremsound")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);

	/* the first MSM5205 clocks the second */
	if (m_adpcm2 != NULL)
	{
		m_adpcm2->vclk_w(1);
		m_adpcm2->vclk_w(0);
	}
}



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

/* All 6 (3*2) AY-3-8910 outputs are tied together
 * and put with 470 Ohm to gnd.
 * The following is a approximation, since
 * it does not take cross-chip mixing effects into account.
 */


/*
 * http://newsgroups.derkeiler.com/Archive/Rec/rec.games.video.arcade.collecting/2006-06/msg03108.html
 *
 * mentions, that moon patrol does work on moon ranger hardware.
 * There is no MSM5250, but a 74LS00 producing white noise for explosions
 */

/* Certain values are different from schematics
 * I did my test to verify against pcb pictures of "tropical angel"
 */

#define M52_R9      560
#define M52_R10     330
#define M52_R12     RES_K(10)
#define M52_R13     RES_K(10)
#define M52_R14     RES_K(10)
#define M52_R15     RES_K(2.2)  /* schematics RES_K(22) , althought 10-Yard states 2.2 */
#define M52_R19     RES_K(10)
#define M52_R22     RES_K(47)
#define M52_R23     RES_K(2.2)
#define M52_R25     RES_K(10)
#define M52_VR1     RES_K(50)

#define M52_C28     CAP_U(1)
#define M52_C30     CAP_U(0.022)
#define M52_C32     CAP_U(0.022)
#define M52_C35     CAP_U(47)
#define M52_C37     CAP_U(0.1)
#define M52_C38     CAP_U(0.0068)

/*
 * C35 is disabled, the mixer would just deliver
 * no signals if it is enabled.
 * TODO: Check discrete mixer
 *
 */

static const discrete_mixer_desc m52_sound_c_stage1 =
	{DISC_MIXER_IS_RESISTOR,
		{M52_R19, M52_R22, M52_R23 },
		{      0,       0,       0 },   /* variable resistors   */
		{M52_C37,       0,       0 },   /* node capacitors      */
				0,      0,              /* rI, rF               */
		M52_C35*0,                      /* cF                   */
		0,                              /* cAmp                 */
		0, 1};

static const discrete_op_amp_filt_info m52_sound_c_sallen_key =
	{ M52_R13, M52_R14, 0, 0, 0,
		M52_C32, M52_C38, 0
	};

static const discrete_mixer_desc m52_sound_c_mix1 =
	{DISC_MIXER_IS_RESISTOR,
		{M52_R25, M52_R15 },
		{      0,       0 },    /* variable resistors   */
		{      0,       0 },    /* node capacitors      */
				0, M52_VR1,     /* rI, rF               */
		0,                      /* cF                   */
		CAP_U(1),               /* cAmp                 */
		0, 1};

static DISCRETE_SOUND_START( m52_sound_c )

	/* Chip AY8910/1 */
	DISCRETE_INPUTX_STREAM(NODE_01, 0, 1.0, 0)
	/* Chip AY8910/2 */
	DISCRETE_INPUTX_STREAM(NODE_02, 1, 1.0, 0)
	/* Chip MSM5250 */
	DISCRETE_INPUTX_STREAM(NODE_03, 2, 1.0, 0)

	/* Just mix the two AY8910s */
	DISCRETE_ADDER2(NODE_09, 1, NODE_01, NODE_02)
	DISCRETE_DIVIDE(NODE_10, 1, NODE_09, 2.0)

	/* Mix in 5 V to MSM5250 signal */
	DISCRETE_MIXER3(NODE_20, 1, NODE_03, 32767.0, 0, &m52_sound_c_stage1)

	/* Sallen - Key Filter */
	/* TODO: R12, C30: This looks like a band pass */
	DISCRETE_RCFILTER(NODE_25, NODE_20, M52_R12, M52_C30)
	DISCRETE_SALLEN_KEY_FILTER(NODE_30, 1, NODE_25, DISC_SALLEN_KEY_LOW_PASS, &m52_sound_c_sallen_key)

	/* Mix signals */
	DISCRETE_MIXER2(NODE_40, 1, NODE_10, NODE_25, &m52_sound_c_mix1)
	DISCRETE_CRFILTER(NODE_45, NODE_40, M52_R10+M52_R9, M52_C28)

	DISCRETE_OUTPUT(NODE_40, 18.0)

DISCRETE_SOUND_END

/*************************************
 *
 *  Address maps
 *
 *************************************/

/* complete address map verified from Moon Patrol/10 Yard Fight schematics */
/* large map uses 8k ROMs, small map uses 4k ROMs; this is selected via a jumper */
static ADDRESS_MAP_START( m52_small_sound_map, AS_PROGRAM, 8, driver_device )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x0fff) AM_DEVWRITE("irem_audio", irem_audio_device, m52_adpcm_w)
	AM_RANGE(0x1000, 0x1fff) AM_DEVWRITE("irem_audio", irem_audio_device, sound_irq_ack_w)
	AM_RANGE(0x2000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( m52_large_sound_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_DEVWRITE("irem_audio", irem_audio_device, m52_adpcm_w)
	AM_RANGE(0x2000, 0x3fff) AM_DEVWRITE("irem_audio", irem_audio_device, sound_irq_ack_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


/* complete address map verified from Kid Niki schematics */
static ADDRESS_MAP_START( m62_sound_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0800, 0x0800) AM_MIRROR(0xf7fc) AM_DEVWRITE("irem_audio", irem_audio_device, sound_irq_ack_w)
	AM_RANGE(0x0801, 0x0802) AM_MIRROR(0xf7fc) AM_DEVWRITE("irem_audio", irem_audio_device, m62_adpcm_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( irem_sound_portmap, AS_IO, 8, driver_device )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_DEVREADWRITE("irem_audio", irem_audio_device, m6803_port1_r, m6803_port1_w)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_DEVREADWRITE("irem_audio", irem_audio_device, m6803_port2_r, m6803_port2_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_FRAGMENT( irem_audio_base )

	/* basic machine hardware */
	MCFG_CPU_ADD("iremsound", M6803, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_CPU_IO_MAP(irem_sound_portmap)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("irem_audio", IREM_AUDIO, 0)

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_3_579545MHz/4) /* verified on pcb */
	MCFG_AY8910_OUTPUT_TYPE(AY8910_SINGLE_OUTPUT | AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(470, 0, 0)
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_AY8910_PORT_B_WRITE_CB(DEVWRITE8("irem_audio", irem_audio_device, ay8910_0_portb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_3_579545MHz/4) /* verified on pcb */
	MCFG_AY8910_OUTPUT_TYPE(AY8910_SINGLE_OUTPUT | AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(470, 0, 0)
	MCFG_AY8910_PORT_A_WRITE_CB(DEVWRITE8("irem_audio", irem_audio_device, ay8910_1_porta_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("msm1", MSM5205, XTAL_384kHz) /* verified on pcb */
	MCFG_MSM5205_VCLK_CB(DEVWRITELINE("irem_audio", irem_audio_device, adpcm_int))          /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S96_4B)      /* default to 4KHz, but can be changed at run time */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("msm2", MSM5205, XTAL_384kHz) /* verified on pcb */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_SEX_4B)      /* default to 4KHz, but can be changed at run time, slave */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( m52_sound_c_audio )

	/* basic machine hardware */
	MCFG_CPU_ADD("iremsound", M6803, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_CPU_IO_MAP(irem_sound_portmap)
	MCFG_CPU_PROGRAM_MAP(m52_small_sound_map)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("irem_audio", IREM_AUDIO, 0)

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_3_579545MHz/4) /* verified on pcb */
	MCFG_AY8910_OUTPUT_TYPE(AY8910_SINGLE_OUTPUT | AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(470, 0, 0)
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_AY8910_PORT_B_WRITE_CB(DEVWRITE8("irem_audio", irem_audio_device, ay8910_0_portb_w))
	MCFG_SOUND_ROUTE_EX(0, "filtermix", 1.0, 0)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_3_579545MHz/4) /* verified on pcb */
	MCFG_AY8910_OUTPUT_TYPE(AY8910_SINGLE_OUTPUT | AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(470, 0, 0)
	MCFG_AY8910_PORT_A_WRITE_CB(DEVWRITE8("irem_audio", irem_audio_device, ay8910_1_porta_w))
	MCFG_SOUND_ROUTE_EX(0, "filtermix", 1.0, 1)

	MCFG_SOUND_ADD("msm1", MSM5205, XTAL_384kHz) /* verified on pcb */
	MCFG_MSM5205_VCLK_CB(DEVWRITELINE("irem_audio", irem_audio_device, adpcm_int))          /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S96_4B)      /* default to 4KHz, but can be changed at run time */
	MCFG_SOUND_ROUTE_EX(0, "filtermix", 1.0, 2)

	MCFG_SOUND_ADD("filtermix", DISCRETE, 0)
	MCFG_DISCRETE_INTF(m52_sound_c)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( m52_large_audio, irem_audio_base )  /* 10 yard fight */

	/* basic machine hardware */
	MCFG_CPU_MODIFY("iremsound")
	MCFG_CPU_PROGRAM_MAP(m52_large_sound_map)
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( m62_audio, irem_audio_base )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("iremsound")
	MCFG_CPU_PROGRAM_MAP(m62_sound_map)
MACHINE_CONFIG_END

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void irem_audio_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
}
