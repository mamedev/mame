// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro PALPROM carrier boards

***************************************************************************/

#ifndef MAME_BUS_BBC_ROM_PAL_H
#define MAME_BUS_BBC_ROM_PAL_H

#pragma once

#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_pal_device

class bbc_pal_device : public device_t,
	public device_bbc_rom_interface
{
protected:
	// construction/destruction
	bbc_pal_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_bbc_rom_interface overrides
	virtual uint32_t get_rom_size() override { return 0x4000; }

	uint8_t m_bank;
};

// ======================> bbc_cciword_device

class bbc_cciword_device : public bbc_pal_device
{
public:
	// construction/destruction
	bbc_cciword_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
};

// ======================> bbc_ccibase_device

class bbc_ccibase_device : public bbc_pal_device
{
public:
	// construction/destruction
	bbc_ccibase_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
};

// ======================> bbc_ccispell_device

class bbc_ccispell_device : public bbc_pal_device
{
public:
	// construction/destruction
	bbc_ccispell_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
};

// ======================> bbc_palqst_device

class bbc_palqst_device : public bbc_pal_device
{
public:
	bbc_palqst_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
};

// ======================> bbc_palwap_device

class bbc_palwap_device : public bbc_pal_device
{
public:
	bbc_palwap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
};

// ======================> bbc_palted_device

class bbc_palted_device : public bbc_pal_device
{
public:
	// construction/destruction
	bbc_palted_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
};

// ======================> bbc_palabep_device

class bbc_palabep_device : public bbc_pal_device
{
public:
	// construction/destruction
	bbc_palabep_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
};

// ======================> bbc_palabe_device

class bbc_palabe_device : public bbc_pal_device
{
public:
	// construction/destruction
	bbc_palabe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
};

// ======================> bbc_palmo2_device

class bbc_palmo2_device : public bbc_pal_device
{
public:
	// construction/destruction
	bbc_palmo2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
};

// ======================> bbc_trilogy_device

class bbc_trilogy_device : public bbc_pal_device
{
public:
	// construction/destruction
	bbc_trilogy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_CCIWORD, bbc_cciword_device)
DECLARE_DEVICE_TYPE(BBC_CCIBASE, bbc_ccibase_device)
DECLARE_DEVICE_TYPE(BBC_CCISPELL, bbc_ccispell_device)
DECLARE_DEVICE_TYPE(BBC_PALQST, bbc_palqst_device)
DECLARE_DEVICE_TYPE(BBC_PALWAP, bbc_palwap_device)
DECLARE_DEVICE_TYPE(BBC_PALTED, bbc_palted_device)
DECLARE_DEVICE_TYPE(BBC_PALABEP, bbc_palabep_device)
DECLARE_DEVICE_TYPE(BBC_PALABE, bbc_palabe_device)
DECLARE_DEVICE_TYPE(BBC_PALMO2, bbc_palmo2_device)
DECLARE_DEVICE_TYPE(BBC_TRILOGY, bbc_trilogy_device)


#endif // MAME_BUS_BBC_ROM_PAL_H
