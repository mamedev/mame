// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68307 */
#ifndef MAME_MACHINE_68307_H
#define MAME_MACHINE_68307_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"


class m68307_cpu_device : public m68000_device
{
public:
	typedef device_delegate<uint8_t (address_space &space, bool dedicated, uint8_t line_mask)> porta_read_delegate;
	typedef device_delegate<void (address_space &space, bool dedicated, uint8_t data, uint8_t line_mask)> porta_write_delegate;
	typedef device_delegate<uint16_t (address_space &space, bool dedicated, uint16_t line_mask)> portb_read_delegate;
	typedef device_delegate<void (address_space &space, bool dedicated, uint16_t data, uint16_t line_mask)> portb_write_delegate;

	m68307_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	/* trampolines so we can specify the 68681 serial configuration when adding the CPU  */
	auto serial_a_tx_callback() { return m_write_a_tx.bind(); }
	auto serial_b_tx_callback() { return m_write_b_tx.bind(); }
	auto serial_inport_callback() { return m_read_inport.bind(); }
	auto serial_outport_callback() { return m_write_outport.bind(); }

	/* callbacks for internal ports */
	void set_port_callbacks(porta_read_delegate &&porta_r, porta_write_delegate &&porta_w, portb_read_delegate &&portb_r, portb_write_delegate &&portb_w);
	uint16_t get_cs(offs_t address);
	void licr2_interrupt();

protected:
	class m68307_sim;
	class m68307_mbus;
	class m68307_timer;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void reset_peripherals(int state);

	void set_ipl(int level);
	void timer0_interrupt(int state);
	void timer1_interrupt(int state);
	void mbus_interrupt(int state);

	uint8_t int_ack(offs_t offset);

	void m68307_duart_irq_handler(int state);
	void m68307_duart_txa(int state) { m_write_a_tx(state); }
	void m68307_duart_txb(int state) { m_write_b_tx(state);  }
	uint8_t m68307_duart_input_r() { return m_read_inport();  }
	void m68307_duart_output_w(uint8_t data) { m_write_outport(data);  }

	int calc_cs(offs_t address) const;

	uint16_t m68307_internal_base_r(offs_t offset, uint16_t mem_mask = ~0);
	void m68307_internal_base_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t m68307_internal_timer_r(offs_t offset, uint16_t mem_mask = ~0);
	void m68307_internal_timer_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t m68307_internal_sim_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void m68307_internal_sim_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t m68307_internal_serial_r(offs_t offset);
	void m68307_internal_serial_w(offs_t offset, uint8_t data);
	uint8_t m68307_internal_mbus_r(offs_t offset);
	void m68307_internal_mbus_w(offs_t offset, uint8_t data);

	void internal_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;

	devcb_write_line m_write_irq, m_write_a_tx, m_write_b_tx;
	devcb_read8 m_read_inport;
	devcb_write8 m_write_outport;

	/* 68307 peripheral modules */
	m68307_sim*    m_m68307SIM;
	m68307_mbus*   m_m68307MBUS;
//  m68307_serial* m_m68307SERIAL;
	m68307_timer*  m_m68307TIMER;

	uint16_t m_m68307_base;
	uint16_t m_m68307_scrhigh;
	uint16_t m_m68307_scrlow;

	int m_m68307_currentcs;

	uint8_t m_ipl;

	porta_read_delegate  m_porta_r;
	porta_write_delegate m_porta_w;
	portb_read_delegate  m_portb_r;
	portb_write_delegate m_portb_w;

	required_device<mc68681_device> m_duart;
};

DECLARE_DEVICE_TYPE(M68307, m68307_cpu_device)

#endif // MAME_MACHINE_68307_H
