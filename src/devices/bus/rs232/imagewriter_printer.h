// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_BUS_IMAGEWRITER_PRINTER_H
#define MAME_BUS_IMAGEWRITER_PRINTER_H

#pragma once

// uncomment this to use the device serial interface for debugging
//#define IMAGEWRITER_USE_DEVICE_SERIAL_INTERFACE



#include "rs232.h"
#ifdef IMAGEWRITER_USE_DEVICE_SERIAL_INTERFACE
#include "imagedev/printer.h"
#endif
#ifdef IMAGEWRITER_USE_DEVICE_SERIAL_INTERFACE
#include "diserial.h"
#endif
#include "cpu/i8085/i8085.h"
#include "bus/a2bus/bitmap_printer.h"
#include "machine/i8155.h"
#include "machine/i8251.h"
#include "machine/steppers.h"
#include "machine/74123.h"
#include "machine/74161.h"
#include "machine/timer.h"

// uncomment this to use the device serial interface for debugging
//#define IMAGEWRITER_USE_DEVICE_SERIAL_INTERFACE

class apple_imagewriter_printer_device : public device_t,
#ifdef IMAGEWRITER_USE_DEVICE_SERIAL_INTERFACE
	public device_serial_interface,
#endif
	public device_rs232_port_interface
{
public:
	apple_imagewriter_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	double last_time = 0.0;
	int last_value;

	double min_time = 1.0 / 9601.0;  // if I make it 1.0 / 9600.0 the division tends to round down

	std::string buildstring = "";

	int ioportsaferead(const char * name)
	{
		// Safe read of ioport (mame does not allow ioport read at init time)
		// Avoids the following error:
		//   Ignoring MAME exception: Input ports cannot be read at init time!
		//   Fatal error: Input ports cannot be read at init time!
		if (ioport(name)->manager().safe_to_read()) return ioport(name)->read();
		else return 0;
	}

//  virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override { device_serial_interface::rx_w(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override
	{
		// pipe the bit state along
#ifdef IMAGEWRITER_USE_DEVICE_SERIAL_INTERFACE
		device_serial_interface::rx_w(state);
#endif
		m_uart-> write_rxd(state);

#ifdef IMAGEWRITER_USE_DEVICE_SERIAL_INTERFACE
		double elapsed_time = machine().time().as_double() - last_time;
		if (ioportsaferead("DEBUGMSG")) printf("INPUT TXD STATE = %x   %f  elapsed=%f\n", state, machine().time().as_double(), machine().time().as_double()-last_time);
		if (min_time > 0.0)
		{
			int block = elapsed_time / min_time;

			if (ioportsaferead("DEBUGMSG"))printf("BLOCK = %d\n",block);
			for (int i=0; i< std::min(16,block); i++)
			{
				if (buildstring.length() < 32)
					buildstring = std::to_string(last_value) + buildstring;
				if (ioportsaferead("DEBUGMSG")) printf("%d",last_value);
			}
			if (ioportsaferead("DEBUGMSG")) printf("\n");
		}
		last_time = machine().time().as_double();
		last_value = state;
#endif
	}
#ifdef IMAGEWRITER_USE_DEVICE_SERIAL_INTERFACE
	DECLARE_WRITE_LINE_MEMBER(update_serial);
#endif

	INPUT_CHANGED_MEMBER(reset_sw)
	{
		if (newval == 0) m_maincpu->reset();
	}

	INPUT_CHANGED_MEMBER(select_sw)
	{   // output from comparator is 5v if switch open, 260mv if switch closed so on press goes from 1 to 0,
		// transition from 1 to 0 clocks the flipflop
		if (oldval == 1 && newval == 0)
		{
//          printf("oldval,newval = %x,%x  flip = %x\n",oldval,newval,m_ic17_flipflop_select_status);
			m_ic17_flipflop_select_status = !m_ic17_flipflop_select_status;
		}
	}

	void rxrdy_handler(uint8_t data)
	{
//      if (data == 1) printf("HANDLE RXRDY %d\n",data);
		m_maincpu->set_input_line(I8085_RST65_LINE, data);
	}

	int m_pulse1_out_last = 1;

	void pulse1_out_handler(uint8_t data) {
		if (m_pulse1_out_last == 1 && data == 0) update_printhead();
		m_pulse1_out_last = data;
//      printf("PULSE1OUT %x\n",data);
	}
	void pulse2_out_handler(uint8_t data) {
		m_pulse1->a_w(data);
//      m_pulse1->b_w(1);  // always 1
//      printf("PULSE2OUT %x\n",data);
	}

	uint8_t maincpu_in_sid_func()
	{
//      printf("CALLING IN SID FUNCTION  value to return = %x\n", ioportsaferead("WIDTH"));
		return ioportsaferead("WIDTH");
	}


// y position adjustment to eliminate gaps from win95 driver
//emu.item(manager.machine.devices[":board2:comat:serport1:imagewriter"].items["0/yposratio0"]):write(0,16)
//emu.item(manager.machine.devices[":board2:comat:serport1:imagewriter"].items["0/yposratio1"]):write(0,18)

//emu.item(manager.machine.devices[":board2:comat:serport1:imagewriter"].items["0/xposratio0"]):write(0,9)
//emu.item(manager.machine.devices[":board2:comat:serport1:imagewriter"].items["0/xposratio1"]):write(0,8)


	// experiment with ct486, dtr -> dsr and cts, or just dtr -> cts both seems to work

	// I think it should be dtr -> dsr and rts -> cts, but ct486 and win95 corrupts printout

	void dtr_handler(uint8_t data) {
		if (ioport("DTR")->read() & 0x1) output_dsr(data ^ ioport("INVERT1")->read());
		if (ioport("DTR")->read() & 0x2) output_cts(data ^ ioport("INVERT1")->read());
//      output_dsr(data);
	}

	void rts_handler(uint8_t data) {
		if (ioport("RTS")->read() & 0x1) output_dsr(data ^ ioport("INVERT2")->read());
		if (ioport("RTS")->read() & 0x2) output_cts(data ^ ioport("INVERT2")->read());
//      output_cts(data);
	}

	void txd_handler(uint8_t data) {
		output_rxd(data);
	}


	u8 m_uart_clock = 0;

	double last_pulse = 0.0;
	TIMER_DEVICE_CALLBACK_MEMBER (pulse_uart_clock)
	{

	// 9600   8251 m_br_factor 16  /1 of 9600 clock
	// 2400   8251 m_br_factor 64  /1 of 9600 clock  same clock as 9600
	// 1200   8251 m_br_factor 16  /8 of 9600 clock
	// 300    8251 m_br_factor 64  /8 of 9600 clock  same clock as 1200

		++ m_baud_clock_divisor_delay %= m_baud_clock_divisor;  // increment divisor delay and wrap around
		if (m_baud_clock_divisor_delay == 0)
		{
			m_uart_clock = !m_uart_clock; m_uart->write_txc(m_uart_clock); m_uart->write_rxc(m_uart_clock);
			[[maybe_unused]] double now = machine().time().as_double();
			[[maybe_unused]] double elapsed_time = now - last_pulse;
	//  printf("pulse uart param=%x   %f  elapsed=%.15f\n", param, now, elapsed_time);
			last_pulse = now;
		}
	 }
protected:
	apple_imagewriter_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
//  virtual void device_reset_after_children() override;
	virtual const tiny_rom_entry *device_rom_region() const override;  // gotta be const and const!
#ifdef IMAGEWRITER_USE_DEVICE_SERIAL_INTERFACE
	virtual void rcv_complete() override;
#endif
	int m_initial_rx_state;

	void mem_map(address_map &map);
	void io_map(address_map &map);

private:
	DECLARE_WRITE_LINE_MEMBER(printer_online);
#ifdef IMAGEWRITER_USE_DEVICE_SERIAL_INTERFACE
	required_device<printer_image_device> m_printer;
#endif
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

#ifdef IMAGEWRITER_USE_DEVICE_SERIAL_INTERFACE
	required_ioport m_rs232_rxbaud;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;
#endif

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


	int m_baud_clock_divisor = 1;
	int m_baud_clock_divisor_delay = 0;

	int m_ic17_flipflop_head = 0;           // connected to 8155 head     (ic17 7474 part 1/2)
	int m_ic17_flipflop_select_status = 0;  // connected to 8155 switches (ic17 7474 part 2/2)
	int m_head_to_last = 0;
	int m_head_pb_last = 0;
	int m_switches_pc_last = 0;
	int m_switches_to_last = 0;
	u16 m_dotpattern = 0;  // got to initialize it or get junk on startup

	int m_left_edge_adjust = -6;  // to get perfect centering with macpaint

	// Centering is kind of tricky because it seems that the printer actually does 162 steps horizontally instead
	// of 160 when printing at 144 dpi, but when it is set to print at 160 dpi it actually does 160 dpi
	// so exact width means 1296 for macpaint 144dpi 8" but 1280 for amiga 160 dpi 0-80 margin.  (8*2 step difference)
	// It's minor, but if you want exact centering it makes a difference.

	int right_offset = 0;
	int left_offset  = 0;
	int m_left_edge  = (MARGIN_INCHES) * dpi * xscale + m_left_edge_adjust;    // 0 for starting at left edge, print shop seems to like -32 for centered print
	int m_right_edge = (PAPER_WIDTH_INCHES + MARGIN_INCHES) * dpi * xscale - 1;  // when it hits the right edge, will return to the left edge and go deselected (so subtracting margin_inches is enough to stop the self test)

	// (should be trivial to make a 15" wide printer by adjusting PAPER_WIDTH_INCHES to 15.00)
	// m_right_edge will get adjusted accordingly to set the return switch

	void darken_pixel(double darkpct, unsigned int& pixel);
	void update_head_pos();
};

DECLARE_DEVICE_TYPE(APPLE_IMAGEWRITER_PRINTER, apple_imagewriter_printer_device)

#endif // MAME_BUS_IMAGEWRITER_PRINTER_H
