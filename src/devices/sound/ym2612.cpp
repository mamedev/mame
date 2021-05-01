// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ym2612.h"


// the YM2612/YM3438 just timeslice the output among all channels
// instead of summing them; turn this on to simulate (may create
// audible issues)
#define MULTIPLEX_OUTPUT (0)


DEFINE_DEVICE_TYPE(YM2612, ym2612_device, "ym2612", "YM2612 OPN2")
DEFINE_DEVICE_TYPE(YM3438, ym3438_device, "ym3438", "YM3438 OPN2C")
DEFINE_DEVICE_TYPE(YMF276, ymf276_device, "ymf276", "YMF276 OPN2L")


//*********************************************************
//  YM2612 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2612_device - constructor
//-------------------------------------------------

ym2612_device::ym2612_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_fm(*this),
	m_stream(nullptr),
	m_busy_duration(m_fm.compute_busy_duration()),
	m_address(0),
	m_dac_data(0),
	m_dac_enable(0),
	m_channel(0)
{
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

u8 ym2612_device::read(offs_t offset)
{
	u8 result = 0;
	switch (offset & 3)
	{
		case 0: // status port, YM2203 compatible
			result = m_fm.status();
			break;

		case 1: // data port (unused)
		case 2: // status port, extended
		case 3: // data port (unused)
			logerror("Unexpected read from YM2612 offset %d\n", offset & 3);
			break;
	}
	return result;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2612_device::write(offs_t offset, u8 value)
{
	switch (offset & 3)
	{
		case 0: // address port
			m_address = value;
			break;

		case 1: // data port

			// ignore if paired with upper address
			if (BIT(m_address, 8))
				break;

			// force an update
			m_stream->update();

			if (m_address == 0x2a)
			{
				// DAC data
				m_dac_data = (m_dac_data & ~0x1fe) | ((value ^ 0x80) << 1);
			}
			else if (m_address == 0x2b)
			{
				// DAC enable
				m_dac_enable = BIT(value, 7);
			}
			else if (m_address == 0x2c)
			{
				// test/low DAC bit
				m_dac_data = (m_dac_data & ~1) | BIT(value, 3);
			}
			else
			{
				// write to FM
				m_fm.write(m_address, value);
			}

			// mark busy for a bit
			m_fm.set_busy_end(machine().time() + m_busy_duration);
			break;

		case 2: // upper address port
			m_address = 0x100 | value;
			break;

		case 3: // upper data port

			// ignore if paired with lower address
			if (!BIT(m_address, 8))
				break;

			// write to FM
			m_stream->update();
			m_fm.write(m_address, value);

			// mark busy for a bit
			m_fm.set_busy_end(machine().time() + m_busy_duration);
			break;
	}
}


//-------------------------------------------------
//  device_start - start of emulation
//-------------------------------------------------

void ym2612_device::device_start()
{
	// create our stream
	m_stream = stream_alloc(0, fm_engine::OUTPUTS, m_fm.sample_rate(clock()));

	// call this for the variants that need to adjust the rate
	device_clock_changed();

	// save our data
	save_item(YMFM_NAME(m_address));
	save_item(YMFM_NAME(m_dac_data));
	save_item(YMFM_NAME(m_dac_enable));
	save_item(YMFM_NAME(m_channel));

	// save the engines
	m_fm.save(*this);
}


//-------------------------------------------------
//  device_reset - start of emulation
//-------------------------------------------------

void ym2612_device::device_reset()
{
	// reset the engines
	m_fm.reset();

	// reset our internal state
	m_dac_enable = 0;
	m_channel = 0;
}


//-------------------------------------------------
//  device_clock_changed - update if clock changes
//-------------------------------------------------

void ym2612_device::device_clock_changed()
{
	u32 const sample_divider = MULTIPLEX_OUTPUT ? fm_engine::CHANNELS : 1;
	m_stream->set_sample_rate(m_fm.sample_rate(clock()) * sample_divider);

	// recompute the busy duration
	m_busy_duration = m_fm.compute_busy_duration();
}


//-------------------------------------------------
//  sound_stream_update - update the sound stream
//-------------------------------------------------

void ym2612_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	sound_stream_update_common(outputs[0], outputs[1], true);
}


//-------------------------------------------------
//  sound_stream_update_common - shared stream
//  update function among subclasses
//-------------------------------------------------

void ym2612_device::sound_stream_update_common(write_stream_view &outl, write_stream_view &outr, bool discontinuity)
{
	u32 const sample_divider = (discontinuity ? 260 : 256) * (MULTIPLEX_OUTPUT ? 1 : fm_engine::CHANNELS);

	// iterate over all target samples
	s32 sums[2] = { 0 };
	for (int sampindex = 0; sampindex < outl.samples(); )
	{
		// clock the FM when we hit channel 0
		if (m_channel == 0)
			m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the current FM channel; YM2612 is 9-bit with intermediate clipping
		s32 outputs[2] = { 0 };
		if (m_channel != 5 || !m_dac_enable)
			m_fm.output(outputs, 5, 256, 1 << m_channel);
		else
			outputs[0] = outputs[1] = s16(m_dac_data << 7) >> 7;

		// hiccup in the internal YM2612 DAC means that there is a rather large
		// step between 0 and -1 (close to 6x the normal step); the approximation
		// below gives a reasonable estimation of this discontinuity, which was
		// fixed in the YM3438
		if (discontinuity)
		{
			if (outputs[0] < 0)
				outputs[0] -= 2;
			else
				outputs[0] += 3;
			if (outputs[1] < 0)
				outputs[1] -= 2;
			else
				outputs[1] += 3;
		}

		// if multiplexing, just scale to 16 bits and output
		if (MULTIPLEX_OUTPUT)
		{
			outl.put_int(sampindex, outputs[0], sample_divider);
			outr.put_int(sampindex, outputs[1], sample_divider);
			sampindex++;
		}

		// if not, accumulate the sums
		else
		{
			sums[0] += outputs[0];
			sums[1] += outputs[1];

			// on the last channel, output the average and reset the sums
			if (m_channel == 5)
			{
				outl.put_int(sampindex, sums[0], sample_divider);
				outr.put_int(sampindex, sums[1], sample_divider);
				sampindex++;
				sums[0] = sums[1] = 0;
			}
		}

		// advance to the next channel
		m_channel++;
		if (m_channel >= 6)
			m_channel = 0;
	}
}



//*********************************************************
//  YM3438 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym3438_device - constructor
//-------------------------------------------------

ym3438_device::ym3438_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ym2612_device(mconfig, tag, owner, clock, YM3438)
{
}


//-------------------------------------------------
//  sound_stream_update - update the sound stream
//-------------------------------------------------

void ym3438_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	sound_stream_update_common(outputs[0], outputs[1], false);
}



//*********************************************************
//  YMF276 DEVICE
//*********************************************************

//-------------------------------------------------
//  ymf276_device - constructor
//-------------------------------------------------

ymf276_device::ymf276_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ym2612_device(mconfig, tag, owner, clock, YMF276)
{
}


//-------------------------------------------------
//  device_clock_changed - update if clock changes
//-------------------------------------------------

void ymf276_device::device_clock_changed()
{
	m_stream->set_sample_rate(m_fm.sample_rate(clock()));
}


//-------------------------------------------------
//  sound_stream_update - update the sound stream
//-------------------------------------------------

void ymf276_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	// mask off channel 6 if DAC is enabled
	u8 const fm_mask = m_dac_enable ? 0x1f : 0x3f;

	// iterate over all target samples
	for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
	{
		// clock the FM
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM content; YMF276 is 14-bit with intermediate clipping
		s32 sums[fm_engine::OUTPUTS] = { 0 };
		m_fm.output(sums, 0, 8191, fm_mask);

		// shifted down 1 bit after mixer
		for (int index = 0; index < fm_engine::OUTPUTS; index++)
			sums[index] >>= 1;

		// add in DAC if enabled
		if (m_dac_enable)
			for (int index = 0; index < fm_engine::OUTPUTS; index++)
				sums[index] += s16(m_dac_data << 7) >> 3;

		// YMF3438 is stereo
		for (int index = 0; index < fm_engine::OUTPUTS; index++)
			outputs[index].put_int_clamp(sampindex, sums[0], 32768);
	}
}

