/**********************************************************************

    IDE64 v4.1 cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __IDE64__
#define __IDE64__


#include "emu.h"
#include "machine/c64exp.h"
#include "machine/ds1302.h"
#include "machine/idectrl.h"
#include "machine/intelfsh.h"
#include "imagedev/harddriv.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_ide64_cartridge_device

class c64_ide64_cartridge_device : public device_t,
								   public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_ide64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "c64_ide64"; }
	virtual void device_start();
	virtual void device_reset();

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw, int hiram);
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw, int hiram);

private:
	required_device<atmel_29c010_device> m_flash_rom;
	required_device<ds1302_device> m_rtc;
	required_device<ide_controller_device> m_ide;

	UINT8 m_bank;
	UINT16 m_ide_data;
	int m_wp;
	int m_enable;
};


// device type definition
extern const device_type C64_IDE64;


#endif
