// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __KC_ROM_H__
#define __KC_ROM_H__

#include "emu.h"
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
	kc_8k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	kc_8k_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// optional information overrides

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual const rom_entry *device_rom_region() const;

	// kcexp_interface overrides
	virtual UINT8 module_id_r() { return 0xfb; }
	virtual void control_w(UINT8 data);
	virtual void read(offs_t offset, UINT8 &data);
	virtual UINT8* get_cart_base();
	virtual DECLARE_WRITE_LINE_MEMBER( mei_w );

protected:
	kcexp_slot_device *m_slot;

	// internal state
	int     m_mei;          // module enable line
	UINT8 * m_rom;
	UINT8   m_enabled;
	UINT16  m_base;
};


// ======================> kc_m006_device

class kc_m006_device :
		public kc_8k_device
{
public:
	// construction/destruction
	kc_m006_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// kcexp_interface overrides
	virtual UINT8 module_id_r() { return 0xfc; }
	virtual void control_w(UINT8 data);
	virtual void read(offs_t offset, UINT8 &data);
};


// ======================> kc_m033_device

class kc_m033_device :
		public kc_8k_device
{
public:
	// construction/destruction
	kc_m033_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_reset();

	// kcexp_interface overrides
	virtual UINT8 module_id_r() { return 0x01; }
	virtual void control_w(UINT8 data);
	virtual void read(offs_t offset, UINT8 &data);

private:
	// internal state
	UINT16  m_bank;
};


// device type definition
extern const device_type KC_STANDARD;
extern const device_type KC_M006;
extern const device_type KC_M033;

#endif  /* __KC_ROM_H__ */
