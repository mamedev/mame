// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Colecovision Super Game Module

***************************************************************************/

#ifndef MAME_BUS_COLECO_EXPANSION_SGM_H
#define MAME_BUS_COLECO_EXPANSION_SGM_H

#pragma once

#include "expansion.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class coleco_sgm_device : public device_t, public device_coleco_expansion_interface
{
public:
	// construction/destruction
	coleco_sgm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	memory_view m_view_lower;
	memory_view m_view_upper;

	std::unique_ptr<uint8_t []> m_ram_lower;
	std::unique_ptr<uint8_t []> m_ram_upper;

	void io_map(address_map &map) ATTR_COLD;

	void upper_enable_w(uint8_t data);
	void lower_enable_w(uint8_t data);
};

// device type declaration
DECLARE_DEVICE_TYPE(COLECO_SGM, coleco_sgm_device)

#endif // MAME_BUS_COLECO_EXPANSION_SGM_H
