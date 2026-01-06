// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_TSCONF_BETA_H
#define MAME_SINCLAIR_TSCONF_BETA_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"


class tsconf_beta_device : public device_t
{
public:
	tsconf_beta_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void tsconf_beta_io(address_map &map) ATTR_COLD;

	auto out_dos_callback() { return m_out_dos_cb.bind(); }
	auto out_vdos_m1_callback() { return m_out_vdos_m1_cb.bind(); }

	u8 status_r();
	u8 track_r();
	u8 sector_r();
	u8 data_r();
	u8 state_r();

	void param_w(offs_t offset, u8 data);
	void command_w(u8 data);
	void track_w(u8 data);
	void sector_w(u8 data);
	void data_w(u8 data);
	void turbo_w(int state);

	void on_m1_w();
	bool dos_r() { return m_dos; }
	bool vdos_r() { return m_vdos; }
	bool dos_io_r() { return m_dos || m_io_forced; }
	void enable_w(bool state);
	void fddvirt_w(u8 fddvirt);
	void io_forced_w(bool io_forced);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	devcb_write_line m_out_dos_cb;
	devcb_write_line m_out_vdos_m1_cb;

private:
	required_device<kr1818vg93_device> m_wd179x;
	required_device_array<floppy_connector, 4> m_floppy;
	output_finder<4> m_floppy_led;
	void fdc_hld_w(int state);
	void motors_control();
	u8 m_control;
	bool m_motor_active;

	bool m_dos;
	bool m_vdos;
	bool m_io_forced;
	u8 m_fddvirt;

	bool pre_vg_in_check();
	bool pre_vg_out_check(bool is_port_match);
};


DECLARE_DEVICE_TYPE(TSCONF_BETA, tsconf_beta_device)

#endif // MAME_SINCLAIR_TSCONF_BETA_H
