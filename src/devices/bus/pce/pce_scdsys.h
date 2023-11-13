// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol, Angelo Salese
#ifndef MAME_BUS_PCE_PCE_SCDSYS_H
#define MAME_BUS_PCE_PCE_SCDSYS_H

#pragma once

#include "pce_slot.h"


// ======================> pce_cdsys3_device

class pce_cdsys3_device : public device_t,
						public device_pce_cart_interface
{
public:
	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ex(offs_t offset) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }

	// construction/destruction
	pce_cdsys3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool region);

	bool m_region; // Cartridge region
};

// ======================> pce_cdsys3j_device

class pce_cdsys3j_device : public pce_cdsys3_device
{
public:
	// construction/destruction
	pce_cdsys3j_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> pce_cdsys3u_device

class pce_cdsys3u_device : public pce_cdsys3_device
{
public:
	// construction/destruction
	pce_cdsys3u_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(PCE_ROM_CDSYS3J,   pce_cdsys3j_device)
DECLARE_DEVICE_TYPE(PCE_ROM_CDSYS3U,   pce_cdsys3u_device)


#endif // MAME_BUS_PCE_PCE_SCDSYS_H
