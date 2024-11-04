// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*
    Photon IK-3 system

    Driver by Mariusz Wojcieszek

    Russian arcade system based on ZX Spectrum home computer clone "Leningrad-1". Changes are:
    - ROM bank switching added
    - original IO addressing replaced by:
       A7 A0
       0  0  ROM bank switch
       0  1  i8255
       1  0  standard ZX FEh port (beeper, border color)
       1  1  nothing
    - added i8255, uses A5 and A6 to select ports, usage are:
       PortA - joystick
       PortB - joystick (unused)
       PortC - bit 0 - Coin in, 1-3 - Time per Coin switches, 4 - block Coin in (out), 5 - NMI (out)

    Each coin buys you 1-6 minutes of game time.
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class photon2_state : public driver_device
{
public:
	photon2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_ppi(*this, "ppi"),
		m_speaker(*this, "speaker"),
		m_spectrum_video_ram(*this, "spectrum_vram")
	{ }

	void photon2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<speaker_sound_device> m_speaker;

	required_shared_ptr<uint8_t> m_spectrum_video_ram;

	int m_spectrum_frame_number = 0;
	int m_spectrum_flash_invert = 0;
	uint8_t m_spectrum_port_fe = 0;
	uint8_t m_nmi_enable = 0;

	void membank_w(uint8_t data);
	uint8_t fe_r();
	void fe_w(uint8_t data);
	void misc_w(uint8_t data);

	void photon2_palette(palette_device &palette) const;

	uint32_t screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_spectrum(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(spec_interrupt_hack);
	void spectrum_io(address_map &map) ATTR_COLD;
	void spectrum_mem(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Video (copied from spectrum driver apart from support
 *  for changing border color mid-frame)
 *
 *************************************/

/* Spectrum screen size in pixels */
#define SPEC_UNSEEN_LINES  16   /* Non-visible scanlines before first border
                                   line. Some of these may be vertical retrace. */
#define SPEC_TOP_BORDER    48   /* Number of border lines before actual screen */
#define SPEC_DISPLAY_YSIZE 192  /* Vertical screen resolution */
#define SPEC_BOTTOM_BORDER 56   /* Number of border lines at bottom of screen */
#define SPEC_SCREEN_HEIGHT (SPEC_TOP_BORDER + SPEC_DISPLAY_YSIZE + SPEC_BOTTOM_BORDER)

#define SPEC_LEFT_BORDER   48   /* Number of left hand border pixels */
#define SPEC_DISPLAY_XSIZE 256  /* Horizontal screen resolution */
#define SPEC_RIGHT_BORDER  48   /* Number of right hand border pixels */
#define SPEC_SCREEN_WIDTH (SPEC_LEFT_BORDER + SPEC_DISPLAY_XSIZE + SPEC_RIGHT_BORDER)

#define SPEC_LEFT_BORDER_CYCLES   24   /* Cycles to display left hand border */
#define SPEC_DISPLAY_XSIZE_CYCLES 128  /* Horizontal screen resolution */
#define SPEC_RIGHT_BORDER_CYCLES  24   /* Cycles to display right hand border */
#define SPEC_RETRACE_CYCLES       48   /* Cycles taken for horizontal retrace */
#define SPEC_CYCLES_PER_LINE      224  /* Number of cycles to display a single line */

static constexpr rgb_t spectrum_palette[16] = {
	rgb_t(0x00, 0x00, 0x00),
	rgb_t(0x00, 0x00, 0xbf),
	rgb_t(0xbf, 0x00, 0x00),
	rgb_t(0xbf, 0x00, 0xbf),
	rgb_t(0x00, 0xbf, 0x00),
	rgb_t(0x00, 0xbf, 0xbf),
	rgb_t(0xbf, 0xbf, 0x00),
	rgb_t(0xbf, 0xbf, 0xbf),
	rgb_t(0x00, 0x00, 0x00),
	rgb_t(0x00, 0x00, 0xff),
	rgb_t(0xff, 0x00, 0x00),
	rgb_t(0xff, 0x00, 0xff),
	rgb_t(0x00, 0xff, 0x00),
	rgb_t(0x00, 0xff, 0xff),
	rgb_t(0xff, 0xff, 0x00),
	rgb_t(0xff, 0xff, 0xff)
};

// Initialise the palette
void photon2_state::photon2_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, spectrum_palette);
}

void photon2_state::video_start()
{
	m_spectrum_frame_number = 0;
	m_spectrum_flash_invert = 0;

	save_item(NAME(m_spectrum_frame_number));
	save_item(NAME(m_spectrum_flash_invert));
	save_item(NAME(m_spectrum_port_fe));
}

#if 0
/* return the color to be used inverting FLASHing colors if necessary */
static inline unsigned char get_display_color (unsigned char color, int invert)
{
	if (invert && (color & 0x80))
			return (color & 0xc0) + ((color & 0x38) >> 3) + ((color & 0x07) << 3);
	else
			return color;
}
#endif

/* Code to change the FLASH status every 25 frames. Note this must be
   independent of frame skip etc. */
void photon2_state::screen_vblank_spectrum(int state)
{
	// rising edge
	if (state)
	{
		m_spectrum_frame_number++;
		if (m_spectrum_frame_number >= 25)
		{
			m_spectrum_frame_number = 0;
			m_spectrum_flash_invert = !m_spectrum_flash_invert;
		}
	}
}

static inline void spectrum_plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color)
{
	bitmap.pix(y, x) = (uint16_t)color;
}

uint32_t photon2_state::screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* for now do a full-refresh */
//  int full_refresh = 1;

	unsigned char const *scr=m_spectrum_video_ram;

	bitmap.fill(m_spectrum_port_fe & 0x07, cliprect);

	for (int y=0; y<192; y++)
	{
		int scrx=SPEC_LEFT_BORDER;
		int scry=((y&7) * 8) + ((y&0x38)>>3) + (y&0xC0);
		unsigned char const *attr=m_spectrum_video_ram + ((scry>>3)*32) + 0x1800;

		for (int x=0;x<32;x++)
		{
			/* Get ink and paper colour with bright */
			unsigned short ink, pap;
			if (m_spectrum_flash_invert && (*attr & 0x80))
			{
				ink=((*attr)>>3) & 0x0f;
				pap=((*attr) & 0x07) + (((*attr)>>3) & 0x08);
			}
			else
			{
				ink=((*attr) & 0x07) + (((*attr)>>3) & 0x08);
				pap=((*attr)>>3) & 0x0f;
			}

			for (int b=0x80;b!=0;b>>=1)
			{
				if (*scr&b)
					spectrum_plot_pixel(bitmap,scrx++,SPEC_TOP_BORDER+scry,ink);
				else
					spectrum_plot_pixel(bitmap,scrx++,SPEC_TOP_BORDER+scry,pap);
			}
			scr++;
			attr++;
		}
	}

	return 0;
}

/*************************************
 *
 *  I/O - memory banking, sound
 *
 *************************************/

void photon2_state::membank_w(uint8_t data)
{
	int bank = 0;
	if (data == 0)
	{
		bank = 0;
	}
	else if (data == 1)
	{
		bank = 1;
	}
	else if (data == 5)
	{
		bank = 2;
	}
	else
	{
		logerror( "Unknown banking write: %02X\n", data);
	}

	membank("mainbank")->set_entry(bank);
}

uint8_t photon2_state::fe_r()
{
	return 0xff;
}

void photon2_state::fe_w(uint8_t data)
{
	m_spectrum_port_fe = data;
	m_speaker->level_w(BIT(data,4));
}

void photon2_state::misc_w(uint8_t data)
{
	m_nmi_enable = !BIT(data,5);
}

/*************************************
 *
 *  Memory maps
 *
 *************************************/

void photon2_state::spectrum_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankr("mainbank");
	map(0x4000, 0x5aff).ram().share("spectrum_vram");
	map(0x5b00, 0xffff).ram();
}

void photon2_state::spectrum_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0x7e).w(FUNC(photon2_state::membank_w));
	map(0x01, 0x01).mirror(0x1e).lrw8(NAME([this]() { return m_ppi->read(0); }), NAME([this](u8 data) { m_ppi->write(0, data); }));
	map(0x21, 0x21).mirror(0x1e).lrw8(NAME([this]() { return m_ppi->read(1); }), NAME([this](u8 data) { m_ppi->write(1, data); }));
	map(0x41, 0x41).mirror(0x1e).lrw8(NAME([this]() { return m_ppi->read(2); }), NAME([this](u8 data) { m_ppi->write(2, data); }));
	map(0x61, 0x61).mirror(0x1e).lrw8(NAME([this]() { return m_ppi->read(3); }), NAME([this](u8 data) { m_ppi->write(3, data); }));
	map(0x80, 0x80).mirror(0x7e).rw(FUNC(photon2_state::fe_r), FUNC(photon2_state::fe_w));
}

/*************************************
 *
 *  Inputs
 *
 *************************************/

static INPUT_PORTS_START( photon2 )
	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COIN")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_PLAYER(1) PORT_IMPULSE(1)
	PORT_DIPNAME( 0x0e, 0x0e, "Time per Coin" )
	PORT_DIPSETTING(    0x00, "1:00" )
	PORT_DIPSETTING(    0x02, "1:30" )
	PORT_DIPSETTING(    0x04, "2:00" )
	PORT_DIPSETTING(    0x06, "2:30" )
	PORT_DIPSETTING(    0x08, "3:00" )
	PORT_DIPSETTING(    0x0a, "4:00" )
	PORT_DIPSETTING(    0x0c, "5:00" )
	PORT_DIPSETTING(    0x0e, "6:00" )
	// todo: check if these are really unused..
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( black )
	PORT_INCLUDE( photon2 )

	PORT_MODIFY("JOY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
INPUT_PORTS_END

/*************************************
 *
 *  Machine
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(photon2_state::spec_interrupt_hack)
{
	int scanline = param;

	if (scanline == SPEC_SCREEN_HEIGHT/2)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
	else if(scanline == 0)
	{
		if ( m_nmi_enable )
		{
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		}
	}
}

void photon2_state::machine_start()
{
	membank("mainbank")->configure_entries(0, 4, memregion("maincpu")->base(), 0x4000);
	membank("mainbank")->set_entry(0);

	save_item(NAME(m_nmi_enable));
}

void photon2_state::photon2(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3500000);        /* 3.5 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &photon2_state::spectrum_mem);
	m_maincpu->set_addrmap(AS_IO, &photon2_state::spectrum_io);

	TIMER(config, "scantimer").configure_scanline(FUNC(photon2_state::spec_interrupt_hack), "screen", 0, 1);

	I8255(config, m_ppi, 0);
	m_ppi->in_pa_callback().set_ioport("JOY");
	m_ppi->in_pc_callback().set_ioport("COIN");
	m_ppi->out_pc_callback().set(FUNC(photon2_state::misc_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50.08);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(SPEC_SCREEN_WIDTH, SPEC_SCREEN_HEIGHT);
	screen.set_visarea(0, SPEC_SCREEN_WIDTH-1, 0, SPEC_SCREEN_HEIGHT-1);
	screen.set_screen_update(FUNC(photon2_state::screen_update_spectrum));
	screen.screen_vblank().set(FUNC(photon2_state::screen_vblank_spectrum));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(photon2_state::photon2_palette), 16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}

/*************************************
 *
 *  Globals
 *
 *************************************/

ROM_START( kok )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "kok00.bin", 0x0000, 0x2000, CRC(ff790d0b) SHA1(26dce26e43c15fd90d99abf25a86ca55ed13de94) )
	ROM_LOAD( "kok01.bin", 0x2000, 0x2000, CRC(bf81811e) SHA1(26f073e49f126a70008256ea74394fbf11649503) )
	ROM_LOAD( "kok10.bin", 0x4000, 0x2000, CRC(73fc7b92) SHA1(226abcb40aa3b8cfa96bc4ac89ba62b79ee79b2a) )
	ROM_LOAD( "kok11.bin", 0x6000, 0x2000, CRC(7de0f54a) SHA1(3ee73f8e133ff3356e0ee8d1918b99d66f5ba53f) )
ROM_END

ROM_START( black )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "black1.bin", 0x0000, 0x2000, CRC(8b02b314) SHA1(2e2de2b21634538515d4d6ca0930c4e3e5d1e80f) )
	ROM_LOAD( "black2.bin", 0x2000, 0x2000, CRC(ed93469a) SHA1(5e23da3f649f5e20f7c8450bfa5d7d0b190f892c) )
	ROM_LOAD( "black5.bin", 0x4000, 0x2000, CRC(f7c0baf5) SHA1(0d0a6f8b7f9bf65be61c8c78270a8c6e60fa3fe9) )
	ROM_LOAD( "black6.bin", 0x6000, 0x2000, CRC(1f60bc18) SHA1(fd1a902c51e01dfc6fa42dac94d25566ce5bb3d7) )
	ROM_LOAD( "black3.bin", 0x8000, 0x2000, CRC(784ea7f4) SHA1(f3008ad180ad14e0728bf0ba78fe85302ef2ff85) )
	ROM_LOAD( "black4.bin", 0xa000, 0x2000, CRC(20281f74) SHA1(83df590e21a44fa07d4bc76818a8d0d0c4de42b3) )
ROM_END

ROM_START( brod )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "brod00.bin", 0x0000, 0x2000, CRC(cbd6653f) SHA1(18b9134529a1e56c6e90f3bcef5102d5d4b352e3) )
	ROM_FILL( 0x2000, 0x2000, 0x00 )
	ROM_LOAD( "brod10.bin", 0x4000, 0x2000, CRC(9c25d44a) SHA1(f78c7e5b4e6f9fe34f81dc574ca335f70b61e68d) )
	ROM_LOAD( "brod11.bin", 0x6000, 0x2000, CRC(f6505a16) SHA1(3b2ccca78fd83855003cc752766df83b19f89364) )
	ROM_LOAD( "brod12.bin", 0x8000, 0x2000, CRC(94e53d47) SHA1(698415c5e25528e3b1dcab7471cc98c1dc9cb335) )
	ROM_LOAD( "brod13.bin", 0xa000, 0x2000, CRC(1177cd17) SHA1(58c5c09a7b857ce6311339c4d0f4d8c1a7e232a3) )
ROM_END

} // anonymous namespace


GAME( 19??,  kok,   0,      photon2, photon2, photon2_state, empty_init, ROT0, "bootleg", "Povar / Sobrat' Buran / Agroprom (Arcade multi-game bootleg of ZX Spectrum 'Cookie', 'Jetpac' & 'Pssst')", MACHINE_SUPPORTS_SAVE ) // originals (c)1983 ACG / Ultimate
GAME( 19??,  black, 0,      photon2, black,   photon2_state, empty_init, ROT0, "bootleg", "Czernyj Korabl (Arcade bootleg of ZX Spectrum 'Blackbeard')",                                              MACHINE_SUPPORTS_SAVE ) // original (c)1988 Toposoft
GAME( 19??,  brod,  0,      photon2, black,   photon2_state, empty_init, ROT0, "bootleg", "Brodjaga (Arcade bootleg of ZX Spectrum 'Inspector Gadget and the Circus of Fear')",                       MACHINE_IMPERFECT_CONTROLS | MACHINE_SUPPORTS_SAVE ) // original (c)1987 BEAM software
