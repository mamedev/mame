// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    SuperXaviX bitmap plotter. Reads and writes packed pixels of one to eight
    bits through a programmable bit address, with incrementing and
    non-incrementing data ports.

    Writes merge into a buffered byte to preserve adjacent pixels. Completed
    bytes and explicit flushes pass through board-supplied memory callbacks,
    allowing the system driver to handle its external memory bus.

***************************************************************************/

#include "emu.h"
#include "xavix_plotter.h"

DEFINE_DEVICE_TYPE(SUPERXAVIX_PLOTTER, superxavix_plotter_device, "superxavix_plotter", "SuperXaviX Bitmap Plotter")

superxavix_plotter_device::superxavix_plotter_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SUPERXAVIX_PLOTTER, tag, owner, clock),
	m_memory_read_cb(*this, 0),
	m_memory_write_cb(*this),
	m_flush_cb(*this)
{
}

void superxavix_plotter_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(superxavix_plotter_device::ctrl_r), FUNC(superxavix_plotter_device::ctrl_w));
	map(0x01, 0x01).lr8(NAME([] () -> u8 { return 0; })).w(FUNC(superxavix_plotter_device::flush_w));
	map(0x02, 0x02).rw(FUNC(superxavix_plotter_device::data_r<true>), FUNC(superxavix_plotter_device::data_w<true>));
	map(0x03, 0x03).rw(FUNC(superxavix_plotter_device::data_r<false>), FUNC(superxavix_plotter_device::data_w<false>));
	map(0x04, 0x07).rw(FUNC(superxavix_plotter_device::address_r), FUNC(superxavix_plotter_device::address_w));
}

void superxavix_plotter_device::device_start()
{
	m_flush_cb.resolve_safe();

	save_item(NAME(m_bit_address));
	save_item(NAME(m_mode));
	save_item(NAME(m_pending_byte_address));
	save_item(NAME(m_byte_pending));
	save_item(NAME(m_pending_byte));
}

void superxavix_plotter_device::device_reset()
{
	m_bit_address = 0;
	m_mode = 0;
	m_pending_byte_address = 0;
	m_byte_pending = false;
	m_pending_byte = 0;
}

u8 superxavix_plotter_device::ctrl_r()
{
	return m_mode;
}

void superxavix_plotter_device::ctrl_w(u8 data)
{
	flush_w(data);
	m_mode = data & 0x07;
}

void superxavix_plotter_device::flush_w(u8 data)
{
	// Flush the plotter byte before any word buffered by the board.
	if (m_byte_pending)
		m_memory_write_cb(m_pending_byte_address, m_pending_byte);

	m_byte_pending = false;
	m_pending_byte = 0;
	m_flush_cb();
}

template <bool Increment>
u8 superxavix_plotter_device::data_r()
{
	// The non-incrementing port peeks without changing the programmed address.
	u8 result = 0;
	u32 bit_address = pixel_address();
	u32 byte_address = ~u32(0);
	u8 source = 0;
	u8 const bit_count = m_mode + 1;

	for (u8 output_bit = 0; output_bit < bit_count; output_bit++)
	{
		u32 const current_byte_address = bit_address >> 3;
		if (current_byte_address != byte_address)
		{
			byte_address = current_byte_address;
			source = (m_byte_pending && (m_pending_byte_address == byte_address))
					? m_pending_byte
					: m_memory_read_cb(byte_address);
		}

		result |= BIT(source, bit_address & 0x07) << output_bit;
		bit_address = (bit_address + 1) & BIT_ADDRESS_MASK;
	}
	if (Increment && !machine().side_effects_disabled())
		m_bit_address = (m_bit_address + m_mode + 1) & BIT_ADDRESS_MASK;
	return result;
}

template <bool Increment>
void superxavix_plotter_device::data_w(u8 data)
{
	u8 bits_remaining = m_mode + 1;
	u32 bit_address = pixel_address();

	while (bits_remaining)
	{
		u32 const byte_address = bit_address >> 3;
		if (!m_byte_pending || (m_pending_byte_address != byte_address))
		{
			if (m_byte_pending)
				m_memory_write_cb(m_pending_byte_address, m_pending_byte);

			m_pending_byte_address = byte_address;
			m_pending_byte = m_memory_read_cb(byte_address);
			m_byte_pending = true;
		}

		u8 const bit_to_write = data & 1;
		data >>= 1;

		u8 const dest_bit = bit_address & 0x07;
		m_pending_byte &= ~(1 << dest_bit);
		m_pending_byte |= (bit_to_write << dest_bit);

		bit_address = (bit_address + 1) & BIT_ADDRESS_MASK;
		if (!(bit_address & 0x07))
		{
			m_memory_write_cb(m_pending_byte_address, m_pending_byte);
			m_byte_pending = false;
		}

		bits_remaining--;
	}
	if constexpr (Increment)
		m_bit_address = (m_bit_address + m_mode + 1) & BIT_ADDRESS_MASK;
}

u8 superxavix_plotter_device::address_r(offs_t offset)
{
	return m_bit_address >> (offset * 8);
}

void superxavix_plotter_device::address_w(offs_t offset, u8 data)
{
	unsigned const shift = offset * 8;
	m_bit_address = ((m_bit_address & ~(u32(0xff) << shift)) | (u32(data) << shift)) & BIT_ADDRESS_MASK;
}

u32 superxavix_plotter_device::pixel_address() const
{
	// Even widths align the selected bits, while cursor increments retain
	// the programmed bit position: widths 2/4/6/8 align to 2/4/2/8 bits.
	u32 const width = m_mode + 1;
	return m_bit_address & ~((width & -width) - 1);
}
