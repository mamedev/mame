// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_ZNMCU_H
#define MAME_MACHINE_ZNMCU_H

#pragma once


DECLARE_DEVICE_TYPE(ZNMCU, znmcu_device)

class znmcu_device : public device_t
{
public:
	znmcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto dsw_handler() { return m_dsw_handler.bind(); }
	auto analog1_handler() { return m_analog1_handler.bind(); }
	auto analog2_handler() { return m_analog2_handler.bind(); }
	auto dataout_handler() { return m_dataout_handler.bind(); }
	auto dsr_handler() { return m_dsr_handler.bind(); }

	WRITE_LINE_MEMBER(write_select);
	WRITE_LINE_MEMBER(write_clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	devcb_read8 m_dsw_handler;
	devcb_read8 m_analog1_handler;
	devcb_read8 m_analog2_handler;
	devcb_write_line m_dataout_handler;
	devcb_write_line m_dsr_handler;

	static const int MaxBytes = 3;
	int m_select;
	int m_clk;
	int m_bit;
	int m_byte;
	int m_databytes;
	uint8_t m_send[MaxBytes];
	emu_timer *m_mcu_timer;
};

#endif // MAME_MACHINE_ZNMCU_H
