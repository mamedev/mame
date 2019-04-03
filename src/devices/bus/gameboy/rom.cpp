// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol
/***********************************************************************************************************

 Game Boy cart emulation


 Here we emulate carts with no RAM and simple bankswitch


 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  gb_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(GB_STD_ROM,    gb_rom_device,        "gb_rom",        "GB Carts")
DEFINE_DEVICE_TYPE(GB_ROM_TAMA5,  gb_rom_tama5_device,  "gb_rom_tama5",  "GB Tamagotchi")
DEFINE_DEVICE_TYPE(GB_ROM_WISDOM, gb_rom_wisdom_device, "gb_rom_wisdom", "GB Wisdom Tree Carts")
DEFINE_DEVICE_TYPE(GB_ROM_YONG,   gb_rom_yong_device,   "gb_rom_yong",   "GB Young Yong Carts")
DEFINE_DEVICE_TYPE(GB_ROM_ATVRAC, gb_rom_atvrac_device, "gb_rom_atvrac", "GB ATV Racin'")
DEFINE_DEVICE_TYPE(GB_ROM_LASAMA, gb_rom_lasama_device, "gb_rom_lasama", "GB LaSaMa")

DEFINE_DEVICE_TYPE(MEGADUCK_ROM,  megaduck_rom_device,  "megaduck_rom",  "MegaDuck Carts")


gb_rom_device::gb_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_gb_cart_interface(mconfig, *this)
{
}

gb_rom_device::gb_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gb_rom_device(mconfig, GB_STD_ROM, tag, owner, clock)
{
}

gb_rom_tama5_device::gb_rom_tama5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gb_rom_device(mconfig, GB_ROM_TAMA5, tag, owner, clock), m_tama5_data(0), m_tama5_addr(0), m_tama5_cmd(0), m_rtc_reg(0)
{
}

gb_rom_wisdom_device::gb_rom_wisdom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gb_rom_device(mconfig, GB_ROM_WISDOM, tag, owner, clock)
{
}

gb_rom_yong_device::gb_rom_yong_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gb_rom_device(mconfig, GB_ROM_YONG, tag, owner, clock)
{
}

gb_rom_atvrac_device::gb_rom_atvrac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gb_rom_device(mconfig, GB_ROM_ATVRAC, tag, owner, clock)
{
}

gb_rom_lasama_device::gb_rom_lasama_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gb_rom_device(mconfig, GB_ROM_LASAMA, tag, owner, clock)
{
}


megaduck_rom_device::megaduck_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_gb_cart_interface(mconfig, *this)
{
}

megaduck_rom_device::megaduck_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megaduck_rom_device(mconfig, MEGADUCK_ROM, tag, owner, clock)
{
}


//-------------------------------------------------
//  shared_start
//-------------------------------------------------

void gb_rom_device::shared_start()
{
	save_item(NAME(m_latch_bank));
	save_item(NAME(m_latch_bank2));
	save_item(NAME(m_ram_bank));
}

//-------------------------------------------------
//  shared_reset
//-------------------------------------------------

void gb_rom_device::shared_reset()
{
	m_ram_bank = 0;
	m_latch_bank = 0;
	m_latch_bank2 = 1;
}

//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void gb_rom_tama5_device::device_start()
{
	shared_start();
	save_item(NAME(m_tama5_data));
	save_item(NAME(m_tama5_addr));
	save_item(NAME(m_tama5_cmd));
	save_item(NAME(m_regs));
	save_item(NAME(m_rtc_reg));
}

void gb_rom_tama5_device::device_reset()
{
	shared_reset();
	m_tama5_data = 0;
	m_tama5_addr= 0;
	m_tama5_cmd = 0;
	memset(m_regs, 0xff, sizeof(m_regs));
	m_rtc_reg = 0xff;
}


// these are identical to shared ones above, but megaduck cart class is not derived from gb cart class...
void megaduck_rom_device::device_start()
{
	save_item(NAME(m_latch_bank));
	save_item(NAME(m_latch_bank2));
	save_item(NAME(m_ram_bank));
}

void megaduck_rom_device::device_reset()
{
	m_ram_bank = 0;
	m_latch_bank = 0;
	m_latch_bank2 = 1;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t gb_rom_device::read_rom(offs_t offset)
{
	m_latch_bank = offset / 0x4000;
	return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
}

uint8_t gb_rom_device::read_ram(offs_t offset)
{
	if (!m_ram.empty())
		return m_ram[ram_bank_map[m_ram_bank] * 0x2000 + offset];
	else
		return 0xff;
}

void gb_rom_device::write_ram(offs_t offset, uint8_t data)
{
	if (!m_ram.empty())
		m_ram[ram_bank_map[m_ram_bank] * 0x2000 + offset] = data;
}


// Tamagotchi

uint8_t gb_rom_tama5_device::read_rom(offs_t offset)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

uint8_t gb_rom_tama5_device::read_ram(offs_t offset)
{
	return m_rtc_reg;
}

void gb_rom_tama5_device::write_ram(offs_t offset, uint8_t data)
{
	switch (offset & 0x0001)
	{
		case 0x0000:    /* Write to data register */
			switch (m_tama5_cmd)
			{
			case 0x00:      /* Bits 0-3 for rom bank selection */
				m_latch_bank2 = (m_latch_bank2 & 0xf0) | (data & 0x0f);
				break;
			case 0x01:      /* Bit 4(-7?) for rom bank selection */
				m_latch_bank2 = (m_latch_bank2 & 0x0f) | ((data & 0x0f) << 4);
				break;
			case 0x04:      /* Data to write lo */
				m_tama5_data = (m_tama5_data & 0xf0) | (data & 0x0f);
				break;
			case 0x05:      /* Data to write hi */
				m_tama5_data = (m_tama5_data & 0x0f) | ((data & 0x0f) << 4);
				break;
			case 0x06:      /* Address selection hi */
				m_tama5_addr = (m_tama5_addr & 0x0f) | ((data & 0x0f) << 4);
				break;
			case 0x07:      /* Address selection lo */
				/* This address always seems to written last, so we'll just
				 execute the command here */
				m_tama5_addr = (m_tama5_addr & 0xf0) | (data & 0x0f);
				switch (m_tama5_addr & 0xe0)
				{
				case 0x00:      /* Write memory */
					//logerror( "Write tama5 memory 0x%02X <- 0x%02X\n", m_tama5_addr & 0x1f, m_tama5_data);
					m_regs[m_tama5_addr & 0x1f] = m_tama5_data;
					break;
				case 0x20:      /* Read memory */
					//logerror( "Read tama5 memory 0x%02X\n", m_tama5_addr & 0x1f);
					m_tama5_data = m_regs[m_tama5_addr & 0x1f];
					break;
				case 0x40:      /* Unknown, some kind of read */
					if ((m_tama5_addr & 0x1f) == 0x12)
						m_tama5_data = 0xff;
				case 0x80:      /* Unknown, some kind of read (when 07=01)/write (when 07=00/02) */
				default:
					logerror( "%s Unknown addressing mode\n", machine().describe_context() );
					break;
				}
				break;
			}
			break;
		case 0x0001:    /* Write to control register */
			switch (data)
			{
			case 0x00:      /* Bits 0-3 for rom bank selection */
			case 0x01:      /* Bits 4-7 for rom bank selection */
			case 0x04:      /* Data write register lo */
			case 0x05:      /* Data write register hi */
			case 0x06:      /* Address register hi */
			case 0x07:      /* Address register lo */
				break;
			case 0x0a:      /* Are we ready for the next command? */
				m_rtc_reg = 0x01;
				break;
			case 0x0c:      /* Data read register lo */
				m_rtc_reg = m_tama5_data & 0x0f;
				break;
			case 0x0d:      /* Data read register hi */
				m_rtc_reg = (m_tama5_data & 0xf0) >> 4;
				break;
			default:
				logerror( "%s Unknown tama5 command 0x%02X\n", machine().describe_context(), data );
				break;
			}
			m_tama5_cmd = data;
			break;
	}
}


// Wisdom Tree

uint8_t gb_rom_wisdom_device::read_rom(offs_t offset)
{
	return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + offset];
}

void gb_rom_wisdom_device::write_bank(offs_t offset, uint8_t data)
{
	if (offset < 0x4000)
		m_latch_bank = (offset << 1) & 0x1ff;
}


// Yong Yong pirate

uint8_t gb_rom_yong_device::read_rom(offs_t offset)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

void gb_rom_yong_device::write_bank(offs_t offset, uint8_t data)
{
	if (offset == 0x2000)
		m_latch_bank2 = data;
}


// ATV Racin pirate (incomplete)

uint8_t gb_rom_atvrac_device::read_rom(offs_t offset)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

void gb_rom_atvrac_device::write_bank(offs_t offset, uint8_t data)
{
	if (offset == 0x3f00)
	{
		if (data == 0)
			data = 1;
		m_latch_bank2 = m_latch_bank | data;
	}
	if (offset == 0x3fc0)
		m_latch_bank = data * 16;
}

// La Sa Ma pirate (incomplete)

uint8_t gb_rom_lasama_device::read_rom(offs_t offset)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

void gb_rom_lasama_device::write_bank(offs_t offset, uint8_t data)
{
	if (offset == 0x2080)
	{
		// Actual banking?
		m_latch_bank2 = m_latch_bank | (data & 0x03);
	}
	if (offset == 0x6000)
	{
		// On boot the following two get written right after each other:
		// 02
		// BE
		// Disable logo switching?
		if (!(data & 0x80))
			m_latch_bank = (data & 0x02) << 1;
	}
}


// MegaDuck carts

uint8_t megaduck_rom_device::read_rom(offs_t offset)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

void megaduck_rom_device::write_bank(offs_t offset, uint8_t data)
{
	if (offset == 0x0001)
		m_latch_bank2 = data;
}

void megaduck_rom_device::write_ram(offs_t offset, uint8_t data)
{
	m_latch_bank = data * 2;
	m_latch_bank2 = data * 2 + 1;
}
