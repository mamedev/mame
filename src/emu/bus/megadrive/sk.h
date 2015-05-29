// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __MD_SK_H
#define __MD_SK_H

#include "md_slot.h"


// ======================> md_rom_sk_device

class md_rom_sk_device : public device_t,
						public device_md_cart_interface
{
public:
	// construction/destruction
	md_rom_sk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	md_rom_sk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);

private:
	required_device<md_cart_slot_device> m_exp;
};


// device type definition
extern const device_type MD_ROM_SK;

#endif
