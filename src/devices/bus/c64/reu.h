// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1700/1750/1764 RAM Expansion Unit emulation

**********************************************************************/

#ifndef MAME_BUS_C64_REU_H
#define MAME_BUS_C64_REU_H

#pragma once


#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "exp.h"
#include "machine/mos8726.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_reu_cartridge_device

class c64_reu_cartridge_device : public device_t, public device_c64_expansion_card_interface
{
protected:
	enum
	{
		TYPE_1700,
		TYPE_1750,
		TYPE_1764
	};

	// construction/destruction
	c64_reu_cartridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant, int jp1, size_t ram_size);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_ba_w(int state) override { m_dmac->ba_w(state); }

	required_device<mos8726_device> m_dmac;
	required_device<generic_slot_device> m_eprom;
	memory_share_creator<uint8_t> m_ram;

	int m_variant;
	int m_jp1;
	size_t m_ram_size;

private:
	void dmac_irq_w(int state);
	void dmac_dma_w(int state);
	uint8_t dmac_c64_r(offs_t offset);
	void dmac_c64_w(offs_t offset, uint8_t data);
	uint8_t dmac_reu_r(offs_t offset);
	void dmac_reu_w(offs_t offset, uint8_t data);

	uint8_t m_dd;
};


// ======================> c64_reu1700_cartridge_device

class c64_reu1700_cartridge_device :  public c64_reu_cartridge_device
{
public:
	// construction/destruction
	c64_reu1700_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> c64_reu1750_cartridge_device

class c64_reu1750_cartridge_device :  public c64_reu_cartridge_device
{
public:
	// construction/destruction
	c64_reu1750_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> c64_reu1700_cartridge_device

class c64_reu1764_cartridge_device :  public c64_reu_cartridge_device
{
public:
	// construction/destruction
	c64_reu1764_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(C64_REU1700, c64_reu1700_cartridge_device)
DECLARE_DEVICE_TYPE(C64_REU1750, c64_reu1750_cartridge_device)
DECLARE_DEVICE_TYPE(C64_REU1764, c64_reu1764_cartridge_device)


#endif // MAME_BUS_C64_REU_H
