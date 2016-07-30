// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    SM 7238 (aka T3300) color/mono text terminal, compatible with DEC VT 240.
    Graphics option adds Tek 401x and DEC ReGIS support on a 512x250 bitmap.

    Technical manual and schematics: http://doc.pdp-11.org.ru/Terminals/CM7238/

    To do:
    - handle more text_control_w bits
    - more character attributes incl. double width and height, color
    - 80/132 columns switching on the fly, reverse video
    - smooth scroll
    - graphics option
    - run vblank from timer output?
    - document hardware and ROM variants, verify if pixel stretching is done

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/km035.h"
#include "machine/nvram.h"

#define KSM_COLUMNS 80  // or 132

#define KSM_TOTAL_HORZ KSM_COLUMNS*10
#define KSM_DISP_HORZ  KSM_COLUMNS*8
#define KSM_HORZ_START KSM_COLUMNS

#define KSM_TOTAL_VERT 260
#define KSM_DISP_VERT  250
#define KSM_VERT_START 5

#define VERBOSE_DBG 1       /* general debug messages */

#define DBG_LOG(N,M,A) \
	do { \
		if(VERBOSE_DBG>=N) \
		{ \
			if( M ) \
				logerror("%11.6f at %s: %-24s",machine().time().as_double(),machine().describe_context(),(char*)M ); \
			logerror A; \
		} \
	} while (0)


class sm7238_state : public driver_device
{
public:
	sm7238_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_p_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram"),
		m_pic8259(*this, "pic8259"),
		m_i8251line(*this, "i8251line"),
		m_rs232(*this, "rs232"),
		m_i8251kbd(*this, "i8251kbd"),
		m_keyboard(*this, "keyboard"),
		m_i8251prn(*this, "i8251prn"),
		m_printer(*this, "prtr"),
		m_t_hblank(*this, "t_hblank"),
		m_t_vblank(*this, "t_vblank"),
		m_t_color(*this, "t_color"),
		m_t_iface(*this, "t_iface"),
		m_screen(*this, "screen")
	{ }

	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	TIMER_DEVICE_CALLBACK_MEMBER( scanline_callback );
	DECLARE_PALETTE_INIT(sm7238);

	DECLARE_WRITE_LINE_MEMBER(write_keyboard_clock);
	DECLARE_WRITE_LINE_MEMBER(write_printer_clock);

	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_WRITE8_MEMBER(text_control_w);

private:
	UINT32 draw_scanline(UINT16 *p, UINT16 offset, UINT8 scanline);
	rectangle m_tmpclip;
	bitmap_ind16 m_tmpbmp;

	void text_memory_clear();
	void recompute_parameters();

	const UINT8 *m_p_chargen;
	struct {
		UINT8 control;
		UINT8 stride;
		UINT16 ptr;
	} m_video;

protected:
	required_shared_ptr<UINT8> m_p_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_device<pic8259_device> m_pic8259;
	required_device<i8251_device> m_i8251line;
	required_device<rs232_port_device> m_rs232;
	required_device<i8251_device> m_i8251kbd;
	required_device<km035_device> m_keyboard;
	required_device<i8251_device> m_i8251prn;
	required_device<rs232_port_device> m_printer;
	required_device<pit8253_device> m_t_hblank;
	required_device<pit8253_device> m_t_vblank;
	required_device<pit8253_device> m_t_color;
	required_device<pit8253_device> m_t_iface;
	required_device<screen_device> m_screen;
};

static ADDRESS_MAP_START( sm7238_mem, AS_PROGRAM, 8, sm7238_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x0000, 0x9fff) AM_ROM
	AM_RANGE (0xa000, 0xa7ff) AM_RAM
	AM_RANGE (0xb000, 0xb3ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE (0xb800, 0xb800) AM_WRITE(text_control_w)
	AM_RANGE (0xbc00, 0xbc00) AM_WRITE(control_w)
	AM_RANGE (0xc000, 0xcfff) AM_RAM // chargen
	AM_RANGE (0xe000, 0xffff) AM_RAM AM_SHARE("videoram") // e000 -- chars, f000 -- attrs
ADDRESS_MAP_END

static ADDRESS_MAP_START( sm7238_io, AS_IO, 8, sm7238_state )
	ADDRESS_MAP_UNMAP_HIGH
//  AM_RANGE (0x40, 0x4f) AM_RAM // LUT
	AM_RANGE (0xa0, 0xa0) AM_DEVREADWRITE("i8251line", i8251_device, data_r, data_w)
	AM_RANGE (0xa1, 0xa1) AM_DEVREADWRITE("i8251line", i8251_device, status_r, control_w)
	AM_RANGE (0xa4, 0xa4) AM_DEVREADWRITE("i8251kbd", i8251_device, data_r, data_w)
	AM_RANGE (0xa5, 0xa5) AM_DEVREADWRITE("i8251kbd", i8251_device, status_r, control_w)
	AM_RANGE (0xa8, 0xab) AM_DEVREADWRITE("t_color", pit8253_device, read, write)
	AM_RANGE (0xac, 0xad) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
	AM_RANGE (0xb0, 0xb3) AM_DEVREADWRITE("t_hblank", pit8253_device, read, write)
	AM_RANGE (0xb4, 0xb7) AM_DEVREADWRITE("t_vblank", pit8253_device, read, write)
	AM_RANGE (0xb8, 0xb8) AM_DEVREADWRITE("i8251prn", i8251_device, data_r, data_w)
	AM_RANGE (0xb9, 0xb9) AM_DEVREADWRITE("i8251prn", i8251_device, status_r, control_w)
	AM_RANGE (0xbc, 0xbf) AM_DEVREADWRITE("t_iface", pit8253_device, read, write)
ADDRESS_MAP_END

void sm7238_state::machine_reset()
{
	memset(&m_video, 0, sizeof(m_video));
}

void sm7238_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();

	m_tmpclip = rectangle(0, KSM_DISP_HORZ-1, 0, KSM_DISP_VERT-1);
	m_tmpbmp.allocate(KSM_DISP_HORZ, KSM_DISP_VERT);
}

WRITE8_MEMBER(sm7238_state::control_w)
{
	DBG_LOG(1,"Control Write", ("%02xh: lut %d nvram %d c2 %d iack %d\n",
		data, BIT(data, 0), BIT(data, 2), BIT(data, 3), BIT(data, 5)));
}

WRITE8_MEMBER(sm7238_state::text_control_w)
{
	if (data ^ m_video.control)
	DBG_LOG(1,"Text Control Write", ("%02xh: 80/132 %d dma %d clr %d dlt %d inv %d ?? %d\n",
		data, BIT(data, 0), BIT(data, 1), BIT(data, 2), BIT(data, 3), BIT(data, 4), BIT(data, 5)));

	if (!BIT(data, 2) && !BIT(data, 3))
		text_memory_clear();

	if (BIT((data ^ m_video.control), 0)) {
		m_video.stride = BIT(data, 0) ? 80 : 132;
//      recompute_parameters();
	}

	m_video.control = data;
}

WRITE_LINE_MEMBER(sm7238_state::write_keyboard_clock)
{
	m_i8251kbd->write_txc(state);
	m_i8251kbd->write_rxc(state);
}

WRITE_LINE_MEMBER(sm7238_state::write_printer_clock)
{
	m_i8251prn->write_txc(state);
	m_i8251prn->write_rxc(state);
}

UINT32 sm7238_state::draw_scanline(UINT16 *p, UINT16 offset, UINT8 scanline)
{
	UINT8 attr, fg, bg, ra, gfx;
	UINT16 x, chr;
	int dw;

	ra = scanline % 10;

	for (x = offset; x < offset + m_video.stride; x++)
	{
		chr = m_p_videoram[x] << 4;
		attr = m_p_videoram[x + 0x1000];
		gfx = m_p_chargen[chr | ra] ^ 255;

		bg = 0; fg = 1;

		/* Process attributes */
		if ((BIT(attr, 1)) && (ra == 9))
		{
			gfx = 0xff; // underline
		}
		// 2 = blink
		if (BIT(attr, 3))
		{
			gfx ^= 0xff; // reverse video
		}
		if (BIT(attr, 4))
			fg = 2; // highlight
		else
			fg = 1;

		dw = 0; // BIT(attr, 4);
		*p++ = BIT(gfx, 7) ? fg : bg;
		if (dw)
		*p++ = BIT(gfx, 7) ? fg : bg;
		*p++ = BIT(gfx, 6) ? fg : bg;
		if (dw)
		*p++ = BIT(gfx, 6) ? fg : bg;
		*p++ = BIT(gfx, 5) ? fg : bg;
		if (dw)
		*p++ = BIT(gfx, 5) ? fg : bg;
		*p++ = BIT(gfx, 4) ? fg : bg;
		if (dw)
		*p++ = BIT(gfx, 4) ? fg : bg;
		*p++ = BIT(gfx, 3) ? fg : bg;
		if (dw)
		*p++ = BIT(gfx, 3) ? fg : bg;
		*p++ = BIT(gfx, 2) ? fg : bg;
		if (dw)
		*p++ = BIT(gfx, 2) ? fg : bg;
		*p++ = BIT(gfx, 1) ? fg : bg;
		if (dw)
		*p++ = BIT(gfx, 1) ? fg : bg;
		*p++ = BIT(gfx, 0) ? fg : bg;
		if (dw)
		*p++ = BIT(gfx, 0) ? fg : bg;
	}
	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(sm7238_state::scanline_callback)
{
	UINT16 y = m_screen->vpos();
	UINT16 o = m_video.ptr;

	if (y < KSM_VERT_START) return;
	y -= KSM_VERT_START;
	if (y >= KSM_DISP_VERT) return;

	if (y == 0)
		m_video.ptr = 0;
	else if (y%10 == 0) {
		m_video.ptr = m_p_videoram[m_video.ptr + m_video.stride + 1] |
			(m_p_videoram[m_video.ptr + 0x1000 + m_video.stride + 1] << 8);
		m_video.ptr &= 0x0fff;
		DBG_LOG(2,"scanline_cb",("y %d row %d old ptr %04x new ptr %04x\n", y, y%10, o, m_video.ptr));
	}

	draw_scanline(&m_tmpbmp.pix16(y), m_video.ptr, y%10);
}

void sm7238_state::text_memory_clear()
{
	int y = 0, ptr = 0;

	do {
		memset(&m_p_videoram[ptr], 0x20, m_video.stride);
		memset(&m_p_videoram[ptr + 0x1000], 0x20, m_video.stride);
		ptr = m_p_videoram[ptr + m_video.stride + 1] | (m_p_videoram[ptr + 0x1000 + m_video.stride + 1] << 8);
		ptr &= 0x0fff;
		y++;
	} while (y<26);
}

void sm7238_state::recompute_parameters()
{
	rectangle visarea;
	int horiz_pix_total = m_video.stride * 8;

	visarea.set(0, horiz_pix_total - 1, 0, KSM_DISP_VERT - 1);
	machine().first_screen()->configure(horiz_pix_total, KSM_DISP_VERT, visarea,
		HZ_TO_ATTOSECONDS((m_video.stride == 80) ? 60 : 57.1 ));
}

UINT32 sm7238_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_video.control, 3))
	copybitmap(bitmap, m_tmpbmp, 0, 0, KSM_HORZ_START, KSM_VERT_START, cliprect);
	return 0;
}

void sm7238_state::screen_eof(screen_device &screen, bool state)
{
	m_pic8259->ir2_w(state);
}


/* F4 Character Displayer */
static const gfx_layout sm7238_charlayout =
{
	8, 12,                  /* most chars use 8x10 pixels */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	16*8                 /* every char takes 16 bytes */
};

static GFXDECODE_START( sm7238 )
	GFXDECODE_ENTRY("chargen", 0x0000, sm7238_charlayout, 0, 1)
GFXDECODE_END

PALETTE_INIT_MEMBER(sm7238_state, sm7238)
{
	palette.set_pen_color(0, rgb_t::black); // black
	palette.set_pen_color(1, 0x00, 0xc0, 0x00); // green
	palette.set_pen_color(2, 0x00, 0xff, 0x00); // highlight
}

static MACHINE_CONFIG_START( sm7238, sm7238_state )
	MCFG_CPU_ADD("maincpu", I8080, XTAL_16_5888MHz/9)
	MCFG_CPU_PROGRAM_MAP(sm7238_mem)
	MCFG_CPU_IO_MAP(sm7238_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", sm7238_state, scanline_callback, "screen", 0, 1)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
#if KSM_COLUMNS == 80
	MCFG_SCREEN_RAW_PARAMS(XTAL_12_5MHz,
		KSM_TOTAL_HORZ, KSM_HORZ_START, KSM_HORZ_START+KSM_DISP_HORZ,
		KSM_TOTAL_VERT, KSM_VERT_START, KSM_VERT_START+KSM_DISP_VERT);
#else
	MCFG_SCREEN_RAW_PARAMS(XTAL_20_625MHz,
		KSM_TOTAL_HORZ, KSM_HORZ_START, KSM_HORZ_START+KSM_DISP_HORZ,
		KSM_TOTAL_VERT, KSM_VERT_START, KSM_VERT_START+KSM_DISP_VERT);
#endif
	MCFG_SCREEN_UPDATE_DRIVER(sm7238_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(sm7238_state, screen_eof)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(sm7238_state, sm7238)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sm7238)

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NOOP)

	MCFG_DEVICE_ADD("t_hblank", PIT8253, 0)
	MCFG_PIT8253_CLK1(XTAL_16_384MHz/9) // XXX workaround -- keyboard is slower and doesn't sync otherwise
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(sm7238_state, write_keyboard_clock))

	MCFG_DEVICE_ADD("t_vblank", PIT8253, 0)
	MCFG_PIT8253_CLK2(XTAL_16_5888MHz/9)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(sm7238_state, write_printer_clock))

	MCFG_DEVICE_ADD("t_color", PIT8253, 0)

	MCFG_DEVICE_ADD("t_iface", PIT8253, 0)
	MCFG_PIT8253_CLK1(XTAL_16_5888MHz/9)
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE("i8251line", i8251_device, write_txc))
	MCFG_PIT8253_CLK2(XTAL_16_5888MHz/9)
	MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE("i8251line", i8251_device, write_rxc))

	// serial connection to host
	MCFG_DEVICE_ADD("i8251line", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir1_w))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "null_modem")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("i8251line", i8251_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("i8251line", i8251_device, write_cts))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("i8251line", i8251_device, write_dsr))

	// serial connection to KM-035 keyboard
	MCFG_DEVICE_ADD("i8251kbd", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("keyboard", km035_device, write_rxd))
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir3_w))

	MCFG_DEVICE_ADD("keyboard", KM035, 0)
	MCFG_KM035_TX_HANDLER(DEVWRITELINE("i8251kbd", i8251_device, write_rxd))
	MCFG_KM035_RTS_HANDLER(DEVWRITELINE("i8251kbd", i8251_device, write_cts))

	// serial connection to printer
	MCFG_DEVICE_ADD("i8251prn", I8251, 0)
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir3_w))

	MCFG_RS232_PORT_ADD("prtr", default_rs232_devices, 0)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("i8251prn", i8251_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("i8251prn", i8251_device, write_cts))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("i8251prn", i8251_device, write_dsr))
MACHINE_CONFIG_END

ROM_START( sm7238 )
	ROM_REGION(0xa000, "maincpu", ROMREGION_ERASE00)
	// version 1.0
	ROM_LOAD( "01_6.064", 0x0000, 0x2000, CRC(10f98d90) SHA1(cbed0b92d8beb558ce27ccd8256e1b3ab7351a58))
	ROM_LOAD( "02_6.064", 0x2000, 0x2000, CRC(b22c0202) SHA1(68a5c45697c4a541a182f0762904d36f4496e344))
	ROM_LOAD( "03_6.064", 0x4000, 0x2000, CRC(3e9d37ad) SHA1(26eb257733a88bd5665a9601813934da27219bc2))
	ROM_LOAD( "04_6.064", 0x6000, 0x2000, CRC(7b8c9e06) SHA1(537bd35749c15ef66656553d9e7ec6a1f9671f98))
	// version 1.1 undumped

	ROM_REGION(0x2000, "chargen", ROMREGION_ERASE00)
	ROM_LOAD( "bsk1_00_2.064", 0x0000, 0x2000, CRC(1e3d5885) SHA1(5afdc10f775f424473c2a78de62e3bfc82bdddd1))
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    INIT                      COMPANY     FULLNAME       FLAGS */
COMP( 1989, sm7238,   0,      0,       sm7238,    0,       driver_device,     0,     "USSR",     "SM 7238",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_KEYBOARD)
