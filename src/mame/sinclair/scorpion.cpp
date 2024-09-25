// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic

#include "emu.h"
#include "beta_m.h"
#include "spec128.h"

#include "bus/spectrum/zxbus.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "speaker.h"

#define LOG_IO   (1U << 1)
#define LOG_MEM  (1U << 2)
#define LOG_WARN (1U << 3)

#define VERBOSE ( /*LOG_GENERAL | LOG_IO | LOG_MEM |*/ LOG_WARN )
#include "logmacro.h"

#define LOGIO(...)   LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMEM(...)  LOGMASKED(LOG_MEM,  __VA_ARGS__)
#define LOGWARN(...) LOGMASKED(LOG_WARN, __VA_ARGS__)

namespace {

class scorpion_state : public spectrum_128_state
{
public:
	scorpion_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_128_state(mconfig, type, tag)
		, m_bankio(*this, "bankio")
		, m_bank0_rom(*this, "bank0_rom")
		, m_beta(*this, BETA_DISK_TAG)
		, m_ay(*this, "ay%u", 0U)
		, m_io_mouse(*this, "mouse_input%u", 1U)
		, m_mod_ay(*this, "MOD_AY")
	{ }

	void scorpion(machine_config &config);
	void profi(machine_config &config);
	void quorum(machine_config &config);

	INPUT_CHANGED_MEMBER(on_nmi);

protected:
	static constexpr u8 ROM_PAGE_SYS = 2;
	static constexpr u8 ROM_PAGE_DOS = 3;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	bool dos()    const { return m_beta->is_active(); }
	bool romram() const { return BIT(m_port_1ffd_data, 0); }
	bool rom1()   const { return BIT(m_port_7ffd_data, 4); }

	void scorpion_io(address_map &map) ATTR_COLD;
	void scorpion_mem(address_map &map) ATTR_COLD;
	void scorpion_switch(address_map &map) ATTR_COLD;
	virtual void scorpion_ioext(address_map &map) ATTR_COLD;
	u8 port_ff_r();
	void port_7ffd_w(u8 data);
	void port_1ffd_w(u8 data);
	virtual void ay_address_w(u8 data);

	virtual rectangle get_screen_area() override;
	virtual void scorpion_update_memory();
	virtual void do_nmi();

	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<17, 0, 0, ENDIANNESS_LITTLE>::specific m_ioext;
	required_device<address_map_bank_device> m_bankio;
	memory_view m_bank0_rom;
	required_device<beta_disk_device> m_beta;
	required_device_array<ay8912_device, 2> m_ay;

	u8 m_is_m1_even;
	u8 m_nmi_pending;
	u8 m_magic_lock;
	u8 m_ay_selected;
	u8 m_ram_banks;

private:
	u8 beta_neutral_r(offs_t offset);
	u8 beta_enable_r(offs_t offset);
	u8 beta_disable_r(offs_t offset);
	INTERRUPT_GEN_MEMBER(scorpion_interrupt);

	required_ioport_array<3> m_io_mouse;
	required_ioport m_mod_ay;
};

class scorpiontb_state : public scorpion_state
{
public:
	scorpiontb_state(const machine_config &mconfig, device_type type, const char *tag)
		: scorpion_state(mconfig, type, tag)
	{ }

	void scorpiontb(machine_config &config);

	INPUT_CHANGED_MEMBER(turbo_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	virtual void scorpion_ioext(address_map &map) override ATTR_COLD;
	virtual void ay_address_w(u8 data) override;

	virtual void scorpion_update_memory() override;

	u8 m_turbo;
	u8 m_prof_plane;

private:
	u8 ay_data_r();

	u8 m_ay_reg;

};

class scorpiongmx_state : public scorpiontb_state
{
public:
	scorpiongmx_state(const machine_config &mconfig, device_type type, const char *tag)
		: scorpiontb_state(mconfig, type, tag)
		, m_io_gmx(*this, "io_gmx")
	{ }

	void scorpiongmx(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	virtual void scorpion_ioext(address_map &map) override ATTR_COLD;
	void global_cfg_w(u8 data);
	u8 port_78fd_r();
	u8 port_7afd_r();
	u8 port_7efd_r();
	void port_7efd_w(u8 data);

	virtual void scorpion_update_memory() override;
	virtual void spectrum_update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	virtual rectangle get_screen_area() override;
	virtual void do_nmi() override;

	memory_view m_io_gmx;

private:
	bool fixrom() const { return BIT(m_port_00_data, 4); }

	u8 m_magic_shift;
	u8 m_gfx_ext = 0;
	u8 m_magic_disabled;
	u8 m_port_00_data;
	u8 m_port_78fd_data;
	u8 m_port_7afd_lock_data;
	u8 m_scroll_lo;
	u8 m_scroll_hi;
	u8 m_port_7efd_lock_data;
	u8 m_port_dffd_data;

};


/****************************************************************************************************/
/* Zs Scorpion 256 */

/*
port 7ffd. full compatibility with Zx spectrum 128. digits are:

D0-D2 - number of RAM page to put in C000-FFFF
D3    - switch of address for RAM of screen. 0 - 4000, 1 - c000
D4    - switch of ROM : 0-zx128, 1-zx48
D5    - 1 in this bit will block further output in port 7FFD, until reset.
*/

/*
port 1ffd - additional port for resources of computer.

D0    - block of ROM in 0-3fff. when set to 1 - allows read/write page 0 of RAM
D1    - selects ROM expansion. this rom contains main part of service monitor.
D2    - not used
D3    - used for output in RS-232C
D4    - extended RAM. set to 1 - connects RAM page with number 8-15 in
    C000-FFFF. number of page is given in gidits D0-D2 of port 7FFD
D5    - signal of strobe for interface centronics. to form the strobe has to be
    set to 1.
D6-D7 - not used. ( yet ? )
*/

/* rom 0=zx128, 1=zx48, 2 = service monitor, 3=tr-dos */

void scorpion_state::scorpion_update_memory()
{
	if (romram() && !m_nmi_pending)
	{
		m_bank_ram[0]->set_entry(0);
		m_bank0_rom.select(0);
		LOGMEM("mem: RAM0(0)");
	}
	else
	{
		const u8 rom_page = BIT(m_port_1ffd_data, 1)
			? ROM_PAGE_SYS
			: (((m_nmi_pending || dos()) << 1) | rom1());

		m_bank_rom[0]->set_entry(rom_page);
		m_bank0_rom.disable();
		LOGMEM("mem: ROM0(%x)", m_bank_rom[0]->entry());
	}

	m_bank_ram[3]->set_entry(((((m_port_1ffd_data & 0xc0) >> 2) | (m_port_1ffd_data & 0x10) >> 1) | (m_port_7ffd_data & 0x07)) % m_ram_banks);
	LOGMEM(", RAM3(%x)\n", m_bank_ram[3]->entry());
}

INTERRUPT_GEN_MEMBER(scorpion_state::scorpion_interrupt)
{
	m_irq_on_timer->adjust(m_screen->time_until_pos(get_screen_area().top(), get_screen_area().left()) - m_screen->clocks_to_attotime(14344 * 2));
}

INPUT_CHANGED_MEMBER(scorpion_state::on_nmi)
{
	m_nmi_pending = newval;
}

void scorpion_state::do_nmi()
{
	m_beta->enable();
	scorpion_update_memory();
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	m_nmi_pending = 0;
}

u8 scorpion_state::port_ff_r()
{
	if (m_magic_lock && !machine().side_effects_disabled())
	{
		LOGIO("Registers lock released\n");
		m_magic_lock = 0;
	}
	return scorpion_state::floating_bus_r();
}

void scorpion_state::port_7ffd_w(u8 data)
{
	/* disable paging */
	if (BIT(m_port_7ffd_data, 5))
		return;

	m_port_7ffd_data = data;
	scorpion_update_memory();

	m_screen->update_now();
	m_screen_location = m_ram->pointer() + ((BIT(m_port_7ffd_data, 3) ? 7 : 5) << 14);
}

void scorpion_state::port_1ffd_w(u8 data)
{
	m_port_1ffd_data = data;
	scorpion_update_memory();
}

void scorpion_state::ay_address_w(u8 data)
{
	if ((m_mod_ay->read() == 1) && ((data & 0xfe) == 0xfe))
		m_ay_selected = data & 1;
	else
		m_ay[m_ay_selected]->address_w(data);
}

u8 scorpion_state::beta_neutral_r(offs_t offset)
{
	if (m_is_m1_even && (m_maincpu->total_cycles() & 1)) m_maincpu->eat_cycles(1);

	return m_program.read_byte(offset);
}

u8 scorpion_state::beta_enable_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (m_is_m1_even && (m_maincpu->total_cycles() & 1)) m_maincpu->eat_cycles(1);
		if (!dos() && rom1())
		{
			m_beta->enable();
			scorpion_update_memory();
		}
	}
	return m_program.read_byte(offset + 0x3d00);
}

u8 scorpion_state::beta_disable_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (m_is_m1_even && (m_maincpu->total_cycles() & 1)) m_maincpu->eat_cycles(1);
		if (m_nmi_pending)
		{
			do_nmi();
		}
		else if (dos())
		{
			m_beta->disable();
			scorpion_update_memory();
		}
	}
	return m_program.read_byte(offset + 0x4000);
}

void scorpion_state::scorpion_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankr(m_bank_rom[0]);
	map(0x0000, 0x3fff).view(m_bank0_rom);
	m_bank0_rom[0](0x0000, 0x3fff).bankrw(m_bank_ram[0]);

	map(0x4000, 0x7fff).bankr(m_bank_ram[1]).w(FUNC(scorpion_state::spectrum_128_ram_w<1>));
	map(0x8000, 0xbfff).bankr(m_bank_ram[2]).w(FUNC(scorpion_state::spectrum_128_ram_w<2>));
	map(0xc000, 0xffff).bankr(m_bank_ram[3]).w(FUNC(scorpion_state::spectrum_128_ram_w<3>));
}

void scorpion_state::scorpion_ioext(address_map &map)
{
	map.unmap_value_high();
	map(0x0022, 0x0022).select(0xffdc) // FE | xxxxxxxxxx1xxx10
		.rw(FUNC(scorpion_state::spectrum_ula_r), FUNC(scorpion_state::spectrum_ula_w));
	map(0x0023, 0x0023).mirror(0xffdc) // FF | xxxxxxxxxx1xxx11
		.r(FUNC(scorpion_state::port_ff_r));
	map(0x0021, 0x0021).mirror(0x13fdc) // 1FFD | 00xxxxxxxx1xxx01
		.w(FUNC(scorpion_state::port_1ffd_w));
	map(0x4021, 0x4021).mirror(0x13fdc) // 7FFD | 01xxxxxxxx1xxx01
		.w(FUNC(scorpion_state::port_7ffd_w));

	map(0xa021, 0xa021).mirror(0x11fdc) // BFFD | 101xxxxxxx1xxx01
		.lw8(NAME([this](u8 data) { m_ay[m_ay_selected]->data_w(data); }));
	map(0xe021, 0xe021).mirror(0x11fdc) // FFFD | 111xxxxxxx1xxx01
		.lr8(NAME([this]() { return m_ay[m_ay_selected]->data_r(); })).w(FUNC(scorpion_state::ay_address_w));

	// Mouse
	map(0xfadf, 0xfadf).lr8(NAME([this]() -> u8 { return 0x80 | (m_io_mouse[2]->read() & 0x07); }));
	map(0xfbdf, 0xfbdf).lr8(NAME([this]() -> u8 { return m_io_mouse[0]->read(); }));
	map(0xffdf, 0xffdf).lr8(NAME([this]() -> u8 { return ~m_io_mouse[1]->read(); }));
	map(0x0003, 0x0003) // 1F | xxxxxxxx0x0xxx11
		.select(0xff5c).lr8(NAME([this]() -> u8 { return (m_beta->state_r() & 0xc0) | 0x00; })); // TODO Kepmston Joystick

	// Shadow
	// DOS + xxxxxxxx0nnxxx11
	map(0x10003, 0x10003).mirror(0xff1c).rw(m_beta, FUNC(beta_disk_device::status_r), FUNC(beta_disk_device::command_w));
	map(0x10023, 0x10023).mirror(0xff1c).rw(m_beta, FUNC(beta_disk_device::track_r), FUNC(beta_disk_device::track_w));
	map(0x10043, 0x10043).mirror(0xff1c).rw(m_beta, FUNC(beta_disk_device::sector_r), FUNC(beta_disk_device::sector_w));
	map(0x10063, 0x10063).mirror(0xff1c).rw(m_beta, FUNC(beta_disk_device::data_r), FUNC(beta_disk_device::data_w));
	map(0x100e3, 0x100e3).mirror(0xff1c).rw(m_beta, FUNC(beta_disk_device::state_r), FUNC(beta_disk_device::param_w));
}

void scorpion_state::scorpion_io(address_map &map)
{
	map(0x0000, 0xffff).lrw8(
		NAME([this](offs_t offset) { return m_ioext.read_byte((dos() << 16) | offset); }),
		NAME([this](offs_t offset, u8 data) { m_ioext.write_byte((dos() << 16) | offset, data); }));
}

void scorpion_state::scorpion_switch(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(scorpion_state::beta_neutral_r)); // Overlap with previous because we want real addresses on the 3e00-3fff range
	map(0x3d00, 0x3dff).r(FUNC(scorpion_state::beta_enable_r));
	map(0x4000, 0xffff).r(FUNC(scorpion_state::beta_disable_r));
}

void scorpion_state::machine_start()
{
	spectrum_128_state::machine_start();

	save_item(NAME(m_port_1ffd_data));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_magic_lock));
	save_item(NAME(m_ay_selected));
	save_item(NAME(m_ram_banks));

	m_maincpu->space(AS_PROGRAM).specific(m_program);
	m_bankio->space(AS_PROGRAM).specific(m_ioext);

	// reconfigure ROMs
	memory_region *rom = memregion("maincpu");
	m_bank_rom[0]->configure_entries(0, rom->bytes() / 0x4000, rom->base() + 0x10000, 0x4000);
	m_ram_banks = m_ram->size() / 0x4000;
	m_bank_ram[0]->configure_entries(0, m_ram_banks, m_ram->pointer(), 0x4000);
	m_bank_ram[2]->configure_entries(0, m_ram_banks, m_ram->pointer(), 0x4000);
}

void scorpion_state::machine_reset()
{
	m_is_m1_even = 1;
	m_nmi_pending = 0;
	m_magic_lock = 0;
	m_ay_selected = 0;
	m_beta->disable();

	m_port_fe_data = 255;
	m_port_7ffd_data = 0;
	m_port_1ffd_data = 0;

	m_bank_ram[2]->set_entry(2);

	scorpion_update_memory();
}

void scorpion_state::video_start()
{
	spectrum_state::video_start();
	m_screen_location = m_ram->pointer() + (5 << 14);
	m_contention_pattern = {};
}

/* F4 Character Displayer */
static const gfx_layout spectrum_charlayout =
{
	8, 8,          /* 8 x 8 characters */
	96,            /* 96 characters */
	1,             /* 1 bits per pixel */
	{ 0 },         /* no bitplanes */
	{STEP8(0, 1)}, /* x offsets */
	{STEP8(0, 8)}, /* y offsets */
	8*8            /* every char takes 8 bytes */
};

static const gfx_layout quorum_charlayout =
{
	8, 8,          /* 8 x 8 characters */
	160,           /* 160 characters */
	1,             /* 1 bits per pixel */
	{ 0 },         /* no bitplanes */
	{STEP8(0, 1)}, /* x offsets */
	{STEP8(0, 8)}, /* y offsets */
	8*8            /* every char takes 8 bytes */
};

static const gfx_layout profi_8_charlayout =
{
	8, 8,          /* 8 x 8 characters */
	224,           /* 224 characters */
	1,             /* 1 bits per pixel */
	{ 0 },         /* no bitplanes */
	{STEP8(0, 1)}, /* x offsets */
	{STEP8(0, 8)}, /* y offsets */
	8*8            /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_scorpion )
	GFXDECODE_ENTRY( "maincpu", 0x17d00, spectrum_charlayout, 7, 8 )
GFXDECODE_END

static GFXDECODE_START( gfx_profi )
	GFXDECODE_ENTRY( "maincpu", 0x17d00, spectrum_charlayout, 7, 8 )
	GFXDECODE_ENTRY( "maincpu", 0x1abfc, profi_8_charlayout, 7, 8 )
	/* There are more characters after this, that haven't been decoded */
GFXDECODE_END

static GFXDECODE_START( gfx_quorum )
	GFXDECODE_ENTRY( "maincpu", 0x1fb00, quorum_charlayout, 7, 8 )
GFXDECODE_END

rectangle scorpion_state::get_screen_area()
{
	return spectrum_state::get_screen_area();
}

INPUT_PORTS_START( scorpion )
	PORT_INCLUDE( spec_plus )

	PORT_MODIFY("NMI")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("NMI") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, scorpion_state, on_nmi, 0)


	PORT_START("mouse_input1")
	PORT_BIT(0xff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(30)

	PORT_START("mouse_input2")
	PORT_BIT(0xff, 0, IPT_MOUSE_Y) PORT_SENSITIVITY(30)

	PORT_START("mouse_input3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Left mouse button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("Right mouse button") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Middle mouse button") PORT_CODE(MOUSECODE_BUTTON3)


	PORT_START("MOD_AY")
	PORT_CONFNAME(0x01, 0x00, "AY MOD")
	PORT_CONFSETTING(0x00, "Single")
	PORT_CONFSETTING(0x01, "TurboSound")
INPUT_PORTS_END

void scorpion_state::scorpion(machine_config &config)
{
	// Yellow PCB
	spectrum(config);
	m_ram->set_default_size("256K");

	m_maincpu->set_memory_map(&scorpion_state::scorpion_mem);
	m_maincpu->set_m1_map(&scorpion_state::scorpion_switch);
	m_maincpu->set_io_map(&scorpion_state::scorpion_io);
	m_maincpu->set_vblank_int("screen", FUNC(scorpion_state::scorpion_interrupt));
	m_maincpu->nomreq_cb().set_nop();

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_scorpion);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	AY8912(config, m_ay[0], 14_MHz_XTAL / 8) // BAC
		.add_route(1, "lspeaker", 0.50)
		.add_route(0, "lspeaker", 0.25)
		.add_route(0, "rspeaker", 0.25)
		.add_route(2, "rspeaker", 0.50);
	AY8912(config, m_ay[1], 14_MHz_XTAL / 8)
		.add_route(1, "lspeaker", 0.50)
		.add_route(0, "lspeaker", 0.25)
		.add_route(0, "rspeaker", 0.25)
		.add_route(2, "rspeaker", 0.50);

	BETA_DISK(config, m_beta, 0);

	config.device_remove("exp");

	ADDRESS_MAP_BANK(config, m_bankio).set_map(&scorpion_state::scorpion_ioext).set_options(ENDIANNESS_LITTLE, 8, 17, 0);

	zxbus_device &zxbus(ZXBUS(config, "zxbus", 0));
	zxbus.set_iospace(m_bankio, AS_PROGRAM);
	ZXBUS_SLOT(config, "zxbus:1", 0, "zxbus", zxbus_cards, nullptr);
	ZXBUS_SLOT(config, "zxbus:2", 0, "zxbus", zxbus_cards, nullptr);
}

void scorpion_state::profi(machine_config &config)
{
	scorpion(config);
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_profi);
}

void scorpion_state::quorum(machine_config &config)
{
	scorpion(config);
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_quorum);
}


/******************************************************************************
 * Scorpion ZS-256 Turbo+
 * ***************************************************************************/
void scorpiontb_state::ay_address_w(u8 data)
{
	m_ay_reg = data;
	scorpion_state::ay_address_w(data);
}

u8 scorpiontb_state::ay_data_r()
{
	return (m_ay_reg == 0x0e)
		? (((m_port_7ffd_data & 0x10) << 1) | (m_port_1ffd_data & 0x10) | (m_port_7ffd_data & 0x0f))
		: m_ay[m_ay_selected]->data_r();
}

INPUT_CHANGED_MEMBER(scorpiontb_state::turbo_changed)
{
	if (newval == 1)
	{
		m_turbo = !m_turbo;
		m_maincpu->set_clock_scale(1 << m_turbo);
		popmessage("Turbo %s\n", m_turbo ? "ON" : "OFF");
	}
}

INPUT_PORTS_START( scorpiontb )
	PORT_INCLUDE( scorpion )

	PORT_START("TURBO")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TURBO") PORT_CODE(KEYCODE_F11) PORT_CHANGED_MEMBER(DEVICE_SELF, scorpiontb_state, turbo_changed, 0)
INPUT_PORTS_END

void scorpiontb_state::scorpiontb(machine_config &config)
{
	scorpion(config);

	rectangle visarea = { get_screen_area().left() - SPEC_LEFT_BORDER, get_screen_area().right() + SPEC_RIGHT_BORDER,
		get_screen_area().top() - SPEC_TOP_BORDER, get_screen_area().bottom() + SPEC_BOTTOM_BORDER };
	m_screen->set_raw(X1 / 2, SPEC_CYCLES_PER_LINE * 2, SPEC_UNSEEN_LINES + SPEC_SCREEN_HEIGHT + 4, visarea);

	m_ram->set_default_size("256K").set_extra_options("1M");
}

void scorpiontb_state::scorpion_update_memory()
{
	LOGMEM("romplane %x, ", m_prof_plane);
	scorpion_state::scorpion_update_memory();
	m_bank_rom[0]->set_entry((m_prof_plane << 2) | m_bank_rom[0]->entry());
}

void scorpiontb_state::machine_start()
{
	scorpion_state::machine_start();

	save_item(NAME(m_prof_plane));
	save_item(NAME(m_ay_reg));
	save_item(NAME(m_turbo));
}

void scorpiontb_state::machine_reset()
{
	m_prof_plane = 0;
	m_ay_reg = -1;

	scorpion_state::machine_reset();
	m_is_m1_even = 0;
	m_turbo = 0;
	m_maincpu->set_clock_scale(1);
}

static const u8 prof_plane_map[] =
{
	0, 1, 2, 3,
	3, 3, 3, 2,
	2, 2, 0, 1,
	1, 0, 1, 0,
};

void scorpiontb_state::video_start()
{
	scorpion_state::video_start();

	address_space &prg = m_maincpu->space(AS_PROGRAM);
	prg.install_read_tap(0x0100, 0x010c, "plane_switch", [this](offs_t offset, u8 &data, u8 mem_mask)
	{
		if (!machine().side_effects_disabled() && !romram() && ((m_bank_rom[0]->entry() & 3) == ROM_PAGE_SYS))
		{
			const u8 offs = offset & 0x000f;
			if ((offs == 0) || (offs == 4) || (offs == 8) || (offs == 0x0c))
			{
				u8 plane = prof_plane_map[(offset & 0x0c) | (m_prof_plane & 0x03)];
				if (m_prof_plane != plane)
				{
					m_prof_plane = plane;
					scorpion_update_memory();
				}
			}
		}
	});
}

void scorpiontb_state::scorpion_ioext(address_map &map)
{
	scorpion_state::scorpion_ioext(map);
	map(0x0021, 0x0021).mirror(0x13fdc) // 1FFD | 00xxxxxxxx1xxx01
		.lr8(NAME([this](offs_t offset) -> u8 { m_turbo = 0; m_maincpu->set_clock_scale(1); return 0xff; }));
	map(0x4021, 0x4021).mirror(0x13fdc) // 7FFD | 01xxxxxxxx1xxx01
		.lr8(NAME([this](offs_t offset) -> u8 { m_turbo = 1; m_maincpu->set_clock_scale(2); return 0xff; }));
	map(0xe021, 0xe021).mirror(0x11fdc) // FFFD | 111xxxxxxx1xxx01
		.rw(FUNC(scorpiontb_state::ay_data_r),  FUNC(scorpiontb_state::ay_address_w));

	// Centronics
	map(0x0001, 0x0001).mirror(0xffdc).nopw();
}


/******************************************************************************
 * Scorpion GMX
 * ***************************************************************************/
void scorpiongmx_state::scorpion_update_memory()
{
	scorpiontb_state::scorpion_update_memory();
	if (BIT(m_port_1ffd_data, 2))
	{
		m_bank_rom[0]->set_entry((m_prof_plane << 2) | ROM_PAGE_DOS);
		m_bank0_rom.disable();
		m_beta->enable();
		LOGMEM("... ROM0(%x)", m_bank_rom[0]->entry());
	}
	m_bank_ram[2]->set_entry((m_port_78fd_data ^ 2) % m_ram_banks);
	LOGMEM("... RAM2(%x)", m_bank_ram[2]->entry());
	m_bank_ram[3]->set_entry((((m_port_dffd_data & 7) << 4) | ((m_port_1ffd_data & 0x10) >> 1) | (m_port_7ffd_data & 0x07)) % m_ram_banks);
	LOGMEM(", RAM3(%x)\n", m_bank_ram[3]->entry());
}

void scorpiongmx_state::spectrum_update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_gfx_ext)
		return scorpiontb_state::spectrum_update_screen(screen, bitmap, cliprect);

	// 640x200 ...
	spectrum_update_border(screen, bitmap, cliprect);

	const bool invert_attrs = u64(screen.frame_number() / m_frame_invert_count) & 1;
	const u8 *screen_location = m_ram->pointer() + ((BIT(m_port_7ffd_data, 3) ? 0x3b : 0x39) << 14);
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		const u16 y = (vpos - get_screen_area().top() + (((m_scroll_hi << 8) | m_scroll_lo) / 80)) % (25 * 8);
		for (u16 hpos = cliprect.left() & 0xfff8; hpos <= cliprect.right();)
		{
			const u16 x = hpos - get_screen_area().left();
			const u8 *scr = screen_location + (y * 80) + (x >> 3);

			const u8 attr = *(scr + (0x40 << 14));
			const u16 fg = ((attr >> 3) & 0x08) | (attr & 0x07);
			const u16 bg = (attr >> 3) & 0x0f;
			const bool invert = (attr & 0x80) && invert_attrs;

			const u8 chunk = *scr;
			for (u8 i = 0x80; i; i >>= 1)
				bitmap.pix(vpos, hpos++) = (chunk & i)
					? (invert ? bg : fg)
					: (invert ? fg : bg);
		}
	}
}

rectangle scorpiongmx_state::get_screen_area()
{
	return m_gfx_ext
		? rectangle { 80, 80 + 639, 80, 80 + 199 }
		: scorpiontb_state::get_screen_area();
}

void scorpiongmx_state::do_nmi()
{
	if (m_magic_disabled)
	{
		LOGIO("NMI ignored as Magic disabled");
		m_nmi_pending = 0;
	}
	else
	{
		m_port_7afd_lock_data = port_7afd_r() & 0x7f;
		m_port_7efd_lock_data = port_7efd_r() & 0x4f;
		LOGIO("NMI: registers locked 7afd=%02x, 7efd=%02x\n", m_port_7afd_lock_data, m_port_7efd_lock_data);
		scorpion_state::do_nmi();
		m_magic_lock = 1;
	}
}

void scorpiongmx_state::global_cfg_w(u8 data)
{
	m_port_00_data = data;
	if (BIT(data, 5)) // BLKEXT
	{
		LOGIO("GMX ports disabled\n");
		m_io_gmx.disable();
	}
	else
		m_io_gmx.select(0);

	if (BIT(data, 3))
	{
		m_magic_shift = 0x88 | (data & 7);
		if (!fixrom())
			m_maincpu->reset();
	}
}

u8 scorpiongmx_state::port_78fd_r()
{
	u8 tmp = (BIT(m_port_fe_data, 1) << 7) // BRD1
		| (m_port_78fd_data & 0x7f);
	tmp |= (m_magic_shift & 1);

	if(!machine().side_effects_disabled())
		m_magic_shift >>= 1;

	return tmp;
}

u8 scorpiongmx_state::port_7afd_r()
{
	u8 data;
	if (m_magic_lock)
		data = m_port_7afd_lock_data;
	else
		data = (BIT(m_port_dffd_data, 0, 3) << 4) | (BIT(m_port_1ffd_data, 4) << 3) | BIT(m_port_7ffd_data, 0, 3);

	data |= BIT(m_port_fe_data, 0) << 7; // BRD0

	return data;
}

u8 scorpiongmx_state::port_7efd_r()
{
	u8 data;
	if (m_magic_lock)
		data = m_port_7efd_lock_data;
	else
		data = (BIT(m_port_1ffd_data, 0) << 6)
			| (m_gfx_ext << 3)
			| (m_turbo << 2)
			| (BIT(m_port_7ffd_data, 3) << 1)
			| BIT(m_port_7ffd_data, 5);

	data |= BIT(m_port_fe_data, 2) << 7 // BRD2
		| (BIT(m_port_00_data, 5) << 5)
		| (BIT(m_port_00_data, 7) << 4);

	return data;
}

void scorpiongmx_state::port_7efd_w(u8 data)
{
	m_turbo = BIT(data, 7);
	m_maincpu->set_clock_scale(1 << m_turbo);

	if (!fixrom())
	{
		m_prof_plane = BIT(data, 4, 3); // D4..6 - A16..18: 28F400
		scorpion_update_memory();
	}

	m_gfx_ext = BIT(data, 3);
	const rectangle scr = get_screen_area();
	const rectangle broder = m_gfx_ext
		? rectangle {24, 24, 40, 40}
		: rectangle {SPEC_LEFT_BORDER, SPEC_RIGHT_BORDER, SPEC_TOP_BORDER, SPEC_BOTTOM_BORDER};
	m_screen->configure((SPEC_CYCLES_PER_LINE * 2) << m_gfx_ext, m_screen->height()
		, {scr.left() - broder.left(), scr.right() + broder.right()
		, scr.top() - broder.top(), scr.bottom() + broder.bottom()}
		, m_screen->frame_period().as_attoseconds());

	m_magic_disabled = BIT(data, 2);

	// D1 - Vpp: 28F400 PowerOn
	// D0 - EWR: 28F400 W
}

void scorpiongmx_state::scorpiongmx(machine_config &config)
{
	scorpiontb(config);
	m_ram->set_default_size("2M");
}

void scorpiongmx_state::machine_start()
{
	scorpiontb_state::machine_start();

	save_item(NAME(m_gfx_ext));
	save_item(NAME(m_magic_disabled));
	save_item(NAME(m_port_00_data));
	save_item(NAME(m_port_78fd_data));
	save_item(NAME(m_port_7afd_lock_data));
	save_item(NAME(m_scroll_lo));
	save_item(NAME(m_scroll_hi));
	save_item(NAME(m_port_7efd_lock_data));
	save_item(NAME(m_port_dffd_data));

	m_port_00_data = 0;
}

void scorpiongmx_state::machine_reset()
{
	m_io_gmx.select(0);

	m_magic_shift = 0;
	m_gfx_ext = 0;
	m_magic_disabled = 0;
	m_port_78fd_data = 0;
	m_port_7afd_lock_data = 0;
	m_scroll_lo = 0;
	m_scroll_hi = 0;
	m_port_7efd_lock_data = 0;
	m_port_dffd_data = 0;

	scorpiontb_state::machine_reset();
}

void scorpiongmx_state::video_start()
{
	scorpion_state::video_start();
}

void scorpiongmx_state::scorpion_ioext(address_map &map)
{
	scorpiontb_state::scorpion_ioext(map);
	map(0x0000, 0x0000).mirror(0xff00).w(FUNC(scorpiongmx_state::global_cfg_w));

	map(0x0000, 0x1ffff).view(m_io_gmx);
	m_io_gmx[0](0x78fd, 0x78fd).mirror(0x10000).r(FUNC(scorpiongmx_state::port_78fd_r))
		.lw8(NAME([this](u8 data) { m_port_78fd_data = data & 0x7f; scorpion_update_memory(); }));
	m_io_gmx[0](0x7afd, 0x7afd).mirror(0x10000).r(FUNC(scorpiongmx_state::port_7afd_r))
		.lw8(NAME([this](u8 data) { m_scroll_lo = data & 0xf0; }));
	m_io_gmx[0](0x7cfd, 0x7cfd).mirror(0x10000)
		.lw8(NAME([this](u8 data) { m_scroll_hi = data & 0x3f; }));
	m_io_gmx[0](0x7efd, 0x7efd).mirror(0x10000).rw(FUNC(scorpiongmx_state::port_7efd_r), FUNC(scorpiongmx_state::port_7efd_w));
	m_io_gmx[0](0xdffd, 0xdffd).mirror(0x10000)
		.lw8(NAME([this](u8 data) { m_port_dffd_data = data & 0x07; scorpion_update_memory(); }));
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(scorpio)
	ROM_REGION(0x90000, "maincpu", 0)
	ROM_DEFAULT_BIOS("v3")

	ROM_SYSTEM_BIOS(0, "v1", "V.2.92")
	ROMX_LOAD("scorp0.rom", 0x010000, 0x4000, CRC(0eb40a09) SHA1(477114ff0fe1388e0979df1423602b21248164e5), ROM_BIOS(0))
	ROMX_LOAD("scorp1.rom", 0x014000, 0x4000, CRC(9d513013) SHA1(367b5a102fb663beee8e7930b8c4acc219c1f7b3), ROM_BIOS(0))
	ROMX_LOAD("scorp2.rom", 0x018000, 0x4000, CRC(fd0d3ce1) SHA1(07783ee295274d8ff15d935bfd787c8ac1d54900), ROM_BIOS(0))
	ROMX_LOAD("scorp3.rom", 0x01c000, 0x4000, CRC(1fe1d003) SHA1(33703e97cc93b7edfcc0334b64233cf81b7930db), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2", "V.2.92 joined")
	ROMX_LOAD( "scorpion.rom", 0x010000, 0x10000, CRC(fef73c28) SHA1(66cecdadf992d8adb9c66deee929eb56600dc9bc), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v3", "V.2.94")
	ROMX_LOAD( "scorp294.rom", 0x010000, 0x10000, CRC(99f57ce1) SHA1(083bb57ad52cc871b92d3e1794fd9790872c3584), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v4", "NeOS 256")
	ROMX_LOAD( "neos_256.rom", 0x010000, 0x4000, CRC(364ae09a) SHA1(bb6db1947415503a6bc48f33c603fb3a0dbb3690), ROM_BIOS(3))
	ROMX_LOAD( "scorp1.rom",   0x014000, 0x4000, CRC(9d513013) SHA1(367b5a102fb663beee8e7930b8c4acc219c1f7b3), ROM_BIOS(3))
	ROMX_LOAD( "scorp2.rom",   0x018000, 0x4000, CRC(fd0d3ce1) SHA1(07783ee295274d8ff15d935bfd787c8ac1d54900), ROM_BIOS(3))
	ROMX_LOAD( "scorp3.rom",   0x01c000, 0x4000, CRC(1fe1d003) SHA1(33703e97cc93b7edfcc0334b64233cf81b7930db), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "scorp_test", "Scorpion Test")
	ROMX_LOAD( "scorp_test.rom", 0x010000, 0x10000, CRC(e0230ca7) SHA1(f38e4d23cb29b4ae3fe8b00a52d7b0f9bb845407), ROM_BIOS(4))

	// This is a ROM for Scorpion emulator which modified in order to hide real hardware details
	//ROMX_LOAD( "scorp402.rom", 0x010000, 0x20000, CRC(9fcf893d) SHA1(0cc7ba60f5cfc36e75bd3a5c90e26b2a1905a970))

	ROM_REGION(0x01000, "keyboard", 0)
	ROM_LOAD( "scrpkey.rom", 0x0000, 0x1000, CRC(e938a510) SHA1(2753993c97ff0fc6cff26ed792929abc1288dc6f))

	ROM_REGION(0x010000, "gsound", 0) // TODO GS connected to ZXBUS
	ROM_LOAD( "gs104.rom", 0x0000, 0x8000, CRC(7a365ba6) SHA1(c865121306eb3a7d811d82fbcc653b4dc1d6fa3d))
	ROM_LOAD( "gs105a.rom", 0x8000, 0x8000, CRC(1cd490c6) SHA1(1ba78923dc7d803e7995c165992e14e4608c2153))
ROM_END

ROM_START(scorpiontb)
	ROM_REGION(0x90000, "maincpu", 0)
	ROM_DEFAULT_BIOS("v4.41lg_uni")

	ROM_SYSTEM_BIOS(0, "v3.9f", "ProfROM V.3.9f")
	ROMX_LOAD( "prof_39f.rom", 0x010000, 0x20000, CRC(c55e64da) SHA1(cec7770fe26350f57f6c325a29db78787dc4521e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v4.01_01", "ProfROM V.4.01 (No ROM disk)")
	ROMX_LOAD( "scorp401_0358765b.rom", 0x010000, 0x40000, CRC(0358765b) SHA1(414c21a4d01fafb6ee1752a51b79bb2f82091625), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v4.01_02", "ProfROM V.4.01 (ZxUnZip, ZxZip, TASM 4.0, SM 2.01, HD Copy 1.7, Test FDD)")
	ROMX_LOAD( "scorp401_520f4c15.rom", 0x010000, 0x40000, CRC(520f4c15) SHA1(8a877b06e69e829b99e06be0ba48bc0fe6458672), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v4.01_03", "ProfROM V.4.01 (Fatall 0.25, SCBoot 1.5, MagOS 6.3)")
	ROMX_LOAD( "scorp401_749516c5.rom", 0x010000, 0x40000, CRC(749516c5) SHA1(712535c4eec26224f39650e4740db131b2d0ed14), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v4.01_04", "ProfROM V.4.01 (Fatall 0.2)")
	ROMX_LOAD( "scorp401_809c249d.rom", 0x010000, 0x40000, CRC(809c249d) SHA1(1e7ea10aeadb33a7d363809ccf31d43a0a8670ac), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v4.01_05", "ProfROM V.4.01 (ZxUnZip, ZxZip, TASM 4.0, SM 2.01, HD Copy 1.7, TestFDD)")
	ROMX_LOAD( "scorp401_847a66e4.rom", 0x010000, 0x40000, CRC(847a66e4) SHA1(a01522cadf18c087f32c1997c5c2e889290921fa), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v4.01_06", "ProfROM V.4.01 (MagOS 6.3, Real Comm 2.6F, TRDNavig 0.75b, ZXunzip 1.2x, Cat HDD, Test ...)")
	ROMX_LOAD( "scorp401_91f513ab.rom", 0x010000, 0x40000, CRC(91f513ab) SHA1(e2b8941591e77c1d35b811a6724b31ff3486d42d), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "v4.01_07", "ProfROM V.4.01 (Commander, ZWord, MAGOS 6.3, STS 5.1, ADS 2.01, FPM 3.05, Ztool)")
	ROMX_LOAD( "scorp401_98275049.rom", 0x010000, 0x40000, CRC(98275049) SHA1(a7518915293a82b70b2fd5261ef886c4dd33fa9b), ROM_BIOS(7))
	ROM_SYSTEM_BIOS(8, "v4.01_08", "ProfROM V.4.01 (Fatall 0.25, Test Scorpion, TASM 4.0, SCBoot, Chess Psion, Tetris, Chuckie Egg, Bomber)")
	ROMX_LOAD( "scorp401_a3b10c26.rom", 0x010000, 0x40000, CRC(a3b10c26) SHA1(945a671183402eee99f852eab93e28a93a81b575), ROM_BIOS(8))
	ROM_SYSTEM_BIOS(9, "v4.01_09", "ProfROM V.4.01 (Fatall 0.25, SCBoot 1.5, MagOS 6.3, Real Command 2.6)")
	ROMX_LOAD( "scorp401_e5476776.rom", 0x010000, 0x40000, CRC(e5476776) SHA1(3dcff254f5bbc1e6c6a5319d21ebbd5c7d65077c), ROM_BIOS(9))
	ROM_SYSTEM_BIOS(10, "v4.01_10", "ProfROM V.4.01 (Fatall 0.25, SCBoot 1.5, Wild Player 3.3, Best View 2.19)")
	ROMX_LOAD( "scorp401_e8900ba7.rom", 0x010000, 0x40000, CRC(e8900ba7) SHA1(5caa403036cc2268db2c35077902e3f432f3c779), ROM_BIOS(10))
	ROM_SYSTEM_BIOS(11, "v4.15", "ProfROM V.4.xx.015")
	ROMX_LOAD( "prof4xx015.rom", 0x010000, 0x40000, CRC(5d4ba991) SHA1(2d1f0bd95909cdff32a96ab55b4bcae547e20bd1), ROM_BIOS(11))
	ROM_SYSTEM_BIOS(12, "v4.15lg", "ProfROM V.4.xx.015 (Basic Looking Glass)")
	ROMX_LOAD( "prof4xx015lg.rom", 0x010000, 0x40000, CRC(3b450ceb) SHA1(e1af7110ad89764ac020c636ab835c39ada061c0), ROM_BIOS(12))
	ROM_SYSTEM_BIOS(13, "v4.27", "ProfROM V.4.xx.027")
	ROMX_LOAD( "prof4xx027.rom", 0x010000, 0x40000, CRC(8da39bed) SHA1(39a601d8af62df95efd5a514b53a4a07571befab), ROM_BIOS(13))
	ROM_SYSTEM_BIOS(14, "v4.27lg", "ProfROM V.4.xx.027 (Basic Looking Glass)")
	ROMX_LOAD( "prof4xx027lg.rom", 0x010000, 0x40000, CRC(50be8b57) SHA1(fa930ec7efe8eee641c889a0ce2ae16f03ebc1f4), ROM_BIOS(14))
	ROM_SYSTEM_BIOS(15, "v4.29", "ProfROM V.4.xx.029")
	ROMX_LOAD( "prof4xx029.rom", 0x010000, 0x40000, CRC(aebe12a3) SHA1(31c5481587554a8173016ccda46e1490cbd9fa5c), ROM_BIOS(15))
	ROM_SYSTEM_BIOS(16, "v4.29lg", "ProfROM V.4.xx.029 (Basic Looking Glass)")
	ROMX_LOAD( "prof4xx029lg.rom", 0x010000, 0x40000, CRC(1ae71a4c) SHA1(c7d8f20134f5623f2498feea5c9efbcb2fd686a3), ROM_BIOS(16))
	ROM_SYSTEM_BIOS(17, "v4.31", "ProfROM V.4.xx.031")
	ROMX_LOAD( "prof4xx031.rom", 0x010000, 0x40000, CRC(43ab9b83) SHA1(cc95edf10ee6bfd16cf738ea648cf4ce35b3b232), ROM_BIOS(17))
	ROM_SYSTEM_BIOS(18, "v4.31lg", "ProfROM V.4.xx.031 (Basic Looking Glass)")
	ROMX_LOAD( "prof4xx031lg.rom", 0x010000, 0x40000, CRC(f7f2936c) SHA1(61f8ca193624c7ed23d54f35cbebb76bc94e96e2), ROM_BIOS(18))
	ROM_SYSTEM_BIOS(19, "v4.31_3d2f", "ProfROM V.4.xx.031(3D2F)")
	ROMX_LOAD( "prof4xx031_3d2f.rom", 0x010000, 0x40000, CRC(e850e0d1) SHA1(48d41013130a5d92c2b2fd04142d5ba6ce5a324a), ROM_BIOS(19))
	ROM_SYSTEM_BIOS(20, "v4.31lg_3d2f", "ProfROM V.4.xx.031 (Basic Looking Glass) (3D2F)")
	ROMX_LOAD( "prof4xx031lg_3d2f.rom", 0x010000, 0x40000, CRC(cc525181) SHA1(985b437e8b35c0c3493a347e76f683e498bd40e4), ROM_BIOS(20))
	ROM_SYSTEM_BIOS(21, "v4.41", "ProfROM V.4.xx.041.8689")
	ROMX_LOAD( "prof4xx041.8689.rom", 0x010000, 0x40000, CRC(35f11072) SHA1(25f8dcd04619e44829e3ba4cb360f2ae9bf8bf01), ROM_BIOS(21))
	ROM_SYSTEM_BIOS(22, "v4.41_3d2f", "ProfROM V.4.xx.041.8689 (3D2F)")
	ROMX_LOAD( "prof4xx041.8689_3d2f.rom", 0x010000, 0x40000, CRC(25c5885a) SHA1(972871f1b19bc2980ec22220d81994223bf6e884), ROM_BIOS(22))
	ROM_SYSTEM_BIOS(23, "v4.41_uni", "ProfROM V.4.xx.041.8689 (UNI)")
	ROMX_LOAD( "prof4xx041.8689_uni.rom", 0x010000, 0x40000, CRC(3914e0aa) SHA1(2a951aa49ccdecf38adc8c901296b352264d621f), ROM_BIOS(23))
	ROM_SYSTEM_BIOS(24, "v4.41lg", "ProfROM V.4.xx.041.8689 (Basic Looking Glass)")
	ROMX_LOAD( "prof4xx041.8689lg.rom", 0x010000, 0x40000, CRC(81a8189d) SHA1(3991c4bf9ab7d184a86f413753f954e43f7337fa), ROM_BIOS(24))
	ROM_SYSTEM_BIOS(25, "v4.41lg_3d2f", "ProfROM V.4.xx.041.8689 (Basic Looking Glass) (3D2F)")
	ROMX_LOAD( "prof4xx041.8689lg_3d2f.rom", 0x010000, 0x40000, CRC(01c7390a) SHA1(84d4518d751979722fde834c568fa652058e55fa), ROM_BIOS(25))
	ROM_SYSTEM_BIOS(26, "v4.41lg_uni", "ProfROM V.4.xx.041.8689 (Basic Looking Glass) (UNI)")
	ROMX_LOAD( "prof4xx041.8689lg_uni.rom", 0x010000, 0x40000, CRC(62df1e38) SHA1(6df006eb8429464c54097f10b6ef2731c52f91b9), ROM_BIOS(26))
	ROM_SYSTEM_BIOS(27, "scorp_test", "Scorpion Test")
	ROMX_LOAD( "scorp_test.rom", 0x010000, 0x10000, CRC(e0230ca7) SHA1(f38e4d23cb29b4ae3fe8b00a52d7b0f9bb845407), ROM_BIOS(27))
ROM_END

ROM_START(scorpiongmx)
	ROM_REGION(0x90000, "maincpu", 0)
	ROM_DEFAULT_BIOS("v6.41_uni")

	ROM_SYSTEM_BIOS(0, "v12500", "GMX Boot Rom 1.2 V. 5.00")
	ROMX_LOAD( "gmx12500.rom", 0x010000, 0x80000, CRC(00df8568) SHA1(dd303d298f96ec4f9fe736eefd3c36fff1dfcdf5), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v12501", "GMX Boot Rom 1.2 V. 5.01")
	ROMX_LOAD( "gmx12501.rom", 0x010000, 0x80000, CRC(34c8d210) SHA1(433237c8b5cd9de3bff3b074b6c7065788306995), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v13500", "GMX Boot Rom 1.3 V. 5.00")
	ROMX_LOAD( "gmx13500.rom", 0x010000, 0x80000, CRC(47c9df88) SHA1(e26823989ce111a086a9e44bfa07fa631019c19d), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v5.27", "ProfROM +GMX V.5.xx.027")
	ROMX_LOAD( "profgmx5xx027.rom", 0x010000, 0x80000, CRC(05dc16f5) SHA1(b786c78e9f332010e3996f3281adf3f75924bede), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v5.29", "ProfROM +GMX V.5.xx.029")
	ROMX_LOAD( "profgmx5xx029.rom", 0x010000, 0x80000, CRC(41016b99) SHA1(86f324d45f4f181c9231a7001b30d5ce3d1512c6), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v5.30", "ProfROM +GMX V.5.xx.030")
	ROMX_LOAD( "profgmx5xx030.rom", 0x010000, 0x80000, CRC(21edd18e) SHA1(90fc58600e2da85fea7dc033a22dcc88633530bc), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v5.30_3d2f", "ProfROM +GMX V.5.xx.030 (3D2F)")
	ROMX_LOAD( "profgmx5xx030_3d2f.rom", 0x010000, 0x80000, CRC(ab6e2380) SHA1(6bd2904364badb00700a7d4576d8396c62916d67), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "v5.31", "ProfROM +GMX V.5.xx.031")
	ROMX_LOAD( "profgmx5xx031.rom", 0x010000, 0x80000, CRC(a2ec83d5) SHA1(e2a022c1bdea516b43828752fc52b112c0bad59c), ROM_BIOS(7))
	ROM_SYSTEM_BIOS(8, "v5.31_3d2f", "ProfROM +GMX V.5.xx.031 (3D2F)")
	ROMX_LOAD( "profgmx5xx031_3d2f.rom", 0x010000, 0x80000, CRC(bb900382) SHA1(1e01444e3eb65dffcb9c41b2b8e034a020198a1e), ROM_BIOS(8))
	ROM_SYSTEM_BIOS(9, "v5.41", "ProfROM +GMX V.5.xx.041.8689")
	ROMX_LOAD( "profgmx5xx041.8689.rom", 0x010000, 0x80000, CRC(fefc285e) SHA1(63ed6e18f8d9cb203c7279f3ecb431950c5b83e6), ROM_BIOS(9))
	ROM_SYSTEM_BIOS(10, "v5.41_3d2f", "ProfROM +GMX V.5.xx.041.8689 (3D2F)")
	ROMX_LOAD( "profgmx5xx041.8689_3d2f.rom", 0x010000, 0x80000, CRC(a2923935) SHA1(1b8cfe2b8e9dab07dc1852c84f36049d6c27cd93), ROM_BIOS(10))
	ROM_SYSTEM_BIOS(11, "v5.41_uni", "ProfROM +GMX V.5.xx.041.8689 (UNI)")
	ROMX_LOAD( "profgmx5xx041.8689_uni.rom", 0x010000, 0x80000, CRC(9b5f6b2b) SHA1(3c2628b5acdbb4c954b8607c2971b66d9f3ccc2a), ROM_BIOS(11))
	ROM_SYSTEM_BIOS(12, "v6.41", "ProfROM +GMX V.6.xx.041.8689")
	ROMX_LOAD( "profgmx6xx041.8689.rom", 0x010000, 0x80000, CRC(c2c6b073) SHA1(bff736c5d0389e171986f0f1c853319bbf704737), ROM_BIOS(12))
	ROM_SYSTEM_BIOS(13, "v6.41_3d2f", "ProfROM +GMX V.6.xx.041.8689 (3D2F)")
	ROMX_LOAD( "profgmx6xx041.8689_3d2f.rom", 0x010000, 0x80000, CRC(37afe94d) SHA1(904d85addf8e38206223f049a4fbeb707fb0dfc5), ROM_BIOS(13))
	ROM_SYSTEM_BIOS(14, "v6.41_uni", "ProfROM +GMX V.6.xx.041.8689 (UNI)")
	ROMX_LOAD( "profgmx6xx041.8689_uni.rom", 0x010000, 0x80000, CRC(04d9aa7c) SHA1(a22d89337874a9b01ed80b8a1c09cc86c56dff69), ROM_BIOS(14))
ROM_END

ROM_START(profi)
	ROM_REGION(0x020000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v1", "ver 1")
	ROMX_LOAD( "profi.rom", 0x010000, 0x10000, CRC(285a0985) SHA1(2b33ab3561e7bc5997da7f0d3a2a906fe7ea960f), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2", "ver 2")
	ROMX_LOAD( "profi_my.rom", 0x010000, 0x10000, CRC(2ffd6cd9) SHA1(1b74a3251358c5f102bb87654f47b02281e15e9c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v3", "ver 3")
	ROMX_LOAD( "profi-p1.rom", 0x010000, 0x10000, CRC(537ddb81) SHA1(00a23e8dc722b248d4f98cb14a600ce7487f2b9c), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v4", "ver 4")
	ROMX_LOAD( "profi_1.rom", 0x010000, 0x10000, CRC(f07fbee8) SHA1(b29c81a94658a4d50274ba953775a49e855534de), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v5", "T-Rex")
	ROMX_LOAD( "profi-p.rom", 0x010000, 0x10000, CRC(314f6b57) SHA1(1507f53ec64dcf5154b5cfce6922f69f70296a53), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v6", "JV Kramis V0.2")
	ROMX_LOAD( "profi32.rom", 0x010000, 0x10000, CRC(77327f52) SHA1(019bd00cc7939741d99b99beac6ae1298652e652), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v7", "Power Of Sound Group")
	ROMX_LOAD( "profi1k.rom", 0x010000, 0x10000, CRC(a932676f) SHA1(907ac56219f325949a7c2fe8168799d9cdd5ba6c), ROM_BIOS(6))
ROM_END

ROM_START(quorum)
	ROM_REGION(0x020000, "maincpu", 0)
	ROM_LOAD("qu7v42.rom",   0x010000, 0x10000, CRC(e950eee5) SHA1(f8e22672722b0038689c6c8bc4acf5392acc9d8c))
ROM_END

ROM_START(bestzx)
	ROM_REGION(0x020000, "maincpu", 0)
	ROM_LOAD( "bestzx.rom", 0x010000, 0x10000, CRC(fc7936e8) SHA1(0d6378c51b2f08a3e2b4c75e64c76c15ae5dc76d))
ROM_END

ROM_START( kay1024 )
	ROM_REGION(0x020000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "v1", "ver 1")
	ROMX_LOAD( "kay98.rom",    0x010000, 0x08000, CRC(7fbf2d43) SHA1(e555f2ed01ecf2231d493bd70a4d79b436e9f10e), ROM_BIOS(0))
	ROMX_LOAD( "trd503.rom",   0x01c000, 0x04000, CRC(10751aba) SHA1(21695e3f2a8f796386ce66eea8a246b0ac44810c), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2", "Kramis V0.2")
	ROMX_LOAD( "kay1024b.rom", 0x010000, 0x10000, CRC(ab99c31e) SHA1(cfa9e6553aea72956fce4f0130c007981d684734), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v3", "Kramis V0.3")
	ROMX_LOAD( "kay1024s.rom", 0x010000, 0x10000, CRC(67351caa) SHA1(1d9c0606b380c000ca1dfa33f90a122ecf9df1f1), ROM_BIOS(2))
ROM_END

} // Anonymous namespace


//    YEAR  NAME         PARENT   COMPAT  MACHINE      INPUT       CLASS               INIT        COMPANY              FULLNAME                        FLAGS
COMP( 1992, scorpio,     spec128, 0,      scorpion,    scorpion,   scorpion_state,     empty_init, "Scorpion, Ltd.",    "Scorpion ZS-256 (Yellow PCB)", 0 )
COMP( 1996, scorpiontb,  spec128, 0,      scorpiontb,  scorpiontb, scorpiontb_state,   empty_init, "Scorpion, Ltd.",    "Scorpion ZS-256 TURBO+",       0 )
COMP( 1998, scorpiongmx, spec128, 0,      scorpiongmx, scorpiontb, scorpiongmx_state,  empty_init, "Scorpion, Ltd.",    "Scorpion GMX",                 0 )
COMP( 1991, profi,       spec128, 0,      profi,       scorpion,   scorpion_state,     empty_init, "Kondor and Kramis", "Profi",                        MACHINE_NOT_WORKING )
COMP( 1998, kay1024,     spec128, 0,      scorpion,    scorpion,   scorpion_state,     empty_init, "NEMO",              "Kay 1024",                     MACHINE_NOT_WORKING )
COMP( 19??, quorum,      spec128, 0,      quorum,      scorpion,   scorpion_state,     empty_init, "<unknown>",         "Quorum",                       MACHINE_NOT_WORKING )
COMP( 19??, bestzx,      spec128, 0,      scorpion,    scorpion,   scorpion_state,     empty_init, "<unknown>",         "BestZX",                       MACHINE_NOT_WORKING )
