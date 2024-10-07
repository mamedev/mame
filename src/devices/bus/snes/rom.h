// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SNES_ROM_H
#define MAME_BUS_SNES_ROM_H

#pragma once

#include "snes_slot.h"


// ======================> sns_rom_device

class sns_rom_device : public device_t, public device_sns_cart_interface
{
public:
	// construction/destruction
	sns_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;

protected:
	sns_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

// ======================> sns_rom_obc1_device

class sns_rom_obc1_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_obc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// additional reading and writing
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

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

	// reading and writing (protection device)
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t m_latch;
};

// ======================> sns_rom_tekken2_device

class sns_rom_tekken2_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_tekken2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing (protection device)
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

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

	// reading and writing (protection device)
	virtual uint8_t chip_read(offs_t offset) override;
};

// ======================> sns_rom_mcpirate1_device

class sns_rom_mcpirate1_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_mcpirate1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;  // bankswitch device

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t m_base_bank;
};

// ======================> sns_rom_mcpirate2_device

class sns_rom_mcpirate2_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_mcpirate2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;  // bankswitch device

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t m_base_bank;
};

// ======================> sns_rom_20col_device

class sns_rom_20col_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_20col_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;  // bankswitch device

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	uint8_t m_base_bank;
};

// ======================> sns_rom_banana_device

class sns_rom_banana_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_banana_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing (protection device)
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
//  virtual void device_start();
//  virtual void device_reset();

	uint8_t m_latch[16];
};

// ======================> sns_rom_bugs_device

class sns_rom_bugs_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_bugs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing (protection device)
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t m_latch[0x800];
};


// device type definition
DECLARE_DEVICE_TYPE(SNS_LOROM,          sns_rom_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_OBC1,     sns_rom_obc1_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_POKEMON,  sns_rom_pokemon_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_TEKKEN2,  sns_rom_tekken2_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_SOULBLAD, sns_rom_soulblad_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_MCPIR1,   sns_rom_mcpirate1_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_MCPIR2,   sns_rom_mcpirate2_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_20COL,    sns_rom_20col_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_BANANA,   sns_rom_banana_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_BUGSLIFE, sns_rom_bugs_device)

#endif // MAME_BUS_SNES_ROM_H
