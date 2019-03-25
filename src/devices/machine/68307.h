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

	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint32_t execute_min_cycles() const override { return 4; }
	virtual uint32_t execute_max_cycles() const override { return 158; }

private:
	void set_ipl(int level);
	DECLARE_WRITE_LINE_MEMBER(timer0_interrupt);
	DECLARE_WRITE_LINE_MEMBER(timer1_interrupt);
	DECLARE_WRITE_LINE_MEMBER(mbus_interrupt);

	IRQ_CALLBACK_MEMBER(int_ack);

	DECLARE_WRITE_LINE_MEMBER(m68307_duart_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(m68307_duart_txa) { m_write_a_tx(state); }
	DECLARE_WRITE_LINE_MEMBER(m68307_duart_txb) { m_write_b_tx(state);  }
	DECLARE_READ8_MEMBER(m68307_duart_input_r) { return m_read_inport();  }
	DECLARE_WRITE8_MEMBER(m68307_duart_output_w) { m_write_outport(data);  }

	void init16_m68307(address_space &space);

	int calc_cs(offs_t address) const;

	DECLARE_READ16_MEMBER( m68307_internal_base_r );
	DECLARE_WRITE16_MEMBER( m68307_internal_base_w );
	DECLARE_READ16_MEMBER( m68307_internal_timer_r );
	DECLARE_WRITE16_MEMBER( m68307_internal_timer_w );
	DECLARE_READ16_MEMBER( m68307_internal_sim_r );
	DECLARE_WRITE16_MEMBER( m68307_internal_sim_w );
	DECLARE_READ8_MEMBER( m68307_internal_serial_r );
	DECLARE_WRITE8_MEMBER( m68307_internal_serial_w );
	DECLARE_READ8_MEMBER( m68307_internal_mbus_r );
	DECLARE_WRITE8_MEMBER( m68307_internal_mbus_w );

	void m68307_internal_map(address_map &map);

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
