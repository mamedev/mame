// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot RAM Expansions

***************************************************************************/

#ifndef MAME_BUS_APRICOT_RAM_H
#define MAME_BUS_APRICOT_RAM_H

#pragma once

#include "expansion.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> apricot_256k_ram_device

class apricot_256k_ram_device : public device_t, public device_apricot_expansion_card_interface
{
public:
	// construction/destruction
	apricot_256k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_ioport m_sw;

	std::vector<uint16_t> m_ram;
};


// ======================> apricot_128k_ram_device

class apricot_128k_ram_device : public device_t, public device_apricot_expansion_card_interface
{
public:
	// construction/destruction
	apricot_128k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_ioport m_strap;

	std::vector<uint16_t> m_ram;
};


// ======================> apricot_512k_ram_device

class apricot_512k_ram_device : public device_t, public device_apricot_expansion_card_interface
{
public:
	// construction/destruction
	apricot_512k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_ioport m_strap;

	std::vector<uint16_t> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(APRICOT_256K_RAM, apricot_256k_ram_device)
DECLARE_DEVICE_TYPE(APRICOT_128K_RAM, apricot_128k_ram_device)
DECLARE_DEVICE_TYPE(APRICOT_512K_RAM, apricot_512k_ram_device)


#endif // MAME_BUS_APRICOT_RAM_H
