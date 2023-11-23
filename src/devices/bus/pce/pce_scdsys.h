// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol
#ifndef MAME_BUS_PCE_PCE_SCDSYS_H
#define MAME_BUS_PCE_PCE_SCDSYS_H

#pragma once

#include "pce_slot.h"


// ======================> pce_cdsys3_base_device

class pce_cdsys3_base_device : public device_t
{
public:
	// construction/destruction
	pce_cdsys3_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_region(bool region) { m_region = region; }

	// reading and writing
	uint8_t register_r(offs_t offset);

	uint8_t *ram() { return m_ram; }

protected:
	// device-level overrides
	virtual void device_start() override { }

private:
	memory_share_creator<uint8_t> m_ram; // internal RAM
	bool m_region = false; // Cartridge region
};

// ======================> pce_cdsys3_device

class pce_cdsys3_device : public device_t,
						public device_pce_cart_interface
{
public:
	// reading and writing
	virtual void install_memory_handlers(address_space &space) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }
	virtual void device_add_mconfig(machine_config &config) override;

	// construction/destruction
	pce_cdsys3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	required_device<pce_cdsys3_base_device> m_cdsys3;
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

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

// device type definition
DECLARE_DEVICE_TYPE(PCE_ROM_CDSYS3_BASE, pce_cdsys3_base_device)
DECLARE_DEVICE_TYPE(PCE_ROM_CDSYS3J,     pce_cdsys3j_device)
DECLARE_DEVICE_TYPE(PCE_ROM_CDSYS3U,     pce_cdsys3u_device)


#endif // MAME_BUS_PCE_PCE_SCDSYS_H
