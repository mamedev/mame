// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __MD_GGENIE_H
#define __MD_GGENIE_H

#include "md_slot.h"


// ======================> md_rom_ggenie_device

class md_rom_ggenie_device : public device_t,
						public device_md_cart_interface
{
public:
	// construction/destruction
	md_rom_ggenie_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;

private:
	required_device<md_cart_slot_device> m_exp;
	UINT16 m_gg_regs[0x20];
	int m_gg_bypass;
	int m_reg_enable;
	UINT16 m_gg_addr[6];
	UINT16 m_gg_data[6];
};


// device type definition
extern const device_type MD_ROM_GAMEGENIE;

#endif
