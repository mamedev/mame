/**********************************************************************

    Wang PC PM-001B Medium-Resolution Video Controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __WANGPC_TIG__
#define __WANGPC_TIG__


#include "emu.h"
#include "machine/wangpcbus.h"
#include "video/upd7220.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_tig_device

class wangpc_tig_device : public device_t,
						  public device_wangpcbus_card_interface
{
public:
	// construction/destruction
	wangpc_tig_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "wangpc_tig"; }

	// device_wangpcbus_card_interface overrides
	virtual UINT16 wangpcbus_iorc_r(address_space &space, offs_t offset, UINT16 mem_mask);
	virtual void wangpcbus_aiowc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data);
	virtual UINT8 wangpcbus_dack_r(address_space &space, int line);
	virtual void wangpcbus_dack_w(address_space &space, int line, UINT8 data);
	virtual bool wangpcbus_have_dack(int line);

private:
	// internal state
	required_device<upd7220_device> m_hgdc0;
	required_device<upd7220_device> m_hgdc1;

	UINT8 m_option;
	UINT8 m_attr[16];
	UINT8 m_underline;
};


// device type definition
extern const device_type WANGPC_TIG;


#endif
