// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __MD_JCART_H
#define __MD_JCART_H

#include "md_slot.h"
#include "machine/i2cmem.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> md_jcart_device

class md_jcart_device : public device_t,
					public device_md_cart_interface
{
public:
	// construction/destruction
	md_jcart_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	md_jcart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;

	required_ioport m_jcart3;
	required_ioport m_jcart4;

private:
	UINT8 m_jcart_io_data[2];
};

// ======================> md_seprom_codemast_device

class md_seprom_codemast_device : public md_jcart_device
{
public:
	// construction/destruction
	md_seprom_codemast_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	md_seprom_codemast_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;

	required_device<i2cmem_device> m_i2cmem;

private:
	UINT8 m_jcart_io_data[2];
	UINT8 m_i2c_mem, m_i2c_clk;
};

// ======================> md_seprom_mm96_device (same read/write as codemast, but different I2C type)

class md_seprom_mm96_device : public md_seprom_codemast_device
{
public:
	// construction/destruction
	md_seprom_mm96_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};



// device type definition
extern const device_type MD_JCART;
extern const device_type MD_SEPROM_CODEMAST;
extern const device_type MD_SEPROM_MM96;

#endif
