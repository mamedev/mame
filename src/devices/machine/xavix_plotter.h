// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_MACHINE_XAVIX_PLOTTER_H
#define MAME_MACHINE_XAVIX_PLOTTER_H

#pragma once

#include <utility>

class superxavix_plotter_device : public device_t
{
public:
	superxavix_plotter_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// Callbacks receive the full byte address; external memory decoding belongs to the board.
	auto memory_read_callback() { return m_memory_read_cb.bind(); }
	auto memory_write_callback() { return m_memory_write_cb.bind(); }
	template <typename... T> void set_flush_callback(T &&... args) { m_flush_cb.set(std::forward<T>(args)...); }

	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u32 BIT_ADDRESS_MASK = 0x3fffffff;

	u8 ctrl_r();
	void ctrl_w(u8 data);
	void flush_w(u8 data);
	template <bool Increment> u8 data_r();
	template <bool Increment> void data_w(u8 data);
	u8 address_r(offs_t offset);
	void address_w(offs_t offset, u8 data);
	u32 pixel_address() const;

	devcb_read8 m_memory_read_cb;
	devcb_write8 m_memory_write_cb;
	device_delegate<void ()> m_flush_cb;

	u32 m_bit_address;
	u8 m_mode;
	u32 m_pending_byte_address;
	bool m_byte_pending;
	u8 m_pending_byte;
};

DECLARE_DEVICE_TYPE(SUPERXAVIX_PLOTTER, superxavix_plotter_device)

#endif // MAME_MACHINE_XAVIX_PLOTTER_H
