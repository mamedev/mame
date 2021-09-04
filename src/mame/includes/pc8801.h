// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/********************************************************************************************

    PC-8801 (c) 1981 NEC

********************************************************************************************/

#ifndef MAME_INCLUDES_PC8801_H
#define MAME_INCLUDES_PC8801_H

#pragma once

#include "cpu/z80/z80.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/i8214.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "machine/upd1990a.h"
#include "machine/pc80s31k.h"
#include "sound/beep.h"
#include "sound/ymopn.h"
#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#define USE_PROPER_I8214 0

#define I8214_TAG       "i8214"
#define UPD1990A_TAG    "upd1990a"
#define I8251_TAG       "i8251"

class pc8801_state : public driver_device
{
public:
	pc8801_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_pc80s31(*this, "pc80s31")
		, m_pic(*this, I8214_TAG)
		, m_rtc(*this, UPD1990A_TAG)
		, m_cassette(*this, "cassette")
		, m_beeper(*this, "beeper")
		, m_opna(*this, "opna")
		, m_opn(*this, "opn")
		, m_palette(*this, "palette")
	{ }

	void pc8801(machine_config &config);
	void pc8801mk2mr(machine_config &config);
	void pc8801fh(machine_config &config);
	void pc8801ma(machine_config &config);
	void pc8801mc(machine_config &config);

protected:
	virtual void video_start() override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<pc80s31_device> m_pc80s31;
	optional_device<i8214_device> m_pic;
	required_device<upd1990a_device> m_rtc;
	required_device<cassette_image_device> m_cassette;
	required_device<beep_device> m_beeper;
	required_device<ym2608_device> m_opna;
	required_device<ym2203_device> m_opn;
	required_device<palette_device> m_palette;

private:
	struct crtc_t
	{
		uint8_t cmd,param_count,cursor_on,status,irq_mask;
		uint8_t param[8][5];
		uint8_t inverse;
	};

	struct mouse_t
	{
		uint8_t phase;
		uint8_t x,y;
		attotime time;
	};

	std::unique_ptr<uint8_t[]> m_work_ram;
	std::unique_ptr<uint8_t[]> m_hi_work_ram;
	std::unique_ptr<uint8_t[]> m_ext_work_ram;
	std::unique_ptr<uint8_t[]> m_gvram;
	uint8_t *m_n80rom;
	uint8_t *m_n88rom;
	uint8_t *m_kanji_rom;
	uint8_t *m_cg_rom;

	uint8_t m_i8255_0_pc;
	uint8_t m_i8255_1_pc;
	uint8_t m_fdc_irq_opcode;
	uint8_t m_ext_rom_bank;
	uint8_t m_gfx_ctrl;
	uint8_t m_vram_sel;
	uint8_t m_misc_ctrl;
	uint8_t m_device_ctrl_data;
	uint8_t m_window_offset_bank;
	uint8_t m_layer_mask;
	uint16_t m_dma_counter[4];
	uint16_t m_dma_address[4];
	uint8_t m_alu_reg[3];
	uint8_t m_dmac_mode;
	uint8_t m_alu_ctrl1;
	uint8_t m_alu_ctrl2;
	uint8_t m_extram_mode;
	uint8_t m_extram_bank;
	uint8_t m_txt_width;
	uint8_t m_txt_color;
#if USE_PROPER_I8214
	uint8_t m_timer_irq_mask;
	uint8_t m_vblank_irq_mask;
	uint8_t m_sound_irq_mask;
	uint8_t m_int_state;
#else
	uint8_t m_i8214_irq_level;
	uint8_t m_vrtc_irq_mask;
	uint8_t m_vrtc_irq_latch;
	uint8_t m_timer_irq_mask;
	uint8_t m_timer_irq_latch;
	uint8_t m_sound_irq_mask;
	uint8_t m_sound_irq_latch;
	uint8_t m_sound_irq_pending;
#endif
	uint8_t m_has_clock_speed;
	uint8_t m_clock_setting;
	uint8_t m_baudrate_val;
	uint8_t m_has_dictionary;
	uint8_t m_dic_ctrl;
	uint8_t m_dic_bank;
	uint8_t m_has_cdrom;
	uint8_t m_cdrom_reg[0x10];
	crtc_t m_crtc;
	mouse_t m_mouse;
	struct { uint8_t r, g, b; } m_palram[8];
	uint8_t m_dmac_ff;
	uint32_t m_knj_addr[2];
	uint32_t m_extram_size;
	uint8_t m_has_opna;

	uint8_t pc8801_alu_r(offs_t offset);
	void pc8801_alu_w(offs_t offset, uint8_t data);
	uint8_t pc8801_wram_r(offs_t offset);
	void pc8801_wram_w(offs_t offset, uint8_t data);
	uint8_t pc8801_ext_wram_r(offs_t offset);
	void pc8801_ext_wram_w(offs_t offset, uint8_t data);
	uint8_t pc8801_nbasic_rom_r(offs_t offset);
	uint8_t pc8801_n88basic_rom_r(offs_t offset);
	uint8_t pc8801_gvram_r(offs_t offset);
	void pc8801_gvram_w(offs_t offset, uint8_t data);
	uint8_t pc8801_high_wram_r(offs_t offset);
	void pc8801_high_wram_w(offs_t offset, uint8_t data);
	uint8_t pc8801ma_dic_r(offs_t offset);
	uint8_t pc8801_cdbios_rom_r(offs_t offset);
	uint8_t pc8801_mem_r(offs_t offset);
	void pc8801_mem_w(offs_t offset, uint8_t data);
	uint8_t pc8801_ctrl_r();
	void pc8801_ctrl_w(uint8_t data);
	uint8_t pc8801_ext_rom_bank_r();
	void pc8801_ext_rom_bank_w(uint8_t data);
	void pc8801_gfx_ctrl_w(uint8_t data);
	uint8_t pc8801_vram_select_r();
	void pc8801_vram_select_w(offs_t offset, uint8_t data);
	void pc8801_irq_level_w(uint8_t data);
	void pc8801_irq_mask_w(uint8_t data);
	uint8_t pc8801_window_bank_r();
	void pc8801_window_bank_w(uint8_t data);
	void pc8801_window_bank_inc_w(uint8_t data);
	uint8_t pc8801_misc_ctrl_r();
	void pc8801_misc_ctrl_w(uint8_t data);
	void pc8801_bgpal_w(uint8_t data);
	void pc8801_palram_w(offs_t offset, uint8_t data);
	void pc8801_layer_masking_w(uint8_t data);
	uint8_t pc8801_crtc_param_r();
	void pc88_crtc_param_w(uint8_t data);
	uint8_t pc8801_crtc_status_r();
	void pc88_crtc_cmd_w(uint8_t data);
	uint8_t pc8801_dmac_r(offs_t offset);
	void pc8801_dmac_w(offs_t offset, uint8_t data);
	uint8_t pc8801_dmac_status_r();
	void pc8801_dmac_mode_w(uint8_t data);
	uint8_t pc8801_extram_mode_r();
	void pc8801_extram_mode_w(uint8_t data);
	uint8_t pc8801_extram_bank_r();
	void pc8801_extram_bank_w(uint8_t data);
	void pc8801_alu_ctrl1_w(uint8_t data);
	void pc8801_alu_ctrl2_w(uint8_t data);
	void pc8801_pcg8100_w(offs_t offset, uint8_t data);
	void pc8801_txt_cmt_ctrl_w(uint8_t data);
	uint8_t pc8801_kanji_r(offs_t offset);
	void pc8801_kanji_w(offs_t offset, uint8_t data);
	uint8_t pc8801_kanji_lv2_r(offs_t offset);
	void pc8801_kanji_lv2_w(offs_t offset, uint8_t data);
	void pc8801_dic_bank_w(uint8_t data);
	void pc8801_dic_ctrl_w(uint8_t data);
	uint8_t pc8801_cdrom_r();
	void pc8801_cdrom_w(offs_t offset, uint8_t data);
	uint8_t pc8801_cpuclock_r();
	uint8_t pc8801_baudrate_r();
	void pc8801_baudrate_w(uint8_t data);
	void pc8801_rtc_w(uint8_t data);
	void upd765_mc_w(uint8_t data);
	uint8_t upd765_tc_r();
	void fdc_irq_vector_w(uint8_t data);
	void fdc_drive_mode_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(txdata_callback);
	DECLARE_WRITE_LINE_MEMBER(rxrdy_w);
	uint8_t pc8801_sound_board_r(offs_t offset);
	void pc8801_sound_board_w(offs_t offset, uint8_t data);
	uint8_t pc8801_opna_r(offs_t offset);
	void pc8801_opna_w(offs_t offset, uint8_t data);
	uint8_t pc8801_unk_r();
	void pc8801_unk_w(uint8_t data);

	uint8_t pc8801_pixel_clock(void);
	void pc8801_dynamic_res_change(void);
	void draw_bitmap_3bpp(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void draw_bitmap_1bpp(bitmap_ind16 &bitmap,const rectangle &cliprect);
	uint8_t calc_cursor_pos(int x,int y,int yi);
	uint8_t extract_text_attribute(uint32_t address,int x, uint8_t width, uint8_t &non_special);
	void pc8801_draw_char(bitmap_ind16 &bitmap,int x,int y,int pal,uint8_t gfx_mode,uint8_t reverse,uint8_t secret,
							uint8_t blink,uint8_t upper,uint8_t lower,int y_size,int width, uint8_t non_special);
	void draw_text(bitmap_ind16 &bitmap,int y_size, uint8_t width);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void pc8801_palette(palette_device &palette) const;
	void pc8801_io(address_map &map);
	void pc8801_mem(address_map &map);
	void pc8801fdc_io(address_map &map);
	void pc8801fdc_mem(address_map &map);
	DECLARE_MACHINE_RESET(pc8801_clock_speed);
	DECLARE_MACHINE_RESET(pc8801_dic);
	DECLARE_MACHINE_RESET(pc8801_cdrom);
	INTERRUPT_GEN_MEMBER(pc8801_vrtc_irq);
	TIMER_CALLBACK_MEMBER(pc8801fd_upd765_tc_to_zero);
	TIMER_DEVICE_CALLBACK_MEMBER(pc8801_rtc_irq);
	uint8_t cpu_8255_c_r();
	void cpu_8255_c_w(uint8_t data);
	uint8_t fdc_8255_c_r();
	void fdc_8255_c_w(uint8_t data);
	uint8_t opn_porta_r();
	uint8_t opn_portb_r();
	void opna_map(address_map &map);
	IRQ_CALLBACK_MEMBER(pc8801_irq_callback);
	DECLARE_WRITE_LINE_MEMBER(pc8801_sound_irq);
};

#endif // MAME_INCLUDES_PC8801_H
