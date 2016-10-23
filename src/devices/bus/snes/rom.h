// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SNS_ROM_H
#define __SNS_ROM_H

#include "snes_slot.h"


// ======================> sns_rom_device

class sns_rom_device : public device_t,
						public device_sns_cart_interface
{
public:
	// construction/destruction
	sns_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	sns_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// reading and writing
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};

// ======================> sns_rom_obc1_device

class sns_rom_obc1_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_obc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// additional reading and writing
	virtual uint8_t chip_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void chip_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	int m_address;
	int m_offset;
	int m_shift;
	uint8_t m_ram[0x2000];
};



// ======================> sns_rom_pokemon_device

class sns_rom_pokemon_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_pokemon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t chip_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;    // protection device
	virtual void chip_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;  // protection device
	uint8_t m_latch;
};

// ======================> sns_rom_tekken2_device

class sns_rom_tekken2_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_tekken2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t chip_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;    // protection device
	virtual void chip_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;  // protection device

	void update_prot(uint32_t offset);

	// bit0-3 prot value, bit4 direction, bit5 function
	// reads must return (prot value) +1/-1/<<1/>>1 depending on bit4 & bit5
	uint8_t m_prot;
};

// ======================> sns_rom_soulblad_device

class sns_rom_soulblad_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_soulblad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t chip_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;    // protection device
};

// ======================> sns_rom_mcpirate1_device

class sns_rom_mcpirate1_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_mcpirate1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void chip_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;  // bankswitch device
	uint8_t m_base_bank;
};

// ======================> sns_rom_mcpirate2_device

class sns_rom_mcpirate2_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_mcpirate2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void chip_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;  // bankswitch device
	uint8_t m_base_bank;
};

// ======================> sns_rom_20col_device

class sns_rom_20col_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_20col_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// reading and writing
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_h(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void chip_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;  // bankswitch device
	uint8_t m_base_bank;
};

// ======================> sns_rom_banana_device

class sns_rom_banana_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_banana_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
//  virtual void device_start();
//  virtual void device_reset();

	// reading and writing
	virtual uint8_t chip_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;    // protection device
	virtual void chip_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;  // protection device
	uint8_t m_latch[16];
};

// ======================> sns_rom_bugs_device

class sns_rom_bugs_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_bugs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t chip_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;    // protection device
	virtual void chip_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;  // protection device
	uint8_t m_latch[0x800];
};


// device type definition
extern const device_type SNS_LOROM;
extern const device_type SNS_LOROM_OBC1;
extern const device_type SNS_LOROM_POKEMON;
extern const device_type SNS_LOROM_TEKKEN2;
extern const device_type SNS_LOROM_SOULBLAD;
extern const device_type SNS_LOROM_MCPIR1;
extern const device_type SNS_LOROM_MCPIR2;
extern const device_type SNS_LOROM_20COL;
extern const device_type SNS_LOROM_BANANA;
extern const device_type SNS_LOROM_BUGSLIFE;

#endif
