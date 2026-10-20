// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Final Expansion v3 cartridge emulation

	The SD2IEC on-board is implemented in src/devices/bus/cbmiec/sd2iec.cpp

**********************************************************************/

#include "emu.h"
#include "fe3.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define AM29F040_TAG    "ic1"
#define ATF1504AS_TAG   "ic4"

// only 512 KB of flash/SRAM is connected, so bits 4-6 of the bank field are unused
#define REG1_BANK \
	(m_reg1 & 0x0f)

#define LORAM_HIDDEN \
	(m_reg2 & REG2_BLK0)

#define BLK1_HIDDEN \
	(m_reg2 & REG2_BLK1)

#define BLK2_HIDDEN \
	(m_reg2 & REG2_BLK2)

#define BLK3_HIDDEN \
	(m_reg2 & REG2_BLK3)

#define BLK5_HIDDEN \
	(m_reg2 & REG2_BLK5)

#define REGISTERS_HIDDEN \
	((m_lockbit && ((m_reg1 & REG1_MODE_MASK) == REG1_START)) || (m_reg2 & REG2_IO3))



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC20_FE3, vic20_final_expansion_3_device, "vic20_fe3", "Final Expansion v3")


//-------------------------------------------------
//  ROM( vic20_fe3 )
//-------------------------------------------------

ROM_START( vic20_fe3 )
	ROM_REGION( 0x80000, AM29F040_TAG, 0 )
	ROM_LOAD( "fe3r029.ic1", 0x00000, 0x80000, CRC(1895818b) SHA1(76aba501833fcd96eb237c0d7ba418124eadab28) )

	ROM_REGION( 0x10b6, ATF1504AS_TAG, 0 )
	ROM_LOAD( "vc20final-v3-2.ic4", 0x000, 0x10b6, CRC(975b7197) SHA1(e64d69870b757a409abeb5f19e34866eef37ab18) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *vic20_final_expansion_3_device::device_rom_region() const
{
	return ROM_NAME( vic20_fe3 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vic20_final_expansion_3_device::device_add_mconfig(machine_config &config)
{
	AMD_29F040(config, m_flash_rom);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_final_expansion_3_device - constructor
//-------------------------------------------------

vic20_final_expansion_3_device::vic20_final_expansion_3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIC20_FE3, tag, owner, clock),
	device_vic20_expansion_card_interface(mconfig, *this),
	m_flash_rom(*this, AM29F040_TAG),
	m_ram(*this, "sram", 0x80000, ENDIANNESS_LITTLE),
	m_reg1(0),
	m_reg2(0),
	m_lockbit(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic20_final_expansion_3_device::device_start()
{
	// state saving
	save_item(NAME(m_reg1));
	save_item(NAME(m_reg2));
	save_item(NAME(m_lockbit));

	m_slot->blk5()[0].install_readwrite_handler(0x0000, 0x1fff, read8sm_delegate(*this, FUNC(vic20_final_expansion_3_device::blk5_r)), write8sm_delegate(*this, FUNC(vic20_final_expansion_3_device::blk5_w)));
	m_slot->io3()[0].install_readwrite_handler(0x002, 0x003, 0x3fc, read8sm_delegate(*this, FUNC(vic20_final_expansion_3_device::read_register)), write8sm_delegate(*this, FUNC(vic20_final_expansion_3_device::write_register)));

	machine().save().register_postload(save_prepost_delegate(FUNC(vic20_final_expansion_3_device::update_map), this));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic20_final_expansion_3_device::device_reset()
{
	m_reg1 = 0;
	m_reg2 = 0;
	m_lockbit = 1;

	update_map();
}


//-------------------------------------------------
//  get_address -
//-------------------------------------------------

offs_t vic20_final_expansion_3_device::get_address(int bank, int block, offs_t offset)
{
	block ^= (m_reg2 >> 5) & 0x03;

	return bank << 15 | block << 13 | offset;
}


//-------------------------------------------------
//  read_register -
//-------------------------------------------------

uint8_t vic20_final_expansion_3_device::read_register(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_reg1;
		break;

	case 1:
		data = m_reg2;
		break;
	}

	return data;
}


//-------------------------------------------------
//  write_register -
//-------------------------------------------------

void vic20_final_expansion_3_device::write_register(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		m_reg1 = data;
		break;

	case 1:
		m_reg2 = data;
		break;
	}

	update_map();
}


//-------------------------------------------------
//  read_target -
//-------------------------------------------------

std::pair<int, int> vic20_final_expansion_3_device::read_target(int block) const
{
	bool const alt = BIT(m_reg1, block + 1);

	switch (m_reg1 & REG1_MODE_MASK)
	{
	case REG1_START:     return (block == 3) ? std::make_pair(TARGET_FLASH, 0) : std::make_pair(TARGET_NONE, 0);
	case REG1_SUPER_ROM: return std::make_pair(TARGET_FLASH, REG1_BANK);
	case REG1_RAM_1:     return std::make_pair(TARGET_RAM, 1);
	case REG1_RAM_2:     return std::make_pair(TARGET_RAM, alt ? 2 : 1);
	case REG1_SUPER_RAM: return std::make_pair(TARGET_RAM, REG1_BANK);
	case REG1_RAM_ROM:   return alt ? std::make_pair(TARGET_FLASH, 0) : std::make_pair(TARGET_RAM, 1);
	case REG1_FLASH:     return std::make_pair(TARGET_FLASH, REG1_BANK);
	default:             return std::make_pair(TARGET_NONE, 0);
	}
}


//-------------------------------------------------
//  write_target -
//-------------------------------------------------

std::pair<int, int> vic20_final_expansion_3_device::write_target(int block) const
{
	bool const alt = BIT(m_reg1, block + 1);

	switch (m_reg1 & REG1_MODE_MASK)
	{
	case REG1_START:     return std::make_pair(TARGET_RAM, 1);
	case REG1_SUPER_ROM: return std::make_pair(TARGET_RAM, 1);
	case REG1_RAM_1:     return std::make_pair(TARGET_RAM, alt ? 2 : 1);
	case REG1_RAM_2:     return std::make_pair(TARGET_RAM, 1);
	case REG1_SUPER_RAM: return std::make_pair(TARGET_RAM, REG1_BANK);
	case REG1_RAM_ROM:   return std::make_pair(TARGET_RAM, alt ? 2 : 1);
	case REG1_FLASH:     return std::make_pair(TARGET_FLASH, REG1_BANK);
	default:             return std::make_pair(TARGET_NONE, 0);
	}
}


//-------------------------------------------------
//  set_lockbit -
//-------------------------------------------------

void vic20_final_expansion_3_device::set_lockbit(int state)
{
	if (m_lockbit != state)
	{
		m_lockbit = state;

		update_io3();
	}
}


//-------------------------------------------------
//  update_map -
//-------------------------------------------------

void vic20_final_expansion_3_device::update_map()
{
	update_loram(m_slot->ram1(), 0x400);
	update_loram(m_slot->ram2(), 0x800);
	update_loram(m_slot->ram3(), 0xc00);
	update_block(m_slot->blk1(), 0);
	update_block(m_slot->blk2(), 1);
	update_block(m_slot->blk3(), 2);
	update_blk5();
	update_io3();
}


//-------------------------------------------------
//  update_loram -
//-------------------------------------------------

void vic20_final_expansion_3_device::update_loram(vic20_expansion_window &window, offs_t offset)
{
	uint8_t const mode = m_reg1 & REG1_MODE_MASK;

	if (LORAM_HIDDEN || (read_target(0).first == TARGET_NONE))
	{
		window.unmap();
	}
	else if (((mode == REG1_RAM_1) || (mode == REG1_RAM_2) || (mode == REG1_RAM_ROM)) && (m_reg1 & REG1_BLK0))
	{
		window[1].install_rom(0x000, 0x3ff, &m_ram[get_address(0, 0, offset)]);
		window.select(1);
	}
	else
	{
		window[0].install_ram(0x000, 0x3ff, &m_ram[get_address(0, 0, offset)]);
		window.select(0);
	}
}


//-------------------------------------------------
//  update_block -
//-------------------------------------------------

void vic20_final_expansion_3_device::update_block(vic20_expansion_window &window, int block)
{
	auto const [rtype, rbank] = read_target(block);
	auto const [wtype, wbank] = write_target(block);
	offs_t const rbase = get_address(rbank, block, 0);
	offs_t const wbase = get_address(wbank, block, 0);
	int const slot = (rtype == TARGET_NONE) ? 1 : 0;

	if (BIT(m_reg2, block + 1) || (wtype == TARGET_NONE))
	{
		window.unmap();
		return;
	}

	switch (rtype)
	{
	case TARGET_RAM:
		window[slot].install_rom(0x0000, 0x1fff, &m_ram[rbase]);
		break;

	case TARGET_FLASH:
		window[slot].install_read_handler(0x0000, 0x1fff, read8sm_delegate(*this, NAME(([this, rbase] (offs_t offset) { return m_flash_rom->read(rbase | offset); }))));
		break;
	}

	switch (wtype)
	{
	case TARGET_RAM:
		window[slot].install_writeonly(0x0000, 0x1fff, &m_ram[wbase]);
		break;

	case TARGET_FLASH:
		window[slot].install_write_handler(0x0000, 0x1fff, write8sm_delegate(*this, NAME(([this, wbase] (offs_t offset, uint8_t data) { m_flash_rom->write(wbase | offset, data); }))));
		break;
	}

	window.select(slot);
}


//-------------------------------------------------
//  update_blk5 -
//-------------------------------------------------

void vic20_final_expansion_3_device::update_blk5()
{
	if (BLK5_HIDDEN || (write_target(3).first == TARGET_NONE))
		m_slot->blk5().unmap();
	else
		m_slot->blk5().select(0);
}


//-------------------------------------------------
//  update_io3 -
//-------------------------------------------------

void vic20_final_expansion_3_device::update_io3()
{
	if (REGISTERS_HIDDEN || (write_target(0).first == TARGET_NONE))
		m_slot->io3().unmap();
	else
		m_slot->io3().select(0);
}


//-------------------------------------------------
//  blk5_r -
//-------------------------------------------------

uint8_t vic20_final_expansion_3_device::blk5_r(offs_t offset)
{
	auto const [type, bank] = read_target(3);
	uint8_t data = (type == TARGET_FLASH) ? m_flash_rom->read(get_address(bank, 3, offset)) : m_ram[get_address(bank, 3, offset)];

	if (!machine().side_effects_disabled())
		set_lockbit(1);

	return data;
}


//-------------------------------------------------
//  blk5_w -
//-------------------------------------------------

void vic20_final_expansion_3_device::blk5_w(offs_t offset, uint8_t data)
{
	auto const [type, bank] = write_target(3);

	if (type == TARGET_FLASH)
		m_flash_rom->write(get_address(bank, 3, offset), data);
	else
		m_ram[get_address(bank, 3, offset)] = data;

	set_lockbit(0);
}
