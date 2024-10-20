// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Morrow Designs MM65K16S memory board emulation

**********************************************************************/

#ifndef MAME_BUS_S100_MM65K16S_H
#define MAME_BUS_S100_MM65K16S_H

#pragma once

#include "s100.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s100_mm65k16s_device

class s100_mm65k16s_device : public device_t,
								public device_s100_card_interface
{
public:
	// construction/destruction
	s100_mm65k16s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_s100_card_interface overrides
	virtual uint8_t s100_smemr_r(offs_t offset) override;
	virtual void s100_mwrt_w(offs_t offset, uint8_t data) override;
	virtual void s100_phantom_w(int state) override;

private:
	memory_share_creator<uint8_t> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(S100_MM65K16S, s100_mm65k16s_device)

#endif // MAME_BUS_S100_MM65K16S_H
