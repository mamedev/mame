// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************

    Proteus III computer.

    2015-10-02 Skeleton [Robbbert]

    Chips:
    6800 @ 894kHz
    6850 (TTY interface)
    6850 (Cassette interface)
    6820 (PIA for Keyboard and video
    6844 DMA
    MC14411 baud rate generator
    CRT96364 CRTC @1008kHz

    There's an undumped 74S287 prom (M24) in the video section.
    It converts ascii control codes into the crtc control codes

    Schematic has lots of errors and omissions.

    To Do:
    - Hook up ACIAs
    - Cassette
    - Need software
    - Need missing PROM, so that all the CRTC controls can be emulated
    - Keyboard may have its own CPU etc, but no info available. There's some
      missing keys and buttons, such as HERE-IS key.

******************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/keyboard.h"


class proteus3_state : public driver_device
{
public:
	proteus3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pia(*this, "pia")
	{ }

	DECLARE_WRITE_LINE_MEMBER(ca2_w);
	DECLARE_WRITE8_MEMBER(video_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT32 screen_update_proteus3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	UINT8 m_video_data;
	UINT8 m_flashcnt;
	UINT16 m_curs_pos;
	UINT8 *m_p_chargen;
	UINT8 *m_p_videoram;
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	//required_device<acia6850_device> m_acia;
};




/******************************************************************************
 Address Maps
******************************************************************************/

static ADDRESS_MAP_START(proteus3_mem, AS_PROGRAM, 8, proteus3_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x8004, 0x8007) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0x8008, 0x8008) AM_DEVREADWRITE("acia1", acia6850_device, status_r, control_w)
	AM_RANGE(0x8009, 0x8009) AM_DEVREADWRITE("acia1", acia6850_device, data_r, data_w)
	AM_RANGE(0x8010, 0x8010) AM_DEVREADWRITE("acia2", acia6850_device, status_r, control_w)
	AM_RANGE(0x8011, 0x8011) AM_DEVREADWRITE("acia2", acia6850_device, data_r, data_w)
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START(proteus3)
INPUT_PORTS_END

WRITE8_MEMBER( proteus3_state::kbd_put )
{
	if (data == 0x08)
		data = 0x0f; // take care of backspace
	m_pia->portb_w(data);
	m_pia->cb1_w(1);
	m_pia->cb1_w(0);
}

WRITE8_MEMBER( proteus3_state::video_w )
{
	m_video_data = data;
}

WRITE_LINE_MEMBER( proteus3_state::ca2_w )
{
	if (state)
	{
		switch(m_video_data)
		{
			case 0x0a: // Line Feed
				if (m_curs_pos > 959) // on bottom line?
				{
					memmove(m_p_videoram, m_p_videoram+64, 960); // scroll
					memset(m_p_videoram+960, 0x20, 64); // blank bottom line
				}
				else
					m_curs_pos += 64;
				break;
			case 0x0d: // Carriage Return
				m_curs_pos &= 0x3c0;
				break;
			case 0x0c: // CLS
				m_curs_pos = 0; // home cursor
				memset(m_p_videoram, 0x20, 1024); // clear screen
				break;
			case 0x0f: // Cursor Left
				if (m_curs_pos)
					m_curs_pos--;
				break;
			case 0x7f: // Erase character under cursor
				m_p_videoram[m_curs_pos] = 0x20;
				break;
			default: // If a displayable character, show it
				if ((m_video_data > 0x1f) && (m_video_data < 0x7f))
				{
					m_p_videoram[m_curs_pos] = m_video_data;
					m_curs_pos++;
					if (m_curs_pos > 1023) // have we run off the bottom?
					{
						m_curs_pos -= 64;
						memmove(m_p_videoram, m_p_videoram+64, 960); // scroll
						memset(m_p_videoram+960, 0x20, 64); // blank bottom line
					}
				}
		}
	}
}

UINT32 proteus3_state::screen_update_proteus3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;
	m_flashcnt++;

	for(y = 0; y < 16; y++ )
	{
		for (ra = 0; ra < 12; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x++)
			{
				gfx = 0;
				if (ra < 8)
				{
					chr = m_p_videoram[x]; // get char in videoram
					gfx = m_p_chargen[(chr<<3) | ra]; // get dot pattern in chargen
				}
				else
				if ((ra == 9) && (m_curs_pos == x) && BIT(m_flashcnt, 4))
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

static GFXDECODE_START( proteus3 )
	GFXDECODE_ENTRY( "chargen", 0, charlayout, 0, 1 )
GFXDECODE_END


void proteus3_state::machine_reset()
{
	m_p_chargen = memregion("chargen")->base();
	m_p_videoram = memregion("vram")->base();
	m_curs_pos = 0;
}


/******************************************************************************
 Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( proteus3, proteus3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_3_579545MHz)  /* Divided by 4 internally */
	MCFG_CPU_PROGRAM_MAP(proteus3_mem)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(200))
	MCFG_SCREEN_SIZE(64*8, 16*12)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 16*12-1)
	MCFG_SCREEN_UPDATE_DRIVER(proteus3_state, screen_update_proteus3)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", proteus3)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(proteus3_state, video_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(proteus3_state, ca2_w))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))

	MCFG_DEVICE_ADD ("acia1", ACIA6850, 0)
	MCFG_DEVICE_ADD ("acia2", ACIA6850, 0)
	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(proteus3_state, kbd_put))
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(proteus3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "14k", "14k BASIC")
	ROMX_LOAD( "bas1.bin",     0xc800, 0x0800, CRC(016bf2d6) SHA1(89605dbede3b6fd101ee0548e5c545a0824fcfd3), ROM_BIOS(1) )
	ROMX_LOAD( "bas2.bin",     0xd000, 0x0800, CRC(39d3e543) SHA1(dd0fe220e3c2a48ce84936301311cbe9f1597ca7), ROM_BIOS(1) )
	ROMX_LOAD( "bas3.bin",     0xd800, 0x0800, CRC(3a41617d) SHA1(175406f4732389e226bc50d27ada39e6ea48de34), ROM_BIOS(1) )
	ROMX_LOAD( "bas4.bin",     0xe000, 0x0800, CRC(ee9d77ee) SHA1(f7e60a1ab88a3accc8ffdc545657c071934d09d2), ROM_BIOS(1) )
	ROMX_LOAD( "bas5.bin",     0xe800, 0x0800, CRC(bd81bb34) SHA1(6325735e5750a9536e63b67048f74711fae1fa42), ROM_BIOS(1) )
	ROMX_LOAD( "bas6.bin",     0xf000, 0x0800, CRC(60cd006b) SHA1(28354f78490da1eb5116cbbc43eaca0670f7f398), ROM_BIOS(1) )
	ROMX_LOAD( "bas7.bin",     0xf800, 0x0800, CRC(84c3dc22) SHA1(8fddba61b5f0270ca2daef32ab5edfd60300c776), ROM_BIOS(1) )
	ROM_FILL( 0xc000, 1, 0 )  // if c000 isn't 0 it assumes a rom is there and jumps to it

	ROM_SYSTEM_BIOS( 1, "8k", "8k BASIC")
	ROMX_LOAD( "proteus3_basic8k.m0", 0xe000, 0x2000, CRC(7d9111c2) SHA1(3c032c9c7f87d22a1a9819b3b812be84404d2ad2), ROM_BIOS(2) )
	ROM_RELOAD( 0xc000, 0x2000 )

	ROM_SYSTEM_BIOS( 2, "8kms", "8k Micro-Systemes BASIC")
	ROMX_LOAD( "ms1_basic8k.bin", 0xe000, 0x2000, CRC(b5476e28) SHA1(c8c2366d549b2645c740be4ab4237e05c3cab4a9), ROM_BIOS(3) )
	ROM_RELOAD( 0xc000, 0x2000 )

	ROM_REGION(0x400, "chargen", 0)
	ROM_LOAD( "proteus3_font.m25",   0x0200, 0x0100, CRC(6a3a30a5) SHA1(ab39bf09722928483e497b87ac2dbd870828893b) )
	ROM_CONTINUE( 0x100, 0x100 )
	ROM_CONTINUE( 0x300, 0x100 )
	ROM_CONTINUE( 0x000, 0x100 )

	ROM_REGION(0x400, "vram", ROMREGION_ERASE00)

	ROM_REGION(0x0800, "user1", 0) // roms not used yet
	// Proteus III - pbug F800-FFFF, expects RAM at F000-F7FF
	ROM_LOAD( "proteus3_pbug.bin", 0x0000, 0x0800, CRC(1118694d) SHA1(2dfc08d405e8f2936f5b0bd1c4007995151abbba) )
ROM_END


/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT     CLASS          INIT      COMPANY                     FULLNAME        FLAGS */
COMP( 1978, proteus3,   0,          0,      proteus3,   proteus3, driver_device,   0,     "Proteus International", "Proteus III", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
