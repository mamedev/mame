// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_APPLE_CUDA_H
#define MAME_APPLE_CUDA_H

#pragma once

/// \brief Base class for Apple Cuda devices.
///
/// Cuda is a semi-custom Motorola 68HC05 microcontroller with
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

	u8 ddr_r(offs_t offset);
	void ddr_w(offs_t offset, u8 data);
	u8 ports_r(offs_t offset);
	void ports_w(offs_t offset, u8 data);
	u8 pll_r();
	void pll_w(u8 data);
	u8 timer_ctrl_r();
	void timer_ctrl_w(u8 data);
	u8 timer_counter_r();
	u8 onesec_r();
	void onesec_w(u8 data);
	u8 pram_r(offs_t offset);
	void pram_w(offs_t offset, u8 data);

	// VIA interface routines
	u8 get_treq() { return m_treq; }
	void set_tip(u8 val) { m_tip = val; }
	void set_byteack(u8 val) { m_byteack = val; }
	u8 get_via_data() { return m_via_data; }
	void set_via_data(u8 dat) { m_via_data = dat; }
	u8 get_via_clock() { return m_via_clock; }
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

	devcb_write_line write_reset, write_linechange, write_via_clock, write_via_data, write_iic_scl, write_iic_sda;

	virtual void cuda_map(address_map &map);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	TIMER_CALLBACK_MEMBER(seconds_tick);
	TIMER_CALLBACK_MEMBER(timer_tick);

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_internal_ram;
	required_region_ptr<u8> m_rom;
	required_region_ptr<u8> m_default_nvram;

private:
	u8 m_ddrs[3]{};
	u8 m_ports[3]{};
	u8 m_pll_ctrl;
	u8 m_timer_ctrl;
	u8 m_onesec;
	u8 m_treq, m_byteack, m_tip, m_via_data, m_via_clock, m_last_adb;
	u8 m_iic_sda;
	u64 m_last_adb_time;
	bool m_cuda_controls_power;
	bool m_adb_in;
	s32 m_reset_line;
	s32 m_adb_dtime;
	emu_timer *m_timer, *m_prog_timer;
	u8 m_pram[0x100]{}, m_disk_pram[0x100]{};
	bool m_pram_loaded;

	void send_port(u8 offset, u8 data);
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

	void cuda_map(address_map &map) override;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	u8 portb_r();
};

class cuda_lite_device : public cuda_device
{
public:
	cuda_lite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void cuda_map(address_map &map) override;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	u8 portb_r();
};

// device type definition
DECLARE_DEVICE_TYPE(CUDA_V2XX, cuda_2xx_device)
DECLARE_DEVICE_TYPE(CUDA_LITE, cuda_lite_device)
DECLARE_DEVICE_TYPE(CUDA_V302, cuda_302_device)

#endif // MAME_APPLE_CUDA_H
