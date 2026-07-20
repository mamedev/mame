// license:BSD-3-Clause
// copyright-holders:A. Lenard
/**********************************************************************

    S8000 ECC Controller
    S8000 Parity Memory

**********************************************************************/

#ifndef MAME_BUS_ZBI_S8K_RAM_H
#define MAME_BUS_ZBI_S8K_RAM_H

#pragma once

#include "zbi.h"
#include "machine/ram.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> zbi_s8k_ecc_ram_card_device

//TODO
class zbi_s8k_ecc_ram_card_device : public device_t, public device_zbi_card_interface
{
public:
	// construction/destruction
	zbi_s8k_ecc_ram_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t card_ram8_r(offs_t offset) override;
	virtual void card_ram8_w(offs_t offset, uint8_t data) override;
	virtual uint16_t card_ram16_r(offs_t offset, uint16_t mask) override;
	virtual void card_ram16_w(offs_t offset, uint16_t data, uint16_t mask) override;
	virtual uint32_t card_ram32_r(offs_t offset, uint32_t mask) override;
	virtual void card_ram32_w(offs_t offset, uint32_t data, uint32_t mask) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<ram_device> m_ram;

private:
	//TODO: What are these??? Check bits for the ECC logic?
	uint16_t regs_r(offs_t offset);
	void regs_w(offs_t offset, uint16_t data);

	uint8_t m_reg1 = 0;
	uint8_t m_reg2 = 0;
};

// ======================> zbi_s8k_parity_ram_card_device

class zbi_s8k_parity_ram_card_device : public device_t, public device_zbi_card_interface
{
public:
	// construction/destruction
	zbi_s8k_parity_ram_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t card_ram8_r(offs_t offset) override;
	virtual void card_ram8_w(offs_t offset, uint8_t data) override;
	virtual uint16_t card_ram16_r(offs_t offset, uint16_t mask) override;
	virtual void card_ram16_w(offs_t offset, uint16_t data, uint16_t mask) override;
	virtual uint32_t card_ram32_r(offs_t offset, uint32_t mask) override;
	virtual void card_ram32_w(offs_t offset, uint32_t data, uint32_t mask) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<ram_device> m_ram;

	uint32_t m_checksize;
	std::unique_ptr<uint8_t[]> m_checkbits;	// 1 check bit per 8 data bits

private:
	bool check_parity(offs_t offset, uint8_t data, bool write);
};

// device type definition
DECLARE_DEVICE_TYPE(ZBI_S8K_ECC_RAM, zbi_s8k_ecc_ram_card_device)
DECLARE_DEVICE_TYPE(ZBI_S8K_PARITY_RAM, zbi_s8k_parity_ram_card_device)

#endif // MAME_BUS_ZBI_S8K_RAM_H
