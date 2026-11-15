// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_MACHINE_XAVIX_IO_H
#define MAME_MACHINE_XAVIX_IO_H

#pragma once

#include "xavix_pio.h"

#include <array>
#include <utility>

class xavix_io_device : public device_t
{
public:
	xavix_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// Callback masks identify pins configured as outputs.
	template <unsigned Port> auto input_callback() { return m_input_cb[Port].bind(); }

	template <unsigned Port> auto output_callback() { return m_output_cb[Port].bind(); }
	auto irq_callback() { return m_irq_cb.bind(); }
	u8 external_input_r(offs_t port, u8 mem_mask);
	void external_output_w(offs_t port, u8 data, u8 mem_mask);

	void map(address_map &map) ATTR_COLD;
	void event_map(address_map &map) ATTR_COLD;

	template <unsigned Event> void trigger_event()
	{
		static_assert(Event < 4);
		if (BIT(m_event_enable, Event))
		{
			m_event_pending |= 1U << Event;
			update_irq();
		}
	}

	virtual void direction_w(offs_t offset, u8 data);
	virtual void data_w(offs_t offset, u8 data);

	virtual u8 direction_r(offs_t offset);
	virtual u8 data_r(offs_t offset);
	template <unsigned Port> u8 direction() const { static_assert(Port < 2); return output_mask(Port); }
	template <unsigned Port> u8 data_latch() { static_assert(Port < 2); return output_latch(Port); }

protected:
	xavix_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u8 output_mask(unsigned port) const;
	virtual u8 output_latch(unsigned port);

private:
	u8 event_enable_r();
	void event_enable_w(u8 data);
	u8 event_status_r();
	void event_ack_w(u8 data);
	void update_irq();
	void update_output(unsigned port);

	devcb_read8::array<2> m_input_cb;

	devcb_write8::array<2> m_output_cb;
	devcb_write_line m_irq_cb;

	std::array<u8, 2> m_direction;
	std::array<u8, 2> m_data;
	u8 m_event_enable;
	u8 m_event_pending;
};

DECLARE_DEVICE_TYPE(XAVIX_IO, xavix_io_device)

class superxavix_io_device : public xavix_io_device
{
public:
	superxavix_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_pio(T &&tag) { m_pio.set_tag(std::forward<T>(tag)); }

	// xavix_io_device implementation
	virtual void direction_w(offs_t offset, u8 data) override;
	virtual void data_w(offs_t offset, u8 data) override;
	virtual u8 direction_r(offs_t offset) override;
	virtual u8 data_r(offs_t offset) override;

protected:
	// xavix_io_device implementation
	virtual u8 output_mask(unsigned port) const override;
	virtual u8 output_latch(unsigned port) override;

private:
	required_device<superxavix_pio_device> m_pio;
};

DECLARE_DEVICE_TYPE(SUPERXAVIX_IO, superxavix_io_device)

#endif // MAME_MACHINE_XAVIX_IO_H
