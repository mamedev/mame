// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "y8950.h"


DEFINE_DEVICE_TYPE(Y8950, y8950_device, "y8950", "Y8950 OPL MSX-Audio")


//*********************************************************
//  Y8950 DEVICE
//*********************************************************

//-------------------------------------------------
//  y8950_device - constructor
//-------------------------------------------------

y8950_device::y8950_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_rom_interface(mconfig, *this),
	m_address(0),
	m_io_ddr(0),
	m_stream(nullptr),
	m_fm(*this),
	m_adpcm_b(*this, read8sm_delegate(*this, FUNC(y8950_device::adpcm_b_read)), write8sm_delegate(*this, FUNC(y8950_device::adpcm_b_write))),
	m_keyboard_read_handler(*this),
	m_keyboard_write_handler(*this),
	m_io_read_handler(*this),
	m_io_write_handler(*this)
{
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

u8 y8950_device::read(offs_t offset)
{
	u8 result = 0xff;
	switch (offset & 1)
	{
		case 0: // status port
			m_stream->update();
			result = combine_status();
			break;

		case 1:	// data port

			switch (m_address)
			{
				case 0x05:	// keyboard in
					result = m_keyboard_read_handler(0);
					break;

				case 0x09:	// ADPCM data
				case 0x1a:
					result = m_adpcm_b.read(offset - 0x07);;
					break;

				case 0x19:	// I/O data
					result = m_io_read_handler(0);
					break;

				default:
					logerror("Unexpected read from Y8950 offset %d\n", offset & 3);
					break;
			}
			break;
	}
	return result;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void y8950_device::write(offs_t offset, u8 value)
{
	switch (offset & 1)
	{
		case 0:	// address port
			m_address = value;
			break;

		case 1: // data port

			// force an update
			m_stream->update();

			// handle special addresses
			switch (m_address)
			{
				case 0x04:	// IRQ control
					m_fm.write(m_address, value);
					combine_status();
					break;

				case 0x06:	// keyboard out
					m_keyboard_write_handler(0, value);
					break;

				case 0x08:	// split FM/ADPCM-B
					m_adpcm_b.write(m_address - 0x07, (value & 0x0f) | 0x80);
					m_fm.write(m_address, value & 0xc0);
					break;

				case 0x07:	// ADPCM-B registers
				case 0x09:
				case 0x0a:
				case 0x0b:
				case 0x0c:
				case 0x0d:
				case 0x0e:
				case 0x0f:
				case 0x10:
				case 0x11:
				case 0x12:
				case 0x15:
				case 0x16:
				case 0x17:
					m_adpcm_b.write(m_address - 0x07, value);
					break;

				case 0x18:	// I/O direction
					m_io_ddr = value & 0x0f;
					break;

				case 0x19:	// I/O data
					m_io_write_handler(0, value & m_io_ddr);
					break;

				default:	// everything else to FM
					m_fm.write(m_address, value);
					break;
			}
	}
}


//-------------------------------------------------
//  device_start - start of emulation
//-------------------------------------------------

void y8950_device::device_start()
{
	// create our stream
	m_stream = stream_alloc(0, fm_engine::OUTPUTS, m_fm.sample_rate(clock()));

	// resolve callbacks
	m_keyboard_read_handler.resolve_safe(0);
	m_keyboard_write_handler.resolve_safe();
	m_io_read_handler.resolve_safe(0);
	m_io_write_handler.resolve_safe();

	// call this for the variants that need to adjust the rate
	device_clock_changed();

	// save our data
	save_item(YMFM_NAME(m_address));
	save_item(YMFM_NAME(m_io_ddr));

	// save the engines
	m_fm.save(*this);
	m_adpcm_b.save(*this);
}


//-------------------------------------------------
//  device_reset - start of emulation
//-------------------------------------------------

void y8950_device::device_reset()
{
	// reset the engines
	m_fm.reset();
	m_adpcm_b.reset();
}


//-------------------------------------------------
//  device_clock_changed - update if clock changes
//-------------------------------------------------

void y8950_device::device_clock_changed()
{
	m_stream->set_sample_rate(m_fm.sample_rate(clock()));
}


//-------------------------------------------------
//  rom_bank_updated - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void y8950_device::rom_bank_updated()
{
	m_stream->update();
}


//-------------------------------------------------
//  sound_stream_update - update the sound stream
//-------------------------------------------------

void y8950_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	// iterate over all target samples
	for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// clock the ADPCM-B engine every cycle
		m_adpcm_b.clock(0x01);

		// update the FM content; clipping is unknown
		s32 sums[std::max<int>(fm_engine::OUTPUTS, 2)] = { 0 };
		m_fm.output(sums, 1, 32767, fm_engine::ALL_CHANNELS);

		// mix in the ADPCM; ADPCM-B is stereo, but only one channel
		// not sure how it's wired up internally
		m_adpcm_b.output(sums, 3, 0x01);

		// convert to 10.3 floating point value for the DAC and back
		// Y8950 is mono
		for (int index = 0; index < fm_engine::OUTPUTS; index++)
			outputs[index].put_int(sampindex, ymfm_roundtrip_fp(sums[index]), 32768);
	}

	// update the status in case of ADPCM EOS
	combine_status();
}


//-------------------------------------------------
//  combine_status - combine status flags from
//  OPN and ADPCM-B, masking out any indicated by
//  the flag control register
//-------------------------------------------------

u8 y8950_device::combine_status()
{
	u8 status = m_fm.status() & ~(STATUS_ADPCM_B_EOS | STATUS_ADPCM_B_BRDY | STATUS_ADPCM_B_PLAYING);
	u8 adpcm_status = m_adpcm_b.status();
	if ((adpcm_status & ymadpcm_b_channel::STATUS_EOS) != 0)
		status |= STATUS_ADPCM_B_EOS;
	if ((adpcm_status & ymadpcm_b_channel::STATUS_BRDY) != 0)
		status |= STATUS_ADPCM_B_BRDY;
	if ((adpcm_status & ymadpcm_b_channel::STATUS_PLAYING) != 0)
		status |= STATUS_ADPCM_B_PLAYING;
	return m_fm.set_reset_status(status, ~status);
}


//-------------------------------------------------
//  adpcm_b_read - callback to read data for the
//  ADPCM-B engine; in this case, from our default
//  address space
//-------------------------------------------------

u8 y8950_device::adpcm_b_read(offs_t offset)
{
	return read_byte(offset);
}


//-------------------------------------------------
//  adpcm_b_write - callback to write data to the
//  ADPCM-B engine; in this case, to our default
//  address space
//-------------------------------------------------

void y8950_device::adpcm_b_write(offs_t offset, u8 data)
{
	space().write_byte(offset, data);
}
