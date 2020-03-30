// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_NES_VT_H
#define MAME_MACHINE_NES_VT_H

#pragma once

#include "cpu/m6502/n2a03.h"
#include "machine/m6502_vtscr.h"
#include "machine/m6502_swap_op_d5_d6.h"
#include "video/ppu2c0x_vt.h"
#include "screen.h"
#include "speaker.h"

class nes_vt_soc_device : public device_t, public device_memory_interface
{
public:
	nes_vt_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void program_map(address_map &map);

	DECLARE_WRITE8_MEMBER(vt03_8000_mapper_w);

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

	void set_201x_descramble(uint8_t reg0, uint8_t reg1, uint8_t reg2, uint8_t reg3, uint8_t reg4, uint8_t reg5)
	{
		m_2012_2017_descramble[0] = reg0; // TOOD: name regs
		m_2012_2017_descramble[1] = reg1;
		m_2012_2017_descramble[2] = reg2;
		m_2012_2017_descramble[3] = reg3;
		m_2012_2017_descramble[4] = reg4;
		m_2012_2017_descramble[5] = reg5;
	};

	void set_8000_scramble(uint8_t reg0, uint8_t reg1, uint8_t reg2, uint8_t reg3, uint8_t reg4, uint8_t reg5, uint8_t reg6, uint8_t reg7);

protected:
	nes_vt_soc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual space_config_vector memory_space_config() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<ppu_vt03_device> m_ppu;
	required_device<nesapu_device> m_apu;

	void nes_vt_map(address_map& map);


	uint32_t get_banks(uint8_t bnk);
	void update_banks();
	uint16_t decode_nt_addr(uint16_t addr);
	DECLARE_WRITE8_MEMBER(vt03_410x_w);
	DECLARE_READ8_MEMBER(vt03_410x_r);
	void scrambled_410x_w(uint16_t offset, uint8_t data);
	DECLARE_READ8_MEMBER(spr_r);
	DECLARE_READ8_MEMBER(chr_r);
	DECLARE_WRITE8_MEMBER(chr_w);
	void scanline_irq(int scanline, int vblank, int blanked);
	void hblank_irq(int scanline, int vblank, int blanked);
	void video_irq(bool hblank, int scanline, int vblank, int blanked);
	DECLARE_READ8_MEMBER(nt_r);
	DECLARE_WRITE8_MEMBER(nt_w);
	int calculate_real_video_address(int addr, int extended, int readtype);
	void scrambled_8000_w(address_space& space, uint16_t offset, uint8_t data);
	void set_410x_scramble(uint8_t reg0, uint8_t reg1);
	DECLARE_READ8_MEMBER(psg1_4014_r);
	DECLARE_READ8_MEMBER(psg1_4015_r);
	DECLARE_WRITE8_MEMBER(psg1_4015_w);
	DECLARE_WRITE8_MEMBER(psg1_4017_w);
	DECLARE_WRITE8_MEMBER(vt_dma_w);
	DECLARE_WRITE8_MEMBER(vt_fixed_dma_w);
	void do_dma(uint8_t data, bool has_ntsc_bug);
	DECLARE_WRITE8_MEMBER(vt03_4034_w);


	DECLARE_READ8_MEMBER(in0_r);
	DECLARE_READ8_MEMBER(in1_r);
	DECLARE_WRITE8_MEMBER(in0_w);

	DECLARE_WRITE8_MEMBER(extra_io_control_w);
	DECLARE_READ8_MEMBER(extrain_01_r);
	DECLARE_READ8_MEMBER(extrain_23_r);
	DECLARE_WRITE8_MEMBER(extraout_01_w);
	DECLARE_WRITE8_MEMBER(extraout_23_w);
	DECLARE_READ8_MEMBER(rs232flags_region_r);

	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

	uint8_t m_410x[0xc];

	uint8_t m_vdma_ctrl;
	int m_timer_irq_enabled;
	int m_timer_running;
	int m_timer_val;

	uint8_t m_8000_scramble[8];
	uint8_t m_410x_scramble[2];

	uint8_t m_8000_addr_latch;
	
	

	uint8_t m_4242;
	uint8_t m_411c;
	uint8_t m_411d;
	uint8_t m_413x[8]; // CY only?

	uint8_t m_initial_e000_bank;
	/* expansion nametable - todo, see if we can refactor NES code to be reusable without having to add full NES bus etc. */
	std::unique_ptr<uint8_t[]> m_ntram;
	std::unique_ptr<uint8_t[]> m_chrram;

private:

	address_space_config        m_space_config;

	DECLARE_WRITE_LINE_MEMBER(apu_irq);
	DECLARE_READ8_MEMBER(apu_read_mem);

	DECLARE_READ8_MEMBER(external_space_read);
	DECLARE_WRITE8_MEMBER(external_space_write);

	int m_bankaddr[4];
	uint16_t m_real_access_address;

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

	uint8_t m_2012_2017_descramble[0x6]; // passed to PPU in reset
};


class nes_vt_soc_scramble_device : public nes_vt_soc_device
{
public:
	nes_vt_soc_scramble_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config& config) override;
};


DECLARE_DEVICE_TYPE(NES_VT_SOC, nes_vt_soc_device)
DECLARE_DEVICE_TYPE(NES_VT_SOC_SCRAMBLE, nes_vt_soc_scramble_device)

#endif // MAME_MACHINE_NES_VT_H
