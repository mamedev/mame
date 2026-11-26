// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen

#ifndef MAME_MACHINE_PL080_H
#define MAME_MACHINE_PL080_H

#pragma once

#include <utility>

class pl080_device : public device_t
{
public:
	pl080_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	template <typename T> void set_dma_space(T &&tag, int space) { m_dma.set_tag(std::forward<T>(tag), space); }
	auto irq_callback() { return m_irq.bind(); }
	template <u32 Request> void request_w(int state) { static_assert(Request < 16); set_request(Request, state); }
	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u32 status_r() { return masked_status(false) | masked_status(true); }
	u32 terminal_status_r() { return masked_status(false); }
	u32 error_status_r() { return masked_status(true); }
	u32 terminal_raw_r() { return m_terminal; }
	u32 error_raw_r() { return m_errors; }
	u32 enabled_channels_r();
	u32 soft_r(offs_t offset) { return m_soft[offset]; }
	u32 configuration_r() { return m_config; }
	u32 sync_r() { return m_sync; }
	u32 identification_r(offs_t offset);
	void terminal_clear_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void error_clear_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void soft_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void configuration_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void sync_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void channel_w(u32 channel, offs_t offset, u32 data, u32 mem_mask);
	u32 masked_status(bool error) const;
	void update_irq();
	void set_request(u32 request, int state);
	bool ready(u32 ch) const;
	void schedule();
	void complete(u32 ch);
	TIMER_CALLBACK_MEMBER(transfer);

	required_address_space m_dma;
	devcb_write_line m_irq;
	emu_timer *m_timer;
	u32 m_channels[8][5];
	u32 m_config, m_sync;
	u16 m_requests, m_soft[4];
	u8 m_terminal, m_errors;
	bool m_irq_state;
};

DECLARE_DEVICE_TYPE(PL080, pl080_device)

#endif // MAME_MACHINE_PL080_H
