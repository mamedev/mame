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
	md_rom_sk_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	md_rom_sk_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;

private:
	required_device<md_cart_slot_device> m_exp;
};


// device type definition
extern const device_type MD_ROM_SK;

#endif
