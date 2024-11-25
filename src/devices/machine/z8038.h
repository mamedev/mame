// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_Z8038_H
#define MAME_MACHINE_Z8038_H

#pragma once

class z8038_device : public device_t
{
public:
	z8038_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// port 1 and 2 I̅N̅T̅ output lines
	template <u8 Port> auto out_int_cb() { return m_out_int_cb[Port - 1].bind(); }

	// port 2 output lines
	auto out_E() { return m_out_E_cb.bind(); }
	auto out_F() { return m_out_F_cb.bind(); }
	auto out_H() { return m_out_H_cb.bind(); }
	auto out_J() { return m_out_J_cb.bind(); }

	// port 2 input lines
	void in_E(int state); // C̅L̅E̅A̅R̅
	void in_F(int state); // Data Direction
	void in_G(int state); // IN0

	// indirect register access
	template <u8 Port> u8 reg_r()           { return reg_r(Port - 1); }
	template <u8 Port> void reg_w(u8 data)  { reg_w(Port - 1, data); }

	// direct register access
	template <u8 Port> void zbus_map(address_map &map) ATTR_COLD;
	template <u8 Port> u8 zbus_reg_r(offs_t offset)            { m_port[Port - 1].reg_state = 1; m_port[Port - 1].reg_pointer = offset & 0xf; return reg_r(Port - 1); }
	template <u8 Port> void zbus_reg_w(offs_t offset, u8 data) { m_port[Port - 1].reg_state = 1; m_port[Port - 1].reg_pointer = offset & 0xf; reg_w(Port - 1, data); }

	// direct fifo access
	template <u8 Port> u8 fifo_r()          { return fifo_r(Port - 1); }
	template <u8 Port> void fifo_w(u8 data) { fifo_w(Port - 1, data); }

protected:
	// standard device_interface overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// primary device read/write handlers
	u8 reg_r(u8 const port);
	void reg_w(u8 const port, u8 data);
	u8 fifo_r(u8 const port);
	void fifo_w(u8 const port, u8 data);

	// register read helpers
	u8 control_0_r(u8 const port) { return m_port[port].control_0; }
	u8 control_1_r(u8 const port);
	template <u8 Number> u8 interrupt_status_r(u8 const port) { return m_port[port].interrupt_status[Number]; }
	u8 interrupt_vector_r(u8 const port);
	u8 byte_count_r(u8 const port);
	u8 byte_count_comparison_r(u8 const port) { return m_port[port].byte_count_comparison; }
	u8 control_2_r(u8 const port) { return (port == 0) ? m_control_2 : 0; }
	u8 control_3_r(u8 const port);
	u8 message_out_r(u8 const port) { return m_port[!port].message_in; }
	u8 message_in_r(u8 const port);
	u8 pattern_match_r(u8 const port) { return m_port[port].pattern_match; }
	u8 pattern_mask_r(u8 const port) { return m_port[port].pattern_mask; }

	// register write helpers
	void control_0_w(u8 const port, u8 data);
	void control_1_w(u8 const port, u8 data);
	template <u8 Number> void interrupt_status_w(u8 const port, u8 data);
	void interrupt_vector_w(u8 const port, u8 data) { m_port[port].interrupt_vector = data; }
	void byte_count_comparison_w(u8 const port, u8 data);
	void control_2_w(u8 const port, u8 data);
	void control_3_w(u8 const port, u8 data);
	void message_out_w(u8 const port, u8 data);
	void pattern_match_w(u8 const port, u8 data) { m_port[port].pattern_match = data; }
	void pattern_mask_w(u8 const port, u8 data) { m_port[port].pattern_mask = data; }

	// other helpers
	void port_reset(u8 const port);
	void fifo_clear();
	void fifo_update();
	TIMER_CALLBACK_MEMBER(int_check);
	void set_int_state(u8 const port, bool asserted);

private:
	enum control_0_mask : u8
	{
		CR0_RESET = 0x01, // reset port
		CR0_RJA   = 0x02, // right-justify address
		CR0_P2M   = 0x0c, // port 2 mode (read-only from port 2 side)
		CR0_VIS   = 0x10, // vector includes status
		CR0_NV    = 0x20, // no vector on interrupt
		CR0_DLC   = 0x40, // disable lower daisy chain
		CR0_MIE   = 0x80, // master interrupt enable
	};
	enum control_0_p2mode_mask : u8
	{
		CR0_P2M_ZBUSCPU  = 0x00, // z-bus cpu
		CR0_P2M_NZBUSCPU = 0x04, // non z-bus cpu
		CR0_P2M_3WHSIO   = 0x08, // 3-wire handshake i/o
		CR0_P2M_2WHSIO   = 0x0c, // 2-wire handshake i/o

		CR0_P2M_IO       = 0x08, // i/o mode
	};
	enum control_1_mask : u8
	{
		CR1_RWE   = 0x01, // R̅E̅Q̅U̅E̅S̅T̅ or W̅A̅I̅T̅ enable
		CR1_RWS   = 0x02, // R̅E̅Q̅U̅E̅S̅T̅ or W̅A̅I̅T̅ select
		CR1_SDBC  = 0x04, // start dma on byte count
		CR1_SDPM  = 0x08, // stop dma on pattern match
		CR1_MMRUS = 0x10, // message mailbox register under service (read only)
		CR1_MMRF  = 0x20, // message mailbox register full (read only)
		CR1_FBCR  = 0x40, // freeze byte count register

		CR1_WMASK = 0x4f,
	};
	enum control_2_mask : u8
	{
		CR2_P2EN  = 0x01, // port 2 side enable
		CR2_P2HE  = 0x02, // port 2 side handshake enable

		CR2_WMASK = 0x03,
	};
	enum control_3_mask : u8
	{
		CR3_P2IN0  = 0x01, // port 2 input line 0
		CR3_P2OUT1 = 0x02, // port 2 output line 1
		CR3_UNUSED = 0x04, // not used (mut be programmed 0)
		CR3_P2OUT3 = 0x08, // port 2 output line 3
		CR3_DIR    = 0x10, // data direction
		CR3_P2DIR  = 0x20, // port 2 controls direction
		CR3_CLR    = 0x40, // clear fifo (active low)
		CR3_P2CLR  = 0x80, // port 2 controls clear
	};
	enum interrupt_status_0_mask : u8
	{
		ISR0_MIP    = 0x20, // message interrupt pending
		ISR0_MIE    = 0x40, // message interrupt enable
		ISR0_MIUS   = 0x80, // message interrupt under service

		ISR0_WMASK  = 0xe0
	};
	enum interrupt_status_1_mask : u8
	{
		ISR1_PMF    = 0x01, // pattern match flag
		ISR1_PMIP   = 0x02, // pattern match interrupt pending
		ISR1_PMIE   = 0x04, // pattern match interrupt enable
		ISR1_PMIUS  = 0x08, // pattern match interrupt under service
		ISR1_DDCIP  = 0x20, // data direction change interrupt pending
		ISR1_DDCIE  = 0x40, // data direction change interrupt enable
		ISR1_DDCIUS = 0x80, // data direction change interrupt under service
	};
	enum interrupt_status_2_mask : u8
	{
		ISR2_UF     = 0x01, // underflow error
		ISR2_EIP    = 0x02, // error interrupt pending
		ISR2_EIE    = 0x04, // error interrupt enable
		ISR2_EIUS   = 0x08, // error interrupt under service
		ISR2_OF     = 0x10, // overflow error
		ISR2_BCCIP  = 0x20, // byte count compare interrupt pending
		ISR2_BCCIE  = 0x40, // byte count compare interrupt enable
		ISR2_BCCIUS = 0x80, // byte count compare interrupt under service
	};
	enum interrupt_status_3_mask : u8
	{
		ISR3_BE   = 0x01, // buffer empty
		ISR3_EIP  = 0x02, // empty interrupt pending
		ISR3_EIE  = 0x04, // empty interrupt enable
		ISR3_EIUS = 0x08, // empty interrupt under service
		ISR3_BF   = 0x10, // buffer full
		ISR3_FIP  = 0x20, // full interrupt pending
		ISR3_FIE  = 0x40, // full interrupt enable
		ISR3_FIUS = 0x80, // full interrupt under service
	};

	// generic interrupt status bit names
	enum interrupt_status_mask : u8
	{
		ISR_LIP  = 0x02,  // low interrupt pending
		ISR_LIE  = 0x04,  // low interrupt enable
		ISR_LIUS = 0x08,  // low interrupt under service
		ISR_HIP  = 0x20,  // high interrupt pending
		ISR_HIE  = 0x40,  // high interrupt enable
		ISR_HIUS = 0x80,  // high interrupt under service

		ISR_LMASK = 0x0e,
		ISR_HMASK = 0xe0,
	};

	devcb_write_line::array<2> m_out_int_cb;
	devcb_write_line m_out_E_cb; // pin number 35
	devcb_write_line m_out_F_cb; // pin number 34
	devcb_write_line m_out_H_cb; // pin number 32
	devcb_write_line m_out_J_cb; // pin number 30

	emu_timer *m_int_check;

	// registers largely or entirely controlled by port 1
	u8 m_control_2;
	u8 m_control_3;

	struct port_state
	{
		// non-register, per-port state
		u8 reg_state;
		u8 reg_pointer;
		u8 int_code;
		bool int_asserted;

		// accessible registers
		u8 control_0;
		u8 control_1;
		u8 interrupt_status[4];
		u8 interrupt_vector;
		u8 byte_count;
		u8 byte_count_comparison;
		u8 message_in;
		u8 pattern_match;
		u8 pattern_mask;
		u8 data_buffer;
	}
	m_port[2];

	util::fifo <u8, 128> m_fifo;
};

DECLARE_DEVICE_TYPE(Z8038, z8038_device)

#endif // MAME_MACHINE_Z8038_H
