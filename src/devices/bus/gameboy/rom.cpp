// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol
/***********************************************************************************************************

 Game Boy cart emulation


 Here we emulate carts with no RAM and simple bankswitch


 TAMA5 Mapper (Used by Tamagotchi 3)
 ============

 Status: partially supported.

 The TAMA5 mapper includes a special RTC chip which communicates through the
 RAM area (0xA000-0xBFFF); most notably addresses 0xA000 and 0xA001 seem to
 be used. In this setup 0xA001 acts like a control register and 0xA000 like
 a data register.

 Accepted values by the TAMA5 control register:
 0x00 - Writing to 0xA000 will set bits 3-0 for rom bank selection.
 0x01 - Writing to 0xA000 will set bits (7-?)4 for rom bank selection.

 0x04 - Bits 3-0 of the value to write
 0x05 - Bits 4-7 of the value to write
 0x06 - Address control hi
        bit 0 - Bit 4 for the address
        bit 3-1 - 000 - Write a byte to the 32 byte memory. The data to be
                        written must be set in registers 0x04 (lo nibble) and
                        0x05 (hi nibble).
                - 001 - Read a byte from the 32 byte memory. The data read
                        will be available in registers 0x0C (lo nibble) and
                        0x0D (hi nibble).
                - 010 - Unknown (occurs just after having started a game and
                        entered a date) (execution at address 1A19)
                - 011 - Unknown (not encountered yet)
                - 100 - Unknown (occurs during booting a game; appears to be
                        some kind of read command as it is followed by a read
                        of the 0x0C register) (execution at address 1B5B)
                - 101 - Unknown (not encountered yet)
                - 110 - Unknown (not encountered yet)
                - 111 - Unknown (not encountered yet)
 0x07 - Address control lo
        bit 3-0 - bits 3-0 for the address

 0x0A - After writing this the lowest 2 bits of A000 determine whether the
        TAMA5 chip is ready to accept the next command. If the lowest 2 bits
        hold the value 01 then the TAMA5 chip is ready for the next command.

 0x0C - Reading from A000 will return bits 3-0 of the data
 0x0D - Reading from A000 will return bits 7-4 of the data

 0x04 - RTC controls? -> RTC/memory?
 0x05 - Write time/memomry?
 0x06 - RTC controls?
 0x07 - RTC controls?

 Unknown sequences:
 During booting a game (1B5B:
 04 <- 00, 06 <- 08, 07 <- 01, followed by read 0C
 when value read from 0C equals 00 followed by the sequence:
 04 <- 01, 06 <- 08, 07 <- 01, followed by read 0C
 the value read from 0C is checked for non-zero, don't know the consequences for either
 yet.

 Initialization after starting a game:
 At address 1A19:
 06 <- 05, 07 <- 02, followed by read 0C, if != 0F => OK, otherwise do something.


 Wisdom Tree mapper
 ==================

 The Wisdom Tree mapper is triggered by writes in the 0x0000-0x3FFF area. The
 address written to determines the bank to switch in in the 0x000-0x7FFF address
 space. This mapper uses 32KB sized banks.


 TODO:
 - YongYong mapper:
   - During start there are 2 writes to 5000 and 5003, it is still unknown what these do.
 - Story of La Sa Ma mapper:
   - This should display the Gowin logo on boot on both DMG and CGB (Not implemented yet)
 - ATV Racing/Rocket Games mapper:
   - How did this overlay the official Nintendo logo at BIOS check time? (Some Sachen titles use a similar trick)

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"

namespace {

class gb_rom_device : public device_t, public device_gb_cart_interface
{
public:
	// construction/destruction
	gb_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	gb_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }

	void shared_start();
	void shared_reset();
};


class gb_rom_tama5_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_tama5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	uint16_t m_tama5_data, m_tama5_addr, m_tama5_cmd;
	uint8_t m_regs[32];
	uint8_t m_rtc_reg;
};


class gb_rom_wisdom_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_wisdom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }
};


class gb_rom_yong_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_yong_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }
};


class gb_rom_atvrac_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_atvrac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }
};


class gb_rom_lasama_device : public gb_rom_device
{
public:
	// construction/destruction
	gb_rom_lasama_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { shared_start(); }
	virtual void device_reset() override { shared_reset(); }
};



class megaduck_rom_device : public device_t, public device_gb_cart_interface
{
public:
	// construction/destruction
	megaduck_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(offs_t offset, uint8_t data) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;

protected:
	megaduck_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};


//-------------------------------------------------
//  gb_rom_device - constructor
//-------------------------------------------------

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
					[[fallthrough]];
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

} // anonymous namespace


// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(GB_STD_ROM,    device_gb_cart_interface, gb_rom_device,        "gb_rom",        "Game Boy Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_TAMA5,  device_gb_cart_interface, gb_rom_tama5_device,  "gb_rom_tama5",  "Game Boy Tamagotchi Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_WISDOM, device_gb_cart_interface, gb_rom_wisdom_device, "gb_rom_wisdom", "Game Boy Wisdom Tree Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_YONG,   device_gb_cart_interface, gb_rom_yong_device,   "gb_rom_yong",   "Game Boy Young Yong Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_ATVRAC, device_gb_cart_interface, gb_rom_atvrac_device, "gb_rom_atvrac", "Game Boy ATV Racin' Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_LASAMA, device_gb_cart_interface, gb_rom_lasama_device, "gb_rom_lasama", "Game Boy LaSaMa Cartridge")

DEFINE_DEVICE_TYPE_PRIVATE(MEGADUCK_ROM,  device_gb_cart_interface, megaduck_rom_device,  "megaduck_rom",  "MegaDuck Cartridge")
