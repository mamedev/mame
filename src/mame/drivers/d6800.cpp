// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************

    The Dream 6800 is a CHIP-8 computer roughly modelled on the Cosmac VIP.
    It was described in Electronics Australia magazine in 4 articles starting
    in May 1979. It has 1k of ROM and 1k of RAM. The video consists of 64x32
    pixels. The keyboard is a hexcode 4x4 matrix, plus a Function key.

    Designed by Michael Bauer, of the Division of Computing and Mathematics
    at Deakin University, Australia.

    NOTE that the display only updates after each 4 digits is entered, and
    you can't see what you type as you change bytes. This is by design.

    The cassette has no checksum, header or blocks. It is simply a stream
    of pulses. The successful loading of a tape is therefore a matter of luck.

    To modify memory, press RST, then enter the 4-digit address (nothing happens
    until the 4th digit is pressed), then press FN, then 0, then the 2 digit data.
    It will enter the data (you won't see anything), then the address will increment.
    Enter the data for this new address. If you want to skip this address, press FN.
    When you're done, press RST. NOTE!!! Do NOT change any of these addresses:
    0000,0001,0006-007F, or the system may crash. It's recommended to start all
    your programs at 0200.

    Function keys:
    FN 0 - Modify memory - see above paragraph.

    FN 1 - Tape load. You must have entered the start address at 0002, and
           the end address+1 at 0004 (big-endian).

    FN 2 - Tape save. You must have entered the start address at 0002, and
           the end address+1 at 0004 (big-endian).

    FN 3 - Run. To use, press RST, then enter the 4-digit start address
           (nothing happens until the 4th digit is pressed), then FN, then 3.

    All CHIP-8 programs load at 0200 (max size 4k), and exec address
    is C000.

    Information and programs can be found at http://chip8.com/?page=78

**********************************************************************************/


#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class d6800_state : public driver_device
{
public:
	d6800_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_pia(*this, "pia")
		, m_beeper(*this, "beeper")
		, m_videoram(*this, "videoram")
		, m_io_x0(*this, "X0")
		, m_io_x1(*this, "X1")
		, m_io_x2(*this, "X2")
		, m_io_x3(*this, "X3")
		, m_io_y0(*this, "Y0")
		, m_io_y1(*this, "Y1")
		, m_io_y2(*this, "Y2")
		, m_io_y3(*this, "Y3")
		, m_io_shift(*this, "SHIFT")
		{ }


	void d6800(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

private:
	DECLARE_READ8_MEMBER( d6800_cassette_r );
	DECLARE_WRITE8_MEMBER( d6800_cassette_w );
	DECLARE_READ8_MEMBER( d6800_keyboard_r );
	DECLARE_WRITE8_MEMBER( d6800_keyboard_w );
	DECLARE_WRITE_LINE_MEMBER( d6800_screen_w );
	uint32_t screen_update_d6800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	void d6800_map(address_map &map);

	uint8_t m_rtc;
	bool m_cb2;
	bool m_cassold;
	uint8_t m_cass_data[4];
	uint8_t m_portb;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<pia6821_device> m_pia;
	required_device<beep_device> m_beeper;
	required_shared_ptr<uint8_t> m_videoram;
	required_ioport m_io_x0;
	required_ioport m_io_x1;
	required_ioport m_io_x2;
	required_ioport m_io_x3;
	required_ioport m_io_y0;
	required_ioport m_io_y1;
	required_ioport m_io_y2;
	required_ioport m_io_y3;
	required_ioport m_io_shift;
};


/* Memory Maps */

void d6800_state::d6800_map(address_map &map)
{
	map(0x0000, 0x00ff).ram();
	map(0x0100, 0x01ff).ram().share("videoram");
	map(0x0200, 0x17ff).ram();
	//map(0x1800, 0x1fff).rom();   // for dreamsoft_1, if we can find a good copy
	map(0x8010, 0x8013).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc000, 0xc7ff).mirror(0x3800).rom();
}

/* Input Ports */

static INPUT_PORTS_START( d6800 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y0")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("Y1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("Y2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("Y3")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("SHIFT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FN") PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RST") PORT_CODE(KEYCODE_LALT) PORT_CHANGED_MEMBER(DEVICE_SELF, d6800_state, reset_button, 0)

	PORT_START("VS")
	/* vblank */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_VBLANK("screen")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(d6800_state::reset_button)
{
	// RESET button wired to POR on the mc6875, which activates the Reset output pin which in turn connects to the CPU's Reset pin.
	if (newval)
		m_pia->reset();
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

/* Video */
uint32_t d6800_state::screen_update_d6800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t x,y,gfx=0;

	for (y = 0; y < 32; y++)
	{
		uint16_t *p = &bitmap.pix16(y);

		for (x = 0; x < 8; x++)
		{
			if (m_cb2)
				gfx = m_videoram[ x | (y<<3)];

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
	return 0;
}

/* NE556 */

TIMER_DEVICE_CALLBACK_MEMBER(d6800_state::kansas_w)
{
	m_cass_data[3]++;

	if (BIT(m_portb, 0) != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = BIT(m_portb, 0);
	}

	if (BIT(m_portb, 0))
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

/* PIA6821 Interface */

TIMER_DEVICE_CALLBACK_MEMBER(d6800_state::kansas_r)
{
	m_rtc++;
	if (m_rtc > 159)
		m_rtc = 0;

	uint8_t data = m_io_x0->read() & m_io_x1->read() & m_io_x2->read() & m_io_x3->read();
	int ca1 = (data == 255) ? 0 : 1;
	int ca2 = m_io_shift->read();
	int cb1 = (m_rtc) ? 1 : 0;

	m_pia->ca1_w(ca1);
	m_pia->ca2_w(ca2);
	m_pia->cb1_w(cb1);

	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	uint8_t cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_cass_data[2] = ((m_cass_data[1] < 12) ? 128 : 0);
		m_cass_data[1] = 0;
	}
}


WRITE_LINE_MEMBER( d6800_state::d6800_screen_w )
{
	m_cb2 = state;
}

READ8_MEMBER( d6800_state::d6800_cassette_r )
{
	/*
	Cassette circuit consists of a 741 op-amp, a 74121 oneshot, and a 74LS74.
	When a pulse arrives, the oneshot is set. After a preset time, it triggers
	and the 74LS74 compares this pulse to the output of the 741. Therefore it
	knows if the tone is 1200 or 2400 Hz. Input to PIA is bit 7.
	*/

	return m_cass_data[2] | m_portb;
}

WRITE8_MEMBER( d6800_state::d6800_cassette_w )
{
	/*
	    A NE556 runs at either 1200 or 2400 Hz, depending on the state of bit 0.
	    This output drives the speaker and the output signal to the cassette player.
	    Bit 6 enables the speaker. Also the speaker is silenced when cassette operations
	    are in progress (DMA/CB2 line low).
	*/

	m_beeper->set_clock(BIT(data, 0) ? 2400 : 1200);
	m_beeper->set_state(BIT(data, 6) & (m_cb2 ? 1 : 0));

	m_portb = data & 0x7f;
}

READ8_MEMBER( d6800_state::d6800_keyboard_r )
{
	/*
	This system reads the key matrix one way, then swaps the input and output
	lines around and reads it another way. This isolates the key that was pressed.
	*/

	uint8_t data = m_io_x0->read() & m_io_x1->read() & m_io_x2->read() & m_io_x3->read()
				& m_io_y0->read() & m_io_y1->read() & m_io_y2->read() & m_io_y3->read();

	return data;
}

WRITE8_MEMBER( d6800_state::d6800_keyboard_w )
{
	/*

	    bit     description

	    PA0     keyboard column 0
	    PA1     keyboard column 1
	    PA2     keyboard column 2
	    PA3     keyboard column 3
	    PA4     keyboard row 0
	    PA5     keyboard row 1
	    PA6     keyboard row 2
	    PA7     keyboard row 3

	*/

}

/* Machine Initialization */

void d6800_state::machine_start()
{
}

void d6800_state::machine_reset()
{
	m_beeper->set_state(0);
	m_rtc = 0;
	m_cass_data[0] = 0;
	m_cass_data[1] = 0;
	m_cass_data[2] = 128;
	m_cass_data[3] = 0;
}

/* Machine Drivers */

QUICKLOAD_LOAD_MEMBER(d6800_state::quickload_cb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int i;
	int quick_addr = 0x200;
	int exec_addr = 0xc000;
	int quick_length;
	std::vector<uint8_t> quick_data;
	int read_;
	image_init_result result = image_init_result::FAIL;

	quick_length = image.length();
	quick_data.resize(quick_length);
	read_ = image.fread( &quick_data[0], quick_length);
	if (read_ != quick_length)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot read the file");
		image.message(" Cannot read the file");
	}
	else
	{
		for (i = 0; i < quick_length; i++)
			if ((quick_addr + i) < 0x1000)
				space.write_byte(i + quick_addr, quick_data[i]);

		/* display a message about the loaded quickload */
		image.message(" Quickload: size=%04X : start=%04X : end=%04X : exec=%04X",quick_length,quick_addr,quick_addr+quick_length,exec_addr);

		// Start the quickload
		if (image.is_filetype("bin"))
			m_maincpu->set_pc(quick_addr);
		else
			m_maincpu->set_pc(exec_addr);

		result = image_init_result::PASS;
	}

	return result;
}

void d6800_state::d6800(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(4'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &d6800_state::d6800_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_size(64, 32);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(d6800_state::screen_update_d6800));
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(25));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 1200).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	PIA6821(config, m_pia, 0);
	m_pia->readpa_handler().set(FUNC(d6800_state::d6800_keyboard_r));
	m_pia->readpb_handler().set(FUNC(d6800_state::d6800_cassette_r));
	m_pia->writepa_handler().set(FUNC(d6800_state::d6800_keyboard_w));
	m_pia->writepb_handler().set(FUNC(d6800_state::d6800_cassette_w));
	m_pia->cb2_handler().set(FUNC(d6800_state::d6800_screen_w));
	m_pia->irqa_handler().set_inputline("maincpu", M6800_IRQ_LINE);
	m_pia->irqb_handler().set_inputline("maincpu", M6800_IRQ_LINE);

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	TIMER(config, "kansas_w").configure_periodic(FUNC(d6800_state::kansas_w), attotime::from_hz(4800));
	TIMER(config, "kansas_r").configure_periodic(FUNC(d6800_state::kansas_r), attotime::from_hz(40000));

	/* quickload */
	QUICKLOAD(config, "quickload", "bin,c8,ch8", attotime::from_seconds(1)).set_load_callback(FUNC(d6800_state::quickload_cb), this);
}

/* ROMs */

ROM_START( d6800 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "0", "Original")   // "chipos"
	ROMX_LOAD( "d6800.bin",     0xc000, 0x0400, CRC(3f97ca2e) SHA1(60f26e57a058262b30befceceab4363a5d65d877), ROM_BIOS(0) )
	ROM_RELOAD(                 0xc400, 0x0400 )
	//ROMX_LOAD( "d6800d1.bin",   0x1800, 0x0800, BAD_DUMP CRC(e552cae3) SHA1(0b90504922d46b9c46278924768c45b1b276709f), ROM_BIOS(0) )   // need a good dump, this one is broken
	ROM_SYSTEM_BIOS(1, "d2", "Dreamsoft2")
	ROMX_LOAD( "d6800d2.bin",   0xc000, 0x0800, CRC(ded5712f) SHA1(f594f313a74d7135c9fdd0bcb0093fc5771a9b7d), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "d2m", "Dreamsoft2m")
	ROMX_LOAD( "d6800d2m.bin",  0xc000, 0x0800, CRC(eec8e56f) SHA1(f587ccbc0872f2982d61120d033f481a862b902b), ROM_BIOS(2) )
ROM_END

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY          FULLNAME      FLAGS
COMP( 1979, d6800, 0,      0,      d6800,   d6800, d6800_state, empty_init, "Michael Bauer", "Dream 6800", 0 )
