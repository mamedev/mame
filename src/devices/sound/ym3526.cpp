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
//  status_r - return the status port (A0=0)
//-------------------------------------------------

u8 ym3526_device::status_r()
{
	return m_fm.status() | 0x06;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

u8 ym3526_device::read(offs_t offset)
{
	// datasheet says status only reads when A0=0
	if ((offset & 1) == 0)
		return status_r();

	// when A0=1 datasheet says "the data on the bus are not guaranteed"
	logerror("Unexpected read from YM3526 offset %d\n", offset & 1);
	return 0xff;
}


//-------------------------------------------------
//  address_w - write to the address port (A0=0)
//-------------------------------------------------

void ym3526_device::address_w(u8 value)
{
	m_address = value;
}


//-------------------------------------------------
//  data_w - write to the data port (A0=1)
//-------------------------------------------------

void ym3526_device::data_w(u8 value)
{
	// force an update
	m_stream->update();

	// write to FM
	m_fm.write(m_address, value);
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym3526_device::write(offs_t offset, u8 value)
{
	// A0 selects between address/data
	if ((offset & 1) == 0)
		address_w(value);
	else
		data_w(value);
}


//-------------------------------------------------
//  device_start - start of emulation
//-------------------------------------------------

void ym3526_device::device_start()
{
	// create our stream
	m_stream = stream_alloc(0, fm_engine::OUTPUTS, m_fm.sample_rate(clock()));

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
