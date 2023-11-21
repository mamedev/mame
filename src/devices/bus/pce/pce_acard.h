// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Angelo Salese
#ifndef MAME_BUS_PCE_PCE_ACARD_H
#define MAME_BUS_PCE_PCE_ACARD_H

#pragma once

#include "pce_scdsys.h"
#include "pce_slot.h"


// ======================> pce_acard_duo_device

class pce_acard_duo_device : public device_t,
						public device_pce_cart_interface
{
public:
	// construction/destruction
	pce_acard_duo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual void install_memory_handlers(address_space &space) override;

protected:
	// construction/destruction
	pce_acard_duo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	uint8_t peripheral_r(offs_t offset);
	void peripheral_w(offs_t offset, uint8_t data);

	/* Arcade Card specific */
	memory_share_creator<uint8_t> m_ram;
	uint8_t   m_ctrl[4]{};
	uint32_t  m_base_addr[4]{};
	uint16_t  m_addr_offset[4]{};
	uint16_t  m_addr_inc[4]{};
	uint32_t  m_shift = 0;
	uint8_t   m_shift_reg = 0;
	uint8_t   m_rotate_reg = 0;
};


// ======================> pce_acard_duo_device

class pce_acard_pro_device : public pce_acard_duo_device

{
public:
	// construction/destruction
	pce_acard_pro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual void install_memory_handlers(address_space &space) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<pce_cdsys3_base_device> m_cdsys3;
};


// device type definition
DECLARE_DEVICE_TYPE(PCE_ROM_ACARD_DUO, pce_acard_duo_device)
DECLARE_DEVICE_TYPE(PCE_ROM_ACARD_PRO, pce_acard_pro_device)


#endif // MAME_BUS_PCE_PCE_ACARD_H
