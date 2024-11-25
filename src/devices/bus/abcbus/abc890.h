// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 890 bus expander emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_ABC890_H
#define MAME_BUS_ABCBUS_ABC890_H

#pragma once

#include "abcbus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc890_device

class abc890_device :  public device_t,
					public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc890_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	abc890_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
	virtual uint8_t abcbus_inp() override;
	virtual void abcbus_out(uint8_t data) override;
	virtual uint8_t abcbus_stat() override;
	virtual void abcbus_c1(uint8_t data) override;
	virtual void abcbus_c2(uint8_t data) override;
	virtual void abcbus_c3(uint8_t data) override;
	virtual void abcbus_c4(uint8_t data) override;
	virtual uint8_t abcbus_xmemfl(offs_t offset) override;
	virtual void abcbus_xmemw(offs_t offset, uint8_t data) override;
};


// ======================> abc_expansion_unit_device

class abc_expansion_unit_device :  public abc890_device
{
public:
	// construction/destruction
	abc_expansion_unit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> abc894_device

class abc894_device :  public abc890_device
{
public:
	// construction/destruction
	abc894_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> abc850_device

class abc850_device :  public abc890_device
{
public:
	// construction/destruction
	abc850_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> abc852_device

class abc852_device :  public abc890_device
{
public:
	// construction/destruction
	abc852_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> abc856_device

class abc856_device :  public abc890_device
{
public:
	// construction/destruction
	abc856_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC_EXPANSION_UNIT, abc_expansion_unit_device)
DECLARE_DEVICE_TYPE(ABC890,             abc890_device)
DECLARE_DEVICE_TYPE(ABC894,             abc894_device)
DECLARE_DEVICE_TYPE(ABC850,             abc850_device)
DECLARE_DEVICE_TYPE(ABC852,             abc852_device)
DECLARE_DEVICE_TYPE(ABC856,             abc856_device)



#endif // MAME_BUS_ABCBUS_ABC890_H
