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
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);

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
	virtual machine_config_constructor device_mconfig_additions() const;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);
};

// ======================> md_eeprom_nbajamte_device

class md_eeprom_nbajamte_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nbajamte_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);
};

// ======================> md_eeprom_cslam_device (same read/write as nbajamte, but different I2C type)

class md_eeprom_cslam_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_cslam_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);
};

// ======================> md_eeprom_nflqb_device (same read/write as nbajamte, but different I2C type)

class md_eeprom_nflqb_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nflqb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);
};

// ======================> md_eeprom_nhlpa_device

class md_eeprom_nhlpa_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_nhlpa_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);
};

// ======================> md_eeprom_blara_device (same read/write as codemast, but different I2C type)

class md_eeprom_blara_device : public md_std_eeprom_device
{
public:
	// construction/destruction
	md_eeprom_blara_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);
};


// device type definition
extern const device_type MD_STD_EEPROM;
extern const device_type MD_EEPROM_NBAJAM;
extern const device_type MD_EEPROM_NBAJAMTE;
extern const device_type MD_EEPROM_CSLAM;
extern const device_type MD_EEPROM_NFLQB;
extern const device_type MD_EEPROM_NHLPA;
extern const device_type MD_EEPROM_BLARA;

#endif
