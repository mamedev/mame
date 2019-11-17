// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Buddha

    Zorro-II IDE controller

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_BUDDHA_H
#define MAME_BUS_AMIGA_ZORRO_BUDDHA_H

#pragma once

#include "zorro.h"
#include "machine/autoconfig.h"
#include "bus/ata/ataintf.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> buddha_device

class buddha_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	// construction/destruction
	buddha_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void device_start() override;
	virtual void device_reset() override;

	// device_zorro2_card_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER( cfgin_w ) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
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

	void mmio_map(address_map &map);

	required_device<ata_interface_device> m_ata_0;
	required_device<ata_interface_device> m_ata_1;

	bool m_ide_interrupts_enabled;
	int m_ide_0_interrupt;
	int m_ide_1_interrupt;
};

// device type definition
DECLARE_DEVICE_TYPE(BUDDHA, buddha_device)

#endif // MAME_BUS_AMIGA_ZORRO_BUDDHA_H
