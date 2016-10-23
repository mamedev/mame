// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __A800_OSS_H
#define __A800_OSS_H

#include "rom.h"


// ======================> a800_rom_oss8k_device

class a800_rom_oss8k_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_oss8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_80xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_d5xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

protected:
	int m_bank;
};


// ======================> a800_rom_oss34_device

class a800_rom_oss34_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_oss34_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_80xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_d5xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

protected:
	int m_bank;
};


// ======================> a800_rom_oss43_device

class a800_rom_oss43_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_oss43_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_80xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_d5xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

protected:
	int m_bank;
};


// ======================> a800_rom_oss91_device

class a800_rom_oss91_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_oss91_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_80xx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_d5xx(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

protected:
	int m_bank;
};



// device type definition
extern const device_type A800_ROM_OSS8K;
extern const device_type A800_ROM_OSS34;
extern const device_type A800_ROM_OSS43;
extern const device_type A800_ROM_OSS91;


#endif
