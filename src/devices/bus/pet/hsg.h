// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CBM 8000 High Speed Graphics (324402-01) card emulation

**********************************************************************/

#ifndef MAME_BUS_PET_HSG_H
#define MAME_BUS_PET_HSG_H

#pragma once

#include "exp.h"
#include "video/ef9365.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm8000_hsg_device

class cbm8000_hsg_device : public device_t, public device_pet_expansion_card_interface
{
protected:
	// construction/destruction
	cbm8000_hsg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_pet_expansion_card_interface overrides
	virtual int pet_norom_r(offs_t offset, int sel) override;
	virtual uint8_t pet_bd_r(offs_t offset, uint8_t data, int &sel) override;
	virtual void pet_bd_w(offs_t offset, uint8_t data, int &sel) override;

	required_device<ef9365_device> m_gdc;

private:
	required_memory_region m_9000;
	required_memory_region m_a000;
};


// ======================> cbm8000_hsg_a_device

class cbm8000_hsg_a_device :  public cbm8000_hsg_device
{
public:
	// construction/destruction
	cbm8000_hsg_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void hsg_a_map(address_map &map) ATTR_COLD;
};


// ======================> cbm8000_hsg_b_device

class cbm8000_hsg_b_device :  public cbm8000_hsg_device
{
public:
	// construction/destruction
	cbm8000_hsg_b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void hsg_b_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(CBM8000_HSG_A, cbm8000_hsg_a_device)
DECLARE_DEVICE_TYPE(CBM8000_HSG_B, cbm8000_hsg_b_device)

#endif // MAME_BUS_PET_HSG_H
