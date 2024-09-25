// license:GPL-2.0+
// copyright-holders:Dirk Best,Paul Danials
/******************************************************************************************************

Nascom 1/2/3

Single board computer


To Do:
- TTY
- Nascom2 has two dipswitch banks, and a memory control header
- Nascom3 (Gemini), nothing usable found

Cassette (nascom1):
  It outputs a string of pulses at 1953.125Hz to indicate a 1, and blank tape for 0. This means that
  no tape will present a 0 to the UART instead of the expected 1 (idle). Part of the cassette
  schematic is missing, so a few liberties have been taken. The result is you can save a file and
  load it back. Haven't found wav files on the net to test with.

Cassette (nascom2):
  Standard Kansas City format, switchable (with a real switch) between 300 and 1200 baud.

*****************************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/ram.h"
#include "machine/z80pio.h"

#include "machine/timer.h"
#include "speaker.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/nasbus/nasbus.h"

#include "emupal.h"
#include "softlist_dev.h"
#include "screen.h"

#include <tuple>


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nascom_state : public driver_device
{
public:
	nascom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_hd6402(*this, "hd6402")
		, m_cass(*this, "cassette")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_videoram(*this, "videoram")
		, m_gfx1_region(*this, "gfx1")
		, m_keyboard(*this, "KEY.%u", 0)
	{ }

	void nascom(machine_config &config);

	void init_nascom();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;

	virtual void machine_reset() override ATTR_COLD;

	uint8_t nascom1_port_00_r();
	void nascom1_port_00_w(uint8_t data);
	uint8_t nascom1_port_01_r();
	void nascom1_port_01_w(uint8_t data);
	uint8_t nascom1_port_02_r();
	int hd6402_si();
	void hd6402_so(int state);

	void screen_update(bitmap_ind16 &bitmap, const rectangle &cliprect, int char_height);

	required_device<ay31015_device> m_hd6402;
	required_device<cassette_image_device> m_cass;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_videoram;
	required_memory_region m_gfx1_region;
	required_ioport_array<8> m_keyboard;

	uint8_t m_kb_select;
	uint8_t m_kb_control;
	bool m_cassinbit, m_cassoutbit, m_cassold;
	u8 m_port00;

	template<int Dest> DECLARE_SNAPSHOT_LOAD_MEMBER( snapshot_cb );
};

class nascom1_state : public nascom_state
{
public:
	nascom1_state(const machine_config &mconfig, device_type type, const char *tag)
		: nascom_state(mconfig, type, tag)
	{ }

	void nascom1(machine_config &config);
	uint32_t screen_update_nascom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	void nascom1_io(address_map &map) ATTR_COLD;
	void nascom1_mem(address_map &map) ATTR_COLD;
	TIMER_DEVICE_CALLBACK_MEMBER(nascom1_kansas_r);
	void nascom1_kansas_w(int state);
	u16 m_cass_cnt[2];
};

class nascom2_state : public nascom_state
{
public:
	nascom2_state(const machine_config &mconfig, device_type type, const char *tag)
		: nascom_state(mconfig, type, tag)
		, m_clock(*this, "uart_clock")
		, m_nasbus(*this, "nasbus")
		, m_socket1(*this, "socket1")
		, m_socket2(*this, "socket2")
		, m_lsw1(*this, "lsw1")
	{ }

	void nascom2(machine_config &config);
	void nascom2c(machine_config &config);

	void init_nascom2c();

	DECLARE_INPUT_CHANGED_MEMBER(cass_speed);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(nascom2_kansas_r);
	void nascom2_kansas_w(int state);
	void ram_disable_w(int state);
	void ram_disable_cpm_w(int state);
	uint32_t screen_update_nascom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	std::pair<std::error_condition, std::string> load_cart(device_image_interface &image, generic_slot_device *slot, int slot_id);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(socket1_load) { return load_cart(image, m_socket1, 1); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(socket2_load) { return load_cart(image, m_socket2, 2); }

	void nascom2_io(address_map &map) ATTR_COLD;
	void nascom2_mem(address_map &map) ATTR_COLD;
	void nascom2c_mem(address_map &map) ATTR_COLD;

	required_device<clock_device> m_clock;
	required_device<nasbus_device> m_nasbus;
	required_device<generic_slot_device> m_socket1;
	required_device<generic_slot_device> m_socket2;
	required_ioport m_lsw1;
	u8 m_cass_data[4];
	bool m_cass_speed;
};


//**************************************************************************
//  KEYBOARD
//**************************************************************************

uint8_t nascom_state::nascom1_port_00_r()
{
	return m_keyboard[m_kb_select]->read() | ~0x7f;
}

void nascom_state::nascom1_port_00_w(uint8_t data)
{
	u8 bits = data ^ m_port00;

	if (BIT(bits, 4))
		m_cass->change_state(BIT(data, 4) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	// d0 falling edge: increment keyboard matrix column select counter
	if (m_kb_control & ~data & 1)
		m_kb_select = (m_kb_select + 1) & 7;

	// d1 falling edge: reset it
	if (m_kb_control & ~data & 2)
		m_kb_select = 0;

	m_kb_control = data & 3;
}


//**************************************************************************
//  CASSETTE
//**************************************************************************

uint8_t nascom_state::nascom1_port_01_r()
{
	return m_hd6402->receive();
}

void nascom_state::nascom1_port_01_w(uint8_t data)
{
	m_hd6402->transmit(data);
}

uint8_t nascom_state::nascom1_port_02_r()
{
	uint8_t data = 0x31; // bits 0,4,5 not used

	m_hd6402->write_swe(0);
	data |= m_hd6402->or_r(  ) ? 0x02 : 0;
	data |= m_hd6402->pe_r(  ) ? 0x04 : 0;
	data |= m_hd6402->fe_r(  ) ? 0x08 : 0;
	data |= m_hd6402->tbmt_r() ? 0x40 : 0;
	data |= m_hd6402->dav_r( ) ? 0x80 : 0;
	m_hd6402->write_swe(1);

	return data;
}

int nascom_state::hd6402_si()
{
	return m_cassinbit;
}

void nascom_state::hd6402_so(int state)
{
	m_cassoutbit = state;
}

void nascom1_state::nascom1_kansas_w(int state)
{
	// incoming 3906.25Hz
	if (state)
	{
		if (m_cassoutbit)
			m_cass->output(BIT(m_cass_cnt[0], 0) ? -1.0 : +1.0); // 1953.125Hz
		else
			m_cass->output(1.0);

		m_cass_cnt[0]++;
	}
	m_hd6402->write_tcp(state);
}

TIMER_DEVICE_CALLBACK_MEMBER( nascom1_state::nascom1_kansas_r )
{
	// cassette - pulses = 1; no pulses = 0
	m_cass_cnt[1]++;
	bool cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cassold)
	{
		m_cassold = cass_ws;
		m_cassinbit = 1;
		m_cass_cnt[1] = 0;
	}
	else
	if (m_cass_cnt[1] > 10)
	{
		m_cass_cnt[1] = 10;
		m_cassinbit = !cass_ws;
	}
}

void nascom2_state::nascom2_kansas_w(int state)
{
	// incoming @19230Hz
	u8 twobit = m_cass_data[3] & 3;

	if (state)
	{
		if (twobit == 0)
			m_cassold = m_cassoutbit;

		if (m_cassold)
			m_cass->output(BIT(m_cass_data[3], 2) ? -1.0 : +1.0); // 2400Hz
		else
			m_cass->output(BIT(m_cass_data[3], 3) ? -1.0 : +1.0); // 1200Hz

		m_cass_data[3]++;
	}

	if (m_cass_speed || !twobit)
	{
		m_hd6402->write_tcp(state);
		m_hd6402->write_rcp(state);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( nascom2_state::nascom2_kansas_r )
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	uint8_t cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_cassinbit = (m_cass_data[1] < 12) ? 1 : 0;
		m_cass_data[1] = 0;
	}
}


//**************************************************************************
//  SNAPSHOTS
//**************************************************************************

template<int Dest>
SNAPSHOT_LOAD_MEMBER(nascom_state::snapshot_cb)
{
	util::random_read &file = image.image_core_file();
	std::error_condition err;

	while (true)
	{
		size_t actual;

		uint8_t line[29];
		std::tie(err, actual) = read(file, &line, sizeof(line));
		if (err || (sizeof(line) != actual))
			break;

		unsigned int addr, b[8];
		if (sscanf((char *)line, "%4x %x %x %x %x %x %x %x %x",
			&addr, &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6], &b[7]) == 9)
		{
			for (int i = 0; i < 8; i++)
			{
				switch (Dest)
				{
				case 0: // snapshot
					m_maincpu->space(AS_PROGRAM).write_byte(addr++, b[i]);
					break;
				case 1: // character rom
					m_gfx1_region->base()[addr++] = b[i];
					break;
				}
			}
		}
		else
		{
			return std::make_pair(image_error::INVALIDIMAGE, "Unsupported file format");
		}
		int dummy = 0x00;
		do
		{
			std::tie(err, actual) = read(file, &dummy, 1);
			if (err || (actual != 1))
				return std::make_pair(err, std::string());
		}
		while (dummy != 0x0a && dummy != 0x1f);
	}

	return std::make_pair(err, std::string());
}


//**************************************************************************
//  SOCKETS
//**************************************************************************

std::pair<std::error_condition, std::string> nascom2_state::load_cart(
		device_image_interface &image,
		generic_slot_device *slot,
		int slot_id)
{
	if (!image.loaded_through_softlist())
	{
		// loading directly from file
		if (slot->length() > 0x1000)
			return std::make_pair(image_error::INVALIDLENGTH, "Unsupported image file size (must be no more than 4K)");

		slot->rom_alloc(slot->length(), GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);

		auto const [err, actual] = read(slot->image_core_file(), slot->get_rom_base(), slot->length());
		if (err || actual != slot->length())
			return std::make_pair(err ? err : std::errc::io_error, std::string());

		// we just assume that socket1 should be loaded to 0xc000 and socket2 to 0xd000
		switch (slot_id)
		{
		case 1:
			m_maincpu->space(AS_PROGRAM).install_rom(0xc000, 0xc000 + slot->length() - 1, slot->get_rom_base());
			break;
		case 2:
			m_maincpu->space(AS_PROGRAM).install_rom(0xd000, 0xd000 + slot->length() - 1, slot->get_rom_base());
			break;
		}
	}
	else
	{
		// loading from software list. this supports multiple regions to load to
		uint8_t *const region_b000 = image.get_software_region("b000");
		uint8_t *const region_c000 = image.get_software_region("c000");
		uint8_t *const region_d000 = image.get_software_region("d000");

		if (region_b000 != nullptr)
		{
			uint32_t size = image.get_software_region_length("b000");
			m_maincpu->space(AS_PROGRAM).install_rom(0xb000, 0xb000 + size - 1, region_b000);
		}

		if (region_c000 != nullptr)
		{
			uint32_t size = image.get_software_region_length("c000");
			m_maincpu->space(AS_PROGRAM).install_rom(0xc000, 0xc000 + size - 1, region_c000);
		}

		if (region_d000 != nullptr)
		{
			uint32_t size = image.get_software_region_length("d000");
			m_maincpu->space(AS_PROGRAM).install_rom(0xd000, 0xd000 + size - 1, region_d000);
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}


//**************************************************************************
//  DRIVER INIT
//**************************************************************************

void nascom_state::machine_reset()
{
	m_kb_select = 0;
	m_kb_control = 0;
	m_port00 = 0;

	// Set up hd6402 pins
	m_hd6402->write_swe(1);

	m_hd6402->write_cs(0);
	m_hd6402->write_np(1);
	m_hd6402->write_nb1(1);
	m_hd6402->write_nb2(1);
	m_hd6402->write_eps(1);
	m_hd6402->write_tsb(1);
	m_hd6402->write_cs(1);
}

void nascom2_state::machine_reset()
{
	// nascom2: restore speed at machine start
	m_cass_speed = ioport("DSW0")->read();

	// base machine reset
	nascom_state::machine_reset();

	// restart address (on the real system, a12 to a15 are forced to 1 for one memory cycle)
	m_maincpu->set_state_int(Z80_PC, m_lsw1->read() << 12);
}

void nascom_state::init_nascom()
{
	// install extra memory
	if (m_ram->size() > 0)
	{
		m_maincpu->space(AS_PROGRAM).install_ram(0x1000, 0x1000 + m_ram->size() - 1, m_ram->pointer());
	}
}


// since we don't know for which regions we should disable ram, we just let other devices
// overwrite the region they need, and re-install our ram when they are disabled
void nascom2_state::ram_disable_w(int state)
{
	if (state)
	{
		// enable ram again
		m_maincpu->space(AS_PROGRAM).install_ram(0x1000, 0x1000 + m_ram->size() - 1, m_ram->pointer());
	}
}

void nascom2_state::init_nascom2c()
{
	// install memory
	m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x0000 + m_ram->size() - 1, m_ram->pointer());
}

void nascom2_state::ram_disable_cpm_w(int state)
{
	if (state)
	{
		// enable ram again
		m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x0000 + m_ram->size() - 1, m_ram->pointer());
	}
}


//**************************************************************************
//  VIDEO
//**************************************************************************

static const gfx_layout nascom1_charlayout =
{
	8, 16,
	128,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	8 * 16
};

static GFXDECODE_START( gfx_nascom1 )
	GFXDECODE_ENTRY("gfx1", 0x0000, nascom1_charlayout, 0, 1)
GFXDECODE_END

uint32_t nascom1_state::screen_update_nascom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update(bitmap, cliprect, 16);
	return 0;
}

static const gfx_layout nascom2_charlayout =
{
	8, 14,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8,  3*8,  4*8,  5*8,  6*8,
		7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8 },
	8 * 16
};

static GFXDECODE_START( gfx_nascom2 )
	GFXDECODE_ENTRY("gfx1", 0x0000, nascom2_charlayout, 0, 1)
GFXDECODE_END

uint32_t nascom2_state::screen_update_nascom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update(bitmap, cliprect, 14);
	return 0;
}

void nascom_state::screen_update(bitmap_ind16 &bitmap, const rectangle &cliprect, int char_height)
{
	for (int sx = 0; sx < 48; sx++)
	{
		m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_videoram[0x03ca + sx], 1, 0, 0, sx * 8, 0);
	}

	for (int sy = 0; sy < 15; sy++)
	{
		for (int sx = 0; sx < 48; sx++)
		{
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_videoram[0x000a + (sy * 64) + sx], 1, 0, 0, sx * 8, (sy + 1) * char_height);
		}
	}
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void nascom1_state::nascom1_mem(address_map &map)
{
	map(0x0000, 0x07ff).rom(); // MONITOR
	map(0x0800, 0x0bff).ram().share("videoram");
	map(0x0c00, 0x0fff).ram(); // WRAM
}

void nascom1_state::nascom1_io(address_map &map)
{
	map.global_mask(0x0f);
	map(0x00, 0x00).rw(FUNC(nascom1_state::nascom1_port_00_r), FUNC(nascom1_state::nascom1_port_00_w));
	map(0x01, 0x01).rw(FUNC(nascom1_state::nascom1_port_01_r), FUNC(nascom1_state::nascom1_port_01_w));
	map(0x02, 0x02).r(FUNC(nascom1_state::nascom1_port_02_r));
	map(0x04, 0x07).rw("z80pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}

void nascom2_state::nascom2_mem(address_map &map)
{
	map(0x0000, 0x07ff).rom(); // MONITOR
	map(0x0800, 0x0bff).ram().share("videoram");
	map(0x0c00, 0x0fff).ram(); // WRAM
	map(0xe000, 0xffff).rom().region("basic", 0);
}

void nascom2_state::nascom2_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(nascom2_state::nascom1_port_00_r), FUNC(nascom2_state::nascom1_port_00_w));
	map(0x01, 0x01).rw(FUNC(nascom2_state::nascom1_port_01_r), FUNC(nascom2_state::nascom1_port_01_w));
	map(0x02, 0x02).r(FUNC(nascom2_state::nascom1_port_02_r));
	map(0x04, 0x07).rw("z80pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}

void nascom2_state::nascom2c_mem(address_map &map)
{
	map(0xf000, 0xf7ff).rom().region("maincpu", 0);
	map(0xf800, 0xfbff).ram().share("videoram");
	map(0xfc00, 0xffff).ram(); // WRAM
}


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( nascom1 )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace ClearScreen") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("New Line")              PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(0xff) PORT_CHAR('@')  // have to press shift to get @
	PORT_BIT(0x48, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)    PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)    PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)    PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)    PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)    PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)    PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)    PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)    PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)    PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)    PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)    PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)    PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)     PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)     PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)     PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>') // > nascom2 only
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR(0xA3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)     PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0') PORT_CHAR('^')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)     PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)     PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)     PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)     PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)     PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( nascom2 )
	PORT_INCLUDE(nascom1)
	PORT_MODIFY("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back CS")       PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter  Escape") PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)  PORT_CHAR(27)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)  PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("KEY.6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                            PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('[') PORT_CHAR('\\')

	PORT_MODIFY("KEY.7")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                            PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(']') PORT_CHAR('_')

	PORT_START("DSW0")
	PORT_DIPNAME(0x01, 0x00, "Cassette Baud Rate") PORT_CHANGED_MEMBER(DEVICE_SELF, nascom2_state, cass_speed, 0)
	PORT_DIPSETTING(0x00, "300")
	PORT_DIPSETTING(0x01, "1200")

	// link switch on board
	PORT_START("lsw1")
	PORT_DIPNAME(0x0f, 0x00, "Restart address")
	PORT_DIPLOCATION("LSW1:1,2,3,4")
	PORT_DIPSETTING(0x00, "0000H")
	PORT_DIPSETTING(0x01, "1000H")
	PORT_DIPSETTING(0x02, "2000H")
	PORT_DIPSETTING(0x03, "3000H")
	PORT_DIPSETTING(0x04, "4000H")
	PORT_DIPSETTING(0x05, "5000H")
	PORT_DIPSETTING(0x06, "6000H")
	PORT_DIPSETTING(0x07, "7000H")
	PORT_DIPSETTING(0x08, "8000H")
	PORT_DIPSETTING(0x09, "9000H")
	PORT_DIPSETTING(0x0a, "A000H")
	PORT_DIPSETTING(0x0b, "B000H")
	PORT_DIPSETTING(0x0c, "C000H")
	PORT_DIPSETTING(0x0d, "D000H")
	PORT_DIPSETTING(0x0e, "E000H")
	PORT_DIPSETTING(0x0f, "F000H")
INPUT_PORTS_END

static INPUT_PORTS_START( nascom2c )
	PORT_INCLUDE(nascom2)

	PORT_MODIFY("lsw1")
	PORT_DIPNAME(0x0f, 0x0f, "Restart address")
	PORT_DIPLOCATION("LSW1:1,2,3,4")
	PORT_DIPSETTING(0x00, "0000H")
	PORT_DIPSETTING(0x01, "1000H")
	PORT_DIPSETTING(0x02, "2000H")
	PORT_DIPSETTING(0x03, "3000H")
	PORT_DIPSETTING(0x04, "4000H")
	PORT_DIPSETTING(0x05, "5000H")
	PORT_DIPSETTING(0x06, "6000H")
	PORT_DIPSETTING(0x07, "7000H")
	PORT_DIPSETTING(0x08, "8000H")
	PORT_DIPSETTING(0x09, "9000H")
	PORT_DIPSETTING(0x0a, "A000H")
	PORT_DIPSETTING(0x0b, "B000H")
	PORT_DIPSETTING(0x0c, "C000H")
	PORT_DIPSETTING(0x0d, "D000H")
	PORT_DIPSETTING(0x0e, "E000H")
	PORT_DIPSETTING(0x0f, "F000H")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(nascom2_state::cass_speed)
{
	m_cass_speed = newval ? 1 : 0;
}


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void nascom_state::nascom(machine_config &config)
{
	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(48 * 8, 16 * 16);
	m_screen->set_visarea(0, 48 * 8 - 1, 0, 16 * 16 - 1);
	m_screen->set_screen_update(FUNC(nascom1_state::screen_update_nascom));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nascom1);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// pio
	Z80PIO(config, "z80pio", 16_MHz_XTAL / 8);

	// internal extra ram
	RAM(config, m_ram).set_default_size("48K").set_extra_options("8K,16K,32K");

	// uart
	AY31015(config, m_hd6402);
	m_hd6402->read_si_callback().set(FUNC(nascom_state::hd6402_si));
	m_hd6402->write_so_callback().set(FUNC(nascom_state::hd6402_so));
	m_hd6402->set_auto_rdav(true);

	// cassette is connected to the uart
	CASSETTE(config, m_cass);
	m_cass->set_interface("nascom_cass");
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	SPEAKER(config, "mono").front_center();
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	// devices
	snapshot_image_device &snapshot(SNAPSHOT(config, "snapshot", "nas", attotime::from_msec(500)));
	snapshot.set_load_callback(FUNC(nascom_state::snapshot_cb<0>));
	snapshot.set_interface("nascom_snap");
	snapshot_image_device &snapchar(SNAPSHOT(config, "snapchar", "chr", attotime::from_msec(500)));
	snapchar.set_load_callback(FUNC(nascom_state::snapshot_cb<1>));
	snapchar.set_interface("nascom_char");
}

void nascom1_state::nascom1(machine_config &config)
{
	nascom(config);

	Z80(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &nascom1_state::nascom1_mem);
	m_maincpu->set_addrmap(AS_IO, &nascom1_state::nascom1_io);

	// software
	SOFTWARE_LIST(config, "snap_list").set_original("nascom_snap").set_filter("NASCOM1");

	clock_device &uart_clock(CLOCK(config, "uart_clock", (16_MHz_XTAL / 16) / 256));
	uart_clock.signal_handler().set(FUNC(nascom1_state::nascom1_kansas_w));
	uart_clock.signal_handler().append(m_hd6402, FUNC(ay31015_device::write_rcp));
	TIMER(config, "kansas_r").configure_periodic(FUNC(nascom1_state::nascom1_kansas_r), attotime::from_hz(40000));
}

void nascom2_state::nascom2(machine_config &config)
{
	nascom(config);

	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &nascom2_state::nascom2_mem);
	m_maincpu->set_addrmap(AS_IO, &nascom2_state::nascom2_io);

	// video hardware
	m_screen->set_size(48 * 8, 16 * 14);
	m_screen->set_visarea(0, 48 * 8 - 1, 0, 16 * 14 - 1);
	m_screen->set_screen_update(FUNC(nascom2_state::screen_update_nascom));

	m_gfxdecode->set_info(gfx_nascom2);

	// generic sockets for ram/rom (todo: support ram here)
	GENERIC_SOCKET(config, m_socket1, generic_plain_slot, "nascom_socket", "bin,rom");
	m_socket1->set_device_load(FUNC(nascom2_state::socket1_load));
	GENERIC_SOCKET(config, m_socket2, generic_plain_slot, "nascom_socket", "bin,rom");
	m_socket2->set_device_load(FUNC(nascom2_state::socket2_load));

	// nasbus expansion bus
	NASBUS(config, m_nasbus);
	m_nasbus->ram_disable().set(FUNC(nascom2_state::ram_disable_w));
	m_nasbus->set_program_space(m_maincpu, AS_PROGRAM);
	m_nasbus->set_io_space(m_maincpu, AS_IO);
	NASBUS_SLOT(config, "nasbus1", m_nasbus, nasbus_slot_cards, nullptr);
	NASBUS_SLOT(config, "nasbus2", m_nasbus, nasbus_slot_cards, nullptr);
	NASBUS_SLOT(config, "nasbus3", m_nasbus, nasbus_slot_cards, nullptr);
	NASBUS_SLOT(config, "nasbus4", m_nasbus, nasbus_slot_cards, nullptr);

	// software
	SOFTWARE_LIST(config, "snap_list").set_original("nascom_snap").set_filter("NASCOM2");
	SOFTWARE_LIST(config, "socket_list").set_original("nascom_socket");
	SOFTWARE_LIST(config, "floppy_list").set_original("nascom_flop");

	CLOCK(config, m_clock, (16_MHz_XTAL / 32) / 26);
	m_clock->signal_handler().set(FUNC(nascom2_state::nascom2_kansas_w));
	TIMER(config, "kansas_r").configure_periodic(FUNC(nascom2_state::nascom2_kansas_r), attotime::from_hz(40000));
}

void nascom2_state::nascom2c(machine_config &config)
{
	nascom2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &nascom2_state::nascom2c_mem);

	m_ram->set_default_size("60K");

	m_nasbus->ram_disable().set(FUNC(nascom2_state::ram_disable_cpm_w));
	subdevice<nasbus_slot_device>("nasbus1")->set_default_option("floppy");
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( nascom1 )
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_DEFAULT_BIOS("t4")
	ROM_SYSTEM_BIOS(0, "t1", "NasBug T1")
	ROMX_LOAD("nasbugt1.ic38", 0x0000, 0x0400, CRC(8ea07054) SHA1(3f9a8632826003d6ea59d2418674d0fb09b83a4c), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "t2", "NasBug T2")
	ROMX_LOAD("nasbugt2.ic38", 0x0000, 0x0400, CRC(e371b58a) SHA1(485b20a560b587cf9bb4208ba203b12b3841689b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "t4", "NasBug T4")
	ROMX_LOAD("nasbugt4.rom", 0x0000, 0x0800, CRC(f391df68) SHA1(00218652927afc6360c57e77d6a4fd32d4e34566), ROM_BIOS(2)) // should really be split in halves for ic38 and ic39
	ROM_SYSTEM_BIOS(3, "bbug", "B-Bug") // by Viewfax 1978
	ROMX_LOAD("bbug.rom",     0x0000, 0x0800, CRC(1b1a340d) SHA1(99ce4d771871b3d797cc465d6245c40e41acef3e), ROM_BIOS(3))

	ROM_REGION(0x0800, "gfx1", 0)
	ROM_LOAD("nascom1.ic16",   0x0000, 0x0800, CRC(33e92a04) SHA1(be6e1cc80e7f95a032759f7df19a43c27ff93a52)) // MCM6576P
ROM_END

ROM_START( nascom2 )
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_DEFAULT_BIOS("ns3")
	ROM_SYSTEM_BIOS(0, "ns1", "Nas-Sys 1")
	ROMX_LOAD("nassys1.ic34", 0x0000, 0x0800, CRC(b6300716) SHA1(29da7d462ba3f569f70ed3ecd93b981f81c7adfa), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ns3", "Nas-Sys 3")
	ROMX_LOAD("nassys3.ic34", 0x0000, 0x0800, CRC(6804e675) SHA1(d55dccec2d1da992a39c38b0b6d24e3809073513), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "ns3a", "Nas-Sys 3 (AVC)")
	ROMX_LOAD("nassys3a.ic34", 0x0000, 0x0800, CRC(39d24a05) SHA1(7bfb574c1f8ce0f460a53b9a6c11c711aabccbb8), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "ns3n", "Nas-Sys 3 (NET)")
	ROMX_LOAD("nassys3n.ic34", 0x0000, 0x0800, CRC(87ef62bb) SHA1(dab81511925be36044b3e8b0ba26a0c717fe83ae), ROM_BIOS(3))

	ROM_REGION(0x2000, "basic", 0)
	ROM_LOAD("basic.ic43", 0x0000, 0x2000, CRC(5cb5197b) SHA1(c41669c2b6d6dea808741a2738426d97bccc9b07))

	ROM_REGION(0x1000, "gfx1", 0)
	ROM_LOAD("nascom1.ic66", 0x0000, 0x0800, CRC(33e92a04) SHA1(be6e1cc80e7f95a032759f7df19a43c27ff93a52))
	ROM_LOAD("nasgra.ic54",  0x0800, 0x0800, CRC(2bc09d32) SHA1(d384297e9b02cbcb283c020da51b3032ff62b1ae))
ROM_END

ROM_START( nascom2c )
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_DEFAULT_BIOS("cpm32")
	ROM_SYSTEM_BIOS(0, "cpm21", "CP/M boot v2.1")
	ROMX_LOAD("cpmbt21.ic34", 0x0000, 0x0800, CRC(44b67ffc) SHA1(60c8335f24798f8de7ad48a4cd03e56a60d87b63), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "cpm32", "CP/M boot v3.2")
	ROMX_LOAD("cpmbt32.ic34", 0x0000, 0x0800, CRC(724f03ba) SHA1(d0958c231e5b121b6c4c97d03c76c207acf90f5a), ROM_BIOS(1))

	ROM_REGION(0x1000, "gfx1", 0)
	ROM_LOAD("nascom1.ic66", 0x0000, 0x0800, CRC(33e92a04) SHA1(be6e1cc80e7f95a032759f7df19a43c27ff93a52))
	ROM_LOAD("nasgra.ic54",  0x0800, 0x0800, CRC(2bc09d32) SHA1(d384297e9b02cbcb283c020da51b3032ff62b1ae))
ROM_END

} // Anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT     CLASS          INIT           COMPANY                  FULLNAME           FLAGS
COMP( 1978, nascom1,  0,       0,      nascom1,  nascom1,  nascom1_state, init_nascom,   "Nascom Microcomputers", "Nascom 1",        MACHINE_NO_SOUND_HW )
COMP( 1979, nascom2,  0,       0,      nascom2,  nascom2,  nascom2_state, init_nascom,   "Nascom Microcomputers", "Nascom 2",        MACHINE_NO_SOUND_HW )
COMP( 1980, nascom2c, nascom2, 0,      nascom2c, nascom2c, nascom2_state, init_nascom2c, "Nascom Microcomputers", "Nascom 2 (CP/M)", MACHINE_NO_SOUND_HW )
