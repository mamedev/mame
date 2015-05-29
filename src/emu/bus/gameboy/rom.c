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

const device_type GB_STD_ROM = &device_creator<gb_rom_device>;
const device_type GB_ROM_TAMA5 = &device_creator<gb_rom_tama5_device>;
const device_type GB_ROM_WISDOM = &device_creator<gb_rom_wisdom_device>;
const device_type GB_ROM_YONG = &device_creator<gb_rom_yong_device>;
const device_type GB_ROM_ATVRAC = &device_creator<gb_rom_atvrac_device>;
const device_type GB_ROM_LASAMA = &device_creator<gb_rom_lasama_device>;

const device_type MEGADUCK_ROM = &device_creator<megaduck_rom_device>;


gb_rom_device::gb_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_gb_cart_interface( mconfig, *this )
{
}

gb_rom_device::gb_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, GB_STD_ROM, "GB Carts", tag, owner, clock, "gb_rom", __FILE__),
						device_gb_cart_interface( mconfig, *this )
{
}

gb_rom_tama5_device::gb_rom_tama5_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gb_rom_device(mconfig, GB_ROM_TAMA5, "GB Tamagotchi", tag, owner, clock, "gb_rom_tama5", __FILE__)
{
}

gb_rom_wisdom_device::gb_rom_wisdom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gb_rom_device(mconfig, GB_ROM_WISDOM, "GB Wisdom Tree Carts", tag, owner, clock, "gb_rom_wisdom", __FILE__)
{
}

gb_rom_yong_device::gb_rom_yong_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gb_rom_device(mconfig, GB_ROM_YONG, "GB Yong Yong Carts", tag, owner, clock, "gb_rom_yong", __FILE__)
{
}

gb_rom_atvrac_device::gb_rom_atvrac_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gb_rom_device(mconfig, GB_ROM_ATVRAC, "GB ATV Racin'", tag, owner, clock, "gb_rom_atvrac", __FILE__)
{
}

gb_rom_lasama_device::gb_rom_lasama_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: gb_rom_device(mconfig, GB_ROM_LASAMA, "GB LaSaMa", tag, owner, clock, "gb_rom_lasama", __FILE__)
{
}


megaduck_rom_device::megaduck_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
					device_gb_cart_interface( mconfig, *this )
{
}

megaduck_rom_device::megaduck_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, MEGADUCK_ROM, "MegaDuck Carts", tag, owner, clock, "megaduck_rom", __FILE__),
					device_gb_cart_interface( mconfig, *this )
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

READ8_MEMBER(gb_rom_device::read_rom)
{
	m_latch_bank = offset / 0x4000;
	return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
}

READ8_MEMBER(gb_rom_device::read_ram)
{
	if (!m_ram.empty())
		return m_ram[ram_bank_map[m_ram_bank] * 0x2000 + offset];
	else
		return 0xff;
}

WRITE8_MEMBER(gb_rom_device::write_ram)
{
	if (!m_ram.empty())
		m_ram[ram_bank_map[m_ram_bank] * 0x2000 + offset] = data;
}


// Tamagotchi

READ8_MEMBER(gb_rom_tama5_device::read_rom)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

READ8_MEMBER(gb_rom_tama5_device::read_ram)
{
	return m_rtc_reg;
}

WRITE8_MEMBER(gb_rom_tama5_device::write_ram)
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
					logerror( "0x%04X: Unknown addressing mode\n", space.device() .safe_pc( ) );
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
				logerror( "0x%04X: Unknown tama5 command 0x%02X\n", space.device() .safe_pc( ), data );
				break;
			}
			m_tama5_cmd = data;
			break;
	}
}


// Wisdom Tree

READ8_MEMBER(gb_rom_wisdom_device::read_rom)
{
	return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + offset];
}

WRITE8_MEMBER(gb_rom_wisdom_device::write_bank)
{
	if (offset < 0x4000)
		m_latch_bank = (offset << 1) & 0x1ff;
}


// Yong Yong pirate

READ8_MEMBER(gb_rom_yong_device::read_rom)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(gb_rom_yong_device::write_bank)
{
	if (offset == 0x2000)
		m_latch_bank2 = data;
}


// ATV Racin pirate (incomplete)

READ8_MEMBER(gb_rom_atvrac_device::read_rom)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(gb_rom_atvrac_device::write_bank)
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

READ8_MEMBER(gb_rom_lasama_device::read_rom)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(gb_rom_lasama_device::write_bank)
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

READ8_MEMBER(megaduck_rom_device::read_rom)
{
	if (offset < 0x4000)
		return m_rom[rom_bank_map[m_latch_bank] * 0x4000 + (offset & 0x3fff)];
	else
		return m_rom[rom_bank_map[m_latch_bank2] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(megaduck_rom_device::write_bank)
{
	if (offset == 0x0001)
		m_latch_bank2 = data;
}

WRITE8_MEMBER(megaduck_rom_device::write_ram)
{
	m_latch_bank = data * 2;
	m_latch_bank2 = data * 2 + 1;
}
