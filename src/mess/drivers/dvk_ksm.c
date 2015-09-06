// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    KSM (Kontroller Simvolnogo Monitora = Character Display Controller),
    a single-board replacement for standalone 15IE-00-013 terminal (ie15.c)
    in later-model DVK desktops.

    MPI (Q-Bus clone) board, consumes only power from the bus.
    Interfaces with MS7004 (DEC LK201 workalike) keyboard and monochrome CRT.

    Hardware revisions (XXX verify everything):
    - 7.102.076 -- has DIP switches, SRAM at 0x2000, model name "KSM"
    - 7.102.228 -- no DIP switches, SRAM at 0x2100, model name "KSM-01"

    Emulates a VT52 without copier (ESC Z response is ESC / M), with
    Hold Screen mode and Graphics character set (but it is unique and
    mapped to a different range -- 100..137).

    F4 + 0..9 on numeric keypad = setup mode.  0 changes serial port speed,
    1..9 toggle one of mode bits:

    1   XON/XOFF    0: Off  1: On
    2   Character set   0: N0/N1  2: N2
    3   Auto LF     0: Off  1: On
    4   Auto repeat 0: On  1: Off
    5   Auto wraparound 0: On  1: Off
    6   Interpret controls  0: Interpret  1: Display
    7   Parity check    0: Off  1: On
    8   Parity bits 0: None  1: Even
    9   Stop bits

    N0/N1 charset has regular ASCII in C0 page and Cyrillic in C1 page,
    switching between them via SI/SO.   N2 charset has uppercase Cyrillic
    chars in place of lowercase Latin ones.

    F1 toggles Hold Screen mode.
    F9 resets terminal (clears memory).
    F20 toggles on/off-line mode.

    Terminfo description would be something like

ksm|DVK KSM,
    am, bw, dch1=\EP, ich1=\EQ,
    acsc=hRiTjXkClJmFnNqUtEuPv\174wKxW.M\054Q\055S\053\136~_{@}Z0\177,
    use=vt52,

    To do:
    - make Caps Lock work
    - verify if pixel stretching is done by hw
    - verify details of hw revisions
    - baud rate selection (missing feature in bitbanger)

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/ms7004.h"
#include "machine/pic8259.h"

#define SCREEN_PAGE (80*48)

#define KSM_TOTAL_HORZ 1000
#define KSM_DISP_HORZ  800
#define KSM_HORZ_START 200

#define KSM_TOTAL_VERT 28*11
#define KSM_DISP_VERT  25*11
#define KSM_VERT_START 2*11

#define KSM_STATUSLINE_TOTAL 11
#define KSM_STATUSLINE_VRAM 0xF8B0

#define VERBOSE_DBG 0       /* general debug messages */

#define DBG_LOG(N,M,A) \
	do { \
		if(VERBOSE_DBG>=N) \
		{ \
			if( M ) \
				logerror("%11.6f at %s: %-24s",machine().time().as_double(),machine().describe_context(),(char*)M ); \
			logerror A; \
		} \
	} while (0)


class ksm_state : public driver_device
{
public:
	ksm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_p_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_pic8259(*this, "pic8259"),
		m_i8251line(*this, "i8251line"),
		m_rs232(*this, "rs232"),
		m_i8251kbd(*this, "i8251kbd"),
		m_ms7004(*this, "ms7004"),
		m_screen(*this, "screen")
	{ }

	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER( scanline_callback );

	DECLARE_WRITE_LINE_MEMBER(write_keyboard_clock);
	DECLARE_WRITE_LINE_MEMBER(write_line_clock);

	DECLARE_WRITE8_MEMBER(ksm_ppi_porta_w);
	DECLARE_WRITE8_MEMBER(ksm_ppi_portc_w);

private:
	UINT32 draw_scanline(UINT16 *p, UINT16 offset, UINT8 scanline);
	rectangle m_tmpclip;
	bitmap_ind16 m_tmpbmp;

	const UINT8 *m_p_chargen;
	struct {
		UINT8 line;
		UINT16 ptr;
	} m_video;

protected:
	required_shared_ptr<UINT8> m_p_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device>  m_pic8259;
	required_device<i8251_device> m_i8251line;
	required_device<rs232_port_device> m_rs232;
	required_device<i8251_device> m_i8251kbd;
	required_device<ms7004_device> m_ms7004;
	required_device<screen_device> m_screen;
};

static ADDRESS_MAP_START( ksm_mem, AS_PROGRAM, 8, ksm_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x0000, 0x0fff) AM_ROM
	AM_RANGE (0x2000, 0x21ff) AM_RAM
	AM_RANGE (0xc000, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( ksm_io, AS_IO, 8, ksm_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x5e, 0x5f) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
	AM_RANGE (0x6e, 0x6e) AM_DEVREADWRITE("i8251kbd", i8251_device, data_r, data_w)
	AM_RANGE (0x6f, 0x6f) AM_DEVREADWRITE("i8251kbd", i8251_device, status_r, control_w)
	AM_RANGE (0x76, 0x76) AM_DEVREADWRITE("i8251line", i8251_device, data_r, data_w)
	AM_RANGE (0x77, 0x77) AM_DEVREADWRITE("i8251line", i8251_device, status_r, control_w)
	AM_RANGE (0x78, 0x7b) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ksm )
	PORT_START("SA1")
	PORT_DIPNAME(0x01, 0x01, "Stop bits")
	PORT_DIPSETTING(0x00, "2 bits")
	PORT_DIPSETTING(0x01, "1 bit")
	PORT_DIPNAME(0x02, 0x00, "Parity bits")
	PORT_DIPSETTING(0x00, "0 bits")
	PORT_DIPSETTING(0x02, "1 bit")
	PORT_DIPNAME(0x04, 0x00, "Parity check")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(0x04, "On")
	PORT_DIPNAME(0x08, 0x00, "Interpret controls")
	PORT_DIPSETTING(0x00, "Interpret")
	PORT_DIPSETTING(0x08, "Display")
	PORT_DIPNAME(0x10, 0x00, "Auto wraparound")
	PORT_DIPSETTING(0x00, "On")
	PORT_DIPSETTING(0x10, "Off")
	PORT_DIPNAME(0x20, 0x00, "Auto repeat")
	PORT_DIPSETTING(0x00, "On")
	PORT_DIPSETTING(0x20, "Off")
	PORT_DIPNAME(0x40, 0x00, "Auto CR/LF")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(0x40, "On")
	PORT_DIPNAME(0x80, 0x00, "Character set")
	PORT_DIPSETTING(0x00, "KOI-8 N0/N1")
	PORT_DIPSETTING(0x80, "KOI-8 N2")
	PORT_START("SA2")
	PORT_DIPNAME(0x01, 0x00, "XON/XOFF")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(0x01, "On")
	PORT_DIPNAME(0x0E, 0x00, "Baud rate")
	PORT_DIPSETTING(0x00, "9600")
	PORT_DIPSETTING(0x02, "4800")
	PORT_DIPSETTING(0x04, "2400")
	PORT_DIPSETTING(0x06, "1200")
	PORT_DIPSETTING(0x08, "600")
	PORT_DIPSETTING(0x0A, "300")
	PORT_DIPSETTING(0x0C, "150")
	PORT_DIPSETTING(0x0E, "75")
INPUT_PORTS_END

void ksm_state::machine_reset()
{
	memset(&m_video, 0, sizeof(m_video));
}

void ksm_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();

	m_tmpclip = rectangle(0, KSM_DISP_HORZ-1, 0, KSM_DISP_VERT-1);
	m_tmpbmp.allocate(KSM_DISP_HORZ, KSM_DISP_VERT);
}

WRITE8_MEMBER(ksm_state::ksm_ppi_porta_w)
{
	DBG_LOG(1,"PPI port A", ("line %d\n", data));
	m_video.line = data;
}

WRITE8_MEMBER(ksm_state::ksm_ppi_portc_w)
{
	DBG_LOG(1,"PPI port C", ("blink %d speed %d\n", BIT(data, 7), ((data >> 4) & 7) ));
}

WRITE_LINE_MEMBER(ksm_state::write_keyboard_clock)
{
//  KSM never sends data to keyboard
//  m_i8251kbd->write_txc(state);
	m_i8251kbd->write_rxc(state);
}

WRITE_LINE_MEMBER(ksm_state::write_line_clock)
{
	m_i8251line->write_txc(state);
	m_i8251line->write_rxc(state);
}

/*
    Raster size is 28x11 scan lines.
    XXX VBlank is active for 2 topmost on-screen rows and 1 at the bottom.

    Usable raster is 800 x 275 pixels (80 x 25 characters).  24 lines are
    available to the user and 25th (topmost) line is the status line.
    Status line displays current serial port speed and 9 setup bits.

    No character attributes are available, but in 'display controls' mode
    control characters stored in memory are shown as blinking chars.

    Character cell is 10 x 11; character generator provides 7 x 8 of that.
    3 extra horizontal pixels are always XXX blank.  Blinking XXX cursor may be
    displayed on 3 extra scan lines.
*/

UINT32 ksm_state::draw_scanline(UINT16 *p, UINT16 offset, UINT8 scanline)
{
	UINT8 gfx, fg, bg, ra, blink;
	UINT16 x, chr;

	bg = 0; fg = 1; ra = scanline % 8;
	blink = (m_screen->frame_number() % 10) > 4;
	if (scanline > 7) {
		offset -= 0x2000;
	}

	for (x = offset; x < offset + 80; x++)
	{
		chr = m_p_videoram[x] << 3;
		gfx = m_p_chargen[chr | ra];

		if ((scanline > 7 && blink) || ((chr < (0x20<<3)) && !blink))
			gfx = 0;

		*p++ = BIT(gfx, 6) ? fg : bg;
		*p++ = BIT(gfx, 5) ? fg : bg;
		*p++ = BIT(gfx, 4) ? fg : bg;
		*p++ = BIT(gfx, 3) ? fg : bg;
		*p++ = BIT(gfx, 2) ? fg : bg;
		*p++ = BIT(gfx, 1) ? fg : bg;
		*p++ = BIT(gfx, 0) ? fg : bg;
		*p++ = bg;
		*p++ = bg;
		*p++ = bg;
	}
	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(ksm_state::scanline_callback)
{
	UINT16 y = m_screen->vpos();
	UINT16 offset;

	DBG_LOG(2,"scanline_cb",
		("addr %02x frame %d x %.4d y %.3d row %.2d\n",
		m_video.line, (int)m_screen->frame_number(), m_screen->hpos(), y, y%11));

	if (y < KSM_VERT_START) return;
	y -= KSM_VERT_START;
	if (y >= KSM_DISP_VERT) return;

	if (y < KSM_STATUSLINE_TOTAL) {
		offset = KSM_STATUSLINE_VRAM - 0xC000;
	} else {
		offset = 0x2000 + 0x30 + (((m_video.line + y/11 - 1) % 48) << 7);
	}

	draw_scanline(&m_tmpbmp.pix16(y), offset, y%11);
}

UINT32 ksm_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_tmpbmp, 0, 0, KSM_HORZ_START, KSM_VERT_START, cliprect);
	return 0;
}


/* F4 Character Displayer */
static const gfx_layout ksm_charlayout =
{
	7, 8,                   /* 7x8 pixels in 10x11 cell */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( ksm )
	GFXDECODE_ENTRY("chargen", 0x0000, ksm_charlayout, 0, 1)
GFXDECODE_END

static MACHINE_CONFIG_START( ksm, ksm_state )
	MCFG_CPU_ADD("maincpu", I8080, XTAL_15_4MHz/10)
	MCFG_CPU_PROGRAM_MAP(ksm_mem)
	MCFG_CPU_IO_MAP(ksm_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("scantimer", ksm_state, scanline_callback, attotime::from_hz(50*28*11))
	MCFG_TIMER_START_DELAY(attotime::from_hz(XTAL_15_4MHz/KSM_HORZ_START))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(ksm_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL_15_4MHz, KSM_TOTAL_HORZ, KSM_HORZ_START,
		KSM_HORZ_START+KSM_DISP_HORZ, KSM_TOTAL_VERT, KSM_VERT_START,
		KSM_VERT_START+KSM_DISP_VERT);

	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ksm)
	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(ksm_state, ksm_ppi_porta_w))
	MCFG_I8255_IN_PORTB_CB(IOPORT("SA1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("SA2"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(ksm_state, ksm_ppi_portc_w))

	// serial connection to host
	MCFG_DEVICE_ADD( "i8251line", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir3_w))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "null_modem")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("i8251line", i8251_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("i8251line", i8251_device, write_cts))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("i8251line", i8251_device, write_dsr))

	MCFG_DEVICE_ADD("line_clock", CLOCK, 9600*16) // 8251 is set to /16 on the clock input
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(ksm_state, write_line_clock))

	// serial connection to MS7004 keyboard
	MCFG_DEVICE_ADD( "i8251kbd", I8251, 0)
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir1_w))

	MCFG_DEVICE_ADD("ms7004", MS7004, 0)
	MCFG_MS7004_TX_HANDLER(DEVWRITELINE("i8251kbd", i8251_device, write_rxd))

	// baud rate is supposed to be 4800 but keyboard is slightly faster
	MCFG_DEVICE_ADD("keyboard_clock", CLOCK, 4960*16)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(ksm_state, write_keyboard_clock))
MACHINE_CONFIG_END


/*
    Assumes that SRAM is at 0x2000, which is where technical manual puts it.
    Chargen has 1 missing pixel in 'G' character.
*/
ROM_START( dvk_ksm )
	ROM_REGION(0x1000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "ksm_04_rom0_d32.bin", 0x0000, 0x0800, CRC(6ad62715) SHA1(20f8f95119bc7fc6e0f16c67864e339a86edb44d))
	ROM_LOAD( "ksm_05_rom1_d33.bin", 0x0800, 0x0800, CRC(5b29bcd2) SHA1(1f4f82c2f88f1e8615ec02076559dc606497e654))

	ROM_REGION(0x0800, "chargen", ROMREGION_ERASE00)
	ROM_LOAD("ksm_03_cg_d31.bin", 0x0000, 0x0800, CRC(98853aa7) SHA1(09b8e1b5b10a00c0b0ae7e36ad1328113d31230a))
ROM_END

/*
    Assumes that SRAM is at 0x2100, otherwise identical.
    Chargen has no missing pixels in 'G' character.
*/
ROM_START( dvk_ksm01 )
	ROM_REGION(0x1000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "ksm_04_rom0_d32.bin", 0x0000, 0x0800, CRC(5276dc9a) SHA1(dd41dfb4cb3f1cf22d96d95f1ff6a27fe4eb9a38))
	ROM_LOAD( "ksm_05_rom1_d33.bin", 0x0800, 0x0800, CRC(5b29bcd2) SHA1(1f4f82c2f88f1e8615ec02076559dc606497e654))

	ROM_REGION(0x0800, "chargen", ROMREGION_ERASE00)
	ROM_LOAD("ksm_03_cg_d31.bin", 0x0000, 0x0800, CRC(98853aa7) SHA1(09b8e1b5b10a00c0b0ae7e36ad1328113d31230a))
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    INIT                      COMPANY     FULLNAME       FLAGS */
COMP( 1986, dvk_ksm,  0,      0,       ksm,       ksm,     driver_device,     0,     "USSR",     "DVK KSM",     0)
COMP( 198?, dvk_ksm01,dvk_ksm,0,       ksm,       ksm,     driver_device,     0,     "USSR",     "DVK KSM-01",  0)
