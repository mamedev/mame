// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    National Semiconductor MM5837

    Digital Noise Source

***************************************************************************/

#include "emu.h"
#include "mm5837.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MM5837, mm5837_device, "mm5837", "MM5837 Digital Noise Source")
DEFINE_DEVICE_TYPE(MM5837_STREAM, mm5837_stream_device, "mm5837_stream", "MM5837 Digital Noise Stream")


//**************************************************************************
//  MM5837 DEVICE
//**************************************************************************

//-------------------------------------------------
//  mm5837_device - constructor
//-------------------------------------------------

mm5837_device::mm5837_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MM5837, tag, owner, clock),
	m_output_cb(*this),
	m_timer(nullptr),
	m_vdd(-12)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm5837_device::device_start()
{
	// get timer
	m_timer = timer_alloc(FUNC(mm5837_device::update_clock_output), this);

	// register for save states
	save_item(NAME(m_source.m_shift));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mm5837_device::device_reset()
{
	m_source.reset();

	if (m_vdd < 16)
		m_timer->adjust(attotime::zero, 0, attotime::from_hz(mm5837_source::frequency(m_vdd)));
	else
		throw emu_fatalerror("%s: Invalid voltage: %d\n", tag(), m_vdd);
}


//-------------------------------------------------
//  update_clock_output -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mm5837_device::update_clock_output)
{
	m_output_cb(m_source.clock());
}



//**************************************************************************
//  MM5837 STREAM DEVICE
//**************************************************************************

//-------------------------------------------------
//  mm5837_stream_device - constructor
//-------------------------------------------------

mm5837_stream_device::mm5837_stream_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MM5837_STREAM, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr),
	m_vdd(-12)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm5837_stream_device::device_start()
{
	m_stream = stream_alloc(0, 1, mm5837_source::frequency(m_vdd));
	save_item(NAME(m_source.m_shift));
}


//-------------------------------------------------
//  sound_stream_update - fill the sound buffer
//-------------------------------------------------

void mm5837_stream_device::sound_stream_update(sound_stream &stream)
{
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
		stream.put(0, sampindex, m_source.clock() ? 1.0 : 0.0);
}
