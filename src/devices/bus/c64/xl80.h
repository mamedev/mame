// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    XL 80 cartridge emulation

**********************************************************************/

#pragma once

#ifndef __XL80__
#define __XL80__


#include "emu.h"
#include "exp.h"
#include "video/mc6845.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_xl80_device

class c64_xl80_device : public device_t,
						public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_xl80_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// not really public
	MC6845_UPDATE_ROW( crtc_update_row );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) override { return 1; }
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw) override { return 0; }

private:
	required_device<h46505_device> m_crtc;
	required_device<palette_device> m_palette;
	required_memory_region m_char_rom;
	optional_shared_ptr<UINT8> m_ram;
};


// device type definition
extern const device_type C64_XL80;


#endif
