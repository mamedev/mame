// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    Miracle Systems QL Trump Card emulation

**********************************************************************/

#pragma once

#ifndef __QL_TRUMP_CARD__
#define __QL_TRUMP_CARD__

#include "exp.h"
#include "machine/wd_fdc.h"
#include "formats/ql_dsk.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ql_trump_card_t

class ql_trump_card_t : public device_t,
						public device_ql_expansion_card_interface
{
public:
	// construction/destruction
	ql_trump_card_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	ql_trump_card_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int ram_size);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_ql_expansion_card_interface overrides
	virtual UINT8 read(address_space &space, offs_t offset, UINT8 data) override;
	virtual void write(address_space &space, offs_t offset, UINT8 data) override;

private:
	required_device<wd1772_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_memory_region m_rom;
	optional_shared_ptr<UINT8> m_ram;

	int m_ram_size;
	bool m_rom_en;
};


// ======================> ql_trump_card_256k_t

class ql_trump_card_256k_t :  public ql_trump_card_t
{
public:
	// construction/destruction
	ql_trump_card_256k_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> ql_trump_card_512k_t

class ql_trump_card_512k_t :  public ql_trump_card_t
{
public:
	// construction/destruction
	ql_trump_card_512k_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> ql_trump_card_768k_t

class ql_trump_card_768k_t :  public ql_trump_card_t
{
public:
	// construction/destruction
	ql_trump_card_768k_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};



// device type definition
extern const device_type QL_TRUMP_CARD;
extern const device_type QL_TRUMP_CARD_256K;
extern const device_type QL_TRUMP_CARD_512K;
extern const device_type QL_TRUMP_CARD_768K;



#endif
