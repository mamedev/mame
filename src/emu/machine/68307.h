/* 68307 */


#pragma once
#ifndef __M68307_H__
#define __M68307_H__

#include "emu.h"
#include "cpu/m68000/m68000.h"

#include "68307sim.h"
#include "68307bus.h"
#include "68307ser.h"
#include "68307tmu.h"




typedef UINT8 (*m68307_porta_read_callback)(address_space &space, bool dedicated, UINT8 line_mask);
typedef void (*m68307_porta_write_callback)(address_space &space, bool dedicated, UINT8 data, UINT8 line_mask);
typedef UINT16 (*m68307_portb_read_callback)(address_space &space, bool dedicated, UINT16 line_mask);
typedef void (*m68307_portb_write_callback)(address_space &space, bool dedicated, UINT16 data, UINT16 line_mask);



class m68307cpu_device : public m68000_device {
public:
	m68307cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);


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
	m68307_serial* m68307SERIAL;
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
	m68307_porta_read_callback m_m68307_porta_r;
	m68307_porta_write_callback m_m68307_porta_w;
	m68307_portb_read_callback m_m68307_portb_r;
	m68307_portb_write_callback m_m68307_portb_w;

	void init16_m68307(address_space &space);
	void init_cpu_m68307(void);

	virtual UINT32 disasm_min_opcode_bytes() const { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const { return 10; };

	virtual UINT32 execute_min_cycles() const { return 4; };
	virtual UINT32 execute_max_cycles() const { return 158; };
protected:

	virtual void device_start();
	virtual void device_reset();

};

static const device_type M68307 = &device_creator<m68307cpu_device>;

extern void m68307_set_port_callbacks(m68307cpu_device *device, m68307_porta_read_callback porta_r, m68307_porta_write_callback porta_w, m68307_portb_read_callback portb_r, m68307_portb_write_callback portb_w);
extern void m68307_set_duart68681(m68307cpu_device* cpudev, mc68681_device *duart68681);
extern UINT16 m68307_get_cs(m68307cpu_device *device, offs_t address);
extern void m68307_timer0_interrupt(m68307cpu_device *cpudev);
extern void m68307_timer1_interrupt(m68307cpu_device *cpudev);
extern void m68307_serial_interrupt(m68307cpu_device *cpudev, int vector);
extern void m68307_mbus_interrupt(m68307cpu_device *cpudev);
extern void m68307_licr2_interrupt(m68307cpu_device *cpudev);

#endif
