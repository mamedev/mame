// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
#ifndef MAME_BUS_XAVIX_SUPRTVPC_PRINTER_H
#define MAME_BUS_XAVIX_SUPRTVPC_PRINTER_H

#pragma once

#include "suprtvpc_exp.h"

#include <memory>

class suprtvpc_printer_device : public device_t, public device_suprtvpc_exp_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::TIMING; }

	suprtvpc_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// device_suprtvpc_exp_interface implementation
	virtual void data_w(int state) override;
	virtual void clock_w(int state) override;
	virtual int ready_r() override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr unsigned ROW_BYTES = 48;
	static constexpr unsigned MAX_ROWS = 0xffff;
	static constexpr rgb_t PALETTE[] = { rgb_t::white(), rgb_t::black() };

	TIMER_CALLBACK_MEMBER(ready);
	TIMER_CALLBACK_MEMBER(abort_page);
	void receive_packet();
	void clear_rows();
	void print_page();

	emu_timer *m_busy_timer;
	emu_timer *m_idle_timer;
	std::unique_ptr<u8[]> m_page;
	u8 m_packet[ROW_BYTES + 1];
	u16 m_bit_count;
	u16 m_height;
	u16 m_row;
	bool m_data;
	bool m_clock;
	bool m_busy;
};

DECLARE_DEVICE_TYPE(SUPRTVPC_PRINTER, suprtvpc_printer_device)

#endif // MAME_BUS_XAVIX_SUPRTVPC_PRINTER_H
