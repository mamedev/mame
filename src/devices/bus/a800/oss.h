// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_A800_OSS_H
#define MAME_BUS_A800_OSS_H

#pragma once

#include "rom.h"


// ======================> a800_rom_oss8k_device

class a800_rom_oss8k_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_oss8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_80xx(offs_t offset) override;
	virtual void write_d5xx(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	int m_bank;
};


// ======================> a800_rom_oss34_device

class a800_rom_oss34_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_oss34_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_80xx(offs_t offset) override;
	virtual void write_d5xx(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	int m_bank;
};


// ======================> a800_rom_oss43_device

class a800_rom_oss43_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_oss43_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_80xx(offs_t offset) override;
	virtual void write_d5xx(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	int m_bank;
};


// ======================> a800_rom_oss91_device

class a800_rom_oss91_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_oss91_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_80xx(offs_t offset) override;
	virtual void write_d5xx(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	int m_bank;
};



// device type definition
DECLARE_DEVICE_TYPE(A800_ROM_OSS8K, a800_rom_oss8k_device)
DECLARE_DEVICE_TYPE(A800_ROM_OSS34, a800_rom_oss34_device)
DECLARE_DEVICE_TYPE(A800_ROM_OSS43, a800_rom_oss43_device)
DECLARE_DEVICE_TYPE(A800_ROM_OSS91, a800_rom_oss91_device)


#endif // MAME_BUS_A800_OSS_H
