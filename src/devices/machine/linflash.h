// license:BSD-3-Clause
// copyright-holders:smf
#pragma once

#ifndef __LINFLASH_H__
#define __LINFLASH_H__

#include "emu.h"
#include "intelfsh.h"
#include "machine/pccard.h"

class linear_flash_pccard_device : public device_t,
	public pccard_interface,
	public device_memory_interface,
	public device_slot_card_interface
{
public:
	virtual DECLARE_READ16_MEMBER(read_memory);
	virtual DECLARE_WRITE16_MEMBER(write_memory);

protected:
	linear_flash_pccard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock,const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config( address_spacenum spacenum = AS_0 ) const override;

	address_space_config m_space_config;
	address_space *m_space;
};


extern const device_type LINEAR_FLASH_PCCARD_16MB;

class linear_flash_pccard_16mb_device : public linear_flash_pccard_device
{
public:
	linear_flash_pccard_16mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};


extern const device_type LINEAR_FLASH_PCCARD_32MB;

class linear_flash_pccard_32mb_device : public linear_flash_pccard_device
{
public:
	linear_flash_pccard_32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};

extern const device_type LINEAR_FLASH_PCCARD_64MB;

class linear_flash_pccard_64mb_device : public linear_flash_pccard_device
{
public:
	linear_flash_pccard_64mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};

#endif
