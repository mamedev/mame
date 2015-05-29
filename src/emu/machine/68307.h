// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68307 */


#pragma once
#ifndef __M68307_H__
#define __M68307_H__

#include "emu.h"
#include "cpu/m68000/m68000.h"

#include "68307sim.h"
#include "68307bus.h"
#include "68307tmu.h"
#include "machine/mc68681.h"


typedef device_delegate<UINT8 (address_space &space, bool dedicated, UINT8 line_mask)> m68307_porta_read_delegate;
typedef device_delegate<void (address_space &space, bool dedicated, UINT8 data, UINT8 line_mask)> m68307_porta_write_delegate;
typedef device_delegate<UINT16 (address_space &space, bool dedicated, UINT16 line_mask)> m68307_portb_read_delegate;
typedef device_delegate<void (address_space &space, bool dedicated, UINT16 data, UINT16 line_mask)> m68307_portb_write_delegate;


/* trampolines so we can specify the 68681 serial configuration when adding the CPU  */
#define MCFG_MC68307_SERIAL_A_TX_CALLBACK(_cb) \
	devcb = &m68307cpu_device::set_a_tx_cb(*device, DEVCB_##_cb);

#define MCFG_MC68307_SERIAL_B_TX_CALLBACK(_cb) \
	devcb = &m68307cpu_device::set_b_tx_cb(*device, DEVCB_##_cb);

// deprecated: use ipX_w() instead
#define MCFG_MC68307_SERIAL_INPORT_CALLBACK(_cb) \
	devcb = &m68307cpu_device::set_inport_cb(*device, DEVCB_##_cb);

#define MCFG_MC68307_SERIAL_OUTPORT_CALLBACK(_cb) \
	devcb = &m68307cpu_device::set_outport_cb(*device, DEVCB_##_cb);


class m68307cpu_device : public m68000_device {
public:
	m68307cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	/* trampolines so we can specify the 68681 serial configuration when adding the CPU  */
	template<class _Object> static devcb_base &set_irq_cb(device_t &device, _Object object) { return downcast<m68307cpu_device &>(device).write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_a_tx_cb(device_t &device, _Object object) { return downcast<m68307cpu_device &>(device).write_a_tx.set_callback(object); }
	template<class _Object> static devcb_base &set_b_tx_cb(device_t &device, _Object object) { return downcast<m68307cpu_device &>(device).write_b_tx.set_callback(object); }
	template<class _Object> static devcb_base &set_inport_cb(device_t &device, _Object object) { return downcast<m68307cpu_device &>(device).read_inport.set_callback(object); }
	template<class _Object> static devcb_base &set_outport_cb(device_t &device, _Object object) { return downcast<m68307cpu_device &>(device).write_outport.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER(m68307_duart_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(m68307_duart_txa){ write_a_tx(state); }
	DECLARE_WRITE_LINE_MEMBER(m68307_duart_txb){ write_b_tx(state);  }
	DECLARE_READ8_MEMBER(m68307_duart_input_r){ return read_inport();  }
	DECLARE_WRITE8_MEMBER(m68307_duart_output_w){ write_outport(data);  }
	devcb_write_line write_irq, write_a_tx, write_b_tx;
	devcb_read8 read_inport;
	devcb_write8 write_outport;

	UINT16 simple_read_immediate_16_m68307(offs_t address);


	UINT8 read_byte_m68307(offs_t address);
	UINT16 read_word_m68307(offs_t address);
	UINT32 read_dword_m68307(offs_t address);
	void write_byte_m68307(offs_t address, UINT8 data);
	void write_word_m68307(offs_t address, UINT16 data);
	void write_dword_m68307(offs_t address, UINT32 data);


	/* 68307 peripheral modules */
	m68307_sim*    m68307SIM;
	m68307_mbus*   m68307MBUS;
//  m68307_serial* m68307SERIAL;
	m68307_timer*  m68307TIMER;

	UINT16 m68307_base;
	UINT16 m68307_scrhigh;
	UINT16 m68307_scrlow;

	int m68307_currentcs;


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


	/* callbacks for internal ports */
	void set_port_callbacks(m68307_porta_read_delegate porta_r, m68307_porta_write_delegate porta_w, m68307_portb_read_delegate portb_r, m68307_portb_write_delegate portb_w);
	void set_interrupt(int level, int vector);
	UINT16 get_cs(offs_t address);
	void timer0_interrupt();
	void timer1_interrupt();
	void serial_interrupt(int vector);
	void mbus_interrupt();
	void licr2_interrupt();

	m68307_porta_read_delegate m_m68307_porta_r;
	m68307_porta_write_delegate m_m68307_porta_w;
	m68307_portb_read_delegate m_m68307_portb_r;
	m68307_portb_write_delegate m_m68307_portb_w;

	void init16_m68307(address_space &space);
	void init_cpu_m68307(void);

	virtual UINT32 disasm_min_opcode_bytes() const { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const { return 10; };

	virtual UINT32 execute_min_cycles() const { return 4; };
	virtual UINT32 execute_max_cycles() const { return 158; };

	required_device<mc68681_device> m_duart;
protected:
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();

private:
};

static const device_type M68307 = &device_creator<m68307cpu_device>;


#endif
