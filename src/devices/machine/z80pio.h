// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Zilog Z80 Parallel Input/Output Controller implementation

***************************************************************************
                            _____   _____
                    D2   1 |*    \_/     | 40  D3
                    D7   2 |             | 39  D4
                    D6   3 |             | 38  D5
                   _CE   4 |             | 37  _M1
                  C/_D   5 |             | 36  _IORQ
                  B/_A   6 |             | 35  RD
                   PA7   7 |             | 34  PB7
                   PA6   8 |             | 33  PB6
                   PA5   9 |             | 32  PB5
                   PA4  10 |    Z8420    | 31  PB4
                   GND  11 |             | 30  PB3
                   PA3  12 |             | 29  PB2
                   PA2  13 |             | 28  PB1
                   PA1  14 |             | 27  PB0
                   PA0  15 |             | 26  +5V
                 _ASTB  16 |             | 25  CLK
                 _BSTB  17 |             | 24  IEI
                  ARDY  18 |             | 23  _INT
                    D0  19 |             | 22  IEO
                    D1  20 |_____________| 21  BRDY

***************************************************************************/

#ifndef MAME_MACHINE_Z80PIO_H
#define MAME_MACHINE_Z80PIO_H

#pragma once

#include "machine/z80daisy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> z80pio_device

class z80pio_device :   public device_t,
						public device_z80daisy_interface
{
public:
	enum
	{
		PORT_A = 0,
		PORT_B,
		PORT_COUNT
	};

	// construction/destruction
	z80pio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto out_int_callback() { return m_out_int_cb.bind(); }
	auto in_pa_callback() { return m_in_pa_cb.bind(); }
	auto out_pa_callback() { return m_out_pa_cb.bind(); }
	auto out_ardy_callback() { return m_out_ardy_cb.bind(); }
	auto in_pb_callback() { return m_in_pb_cb.bind(); }
	auto out_pb_callback() { return m_out_pb_cb.bind(); }
	auto out_brdy_callback() { return m_out_brdy_cb.bind(); }

	// I/O line access
	int rdy(int which) { return m_port[which].rdy(); }
	void strobe(int which, bool state) { m_port[which].strobe(state); }
	int rdy_a() { return rdy(PORT_A); }
	int rdy_b() { return rdy(PORT_B); }
	void strobe_a(int state) { strobe(PORT_A, state); }
	void strobe_b(int state) { strobe(PORT_B, state); }

	// control register I/O
	uint8_t control_read();
	void control_write(int offset, uint8_t data) { m_port[offset & 1].control_write(data); }
	void control_a_write(uint8_t data) { control_write(PORT_A, data); }
	void control_b_write(uint8_t data) { control_write(PORT_B, data); }

	// data register I/O
	uint8_t data_read(int offset) { return m_port[offset & 1].data_read(); }
	void data_write(int offset, uint8_t data) { m_port[offset & 1].data_write(data); }
	uint8_t data_a_read() { return data_read(PORT_A); }
	uint8_t data_b_read() { return data_read(PORT_B); }
	void data_a_write(uint8_t data) { data_write(PORT_A, data); }
	void data_b_write(uint8_t data) { data_write(PORT_B, data); }

	// port I/O
	uint8_t port_read(int offset) { return m_port[offset & 1].read(); }
	void port_write(int offset, uint8_t data) { m_port[offset & 1].write(data); }
	void port_write(int offset, int bit, int state) { port_write(offset, (m_port[offset & 1].m_input & ~(1 << bit)) | (state << bit));  }
	uint8_t port_a_read() { return port_read(PORT_A); }
	uint8_t port_b_read() { return port_read(PORT_B); }
	void port_a_write(uint8_t data) { port_write(PORT_A, data); }
	void port_b_write(uint8_t data) { port_write(PORT_B, data); }
	void pa0_w(int state) { port_write(PORT_A, 0, state); }
	void pa1_w(int state) { port_write(PORT_A, 1, state); }
	void pa2_w(int state) { port_write(PORT_A, 2, state); }
	void pa3_w(int state) { port_write(PORT_A, 3, state); }
	void pa4_w(int state) { port_write(PORT_A, 4, state); }
	void pa5_w(int state) { port_write(PORT_A, 5, state); }
	void pa6_w(int state) { port_write(PORT_A, 6, state); }
	void pa7_w(int state) { port_write(PORT_A, 7, state); }
	void pb0_w(int state) { port_write(PORT_B, 0, state); }
	void pb1_w(int state) { port_write(PORT_B, 1, state); }
	void pb2_w(int state) { port_write(PORT_B, 2, state); }
	void pb3_w(int state) { port_write(PORT_B, 3, state); }
	void pb4_w(int state) { port_write(PORT_B, 4, state); }
	void pb5_w(int state) { port_write(PORT_B, 5, state); }
	void pb6_w(int state) { port_write(PORT_B, 6, state); }
	void pb7_w(int state) { port_write(PORT_B, 7, state); }

	// standard read/write, with C/D in bit 1, B/A in bit 0
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// alternate read/write, with C/D in bit 0, B/A in bit 1
	u8 read_alt(offs_t offset);
	void write_alt(offs_t offset, u8 data);

private:
	enum
	{
		MODE_OUTPUT = 0,
		MODE_INPUT,
		MODE_BIDIRECTIONAL,
		MODE_BIT_CONTROL
	};

	enum
	{
		ANY = 0,
		IOR,
		MASK
	};

	enum
	{
		ICW_ENABLE_INT    = 0x80,
		ICW_AND_OR        = 0x40,
		ICW_AND           = 0x40,
		ICW_OR            = 0x00,
		ICW_HIGH_LOW      = 0x20,
		ICW_HIGH          = 0x20,
		ICW_LOW           = 0x00,
		ICW_MASK_FOLLOWS  = 0x10
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	// internal helpers
	void check_interrupts();

	// a single PIO port
	class pio_port
	{
		friend class z80pio_device;

	public:
		pio_port();

		void start(z80pio_device *device, int index);
		void reset();

		void trigger_interrupt();

		int rdy() const { return m_rdy; }
		void set_rdy(bool state);
		void set_mode(int mode);
		void strobe(bool state);

		uint8_t read();
		void write(uint8_t data);

		void control_write(uint8_t data);

		uint8_t data_read();
		void data_write(uint8_t data);

	private:
		void check_interrupts() { m_device->check_interrupts(); }

		z80pio_device *             m_device;
		int                         m_index;

		int m_mode;                 // mode register
		int m_next_control_word;    // next control word
		uint8_t m_input;            // input latch
		uint8_t m_output;           // output latch
		uint8_t m_ior;              // input/output register
		bool m_rdy;                 // ready
		bool m_stb;                 // strobe

		// interrupts
		bool m_ie;                  // interrupt enabled
		bool m_ip;                  // interrupt pending
		bool m_ius;                 // interrupt under service
		uint8_t m_icw;              // interrupt control word
		uint8_t m_vector;           // interrupt vector
		uint8_t m_mask;             // interrupt mask
		bool m_match;               // logic equation match
	};

	// internal state
	pio_port            m_port[2];
	devcb_write_line    m_out_int_cb;

	devcb_read8         m_in_pa_cb;
	devcb_write8        m_out_pa_cb;
	devcb_write_line    m_out_ardy_cb;

	devcb_read8         m_in_pb_cb;
	devcb_write8        m_out_pb_cb;
	devcb_write_line    m_out_brdy_cb;
};


// device type definition
DECLARE_DEVICE_TYPE(Z80PIO, z80pio_device)

#endif // MAME_MACHINE_Z80PIO_H
