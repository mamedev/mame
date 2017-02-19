// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Morrow Designs MM65K16S memory board emulation

**********************************************************************/

#pragma once

#ifndef __S100_MM65K16S__
#define __S100_MM65K16S__

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
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_s100_card_interface overrides
	virtual uint8_t s100_smemr_r(address_space &space, offs_t offset) override;
	virtual void s100_mwrt_w(address_space &space, offs_t offset, uint8_t data) override;
	virtual void s100_phantom_w(int state) override;

private:
	optional_shared_ptr<uint8_t> m_ram;
};


// device type definition
extern const device_type S100_MM65K16S;


#endif
