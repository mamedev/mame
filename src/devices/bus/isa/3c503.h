// license:BSD-3-Clause
// copyright-holders:Carl
/* 3com Etherlink II 3c503 */

#ifndef MAME_BUS_ISA_3C503_H
#define MAME_BUS_ISA_3C503_H

#pragma once

#include "isa.h"
#include "machine/dp8390.h"

class el2_3c503_device : public device_t, public device_isa8_card_interface
{
public:
	el2_3c503_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t el2_3c503_loport_r(offs_t offset);
	void el2_3c503_loport_w(offs_t offset, uint8_t data);
	uint8_t el2_3c503_hiport_r(offs_t offset);
	void el2_3c503_hiport_w(offs_t offset, uint8_t data);
	void eop_w(int state) override;
	uint8_t dack_r(int line) override;
	void dack_w(int line, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void el2_3c503_irq_w(int state);

	required_device<dp8390d_device> m_dp8390;
	uint8_t m_board_ram[8*1024];
	uint8_t m_rom[8*1024];
	uint8_t m_prom[32];
	uint8_t m_irq_state;

	uint8_t el2_3c503_mem_read(offs_t offset);
	void el2_3c503_mem_write(offs_t offset, uint8_t data);

	void set_irq(int state);
	void set_drq(int state);

	struct {
		uint8_t pstr;
		uint8_t pspr;
		uint8_t dqtr;
		uint8_t bcfr;
		uint8_t pcfr;
		uint8_t gacfr;
		uint8_t ctrl;
		uint8_t streg;
		uint8_t idcfr;
		uint16_t da;
		uint32_t vptr;
		uint8_t rfmsb;
		uint8_t rflsb;
	} m_regs;
};

DECLARE_DEVICE_TYPE(EL2_3C503, el2_3c503_device)

#endif // MAME_BUS_ISA_3C503_H
