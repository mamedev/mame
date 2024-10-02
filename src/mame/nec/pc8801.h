// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/********************************************************************************************

    PC-8801 (c) 1981 NEC

********************************************************************************************/

#ifndef MAME_NEC_PC8801_H
#define MAME_NEC_PC8801_H

#pragma once

#include "pc8001.h"

#include "bus/centronics/ctronics.h"
#include "bus/msx/ctrl/ctrl.h"
#include "bus/pc8801/pc8801_31.h"
#include "bus/pc8801/pc8801_exp.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/i8214.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "machine/upd1990a.h"
#include "pc80s31k.h"
#include "sound/beep.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


#define I8214_TAG       "i8214"

class pc8801_state : public pc8001_base_state
{
public:
	pc8801_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc8001_base_state(mconfig, type, tag)
//      , m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_pc80s31(*this, "pc80s31")
		, m_pic(*this, I8214_TAG)
		, m_usart(*this, "usart")
//      , m_cassette(*this, "cassette")
		, m_beeper(*this, "beeper")
		, m_lspeaker(*this, "lspeaker")
		, m_rspeaker(*this, "rspeaker")
		, m_palette(*this, "palette")
		, m_n80rom(*this, "n80rom")
		, m_n88rom(*this, "n88rom")
//      , m_cg_rom(*this, "cgrom")
		, m_kanji_rom(*this, "kanji")
		, m_kanji_lv2_rom(*this, "kanji_lv2")
		, m_mouse_port(*this, "mouseport") // labelled "マウス" (mouse) - can't use "mouse" because of core -mouse option
		, m_exp(*this, "exp")
	{
	}

	void pc8801(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);

	virtual UPD3301_FETCH_ATTRIBUTE( attr_fetch ) override;

	virtual uint8_t dma_mem_r(offs_t offset) override;

	virtual uint8_t dictionary_rom_r(offs_t offset);
	virtual bool dictionary_rom_enable();

	virtual uint8_t cdbios_rom_r(offs_t offset);
	virtual bool cdbios_rom_enable();
	virtual void main_io(address_map &map) ATTR_COLD;

//  required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<pc80s31_device> m_pc80s31;
	optional_device<i8214_device> m_pic;
//  required_device<upd1990a_device> m_rtc;
	required_device<i8251_device> m_usart;
//  required_device<cassette_image_device> m_cassette;
	required_device<beep_device> m_beeper;
	required_device<speaker_device> m_lspeaker;
	required_device<speaker_device> m_rspeaker;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_n80rom;
	required_region_ptr<u8> m_n88rom;
//  required_region_ptr<u8> m_cg_rom;
	required_region_ptr<u8> m_kanji_rom;
	required_region_ptr<u8> m_kanji_lv2_rom;
	required_device<msx_general_purpose_port_device> m_mouse_port;
	required_device<pc8801_exp_slot_device> m_exp;

	void int4_irq_w(int state);

	uint8_t m_gfx_ctrl = 0;

private:
	void main_map(address_map &map) ATTR_COLD;

	std::unique_ptr<uint8_t[]> m_work_ram;
	std::unique_ptr<uint8_t[]> m_hi_work_ram;
	std::unique_ptr<uint8_t[]> m_ext_work_ram;
	std::unique_ptr<uint8_t[]> m_gvram;

	std::array<std::array<u16, 80>, 400> m_attr_info = {};

	uint8_t m_ext_rom_bank = 0;
	uint8_t m_vram_sel = 0;
	uint8_t m_misc_ctrl = 0;
	uint8_t m_device_ctrl_data = 0;
	uint8_t m_window_offset_bank = 0;
	bool m_text_layer_mask = false;
	u8 m_bitmap_layer_mask = 0;
	uint8_t m_alu_reg[3]{};
	uint8_t m_alu_ctrl1 = 0;
	uint8_t m_alu_ctrl2 = 0;
	uint8_t m_extram_mode = 0;
	uint8_t m_extram_bank = 0;
	uint32_t m_extram_size = 0;

	struct { uint8_t r = 0, g = 0, b = 0; } m_palram[8];
	enum {
		BGPAL_PEN = 8,
		BORDER_PEN = 9
	};

	uint32_t m_knj_addr[2]{};

	uint8_t alu_r(offs_t offset);
	void alu_w(offs_t offset, uint8_t data);
	uint8_t wram_r(offs_t offset);
	void wram_w(offs_t offset, uint8_t data);
	uint8_t ext_wram_r(offs_t offset);
	void ext_wram_w(offs_t offset, uint8_t data);
	uint8_t nbasic_rom_r(offs_t offset);
	uint8_t n88basic_rom_r(offs_t offset);
	uint8_t gvram_r(offs_t offset);
	void gvram_w(offs_t offset, uint8_t data);
	uint8_t high_wram_r(offs_t offset);
	void high_wram_w(offs_t offset, uint8_t data);
	uint8_t ext_rom_bank_r();
	void ext_rom_bank_w(uint8_t data);
//  void port30_w(uint8_t data);
	void port31_w(uint8_t data);
	uint8_t port40_r();
	void port40_w(uint8_t data);
	uint8_t vram_select_r();
	void vram_select_w(offs_t offset, uint8_t data);
	void irq_level_w(uint8_t data);
	void irq_mask_w(uint8_t data);
	uint8_t window_bank_r();
	void window_bank_w(uint8_t data);
	void window_bank_inc_w(uint8_t data);
	uint8_t misc_ctrl_r();
	void misc_ctrl_w(uint8_t data);
	void bgpal_w(uint8_t data);
	void palram_w(offs_t offset, uint8_t data);
	void layer_masking_w(uint8_t data);
	uint8_t extram_mode_r();
	void extram_mode_w(uint8_t data);
	uint8_t extram_bank_r();
	void extram_bank_w(uint8_t data);
	void alu_ctrl1_w(uint8_t data);
	void alu_ctrl2_w(uint8_t data);
	template <unsigned kanji_level> uint8_t kanji_r(offs_t offset);
	template <unsigned kanji_level> void kanji_w(offs_t offset, uint8_t data);
//  void rtc_w(uint8_t data);

	void txdata_callback(int state);

	// video section
	void draw_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, palette_device *palette, std::function<u8(u32 bitmap_offset, int y, int x, int xi)> dot_func);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void palette_reset();
	bitmap_rgb32 m_text_bitmap;

	// irq section
	void rxrdy_irq_w(int state);
	void vrtc_irq_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(clock_irq_w);
	IRQ_CALLBACK_MEMBER(int_ack_cb);
	void irq_w(int state);

	struct {
		u8 enable = 0, pending = 0;
	} m_irq_state;

	bool m_sound_irq_enable = false;
	bool m_sound_irq_pending = false;

	enum {
		RXRDY_IRQ_LEVEL = 0,
		VRTC_IRQ_LEVEL,
		CLOCK_IRQ_LEVEL,
		INT3_IRQ_LEVEL,
		INT4_IRQ_LEVEL,
		INT5_IRQ_LEVEL,
		FDCINT1_IRQ_LEVEL,
		FDCINT2_IRQ_LEVEL
	};

	void assert_irq(u8 level);
	void check_irq(u8 level);
};

class pc8801mk2sr_state : public pc8801_state
{
public:
	pc8801mk2sr_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc8801_state(mconfig, type, tag)
		, m_opn(*this, "opn")
	{ }

	void pc8801mk2sr(machine_config &config);
	void pc8801mk2mr(machine_config &config);

protected:
	virtual void main_io(address_map &map) override ATTR_COLD;

	uint8_t opn_porta_r();
	uint8_t opn_portb_r();
	void opn_portb_w(uint8_t data);

private:
	optional_device<ym2203_device> m_opn;
};

// both FH and MH family bases sports selectable 8/4 MHz CPU clock switch
class pc8801fh_state : public pc8801mk2sr_state
{
public:
	pc8801fh_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc8801mk2sr_state(mconfig, type, tag)
		, m_opna(*this, "opna")
	{ }

	void pc8801fh(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void main_io(address_map &map) override ATTR_COLD;

private:
	required_device<ym2608_device> m_opna;
	void opna_map(address_map &map) ATTR_COLD;

	uint8_t cpuclock_r();
	uint8_t baudrate_r();
	void baudrate_w(uint8_t data);

	uint8_t m_clock_setting = 0;
	uint8_t m_baudrate_val = 0;
};

class pc8801ma_state : public pc8801fh_state
{
public:
	pc8801ma_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc8801fh_state(mconfig, type, tag)
		, m_dictionary_rom(*this, "dictionary")
	{ }

	void pc8801ma(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void main_io(address_map &map) override ATTR_COLD;

	virtual uint8_t dictionary_rom_r(offs_t offset) override;
	virtual bool dictionary_rom_enable() override;

private:
	void dic_bank_w(uint8_t data);
	void dic_ctrl_w(uint8_t data);
	required_region_ptr<u8> m_dictionary_rom;

	uint8_t m_dic_ctrl = 0;
	uint8_t m_dic_bank = 0;
};

class pc8801mc_state : public pc8801ma_state
{
public:
	pc8801mc_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc8801ma_state(mconfig, type, tag)
		, m_cdrom_if(*this, "cdrom_if")
		, m_cdrom_bios(*this, "cdrom_bios")
	{ }

	void pc8801mc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void main_io(address_map &map) override ATTR_COLD;

private:
	virtual uint8_t cdbios_rom_r(offs_t offset) override;
	virtual bool cdbios_rom_enable() override;

	required_device<pc8801_31_device> m_cdrom_if;
	required_region_ptr<u8> m_cdrom_bios;

	bool m_cdrom_bank = true;
};

#endif // MAME_NEC_PC8801_H
