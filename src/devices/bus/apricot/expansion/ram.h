// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot RAM Expansions

***************************************************************************/

#pragma once

#ifndef __APRICOT_RAM__
#define __APRICOT_RAM__

#include "emu.h"
#include "expansion.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> apricot_256k_ram_device

class apricot_256k_ram_device : public device_t, public device_apricot_expansion_card_interface
{
public:
	// construction/destruction
	apricot_256k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_ioport m_sw;

	std::vector<UINT16> m_ram;
};


// ======================> apricot_128k_ram_device

class apricot_128k_ram_device : public device_t, public device_apricot_expansion_card_interface
{
public:
	// construction/destruction
	apricot_128k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_ioport m_strap;

	std::vector<UINT16> m_ram;
};


// ======================> apricot_512k_ram_device

class apricot_512k_ram_device : public device_t, public device_apricot_expansion_card_interface
{
public:
	// construction/destruction
	apricot_512k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_ioport m_strap;

	std::vector<UINT16> m_ram;
};


// device type definition
extern const device_type APRICOT_256K_RAM;
extern const device_type APRICOT_128K_RAM;
extern const device_type APRICOT_512K_RAM;


#endif // __APRICOT_RAM__
