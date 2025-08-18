// license:BSD-3-Clause
// copyright-holders:Daniel Tremblay
#ifndef MAME_F256_F256_H
#define MAME_F256_F256_H

#pragma once

#include "machine/ram.h"
#include "screen.h"
#include "machine/bq4847.h"
#include "machine/6522via.h"
#include "sound/sn76496.h"
#include "sound/ymopl.h"
#include "sound/mos6581.h"
#include "utf8.h"
#include "tiny_vicky.h"
#include "speaker.h"
#include "machine/spi_sdcard.h"
// PS/2 mouse and keyboard
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/hle_mouse.h"
#include "debug/debugcon.h"
#include "debugger.h"
//#include "bus/cbmiec/cbmiec.h"
//#include "bus/cbmiec/c1581.h"
#include "machine/ins8250.h"

#define MASTER_CLOCK        (XTAL(25'175'000))
#define MUSIC_CLOCK         (XTAL(14'318'181))
#define MAINCPU_TAG                 "maincpu"
#define RAM_TAG                     "ram"
#define IOPAGE_TAG                  "iopage"
#define ROM_TAG                     "rom"
#define FONT_TAG                    "font"
#define FLASH_TAG                   "flash"
#define VICKY_VIDEO_TAG             "vicky"
#define SCREEN_TAG                  "screen"

class f256_state : public driver_device
{
public:
	f256_state(const machine_config &mconfig, device_type type, const char *tag);
	~f256_state();
    void f256k(machine_config &config);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
    required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device_array<ram_device, 4> m_iopage;
	required_memory_region m_rom;
	required_memory_region m_font;
	required_device<screen_device> m_screen;
	required_device<bq4802_device> m_rtc;
	required_ioport_array<8> m_keyboard; // the number 8 will require 8 COL
	required_device<via6522_device> m_via6522_0, m_via6522_1;
	required_device<sn76489_device> m_sn0, m_sn1;
	required_device<ymf262_device> m_opl3;
	required_device<mos6581_device> m_sid0, m_sid1;
	required_ioport m_joy1, m_joy2;

	required_device<tiny_vicky_video_device> m_video;
	//required_device<cbm_iec_device> m_iec;
	optional_device<pc_kbdc_device> m_ps2_keyboard;
	optional_device<hle_ps2_mouse_device> m_mouse;
	required_device<ns16550_device> m_uart;

	// SD Card stuff
	TIMER_CALLBACK_MEMBER(spi_clock);
	required_device<spi_sdcard_device> m_sdcard;
	emu_timer *m_spi_clock;
	bool m_spi_clock_state;
	bool m_spi_clock_sysclk;
	int m_spi_clock_cycles;
	int m_in_bit = 0;
	u8 spi_sd_enabled = 0;
	uint8_t m_in_latch = 0;
	uint8_t m_out_latch = 0;
	// End of SD Card stuff

    void program_map(address_map &map);
	void data_map(address_map &map);

	uint8_t m_mmu_reg, m_ioreg;
	uint8_t mmu_lut[32];
	void reset_mmu();
	u8   lut_r(offs_t offset);
	void lut_w(offs_t offset, u8 data);
	u8   mem_r(offs_t offset);
	void mem_w(offs_t offset, u8 data);

	// screen update
	void sof_interrtupt(int state);
	void sol_interrtupt(int state);
	void rtc_interrupt_handler(int state);
	void via0_interrupt(int state);
	void via1_interrupt(int state);
	uint8_t m_interrupt_reg[3] = { 0, 0 ,0};
	uint8_t m_interrupt_masks[3] = { 0xFF, 0xFF, 0xFF};
	uint8_t m_interrupt_edge[3] = { 0xFF, 0xFF, 0xFF};
	uint8_t m_interrupt_polarity[3] = {0, 0, 0};

	// VIA0 - Atari joystick functions
	u8 via0_system_porta_r();
	u8 via0_system_portb_r();
	void via0_system_porta_w(u8 data);
	void via0_system_portb_w(u8 data);
	void via0_ca2_write(u8 data);
	void via0_cb2_write(u8 data);

	// VIA1 - Internal Keyboard
	uint8_t m_via_keyboard_port_a = 0xFF;
	uint8_t m_via_keyboard_port_b = 0xFF;
	uint8_t m_via_joy1 = 0xFF;

	u8 via1_system_porta_r();
	u8 via1_system_portb_r();
	void via1_system_porta_w(u8 data);
	void via1_system_portb_w(u8 data);
	void via1_ca2_write(u8 data);
	void via1_cb2_write(u8 data);

	// codec
	uint8_t m_codec[16] = {};
	TIMER_CALLBACK_MEMBER(codec_done);

	// PS/2
	uint8_t m_ps2[16] = {};
	bool isK_WR = false;
	bool isM_WR = false;
	bool K_AK = false;
	bool M_AK = false;

	uint8_t kbPacketCntr = 0;
	uint8_t msPacketCntr = 0;
	int kbQLen = 0;
	int msQLen = 0;
	uint8_t kbFifo[6] = {};
	uint8_t msFifo[3] = {};

	// mouse
	// uint16_t m_mouse_x = 0, m_mouse_y = 0;
	// bool m_mouse_enabled = false;
	// uint8_t m_mouse_mode = 0;

	// SDCard
	void write_sd_control(u8 ctrl);
	void write_sd_data(u8 data);

	// Math Copro
	uint32_t m_multiplication_result = 0;
	uint32_t m_addition_result = 0;
	uint16_t m_division_result = 0;
	uint16_t m_division_remainder = 0;
	void unsignedMultiplier(int baseAddr);
	void unsignedDivider(int baseAddr);
	void unsignedAdder(int baseAddr);

	// Random Number Generator (RNG)
	uint16_t m_seed = 0;
	bool m_rng_enabled = false;
	uint8_t get_random();

	// DMA
	uint8_t m_dma_status = 0;
	void perform2DFillDMA();
    void performLinearFillDMA();
    void perform2DDMA();
    void performLinearDMA();
	void dma_interrupt_handler(int state);

	// Timers
	uint8_t m_timer0_eq = 0;
	uint32_t m_timer0_load = 0;
	uint32_t m_timer0_val = 0;
	TIMER_CALLBACK_MEMBER(timer0);
	emu_timer *m_timer0;
	void timer0_interrupt_handler(int state);

	uint8_t m_timer1_eq = 0;
	uint32_t m_timer1_load = 0;
	uint32_t m_timer1_val = 0;
	TIMER_CALLBACK_MEMBER(timer1);
	emu_timer *m_timer1;
	void timer1_interrupt_handler(int state);

	uint16_t m_opl3_reg = 0;

	// IEC
	inline void update_iec();
	void iec_srq_w(int state);
	void iec_data_w(int state);
	void iec_atn_w(int state);
	void iec_clk_w(int state);
	int m_iec_data_out;
	int m_iec_srq;
	uint8_t m_iec_in, m_iec_out;
};

#endif // MAME_F256_F256_H
