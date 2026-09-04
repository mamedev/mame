// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, R. Belmont
#pragma once

#ifndef MAME_NINTENDO_NDS_H
#define MAME_NINTENDO_NDS_H

#include "bus/gba/gba_slot.h"
#include "bus/nds/ndsslot.h"
#include "cpu/arm7/arm7.h"
#include "machine/bankdev.h"
#include "machine/s35180.h"
#include "machine/timer.h"
#include "screen.h"
#include "video/gba_ppu.h"
#include "sound/nds_sound.h"
#include "sound/gb.h"
#include "sound/dac.h"
#include "speaker.h"

class nds_state : public driver_device
{
public:
	nds_state(const machine_config &mconfig, device_type type, const char *tag);

	void nds(machine_config &config);

private:
	// LCD timing
	static constexpr int TOTAL_LINES = 263;
	static constexpr int VISIBLE_LINES = 192;
	static constexpr int TOTAL_DOTS = 355;
	static constexpr int VISIBLE_DOTS = 256;

	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	// ARM7
	uint32_t arm7_sio_r(offs_t offset);
	void arm7_sio_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t arm7_rcnt_r(offs_t offset, uint32_t mem_mask = ~0);
	void arm7_rcnt_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t arm7_spi_cnt_r();
	void arm7_spi_cnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t arm7_spi_data_r();
	void arm7_spi_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t arm7_wramstat_r();
	uint8_t arm7_postflg_r();
	void arm7_postflg_w(uint8_t data);
	void arm7_haltcnt_w(uint8_t data);
	uint16_t arm7_powcnt_r();
	void arm7_powcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void arm7_biosprot_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// ARM9
	uint32_t arm9_vramcnt_abcd_r();
	void arm9_vramcnt_a_w(uint8_t data);
	void arm9_vramcnt_b_w(uint8_t data);
	void arm9_vramcnt_c_w(uint8_t data);
	void arm9_vramcnt_d_w(uint8_t data);
	uint32_t arm9_vramcnt_efg_wramcnt_r();
	void arm9_vramcnt_e_w(uint8_t data);
	void arm9_vramcnt_f_w(uint8_t data);
	void arm9_vramcnt_g_w(uint8_t data);
	void arm9_wramcnt_w(uint8_t data);
	uint32_t arm9_vramcnt_hi_r();
	void arm9_vramcnt_h_w(uint8_t data);
	void arm9_vramcnt_i_w(uint8_t data);
	uint32_t arm9_divcnt_r();
	void arm9_divcnt_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t arm9_div_numer_lsw_r();
	void arm9_div_numer_lsw_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t arm9_div_numer_msw_r();
	void arm9_div_numer_msw_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t arm9_div_denom_lsw_r();
	void arm9_div_denom_lsw_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t arm9_div_denom_msw_r();
	void arm9_div_denom_msw_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t arm9_sqrtcnt_r();
	void arm9_sqrtcnt_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t arm9_sqrt_param_lsw_r();
	void arm9_sqrt_param_lsw_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t arm9_sqrt_param_msw_r();
	void arm9_sqrt_param_msw_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t arm9_postflg_r();
	void arm9_postflg_w(uint8_t data);
	uint16_t arm9_powcnt_r();
	void arm9_powcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// registers shared by both CPUs (timers, DMA, IRQ controller, IPC, keypad, LCD status)
	template <int Cpu> uint32_t dispstat_r(offs_t offset, uint32_t mem_mask = ~0);
	template <int Cpu> void dispstat_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <int Cpu> uint32_t dma_r(offs_t offset, uint32_t mem_mask = ~0);
	template <int Cpu> void dma_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <int Cpu> uint32_t timer_r(offs_t offset, uint32_t mem_mask = ~0);
	template <int Cpu> void timer_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <int Cpu> uint32_t keyinput_r(offs_t offset, uint32_t mem_mask = ~0);
	template <int Cpu> void keyinput_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <int Cpu> uint32_t ipcsync_r(offs_t offset, uint32_t mem_mask = ~0);
	template <int Cpu> void ipcsync_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t auxspi_cnt_r(offs_t offset, uint16_t mem_mask = ~0);
	void auxspi_cnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t auxspi_data_r();
	void auxspi_data_w(uint16_t data);
	uint32_t gamecard_rom_ctrl_r(offs_t offset, uint32_t mem_mask = ~0);
	template <int Cpu> void gamecard_rom_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t gamecard_command_r(offs_t offset);
	void gamecard_command_w(offs_t offset, uint8_t data);
	uint16_t exmemcnt_r();
	template <int Cpu> void exmemcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <int Cpu> uint32_t ime_r(offs_t offset, uint32_t mem_mask = ~0);
	template <int Cpu> void ime_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <int Cpu> uint32_t ie_r();
	template <int Cpu> void ie_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <int Cpu> uint32_t if_r();
	template <int Cpu> void if_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// VRAM regions, in the order the banks can be assigned to them
	enum : int
	{
		VRAM_REGION_BG_A = 0,
		VRAM_REGION_OBJ_A,
		VRAM_REGION_BG_B,
		VRAM_REGION_OBJ_B,
		VRAM_REGION_ARM7,
		VRAM_REGION_BGEXTPAL_A,     // not CPU-visible: only the 2D engines read these
		VRAM_REGION_OBJEXTPAL_A,
		VRAM_REGION_BGEXTPAL_B,
		VRAM_REGION_OBJEXTPAL_B,
		VRAM_REGION_COUNT
	};

	static constexpr uint32_t VRAM_WORDS = 0xa4000 / 4;      // 656K, banks A..I back to back
	static constexpr int VRAM_PAGE_WORDS = 0x4000 / 4;       // banks are allocated in 16K pages
	static constexpr int VRAM_MAX_PAGES = 32;                // engine A BG is the largest region

	void update_vram_mapping();
	void map_vram_bank(int region, int page, int pages, uint32_t bankoff);
	uint32_t vram_r(int region, offs_t offset);
	void vram_w(int region, offs_t offset, uint32_t data, uint32_t mem_mask);

	template <int Region> uint32_t vram_region_r(offs_t offset) { return vram_r(Region, offset); }
	template <int Region> void vram_region_w(offs_t offset, uint32_t data, uint32_t mem_mask) { vram_w(Region, offset, data, mem_mask); }

	uint32_t vram_lcdc_r(offs_t offset);
	void vram_lcdc_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	std::unique_ptr<uint32_t[]> m_vram;
	uint32_t m_vram_page_off[VRAM_REGION_COUNT][VRAM_MAX_PAGES][4];
	uint8_t m_vram_page_count[VRAM_REGION_COUNT][VRAM_MAX_PAGES];
	bool m_vram_bank_mapped[9];

	uint32_t wram_first_half_r(offs_t offset);
	uint32_t wram_second_half_r(offs_t offset);
	void wram_first_half_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void wram_second_half_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t wram_arm7mirror_r(offs_t offset);
	void wram_arm7mirror_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void nds7_wram_map(address_map &map) ATTR_COLD;
	void nds9_wram_map(address_map &map) ATTR_COLD;
	void nds_arm7_map(address_map &map) ATTR_COLD;
	void nds_arm9_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// LCDs: each 2D engine renders into its own bitmap, POWCNT1 routes them to the screens
	virtual void video_start() override ATTR_COLD;
	void draw_scanline(int line);

	bitmap_rgb32 m_engine_bitmap[2];
	bitmap_rgb32 m_gba_work;                     // GBA mode: lines land here, copied out whole at V-blank

	required_device<arm7_cpu_device> m_arm7;
	required_device<arm946es_cpu_device> m_arm9;
	required_region_ptr<uint8_t> m_firmware;
	optional_region_ptr<uint8_t> m_gbabios;
	required_device<address_map_bank_device> m_arm7wrambnk, m_arm9wrambnk;
	required_shared_ptr<uint32_t> m_arm7ram;
	required_shared_ptr<uint32_t> m_palette, m_oam;
	required_device_array<screen_device, 2> m_screen;
	required_device_array<gba_ppu_device, 2> m_ppu;
	required_device<nds_sound_device> m_ndssound;
	required_device<s35180_device> m_rtc;
	required_device<gameboy_sound_device> m_gbsound;
	required_device_array<dac_8bit_r2r_twos_complement_device, 2> m_gba_ldac, m_gba_rdac;
	required_device<nds_cart_slot_device> m_ndscart;
	required_device<gba_cart_slot_device> m_gbacart;
	required_ioport m_keys, m_extkeys;
	required_ioport m_touch_x, m_touch_y;

	enum {
		VRAMCNT_A_OFFSET = (0x240/4),
		WRAMSTAT_OFFSET = (0x240/4),
		WRAMCNT_OFFSET = (0x244/4),
		VRAMCNT_H_OFFSET = (0x248/4),
		POSTFLG_PBF_SHIFT = 0,
		POSTFLG_RAM_SHIFT = 1,
		POSTFLG_PBF_MASK = (1 << POSTFLG_PBF_SHIFT),
		POSTFLG_RAM_MASK = (1 << POSTFLG_RAM_SHIFT),
		GAMECARD_DATA_READY = (1 << 23),
		GAMECARD_BLOCK_BUSY = (1 << 31)
	};

	uint8_t m_arm7_postflg;
	uint8_t m_arm9_postflg;
	uint32_t m_ime[2], m_ie[2], m_if[2];
	uint16_t m_ipcsync[2];
	uint32_t m_WRAM[0x8000/4];
	uint8_t m_wramcnt;
	uint8_t m_vramcnta, m_vramcntb, m_vramcntc, m_vramcntd;
	uint8_t m_vramcnte, m_vramcntf, m_vramcntg, m_vramcnth, m_vramcnti;
	uint16_t m_exmemcnt;
	uint16_t m_powcnt[2];
	uint32_t m_biosprot;
	bool m_halted[2];

	uint32_t m_sioregs[4];
	uint16_t m_rcnt;
	uint32_t m_card_seed[3];

	// LCD
	uint16_t m_dispstat[2];
	uint16_t m_vcount;
	emu_timer *m_scanline_timer;
	emu_timer *m_hblank_timer;
	TIMER_CALLBACK_MEMBER(scanline_tick);
	TIMER_CALLBACK_MEMBER(hblank_tick);

	// keypad
	uint16_t m_keycnt[2];
	void update_keypad_irq(int cpu);

	// interrupt controller
	void request_irq(int cpu, uint32_t int_type);
	void update_irqs(int cpu);
	void set_halted(int cpu, bool halted);

	// IPC FIFO
	uint16_t m_ipcfifocnt[2];
	uint32_t m_ipcfifo[2][16];
	uint32_t m_ipcfifo_last[2];
	uint8_t m_ipcfifo_head[2], m_ipcfifo_count[2];
	template <int Cpu> uint32_t ipcfifo_recv();
	template <int Cpu> void ipcfifo_send(uint32_t data);
	template <int Cpu> uint16_t ipcfifo_cnt_r();
	template <int Cpu> void ipcfifo_cnt_w(uint16_t data);

	// SPI bus
	uint16_t m_spi_cnt;
	uint8_t m_spi_data;
	uint8_t spi_transfer(uint8_t data);
	void spi_deselect();

	// SPI: firmware serial flash
	std::unique_ptr<uint8_t[]> m_fw_ram;
	uint8_t m_fw_cmd, m_fw_stat;
	uint32_t m_fw_addr;
	uint32_t m_fw_bytes;
	bool m_fw_powerdown;
	uint8_t firmware_spi_transfer(uint8_t data);

	// SPI: power management device
	uint8_t m_pm_regs[8];
	uint8_t m_pm_index;
	bool m_pm_have_index;
	uint8_t powerman_spi_transfer(uint8_t data);

	// SPI: touch screen controller
	uint16_t m_tsc_result;
	uint32_t m_tsc_pos;
	uint8_t tsc_spi_transfer(uint8_t data);
	bool touch_pressed() const;

	// the RTC port: the three chip lines plus their direction bits
	uint8_t m_rtc_io;
	uint8_t rtc_r();
	void rtc_w(uint8_t data);

	// GBA slot: EXMEMCNT bit 7 gives it to one CPU, the other one reads open bus
	template <int Cpu> uint32_t gba_rom_r(offs_t offset);
	template <int Cpu> uint32_t gba_ram_r(offs_t offset, uint32_t mem_mask = ~0);
	template <int Cpu> void gba_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// GBA mode: the ARM7 becomes a GBA (HALTCNT = 0x40), the ARM9 stops
	bool m_gba_mode;
	int m_total_lines;
	int m_visible_lines;
	uint32_t m_gba_soundregs[0x50/4];
	uint16_t m_gba_waitcnt;
	uint32_t m_gba_bios_prefetch;

	// GBA mode DirectSound: two FIFOs fed by DMA, drained to the DACs on timer overflow
	struct gba_fifo_t
	{
		int32_t ptr, in, size, remains;
		uint32_t sample;
		uint32_t word[8];
	};
	gba_fifo_t m_gba_fifo[2];

	void set_lcd_timing();
	void enter_gba_mode();
	void install_ds_arm7_map();
	void install_gba_map();
	uint32_t gba_pak_r(offs_t offset);
	uint32_t gba_bios_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t gba_open_bus_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t gba_sound_r(offs_t offset);
	void gba_sound_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void gba_audio_tick(int ref);
	uint16_t gba_ie_r();
	void gba_ie_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t gba_if_r();
	void gba_if_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t gba_waitcnt_r();
	void gba_waitcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t gba_postflg_r();
	void gba_postflg_w(uint8_t data);
	void gba_haltcnt_w(uint8_t data);
	uint32_t gba_memctrl_r();

	// gamecard
	uint16_t m_auxspicnt;
	uint8_t m_auxspidata;
	uint32_t m_romctrl;
	uint8_t m_card_command[8];
	uint32_t m_cartdata_len;
	uint8_t m_card_cpu;
	void gamecard_start_transfer();
	uint32_t gamecard_data_r();
	void gamecard_end_transfer();

	// DMA
	emu_timer *m_dma_timer[8];
	uint32_t m_dma_srcreg[8], m_dma_dstreg[8], m_dma_ctrl[8];
	uint32_t m_dma_src[8], m_dma_dst[8], m_dma_cnt[8];
	uint32_t m_dma_fill[4];
	void dma_control_w(int ch, uint32_t data, uint32_t mem_mask);
	bool dma_trigger(int cpu, int mode);
	void dma_exec(int ch);

	// Timers
	uint32_t m_timer_regs[8];
	uint16_t m_timer_reload[8];
	double m_timer_hz[8];
	attotime m_timer_start[8];

	emu_timer *m_tmr_timer[8];

	TIMER_CALLBACK_MEMBER(dma_complete);
	TIMER_CALLBACK_MEMBER(timer_expire);

	uint16_t timer_count_r(int timer);
	void timer_start(int timer, uint32_t old_regs, uint32_t data);
	void timer_tick_countup(int timer);

	// ARM9 divider and square root units
	uint16_t m_divcnt, m_sqrtcnt;
	uint64_t m_div_numer, m_div_denom;
	uint64_t m_div_result, m_divrem_result;
	uint64_t m_sqrt_param;
	uint32_t m_sqrt_result;
	void div_calculate();
	void sqrt_calculate();
};

#endif // MAME_NINTENDO_NDS_H
