// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Poisk-1 HDC device (model B942)

**********************************************************************/

#ifndef MAME_BUS_ISA_P1_HDC_H
#define MAME_BUS_ISA_P1_HDC_H

#pragma once


#include "isa.h"
#include "imagedev/harddriv.h"
#include "machine/wd2010.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class p1_hdc_device : public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	p1_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<wd2010_device> m_hdc;

	// uint8_t m_ram[0x800];

public:
	uint8_t p1_HDC_r(offs_t offset);
	void p1_HDC_w(offs_t offset, uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(P1_HDC, p1_hdc_device)


#endif // MAME_BUS_ISA_P1_HDC_H
