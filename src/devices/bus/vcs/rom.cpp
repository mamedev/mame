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



bool a26_rom_base_device::is_ram_present(size_t minimum_size)
{
	return m_ram.size() >= minimum_size;
}

uint8_t a26_rom_base_device::read_ram(offs_t offset)
{
	return m_ram[offset];
}

void a26_rom_base_device::write_ram(offs_t offset, uint8_t data)
{
	m_ram[offset] = data;
}


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
	if (m_rom_size < 0x1000)
		space->install_rom(0x1000, 0x17ff, 0x800, get_rom_base());
	else
		space->install_rom(0x1000, 0x1fff, get_rom_base());
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
	, m_bank(*this, "bank")
{
}

a26_rom_f6_device::a26_rom_f6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f6_device(mconfig, A26_ROM_F6, tag, owner, clock)
{
}

void a26_rom_f6_device::device_reset()
{
	m_bank->set_entry(get_start_bank());
}

void a26_rom_f6_device::install_super_chip_handlers(address_space *space)
{
	if (is_ram_present(0x80)) {
		space->install_read_handler(0x1080, 0x10ff, read8sm_delegate(*this, FUNC(a26_rom_f6_device::read_ram)));
		space->install_write_handler(0x1000, 0x107f, write8sm_delegate(*this, FUNC(a26_rom_f6_device::write_ram)));
	}
}

void a26_rom_f6_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 4, get_rom_base(), 0x1000);
	space->install_read_bank(0x1000, 0x1fff, m_bank);
	space->install_write_handler(0x1ff6, 0x1ff9, write8sm_delegate(*this, FUNC(a26_rom_f6_device::switch_bank)));
	install_super_chip_handlers(space);
	space->install_read_tap(0x1ff6, 0x1ff9, "bank",
			[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) switch_bank(address - 0x1ff6, 0); });
}

void a26_rom_f6_device::switch_bank(offs_t offset, uint8_t data)
{
	m_bank->set_entry(offset);
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

void a26_rom_f4_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 8, get_rom_base(), 0x1000);
	space->install_read_bank(0x1000, 0x1fff, m_bank);
	space->install_write_handler(0x1ff4, 0x1ffb, write8sm_delegate(*this, FUNC(a26_rom_f4_device::switch_bank)));
	install_super_chip_handlers(space);
	space->install_read_tap(0x1ff4, 0x1ffb, "bank",
			[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) switch_bank(address - 0x1ff4, 0); });
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
	m_bank->configure_entries(0, 2, get_rom_base(), 0x1000);
	space->install_read_bank(0x1000, 0x1fff, m_bank);
	space->install_write_handler(0x1ff8, 0x1ff9, write8sm_delegate(*this, FUNC(a26_rom_f8_device::switch_bank)));
	install_super_chip_handlers(space);
	space->install_read_tap(0x1ff8, 0x1ff9, "bank",
			[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) switch_bank(address - 0x1ff8, 0); });
}


a26_rom_f8_sw_device::a26_rom_f8_sw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f8_device(mconfig, A26_ROM_F8_SW, tag, owner, clock)
{
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
	m_bank->configure_entries(0, 3, get_rom_base(), 0x1000);
	space->install_read_bank(0x1000, 0x1fff, m_bank);
	space->install_write_handler(0x1ff8, 0x1ffa, write8sm_delegate(*this, FUNC(a26_rom_fa_device::switch_bank)));
	space->install_read_tap(0x1ff8, 0x1ffa, "bank",
			[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) switch_bank(address - 0x1ff8, 0); });
	space->install_read_handler(0x1100, 0x11ff, read8sm_delegate(*this, FUNC(a26_rom_fa_device::read_ram)));
	space->install_write_handler(0x1000, 0x10ff, write8sm_delegate(*this, FUNC(a26_rom_fa_device::write_ram)));
}



/*-------------------------------------------------
 "FE Bankswitch" Carts:
 read/write access to 0x01fe-0x1ff determines the
 4K ROM bank to be read

 GAMES: Activision 8K games like Decathlon

 -------------------------------------------------*/

a26_rom_fe_device::a26_rom_fe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_FE, tag, owner, clock)
	, m_bank(*this, "bank")
	, m_trigger_on_next_access(false)
	, m_ignore_first_read(true)
{
}

void a26_rom_fe_device::device_start()
{
	save_item(NAME(m_trigger_on_next_access));
	save_item(NAME(m_ignore_first_read));
}

void a26_rom_fe_device::device_reset()
{
	m_bank->set_entry(0);
	m_trigger_on_next_access = false;
	// During cpu startup, before reading the boot vector a read from $01fe occurs
	m_ignore_first_read = true;
}

void a26_rom_fe_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 8, get_rom_base(), 0x1000);

	space->install_read_bank(0x1000, 0x1fff, m_bank);
	space->install_readwrite_tap(0x1fe, 0x1fe, "trigger_bank",
			[this] (offs_t, u8 &, u8) { if (!machine().side_effects_disabled()) trigger_bank(); },
			[this] (offs_t, u8 &, u8) { if (!machine().side_effects_disabled()) trigger_bank(); });
	space->install_read_tap(0x1ff, 0x1ff, "switch_bank1",
			[this] (offs_t, u8 &data, u8) { if (!machine().side_effects_disabled()) switch_bank(data); });
	space->install_read_tap(0x1000, 0x1fff, "switch_bank2",
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

void a26_rom_fe_device::switch_bank(uint8_t data)
{
	if (m_trigger_on_next_access)
	{
		m_bank->set_entry(BIT(data, 5) ? 0 : 1);
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
	: a26_rom_base_device(mconfig, A26_ROM_3E, tag, owner, clock)
	, m_rom_bank(*this, "rom_bank")
	, m_ram_bank(*this, "ram_bank")
	, m_view(*this, "view")
	, m_rom_bank_mask(0)
	, m_ram_bank_mask(0)
{
}

void a26_rom_3e_device::device_reset()
{
	m_rom_bank->set_entry(m_rom_bank_mask);
	m_view.select(0);
}

void a26_rom_3e_device::install_memory_handlers(address_space *space)
{
	m_rom_bank_mask = (get_rom_size() / 0x800) - 1;
	m_ram_bank_mask = (get_ram_size() / 0x400) - 1;
	m_rom_bank->configure_entries(0, m_rom_bank_mask + 1, get_rom_base(), 0x800);
	m_ram_bank->configure_entries(0, m_ram_bank_mask + 1, get_ram_base(), 0x400);

	space->install_view(0x1000, 0x17ff, m_view);
	// RAM disabled
	m_view[0].install_read_bank(0x1000, 0x17ff, m_rom_bank);
	// RAM enabled
	m_view[1].install_read_bank(0x1000, 0x17ff, m_rom_bank);
	m_view[1].install_read_bank(0x1000, 0x13ff, m_ram_bank);
	m_view[1].install_write_bank(0x1400, 0x17ff, m_ram_bank);

	space->install_rom(0x1800, 0x1fff, get_rom_base() + m_rom_bank_mask * 0x800);
	space->install_write_tap(0x3e, 0x3e, "ram_bank",
			[this] (offs_t address, u8 &data, u8) { if (!machine().side_effects_disabled()) select_ram_bank(address, data); });
	space->install_write_tap(0x3f, 0x3f, "rom_bank",
			[this] (offs_t address, u8 &data, u8) { if (!machine().side_effects_disabled()) select_rom_bank(address, data); });
}

void a26_rom_3e_device::select_ram_bank(offs_t address, uint8_t data)
{
	m_ram_bank->set_entry(data & m_ram_bank_mask);
	m_view.select(1);
}

void a26_rom_3e_device::select_rom_bank(offs_t address, uint8_t data)
{
	m_rom_bank->set_entry(data & m_rom_bank_mask);
	m_view.select(0);
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
	: a26_rom_base_device(mconfig, A26_ROM_3F, tag, owner, clock)
	, m_bank(*this, "bank")
	, m_bank_mask(0)
{
}

void a26_rom_3f_device::device_reset()
{
	m_bank->set_entry(m_bank_mask);
}

void a26_rom_3f_device::install_memory_handlers(address_space *space)
{
	m_bank_mask = (get_rom_size() / 0x800) - 1;
	m_bank->configure_entries(0, get_rom_size() / 0x800, get_rom_base(), 0x800);

	space->install_read_bank(0x1000, 0x17ff, m_bank);
	// last bank
	space->install_rom(0x1800, 0x1fff, get_rom_base() + m_bank_mask * 0x800);
	space->install_write_tap(0x00, 0x3f, "bank",
			[this] (offs_t offset, u8 &data, u8) { if (!machine().side_effects_disabled()) select_bank(offset, data); });
}

void a26_rom_3f_device::select_bank(offs_t offset, uint8_t data)
{
	m_bank->set_entry(data & m_bank_mask);
}



/*-------------------------------------------------
 "E0 Bankswitch" Carts:
 read/write access to 0x1fe0-0x1ff7 determines the
 1K ROM bank to be read in each 1K chunk (0x1c00-0x1fff
 always points to the last 1K of the ROM)

 GAMES: Parker Bros. 8K games like Gyruss and Popeye

 -------------------------------------------------*/

a26_rom_e0_device::a26_rom_e0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_E0, tag, owner, clock)
	, m_bank(*this, "bank%u", 0U)
{
}

void a26_rom_e0_device::device_reset()
{
	m_bank[0]->set_entry(4);
	m_bank[1]->set_entry(5);
	m_bank[2]->set_entry(6);
}

void a26_rom_e0_device::install_memory_handlers(address_space *space)
{
	m_bank[0]->configure_entries(0, 8, get_rom_base(), 0x400);
	m_bank[1]->configure_entries(0, 8, get_rom_base(), 0x400);
	m_bank[2]->configure_entries(0, 8, get_rom_base(), 0x400);

	space->install_read_bank(0x1000, 0x13ff, m_bank[0]);
	space->install_read_bank(0x1400, 0x17ff, m_bank[1]);
	space->install_read_bank(0x1800, 0x1bff, m_bank[2]);
	space->install_rom(0x1c00, 0x1fff, get_rom_base() + 7 * 0x400);
	space->install_write_handler(0x1fe0, 0x1fe7, write8sm_delegate(*this, FUNC(a26_rom_e0_device::switch_bank<0>)));
	space->install_write_handler(0x1fe8, 0x1fef, write8sm_delegate(*this, FUNC(a26_rom_e0_device::switch_bank<1>)));
	space->install_write_handler(0x1ff0, 0x1ff7, write8sm_delegate(*this, FUNC(a26_rom_e0_device::switch_bank<2>)));
	space->install_read_tap(0x1fe0, 0x1fe7, "bank",
			[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) switch_bank<0>(address, 0); });
	space->install_read_tap(0x1fe8, 0x1fef, "bank",
			[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) switch_bank<1>(address, 0); });
	space->install_read_tap(0x1ff0, 0x1ff7, "bank",
			[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) switch_bank<2>(address, 0); });
}

template <uint8_t Bank>
void a26_rom_e0_device::switch_bank(offs_t offset, uint8_t data)
{	
	m_bank[Bank]->set_entry(offset & 7);
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
	: a26_rom_base_device(mconfig, A26_ROM_E7, tag, owner, clock)
	, m_rom_bank(*this, "rom_bank")
	, m_lo_ram_bank(*this, "low_ram")
	, m_hi_ram_bank(*this, "high_ram")
	, m_view(*this, "view")
{
}

void a26_rom_e7_device::device_reset()
{
	m_rom_bank->set_entry(0);
	m_hi_ram_bank->set_entry(0);
}

void a26_rom_e7_device::install_memory_handlers(address_space *space)
{
	m_rom_bank->configure_entries(0, 8, get_rom_base(), 0x800);
	m_lo_ram_bank->configure_entry(0, get_ram_base() + 0x400);
	m_hi_ram_bank->configure_entries(0, 4, get_ram_base(), 0x100);

	space->install_view(0x1000, 0x17ff, m_view);
	// RAM disabled
	m_view[0].install_read_bank(0x1000, 0x17ff, m_rom_bank);
	// RAM enabled
	m_view[1].install_read_bank(0x1000, 0x17ff, m_rom_bank);
	m_view[1].install_write_bank(0x1000, 0x13ff, m_lo_ram_bank);
	m_view[1].install_read_bank(0x1400, 0x17ff, m_lo_ram_bank);

	space->install_rom(0x1800, 0x1fff, get_rom_base() + 7 * 0x800);
	space->install_write_bank(0x1800, 0x18ff, m_hi_ram_bank);
	space->install_read_bank(0x1900, 0x19ff, m_hi_ram_bank);
	space->install_write_handler(0x1fe0, 0x1fe7, write8sm_delegate(*this, FUNC(a26_rom_e7_device::switch_rom_bank)));
	space->install_read_tap(0x1fe0, 0x1fe7, "rom_bank",
			[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) switch_rom_bank(address - 0x1fe0, 0); });
	space->install_write_handler(0x1fe8, 0x1feb, write8sm_delegate(*this, FUNC(a26_rom_e7_device::switch_ram_bank)));
	space->install_read_tap(0x1fe8, 0x1feb, "ram_bank",
			[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) switch_ram_bank(address - 0x1fe8, 0); });
}

void a26_rom_e7_device::switch_rom_bank(offs_t offset, uint8_t data)
{
	m_view.select(offset == 0x07 ? 1 : 0);
	m_rom_bank->set_entry(offset);
}

void a26_rom_e7_device::switch_ram_bank(offs_t offset, uint8_t data)
{
	m_hi_ram_bank->set_entry(offset);
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
	: a26_rom_base_device(mconfig, A26_ROM_UA, tag, owner, clock)
	, m_bank(*this, "bank")
{
}

void a26_rom_ua_device::device_reset()
{
	m_bank->set_entry(0);
}

void a26_rom_ua_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 2, get_rom_base(), 0x1000);

	space->install_read_bank(0x1000, 0x1fff, m_bank);
	space->install_readwrite_tap(0x200, 0x27f, "bank",
			[this] (offs_t offset, u8 &data, u8) { if (!machine().side_effects_disabled()) change_bank(offset); },
			[this] (offs_t offset, u8 &data, u8) { if (!machine().side_effects_disabled()) change_bank(offset); });
}

void a26_rom_ua_device::change_bank(offs_t offset)
{
	m_bank->set_entry((offset >> 6) & 1);
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
	space->install_rom(0x1000, 0x17ff, 0x800, get_rom_base());
	space->install_read_handler(0x1000, 0x13ff, read8sm_delegate(*this, FUNC(a26_rom_cv_device::read_ram)));
	space->install_write_handler(0x1400, 0x17ff, write8sm_delegate(*this, FUNC(a26_rom_cv_device::write_ram)));
}



/*-------------------------------------------------
 Dynacom Megaboy Carts (aka "F0 Banswitch"):
 read/write access to 0x1ff0 determines the 4K ROM
 bank to be read (each access increases the bank index
 up to 16, since the cart was 64K wide)

 GAMES: Megaboy by Dynacom

 -------------------------------------------------*/

a26_rom_dc_device::a26_rom_dc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_DC, tag, owner, clock)
	, m_bank(*this, "bank")
{
}

void a26_rom_dc_device::device_reset()
{
	m_bank->set_entry(0);
}

void a26_rom_dc_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 16, get_rom_base(), 0x1000);

	space->install_read_bank(0x1000, 0x1fff, m_bank);
	space->install_read_handler(0x1fec, 0x1fec, read8sm_delegate(*this, FUNC(a26_rom_dc_device::read_current_bank)));
	space->install_write_handler(0x1ff0, 0x1ff0, write8sm_delegate(*this, FUNC(a26_rom_dc_device::switch_bank)));
	space->install_read_tap(0x1ff0, 0x1ff0, "bank",
			[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) switch_bank(0, 0); });
}

uint8_t a26_rom_dc_device::read_current_bank(offs_t offset)
{
	return m_bank->entry();
}

void a26_rom_dc_device::switch_bank(offs_t offset, uint8_t data)
{
	m_bank->set_entry((m_bank->entry() + 1) & 0x0f);
}



/*-------------------------------------------------
 "FV Bankswitch" Carts:
 The first access to 0x1fd0 switch the bank.

 GAMES: Challenge by HES

 -------------------------------------------------*/

a26_rom_fv_device::a26_rom_fv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_FV, tag, owner, clock)
	, m_bank(*this, "bank")
{
}

void a26_rom_fv_device::device_reset()
{
	m_bank->set_entry(0);
}

void a26_rom_fv_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 2, get_rom_base(), 0x1000);

	space->install_read_bank(0x1000, 0x1fff, m_bank);
	space->install_write_handler(0x1fd0, 0x1fd0, write8sm_delegate(*this, FUNC(a26_rom_fv_device::switch_bank)));
	space->install_read_tap(0x1fd0, 0x1fd0, "bank",
			[this] (offs_t, u8 &, u8) { if (!machine().side_effects_disabled()) switch_bank(0, 0); });
}

void a26_rom_fv_device::switch_bank(offs_t offset, uint8_t data)
{
	m_bank->set_entry(1);
}



/*-------------------------------------------------
 "JVP Bankswitch" Carts:
 read/write access to 0x0fa0-0x0fc0 determines the
 4K ROM bank to be read.
 
 GAMES: No test case!?!

 -------------------------------------------------*/

a26_rom_jvp_device::a26_rom_jvp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_JVP, tag, owner, clock)
	, m_bank(*this, "bank")
{
}

void a26_rom_jvp_device::device_reset()
{
	m_bank->set_entry(0);
}

void a26_rom_jvp_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 2, get_rom_base(), 0x1000);

	space->install_read_bank(0x1000, 0x1fff, m_bank);
	space->install_readwrite_tap(0xfa0, 0xfa0, "bank_fa0",
			[this] (offs_t, u8 &, u8) { if (!machine().side_effects_disabled()) change_bank(); },
			[this] (offs_t, u8 &, u8) { if (!machine().side_effects_disabled()) change_bank(); });
	space->install_readwrite_tap(0xfc0, 0xfc0, "bank_fc0",
			[this] (offs_t, u8 &, u8) { if (!machine().side_effects_disabled()) change_bank(); },
			[this] (offs_t, u8 &, u8) { if (!machine().side_effects_disabled()) change_bank(); });
}

void a26_rom_jvp_device::change_bank()
{
	m_bank->set_entry(m_bank->entry() ^ 1);
}



/*-------------------------------------------------
 4 in 1 Carts (Reset based):
 the 4K bank changes at each reset

 GAMES: 4 in 1 carts

 -------------------------------------------------*/

a26_rom_4in1_device::a26_rom_4in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_4IN1, tag, owner, clock)
	, m_bank(*this, "bank")
	, m_current_game(0)
{
}

void a26_rom_4in1_device::device_start()
{
	save_item(NAME(m_current_game));
	m_current_game = 0;
}

void a26_rom_4in1_device::device_reset()
{
	m_bank->set_entry(m_current_game);
	m_current_game = (m_current_game + 1) & 3;
}

void a26_rom_4in1_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 4, get_rom_base(), 0x1000);

	space->install_read_bank(0x1000, 0x1fff, m_bank);
}



/*-------------------------------------------------
 8 in 1 Carts (Reset based):
 the 8K banks change at each reset, and internally
 each game runs as a F8-bankswitched cart

 GAMES: 8 in 1 cart

 -------------------------------------------------*/

a26_rom_8in1_device::a26_rom_8in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_8IN1, tag, owner, clock)
	, m_bank(*this, "bank")
{
}

void a26_rom_8in1_device::device_start()
{
	save_item(NAME(m_game_bank));

	// starting from last game so first reset will get us to game #0
	m_game_bank = 14;
}

void a26_rom_8in1_device::device_reset()
{
	m_game_bank = (m_game_bank + 2) & 15;
	m_bank->set_entry(m_game_bank);
}

void a26_rom_8in1_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 16, get_rom_base(), 0x1000);

	space->install_read_bank(0x1000, 0x1fff, m_bank);
	space->install_write_handler(0x1ff8, 0x1ff9, write8sm_delegate(*this, FUNC(a26_rom_8in1_device::switch_bank)));
	space->install_read_tap(0x1ff8, 0x1ff9, "bank",
			[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) switch_bank(address - 0x1ff8, 0); });
}

void a26_rom_8in1_device::switch_bank(offs_t offset, uint8_t data)
{
	m_bank->set_entry(m_game_bank + offset);
}



/*-------------------------------------------------
 32 in 1 Carts (Reset based):
 the 2K banks change at each reset

 GAMES: 32 in 1 cart

 -------------------------------------------------*/

a26_rom_32in1_device::a26_rom_32in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_32IN1, tag, owner, clock)
	, m_bank(*this, "bank")
	, m_current_game(0)
{
}

void a26_rom_32in1_device::device_start()
{
	save_item(NAME(m_current_game));
	m_current_game = 0;
}

void a26_rom_32in1_device::device_reset()
{
	m_bank->set_entry(m_current_game);
	m_current_game = (m_current_game + 1) & 0x1f;
}

void a26_rom_32in1_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 32, get_rom_base(), 0x800);

	space->install_read_bank(0x1000, 0x17ff, 0x800, m_bank);
}



/*-------------------------------------------------
 "X07 Bankswitch" Carts:
 banking done with a PALC22V10B
 implementation based on information at
 http://blog.kevtris.org/blogfiles/Atari%202600%20Mappers.txt
 --------------------------------------------------*/

a26_rom_x07_device::a26_rom_x07_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_X07, tag, owner, clock)
	, m_bank(*this, "bank")
{
}

void a26_rom_x07_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 16, get_rom_base(), 0x1000);

	space->install_read_bank(0x1000, 0x1fff, m_bank);
	/*
	A13           A0
	----------------
	0 0xxx 0nxx xxxx
	*/
	space->install_readwrite_tap(0x0000, 0x0000, 0x077f, "bank1",
		[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) change_bank1(address); },
		[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) change_bank1(address); });
	/*
	A13           A0
	----------------
	0 1xxx nnnn 1101
	*/
	space->install_readwrite_tap(0x080d, 0x080d, 0x07f0, "bank2",
		[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) change_bank2(address); },
		[this] (offs_t address, u8 &, u8) { if (!machine().side_effects_disabled()) change_bank2(address); });
}

void a26_rom_x07_device::change_bank1(offs_t address)
{
	// only has an effect if bank is already 14 or 15
	if (m_bank->entry() >= 14)
	{
		if (address & 0x0040)
			m_bank->set_entry(15);
		else
			m_bank->set_entry(14);
	}
}

void a26_rom_x07_device::change_bank2(offs_t address)
{
	m_bank->set_entry((address >> 4) & 0x0f);
}
