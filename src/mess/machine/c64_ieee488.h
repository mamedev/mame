/**********************************************************************

    Commodore IEEE-488 cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __C64_IEEE488__
#define __C64_IEEE488__


#include "emu.h"
#include "machine/6525tpi.h"
#include "machine/c64exp.h"
#include "machine/cbmipt.h"
#include "machine/ieee488.h"
#include "video/mc6845.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_ieee488_device

class c64_ieee488_device : public device_t,
					       public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// not really public
	DECLARE_READ8_MEMBER( tpi_pa_r );
	DECLARE_WRITE8_MEMBER( tpi_pa_w );
	DECLARE_READ8_MEMBER( tpi_pc_r );
	DECLARE_WRITE8_MEMBER( tpi_pc_w );
	DECLARE_READ8_MEMBER( dma_cd_r );
	DECLARE_WRITE8_MEMBER( dma_cd_w );
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );
	DECLARE_WRITE_LINE_MEMBER( dma_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "c64_ieee488"; }
	virtual void device_start();
	virtual void device_reset();

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw, int hiram);

private:
	required_device<device_t> m_tpi;
	required_device<ieee488_device> m_bus;
	required_device<c64_expansion_slot_device> m_exp;

	int m_roml_sel;
};



// device type definition
extern const device_type C64_IEEE488;


#endif
