// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Autocue RAM Disc

**********************************************************************/


#ifndef MAME_BUS_BBC_EXP_AUTOCUE_H
#define MAME_BUS_BBC_EXP_AUTOCUE_H

#include "exp.h"
#include "machine/nvram.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_autocue_device :
	public device_t,
	public device_bbc_exp_interface
{
public:
	// construction/destruction
	bbc_autocue_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	required_device<nvram_device> m_nvram;
	std::unique_ptr<uint8_t[]> m_ram;

	uint16_t m_ram_page;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_AUTOCUE, bbc_autocue_device);


#endif /* MAME_BUS_BBC_EXP_AUTOCUE_H */
