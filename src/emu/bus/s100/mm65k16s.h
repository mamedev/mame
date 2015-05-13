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
	virtual const rom_entry *device_rom_region() const;
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_s100_card_interface overrides
	virtual UINT8 s100_smemr_r(address_space &space, offs_t offset);
	virtual void s100_mwrt_w(address_space &space, offs_t offset, UINT8 data);
	virtual void s100_phantom_w(int state);

private:
	optional_shared_ptr<UINT8> m_ram;
};


// device type definition
extern const device_type S100_MM65K16S;


#endif
