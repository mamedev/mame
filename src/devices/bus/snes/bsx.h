// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SNES_BSX_H
#define MAME_BUS_SNES_BSX_H

#pragma once

#include "snes_slot.h"
#include "rom.h"
#include "rom21.h"

// ======================> sns_rom_bsx_device

class sns_rom_bsx_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_bsx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// additional reading and writing
	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

protected:
	class bsx_base
	{
	public:
		bsx_base(running_machine &machine);
		running_machine &machine() const { return m_machine; }

		void init();
		uint8_t read(uint32_t offset);
		void write(uint32_t offset, uint8_t data);

	private:
		// regs
		uint8_t regs[0x18];       // 0x2188-0x219f

		// counter + clock
		uint8_t r2192_counter;
		uint8_t r2192_hour, r2192_minute, r2192_second;

		running_machine& m_machine;
	};

	sns_rom_bsx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void access_update();

	// base regs
	std::unique_ptr<bsx_base> m_base_unit;

	// cart regs
	uint8_t m_cart_regs[16];
	uint8_t access_00_1f; // 1 = CART, 0 = NOTHING
	uint8_t access_80_9f; // 1 = CART, 0 = NOTHING
	uint8_t access_40_4f; // 1 = NOTHING, 0 = PRAM
	uint8_t access_50_5f; // 1 = NOTHING, 0 = PRAM
	uint8_t access_60_6f; // 1 = PRAM, 0 = NOTHING
	uint8_t rom_access;   // 2 = HiROM, 1 = LoROM, 0 = PRAM

	uint8_t m_pram[0x80000];

private:
	required_device<sns_bsx_cart_slot_device> m_slot;
};

// ======================> sns_rom_bsxlo_device

class sns_rom_bsxlo_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_bsxlo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// additional reading and writing
	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<sns_bsx_cart_slot_device> m_slot;
};

// ======================> sns_rom_bsxhi_device

class sns_rom_bsxhi_device : public sns_rom21_device
{
public:
	// construction/destruction
	sns_rom_bsxhi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// additional reading and writing
	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<sns_bsx_cart_slot_device> m_slot;
};


// ======================> sns_rom_bsmempak_device

class sns_rom_bsmempak_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_bsmempak_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// additional reading and writing
	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
//  virtual void write_h(offs_t offset, uint8_t data) override;
//  virtual uint8_t chip_read(offs_t offset) override;
//  virtual void chip_write(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// flash regs
	uint32_t m_command;
	uint8_t m_write_old;
	uint8_t m_write_new;

	int m_flash_enable;
	int m_read_enable;
	int m_write_enable;
};


// device type definition
DECLARE_DEVICE_TYPE(SNS_ROM_BSX,   sns_rom_bsx_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_BSX, sns_rom_bsxlo_device)
DECLARE_DEVICE_TYPE(SNS_HIROM_BSX, sns_rom_bsxhi_device)
DECLARE_DEVICE_TYPE(SNS_BSMEMPAK,  sns_rom_bsmempak_device)

#endif // MAME_BUS_SNES_BSX_H
