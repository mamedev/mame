// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 MegaDrive / Genesis cart + I2C EEPROM emulation


 TODO: EEPROM hookup verification in most games.


 I2C games mapping table:

 Game Name                         |   SDA_IN   |  SDA_OUT   |     SCL    |  SIZE_MASK     | PAGE_MASK | WORKS |
 ----------------------------------|------------|------------|------------|----------------|-----------|-------|
 NBA Jam                           | 0x200001-1 | 0x200001-0 | 0x200001-1 | 0x00ff (24C02) |   0x03    |       |
 NBA Jam TE                        | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x00ff (24C02) |   0x03    |       |
 NBA Jam TE (32x)                  | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x00ff (24C02) |   0x03    |       |
 NFL Quarterback Club              | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x00ff (24C02) |   0x03    |       |
 NFL Quarterback Club 96           | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x07ff (24C16) |   0x07    |       |
 College Slam                      | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x1fff (24C64) |   0x07    |       |
 Frank Thomas Big Hurt Baseball    | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x1fff (24C64) |   0x07    |       |
 NHLPA Hockey 93                   | 0x200001-7 | 0x200001-7 | 0x200001-6 | 0x007f (24C01) |   0x03    |       |
 Rings of Power                    | 0x200001-7 | 0x200001-7 | 0x200001-6 | 0x007f (24C01) |   0x03    |       |
 Honoo no Toukyuuji - Dodge Danpei | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    |  Unk. |
 Evander Holyfield's Boxing        | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    |  Yes  |
 Greatest Heavyweights of the Ring | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    |  Yes  |
 Wonder Boy V                      | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    |  Yes  |
 Sports Talk Baseball              | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    |  Yes  |
 Megaman - The Wily Wars           | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    |  Yes  | **
 Micro Machines 2                  | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x03ff (24C08) |   0x0f    |  Yes  |
 Micro Machines Military           | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x03ff (24C08) |   0x0f    |  Yes  |
 Micro Machines 96                 | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x07ff (24C16) |   0x0f    |  Yes  |
 Brian Lara Cricket 96             | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x1fff (24C64) |   0x??*   |       |
 Shane Warne Cricket               | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x1fff (24C64) |   0x??*   |       |
 ----------------------------------|------------|------------|------------|----------------|-----------|-------|

 * Not specified in Eke-Eke's document
 ** Original Rockman Mega World (J) set uses normal backup RAM

 Micro Machines 2, Micro Machines Military, Micro Machines 96 are emulated in md_jcart

***********************************************************************************************************/



#include "emu.h"
#include "eeprom.h"


//-------------------------------------------------
//  md_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(MD_STD_EEPROM,      md_std_eeprom_device,      "md_std_eeprom",      "MD Standard cart + EEPROM")
DEFINE_DEVICE_TYPE(MD_EEPROM_NBAJAM,   md_eeprom_nbajam_device,   "md_eeprom_nbajam",   "MD NBA Jam")
DEFINE_DEVICE_TYPE(MD_EEPROM_NBAJAMTE, md_eeprom_nbajamte_device, "md_eeprom_nbajamte", "MD NBA Jam TE") // and a few more
DEFINE_DEVICE_TYPE(MD_EEPROM_NFLQB96,  md_eeprom_nflqb96_device,  "md_eeprom_nflqb96",  "MD NFL Quarterback 96")
DEFINE_DEVICE_TYPE(MD_EEPROM_CSLAM,    md_eeprom_cslam_device,    "md_eeprom_cslam",    "MD College Slam")
DEFINE_DEVICE_TYPE(MD_EEPROM_NHLPA,    md_eeprom_nhlpa_device,    "md_eeprom_nhlpa",    "MD NHLPA 93")
DEFINE_DEVICE_TYPE(MD_EEPROM_BLARA,    md_eeprom_blara_device,    "md_eeprom_blara",    "MD Brian Lara")
DEFINE_DEVICE_TYPE(MD_EEPROM_MODE1,    md_eeprom_mode1_device,    "md_eeprom_mode1",    "MD Serial EEPROM Mode 1")


md_std_eeprom_device::md_std_eeprom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_md_cart_interface(mconfig, *this)
	, m_i2cmem(*this, "i2cmem"), m_i2c_mem(0), m_i2c_clk(0)
{
}

md_std_eeprom_device::md_std_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_eeprom_device(mconfig, MD_STD_EEPROM, tag, owner, clock)
{
}

md_eeprom_nbajam_device::md_eeprom_nbajam_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_eeprom_device(mconfig, MD_EEPROM_NBAJAM, tag, owner, clock)
{
}

md_eeprom_nbajamte_device::md_eeprom_nbajamte_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_eeprom_device(mconfig, MD_EEPROM_NBAJAMTE, tag, owner, clock)
{
}

md_eeprom_nflqb96_device::md_eeprom_nflqb96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_eeprom_device(mconfig, MD_EEPROM_NFLQB96, tag, owner, clock)
{
}

md_eeprom_cslam_device::md_eeprom_cslam_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_eeprom_device(mconfig, MD_EEPROM_CSLAM, tag, owner, clock)
{
}

md_eeprom_nhlpa_device::md_eeprom_nhlpa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_eeprom_device(mconfig, MD_EEPROM_NHLPA, tag, owner, clock)
{
}

md_eeprom_blara_device::md_eeprom_blara_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_eeprom_device(mconfig, MD_EEPROM_BLARA, tag, owner, clock)
{
}

md_eeprom_mode1_device::md_eeprom_mode1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_eeprom_device(mconfig, MD_EEPROM_MODE1, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void md_std_eeprom_device::device_add_mconfig(machine_config &config)
{
	I2C_24C01(config, m_i2cmem);
}

void md_eeprom_nbajam_device::device_add_mconfig(machine_config &config)
{
	// nflqb uses a 24LC02B
	I2C_24C02(config, m_i2cmem);
}

void md_eeprom_nbajamte_device::device_add_mconfig(machine_config &config)
{
	I2C_24C04(config, m_i2cmem); // 24LC04B
}

void md_eeprom_cslam_device::device_add_mconfig(machine_config &config)
{
	I2C_24C65(config, m_i2cmem);
}

void md_eeprom_nflqb96_device::device_add_mconfig(machine_config &config)
{
	I2C_24C16(config, m_i2cmem);
}

void md_eeprom_nhlpa_device::device_add_mconfig(machine_config &config)
{
	I2C_24C01(config, m_i2cmem);
}

void md_eeprom_blara_device::device_add_mconfig(machine_config &config)
{
	I2C_24C65(config, m_i2cmem);
}

void md_eeprom_mode1_device::device_add_mconfig(machine_config &config)
{
	I2C_X24C01(config, m_i2cmem);
}

void md_std_eeprom_device::device_start()
{
	save_item(NAME(m_i2c_mem));
	save_item(NAME(m_i2c_clk));
}

void md_std_eeprom_device::device_reset()
{
	m_i2c_mem = 0;
	m_i2c_clk = 0;
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------
 CART + EEPROM
 -------------------------------------------------*/

uint16_t md_std_eeprom_device::read(offs_t offset)
{
	if (offset == 0x200000/2)
	{
		const uint8_t data = m_i2cmem->read_sda();
		return ((uint16_t)data << 8) | data;
	}
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_std_eeprom_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x200000/2)
	{
		m_i2c_clk = BIT(data, 1);
		m_i2c_mem = BIT(data, 0);
		m_i2cmem->write_scl(m_i2c_clk);
		m_i2cmem->write_sda(m_i2c_mem);
	}
}

uint16_t md_eeprom_nbajam_device::read(offs_t offset)
{
	if (offset == 0x200000/2)
	{
//      m_i2c_mem = m_i2cmem->read_sda() << 1;
		return m_i2cmem->read_sda() << 1;
	}
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_eeprom_nbajam_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x200000/2)
	{
		m_i2c_clk = BIT(data, 1);
		m_i2c_mem = BIT(data, 0);
		m_i2cmem->write_scl(m_i2c_clk);
		m_i2cmem->write_sda(m_i2c_mem);
	}
}

uint16_t md_eeprom_nbajamte_device::read(offs_t offset)
{
	if (offset == 0x200000/2)
	{
//      m_i2c_mem = m_i2cmem->read_sda();
		return m_i2cmem->read_sda();
	}
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_eeprom_nbajamte_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x200000/2)
	{
		if(ACCESSING_BITS_8_15)
		{
			m_i2c_clk = BIT(data, 8);
			m_i2cmem->write_scl(m_i2c_clk);
		}

		if(ACCESSING_BITS_0_7)
		{
			m_i2c_mem = BIT(data, 0);
			m_i2cmem->write_sda(m_i2c_mem);
		}
	}
}

// same as NBAJAMTE above... derived class?
uint16_t md_eeprom_cslam_device::read(offs_t offset)
{
	if (offset == 0x200000/2)
	{
//      m_i2c_mem = m_i2cmem->read_sda();
		return m_i2cmem->read_sda();
	}
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_eeprom_cslam_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x200000/2)
	{
		if(ACCESSING_BITS_8_15)
		{
			m_i2c_clk = BIT(data, 8);
			m_i2cmem->write_scl(m_i2c_clk);
		}

		if(ACCESSING_BITS_0_7)
		{
			m_i2c_mem = BIT(data, 0);
			m_i2cmem->write_sda(m_i2c_mem);
		}
	}
}

// same as NBAJAMTE above... derived class?
uint16_t md_eeprom_nflqb96_device::read(offs_t offset)
{
	if (offset == 0x200000/2)
	{
//      m_i2c_mem = i2cmem_sda_read(m_i2cmem);
		return m_i2cmem->read_sda();
	}
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_eeprom_nflqb96_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x200000/2)
	{
		if(ACCESSING_BITS_8_15)
		{
			m_i2c_clk = BIT(data, 8);
			m_i2cmem->write_scl(m_i2c_clk);
		}

		if(ACCESSING_BITS_0_7)
		{
			m_i2c_mem = BIT(data, 0);
			m_i2cmem->write_sda(m_i2c_mem);
		}
	}
}

uint16_t md_eeprom_nhlpa_device::read(offs_t offset)
{
	if (offset == 0x200000/2)
	{
//      m_i2c_mem = m_i2cmem->read_sda();
		return (m_i2cmem->read_sda() & 1) << 7;
	}
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_eeprom_nhlpa_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x200000/2)
	{
		m_i2c_clk = BIT(data, 6);
		m_i2c_mem = BIT(data, 7);
		m_i2cmem->write_scl(m_i2c_clk);
		m_i2cmem->write_sda(m_i2c_mem);
	}
}

uint16_t md_eeprom_blara_device::read(offs_t offset)
{
	if (offset == 0x380000/2)
	{
//      m_i2c_mem = m_i2cmem->read_sda();
		return (m_i2cmem->read_sda() & 1) << 7;
	}
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

void md_eeprom_blara_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x380000/2)
	{
		m_i2c_clk = BIT(data, 9);
		m_i2c_mem = BIT(data, 8);
		m_i2cmem->write_scl(m_i2c_clk);
		m_i2cmem->write_sda(m_i2c_mem);
	}
}

