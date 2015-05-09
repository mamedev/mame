// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Buddha

    Zorro-II IDE controller

***************************************************************************/

#pragma once

#ifndef __BUDDHA_H__
#define __BUDDHA_H__

#include "emu.h"
#include "zorro.h"
#include "machine/autoconfig.h"
#include "machine/ataintf.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> buddha_device

class buddha_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	// construction/destruction
	buddha_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// speed register
	DECLARE_READ16_MEMBER( speed_r );
	DECLARE_WRITE16_MEMBER( speed_w );

	// ide register
	DECLARE_READ16_MEMBER( ide_0_cs0_r );
	DECLARE_WRITE16_MEMBER( ide_0_cs0_w );
	DECLARE_READ16_MEMBER( ide_0_cs1_r );
	DECLARE_WRITE16_MEMBER( ide_0_cs1_w );
	DECLARE_READ16_MEMBER( ide_1_cs0_r );
	DECLARE_WRITE16_MEMBER( ide_1_cs0_w );
	DECLARE_READ16_MEMBER( ide_1_cs1_r );
	DECLARE_WRITE16_MEMBER( ide_1_cs1_w );

	// interrupt register
	DECLARE_READ16_MEMBER( ide_0_interrupt_r );
	DECLARE_READ16_MEMBER( ide_1_interrupt_r );
	DECLARE_WRITE16_MEMBER( ide_interrupt_enable_w );

	DECLARE_WRITE_LINE_MEMBER( ide_0_interrupt_w );
	DECLARE_WRITE_LINE_MEMBER( ide_1_interrupt_w );

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;

	virtual void device_start();
	virtual void device_reset();

	// device_zorro2_card_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER( cfgin_w );

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address);

private:
	required_device<ata_interface_device> m_ata_0;
	required_device<ata_interface_device> m_ata_1;

	bool m_ide_interrupts_enabled;
	int m_ide_0_interrupt;
	int m_ide_1_interrupt;
};

// device type definition
extern const device_type BUDDHA;

#endif
