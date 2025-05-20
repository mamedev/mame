// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NINTENDO_NES_VT_SOC_H
#define MAME_NINTENDO_NES_VT_SOC_H

#pragma once

#include "cpu/m6502/rp2a03.h"
#include "sound/nes_apu_vt.h"
#include "m6502_swap_op_d5_d6.h"
#include "video/ppu2c0x_vt.h"
#include "screen.h"
#include "speaker.h"

class nes_vt02_vt03_soc_device : public device_t, public device_memory_interface
{
public:
	nes_vt02_vt03_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void program_map(address_map &map) ATTR_COLD;

	void vt03_8000_mapper_w(offs_t offset, uint8_t data);

	auto set_4150_write_cb() { return m_4150_write_cb.bind(); }
	auto set_41e6_write_cb() { return m_41e6_write_cb.bind(); }

	// 8-bit ports
	auto write_0_callback() { return m_write_0_callback.bind(); }
	auto read_0_callback() { return m_read_0_callback.bind(); }
	auto read_1_callback() { return m_read_1_callback.bind(); }

	// 4-bit ports
	auto extra_read_0_callback() { return m_extra_read_0_callback.bind(); }
	auto extra_read_1_callback() { return m_extra_read_1_callback.bind(); }
	auto extra_read_2_callback() { return m_extra_read_2_callback.bind(); }
	auto extra_read_3_callback() { return m_extra_read_3_callback.bind(); }

	auto extra_write_0_callback() { return m_extra_write_0_callback.bind(); }
	auto extra_write_1_callback() { return m_extra_write_1_callback.bind(); }
	auto extra_write_2_callback() { return m_extra_write_2_callback.bind(); }
	auto extra_write_3_callback() { return m_extra_write_3_callback.bind(); }

	void set_8000_scramble(uint8_t reg0, uint8_t reg1, uint8_t reg2, uint8_t reg3, uint8_t reg4, uint8_t reg5);
	void set_8006_scramble(uint8_t reg6, uint8_t reg7);
	void set_410x_scramble(uint8_t reg0, uint8_t reg1);
	void force_bad_dma() { m_force_baddma = true; }
	void force_raster_timing_hack() { m_use_raster_timing_hack = true; }

	void set_default_palette_mode(vtxx_pal_mode pmode) { m_default_palette_mode = pmode; }

protected:
	nes_vt02_vt03_soc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<ppu_vt03_device> m_ppu;
	required_device<nes_apu_vt_device> m_apu;

	void nes_vt_map(address_map &map) ATTR_COLD;
	virtual void nes_vt_2012_to_2017_regs(address_map &map);

	uint32_t get_banks(uint8_t bnk);
	void update_banks();
	uint16_t decode_nt_addr(uint16_t addr);
	void vt03_410x_w(offs_t offset, uint8_t data);
	uint8_t vt03_410x_r(offs_t offset);
	void scrambled_410x_w(uint16_t offset, uint8_t data);
	uint8_t spr_r(offs_t offset);
	uint8_t chr_r(offs_t offset);
	void chr_w(offs_t offset, uint8_t data);
	void scanline_irq(int scanline, bool vblank, bool blanked);
	void hblank_irq(int scanline, bool vblank, bool blanked);
	void video_irq(bool hblank, int scanline, bool vblank, bool blanked);
	uint8_t nt_r(offs_t offset);
	void nt_w(offs_t offset, uint8_t data);
	int calculate_real_video_address(int addr, int readtype);
	void scrambled_8000_w(uint16_t offset, uint8_t data);
	virtual void vt_dma_w(uint8_t data);
	void do_dma(uint8_t data, bool has_ntsc_bug);
	void vt03_4034_w(uint8_t data);
	void vt3xx_4024_new_dma_middle_w(uint8_t data);

	uint8_t in0_r();
	uint8_t in1_r();
	void in0_w(offs_t offset, uint8_t data);

	void extra_io_control_w(uint8_t data);
	uint8_t extrain_01_r();
	uint8_t extrain_23_r();
	void extraout_01_w(uint8_t data);
	void extraout_23_w(uint8_t data);
	uint8_t rs232flags_region_r();

	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

	uint8_t m_410x[0xc]{};

	uint8_t m_vdma_ctrl = 0;
	uint8_t m_4024_newdma;

	int m_timer_irq_enabled = 0;
	int m_timer_running = 0;
	int m_timer_val = 0;

	uint8_t m_8000_scramble[6];
	uint8_t m_8006_scramble[2];
	uint8_t m_410x_scramble[2];

	uint8_t m_8000_addr_latch = 0;

	uint8_t m_relative[2];

	uint8_t m_4242 = 0;
	uint8_t m_411c = 0;
	uint8_t m_411d = 0;

	uint8_t m_initial_e000_bank;
	/* expansion nametable - todo, see if we can refactor NES code to be reusable without having to add full NES bus etc. */
	std::unique_ptr<uint8_t[]> m_ntram;
	std::unique_ptr<uint8_t[]> m_chrram;

	void apu_irq(int state);
	uint8_t apu_read_mem(offs_t offset);

	uint8_t external_space_read(offs_t offset);
	void external_space_write(offs_t offset, uint8_t data);
	// additional relative offset for everything on vt3xx sets (seems to address up to 32mbytes only still?)
	int get_relative() { return (m_relative[0] + ((m_relative[1] & 0x0f) << 8)) * 0x2000; }

	void do_pal_timings_and_ppu_replacement(machine_config& config);

	devcb_write8 m_4150_write_cb;
	devcb_write8 m_41e6_write_cb;

private:

	address_space_config        m_space_config;

	int m_bankaddr[4]{};

	devcb_write8 m_write_0_callback;
	devcb_read8 m_read_0_callback;
	devcb_read8 m_read_1_callback;

	devcb_write8 m_extra_write_0_callback;
	devcb_write8 m_extra_write_1_callback;
	devcb_write8 m_extra_write_2_callback;
	devcb_write8 m_extra_write_3_callback;

	devcb_read8 m_extra_read_0_callback;
	devcb_read8 m_extra_read_1_callback;
	devcb_read8 m_extra_read_2_callback;
	devcb_read8 m_extra_read_3_callback;

	vtxx_pal_mode m_default_palette_mode;
	bool m_force_baddma = false;
	bool m_use_raster_timing_hack = false;
};

class nes_vt02_vt03_soc_pal_device : public nes_vt02_vt03_soc_device
{
public:
	nes_vt02_vt03_soc_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override;
};


class nes_vt02_vt03_soc_waixing_device : public nes_vt02_vt03_soc_device
{
public:
	nes_vt02_vt03_soc_waixing_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	nes_vt02_vt03_soc_waixing_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void nes_vt_2012_to_2017_regs(address_map &map) override;
};

class nes_vt02_vt03_soc_waixing_pal_device : public nes_vt02_vt03_soc_waixing_device
{
public:
	nes_vt02_vt03_soc_waixing_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override;
};

class nes_vt02_vt03_soc_hummer_device : public nes_vt02_vt03_soc_device
{
public:
	nes_vt02_vt03_soc_hummer_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void nes_vt_2012_to_2017_regs(address_map &map) override;
};

class nes_vt02_vt03_soc_sports_device : public nes_vt02_vt03_soc_device
{
public:
	nes_vt02_vt03_soc_sports_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	nes_vt02_vt03_soc_sports_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void nes_vt_2012_to_2017_regs(address_map &map) override;
};

class nes_vt02_vt03_soc_sports_pal_device : public nes_vt02_vt03_soc_sports_device
{
public:
	nes_vt02_vt03_soc_sports_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override;
};

class nes_vt02_vt03_soc_scramble_device : public nes_vt02_vt03_soc_device
{
public:
	nes_vt02_vt03_soc_scramble_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override;
};

class nes_vt02_vt03_soc_scramble_pal_device : public nes_vt02_vt03_soc_device
{
public:
	nes_vt02_vt03_soc_scramble_pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override;
};

DECLARE_DEVICE_TYPE(NES_VT02_VT03_SOC, nes_vt02_vt03_soc_device)
DECLARE_DEVICE_TYPE(NES_VT02_VT03_SOC_PAL, nes_vt02_vt03_soc_pal_device)

DECLARE_DEVICE_TYPE(NES_VT02_VT03_SOC_WAIXING,     nes_vt02_vt03_soc_waixing_device)
DECLARE_DEVICE_TYPE(NES_VT02_VT03_SOC_WAIXING_PAL, nes_vt02_vt03_soc_waixing_pal_device)

DECLARE_DEVICE_TYPE(NES_VT02_VT03_SOC_HUMMER, nes_vt02_vt03_soc_hummer_device)

DECLARE_DEVICE_TYPE(NES_VT02_VT03_SOC_SPORTS, nes_vt02_vt03_soc_sports_device)
DECLARE_DEVICE_TYPE(NES_VT02_VT03_SOC_SPORTS_PAL, nes_vt02_vt03_soc_sports_pal_device)

DECLARE_DEVICE_TYPE(NES_VT02_VT03_SOC_SCRAMBLE, nes_vt02_vt03_soc_scramble_device)
DECLARE_DEVICE_TYPE(NES_VT02_VT03_SOC_SCRAMBLE_PAL, nes_vt02_vt03_soc_scramble_pal_device)

#endif // MAME_NINTENDO_NES_VT_SOC_H
