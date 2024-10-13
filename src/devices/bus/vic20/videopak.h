// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Data 20 Corporation Video Pak cartridge emulation
    aka Data 20 Display Manager aka Protecto 40/80

**********************************************************************/

#ifndef MAME_BUS_VIC20_VIDEOPAK_H
#define MAME_BUS_VIC20_VIDEOPAK_H

#pragma once


#include "exp.h"
#include "video/mc6845.h"
#include "emupal.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic20_video_pak_device

class vic20_video_pak_device : public device_t,
						  public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic20_video_pak_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_vic20_expansion_card_interface overrides
	virtual uint8_t vic20_cd_r(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;
	virtual void vic20_cd_w(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;

private:
	MC6845_UPDATE_ROW( crtc_update_row );

	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_memory_region m_char_rom;
	memory_share_creator<uint8_t> m_videoram;
	memory_share_creator<uint8_t> m_ram;

	bool m_case;
	bool m_bank_size;
	bool m_bank_lsb;
	bool m_bank_msb;
	bool m_ram_enable;
	bool m_columns;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC20_VIDEO_PAK, vic20_video_pak_device)

#endif // MAME_BUS_VIC20_VIDEOPAK_H
