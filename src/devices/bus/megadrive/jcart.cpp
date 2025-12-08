// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 MegaDrive / Genesis J-Cart + I2C EEPROM emulation


 Emulation based on earlier research by ElBarto and Eke-Eke


 I2C games mapping table:

 Game Name                         |   SDA_IN   |  SDA_OUT   |     SCL    |  SIZE_MASK     | PAGE_MASK | WORKS |
 ----------------------------------|------------|------------|------------|----------------|-----------|-------|
 Micro Machines 2                  | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x03ff (24C08) |   0x0f    |  Yes  |
 Micro Machines Military           | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x03ff (24C08) |   0x0f    |  Yes  |
 Micro Machines 96                 | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x07ff (24C16) |   0x0f    |  Yes  |
 ----------------------------------|------------|------------|------------|----------------|-----------|-------|

***********************************************************************************************************/



#include "emu.h"
#include "jcart.h"

#include "bus/sms_ctrl/controllers.h"


//-------------------------------------------------
//  md_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(MD_JCART,           md_jcart_device,           "md_jcart",           "MD J-Cart games")
DEFINE_DEVICE_TYPE(MD_SEPROM_CODEMAST, md_seprom_codemast_device, "md_seprom_codemast", "MD J-Cart games + SEPROM")
DEFINE_DEVICE_TYPE(MD_SEPROM_MM96,     md_seprom_mm96_device,     "md_seprom_mm96",     "MD Micro Machines 96")

// Sampras, Super Skidmarks?
md_jcart_device::md_jcart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_md_cart_interface(mconfig, *this)
	, m_ctrl_ports(*this, "control%u", 3U)
{
}

md_jcart_device::md_jcart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_jcart_device(mconfig, MD_JCART, tag, owner, clock)
{
}

// Micro Machines 2, Micro Machines Military
md_seprom_codemast_device::md_seprom_codemast_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: md_jcart_device(mconfig, type, tag, owner, clock)
	, m_i2cmem(*this, "i2cmem"), m_i2c_mem(0), m_i2c_clk(0)
{
}

md_seprom_codemast_device::md_seprom_codemast_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_seprom_codemast_device(mconfig, MD_SEPROM_CODEMAST, tag, owner, clock)
{
}

// Micro Machines 96
md_seprom_mm96_device::md_seprom_mm96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_seprom_codemast_device(mconfig, MD_SEPROM_MM96, tag, owner, clock)
{
}


//-------------------------------------------------
//  SERIAL I2C DEVICE
//-------------------------------------------------


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void md_jcart_device::device_add_mconfig(machine_config &config)
{
	SMS_CONTROL_PORT(config, m_ctrl_ports[0], sms_control_port_devices, SMS_CTRL_OPTION_MD_PAD);
	m_ctrl_ports[0]->th_handler().set(FUNC(md_jcart_device::th_in<0>));

	SMS_CONTROL_PORT(config, m_ctrl_ports[1], sms_control_port_devices, SMS_CTRL_OPTION_MD_PAD);
	m_ctrl_ports[1]->th_handler().set(FUNC(md_jcart_device::th_in<1>));
}

void md_seprom_codemast_device::device_add_mconfig(machine_config &config)
{
	md_jcart_device::device_add_mconfig(config);

	I2C_24C08(config, m_i2cmem);
}

void md_seprom_mm96_device::device_add_mconfig(machine_config &config)
{
	md_jcart_device::device_add_mconfig(config);

	I2C_24C16(config, m_i2cmem); // 24C16A
}


void md_jcart_device::device_resolve_objects()
{
	m_th_in[0] = m_th_in[1] = 0x40;
	m_th_out = 0x40;
}


void md_jcart_device::device_start()
{
	save_item(NAME(m_th_in));
	save_item(NAME(m_th_out));
}

void md_jcart_device::device_reset()
{
	// TODO: does this cause the TH outputs to reset to high?
}

void md_seprom_codemast_device::device_start()
{
	md_jcart_device::device_start();

	save_item(NAME(m_i2c_mem));
	save_item(NAME(m_i2c_clk));
	save_item(NAME(m_jcart_io_data));
}

void md_seprom_codemast_device::device_reset()
{
	md_jcart_device::device_reset();

	m_i2c_mem = 0;
	m_i2c_clk = 0;
	m_jcart_io_data[0] = 0;
	m_jcart_io_data[1] = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------
 J-CART ONLY (Pete Sampras Tennis)
 -------------------------------------------------*/

uint16_t md_jcart_device::read(offs_t offset)
{
	if (offset == 0x38fffe/2)
	{
		uint16_t const ctrl3 = (m_ctrl_ports[0]->in_r() & 0x3f) | (m_th_in[0] & m_th_out) | 0x80;
		uint16_t const ctrl4 = (m_ctrl_ports[1]->in_r() & 0x3f) | (m_th_in[1] & m_th_out) | 0x80;
		return ctrl3 | (ctrl4 << 8);
	}
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_jcart_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x38fffe/2)
	{
		// assume TH only actively driven low
		m_th_out = BIT(data, 0) << 6;
		m_ctrl_ports[0]->out_w(m_th_out | 0x3f, ~m_th_out & 0x40);
		m_ctrl_ports[1]->out_w(m_th_out | 0x3f, ~m_th_out & 0x40);
	}
}

template <unsigned N>
void md_jcart_device::th_in(int state)
{
	m_th_in[N] = state ? 0x40 : 0x00;
}


/*-------------------------------------------------
 J-CART + SEPROM
 -------------------------------------------------*/

uint16_t md_seprom_codemast_device::read(offs_t offset)
{
	if (offset == 0x380000/2)
	{
		m_i2c_mem = m_i2cmem->read_sda();
		return (m_i2c_mem & 1) << 7;
	}
	else
	{
		return md_jcart_device::read(offset);
	}
}

void md_seprom_codemast_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x300000/2)
	{
		m_i2c_clk = BIT(data, 9);
		m_i2c_mem = BIT(data, 8);
		m_i2cmem->write_scl(m_i2c_clk);
		m_i2cmem->write_sda(m_i2c_mem);
	}
	else
	{
		md_jcart_device::write(offset, data, mem_mask);
	}
}
