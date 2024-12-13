// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb, Olivier Galibert

#ifndef MAME_CPU_M6805_HD6305_H
#define MAME_CPU_M6805_HD6305_H

#pragma once

#include "m6805.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type declarations
DECLARE_DEVICE_TYPE(HD6305V0,  hd6305v0_device)
DECLARE_DEVICE_TYPE(HD6305Y0,  hd6305y0_device)
DECLARE_DEVICE_TYPE(HD6305Y2,  hd6305y2_device)
DECLARE_DEVICE_TYPE(HD63705Z0, hd63705z0_device)


// ======================> hd6305_device

class hd6305_device : public m6805_base_device
{
public:
	auto write_sci_tx() { return m_sci_tx.bind(); }
	auto write_sci_clk() { return m_sci_clk.bind(); }

protected:
	// construction/destruction
	hd6305_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			uint32_t clock,
			device_type const type,
			configuration_params const &params,
			address_map_constructor internal_map);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void interrupt_vector() override;

	virtual bool test_il() override { return m_nmi_state != CLEAR_LINE; }

	template<int Port> u8 port_r();
	template<int Port> void port_w(u8 data);
	template<int Port> u8 port_ddr_r();
	template<int Port> void port_ddr_w(u8 data);

	u8 timer_data_r();
	void timer_data_w(u8 data);
	u8 timer_ctrl_r();
	void timer_ctrl_w(u8 data);
	void timer_update_regs();
	void timer_wait_next_timeout();

	u8 misc_r();
	void misc_w(u8 data);

	u8 sci_ctrl_r();
	void sci_ctrl_w(u8 data);
	u8 sci_ssr_r();
	void sci_ssr_w(u8 data);
	u8 sci_data_r();
	void sci_data_w(u8 data);

	void sci_timer_step();

	TIMER_CALLBACK_MEMBER(timer_cb);
	TIMER_CALLBACK_MEMBER(sci_cb);

	devcb_write_line m_sci_clk;
	devcb_write_line m_sci_tx;

	// maximum 9 I/O ports
	devcb_read8::array<9> m_read_port;
	devcb_write8::array<9> m_write_port;
	std::array<u8, 9> m_port_data;
	std::array<u8, 9> m_port_ddr;
	std::array<u8, 9> m_port_ddr_override;

	emu_timer *m_timer_timer;
	emu_timer *m_timer_sci;

	u64 m_timer_last_update;

	u8 m_tdr;
	u8 m_prescaler;
	u8 m_tcr;
	u8 m_ssr;
	u8 m_scr;
	u8 m_mr;
	u8 m_sci_tx_data;

	bool m_sci_tx_filled;
	u8 m_sci_tx_byte;
	u8 m_sci_tx_step;
};

// ======================> hd6305v0_device

class hd6305v0_device : public hd6305_device
{
public:
	// construction/destruction
	hd6305v0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_porta()  { return m_read_port [0].bind(); }
	auto write_porta() { return m_write_port[0].bind(); }
	auto read_portb()  { return m_read_port [1].bind(); }
	auto write_portb() { return m_write_port[1].bind(); }
	auto read_portc()  { return m_read_port [2].bind(); }
	auto write_portc() { return m_write_port[2].bind(); }
	auto read_portd()  { return m_read_port [3].bind(); }
	auto write_portd() { return m_write_port[3].bind(); }

private:
	void internal_map(address_map &map) ATTR_COLD;
};

// ======================> hd6305y0_device

class hd6305y0_device : public hd6305_device
{
public:
	// construction/destruction
	hd6305y0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_porta()  { return m_read_port [0].bind(); }
	auto write_porta() { return m_write_port[0].bind(); }
	auto read_portb()  { return m_read_port [1].bind(); }
	auto write_portb() { return m_write_port[1].bind(); }
	auto read_portc()  { return m_read_port [2].bind(); }
	auto write_portc() { return m_write_port[2].bind(); }
	auto read_portd()  { return m_read_port [3].bind(); }
	auto write_porte() { return m_write_port[4].bind(); }
	auto write_portf() { return m_write_port[5].bind(); }
	auto read_portg()  { return m_read_port [6].bind(); }
	auto write_portg() { return m_write_port[6].bind(); }

	// port G voltage can override DDR, and read the input pins no matter the DDR value
	void read_portg_override_mask(u8 mask) { m_port_ddr_override[6] = mask; }

protected:
	virtual void device_reset() override ATTR_COLD;

private:
	void internal_map(address_map &map) ATTR_COLD;
};

// ======================> hd6305y2_device

class hd6305y2_device : public hd6305_device
{
public:
	// construction/destruction
	hd6305y2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	void internal_map(address_map &map) ATTR_COLD;
};

// ======================> hd63705z0_device

class hd63705z0_device : public hd6305_device
{
public:
	// construction/destruction
	hd63705z0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void execute_set_input(int inputnum, int state) override;
	virtual void interrupt_vector() override;
	virtual void interrupt() override;

private:
	void internal_map(address_map &map) ATTR_COLD;
};

/****************************************************************************
 * HD6305 section
 ****************************************************************************/

#define HD6305_INT_IRQ1             0x00
#define HD6305_INT_IRQ2             0x01
#define HD6305_INT_TIMER1           0x02
#define HD6305_INT_TIMER2           0x03
#define HD6305_INT_SCI              0x04

/****************************************************************************
 * HD63705 section
 ****************************************************************************/

#define HD63705_A                   M6805_A
#define HD63705_PC                  M6805_PC
#define HD63705_S                   M6805_S
#define HD63705_X                   M6805_X
#define HD63705_CC                  M6805_CC
#define HD63705_NMI_STATE           M6805_IRQ_STATE
#define HD63705_IRQ1_STATE          M6805_IRQ_STATE+1
#define HD63705_IRQ2_STATE          M6805_IRQ_STATE+2
#define HD63705_ADCONV_STATE        M6805_IRQ_STATE+3

#define HD63705_INT_MASK            0x1ff

#define HD63705_INT_IRQ1            0x00
#define HD63705_INT_IRQ2            0x01
#define HD63705_INT_TIMER1          0x02
#define HD63705_INT_TIMER2          0x03
#define HD63705_INT_TIMER3          0x04
#define HD63705_INT_PCI             0x05
#define HD63705_INT_SCI             0x06
#define HD63705_INT_ADCONV          0x07
#define HD63705_INT_NMI             0x08

#endif // MAME_CPU_M6805_HD6305_H
