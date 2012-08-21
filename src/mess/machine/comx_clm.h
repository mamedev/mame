/**********************************************************************

    COMX-35 80-Column Card emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __COMX_CLM__
#define __COMX_CLM__


#include "emu.h"
#include "machine/comxexp.h"
#include "video/mc6845.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_clm_device

class comx_clm_device : public device_t,
					    public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_clm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	// not really public
	void crtc_update_row(mc6845_device *device, bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16 ma, UINT8 ra, UINT16 y, UINT8 x_count, INT8 cursor_x, void *param);
	DECLARE_WRITE_LINE_MEMBER( hsync_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
    virtual void device_config_complete() { m_shortname = "comx_clm"; }

	// device_comx_expansion_card_interface overrides
	virtual void comx_ds_w(int state);
	virtual UINT8 comx_mrd_r(offs_t offset, int *extrom);
	virtual void comx_mwr_w(offs_t offset, UINT8 data);

private:
	required_device<mc6845_device> m_crtc;

	int m_ds;				// device select
	UINT8 *m_rom;			// program ROM
	UINT8 *m_char_rom;		// character ROM
	UINT8 *m_video_ram;		// video RAM
};


// device type definition
extern const device_type COMX_CLM;


#endif
