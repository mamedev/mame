// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Digital Data Pack emulation

**********************************************************************/

#ifndef MAME_BUS_ADAMNET_DDP_H
#define MAME_BUS_ADAMNET_DDP_H

#pragma once

#include "adamnet.h"
#include "cpu/m6800/m6801.h"
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
	adam_digital_data_pack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_adamnet_card_interface overrides
	virtual void adamnet_reset_w(int state) override;

private:
	required_device<m6801_cpu_device> m_maincpu;
	required_device<cassette_image_device> m_ddp0;
	required_device<cassette_image_device> m_ddp1;

	int m_wr0;
	int m_wr1;
	int m_track;

	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_READ8_MEMBER( p2_r );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ8_MEMBER( p4_r );

	void adam_ddp_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(ADAM_DDP, adam_digital_data_pack_device)

#endif // MAME_BUS_ADAMNET_DDP_H
