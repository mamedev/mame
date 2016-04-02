// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 80-Column Card emulation

**********************************************************************/

#pragma once

#ifndef __COMX_CLM__
#define __COMX_CLM__

#include "emu.h"
#include "exp.h"
#include "video/mc6845.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_clm_device

class comx_clm_device : public device_t,
						public device_comx_expansion_card_interface,
						public device_gfx_interface
{
public:
	// construction/destruction
	comx_clm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// not really public
	MC6845_UPDATE_ROW( crtc_update_row );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_comx_expansion_card_interface overrides
	virtual int comx_ef4_r() override;
	virtual UINT8 comx_mrd_r(address_space &space, offs_t offset, int *extrom) override;
	virtual void comx_mwr_w(address_space &space, offs_t offset, UINT8 data) override;

private:
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_memory_region m_rom;
	required_memory_region m_char_rom;
	optional_shared_ptr<UINT8> m_video_ram;
};


// device type definition
extern const device_type COMX_CLM;


#endif
