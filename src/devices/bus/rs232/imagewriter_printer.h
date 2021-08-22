// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_IMAGEWRITER_PRINTER_H
#define MAME_BUS_IMAGEWRITER_PRINTER_H

#pragma once

#include "rs232.h"
#include "imagedev/printer.h"
#include "diserial.h"
#include "cpu/i8085/i8085.h"
#include "bus/a2bus/bitmap_printer.h"
#include "machine/i8155.h"
#include "machine/i8251.h"
#include "machine/steppers.h"
#include "machine/74161.h"

class apple_imagewriter_printer_device : public device_t,
	public device_serial_interface,
	public device_rs232_port_interface
{
public:
	apple_imagewriter_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override { device_serial_interface::rx_w(state); }

	DECLARE_WRITE_LINE_MEMBER(update_serial);

	void optic_handler(uint8_t data) { printf("HANDLE OPTIC %d\n",data);}
	void rxrdy_handler(uint8_t data) {
			printf("HANDLE RXRDY %d\n",data);
			m_maincpu->set_input_line(I8085_RST55_LINE, data);
	}

protected:
	apple_imagewriter_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;  // gotta be const and const!
	virtual void rcv_complete() override;
	int m_initial_rx_state;

	void mem_map(address_map &map);
	void io_map(address_map &map);

private:
	DECLARE_WRITE_LINE_MEMBER(printer_online);

	required_device<printer_image_device> m_printer;
	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8251_device> m_uart;
	required_device<i8155_device> m_8155head;
	required_device<i8155_device> m_8155switch;
	required_device<ttl74163_device> m_count;

	required_device<bitmap_printer_device> m_bitmap_printer;
	required_device<stepper_device> m_pf_stepper;
	required_device<stepper_device> m_cr_stepper;

	required_ioport m_rs232_rxbaud;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;

	void maincpu_out_sod_func(uint8_t data);

	uint8_t head_pa_r(offs_t offset);
	void head_pa_w(uint8_t data);
	uint8_t head_pb_r(offs_t offset);
//  void head_pb_w(offs_t offset, uint8_t data);    <<  not an offset, data  should be just data
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


	void testing_timerout(uint8_t data) {   printf("TESTING TIMEROUT data= %x   TIME = %f  %s\n",data, machine().time().as_double(), machine().describe_context().c_str());}

	int xdirection = 0;
	int newpageflag = 0;

	int page_count = 0;

	const int dpi = 144;
	const double PAPER_WIDTH_INCHES = 8.5;
	const double PAPER_HEIGHT_INCHES = 11.0;
	const double MARGIN_INCHES = 0.5;
	const int PAPER_WIDTH  = PAPER_WIDTH_INCHES * dpi;  // 8.5 inches wide
	const int PAPER_HEIGHT = PAPER_HEIGHT_INCHES * dpi;   // 11  inches high
	const int PAPER_SCREEN_HEIGHT = 384; // match the height of the apple II driver
	const int distfrombottom = 50;

	int m_xpos = PAPER_WIDTH / 2;  // middle of paper (paper width in pixels)
	int m_ypos = 30;
	s32 x_pixel_coord(s32 xpos) { return xpos / 1; }  // x position
	s32 y_pixel_coord(s32 ypos) { return ypos / 2; }  // y position given in half steps

	int update_stepper_delta(stepper_device * stepper, uint8_t stepper_pattern, const char * name, int direction);
	void update_printhead();
	void update_pf_stepper(uint8_t data);
	void update_cr_stepper(uint8_t data);

	int m_ic17_flipflop = 0;
	int m_head_to_last = 0;
	int m_head_pb_last = 0;
	u16 m_dotpattern;

	int m_select_status;
	int right_offset = 0;
	int left_offset = 3;
	int m_left_edge  = MARGIN_INCHES / 2.0 * dpi;
	int m_right_edge = (PAPER_WIDTH_INCHES - MARGIN_INCHES) * dpi + m_left_edge - 1;

	// (should be trivial to make a 15" wide printer by adjusting PAPER_WIDTH_INCHES to 15.00)
	// m_right_edge will get adjusted accordingly to set the return switch

	void darken_pixel(double darkpct, unsigned int& pixel);
	void update_head_pos();
};

DECLARE_DEVICE_TYPE(APPLE_IMAGEWRITER_PRINTER, apple_imagewriter_printer_device)

#endif // MAME_BUS_IMAGEWRITER_PRINTER_H
