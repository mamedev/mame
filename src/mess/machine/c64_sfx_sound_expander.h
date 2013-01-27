/**********************************************************************

    Commodore SFX Sound Expander cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __SFX_SOUND_EXPANDER__
#define __SFX_SOUND_EXPANDER__

#include "emu.h"
#include "machine/c64exp.h"
#include "machine/cbmipt.h"
#include "sound/3526intf.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_sfx_sound_expander_cartridge_device

class c64_sfx_sound_expander_cartridge_device : public device_t,
												public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_sfx_sound_expander_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	// not really public
	DECLARE_READ8_MEMBER( dma_cd_r );
	DECLARE_WRITE8_MEMBER( dma_cd_w );
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );
	DECLARE_WRITE_LINE_MEMBER( dma_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "c64_sfx_sound_expander"; }
	virtual void device_start();
	virtual void device_reset();

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw, int hiram);
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw, int hiram);

private:
	required_device<ym3526_device> m_opl;
	required_device<c64_expansion_slot_device> m_exp;
	required_ioport m_kb0;
	required_ioport m_kb1;
	required_ioport m_kb2;
	required_ioport m_kb3;
	required_ioport m_kb4;
	required_ioport m_kb5;
	required_ioport m_kb6;
	required_ioport m_kb7;
};


// device type definition
extern const device_type C64_SFX_SOUND_EXPANDER;



#endif
