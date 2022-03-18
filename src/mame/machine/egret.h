// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_EGRET_H
#define MAME_MACHINE_EGRET_H

#pragma once

#define USE_BUS_ADB (0)

#if USE_BUS_ADB
#include "bus/adb/adb.h"
#endif

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define EGRET_TAG   "egret"

#define EGRET_341S0851  0x1100
#define EGRET_341S0850  0x2200
#define EGRET_344S0100  0x3300


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> egret_device

class egret_device :  public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	egret_device(const machine_config &mconfig, const char *tag, device_t *owner, int type)
		: egret_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_type(type);
	}

	egret_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration helpers
	void set_type(int type) { rom_offset = type; }

	// device_config_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	uint8_t ddr_r(offs_t offset);
	void ddr_w(offs_t offset, uint8_t data);
	uint8_t ports_r(offs_t offset);
	void ports_w(offs_t offset, uint8_t data);
	uint8_t pll_r();
	void pll_w(uint8_t data);
	uint8_t timer_ctrl_r();
	void timer_ctrl_w(uint8_t data);
	uint8_t timer_counter_r();
	void timer_counter_w(uint8_t data);
	uint8_t onesec_r();
	void onesec_w(uint8_t data);
	uint8_t pram_r(offs_t offset);
	void pram_w(offs_t offset, uint8_t data);

#if USE_BUS_ADB
	void adb_w(int id, int state);
	void adb_poweron_w(int id, int state);
	void adb_change();
#endif

	// interface routines
	uint8_t get_xcvr_session() { return xcvr_session; }
	void set_via_full(uint8_t val) { via_full = val; }
	void set_sys_session(uint8_t val) { sys_session = val; }
	uint8_t get_via_data() { return via_data; }
	void set_via_data(uint8_t dat) { via_data = dat; }
	uint8_t get_via_clock() { return via_clock; }
	void set_adb_line(int linestate) { adb_in = (linestate == ASSERT_LINE) ? true : false; }

	int rom_offset;

	auto reset_callback() { return write_reset.bind(); }
	auto linechange_callback() { return write_linechange.bind(); }
	auto via_clock_callback() { return write_via_clock.bind(); }
	auto via_data_callback() { return write_via_data.bind(); }

	devcb_write_line write_reset, write_linechange;
	devcb_write_line write_via_clock, write_via_data;

	void egret_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	required_device<cpu_device> m_maincpu;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	uint8_t ddrs[3];
	uint8_t ports[3];
	uint8_t pll_ctrl;
	uint8_t timer_ctrl;
	uint8_t timer_counter;
	uint8_t onesec;
	uint8_t xcvr_session, via_full, sys_session, via_data, via_clock, last_adb;
	uint64_t last_adb_time;
	bool egret_controls_power;
	bool adb_in;
	int reset_line;
	int m_adb_dtime;
	emu_timer *m_timer;
	uint8_t pram[0x100], disk_pram[0x100];
	bool pram_loaded;

	#if USE_BUS_ADB
	optional_device <adb_connector> m_adb_connector[2];
	adb_device *m_adb_device[2];
	bool m_adb_device_out[2];
	bool m_adb_device_poweron[2];
	bool m_adb_out;
	#endif

	void send_port(uint8_t offset, uint8_t data);
};

// device type definition
DECLARE_DEVICE_TYPE(EGRET, egret_device)

#endif // MAME_MACHINE_EGRET_H
