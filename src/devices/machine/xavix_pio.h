// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_MACHINE_XAVIX_PIO_H
#define MAME_MACHINE_XAVIX_PIO_H

#pragma once

#include <array>

class superxavix_pio_device : public device_t
{
public:
	superxavix_pio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// Callback masks identify pins configured as outputs.
	template <unsigned Port> auto input_callback() { return m_input_cb[Port].bind(); }
	template <unsigned Port> auto input_mask_callback() { return m_input_mask_cb[Port].bind(); }

	template <unsigned Port> auto output_callback() { return m_output_cb[Port].bind(); }
	auto legacy_input_callback() { return m_legacy_input_cb.bind(); }
	auto legacy_input_mask_callback() { return m_legacy_input_mask_cb.bind(); }
	auto legacy_output_callback() { return m_legacy_output_cb.bind(); }
	template <unsigned Port> void set_legacy_output_mask(u8 mask) { static_assert(Port < 2); m_legacy_output_mask[Port] = mask; }

	void map(address_map &map) ATTR_COLD;
	u8 output_mask(unsigned port) const { return m_pio_direction[port] & m_pio_pull[port]; }

	u8 pio_direction_r(offs_t offset);
	void pio_direction_w(offs_t offset, u8 data);

	u8 pio_output_r(offs_t offset);
	void pio_output_w(offs_t offset, u8 data);

	u8 pio_input_r(offs_t offset);
	u8 pio_pull_r(offs_t offset);
	void pio_pull_w(offs_t offset, u8 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void update_output(unsigned port);
	u8 input_r(unsigned port);

	devcb_read8::array<3> m_input_cb;
	devcb_read8::array<3> m_input_mask_cb;
	devcb_write8::array<3> m_output_cb;
	devcb_read8 m_legacy_input_cb;
	devcb_read8 m_legacy_input_mask_cb;
	devcb_write8 m_legacy_output_cb;
	std::array<u8, 2> m_legacy_output_mask;

	std::array<u8, 3> m_pio_direction;
	std::array<u8, 3> m_pio_output;
	std::array<u8, 3> m_pio_pull;
};

DECLARE_DEVICE_TYPE(SUPERXAVIX_PIO, superxavix_pio_device)

#endif // MAME_MACHINE_XAVIX_PIO_H
