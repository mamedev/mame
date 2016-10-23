// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SCV_ROM_H
#define __SCV_ROM_H

#include "slot.h"


// ======================> scv_rom8_device

class scv_rom8_device : public device_t,
						public device_scv_cart_interface
{
public:
	// construction/destruction
	scv_rom8_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	scv_rom8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};

// ======================> scv_rom16_device

class scv_rom16_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};


// ======================> scv_rom32_device

class scv_rom32_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};


// ======================> scv_rom32ram8_device

class scv_rom32ram8_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom32ram8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_cart(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	uint8_t m_ram_enabled;
};


// ======================> scv_rom64_device

class scv_rom64_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	uint8_t m_bank_base;
};


// ======================> scv_rom128_device

class scv_rom128_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	uint8_t m_bank_base;
};


// ======================> scv_rom128ram4_device

class scv_rom128ram4_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom128ram4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_cart(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_cart(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	uint8_t m_bank_base, m_ram_enabled;
};



// device type definition
extern const device_type SCV_ROM8K;
extern const device_type SCV_ROM16K;
extern const device_type SCV_ROM32K;
extern const device_type SCV_ROM32K_RAM8K;
extern const device_type SCV_ROM64K;
extern const device_type SCV_ROM128K;
extern const device_type SCV_ROM128K_RAM4K;



#endif
