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

const device_type CHANF_ROM_STD = &device_creator<chanf_rom_device>;
const device_type CHANF_ROM_MAZE = &device_creator<chanf_maze_device>;
const device_type CHANF_ROM_HANGMAN = &device_creator<chanf_hangman_device>;
const device_type CHANF_ROM_CHESS = &device_creator<chanf_chess_device>;
const device_type CHANF_ROM_MULTI_OLD = &device_creator<chanf_multi_old_device>;
const device_type CHANF_ROM_MULTI_FINAL = &device_creator<chanf_multi_final_device>;


chanf_rom_device::chanf_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_channelf_cart_interface( mconfig, *this ), m_addr_latch(0), m_addr(0), m_read_write(0), m_data0(0)
				{
}

chanf_rom_device::chanf_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, CHANF_ROM_STD, "Channel F Standard Carts", tag, owner, clock, "chanf_rom", __FILE__),
						device_channelf_cart_interface( mconfig, *this ), m_addr_latch(0), m_addr(0), m_read_write(0), m_data0(0)
				{
}

chanf_maze_device::chanf_maze_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: chanf_rom_device(mconfig, CHANF_ROM_MAZE, "Channel F Maze Cart", tag, owner, clock, "chanf_maze", __FILE__)
{
}

chanf_hangman_device::chanf_hangman_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: chanf_rom_device(mconfig, CHANF_ROM_HANGMAN, "Channel F Hangman Cart", tag, owner, clock, "chanf_hang", __FILE__)
{
}

chanf_chess_device::chanf_chess_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: chanf_rom_device(mconfig, CHANF_ROM_CHESS, "Channel F Chess Cart", tag, owner, clock, "chanf_chess", __FILE__)
{
}

chanf_multi_old_device::chanf_multi_old_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: chanf_rom_device(mconfig, CHANF_ROM_MULTI_OLD, "Channel F Multigame (Earlier Version) Cart", tag, owner, clock, "chanf_multi_old", __FILE__), m_base_bank(0)
				{
}

chanf_multi_final_device::chanf_multi_final_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: chanf_rom_device(mconfig, CHANF_ROM_MULTI_FINAL, "Channel F Multigame (Final Version) Cart", tag, owner, clock, "chanf_multi_fin", __FILE__), m_base_bank(0), m_half_bank(0)
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
UINT8 chanf_rom_device::common_read_2102(UINT32 offset)
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

void chanf_rom_device::common_write_2102(UINT32 offset, UINT8 data)
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
		m_addr_latch = (m_addr_latch & 0x0c) | (BITSWAP16((UINT16) data, 15, 14, 13, 12, 11, 10, 7, 6, 5, 3, 2, 1, 9, 8, 4, 0));
	}
}


// These are shared among Schach & Multigame cart types (not directly used by base chanf_rom_device)
UINT8 chanf_rom_device::common_read_3853(UINT32 offset)
{
	if (offset < m_ram.size())
		return m_ram[offset];
	else
		return 0xff;
}

void chanf_rom_device::common_write_3853(UINT32 offset, UINT8 data)
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
