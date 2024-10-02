// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SID Soundchip Interface for SAM Coupe

***************************************************************************/

#ifndef MAME_BUS_SAMCOUPE_EXPANSION_SID_H
#define MAME_BUS_SAMCOUPE_EXPANSION_SID_H

#pragma once

#include "expansion.h"
#include "sound/mos6581.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sam_sid_device

class sam_sid_device : public device_t, public device_samcoupe_expansion_interface
{
public:
	// construction/destruction
	sam_sid_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual void iorq_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;

	required_device<mos6581_device> m_sid;
};

// ======================> sam_sid6581_device

class sam_sid6581_device : public sam_sid_device
{
public:
	// construction/destruction
	sam_sid6581_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

// ======================> sam_sid8580_device

class sam_sid8580_device : public sam_sid_device
{
public:
	// construction/destruction
	sam_sid8580_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(SAM_SID6581, sam_sid6581_device)
DECLARE_DEVICE_TYPE(SAM_SID8580, sam_sid8580_device)

#endif // MAME_BUS_SAMCOUPE_EXPANSION_SID_H
