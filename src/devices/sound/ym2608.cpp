// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ym2608.h"


DEFINE_DEVICE_TYPE(YM2608, ym2608_device, "ym2608", "YM2608 OPNA")


//*********************************************************
//  CONSTANTS
//*********************************************************

enum : u8
{
	STATUS_ADPCM_B_EOS = 0x04,
	STATUS_ADPCM_B_BRDY = 0x08,
	STATUS_ADPCM_B_ZERO = 0x10,
	STATUS_ADPCM_B_PLAYING = 0x20
};


//*********************************************************
//  YM2608 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2608_device - constructor
//-------------------------------------------------

ym2608_device::ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ay8910_device(mconfig, YM2608, tag, owner, clock, PSG_TYPE_YM, 1, 2),
	device_rom_interface(mconfig, *this),
	m_internal(*this, "internal"),
	m_fm(*this),
	m_adpcm_a(*this, read8sm_delegate(*this, FUNC(ym2608_device::adpcm_a_read)), 0),
	m_adpcm_b(*this, read8sm_delegate(*this, FUNC(ym2608_device::adpcm_b_read)), write8sm_delegate(*this, FUNC(ym2608_device::adpcm_b_write))),
	m_stream(nullptr),
	m_busy_duration(m_fm.compute_busy_duration()),
	m_address(0),
	m_irq_enable(0x1f),
	m_flag_control(0x1c)
{
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

u8 ym2608_device::read(offs_t offset)
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
			result = combine_status();
			break;

		case 3: // ADPCM-B data
			if (m_address < 0x10)
				result = m_adpcm_b.read(m_address);
			break;
	}
	return result;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2608_device::write(offs_t offset, u8 value)
{
	switch (offset & 3)
	{
		case 0: // address port
			m_address = value;
			if (m_address < 0x10)
			{
				// write register to SSG emulator
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

			// ignore if paired with upper address
			if (BIT(m_address, 8))
				break;

			if (m_address < 0x10)
			{
				// write to SSG
				ay8910_write_ym(1, value);
			}
			else if (m_address < 0x20)
			{
				// write to ADPCM-A
				m_stream->update();
				m_adpcm_a.write(m_address & 0x0f, value);
			}
			else if (m_address == 0x29)
			{
				// special IRQ mask register
				m_stream->update();
				m_irq_enable = value;
				m_fm.set_irq_mask(m_irq_enable & ~m_flag_control & 0x1f);
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

			if (m_address < 0x110)
			{
				// write to ADPCM-B
				m_stream->update();
				m_adpcm_b.write(m_address & 0x0f, value);
			}
			else if (m_address == 0x110)
			{
				// IRQ flag control
				m_stream->update();
				if (BIT(value, 7))
					m_fm.set_reset_status(0, 0xff);
				else
				{
					m_flag_control = value;
					m_fm.set_irq_mask(m_irq_enable & ~m_flag_control & 0x1f);
				}
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

void ym2608_device::device_start()
{
	// call our parent
	ay8910_device::device_start();

	// create our stream
	m_stream = stream_alloc(0, fm_engine::OUTPUTS, m_fm.sample_rate(clock()));

	// save our data
	save_item(YMFM_NAME(m_address));
	save_item(YMFM_NAME(m_irq_enable));
	save_item(YMFM_NAME(m_flag_control));

	// save the engines
	m_fm.save(*this);
	m_adpcm_a.save(*this);
	m_adpcm_b.save(*this);
}


//-------------------------------------------------
//  device_reset - start of emulation
//-------------------------------------------------

void ym2608_device::device_reset()
{
	// call our parent
	ay8910_device::device_reset();

	// reset the engines
	m_fm.reset();
	m_adpcm_a.reset();
	m_adpcm_b.reset();

	// configure ADPCM percussion sounds
	m_adpcm_a.set_start_end(0, 0x0000, 0x01bf); // bass drum
	m_adpcm_a.set_start_end(1, 0x01c0, 0x043f); // snare drum
	m_adpcm_a.set_start_end(2, 0x0440, 0x1b7f); // top cymbal
	m_adpcm_a.set_start_end(3, 0x1b80, 0x1cff); // high hat
	m_adpcm_a.set_start_end(4, 0x1d00, 0x1f7f); // tom tom
	m_adpcm_a.set_start_end(5, 0x1f80, 0x1fff); // rim shot

	// initialize our special interrupt states
	m_irq_enable = 0x1f;
	m_flag_control = 0x1c;
	combine_status();
}


//-------------------------------------------------
//  device_clock_changed - update if clock changes
//-------------------------------------------------

void ym2608_device::device_clock_changed()
{
	// refresh via prescale
	update_prescale(m_fm.clock_prescale());
}


//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region
//-------------------------------------------------

ROM_START( ym2608 )
	ROM_REGION( 0x2000, "internal", 0 )
	//
	// While this rom was dumped by output analysis, not decap, it was tested
	// by playing it back into the chip as an external adpcm sample and produced
	// an identical dac result. a decap would be nice to verify things 100%,
	// but there is currently no reason to think this rom dump is incorrect.
	//
	// offset 0x0000: Source: 01BD.ROM  Length:  448 / 0x000001C0
	// offset 0x01C0: Source: 02SD.ROM  Length:  640 / 0x00000280
	// offset 0x0440: Source: 04TOP.ROM Length: 5952 / 0x00001740
	// offset 0x1B80: Source: 08HH.ROM  Length:  384 / 0x00000180
	// offset 0x1D00: Source: 10TOM.ROM Length:  640 / 0x00000280
	// offset 0x1F80: Source: 20RIM.ROM Length:  128 / 0x00000080
	//
	ROM_LOAD16_WORD( "ym2608_adpcm_rom.bin", 0x0000, 0x2000, CRC(23c9e0d8) SHA1(50b6c3e288eaa12ad275d4f323267bb72b0445df) )
ROM_END

const tiny_rom_entry *ym2608_device::device_rom_region() const
{
	return ROM_NAME( ym2608 );
}


//-------------------------------------------------
//  rom_bank_updated - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void ym2608_device::rom_bank_updated()
{
	m_stream->update();
}


//-------------------------------------------------
//  sound_stream_update - update the sound stream
//-------------------------------------------------

void ym2608_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	// if this is not our stream, pass it on
	if (&stream != m_stream)
	{
		ay8910_device::sound_stream_update(stream, inputs, outputs);
		return;
	}

	// top bit of the IRQ enable flags controls 3-channel vs 6-channel mode
	u8 fmmask = BIT(m_irq_enable, 7) ? 0x3f : 0x07;

	// iterate over all target samples
	for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
	{
		// clock the FM
		u32 env_counter = m_fm.clock(fmmask);

		// clock the ADPCM-A engine on every envelope cycle
		// (channels 4 and 5 clock every 2 envelope clocks)
		if (BIT(env_counter, 0, 2) == 0)
			m_adpcm_a.clock(BIT(env_counter, 2) ? 0x0f : 0x3f);

		// clock the ADPCM-B engine every cycle
		m_adpcm_b.clock(0x01);

		// update the FM content; YM2608 is 13-bit with no intermediate clipping
		s32 sums[fm_engine::OUTPUTS] = { 0 };
		m_fm.output(sums, 1, 32767, fmmask);

		// mix in the ADPCM
		m_adpcm_a.output(sums, 0x3f);
		m_adpcm_b.output(sums, 2, 0x01);

		// YM2608 is stereo
		for (int index = 0; index < fm_engine::OUTPUTS; index++)
			outputs[index].put_int_clamp(sampindex, sums[index], 32768);
	}
}


//-------------------------------------------------
//  update_prescale - set a new prescale value and
//  update clocks as needed
//-------------------------------------------------

void ym2608_device::update_prescale(u8 newval)
{
	// inform the FM engine and refresh our clock rate
	m_fm.set_clock_prescale(newval);
	m_stream->set_sample_rate(m_fm.sample_rate(clock()));
	logerror("Prescale = %d; sample_rate = %d\n", newval, m_fm.sample_rate(clock()));

	// also scale the SSG streams
	// mapping is (FM->SSG): 6->4, 3->2, 2->1
	u8 ssg_scale = 2 * newval / 3;
	// QUESTION: where does the *2 come from??
	ay_set_clock(clock() / ssg_scale);

	// recompute the busy duration
	m_busy_duration = m_fm.compute_busy_duration();
}


//-------------------------------------------------
//  combine_status - combine status flags from
//  FM and ADPCM-B, masking out any indicated by
//  the flag control register
//-------------------------------------------------

u8 ym2608_device::combine_status()
{
	u8 status = m_fm.status() & ~(STATUS_ADPCM_B_EOS | STATUS_ADPCM_B_BRDY | STATUS_ADPCM_B_PLAYING);
	u8 adpcm_status = m_adpcm_b.status();
	if ((adpcm_status & ymadpcm_b_channel::STATUS_EOS) != 0)
		status |= STATUS_ADPCM_B_EOS;
	if ((adpcm_status & ymadpcm_b_channel::STATUS_BRDY) != 0)
		status |= STATUS_ADPCM_B_BRDY;
	if ((adpcm_status & ymadpcm_b_channel::STATUS_PLAYING) != 0)
		status |= STATUS_ADPCM_B_PLAYING;
	status &= ~(m_flag_control & 0x1f);
	m_fm.set_reset_status(status, ~status);
	return status;
}


//-------------------------------------------------
//  adpcm_a_read - callback to read data for the
//  ADPCM-A engine; in this case, from the internal
//  ROM containing drum samples
//-------------------------------------------------

u8 ym2608_device::adpcm_a_read(offs_t offset)
{
	return m_internal->as_u8(offset % m_internal->bytes());
}


//-------------------------------------------------
//  adpcm_b_read - callback to read data for the
//  ADPCM-B engine; in this case, from our default
//  address space
//-------------------------------------------------

u8 ym2608_device::adpcm_b_read(offs_t offset)
{
	return space(0).read_byte(offset);
}


//-------------------------------------------------
//  adpcm_b_write - callback to write data to the
//  ADPCM-B engine; in this case, to our default
//  address space
//-------------------------------------------------

void ym2608_device::adpcm_b_write(offs_t offset, u8 data)
{
	space(0).write_byte(offset, data);
}
