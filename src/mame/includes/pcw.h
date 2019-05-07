// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/pcw.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_PCW_H
#define MAME_INCLUDES_PCW_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "emupal.h"
#include "screen.h"

#define PCW_BORDER_HEIGHT 8
#define PCW_BORDER_WIDTH 8
#define PCW_NUM_COLOURS 2
#define PCW_DISPLAY_WIDTH 720
#define PCW_DISPLAY_HEIGHT 256

#define PCW_SCREEN_WIDTH    (PCW_DISPLAY_WIDTH + (PCW_BORDER_WIDTH<<1))
#define PCW_SCREEN_HEIGHT   (PCW_DISPLAY_HEIGHT  + (PCW_BORDER_HEIGHT<<1))
#define PCW_PRINTER_WIDTH   (80*16)
#define PCW_PRINTER_HEIGHT  (20*16)


class pcw_state : public driver_device
{
public:
	pcw_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_printer_mcu(*this, "printer_mcu")
		, m_keyboard_mcu(*this, "keyboard_mcu")
		, m_fdc(*this, "upd765")
		, m_floppy(*this, "upd765:%u", 0U)
		, m_ram(*this, RAM_TAG)
		, m_beeper(*this, "beeper")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_ppalette(*this, "ppalette")
		, m_rdbanks(*this, "bank%u", 1U)
		, m_wrbanks(*this, "bank%u", 5U)
		, m_iptlines(*this, "LINE%u", 0U)
	{ }

	int m_boot;
	int m_system_status;
	int m_fdc_interrupt_code;
	int m_interrupt_counter;
	uint8_t m_banks[4];
	unsigned char m_bank_force;
	uint8_t m_timer_irq_flag;
	uint8_t m_nmi_flag;
	int16_t m_printer_headpos;
	uint16_t m_kb_scan_row;
	uint8_t m_mcu_keyboard_data[16];
	uint8_t m_mcu_transmit_reset_seq;
	uint8_t m_mcu_transmit_count;
	uint8_t m_mcu_selected;
	uint8_t m_mcu_buffer;
	uint8_t m_mcu_prev;
	unsigned int m_roller_ram_addr;
	unsigned short m_roller_ram_offset;
	unsigned char m_vdu_video_control_register;
	uint8_t m_printer_serial;  // value if shift/store data pin
	uint8_t m_printer_shift;  // state of shift register
	uint8_t m_printer_shift_output;  // output presented to the paper feed motor and print head motor
	uint8_t m_head_motor_state;
	uint8_t m_linefeed_motor_state;
	uint16_t m_printer_pins;
	uint8_t m_printer_p2;  // MCU port P2 state
	uint32_t m_paper_feed;  // amount of paper fed through printer, by n/360 inches.  One line feed is 61/360in (from the linefeed command in CP/M;s ptr menu)
	std::unique_ptr<bitmap_ind16> m_prn_output;
	uint8_t m_printer_p2_prev;
	emu_timer *m_prn_stepper;
	emu_timer *m_prn_pins;
	emu_timer *m_pulse_timer;
	emu_timer *m_beep_setup_timer;
	DECLARE_READ8_MEMBER(pcw_keyboard_r);
	DECLARE_READ8_MEMBER(pcw_keyboard_data_r);
	DECLARE_READ8_MEMBER(pcw_interrupt_counter_r);
	DECLARE_WRITE8_MEMBER(pcw_bank_select_w);
	DECLARE_WRITE8_MEMBER(pcw_bank_force_selection_w);
	DECLARE_WRITE8_MEMBER(pcw_roller_ram_addr_w);
	DECLARE_WRITE8_MEMBER(pcw_pointer_table_top_scan_w);
	DECLARE_WRITE8_MEMBER(pcw_vdu_video_control_register_w);
	DECLARE_WRITE8_MEMBER(pcw_system_control_w);
	DECLARE_READ8_MEMBER(pcw_system_status_r);
	DECLARE_READ8_MEMBER(pcw_expansion_r);
	DECLARE_WRITE8_MEMBER(pcw_expansion_w);
	DECLARE_WRITE8_MEMBER(pcw_printer_data_w);
	DECLARE_WRITE8_MEMBER(pcw_printer_command_w);
	DECLARE_READ8_MEMBER(pcw_printer_data_r);
	DECLARE_READ8_MEMBER(pcw_printer_status_r);
	DECLARE_READ8_MEMBER(mcu_printer_p1_r);
	DECLARE_WRITE8_MEMBER(mcu_printer_p1_w);
	DECLARE_READ8_MEMBER(mcu_printer_p2_r);
	DECLARE_WRITE8_MEMBER(mcu_printer_p2_w);
	DECLARE_READ_LINE_MEMBER(mcu_printer_t1_r);
	DECLARE_READ_LINE_MEMBER(mcu_printer_t0_r);
	DECLARE_READ8_MEMBER(mcu_kb_scan_r);
	DECLARE_WRITE8_MEMBER(mcu_kb_scan_w);
	DECLARE_READ8_MEMBER(mcu_kb_scan_high_r);
	DECLARE_WRITE8_MEMBER(mcu_kb_scan_high_w);
	DECLARE_READ8_MEMBER(mcu_kb_data_r);
	DECLARE_READ_LINE_MEMBER(mcu_kb_t1_r);
	DECLARE_READ_LINE_MEMBER(mcu_kb_t0_r);
	DECLARE_READ8_MEMBER(pcw9512_parallel_r);
	DECLARE_WRITE8_MEMBER(pcw9512_parallel_w);
	void mcu_transmit_serial(uint8_t bit);
	void init_pcw();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void set_8xxx_palette(palette_device &palette) const;
	void set_9xxx_palette(palette_device &palette) const;
	void set_printer_palette(palette_device &palette) const;
	uint32_t screen_update_pcw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pcw_printer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(pcw_timer_pulse);
	TIMER_CALLBACK_MEMBER(pcw_stepper_callback);
	TIMER_CALLBACK_MEMBER(pcw_pins_callback);
	TIMER_CALLBACK_MEMBER(setup_beep);
	TIMER_DEVICE_CALLBACK_MEMBER(pcw_timer_interrupt);

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_WRITE_LINE_MEMBER( pcw_fdc_interrupt );
	required_device<cpu_device> m_maincpu;
	required_device<i8041_device> m_printer_mcu;
	required_device<i8048_device> m_keyboard_mcu;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<ram_device> m_ram;
	required_device<beep_device> m_beeper;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<palette_device> m_ppalette;
	required_memory_bank_array<4> m_rdbanks, m_wrbanks;
	required_ioport_array<16> m_iptlines;

	inline void pcw_plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color);
	void pcw_update_interrupt_counter();
	void pcw_update_irqs();
	void pcw_update_read_memory_block(int block, int bank);
	void pcw_update_write_memory_block(int block, int bank);
	void pcw_update_mem(int block, int data);
	int pcw_get_sys_status();
	void pcw_printer_fire_pins(uint16_t pins);
	void pcw(machine_config &config);
	void pcw8256(machine_config &config);
	void pcw8512(machine_config &config);
	void pcw9512(machine_config &config);
	void pcw9256(machine_config &config);
	void pcw9512p(machine_config &config);
	void pcw10(machine_config &config);
	void pcw9512_io(address_map &map);
	void pcw_io(address_map &map);
	void pcw_map(address_map &map);
};

#endif // MAME_INCLUDES_PCW_H
