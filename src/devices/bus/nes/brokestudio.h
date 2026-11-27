// license:BSD-3-Clause
// copyright-holders:Brian H. Gilbert
#ifndef MAME_BUS_NES_BROKESTUDIO_H
#define MAME_BUS_NES_BROKESTUDIO_H

#pragma once

#include "nxrom.h"
#include "sound/vrc6.h"
#include "video/ppu2c0x.h"


// ======================> nes_rainbow_device

class nes_rainbow_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_rainbow_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual u8 read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual u8 chr_r(offs_t offset) override;
	virtual void chr_w(offs_t offset, u8 data) override;
	virtual u8 nt_r(offs_t offset) override;
	virtual void nt_w(offs_t offset, u8 data) override;

	// the CPU cycle IRQ, the cycle parity, the jitter counter and the "PPU stopped reading" test all count M2 periods
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;

	virtual void pcb_reset() override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr u32 FPGA_RAM_SIZE = 0x2000;

	// memory sources
	u8 *work_ram(u32 &size);
	u8 read_mem(int type, u32 offset);                      // type 0: PRG-ROM, 1: PRG-RAM, 2: FPGA-RAM
	void write_mem(int type, u32 offset, u8 data);
	u8 read_chr_abs(u32 addr);                              // pattern data by absolute address in the selected CHR source

	// banking
	void map_high(int slot, int slots, int reg);
	void map_low(int slot, int slots, int reg);
	void update_state();

	// PPU side
	u8 ppu_read(u16 addr);
	u8 internal_read(u16 addr);
	u8 pattern_data(u16 addr);
	u8 nt_data(u16 addr);
	void detect_scanline_start(u16 addr);
	void update_in_window();
	void update_irq();
	void ack_cpu_irq();
	void generate_oam_slow_update();
	void generate_oam_ext_update();

	// PRG: $8000-$FFFF in eight 4 KiB slots, $6000-$7FFF in two 4 KiB slots
	u8 m_hi_mode;
	u8 m_lo_mode;
	u16 m_hi[8];                // upper bits (bits 15:8) and lower bits (bits 7:0) of the PRG bank registers
	u16 m_lo[2];
	u8 m_hi_type[8];
	u32 m_hi_off[8];
	u8 m_lo_type[2];
	u32 m_lo_off[2];
	u8 m_fpga_bank;

	// CHR
	u8 m_chr_mode;
	u8 m_chr_select;
	bool m_window_en;
	bool m_sprite_ext;
	u8 m_bgext_offset;
	u16 m_chr_reg[16];
	u32 m_chr_off[16];          // offset of each 512 byte CHR slot
	u8 m_nt_bank[4];
	u8 m_nt_ctl[4];
	u8 m_fill_tile;
	u8 m_fill_attr;

	// window split
	u8 m_win_ctl;
	u8 m_win_bank;
	u8 m_win_x1, m_win_x2, m_win_y1, m_win_y2;
	u8 m_win_scroll_x, m_win_scroll_y;
	bool m_in_window;

	// scanline IRQ and PPU fetch tracking
	bool m_sl_en, m_sl_pend;
	u8 m_sl_line, m_sl_offset;
	u16 m_last_ppu_addr;
	s16 m_scanline_counter;
	u8 m_ppu_idle;
	u8 m_nt_read_counter;
	u8 m_ppu_read_counter;
	bool m_in_frame, m_in_hblank;
	u8 m_jitter;
	u8 m_parity;
	u8 m_nt_fetch_counter;
	s8 m_last_sprite_slot;   // the sprite slot of the previous nametable-range fetch, -1 outside the sprite fetches
	bool m_override_tile_fetch;
	u8 m_ext_data;

	// CPU cycle IRQ
	u16 m_cpu_irq_counter, m_cpu_irq_latch;
	bool m_cpu_irq_en, m_cpu_irq_pend, m_cpu_irq_en_after_ack, m_cpu_irq_ack_4011;
	bool m_irq_asserted;

	// FPGA-RAM auto reader/writer
	u16 m_fpga_addr;
	u8 m_fpga_inc;

	// vector redirection
	bool m_nmi_vec_en, m_irq_vec_en;
	u16 m_nmi_vec, m_irq_vec;

	// OAM routines
	u8 m_oam_slow_page, m_oam_ext_page, m_oam_limit;
	u8 m_oam_code[0x506];
	bool m_oam_locked;
	u8 m_sprite_ext_data[64];
	u8 m_sprite_ext_bank;

	// Sprite Extended Mode: the FPGA cannot see the PPU's OAM, so it keeps its own copy of each sprite's Y from the CPU's
	// writes to $2003/$2004 (OAM DMA included) and the sprite size from $2000, and picks each line's sprites itself
	void sprite_eval();
	required_device<ppu2c0x_device> m_ppu;
	u8 m_oam_y[64];
	u8 m_oam_addr;
	bool m_large_sprites;
	u8 m_oam_map[8];
	memory_passthrough_handler m_bus_tap;
	bool m_bus_tap_on = false;

	// Wi-Fi (the ESP is not emulated)
	bool m_esp_en, m_wifi_irq_en;
	u8 m_send_src, m_recv_dst;

	// audio
	u8 m_audio_ctl, m_audio_vol;
	required_device<vrc6snd_device> m_vrc6snd;

	u8 m_fpga[FPGA_RAM_SIZE];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_RAINBOW, nes_rainbow_device)

#endif // MAME_BUS_NES_BROKESTUDIO_H
