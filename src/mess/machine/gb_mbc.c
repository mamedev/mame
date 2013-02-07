/***********************************************************************************************************
 
 Game Boy carts with MBC (Memory Bank Controller)
 
 
 TODO: add proper RTC and Rumble support

 ***********************************************************************************************************/


#include "emu.h"
#include "machine/gb_mbc.h"


//-------------------------------------------------
//  gb_rom_mbc*_device - constructor
//-------------------------------------------------

const device_type GB_ROM_MBC1 = &device_creator<gb_rom_mbc1_device>;
const device_type GB_ROM_MBC1K = &device_creator<gb_rom_mbc1k_device>;
const device_type GB_ROM_MBC2 = &device_creator<gb_rom_mbc2_device>;
const device_type GB_ROM_MBC3 = &device_creator<gb_rom_mbc3_device>;
const device_type GB_ROM_MBC5 = &device_creator<gb_rom_mbc5_device>;
const device_type GB_ROM_MBC6 = &device_creator<gb_rom_mbc6_device>;
const device_type GB_ROM_MBC7 = &device_creator<gb_rom_mbc7_device>;
const device_type GB_ROM_MMM01 = &device_creator<gb_rom_mmm01_device>;


gb_rom_mbc_device::gb_rom_mbc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, type, name, tag, owner, clock),
						device_gb_cart_interface( mconfig, *this )
{
}

gb_rom_mbc1_device::gb_rom_mbc1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gb_rom_mbc_device(mconfig, GB_ROM_MBC1, "GB MBC1 Carts", tag, owner, clock)
{
}

gb_rom_mbc1k_device::gb_rom_mbc1k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gb_rom_mbc_device(mconfig, GB_ROM_MBC1K, "GB MBC1 Korean Carts", tag, owner, clock)
{
}

gb_rom_mbc2_device::gb_rom_mbc2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gb_rom_mbc_device(mconfig, GB_ROM_MBC2, "GB MBC2 Carts", tag, owner, clock)
{
}

gb_rom_mbc3_device::gb_rom_mbc3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gb_rom_mbc_device(mconfig, GB_ROM_MBC3, "GB MBC3 Carts", tag, owner, clock)
{
}

gb_rom_mbc5_device::gb_rom_mbc5_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gb_rom_mbc_device(mconfig, GB_ROM_MBC5, "GB MBC5 Carts", tag, owner, clock)
{
}

gb_rom_mbc6_device::gb_rom_mbc6_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gb_rom_mbc_device(mconfig, GB_ROM_MBC6, "GB MBC6 Carts", tag, owner, clock)
{
}

gb_rom_mbc7_device::gb_rom_mbc7_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gb_rom_mbc_device(mconfig, GB_ROM_MBC7, "GB MBC7 Carts", tag, owner, clock)
{
}

gb_rom_mmm01_device::gb_rom_mmm01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gb_rom_mbc_device(mconfig, GB_ROM_MMM01, "GB MMM01 Carts", tag, owner, clock)
{
}


void gb_rom_mbc_device::device_start()
{
	has_timer = FALSE;
	has_rumble = FALSE;

	m_latch_bank = 0;
	m_latch_bank2 = 1;
	m_ram_enable = 0;
	m_ram_bank = 0;
	m_mode = 0;
	save_item(NAME(m_latch_bank));
	save_item(NAME(m_latch_bank2));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_ram_enable));
	save_item(NAME(m_mode));
}

void gb_rom_mbc1_device::device_start()
{
	has_timer = FALSE;
	has_rumble = FALSE;
	
	m_latch_bank = 0;
	m_latch_bank2 = 1;
	m_ram_bank = 0;
	m_ram_enable = 0;
	m_mode = 0;
	save_item(NAME(m_latch_bank));
	save_item(NAME(m_latch_bank2));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_ram_enable));
	save_item(NAME(m_mode));
}

void gb_rom_mbc1k_device::device_start()
{
	has_timer = FALSE;
	has_rumble = FALSE;
	
	m_latch_bank = 0;
	m_latch_bank2 = 1;
	m_ram_bank = 0;
	m_ram_enable = 0;
	m_mode = 0;
	save_item(NAME(m_latch_bank));
	save_item(NAME(m_latch_bank2));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_ram_enable));
	save_item(NAME(m_mode));
}

void gb_rom_mbc2_device::device_start()
{
	has_timer = FALSE;
	has_rumble = FALSE;
	
	m_latch_bank = 0;
	m_latch_bank2 = 1;
	m_ram_bank = 0;
	m_ram_enable = 0;
	m_mode = 0;
	save_item(NAME(m_latch_bank));
	save_item(NAME(m_latch_bank2));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_ram_enable));
	save_item(NAME(m_mode));
}

void gb_rom_mbc3_device::device_start()
{
	has_timer = FALSE;
	has_rumble = FALSE;

	m_latch_bank = 0;
	m_latch_bank2 = 1;
	m_ram_bank = 0;
	m_ram_enable = 0;
	m_mode = 0;
	memset(m_rtc_map, 0, sizeof(m_rtc_map));
	save_item(NAME(m_latch_bank));
	save_item(NAME(m_latch_bank2));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_ram_enable));
	save_item(NAME(m_mode));
	save_item(NAME(m_rtc_map));
}

void gb_rom_mbc5_device::device_start()
{
	has_timer = FALSE;
	has_rumble = FALSE;
	
	m_latch_bank = 0;
	m_latch_bank2 = 1;
	m_ram_bank = 0;
	m_ram_enable = 0;
	m_mode = 0;
	save_item(NAME(m_latch_bank));
	save_item(NAME(m_latch_bank2));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_ram_enable));
	save_item(NAME(m_mode));
}

void gb_rom_mbc6_device::device_start()
{
	has_timer = FALSE;
	has_rumble = FALSE;
	
	m_bank_4000 = 2;	// correct default?
	m_bank_6000 = 3;	// correct default?
	m_latch1 = 0;	// correct default?
	m_latch2 = 0;	// correct default?

	m_latch_bank = 2;	// correct default?
	m_latch_bank2 = 3;	// correct default?
	m_ram_bank = 0;
	m_ram_enable = 0;
	m_mode = 0;

	save_item(NAME(m_bank_4000));
	save_item(NAME(m_bank_6000));
	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
	save_item(NAME(m_latch_bank));
	save_item(NAME(m_latch_bank2));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_ram_enable));
	save_item(NAME(m_mode));
}

void gb_rom_mbc7_device::device_start()
{
	has_timer = FALSE;
	has_rumble = TRUE;
	
	m_latch_bank = 0;
	m_latch_bank2 = 1;
	m_ram_bank = 0;
	m_ram_enable = 0;
	save_item(NAME(m_latch_bank));
	save_item(NAME(m_latch_bank2));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_ram_enable));
}

void gb_rom_mmm01_device::device_start()
{
	has_timer = FALSE;
	has_rumble = TRUE;
	
	m_latch_bank = 0x200 - 2;
	m_latch_bank2 = 0x200 - 1;
	m_ram_bank = 0;
	m_bank_mask = 0xff;
	m_bank = 0;
	m_reg = 0;
	save_item(NAME(m_latch_bank));
	save_item(NAME(m_latch_bank2));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_bank_mask));
	save_item(NAME(m_bank));
	save_item(NAME(m_reg));
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(gb_rom_mbc_device::read_rom)
{
	return m_rom[rom_bank_map[m_latch_bank] + offset];
}

READ8_MEMBER(gb_rom_mbc_device::read_ram)
{
	if (m_ram)
		return m_ram[ram_bank_map[m_ram_bank] * 0x2000 + offset];
	else
		return 0xff;
}

WRITE8_MEMBER(gb_rom_mbc_device::write_ram)
{
	if (m_ram)
		m_ram[ram_bank_map[m_ram_bank] * 0x2000 + offset] = data;
}


// MBC1


READ8_MEMBER(gb_rom_mbc1_device::read_rom)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(gb_rom_mbc1_device::write_bank)
{
	if (offset < 0x2000)
		m_ram_enable = ((data & 0x0f) == 0x0a) ? 1 : 0;
	else if (offset < 0x4000)
	{
		// 5bits only
		data &= 0x1f;
		// bank = 0 => bank = 1
		if (data == 0)
			data = 1;
		
		m_latch_bank2 = (m_latch_bank2 & 0x01e0) | data;
	}
	else if (offset < 0x6000)
	{
		// 2bits only
		data &= 0x3;
		m_latch_bank2 = (m_latch_bank2 & 0x001f) | (data << 5);
	}
	else
		m_mode = data & 0x1;
}

READ8_MEMBER(gb_rom_mbc1_device::read_ram)
{
	if (m_ram && m_ram_enable)
	{
		m_ram_bank = m_mode ? (m_latch_bank2 >> 5) : 0;
		return m_ram[ram_bank_map[m_ram_bank] * 0x2000 + offset];
	}
	else
		return 0xff;
}

WRITE8_MEMBER(gb_rom_mbc1_device::write_ram)
{
	if (m_ram && m_ram_enable)
	{
		m_ram_bank = m_mode ? (m_latch_bank2 >> 5) : 0;
		m_ram[ram_bank_map[m_ram_bank] * 0x2000 + offset] = data;
	}
}


// MBC1 Korean variant (used by Bomberman Selection)

READ8_MEMBER(gb_rom_mbc1k_device::read_rom)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(gb_rom_mbc1k_device::write_bank)
{
	if (offset < 0x2000)
		m_ram_enable = ((data & 0x0f) == 0x0a) ? 1 : 0;
	else if (offset < 0x4000)
	{
		// 4bits only?
		data &= 0x0f;
		// bank = 0 => bank = 1
		if (data == 0)
			data = 1;
		
		m_latch_bank2 = (m_latch_bank2 & 0x01f0) | data;
	}
	else if (offset < 0x6000)
	{
		// 2bits only
		data &= 0x3;
		m_latch_bank2 = (m_latch_bank2 & 0x000f) | (data << 4);
		m_latch_bank = m_latch_bank2 & 0x30;
	}
	else
		m_mode = data & 0x1;
}

// RAM access is the same as usual MBC1
READ8_MEMBER(gb_rom_mbc1k_device::read_ram)
{
	if (m_ram && m_ram_enable)
	{
		m_ram_bank = m_mode ? (m_latch_bank2 >> 5) : 0;
		return m_ram[ram_bank_map[m_ram_bank] * 0x2000 + offset];
	}
	else
		return 0xff;
}

WRITE8_MEMBER(gb_rom_mbc1k_device::write_ram)
{
	if (m_ram && m_ram_enable)
	{
		m_ram_bank = m_mode ? (m_latch_bank2 >> 5) : 0;
		m_ram[ram_bank_map[m_ram_bank] * 0x2000 + offset] = data;
	}
}

// MBC2

READ8_MEMBER(gb_rom_mbc2_device::read_rom)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(gb_rom_mbc2_device::write_bank)
{
	if (offset < 0x2000)
		m_ram_enable = ((data & 0x0f) == 0x0a) ? 1 : 0;
	else if (offset < 0x4000)
	{		
		// 4bits only
		data &= 0x0f;
		// bank = 0 => bank = 1
		if (data == 0)
			data = 1;
		
		// The least significant bit of the upper address byte must be 1
		if (offset & 0x0100)
			m_latch_bank2 = (m_latch_bank2 & 0x100) | data;
	}
}

// 1 bank only??
READ8_MEMBER(gb_rom_mbc2_device::read_ram)
{
	if (m_ram && m_ram_enable)
		return m_ram[ram_bank_map[m_ram_bank] * 0x2000 + (offset & 0x1fff)];
	else
		return 0xff;
}

WRITE8_MEMBER(gb_rom_mbc2_device::write_ram)
{
	if (m_ram && m_ram_enable)
		m_ram[ram_bank_map[m_ram_bank] * 0x2000 + (offset & 0x1fff)] = data;
}


// MBC3

READ8_MEMBER(gb_rom_mbc3_device::read_rom)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(gb_rom_mbc3_device::write_bank)
{
	if (offset < 0x2000)
		m_ram_enable = ((data & 0x0f) == 0x0a) ? 1 : 0;
	else if (offset < 0x4000)
	{
		// 7bits
		data &= 0x7f;
		/* Selecting bank 0 == selecting bank 1 */
		if (data == 0)
			data = 1;
		
		m_latch_bank2 = data;
	}
	else if (offset < 0x6000)
	{
		m_ram_bank = data;
	}
	else
	{
		if (has_timer)
		{
			/* FIXME: RTC Latch goes here */
			m_rtc_map[0] = 50;    /* Seconds */
			m_rtc_map[1] = 40;    /* Minutes */
			m_rtc_map[2] = 15;    /* Hours */
			m_rtc_map[3] = 25;    /* Day counter lowest 8 bits */
			m_rtc_map[4] = 0x01;  /* Day counter upper bit, timer off, no day overflow occurred (bit7) */
		}
	}
}

READ8_MEMBER(gb_rom_mbc3_device::read_ram)
{
	if (m_ram_bank < 4 && m_ram_enable)
	{	// RAM
		if (m_ram)
			return m_ram[ram_bank_map[m_ram_bank] * 0x2000 + (offset & 0x1fff)];		
	}
	if (m_ram_bank >= 0x8 && m_ram_bank <= 0xc)
	{	// RAM
		if (has_timer)
			return m_rtc_map[m_ram_bank - 8];		
	}
	return 0xff;
}

WRITE8_MEMBER(gb_rom_mbc3_device::write_ram)
{
	if (m_ram_bank < 4 && m_ram_enable)
	{	// RAM
		if (m_ram)
			m_ram[ram_bank_map[m_ram_bank] * 0x2000 + (offset & 0x1fff)] = data;
	}
	if (m_ram_bank >= 0x8 && m_ram_bank <= 0xc)
	{	// RAM
		if (has_timer)
		{
		// what to do here?
		}
	}
}

// MBC5

READ8_MEMBER(gb_rom_mbc5_device::read_rom)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(gb_rom_mbc5_device::write_bank)
{
	if (offset < 0x2000)
		m_ram_enable = ((data & 0x0f) == 0x0a) ? 1 : 0;
	else if (offset < 0x4000)
	{
		// MBC5 has a 9 bit bank select
		// Writing into 2000-2fff sets the lower 8 bits
		// Writing into 3000-3fff sets the 9th bit
		if (offset & 0x1000)
			m_latch_bank2 = (m_latch_bank2 & 0xff) | ((data & 0x01) << 8);
		else
			m_latch_bank2 = (m_latch_bank2 & 0x100) | data;
	}
	else if (offset < 0x6000)
	{
		data &= 0x0f;
		if (has_rumble)
			data &= 0x7;
		m_ram_bank = data;
	}
}

READ8_MEMBER(gb_rom_mbc5_device::read_ram)
{
	if (m_ram && m_ram_enable)
		return m_ram[ram_bank_map[m_ram_bank] * 0x2000 + (offset & 0x1fff)];
	else
		return 0xff;
}

WRITE8_MEMBER(gb_rom_mbc5_device::write_ram)
{
	if (m_ram && m_ram_enable)
		m_ram[ram_bank_map[m_ram_bank] * 0x2000 + (offset & 0x1fff)] = data;
}

// MBC6

READ8_MEMBER(gb_rom_mbc6_device::read_rom)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else if (offset < 0x6000)
		return m_rom[rom_bank_map[m_bank_4000 >> 1] * 0x4000 + (m_bank_4000 & 0x01) * 0x2000 + (offset & 0x1fff)];
	else
		return m_rom[rom_bank_map[m_bank_6000 >> 1] * 0x4000 + (m_bank_6000 & 0x01) * 0x2000 + (offset & 0x1fff)];
}

WRITE8_MEMBER(gb_rom_mbc6_device::write_bank)
{
	if (offset < 0x2000)
	{
		logerror( "0x%04X: write to mbc6 ram enable area: %04X <- 0x%02X\n", space.device().safe_pc(), offset, data );
	}
	else if (offset < 0x3000)
	{
		if (!(offset & 0x0800))
			m_latch1 = data;
		else if (data == 0x00)
			m_bank_4000 = m_latch1;
	}
	else if (offset < 0x4000)
	{
		if (!(offset & 0x0800))
			m_latch2 = data;
		else if (data == 0x00)
			m_bank_6000 = m_latch2;
	}
}

READ8_MEMBER(gb_rom_mbc6_device::read_ram)
{
	if (m_ram)
		return m_ram[ram_bank_map[m_ram_bank] * 0x2000 + (offset & 0x1fff)];
	else
		return 0xff;
}

WRITE8_MEMBER(gb_rom_mbc6_device::write_ram)
{
	if (m_ram)
		m_ram[ram_bank_map[m_ram_bank] * 0x2000 + (offset & 0x1fff)] = data;
}

// MBC7

READ8_MEMBER(gb_rom_mbc7_device::read_rom)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(gb_rom_mbc7_device::write_bank)
{
	if (offset < 0x2000)
	{
		// FIXME: Add RAM enable support
		logerror("0x%04X: Write to ram enable register 0x%04X <- 0x%02X\n", space.device().safe_pc( ), offset, data);
	}
	else if (offset < 0x3000)
	{
		logerror( "0x%04X: write to mbc7 rom select register: 0x%04X <- 0x%02X\n", space.device() .safe_pc( ), 0x2000 + offset, data );
		/* Bit 12 must be set for writing to the mbc register */
		if (offset & 0x0100)
			m_latch_bank2 = data;
	}
	else
	{
		logerror( "0x%04X: write to mbc7 rom area: 0x%04X <- 0x%02X\n", space.device() .safe_pc( ), 0x3000 + offset, data );
		/* Bit 12 must be set for writing to the mbc register */
		if (offset & 0x0100)
		{
			switch (offset & 0x7000)
			{
				case 0x3000:    /* 0x3000-0x3fff */
				case 0x4000:    /* 0x4000-0x4fff */
				case 0x5000:    /* 0x5000-0x5fff */
				case 0x6000:    /* 0x6000-0x6fff */
				case 0x7000:    /* 0x7000-0x7fff */
					break;
			}
		}
	}
}

READ8_MEMBER(gb_rom_mbc7_device::read_ram)
{
	if (m_ram)
		return m_ram[ram_bank_map[m_ram_bank] * 0x2000 + (offset & 0x1fff)];
	else
		return 0xff;
}

WRITE8_MEMBER(gb_rom_mbc7_device::write_ram)
{
	if (m_ram)
		m_ram[ram_bank_map[m_ram_bank] * 0x2000 + (offset & 0x1fff)] = data;
}


// MMM01
// This mmm01 implementation is mostly guess work, no clue how correct it all is

READ8_MEMBER(gb_rom_mmm01_device::read_rom)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + offset];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(gb_rom_mmm01_device::write_bank)
{
	if (offset < 0x2000)
	{
		if (data & 0x40)
		{
			m_latch_bank = m_reg;
			m_latch_bank2 = m_latch_bank + m_bank;
		}
	}
	else if (offset < 0x4000)
	{
		m_reg = data & ((m_rom_size / 0x4000) - 1);
		m_bank = m_reg & m_bank_mask;
		if (m_bank == 0)
			m_bank = 1;
		m_latch_bank2 = m_latch_bank + m_bank;
	}
	else if (offset < 0x6000)
		logerror("0x%04X: write 0x%02X to 0x%04X\n", space.device().safe_pc(), data, offset);
	else
	{
		logerror( "0x%04X: write 0x%02X to 0x%04X\n", space.device().safe_pc(), data, offset);
		/* Not sure if this is correct, Taito Variety Pack sets these values */
		/* Momotarou Collection 2 writes 01 and 21 here */
		switch (data)
		{
			case 0x30:  m_bank_mask = 0x07;   break;
			case 0x38:  m_bank_mask = 0x03;   break;
			default:    m_bank_mask = 0xff; break;
		}
	}
}

