/**********************************************************************

    COMX-35E Expansion Box emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __COMX_EB__
#define __COMX_EB__


#include "emu.h"
#include "machine/comxexp.h"
#include "machine/comx_clm.h"
#include "machine/comx_epr.h"
#include "machine/comx_fd.h"
#include "machine/comx_joy.h"
#include "machine/comx_prn.h"
#include "machine/comx_ram.h"
#include "machine/comx_thm.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define MAX_EB_SLOTS	4



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_eb_device

class comx_eb_device : public device_t,
					   public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_eb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	// not really public
	void set_int(const char *tag, int state);
	void set_ef4(const char *tag, int state);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
    virtual void device_config_complete() { m_shortname = "comx_eb"; }

	// device_comx_expansion_card_interface overrides
	virtual void comx_q_w(int state);
	virtual UINT8 comx_mrd_r(offs_t offset, int *extrom);
	virtual void comx_mwr_w(offs_t offset, UINT8 data);
	virtual UINT8 comx_io_r(offs_t offset);
	virtual void comx_io_w(offs_t offset, UINT8 data);
	virtual UINT32 comx_screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	UINT8 *m_rom;				// program ROM

	comx_expansion_slot_device	*m_expansion_slot[MAX_EB_SLOTS];
	int m_int[MAX_EB_SLOTS];
	int m_ef4[MAX_EB_SLOTS];

	UINT8 m_select;
};


// device type definition
extern const device_type COMX_EB;


#endif
