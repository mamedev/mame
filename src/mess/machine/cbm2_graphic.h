/**********************************************************************

    CBM 500/600/700 High Resolution Graphics cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __CBM2_GRAPHIC__
#define __CBM2_GRAPHIC__


#include "emu.h"
#include "machine/cbm2exp.h"
#include "video/ef9345.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm2_graphic_cartridge_device

class cbm2_graphic_cartridge_device : public device_t,
										public device_cbm2_expansion_card_interface
{
public:
	// construction/destruction
	cbm2_graphic_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	enum
	{
		TYPE_A,
		TYPE_B
	};

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "cbm2_graphic"; }
	virtual void device_start();
	virtual void device_reset();

	// device_cbm2_expansion_card_interface overrides
	virtual UINT8 cbm2_bd_r(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3);
	virtual void cbm2_bd_w(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3);

private:
	//required_device<ef9345_device> m_gdc;

	int m_variant;
};


// device type definition
extern const device_type CBM2_GRAPHIC;


#endif
