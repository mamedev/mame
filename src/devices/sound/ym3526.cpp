// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ym3526.h"


DEFINE_DEVICE_TYPE(YM3526, ym3526_device, "ym3526", "YM3526 OPL")


//*********************************************************
//  YM3526 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym3526_device - constructor
//-------------------------------------------------

ym3526_device::ym3526_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_address(0),
	m_stream(nullptr),
	m_fm(*this)
{
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

u8 ym3526_device::read(offs_t offset)
{
	u8 result = 0xff;
	switch (offset & 1)
	{
		case 0: // status port
			result = m_fm.status() | 0x06;
			break;

		case 1:	// data port (unused)
			logerror("Unexpected read from YM3526 offset %d\n", offset & 3);
			break;
	}
	return result;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym3526_device::write(offs_t offset, u8 value)
{
	switch (offset & 1)
	{
		case 0:	// address port
			m_address = value;
			break;

		case 1: // data port

			// force an update
			m_stream->update();

			// write to FM
//			if (m_address == 4)
//				printf("Control = %02X\n", value);
			m_fm.write(m_address, value);
			break;
	}
}


//-------------------------------------------------
//  device_start - start of emulation
//-------------------------------------------------

void ym3526_device::device_start()
{
	// create our stream
	m_stream = stream_alloc(0, fm_engine::OUTPUTS, m_fm.sample_rate(clock()));

	// call this for the variants that need to adjust the rate
	device_clock_changed();

	// save our data
	save_item(YMFM_NAME(m_address));

	// save the engines
	m_fm.save(*this);
}


//-------------------------------------------------
//  device_reset - start of emulation
//-------------------------------------------------

void ym3526_device::device_reset()
{
	// reset the engines
	m_fm.reset();
}


//-------------------------------------------------
//  device_clock_changed - update if clock changes
//-------------------------------------------------

void ym3526_device::device_clock_changed()
{
	m_stream->set_sample_rate(m_fm.sample_rate(clock()));
}


//-------------------------------------------------
//  sound_stream_update - update the sound stream
//-------------------------------------------------

void ym3526_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	// iterate over all target samples
	for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM content; clipping is unknown
		s32 sums[fm_engine::OUTPUTS] = { 0 };
		m_fm.output(sums, 1, 32767, fm_engine::ALL_CHANNELS);

		// convert to 10.3 floating point value for the DAC and back
		// YM3526 is mono
		for (int index = 0; index < fm_engine::OUTPUTS; index++)
			outputs[index].put_int(sampindex, ymfm_roundtrip_fp(sums[index]), 32768);
	}
}
