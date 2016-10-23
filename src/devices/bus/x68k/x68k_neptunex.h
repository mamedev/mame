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

#ifndef X68K_NEPTUNEX_H_
#define X68K_NEPTUNEX_H_

#include "emu.h"
#include "machine/dp8390.h"
#include "x68kexp.h"

#define NEPTUNE_IRQ_VECTOR 0xf9

class x68k_neptune_device : public device_t,
							public device_x68k_expansion_card_interface
{
public:
	// construction/destruction
	x68k_neptune_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	void x68k_neptune_irq_w(int state);
	uint8_t x68k_neptune_mem_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void x68k_neptune_mem_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t x68k_neptune_port_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_neptune_port_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	x68k_expansion_slot_device *m_slot;

	required_device<dp8390d_device> m_dp8390;
	uint8_t m_board_ram[16*1024];
	uint8_t m_prom[16];
};

// device type definition
extern const device_type X68K_NEPTUNEX;

#endif /* X68K_NEPTUNEX_H_ */
