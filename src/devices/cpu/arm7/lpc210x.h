// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_ARM7_LPC2103_H
#define MAME_CPU_ARM7_LPC2103_H

#pragma once

#include "arm7.h"

#include "machine/vic_pl192.h"


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

	uint32_t arm_E01FC088_r();
	uint32_t flash_r(offs_t offset);
	void flash_w(offs_t offset, uint32_t data);

	// timer 0 / 1
	uint32_t timer0_r(offs_t offset, uint32_t mem_mask = ~0) { return read_timer(0, offset, mem_mask); }
	void timer0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { write_timer(0, offset, data, mem_mask); }

	uint32_t timer1_r(offs_t offset, uint32_t mem_mask = ~0) { return read_timer(1, offset, mem_mask); }
	void timer1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { write_timer(1, offset, data, mem_mask); }

	void write_timer(int timer, int offset, uint32_t data, uint32_t mem_mask);
	uint32_t read_timer(int timer, int offset, uint32_t mem_mask);

	// PIN select block
	uint32_t pin_r(offs_t offset, uint32_t mem_mask = ~0);
	void pin_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	//PLL Phase Locked Loop
	uint32_t pll_r(offs_t offset, uint32_t mem_mask = ~0);
	void pll_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	//MAM memory controller
	uint32_t mam_r(offs_t offset, uint32_t mem_mask = ~0);
	void mam_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	//APB divider
	uint32_t apbdiv_r(offs_t offset, uint32_t mem_mask = ~0);
	void apbdiv_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	//syscon misc
	uint32_t scs_r(offs_t offset, uint32_t mem_mask = ~0);
	void scs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// fio
	uint32_t fio_r(offs_t offset, uint32_t mem_mask = ~0);
	void fio_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// todo, use an appropriate flash type instead
	uint8_t m_flash[0x8000]; // needs to be public because the harmony/melody device injects contents with memcpy, yuck

	void lpc2103_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

	uint32_t m_TxPR[2];

private:
	address_space_config m_program_config;

	required_device<vic_pl190_device> m_vic;
};


// device type definition
DECLARE_DEVICE_TYPE(LPC2103, lpc210x_device)

#endif // MAME_CPU_ARM7_LPC2103_H
