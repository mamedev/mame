// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_APPLE_EGRET_H
#define MAME_APPLE_EGRET_H

#pragma once

#include "cpu/m6805/m68hc05e1.h"

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

	auto reset_callback() { return write_reset.bind(); }
	auto linechange_callback() { return write_linechange.bind(); }
	auto via_clock_callback() { return write_via_clock.bind(); }
	auto via_data_callback() { return write_via_data.bind(); }
	auto dfac_scl_callback() { return write_dfac_scl.bind(); }
	auto dfac_sda_callback() { return write_dfac_sda.bind(); }
	auto dfac_latch_callback() { return write_dfac_latch.bind(); }

	devcb_write_line write_reset, write_linechange;
	devcb_write_line write_via_clock, write_via_data;
	devcb_write_line write_dfac_scl, write_dfac_sda, write_dfac_latch;

	required_device<m68hc05e1_device> m_maincpu;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
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
	u8 m_disk_pram[0x100]{};
	bool m_pram_loaded;

#if USE_BUS_ADB
	optional_device <adb_connector> m_adb_connector[2];
	adb_device *m_adb_device[2]{};
	bool m_adb_device_out[2]{};
	bool m_adb_device_poweron[2]{};
	bool m_adb_out = false;
	#endif

	u8 pa_r();
	u8 pb_r();
	u8 pc_r();
	void pa_w(u8 data);
	void pb_w(u8 data);
	void pc_w(u8 data);
};

// device type definition
DECLARE_DEVICE_TYPE(EGRET, egret_device)

#endif // MAME_APPLE_EGRET_H
