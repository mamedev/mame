// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ymf262.h"


DEFINE_DEVICE_TYPE(YMF262, ymf262_device, "ymf262", "YMF262 OPL3")


//*********************************************************
//  YMF262 DEVICE
//*********************************************************

//-------------------------------------------------
//  ymf262_device - constructor
//-------------------------------------------------

ymf262_device::ymf262_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
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

u8 ymf262_device::read(offs_t offset)
{
	u8 result = 0x00;
	switch (offset & 3)
	{
		case 0:     // status port (A0=0, A1=0)
			result = m_fm.status();
			break;

		default:    // datasheet says anything else is not guaranteed
			logerror("Unexpected read from YMF262 offset %d\n", offset & 3);
			break;
	}
	return result;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ymf262_device::write(offs_t offset, u8 value)
{
	switch (offset & 1)
	{
		case 0: // address ports - A1 references upper bank
			m_address = value | (BIT(offset, 1) << 8);

			// tests reveal that in compatibility mode, upper bit is masked
			// except for register 0x105
			if (m_fm.regs().newflag() == 0 && m_address != 0x105)
				m_address &= 0xff;
			break;

		case 1: // data ports (A1 is ignored)

			// force an update
			m_stream->update();

			// write to FM
			m_fm.write(m_address, value);
			break;
	}
}


//-------------------------------------------------
//  device_start - start of emulation
//-------------------------------------------------

void ymf262_device::device_start()
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

void ymf262_device::device_reset()
{
	// reset the engines
	m_fm.reset();
}


//-------------------------------------------------
//  device_clock_changed - update if clock changes
//-------------------------------------------------

void ymf262_device::device_clock_changed()
{
	m_stream->set_sample_rate(m_fm.sample_rate(clock()));
}


//-------------------------------------------------
//  sound_stream_update - update the sound stream
//-------------------------------------------------

void ymf262_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	// iterate over all target samples
	for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM content; clipping is unknown
		s32 sums[fm_engine::OUTPUTS] = { 0 };
		m_fm.output(sums, 0, 32767, fm_engine::ALL_CHANNELS);

		// YMF262 outputs straight 16-bit data in 4 channels
		for (int index = 0; index < fm_engine::OUTPUTS; index++)
			outputs[index].put_int(sampindex, sums[index], 32768);
	}
}
