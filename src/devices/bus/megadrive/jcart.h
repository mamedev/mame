// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_MEGADRIVE_JCART_H
#define MAME_BUS_MEGADRIVE_JCART_H

#include "md_slot.h"

#include "bus/sms_ctrl/smsctrl.h"
#include "machine/i2cmem.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class md_jcart_device : public device_t,
					public device_md_cart_interface
{
public:
	// construction/destruction
	md_jcart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	md_jcart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template <unsigned N> void th_in(int state);

	required_device_array<sms_control_port_device, 2> m_ctrl_ports;
	uint8_t m_th_in[2];
	uint8_t m_th_out;
};


class md_seprom_codemast_device : public md_jcart_device
{
public:
	// construction/destruction
	md_seprom_codemast_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	md_seprom_codemast_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	required_device<i2cmem_device> m_i2cmem;

private:
	uint8_t m_jcart_io_data[2];
	uint8_t m_i2c_mem, m_i2c_clk;
};


class md_seprom_mm96_device : public md_seprom_codemast_device
{
public:
	// construction/destruction
	md_seprom_mm96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(MD_JCART,           md_jcart_device)
DECLARE_DEVICE_TYPE(MD_SEPROM_CODEMAST, md_seprom_codemast_device)
DECLARE_DEVICE_TYPE(MD_SEPROM_MM96,     md_seprom_mm96_device)

#endif // MAME_BUS_MEGADRIVE_JCART_H
