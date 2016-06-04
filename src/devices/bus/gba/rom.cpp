// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz,Fabio Priuli
/***********************************************************************************************************


 Game Boy Advance cart emulation


 We support carts with several kind of Save RAM (actual SRAM, Flash RAM or EEPROM)



 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  gba_rom_device - constructor
//-------------------------------------------------

const device_type GBA_ROM_STD = &device_creator<gba_rom_device>;
const device_type GBA_ROM_SRAM = &device_creator<gba_rom_sram_device>;
const device_type GBA_ROM_EEPROM = &device_creator<gba_rom_eeprom_device>;
const device_type GBA_ROM_EEPROM64 = &device_creator<gba_rom_eeprom64_device>;
const device_type GBA_ROM_FLASH = &device_creator<gba_rom_flash_device>;
const device_type GBA_ROM_FLASH1M = &device_creator<gba_rom_flash1m_device>;
const device_type GBA_ROM_3DMATRIX = &device_creator<gba_rom_3dmatrix_device>;


gba_rom_device::gba_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_gba_cart_interface( mconfig, *this )
{
}

gba_rom_device::gba_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, GBA_ROM_STD, "GBA Carts", tag, owner, clock, "gba_rom", __FILE__),
						device_gba_cart_interface( mconfig, *this )
{
}

gba_rom_sram_device::gba_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_device(mconfig, GBA_ROM_SRAM, "GBA Carts + SRAM", tag, owner, clock, "gba_sram", __FILE__)
{
}

gba_rom_eeprom_device::gba_rom_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_device(mconfig, GBA_ROM_EEPROM, "GBA Carts + EEPROM", tag, owner, clock, "gba_eeprom", __FILE__)
{
}

gba_rom_eeprom64_device::gba_rom_eeprom64_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_device(mconfig, GBA_ROM_EEPROM64, "GBA Carts + EEPROM 64K", tag, owner, clock, "gba_eeprom64", __FILE__)
{
}

gba_rom_flash_device::gba_rom_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_device(mconfig, GBA_ROM_FLASH, "GBA Carts + Panasonic Flash", tag, owner, clock, "gba_flash", __FILE__), m_flash_mask(0),
						m_flash(*this, "flash")
{
}

gba_rom_flash1m_device::gba_rom_flash1m_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_device(mconfig, GBA_ROM_FLASH1M, "GBA Carts + Sanyo Flash", tag, owner, clock, "gba_flash1m", __FILE__), m_flash_mask(0),
						m_flash(*this, "flash")
{
}

gba_rom_3dmatrix_device::gba_rom_3dmatrix_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gba_rom_device(mconfig, GBA_ROM_3DMATRIX, "GBA Carts + 3D Matrix Memory Mapper", tag, owner, clock, "gba_3dmatrix", __FILE__)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void gba_rom_device::device_start()
{
}

void gba_rom_device::device_reset()
{
}

void gba_rom_flash_device::device_reset()
{
	m_flash_mask = 0xffff/4;
}

void gba_rom_flash1m_device::device_reset()
{
	m_flash_mask = 0x1ffff/4;
}


void gba_rom_eeprom_device::device_start()
{
	// for the moment we use a custom eeprom implementation, so we alloc/save it as nvram
	nvram_alloc(0x200);
	m_eeprom = std::make_unique<gba_eeprom_device>(machine(), (UINT8*)get_nvram_base(), get_nvram_size(), 6);
}

void gba_rom_eeprom64_device::device_start()
{
	// for the moment we use a custom eeprom implementation, so we alloc/save it as nvram
	nvram_alloc(0x2000);
	m_eeprom = std::make_unique<gba_eeprom_device>(machine(), (UINT8*)get_nvram_base(), get_nvram_size(), 14);
}

void gba_rom_3dmatrix_device::device_start()
{
	save_item(NAME(m_src));
	save_item(NAME(m_dst));
	save_item(NAME(m_nblock));
}

void gba_rom_3dmatrix_device::device_reset()
{
	m_src = 0;
	m_dst = 0;
	m_nblock = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/


/*-------------------------------------------------
 Carts with SRAM
 -------------------------------------------------*/

READ32_MEMBER(gba_rom_sram_device::read_ram)
{
	if (!m_nvram.empty() && offset < m_nvram.size())
		return m_nvram[offset];
	else    // this cannot actually happen...
		return 0xffffffff;
}

WRITE32_MEMBER(gba_rom_sram_device::write_ram)
{
	if (!m_nvram.empty() && offset < m_nvram.size())
		COMBINE_DATA(&m_nvram[offset]);
}


/*-------------------------------------------------
 Carts with Flash RAM
 -------------------------------------------------*/

static MACHINE_CONFIG_FRAGMENT( panasonic_flash )
	MCFG_PANASONIC_MN63F805MNP_ADD("flash")
MACHINE_CONFIG_END

machine_config_constructor gba_rom_flash_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( panasonic_flash );
}


READ32_MEMBER(gba_rom_flash_device::read_ram)
{
	UINT32 rv = 0;

	offset &= m_flash_mask;

	if (mem_mask & 0xff)
		rv |= m_flash->read(offset * 4);
	if (mem_mask & 0xff00)
		rv |= m_flash->read((offset * 4) + 1) << 8;
	if (mem_mask & 0xff0000)
		rv |= m_flash->read((offset * 4) + 2) << 16;
	if (mem_mask & 0xff000000)
		rv |= m_flash->read((offset * 4) + 3) << 24;

	return rv;
}

WRITE32_MEMBER(gba_rom_flash_device::write_ram)
{
	offset &= m_flash_mask;

	switch (mem_mask)
	{
		case 0xff:
			m_flash->write(offset * 4, data & 0xff);
			break;
		case 0xff00:
			m_flash->write((offset * 4) + 1, (data >> 8) & 0xff);
			break;
		case 0xff0000:
			m_flash->write((offset * 4) + 2, (data >> 16) & 0xff);
			break;
		case 0xff000000:
			m_flash->write((offset * 4) + 3, (data >> 24) & 0xff);
			break;
		default:
			fatalerror("Unknown mem_mask for GBA flash write %x\n", mem_mask);
	}
}

static MACHINE_CONFIG_FRAGMENT( sanyo_flash )
	MCFG_SANYO_LE26FV10N1TS_ADD("flash")
MACHINE_CONFIG_END

machine_config_constructor gba_rom_flash1m_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sanyo_flash );
}


READ32_MEMBER(gba_rom_flash1m_device::read_ram)
{
	UINT32 rv = 0;

	offset &= m_flash_mask;

	if (mem_mask & 0xff)
		rv |= m_flash->read(offset * 4);
	if (mem_mask & 0xff00)
		rv |= m_flash->read((offset * 4) + 1) << 8;
	if (mem_mask & 0xff0000)
		rv |= m_flash->read((offset * 4) + 2) << 16;
	if (mem_mask & 0xff000000)
		rv |= m_flash->read((offset * 4) + 3) << 24;

	return rv;
}

WRITE32_MEMBER(gba_rom_flash1m_device::write_ram)
{
	offset &= m_flash_mask;

	switch (mem_mask)
	{
		case 0xff:
			m_flash->write(offset * 4, data & 0xff);
			break;
		case 0xff00:
			m_flash->write((offset * 4) + 1, (data >> 8) & 0xff);
			break;
		case 0xff0000:
			m_flash->write((offset * 4) + 2, (data >> 16) & 0xff);
			break;
		case 0xff000000:
			m_flash->write((offset * 4) + 3, (data >> 24) & 0xff);
			break;
		default:
			fatalerror("Unknown mem_mask for GBA flash write %x\n", mem_mask);
	}
}


/*-------------------------------------------------
 Carts with EEPROM

 TODO: can this sketchy EEPROM device be merged
 with the core implementation?
 -------------------------------------------------*/

// GBA EEPROM Device

gba_eeprom_device::gba_eeprom_device(running_machine &machine, UINT8 *eeprom, UINT32 size, int addr_bits) :
			m_state(EEP_IDLE),
			m_machine(machine)
{
	m_data = eeprom;
	m_data_size = size;
	m_addr_bits = addr_bits;

	m_machine.save().save_item(m_state, "GBA_EEPROM/m_state");
	m_machine.save().save_item(m_command, "GBA_EEPROM/m_command");
	m_machine.save().save_item(m_count, "GBA_EEPROM/m_count");
	m_machine.save().save_item(m_addr, "GBA_EEPROM/m_addr");
	m_machine.save().save_item(m_bits, "GBA_EEPROM/m_bits");
	m_machine.save().save_item(m_eep_data, "GBA_EEPROM/m_eep_data");
}

UINT32 gba_eeprom_device::read()
{
	UINT32 out;

	switch (m_state)
	{
		case EEP_IDLE:
//          printf("eeprom_r: @ %x, mask %08x (state %d) (PC=%x) = %d\n", offset, ~mem_mask, m_state, activecpu_get_pc(), 1);
			return 0x00010001;  // "ready"

		case EEP_READFIRST:
			m_count--;

			if (!m_count)
			{
				m_count = 64;
				m_bits = 0;
				m_eep_data = 0;
				m_state = EEP_READ;
			}
			break;
		case EEP_READ:
			if ((m_bits == 0) && (m_count))
			{
				if (m_addr >= m_data_size)
				{
					fatalerror("eeprom: invalid address (%x)\n", m_addr);
				}
				m_eep_data = m_data[m_addr];
				//printf("EEPROM read @ %x = %x (%x)\n", m_addr, m_eep_data, (m_eep_data & 0x80) ? 1 : 0);
				m_addr++;
				m_bits = 8;
			}

			out = (m_eep_data & 0x80) ? 1 : 0;
			out |= (out<<16);
			m_eep_data <<= 1;

			m_bits--;
			m_count--;

			if (!m_count)
			{
				m_state = EEP_IDLE;
			}

//          printf("out = %08x\n", out);
//          printf("eeprom_r: @ %x, mask %08x (state %d) (PC=%x) = %08x\n", offset, ~mem_mask, m_state, activecpu_get_pc(), out);
			return out;
	}
//  printf("eeprom_r: @ %x, mask %08x (state %d) (PC=%x) = %d\n", offset, ~mem_mask, m_state, space.device().safe_pc(), 0);
	return 0;
}

void gba_eeprom_device::write(UINT32 data)
{
//  printf("eeprom_w: %x @ %x (state %d) (PC=%x)\n", data, offset, m_state, space.device().safe_pc());
	switch (m_state)
	{
		case EEP_IDLE:
			if (data == 1)
				m_state++;
			break;

		case EEP_COMMAND:
			if (data == 1)
				m_command = EEP_READFIRST;
			else
				m_command = EEP_WRITE;
			m_state = EEP_ADDR;
			m_count = m_addr_bits;
			m_addr = 0;
			break;

		case EEP_ADDR:
			m_addr <<= 1;
			m_addr |= (data & 1);
			m_count--;
			if (!m_count)
			{
				m_addr *= 8; // each address points to 8 bytes
				if (m_command == EEP_READFIRST)
					m_state = EEP_AFTERADDR;
				else
				{
					m_count = 64;
					m_bits = 8;
					m_state = EEP_WRITE;
					m_eep_data = 0;
				}
			}
			break;

		case EEP_AFTERADDR:
			m_state = m_command;
			m_count = 64;
			m_bits = 0;
			m_eep_data = 0;
			if (m_state == EEP_READFIRST)
				m_count = 4;
			break;

		case EEP_WRITE:
			m_eep_data <<= 1;
			m_eep_data |= (data & 1);
			m_bits--;
			m_count--;

			if (m_bits == 0)
			{
				osd_printf_verbose("%08x: EEPROM: %02x to %x\n", machine().device("maincpu")->safe_pc(), m_eep_data, m_addr);
				if (m_addr >= m_data_size)
					fatalerror("eeprom: invalid address (%x)\n", m_addr);

				m_data[m_addr] = m_eep_data;
				m_addr++;
				m_eep_data = 0;
				m_bits = 8;
			}

			if (!m_count)
				m_state = EEP_AFTERWRITE;
			break;

		case EEP_AFTERWRITE:
			m_state = EEP_IDLE;
			break;
	}
}



READ32_MEMBER(gba_rom_eeprom_device::read_ram)
{
	// Larger games have smaller access to EERPOM content
	if (m_rom_size > (16 * 1024 * 1024) && offset < 0xffff00/4)
		return 0xffffffff;

	return m_eeprom->read();
}

WRITE32_MEMBER(gba_rom_eeprom_device::write_ram)
{
	// Larger games have smaller access to EEPROM content
	if (m_rom_size > (16 * 1024 * 1024) && offset < 0xffff00/4)
		return;

	if (~mem_mask == 0x0000ffff)
		data >>= 16;

	m_eeprom->write(data);
}

READ32_MEMBER(gba_rom_eeprom64_device::read_ram)
{
	// Larger games have smaller access to EERPOM content
	if (m_rom_size > (16 * 1024 * 1024) && offset < 0xffff00/4)
		return 0xffffffff;

	return m_eeprom->read();
}

WRITE32_MEMBER(gba_rom_eeprom64_device::write_ram)
{
	// Larger games have smaller access to EEPROM content
	if (m_rom_size > (16 * 1024 * 1024) && offset < 0xffff00/4)
		return;

	if (~mem_mask == 0x0000ffff)
		data >>= 16;

	m_eeprom->write(data);
}


/*-------------------------------------------------
 Carts with 3D Matrix Memory controller
 
 Used by Video carts with 64MB ROM chips
 Emulation based on the reverse engineering efforts
 by endrift
 
 The Memory controller basically behaves like a DMA
 chip by writing first source and destination address,
 then the number of 512K blocks to copy and finally
 by issuing the transfer command.
 Disney Collection 2 carts uses command 0x01 to start
 the transfer, other carts might use 0x11 but currently
 they die before getting to the mapper communication
 (CPU emulation issue? cart mapping issue? still unknown)
 
 To investigate:
 - why the other carts fail
 - which addresses might be used by the mapper
   (Disney Collection 2 uses 0x08800180-0x0880018f
   but it might well be possible to issue commands 
   in an extended range...)
 - which bus addresses can be used by the mapper
   (currently we restrict the mapping in the range
   0x08000000-0x09ffffff but maybe also the rest of
   the cart "range" is accessible...)
 -------------------------------------------------*/

WRITE32_MEMBER(gba_rom_3dmatrix_device::write_mapper)
{
	//printf("mapper write 0x%.8X - 0x%X\n", offset, data);
	switch (offset & 3)
	{
		case 0:
			if (data == 0x1)	// transfer data
				memcpy((UINT8 *)m_romhlp + m_dst, (UINT8 *)m_rom + m_src, m_nblock * 0x200);
			else
				printf("Unknown mapper command 0x%X\n", data);
			break;
		case 1:
			m_src = data;
			break;
		case 2:
			if (data >= 0xa000000)
				printf("Unknown transfer destination 0x%X\n", data);
			m_dst = (data & 0x1ffffff);
			break;
		case 3:
		default:
			m_nblock = data;
			break;
	}
}
