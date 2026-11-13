// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_MEGADRIVE_EEPROM_H
#define MAME_BUS_MEGADRIVE_EEPROM_H

#pragma once

#include "md_slot.h"
#include "machine/i2cmem.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> md_std_eeprom_device

class md_std_eeprom_device : public device_t,
						public device_md_cart_interface
{
public:
	// construction/destruction
	md_std_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	md_std_eeprom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	required_device<i2cmem_device> m_i2cmem;
	uint8_t m_i2c_mem, m_i2c_clk;
};

// ======================> md_eeprom_nbajam_device

class md_eeprom_nbajam_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nbajam_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

// ======================> md_eeprom_nbajamte_device

class md_eeprom_nbajamte_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nbajamte_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

// ======================> md_eeprom_cslam_device (same read/write as nbajamte, but different I2C type)

class md_eeprom_cslam_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_cslam_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

// ======================> md_eeprom_nflqb96_device (same read/write as nbajamte, but different I2C type)

class md_eeprom_nflqb96_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nflqb96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

// ======================> md_eeprom_nhlpa_device

class md_eeprom_nhlpa_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nhlpa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

// ======================> md_eeprom_blara_device (same read/write as codemast, but different I2C type)

class md_eeprom_blara_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_blara_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
};

// ======================> md_eeprom_mode1_device

class md_eeprom_mode1_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_mode1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(MD_STD_EEPROM,      md_std_eeprom_device)
DECLARE_DEVICE_TYPE(MD_EEPROM_NBAJAM,   md_eeprom_nbajam_device)
DECLARE_DEVICE_TYPE(MD_EEPROM_NBAJAMTE, md_eeprom_nbajamte_device)
DECLARE_DEVICE_TYPE(MD_EEPROM_CSLAM,    md_eeprom_cslam_device)
DECLARE_DEVICE_TYPE(MD_EEPROM_NFLQB96,  md_eeprom_nflqb96_device)
DECLARE_DEVICE_TYPE(MD_EEPROM_NHLPA,    md_eeprom_nhlpa_device)
DECLARE_DEVICE_TYPE(MD_EEPROM_BLARA,    md_eeprom_blara_device)
DECLARE_DEVICE_TYPE(MD_EEPROM_MODE1,    md_eeprom_mode1_device)


#endif // MAME_BUS_MEGADRIVE_EEPROM_H
