// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench, Stephane Humbert

#ifndef MAME_TOAPLAN_TOAPLAN_DSP_H
#define MAME_TOAPLAN_TOAPLAN_DSP_H

#pragma once

#include "cpu/tms320c1x/tms320c1x.h"

class toaplan_dsp_device : public device_t
{
public:
	using toaplan_dsp_host_addr_delegate = device_delegate<void (u16 data, u32 &seg, u32 &addr)>;
	using toaplan_dsp_host_read_delegate = device_delegate<u16 (u32 seg, u32 addr)>;
	using toaplan_dsp_host_write_delegate = device_delegate<bool (u32 seg, u32 addr, u16 data)>;

	toaplan_dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configurations
	template <typename... T> void set_host_addr_callback(T &&... args) { m_host_addr_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_host_read_callback(T &&... args) { m_host_r_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_host_write_callback(T &&... args) { m_host_w_cb.set(std::forward<T>(args)...); }
	auto halt_callback() { return m_halt_cb.bind(); }

	// host interfaces
	void dsp_int_w(int enable);

	// getters
	tms320c10_device &dsp() { return *m_dsp; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// devices
	required_device<tms320c10_device> m_dsp;

	// callbacks
	toaplan_dsp_host_addr_delegate m_host_addr_cb;
	toaplan_dsp_host_read_delegate m_host_r_cb;
	toaplan_dsp_host_write_delegate m_host_w_cb;
	devcb_write_line m_halt_cb;

	// internal states
	s32 m_dsp_on = 0;
	s32 m_dsp_bio = 0;
	bool m_dsp_execute = false;
	u32 m_dsp_addr_w = 0;
	u32 m_main_ram_seg = 0;

	void dsp_addrsel_w(u16 data);
	u16 dsp_r();
	void dsp_w(u16 data);
	void dsp_bio_w(u16 data);
	int bio_r();

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(TOAPLAN_DSP, toaplan_dsp_device)

#endif // MAME_TOAPLAN_TOAPLAN_DSP_H
