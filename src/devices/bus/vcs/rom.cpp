// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 A2600 VCS ROM cart emulation
 Simple cart hardware with no additional hw

 Mapper implementation based on the wonderful docs by Kevtris
 http://blog.kevtris.org/blogfiles/Atari%202600%20Mappers.txt

 (also inspired by previous work by Wilbert Pol et al.)

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  a26_rom_*k_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(A26_ROM_2K_4K, a26_rom_2k_4k_device, "vcs_2k_4k", "Atari VCS 2600 2K/4K ROM Carts")
DEFINE_DEVICE_TYPE(A26_ROM_F4,    a26_rom_f4_device,    "vcs_f4",    "Atari VCS 2600 ROM Carts w/F4 bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_F6,    a26_rom_f6_device,    "vcs_f6",    "Atari VCS 2600 ROM Carts w/F6 bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_F8,    a26_rom_f8_device,    "vcs_f8",    "Atari VCS 2600 ROM Carts w/F8 bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_F8_SW, a26_rom_f8_sw_device, "vcs_f8_sw", "Atari VCS 2600 ROM Cart Snow White")
DEFINE_DEVICE_TYPE(A26_ROM_FA,    a26_rom_fa_device,    "vcs_fa",    "Atari VCS 2600 ROM Carts w/FA bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_FE,    a26_rom_fe_device,    "vcs_fe",    "Atari VCS 2600 ROM Carts w/FE bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_3E,    a26_rom_3e_device,    "vcs_3e",    "Atari VCS 2600 ROM Carts w/3E bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_3F,    a26_rom_3f_device,    "vcs_3f",    "Atari VCS 2600 ROM Carts w/3F bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_E0,    a26_rom_e0_device,    "vcs_e0",    "Atari VCS 2600 ROM Carts w/E0 bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_E7,    a26_rom_e7_device,    "vcs_e7",    "Atari VCS 2600 ROM Carts w/E7 bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_UA,    a26_rom_ua_device,    "vcs_ua",    "Atari VCS 2600 ROM Carts w/UA bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_CV,    a26_rom_cv_device,    "vcs_cv",    "Atari VCS 2600 ROM Carts w/Commavid bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_DC,    a26_rom_dc_device,    "vcs_dc",    "Atari VCS 2600 ROM Carts w/Dynacom bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_FV,    a26_rom_fv_device,    "vcs_fv",    "Atari VCS 2600 ROM Carts w/FV bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_JVP,   a26_rom_jvp_device,   "vcs_jvp",   "Atari VCS 2600 ROM Carts w/JVP bankswitch")
DEFINE_DEVICE_TYPE(A26_ROM_4IN1,  a26_rom_4in1_device,  "vcs_4in1",  "Atari VCS 2600 ROM Cart 4 in 1")
DEFINE_DEVICE_TYPE(A26_ROM_8IN1,  a26_rom_8in1_device,  "vcs_8in1",  "Atari VCS 2600 ROM Cart 8 in 1")
DEFINE_DEVICE_TYPE(A26_ROM_32IN1, a26_rom_32in1_device, "vcs_32in1", "Atari VCS 2600 ROM Cart 32 in 1")
DEFINE_DEVICE_TYPE(A26_ROM_X07,   a26_rom_x07_device,   "vcs_x07",   "Atari VCS 2600 ROM Carts w/X07 bankswitch")



/*-------------------------------------------------
 BASE 2K & 4K Carts:
 no bankswitch

 GAMES: a large majority
 -------------------------------------------------*/

a26_rom_2k_4k_device::a26_rom_2k_4k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_2K_4K, tag, owner, clock)
{
}

void a26_rom_2k_4k_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_2k_4k_device::read)));
}

uint8_t a26_rom_2k_4k_device::read(offs_t offset)
{
	return m_rom[offset & (m_rom_size - 1)];
}



/*-------------------------------------------------
 "F6 Bankswitch" Carts:
 read/write access to 0x1ff6-0x1ff9 determines the
 4K ROM bank to be read

 GAMES: Atari 16K games, like Crossbow, Crystal
 Castles and the 2-in-1 carts

 -------------------------------------------------*/

a26_rom_f6_device::a26_rom_f6_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, type, tag, owner, clock)
	, m_base_bank(0xff)
{
}

a26_rom_f6_device::a26_rom_f6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_F6, tag, owner, clock)
{
}

void a26_rom_f6_device::device_start()
{
	save_item(NAME(m_base_bank));
}

void a26_rom_f6_device::device_reset()
{
	m_base_bank = 0;
}

void a26_rom_f6_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_f6_device::read)));
	space->install_write_handler(0x1000, 0x1fff, write8sm_delegate(*this, FUNC(a26_rom_f6_device::write)));
}

uint8_t a26_rom_f6_device::read(offs_t offset)
{
	// Super Chip RAM reads are mapped in 0x1080-0x10ff
	if (!m_ram.empty() && offset >= 0x80 && offset < 0x100)
	{
		return m_ram[offset & (m_ram.size() - 1)];
	}

	// update banks
	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
			case 0x0ff6:
			case 0x0ff7:
			case 0x0ff8:
			case 0x0ff9:
				m_base_bank = offset - 0x0ff6;
				break;
		}
	}

	return m_rom[offset + (m_base_bank * 0x1000)];
}

void a26_rom_f6_device::write(offs_t offset, uint8_t data)
{
	// Super Chip RAM writes are mapped in 0x1000-0x107f
	if (!m_ram.empty() && offset < 0x80)
	{
		m_ram[offset & (m_ram.size() - 1)] = data;
		return;
	}

	switch (offset)
	{
		case 0x0ff6:
		case 0x0ff7:
		case 0x0ff8:
		case 0x0ff9:
			m_base_bank = offset - 0x0ff6;
			break;
		default:
			logerror("Write Bank outside expected range (0x%X).\n", offset + 0x1000);
			break;
	}
}



/*-------------------------------------------------
 "F4 Bankswitch" Carts:
 read/write access to 0x1ff4-0x1ffb determines the
 4K ROM bank to be read

 GAMES: Fatal Run
 -------------------------------------------------*/

a26_rom_f4_device::a26_rom_f4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_F4, tag, owner, clock)
{
}

void a26_rom_f4_device::device_reset()
{
	m_base_bank = 7;
}

void a26_rom_f4_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_f4_device::read)));
	space->install_write_handler(0x1000, 0x1fff, write8sm_delegate(*this, FUNC(a26_rom_f4_device::write)));
}

uint8_t a26_rom_f4_device::read(offs_t offset)
{
	// Super Chip RAM reads are mapped in 0x1080-0x10ff
	if (!m_ram.empty() && offset >= 0x80 && offset < 0x100)
	{
		return m_ram[offset & (m_ram.size() - 1)];
	}

	// update banks
	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
			case 0x0ff4:
			case 0x0ff5:
			case 0x0ff6:
			case 0x0ff7:
			case 0x0ff8:
			case 0x0ff9:
			case 0x0ffa:
			case 0x0ffb:
				m_base_bank = offset - 0x0ff4;
				break;
		}
	}

	return m_rom[offset + (m_base_bank * 0x1000)];
}

void a26_rom_f4_device::write(offs_t offset, uint8_t data)
{
	// Super Chip RAM writes are mapped in 0x1000-0x107f
	if (!m_ram.empty() && offset < 0x80)
	{
		m_ram[offset & (m_ram.size() - 1)] = data;
		return;
	}

	switch (offset)
	{
		case 0x0ff4:
		case 0x0ff5:
		case 0x0ff6:
		case 0x0ff7:
		case 0x0ff8:
		case 0x0ff9:
		case 0x0ffa:
		case 0x0ffb:
			m_base_bank = offset - 0x0ff4;
			break;
		default:
			logerror("Write Bank outside expected range (0x%X).\n", offset + 0x1000);
			break;
	}
}


/*-------------------------------------------------
 "F8 Bankswitch" Carts:
 read/write access to 0x1ff8-0x1ff9 determines the
 4K ROM bank to be read

 GAMES: Atari 8K games, like Asteroids, Battlezone
 and Taz

 -------------------------------------------------*/

a26_rom_f8_device::a26_rom_f8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, type, tag, owner, clock)
{
}

a26_rom_f8_device::a26_rom_f8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f8_device(mconfig, A26_ROM_F8, tag, owner, clock)
{
}

void a26_rom_f8_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_f8_device::read)));
	space->install_write_handler(0x1000, 0x1fff, write8sm_delegate(*this, FUNC(a26_rom_f8_device::write)));
}

uint8_t a26_rom_f8_device::read(offs_t offset)
{
	// Super Chip RAM reads are mapped in 0x1080-0x10ff
	if (!m_ram.empty() && offset >= 0x80 && offset < 0x100)
	{
		return m_ram[offset & (m_ram.size() - 1)];
	}

	// update banks
	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
			case 0x0ff8:
			case 0x0ff9:
				m_base_bank = offset - 0x0ff8;
				break;
		}
	}

	return m_rom[offset + (m_base_bank * 0x1000)];
}

void a26_rom_f8_device::write(offs_t offset, uint8_t data)
{
	// Super Chip RAM writes are mapped in 0x1000-0x107f
	if (!m_ram.empty() && offset < 0x80)
	{
		m_ram[offset & (m_ram.size() - 1)] = data;
		return;
	}

	switch (offset)
	{
		case 0x0ff8:
		case 0x0ff9:
			m_base_bank = offset - 0x0ff8;
			break;
		default:
			logerror("Write Bank outside expected range (0x%X).\n", offset + 0x1000);
			break;
	}
}


a26_rom_f8_sw_device::a26_rom_f8_sw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f8_device(mconfig, A26_ROM_F8_SW, tag, owner, clock)
{
}

void a26_rom_f8_sw_device::device_reset()
{
	// Snow White proto starts from bank 1!!!
	m_base_bank = 1;
}




/*-------------------------------------------------
 "FA Bankswitch" Carts:
 read/write access to 0x1ff8-0x1ffa determines the
 4K ROM bank to be read
 These games contained the CBS RAM+ chip (256bytes
 of RAM)

 GAMES: CBS RAM Plus games like Omega Race and Tunnel
 Runner

 -------------------------------------------------*/

a26_rom_fa_device::a26_rom_fa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_FA, tag, owner, clock)
{
}

void a26_rom_fa_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_fa_device::read)));
	space->install_write_handler(0x1000, 0x1fff, write8sm_delegate(*this, FUNC(a26_rom_fa_device::write)));
}

uint8_t a26_rom_fa_device::read(offs_t offset)
{
	// CBS RAM+ reads are mapped in 0x1100-0x11ff
	if (!m_ram.empty() && offset >= 0x100 && offset < 0x200)
	{
		return m_ram[offset & (m_ram.size() - 1)];
	}

	// update banks
	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
			case 0x0ff8:
			case 0x0ff9:
			case 0x0ffa:
				m_base_bank = offset - 0x0ff8;
				break;
		}
	}

	return m_rom[offset + (m_base_bank * 0x1000)];
}

void a26_rom_fa_device::write(offs_t offset, uint8_t data)
{
	// CBS RAM+ writes are mapped in 0x1000-0x10ff
	if (!m_ram.empty() && offset < 0x100)
	{
		m_ram[offset & (m_ram.size() - 1)] = data;
		return;
	}

	switch (offset)
	{
		case 0x0ff8:
		case 0x0ff9:
		case 0x0ffa:
			m_base_bank = offset - 0x0ff8;
			break;
		default:
			logerror("Write Bank outside expected range (0x%X).\n", offset + 0x1000);
			break;
	}
}



/*-------------------------------------------------
 "FE Bankswitch" Carts:
 read/write access to 0x01fe-0x1ff determines the
 4K ROM bank to be read

 GAMES: Activision 8K games like Decathlon

 -------------------------------------------------*/

a26_rom_fe_device::a26_rom_fe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_FE, tag, owner, clock)
	, m_base_bank(0)
	, m_trigger_on_next_access(0)
{
}

void a26_rom_fe_device::device_start()
{
	save_item(NAME(m_base_bank));
	save_item(NAME(m_trigger_on_next_access));
}

void a26_rom_fe_device::device_reset()
{
	m_base_bank = 0;
	m_trigger_on_next_access = false;
	// During cpu startup, before reading the boot vector a read from $01fe occurs
	m_ignore_first_read = true;
}

void a26_rom_fe_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_fe_device::read)));
	space->install_write_handler(0x1000, 0x1fff, write8sm_delegate(*this, FUNC(a26_rom_fe_device::write)));
	space->install_readwrite_tap(0x1fe, 0x1fe, "trigger_bank",
			[this] (offs_t, u8 &, u8) { if (!machine().side_effects_disabled()) trigger_bank(); },
			[this] (offs_t, u8 &, u8) { if (!machine().side_effects_disabled()) trigger_bank(); });
	space->install_read_tap(0x1ff, 0x1ff, "bank",
			[this] (offs_t, u8 &data, u8) { if (!machine().side_effects_disabled()) switch_bank(data); });
}

/*

 There seems to be a kind of lag between the writing to address 0x1FE and the
 Activision switcher springing into action. It waits for the next byte to arrive
 on the data bus, which is the new PCH in the case of a JSR, and the PCH of the
 stored PC on the stack in the case of an RTS.

 depending on last byte & 0x20 -> 0x00 -> switch to bank #1
 -> 0x20 -> switch to bank #0

 */

uint8_t a26_rom_fe_device::read(offs_t offset)
{
	// Super Chip RAM reads are mapped in 0x1080-0x10ff
	if (!m_ram.empty() && offset >= 0x80 && offset < 0x100)
	{
		return m_ram[offset & (m_ram.size() - 1)];
	}

	uint8_t const data = m_rom[offset + (m_base_bank * 0x1000)];

	if (!machine().side_effects_disabled())
	{
		switch_bank(data);
	}

	return data;
}

void a26_rom_fe_device::write(offs_t offset, uint8_t data)
{
	// Super Chip RAM writes are mapped in 0x1000-0x107f
	if (!m_ram.empty() && offset < 0x80)
	{
		m_ram[offset & (m_ram.size() - 1)] = data;
	}
}

void a26_rom_fe_device::switch_bank(uint8_t data)
{
	if (m_trigger_on_next_access)
	{
		m_base_bank = BIT(data, 5) ? 0 : 1;
		m_trigger_on_next_access = false;
	}
}

void a26_rom_fe_device::trigger_bank()
{
	if (m_ignore_first_read)
		m_ignore_first_read = false;
	else
		// The next byte on the data bus determines which bank to switch to
		m_trigger_on_next_access = true;
}



/*-------------------------------------------------
 "3E Bankswitch" Carts:
 write access to 0x3e determines the 2K ROM bank to
 be read, write access to 0x3f determines the RAM bank
 to be read

 GAMES: Boulder Dash (Homebrew)

 -------------------------------------------------*/

a26_rom_3e_device::a26_rom_3e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_3E, tag, owner, clock)
	, m_bank_mask(0)
	, m_ram_bank(0)
	, m_ram_enable(false)
{
}

void a26_rom_3e_device::device_start()
{
	a26_rom_f6_device::device_start();
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_ram_enable));
}

void a26_rom_3e_device::device_reset()
{
	m_bank_mask = (m_rom_size / 0x800) - 1;
	m_base_bank = m_bank_mask;
	m_ram_bank = 0;
	m_ram_enable = false;
}

void a26_rom_3e_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_3e_device::read)));
	space->install_write_handler(0x1000, 0x1fff, write8sm_delegate(*this, FUNC(a26_rom_3e_device::write)));
	space->install_write_tap(0x00, 0x3f, "bank",
			[this] (offs_t offset, u8 &data, u8) { if (!machine().side_effects_disabled()) write_bank(offset, data); });
}

uint8_t a26_rom_3e_device::read(offs_t offset)
{
	if (!m_ram.empty() && m_ram_enable && offset < 0x400)
		return m_ram[offset + (m_ram_bank * 0x400)];

	if (offset >= 0x800)
		return m_rom[(offset & 0x7ff) + m_bank_mask * 0x800];
	else
		return m_rom[offset + m_base_bank * 0x800];
}

void a26_rom_3e_device::write(offs_t offset, uint8_t data)
{
	if (!m_ram.empty() && m_ram_enable && offset >= 0x400 && offset < 0x800)
		m_ram[(offset & 0x3ff) + (m_ram_bank * 0x400)] = data;
}

void a26_rom_3e_device::write_bank(offs_t offset, uint8_t data)
{
	if (offset == 0x3f)
	{
		m_base_bank = data & m_bank_mask;
		m_ram_enable = false;
	}
	else if (offset == 0x3e)
	{
		m_ram_bank = data & 0x1f;
		m_ram_enable = true;
	}
}



/*-------------------------------------------------
 "3F Bankswitch" Carts:
 write access to 0x00-0x3f determines the 2K ROM bank
 to be read

 GAMES: Tigervision 8K games like Espial and Miner
 2049er. Extended version with bankswitch up to 512K
 shall be supported as well (but we lack a test case)

 -------------------------------------------------*/

a26_rom_3f_device::a26_rom_3f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_3F, tag, owner, clock)
	, m_bank_mask(0)
{
}

void a26_rom_3f_device::device_reset()
{
	m_bank_mask = (m_rom_size / 0x800) - 1;
	m_base_bank = m_bank_mask;
}

void a26_rom_3f_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_3f_device::read)));
	space->install_write_handler(0x1000, 0x1fff, write8sm_delegate(*this, FUNC(a26_rom_3f_device::write)));
	space->install_write_tap(0x00, 0x3f, "bank",
			[this] (offs_t offset, u8 &data, u8) { if (!machine().side_effects_disabled()) write_bank(offset, data); });
}

uint8_t a26_rom_3f_device::read(offs_t offset)
{
	if (offset >= 0x800)
		return m_rom[(offset & 0x7ff)  + m_bank_mask * 0x800];
	else
		return m_rom[offset + m_base_bank * 0x800];
}

void a26_rom_3f_device::write_bank(offs_t offset, uint8_t data)
{
	m_base_bank = data & m_bank_mask;
}



/*-------------------------------------------------
 "E0 Bankswitch" Carts:
 read/write access to 0x1fe0-0x1ff8 determines the
 1K ROM bank to be read in each 1K chunk (0x1c00-0x1fff
 always points to the last 1K of the ROM)

 GAMES: Parker Bros. 8K games like Gyruss and Popeye

 -------------------------------------------------*/

a26_rom_e0_device::a26_rom_e0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_E0, tag, owner, clock)
{
}

void a26_rom_e0_device::device_start()
{
	save_item(NAME(m_base_banks));
}

void a26_rom_e0_device::device_reset()
{
	m_base_banks[0] = 4;
	m_base_banks[1] = 5;
	m_base_banks[2] = 6;
	m_base_banks[3] = 7;
}

void a26_rom_e0_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_e0_device::read)));
	space->install_write_handler(0x1000, 0x1fff, write8sm_delegate(*this, FUNC(a26_rom_e0_device::write)));
}

uint8_t a26_rom_e0_device::read(offs_t offset)
{
	// update banks
	if (!machine().side_effects_disabled())
	{
		if (offset >= 0xfe0 && offset <= 0xff8)
			m_base_banks[(offset >> 3) & 3] = offset & 7;
	}

	return m_rom[(offset & 0x3ff) + (m_base_banks[(offset >> 10) & 3] * 0x400)];
}

void a26_rom_e0_device::write(offs_t offset, uint8_t data)
{
	if (offset >= 0xfe0 && offset <= 0xff8)
		m_base_banks[(offset >> 3) & 3] = offset & 7;
}



/*-------------------------------------------------
 "E7 Bankswitch" Carts:
 this PCB can handle up to 16K of ROM and 2K of RAM,
 with the following layout
 1000-17ff is selectable bank
 1800-19ff is RAM
 1a00-1fff is fixed to the last 0x600 of ROM

 The selectable bank can be ROM (if selected by
 0x1fe0-0x1fe6 access) or a first 1K of RAM (if
 selected by 0x1fe7 access).
 The other 256byte RAM bank can be one of the
 four different chunks forming the other 1K of RAM
 (the bank is selected by accessing 0x1fe8-0x1feb)

 GAMES: M Network 16K games like Burgertime and
 Bump'n Jump

 -------------------------------------------------*/

a26_rom_e7_device::a26_rom_e7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_E7, tag, owner, clock), m_ram_bank(0)
{
}

void a26_rom_e7_device::device_start()
{
	a26_rom_f6_device::device_start();
	save_item(NAME(m_ram_bank));
}

void a26_rom_e7_device::device_reset()
{
	m_base_bank = 0;
	m_ram_bank = 0;
}

void a26_rom_e7_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_e7_device::read)));
	space->install_write_handler(0x1000, 0x1fff, write8sm_delegate(*this, FUNC(a26_rom_e7_device::write)));
}

uint8_t a26_rom_e7_device::read(offs_t offset)
{
	// update banks
	if (!machine().side_effects_disabled())
	{
		if (offset >= 0xfe0 && offset <= 0xfe7)
			m_base_bank = offset - 0xfe0;
		if (offset >= 0xfe8 && offset <= 0xfeb)
			m_ram_bank = offset - 0xfe8;
	}

	if (!m_ram.empty())
	{
		// 1K of RAM
		if (m_base_bank == 0x07 && offset >= 0x400 && offset < 0x800)
			return m_ram[0x400 + (offset & 0x3ff)];
		// the other 1K of RAM
		if (offset >= 0x900 && offset < 0xa00)
		{
			offset -= 0x900;
			return m_ram[offset + (m_ram_bank * 0x100)];
		}
	}

	if (offset > 0x800)
		return m_rom[(offset & 0x7ff) + 0x3800];
	else
		return m_rom[(offset & 0x7ff) + (m_base_bank * 0x800)];
}

void a26_rom_e7_device::write(offs_t offset, uint8_t data)
{
	if (offset >= 0xfe0 && offset <= 0xfe7)
		m_base_bank = offset - 0xfe0;
	if (offset >= 0xfe8 && offset <= 0xfeb)
		m_ram_bank = offset - 0xfe8;

	if (!m_ram.empty())
	{
		// 1K of RAM
		if (m_base_bank == 0x07 && offset < 0x400)
			m_ram[0x400 + (offset & 0x3ff)] = data;
		// the other 1K of RAM
		if (offset >= 0x800 && offset < 0x900)
		{
			offset -= 0x800;
			m_ram[offset + (m_ram_bank * 0x100)] = data;
		}
	}
}



/*-------------------------------------------------
 "UA Bankswitch" Carts:
 read/write access to 0x200-0x27f determines the
 4K ROM bank to be read (0x220-0x23f for low 4K,
 0x240-0x27f for high 4K)

 GAMES: UA Ltd. 8K games like Funky Flash and
 Pleaides

 -------------------------------------------------*/

a26_rom_ua_device::a26_rom_ua_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_UA, tag, owner, clock)
{
}

void a26_rom_ua_device::device_reset()
{
	m_base_bank = 0;
}

void a26_rom_ua_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_ua_device::read)));
	space->install_readwrite_tap(0x200, 0x27f, "bank",
			[this] (offs_t offset, u8 &data, u8) { if (!machine().side_effects_disabled()) change_bank(offset); },
			[this] (offs_t offset, u8 &data, u8) { if (!machine().side_effects_disabled()) change_bank(offset); });
}

uint8_t a26_rom_ua_device::read(offs_t offset)
{
	return m_rom[(offset + (m_base_bank * 0x1000)) & (m_rom_size - 1)];
}

void a26_rom_ua_device::change_bank(offs_t offset)
{
	m_base_bank = offset >> 6;
}



/*-------------------------------------------------
 Commavid Carts:
 It allows for both ROM and RAM on the cartridge,
 without using bankswitching.  There's 2K of ROM
 and 1K of RAM.

 GAMES: Magicard and Video Life by Commavid

 -------------------------------------------------*/

a26_rom_cv_device::a26_rom_cv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_CV, tag, owner, clock)
{
}

void a26_rom_cv_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_cv_device::read)));
	space->install_write_handler(0x1000, 0x1fff, write8sm_delegate(*this, FUNC(a26_rom_cv_device::write)));
}

uint8_t a26_rom_cv_device::read(offs_t offset)
{
	if (!m_ram.empty() && offset < 0x400)
	{
		return m_ram[offset & (m_ram.size() - 1)];
	}

	// games shall not read from 0x1400-0x17ff (RAM write)
	// but we fall back to ROM just in case...
	return m_rom[offset & 0x7ff];
}

void a26_rom_cv_device::write(offs_t offset, uint8_t data)
{
	if (!m_ram.empty() && offset >= 0x400 && offset < 0x800)
	{
		m_ram[offset & (m_ram.size() - 1)] = data;
	}
}



/*-------------------------------------------------
 Dynacom Megaboy Carts (aka "F0 Banswitch"):
 read/write access to 0x1ff0 determines the 4K ROM
 bank to be read (each access increases the bank index
 up to 16, since the cart was 64K wide)

 GAMES: Megaboy by Dynacom

 -------------------------------------------------*/

a26_rom_dc_device::a26_rom_dc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_DC, tag, owner, clock)
{
}

void a26_rom_dc_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_dc_device::read)));
	space->install_write_handler(0x1000, 0x1fff, write8sm_delegate(*this, FUNC(a26_rom_dc_device::write)));
}

uint8_t a26_rom_dc_device::read(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (offset == 0xff0)
			m_base_bank = (m_base_bank + 1) & 0x0f;
	}

	if (offset == 0xfec)
		return m_base_bank;

	return m_rom[offset + (m_base_bank * 0x1000)];
}

void a26_rom_dc_device::write(offs_t offset, uint8_t data)
{
	if (offset == 0xff0)
		m_base_bank = (m_base_bank + 1) & 0x0f;
}



/*-------------------------------------------------
 "FV Bankswitch" Carts:
 The first access to 0x1fd0 switch the bank, but
 only if pc() & 0x1f00 == 0x1f00!

 GAMES: Challenge by HES

 -------------------------------------------------*/

a26_rom_fv_device::a26_rom_fv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_FV, tag, owner, clock)
	, m_locked(false)
{
}

void a26_rom_fv_device::device_start()
{
	a26_rom_f6_device::device_start();
	save_item(NAME(m_locked));
}

void a26_rom_fv_device::device_reset()
{
	m_base_bank = 0;
	m_locked = false;
}

void a26_rom_fv_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_fv_device::read)));
	space->install_write_handler(0x1000, 0x1fff, write8sm_delegate(*this, FUNC(a26_rom_fv_device::write)));
}

uint8_t a26_rom_fv_device::read(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (offset == 0xfd0)
		{
			if (!m_locked)
			{
				m_locked = true;
				m_base_bank = m_base_bank ^ 0x01;
			}
		}
	}

	return m_rom[offset + (m_base_bank * 0x1000)];
}

void a26_rom_fv_device::write(offs_t offset, uint8_t data)
{
	if (offset == 0xfd0)
	{
		if (!m_locked)
		{
			m_locked = true;
			m_base_bank = m_base_bank ^ 0x01;
		}
	}
}



/*-------------------------------------------------
 "JVP Bankswitch" Carts:
 read/write access to 0x0fa0-0x0fc0 determines the
 4K ROM bank to be read (notice that this overlaps
 the RIOT, currently handled in the main driver until
 I can better investigate the behavior)

 GAMES: No test case!?!

 -------------------------------------------------*/

a26_rom_jvp_device::a26_rom_jvp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_JVP, tag, owner, clock)
{
}

void a26_rom_jvp_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_jvp_device::read)));
	space->install_readwrite_tap(0xfa0, 0xfc0, "bank",
			[this] (offs_t offset, u8 &data, u8) { if (!machine().side_effects_disabled()) change_bank(offset); },
			[this] (offs_t offset, u8 &data, u8) { if (!machine().side_effects_disabled()) change_bank(offset); });
}

uint8_t a26_rom_jvp_device::read(offs_t offset)
{
	return m_rom[offset + (m_base_bank * 0x1000)];
}

void a26_rom_jvp_device::change_bank(offs_t offset)
{
	switch (offset)
	{
		case 0x00:
		case 0x20:
			m_base_bank ^= 1;
			break;
		default:
			//printf("%04X: write to unknown mapper address %02X\n", m_maincpu->pc(), 0xfa0 + offset);
			break;
	}
}



/*-------------------------------------------------
 4 in 1 Carts (Reset based):
 the 4K bank changes at each reset

 GAMES: 4 in 1 carts

 -------------------------------------------------*/

a26_rom_4in1_device::a26_rom_4in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_4IN1, tag, owner, clock)
{
}

void a26_rom_4in1_device::device_reset()
{
	m_base_bank = (m_base_bank + 1) & 3;
}

void a26_rom_4in1_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_4in1_device::read)));
}

uint8_t a26_rom_4in1_device::read(offs_t offset)
{
	return m_rom[offset + (m_base_bank * 0x1000)];
}



/*-------------------------------------------------
 8 in 1 Carts (Reset based):
 the 8K banks change at each reset, and internally
 each game runs as a F8-bankswitched cart

 GAMES: 8 in 1 cart

 -------------------------------------------------*/

a26_rom_8in1_device::a26_rom_8in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_8IN1, tag, owner, clock)
	, m_reset_bank(0xff)
{
}

void a26_rom_8in1_device::device_start()
{
	a26_rom_f6_device::device_start();
	save_item(NAME(m_reset_bank));
}

void a26_rom_8in1_device::device_reset()
{
	// we use two different bank counters: the main one for the 8x8K chunks,
	// and the usual one (m_base_bank) for the 4K bank of the current game
	m_reset_bank = (m_reset_bank + 1) & 7;
	m_base_bank = 0;
}

void a26_rom_8in1_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_8in1_device::read)));
	space->install_write_handler(0x1000, 0x1fff, write8sm_delegate(*this, FUNC(a26_rom_8in1_device::write)));
}

uint8_t a26_rom_8in1_device::read(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
			case 0x0ff8:
			case 0x0ff9:
				m_base_bank = offset - 0x0ff8;
				break;
		}
	}

	return m_rom[offset + (m_base_bank * 0x1000) + (m_reset_bank * 0x2000)];
}

void a26_rom_8in1_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x0ff8:
		case 0x0ff9:
			m_base_bank = offset - 0x0ff8;
			break;
		default:
			logerror("Write Bank outside expected range (0x%X).\n", offset + 0x1000);
			break;
	}
}



/*-------------------------------------------------
 32 in 1 Carts (Reset based):
 the 2K banks change at each reset

 GAMES: 32 in 1 cart

 -------------------------------------------------*/

a26_rom_32in1_device::a26_rom_32in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_32IN1, tag, owner, clock)
{
}

void a26_rom_32in1_device::device_reset()
{
	m_base_bank = (m_base_bank + 1) & 0x1f;
}

void a26_rom_32in1_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_32in1_device::read)));
}

uint8_t a26_rom_32in1_device::read(offs_t offset)
{
	return m_rom[(offset & 0x7ff) + (m_base_bank * 0x800)];
}



/*-------------------------------------------------
 "X07 Bankswitch" Carts:
 banking done with a PALC22V10B
 implementation based on information at
 http://blog.kevtris.org/blogfiles/Atari%202600%20Mappers.txt
 --------------------------------------------------*/

a26_rom_x07_device::a26_rom_x07_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_X07, tag, owner, clock)
{
}

void a26_rom_x07_device::install_memory_handlers(address_space *space)
{
	space->install_read_handler(0x1000, 0x1fff, read8sm_delegate(*this, FUNC(a26_rom_x07_device::read)));
	space->install_readwrite_tap(0x0000, 0x0fff, "bank",
		[this] (offs_t offset, u8 &, u8) { if (!machine().side_effects_disabled()) change_bank(offset); },
		[this] (offs_t offset, u8 &, u8) { if (!machine().side_effects_disabled()) change_bank(offset); });
}

uint8_t a26_rom_x07_device::read(offs_t offset)
{
	return m_rom[offset + (m_base_bank * 0x1000)];
}

void a26_rom_x07_device::change_bank(offs_t offset)
{
	/*
	A13           A0
	----------------
	0 1xxx nnnn 1101
	*/
	if ((offset & 0x180f) == 0x080d)
		m_base_bank = (offset & 0x00f0) >> 4;

	/*
	A13           A0
	----------------
	0 0xxx 0nxx xxxx
	*/
	if ((offset & 0x1880) == 0x0000)
	{
		// only has an effect if bank is already 14 or 15
		if (m_base_bank == 14 || m_base_bank == 15)
		{
			if (offset & 0x0040)
				m_base_bank = 15;
			else
				m_base_bank = 14;
		}
	}
}
