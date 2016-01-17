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

#ifndef __Z80PIO__
#define __Z80PIO__

#include "cpu/z80/z80daisy.h"


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_Z80PIO_OUT_INT_CB(_devcb) \
	devcb = &z80pio_device::set_out_int_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80PIO_IN_PA_CB(_devcb) \
	devcb = &z80pio_device::set_in_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80PIO_OUT_PA_CB(_devcb) \
	devcb = &z80pio_device::set_out_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80PIO_OUT_ARDY_CB(_devcb) \
	devcb = &z80pio_device::set_out_ardy_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80PIO_IN_PB_CB(_devcb) \
	devcb = &z80pio_device::set_in_pb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80PIO_OUT_PB_CB(_devcb) \
	devcb = &z80pio_device::set_out_pb_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80PIO_OUT_BRDY_CB(_devcb) \
	devcb = &z80pio_device::set_out_brdy_callback(*device, DEVCB_##_devcb);


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
	z80pio_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_int_callback(device_t &device, _Object object) { return downcast<z80pio_device &>(device).m_out_int_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_pa_callback(device_t &device, _Object object) { return downcast<z80pio_device &>(device).m_in_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pa_callback(device_t &device, _Object object) { return downcast<z80pio_device &>(device).m_out_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_ardy_callback(device_t &device, _Object object) { return downcast<z80pio_device &>(device).m_out_ardy_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_pb_callback(device_t &device, _Object object) { return downcast<z80pio_device &>(device).m_in_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pb_callback(device_t &device, _Object object) { return downcast<z80pio_device &>(device).m_out_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_brdy_callback(device_t &device, _Object object) { return downcast<z80pio_device &>(device).m_out_brdy_cb.set_callback(object); }

	// I/O line access
	int rdy(int which) { return m_port[which].rdy(); }
	void strobe(int which, bool state) { m_port[which].strobe(state); }
	DECLARE_READ_LINE_MEMBER( rdy_a ) { return rdy(PORT_A); }
	DECLARE_READ_LINE_MEMBER( rdy_b ) { return rdy(PORT_B); }
	DECLARE_WRITE_LINE_MEMBER( strobe_a ) { strobe(PORT_A, state); }
	DECLARE_WRITE_LINE_MEMBER( strobe_b ) { strobe(PORT_B, state); }

	// control register I/O
	UINT8 control_read();
	void control_write(int offset, UINT8 data) { m_port[offset & 1].control_write(data); }
	void control_a_write(UINT8 data) { control_write(PORT_A, data); }
	void control_b_write(UINT8 data) { control_write(PORT_B, data); }

	// data register I/O
	UINT8 data_read(int offset) { return m_port[offset & 1].data_read(); }
	void data_write(int offset, UINT8 data) { m_port[offset & 1].data_write(data); }
	UINT8 data_a_read() { return data_read(PORT_A); }
	UINT8 data_b_read() { return data_read(PORT_B); }
	void data_a_write(UINT8 data) { data_write(PORT_A, data); }
	void data_b_write(UINT8 data) { data_write(PORT_B, data); }

	// port I/O
	UINT8 port_read(int offset) { return m_port[offset & 1].read(); }
	void port_write(int offset, UINT8 data) { m_port[offset & 1].write(data); }
	void port_write(int offset, int bit, int state) { port_write(offset, (m_port[offset & 1].m_input & ~(1 << bit)) | (state << bit));  }
	UINT8 port_a_read() { return port_read(PORT_A); }
	UINT8 port_b_read() { return port_read(PORT_B); }
	void port_a_write(UINT8 data) { port_write(PORT_A, data); }
	void port_b_write(UINT8 data) { port_write(PORT_B, data); }
	DECLARE_WRITE8_MEMBER( pa_w ) { port_a_write(data); }
	DECLARE_READ8_MEMBER( pa_r ) { return port_a_read(); }
	DECLARE_WRITE8_MEMBER( pb_w ) { port_b_write(data); }
	DECLARE_READ8_MEMBER( pb_r ) { return port_b_read(); }
	DECLARE_WRITE_LINE_MEMBER( pa0_w ) { port_write(PORT_A, 0, state); }
	DECLARE_WRITE_LINE_MEMBER( pa1_w ) { port_write(PORT_A, 1, state); }
	DECLARE_WRITE_LINE_MEMBER( pa2_w ) { port_write(PORT_A, 2, state); }
	DECLARE_WRITE_LINE_MEMBER( pa3_w ) { port_write(PORT_A, 3, state); }
	DECLARE_WRITE_LINE_MEMBER( pa4_w ) { port_write(PORT_A, 4, state); }
	DECLARE_WRITE_LINE_MEMBER( pa5_w ) { port_write(PORT_A, 5, state); }
	DECLARE_WRITE_LINE_MEMBER( pa6_w ) { port_write(PORT_A, 6, state); }
	DECLARE_WRITE_LINE_MEMBER( pa7_w ) { port_write(PORT_A, 7, state); }
	DECLARE_WRITE_LINE_MEMBER( pb0_w ) { port_write(PORT_B, 0, state); }
	DECLARE_WRITE_LINE_MEMBER( pb1_w ) { port_write(PORT_B, 1, state); }
	DECLARE_WRITE_LINE_MEMBER( pb2_w ) { port_write(PORT_B, 2, state); }
	DECLARE_WRITE_LINE_MEMBER( pb3_w ) { port_write(PORT_B, 3, state); }
	DECLARE_WRITE_LINE_MEMBER( pb4_w ) { port_write(PORT_B, 4, state); }
	DECLARE_WRITE_LINE_MEMBER( pb5_w ) { port_write(PORT_B, 5, state); }
	DECLARE_WRITE_LINE_MEMBER( pb6_w ) { port_write(PORT_B, 6, state); }
	DECLARE_WRITE_LINE_MEMBER( pb7_w ) { port_write(PORT_B, 7, state); }

	// standard read/write, with C/D in bit 1, B/A in bit 0
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	// alternate read/write, with C/D in bit 0, B/A in bit 1
	DECLARE_READ8_MEMBER( read_alt );
	DECLARE_WRITE8_MEMBER( write_alt );

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
	virtual void device_start() override;
	virtual void device_reset() override;

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

		UINT8 read();
		void write(UINT8 data);

		void control_write(UINT8 data);

		UINT8 data_read();
		void data_write(UINT8 data);

	private:
		void check_interrupts() { m_device->check_interrupts(); }

		z80pio_device *             m_device;
		int                         m_index;

		int m_mode;                 // mode register
		int m_next_control_word;    // next control word
		UINT8 m_input;              // input latch
		UINT8 m_output;             // output latch
		UINT8 m_ior;                // input/output register
		bool m_rdy;                 // ready
		bool m_stb;                 // strobe

		// interrupts
		bool m_ie;                  // interrupt enabled
		bool m_ip;                  // interrupt pending
		bool m_ius;                 // interrupt under service
		UINT8 m_icw;                // interrupt control word
		UINT8 m_vector;             // interrupt vector
		UINT8 m_mask;               // interrupt mask
		bool m_match;               // logic equation match
	};

	// internal state
	pio_port             m_port[2];
	devcb_write_line    m_out_int_cb;

	devcb_read8         m_in_pa_cb;
	devcb_write8        m_out_pa_cb;
	devcb_write_line    m_out_ardy_cb;

	devcb_read8         m_in_pb_cb;
	devcb_write8        m_out_pb_cb;
	devcb_write_line    m_out_brdy_cb;
};


// device type definition
extern const device_type Z80PIO;


#endif
