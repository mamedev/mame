// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Nascom 1/2/3

    Single board computer

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/z80pio.h"
#include "machine/ay31015.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/nasbus/nasbus.h"


//**************************************************************************
//  CONSTANTS/MACROS
//**************************************************************************

#define NASCOM1_KEY_RESET   0x02
#define NASCOM1_KEY_INCR    0x01


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct nascom1_portstat_t
{
	UINT8   stat_flags;
	UINT8   stat_count;
};

class nascom_state : public driver_device
{
public:
	nascom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_hd6402(*this, "hd6402"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_keyboard(*this, "KEY")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<ay31015_device> m_hd6402;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT8> m_videoram;
	required_ioport_array<9> m_keyboard;

	int m_tape_size;
	UINT8 *m_tape_image;
	int m_tape_index;
	nascom1_portstat_t m_portstat;

	DECLARE_READ8_MEMBER(nascom1_port_00_r);
	DECLARE_WRITE8_MEMBER(nascom1_port_00_w);
	DECLARE_READ8_MEMBER(nascom1_port_01_r);
	DECLARE_WRITE8_MEMBER(nascom1_port_01_w);
	DECLARE_READ8_MEMBER(nascom1_port_02_r);
	DECLARE_DRIVER_INIT(nascom);
	void screen_update(bitmap_ind16 &bitmap, const rectangle &cliprect, int char_height);
	DECLARE_READ8_MEMBER(nascom1_hd6402_si);
	DECLARE_WRITE8_MEMBER(nascom1_hd6402_so);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( nascom1_cassette );
	DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER( nascom1_cassette );
	DECLARE_SNAPSHOT_LOAD_MEMBER( nascom1 );

protected:
	virtual void machine_reset();

private:

};

class nascom1_state : public nascom_state
{
public:
	nascom1_state(const machine_config &mconfig, device_type type, const char *tag) :
	nascom_state(mconfig, type, tag)
	{}

	UINT32 screen_update_nascom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
};

class nascom2_state : public nascom_state
{
public:
	nascom2_state(const machine_config &mconfig, device_type type, const char *tag) :
	nascom_state(mconfig, type, tag),
	m_nasbus(*this, "nasbus"),
	m_socket1(*this, "socket1"),
	m_socket2(*this, "socket2"),
	m_lsw1(*this, "lsw1")
	{}

	DECLARE_WRITE_LINE_MEMBER(ram_disable_w);
	DECLARE_WRITE_LINE_MEMBER(ram_disable_cpm_w);
	DECLARE_DRIVER_INIT(nascom2);
	DECLARE_DRIVER_INIT(nascom2c);
	UINT32 screen_update_nascom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	int load_cart(device_image_interface &image, generic_slot_device *slot, int slot_id);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(socket1_load) { return load_cart(image, m_socket1, 1); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(socket2_load) { return load_cart(image, m_socket2, 2); }

protected:
	virtual void machine_reset();

private:
	required_device<nasbus_device> m_nasbus;
	required_device<generic_slot_device> m_socket1;
	required_device<generic_slot_device> m_socket2;
	required_ioport m_lsw1;
};


//**************************************************************************
//  KEYBOARD
//**************************************************************************

READ8_MEMBER( nascom_state::nascom1_port_00_r )
{
	if (m_portstat.stat_count < 9)
		return ((m_keyboard[m_portstat.stat_count])->read() | ~0x7f);

	return 0xff;
}

WRITE8_MEMBER( nascom_state::nascom1_port_00_w )
{
	m_cassette->change_state(
		(data & 0x10) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	if (!(data & NASCOM1_KEY_RESET))
	{
		if (m_portstat.stat_flags & NASCOM1_KEY_RESET)
			m_portstat.stat_count = 0;
	}
	else
		m_portstat.stat_flags = NASCOM1_KEY_RESET;

	if (!(data & NASCOM1_KEY_INCR))
	{
		if (m_portstat.stat_flags & NASCOM1_KEY_INCR)
			m_portstat.stat_count++;
	}
	else
		m_portstat.stat_flags = NASCOM1_KEY_INCR;
}


//**************************************************************************
//  CASSETTE
//**************************************************************************

READ8_MEMBER( nascom_state::nascom1_port_01_r )
{
	return m_hd6402->get_received_data();
}


WRITE8_MEMBER( nascom_state::nascom1_port_01_w )
{
	m_hd6402->set_transmit_data(data);
}

READ8_MEMBER( nascom_state::nascom1_port_02_r )
{
	UINT8 data = 0x31;

	m_hd6402->set_input_pin(AY31015_SWE, 0);
	data |= m_hd6402->get_output_pin(AY31015_OR  ) ? 0x02 : 0;
	data |= m_hd6402->get_output_pin(AY31015_PE  ) ? 0x04 : 0;
	data |= m_hd6402->get_output_pin(AY31015_FE  ) ? 0x08 : 0;
	data |= m_hd6402->get_output_pin(AY31015_TBMT) ? 0x40 : 0;
	data |= m_hd6402->get_output_pin(AY31015_DAV ) ? 0x80 : 0;
	m_hd6402->set_input_pin(AY31015_SWE, 1);

	return data;
}

READ8_MEMBER( nascom_state::nascom1_hd6402_si )
{
	return 1;
}

WRITE8_MEMBER( nascom_state::nascom1_hd6402_so )
{
}

DEVICE_IMAGE_LOAD_MEMBER( nascom_state, nascom1_cassette )
{
	m_tape_size = image.length();
	m_tape_image = (UINT8*)image.ptr();

	if (!m_tape_image)
		return IMAGE_INIT_FAIL;

	m_tape_index = 0;
	return IMAGE_INIT_PASS;
}

DEVICE_IMAGE_UNLOAD_MEMBER( nascom_state, nascom1_cassette )
{
	m_tape_image = NULL;
	m_tape_size = m_tape_index = 0;
}


//**************************************************************************
//  SNAPSHOTS
//**************************************************************************

SNAPSHOT_LOAD_MEMBER( nascom_state, nascom1 )
{
	UINT8 line[35];

	while (image.fread( &line, sizeof(line)) == sizeof(line))
	{
		unsigned int addr, b0, b1, b2, b3, b4, b5, b6, b7, dummy;

		if (sscanf((char *)line, "%x %x %x %x %x %x %x %x %x %x\010\010\n",
			&addr, &b0, &b1, &b2, &b3, &b4, &b5, &b6, &b7, &dummy) == 10)
		{
			m_maincpu->space(AS_PROGRAM).write_byte(addr++, b0);
			m_maincpu->space(AS_PROGRAM).write_byte(addr++, b1);
			m_maincpu->space(AS_PROGRAM).write_byte(addr++, b2);
			m_maincpu->space(AS_PROGRAM).write_byte(addr++, b3);
			m_maincpu->space(AS_PROGRAM).write_byte(addr++, b4);
			m_maincpu->space(AS_PROGRAM).write_byte(addr++, b5);
			m_maincpu->space(AS_PROGRAM).write_byte(addr++, b6);
			m_maincpu->space(AS_PROGRAM).write_byte(addr++, b7);
		}
	}

	return IMAGE_INIT_PASS;
}


//**************************************************************************
//  SOCKETS
//**************************************************************************

int nascom2_state::load_cart(device_image_interface &image, generic_slot_device *slot, int slot_id)
{
	// loading directly from file
	if (image.software_entry() == NULL)
	{
		if (slot->length() > 0x1000)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported file size");
			return IMAGE_INIT_FAIL;
		}

		slot->rom_alloc(slot->length(), GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
		slot->fread(slot->get_rom_base(), slot->length());

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

	// loading from software list. this supports multiple regions to load to
	else
	{
		UINT8 *region_b000 = image.get_software_region("b000");
		UINT8 *region_c000 = image.get_software_region("c000");
		UINT8 *region_d000 = image.get_software_region("d000");

		if (region_b000 != NULL)
		{
			UINT32 size = image.get_software_region_length("b000");
			m_maincpu->space(AS_PROGRAM).install_rom(0xb000, 0xb000 + size - 1, region_b000);
		}

		if (region_c000 != NULL)
		{
			UINT32 size = image.get_software_region_length("c000");
			m_maincpu->space(AS_PROGRAM).install_rom(0xc000, 0xc000 + size - 1, region_c000);
		}

		if (region_d000 != NULL)
		{
			UINT32 size = image.get_software_region_length("d000");
			m_maincpu->space(AS_PROGRAM).install_rom(0xd000, 0xd000 + size - 1, region_d000);
		}
	}

	return IMAGE_INIT_PASS;
}


//**************************************************************************
//  DRIVER INIT
//**************************************************************************

void nascom_state::machine_reset()
{
	// Set up hd6402 pins
	m_hd6402->set_input_pin(AY31015_SWE, 1);

	m_hd6402->set_input_pin(AY31015_CS, 0);
	m_hd6402->set_input_pin(AY31015_NP, 1);
	m_hd6402->set_input_pin(AY31015_NB1, 1);
	m_hd6402->set_input_pin(AY31015_NB2, 1);
	m_hd6402->set_input_pin(AY31015_EPS, 1);
	m_hd6402->set_input_pin(AY31015_TSB, 1);
	m_hd6402->set_input_pin(AY31015_CS, 1);
}

DRIVER_INIT_MEMBER( nascom_state, nascom )
{
	// install extra memory
	if (m_ram->size() > 0)
	{
		m_maincpu->space(AS_PROGRAM).install_ram(0x1000, 0x1000 + m_ram->size() - 1, m_ram->pointer());
	}
}

void nascom2_state::machine_reset()
{
	// base machine reset
	nascom_state::machine_reset();

	// restart address (on the real system, a12 to a15 are forced to 1 for one memory cycle)
	m_maincpu->set_state_int(Z80_PC, m_lsw1->read() << 12);
}

DRIVER_INIT_MEMBER( nascom2_state, nascom2 )
{
	DRIVER_INIT_CALL(nascom);

	// setup nasbus
	m_nasbus->set_program_space(&m_maincpu->space(AS_PROGRAM));
	m_nasbus->set_io_space(&m_maincpu->space(AS_IO));
}

// since we don't know for which regions we should disable ram, we just let other devices
// overwrite the region they need, and re-install our ram when they are disabled
WRITE_LINE_MEMBER( nascom2_state::ram_disable_w )
{
	if (state)
	{
		// enable ram again
		m_maincpu->space(AS_PROGRAM).install_ram(0x1000, 0x1000 + m_ram->size() - 1, m_ram->pointer());
	}
}

DRIVER_INIT_MEMBER( nascom2_state, nascom2c )
{
	// install memory
	m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x0000 + m_ram->size() - 1, m_ram->pointer());

	// setup nasbus
	m_nasbus->set_program_space(&m_maincpu->space(AS_PROGRAM));
	m_nasbus->set_io_space(&m_maincpu->space(AS_IO));
}

WRITE_LINE_MEMBER( nascom2_state::ram_disable_cpm_w )
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

static GFXDECODE_START( nascom1 )
	GFXDECODE_ENTRY("gfx1", 0x0000, nascom1_charlayout, 0, 1)
GFXDECODE_END

UINT32 nascom1_state::screen_update_nascom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

static GFXDECODE_START( nascom2 )
	GFXDECODE_ENTRY("gfx1", 0x0000, nascom2_charlayout, 0, 1)
GFXDECODE_END

UINT32 nascom2_state::screen_update_nascom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

static ADDRESS_MAP_START( nascom1_mem, AS_PROGRAM, 8, nascom1_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM // MONITOR
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM // WRAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( nascom1_io, AS_IO, 8, nascom1_state )
	ADDRESS_MAP_GLOBAL_MASK(0x0f)
	AM_RANGE(0x00, 0x00) AM_READWRITE(nascom1_port_00_r, nascom1_port_00_w)
	AM_RANGE(0x01, 0x01) AM_READWRITE(nascom1_port_01_r, nascom1_port_01_w)
	AM_RANGE(0x02, 0x02) AM_READ(nascom1_port_02_r)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("z80pio", z80pio_device, read, write )
ADDRESS_MAP_END

static ADDRESS_MAP_START( nascom2_mem, AS_PROGRAM, 8, nascom2_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM // MONITOR
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM // WRAM
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("basic", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( nascom2_io, AS_IO, 8, nascom2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(nascom1_port_00_r, nascom1_port_00_w)
	AM_RANGE(0x01, 0x01) AM_READWRITE(nascom1_port_01_r, nascom1_port_01_w)
	AM_RANGE(0x02, 0x02) AM_READ(nascom1_port_02_r)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("z80pio", z80pio_device, read, write )
ADDRESS_MAP_END

static ADDRESS_MAP_START( nascom2c_mem, AS_PROGRAM, 8, nascom2_state )
	AM_RANGE(0xf000, 0xf7ff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0xf800, 0xfbff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xfc00, 0xffff) AM_RAM // WRAM
ADDRESS_MAP_END


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( nascom1 )
	PORT_START("KEY.0")
	PORT_BIT(0x6f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

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
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('\xA3')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)     PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0')
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

	PORT_START("KEY.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace ClearScreen") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("New Line")              PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x58, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(UCHAR_SHIFT_2) PORT_CHAR('@')
INPUT_PORTS_END

static INPUT_PORTS_START( nascom2 )
	PORT_INCLUDE(nascom1)

	PORT_MODIFY("KEY.6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                            PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('[') PORT_CHAR('\\')

	PORT_MODIFY("KEY.7")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                            PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(']') PORT_CHAR('_')

	PORT_MODIFY("KEY.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back CS")       PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter  Escape") PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)  PORT_CHAR(27)

	// link switch on board
	PORT_START("lsw1")
	PORT_DIPNAME(0x0f, 0x00, "Restart address")
	PORT_DIPLOCATION("LSW1:2,3,4,5")
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
	PORT_DIPLOCATION("LSW1:2,3,4,5")
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


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

static MACHINE_CONFIG_START( nascom1, nascom1_state )
	// main cpu
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 8)
	MCFG_CPU_PROGRAM_MAP(nascom1_mem)
	MCFG_CPU_IO_MAP(nascom1_io)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(48 * 8, 16 * 16)
	MCFG_SCREEN_VISIBLE_AREA(0, 48 * 8 - 1, 0, 16 * 16 - 1)
	MCFG_SCREEN_UPDATE_DRIVER(nascom1_state, screen_update_nascom)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", nascom1)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	// uart
	MCFG_DEVICE_ADD( "hd6402", AY31015, 0 )
	MCFG_AY31015_TX_CLOCK(( XTAL_16MHz / 16 ) / 256)
	MCFG_AY31015_RX_CLOCK(( XTAL_16MHz / 16 ) / 256)
	MCFG_AY51013_READ_SI_CB(READ8(nascom_state, nascom1_hd6402_si))
	MCFG_AY51013_WRITE_SO_CB(WRITE8(nascom_state, nascom1_hd6402_so))

	// cassette is connected to the uart
	MCFG_CASSETTE_ADD("cassette")

	// pio
	MCFG_DEVICE_ADD("z80pio", Z80PIO, XTAL_16MHz/8)

	// internal extra ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("48K")
	MCFG_RAM_EXTRA_OPTIONS("8K,16K,32K")

	// devices
	MCFG_SNAPSHOT_ADD("snapshot", nascom_state, nascom1, "nas", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( nascom2, nascom1, nascom2_state )
	MCFG_CPU_REPLACE("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(nascom2_mem)
	MCFG_CPU_IO_MAP(nascom2_io)

	// video hardware
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(48 * 8, 16 * 14)
	MCFG_SCREEN_VISIBLE_AREA(0, 48 * 8 - 1, 0, 16 * 14 - 1)
	MCFG_SCREEN_UPDATE_DRIVER(nascom2_state, screen_update_nascom)

	MCFG_GFXDECODE_MODIFY("gfxdecode", nascom2)

	// generic sockets for ram/rom (todo: support ram here)
	MCFG_GENERIC_SOCKET_ADD("socket1", generic_plain_slot, "nascom_socket")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(nascom2_state, socket1_load)
	MCFG_GENERIC_SOCKET_ADD("socket2", generic_plain_slot, "nascom_socket")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(nascom2_state, socket2_load)

	MCFG_SOFTWARE_LIST_ADD("socket_list", "nascom_socket")

	// nasbus expansion bus
	MCFG_NASBUS_ADD(NASBUS_TAG)
	MCFG_NASBUS_RAM_DISABLE_HANDLER(WRITELINE(nascom2_state, ram_disable_w))
	MCFG_NASBUS_SLOT_ADD("nasbus1", nasbus_slot_cards, NULL)
	MCFG_NASBUS_SLOT_ADD("nasbus2", nasbus_slot_cards, NULL)
	MCFG_NASBUS_SLOT_ADD("nasbus3", nasbus_slot_cards, NULL)
	MCFG_NASBUS_SLOT_ADD("nasbus4", nasbus_slot_cards, NULL)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( nascom2c, nascom2, nascom2_state )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(nascom2c_mem)

	MCFG_DEVICE_REMOVE(RAM_TAG)
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("60K")

	MCFG_DEVICE_MODIFY(NASBUS_TAG)
	MCFG_NASBUS_RAM_DISABLE_HANDLER(WRITELINE(nascom2_state, ram_disable_cpm_w))
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( nascom1 )
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "T4", "NasBug T4")
	ROMX_LOAD("nasbugt4.rom", 0x0000, 0x0800, CRC(f391df68) SHA1(00218652927afc6360c57e77d6a4fd32d4e34566), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "T1", "NasBug T1")
	ROMX_LOAD("nasbugt1.rom", 0x0000, 0x0400, CRC(8ea07054) SHA1(3f9a8632826003d6ea59d2418674d0fb09b83a4c), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "T2", "NasBug T2")
	ROMX_LOAD("nasbugt2.rom", 0x0000, 0x0400, CRC(e371b58a) SHA1(485b20a560b587cf9bb4208ba203b12b3841689b), ROM_BIOS(3))

	ROM_REGION(0x0800, "gfx1", 0)
	ROM_LOAD("nascom1.chr",   0x0000, 0x0800, CRC(33e92a04) SHA1(be6e1cc80e7f95a032759f7df19a43c27ff93a52))
ROM_END

ROM_START( nascom2 )
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "NS3", "NasSys 3")
	ROMX_LOAD("nassys3.rom", 0x0000, 0x0800, CRC(3da17373) SHA1(5fbda15765f04e4cd08cf95c8d82ce217889f240), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "NS1", "NasSys 1")
	ROMX_LOAD("nassys1.rom", 0x0000, 0x0800, CRC(b6300716) SHA1(29da7d462ba3f569f70ed3ecd93b981f81c7adfa), ROM_BIOS(2))

	ROM_REGION(0x2000, "basic", 0)
	ROM_LOAD("basic.rom", 0x0000, 0x2000, CRC(5cb5197b) SHA1(c41669c2b6d6dea808741a2738426d97bccc9b07))

	ROM_REGION(0x1000, "gfx1", 0)
	ROM_LOAD("nascom1.chr", 0x0000, 0x0800, CRC(33e92a04) SHA1(be6e1cc80e7f95a032759f7df19a43c27ff93a52))
	ROM_LOAD("nasgra.chr",  0x0800, 0x0800, CRC(2bc09d32) SHA1(d384297e9b02cbcb283c020da51b3032ff62b1ae))
ROM_END

ROM_START( nascom2c )
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("cpmboot.rom", 0x0000, 0x0800, CRC(44b67ffc) SHA1(60c8335f24798f8de7ad48a4cd03e56a60d87b63))

	ROM_REGION(0x1000, "gfx1", 0)
	ROM_LOAD("nascom1.chr", 0x0000, 0x0800, CRC(33e92a04) SHA1(be6e1cc80e7f95a032759f7df19a43c27ff93a52))
	ROM_LOAD("nasgra.chr",  0x0800, 0x0800, CRC(2bc09d32) SHA1(d384297e9b02cbcb283c020da51b3032ff62b1ae))
ROM_END


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS          INIT     COMPANY                  FULLNAME           FLAGS */
COMP( 1977, nascom1,  0,        0,      nascom1,  nascom1,  nascom_state,  nascom,  "Nascom Microcomputers", "Nascom 1",        GAME_NO_SOUND_HW )
COMP( 1979, nascom2,  0,        0,      nascom2,  nascom2,  nascom2_state, nascom2,  "Nascom Microcomputers", "Nascom 2",        GAME_NO_SOUND_HW )
COMP( 1980, nascom2c, nascom2,  0,      nascom2c, nascom2c, nascom2_state, nascom2c, "Nascom Microcomputers", "Nascom 2 (CP/M)", GAME_NO_SOUND_HW )
