// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SNS_BSX_H
#define __SNS_BSX_H

#include "snes_slot.h"
#include "rom.h"
#include "rom21.h"

class BSX_base
{
public:
	BSX_base(running_machine &machine);
	running_machine &machine() const { return m_machine; }

	void init();
	UINT8 read(UINT32 offset);
	void write(UINT32 offset, UINT8 data);

private:
	// regs
	UINT8 regs[0x18];       // 0x2188-0x219f

	// counter + clock
	UINT8 r2192_counter;
	UINT8 r2192_hour, r2192_minute, r2192_second;

	running_machine& m_machine;
};

// ======================> sns_rom_bsx_device

class sns_rom_bsx_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_bsx_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	sns_rom_bsx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_h);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual DECLARE_READ8_MEMBER(chip_read);
	virtual DECLARE_WRITE8_MEMBER(chip_write);

	// base regs
	BSX_base *m_base_unit;

	// cart regs
	UINT8 m_cart_regs[16];
	UINT8 access_00_1f; // 1 = CART, 0 = NOTHING
	UINT8 access_80_9f; // 1 = CART, 0 = NOTHING
	UINT8 access_40_4f; // 1 = NOTHING, 0 = PRAM
	UINT8 access_50_5f; // 1 = NOTHING, 0 = PRAM
	UINT8 access_60_6f; // 1 = PRAM, 0 = NOTHING
	UINT8 rom_access;   // 2 = HiROM, 1 = LoROM, 0 = PRAM
	void access_update();


	UINT8 m_pram[0x80000];

private:
	required_device<sns_bsx_cart_slot_device> m_slot;
};

// ======================> sns_rom_bsxlo_device

class sns_rom_bsxlo_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_bsxlo_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_h);

private:
	required_device<sns_bsx_cart_slot_device> m_slot;
};

// ======================> sns_rom_bsxhi_device

class sns_rom_bsxhi_device : public sns_rom21_device
{
public:
	// construction/destruction
	sns_rom_bsxhi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_h);

private:
	required_device<sns_bsx_cart_slot_device> m_slot;
};


// ======================> sns_rom_bsmempak_device

class sns_rom_bsmempak_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_bsmempak_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_h);
	virtual DECLARE_WRITE8_MEMBER(write_l);
//  virtual DECLARE_WRITE8_MEMBER(write_h);
//  virtual DECLARE_READ8_MEMBER(chip_read);
//  virtual DECLARE_WRITE8_MEMBER(chip_write);

	// flash regs
	UINT32 m_command;
	UINT8 m_write_old;
	UINT8 m_write_new;

	int m_flash_enable;
	int m_read_enable;
	int m_write_enable;
};


// device type definition
extern const device_type SNS_ROM_BSX;
extern const device_type SNS_LOROM_BSX;
extern const device_type SNS_HIROM_BSX;
extern const device_type SNS_BSMEMPAK;

#endif
