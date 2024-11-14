// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * x68k_neptunex.h
 *
 * Neptune-X NE2000-based ethernet board for the X68000
 *
 * Map:
 * 0xECE000-0xECE3FF: "Y0"
 * 0xECE400-0xECE7FF: "Y1"
 */

#ifndef MAME_BUS_X68K_X68K_NEPTUNEX_H
#define MAME_BUS_X68K_X68K_NEPTUNEX_H

#pragma once

#include "x68kexp.h"
#include "machine/dp8390.h"

#define NEPTUNE_IRQ_VECTOR 0xf9

class x68k_neptune_device : public device_t,
							public device_x68k_expansion_card_interface
{
public:
	// construction/destruction
	x68k_neptune_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t x68k_neptune_port_r(offs_t offset, uint16_t mem_mask = ~0);
	void x68k_neptune_port_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_x68k_expansion_card_interface overrides
	virtual uint8_t iack2() override;

private:
	uint8_t x68k_neptune_mem_read(offs_t offset);
	void x68k_neptune_mem_write(offs_t offset, uint8_t data);
	void x68k_neptune_irq_w(int state);

	x68k_expansion_slot_device *m_slot;

	required_device<dp8390d_device> m_dp8390;
	uint8_t m_board_ram[16*1024];
	uint8_t m_prom[16];
};

// device type definition
DECLARE_DEVICE_TYPE(X68K_NEPTUNEX, x68k_neptune_device)

#endif // MAME_BUS_X68K_X68K_NEPTUNEX_H
