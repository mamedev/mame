// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_KC_ROM_H
#define MAME_BUS_KC_ROM_H

#pragma once

#include "kc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> kc_8k_device

class kc_8k_device :
		public device_t,
		public device_kcexp_interface
{
public:
	// construction/destruction
	kc_8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	kc_8k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// kcexp_interface overrides
	virtual uint8_t module_id_r() override { return 0xfb; }
	virtual void control_w(uint8_t data) override;
	virtual void read(offs_t offset, uint8_t &data) override;
	virtual uint8_t* get_cart_base() override;
	virtual void mei_w(int state) override;

protected:
	kcexp_slot_device *m_slot;

	// internal state
	int     m_mei;          // module enable line
	uint8_t * m_rom;
	uint8_t   m_enabled;
	uint16_t  m_base;
};


// ======================> kc_m006_device

class kc_m006_device :
		public kc_8k_device
{
public:
	// construction/destruction
	kc_m006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// kcexp_interface overrides
	virtual uint8_t module_id_r() override { return 0xfc; }
	virtual void control_w(uint8_t data) override;
	virtual void read(offs_t offset, uint8_t &data) override;
};


// ======================> kc_m033_device

class kc_m033_device :
		public kc_8k_device
{
public:
	// construction/destruction
	kc_m033_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;

	// kcexp_interface overrides
	virtual uint8_t module_id_r() override { return 0x01; }
	virtual void control_w(uint8_t data) override;
	virtual void read(offs_t offset, uint8_t &data) override;

private:
	// internal state
	uint16_t  m_bank;
};


// device type definition
DECLARE_DEVICE_TYPE(KC_STANDARD, kc_8k_device)
DECLARE_DEVICE_TYPE(KC_M006,     kc_m006_device)
DECLARE_DEVICE_TYPE(KC_M033,     kc_m033_device)

#endif // MAME_BUS_KC_ROM_H
