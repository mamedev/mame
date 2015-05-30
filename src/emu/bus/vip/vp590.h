// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Color Board VP590 emulation

**********************************************************************/

#pragma once

#ifndef __VP590__
#define __VP590__

#include "emu.h"
#include "machine/rescap.h"
#include "exp.h"
#include "video/cdp1862.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vp590_device

class vp590_device : public device_t,
						public device_vip_expansion_card_interface
{
public:
	// construction/destruction
	vp590_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	// not really public
	DECLARE_READ_LINE_MEMBER( rd_r );
	DECLARE_READ_LINE_MEMBER( bd_r );
	DECLARE_READ_LINE_MEMBER( gd_r );

protected:
	// device-level overrides
	virtual void device_start();

	// device_vip_expansion_card_interface overrides
	virtual void vip_program_w(address_space &space, offs_t offset, UINT8 data, int cdef, int *minh);
	virtual void vip_io_w(address_space &space, offs_t offset, UINT8 data);
	virtual void vip_dma_w(address_space &space, offs_t offset, UINT8 data);
	virtual UINT32 vip_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual int vip_ef3_r();
	virtual int vip_ef4_r();

private:
	required_device<cdp1862_device> m_cgc;
	optional_shared_ptr<UINT8> m_color_ram;
	required_ioport m_j1;
	required_ioport m_j2;

	int m_a12;
	UINT8 m_color;
	UINT8 m_keylatch;
};


// device type definition
extern const device_type VP590;


#endif
