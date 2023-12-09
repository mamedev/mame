// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_APPLE_EGRET_H
#define MAME_APPLE_EGRET_H

#pragma once

#define USE_BUS_ADB (0)

#if USE_BUS_ADB
#include "bus/adb/adb.h"
#endif

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/// \brief Base class for Apple Egret devices.
///
/// Egret is a semi-custom Motorola 68HC05 microcontroller with
/// on-board RAM and ROM plus several GPIO pins.  Egret handles
/// simple power management, the Apple Desktop Bus, I2C, real-time
/// clock, and parameter RAM.
class egret_device :  public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	egret_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// device_config_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	u8 ddr_r(offs_t offset);
	void ddr_w(offs_t offset, u8 data);
	u8 ports_r(offs_t offset);
	void ports_w(offs_t offset, u8 data);
	u8 pll_r();
	void pll_w(u8 data);
	u8 timer_ctrl_r();
	void timer_ctrl_w(u8 data);
	u8 timer_counter_r();
	void timer_counter_w(u8 data);
	u8 onesec_r();
	void onesec_w(u8 data);
	u8 pram_r(offs_t offset);
	void pram_w(offs_t offset, u8 data);

#if USE_BUS_ADB
	void adb_w(int id, int state);
	void adb_poweron_w(int id, int state);
	void adb_change();
#endif

	// interface routines
	u8 get_xcvr_session() { return m_xcvr_session; }
	void set_via_full(u8 val) { m_via_full = val; }
	void set_sys_session(u8 val) { m_sys_session = val; }
	u8 get_via_data() { return m_via_data; }
	void set_via_data(u8 dat) { m_via_data = dat; }
	u8 get_via_clock() { return m_via_clock; }
	void set_adb_line(int linestate) { m_adb_in = (linestate == ASSERT_LINE) ? true : false; }
	void set_iic_sda(u8 data) { m_iic_sda = (data & 1); }

	auto reset_callback() { return write_reset.bind(); }
	auto linechange_callback() { return write_linechange.bind(); }
	auto via_clock_callback() { return write_via_clock.bind(); }
	auto via_data_callback() { return write_via_data.bind(); }
	auto iic_scl_callback() { return write_iic_scl.bind(); }
	auto iic_sda_callback() { return write_iic_sda.bind(); }

	devcb_write_line write_reset, write_linechange;
	devcb_write_line write_via_clock, write_via_data;
	devcb_write_line write_iic_scl, write_iic_sda;

	void egret_map(address_map &map);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	TIMER_CALLBACK_MEMBER(seconds_tick);

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_internal_ram;
	required_region_ptr<u8> m_rom;

private:
	u8 m_ddrs[3]{};
	u8 m_ports[3]{};
	u8 m_pll_ctrl;
	u8 m_timer_ctrl;
	u8 m_timer_counter;
	u8 m_onesec;
	u8 m_xcvr_session;
	u8 m_via_full;
	u8 m_sys_session;
	u8 m_via_data;
	u8 m_via_clock;
	u8 m_last_adb;
	u64 m_last_adb_time;
	bool m_egret_controls_power;
	bool m_adb_in;
	s32 m_reset_line;
	s32 m_adb_dtime;
	emu_timer *m_timer;
	u8 m_pram[0x100]{};
	u8 m_disk_pram[0x100]{};
	bool m_pram_loaded;
	u8 m_iic_sda;

#if USE_BUS_ADB
	optional_device <adb_connector> m_adb_connector[2];
	adb_device *m_adb_device[2]{};
	bool m_adb_device_out[2]{};
	bool m_adb_device_poweron[2]{};
	bool m_adb_out = false;
	#endif

	void send_port(u8 offset, u8 data);
};

// device type definition
DECLARE_DEVICE_TYPE(EGRET, egret_device)

#endif // MAME_APPLE_EGRET_H
