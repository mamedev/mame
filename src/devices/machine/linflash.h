// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_LINFLASH_H
#define MAME_MACHINE_LINFLASH_H

#pragma once

#include "intelfsh.h"
#include "machine/pccard.h"

class linear_flash_pccard_device : public device_t,
	public pccard_interface,
	public device_memory_interface,
	public device_slot_card_interface
{
public:
	virtual DECLARE_READ16_MEMBER(read_memory) override;
	virtual DECLARE_WRITE16_MEMBER(write_memory) override;

protected:
	linear_flash_pccard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_space_config;
	address_space *m_space;
};


class linear_flash_pccard_16mb_device : public linear_flash_pccard_device
{
public:
	linear_flash_pccard_16mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void linear_flash_pccard_16mb(address_map &map);
};


class linear_flash_pccard_32mb_device : public linear_flash_pccard_device
{
public:
	linear_flash_pccard_32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void linear_flash_pccard_32mb(address_map &map);
};


class linear_flash_pccard_64mb_device : public linear_flash_pccard_device
{
public:
	linear_flash_pccard_64mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void linear_flash_pccard_64mb(address_map &map);
};


DECLARE_DEVICE_TYPE(LINEAR_FLASH_PCCARD_16MB, linear_flash_pccard_16mb_device)
DECLARE_DEVICE_TYPE(LINEAR_FLASH_PCCARD_32MB, linear_flash_pccard_32mb_device)
DECLARE_DEVICE_TYPE(LINEAR_FLASH_PCCARD_64MB, linear_flash_pccard_64mb_device)

#endif // MAME_MACHINE_LINFLASH_H
