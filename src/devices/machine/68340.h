// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68340 */


#pragma once
#ifndef __M68340_H__
#define __M68340_H__

#include "emu.h"
#include "cpu/m68000/m68000.h"

#include "68340sim.h"
#include "68340dma.h"
#include "68340ser.h"
#include "68340tmu.h"





class m68340cpu_device : public fscpu32_device {
public:
	m68340cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);


	int m68340_currentcs;

	/* 68340 peripheral modules */
	m68340_sim*    m68340SIM;
	m68340_dma*    m68340DMA;
	m68340_serial* m68340SERIAL;
	m68340_timer*  m68340TIMER;

	uint32_t m68340_base;

	uint16_t m_avr;
	uint16_t m_picr;
	uint16_t m_pitr;

	uint16_t get_cs(offs_t address);

	uint32_t m68340_internal_base_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void m68340_internal_base_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t m68340_internal_dma_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void m68340_internal_dma_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t m68340_internal_serial_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void m68340_internal_serial_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	uint16_t m68340_internal_sim_r(address_space &space, offs_t offset, uint16_t mem_mask);
	uint8_t m68340_internal_sim_ports_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint32_t m68340_internal_sim_cs_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void m68340_internal_sim_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);
	void m68340_internal_sim_ports_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void m68340_internal_sim_cs_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t m68340_internal_timer_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void m68340_internal_timer_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);

	emu_timer *m_irq_timer;
	void periodic_interrupt_timer_callback(void *ptr, int32_t param);
	void start_68340_sim(void);
	void do_timer_irq(void);
protected:

	virtual void device_start() override;
	virtual void device_reset() override;

};

static const device_type M68340 = &device_creator<m68340cpu_device>;





#endif
