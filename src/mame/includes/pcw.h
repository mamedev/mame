// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/pcw.h
 *
 ****************************************************************************/

#ifndef PCW_H_
#define PCW_H_

#include "machine/upd765.h"
#include "machine/ram.h"
#include "sound/beep.h"

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
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_fdc(*this, "upd765"),
			m_ram(*this, RAM_TAG),
			m_beeper(*this, "beeper"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette")
	{ }

	int m_boot;
	int m_system_status;
	int m_fdc_interrupt_code;
	int m_interrupt_counter;
	UINT8 m_banks[4];
	unsigned char m_bank_force;
	UINT8 m_timer_irq_flag;
	UINT8 m_nmi_flag;
	UINT8 m_printer_command;
	UINT8 m_printer_data;
	UINT8 m_printer_status;
	INT16 m_printer_headpos;
	UINT16 m_kb_scan_row;
	UINT8 m_mcu_keyboard_data[16];
	UINT8 m_mcu_transmit_reset_seq;
	UINT8 m_mcu_transmit_count;
	UINT8 m_mcu_selected;
	UINT8 m_mcu_buffer;
	UINT8 m_mcu_prev;
	unsigned int m_roller_ram_addr;
	unsigned short m_roller_ram_offset;
	unsigned char m_vdu_video_control_register;
	UINT8 m_printer_serial;  // value if shift/store data pin
	UINT8 m_printer_shift;  // state of shift register
	UINT8 m_printer_shift_output;  // output presented to the paper feed motor and print head motor
	UINT8 m_head_motor_state;
	UINT8 m_linefeed_motor_state;
	UINT16 m_printer_pins;
	UINT8 m_printer_p2;  // MCU port P2 state
	UINT32 m_paper_feed;  // amount of paper fed through printer, by n/360 inches.  One line feed is 61/360in (from the linefeed command in CP/M;s ptr menu)
	std::unique_ptr<bitmap_ind16> m_prn_output;
	UINT8 m_printer_p2_prev;
	emu_timer* m_prn_stepper;
	emu_timer* m_prn_pins;
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
	DECLARE_READ8_MEMBER(mcu_printer_t1_r);
	DECLARE_READ8_MEMBER(mcu_printer_t0_r);
	DECLARE_READ8_MEMBER(mcu_kb_scan_r);
	DECLARE_WRITE8_MEMBER(mcu_kb_scan_w);
	DECLARE_READ8_MEMBER(mcu_kb_scan_high_r);
	DECLARE_WRITE8_MEMBER(mcu_kb_scan_high_w);
	DECLARE_READ8_MEMBER(mcu_kb_data_r);
	DECLARE_READ8_MEMBER(mcu_kb_t1_r);
	DECLARE_READ8_MEMBER(mcu_kb_t0_r);
	DECLARE_READ8_MEMBER(pcw9512_parallel_r);
	DECLARE_WRITE8_MEMBER(pcw9512_parallel_w);
	void mcu_transmit_serial(UINT8 bit);
	DECLARE_DRIVER_INIT(pcw);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(pcw);
	UINT32 screen_update_pcw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pcw_printer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(pcw_timer_pulse);
	TIMER_CALLBACK_MEMBER(pcw_stepper_callback);
	TIMER_CALLBACK_MEMBER(pcw_pins_callback);
	TIMER_CALLBACK_MEMBER(setup_beep);
	TIMER_DEVICE_CALLBACK_MEMBER(pcw_timer_interrupt);

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_WRITE_LINE_MEMBER( pcw_fdc_interrupt );
	required_device<cpu_device> m_maincpu;
	required_device<upd765a_device> m_fdc;
	required_device<ram_device> m_ram;
	required_device<beep_device> m_beeper;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	inline void pcw_plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color);
	void pcw_update_interrupt_counter();
	void pcw_update_irqs();
	void pcw_update_read_memory_block(int block, int bank);
	void pcw_update_write_memory_block(int block, int bank);
	void pcw_update_mem(int block, int data);
	int pcw_get_sys_status();
	void pcw_printer_fire_pins(UINT16 pins);
};

#endif /* PCW_H_ */
