// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68340 */


#pragma once
#ifndef __M68340_H__
#define __M68340_H__

#include "cpu/m68000/m68000.h"

#include "68340sim.h"
#include "68340dma.h"
#include "68340ser.h"
#include "68340tmu.h"


class m68340cpu_device : public fscpu32_device
{
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

	READ32_MEMBER( m68340_internal_base_r );
	WRITE32_MEMBER( m68340_internal_base_w );
	READ32_MEMBER( m68340_internal_dma_r );
	WRITE32_MEMBER( m68340_internal_dma_w );
	READ32_MEMBER( m68340_internal_serial_r );
	WRITE32_MEMBER( m68340_internal_serial_w );
	READ16_MEMBER( m68340_internal_sim_r );
	READ8_MEMBER( m68340_internal_sim_ports_r );
	READ32_MEMBER( m68340_internal_sim_cs_r );
	WRITE16_MEMBER( m68340_internal_sim_w );
	WRITE8_MEMBER( m68340_internal_sim_ports_w );
	WRITE32_MEMBER( m68340_internal_sim_cs_w );
	READ32_MEMBER( m68340_internal_timer_r );
	WRITE32_MEMBER( m68340_internal_timer_w );

	emu_timer *m_irq_timer;
	TIMER_CALLBACK_MEMBER(periodic_interrupt_timer_callback);
	void start_68340_sim(void);
	void do_timer_irq(void);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

};

extern const device_type M68340;

#endif
