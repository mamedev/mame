// license:BSD-3-Clause
// copyright-holders:MetalliC
#ifndef MAME_MACHINE_M3COMM_H
#define MAME_MACHINE_M3COMM_H

#pragma once

#include "machine/ram.h"
#include "cpu/m68000/m68000.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class m3comm_device : public device_t
{
public:
	// construction/destruction
	m3comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void m3_map(address_map &map);

	DECLARE_READ16_MEMBER(ctrl_r);
	DECLARE_WRITE16_MEMBER(ctrl_w);

	DECLARE_READ16_MEMBER(ioregs_r);
	DECLARE_WRITE16_MEMBER(ioregs_w);

	DECLARE_READ16_MEMBER(m3_m68k_ram_r);
	DECLARE_WRITE16_MEMBER(m3_m68k_ram_w);
	DECLARE_READ8_MEMBER(m3_comm_ram_r);
	DECLARE_WRITE8_MEMBER(m3_comm_ram_w);
	DECLARE_READ16_MEMBER(m3_ioregs_r);
	DECLARE_WRITE16_MEMBER(m3_ioregs_w);

	DECLARE_READ16_MEMBER(naomi_r);
	DECLARE_WRITE16_MEMBER(naomi_w);

	void m3comm_mem(address_map &map);
protected:
	enum { TIMER_IRQ5 = 1 };

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	uint16_t naomi_control;
	uint16_t naomi_offset;
	uint16_t m_status0;
	uint16_t m_status1;
	uint16_t m_commbank;

	uint16_t recv_offset;
	uint16_t recv_size;
	uint16_t send_offset;
	uint16_t send_size;


	emu_file m_line_rx;    // rx line - can be either differential, simple serial or toslink
	emu_file m_line_tx;    // tx line - is differential, simple serial and toslink
	char m_localhost[256];
	char m_remotehost[256];

	emu_timer *timer;

	required_shared_ptr<uint16_t> m68k_ram;
	required_device<m68000_device> m_commcpu;
	required_device<ram_device> m_ram;
};

// device type definition
DECLARE_DEVICE_TYPE(M3COMM, m3comm_device)

#endif  // MAME_MACHINE_M3COMM_H
