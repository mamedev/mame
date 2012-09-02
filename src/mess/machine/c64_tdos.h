/**********************************************************************

    Triton QD TDOS cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __TDOS__
#define __TDOS__


#include "emu.h"
#include "imagedev/flopdrv.h"
#include "machine/c64exp.h"
#include "machine/cbmipt.h"
#include "machine/mc6852.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_tdos_cartridge_device

class c64_tdos_cartridge_device : public device_t,
								  public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_tdos_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// not really public
	DECLARE_READ8_MEMBER( dma_cd_r );
	DECLARE_WRITE8_MEMBER( dma_cd_w );
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );
	DECLARE_WRITE_LINE_MEMBER( dma_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "c64_tdos"; }
	virtual void device_start();
	virtual void device_reset();

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2);
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2);
	virtual int c64_game_r(offs_t offset, int ba, int rw, int hiram);
	virtual int c64_exrom_r(offs_t offset, int ba, int rw, int hiram);

private:
	required_device<mc6852_device> m_ssda;
	required_device<c64_expansion_slot_device> m_exp;
};


// device type definition
extern const device_type C64_TDOS;


#endif
