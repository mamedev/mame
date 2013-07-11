/**********************************************************************

    Amstrad PC1640 Integrated Graphics Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __ISA8_PC1640_IGA__
#define __ISA8_PC1640_IGA__

#include "emu.h"
#include "machine/isa.h"
#include "video/mc6845.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_pc1640_iga_device

class isa8_pc1640_iga_device : public device_t,
							   public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_pc1640_iga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;
	
	DECLARE_READ8_MEMBER( video_ram_r );
	DECLARE_WRITE8_MEMBER( video_ram_w );
	DECLARE_READ8_MEMBER( iga_r );
	DECLARE_WRITE8_MEMBER( iga_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	required_device<ams40041_device> m_vdu;
	optional_shared_ptr<UINT8> m_video_ram;

	int m_lpen;
	UINT8 m_egc_ctrl;
	UINT8 m_emcr;           // extended mode control register
	UINT8 m_emcrp;          // extended mode control register protection read counter
	UINT8 m_sar;            // sequencer address register
	UINT8 m_sdr[8];         // sequencer data registers
	UINT8 m_gcar;           // graphics controller address register
	UINT8 m_gcdr[16];       // graphics controller data registers
	UINT8 m_crtcar;         // CRT controller address register
	UINT8 m_crtcdr[32];     // CRT controller data registers
	UINT8 m_plr;            // Plantronics mode register
};


// device type definition
extern const device_type ISA8_PC1640_IGA;



#endif
