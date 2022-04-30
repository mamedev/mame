// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NEGADRIVE_JCART_H
#define MAME_BUS_NEGADRIVE_JCART_H

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
	md_jcart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	md_jcart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	required_ioport m_jcart3;
	required_ioport m_jcart4;

private:
	uint8_t m_jcart_io_data[2];
};

// ======================> md_seprom_codemast_device

class md_seprom_codemast_device : public md_jcart_device
{
public:
	// construction/destruction
	md_seprom_codemast_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	md_seprom_codemast_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	required_device<i2cmem_device> m_i2cmem;

private:
	uint8_t m_jcart_io_data[2];
	uint8_t m_i2c_mem, m_i2c_clk;
};

// ======================> md_seprom_mm96_device (same read/write as codemast, but different I2C type)

class md_seprom_mm96_device : public md_seprom_codemast_device
{
public:
	// construction/destruction
	md_seprom_mm96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
};


// device type definition
DECLARE_DEVICE_TYPE(MD_JCART,           md_jcart_device)
DECLARE_DEVICE_TYPE(MD_SEPROM_CODEMAST, md_seprom_codemast_device)
DECLARE_DEVICE_TYPE(MD_SEPROM_MM96,     md_seprom_mm96_device)

#endif // MAME_BUS_NEGADRIVE_JCART_H
