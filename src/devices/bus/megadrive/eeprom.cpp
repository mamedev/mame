// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 MegaDrive / Genesis cart+EEPROM emulation


 TODO: proper EEPROM emulation, still not worked on (just hooked up the I2C device)


 i2c games mapping table:

 game name                         |   SDA_IN   |  SDA_OUT   |     SCL    |  SIZE_MASK     | PAGE_MASK |
 ----------------------------------|------------|------------|------------|----------------|-----------|
 NBA Jam                           | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x00ff (24C02) |   0x03    | xx
 NBA Jam TE                        | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x00ff (24C02) |   0x03    | xx
 NBA Jam TE (32x)                  | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x00ff (24C02) |   0x03    |
 NFL Quarterback Club              | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x00ff (24C02) |   0x03    | xx
 NFL Quarterback Club 96           | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x07ff (24C16) |   0x07    | xx
 College Slam                      | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x1fff (24C64) |   0x07    | xx
 Frank Thomas Big Hurt Baseball    | 0x200001-0 | 0x200001-0 | 0x200000-0 | 0x1fff (24C64) |   0x07    | xx
 NHLPA Hockey 93                   | 0x200001-7 | 0x200001-7 | 0x200001-6 | 0x007f (24C01) |   0x03    | xx
 Rings of Power                    | 0x200001-7 | 0x200001-7 | 0x200001-6 | 0x007f (24C01) |   0x03    | xx
 Evander Holyfield's Boxing        | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    | xx
 Greatest Heavyweights of the Ring | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    | xx
 Wonder Boy V                      | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    | xx
 Sports Talk Baseball              | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    | xx
 Megaman - the Wily Wars           | 0x200001-0 | 0x200001-0 | 0x200001-1 | 0x007f (24C01) |   0x03    | xx **
 Micro Machines 2                  | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x03ff (24C08) |   0x0f    |
 Micro Machines Military           | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x03ff (24C08) |   0x0f    |
 Micro Machines 96                 | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x07ff (24C16) |   0x0f    |
 Brian Lara Cricket 96             | 0x380001-7 | 0x300000-0*| 0x300000-1*| 0x1fff (24C64) |   0x??*   |
 ----------------------------------|------------|------------|------------|----------------|-----------|

 * Notes: check these
 ** original Rockman Mega World (J) set uses normal backup RAM


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
DEFINE_DEVICE_TYPE(MD_EEPROM_NFLQB,    md_eeprom_nflqb_device,    "md_eeprom_nflqb",    "MD NFL Quarterback 96")
DEFINE_DEVICE_TYPE(MD_EEPROM_CSLAM,    md_eeprom_cslam_device,    "md_eeprom_cslam",    "MD College Slam")
DEFINE_DEVICE_TYPE(MD_EEPROM_NHLPA,    md_eeprom_nhlpa_device,    "md_eeprom_nhlpa",    "MD NHLPA 93")
DEFINE_DEVICE_TYPE(MD_EEPROM_BLARA,    md_eeprom_blara_device,    "md_eeprom_blara",    "MD Brian Lara")


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

md_eeprom_nflqb_device::md_eeprom_nflqb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_eeprom_device(mconfig, MD_EEPROM_NFLQB, tag, owner, clock)
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


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void md_std_eeprom_device::device_add_mconfig(machine_config &config)
{
	I2C_24C01(config, m_i2cmem);
}

void md_eeprom_nbajam_device::device_add_mconfig(machine_config &config)
{
	I2C_24C02(config, m_i2cmem);
}

void md_eeprom_nbajamte_device::device_add_mconfig(machine_config &config)
{
	I2C_24C01(config, m_i2cmem);
}

void md_eeprom_cslam_device::device_add_mconfig(machine_config &config)
{
	I2C_24C64(config, m_i2cmem);
}

void md_eeprom_nflqb_device::device_add_mconfig(machine_config &config)
{
	I2C_24C16(config, m_i2cmem);
}

void md_eeprom_nhlpa_device::device_add_mconfig(machine_config &config)
{
	I2C_24C01(config, m_i2cmem);
}

void md_eeprom_blara_device::device_add_mconfig(machine_config &config)
{
	I2C_24C64(config, m_i2cmem);
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

READ16_MEMBER(md_std_eeprom_device::read)
{
	if (offset == 0x200000/2)
	{
		return m_i2cmem->read_sda();
	}
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

WRITE16_MEMBER(md_std_eeprom_device::write)
{
	if (offset == 0x200000/2)
	{
		m_i2c_clk = BIT(data, 1);
		m_i2c_mem = BIT(data, 0);
		m_i2cmem->write_scl(m_i2c_clk);
		m_i2cmem->write_sda(m_i2c_mem);
	}
}

READ16_MEMBER(md_eeprom_nbajam_device::read)
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

WRITE16_MEMBER(md_eeprom_nbajam_device::write)
{
	if (offset == 0x200000/2)
	{
		m_i2c_clk = BIT(data, 1);
		m_i2c_mem = BIT(data, 0);
		m_i2cmem->write_scl(m_i2c_clk);
		m_i2cmem->write_sda(m_i2c_mem);
	}
}

READ16_MEMBER(md_eeprom_nbajamte_device::read)
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

WRITE16_MEMBER(md_eeprom_nbajamte_device::write)
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
READ16_MEMBER(md_eeprom_cslam_device::read)
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

WRITE16_MEMBER(md_eeprom_cslam_device::write)
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
READ16_MEMBER(md_eeprom_nflqb_device::read)
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

WRITE16_MEMBER(md_eeprom_nflqb_device::write)
{
	if (offset == 0x200000/2)
	{
		m_i2c_clk = BIT(data, 8);
		m_i2c_mem = BIT(data, 0);
		m_i2cmem->write_scl(m_i2c_clk);
		m_i2cmem->write_sda(m_i2c_mem);
	}
}

READ16_MEMBER(md_eeprom_nhlpa_device::read)
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

WRITE16_MEMBER(md_eeprom_nhlpa_device::write)
{
	if (offset == 0x200000/2)
	{
		m_i2c_clk = BIT(data, 6);
		m_i2c_mem = BIT(data, 7);
		m_i2cmem->write_scl(m_i2c_clk);
		m_i2cmem->write_sda(m_i2c_mem);
	}
}

READ16_MEMBER(md_eeprom_blara_device::read)
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

WRITE16_MEMBER(md_eeprom_blara_device::write)
{
	if (offset == 0x380000/2)
	{
		m_i2c_clk = BIT(data, 9);
		m_i2c_mem = BIT(data, 8);
		m_i2cmem->write_scl(m_i2c_clk);
		m_i2cmem->write_sda(m_i2c_mem);
	}
}

// TEMPORARY ADDITION UNTIL WE FIND OUT WHAT IS MISSING IN THE CORE X24C02 CODE
// THIS IS A CUSTOM I2C EEPROM EMULATION THAT ALLOWS NBA JAM TO WORK

DEFINE_DEVICE_TYPE(MD_EEPROM_NBAJAM_ALT, md_eeprom_nbajam_alt_device, "md_eeprom_nbajama", "MD NBA Jam (Alt)")

md_eeprom_nbajam_alt_device::md_eeprom_nbajam_alt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_std_eeprom_device(mconfig, MD_EEPROM_NBAJAM_ALT, tag, owner, clock)
{
}

void md_eeprom_nbajam_alt_device::device_start()
{
	eeprom_i2c_init();
}


void md_eeprom_nbajam_alt_device::eeprom_i2c_init()
{
	m_eeprom_cnt = 0;
	m_eeprom_readwrite = 0;
	m_eeprom_slave_mask = 0;
	m_eeprom_word_address = 0;
	m_eeprom_devsel = 0;
	m_eeprom_byte = 0;

	m_eeprom_sda = m_eeprom_prev_sda = 1;
	m_eeprom_scl = m_eeprom_prev_scl = 1;
	m_eeprom_cur_state = STATE_I2C_IDLE;

	m_eeprom_mask = 0xff;
	m_eeprom_pagewrite_mask = 0x03;
}


// this is analogous to i2cmem.cpp implementation of write_sda
void md_eeprom_nbajam_alt_device::idle_devsel_check()
{
	if (m_eeprom_scl)
	{
		// All commands are preceded by the start condition, which is a HIGH to LOW
		// transition of SDA when SCL is HIGH
		if (m_eeprom_prev_sda && !m_eeprom_sda)
		{
			m_eeprom_cnt = 0;
			m_eeprom_slave_mask = 0;
			m_eeprom_cur_state = STATE_I2C_DEVSEL;
		}
		// All communications must be terminated by a stop condition, which is a LOW to HIGH
		// transition of SDA when SCL is HIGH
		else if (!m_eeprom_prev_sda && m_eeprom_sda)
		{
			m_eeprom_cur_state = STATE_I2C_IDLE;
		}
	}
}

void md_eeprom_nbajam_alt_device::eeprom_i2c_update(void)
{
	switch (m_eeprom_cur_state)
	{
		case STATE_I2C_IDLE:
			idle_devsel_check();
			break;

		case STATE_I2C_WAIT_STOP:
			if (!m_eeprom_prev_sda && m_eeprom_sda && m_eeprom_scl)
				m_eeprom_cur_state = STATE_I2C_IDLE;
			break;

		// device select: there can be up to 8 EEPROM in series, so that we start with writing 3 bits
		// to identify which device has to be accessed, followed by a Read/Write bit to specify the action
		case STATE_I2C_DEVSEL:
			idle_devsel_check();

			// LOW to HIGH transition of SCL = prepare to transmit, by moving to cnt = 1
			if (!m_eeprom_prev_scl && m_eeprom_scl)
			{
				if (m_eeprom_cnt <= 4)
				{
					// here we would transmit the Device Type Identifier which is 0101 for X24C02
					// but apparently the game does not check it, so let's skip it
				}
				else if ((m_eeprom_cnt > 4) && (m_eeprom_cnt < 8))
				{
					// here store the 3 bits of DEVICE ADDRESS
					m_eeprom_devsel = ((m_eeprom_devsel << 1) | m_eeprom_sda) & 0xff;
				}
				else if (m_eeprom_cnt == 8)
					m_eeprom_readwrite = m_eeprom_sda;
			}

			// HIGH to LOW transition of SCL = a new bit has been put on SDA line and we can read it
			// provided we already got the LOW to HIGH transition above
			if (m_eeprom_prev_scl && !m_eeprom_scl)
			{
				if (m_eeprom_cnt < 9)
					m_eeprom_cnt++;
				else
				{
					// ACK
					m_eeprom_cnt = 1;
					m_eeprom_cur_state = m_eeprom_readwrite ? STATE_I2C_READ_DATA : STATE_I2C_GET_WORD_ADDR;
				}
			}
			break;

		// read operation = count 8 operations and at 9th check whether ACK has been issued
		case STATE_I2C_READ_DATA:
			idle_devsel_check();

			// HIGH to LOW transition of SCL
			if (m_eeprom_prev_scl && !m_eeprom_scl)
			{
				if (m_eeprom_cnt < 9)
					m_eeprom_cnt++;
				else
					m_eeprom_cnt = 1;
			}

			// LOW to HIGH transition of SCL
			if (!m_eeprom_prev_scl && m_eeprom_scl)
			{
				if (m_eeprom_cnt == 9)
				{
					// no ACK
					if (m_eeprom_sda)
						m_eeprom_cur_state = STATE_I2C_WAIT_STOP;
				}
			}

			break;

		// For a write operation, the x24c02 requires a second address field. This address field is the
		// word address, comprised of eight bits, providing access to any one of the 256 words of memory.
		case STATE_I2C_GET_WORD_ADDR:
			idle_devsel_check();

			// HIGH to LOW transition of SCL
			if (m_eeprom_prev_scl && !m_eeprom_scl)
			{
				if (m_eeprom_cnt < 9)
					m_eeprom_cnt++;
				else
				{
					// ACK
					m_eeprom_cnt = 1;
					m_eeprom_cur_state = STATE_I2C_WRITE_DATA;
					m_eeprom_byte = 0;
					m_eeprom_word_address &= m_eeprom_mask;
				}
			}

			// LOW to HIGH transition of SCL
			if (!m_eeprom_prev_scl && m_eeprom_scl)
			{
				if (m_eeprom_cnt < 9)
					m_eeprom_word_address = ((m_eeprom_word_address << 1) | m_eeprom_sda) & 0xff;
			}
			break;

		// write operation
		case STATE_I2C_WRITE_DATA:
			idle_devsel_check();

			// HIGH to LOW transition of SCL
			if (m_eeprom_prev_scl && !m_eeprom_scl)
			{
				if (m_eeprom_cnt < 9)
					m_eeprom_cnt++;
				else
					m_eeprom_cnt = 1;
			}

			// LOW to HIGH transition of SCL
			if (!m_eeprom_prev_scl && m_eeprom_scl)
			{
				if (m_eeprom_cnt < 9)
					m_eeprom_byte = ((m_eeprom_byte << 1) | m_eeprom_sda) & 0xff;
				else
				{
					uint8_t *nvram = (uint8_t *)&m_nvram[0];
					uint16_t sram_address = m_eeprom_slave_mask | (m_eeprom_devsel * 0x100) | m_eeprom_word_address;
					nvram[sram_address & 0xffff] = m_eeprom_byte;
					m_eeprom_byte = 0;

					//printf("Write EEPROM : status %d addr %x data %x (count 8)\n", m_eeprom_cur_state, sram_address, nvram[sram_address]);
					// WORD ADDRESS++
					m_eeprom_word_address = (m_eeprom_word_address & ~m_eeprom_pagewrite_mask) |
											((m_eeprom_word_address + 1) & m_eeprom_pagewrite_mask);
				}
			}
			break;
	}

	m_eeprom_prev_scl = m_eeprom_scl;
	m_eeprom_prev_sda = m_eeprom_sda;
	//printf("Write line : status %d SDA %x SCL %x (count %d)\n", m_eeprom_cur_state, m_eeprom_sda, m_eeprom_scl, m_eeprom_cnt);
}

uint8_t md_eeprom_nbajam_alt_device::eeprom_i2c_out()
{
	uint8_t res = m_eeprom_sda;

	switch (m_eeprom_cur_state)
	{
		case STATE_I2C_READ_DATA:
			if (m_eeprom_cnt < 9)
			{
				uint8_t *nvram = (uint8_t *)&m_nvram[0];
				uint16_t sram_address = m_eeprom_slave_mask | (m_eeprom_devsel * 0x100) | m_eeprom_word_address;
				sram_address &= 0xffff;

				res = (nvram[sram_address] >> (8 - m_eeprom_cnt)) & 1;

				if (m_eeprom_cnt == 8)
				{
					//printf("Read EEPROM : status %d addr %x data %x (count 8)\n", m_eeprom_cur_state, sram_address, nvram[sram_address]);
					// WORD ADDRESS++
					m_eeprom_word_address++;
					m_eeprom_word_address &= m_eeprom_mask;
				}
			}
			break;
		case STATE_I2C_DEVSEL:
		case STATE_I2C_GET_WORD_ADDR:
		case STATE_I2C_WRITE_DATA:
			if (m_eeprom_cnt == 9)
				res = 0;
			break;

		default:
			break;
	}

	//printf("Read line : status %d data %x (count %d)\n", m_eeprom_cur_state, res, m_eeprom_cnt);
	return res;
}


READ16_MEMBER(md_eeprom_nbajam_alt_device::read)
{
	if (offset == 0x200000/2)
	{
		return eeprom_i2c_out() << 1;
	}
	if (offset < 0x400000/2)
		return m_rom[MD_ADDR(offset)];
	else
		return 0xffff;
}

WRITE16_MEMBER(md_eeprom_nbajam_alt_device::write)
{
	if (offset == 0x200000/2)
	{
		m_eeprom_sda = BIT(data, 0);
		m_eeprom_scl = BIT(data, 1);
		eeprom_i2c_update();
	}
}
