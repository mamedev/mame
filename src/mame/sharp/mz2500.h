// license:BSD-3-Clause
// copyright-holders:Angelo Salese

// NOTE: we need this header, later "undumped" MZ-2800 supersets this with a 286

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
#include "sound/spkrdev.h"
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
	mz2500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_rtc(*this, RP5C15_TAG)
		, m_pit(*this, "pit")
		, m_dac1bit(*this, "dac1bit")
		, m_gfxdecode(*this, "gfxdecode")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_selected_floppy(nullptr)
		, m_joy(*this, "joy%u", 1U)
		, m_palette(*this, "palette")
		, m_rambank(*this, "rambank%u", 0)
		, m_tvram(*this, "tvram")
		, m_cgram(*this, "cgram")
		, m_wram(*this, "wram")
	{ }

	void mz2500(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(boot_reset_cb);
	DECLARE_INPUT_CHANGED_MEMBER(ipl_reset_cb);

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<rp5c15_device> m_rtc;
	required_device<pit8253_device> m_pit;
	required_device<speaker_sound_device> m_dac1bit;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<mb8876_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	floppy_image_device *m_selected_floppy;
	required_device_array<msx_general_purpose_port_device, 2> m_joy;
	required_device<palette_device> m_palette;
	required_device_array<address_map_bank_device, 8> m_rambank;
	required_shared_ptr<u8> m_tvram;
	required_shared_ptr<u8> m_cgram;
	required_shared_ptr<u8> m_wram;

	u8 *m_ipl_rom = nullptr;
	u8 *m_kanji_rom = nullptr;
	u8 *m_kanji2_rom = nullptr;
	std::unique_ptr<u8[]> m_pcg_ram;
	std::unique_ptr<u8[]> m_emm_ram;
	u8 *m_dic_rom = nullptr;
	u8 *m_phone_rom = nullptr;
	u8 *m_iplpro_rom = nullptr;

	emu_timer *m_ipl_reset_timer = nullptr;

	u8 m_bank_val[8]{};
	u8 m_bank_addr = 0;
	u8 m_irq_sel = 0;
	u8 m_irq_vector[4]{};
	u8 m_irq_mask[4]{};
	u8 m_irq_pending[4]{};
	u8 m_kanji_bank = 0;
	u8 m_dic_bank = 0;
	u8 m_fdc_reverse = 0;
	u8 m_key_mux = 0;
	u8 m_monitor_type = 0;
	u8 m_text_reg[0x100]{};
	u8 m_text_reg_index = 0;
	u8 m_text_col_size = 0;
	u8 m_text_font_reg = 0;
	u8 m_pal_select = 0;
	uint16_t m_cg_vs = 0;
	uint16_t m_cg_ve = 0;
	uint16_t m_cg_hs = 0;
	uint16_t m_cg_he = 0;
	int16_t m_tv_vs = 0;
	int16_t m_tv_ve = 0;
	int16_t m_tv_hs = 0;
	int16_t m_tv_he = 0;
	u8 m_cg_latch[4];
	u8 m_cg_reg_index = 0;
	u8 m_cg_reg[0x20];
	u8 m_clut16[0x10];
	uint16_t m_clut256[0x100];
	u8 m_cg_mask = 0;
	int m_scr_x_size = 0;
	int m_scr_y_size = 0;
	u8 m_cg_clear_flag = 0;
	uint32_t m_rom_index = 0;
	u8 m_hrom_index = 0;
	u8 m_lrom_index = 0;
	struct { u8 r = 0, g = 0, b = 0; } m_pal[16];
	u8 m_joy_mode = 0;
	uint16_t m_kanji_index = 0;
	uint32_t m_emm_offset = 0;
	u8 m_old_portc = 0;
	u8 m_prev_col_val = 0;
	u8 m_pio_latchb = 0;
	u8 m_ym_porta = 0;
	u8 m_screen_enable = 0;

	u8 bank_addr_r();
	void bank_addr_w(u8 data);
	u8 bank_data_r();
	void bank_data_w(u8 data);
	void bank_mode_w(u8 data);
	void kanji_bank_w(u8 data);
	void dictionary_bank_w(u8 data);
	u8 crtc_hvblank_r();
	void tv_crtc_w(offs_t offset, u8 data);
	void irq_sel_w(u8 data);
	void irq_data_w(u8 data);
	u8 rom_r(offs_t offset);
	void rom_w(offs_t offset, u8 data);
	void palette4096_io_w(offs_t offset, u8 data);
	u8 bplane_latch_r();
	u8 rplane_latch_r();
	u8 gplane_latch_r();
	u8 iplane_latch_r();
	void cg_addr_w(u8 data);
	void cg_data_w(u8 data);
	void timer_w(u8 data);
	u8 joystick_r();
	void joystick_w(u8 data);
	u8 kanji_r(offs_t offset);
	void kanji_w(offs_t offset, u8 data);
	u8 rp5c15_8_r(offs_t offset);
	void rp5c15_8_w(offs_t offset, u8 data);
	u8 emm_data_r(offs_t offset);
	void emm_address_w(offs_t offset, u8 data);
	void emm_data_w(offs_t offset, u8 data);

	u8 rmw_r(offs_t offset);
	void rmw_w(offs_t offset, u8 data);
	u8 kanji_pcg_r(offs_t offset);
	void kanji_pcg_w(offs_t offset, u8 data);
	u8 dict_rom_r(offs_t offset);

	TIMER_CALLBACK_MEMBER(ipl_timer_reset_cb);

	u8 cg_latch_compare();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void palette_init(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_cb);

	void floppy_select_w(u8 data);
	void floppy_side_w(u8 data);
	void floppy_dden_w(u8 data);

	u8 ppi_porta_r();
	u8 ppi_portb_r();
	u8 ppi_portc_r();
	void ppi_porta_w(u8 data);
	void ppi_portb_w(u8 data);
	void ppi_portc_w(u8 data);
	void pio_porta_w(u8 data);
	u8 pio_porta_r();
	u8 opn_porta_r();
	void opn_porta_w(u8 data);
	void pit8253_clk0_irq(int state);
	void rtc_alarm_irq(int state);
	IRQ_CALLBACK_MEMBER( irq_ack_cb );

	void draw_80x25(bitmap_ind16 &bitmap,const rectangle &cliprect,uint16_t map_addr);
	void draw_40x25(bitmap_ind16 &bitmap,const rectangle &cliprect,int plane,uint16_t map_addr);
	void draw_cg4_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri);
	void draw_cg16_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int plane,int x_size,int pri);
	void draw_cg256_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int plane,int pri);
	void draw_tv_screen(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void draw_cg_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri);

	void draw_pixel(bitmap_ind16 &bitmap,int x,int y,uint16_t  pen,u8 width,u8 height);
	void crtc_reconfigure_screen();
	static u8 pal_256_param(int index, int param);
	void reset_banks(u8 type);

	void z80_io(address_map &map) ATTR_COLD;
	void z80_map(address_map &map) ATTR_COLD;
	void bank_window_map(address_map &map) ATTR_COLD;
};

#endif // MAME_SHARP_MZ2500_H
