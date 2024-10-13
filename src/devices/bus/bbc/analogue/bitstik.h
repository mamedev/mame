// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANF04 Bitstik

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_BitStik.html

    Robocom Bitstik 2

**********************************************************************/


#ifndef MAME_BUS_BBC_ANALOGUE_BITSTIK_H
#define MAME_BUS_BBC_ANALOGUE_BITSTIK_H

#pragma once


#include "analogue.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_bitstik_device

class bbc_bitstik_device :
	public device_t,
	public device_bbc_analogue_interface
{
protected:
	// construction/destruction
	bbc_bitstik_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t ch_r(int channel) override;
	virtual uint8_t pb_r() override;

private:
	required_ioport_array<4> m_channel;
	required_ioport m_buttons;
};

class bbc_bitstik1_device : public bbc_bitstik_device
{
public:
	bbc_bitstik1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class bbc_bitstik2_device : public bbc_bitstik_device
{
public:
	bbc_bitstik2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(BBC_BITSTIK1, bbc_bitstik1_device)
DECLARE_DEVICE_TYPE(BBC_BITSTIK2, bbc_bitstik2_device)


#endif // MAME_BUS_BBC_ANALOGUE_BITSTIK_H
