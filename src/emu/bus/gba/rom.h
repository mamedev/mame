// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz,Fabio Priuli
#ifndef __GBA_ROM_H
#define __GBA_ROM_H

#include "gba_slot.h"
#include "machine/intelfsh.h"


// ======================> gba_rom_device

class gba_rom_device : public device_t,
						public device_gba_cart_interface
{
public:
	// construction/destruction
	gba_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	gba_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_rom) { return m_rom[offset]; }
};

// ======================> gba_rom_sram_device

class gba_rom_sram_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_ram);
	virtual DECLARE_WRITE32_MEMBER(write_ram);
};

// ======================> gba_rom_flash_device

class gba_rom_flash_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_ram);
	virtual DECLARE_WRITE32_MEMBER(write_ram);

private:
	//UINT32 m_flash_size;
	UINT32 m_flash_mask;
	required_device<intelfsh8_device> m_flash;
};

// ======================> gba_rom_flash1m_device

class gba_rom_flash1m_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_flash1m_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_ram);
	virtual DECLARE_WRITE32_MEMBER(write_ram);

private:
	//UINT32 m_flash_size;
	UINT32 m_flash_mask;
	required_device<intelfsh8_device> m_flash;
};

// GBA EEPROM device
// TODO: is it possible to merge this with the standard EEPROM devices in the core?

enum
{
	EEP_IDLE = 0,
	EEP_COMMAND,
	EEP_ADDR,
	EEP_AFTERADDR,
	EEP_READ,
	EEP_WRITE,
	EEP_AFTERWRITE,
	EEP_READFIRST
};

class gba_eeprom_device
{
public:
	gba_eeprom_device(running_machine &machine, UINT8 *eeprom, UINT32 size, int addr_bits);
	running_machine &machine() const { return m_machine; }

	UINT32 read();
	void write(UINT32 data);

protected:
	UINT8 *m_data;
	UINT32 m_data_size;
	int m_state;
	int m_command;
	int m_count;
	int m_addr;
	int m_bits;
	int m_addr_bits;
	UINT8 m_eep_data;

	running_machine& m_machine;
};


// ======================> gba_rom_eeprom_device

class gba_rom_eeprom_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_ram);
	virtual DECLARE_WRITE32_MEMBER(write_ram);

private:
	auto_pointer<gba_eeprom_device> m_eeprom;
};


// ======================> gba_rom_eeprom64_device

class gba_rom_eeprom64_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_eeprom64_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_ram);
	virtual DECLARE_WRITE32_MEMBER(write_ram);

private:
	auto_pointer<gba_eeprom_device> m_eeprom;
};


// device type definition
extern const device_type GBA_ROM_STD;
extern const device_type GBA_ROM_SRAM;
extern const device_type GBA_ROM_EEPROM;
extern const device_type GBA_ROM_EEPROM64;
extern const device_type GBA_ROM_FLASH;
extern const device_type GBA_ROM_FLASH1M;



#endif
