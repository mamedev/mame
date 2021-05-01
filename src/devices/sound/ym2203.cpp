// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ym2203.h"


DEFINE_DEVICE_TYPE(YM2203, ym2203_device, "ym2203", "YM2203 OPN")


//*********************************************************
//  YM2203 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2203_device - constructor
//-------------------------------------------------

ym2203_device::ym2203_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ay8910_device(mconfig, YM2203, tag, owner, clock, PSG_TYPE_YM, 3, 2),
	m_fm(*this),
	m_stream(nullptr),
	m_busy_duration(m_fm.compute_busy_duration()),
	m_address(0)
{
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

u8 ym2203_device::read(offs_t offset)
{
	u8 result = 0;
	switch (offset & 1)
	{
		case 0: // status port
			result = m_fm.status();
			break;

		case 1: // data port (only SSG)
			if (m_address < 0x10)
				result = ay8910_read_ym();
			break;
	}
	return result;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2203_device::write(offs_t offset, u8 value)
{
	switch (offset & 1)
	{
		case 0: // address port
			m_address = value;
			if (m_address < 0x10)
			{
				// write register to SSG
				ay8910_write_ym(0, m_address);
			}
			else if (m_address >= 0x2d && m_address <= 0x2f)
			{
				// prescaler select : 2d,2e,2f
				if (m_address == 0x2d)
					update_prescale(6);
				else if (m_address == 0x2e && m_fm.clock_prescale() == 6)
					update_prescale(3);
				else if (m_address == 0x2f)
					update_prescale(2);
			}
			break;

		case 1: // data port
			if (m_address < 0x10)
			{
				// write to SSG
				ay8910_write_ym(1, value);
			}
			else
			{
				// write to FM
				m_stream->update();
				m_fm.write(m_address, value);
			}

			// mark busy for a bit
			m_fm.set_busy_end(machine().time() + m_busy_duration);
			break;
	}
}


//-------------------------------------------------
//  device_start - start of emulation
//-------------------------------------------------

void ym2203_device::device_start()
{
	// start the SSG device
	ay8910_device::device_start();

	// create our stream
	m_stream = stream_alloc(0, fm_engine::OUTPUTS, m_fm.sample_rate(clock()));

	// save our data
	save_item(YMFM_NAME(m_address));

	// save the FM engine
	m_fm.save(*this);
}


//-------------------------------------------------
//  device_reset - start of emulation
//-------------------------------------------------

void ym2203_device::device_reset()
{
	// reset the SSG device
	ay8910_device::device_reset();

	// reset the FM engine
	m_fm.reset();
}


//-------------------------------------------------
//  device_clock_changed - update if clock changes
//-------------------------------------------------

void ym2203_device::device_clock_changed()
{
	// refresh via prescale
	update_prescale(m_fm.clock_prescale());
}


//-------------------------------------------------
//  sound_stream_update - update the sound stream
//-------------------------------------------------

void ym2203_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	// if this is not our stream, pass it on
	if (&stream != m_stream)
	{
		ay8910_device::sound_stream_update(stream, inputs, outputs);
		return;
	}

	// iterate over all target samples
	for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM content; YM2203 is full 14-bit with no intermediate clipping
		s32 sums[fm_engine::OUTPUTS] = { 0 };
		m_fm.output(sums, 0, 32767, fm_engine::ALL_CHANNELS);

		// convert to 10.3 floating point value for the DAC and back
		// YM2203 is mono
		for (int index = 0; index < fm_engine::OUTPUTS; index++)
			outputs[index].put_int(sampindex, ymfm_roundtrip_fp(sums[index]), 32768);
	}
}


//-------------------------------------------------
//  update_prescale - set a new prescale value and
//  update clocks as needed
//-------------------------------------------------

void ym2203_device::update_prescale(u8 newval)
{
	// inform the FM engine and refresh our clock rate
	m_fm.set_clock_prescale(newval);
	m_stream->set_sample_rate(m_fm.sample_rate(clock()));
	logerror("Prescale = %d; sample_rate = %d\n", newval, m_fm.sample_rate(clock()));

	// also scale the SSG streams
	// mapping is (FM->SSG): 6->4, 3->2, 2->1
	u8 ssg_scale = 2 * newval / 3;
	// QUESTION: where does the *2 come from??
	ay_set_clock(clock() * 2 / ssg_scale);

	// recompute the busy duration
	m_busy_duration = m_fm.compute_busy_duration();
}
