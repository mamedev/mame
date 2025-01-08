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

// ======================> md_eeprom_nflqb_device (same read/write as nbajamte, but different I2C type)

class md_eeprom_nflqb_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nflqb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
DECLARE_DEVICE_TYPE(MD_EEPROM_NFLQB,    md_eeprom_nflqb_device)
DECLARE_DEVICE_TYPE(MD_EEPROM_NHLPA,    md_eeprom_nhlpa_device)
DECLARE_DEVICE_TYPE(MD_EEPROM_BLARA,    md_eeprom_blara_device)
DECLARE_DEVICE_TYPE(MD_EEPROM_MODE1,    md_eeprom_mode1_device)




// TEMPORARY ADDITION UNTIL WE FIND OUT WHAT IS MISSING IN THE CORE X24C02 CODE
// THIS IS A CUSTOM I2C EEPROM EMULATION THAT ALLOWS NBA JAM TO WORK
enum
{
	STATE_I2C_IDLE = 0,
	STATE_I2C_WAIT_STOP,
	STATE_I2C_DEVSEL,
	STATE_I2C_GET_WORD_ADDR,
	STATE_I2C_WRITE_DATA,
	STATE_I2C_READ_DATA
};

// ======================> md_eeprom_nbajam_alt_device

class md_eeprom_nbajam_alt_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nbajam_alt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void eeprom_i2c_init();
	void idle_devsel_check();
	void eeprom_i2c_update();
	uint8_t eeprom_i2c_out();

	std::vector<uint8_t> m_sram;

	// EEPROM runtime vars
	uint8_t m_eeprom_sda;     // current SDA
	uint8_t m_eeprom_prev_sda;    // previous SDA
	uint8_t m_eeprom_scl;     // current SCL
	uint8_t m_eeprom_prev_scl;   // previous SCL
	uint8_t m_eeprom_cnt;     // operation count in 0-9
	uint8_t m_eeprom_readwrite;     // read/write bit
	uint16_t m_eeprom_slave_mask; // dev addr
	uint16_t m_eeprom_word_address;  // memory addr
	uint16_t m_eeprom_devsel;  // selected device
	uint16_t m_eeprom_byte;  // byte to be written
	int m_eeprom_cur_state;  // current state
	// EEPROM physical characteristics (configured at init)
	uint16_t m_eeprom_mask;       // size of the memory - 1
	uint16_t m_eeprom_pagewrite_mask;  // max number of bytes that can be written in a single write cycle

};

DECLARE_DEVICE_TYPE(MD_EEPROM_NBAJAM_ALT, md_eeprom_nbajam_alt_device)


#endif // MAME_BUS_MEGADRIVE_EEPROM_H
