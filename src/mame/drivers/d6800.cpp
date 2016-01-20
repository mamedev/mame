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

    Function keys:
    FN 0 - Modify memory - firstly enter a 4-digit address, then 2-digit data
                    the address will increment by itself, enter the next byte.
                    FN by itself will step to the next address.

    FN 1 - Tape load. You must have entered the start address at 0002, and
           the end address+1 at 0004 (big-endian).

    FN 2 - Tape save. You must have entered the start address at 0002, and
           the end address+1 at 0004 (big-endian).

    FN 3 - Run. You must have entered the 4-digit go address first.

    All CHIP-8 programs load at 0x200 (max size 4k), and exec address
    is C000.

    Information and programs can be found at http://chip8.com/?page=78


**********************************************************************************/


#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "sound/beep.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "sound/wave.h"
#include "machine/6821pia.h"


class d6800_state : public driver_device
{
public:
	d6800_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cass(*this, "cassette"),
		m_pia(*this, "pia"),
		m_beeper(*this, "beeper"),
		m_videoram(*this, "videoram"),
		m_io_x0(*this, "X0"),
		m_io_x1(*this, "X1"),
		m_io_x2(*this, "X2"),
		m_io_x3(*this, "X3"),
		m_io_y0(*this, "Y0"),
		m_io_y1(*this, "Y1"),
		m_io_y2(*this, "Y2"),
		m_io_y3(*this, "Y3"),
		m_io_shift(*this, "SHIFT") { }

	DECLARE_READ8_MEMBER( d6800_cassette_r );
	DECLARE_WRITE8_MEMBER( d6800_cassette_w );
	DECLARE_READ8_MEMBER( d6800_keyboard_r );
	DECLARE_WRITE8_MEMBER( d6800_keyboard_w );
	DECLARE_WRITE_LINE_MEMBER( d6800_screen_w );
	UINT32 screen_update_d6800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(d6800_c);
	TIMER_DEVICE_CALLBACK_MEMBER(d6800_p);
	DECLARE_QUICKLOAD_LOAD_MEMBER( d6800 );
protected:
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<pia6821_device> m_pia;
	required_device<beep_device> m_beeper;
	required_shared_ptr<UINT8> m_videoram;
	required_ioport m_io_x0;
	required_ioport m_io_x1;
	required_ioport m_io_x2;
	required_ioport m_io_x3;
	required_ioport m_io_y0;
	required_ioport m_io_y1;
	required_ioport m_io_y2;
	required_ioport m_io_y3;
	required_ioport m_io_shift;
private:
	UINT8 m_rtc;
	bool m_cb2;
	bool m_cassold;
	UINT8 m_cass_data[4];
	UINT8 m_portb;
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


/* Memory Maps */

static ADDRESS_MAP_START( d6800_map, AS_PROGRAM, 8, d6800_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x01ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x0200, 0x0fff) AM_RAM
	AM_RANGE(0x8010, 0x8013) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0xc000, 0xc7ff) AM_MIRROR(0x3800) AM_ROM
ADDRESS_MAP_END

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

	PORT_START("VS")
	/* vblank */
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_VBLANK("screen")
INPUT_PORTS_END

/* Video */

UINT32 d6800_state::screen_update_d6800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 x,y,gfx=0;

	for (y = 0; y < 32; y++)
	{
		UINT16 *p = &bitmap.pix16(y);

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

TIMER_DEVICE_CALLBACK_MEMBER(d6800_state::d6800_c)
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

TIMER_DEVICE_CALLBACK_MEMBER(d6800_state::d6800_p)
{
	m_rtc++;
	if (m_rtc > 159)
		m_rtc = 0;

	UINT8 data = m_io_x0->read() & m_io_x1->read() & m_io_x2->read() & m_io_x3->read();
	int ca1 = (data == 255) ? 0 : 1;
	int ca2 = m_io_shift->read();
	int cb1 = (m_rtc) ? 1 : 0;

	m_pia->ca1_w(ca1);
	m_pia->ca2_w(ca2);
	m_pia->cb1_w(cb1);

	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	UINT8 cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

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

	m_beeper->set_frequency(BIT(data, 0) ? 2400 : 1200);
	m_beeper->set_state(BIT(data, 6) & (m_cb2 ? 1 : 0));

	m_portb = data & 0x7f;
}

READ8_MEMBER( d6800_state::d6800_keyboard_r )
{
	/*
	This system reads the key matrix one way, then swaps the input and output
	lines around and reads it another way. This isolates the key that was pressed.
	*/

	UINT8 data = m_io_x0->read() & m_io_x1->read() & m_io_x2->read() & m_io_x3->read()
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

QUICKLOAD_LOAD_MEMBER( d6800_state, d6800 )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int i;
	int quick_addr = 0x200;
	int exec_addr = 0xc000;
	int quick_length;
	dynamic_buffer quick_data;
	int read_;
	int result = IMAGE_INIT_FAIL;

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
		if (strcmp(image.filetype(), "bin") == 0)
			m_maincpu->set_pc(quick_addr);
		else
			m_maincpu->set_pc(exec_addr);

		result = IMAGE_INIT_PASS;
	}

	return result;
}

static MACHINE_CONFIG_START( d6800, d6800_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6800, XTAL_4MHz/4)
	MCFG_CPU_PROGRAM_MAP(d6800_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(64, 32)
	MCFG_SCREEN_VISIBLE_AREA(0, 63, 0, 31)
	MCFG_SCREEN_UPDATE_DRIVER(d6800_state, screen_update_d6800)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(25))
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* devices */
	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(d6800_state, d6800_keyboard_r))
	MCFG_PIA_READPB_HANDLER(READ8(d6800_state, d6800_cassette_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(d6800_state, d6800_keyboard_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(d6800_state, d6800_cassette_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(d6800_state, d6800_screen_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("d6800_c", d6800_state, d6800_c, attotime::from_hz(4800))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("d6800_p", d6800_state, d6800_p, attotime::from_hz(40000))

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", d6800_state, d6800, "bin,c8,ch8", 1)
MACHINE_CONFIG_END

/* ROMs */

ROM_START( d6800 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "0", "Original")
	ROMX_LOAD( "d6800.bin", 0xc000, 0x0400, CRC(3f97ca2e) SHA1(60f26e57a058262b30befceceab4363a5d65d877), ROM_BIOS(1) )
	ROMX_LOAD( "d6800.bin", 0xc400, 0x0400, CRC(3f97ca2e) SHA1(60f26e57a058262b30befceceab4363a5d65d877), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "1", "Dreamsoft")
	ROMX_LOAD( "d6800d.bin", 0xc000, 0x0800, CRC(ded5712f) SHA1(f594f313a74d7135c9fdd0bcb0093fc5771a9b7d), ROM_BIOS(2) )
ROM_END

/*    YEAR  NAME   PARENT  COMPAT  MACHINE   INPUT  CLASS,          INIT      COMPANY        FULLNAME      FLAGS */
COMP( 1979, d6800, 0,      0,      d6800,    d6800, driver_device,   0,   "Michael Bauer", "Dream 6800", 0 )
