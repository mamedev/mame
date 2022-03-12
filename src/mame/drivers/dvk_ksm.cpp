// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    KSM (Kontroller Simvolnogo Monitora = Character Display Controller),
    a single-board replacement for standalone 15IE-00-013 terminal (ie15.c
    driver in MAME) in later-model DVK desktops.

    MPI (Q-Bus clone) board, consumes only power from the bus.
    Interfaces with MS7004 (DEC LK201 workalike) keyboard and monochrome CRT.

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

    ESC toggles Cyrillic/Latin mode (depends in the host's terminal driver)
    F1 toggles Hold Screen mode (also depends in the host's terminal driver)
    F9 resets terminal (clears memory).
    F20 toggles on/off-line mode.

    Terminfo description:

ksm|DVK KSM,
    am, bw, dch1=\EP, ich1=\EQ,
    acsc=hRiTjXkClJmFnNqUtEuPv\174wKxW.M\054Q\055S\053\136~_{@}Z0\177,
    use=vt52,

    To do:
    - verify if pixel stretching is done by hw
    - verify details of hw revisions.  known ones:
      - decimal 7.102.076 -- has DIP switches, model name "KSM".
      - decimal 7.102.228 -- no DIP switches, model name "KSM-01" -- no dump.

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/ms7004.h"
#include "machine/pic8259.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"


static constexpr int SCREEN_PAGE = 80 * 48;

static constexpr int KSM_TOTAL_HORZ = 1000;
static constexpr int KSM_DISP_HORZ = 800;
static constexpr int KSM_HORZ_START = 200;

static constexpr int KSM_TOTAL_VERT = (28 * 11);
static constexpr int KSM_DISP_VERT = (25 * 11);
static constexpr int KSM_VERT_START = (2 * 11);

static constexpr int KSM_STATUSLINE_TOTAL = 11;
static constexpr int KSM_STATUSLINE_VRAM = 0xF8B0;


//#define LOG_GENERAL (1U <<  0) //defined in logmacro.h already
#define LOG_KEYBOARD  (1U <<  1)
#define LOG_DEBUG     (1U <<  2)

//#define VERBOSE (LOG_DEBUG)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGKBD(...) LOGMASKED(LOG_KEYBOARD, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


class ksm_state : public driver_device
{
public:
	ksm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_pic8259(*this, "pic8259")
		, m_i8251line(*this, "i8251line")
		, m_rs232(*this, "rs232")
		, m_i8251kbd(*this, "i8251kbd")
		, m_ms7004(*this, "ms7004")
		, m_screen(*this, "screen")
		, m_p_chargen(*this, "chargen")
	{ }

	void ksm(machine_config &config);

private:
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);

	virtual void machine_reset() override;
	virtual void machine_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	DECLARE_WRITE_LINE_MEMBER(write_keyboard_clock);

	DECLARE_WRITE_LINE_MEMBER(write_brga);
	DECLARE_WRITE_LINE_MEMBER(write_brgb);
	DECLARE_WRITE_LINE_MEMBER(write_brgc);

	void ksm_ppi_porta_w(uint8_t data);
	void ksm_ppi_portc_w(uint8_t data);

	void ksm_io(address_map &map);
	void ksm_mem(address_map &map);

	uint32_t draw_scanline(uint16_t *p, uint16_t offset, uint8_t scanline);
	rectangle m_tmpclip;
	bitmap_ind16 m_tmpbmp;

	struct
	{
		uint8_t line;
		uint16_t ptr;
	} m_video;

	enum
	{
		TIMER_ID_BRG = 0
	};

	bool brg_state;
	int brga, brgb, brgc;
	emu_timer *m_brg = nullptr;

	void update_brg(bool a, bool b, int c);

	required_shared_ptr<uint8_t> m_p_videoram;
	required_device<i8080_cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic8259;
	required_device<i8251_device> m_i8251line;
	required_device<rs232_port_device> m_rs232;
	required_device<i8251_device> m_i8251kbd;
	required_device<ms7004_device> m_ms7004;
	required_device<screen_device> m_screen;
	required_region_ptr<u8> m_p_chargen;
};

void ksm_state::ksm_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
	map(0x2000, 0x20ff).ram().mirror(0x0700);
	map(0xc000, 0xffff).ram().share("videoram");
}

void ksm_state::ksm_io(address_map &map)
{
	map.unmap_value_high();
	map(0x5e, 0x5f).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x6e, 0x6f).rw(m_i8251kbd, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x76, 0x77).rw(m_i8251line, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x78, 0x7b).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

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

void ksm_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_ID_BRG)
	{
		brg_state = !brg_state;
		m_i8251line->write_txc(brg_state);
		m_i8251line->write_rxc(brg_state);
	}
}

void ksm_state::machine_reset()
{
	memset(&m_video, 0, sizeof(m_video));
	brga = 0;
	brgb = 0;
	brgc = 0;
	brg_state = 0;
}

void ksm_state::machine_start()
{
	m_tmpclip = rectangle(0, KSM_DISP_HORZ - 1, 0, KSM_DISP_VERT - 1);
	m_tmpbmp.allocate(KSM_DISP_HORZ, KSM_DISP_VERT);

	m_brg = timer_alloc(TIMER_ID_BRG);
}

void ksm_state::ksm_ppi_porta_w(uint8_t data)
{
	LOG("PPI port A line %d\n", data);
	m_video.line = data;
}

void ksm_state::ksm_ppi_portc_w(uint8_t data)
{
	brgc = (data >> 5) & 3;

	LOG("PPI port C raw %02x blink %d speed %d\n", data, BIT(data, 7), brgc);

	update_brg(brga, brgb, brgc);
}

WRITE_LINE_MEMBER(ksm_state::write_keyboard_clock)
{
	m_i8251kbd->write_txc(state);
	m_i8251kbd->write_rxc(state);
}

WRITE_LINE_MEMBER(ksm_state::write_brga)
{
	brga = state;
	update_brg(brga, brgb, brgc);
}

WRITE_LINE_MEMBER(ksm_state::write_brgb)
{
	brgb = state;
	update_brg(brga, brgb, brgc);
}

void ksm_state::update_brg(bool a, bool b, int c)
{
	LOGDBG("brg %d %d %d\n", a, b, c);

	if (a && b) return;

	switch ((a << 3) + (b << 2) + c)
	{
	case 0xa:
		m_brg->adjust(attotime::from_hz(9600*16*2), 0, attotime::from_hz(9600*16*2));
		break;

	case 0x8:
		m_brg->adjust(attotime::from_hz(4800*16*2), 0, attotime::from_hz(4800*16*2));
		break;

	case 0x4:
		m_brg->adjust(attotime::from_hz(2400*16*2), 0, attotime::from_hz(2400*16*2));
		break;

	case 0x5:
		m_brg->adjust(attotime::from_hz(1200*16*2), 0, attotime::from_hz(1200*16*2));
		break;

	case 0x6:
		m_brg->adjust(attotime::from_hz(600*16*2), 0, attotime::from_hz(600*16*2));
		break;

	case 0x7:
		m_brg->adjust(attotime::from_hz(300*16*2), 0, attotime::from_hz(300*16*2));
		break;
	}
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

uint32_t ksm_state::draw_scanline(uint16_t *p, uint16_t offset, uint8_t scanline)
{
	uint8_t gfx, fg, bg, ra, blink;
	uint16_t x, chr;

	bg = 0;
	fg = 1;
	ra = scanline % 8;
	blink = (m_screen->frame_number() % 10) > 4;
	if (scanline > 7)
	{
		offset -= 0x2000;
	}

	for (x = offset; x < offset + 80; x++)
	{
		chr = m_p_videoram[x] << 3;
		gfx = m_p_chargen[chr | ra];

		if ((scanline > 7 && blink) || ((chr < (0x20 << 3)) && !blink)) gfx = 0;

		for (int i = 6; i >= 0; i--)
		{
			*p++ = BIT(gfx, i) ? fg : bg;
		}
		*p++ = bg;
		*p++ = bg;
		*p++ = bg;
	}
	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(ksm_state::scanline_callback)
{
	uint16_t y = m_screen->vpos();

	LOGDBG("scanline_cb addr %02x frame %d x %.4d y %.3d row %.2d\n",
		m_video.line, (int)m_screen->frame_number(), m_screen->hpos(), y, y%11);

	if (y < KSM_VERT_START) return;
	y -= KSM_VERT_START;
	if (y >= KSM_DISP_VERT) return;

	uint16_t offset;
	if (y < KSM_STATUSLINE_TOTAL)
	{
		offset = KSM_STATUSLINE_VRAM - 0xC000;
	}
	else
	{
		offset = 0x2000 + 0x30 + (((m_video.line + y / 11 - 1) % 48) << 7);
	}

	draw_scanline(&m_tmpbmp.pix(y), offset, y % 11);
}

uint32_t ksm_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

static GFXDECODE_START( gfx_ksm )
	GFXDECODE_ENTRY("chargen", 0x0000, ksm_charlayout, 0, 1)
GFXDECODE_END

void ksm_state::ksm(machine_config &config)
{
	I8080(config, m_maincpu, XTAL(15'400'000) / 10);
	m_maincpu->set_addrmap(AS_PROGRAM, &ksm_state::ksm_mem);
	m_maincpu->set_addrmap(AS_IO, &ksm_state::ksm_io);
	m_maincpu->in_inta_func().set("pic8259", FUNC(pic8259_device::acknowledge));

	TIMER(config, "scantimer").configure_scanline(FUNC(ksm_state::scanline_callback), "screen", 0, 1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());
	m_screen->set_screen_update(FUNC(ksm_state::screen_update));
	m_screen->set_raw(XTAL(15'400'000), KSM_TOTAL_HORZ, KSM_HORZ_START,
		KSM_HORZ_START+KSM_DISP_HORZ, KSM_TOTAL_VERT, KSM_VERT_START,
		KSM_VERT_START+KSM_DISP_VERT);
	m_screen->set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_ksm);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	PIC8259(config, m_pic8259, 0);
	m_pic8259->out_int_callback().set_inputline(m_maincpu, 0);

	// D30
	i8255_device &ppi(I8255(config, "ppi8255"));
	ppi.out_pa_callback().set(FUNC(ksm_state::ksm_ppi_porta_w));
	ppi.in_pb_callback().set_ioport("SA1");
	ppi.in_pc_callback().set_ioport("SA2");
	ppi.out_pc_callback().set(FUNC(ksm_state::ksm_ppi_portc_w));

	// D42 - serial connection to host
	I8251(config, m_i8251line, 0);
	m_i8251line->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_i8251line->rxrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir3_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, "null_modem");
	m_rs232->rxd_handler().set(m_i8251line, FUNC(i8251_device::write_rxd));
	m_rs232->cts_handler().set(m_i8251line, FUNC(i8251_device::write_cts));
	m_rs232->dsr_handler().set(m_i8251line, FUNC(i8251_device::write_dsr));

	// D41 - serial connection to MS7004 keyboard
	I8251(config, m_i8251kbd, 0);
	m_i8251kbd->rxrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir1_w));
	m_i8251kbd->rts_handler().set(FUNC(ksm_state::write_brga));
	m_i8251kbd->dtr_handler().set(FUNC(ksm_state::write_brgb));

	MS7004(config, m_ms7004, 0);
	m_ms7004->tx_handler().set(m_i8251kbd, FUNC(i8251_device::write_rxd));

	// baud rate is supposed to be 4800 but keyboard is slightly faster
	clock_device &keyboard_clock(CLOCK(config, "keyboard_clock", 4960 * 16));
	keyboard_clock.signal_handler().set(FUNC(ksm_state::write_keyboard_clock));
}

ROM_START( dvk_ksm )
	ROM_REGION(0x1000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "ksm_04_rom0_d32.bin", 0x0000, 0x0800, CRC(6ad62715) SHA1(20f8f95119bc7fc6e0f16c67864e339a86edb44d))
	ROM_LOAD( "ksm_05_rom1_d33.bin", 0x0800, 0x0800, CRC(5b29bcd2) SHA1(1f4f82c2f88f1e8615ec02076559dc606497e654))

	ROM_REGION(0x0800, "chargen", ROMREGION_ERASE00)
	ROM_LOAD("ksm_03_cg_d31.bin", 0x0000, 0x0800, CRC(6a8477e2) SHA1(c7871a96f135db05c3c8d718fbdf1728e22e72b7))
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME   FLAGS */
COMP( 1986, dvk_ksm, 0,      0,      ksm,     ksm,   ksm_state, empty_init, "USSR",  "DVK KSM", 0 )
