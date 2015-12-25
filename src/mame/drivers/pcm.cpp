// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        PC/M by Miodrag Milanovic

        http://www.li-pro.net/pcm.phtml  (in German)

        12/05/2009 Preliminary driver.

        14/02/2011 Added keyboard (from terminal).

        Commands:
        1 select memory bank 1
        2 select memory bank 2
        B
        C start cp/m from the inbuilt CCP
        D Debugger
        Fx Format disk A or B
        G  Jump to address
        I List files on tape
        L filename.typ  Load file from tape
        R read from disk
        S filename aaaa / bbbb save a file to tape
        V verify
        W write to disk
        X
        Z set tape baud (1200, 2400, 3600 (default), 4800)
        filename   start running this .COM file

        Therefore if you enter random input, it will lock up while trying to
        load up a file of that name. Filenames on disk and tape are of the
        standard 8.3 format. You must specify an extension.

        Here is an example of starting the debugger, executing a command in
        it, then exiting back to the monitor.

        D
        U
        E

        In practice, the I and R commands produce an error, while all disk
        commands are directed to tape. The F command lists the files on a
        tape.

        ToDo:
        - Add bankswitching
        - Add NMI generator
        - Find out if there really is any floppy-disk feature - the schematic
          has no mention of it.
        - Add the 6 LEDs.

****************************************************************************/

#include "emu.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "imagedev/cassette.h"
#include "sound/speaker.h"
#include "sound/wave.h"
#include "machine/k7659kb.h"


class pcm_state : public driver_device
{
public:
	pcm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_pio_s(*this, "z80pio_s"),
	m_pio_u(*this, "z80pio_u"),
	m_sio(*this, "z80sio"),
	m_ctc_s(*this, "z80ctc_s"),
	m_ctc_u(*this, "z80ctc_u"),
	m_speaker(*this, "speaker"),
	m_cass(*this, "cassette"),
	m_p_videoram(*this, "videoram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio_s;
	required_device<z80pio_device> m_pio_u;
	required_device<z80sio0_device> m_sio;
	required_device<z80ctc_device> m_ctc_s;
	required_device<z80ctc_device> m_ctc_u;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cass;
	DECLARE_READ8_MEMBER( pcm_85_r );
	DECLARE_WRITE_LINE_MEMBER( pcm_82_w );
	DECLARE_WRITE8_MEMBER( pcm_85_w );
	UINT8 *m_p_chargen;
	bool m_cone;
	required_shared_ptr<UINT8> m_p_videoram;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	UINT8 m_85;
};


WRITE_LINE_MEMBER( pcm_state::pcm_82_w )
{
	if (state)
	{
		m_cone ^= 1;
		m_speaker->level_w(m_cone);
	}
}


/* PIO connections as far as i could decipher

PortA is input and connects to the keyboard
PortB is mostly output and connects to a series of LEDs,
      but also carries the cassette control & data lines.

A0-A6 ascii codes from the keyboard
A7 strobe, high while a key is pressed
B0 power indicator LED
B1 Run/Stop LED
B2 Sound on/off LED
B3 n/c
B4 High=Save, Low=Load LED
B5 Motor On LED
B6 Save data
B7 Load data
There is also a HALT LED, connected directly to the processor.
*/


READ8_MEMBER( pcm_state::pcm_85_r )
{
	UINT8 data = m_85 & 0x7f;

	if ((m_cass)->input() > 0.03)
		data |= 0x80;

	return data;
}

WRITE8_MEMBER( pcm_state::pcm_85_w )
{
	if (BIT(data, 5))
		m_cass->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
	else
		m_cass->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);

	m_cass->output( BIT(data, 6) ? -1.0 : +1.0);
	m_85 = data;
}



static ADDRESS_MAP_START(pcm_mem, AS_PROGRAM, 8, pcm_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x1fff ) AM_ROM  // ROM
	AM_RANGE( 0x2000, 0xf7ff ) AM_RAM  // RAM
	AM_RANGE( 0xf800, 0xffff ) AM_RAM AM_SHARE("videoram") // Video RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(pcm_io, AS_IO, 8, pcm_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("z80ctc_s", z80ctc_device, read, write) // system CTC
	AM_RANGE(0x84, 0x87) AM_DEVREADWRITE("z80pio_s", z80pio_device, read, write) // system PIO
	AM_RANGE(0x88, 0x8B) AM_DEVREADWRITE("z80sio", z80sio0_device, cd_ba_r, cd_ba_w) // SIO
	AM_RANGE(0x8C, 0x8F) AM_DEVREADWRITE("z80ctc_u", z80ctc_device, read, write) // user CTC
	AM_RANGE(0x90, 0x93) AM_DEVREADWRITE("z80pio_u", z80pio_device, read, write) // user PIO
	//AM_RANGE(0x94, 0x97) // bank select
	//AM_RANGE(0x98, 0x9B) // NMI generator
	//AM_RANGE(0x9C, 0x9F) // io ports available to the user
	// disk controller?
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pcm )
INPUT_PORTS_END

void pcm_state::machine_reset()
{
}

void pcm_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

UINT32 pcm_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0x400,x;

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x++)
			{
				chr = m_p_videoram[x];

				gfx = m_p_chargen[(chr<<3) | ra];

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
		ma+=64;
	}
	return 0;
}


static const z80_daisy_config pcm_daisy_chain[] =
{
	{ "z80ctc_s" },     /* System ctc */
	{ "z80pio_s" },     /* System pio */
	{ "z80sio" },       /* sio */
	{ "z80pio_u" },     /* User pio */
	{ "z80ctc_u" },     /* User ctc */
	{ nullptr }
};


/* F4 Character Displayer */
static const gfx_layout pcm_charlayout =
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

static GFXDECODE_START( pcm )
	GFXDECODE_ENTRY( "chargen", 0x0000, pcm_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( pcm, pcm_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_10MHz /4)
	MCFG_CPU_PROGRAM_MAP(pcm_mem)
	MCFG_CPU_IO_MAP(pcm_io)
	MCFG_CPU_CONFIG(pcm_daisy_chain)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(pcm_state, screen_update)
	MCFG_SCREEN_SIZE(64*8, 16*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 16*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pcm)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* Sound */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* Devices */
	MCFG_K7659_KEYBOARD_ADD()
	MCFG_CASSETTE_ADD("cassette")

	MCFG_DEVICE_ADD("z80pio_u", Z80PIO, XTAL_10MHz/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("z80pio_s", Z80PIO, XTAL_10MHz/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(DEVREAD8(K7659_KEYBOARD_TAG, k7659_keyboard_device, read))
	MCFG_Z80PIO_IN_PB_CB(READ8(pcm_state, pcm_85_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(pcm_state, pcm_85_w))

	MCFG_Z80SIO0_ADD("z80sio", 4800, 0, 0, 0, 0) // clocks come from the system ctc

	MCFG_DEVICE_ADD("z80ctc_u", Z80CTC, XTAL_10MHz /4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("z80ctc_s", Z80CTC, XTAL_10MHz /4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	// ZC0 : SIO channel A clock
	// ZC1 : SIO channel B clock
	MCFG_Z80CTC_ZC2_CB(WRITELINE(pcm_state, pcm_82_w))  // speaker
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pcm )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v202", "Version 2.02" )
	ROMX_LOAD( "bios_v202.d14", 0x0000, 0x0800, CRC(27c24892) SHA1(a97bf9ef075de91330dc0c7cfd3bb6c7a88bb585), ROM_BIOS(1))
	ROMX_LOAD( "bios_v202.d15", 0x0800, 0x0800, CRC(e9cedc70) SHA1(913c526283d9289d0cb2157985bb48193df7aa16), ROM_BIOS(1))
	ROMX_LOAD( "bios_v202.d16", 0x1000, 0x0800, CRC(abe12001) SHA1(d8f0db6b141736d7715d588384fa49ab386bcc55), ROM_BIOS(1))
	ROMX_LOAD( "bios_v202.d17", 0x1800, 0x0800, CRC(2d48d1cc) SHA1(36a825140124dbe10d267fdf28b3eacec6f6d556), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v210", "Version 2.10" )
	ROMX_LOAD( "bios_v210.d14", 0x0000, 0x0800, CRC(45923112) SHA1(dde922533ebd0f6ac06d25b9786830ee3c7178b9), ROM_BIOS(2))
	ROMX_LOAD( "bios_v210.d15", 0x0800, 0x0800, CRC(e9cedc70) SHA1(913c526283d9289d0cb2157985bb48193df7aa16), ROM_BIOS(2))
	ROMX_LOAD( "bios_v210.d16", 0x1000, 0x0800, CRC(ee9ed77b) SHA1(12ea18e3e280f2a0657ff11c7bcdd658d325235c), ROM_BIOS(2))
	ROMX_LOAD( "bios_v210.d17", 0x1800, 0x0800, CRC(2d48d1cc) SHA1(36a825140124dbe10d267fdf28b3eacec6f6d556), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "v330", "Version 3.30" )
	ROMX_LOAD( "bios_v330.d14", 0x0000, 0x0800, CRC(9bbfee10) SHA1(895002f2f4c711278f1e2d0e2a987e2d31472e4f), ROM_BIOS(3))
	ROMX_LOAD( "bios_v330.d15", 0x0800, 0x0800, CRC(4f8d5b40) SHA1(440b0be4cf45a5d450f9eb7684ceb809450585dc), ROM_BIOS(3))
	ROMX_LOAD( "bios_v330.d16", 0x1000, 0x0800, CRC(93fd0d91) SHA1(c8f1bbb63eca3c93560622581ecbb588716aeb91), ROM_BIOS(3))
	ROMX_LOAD( "bios_v330.d17", 0x1800, 0x0800, CRC(d8c7ce33) SHA1(9030d9a73ef1c12a31ac2cb9a593fb2a5097f24d), ROM_BIOS(3))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD( "charrom.d113", 0x0000, 0x0800, CRC(5684b3c3) SHA1(418054aa70a0fd120611e32059eb2051d3b82b5a))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY        FULLNAME       FLAGS */
COMP( 1988, pcm,    0,      0,       pcm,       pcm, driver_device,      0,  "Mugler/Mathes",  "PC/M", 0)
