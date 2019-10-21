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

	DECLARE_READ16_MEMBER(x68k_neptune_port_r);
	DECLARE_WRITE16_MEMBER(x68k_neptune_port_w);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_x68k_expansion_card_interface overrides
	virtual uint8_t iack2() override;

private:
	DECLARE_READ8_MEMBER(x68k_neptune_mem_read);
	DECLARE_WRITE8_MEMBER(x68k_neptune_mem_write);
	void x68k_neptune_irq_w(int state);

	x68k_expansion_slot_device *m_slot;

	required_device<dp8390d_device> m_dp8390;
	uint8_t m_board_ram[16*1024];
	uint8_t m_prom[16];
};

// device type definition
DECLARE_DEVICE_TYPE(X68K_NEPTUNEX, x68k_neptune_device)

#endif // MAME_BUS_X68K_X68K_NEPTUNEX_H
