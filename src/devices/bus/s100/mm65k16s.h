// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Morrow Designs MM65K16S memory board emulation

**********************************************************************/

#pragma once

#ifndef __S100_MM65K16S__
#define __S100_MM65K16S__

#include "emu.h"
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
	s100_mm65k16s_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_s100_card_interface overrides
	virtual UINT8 s100_smemr_r(address_space &space, offs_t offset) override;
	virtual void s100_mwrt_w(address_space &space, offs_t offset, UINT8 data) override;
	virtual void s100_phantom_w(int state) override;

private:
	optional_shared_ptr<UINT8> m_ram;
};


// device type definition
extern const device_type S100_MM65K16S;


#endif
