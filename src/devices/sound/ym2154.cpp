// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#include "emu.h"
#include "ym2154.h"


//**************************************************************************
//  YM2154 DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(YM2154, ym2154_device, "ym2154", "YM2154 (RYP4)")

//-------------------------------------------------
//  ym2154_device - constructor
//-------------------------------------------------

ym2154_device::ym2154_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, YM2154, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_memory_interface(mconfig, *this),
	m_stream(nullptr),
	m_timer(nullptr),
	m_update_irq(*this),
	m_io_read(*this, 0),
	m_io_write(*this),
	m_group0_config("group0", ENDIANNESS_LITTLE, 8, 18, 0),
	m_group1_config("group1", ENDIANNESS_LITTLE, 8, 18, 0),
	m_group0_region(*this, "group0"),
	m_group1_region(*this, "group1")
{
}


//-------------------------------------------------
//  read - handle a device read
//-------------------------------------------------

u8 ym2154_device::read(offs_t offset)
{
	u8 result = 0xff;
	switch (offset & 0x7f)
	{
		case 0x01: // A/D converter registers
		case 0x02: // A/D converter registers
		case 0x03: // A/D converter registers
		case 0x04: // A/D converter registers
		case 0x05: // A/D converter registers
		case 0x06: // A/D converter registers
		case 0x07: // A/D converter registers
		case 0x08: // A/D converter registers
		case 0x09: // A/D converter registers
		case 0x0a: // A/D converter registers
			result = m_io_read(offset - 1);
			break;

		case 0x0e: // IRQ ack
			update_irq_state(0);
			result = m_irq_count;
			break;
	}
	return result;
}


//-------------------------------------------------
//  write - handle a device write
//-------------------------------------------------

void ym2154_device::write(offs_t offset, u8 data)
{
	m_stream->update();

	u8 chan = BIT(offset, 0, 3);
	u8 old;
	switch (offset & 0x7f)
	{
		// timer count
		case 0x02:
			if (BIT(data, 7) != 0)
				m_timer_count = (m_timer_count & 0x0f) | (BIT(data, 0, 7) << 4);
			else
				m_timer_count = (m_timer_count & 0x7f0) | BIT(data, 0, 4);
			break;

		// timer enable/output
		case 0x03:
			old = m_timer_enable;
			m_timer_enable = BIT(data, 2);
			if (!m_timer_enable)
				m_timer->enable(false);
			else if (m_timer_enable && !old)
				m_timer->adjust((2048 - m_timer_count) * attotime::from_hz(sample_rate()));
			m_io_write(0, BIT(data, 4, 4) ^ 0x0f);
			break;

		// output level
		case 0x04:
			m_total_level = BIT(data, 0, 6);
			break;

		// group 1 trigger
		case 0x05:
			for (int chan = 0; chan < 6; chan++)
				if (BIT(data, chan))
					m_channel[6 + chan].start();
			break;

		// group 0 trigger
		case 0x06:
			for (int chan = 0; chan < 6; chan++)
				if (BIT(data, chan))
					m_channel[0 + chan].start();
			break;

		// DAC mode (not implemented)
		case 0x07:
			break;

		// pan
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d:
			m_channel[0 + chan].m_panpot = BIT(data, 0, 4);
			m_channel[6 + chan].m_panpot = BIT(data, 4, 4);
			break;

		// rate/level, group 0
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15:
			m_channel[0 + chan].m_output_level = BIT(data, 0, 5);
			m_channel[0 + chan].m_rate = BIT(data, 5, 2);
			break;

		// rate/level, group 1
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d:
			m_channel[6 + chan].m_output_level = BIT(data, 0, 5);
			m_channel[6 + chan].m_rate = BIT(data, 5, 2);
			break;

		// sample position A, group 0
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25:
			m_channel[0 + chan].m_start = (m_channel[0 + chan].m_start & 0x0f) | (data << 4);
			break;

		// sample position B, group 0
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d:
			m_channel[0 + chan].m_start = (m_channel[0 + chan].m_start & 0xff0) | (data >> 4);
			m_channel[0 + chan].m_end = (m_channel[0 + chan].m_end & 0xff) | ((data & 0xf) << 8);
			break;

		// sample position C, group 0
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35:
			m_channel[0 + chan].m_end = (m_channel[0 + chan].m_end & 0xf00) | data;
			break;

		// sample position A, group 1
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d:
			m_channel[6 + chan].m_start = (m_channel[6 + chan].m_start & 0x0f) | (data << 4);
			break;

		// sample position B, group 1
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45:
			m_channel[6 + chan].m_start = (m_channel[6 + chan].m_start & 0xff0) | (data >> 4);
			m_channel[6 + chan].m_end = (m_channel[6 + chan].m_end & 0xff) | ((data & 0xf) << 8);
			break;

		// sample position C, group 1
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d:
			m_channel[6 + chan].m_end = (m_channel[6 + chan].m_end & 0xf00) | data;
			break;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym2154_device::device_start()
{
	// allocate our timer
	m_timer = timer_alloc(FUNC(ym2154_device::delayed_irq), this);

	// allocate our stream
	m_stream = stream_alloc(0, 2, sample_rate());

	// now register the blob for save, on the assumption the size won't change
	save_item(NAME(m_timer_count));
	save_item(NAME(m_timer_enable));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_total_level));
	save_item(STRUCT_MEMBER(m_channel, m_pos));
	save_item(STRUCT_MEMBER(m_channel, m_start));
	save_item(STRUCT_MEMBER(m_channel, m_end));
	save_item(STRUCT_MEMBER(m_channel, m_panpot));
	save_item(STRUCT_MEMBER(m_channel, m_output_level));
	save_item(STRUCT_MEMBER(m_channel, m_rate));

	// automatically map memory regions if not configured externally
	if (!has_configured_map(0) && !has_configured_map(1))
	{
		if (m_group0_region)
			space(0).install_rom(0, m_group0_region->bytes() - 1, m_group0_region->base());

		if (m_group1_region)
			space(1).install_rom(0, m_group1_region->bytes() - 1, m_group1_region->base());
		else if (m_group0_region)
			space(1).install_rom(0, m_group0_region->bytes() - 1, m_group0_region->base());
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym2154_device::device_reset()
{
	for (int chan = 0; chan < 12; chan++)
		m_channel[chan].m_pos = 0xfffffff;
}


//-------------------------------------------------
//  device_clock_changed - clock changed signal
//-------------------------------------------------

void ym2154_device::device_clock_changed()
{
	if (m_stream != nullptr)
		m_stream->set_sample_rate(sample_rate());
}


//-------------------------------------------------
//  delayed_irq -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(ym2154_device::delayed_irq)
{
	update_irq_state(1);
	m_timer->adjust((2048 - m_timer_count) * attotime::from_hz(sample_rate()));
}


//-------------------------------------------------
//  sound_stream_update - generate sound data
//-------------------------------------------------

void ym2154_device::sound_stream_update(sound_stream &stream)
{
	static const uint16_t voltable[8] = { 0x7fa,0x751,0x6b5,0x627,0x5a4,0x52c,0x4be,0x45a };

	for (int chan = 0; chan < 12; chan++)
	{
		auto &channel = m_channel[chan];

		// not sure how the "rate" really works but it's used to stop a sample from
		// playing so just treat it as such
		if (channel.m_rate == 3)
			m_channel[chan].m_pos = 0xfffffff;

		if ((channel.m_pos >> ADDR_SHIFT) <= channel.m_end && channel.m_panpot != 0)
		{
			uint32_t vol = (channel.m_output_level ^ 0x1f) + (m_total_level ^ 0x3f);

			uint32_t lvol = vol;
			if (channel.m_panpot > 8)
				lvol += 4 * (channel.m_panpot - 8);
			lvol = voltable[lvol & 7] >> (lvol >> 3);

			uint32_t rvol = vol;
			if (channel.m_panpot < 7)
				rvol += 4 * (7 - channel.m_panpot);
			rvol = voltable[rvol & 7] >> (rvol >> 3);

			auto &source = space(chan / 6);
			for (int sampindex = 0; sampindex < stream.samples() && (channel.m_pos >> ADDR_SHIFT) <= channel.m_end; sampindex++)
			{
				uint8_t raw = source.read_byte(channel.m_pos++);

				// seems to be ulaw encoded (but with no inversion); this might
				// also be able to be folded into the volume calculations but
				// since it's all guesswork leave them separate for now
				int16_t sample = 0x21 | ((raw & 0x0f) << 1);
				sample <<= (raw >> 4) & 7;
				if (BIT(raw, 7))
					sample = -sample;

				stream.add_int(0, sampindex, sample * lvol, 0x2000 * 0x800);
				stream.add_int(1, sampindex, sample * rvol, 0x2000 * 0x800);
			}
		}
	}
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector ym2154_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_group0_config),
		std::make_pair(1, &m_group1_config)
	};
}
