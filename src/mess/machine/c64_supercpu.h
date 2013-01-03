/**********************************************************************

    CMD SuperCPU v2 + SuperRAM emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __SUPERCPU__
#define __SUPERCPU__

#include "emu.h"
#include "machine/c64exp.h"
#include "machine/cbmipt.h"
#include "cpu/g65816/g65816.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_supercpu_device

class c64_supercpu_device : public device_t,
					    	public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_supercpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_INPUT_CHANGED_MEMBER( reset );

	DECLARE_READ8_MEMBER( dma_cd_r );
	DECLARE_WRITE8_MEMBER( dma_cd_w );
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );
	DECLARE_WRITE_LINE_MEMBER( dma_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );

protected:
	// device-level overrides
    virtual void device_config_complete() { m_shortname = "c64_supercpu"; }
	virtual void device_start();
	virtual void device_reset();

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw, int hiram);
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw, int hiram);

private:
	required_device<legacy_cpu_device> m_maincpu;
	required_device<c64_expansion_slot_device> m_exp;

	required_shared_ptr<UINT8> m_sram;
	required_shared_ptr<UINT8> m_dimm;
};


// device type definition
extern const device_type C64_SUPERCPU;



#endif
