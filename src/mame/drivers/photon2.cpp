// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*
    Photon IK2 system

    Driver by Mariusz Wojcieszek

    Russian arcade system based on ZX Spectrum home computer.

    Each coin buys you 1-6 minutes of game time.
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/speaker.h"


class photon2_state : public driver_device
{
public:
	photon2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_speaker(*this, "speaker"),
		m_spectrum_video_ram(*this, "spectrum_vram") { }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;

	required_shared_ptr<UINT8> m_spectrum_video_ram;

	int m_spectrum_frame_number;
	int m_spectrum_flash_invert;
	UINT8 m_spectrum_port_fe;
	UINT8 m_nmi_enable;

	DECLARE_WRITE8_MEMBER(membank_w);
	DECLARE_READ8_MEMBER(fe_r);
	DECLARE_WRITE8_MEMBER(fe_w);
	DECLARE_WRITE8_MEMBER(misc_w);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(photon2);

	UINT32 screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_spectrum(screen_device &screen, bool state);

	TIMER_DEVICE_CALLBACK_MEMBER(spec_interrupt_hack);
};


/*************************************
 *
 *  Video (copied from MESS apart from support
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
#define SPEC_RETRACE_CYCLES       48   /* Cycles taken for horizonal retrace */
#define SPEC_CYCLES_PER_LINE      224  /* Number of cycles to display a single line */

static const rgb_t spectrum_palette[16] = {
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

/* Initialise the palette */
PALETTE_INIT_MEMBER(photon2_state, photon2)
{
	palette.set_pen_colors(0, spectrum_palette, ARRAY_LENGTH(spectrum_palette));
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
void photon2_state::screen_eof_spectrum(screen_device &screen, bool state)
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

static inline void spectrum_plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color)
{
	bitmap.pix16(y, x) = (UINT16)color;
}

UINT32 photon2_state::screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* for now do a full-refresh */
	int x, y, b, scrx, scry;
	unsigned short ink, pap;
	unsigned char *attr, *scr;
//  int full_refresh = 1;

	scr=m_spectrum_video_ram;

	bitmap.fill(m_spectrum_port_fe & 0x07, cliprect);

	for (y=0; y<192; y++)
	{
		scrx=SPEC_LEFT_BORDER;
		scry=((y&7) * 8) + ((y&0x38)>>3) + (y&0xC0);
		attr=m_spectrum_video_ram + ((scry>>3)*32) + 0x1800;

		for (x=0;x<32;x++)
		{
				/* Get ink and paper colour with bright */
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

				for (b=0x80;b!=0;b>>=1)
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

WRITE8_MEMBER(photon2_state::membank_w)
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

READ8_MEMBER(photon2_state::fe_r)
{
	return 0xff;
}

WRITE8_MEMBER(photon2_state::fe_w)
{
	m_spectrum_port_fe = data;
	m_speaker->level_w(BIT(data,4));
}

WRITE8_MEMBER(photon2_state::misc_w)
{
	m_nmi_enable = !BIT(data,5);
}

/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START (spectrum_mem, AS_PROGRAM, 8, photon2_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("mainbank")
	AM_RANGE(0x4000, 0x5aff) AM_RAM AM_SHARE("spectrum_vram")
	AM_RANGE(0x5b00, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START (spectrum_io, AS_IO, 8, photon2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x1f, 0x1f) AM_READ_PORT("JOY")
	AM_RANGE(0x5b, 0x5b) AM_READ_PORT("COIN") AM_WRITE(misc_w)
	AM_RANGE(0x7a, 0x7a) AM_WRITE(membank_w)
	AM_RANGE(0x7b, 0x7b) AM_WRITENOP // unknown write
	AM_RANGE(0x7e, 0x7e) AM_WRITE(membank_w)
	AM_RANGE(0xfe, 0xfe) AM_READWRITE(fe_r, fe_w)
ADDRESS_MAP_END

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
			m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		}
	}
}

void photon2_state::machine_start()
{
	membank("mainbank")->configure_entries(0, 4, memregion("maincpu")->base(), 0x4000);
	membank("mainbank")->set_entry(0);

	save_item(NAME(m_nmi_enable));
}

static MACHINE_CONFIG_START( photon2, photon2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3500000)        /* 3.5 MHz */
	MCFG_CPU_PROGRAM_MAP(spectrum_mem)
	MCFG_CPU_IO_MAP(spectrum_io)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", photon2_state, spec_interrupt_hack, "screen", 0, 1)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50.08)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(SPEC_SCREEN_WIDTH, SPEC_SCREEN_HEIGHT)
	MCFG_SCREEN_VISIBLE_AREA(0, SPEC_SCREEN_WIDTH-1, 0, SPEC_SCREEN_HEIGHT-1)
	MCFG_SCREEN_UPDATE_DRIVER(photon2_state, screen_update_spectrum)
	MCFG_SCREEN_VBLANK_DRIVER(photon2_state, screen_eof_spectrum)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(photon2_state, photon2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_CONFIG_END

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

GAME( 19??,  kok,   0,      photon2, photon2, driver_device, 0, ROT0, "bootleg", "Povar / Sobrat' Buran / Agroprom (Arcade multi-game bootleg of ZX Spectrum 'Cookie', 'Jetpac' & 'Pssst')", MACHINE_SUPPORTS_SAVE ) // originals (c)1983 ACG / Ultimate
GAME( 19??,  black, 0,      photon2, black, driver_device,   0, ROT0, "bootleg", "Czernyj Korabl (Arcade bootleg of ZX Spectrum 'Blackbeard')", MACHINE_SUPPORTS_SAVE ) // original (c)1988 Toposoft
GAME( 19??,  brod,  0,      photon2, black, driver_device,   0, ROT0, "bootleg", "Brodjaga (Arcade bootleg of ZX Spectrum 'Inspector Gadget and the Circus of Fear')", MACHINE_SUPPORTS_SAVE ) // original (c)1987 BEAM software
