// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __CUDA_H__
#define __CUDA_H__

#include "emu.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CUDA_TAG    "cuda"

#define CUDA_341S0060   0x1100  // v2.40 (Most common: Performa/Quadra 6xx, PowerMac x200, x400, x500, Pippin, Gossamer G3)
#define CUDA_341S0788   0x2200  // v2.37 (LC 475/575/Quadra 605, Quadra 660AV/840AV, PowerMac x200)
#define CUDA_341S0417   0x3300  // v2.35 (Color Classic)

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CUDA_ADD(_type) \
	MCFG_DEVICE_ADD(CUDA_TAG, CUDA, 0) \
	MCFG_CUDA_TYPE(_type)

#define MCFG_CUDA_REPLACE(_type) \
	MCFG_DEVICE_REPLACE(CUDA_TAG, CUDA, 0) \
	MCFG_CUDA_TYPE(_type)

#define MCFG_CUDA_REMOVE() \
	MCFG_DEVICE_REMOVE(CUDA_TAG)

#define MCFG_CUDA_TYPE(_type) \
	cuda_device::static_set_type(*device, _type);

#define MCFG_CUDA_REMOVE() \
	MCFG_DEVICE_REMOVE(CUDA_TAG)

#define MCFG_CUDA_RESET_CALLBACK(_cb) \
	devcb = &cuda_device::set_reset_cb(*device, DEVCB_##_cb);

#define MCFG_CUDA_LINECHANGE_CALLBACK(_cb) \
	devcb = &cuda_device::set_linechange_cb(*device, DEVCB_##_cb);

#define MCFG_CUDA_VIA_CLOCK_CALLBACK(_cb) \
	devcb = &cuda_device::set_via_clock_cb(*device, DEVCB_##_cb);

#define MCFG_CUDA_VIA_DATA_CALLBACK(_cb) \
	devcb = &cuda_device::set_via_data_cb(*device, DEVCB_##_cb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cuda_device

class cuda_device :  public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	cuda_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_type(device_t &device, int type);

	// device_config_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

	DECLARE_READ8_MEMBER( ddr_r );
	DECLARE_WRITE8_MEMBER( ddr_w );
	DECLARE_READ8_MEMBER( ports_r );
	DECLARE_WRITE8_MEMBER( ports_w );
	DECLARE_READ8_MEMBER( pll_r );
	DECLARE_WRITE8_MEMBER( pll_w );
	DECLARE_READ8_MEMBER( timer_ctrl_r );
	DECLARE_WRITE8_MEMBER( timer_ctrl_w );
	DECLARE_READ8_MEMBER( timer_counter_r );
	DECLARE_WRITE8_MEMBER( timer_counter_w );
	DECLARE_READ8_MEMBER( onesec_r );
	DECLARE_WRITE8_MEMBER( onesec_w );
	DECLARE_READ8_MEMBER( pram_r );
	DECLARE_WRITE8_MEMBER( pram_w );

	// VIA interface routines
	UINT8 get_treq() { return treq; }
	void set_tip(UINT8 val) { tip = val; }
	void set_byteack(UINT8 val) { byteack = val; }
	UINT8 get_via_data() { return via_data; }
	void set_via_data(UINT8 dat) { via_data = dat; }
	UINT8 get_via_clock() { return via_clock; }
	void set_adb_line(int linestate) { adb_in = (linestate == ASSERT_LINE) ? true : false; }
	int get_adb_dtime() { return m_adb_dtime; }

	int rom_offset;

	template<class _Object> static devcb_base &set_reset_cb(device_t &device, _Object wr) { return downcast<cuda_device &>(device).write_reset.set_callback(wr); }
	template<class _Object> static devcb_base &set_linechange_cb(device_t &device, _Object wr) { return downcast<cuda_device &>(device).write_linechange.set_callback(wr); }
	template<class _Object> static devcb_base &set_via_clock_cb(device_t &device, _Object wr) { return downcast<cuda_device &>(device).write_via_clock.set_callback(wr); }
	template<class _Object> static devcb_base &set_via_data_cb(device_t &device, _Object wr) { return downcast<cuda_device &>(device).write_via_data.set_callback(wr); }

	devcb_write_line write_reset, write_linechange, write_via_clock, write_via_data;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	required_device<cpu_device> m_maincpu;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	UINT8 ddrs[3];
	UINT8 ports[3];
	UINT8 pll_ctrl;
	UINT8 timer_ctrl;
	UINT8 timer_counter, ripple_counter;
	UINT8 onesec;
	UINT8 treq, byteack, tip, via_data, via_clock, last_adb;
	UINT64 last_adb_time;
	bool cuda_controls_power;
	bool adb_in;
	int reset_line;
	int m_adb_dtime;
	emu_timer *m_timer, *m_prog_timer;
	UINT8 pram[0x100], disk_pram[0x100];
	bool pram_loaded;

	void send_port(address_space &space, UINT8 offset, UINT8 data);
};

// device type definition
extern const device_type CUDA;

#endif
