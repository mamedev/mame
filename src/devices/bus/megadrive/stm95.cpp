// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, MetalliC
/***************************************************************************


 MegaDrive / Genesis Cart + STM95 EEPROM device


 TO DO: split STM95 to a separate device...

***************************************************************************/


#include "emu.h"
#include "stm95.h"


stm95_eeprom_device::stm95_eeprom_device(running_machine &machine, UINT8 *eeprom) :
			stm_state(IDLE),
			stream_pos(0),
			m_machine(machine)
{
	eeprom_data = eeprom;
	m_machine.save().save_item(latch, "STM95/latch");
	m_machine.save().save_item(reset_line, "STM95/reset_line");
	m_machine.save().save_item(sck_line, "STM95/sck_line");
	m_machine.save().save_item(WEL, "STM95/WEL");
	m_machine.save().save_item(stream_pos, "STM95/stream_pos");
	m_machine.save().save_item(stream_data, "STM95/stream_data");
	m_machine.save().save_item(eeprom_addr, "STM95/eeprom_addr");
}

void stm95_eeprom_device::set_cs_line(int state)
{
	reset_line = state;
	if (reset_line != CLEAR_LINE)
	{
		stream_pos = 0;
		stm_state = IDLE;
	}
}

void stm95_eeprom_device::set_si_line(int state)
{
	latch = state;
}

int stm95_eeprom_device::get_so_line(void)
{
	if (stm_state == READING || stm_state == CMD_RDSR)
		return (stream_data >> 8) & 1;
	else
		return 0;
}

void stm95_eeprom_device::set_sck_line(int state)
{
	if (reset_line == CLEAR_LINE)
	{
		if (state == ASSERT_LINE && sck_line == CLEAR_LINE)
		{
			switch (stm_state)
			{
				case IDLE:
					stream_data = (stream_data << 1) | (latch ? 1 : 0);
					stream_pos++;
					if (stream_pos == 8)
					{
						stream_pos = 0;
						//printf("STM95 EEPROM: got cmd %02X\n", stream_data&0xff);
						switch(stream_data & 0xff)
						{
							case 0x01:  // write status register
								if (WEL != 0)
									stm_state = CMD_WRSR;
								WEL = 0;
								break;
							case 0x02:  // write
								if (WEL != 0)
									stm_state = CMD_WRITE;
								stream_data = 0;
								WEL = 0;
								break;
							case 0x03:  // read
								stm_state = M95320_CMD_READ;
								stream_data = 0;
								break;
							case 0x04:  // write disable
								WEL = 0;
								break;
							case 0x05:  // read status register
								stm_state = CMD_RDSR;
								stream_data = WEL<<1;
								break;
							case 0x06:  // write enable
								WEL = 1;
								break;
							default:
								machine().logerror("STM95 EEPROM: unknown cmd %02X\n", stream_data&0xff);
						}
					}
					break;
				case CMD_WRSR:
					stream_pos++;       // just skip, don't care block protection
					if (stream_pos == 8)
					{
						stm_state = IDLE;
						stream_pos = 0;
					}
					break;
				case CMD_RDSR:
					stream_data = stream_data<<1;
					stream_pos++;
					if (stream_pos == 8)
					{
						stm_state = IDLE;
						stream_pos = 0;
					}
					break;
				case M95320_CMD_READ:
					stream_data = (stream_data << 1) | (latch ? 1 : 0);
					stream_pos++;
					if (stream_pos == 16)
					{
						eeprom_addr = stream_data & (M95320_SIZE - 1);
						stream_data = eeprom_data[eeprom_addr];
						stm_state = READING;
						stream_pos = 0;
					}
					break;
				case READING:
					stream_data = stream_data<<1;
					stream_pos++;
					if (stream_pos == 8)
					{
						if (++eeprom_addr == M95320_SIZE)
							eeprom_addr = 0;
						stream_data |= eeprom_data[eeprom_addr];
						stream_pos = 0;
					}
					break;
				case CMD_WRITE:
					stream_data = (stream_data << 1) | (latch ? 1 : 0);
					stream_pos++;
					if (stream_pos == 16)
					{
						eeprom_addr = stream_data & (M95320_SIZE - 1);
						stm_state = WRITING;
						stream_pos = 0;
					}
					break;
				case WRITING:
					stream_data = (stream_data << 1) | (latch ? 1 : 0);
					stream_pos++;
					if (stream_pos == 8)
					{
						eeprom_data[eeprom_addr] = stream_data;
						if (++eeprom_addr == M95320_SIZE)
							eeprom_addr = 0;
						stream_pos = 0;
					}
					break;
			}
		}
	}
	sck_line = state;
}



//-------------------------------------------------
//  md_rom_device - constructor
//-------------------------------------------------

const device_type MD_EEPROM_STM95 = &device_creator<md_eeprom_stm95_device>;


md_eeprom_stm95_device::md_eeprom_stm95_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
					device_md_cart_interface( mconfig, *this )
{
}

md_eeprom_stm95_device::md_eeprom_stm95_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, MD_EEPROM_STM95, "MD Cart + EEPROM STM95", tag, owner, clock, "md_eeprom_stm95", __FILE__),
					device_md_cart_interface( mconfig, *this )
{
}


void md_eeprom_stm95_device::device_start()
{
	nvram_alloc(M95320_SIZE);
	m_stm95.reset(global_alloc(stm95_eeprom_device(machine(), (UINT8*)get_nvram_base())));

	save_item(NAME(m_rdcnt));
	save_item(NAME(m_bank));
}

void md_eeprom_stm95_device::device_reset()
{
	m_rdcnt = 0;
	m_bank[0] = 0;
	m_bank[1] = 0;
	m_bank[2] = 0;
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ16_MEMBER(md_eeprom_stm95_device::read)
{
	if (offset == 0x0015e6/2 || offset == 0x0015e8/2)
	{
		// ugly hack until we don't know much about game protection
		// first 3 reads from 15e6 return 0x00000010, then normal 0x00018010 value for crc check
		UINT16 res;
		offset -= 0x0015e6/2;
		logerror("read 0x15e6 %d\n", m_rdcnt);
		if (m_rdcnt < 6)
		{
			m_rdcnt++;
			res = offset ? 0x10 : 0;
		}
		else
			res = offset ? 0x8010 : 0x0001;
		return res;
	}
	if (offset < 0x280000/2)
		return m_rom[offset];
	else    // last 0x180000 are bankswitched
	{
		UINT8 bank = (offset - 0x280000/2) >> 18;
		return m_rom[(offset & 0x7ffff/2) + (m_bank[bank] * 0x80000)/2];
	}
}

READ16_MEMBER(md_eeprom_stm95_device::read_a13)
{
	if (offset == 0x0a/2)
	{
		return m_stm95->get_so_line() & 1;
	}
	return 0xffff;
}

WRITE16_MEMBER(md_eeprom_stm95_device::write_a13)
{
	if (offset == 0x00/2)
	{
		logerror("A13001 write %02x\n", data);
	}
	else if (offset < 0x08/2)
	{
		m_bank[offset - 1] = data & 0x0f;
	}
	else if (offset < 0x0a/2)
	{
		m_stm95->set_si_line(BIT(data, 0));
		m_stm95->set_sck_line(BIT(data, 1));
		m_stm95->set_halt_line(BIT(data, 2));
		m_stm95->set_cs_line(BIT(data, 3));
	}
}
