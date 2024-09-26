// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************

    Proteus III computer.

    2015-10-02 Skeleton [Robbbert]

    Chips:
    6800 @ 894kHz
    6850 (TTY keyboard interface)
    6850 (Cassette interface)
    6820 (PIA for Keyboard and video
    6844 DMA
    MC14411 baud rate generator
    CRT96364 CRTC @1008kHz

    There's an undumped 74S287 prom (M24) in the video section.
    It converts ascii control codes into the crtc control codes

    Schematic has lots of errors and omissions.

    Like many systems of the time, the cassette is grossly over-complicated,
    using 12 chips for a standard Kansas-city interface. Speed = 600 baud.

    To use the serial keyboard, type in the command: PORT#1
    and to go back to the parallel keyboard type in: PORT#0

    The Basic seems rather buggy and does odd things from time to time.
    bios 0 doesn't seem to have any way to backspace and type over
    bios 0 is the only one to have the EDIT command, although no idea how
    to use it.
    bios 2 is from a compatible system called "Micro Systemes 1", from the
    same company.

    To Do:
    - Add support for k7 cassette files.
    - Need software
    - Need missing PROM, so that all the CRTC controls can be emulated
    - Keyboard may have its own CPU etc, but no info available.
    - Missing buttons: BYE, PANIC, SPEED, HERE-IS. Could be more.
    - Should be able to type in some low-res graphics from the keyboard, not implemented.

******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "imagedev/cassette.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/mc14411.h"
#include "machine/clock.h"
#include "machine/keyboard.h"
#include "machine/timer.h"

#include "bus/rs232/rs232.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class proteus3_state : public driver_device
{
public:
	proteus3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_pia(*this, "pia")
		, m_brg(*this, "brg")
		, m_acia1(*this, "acia1")
		, m_acia2(*this, "acia2")
		, m_cass(*this, "cassette")
		, m_serial(*this, "SERIAL")
	{ }

	void proteus3(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void ca2_w(int state);
	void video_w(u8 data);
	void kbd_put(u8 data);
	void acia1_clock_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	u32 screen_update_proteus3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// Clocks
	void write_acia_clocks(int id, int state);
	void write_f1_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F1, state); }
	void write_f2_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F2, state); }
	void write_f3_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F3, state); }
	void write_f4_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F4, state); }
	void write_f5_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F5, state); }
	void write_f6_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F6, state); }
	void write_f7_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F7, state); }
	void write_f8_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F8, state); }
	void write_f9_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F9, state); }
	void write_f10_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F10, state); }
	void write_f11_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F11, state); }
	void write_f12_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F12, state); }
	void write_f13_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F13, state); }
	void write_f14_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F14, state); }
	void write_f15_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F15, state); }

	void mem_map(address_map &map) ATTR_COLD;

	u8 m_video_data = 0U;
	u8 m_flashcnt = 0U;
	u16 m_curs_pos = 0U;
	u8 m_cass_data[4]{};
	bool m_cassbit = false, m_cassold = false, m_cassinbit = false;
	std::unique_ptr<u8[]> m_vram;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_device<pia6821_device> m_pia;
	required_device<mc14411_device> m_brg;
	required_device<acia6850_device> m_acia1; // cassette uart
	required_device<acia6850_device> m_acia2; // tty keyboard uart
	required_device<cassette_image_device> m_cass;

	// hardware configuration and things that need rewiring
	required_ioport             m_serial;
};




/******************************************************************************
 Address Maps
******************************************************************************/

void proteus3_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();
	map(0x8004, 0x8007).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8008, 0x8009).rw(m_acia1, FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // cassette
	map(0x8010, 0x8011).rw(m_acia2, FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // serial keyboard 7E2 (never writes data)
	map(0xc000, 0xffff).rom().region("maincpu", 0);
}


/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START(proteus3)
	PORT_START("SERIAL")
	PORT_CONFNAME(0x0F , 0x00 , "Serial Baud Rate") // F1-F16 pins on MC14411 in X16
	PORT_CONFSETTING(mc14411_device::TIMER_F1,  "9600")
	PORT_CONFSETTING(mc14411_device::TIMER_F2,  "7200")
	PORT_CONFSETTING(mc14411_device::TIMER_F3,  "4800")
	PORT_CONFSETTING(mc14411_device::TIMER_F4,  "3600")
	PORT_CONFSETTING(mc14411_device::TIMER_F5,  "2400")
	PORT_CONFSETTING(mc14411_device::TIMER_F6,  "1800")
	PORT_CONFSETTING(mc14411_device::TIMER_F7,  "1200")
	PORT_CONFSETTING(mc14411_device::TIMER_F8,  "600")
	PORT_CONFSETTING(mc14411_device::TIMER_F9,  "300")
	PORT_CONFSETTING(mc14411_device::TIMER_F10, "200")
	PORT_CONFSETTING(mc14411_device::TIMER_F11, "150")
	PORT_CONFSETTING(mc14411_device::TIMER_F12, "134.5")
	PORT_CONFSETTING(mc14411_device::TIMER_F13, "110")
	PORT_CONFSETTING(mc14411_device::TIMER_F14, "75")
	PORT_CONFSETTING(mc14411_device::TIMER_F15, "57600")
	PORT_CONFSETTING(mc14411_device::TIMER_F16, "115200")
INPUT_PORTS_END

void proteus3_state::kbd_put(u8 data)
{
	if (data == 0x08)
		data = 0x0f; // take care of backspace (bios 1 and 2)
	m_pia->portb_w(data);
	m_pia->cb1_w(1);
	m_pia->cb1_w(0);
}

void proteus3_state::write_acia_clocks(int id, int state)
{
	if (id == m_serial->read()) // Configurable serial port
	{
		m_acia2->write_txc(state);
		m_acia2->write_rxc(state);
	}
	if (id == mc14411_device::TIMER_F8) // Fixed bitrate for the cassette interface
	{
		acia1_clock_w(state);
	}
}

/******************************************************************************
 Cassette
******************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER( proteus3_state::kansas_r )
{
	// no tape - set uart to idle
	m_cass_data[1]++;
	if (m_cass_data[1] > 32)
	{
		m_cass_data[1] = 32;
		m_cassinbit = 1;
	}

	/* cassette - turn 1200/2400Hz to a bit */
	u8 cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_cassinbit = (m_cass_data[1] < 12) ? 1 : 0;
		m_cass_data[1] = 0;
	}
}

void proteus3_state::acia1_clock_w(int state)
{
	// Save - 8N2 - /16 - 600baud
	// Load - 8N2 - /1
	u8 twobit = m_cass_data[3] & 15;
	// incoming @9600Hz
	if (state)
	{
		if (twobit == 0)
		{
			m_cassold = m_cassbit;
			// synchronous rx
			m_acia1->write_rxc(0);
			m_acia1->write_rxd(m_cassinbit);
			m_acia1->write_rxc(1);
		}

		if (m_cassold)
			m_cass->output(BIT(m_cass_data[3], 1) ? +1.0 : -1.0); // 2400Hz
		else
			m_cass->output(BIT(m_cass_data[3], 2) ? +1.0 : -1.0); // 1200Hz

		m_cass_data[3]++;
	}

	m_acia1->write_txc(state);
}


/******************************************************************************
 Video
******************************************************************************/
void proteus3_state::video_w(u8 data)
{
	m_video_data = data;
}

void proteus3_state::ca2_w(int state)
{
	if (state)
	{
		switch(m_video_data)
		{
			case 0x0a: // Line Feed
				if (m_curs_pos > 959) // on bottom line?
				{
					memmove(m_vram.get(), m_vram.get()+64, 960); // scroll
					memset(m_vram.get()+960, 0x20, 64); // blank bottom line
				}
				else
					m_curs_pos += 64;
				break;
			case 0x0d: // Carriage Return
				m_curs_pos &= 0x3c0;
				break;
			case 0x0c: // CLS
				m_curs_pos = 0; // home cursor
				memset(m_vram.get(), 0x20, 1024); // clear screen
				break;
			case 0x0f: // Cursor Left
				if (m_curs_pos)
					m_curs_pos--;
				break;
			case 0x7f: // Erase character under cursor
				m_vram[m_curs_pos] = 0x20;
				break;
			default: // If a displayable character, show it
				if ((m_video_data > 0x1f) && (m_video_data < 0x7f))
				{
					m_vram[m_curs_pos] = m_video_data;
					m_curs_pos++;
					if (m_curs_pos > 1023) // have we run off the bottom?
					{
						m_curs_pos -= 64;
						memmove(m_vram.get(), m_vram.get()+64, 960); // scroll
						memset(m_vram.get()+960, 0x20, 64); // blank bottom line
					}
				}
		}
	}
}

u32 proteus3_state::screen_update_proteus3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy=0,ma=0;
	m_flashcnt++;

	for (u8 y = 0; y < 16; y++ )
	{
		for (u8 ra = 0; ra < 12; ra++)
		{
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = ma; x < ma + 64; x++)
			{
				u8 gfx = 0;
				if (ra < 8)
				{
					u8 chr = m_vram[x]; // get char in videoram
					gfx = m_p_chargen[(chr<<3) | ra]; // get dot pattern in chargen
				}
				else if ((ra == 9) && (m_curs_pos == x) && BIT(m_flashcnt, 4))
					gfx = 0xff;

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 0);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 7);
			}
		}
		ma+=64;
	}
	return 0;
}


/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 8,                  /* 8 x 8 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                    /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_proteus3 )
	GFXDECODE_ENTRY( "chargen", 0, charlayout, 0, 1 )
GFXDECODE_END


void proteus3_state::machine_reset()
{
	m_curs_pos = 0;
	m_cass_data[0] = m_cass_data[1] = m_cass_data[2] = m_cass_data[3] = 0;
	m_cassbit = 1;
	m_cassold = 1;
	m_acia1->write_rxd(1);

	// Set up the BRG divider. RSA is a jumper setting and RSB is always set High
	m_brg->rsa_w( CLEAR_LINE );
	m_brg->rsb_w( ASSERT_LINE );

	// Disable all configured timers, only enabling the used ones
	m_brg->timer_disable_all();
	m_brg->timer_enable((mc14411_device::timer_id) m_serial->read(), true); // Serial port
	m_brg->timer_enable( mc14411_device::TIMER_F8, true); // Cassette interface
}

void proteus3_state::machine_start()
{
	m_vram = make_unique_clear<u8[]>(0x0400);
	save_pointer(NAME(m_vram), 0x0400);
	save_item(NAME(m_video_data));
	save_item(NAME(m_flashcnt));
	save_item(NAME(m_curs_pos));
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
	save_item(NAME(m_cassinbit));

	m_flashcnt = 0;
}

/******************************************************************************
 Machine Drivers
******************************************************************************/

void proteus3_state::proteus3(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(3'579'545));  /* Divided by 4 internally */
	m_maincpu->set_addrmap(AS_PROGRAM, &proteus3_state::mem_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(200));
	screen.set_size(64*8, 16*12);
	screen.set_visarea(0, 64*8-1, 0, 16*12-1);
	screen.set_screen_update(FUNC(proteus3_state::screen_update_proteus3));
	screen.set_palette("palette");
	GFXDECODE(config, "gfxdecode", "palette", gfx_proteus3);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* Devices */
	PIA6821(config, m_pia);
	m_pia->writepa_handler().set(FUNC(proteus3_state::video_w));
	m_pia->ca2_handler().set(FUNC(proteus3_state::ca2_w));
	m_pia->irqb_handler().set_inputline("maincpu", M6800_IRQ_LINE);

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(proteus3_state::kbd_put));

	/* cassette */
	ACIA6850(config, m_acia1, 0);
	m_acia1->txd_handler().set([this] (bool state) { m_cassbit = state; });

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_r").configure_periodic(FUNC(proteus3_state::kansas_r), attotime::from_hz(40000));

	// optional tty keyboard
	ACIA6850(config, m_acia2, 0);
	m_acia2->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia2->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "keyboard"));
	rs232.rxd_handler().set(m_acia2, FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set(m_acia2, FUNC(acia6850_device::write_cts));

	/* Bit Rate Generator */
	MC14411(config, m_brg, XTAL(1'843'200)); // crystal needs verification but is the likely one
	m_brg->out_f<1>().set(FUNC(proteus3_state::write_f1_clock));
	m_brg->out_f<2>().set(FUNC(proteus3_state::write_f2_clock));
	m_brg->out_f<3>().set(FUNC(proteus3_state::write_f3_clock));
	m_brg->out_f<4>().set(FUNC(proteus3_state::write_f4_clock));
	m_brg->out_f<5>().set(FUNC(proteus3_state::write_f5_clock));
	m_brg->out_f<6>().set(FUNC(proteus3_state::write_f6_clock));
	m_brg->out_f<7>().set(FUNC(proteus3_state::write_f7_clock));
	m_brg->out_f<8>().set(FUNC(proteus3_state::write_f8_clock));
	m_brg->out_f<9>().set(FUNC(proteus3_state::write_f9_clock));
	m_brg->out_f<10>().set(FUNC(proteus3_state::write_f10_clock));
	m_brg->out_f<11>().set(FUNC(proteus3_state::write_f11_clock));
	m_brg->out_f<12>().set(FUNC(proteus3_state::write_f12_clock));
	m_brg->out_f<13>().set(FUNC(proteus3_state::write_f13_clock));
	m_brg->out_f<14>().set(FUNC(proteus3_state::write_f14_clock));
	m_brg->out_f<15>().set(FUNC(proteus3_state::write_f15_clock));
}



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(proteus3)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASE00)  // if c000 isn't 0 it assumes a rom is there and jumps to it
	ROM_SYSTEM_BIOS( 0, "14k", "14k BASIC")
	ROMX_LOAD( "bas1.bin",     0x0800, 0x0800, CRC(016bf2d6) SHA1(89605dbede3b6fd101ee0548e5c545a0824fcfd3), ROM_BIOS(0) )
	ROMX_LOAD( "bas2.bin",     0x1000, 0x0800, CRC(39d3e543) SHA1(dd0fe220e3c2a48ce84936301311cbe9f1597ca7), ROM_BIOS(0) )
	ROMX_LOAD( "bas3.bin",     0x1800, 0x0800, CRC(3a41617d) SHA1(175406f4732389e226bc50d27ada39e6ea48de34), ROM_BIOS(0) )
	ROMX_LOAD( "bas4.bin",     0x2000, 0x0800, CRC(ee9d77ee) SHA1(f7e60a1ab88a3accc8ffdc545657c071934d09d2), ROM_BIOS(0) )
	ROMX_LOAD( "bas5.bin",     0x2800, 0x0800, CRC(bd81bb34) SHA1(6325735e5750a9536e63b67048f74711fae1fa42), ROM_BIOS(0) )
	ROMX_LOAD( "bas6.bin",     0x3000, 0x0800, CRC(60cd006b) SHA1(28354f78490da1eb5116cbbc43eaca0670f7f398), ROM_BIOS(0) )
	ROMX_LOAD( "bas7.bin",     0x3800, 0x0800, CRC(84c3dc22) SHA1(8fddba61b5f0270ca2daef32ab5edfd60300c776), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "8k", "8k BASIC")
	ROMX_LOAD( "proteus3_basic8k.m0", 0x2000, 0x2000, CRC(7d9111c2) SHA1(3c032c9c7f87d22a1a9819b3b812be84404d2ad2), ROM_BIOS(1) )
	ROM_RELOAD( 0x0000, 0x2000 )

	ROM_SYSTEM_BIOS( 2, "8kms", "8k Micro-Systemes BASIC")
	ROMX_LOAD( "ms1_basic8k.bin", 0x2000, 0x2000, CRC(b5476e28) SHA1(c8c2366d549b2645c740be4ab4237e05c3cab4a9), ROM_BIOS(2) )
	ROM_RELOAD( 0x0000, 0x2000 )

	ROM_REGION(0x0400, "chargen", 0)
	ROM_LOAD( "proteus3_font.m25",   0x0200, 0x0100, CRC(6a3a30a5) SHA1(ab39bf09722928483e497b87ac2dbd870828893b) )
	ROM_CONTINUE( 0x100, 0x100 )
	ROM_CONTINUE( 0x300, 0x100 )
	ROM_CONTINUE( 0x000, 0x100 )

	ROM_REGION(0x0800, "user1", 0) // roms not used yet
	// Proteus III - pbug F800-FFFF, expects RAM at F000-F7FF
	ROM_LOAD( "proteus3_pbug.bin", 0x0000, 0x0800, CRC(1118694d) SHA1(2dfc08d405e8f2936f5b0bd1c4007995151abbba) )
ROM_END

} // Anonymous namespace


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                  FULLNAME       FLAGS
COMP( 1978, proteus3, 0,      0,      proteus3, proteus3, proteus3_state, empty_init, "Proteus International", "Proteus III", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
