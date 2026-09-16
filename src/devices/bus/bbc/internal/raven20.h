// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Raven-20 RAM expansion board

**********************************************************************/


#ifndef MAME_BUS_BBC_INTERNAL_RAVEN20_H
#define MAME_BUS_BBC_INTERNAL_RAVEN20_H

#include "internal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_raven20_device :
	public device_t,
	public device_bbc_internal_interface
{
public:
	// construction/destruction
	bbc_raven20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual bool overrides_ram() override { return true; }
	virtual uint8_t ram_r(offs_t offset) override;
	virtual void ram_w(offs_t offset, uint8_t data) override;

private:
	void enable_w(offs_t offset, uint8_t data);
	void disable_w(offs_t offset, uint8_t data);

	bool m_shadow;
	std::unique_ptr<uint8_t[]> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_RAVEN20, bbc_raven20_device)


#endif // MAME_BUS_BBC_INTERNAL_RAVEN20_H
