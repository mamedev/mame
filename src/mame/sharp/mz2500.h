// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/********************************************************************************************************************************

    Sharp MZ-2500 (c) 1985 Sharp Corporation

********************************************************************************************************************************/
#ifndef MAME_SHARP_MZ2500_H
#define MAME_SHARP_MZ2500_H

#pragma once

#include "bus/msx/ctrl/ctrl.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/rp5c15.h"
#include "machine/wd_fdc.h"
#include "machine/z80sio.h"
#include "machine/z80pio.h"
#include "sound/beep.h"
#include "sound/ymopn.h"
#include "machine/bankdev.h"
#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

//#include "imagedev/cassette.h"
#include "imagedev/floppy.h"

#define RP5C15_TAG      "rp5c15"

class mz2500_state : public driver_device
{
public:
	mz2500_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_rtc(*this, RP5C15_TAG),
		m_pit(*this, "pit"),
		m_beeper(*this, "beeper"),
		m_gfxdecode(*this, "gfxdecode"),
		m_fdc(*this, "mb8876"),
		m_floppy(*this, "mb8876:%u", 0U),
		m_selected_floppy(nullptr),
		m_joy(*this, "joy%u", 1U),
		m_palette(*this, "palette"),
		m_rambank(*this, "rambank%u", 0),
		m_tvram(*this, "tvram"),
		m_cgram(*this, "cgram"),
		m_wram(*this, "wram")
	{ }

	void mz2500(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<rp5c15_device> m_rtc;
	required_device<pit8253_device> m_pit;
	required_device<beep_device> m_beeper;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<mb8876_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	floppy_image_device *m_selected_floppy;
	required_device_array<msx_general_purpose_port_device, 2> m_joy;
	required_device<palette_device> m_palette;
	required_device_array<address_map_bank_device, 8> m_rambank;
	required_shared_ptr<uint8_t> m_tvram;
	required_shared_ptr<uint8_t> m_cgram;
	required_shared_ptr<uint8_t> m_wram;

	uint8_t *m_ipl_rom = nullptr;
	uint8_t *m_kanji_rom = nullptr;
	uint8_t *m_kanji2_rom = nullptr;
	std::unique_ptr<uint8_t[]> m_pcg_ram;
	std::unique_ptr<uint8_t[]> m_emm_ram;
	uint8_t *m_dic_rom = nullptr;
	uint8_t *m_phone_rom = nullptr;
	uint8_t *m_iplpro_rom = nullptr;

	uint8_t m_bank_val[8]{};
	uint8_t m_bank_addr = 0;
	uint8_t m_irq_sel = 0;
	uint8_t m_irq_vector[4]{};
	uint8_t m_irq_mask[4]{};
	uint8_t m_irq_pending[4]{};
	uint8_t m_kanji_bank = 0;
	uint8_t m_dic_bank = 0;
	uint8_t m_fdc_reverse = 0;
	uint8_t m_key_mux = 0;
	uint8_t m_monitor_type = 0;
	uint8_t m_text_reg[0x100]{};
	uint8_t m_text_reg_index = 0;
	uint8_t m_text_col_size = 0;
	uint8_t m_text_font_reg = 0;
	uint8_t m_pal_select = 0;
	uint16_t m_cg_vs = 0;
	uint16_t m_cg_ve = 0;
	uint16_t m_cg_hs = 0;
	uint16_t m_cg_he = 0;
	int16_t m_tv_vs = 0;
	int16_t m_tv_ve = 0;
	int16_t m_tv_hs = 0;
	int16_t m_tv_he = 0;
	uint8_t m_cg_latch[4];
	uint8_t m_cg_reg_index = 0;
	uint8_t m_cg_reg[0x20];
	uint8_t m_clut16[0x10];
	uint16_t m_clut256[0x100];
	uint8_t m_cg_mask = 0;
	int m_scr_x_size = 0;
	int m_scr_y_size = 0;
	uint8_t m_cg_clear_flag = 0;
	uint32_t m_rom_index = 0;
	uint8_t m_hrom_index = 0;
	uint8_t m_lrom_index = 0;
	struct { uint8_t r = 0, g = 0, b = 0; } m_pal[16];
	uint8_t m_joy_mode = 0;
	uint16_t m_kanji_index = 0;
	uint32_t m_emm_offset = 0;
	uint8_t m_old_portc = 0;
	uint8_t m_prev_col_val = 0;
	uint8_t m_pio_latchb = 0;
	uint8_t m_ym_porta = 0;
	uint8_t m_screen_enable = 0;
	uint8_t mz2500_bank_addr_r();
	void mz2500_bank_addr_w(uint8_t data);
	uint8_t mz2500_bank_data_r();
	void mz2500_bank_data_w(uint8_t data);
	void mz2500_kanji_bank_w(uint8_t data);
	void mz2500_dictionary_bank_w(uint8_t data);
	uint8_t mz2500_crtc_hvblank_r();
	void mz2500_tv_crtc_w(offs_t offset, uint8_t data);
	void mz2500_irq_sel_w(uint8_t data);
	void mz2500_irq_data_w(uint8_t data);
	uint8_t mz2500_rom_r();
	void mz2500_rom_w(uint8_t data);
	void palette4096_io_w(uint8_t data);
	uint8_t mz2500_bplane_latch_r();
	uint8_t mz2500_rplane_latch_r();
	uint8_t mz2500_gplane_latch_r();
	uint8_t mz2500_iplane_latch_r();
	void mz2500_cg_addr_w(uint8_t data);
	void mz2500_cg_data_w(uint8_t data);
	void timer_w(uint8_t data);
	uint8_t mz2500_joystick_r();
	void mz2500_joystick_w(uint8_t data);
	uint8_t mz2500_kanji_r(offs_t offset);
	void mz2500_kanji_w(offs_t offset, uint8_t data);
	uint8_t rp5c15_8_r();
	void rp5c15_8_w(uint8_t data);
	uint8_t mz2500_emm_data_r();
	void mz2500_emm_addr_w(uint8_t data);
	void mz2500_emm_data_w(uint8_t data);

	uint8_t rmw_r(offs_t offset);
	void rmw_w(offs_t offset, uint8_t data);
	uint8_t kanji_pcg_r(offs_t offset);
	void kanji_pcg_w(offs_t offset, uint8_t data);
	uint8_t dict_rom_r(offs_t offset);

	uint8_t mz2500_cg_latch_compare();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void mz2500_palette(palette_device &palette) const;
	uint32_t screen_update_mz2500(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(mz2500_vbl);

	void floppy_select_w(uint8_t data);
	void floppy_side_w(uint8_t data);
	void floppy_dden_w(uint8_t data);

	uint8_t mz2500_porta_r();
	uint8_t mz2500_portb_r();
	uint8_t mz2500_portc_r();
	void mz2500_porta_w(uint8_t data);
	void mz2500_portb_w(uint8_t data);
	void mz2500_portc_w(uint8_t data);
	void mz2500_pio1_porta_w(uint8_t data);
	uint8_t mz2500_pio1_porta_r();
	uint8_t opn_porta_r();
	void opn_porta_w(uint8_t data);
	void pit8253_clk0_irq(int state);
	void mz2500_rtc_alarm_irq(int state);
	IRQ_CALLBACK_MEMBER( mz2500_irq_ack );

	void draw_80x25(bitmap_ind16 &bitmap,const rectangle &cliprect,uint16_t map_addr);
	void draw_40x25(bitmap_ind16 &bitmap,const rectangle &cliprect,int plane,uint16_t map_addr);
	void draw_cg4_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri);
	void draw_cg16_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int plane,int x_size,int pri);
	void draw_cg256_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int plane,int pri);
	void draw_tv_screen(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void draw_cg_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri);

	void mz2500_draw_pixel(bitmap_ind16 &bitmap,int x,int y,uint16_t  pen,uint8_t width,uint8_t height);
	void mz2500_reconfigure_screen();
	static uint8_t pal_256_param(int index, int param);
	void reset_banks(uint8_t type);

	void mz2500_io(address_map &map) ATTR_COLD;
	void mz2500_map(address_map &map) ATTR_COLD;
	void mz2500_bank_window_map(address_map &map) ATTR_COLD;
};

#endif // MAME_SHARP_MZ2500_H
