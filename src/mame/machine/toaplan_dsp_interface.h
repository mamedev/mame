// license:BSD-3-Clause
// copyright-holders:Quench
#ifndef MAME_MACHINE_TOAPLAN_DSP_INTERFACE_H
#define MAME_MACHINE_TOAPLAN_DSP_INTERFACE_H

#pragma once

#include "cpu/tms32010/tms32010.h"

// base class
class toaplan_dsp_intf_device : public device_t
{
public:
	// construction/destruction
	toaplan_dsp_intf_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configurations
	typedef device_delegate<void (u32 &main_ram_seg, u32 &dsp_addr, u16 data)> dsp_addr_cb_delegate;
	typedef device_delegate<bool (u32 main_ram_seg, u32 dsp_addr, u16 &data)> dsp_read_cb_delegate;
	typedef device_delegate<bool (u32 main_ram_seg, u32 dsp_addr, bool &dsp_execute, u16 data)> dsp_write_cb_delegate;

	template <typename T> void set_dsp_tag(T &&tag) { m_dsp.set_tag(std::forward<T>(tag)); }

	auto halt_callback() { return m_halt_cb.bind(); }
	template <typename... T> void set_dsp_addr_callback(T &&... args) { m_dsp_addr_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_dsp_read_callback(T &&... args) { m_dsp_read_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_dsp_write_callback(T &&... args) { m_dsp_write_cb.set(std::forward<T>(args)...); }

	// external accesses
	void dsp_addrsel_w(u16 data);
	u16 dsp_r();
	void dsp_w(u16 data);
	void dsp_bio_w(u16 data);

	DECLARE_READ_LINE_MEMBER(bio_r);
	DECLARE_WRITE_LINE_MEMBER(dsp_int_w);

protected:
	virtual void device_resolve_objects() override;
	virtual void device_post_load() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	u8 m_dsp_on = CLEAR_LINE;
	u8 m_dsp_bio = CLEAR_LINE;
	bool m_dsp_execute = false;
	u32 m_dsp_addr_w = 0;
	u32 m_main_ram_seg = 0;

	void dsp_ctrl_w(u8 data);

	required_device<tms32010_device> m_dsp;

	devcb_write_line m_halt_cb;
	dsp_addr_cb_delegate m_dsp_addr_cb;
	dsp_read_cb_delegate m_dsp_read_cb;
	dsp_write_cb_delegate m_dsp_write_cb;
};

DECLARE_DEVICE_TYPE(TOAPLAN_DSP_INTF, toaplan_dsp_intf_device)

#endif // MAME_MACHINE_TOAPLAN_DSP_INTERFACE_H
