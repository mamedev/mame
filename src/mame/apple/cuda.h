// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_APPLE_CUDA_H
#define MAME_APPLE_CUDA_H

#pragma once

#include "cpu/m6805/m68hc05e1.h"

/// \brief Base class for Apple Cuda devices.
///
/// Cuda is a semi-custom Motorola 68HC05E1 microcontroller with
/// on-board RAM and ROM plus several GPIO pins.  Cuda handles
/// simple power management, the Apple Desktop Bus, I2C, real-time
/// clock, and parameter RAM.
class cuda_device :  public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	cuda_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_config_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// VIA interface routines
	u8 get_treq() { return m_treq; }
	void set_tip(u8 val) { m_tip = val; }
	void set_byteack(u8 val) { m_byteack = val; }
	u8 get_via_data() { return m_via_data; }
	void set_via_data(u8 dat) { m_via_data = dat; }
	void set_adb_line(int linestate) { m_adb_in = (linestate == ASSERT_LINE) ? true : false; }
	void set_iic_sda(u8 data) { m_iic_sda = (data & 1); }
	int get_adb_dtime() { return m_adb_dtime; }

	int rom_offset;

	auto reset_callback() { return write_reset.bind(); }
	auto linechange_callback() { return write_linechange.bind(); }
	auto via_clock_callback() { return write_via_clock.bind(); }
	auto via_data_callback() { return write_via_data.bind(); }
	auto iic_scl_callback() { return write_iic_scl.bind(); }
	auto iic_sda_callback() { return write_iic_sda.bind(); }
	auto dfac_latch_callback() { return write_dfac_latch.bind(); }

	devcb_write_line write_reset, write_linechange, write_via_clock, write_via_data;
	devcb_write_line write_iic_scl, write_iic_sda, write_dfac_latch;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	required_device<m68hc05e1_device> m_maincpu;
	required_region_ptr<u8> m_default_nvram;

	u8 pa_r();
	u8 pb_r();
	u8 pc_r();
	void pa_w(u8 data);
	void pb_w(u8 data);
	void pc_w(u8 data);

private:
	u8 m_treq, m_byteack, m_tip, m_via_data, m_last_adb;
	u8 m_iic_sda;
	u64 m_last_adb_time;
	bool m_cuda_controls_power;
	bool m_adb_in;
	s32 m_reset_line;
	s32 m_adb_dtime;
	u8 m_disk_pram[0x100]{};
	bool m_pram_loaded;
};

class cuda_2xx_device : public cuda_device
{
public:
	cuda_2xx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class cuda_302_device : public cuda_device
{
public:
	cuda_302_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
};

class cuda_lite_device : public cuda_device
{
public:
	cuda_lite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual void device_add_mconfig(machine_config &config) override;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
};

// device type definition
DECLARE_DEVICE_TYPE(CUDA_V2XX, cuda_2xx_device)
DECLARE_DEVICE_TYPE(CUDA_LITE, cuda_lite_device)
DECLARE_DEVICE_TYPE(CUDA_V302, cuda_302_device)

#endif // MAME_APPLE_CUDA_H
