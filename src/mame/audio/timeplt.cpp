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
#include "includes/timeplt.h"
#include "audio/timeplt.h"

#include "machine/gen_latch.h"
#include "speaker.h"


#define MASTER_CLOCK         XTAL(14'318'181)


DEFINE_DEVICE_TYPE(TIMEPLT_AUDIO, timeplt_audio_device, "timplt_audio", "Time Pilot Audio")

timeplt_audio_device::timeplt_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TIMEPLT_AUDIO, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_soundcpu(*this, "tpsound")
	, m_soundlatch(*this, "soundlatch")
	, m_filter_0(*this, "filter.0.%u", 0)
	, m_filter_1(*this, "filter.1.%u", 0)
	, m_last_irq_state(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void timeplt_audio_device::device_start()
{
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

void timeplt_audio_device::filter_w(filter_rc_device &device, int data)
{
	int C = 0;

	if (data & 1)
		C += 220000;    /* 220000pF = 0.220uF */
	if (data & 2)
		C +=  47000;    /*  47000pF = 0.047uF */

	device.filter_rc_set_RC(filter_rc_device::LOWPASS, 1000, 5100, 0, CAP_P(C));
}


WRITE8_MEMBER( timeplt_audio_device::filter_w )
{
	filter_w(*m_filter_1[0], (offset >>  0) & 3);
	filter_w(*m_filter_1[1], (offset >>  2) & 3);
	filter_w(*m_filter_1[2], (offset >>  4) & 3);
	filter_w(*m_filter_0[0], (offset >>  6) & 3);
	filter_w(*m_filter_0[1], (offset >>  8) & 3);
	filter_w(*m_filter_0[2], (offset >> 10) & 3);
}



/*************************************
 *
 *  External interfaces
 *
 *************************************/

WRITE8_MEMBER(timeplt_audio_device::sound_data_w)
{
	m_soundlatch->write(space, 0, data);
}


WRITE_LINE_MEMBER(timeplt_audio_device::sh_irqtrigger_w)
{
	if (m_last_irq_state == 0 && state)
	{
		/* setting bit 0 low then high triggers IRQ on the sound CPU */
		m_soundcpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
	}

	m_last_irq_state = state;
}


WRITE_LINE_MEMBER(timeplt_audio_device::mute_w)
{
	// controls pin 6 (DC audio mute) of LA4460 amplifier
	machine().sound().system_mute(state);
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

ADDRESS_MAP_START(timeplt_audio_device::timeplt_sound_map)
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x33ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x0fff) AM_DEVREADWRITE("ay1", ay8910_device, data_r, data_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0x0fff) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x0fff) AM_DEVREADWRITE("ay2", ay8910_device, data_r, data_w)
	AM_RANGE(0x7000, 0x7000) AM_MIRROR(0x0fff) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0x8000, 0xffff) AM_WRITE(filter_w)
ADDRESS_MAP_END


ADDRESS_MAP_START(timeplt_audio_device::locomotn_sound_map)
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x3000, 0x3fff) AM_WRITE(filter_w)
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

MACHINE_CONFIG_START(timeplt_audio_device::timeplt_sound)

	/* basic machine hardware */
	MCFG_CPU_ADD("tpsound",Z80,MASTER_CLOCK/8)
	MCFG_CPU_PROGRAM_MAP(timeplt_sound_map)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_GENERIC_LATCH_8_ADD("soundlatch")

	MCFG_SOUND_ADD("ay1", AY8910, MASTER_CLOCK/8)
	MCFG_AY8910_PORT_A_READ_CB(DEVREAD8("soundlatch", generic_latch_8_device, read))
	MCFG_AY8910_PORT_B_READ_CB(READ8(timeplt_audio_device, portB_r))
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


MACHINE_CONFIG_START(timeplt_audio_device::locomotn_sound)
	timeplt_sound(config);

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
