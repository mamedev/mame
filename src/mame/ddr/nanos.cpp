// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Nanos

2009-05-12 Skeleton driver.

Status:
- Waiting for a floppy disk. Need software to proceed.

****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/z80daisy.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/z80pio.h"

#include "formats/nanos_dsk.h"

#include "emupal.h"
#include "screen.h"


namespace {

class nanos_state : public driver_device
{
public:
	nanos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_pio(*this, "pio")
		, m_pio0(*this, "pio0")
		, m_pio1(*this, "pio1")
		, m_sio0(*this, "sio0")
		, m_sio1(*this, "sio1")
		, m_ctc0(*this, "ctc0")
		, m_ctc1(*this, "ctc1")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_key_t(*this, "keyboard_timer")
		, m_p_chargen(*this, "chargen")
		, m_vram(*this, "videoram")
		, m_io_keyboard(*this, "LINE%u", 0U)
		, m_io_linec(*this, "LINEC")
	{ }

	void nanos(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void tc_w(uint8_t data);
	void ctc_z0_w(int state);
	void ctc_z1_w(int state);
	void ctc_z2_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);
	void z80daisy_interrupt(int state);
	uint8_t port_a_r();
	uint8_t port_b_r();
	void port_b_w(uint8_t data);
	static void floppy_formats(format_registration &fr);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t m_key_command;
	uint8_t m_last_code;
	uint8_t m_key_pressed;
	uint8_t row_number(uint8_t code);

	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
	required_device<z80pio_device> m_pio;
	required_device<z80pio_device> m_pio0;
	required_device<z80pio_device> m_pio1;
	required_device<z80sio_device> m_sio0;
	required_device<z80sio_device> m_sio1;
	required_device<z80ctc_device> m_ctc0;
	required_device<z80ctc_device> m_ctc1;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<timer_device> m_key_t;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_vram;
	required_ioport_array<7> m_io_keyboard;
	required_ioport m_io_linec;
};



void nanos_state::mem_map(address_map &map)
{
	map(0x0000, 0xf7ff).ram().share("mainram");
	map(0x0000, 0x0fff).bankr(m_bank1);
	map(0xf800, 0xffff).ram().share("videoram");
}

void nanos_state::tc_w(uint8_t data)
{
	m_fdc->tc_w(BIT(data,1));
}


/* Z80-CTC Interface */

void nanos_state::ctc_z0_w(int state)
{
}

void nanos_state::ctc_z1_w(int state)
{
}

void nanos_state::ctc_z2_w(int state)
{
}

/* Z80-SIO Interface */

void nanos_state::z80daisy_interrupt(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}

/* Z80 Daisy Chain */

static const z80_daisy_config daisy_chain[] =
{
	{ "pio" },
	{ "pio0" },
	{ "pio1" },
	{ "sio0" },
	{ "ctc0" },
	{ "sio1" },
	{ "ctc1" },
	{ nullptr }
};

void nanos_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	/* CPU card */
	map(0x00, 0x03).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));

	/* I/O card */
	map(0x80, 0x83).rw(m_pio0, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x84, 0x87).rw(m_sio0, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x88, 0x8B).rw(m_pio1, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x8C, 0x8F).rw(m_ctc0, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));

	/* FDC card */
	map(0x92, 0x92).w(FUNC(nanos_state::tc_w));
	map(0x94, 0x95).m(m_fdc, FUNC(upd765a_device::map));
	/* V24+IFSS card */
	map(0xA0, 0xA3).rw(m_sio0, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0xA4, 0xA7).rw(m_ctc1, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));

	/* 256-k RAM card I  -  64k OS-Memory + 192k-RAM-Floppy */
	//map(0xC0, 0xC7)

	/* 256-k RAM card II -  64k OS-Memory + 192k-RAM-Floppy */
	//map(0xC8, 0xCF)
}

/* Input ports */
static INPUT_PORTS_START( nanos )
	PORT_START("LINEC")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("]")  PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("~")  PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL")PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LF") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
INPUT_PORTS_END


uint32_t nanos_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  static uint8_t framecnt=0;
	uint16_t sy=0,ma=0;

//  framecnt++;

	for (uint8_t y = 0; y < 25; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 80; x++)
			{
				uint8_t gfx;
				if (ra < 8)
				{
					uint8_t const chr = m_vram[x];

					/* get pattern of pixels for that character scanline */
					gfx = m_p_chargen[(chr<<3) | ra ];
				}
				else
					gfx = 0;

				/* Display a scanline of a character (8 pixels) */
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
		ma+=80;
	}
	return 0;
}

uint8_t nanos_state::port_a_r()
{
	if (m_key_command==0)
		return m_key_pressed;
	else
	{
		uint8_t retVal = m_last_code;
		m_last_code = 0;
		return retVal;
	}
}

uint8_t nanos_state::port_b_r()
{
	return 0xff;
}


void nanos_state::port_b_w(uint8_t data)
{
	m_key_command = BIT(data,1);

	m_bank1->set_entry(BIT(data,7));
}

uint8_t nanos_state::row_number(uint8_t code)
{
	if (BIT(code, 0)) return 0;
	if (BIT(code, 1)) return 1;
	if (BIT(code, 2)) return 2;
	if (BIT(code, 3)) return 3;
	if (BIT(code, 4)) return 4;
	if (BIT(code, 5)) return 5;
	if (BIT(code, 6)) return 6;
	if (BIT(code, 7)) return 7;
	return 0;
}

// TODO: clean this up when the machine starts to work.
TIMER_DEVICE_CALLBACK_MEMBER(nanos_state::keyboard_callback)
{
	uint8_t key_code = 0;
	bool shift = BIT(m_io_linec->read(), 1);
	bool ctrl =  BIT(m_io_linec->read(), 0);
	m_key_pressed = 0xff;
	for(int i = 0; i < 7; i++)
	{
		uint8_t code = m_io_keyboard[i]->read();
		if (code != 0)
		{
			if (i==0 && !shift)
				key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs

			if (i==0 && shift)
				key_code = 0x20 + row_number(code) + 8*i; // for shifted numbers

			if (i==1 && !shift)
			{
				if (row_number(code) < 4)
					key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
				else
					key_code = 0x20 + row_number(code) + 8*i; // for numbers and some signs
			}

			if (i==1 && shift)
			{
				if (row_number(code) < 4)
					key_code = 0x20 + row_number(code) + 8*i; // for numbers and some signs
				else
					key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
			}

			if (i>=2 && i<=4 && shift && !ctrl)
				key_code = 0x60 + row_number(code) + (i-2)*8; // for small letters

			if (i>=2 && i<=4 && !shift && !ctrl)
				key_code = 0x40 + row_number(code) + (i-2)*8; // for big letters

			if (i>=2 && i<=4 && ctrl)
				key_code = 0x00 + row_number(code) + (i-2)*8; // for CTRL + letters

			if (i==5 && shift && !ctrl)
			{
				if (row_number(code)<7)
					key_code = 0x60 + row_number(code) + (i-2)*8; // for small letters
				else
					key_code = 0x40 + row_number(code) + (i-2)*8; // for signs it is switched
			}

			if (i==5 && !shift && !ctrl)
			{
				if (row_number(code)<7)
					key_code = 0x40 + row_number(code) + (i-2)*8; // for small letters
				else
					key_code = 0x60 + row_number(code) + (i-2)*8; // for signs it is switched
			}

			if (i==5 && !shift && ctrl)
				key_code = 0x00 + row_number(code) + (i-2)*8; // for letters + ctrl

			if (i==6)
			{
				switch(row_number(code))
				{
					case 0: key_code = 0x11; break;
					case 1: key_code = 0x12; break;
					case 2: key_code = 0x13; break;
					case 3: key_code = 0x14; break;
					case 4: key_code = 0x20; break; // Space
					case 5: key_code = 0x0D; break; // Enter
					case 6: key_code = 0x09; break; // TAB
					case 7: key_code = 0x0A; break; // LF
				}
			}
			m_last_code = key_code;
		}
	}

	if (key_code==0)
		m_key_pressed = 0xf7;
}

void nanos_state::machine_start()
{
	save_item(NAME(m_key_command));
	save_item(NAME(m_last_code));
	save_item(NAME(m_key_pressed));

	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);
}

void nanos_state::machine_reset()
{
	m_bank1->set_entry(1);
	m_floppy->get_device()->mon_w(0);
	m_key_pressed = 0xff;
}

void nanos_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_NANOS_FORMAT);
}

static void nanos_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
}

/* F4 Character Displayer */
static const gfx_layout nanos_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_nanos )
	GFXDECODE_ENTRY( "chargen", 0x0000, nanos_charlayout, 0, 1 )
GFXDECODE_END

void nanos_state::nanos(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &nanos_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &nanos_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(nanos_state::screen_update));
	screen.set_size(80*8, 25*10);
	screen.set_visarea(0,80*8-1,0,25*10-1);
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_nanos);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* devices */
	Z80CTC(config, m_ctc0, XTAL(4'000'000));
	m_ctc0->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc0->zc_callback<0>().set(FUNC(nanos_state::ctc_z0_w));
	m_ctc0->zc_callback<1>().set(FUNC(nanos_state::ctc_z1_w));
	m_ctc0->zc_callback<2>().set(FUNC(nanos_state::ctc_z2_w));

	Z80CTC(config, m_ctc1, XTAL(4'000'000));
	m_ctc1->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc1->zc_callback<0>().set(FUNC(nanos_state::ctc_z0_w));
	m_ctc1->zc_callback<1>().set(FUNC(nanos_state::ctc_z1_w));
	m_ctc1->zc_callback<2>().set(FUNC(nanos_state::ctc_z2_w));

	Z80PIO(config, m_pio0, XTAL(4'000'000));
	m_pio0->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80PIO(config, m_pio1, XTAL(4'000'000));
	m_pio1->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80SIO(config, m_sio0, XTAL(4'000'000));
	m_sio0->out_int_callback().set(FUNC(nanos_state::z80daisy_interrupt));

	Z80SIO(config, m_sio1, XTAL(4'000'000));
	m_sio1->out_int_callback().set(FUNC(nanos_state::z80daisy_interrupt));

	Z80PIO(config, m_pio, XTAL(4'000'000));
	m_pio->in_pa_callback().set(FUNC(nanos_state::port_a_r));
	m_pio->in_pb_callback().set(FUNC(nanos_state::port_b_r));
	m_pio->out_pb_callback().set(FUNC(nanos_state::port_b_w));

	/* UPD765 */
	UPD765A(config, m_fdc, 8'000'000, false, true);
	FLOPPY_CONNECTOR(config, m_floppy, nanos_floppies, "525hd", nanos_state::floppy_formats);

	TIMER(config, "keyboard_timer").configure_periodic(FUNC(nanos_state::keyboard_callback), attotime::from_hz(240));
}

/* ROM definition */
ROM_START( nanos )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "k7634_1.rom", 0x0000, 0x0800, CRC(8e34e6ac) SHA1(fd342f6effe991823c2a310737fbfcba213c4fe3))
	ROM_LOAD( "k7634_2.rom", 0x0800, 0x0180, CRC(4e01b02b) SHA1(8a279da886555c7470a1afcbb3a99693ea13c237))

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "zg_nanos.rom", 0x0000, 0x0800, CRC(5682d3f9) SHA1(5b738972c815757821c050ee38b002654f8da163))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                                                FULLNAME  FLAGS */
COMP( 1985, nanos, 0,      0,      nanos,   nanos, nanos_state, empty_init, "Ingenieurhochschule fur Seefahrt Warnemunde/Wustrow", "NANOS",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
