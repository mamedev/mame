// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MultiMAX 1MB ROM / 2KB RAM cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_VIC10_MULTIMAX_H
#define MAME_BUS_VIC10_MULTIMAX_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic10_multimax_device

class vic10_multimax_device : public device_t, public device_vic10_expansion_card_interface
{
public:
	// construction/destruction
	vic10_multimax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	void latch_w(uint8_t data);
	void update_banks();

	memory_share_creator<uint8_t> m_exram;

	uint8_t m_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC10_MULTIMAX, vic10_multimax_device)

#endif // MAME_BUS_VIC10_MULTIMAX_H
