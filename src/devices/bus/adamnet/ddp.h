// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Digital Data Pack emulation

**********************************************************************/

#pragma once

#ifndef __ADAM_DDP__
#define __ADAM_DDP__

#include "emu.h"
#include "adamnet.h"
#include "cpu/m6800/m6800.h"
#include "formats/adam_cas.h"
#include "imagedev/cassette.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adam_digital_data_pack_device

class adam_digital_data_pack_device :  public device_t,
										public device_adamnet_card_interface
{
public:
	// construction/destruction
	adam_digital_data_pack_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	// not really public
	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_READ8_MEMBER( p2_r );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ8_MEMBER( p4_r );

protected:
	// device-level overrides
	virtual void device_start();

	// device_adamnet_card_interface overrides
	virtual void adamnet_reset_w(int state);

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_ddp0;
	required_device<cassette_image_device> m_ddp1;

	int m_wr0;
	int m_wr1;
	int m_track;
};


// device type definition
extern const device_type ADAM_DDP;



#endif
