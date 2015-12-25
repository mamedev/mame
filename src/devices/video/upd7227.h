// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    uPD7227 Intelligent Dot-Matrix LCD Controller/Driver emulation

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
	upd7227_device::static_set_offsets(*device, _sx, _sy);



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> upd7227_device

class upd7227_device :  public device_t,
						public device_memory_interface
{
public:
	// construction/destruction
	upd7227_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_offsets(device_t &device, int sx, int sy);

	DECLARE_WRITE_LINE_MEMBER( cs_w );
	DECLARE_WRITE_LINE_MEMBER( cd_w );
	DECLARE_WRITE_LINE_MEMBER( sck_w );
	DECLARE_WRITE_LINE_MEMBER( si_w );
	DECLARE_READ_LINE_MEMBER( so_r );

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	address_space_config        m_space_config;

private:
	enum
	{
		CMD_SMM         = 0x18,
		CMD_SFF         = 0x10,
		CMD_LDPI        = 0x80,
		CMD_SWM         = 0x64,
		CMD_SRM         = 0x60,
		CMD_SANDM       = 0x6c,
		CMD_SORM        = 0x68,
		CMD_SCM         = 0x72,
		CMD_BSET        = 0x40,
		CMD_BRESET      = 0x20,
		CMD_DISP_ON     = 0x09,
		CMD_DISP_OFF    = 0x08
	};

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
