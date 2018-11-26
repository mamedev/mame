// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *
 *  FM-7 header file
 *
 */
#ifndef MAME_INCLUDES_FM7_H
#define MAME_INCLUDES_FM7_H

#pragma once


#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "sound/beep.h"
#include "sound/2203intf.h"
#include "machine/wd_fdc.h"
#include "machine/bankdev.h"
#include "emupal.h"


// Interrupt flags
#define IRQ_FLAG_KEY      0x01
#define IRQ_FLAG_PRINTER  0x02
#define IRQ_FLAG_TIMER    0x04
#define IRQ_FLAG_OTHER    0x08
// the following are not read in port 0xfd03
#define IRQ_FLAG_MFD      0x10
#define IRQ_FLAG_TXRDY    0x20
#define IRQ_FLAG_RXRDY    0x40
#define IRQ_FLAG_SYNDET   0x80

// system types
#define SYS_FM7        1
#define SYS_FM77AV     2
#define SYS_FM77AV40EX 3
#define SYS_FM11       4
#define SYS_FM16       5

// keyboard scancode formats
#define KEY_MODE_FM7   0 // FM-7 ASCII type code
#define KEY_MODE_FM16B 1 // FM-16B (FM-77AV and later only)
#define KEY_MODE_SCAN  2 // Scancode Make/Break (PC-like)

struct fm7_encoder_t
{
	uint8_t buffer[12];
	uint8_t tx_count;
	uint8_t rx_count;
	uint8_t command_length;
	uint8_t answer_length;
	uint8_t latch;  // 0=ready to receive
	uint8_t ack;
	uint8_t position;
};

struct fm7_mmr_t
{
	uint8_t bank_addr[8][16];
	uint8_t segment;
	uint8_t window_offset;
	uint8_t enabled;
	uint8_t mode;
};

struct fm7_video_t
{
	uint8_t sub_busy;
	uint8_t sub_halt;
	uint8_t sub_reset;  // high if reset caused by subrom change
	uint8_t attn_irq;
	uint8_t vram_access;  // VRAM access flag
	uint8_t crt_enable;
	uint16_t vram_offset;
	uint16_t vram_offset2;
	uint8_t fm7_pal[8];
	uint16_t fm77av_pal_selected;
	uint8_t subrom;  // currently active sub CPU ROM (AV only)
	uint8_t cgrom;  // currently active CGROM (AV only)
	uint8_t modestatus;
	uint8_t multi_page;
	uint8_t fine_offset;
	uint8_t nmi_mask;
	uint8_t active_video_page;
	uint8_t display_video_page;
	uint8_t vsync_flag;
};

struct fm7_alu_t
{
	uint8_t command;
	uint8_t lcolour;
	uint8_t mask;
	uint8_t compare_data;
	uint8_t compare[8];
	uint8_t bank_disable;
	uint8_t tilepaint_b;
	uint8_t tilepaint_r;
	uint8_t tilepaint_g;
	uint16_t addr_offset;
	uint16_t line_style;
	uint16_t x0;
	uint16_t x1;
	uint16_t y0;
	uint16_t y1;
	uint8_t busy;
};


class fm7_state : public driver_device
{
public:
	fm7_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_shared_ram(*this, "shared_ram"),
		m_boot_ram(*this, "boot_ram"),
		m_maincpu(*this, "maincpu"),
		m_sub(*this, "sub"),
		m_x86(*this, "x86"),
		m_cassette(*this, "cassette"),
		m_beeper(*this, "beeper"),
		m_ym(*this, "ym"),
		m_psg(*this, "psg"),
		m_screen(*this, "screen"),
		m_centronics(*this, "centronics"),
		m_cent_data_out(*this, "cent_data_out"),
		m_fdc(*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_floppy1(*this, "fdc:1"),
		m_floppy(nullptr),
		m_ram_ptr(*this, "maincpu"),
		m_rom_ptr(*this, "init"),
		m_basic_ptr(*this, "fbasic"),
		m_kanji(*this, "kanji1"),
		m_kb_ports(*this, "key%u", 1),
		m_keymod(*this, "key_modifiers"),
		m_joy1(*this, "joy1"),
		m_joy2(*this, "joy2"),
		m_dsw(*this, "DSW"),
		m_palette(*this, "palette"),
		m_av_palette(*this, "av_palette"),
		m_avbank(*this, "av_bank%u", 1)
	{
	}

	void fm16beta(machine_config &config);
	void fm8(machine_config &config);
	void fm7(machine_config &config);
	void fm77av(machine_config &config);
	void fm11(machine_config &config);

	void init_fm7();

private:
	enum
	{
		TIMER_FM7_BEEPER_OFF,
		TIMER_FM77AV_ENCODER_ACK,
		TIMER_FM7_IRQ,
		TIMER_FM7_SUBTIMER_IRQ,
		TIMER_FM7_KEYBOARD_POLL,
		TIMER_FM77AV_ALU_TASK_END,
		TIMER_FM77AV_VSYNC
	};

	virtual void machine_reset() override;
	virtual void video_start() override;

	DECLARE_MACHINE_START(fm7);
	DECLARE_MACHINE_START(fm77av);
	DECLARE_MACHINE_START(fm11);
	DECLARE_MACHINE_START(fm16);

	DECLARE_WRITE_LINE_MEMBER(fm7_fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(fm7_fdc_drq_w);
	DECLARE_READ8_MEMBER(fm77av_joy_1_r);
	DECLARE_READ8_MEMBER(fm77av_joy_2_r);
	DECLARE_WRITE_LINE_MEMBER(fm77av_fmirq);

	DECLARE_READ8_MEMBER(fm7_subintf_r);
	DECLARE_WRITE8_MEMBER(fm7_subintf_w);
	DECLARE_READ8_MEMBER(fm7_sub_busyflag_r);
	DECLARE_WRITE8_MEMBER(fm7_sub_busyflag_w);
	DECLARE_READ8_MEMBER(fm7_cancel_ack);
	DECLARE_READ8_MEMBER(fm7_attn_irq_r);
	DECLARE_READ8_MEMBER(fm7_vram_access_r);
	DECLARE_WRITE8_MEMBER(fm7_vram_access_w);
	DECLARE_READ8_MEMBER(fm7_vram_r);
	DECLARE_WRITE8_MEMBER(fm7_vram_w);
	DECLARE_WRITE8_MEMBER(fm7_vram_banked_w);
	DECLARE_READ8_MEMBER(fm7_vram0_r);
	DECLARE_READ8_MEMBER(fm7_vram1_r);
	DECLARE_READ8_MEMBER(fm7_vram2_r);
	DECLARE_READ8_MEMBER(fm7_vram3_r);
	DECLARE_READ8_MEMBER(fm7_vram4_r);
	DECLARE_READ8_MEMBER(fm7_vram5_r);
	DECLARE_READ8_MEMBER(fm7_vram6_r);
	DECLARE_READ8_MEMBER(fm7_vram7_r);
	DECLARE_READ8_MEMBER(fm7_vram8_r);
	DECLARE_READ8_MEMBER(fm7_vram9_r);
	DECLARE_READ8_MEMBER(fm7_vramA_r);
	DECLARE_READ8_MEMBER(fm7_vramB_r);
	DECLARE_WRITE8_MEMBER(fm7_vram0_w);
	DECLARE_WRITE8_MEMBER(fm7_vram1_w);
	DECLARE_WRITE8_MEMBER(fm7_vram2_w);
	DECLARE_WRITE8_MEMBER(fm7_vram3_w);
	DECLARE_WRITE8_MEMBER(fm7_vram4_w);
	DECLARE_WRITE8_MEMBER(fm7_vram5_w);
	DECLARE_WRITE8_MEMBER(fm7_vram6_w);
	DECLARE_WRITE8_MEMBER(fm7_vram7_w);
	DECLARE_WRITE8_MEMBER(fm7_vram8_w);
	DECLARE_WRITE8_MEMBER(fm7_vram9_w);
	DECLARE_WRITE8_MEMBER(fm7_vramA_w);
	DECLARE_WRITE8_MEMBER(fm7_vramB_w);
	DECLARE_READ8_MEMBER(fm7_crt_r);
	DECLARE_WRITE8_MEMBER(fm7_crt_w);
	DECLARE_WRITE8_MEMBER(fm7_vram_offset_w);
	DECLARE_WRITE8_MEMBER(fm7_multipage_w);
	DECLARE_READ8_MEMBER(fm7_palette_r);
	DECLARE_WRITE8_MEMBER(fm7_palette_w);
	DECLARE_WRITE8_MEMBER(fm77av_analog_palette_w);
	DECLARE_READ8_MEMBER(fm77av_video_flags_r);
	DECLARE_WRITE8_MEMBER(fm77av_video_flags_w);
	DECLARE_READ8_MEMBER(fm77av_sub_modestatus_r);
	DECLARE_WRITE8_MEMBER(fm77av_sub_modestatus_w);
	DECLARE_WRITE8_MEMBER(fm77av_sub_bank_w);
	DECLARE_READ8_MEMBER(fm77av_alu_r);
	DECLARE_WRITE8_MEMBER(fm77av_alu_w);
	DECLARE_READ8_MEMBER(fm7_sub_ram_ports_banked_r);
	DECLARE_WRITE8_MEMBER(fm7_sub_ram_ports_banked_w);
	DECLARE_READ8_MEMBER(fm7_console_ram_banked_r);
	DECLARE_WRITE8_MEMBER(fm7_console_ram_banked_w);
	DECLARE_WRITE8_MEMBER(fm7_irq_mask_w);
	DECLARE_READ8_MEMBER(fm7_irq_cause_r);
	DECLARE_WRITE8_MEMBER(fm7_beeper_w);
	DECLARE_READ8_MEMBER(fm7_sub_beeper_r);
	DECLARE_READ8_MEMBER(vector_r);
	DECLARE_WRITE8_MEMBER(vector_w);
	DECLARE_READ8_MEMBER(fm7_fd04_r);
	DECLARE_READ8_MEMBER(fm7_rom_en_r);
	DECLARE_WRITE8_MEMBER(fm7_rom_en_w);
	DECLARE_WRITE8_MEMBER(fm7_init_en_w);
	DECLARE_READ8_MEMBER(fm7_fdc_r);
	DECLARE_WRITE8_MEMBER(fm7_fdc_w);
	DECLARE_READ8_MEMBER(fm7_keyboard_r);
	DECLARE_READ8_MEMBER(fm7_sub_keyboard_r);
	DECLARE_READ8_MEMBER(fm77av_key_encoder_r);
	DECLARE_WRITE8_MEMBER(fm77av_key_encoder_w);
	DECLARE_READ8_MEMBER(fm7_cassette_printer_r);
	DECLARE_WRITE8_MEMBER(fm7_cassette_printer_w);
	DECLARE_READ8_MEMBER(fm77av_boot_mode_r);
	DECLARE_READ8_MEMBER(fm7_psg_select_r);
	DECLARE_WRITE8_MEMBER(fm7_psg_select_w);
	DECLARE_WRITE8_MEMBER(fm77av_ym_select_w);
	DECLARE_READ8_MEMBER(fm7_psg_data_r);
	DECLARE_WRITE8_MEMBER(fm7_psg_data_w);
	DECLARE_WRITE8_MEMBER(fm77av_bootram_w);
	DECLARE_READ8_MEMBER(fm7_main_shared_r);
	DECLARE_WRITE8_MEMBER(fm7_main_shared_w);
	DECLARE_READ8_MEMBER(fm7_fmirq_r);
	DECLARE_READ8_MEMBER(fm7_unknown_r);
	DECLARE_READ8_MEMBER(fm7_mmr_r);
	DECLARE_WRITE8_MEMBER(fm7_mmr_w);
	DECLARE_READ8_MEMBER(fm7_kanji_r);
	DECLARE_WRITE8_MEMBER(fm7_kanji_w);

	IRQ_CALLBACK_MEMBER(fm7_irq_ack);
	IRQ_CALLBACK_MEMBER(fm7_sub_irq_ack);

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_fault);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_ack);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_perror);

	uint32_t screen_update_fm7(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void fm11_mem(address_map &map);
	void fm11_sub_mem(address_map &map);
	void fm11_x86_io(address_map &map);
	void fm11_x86_mem(address_map &map);
	void fm16_io(address_map &map);
	void fm16_mem(address_map &map);
	void fm16_sub_mem(address_map &map);
	void fm77av_mem(address_map &map);
	void fm77av_sub_mem(address_map &map);
	void fm7_banked_mem(address_map &map);
	void fm7_mem(address_map &map);
	void fm7_sub_mem(address_map &map);
	void fm8_mem(address_map &map);

	optional_shared_ptr<uint8_t> m_shared_ram;
	optional_shared_ptr<uint8_t> m_boot_ram;

	uint8_t           m_irq_flags;
	uint8_t           m_irq_mask;
	emu_timer*      m_timer;
	emu_timer*      m_subtimer;
	emu_timer*      m_keyboard_timer;
	uint8_t           m_basic_rom_en;
	uint8_t           m_init_rom_en;

	unsigned int    m_key_delay;
	unsigned int    m_key_repeat;
	uint16_t          m_current_scancode;
	uint32_t          m_key_data[4];
	uint32_t          m_mod_data;
	uint8_t           m_key_scan_mode;
	uint8_t           m_break_flag;

	uint8_t           m_psg_regsel;
	uint8_t           m_psg_data;

	uint8_t           m_fdc_side;
	uint8_t           m_fdc_drive;
	uint8_t           m_fdc_irq_flag;
	uint8_t           m_fdc_drq_flag;

	uint8_t           m_fm77av_ym_irq;
	uint8_t           m_speaker_active;

	uint16_t          m_kanji_address;
	fm7_encoder_t   m_encoder;
	fm7_mmr_t       m_mmr;
	uint8_t           m_cp_prev;

	std::unique_ptr<uint8_t[]>    m_video_ram;
	emu_timer*                  m_fm77av_vsync_timer;
	uint8_t m_type;
	fm7_video_t     m_video;
	fm7_alu_t       m_alu;
	int             m_sb_prev;

	void fm77av_encoder_setup_command();
	void fm77av_encoder_handle_command();
	TIMER_CALLBACK_MEMBER(fm7_beeper_off);
	TIMER_CALLBACK_MEMBER(fm77av_encoder_ack);
	TIMER_CALLBACK_MEMBER(fm7_timer_irq);
	TIMER_CALLBACK_MEMBER(fm7_subtimer_irq);
	TIMER_CALLBACK_MEMBER(fm7_keyboard_poll);
	TIMER_CALLBACK_MEMBER(fm77av_alu_task_end);
	TIMER_CALLBACK_MEMBER(fm77av_vsync);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_sub;
	optional_device<cpu_device> m_x86;
	required_device<cassette_image_device> m_cassette;
	required_device<beep_device> m_beeper;
	optional_device<ym2203_device> m_ym;
	optional_device<ay8910_device> m_psg;
	required_device<screen_device> m_screen;

	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;

	required_device<mb8877_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;

	optional_region_ptr<uint8_t> m_ram_ptr;
	optional_region_ptr<uint8_t> m_rom_ptr;
	optional_region_ptr<uint8_t> m_basic_ptr;

	void fm7_alu_mask_write(uint32_t offset, int bank, uint8_t dat);
	void fm7_alu_function_compare(uint32_t offset);
	void fm7_alu_function_pset(uint32_t offset);
	void fm7_alu_function_or(uint32_t offset);
	void fm7_alu_function_and(uint32_t offset);
	void fm7_alu_function_xor(uint32_t offset);
	void fm7_alu_function_not(uint32_t offset);
	void fm7_alu_function_invalid(uint32_t offset);
	void fm7_alu_function_tilepaint(uint32_t offset);
	void fm7_alu_function(uint32_t offset);
	uint32_t fm7_line_set_pixel(int x, int y);
	void fm77av_line_draw();
	void main_irq_set_flag(uint8_t flag);
	void main_irq_clear_flag(uint8_t flag);
	void fm7_update_psg();
	void fm7_update_bank(address_space & space, int bank, uint8_t physical);
	void fm7_mmr_refresh(address_space& space);
	void key_press(uint16_t scancode);
	void fm7_keyboard_poll_scan();

	int m_centronics_busy;
	int m_centronics_fault;
	int m_centronics_ack;
	int m_centronics_perror;

	optional_memory_region m_kanji;
	required_ioport_array<3> m_kb_ports;
	required_ioport m_keymod;
	required_ioport m_joy1;
	required_ioport m_joy2;
	required_ioport m_dsw;
	required_device<palette_device> m_palette;
	optional_device<palette_device> m_av_palette;

	optional_device_array<address_map_bank_device, 16> m_avbank;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_FM7_H
