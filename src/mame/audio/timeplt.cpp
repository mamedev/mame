// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    This code is used by the following module:

    timeplt.c
    pooyan.c
    rallyx.c (for locomotn)
    tutankhm.c
    rocnrope.c

***************************************************************************/

#include "emu.h"
#include "audio/timeplt.h"


#define MASTER_CLOCK         XTAL_14_31818MHz


const device_type TIMEPLT_AUDIO = &device_creator<timeplt_audio_device>;

timeplt_audio_device::timeplt_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TIMEPLT_AUDIO, "Time Pilot Audio", tag, owner, clock, "timeplt_audio", __FILE__),
		device_sound_interface(mconfig, *this),
		m_last_irq_state(0)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void timeplt_audio_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void timeplt_audio_device::device_start()
{
	m_soundcpu = machine().device<cpu_device>("tpsound");
	m_filter_0_0 = machine().device("filter.0.0");
	m_filter_0_1 = machine().device("filter.0.1");
	m_filter_0_2 = machine().device("filter.0.2");
	m_filter_1_0 = machine().device("filter.1.0");
	m_filter_1_1 = machine().device("filter.1.1");
	m_filter_1_2 = machine().device("filter.1.2");

	m_last_irq_state = 0;
	save_item(NAME(m_last_irq_state));
}


/*************************************
 *
 *  Sound timer
 *
 *************************************/

/* The timer clock which feeds the upper 4 bits of                      */
/* AY-3-8910 port A is based on the same clock                          */
/* feeding the sound CPU Z80.  It is a divide by                        */
/* 5120, formed by a standard divide by 512,                            */
/* followed by a divide by 10 using a 4 bit                             */
/* bi-quinary count sequence. (See LS90 data sheet                      */
/* for an example).                                                     */
/*                                                                      */
/* Bit 4 comes from the output of the divide by 1024                    */
/*       0, 1, 0, 1, 0, 1, 0, 1, 0, 1                                   */
/* Bit 5 comes from the QC output of the LS90 producing a sequence of   */
/*       0, 0, 1, 1, 0, 0, 1, 1, 1, 0                                   */
/* Bit 6 comes from the QD output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 1, 0, 0, 0, 0, 1                                   */
/* Bit 7 comes from the QA output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 0, 1, 1, 1, 1, 1                                   */

READ8_MEMBER( timeplt_audio_device::portB_r )
{
	static const int timeplt_timer[10] =
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x90, 0xa0, 0xb0, 0xa0, 0xd0
	};

	return timeplt_timer[(m_soundcpu->total_cycles() / 512) % 10];
}



/*************************************
 *
 *  Filter controls
 *
 *************************************/

void timeplt_audio_device::filter_w( device_t *device, int data )
{
	int C = 0;

	if (data & 1)
		C += 220000;    /* 220000pF = 0.220uF */
	if (data & 2)
		C +=  47000;    /*  47000pF = 0.047uF */

	dynamic_cast<filter_rc_device*>(device)->filter_rc_set_RC(FLT_RC_LOWPASS, 1000, 5100, 0, CAP_P(C));
}


WRITE8_MEMBER( timeplt_audio_device::filter_w )
{
	filter_w(m_filter_1_0, (offset >>  0) & 3);
	filter_w(m_filter_1_1, (offset >>  2) & 3);
	filter_w(m_filter_1_2, (offset >>  4) & 3);
	filter_w(m_filter_0_0, (offset >>  6) & 3);
	filter_w(m_filter_0_1, (offset >>  8) & 3);
	filter_w(m_filter_0_2, (offset >> 10) & 3);
}



/*************************************
 *
 *  External interfaces
 *
 *************************************/

WRITE8_MEMBER( timeplt_audio_device::sh_irqtrigger_w )
{
	if (m_last_irq_state == 0 && data)
	{
		/* setting bit 0 low then high triggers IRQ on the sound CPU */
		m_soundcpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
	}

	m_last_irq_state = data;
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( timeplt_sound_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x33ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x0fff) AM_DEVREADWRITE("ay1", ay8910_device, data_r, data_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0x0fff) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x0fff) AM_DEVREADWRITE("ay2", ay8910_device, data_r, data_w)
	AM_RANGE(0x7000, 0x7000) AM_MIRROR(0x0fff) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0x8000, 0xffff) AM_DEVWRITE("timeplt_audio", timeplt_audio_device, filter_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( locomotn_sound_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x3000, 0x3fff) AM_DEVWRITE("timeplt_audio", timeplt_audio_device, filter_w)
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x0fff) AM_DEVREADWRITE("ay1", ay8910_device, data_r, data_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0x0fff) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x0fff) AM_DEVREADWRITE("ay2", ay8910_device, data_r, data_w)
	AM_RANGE(0x7000, 0x7000) AM_MIRROR(0x0fff) AM_DEVWRITE("ay2", ay8910_device, address_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( timeplt_sound )

	/* basic machine hardware */
	MCFG_CPU_ADD("tpsound",Z80,MASTER_CLOCK/8)
	MCFG_CPU_PROGRAM_MAP(timeplt_sound_map)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("timeplt_audio", TIMEPLT_AUDIO, 0)

	MCFG_SOUND_ADD("ay1", AY8910, MASTER_CLOCK/8)
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_AY8910_PORT_B_READ_CB(DEVREAD8("timeplt_audio", timeplt_audio_device, portB_r))
	MCFG_SOUND_ROUTE(0, "filter.0.0", 0.60)
	MCFG_SOUND_ROUTE(1, "filter.0.1", 0.60)
	MCFG_SOUND_ROUTE(2, "filter.0.2", 0.60)

	MCFG_SOUND_ADD("ay2", AY8910, MASTER_CLOCK/8)
	MCFG_SOUND_ROUTE(0, "filter.1.0", 0.60)
	MCFG_SOUND_ROUTE(1, "filter.1.1", 0.60)
	MCFG_SOUND_ROUTE(2, "filter.1.2", 0.60)

	MCFG_FILTER_RC_ADD("filter.0.0", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_FILTER_RC_ADD("filter.0.1", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_FILTER_RC_ADD("filter.0.2", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_FILTER_RC_ADD("filter.1.0", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_FILTER_RC_ADD("filter.1.1", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_FILTER_RC_ADD("filter.1.2", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( locomotn_sound, timeplt_sound )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("tpsound")
	MCFG_CPU_PROGRAM_MAP(locomotn_sound_map)
MACHINE_CONFIG_END

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void timeplt_audio_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}
