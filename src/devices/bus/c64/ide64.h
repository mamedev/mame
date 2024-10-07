// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IDE64 v4.1 cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_IDE64_H
#define MAME_BUS_C64_IDE64_H

#pragma once


#include "exp.h"
#include "bus/ata/ataintf.h"
#include "machine/ds1302.h"
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
	c64_ide64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) override;
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw) override;

private:
	required_device<atmel_29c010_device> m_flash_rom;
	required_device<ds1302_device> m_rtc;
	required_device<ata_interface_device> m_ata;
	required_ioport m_jp1;
	memory_share_creator<uint8_t> m_ram;

	uint8_t m_bank;
	uint16_t m_ata_data;
	int m_wp;
	int m_enable;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_IDE64, c64_ide64_cartridge_device)


#endif // MAME_BUS_C64_IDE64_H
