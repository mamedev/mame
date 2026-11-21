// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
#ifndef MAME_MACHINE_NT3580_H
#define MAME_MACHINE_NT3580_H

#pragma once

#include "cpu/m6502/m6502.h"

// Novatek NT3580F sound controller: original external firmware is executed.
class nt3580_device : public device_t
{
public:
	static constexpr feature_type imperfect_features() { return feature::TIMING; }

	nt3580_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto sound_read_callback() { return m_sound_r.bind(); }
	auto sound_write_callback() { return m_sound_w.bind(); }

	void sound_irq_w(int state);
	void host_data_w(u8 data);
	void host_control_w(u8 data);
	u8 host_status_r() const;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

private:
	void program_map(address_map &map) ATTR_COLD;
	u8 peripheral_r(offs_t offset);
	void peripheral_w(offs_t offset, u8 data);
	u8 irq_status_r();
	void irq_enable_w(u8 data);
	void update_irq();
	void reset_registers();
	u8 sound_r(offs_t offset) { return m_sound_r(offset); }
	void sound_w(offs_t offset, u8 data) { m_sound_w(offset, data); }
	TIMER_CALLBACK_MEMBER(timer_tick);
	TIMER_CALLBACK_MEMBER(host_data_sync);
	TIMER_CALLBACK_MEMBER(host_control_sync);
	TIMER_CALLBACK_MEMBER(host_output_sync);

	required_device<m6502_device> m_cpu;
	required_region_ptr<u8> m_rom;
	devcb_read8 m_sound_r;
	devcb_write8 m_sound_w;
	emu_timer *m_timers[2];
	u8 m_ports[0x80];
	u8 m_irq_enable;
	u8 m_irq_pending;
	u8 m_host_data;
	u8 m_host_control;
	bool m_host_written;
	bool m_host_pending;
	bool m_sound_irq;
	bool m_reset;
};

DECLARE_DEVICE_TYPE(NT3580, nt3580_device)

#endif // MAME_MACHINE_NT3580_H
