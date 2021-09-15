// license:BSD-3-Clause
// copyright-holders:Golden Child
#ifndef MAME_BUS_IMAGEWRITER_PRINTER_H
#define MAME_BUS_IMAGEWRITER_PRINTER_H

#pragma once

#include "rs232.h"
#include "cpu/i8085/i8085.h"
#include "bus/a2bus/bitmap_printer.h"
#include "machine/i8155.h"
#include "machine/i8251.h"
#include "machine/steppers.h"
#include "machine/74123.h"
//#include "machine/74161.h"
#include "machine/timer.h"

class apple_imagewriter_printer_device : public device_t,
	public device_rs232_port_interface
{
public:
	apple_imagewriter_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(reset_sw);
	DECLARE_INPUT_CHANGED_MEMBER(select_sw);


protected:
	apple_imagewriter_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void device_start() override;
	virtual void device_reset() override;

	void mem_map(address_map &map);
	void io_map(address_map &map);

private:
	int m_initial_rx_state;

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8251_device> m_uart;
	required_device<i8155_device> m_8155head;
	required_device<i8155_device> m_8155switch;
	required_device<ttl74123_device> m_pulse1;
	required_device<ttl74123_device> m_pulse2;

	required_device<bitmap_printer_device> m_bitmap_printer;
	required_device<stepper_device> m_pf_stepper;
	required_device<stepper_device> m_cr_stepper;

	required_device<timer_device> m_timer_rxclock;

	void maincpu_out_sod_func(uint8_t data);

	uint8_t head_pa_r(offs_t offset);
	void head_pa_w(uint8_t data);
	uint8_t head_pb_r(offs_t offset);
	void head_pb_w(uint8_t data);
	uint8_t head_pc_r(offs_t offset);
	void head_pc_w(uint8_t data);
	void head_to(uint8_t data);

	uint8_t switch_pa_r(offs_t offset);
	void switch_pa_w(uint8_t data);
	uint8_t switch_pb_r(offs_t offset);
	void switch_pb_w(uint8_t data);
	uint8_t switch_pc_r(offs_t offset);
	void switch_pc_w(uint8_t data);
	void switch_to(uint8_t data);

	int ioportsaferead(const char * name);

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override;

	void rxrdy_handler(uint8_t data);

	int m_pulse1_out_last = 1;

	void pulse1_out_handler(uint8_t data);
	void pulse2_out_handler(uint8_t data);

	uint8_t maincpu_in_sid_func();
	void dtr_handler(uint8_t data);
	void rts_handler(uint8_t data);

	void txd_handler(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER (pulse_uart_clock);


	int xdirection = 0;
	int newpageflag = 0;
	int page_count = 0;

	XTAL baseCLK = 9.8304_MHz_XTAL;  // base clock to 8085 cpu = 9.8304 Mhz
	XTAL CLK2 = baseCLK / 2;         // CLK2 name from Sams schematic = 4.9152 Mhz
	XTAL CLK1 = CLK2 / 2;            // CLK1 name from Sams schematic = 2.4576 Mhz

	const int dpi = 144;
	const double xscale = 9.0 / 8.0; // 1.125  (stepper moves at 162 dpi, not 144 dpi)
	const double PAPER_WIDTH_INCHES = 8.5;
	const double PAPER_HEIGHT_INCHES = 11.0;
	const double MARGIN_INCHES = .25;
	const int PAPER_WIDTH  = PAPER_WIDTH_INCHES * dpi * xscale;  // 8.5 inches wide
	const int PAPER_HEIGHT = PAPER_HEIGHT_INCHES * dpi;          // 11  inches high
	const int PAPER_SCREEN_HEIGHT = 384; // match the height of the apple II driver
	const int distfrombottom = 50;

	int xposratio0 = 144;
	int xposratio1 = 144;
	int yposratio0 = 18;
	int yposratio1 = 18;

	int m_xpos = PAPER_WIDTH / 2;  // middle of paper (paper width in pixels)
	int m_ypos = 30;
	s32 x_pixel_coord(s32 xpos) { return xpos * xposratio0 / xposratio1; }  // x position
	s32 y_pixel_coord(s32 ypos) { return ypos * yposratio0 / yposratio1; }  // y position given in half steps

	int update_stepper_delta(stepper_device * stepper, uint8_t stepper_pattern, const char * name, int direction);
	void update_printhead();
	void update_pf_stepper(uint8_t data);
	void update_cr_stepper(uint8_t data);

	u8 m_uart_clock = 0;
	int m_baud_clock_divisor = 1;
	int m_baud_clock_divisor_delay = 0;

	int m_ic17_flipflop_head = 0;           // connected to 8155 head     (ic17 7474 part 1/2)
	int m_ic17_flipflop_select_status = 0;  // connected to 8155 switches (ic17 7474 part 2/2)
	int m_head_to_last = 0;
	int m_head_pb_last = 0;
	int m_switches_pc_last = 0;
	int m_switches_to_last = 0;
	u16 m_dotpattern = 0;

	int m_left_edge_adjust = -6;  // to get perfect centering with macpaint

	int right_offset = 0;
	int left_offset  = 0;
	int m_left_edge  = (MARGIN_INCHES) * dpi * xscale + m_left_edge_adjust;    // 0 for starting at left edge, print shop seems to like -32 for centered print
	int m_right_edge = (PAPER_WIDTH_INCHES + MARGIN_INCHES) * dpi * xscale - 1;  // when it hits the right edge, will return to the left edge and go deselected (so subtracting margin_inches is enough to stop the self test)

	void darken_pixel(double darkpct, unsigned int& pixel);
	void update_head_pos();
};

DECLARE_DEVICE_TYPE(APPLE_IMAGEWRITER_PRINTER, apple_imagewriter_printer_device)

#endif // MAME_BUS_IMAGEWRITER_PRINTER_H
