// license:BSD-3-Clause
// copyright-holders:David Haywood

#pragma once

#ifndef __LPC2103__
#define __LPC2103__

#include "emu.h"
#include "arm7.h"
#include "arm7core.h"

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class lpc210x_device : public arm7_cpu_device
{
public:
	lpc210x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);

	// static configuration helpers

	// todo, use an appropriate flash type instead
	uint8_t m_flash[0x8000];


	uint32_t arm_E01FC088_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t flash_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void flash_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	// timer 0 / 1
	uint32_t timer0_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff) { return read_timer(space, 0, offset, mem_mask); }
	void timer0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff) { write_timer(space, 0, offset, data, mem_mask); }

	uint32_t timer1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff) { return read_timer(space, 1, offset, mem_mask); }
	void timer1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff) { write_timer(space, 1, offset, data, mem_mask); }

	void write_timer(address_space &space, int timer, int offset, uint32_t data, uint32_t mem_mask);
	uint32_t read_timer(address_space &space, int timer, int offset, uint32_t mem_mask);

	uint32_t m_TxPR[2];

	// VIC
	uint32_t vic_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void vic_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	// PIN select block
	uint32_t pin_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void pin_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	//PLL Phase Locked Loop
	uint32_t pll_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void pll_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	//MAM memory controller
	uint32_t mam_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void mam_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	//APB divider
	uint32_t apbdiv_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void apbdiv_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	//syscon misc
	uint32_t scs_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void scs_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	// fio
	uint32_t fio_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void fio_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;






private:
	address_space_config m_program_config;





};


// device type definition
extern const device_type LPC2103;


#endif /// __LPC2103__
