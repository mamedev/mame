// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
/***************************************************************************

Jupiter Ace memory map

    CPU: Z80
        0000-1fff ROM
        2000-23ff Mirror of 2400-27FF
        2400-27ff RAM (1K RAM used for screen and edit/cassette buffer)
        2800-2bff Mirror of 2C00-2FFF
        2c00-2fff RAM (1K RAM for char set, write only)
        3000-3bff Mirror of 3C00-3FFF
        3c00-3fff RAM (1K RAM standard)
        4000-7fff RAM (16K Expansion)
        8000-ffff RAM (48K Expansion)

Interrupts:

    IRQ:
        50Hz vertical sync

Ports:

    Out 0xfe:
        Tape and buzzer

    In 0xfe:
        Keyboard input, tape, and buzzer

    https://www.jupiter-ace.co.uk/

TODO:

    - Ace Colour Board
    - 96K ram expansion
    - Deep Thought disc interface (6850, 6821)

***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/z80pio.h"
#include "sound/ay8910.h"
#include "sound/sp0256.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/ace_tap.h"


namespace {

class ace_state : public driver_device
{
public:
	ace_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "i8255")
		, m_z80pio(*this, "z80pio")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_centronics(*this, "centronics")
		, m_ram(*this, "ram")
		, m_sp0256(*this, "sp0256")
		, m_video_ram(*this, "video_ram")
		, m_char_ram(*this, "char_ram")
		, m_key(*this, "KEY%u", 0)
		, m_joy(*this, "JOY")
	{ }

	void ace(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	SNAPSHOT_LOAD_MEMBER(snapshot_cb);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t io_r(offs_t offset);
	void io_w(uint8_t data);

	template<int N> uint8_t ppi_r() { return m_ppi->read(N); }
	template<int N> void ppi_w(uint8_t data) { m_ppi->write(N, data); }
	uint8_t sby_r();
	void ald_w(uint8_t data);

	template<int N> uint8_t pio_r() { return m_z80pio->read(N); }
	template<int N> void pio_w(uint8_t data) { m_z80pio->write(N, data); }
	uint8_t pio_pa_r();
	void pio_pa_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(set_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(clear_irq);

	void ace_io(address_map &map) ATTR_COLD;
	void ace_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<z80pio_device> m_z80pio;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;
	required_device<sp0256_device> m_sp0256;
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_char_ram;
	required_ioport_array<8> m_key;
	required_ioport m_joy;
};


/* Load in .ace files. These are memory images of 0x2000 to 0x7fff
   and compressed as follows:

   ED 00        : End marker
   ED <cnt> <byt>   : repeat <byt> count <cnt:1-240> times
   <byt>        : <byt>
*/

//**************************************************************************
//  Snapshot Handling
//**************************************************************************

SNAPSHOT_LOAD_MEMBER(ace_state::snapshot_cb)
{
	if (m_ram->size() < 0x4000)
		return std::make_pair(image_error::UNSUPPORTED, "At least 16KB RAM expansion required");

	u16 ace_index = 0x2000;
	bool done = false;

	logerror("Loading file %s.\r\n", image.filename());
	std::vector<uint8_t> RAM(0x10000);
	while (!done && (ace_index < 0x8001))
	{
		uint8_t ace_repeat, ace_byte;
		image.fread(&ace_byte, 1);
		if (ace_byte == 0xed)
		{
			image.fread(&ace_byte, 1);
			switch (ace_byte)
			{
			case 0x00:
					logerror("File loaded!\r\n");
					done = true;
					break;
			case 0x01:
					image.fread(&ace_byte, 1);
					RAM[ace_index++] = ace_byte;
					break;
			default:
					image.fread(&ace_repeat, 1);
					for (u8 loop = 0; loop < ace_byte; loop++)
						RAM[ace_index++] = ace_repeat;
					break;
			}
		}
		else
			RAM[ace_index++] = ace_byte;
	}

	logerror("Decoded %X bytes.\r\n", ace_index-0x2000);

	if (!done)
		return std::make_pair(image_error::INVALIDIMAGE, "Invalid snapshot file: EOF marker not found");

	// patch CPU registers
	// Some games do not follow the standard, and have rubbish in the CPU area. So,
	// we check that some other bytes are correct.
	// 2080 = memory size of original machine, should be 0000 or 8000 or C000.
	// 2118 = new stack pointer, do not use if between 8000 and FF00.

	ace_index = RAM[0x2080] | (RAM[0x2081] << 8);

	cpu_device *cpu = m_maincpu;
	if ((ace_index & 0x3fff) == 0)
	{
		cpu->set_state_int(Z80_AF, RAM[0x2100] | (RAM[0x2101] << 8));
		cpu->set_state_int(Z80_BC, RAM[0x2104] | (RAM[0x2105] << 8));
		cpu->set_state_int(Z80_DE, RAM[0x2108] | (RAM[0x2109] << 8));
		cpu->set_state_int(Z80_HL, RAM[0x210c] | (RAM[0x210d] << 8));
		cpu->set_state_int(Z80_IX, RAM[0x2110] | (RAM[0x2111] << 8));
		cpu->set_state_int(Z80_IY, RAM[0x2114] | (RAM[0x2115] << 8));
		cpu->set_pc(RAM[0x211c] | (RAM[0x211d] << 8));
		cpu->set_state_int(Z80_AF2, RAM[0x2120] | (RAM[0x2121] << 8));
		cpu->set_state_int(Z80_BC2, RAM[0x2124] | (RAM[0x2125] << 8));
		cpu->set_state_int(Z80_DE2, RAM[0x2128] | (RAM[0x2129] << 8));
		cpu->set_state_int(Z80_HL2, RAM[0x212c] | (RAM[0x212d] << 8));
		cpu->set_state_int(Z80_IM, RAM[0x2130]);
		cpu->set_state_int(Z80_IFF1, RAM[0x2134]);
		cpu->set_state_int(Z80_IFF2, RAM[0x2138]);
		cpu->set_state_int(Z80_I, RAM[0x213c]);
		cpu->set_state_int(Z80_R, RAM[0x2140]);

		if ((RAM[0x2119] < 0x80) || !ace_index)
			cpu->set_state_int(Z80_SP, RAM[0x2118] | (RAM[0x2119] << 8));
	}

	// copy data to the address space
	address_space &space = cpu->space(AS_PROGRAM);
	for (ace_index = 0x2000; ace_index < 0x8000; ace_index++)
		space.write_byte(ace_index, RAM[ace_index]);

	return std::make_pair(std::error_condition(), std::string());
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  I/O port 0xFE
//-------------------------------------------------

uint8_t ace_state::io_r(offs_t offset)
{
	uint8_t data = 0xff;

	// read keyboard rows
	for (int i = 0; i < 8; i++)
		if (!BIT(offset, i + 8))
			data &= m_key[i]->read();

	if (m_cassette->input() > 0.03)
		data &= ~0x20;

	if (!machine().side_effects_disabled())
		m_speaker->level_w(0);

	return data;
}

void ace_state::io_w(uint8_t data)
{
	m_cassette->output(BIT(data, 3) ? +1.0 : -1.0);
	m_speaker->level_w(1);
}


//-------------------------------------------------
//  I8255A
//-------------------------------------------------

uint8_t ace_state::sby_r()
{
	/*

	    bit     description

	    PC0     SP0256 SBY
	    PC1
	    PC2
	    PC3
	    PC4
	    PC5
	    PC6
	    PC7

	*/

	return m_sp0256->sby_r();
}

void ace_state::ald_w(uint8_t data)
{
	/*

	    bit     description

	    PA0     SP0256 A1
	    PA1     SP0256 A2
	    PA2     SP0256 A3
	    PA3     SP0256 A4
	    PA4     SP0256 A5
	    PA5     SP0256 A6
	    PA6     SP0256 _ALD
	    PA7

	*/

	if (!BIT(data, 6))
	{
		m_sp0256->ald_w(data & 0x3f);
	}
}


//-------------------------------------------------
//  Z80PIO
//-------------------------------------------------

uint8_t ace_state::pio_pa_r()
{
	/*

	    bit     description

	    PA0
	    PA1     RxD
	    PA2
	    PA3
	    PA4
	    PA5
	    PA6
	    PA7

	*/

	return 0;
}

void ace_state::pio_pa_w(uint8_t data)
{
	/*

	    bit     description

	    PA0     RTS
	    PA1
	    PA2     CTS
	    PA3     TxD
	    PA4
	    PA5
	    PA6     STB
	    PA7

	*/

	// centronics strobe
	m_centronics->write_strobe(!BIT(data, 6));
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void ace_state::ace_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).mirror(0x0400).ram().share(m_video_ram);
	map(0x2800, 0x2bff).mirror(0x0400).ram().share(m_char_ram);
	map(0x3000, 0x33ff).mirror(0x0c00).ram();
	map(0x4000, 0xffff).ram();
}

void ace_state::ace_io(address_map &map)
{
	map(0x00, 0x00).mirror(0x00fe).select(0xff00).rw(FUNC(ace_state::io_r), FUNC(ace_state::io_w));
	map(0x01, 0x01).mirror(0xff00).portr(m_joy);
	map(0x41, 0x41).mirror(0xff80).rw(FUNC(ace_state::ppi_r<0>), FUNC(ace_state::ppi_w<0>));
	map(0x43, 0x43).mirror(0xff80).rw(FUNC(ace_state::ppi_r<1>), FUNC(ace_state::ppi_w<1>));
	map(0x45, 0x45).mirror(0xff80).rw(FUNC(ace_state::ppi_r<2>), FUNC(ace_state::ppi_w<2>));
	map(0x47, 0x47).mirror(0xff80).rw(FUNC(ace_state::ppi_r<3>), FUNC(ace_state::ppi_w<3>));
	map(0x81, 0x81).mirror(0xff38).rw(FUNC(ace_state::pio_r<0>), FUNC(ace_state::pio_w<0>));
	map(0x83, 0x83).mirror(0xff38).rw(FUNC(ace_state::pio_r<1>), FUNC(ace_state::pio_w<1>));
	map(0x85, 0x85).mirror(0xff38).rw(FUNC(ace_state::pio_r<2>), FUNC(ace_state::pio_w<2>));
	map(0x87, 0x87).mirror(0xff38).rw(FUNC(ace_state::pio_r<3>), FUNC(ace_state::pio_w<3>));
	map(0xfd, 0xfd).mirror(0xff00).w("ay8910", FUNC(ay8910_device::address_w));
	map(0xff, 0xff).mirror(0xff00).rw("ay8910", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( ace )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_2) PORT_NAME("Symbol Shift")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(':')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(U'£')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR('?')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR('~')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR('|')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR('\\')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR('{')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR('}')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR('<')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR('>')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!') // DELETE LINE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@') // CAPS LOCK
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$') // INVERSE VIDEO
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_LEFT) PORT_CHAR('5') PORT_CHAR('%') PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR('_') // DELETE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')') // GRAPHICS
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR('8') PORT_CHAR('(') PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_DOWN)  PORT_CHAR('7') PORT_CHAR('\'') PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_UP)    PORT_CHAR('6') PORT_CHAR('&') PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(';')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(U'©')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(']')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR('[')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR('-')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR('^') PORT_NAME(u8"h  H  \u2191") // U+2191 = ↑
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ') PORT_NAME("Space / Break")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR('.')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(',')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR('/')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



//**************************************************************************
//  VIDEO
//**************************************************************************

//-------------------------------------------------
//  gfx_layout
//-------------------------------------------------

static const gfx_layout ace_charlayout =
{
	8,8,
	128,
	1,
	{ RGN_FRAC(0,1) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_ace )
	GFXDECODE_ENTRY( "maincpu", 0xfc00, ace_charlayout, 0, 1 )
GFXDECODE_END


//-------------------------------------------------
//  interrupt
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(ace_state::set_irq)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(ace_state::clear_irq)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}


//-------------------------------------------------
//  screen_update
//-------------------------------------------------

uint32_t ace_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy = 56, ma = 0;
	for (uint8_t y = 0; y < 24; y++)
	{
		for (uint8_t ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++, 40);

			for (uint16_t x = ma; x < ma+32; x++)
			{
				uint8_t const chr = m_video_ram[x];

				// get pattern of pixels for that character scanline
				uint8_t const gfx = m_char_ram[((chr & 0x7f) << 3) | ra] ^ (BIT(chr, 7) ? 0xff : 0);

				// display a scanline of a character (8 pixels)
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma += 32;
	}
	return 0;
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

void ace_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	// configure RAM
	switch (m_ram->size())
	{
	case 1*1024:
		program.unmap_readwrite(0x4000, 0xffff);
		break;

	case 16*1024:
		program.unmap_readwrite(0x8000, 0xffff);
		break;

	case 32*1024:
		program.unmap_readwrite(0xc000, 0xffff);
		break;
	}
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

void ace_state::ace(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 6.5_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ace_state::ace_mem);
	m_maincpu->set_addrmap(AS_IO, &ace_state::ace_io);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(ace_state::screen_update));
	screen.set_raw(6.5_MHz_XTAL, 416, 0, 336, 312, 0, 304);
	screen.set_palette("palette");

	TIMER(config, "set_irq").configure_scanline(FUNC(ace_state::set_irq), "screen", 31*8, 264);
	TIMER(config, "clear_irq").configure_scanline(FUNC(ace_state::clear_irq), "screen", 32*8, 264);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", "palette", gfx_ace);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 1.00);

	AY8910(config, "ay8910", 6.5_MHz_XTAL / 2).add_route(ALL_OUTPUTS, "mono", 0.25);

	SP0256(config, m_sp0256, 3_MHz_XTAL);
	m_sp0256->add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(ace_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("jupace_cass");

	// snapshot
	snapshot_image_device &snapshot(SNAPSHOT(config, "snapshot", "ace"));
	snapshot.set_delay(attotime::from_double(1.0));
	snapshot.set_load_callback(FUNC(ace_state::snapshot_cb));
	snapshot.set_interface("jupace_snap");

	I8255A(config, m_ppi);
	m_ppi->in_pb_callback().set(FUNC(ace_state::sby_r));
	m_ppi->out_pb_callback().set(FUNC(ace_state::ald_w));

	Z80PIO(config, m_z80pio, 6.5_MHz_XTAL/2);
	m_z80pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_z80pio->in_pa_callback().set(FUNC(ace_state::pio_pa_r));
	m_z80pio->out_pa_callback().set(FUNC(ace_state::pio_pa_w));
	m_z80pio->out_pb_callback().set("cent_data_out", FUNC(output_latch_device::write));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	// internal ram
	RAM(config, "ram").set_default_size("1K").set_extra_options("16K,32K,48K");

	SOFTWARE_LIST(config, "cass_list").set_original("jupace_cass");
	SOFTWARE_LIST(config, "snap_list").set_original("jupace_snap");
}



//**************************************************************************
//  ROMS
//**************************************************************************

ROM_START( jupace )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "rom-a.z1", 0x0000, 0x1000, CRC(dc8438a5) SHA1(8fa97eb71e5dd17c7d190c6587ee3840f839347c) )
	ROM_LOAD( "rom-b.z2", 0x1000, 0x1000, CRC(4009f636) SHA1(98c5d4bcd74bcf014268cf4c00b2007ea5cc21f3) )

	ROM_REGION( 0x1000, "fdc", 0 ) // Deep Thought disc interface
	ROM_LOAD( "dos_4.bin", 0x0000, 0x1000, CRC(04c70448) SHA1(53ddcced6ae2feafd687a3b55864726656b71412) )

	ROM_REGION( 0x800, "sp0256", 0 )
	ROM_LOAD( "sp0256-al2.ic1", 0x000, 0x800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

} // anonymous namespace



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY           FULLNAME       FLAGS
COMP( 1981, jupace, 0,      0,      ace,     ace,   ace_state, empty_init, "Jupiter Cantab", "Jupiter Ace", MACHINE_SUPPORTS_SAVE )
