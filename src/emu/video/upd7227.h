/**********************************************************************

    uPD7227 Liquid Crystal Display Controller/Driver emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __UPD7227__
#define __UPD7227__

#include "emu.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_UPD7227_ADD(_tag, _sx, _sy) \
	MCFG_DEVICE_ADD(_tag, UPD7227, 0) \
	upd7227_device::static_set_config(*device, _sx, _sy);



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> upd7227_device

class upd7227_device :  public device_t
{
public:
	// construction/destruction
	upd7227_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_config(device_t &device, int sx, int sy);

	DECLARE_WRITE_LINE_MEMBER( cs_w );
	DECLARE_WRITE_LINE_MEMBER( cd_w );
	DECLARE_WRITE_LINE_MEMBER( sck_w );
	DECLARE_WRITE_LINE_MEMBER( si_w );
	DECLARE_READ_LINE_MEMBER( so_r );

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	int m_sx;
	int m_sy;

	int m_cs;
	int m_cd;
	int m_sck;
	int m_si;
	int m_so;
};


// device type definition
extern const device_type UPD7227;



#endif
