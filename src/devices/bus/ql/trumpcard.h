// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    Miracle Systems QL Trump Card emulation

**********************************************************************/

#ifndef MAME_BUS_QL_TRUMPCARD_H
#define MAME_BUS_QL_TRUMPCARD_H

#pragma once

#include "exp.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ql_trump_card_device

class ql_trump_card_device : public device_t, public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	ql_trump_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	ql_trump_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int ram_size);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_ql_expansion_card_interface overrides
	virtual uint8_t read(offs_t offset, uint8_t data) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:

	static void floppy_formats(format_registration &fr);

	required_device<wd1772_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_memory_region m_rom;
	memory_share_creator<uint8_t> m_ram;

	int m_ram_size;
	bool m_rom_en;
};


// ======================> ql_trump_card_256k_device

class ql_trump_card_256k_device :  public ql_trump_card_device
{
public:
	// construction/destruction
	ql_trump_card_256k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> ql_trump_card_512k_device

class ql_trump_card_512k_device :  public ql_trump_card_device
{
public:
	// construction/destruction
	ql_trump_card_512k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> ql_trump_card_768k_device

class ql_trump_card_768k_device :  public ql_trump_card_device
{
public:
	// construction/destruction
	ql_trump_card_768k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(QL_TRUMP_CARD,       ql_trump_card_device)
DECLARE_DEVICE_TYPE(QL_TRUMP_CARD_256K,  ql_trump_card_256k_device)
DECLARE_DEVICE_TYPE(QL_TRUMP_CARD_512K,  ql_trump_card_512k_device)
DECLARE_DEVICE_TYPE(QL_TRUMP_CARD_768K,  ql_trump_card_768k_device)

#endif // MAME_BUS_QL_TRUMPCARD_H
