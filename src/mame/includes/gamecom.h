// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Robbbert
/*****************************************************************************
 *
 * includes/gamecom.h
 *
 * Tiger Game.com
 *
 * Driver by Wilbert Pol
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_GAMECOM_H
#define MAME_INCLUDES_GAMECOM_H

#pragma once

#include "cpu/sm8500/sm8500.h"
#include "sound/dac.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "machine/nvram.h"
#include "emupal.h"

/* SM8521 register addresses */
enum
{
	SM8521_R0       = 0x00,
	SM8521_R1       = 0x01,
	SM8521_R2       = 0x02,
	SM8521_R3       = 0x03,
	SM8521_R4       = 0x04,
	SM8521_R5       = 0x05,
	SM8521_R6       = 0x06,
	SM8521_R7       = 0x07,
	SM8521_R8       = 0x08,
	SM8521_R9       = 0x09,
	SM8521_R10      = 0x0A,
	SM8521_R11      = 0x0B,
	SM8521_R12      = 0x0C,
	SM8521_R13      = 0x0D,
	SM8521_R14      = 0x0E,
	SM8521_R15      = 0x0F,
	SM8521_IE0      = 0x10,
	SM8521_IE1      = 0x11,
	SM8521_IR0      = 0x12,
	SM8521_IR1      = 0x13,
	SM8521_P0       = 0x14,
	SM8521_P1       = 0x15,
	SM8521_P2       = 0x16,
	SM8521_P3       = 0x17,
	SM8521_18       = 0x18, /* reserved */
	SM8521_SYS      = 0x19,
	SM8521_CKC      = 0x1A,
	SM8521_1B       = 0x1B, /* reserved */
	SM8521_SPH      = 0x1C,
	SM8521_SPL      = 0x1D,
	SM8521_PS0      = 0x1E,
	SM8521_PS1      = 0x1F,
	SM8521_P0C      = 0x20,
	SM8521_P1C      = 0x21,
	SM8521_P2C      = 0x22,
	SM8521_P3C      = 0x23,
	SM8521_MMU0     = 0x24,
	SM8521_MMU1     = 0x25,
	SM8521_MMU2     = 0x26,
	SM8521_MMU3     = 0x27,
	SM8521_MMU4     = 0x28,
	SM8521_29       = 0x29, /* reserved */
	SM8521_2A       = 0x2A, /* reserved */
	SM8521_URTT     = 0x2B,
	SM8521_URTR     = 0x2C,
	SM8521_URTS     = 0x2D,
	SM8521_URTC     = 0x2E,
	SM8521_2F       = 0x2F, /* reserved */
	SM8521_LCDC     = 0x30,
	SM8521_LCH      = 0x31,
	SM8521_LCV      = 0x32,
	SM8521_33       = 0x33, /* reserved */
	SM8521_DMC      = 0x34,
	SM8521_DMX1     = 0x35,
	SM8521_DMY1     = 0x36,
	SM8521_DMDX     = 0x37,
	SM8521_DMDY     = 0x38,
	SM8521_DMX2     = 0x39,
	SM8521_DMY2     = 0x3A,
	SM8521_DMPL     = 0x3B,
	SM8521_DMBR     = 0x3C,
	SM8521_DMVP     = 0x3D,
	SM8521_3E       = 0x3E, /* reserved */
	SM8521_3F       = 0x3F, /* reserved */
	SM8521_SGC      = 0x40,
	SM8521_41       = 0x41, /* reserved */
	SM8521_SG0L     = 0x42,
	SM8521_43       = 0x43, /* reserved */
	SM8521_SG1L     = 0x44,
	SM8521_45       = 0x45, /* reserved */
	SM8521_SG0TH    = 0x46,
	SM8521_SG0TL    = 0x47,
	SM8521_SG1TH    = 0x48,
	SM8521_SG1TL    = 0x49,
	SM8521_SG2L     = 0x4A,
	SM8521_4B       = 0x4B, /* reserved */
	SM8521_SG2TH    = 0x4C,
	SM8521_SG2TL    = 0x4D,
	SM8521_SGDA     = 0x4E,
	SM8521_4F       = 0x4F, /* reserved */
	SM8521_TM0C     = 0x50,
	SM8521_TM0D     = 0x51,
	SM8521_TM1C     = 0x52,
	SM8521_TM1D     = 0x53,
	SM8521_CLKT     = 0x54,
	SM8521_55       = 0x55, /* reserved */
	SM8521_56       = 0x56, /* reserved */
	SM8521_57       = 0x57, /* reserved */
	SM8521_58       = 0x58, /* reserved */
	SM8521_59       = 0x59, /* reserved */
	SM8521_5A       = 0x5A, /* reserved */
	SM8521_5B       = 0x5B, /* reserved */
	SM8521_5C       = 0x5C, /* reserved */
	SM8521_5D       = 0x5D, /* reserved */
	SM8521_WDT      = 0x5E,
	SM8521_WDTC     = 0x5F,
	SM8521_SG0W0    = 0x60,
	SM8521_SG0W1    = 0x61,
	SM8521_SG0W2    = 0x62,
	SM8521_SG0W3    = 0x63,
	SM8521_SG0W4    = 0x64,
	SM8521_SG0W5    = 0x65,
	SM8521_SG0W6    = 0x66,
	SM8521_SG0W7    = 0x67,
	SM8521_SG0W8    = 0x68,
	SM8521_SG0W9    = 0x69,
	SM8521_SG0W10   = 0x6A,
	SM8521_SG0W11   = 0x6B,
	SM8521_SG0W12   = 0x6C,
	SM8521_SG0W13   = 0x6D,
	SM8521_SG0W14   = 0x6E,
	SM8521_SG0W15   = 0x6F,
	SM8521_SG1W0    = 0x70,
	SM8521_SG1W1    = 0x71,
	SM8521_SG1W2    = 0x72,
	SM8521_SG1W3    = 0x73,
	SM8521_SG1W4    = 0x74,
	SM8521_SG1W5    = 0x75,
	SM8521_SG1W6    = 0x76,
	SM8521_SG1W7    = 0x77,
	SM8521_SG1W8    = 0x78,
	SM8521_SG1W9    = 0x79,
	SM8521_SG1W10   = 0x7A,
	SM8521_SG1W11   = 0x7B,
	SM8521_SG1W12   = 0x7C,
	SM8521_SG1W13   = 0x7D,
	SM8521_SG1W14   = 0x7E,
	SM8521_SG1W15   = 0x7F
};

struct GAMECOM_DMA
{
	u8 width_x = 0U;
	u8 width_y = 0U;
	u8 source_x = 0U;
	u8 source_x_current = 0U;
	u8 source_y = 0U;
	u8 source_width = 0U;
	u8 dest_x = 0U;
	u8 dest_x_current = 0U;
	u8 dest_y = 0U;
	u8 dest_width = 0U;
	u8 palette = 0U;
	u8 block_width = 0U;
	u8 block_height = 0U;
	u8 *source_bank = 0U;
	u16 source_current = 0U;
	u16 source_line = 0U;
	u16 source_mask = 0U;
	u8 *dest_bank = 0;
	u16 dest_current = 0U;
	u16 dest_line = 0U;
	u16 dest_mask = 0U;
	u8 transfer_mode = 0U;
	s16 adjust_x = 0U;
	bool decrement_y = 0;
	bool overwrite_mode = 0;
};

struct GAMECOM_TIMER
{
	bool enabled = 0;
	u32 prescale_count = 0U;
	u32 prescale_max = 0U;
	u8 upcounter_max = 0U;
};

struct gamecom_sound_t
{
	uint8_t sgc = 0U;
	uint8_t sg0l = 0U;
	uint8_t sg1l = 0U;
	uint8_t sg2l = 0U;
	uint16_t sg0t = 0U;
	uint16_t sg1t = 0U;
	uint16_t sg2t = 0U;
	uint8_t sgda = 0U;
	uint8_t sg0w[16]{};
	uint8_t sg1w[16]{};
};


class gamecom_state : public driver_device
{
public:
	gamecom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this,"videoram")
		, m_p_nvram(*this,"nvram")
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_dac(*this, "dac")
		, m_dac0(*this, "dac0")
		, m_dac1(*this, "dac1")
		, m_cart1(*this, "cartslot1")
		, m_cart2(*this, "cartslot2")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
		, m_share_maincpu(*this, "maincpu")
		, m_region_kernel(*this, "kernel")
		, m_io_in0(*this, "IN0")
		, m_io_in1(*this, "IN1")
		, m_io_in2(*this, "IN2")
		, m_io_grid(*this, "GRID.%u", 0U)
	{
	}

	void gamecom(machine_config &config);

	void init_gamecom();

private:

	uint8_t gamecom_internal_r(offs_t offset);
	uint8_t gamecom_pio_r(offs_t offset);
	void gamecom_internal_w(offs_t offset, uint8_t data);
	void gamecom_pio_w(offs_t offset, uint8_t data);
	void gamecom_palette(palette_device &palette) const;
	INTERRUPT_GEN_MEMBER(gamecom_interrupt);
	TIMER_CALLBACK_MEMBER(gamecom_clock_timer_callback);
	TIMER_CALLBACK_MEMBER(gamecom_sound0_timer_callback);
	TIMER_CALLBACK_MEMBER(gamecom_sound1_timer_callback);
	TIMER_CALLBACK_MEMBER(gamecom_scanline);
	void gamecom_handle_dma(uint8_t data);
	void gamecom_update_timers(uint8_t data);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( cart1_load );
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( cart2_load );
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gamecom_mem_map(address_map &map);

	uint8_t *m_p_ram = 0;
	uint8_t *m_cart_ptr = 0;
	uint8_t m_lcdc_reg = 0U;
	uint8_t m_lch_reg = 0U;
	uint8_t m_lcv_reg = 0U;
	uint8_t m_sound0_cnt = 0U;
	uint8_t m_sound1_cnt = 0U;
	uint16_t m_scanline = 0U;
	uint16_t m_base_address = 0U;
	memory_region *m_cart1_rom = 0;
	memory_region *m_cart2_rom = 0;
	emu_timer *m_clock_timer = 0;
	emu_timer *m_sound0_timer = 0;
	emu_timer *m_sound1_timer = 0;
	emu_timer *m_scanline_timer = 0;
	GAMECOM_DMA m_dma;
	GAMECOM_TIMER m_timer[2]{};
	gamecom_sound_t m_sound;
	bitmap_ind16 m_bitmap = 0;
	void gamecom_set_mmu(uint8_t mmu, uint8_t data);
	void handle_stylus_press(int column);
	void recompute_lcd_params();
	void handle_input_press(uint16_t mux_data);
	image_init_result common_load(device_image_interface &image, generic_slot_device *slot);
	virtual void machine_reset() override;
	virtual void video_start() override;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_shared_ptr<uint8_t> m_p_nvram;
	required_device<sm8500_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<dac_byte_interface> m_dac;
	required_device<dac_byte_interface> m_dac0;
	required_device<dac_byte_interface> m_dac1;
	required_device<generic_slot_device> m_cart1;
	required_device<generic_slot_device> m_cart2;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_shared_ptr<u8> m_share_maincpu;
	required_memory_region m_region_kernel;
	required_ioport m_io_in0;
	required_ioport m_io_in1;
	required_ioport m_io_in2;
	required_ioport_array<13> m_io_grid;
};

#endif // MAME_INCLUDES_GAMECOM_H
