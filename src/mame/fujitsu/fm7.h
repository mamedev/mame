// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *
 *  FM-7 header file
 *
 */
#ifndef MAME_FUJITSU_FM7_H
#define MAME_FUJITSU_FM7_H

#pragma once


#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "sound/ay8910.h"
#include "sound/beep.h"
#include "sound/ymopn.h"
#include "machine/wd_fdc.h"
#include "machine/bankdev.h"
#include "emupal.h"


class fm7_state : public driver_device
{
public:
	fm7_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_shared_ram(*this, "shared_ram"),
		m_a15_ram(*this, "a15_ram"),
		m_vectors(*this, "vectors"),
		m_maincpu(*this, "maincpu"),
		m_sub(*this, "sub"),
		m_cassette(*this, "cassette"),
		m_beeper(*this, "beeper"),
		m_psg(*this, "psg"),
		m_screen(*this, "screen"),
		m_centronics(*this, "centronics"),
		m_cent_data_out(*this, "cent_data_out"),
		m_fdc(*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_floppy1(*this, "fdc:1"),
		m_floppy(nullptr),
		m_rom_ptr(*this, "init"),
		m_btrom_ptr(*this, "boot"),
		m_basic_ptr(*this, "fbasic"),
		m_kanji(*this, "kanji1"),
		m_kb_ports(*this, "key%u", 1),
		m_keymod(*this, "key_modifiers"),
		m_joy1(*this, "joy1"),
		m_dsw(*this, "DSW"),
		m_palette(*this, "palette"),
		m_av_palette(*this, "av_palette")
	{
	}

	void fm16beta(machine_config &config);
	void fm8(machine_config &config);
	void fm7(machine_config &config);

	void init_fm7();

protected:
	// Interrupt flags
	enum : uint8_t
	{
		IRQ_FLAG_KEY     = 0x01,
		IRQ_FLAG_PRINTER = 0x02,
		IRQ_FLAG_TIMER   = 0x04,
		IRQ_FLAG_OTHER   = 0x08,
		// the following are not read in port 0xfd03
		IRQ_FLAG_MFD     = 0x10,
		IRQ_FLAG_TXRDY   = 0x20,
		IRQ_FLAG_RXRDY   = 0x40,
		IRQ_FLAG_SYNDET  = 0x80
	};

	// system types
	enum
	{
		SYS_FM7        = 1,
		SYS_FM77AV     = 2,
		SYS_FM77AV40EX = 3,
		SYS_FM11       = 4,
		SYS_FM16       = 5
	};

	// keyboard scancode formats
	enum
	{
		KEY_MODE_FM7   = 0, // FM-7 ASCII type code
		KEY_MODE_FM16B = 1, // FM-16B (FM-77AV and later only)
		KEY_MODE_SCAN  = 2  // Scancode Make/Break (PC-like)
	};

	struct fm7_video_t
	{
		uint8_t sub_busy = 0U;
		uint8_t sub_halt = 0U;
		uint8_t sub_reset = 0U;  // high if reset caused by subrom change
		uint8_t attn_irq = 0U;
		uint8_t vram_access = 0U;  // VRAM access flag
		uint8_t crt_enable = 0U;
		uint16_t vram_offset = 0U;
		uint16_t vram_offset2 = 0U;
		uint8_t fm7_pal[8]{};
		uint16_t fm77av_pal_selected = 0U;
		uint8_t subrom = 0U;  // currently active sub CPU ROM (AV only)
		uint8_t cgrom = 0U;  // currently active CGROM (AV only)
		uint8_t modestatus = 0U;
		uint8_t multi_page = 0U;
		uint8_t fine_offset = 0U;
		uint8_t nmi_mask = 0U;
		uint8_t active_video_page = 0U;
		uint8_t display_video_page = 0U;
		uint8_t vsync_flag = 0U;
	};

	struct fm7_alu_t
	{
		uint8_t command = 0U;
		uint8_t lcolour = 0U;
		uint8_t mask = 0U;
		uint8_t compare_data = 0U;
		uint8_t compare[8]{};
		uint8_t bank_disable = 0U;
		uint8_t tilepaint_b = 0U;
		uint8_t tilepaint_r = 0U;
		uint8_t tilepaint_g = 0U;
		uint16_t addr_offset = 0U;
		uint16_t line_style = 0U;
		uint16_t x0 = 0U;
		uint16_t x1 = 0U;
		uint16_t y0 = 0U;
		uint16_t y1 = 0U;
		uint8_t busy = 0U;
	};

	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	virtual void fm7_alu_function(uint32_t offset) { }
	virtual void fm7_mmr_refresh(address_space &space) { }

	DECLARE_MACHINE_START(fm7);
	DECLARE_MACHINE_START(fm16);

	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	uint8_t subintf_r();
	void subintf_w(uint8_t data);
	uint8_t sub_busyflag_r();
	void sub_busyflag_w(uint8_t data);
	uint8_t cancel_ack();
	uint8_t attn_irq_r();
	uint8_t vram_access_r();
	void vram_access_w(uint8_t data);
	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	uint8_t crt_r();
	void crt_w(uint8_t data);
	void vram_offset_w(offs_t offset, uint8_t data);
	void multipage_w(uint8_t data);
	uint8_t palette_r(offs_t offset);
	void palette_w(offs_t offset, uint8_t data);
	void irq_mask_w(uint8_t data);
	uint8_t irq_cause_r();
	void beeper_w(uint8_t data);
	uint8_t sub_beeper_r();
	uint8_t fd04_r();
	uint8_t rom_en_r(address_space &space);
	void rom_en_w(address_space &space, uint8_t data);
	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);
	uint8_t keyboard_r(offs_t offset);
	uint8_t sub_keyboard_r(offs_t offset);
	uint8_t cassette_printer_r();
	void cassette_printer_w(offs_t offset, uint8_t data);
	uint8_t psg_select_r();
	void psg_select_w(uint8_t data);
	uint8_t psg_data_r();
	void psg_data_w(uint8_t data);
	uint8_t main_shared_r(offs_t offset);
	void main_shared_w(offs_t offset, uint8_t data);
	uint8_t unknown_r();
	uint8_t kanji_r(offs_t offset);
	void kanji_w(offs_t offset, uint8_t data);

	IRQ_CALLBACK_MEMBER(irq_ack);
	IRQ_CALLBACK_MEMBER(sub_irq_ack);

	void write_centronics_busy(int state);
	void write_centronics_fault(int state);
	void write_centronics_ack(int state);
	void write_centronics_perror(int state);

	uint32_t screen_update_fm7(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void fm16_io(address_map &map) ATTR_COLD;
	void fm16_mem(address_map &map) ATTR_COLD;
	void fm16_sub_mem(address_map &map) ATTR_COLD;
	void fm7_mem(address_map &map) ATTR_COLD;
	void fm7_sub_mem(address_map &map) ATTR_COLD;
	void fm8_mem(address_map &map) ATTR_COLD;

	optional_shared_ptr<uint8_t> m_shared_ram;
	optional_shared_ptr<uint8_t> m_a15_ram;
	optional_shared_ptr<uint8_t> m_vectors;

	uint8_t           m_irq_flags = 0U;
	uint8_t           m_irq_mask = 0U;
	emu_timer*      m_beeper_off_timer = nullptr;
	emu_timer*      m_timer = nullptr;
	emu_timer*      m_subtimer = nullptr;
	emu_timer*      m_keyboard_timer = nullptr;
	bool            m_basic_rom_en = false;
	bool            m_init_rom_en = false;

	unsigned int    m_key_delay = 0U;
	unsigned int    m_key_repeat = 0U;
	uint16_t          m_current_scancode = 0U;
	uint32_t          m_key_data[4]{};
	uint32_t          m_mod_data = 0U;
	uint8_t           m_key_scan_mode = 0U;
	uint8_t           m_break_flag = 0U;

	uint8_t           m_psg_regsel = 0U;
	uint8_t           m_psg_data = 0U;

	uint8_t           m_fdc_side = 0U;
	uint8_t           m_fdc_drive = 0U;
	uint8_t           m_fdc_irq_flag = 0U;
	uint8_t           m_fdc_drq_flag = 0U;

	uint8_t           m_speaker_active = 0U;

	uint16_t          m_kanji_address = 0U;
	uint8_t           m_cp_prev = 0U;

	std::unique_ptr<uint8_t[]>    m_video_ram{};
	uint8_t m_type = 0U;
	fm7_video_t     m_video{};
	fm7_alu_t       m_alu{};
	int             m_sb_prev = 0;

	TIMER_CALLBACK_MEMBER(beeper_off);
	TIMER_CALLBACK_MEMBER(timer_irq);
	TIMER_CALLBACK_MEMBER(subtimer_irq);
	TIMER_CALLBACK_MEMBER(keyboard_poll);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_sub;
	required_device<cassette_image_device> m_cassette;
	required_device<beep_device> m_beeper;
	optional_device<ay8910_device> m_psg;
	required_device<screen_device> m_screen;

	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;

	required_device<mb8877_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy = nullptr;

	optional_region_ptr<uint8_t> m_rom_ptr;
	optional_region_ptr<uint8_t> m_btrom_ptr;
	optional_region_ptr<uint8_t> m_basic_ptr;

	void main_irq_set_flag(uint8_t flag);
	void main_irq_clear_flag(uint8_t flag);
	virtual void fm7_update_psg();
	void key_press(uint16_t scancode);
	void keyboard_poll_scan();

	int m_centronics_busy = 0;
	int m_centronics_fault = 0;
	int m_centronics_ack = 0;
	int m_centronics_perror = 0;

	optional_memory_region m_kanji;
	required_ioport_array<3> m_kb_ports;
	required_ioport m_keymod;
	required_ioport m_joy1;
	required_ioport m_dsw;
	required_device<palette_device> m_palette;
	optional_device<palette_device> m_av_palette;
};

class fm77_state : public fm7_state
{
public:
	fm77_state(const machine_config &mconfig, device_type type, const char *tag) :
		fm7_state(mconfig, type, tag),
		m_avbank(*this, "av_bank%u", 1),
		m_ym(*this, "ym"),
		m_boot_ram(*this, "boot_ram"),
		m_extended_ram(*this, "extended_ram"),
		m_fbasic_bank_ram(*this, "fbasic_bank_w"),
		m_init_bank_ram(*this, "init_bank_w")
	{
	}

	void fm77av(machine_config &config);

protected:
	struct fm7_encoder_t
	{
		uint8_t buffer[12]{};
		uint8_t tx_count = 0U;
		uint8_t rx_count = 0U;
		uint8_t command_length = 0U;
		uint8_t answer_length = 0U;
		uint8_t latch = 0U;  // 0=ready to receive
		uint8_t ack = 0U;
		uint8_t position = 0U;
	};

	struct fm7_mmr_t
	{
		uint8_t bank_addr[8][16]{};
		uint8_t segment = 0U;
		uint8_t window_offset = 0U;
		uint8_t enabled = 0U;
		uint8_t mode = 0U;
	};

	virtual void machine_reset() override ATTR_COLD;

	DECLARE_MACHINE_START(fm77av);

	void av_encoder_setup_command();
	void av_encoder_handle_command();
	TIMER_CALLBACK_MEMBER(av_encoder_ack);
	TIMER_CALLBACK_MEMBER(av_alu_task_end);
	TIMER_CALLBACK_MEMBER(av_vsync);

	void av_fmirq(int state);

	void av_analog_palette_w(offs_t offset, uint8_t data);
	uint8_t av_video_flags_r();
	void av_video_flags_w(uint8_t data);
	uint8_t av_sub_modestatus_r();
	void av_sub_modestatus_w(uint8_t data);
	void av_sub_bank_w(uint8_t data);
	uint8_t av_alu_r(offs_t offset);
	void av_alu_w(offs_t offset, uint8_t data);
	void av_bootram_w(offs_t offset, uint8_t data);
	uint8_t av_key_encoder_r(offs_t offset);
	void av_key_encoder_w(offs_t offset, uint8_t data);
	uint8_t av_boot_mode_r();
	void av_ym_select_w(uint8_t data);
	uint8_t vector_r(offs_t offset);
	void init_en_w(address_space &space, uint8_t data);
	uint8_t fmirq_r();
	virtual void fm7_update_psg() override;

	uint8_t mmr_r(offs_t offset);
	void mmr_w(address_space &space, offs_t offset, uint8_t data);
	void fm7_update_bank(int bank, uint8_t physical);
	virtual void fm7_mmr_refresh(address_space &space) override;

	void alu_mask_write(uint32_t offset, int bank, uint8_t dat);
	void alu_function_compare(uint32_t offset);
	void alu_function_pset(uint32_t offset);
	void alu_function_or(uint32_t offset);
	void alu_function_and(uint32_t offset);
	void alu_function_xor(uint32_t offset);
	void alu_function_not(uint32_t offset);
	void alu_function_invalid(uint32_t offset);
	void alu_function_tilepaint(uint32_t offset);
	virtual void fm7_alu_function(uint32_t offset) override;
	uint32_t av_line_set_pixel(int x, int y);
	void av_line_draw();

	void fm77av_mem(address_map &map) ATTR_COLD;
	void fm77av_sub_mem(address_map &map) ATTR_COLD;
	void fm7_banked_mem(address_map &map) ATTR_COLD;

	required_device_array<address_map_bank_device, 16> m_avbank;
	optional_device<ym2203_device> m_ym;
	required_shared_ptr<uint8_t> m_boot_ram;
	required_shared_ptr<uint8_t> m_extended_ram;
	required_shared_ptr<uint8_t> m_fbasic_bank_ram;
	required_shared_ptr<uint8_t> m_init_bank_ram;

	fm7_encoder_t   m_encoder{};
	fm7_mmr_t       m_mmr{};

	emu_timer *m_encoder_ack_timer = nullptr;
	emu_timer *m_alu_task_end_timer = nullptr;
	emu_timer *m_vsync_timer = nullptr;

	uint8_t           m_fm77av_ym_irq = 0U;
};

class fm11_state : public fm77_state
{
public:
	fm11_state(const machine_config &mconfig, device_type type, const char *tag) :
		fm77_state(mconfig, type, tag),
		m_x86(*this, "x86")
	{
	}

	void fm11(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	DECLARE_MACHINE_START(fm11);

	void fm11_mem(address_map &map) ATTR_COLD;
	void fm11_sub_mem(address_map &map) ATTR_COLD;
	void fm11_x86_io(address_map &map) ATTR_COLD;
	void fm11_x86_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_x86;
};

#endif // MAME_FUJITSU_FM7_H
