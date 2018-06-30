// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Fairchild Channel F cart emulation

 Three kind of carts:
 - ROM only (the vast majority of carts)
 - ROM + 2102 RAM chip (used by carts 10 and 18, with different I/O ports)
 - ROM + 3853 RAM chip (used by Schach + some homebrew)

 Based on Sean Riddle's documentation (especially for the 2102 RAM!)

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  chanf_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(CHANF_ROM_STD,         chanf_rom_device,         "chanf_rom",       "Channel F Standard Carts")
DEFINE_DEVICE_TYPE(CHANF_ROM_MAZE,        chanf_maze_device,        "chanf_maze",      "Channel F Maze Cart")
DEFINE_DEVICE_TYPE(CHANF_ROM_HANGMAN,     chanf_hangman_device,     "chanf_hang",      "Channel F Hangman Cart")
DEFINE_DEVICE_TYPE(CHANF_ROM_CHESS,       chanf_chess_device,       "chanf_chess",     "Channel F Chess Cart")
DEFINE_DEVICE_TYPE(CHANF_ROM_MULTI_OLD,   chanf_multi_old_device,   "chanf_multi_old", "Channel F Multigame (Earlier Version) Cart")
DEFINE_DEVICE_TYPE(CHANF_ROM_MULTI_FINAL, chanf_multi_final_device, "chanf_multi_fin", "Channel F Multigame (Final Version) Cart")


chanf_rom_device::chanf_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_channelf_cart_interface(mconfig, *this)
	, m_addr_latch(0), m_addr(0), m_read_write(0), m_data0(0)
{
}

chanf_rom_device::chanf_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: chanf_rom_device(mconfig, CHANF_ROM_STD, tag, owner, clock)
{
}

chanf_maze_device::chanf_maze_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: chanf_rom_device(mconfig, CHANF_ROM_MAZE, tag, owner, clock)
{
}

chanf_hangman_device::chanf_hangman_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: chanf_rom_device(mconfig, CHANF_ROM_HANGMAN, tag, owner, clock)
{
}

chanf_chess_device::chanf_chess_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: chanf_rom_device(mconfig, CHANF_ROM_CHESS, tag, owner, clock)
{
}

chanf_multi_old_device::chanf_multi_old_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: chanf_rom_device(mconfig, CHANF_ROM_MULTI_OLD, tag, owner, clock), m_base_bank(0)
{
}

chanf_multi_final_device::chanf_multi_final_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: chanf_rom_device(mconfig, CHANF_ROM_MULTI_FINAL, tag, owner, clock), m_base_bank(0), m_half_bank(0)
{
}



//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------


void chanf_maze_device::device_start()
{
	// 2012 RAM related
	save_item(NAME(m_latch));
	save_item(NAME(m_addr_latch));
	save_item(NAME(m_addr));
	save_item(NAME(m_read_write));
	save_item(NAME(m_data0));
}

void chanf_maze_device::device_reset()
{
	m_latch[0] = 0;
	m_latch[1] = 0;
	m_addr = 0;
	m_addr_latch = 0;
	m_read_write = 0;
	m_data0 = 0;
}


void chanf_hangman_device::device_start()
{
	// 2012 RAM related
	save_item(NAME(m_latch));
	save_item(NAME(m_addr_latch));
	save_item(NAME(m_addr));
	save_item(NAME(m_read_write));
	save_item(NAME(m_data0));
}

void chanf_hangman_device::device_reset()
{
	m_latch[0] = 0;
	m_latch[1] = 0;
	m_addr = 0;
	m_addr_latch = 0;
	m_read_write = 0;
	m_data0 = 0;
}


void chanf_multi_old_device::device_start()
{
	save_item(NAME(m_base_bank));
}

void chanf_multi_old_device::device_reset()
{
	m_base_bank = 0;
}


void chanf_multi_final_device::device_start()
{
	save_item(NAME(m_base_bank));
	save_item(NAME(m_half_bank));
}

void chanf_multi_final_device::device_reset()
{
	m_base_bank = 0;
	m_half_bank = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(chanf_rom_device::read_rom)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}

// These are shared among Maze & Hangman cart types (not directly used by base chanf_rom_device)
uint8_t chanf_rom_device::common_read_2102(uint32_t offset)
{
	if (offset == 0)
	{
		if (m_read_write == 0)
		{
			m_addr = m_addr_latch;
			m_data0 = m_ram[m_addr] & 1;
			return (m_latch[0] & 0x7f) | (m_data0 << 7);
		}

		return m_latch[0];
	}
	else
		return m_latch[1];
}

void chanf_rom_device::common_write_2102(uint32_t offset, uint8_t data)
{
	if (offset == 0)
	{
		m_latch[0] = data;

		m_read_write = BIT(data, 0);

		m_addr_latch = (m_addr_latch & 0x3f3) | (BIT(data, 2) << 2) | (BIT(data, 1) << 3);  // bits 2,3 come from this write!
		m_addr = m_addr_latch;

		m_data0 = BIT(data, 3);

		if (m_read_write == 1)
			m_ram[m_addr] = m_data0;
	}
	else
	{
		m_latch[1] = data;
		// all bits but 2,3 come from this write, but they are shuffled
		// notice that data is 8bits, so when swapping bit8 & bit9 are always 0!
		m_addr_latch = (m_addr_latch & 0x0c) | (bitswap<16>((uint16_t) data, 15, 14, 13, 12, 11, 10, 7, 6, 5, 3, 2, 1, 9, 8, 4, 0));
	}
}


// These are shared among Schach & Multigame cart types (not directly used by base chanf_rom_device)
uint8_t chanf_rom_device::common_read_3853(uint32_t offset)
{
	if (offset < m_ram.size())
		return m_ram[offset];
	else
		return 0xff;
}

void chanf_rom_device::common_write_3853(uint32_t offset, uint8_t data)
{
	if (offset < m_ram.size())
		m_ram[offset] = data;
}

READ8_MEMBER(chanf_multi_old_device::read_rom)
{
	if (offset < 0x2000)
		return m_rom[offset + m_base_bank * 0x2000];
	else
		return 0xff;
}

WRITE8_MEMBER(chanf_multi_old_device::write_bank)
{
	//printf("0x%x\n", data);
	m_base_bank = data & 0x1f;
}

READ8_MEMBER(chanf_multi_final_device::read_rom)
{
	if (offset < 0x2000)
		return m_rom[offset + (m_base_bank * 0x2000) + (m_half_bank * 0x1000)];
	else
		return 0xff;
}

WRITE8_MEMBER(chanf_multi_final_device::write_bank)
{
	//printf("0x%x\n", data);
	m_base_bank = data & 0x1f;
	m_half_bank = BIT(data, 5);
}
