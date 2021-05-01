// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ym2610.h"


DEFINE_DEVICE_TYPE(YM2610, ym2610_device, "ym2610", "YM2610 OPNB")
DEFINE_DEVICE_TYPE(YM2610B, ym2610b_device, "ym2610b", "YM2610B OPNB2")


//*********************************************************
//  YM2610/YM2610B DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2610_device - constructor
//-------------------------------------------------

ym2610_device::ym2610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type, u8 fm_mask) :
	ay8910_device(mconfig, type, tag, owner, clock, PSG_TYPE_YM, 1, 0),
	device_memory_interface(mconfig, *this),
	m_adpcm_a_config("adpcm-a", ENDIANNESS_LITTLE, 8, 24, 0),
	m_adpcm_b_config("adpcm-b", ENDIANNESS_LITTLE, 8, 24, 0),
	m_adpcm_a_region(*this, "adpcma"),
	m_adpcm_b_region(*this, "adpcmb"),
	m_fm(*this),
	m_adpcm_a(*this, read8sm_delegate(*this, FUNC(ym2610_device::adpcm_a_read)), 8),
	m_adpcm_b(*this, read8sm_delegate(*this, FUNC(ym2610_device::adpcm_b_read)), write8sm_delegate(*this), 8),
	m_stream(nullptr),
	m_busy_duration(m_fm.compute_busy_duration()),
	m_address(0),
	m_fm_mask(fm_mask),
	m_eos_status(0x00),
	m_flag_mask(0xbf)
{
}


//-------------------------------------------------
//  ym2610b_device - constructor
//-------------------------------------------------

ym2610b_device::ym2610b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ym2610_device(mconfig, tag, owner, clock, YM2610B, 0x3f)
{
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

u8 ym2610_device::read(offs_t offset)
{
	u8 result = 0;
	switch (offset & 3)
	{
		case 0: // status port, YM2203 compatible
			result = m_fm.status() & (fm_engine::STATUS_TIMERA | fm_engine::STATUS_TIMERB | fm_engine::STATUS_BUSY);
			break;

		case 1: // data port (only SSG)
			if (m_address < 0x10)
				result = ay8910_read_ym();
			else if (m_address == 0xff)
				result = 1;  // ID code
			break;

		case 2: // status port, extended
			m_stream->update();
			result = m_eos_status & m_flag_mask;
			break;

		case 3: // ADPCM-B data
			break;
	}
	return result;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2610_device::write(offs_t offset, u8 value)
{
	switch (offset & 3)
	{
		case 0: // address port
			m_address = value;

			// write register to SSG emulator
			if (m_address < 0x10)
				ay8910_write_ym(0, m_address);
			break;

		case 1: // data port

			// ignore if paired with upper address
			if (BIT(m_address, 8))
				break;

			if (m_address < 0x10)
			{
				// write to SSG
				ay8910_write_ym(1, value);
			}
			else if (m_address < 0x1c)
			{
				// write to ADPCM-B
				m_stream->update();
				u8 address = m_address & 0x0f;

				// YM2610 effectively forces external mode on, and disables recording
				if (address == 0)
					value = (value | 0x20) & ~0x40;
				m_adpcm_b.write(address, value);
			}
			else if (m_address == 0x1c)
			{
				// EOS flag reset
				m_stream->update();
				m_flag_mask = ~value;
				m_eos_status &= ~value;
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

		case 2: // upper address port
			m_address = 0x100 | value;
			break;

		case 3: // upper data port

			// ignore if paired with lower address
			if (!BIT(m_address, 8))
				break;

			if (m_address < 0x130)
			{
				// write to ADPCM-A
				m_stream->update();
				m_adpcm_a.write(m_address & 0x3f, value);
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
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector ym2610_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_adpcm_a_config),
		std::make_pair(1, &m_adpcm_b_config)
	};
}


//-------------------------------------------------
//  device_start - start of emulation
//-------------------------------------------------

void ym2610_device::device_start()
{
	// call our parent
	ay8910_device::device_start();

	// create our stream
	m_stream = stream_alloc(0, fm_engine::OUTPUTS, m_fm.sample_rate(clock()));

	// save our data
	save_item(YMFM_NAME(m_address));
	save_item(YMFM_NAME(m_eos_status));
	save_item(YMFM_NAME(m_flag_mask));

	// save the engines
	m_fm.save(*this);
	m_adpcm_a.save(*this);
	m_adpcm_b.save(*this);

	// automatically map memory regions if not configured externally
	if (!has_configured_map(0) && !has_configured_map(1))
	{
		if (m_adpcm_a_region)
			space(0).install_rom(0, m_adpcm_a_region->bytes() - 1, m_adpcm_a_region->base());

		if (m_adpcm_b_region)
			space(1).install_rom(0, m_adpcm_b_region->bytes() - 1, m_adpcm_b_region->base());
		else if (m_adpcm_a_region)
			space(1).install_rom(0, m_adpcm_a_region->bytes() - 1, m_adpcm_a_region->base());
	}
}


//-------------------------------------------------
//  device_reset - start of emulation
//-------------------------------------------------

void ym2610_device::device_reset()
{
	// call our parent
	ay8910_device::device_reset();

	// reset the engines
	m_fm.reset();
	m_adpcm_a.reset();
	m_adpcm_b.reset();

	// initialize our special interrupt states
	m_eos_status = 0x00;
	m_flag_mask = 0xbf;
}


//-------------------------------------------------
//  device_clock_changed - update if clock changes
//-------------------------------------------------

void ym2610_device::device_clock_changed()
{
	m_stream->set_sample_rate(m_fm.sample_rate(clock()));
	ay_set_clock(clock() / 4);

	// recompute the busy duration
	m_busy_duration = m_fm.compute_busy_duration();
}


//-------------------------------------------------
//  sound_stream_update - update the sound stream
//-------------------------------------------------

void ym2610_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
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
		// clock the FM
		u32 env_counter = m_fm.clock(m_fm_mask);

		// clock the ADPCM-A engine on every envelope cycle
		if (BIT(env_counter, 0, 2) == 0)
			m_eos_status |= m_adpcm_a.clock(0x3f);

		// clock the ADPCM-B engine every cycle
		m_adpcm_b.clock(0x01);
		if ((m_adpcm_b.status() & ymadpcm_b_channel::STATUS_EOS) != 0)
			m_eos_status |= 0x80;

		// update the FM content; YM2610 is 13-bit with no intermediate clipping
		s32 sums[fm_engine::OUTPUTS] = { 0 };
		m_fm.output(sums, 1, 32767, m_fm_mask);

		// mix in the ADPCM
		m_adpcm_a.output(sums, 0x3f);
		m_adpcm_b.output(sums, 2, 0x01);

		// YM2608 is stereo
		for (int index = 0; index < fm_engine::OUTPUTS; index++)
			outputs[index].put_int_clamp(sampindex, sums[index], 32768);
	}
}


//-------------------------------------------------
//  adpcm_a_read - callback to read data for the
//  ADPCM-A engine; in this case, from address
//  space 0
//-------------------------------------------------

u8 ym2610_device::adpcm_a_read(offs_t offset)
{
	return space(0).read_byte(offset);
}


//-------------------------------------------------
//  adpcm_b_read - callback to read data for the
//  ADPCM-B engine; in this case, from address
//  space 1
//-------------------------------------------------

u8 ym2610_device::adpcm_b_read(offs_t offset)
{
	return space(1).read_byte(offset);
}
