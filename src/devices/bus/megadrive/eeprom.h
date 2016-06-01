// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __MD_EEPROM_H
#define __MD_EEPROM_H

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
	md_std_eeprom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	md_std_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;

	required_device<i2cmem_device> m_i2cmem;
	UINT8 m_i2c_mem, m_i2c_clk;
};

// ======================> md_eeprom_nbajam_device

class md_eeprom_nbajam_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nbajam_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;
};

// ======================> md_eeprom_nbajamte_device

class md_eeprom_nbajamte_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nbajamte_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;
};

// ======================> md_eeprom_cslam_device (same read/write as nbajamte, but different I2C type)

class md_eeprom_cslam_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_cslam_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;
};

// ======================> md_eeprom_nflqb_device (same read/write as nbajamte, but different I2C type)

class md_eeprom_nflqb_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nflqb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;
};

// ======================> md_eeprom_nhlpa_device

class md_eeprom_nhlpa_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nhlpa_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;
};

// ======================> md_eeprom_blara_device (same read/write as codemast, but different I2C type)

class md_eeprom_blara_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_blara_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;
};


// device type definition
extern const device_type MD_STD_EEPROM;
extern const device_type MD_EEPROM_NBAJAM;
extern const device_type MD_EEPROM_NBAJAMTE;
extern const device_type MD_EEPROM_CSLAM;
extern const device_type MD_EEPROM_NFLQB;
extern const device_type MD_EEPROM_NHLPA;
extern const device_type MD_EEPROM_BLARA;




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

// ======================> md_eeprom_nbajam_device_alt

class md_eeprom_nbajam_device_alt : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nbajam_device_alt(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
//  virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;

	std::vector<UINT8> m_sram;

	void eeprom_i2c_init();
	void idle_devsel_check();
	void eeprom_i2c_update();
	UINT8 eeprom_i2c_out();

private:
	// EEPROM runtime vars
	UINT8 m_eeprom_sda;     // current SDA
	UINT8 m_eeprom_prev_sda;    // previous SDA
	UINT8 m_eeprom_scl;     // current SCL
	UINT8 m_eeprom_prev_scl;   // previous SCL
	UINT8 m_eeprom_cnt;     // operation count in 0-9
	UINT8 m_eeprom_readwrite;     // read/write bit
	UINT16 m_eeprom_slave_mask; // dev addr
	UINT16 m_eeprom_word_address;  // memory addr
	UINT16 m_eeprom_devsel;  // selected device
	UINT16 m_eeprom_byte;  // byte to be written
	int m_eeprom_cur_state;  // current state
	// EEPROM physical characteristics (configured at init)
	UINT16 m_eeprom_mask;       // size of the memory - 1
	UINT16 m_eeprom_pagewrite_mask;  // max number of bytes that can be written in a single write cycle

};

extern const device_type MD_EEPROM_NBAJAM_ALT;



#endif
