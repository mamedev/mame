// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_CUDA_H
#define MAME_MACHINE_CUDA_H

#pragma once


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CUDA_TAG    "cuda"

#define CUDA_341S0060   0x1100  // v2.40 (Most common: Performa/Quadra 6xx, PowerMac x200, x400, x500, Pippin, Gossamer G3)
#define CUDA_341S0788   0x2200  // v2.37 (LC 475/575/Quadra 605, Quadra 660AV/840AV, PowerMac x200)
#define CUDA_341S0417   0x3300  // v2.35 (Color Classic)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cuda_device

class cuda_device :  public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	cuda_device(const machine_config &mconfig, const char *tag, device_t *owner, int type)
		: cuda_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_type(type);
	}

	cuda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

	// VIA interface routines
	uint8_t get_treq() { return treq; }
	void set_tip(uint8_t val) { tip = val; }
	void set_byteack(uint8_t val) { byteack = val; }
	uint8_t get_via_data() { return via_data; }
	void set_via_data(uint8_t dat) { via_data = dat; }
	uint8_t get_via_clock() { return via_clock; }
	void set_adb_line(int linestate) { adb_in = (linestate == ASSERT_LINE) ? true : false; }
	int get_adb_dtime() { return m_adb_dtime; }

	int rom_offset;

	auto reset_callback() { return write_reset.bind(); }
	auto linechange_callback() { return write_linechange.bind(); }
	auto via_clock_callback() { return write_via_clock.bind(); }
	auto via_data_callback() { return write_via_data.bind(); }

	devcb_write_line write_reset, write_linechange, write_via_clock, write_via_data;

	void cuda_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	required_device<cpu_device> m_maincpu;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	uint8_t ddrs[3]{};
	uint8_t ports[3]{};
	uint8_t pll_ctrl = 0;
	uint8_t timer_ctrl = 0;
	uint8_t timer_counter = 0, ripple_counter = 0;
	uint8_t onesec = 0;
	uint8_t treq = 0, byteack = 0, tip = 0, via_data = 0, via_clock = 0, last_adb = 0;
	uint64_t last_adb_time = 0;
	bool cuda_controls_power = false;
	bool adb_in = false;
	int reset_line = 0;
	int m_adb_dtime = 0;
	emu_timer *m_timer = nullptr, *m_prog_timer = nullptr;
	uint8_t pram[0x100]{}, disk_pram[0x100]{};
	bool pram_loaded = false;

	void send_port(uint8_t offset, uint8_t data);
};

// device type definition
DECLARE_DEVICE_TYPE(CUDA, cuda_device)

#endif // MAME_MACHINE_CUDA_H
