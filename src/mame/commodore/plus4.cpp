// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

    - clean up TED
    - T6721 speech chip

*/

#include "emu.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "bus/cbmiec/cbmiec.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/pet/c2n.h"
#include "bus/pet/cass.h"
#include "bus/pet/diag264_lb_tape.h"
#include "bus/plus4/exp.h"
#include "bus/plus4/user.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "cpu/m6502/m7501.h"
#include "imagedev/snapquik.h"
#include "cbm_snqk.h"
#include "machine/input_merger.h"
#include "machine/mos6529.h"
#include "machine/mos6551.h"
#include "machine/mos8706.h"
#include "machine/pla.h"
#include "sound/mos7360.h"
#include "sound/t6721a.h"


namespace {

#define MOS7360_TAG         "u1"
#define MOS6551_TAG         "u3"
#define MOS6529_USER_TAG    "u5"
#define MOS6529_KB_TAG      "u27"
#define T6721A_TAG          "t6721a"
#define MOS8706_TAG         "mos8706"
#define PLA_TAG             "u19"
#define SCREEN_TAG          "screen"
#define CONTROL1_TAG        "joy1"
#define CONTROL2_TAG        "joy2"
#define PET_USER_PORT_TAG   "user"

class plus4_state : public driver_device
{
public:
	plus4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "u2"),
		m_ted(*this, MOS7360_TAG),
		m_acia(*this, MOS6551_TAG),
		m_spi_user(*this, MOS6529_USER_TAG),
		m_spi_kb(*this, MOS6529_KB_TAG),
		m_vslsi(*this, MOS8706_TAG),
		m_iec(*this, CBM_IEC_TAG),
		m_joy1(*this, CONTROL1_TAG),
		m_joy2(*this, CONTROL2_TAG),
		m_exp(*this, "exp"),
		m_user(*this, PET_USER_PORT_TAG),
		m_ram(*this, "ram"),
		m_cassette(*this, PET_DATASSETTE_PORT_TAG),
		m_kernal(*this, "kernal"),
		m_function(*this, "function"),
		m_function_lo(*this, "function_lo"),
		m_function_hi(*this, "function_hi"),
		m_c2(*this, "c2"),
		m_lo(*this, "lo"),
		m_hi(*this, "hi"),
		m_video_lo(*this, "video_lo"),
		m_video_hi(*this, "video_hi"),
		m_row(*this, "ROW%u", 0),
		m_lock(*this, "LOCK"),
		m_portswap(*this, "JOYSWAP"),
		m_addr(0)
	{ }

	void plus4(machine_config &config);
	void plus4p(machine_config &config);
	void plus4n(machine_config &config);
	void c264(machine_config &config);

	void cpu_w(uint8_t data);

protected:
	required_device<m7501_device> m_maincpu;
	required_device<mos7360_device> m_ted;
	optional_device<mos6551_device> m_acia;
	optional_device<mos6529_device> m_spi_user;
	required_device<mos6529_device> m_spi_kb;
	optional_device<mos8706_device> m_vslsi;
	required_device<cbm_iec_device> m_iec;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<plus4_expansion_slot_device> m_exp;
	optional_device<pet_user_port_device> m_user;
	required_shared_ptr<uint8_t> m_ram;
	required_device<pet_datassette_port_device> m_cassette;
	required_memory_region m_kernal;
	optional_memory_region m_function;
	optional_device<generic_slot_device> m_function_lo;
	optional_device<generic_slot_device> m_function_hi;
	optional_memory_region m_c2;
	memory_view m_lo;
	memory_view m_hi;
	memory_view m_video_lo;
	memory_view m_video_hi;
	required_ioport_array<8> m_row;
	required_ioport m_lock;
	optional_ioport m_portswap;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	void install_lo_rom(address_space_installer &space, offs_t base, uint8_t *rom);
	void install_hi_rom(address_space_installer &space, offs_t base, uint8_t *rom);
	void install_lo_view(memory_view &view, offs_t base);
	void install_hi_view(memory_view &view, offs_t base);
	uint8_t *socket_rom(generic_slot_device *socket, offs_t offset);
	void install_views();
	void update_banks();

	uint8_t acia_r(offs_t offset);
	void acia_w(offs_t offset, uint8_t data);
	uint8_t user_r();
	void user_w(uint8_t data);
	uint8_t vslsi_r(offs_t offset);
	void vslsi_w(offs_t offset, uint8_t data);
	void addr_w(offs_t offset, uint8_t data);
	void ted_rom_w(offs_t offset, uint8_t data);

	uint8_t cpu_r();

	uint8_t ted_k_r(offs_t offset);

	void write_kb0(int state) { if (state) m_kb |= 1; else m_kb &= ~1; }
	void write_kb1(int state) { if (state) m_kb |= 2; else m_kb &= ~2; }
	void write_kb2(int state) { if (state) m_kb |= 4; else m_kb &= ~4; }
	void write_kb3(int state) { if (state) m_kb |= 8; else m_kb &= ~8; }
	void write_kb4(int state) { if (state) m_kb |= 16; else m_kb &= ~16; }
	void write_kb5(int state) { if (state) m_kb |= 32; else m_kb &= ~32; }
	void write_kb6(int state) { if (state) m_kb |= 64; else m_kb &= ~64; }
	void write_kb7(int state) { if (state) m_kb |= 128; else m_kb &= ~128; }

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload) { return general_cbm_loadsnap(image, m_maincpu->space(AS_PROGRAM), 0, cbm_quick_sethiaddress); }

	bool m_iec_atn;
	bool m_iec_clk;
	bool m_iec_data;
	emu_timer *m_iec_sync_timer;
	TIMER_CALLBACK_MEMBER(iec_sync_tick);

	enum
	{
		VIEW_RAM = 0,
		VIEW_INTERNAL,
		VIEW_FUNCTION,
		VIEW_C1,
		VIEW_C2
	};

	// memory state
	uint8_t m_addr;
	std::unique_ptr<uint8_t[]> m_socket_rom;
	uint8_t *m_function_rom[2];

	// keyboard state
	uint8_t m_kb;

	template <offs_t RamSize> void plus4_mem(address_map &map) ATTR_COLD;
	template <offs_t RamSize> void ted_videoram_map(address_map &map) ATTR_COLD;
};


class c16_state : public plus4_state
{
public:
	c16_state(const machine_config &mconfig, device_type type, const char *tag)
		: plus4_state(mconfig, type, tag)
	{ }

	void v364(machine_config &config);
	void c16n(machine_config &config);
	void c16p(machine_config &config);
	void c232(machine_config &config);

private:
	uint8_t cpu_r();
};



//**************************************************************************
//  MEMORY MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  install_lo_rom -
//-------------------------------------------------

void plus4_state::install_lo_rom(address_space_installer &space, offs_t base, uint8_t *rom)
{
	if (rom)
		space.install_rom(base, base + 0x3fff, rom);
	else
		space.install_read_handler(base, base + 0x3fff, emu::rw_delegate(*m_ted, FUNC(mos7360_device::bus_r)));
}


//-------------------------------------------------
//  install_hi_rom -
//-------------------------------------------------

void plus4_state::install_hi_rom(address_space_installer &space, offs_t base, uint8_t *rom)
{
	if (rom)
	{
		space.install_rom(base, base + 0x3bff, rom);
		space.install_rom(base + 0x3f20, base + 0x3fff, rom + 0x3f20);
	}
	else
	{
		space.install_read_handler(base, base + 0x3bff, emu::rw_delegate(*m_ted, FUNC(mos7360_device::bus_r)));
		space.install_read_handler(base + 0x3f20, base + 0x3fff, emu::rw_delegate(*m_ted, FUNC(mos7360_device::bus_r)));
	}

	space.install_rom(base + 0x3c00, base + 0x3cff, m_kernal->base() + 0x7c00);
}


//-------------------------------------------------
//  install_lo_view -
//-------------------------------------------------

void plus4_state::install_lo_view(memory_view &view, offs_t base)
{
	install_lo_rom(view[VIEW_INTERNAL], base, m_kernal->base());
	install_lo_rom(view[VIEW_FUNCTION], base, m_function_rom[0]);
	install_lo_rom(view[VIEW_C1], base, nullptr);
	install_lo_rom(view[VIEW_C2], base, m_c2 ? m_c2->base() : nullptr);
}


//-------------------------------------------------
//  install_hi_view -
//-------------------------------------------------

void plus4_state::install_hi_view(memory_view &view, offs_t base)
{
	install_hi_rom(view[VIEW_INTERNAL], base, m_kernal->base() + 0x4000);
	install_hi_rom(view[VIEW_FUNCTION], base, m_function_rom[1]);
	install_hi_rom(view[VIEW_C1], base, nullptr);
	install_hi_rom(view[VIEW_C2], base, m_c2 ? m_c2->base() + 0x4000 : nullptr);
}


//-------------------------------------------------
//  socket_rom - mirror a function ROM socket
//  into a 16K bank
//-------------------------------------------------

uint8_t *plus4_state::socket_rom(generic_slot_device *socket, offs_t offset)
{
	if (!socket || !socket->exists() || !socket->get_rom_size())
		return nullptr;

	if (!m_socket_rom)
		m_socket_rom = std::make_unique<uint8_t[]>(0x8000);

	uint8_t *const rom = socket->get_rom_base();
	uint32_t const size = socket->get_rom_size();

	for (offs_t i = 0; i < 0x4000; i++)
		m_socket_rom[offset + i] = rom[i % size];

	return &m_socket_rom[offset];
}


//-------------------------------------------------
//  install_views -
//-------------------------------------------------

void plus4_state::install_views()
{
	if (m_function)
	{
		m_function_rom[0] = m_function->base();
		m_function_rom[1] = m_function->base() + 0x4000;
	}
	else
	{
		m_function_rom[0] = socket_rom(m_function_lo, 0x0000);
		m_function_rom[1] = socket_rom(m_function_hi, 0x4000);
	}

	m_lo[VIEW_RAM];
	m_hi[VIEW_RAM];
	install_lo_view(m_lo, 0x8000);
	install_hi_view(m_hi, 0xc000);
	install_lo_view(m_video_lo, 0x18000);
	install_hi_view(m_video_hi, 0x1c000);

	m_exp->c1l().install_views(m_lo[VIEW_C1], &m_video_lo[VIEW_C1]);
	m_exp->c1h().install_views(m_hi[VIEW_C1], &m_video_hi[VIEW_C1]);
	m_exp->c2l().install_views(m_lo[VIEW_C2], &m_video_lo[VIEW_C2]);
	m_exp->c2h().install_views(m_hi[VIEW_C2], &m_video_hi[VIEW_C2]);
	m_exp->io().install_views(m_maincpu->space(AS_PROGRAM));
}


//-------------------------------------------------
//  update_banks -
//-------------------------------------------------

void plus4_state::update_banks()
{
	int const lo = VIEW_INTERNAL + (m_addr & 0x03);
	int const hi = VIEW_INTERNAL + ((m_addr >> 2) & 0x03);

	if (m_ted->rom())
	{
		m_lo.select(lo);
		m_hi.select(hi);
	}
	else
	{
		m_lo.select(VIEW_RAM);
		m_hi.select(VIEW_RAM);
	}

	m_video_lo.select(lo);
	m_video_hi.select(hi);
}


//-------------------------------------------------
//  acia_r -
//-------------------------------------------------

uint8_t plus4_state::acia_r(offs_t offset)
{
	return m_acia ? m_acia->read(offset) : m_ted->bus_r();
}


//-------------------------------------------------
//  acia_w -
//-------------------------------------------------

void plus4_state::acia_w(offs_t offset, uint8_t data)
{
	if (m_acia)
		m_acia->write(offset, data);
}


//-------------------------------------------------
//  user_r -
//-------------------------------------------------

uint8_t plus4_state::user_r()
{
	if (m_spi_user)
		return m_spi_user->read() & ~(!m_cassette->sense_r() << 2);

	return (m_ted->bus_r() & ~0x04) | (m_cassette->sense_r() << 2);
}


//-------------------------------------------------
//  user_w -
//-------------------------------------------------

void plus4_state::user_w(uint8_t data)
{
	if (m_spi_user)
		m_spi_user->write(data);
}


//-------------------------------------------------
//  vslsi_r -
//-------------------------------------------------

uint8_t plus4_state::vslsi_r(offs_t offset)
{
	return m_vslsi ? m_vslsi->read(offset) : m_ted->bus_r();
}


//-------------------------------------------------
//  vslsi_w -
//-------------------------------------------------

void plus4_state::vslsi_w(offs_t offset, uint8_t data)
{
	if (m_vslsi)
		m_vslsi->write(offset, data);
}


//-------------------------------------------------
//  addr_w -
//-------------------------------------------------

void plus4_state::addr_w(offs_t offset, uint8_t data)
{
	m_addr = offset & 0x0f;

	update_banks();
}


//-------------------------------------------------
//  ted_rom_w -
//-------------------------------------------------

void plus4_state::ted_rom_w(offs_t offset, uint8_t data)
{
	m_ted->write(0x3e + offset, data);

	update_banks();
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( plus4_mem )
//-------------------------------------------------

template <offs_t RamSize>
void plus4_state::plus4_mem(address_map &map)
{
	map(0x0000, RamSize - 1).mirror(0xffff & ~(RamSize - 1)).ram().share(m_ram);
	map(0xfd00, 0xff1f).r(m_ted, FUNC(mos7360_device::bus_r)).nopw();
	map(0xfd00, 0xfd03).mirror(0x0c).rw(FUNC(plus4_state::acia_r), FUNC(plus4_state::acia_w));
	map(0xfd10, 0xfd1f).rw(FUNC(plus4_state::user_r), FUNC(plus4_state::user_w));
	map(0xfd20, 0xfd23).mirror(0x0c).rw(FUNC(plus4_state::vslsi_r), FUNC(plus4_state::vslsi_w));
	map(0xfd30, 0xfd3f).rw(m_spi_kb, FUNC(mos6529_device::read), FUNC(mos6529_device::write));
	map(0xfdd0, 0xfddf).w(FUNC(plus4_state::addr_w));
	map(0xff00, 0xff1f).rw(m_ted, FUNC(mos7360_device::read), FUNC(mos7360_device::write));
	map(0xff3e, 0xff3f).w(FUNC(plus4_state::ted_rom_w));
	map(0x8000, 0xbfff).view(m_lo);
	map(0xc000, 0xffff).view(m_hi);
	m_hi[VIEW_RAM](0xff3e, 0xff3f).lr8(NAME([] () { return 0xff; }));
}


//-------------------------------------------------
//  ADDRESS_MAP( ted_videoram_map )
//-------------------------------------------------

template <offs_t RamSize>
void plus4_state::ted_videoram_map(address_map &map)
{
	map(0x00000, RamSize - 1).mirror(0x1ffff & ~(RamSize - 1)).ram().share(m_ram);
	map(0x18000, 0x1bfff).view(m_video_lo);
	map(0x1c000, 0x1ffff).view(m_video_hi);
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( plus4 )
//-------------------------------------------------

static INPUT_PORTS_START( plus4 )
	PORT_START( "ROW0" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE)              PORT_CHAR('@')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)                                    PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)                                    PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)                                    PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HELP F7") PORT_CODE(KEYCODE_F4)               PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                            PORT_CHAR(0xA3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)             PORT_CHAR(13)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INST DEL") PORT_CODE(KEYCODE_BACKSPACE)       PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START( "ROW1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left & Right)") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)         PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)         PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('#')

	PORT_START( "ROW2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)         PORT_CHAR('X')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)         PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)         PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)         PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)         PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_CHAR('R')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')

	PORT_START( "ROW3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)         PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)         PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)         PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)         PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)         PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START( "ROW4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)         PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)         PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)         PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0  \xE2\x86\x91") PORT_CODE(KEYCODE_0)        PORT_CHAR('0') PORT_CHAR('^')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)         PORT_CHAR('J')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)         PORT_CHAR('I')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(')')

	PORT_START( "ROW5" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)                                PORT_CHAR('-')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)                                    PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_CHAR('L')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)         PORT_CHAR('P')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)                                  PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START( "ROW6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)                             PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("=  Pi  \xE2\x86\x90") PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR('=') PORT_CHAR(0x03C0) PORT_CHAR(0x2190)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)                               PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT)                             PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)                             PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT)                            PORT_CHAR('*')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)                              PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START( "ROW7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN STOP") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CBM") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)                             PORT_CHAR(' ')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)                                 PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Control") PORT_CODE(KEYCODE_TAB)          PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Home Clear") PORT_CODE(KEYCODE_DEL)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1') PORT_CHAR('!')

	PORT_START( "LOCK" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "JOYSWAP" )
	PORT_CONFNAME( 0x01, 0x00, "Swap joystick ports" )
	PORT_CONFSETTING( 0x01, "Joystick in swapped port" )
	PORT_CONFSETTING( 0x00, "Joystick in assigned port" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( c16 )
//-------------------------------------------------

static INPUT_PORTS_START( c16 )
	PORT_INCLUDE( plus4 )

	PORT_MODIFY( "ROW0" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT)                                PORT_CHAR(0xA3)

	PORT_MODIFY( "ROW5" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)                  PORT_CHAR('-')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)                                    PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP)                                  PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_MODIFY( "ROW6" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                            PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("=  Pi  \xE2\x86\x90") PORT_CODE(KEYCODE_PGDN) PORT_CHAR('=') PORT_CHAR(0x03C0) PORT_CHAR(0x2190)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)                                PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)                             PORT_CHAR('*')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)                                 PORT_CHAR(UCHAR_MAMEKEY(LEFT))
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  M6510_INTERFACE( cpu_intf )
//-------------------------------------------------

uint8_t plus4_state::cpu_r()
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       CST RD
	    5
	    6       IEC CLK IN
	    7       IEC DATA IN

	*/

	uint8_t data = 0x2f;

	// cassette read
	data |= m_cassette->read() << 4;

	// serial clock
	data |= m_iec->clk_r() << 6;

	// serial data
	data |= m_iec->data_r() << 7;

	return data;
}

uint8_t c16_state::cpu_r()
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       CST RD
	    5
	    6       IEC CLK IN
	    7       IEC DATA IN

	*/

	uint8_t data = 0;

	// cassette read
	data |= m_cassette->read() << 4;

	// serial clock
	data |= m_iec->clk_r() << 6;

	// serial data
	data |= m_iec->data_r() << 7;

	return data;
}

void plus4_state::cpu_w(uint8_t data)
{
	/*

	    bit     description

	    0       IEC DATA
	    1       IEC CLK, CST WR
	    2       IEC ATN
	    3       CST MTR
	    4
	    5
	    6       (CST WR)
	    7

	*/

	//logerror("%s cpu write %02x\n", machine().describe_context(), data);

	// serial data
	m_iec_data = !BIT(data, 0);

	// serial clock
	m_iec_clk = !BIT(data, 1);

	// serial attention
	m_iec_atn = !BIT(data, 2);

	// cassette motor
	m_cassette->motor_w(BIT(data, 3));

	// cassette write
	m_cassette->write(!BIT(data, 1));

	m_iec_sync_timer->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(plus4_state::iec_sync_tick)
{
	m_iec->host_atn_w(m_iec_atn);
	m_iec->host_clk_w(m_iec_clk);
	m_iec->host_data_w(m_iec_data);
}


//-------------------------------------------------
//  ted7360_interface ted_intf
//-------------------------------------------------

uint8_t plus4_state::ted_k_r(offs_t offset)
{
	/*

	    bit     description

	    0       JOY A0, JOY B0
	    1       JOY A1, JOY B1
	    2       JOY A2, JOY B2
	    3       JOY A3, JOY B3
	    4
	    5
	    6       BTN A
	    7       BTN B

	*/

	uint8_t data = 0xff;
	vcs_control_port_device *cur1 = m_portswap->read() ? m_joy2 : m_joy1;
	vcs_control_port_device *cur2 = m_portswap->read() ? m_joy1 : m_joy2;

	// joystick
	if (!BIT(offset, 2))
	{
		uint8_t joy_a = cur1->read_joy();

		data &= (0xf0 | (joy_a & 0x0f));
		data &= ~(!BIT(joy_a, 5) << 6);
	}

	if (!BIT(offset, 1))
	{
		uint8_t joy_b = cur2->read_joy();

		data &= (0xf0 | (joy_b & 0x0f));
		data &= ~(!BIT(joy_b, 5) << 7);
	}

	// keyboard
	if (!BIT(m_kb, 7)) data &= m_row[7]->read();
	if (!BIT(m_kb, 6)) data &= m_row[6]->read();
	if (!BIT(m_kb, 5)) data &= m_row[5]->read();
	if (!BIT(m_kb, 4)) data &= m_row[4]->read();
	if (!BIT(m_kb, 3)) data &= m_row[3]->read();
	if (!BIT(m_kb, 2)) data &= m_row[2]->read();
	if (!BIT(m_kb, 1)) data &= m_row[1]->read() & m_lock->read();
	if (!BIT(m_kb, 0)) data &= m_row[0]->read();

	return data;
}



//-------------------------------------------------
//  SLOT_INTERFACE( cbm_datassette_devices )
//-------------------------------------------------

void plus4_datassette_devices(device_slot_interface &device)
{
	device.option_add("c1531", C1531);
	device.option_add("diag264", DIAG264_CASSETTE_LOOPBACK);
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( plus4 )
//-------------------------------------------------

void plus4_state::machine_start()
{
	m_iec_sync_timer = timer_alloc(FUNC(plus4_state::iec_sync_tick), this);

	// initialize memory
	uint8_t data = 0xff;

	for (offs_t offset = 0; offset < m_ram.bytes(); offset++)
	{
		m_ram[offset] = data;
		if (!(offset % 64)) data ^= 0xff;
	}

	if (!strcmp(machine().system().name, "c264") && (m_kernal->base()[0x5831] == 0x0d))
		m_kernal->base()[0x5831] = 0x0f;

	install_views();

	// state saving
	save_item(NAME(m_addr));
	save_item(NAME(m_kb));

	if (m_acia)
	{
		m_acia->write_cts(0);
	}

	m_spi_kb->write_p0(1);
	m_spi_kb->write_p1(1);
	m_spi_kb->write_p2(1);
	m_spi_kb->write_p3(1);
	m_spi_kb->write_p4(1);
	m_spi_kb->write_p5(1);
	m_spi_kb->write_p6(1);
	m_spi_kb->write_p7(1);
}


void plus4_state::machine_reset()
{
	if (m_user)
	{
		m_user->write_3(0);
		m_user->write_3(1);
	}

	m_addr = 0;

	update_banks();
}


void plus4_state::device_post_load()
{
	update_banks();
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( plus4 )
//-------------------------------------------------

void plus4_state::plus4(machine_config &config)
{
	// basic machine hardware
	M7501(config, m_maincpu, 0); // derived configurations will set clock frequency
	m_maincpu->set_addrmap(AS_PROGRAM, &plus4_state::plus4_mem<0x10000>);
	m_maincpu->read_callback().set(FUNC(plus4_state::cpu_r));
	m_maincpu->write_callback().set(FUNC(plus4_state::cpu_w));
	config.set_perfect_quantum(m_maincpu);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, m7501_device::IRQ_LINE);

	// video and sound hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG));
	screen.set_refresh_hz(mos7360_device::PAL_VRETRACERATE);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(384, 288);
	screen.set_visarea(0, 384 - 1, 0, 288 - 1);
	screen.set_screen_update(MOS7360_TAG, FUNC(mos7360_device::screen_update));

	SPEAKER(config, "mono").front_center();

	MOS7360(config, m_ted);
	m_ted->set_cpu_tag(m_maincpu);
	m_ted->set_addrmap(0, &plus4_state::ted_videoram_map<0x10000>);
	m_ted->set_screen(SCREEN_TAG);
	m_ted->write_irq_callback().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_ted->read_k_callback().set(FUNC(plus4_state::ted_k_r));
	m_ted->add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	PLS100(config, PLA_TAG);

	PET_USER_PORT(config, m_user, plus4_user_port_cards, nullptr);
	m_user->p4_handler().set(m_spi_user, FUNC(mos6529_device::write_p2)); // cassette sense
	m_user->p5_handler().set(m_spi_user, FUNC(mos6529_device::write_p3));
	m_user->p6_handler().set(m_spi_user, FUNC(mos6529_device::write_p4));
	m_user->p7_handler().set(m_spi_user, FUNC(mos6529_device::write_p5));
	m_user->p8_handler().set(m_acia, FUNC(mos6551_device::write_rxc));
	m_user->pb_handler().set(m_spi_user, FUNC(mos6529_device::write_p0));
	m_user->pc_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	m_user->pf_handler().set(m_spi_user, FUNC(mos6529_device::write_p7));
	m_user->ph_handler().set(m_acia, FUNC(mos6551_device::write_dcd)).invert(); // TODO: add missing pull up before inverter
	m_user->pj_handler().set(m_spi_user, FUNC(mos6529_device::write_p6));
	m_user->pk_handler().set(m_spi_user, FUNC(mos6529_device::write_p1));
	m_user->pl_handler().set(m_acia, FUNC(mos6551_device::write_dsr)).invert(); // TODO: add missing pull up before inverter

	MOS6551(config, m_acia);
	m_acia->set_xtal(1.8432_MHz_XTAL);
	m_acia->rxc_handler().set(m_user, FUNC(pet_user_port_device::write_8));
	m_acia->rts_handler().set(m_user, FUNC(pet_user_port_device::write_d)).invert();
	m_acia->dtr_handler().set(m_user, FUNC(pet_user_port_device::write_e)).invert();
	m_acia->txd_handler().set(m_user, FUNC(pet_user_port_device::write_m));
	m_acia->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	MOS6529(config, m_spi_user);
	m_spi_user->p_handler<0>().set(m_user, FUNC(pet_user_port_device::write_b));
	m_spi_user->p_handler<1>().set(m_user, FUNC(pet_user_port_device::write_k));
	m_spi_user->p_handler<2>().set(m_user, FUNC(pet_user_port_device::write_4));
	m_spi_user->p_handler<3>().set(m_user, FUNC(pet_user_port_device::write_5));
	m_spi_user->p_handler<4>().set(m_user, FUNC(pet_user_port_device::write_6));
	m_spi_user->p_handler<5>().set(m_user, FUNC(pet_user_port_device::write_7));
	m_spi_user->p_handler<6>().set(m_user, FUNC(pet_user_port_device::write_j));
	m_spi_user->p_handler<7>().set(m_user, FUNC(pet_user_port_device::write_f));

	MOS6529(config, m_spi_kb);
	m_spi_kb->p_handler<0>().set(FUNC(plus4_state::write_kb0));
	m_spi_kb->p_handler<1>().set(FUNC(plus4_state::write_kb1));
	m_spi_kb->p_handler<2>().set(FUNC(plus4_state::write_kb2));
	m_spi_kb->p_handler<3>().set(FUNC(plus4_state::write_kb3));
	m_spi_kb->p_handler<4>().set(FUNC(plus4_state::write_kb4));
	m_spi_kb->p_handler<5>().set(FUNC(plus4_state::write_kb5));
	m_spi_kb->p_handler<6>().set(FUNC(plus4_state::write_kb6));
	m_spi_kb->p_handler<7>().set(FUNC(plus4_state::write_kb7));

	PET_DATASSETTE_PORT(config, m_cassette, plus4_datassette_devices, "c1531");
	m_cassette->read_handler().set_nop();

	cbm_iec_slot_device::add(config, m_iec, nullptr);
	m_iec->atn_callback().set(m_user, FUNC(pet_user_port_device::write_9));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, nullptr);
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, "joy");

	PLUS4_EXPANSION_SLOT(config, m_exp, XTAL(14'318'181)/16, plus4_expansion_cards, "c1551");
	m_exp->irq_wr_callback().set("mainirq", FUNC(input_merger_device::in_w<2>));
	m_exp->aec_wr_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);

	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "p00,prg", attotime::from_msec(100)));
	quickload.set_load_callback(FUNC(plus4_state::quickload));
	quickload.set_interface("cbm_quik");
}


//-------------------------------------------------
//  machine_config( plus4p )
//-------------------------------------------------

void plus4_state::plus4p(machine_config &config)
{
	plus4(config);
	m_maincpu->set_clock(XTAL(17'734'470)/20);
	m_ted->set_clock(XTAL(17'734'470));
	m_exp->set_clock(XTAL(17'734'470)/20);

	// software list
	SOFTWARE_LIST(config, "cart_list").set_original("plus4_cart");
	SOFTWARE_LIST(config, "cass_list").set_original("plus4_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("plus4_flop");
	SOFTWARE_LIST(config, "quik_list").set_original("plus4_quik");
	SOFTWARE_LIST(config, "sdcard_list").set_original("cbm_sd");
	subdevice<software_list_device>("cart_list")->set_filter("PAL");
	subdevice<software_list_device>("cass_list")->set_filter("PAL");
	subdevice<software_list_device>("flop_list")->set_filter("PAL");
	subdevice<software_list_device>("quik_list")->set_filter("PAL");
	subdevice<software_list_device>("sdcard_list")->set_filter("PAL");
}

//-------------------------------------------------
//  machine_config( plus4n )
//-------------------------------------------------

void plus4_state::plus4n(machine_config &config)
{
	plus4(config);
	m_maincpu->set_clock(XTAL(14'318'181)/16);
	m_ted->set_clock(XTAL(14'318'181));

	screen_device &screen(*subdevice<screen_device>(SCREEN_TAG));
	screen.set_refresh_hz(mos7360_device::NTSC_VRETRACERATE);
	screen.set_size(384, 240);
	screen.set_visarea(0, 384 - 1, 0, 240 - 1);

	// software list
	SOFTWARE_LIST(config, "cart_list").set_original("plus4_cart");
	SOFTWARE_LIST(config, "cass_list").set_original("plus4_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("plus4_flop");
	SOFTWARE_LIST(config, "quik_list").set_original("plus4_quik");
	SOFTWARE_LIST(config, "sdcard_list").set_original("cbm_sd");
	subdevice<software_list_device>("cart_list")->set_filter("NTSC");
	subdevice<software_list_device>("cass_list")->set_filter("NTSC");
	subdevice<software_list_device>("flop_list")->set_filter("NTSC");
	subdevice<software_list_device>("quik_list")->set_filter("NTSC");
	subdevice<software_list_device>("sdcard_list")->set_filter("NTSC");
}


//-------------------------------------------------
//  machine_config( c264 )
//-------------------------------------------------

void plus4_state::c264(machine_config &config)
{
	plus4n(config);

	GENERIC_SOCKET(config, m_function_lo, generic_plain_slot, "c264_rom", "bin,rom");
	GENERIC_SOCKET(config, m_function_hi, generic_plain_slot, "c264_rom", "bin,rom");
}


//-------------------------------------------------
//  machine_config( c16n )
//-------------------------------------------------

void c16_state::c16n(machine_config &config)
{
	plus4n(config);
	m_maincpu->read_callback().set(FUNC(c16_state::cpu_r));
	m_maincpu->write_callback().set(FUNC(plus4_state::cpu_w));

	config.device_remove(MOS6551_TAG);
	config.device_remove(MOS6529_USER_TAG);
	config.device_remove(PET_USER_PORT_TAG);

	m_iec->atn_callback().set_nop();

	m_maincpu->set_addrmap(AS_PROGRAM, &c16_state::plus4_mem<0x4000>);
	m_ted->set_addrmap(0, &c16_state::ted_videoram_map<0x4000>);
}


//-------------------------------------------------
//  machine_config( c16p )
//-------------------------------------------------

void c16_state::c16p(machine_config &config)
{
	plus4p(config);
	m_maincpu->read_callback().set(FUNC(c16_state::cpu_r));
	m_maincpu->write_callback().set(FUNC(plus4_state::cpu_w));

	config.device_remove(MOS6551_TAG);
	config.device_remove(MOS6529_USER_TAG);
	config.device_remove(PET_USER_PORT_TAG);

	m_iec->atn_callback().set_nop();

	m_maincpu->set_addrmap(AS_PROGRAM, &c16_state::plus4_mem<0x4000>);
	m_ted->set_addrmap(0, &c16_state::ted_videoram_map<0x4000>);
}


void c16_state::c232(machine_config &config)
{
	c16p(config);

	GENERIC_SOCKET(config, m_function_lo, generic_plain_slot, "c264_rom", "bin,rom");
	GENERIC_SOCKET(config, m_function_hi, generic_plain_slot, "c264_rom", "bin,rom");

	m_maincpu->set_addrmap(AS_PROGRAM, &c16_state::plus4_mem<0x8000>);
	m_ted->set_addrmap(0, &c16_state::ted_videoram_map<0x8000>);
}


//-------------------------------------------------
//  machine_config( v364 )
//-------------------------------------------------

void c16_state::v364(machine_config &config)
{
	plus4n(config);
	T6721A(config, T6721A_TAG, XTAL(640'000)).add_route(ALL_OUTPUTS, "mono", 0.25);

	MOS8706(config, m_vslsi, XTAL(14'318'181)/16);
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( c264 )
//-------------------------------------------------

ROM_START( c264 )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "basic-264.bin", 0x0000, 0x4000, CRC(6a2fc8e3) SHA1(473fce23afa07000cdca899fbcffd6961b36a8a0) )
	ROM_LOAD( "kernal-264.bin", 0x4000, 0x4000, CRC(c57d5dfd) SHA1(dfaec5b2a03c25e5626b5539f936b5f2688e657c) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02", 0x00, 0xf5, CRC(328538af) SHA1(ccda76572e6c164c31454c8ce083e161e1ddfe0a) )
ROM_END


//-------------------------------------------------
//  ROM( c232 )
//-------------------------------------------------

ROM_START( c232 )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "318006-01.u4", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )
	ROM_LOAD( "318004-01.u5", 0x4000, 0x4000, CRC(dbdc3319) SHA1(3c77caf72914c1c0a0875b3a7f6935cd30c54201) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u7", 0x00, 0xf5, CRC(328538af) SHA1(ccda76572e6c164c31454c8ce083e161e1ddfe0a) )
ROM_END


//-------------------------------------------------
//  ROM( v364 )
//-------------------------------------------------

ROM_START( v364 )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "318006-01", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )
	ROM_LOAD( "kern364p", 0x4000, 0x4000, CRC(84fd4f7a) SHA1(b9a5b5dacd57ca117ef0b3af29e91998bf4d7e5f) )

	ROM_REGION( 0x8000, "function", 0 )
	ROM_LOAD( "317053-01", 0x0000, 0x4000, CRC(4fd1d8cb) SHA1(3b69f6e7cb4c18bb08e203fb18b7dabfa853390f) )
	ROM_LOAD( "317054-01", 0x4000, 0x4000, CRC(109de2fc) SHA1(0ad7ac2db7da692d972e586ca0dfd747d82c7693) )

	ROM_REGION( 0x8000, "c2", 0 )
	ROM_LOAD( "spk3cc4.bin", 0x0000, 0x4000, CRC(5227c2ee) SHA1(59af401cbb2194f689898271c6e8aafa28a7af11) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02", 0x00, 0xf5, CRC(328538af) SHA1(ccda76572e6c164c31454c8ce083e161e1ddfe0a) )
ROM_END


//-------------------------------------------------
//  ROM( plus4 )
//-------------------------------------------------

ROM_START( plus4 )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_DEFAULT_BIOS("r5")
	ROM_SYSTEM_BIOS( 0, "r4", "Revision 4" )
	ROMX_LOAD( "318005-04.u24", 0x4000, 0x4000, CRC(799a633d) SHA1(5df52c693387c0e2b5d682613a3b5a65477311cf), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r5", "Revision 5" )
	ROMX_LOAD( "318005-05.u24", 0x4000, 0x4000, CRC(70295038) SHA1(a3d9e5be091b98de39a046ab167fb7632d053682), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos plus4.u24", 0x0000, 0x8000, CRC(818d3f45) SHA1(9bc1b1c3da9ca642deae717905f990d8e36e6c3b), ROM_BIOS(2) ) // first half contains R5 kernal
	ROM_SYSTEM_BIOS( 3, "diag264", "Diag264 v0.97" )
	ROMX_LOAD( "diag264_097_ntsc_kernal.u24", 0x4000, 0x4000, CRC(6423deaa) SHA1(6a3f63f6cb3cee2a0dd153fe3fb60968a834dd6c), ROM_BIOS(3) )

	ROM_LOAD( "318006-01.u23", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_REGION( 0x8000, "function", 0 )
	ROM_LOAD( "317053-01.u25", 0x0000, 0x4000, CRC(4fd1d8cb) SHA1(3b69f6e7cb4c18bb08e203fb18b7dabfa853390f) )
	ROM_LOAD( "317054-01.u26", 0x4000, 0x4000, CRC(109de2fc) SHA1(0ad7ac2db7da692d972e586ca0dfd747d82c7693) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u19", 0x00, 0xf5, CRC(328538af) SHA1(ccda76572e6c164c31454c8ce083e161e1ddfe0a) )
ROM_END


//-------------------------------------------------
//  ROM( plus4p )
//-------------------------------------------------

ROM_START( plus4p )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "318006-01.u23", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_DEFAULT_BIOS("r5")
	ROM_SYSTEM_BIOS( 0, "r3", "Revision 3" )
	ROMX_LOAD( "318004-03.u24", 0x4000, 0x4000, CRC(77bab934) SHA1(97814dab9d757fe5a3a61d357a9a81da588a9783), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r4", "Revision 4" )
	ROMX_LOAD( "318004-04.u24", 0x4000, 0x4000, CRC(be54ed79) SHA1(514ad3c29d01a2c0a3b143d9c1d4143b1912b793), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "r5", "Revision 5" )
	ROMX_LOAD( "318004-05.u24", 0x4000, 0x4000, CRC(71c07bd4) SHA1(7c7e07f016391174a557e790c4ef1cbe33512cdb), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "diag264", "Diag264 v0.97" )
	ROMX_LOAD( "diag264_097_pal_kernal.u24", 0x4000, 0x4000, CRC(bf0b3657) SHA1(47c731739f6c1bd1c8446b2cacfe1eaddb5df966), ROM_BIOS(3) )

	ROM_REGION( 0x8000, "function", 0 )
	ROM_LOAD( "317053-01.u25", 0x0000, 0x4000, CRC(4fd1d8cb) SHA1(3b69f6e7cb4c18bb08e203fb18b7dabfa853390f) )
	ROM_LOAD( "317054-01.u26", 0x4000, 0x4000, CRC(109de2fc) SHA1(0ad7ac2db7da692d972e586ca0dfd747d82c7693) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u19", 0x00, 0xf5, CRC(328538af) SHA1(ccda76572e6c164c31454c8ce083e161e1ddfe0a) )
ROM_END


//-------------------------------------------------
//  ROM( c16 )
//-------------------------------------------------

ROM_START( c16 )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_DEFAULT_BIOS("r5")
	ROM_SYSTEM_BIOS( 0, "r4", "Revision 4" )
	ROMX_LOAD( "318005-04.u24", 0x4000, 0x4000, CRC(799a633d) SHA1(5df52c693387c0e2b5d682613a3b5a65477311cf), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r5", "Revision 5" )
	ROMX_LOAD( "318005-05.u24", 0x4000, 0x4000, CRC(70295038) SHA1(a3d9e5be091b98de39a046ab167fb7632d053682), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos plus4.u24", 0x0000, 0x8000, CRC(818d3f45) SHA1(9bc1b1c3da9ca642deae717905f990d8e36e6c3b), ROM_BIOS(2) ) // first half contains R5 kernal

	ROM_LOAD( "318006-01.u23", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u19", 0x00, 0xf5, CRC(328538af) SHA1(ccda76572e6c164c31454c8ce083e161e1ddfe0a) )
ROM_END


//-------------------------------------------------
//  ROM( c16p )
//-------------------------------------------------

ROM_START( c16p )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "318006-01.u3", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_DEFAULT_BIOS("r5")
	ROM_SYSTEM_BIOS( 0, "r3", "Revision 3" )
	ROMX_LOAD( "318004-03.u4", 0x4000, 0x4000, CRC(77bab934) SHA1(97814dab9d757fe5a3a61d357a9a81da588a9783), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r4", "Revision 4" )
	ROMX_LOAD( "318004-04.u4", 0x4000, 0x4000, CRC(be54ed79) SHA1(514ad3c29d01a2c0a3b143d9c1d4143b1912b793), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "r5", "Revision 5" )
	ROMX_LOAD( "318004-05.u4", 0x4000, 0x4000, CRC(71c07bd4) SHA1(7c7e07f016391174a557e790c4ef1cbe33512cdb), ROM_BIOS(2) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u16", 0x00, 0xf5, CRC(328538af) SHA1(ccda76572e6c164c31454c8ce083e161e1ddfe0a) )
ROM_END


//-------------------------------------------------
//  ROM( c16_hu )
//-------------------------------------------------

ROM_START( c16_hu )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "318006-01.u3", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "318030-01.u4", 0x4000, 0x4000, NO_DUMP, ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "318030-02.u4", 0x4000, 0x4000, CRC(775f60c5) SHA1(20cf3c4bf6c54ef09799af41887218933f2e27ee), ROM_BIOS(1) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u16", 0x00, 0xf5, CRC(328538af) SHA1(ccda76572e6c164c31454c8ce083e161e1ddfe0a) )
ROM_END


//-------------------------------------------------
//  ROM( c116 )
//-------------------------------------------------

ROM_START( c116 )
	ROM_REGION( 0x8000, "kernal", 0 )
	ROM_LOAD( "318006-01.u3", 0x0000, 0x4000, CRC(74eaae87) SHA1(161c96b4ad20f3a4f2321808e37a5ded26a135dd) )

	ROM_DEFAULT_BIOS("r5")
	ROM_SYSTEM_BIOS( 0, "r3", "Revision 3" )
	ROMX_LOAD( "318004-03.u4", 0x4000, 0x4000, CRC(77bab934) SHA1(97814dab9d757fe5a3a61d357a9a81da588a9783), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r4", "Revision 4" )
	ROMX_LOAD( "318004-04.u4", 0x4000, 0x4000, CRC(be54ed79) SHA1(514ad3c29d01a2c0a3b143d9c1d4143b1912b793), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "r5", "Revision 5" )
	ROMX_LOAD( "318004-05.u4", 0x4000, 0x4000, CRC(71c07bd4) SHA1(7c7e07f016391174a557e790c4ef1cbe33512cdb), ROM_BIOS(2) )

	ROM_REGION( 0xf5, PLA_TAG, 0 )
	ROM_LOAD( "251641-02.u101", 0x00, 0xf5, CRC(328538af) SHA1(ccda76572e6c164c31454c8ce083e161e1ddfe0a) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                        FULLNAME                      FLAGS
COMP( 1984, c264,   0,      0,      c264,    plus4, plus4_state, empty_init, "Commodore Business Machines", "Commodore 264 (Prototype)",  MACHINE_SUPPORTS_SAVE )
COMP( 1984, c232,   c264,   0,      c232,    plus4, c16_state,   empty_init, "Commodore Business Machines", "Commodore 232 (Prototype)",  MACHINE_SUPPORTS_SAVE )
COMP( 1984, v364,   c264,   0,      v364,    plus4, c16_state,   empty_init, "Commodore Business Machines", "Commodore V364 (Prototype)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1984, plus4,  c264,   0,      plus4n,  plus4, plus4_state, empty_init, "Commodore Business Machines", "Plus/4 (NTSC)",              MACHINE_SUPPORTS_SAVE )
COMP( 1984, plus4p, c264,   0,      plus4p,  plus4, plus4_state, empty_init, "Commodore Business Machines", "Plus/4 (PAL)",               MACHINE_SUPPORTS_SAVE )
COMP( 1984, c16,    c264,   0,      c16n,    c16,   c16_state,   empty_init, "Commodore Business Machines", "Commodore 16 (NTSC)",        MACHINE_SUPPORTS_SAVE )
COMP( 1984, c16p,   c264,   0,      c16p,    c16,   c16_state,   empty_init, "Commodore Business Machines", "Commodore 16 (PAL)",         MACHINE_SUPPORTS_SAVE )
COMP( 1984, c16_hu, c264,   0,      c16p,    c16,   c16_state,   empty_init, "Commodore Business Machines", "Commodore 16 (Hungary)",     MACHINE_SUPPORTS_SAVE )
COMP( 1984, c116,   c264,   0,      c16p,    c16,   c16_state,   empty_init, "Commodore Business Machines", "Commodore 116",              MACHINE_SUPPORTS_SAVE )
