/***************************************************************************

        15IE-00-013 Terminal

            board images : http://asvcorp.ru/darch/hardware/pdp/fryazin-display/index.html

        29/06/2012 Skeleton driver.

    A serial (RS232 or current loop) green-screen terminal, mostly VT52 compatible
    (no Hold Screen mode and no graphics character set, but has Cyrillic characters).
    The top line is a status line.

****************************************************************************/

#include "emu.h"
#include "cpu/ie15/ie15.h"
#include "imagedev/bitbngr.h"
#include "machine/keyboard.h"
#include "sound/beep.h"

#define SCREEN_PAGE (80*48)

#define IE_1        0x80
#define IE_KB_ACK   1

#define IE15_TOTAL_HORZ 1000
#define IE15_TOTAL_VERT 28*11

#define IE15_DISP_HORZ  800
#define IE15_DISP_VERT  25*11

#define IE15_HORZ_START 100
#define IE15_VERT_START 2*11

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

#if VERBOSE_DBG > 0
#define LOOPBACK (m_io_keyboard->read() & 0x20)
#else
#define LOOPBACK (0)
#endif

#define BITBANGER_TAG   "bitbanger"
#define KEYBOARD_TAG "keyboard"

class ie15_state : public driver_device
{
public:
	ie15_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_beeper(*this, "beeper"),
		m_bitbanger(*this, BITBANGER_TAG),
		m_io_keyboard(*this, KEYBOARD_TAG)
	{ }

	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(ie15);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_hle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);
	DECLARE_WRITE8_MEMBER(kbd_put);
	static const bitbanger_config ie15_bitbanger_config;
	TIMER_CALLBACK_MEMBER(serial_tx_callback);
	emu_timer  *m_serial_tx_timer;

	DECLARE_WRITE8_MEMBER( mem_w );
	DECLARE_READ8_MEMBER( mem_r );
	DECLARE_WRITE8_MEMBER( mem_addr_lo_w );
	DECLARE_WRITE8_MEMBER( mem_addr_hi_w );
	DECLARE_WRITE8_MEMBER( mem_addr_inc_w );
	DECLARE_WRITE8_MEMBER( mem_addr_dec_w );
	DECLARE_READ8_MEMBER( flag_r );
	DECLARE_WRITE8_MEMBER( flag_w );
	DECLARE_WRITE8_MEMBER( beep_w );
	DECLARE_READ8_MEMBER( kb_r );
	DECLARE_READ8_MEMBER( kb_ready_r );
	DECLARE_READ8_MEMBER( kb_s_red_r );
	DECLARE_READ8_MEMBER( kb_s_sdv_r );
	DECLARE_READ8_MEMBER( kb_s_dk_r );
	DECLARE_READ8_MEMBER( kb_s_dupl_r );
	DECLARE_READ8_MEMBER( kb_s_lin_r );
	DECLARE_WRITE8_MEMBER( kb_ready_w );
	DECLARE_READ8_MEMBER( serial_tx_ready_r );
	DECLARE_WRITE8_MEMBER( serial_w );
	DECLARE_READ8_MEMBER( serial_rx_ready_r );
	DECLARE_READ8_MEMBER( serial_r );
#if 0
	DECLARE_WRITE8_MEMBER( serial_speed_w );
#endif

private:
	TIMER_CALLBACK_MEMBER(ie15_beepoff);
	UINT32 draw_scanline(UINT16 *p, UINT16 offset, UINT8 scanline, UINT8 y);
	bitmap_ind16 m_tmpbmp;

	const UINT8 *m_p_chargen;
	UINT8 *m_p_videoram;
	UINT8 m_beep;
	UINT8 m_cursor;
	UINT8 m_kb_data;
	UINT8 m_kb_flag;
	UINT8 m_kb_flag0;
	UINT8 m_latch;
	UINT8 m_ruslat;
	UINT8 m_serial_rx_bits;
	UINT8 m_serial_rx_buffer;
	UINT8 m_serial_rx_data;
	UINT8 m_serial_tx_bits;
	UINT8 m_serial_tx_data;
	UINT8 m_statusline;
	UINT8 m_video;
	UINT32 m_videoptr;
	UINT32 m_videoptr_2;

	static void bitbanger_callback(running_machine &machine, UINT8 bit);
	DECLARE_WRITE_LINE_MEMBER( serial_rx_callback );

protected:
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beeper;
	required_device<bitbanger_device> m_bitbanger;
	required_ioport m_io_keyboard;
};

READ8_MEMBER( ie15_state::mem_r ) {
	UINT8 ret;

	ret = m_p_videoram[m_videoptr];
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0 && m_videoptr >= SCREEN_PAGE)
	{
		DBG_LOG(2,"memory",("R @ %03x == %02x\n", m_videoptr, ret));
	}
	m_videoptr++;
	m_videoptr &= 0xfff;
	m_latch = 0;
	return ret;
}

WRITE8_MEMBER( ie15_state::mem_w ) {
	if ((m_latch ^= 1) == 0) {
		if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0 && m_videoptr >= SCREEN_PAGE)
		{
			DBG_LOG(2,"memory",("W @ %03x <- %02x\n", m_videoptr, data));
		}
		m_p_videoram[m_videoptr++] = data;
		m_videoptr &= 0xfff;
	}
}

WRITE8_MEMBER( ie15_state::mem_addr_inc_w ) {
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		DBG_LOG(2,"memory",("++ %03x\n", m_videoptr));
	}
	m_videoptr++;
	m_videoptr &= 0xfff;
	if (m_video)
		m_videoptr_2 = m_videoptr;
}

WRITE8_MEMBER( ie15_state::mem_addr_dec_w ) {
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		DBG_LOG(2,"memory",("-- %03x\n", m_videoptr));
	}
	m_videoptr--;
	m_videoptr &= 0xfff;
	if (m_video)
		m_videoptr_2 = m_videoptr;
}

WRITE8_MEMBER( ie15_state::mem_addr_lo_w ) {
	UINT16 tmp = m_videoptr;

	tmp &= 0xff0;
	tmp |= ((data >> 4) & 0xf);
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		DBG_LOG(2,"memory",("lo %03x <- %02x = %03x\n", m_videoptr, data, tmp));
	}
	m_videoptr = tmp;
	if (m_video)
		m_videoptr_2 = tmp;
}

WRITE8_MEMBER( ie15_state::mem_addr_hi_w ) {
	UINT16 tmp = m_videoptr;

	tmp &= 0xf;
	tmp |= (data << 4);
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		DBG_LOG(2,"memory",("hi %03x <- %02x = %03x\n", m_videoptr, data, tmp));
	}
	m_videoptr = tmp;
	if (m_video)
		m_videoptr_2 = tmp;
}

TIMER_CALLBACK_MEMBER(ie15_state::ie15_beepoff)
{
	machine().device<beep_device>("beeper")->set_state(0);
}

WRITE8_MEMBER( ie15_state::beep_w ) {
	UINT16 length = (m_beep&128)?150:400;

	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		DBG_LOG(1,"beep",("(%s)\n", m_beep?"short":"long"));
	}
	machine().scheduler().timer_set(attotime::from_msec(length), timer_expired_delegate(FUNC(ie15_state::ie15_beepoff),this));
	machine().device<beep_device>("beeper")->set_state(1);
}

// active high
READ8_MEMBER( ie15_state::kb_r ) {
	DBG_LOG(2,"keyboard",("R %02X '%c'\n", m_kb_data, m_kb_data < 0x20?' ':m_kb_data));
	return m_kb_data;
}

// active low
READ8_MEMBER( ie15_state::kb_ready_r ) {
	m_kb_flag &= IE_1;
	if (m_kb_flag != m_kb_flag0) {
		DBG_LOG(2,"keyboard",("? %c\n", m_kb_flag?'n':'y'));
		m_kb_flag0 = m_kb_flag;
	}
	return m_kb_flag;
}

// active low
WRITE8_MEMBER( ie15_state::kb_ready_w ) {
	DBG_LOG(2,"keyboard",("clear ready\n"));
	m_kb_flag = IE_1 | IE_KB_ACK;
}


// active high; active = interpret controls, inactive = display controls
READ8_MEMBER( ie15_state::kb_s_red_r ) {
	return m_io_keyboard->read() & 0x01 ? IE_1 : 0;
}

// active high; active = setup mode
READ8_MEMBER( ie15_state::kb_s_sdv_r ) {
	return m_io_keyboard->read() & 0x02 ? IE_1 : 0;
}

// active high, XXX stub
READ8_MEMBER( ie15_state::kb_s_dk_r ) {
	return 0;
}

// active low; active = full duplex, inactive = half duplex
READ8_MEMBER( ie15_state::kb_s_dupl_r ) {
	return m_io_keyboard->read() & 0x08 ? IE_1 : 0;
}

// active high; active = on-line, inactive = local editing
READ8_MEMBER( ie15_state::kb_s_lin_r ) {
	return m_io_keyboard->read() & 0x10 ? IE_1 : 0;
}

// active low
READ8_MEMBER( ie15_state::serial_rx_ready_r ) {
	if (LOOPBACK)
		return m_serial_tx_data ? 0 : IE_1;
	else
		return m_serial_rx_data ? 0 : IE_1;
}

// not called unless data is ready
READ8_MEMBER( ie15_state::serial_r ) {
	UINT8 tmp;

	if (LOOPBACK) {
		tmp = m_serial_tx_data;
		m_serial_tx_data = 0;
	} else {
		tmp = m_serial_rx_data;
		m_serial_rx_data = 0;
	}
	DBG_LOG(2,"serial",("R %02X '%c'\n", tmp, tmp < 0x20?' ':tmp));
	return tmp;
}

/*
    m_serial_rx_buffer  incoming bits.
    m_serial_rx_bits    number of bits in _buffer.
    m_serial_rx_data    complete byte, ready to be read by host, or 0 if no data.
*/

WRITE_LINE_MEMBER( ie15_state::serial_rx_callback )
{
	UINT8 tmp = m_serial_rx_bits;

	switch (m_serial_rx_bits) {
		// wait for start bit (0)
		case 10:
			m_serial_rx_bits = 0;
		case 0:
			if (!state) {
				m_serial_rx_bits++;
				m_serial_rx_buffer = 0;
			}
			break;
		// stuff incoming bits into byte buffer
		case 1: case 2: case 3: case 4:
		case 5: case 6: case 7: case 8:
			m_serial_rx_buffer |= state << (m_serial_rx_bits-1);
			m_serial_rx_bits++;
			break;
		// expecting stop bit (1)
		case 9:
			if (state && !m_serial_rx_data) {
				m_serial_rx_data = m_serial_rx_buffer;
				m_serial_rx_bits++;
			} else
				m_serial_rx_bits = 0;
			break;
		default:
			// overflow
			break;
	}
	DBG_LOG(2,"serial",("r %d bits %02d->%02d buffer %02X data %02X\n",
		state, tmp, m_serial_rx_bits, m_serial_rx_buffer, m_serial_rx_data));
}

const bitbanger_config ie15_state::ie15_bitbanger_config =
{
	DEVCB_DRIVER_LINE_MEMBER(ie15_state, serial_rx_callback),   /* callback */
	BITBANGER_PRINTER,                                          /* default mode */
	BITBANGER_9600,                                             /* default output baud */
	BITBANGER_0PERCENT                                          /* default fine tune adjustment */
};

// active high
READ8_MEMBER( ie15_state::serial_tx_ready_r ) {
	return m_serial_tx_data ? 0 : IE_1;
}

WRITE8_MEMBER( ie15_state::serial_w ) {
	DBG_LOG(2,"serial",("W %02X '%c'\n", data, data < 0x20?' ':data));
	if (LOOPBACK) {
		if (!m_serial_tx_data)
			m_serial_tx_data = data;
	} else {
		/* 1 start bit */
		m_bitbanger->output(0);
		/* 8 data bits, 1 stop bit, no parity */
		m_serial_tx_bits = 8;
		m_serial_tx_data = data;
		m_serial_tx_timer->adjust(attotime::from_hz(9600));
	}
}

TIMER_CALLBACK_MEMBER(ie15_state::serial_tx_callback) {
	if (!m_serial_tx_bits) {
		m_bitbanger->output(1);
		m_serial_tx_data = 0;
	} else {
		m_bitbanger->output(BIT(m_serial_tx_data, 8-(m_serial_tx_bits--)));
		m_serial_tx_timer->adjust(attotime::from_hz(9600));
	}
}

READ8_MEMBER( ie15_state::flag_r ) {
	UINT8 ret = 0;

	switch (offset)
	{
		case 0: // hsync pulse (not hblank)
			ret = machine().first_screen()->hpos() < IE15_HORZ_START;
			break;
		case 1: // marker scanline
			ret = (machine().first_screen()->vpos() % 11) > 7;
			break;
		case 2: // vblank
			ret = !machine().first_screen()->vblank();
			break;
		case 4:
			ret = m_ruslat;
			break;
		default:
			break;
	}
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0 && ret)
	{
		DBG_LOG(2,"flag",("read %d: %d\n", offset, ret));
	}
	return ret;
}

WRITE8_MEMBER( ie15_state::flag_w ) {
	switch (offset)
	{
		case 0:
			m_video = data;
			break;
		case 1:
			m_cursor = data;
			break;
		case 2:
			m_beep = data;
			break;
		case 3:
			m_statusline = data;
			break;
		case 4:
			m_ruslat = data;
			break;
		default:
			break;
	}
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		DBG_LOG(2,"flag",("%sset %d\n", data?"":"re", offset));
	}
}

static ADDRESS_MAP_START(ie15_mem, AS_PROGRAM, 8, ie15_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ie15_io, AS_IO, 8, ie15_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(000, 000) AM_READ(mem_r) AM_WRITE(mem_w)   // 00h W: memory request, R: memory data [6.1.2.2]
	AM_RANGE(001, 001) AM_READ(serial_rx_ready_r) AM_WRITENOP   // 01h W: memory latch [6.1.2.2]
	AM_RANGE(002, 002) AM_WRITE(mem_addr_hi_w)      // 02h W: memory address high [6.1.2.2]
	AM_RANGE(003, 003) AM_WRITE(mem_addr_lo_w)      // 03h W: memory address low [6.1.2.2]
	AM_RANGE(004, 004) AM_WRITE(mem_addr_inc_w)     // 04h W: memory address counter + [6.1.2.2]
	AM_RANGE(005, 005) AM_WRITE(mem_addr_dec_w)     // 05h W: memory address counter - [6.1.2.2]
	AM_RANGE(006, 006) AM_READ(serial_r) AM_WRITE(serial_w)     // 06h W: serial port data [6.1.5.4]
// port 7 is handled in cpu core
	AM_RANGE(010, 010) AM_READ(serial_tx_ready_r) AM_WRITE(beep_w)  // 08h W: speaker control [6.1.5.4]
	AM_RANGE(011, 011) AM_READ(kb_r)            // 09h R: keyboard data [6.1.5.2]
	AM_RANGE(012, 012) AM_READ(kb_s_red_r)          // 0Ah I: keyboard mode "RED" [6.1.5.2]
	AM_RANGE(013, 013) AM_READ(kb_ready_r)          // 0Bh R: keyboard data ready [6.1.5.2]
	AM_RANGE(014, 014) AM_READ(kb_s_sdv_r) AM_WRITENOP  // 0Ch W: serial port speed [6.1.3.1], R: keyboard mode "SDV" [6.1.5.2]
	AM_RANGE(015, 015) AM_READ(kb_s_dk_r) AM_WRITE(kb_ready_w)  // 0Dh I: keyboard mode "DK" [6.1.5.2]
	AM_RANGE(016, 016) AM_READ(kb_s_dupl_r)         // 0Eh I: keyboard mode "DUPL" [6.1.5.2]
	AM_RANGE(017, 017) AM_READ(kb_s_lin_r)          // 0Fh I: keyboard mode "LIN" [6.1.5.2]
// simulation of flag registers
	AM_RANGE(020, 027) AM_READ(flag_r) AM_WRITE(flag_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ie15 )
	PORT_START("keyboard")
	PORT_DIPNAME(0x01, 0x00, "RED mode")
	PORT_DIPSETTING(0x00, "Off" )
	PORT_DIPSETTING(0x01, "On" )
	PORT_DIPNAME(0x02, 0x00, "SDV mode (Setup)")
	PORT_DIPSETTING(0x00, "Off" )
	PORT_DIPSETTING(0x02, "On" )
	PORT_DIPNAME(0x08, 0x00, "DUPL mode")
	PORT_DIPSETTING(0x00, "Off" )
	PORT_DIPSETTING(0x08, "On" )
	PORT_DIPNAME(0x10, 0x00, "LIN mode")
	PORT_DIPSETTING(0x00, "Off" )
	PORT_DIPSETTING(0x10, "On" )
	PORT_DIPNAME(0x20, 0x00, "digital loopback")
	PORT_DIPSETTING(0x00, "Off" )
	PORT_DIPSETTING(0x20, "On" )
INPUT_PORTS_END

WRITE8_MEMBER( ie15_state::kbd_put )
{
	DBG_LOG(2,"keyboard",("W %02X<-%02X '%c' %c\n", m_kb_data, data, data < 0x20?' ':data, m_kb_flag?'n':'y'));
	if (m_kb_flag == IE_1) {
		m_kb_data = data;
		m_kb_flag = 0;
	}
}

static ASCII_KEYBOARD_INTERFACE( keyboard_intf )
{
	DEVCB_DRIVER_MEMBER(ie15_state, kbd_put)
};


void ie15_state::machine_reset()
{
	m_ruslat = m_beep = m_statusline = m_cursor = m_video = m_kb_data = m_kb_flag0 = 0;
	m_serial_tx_data = m_serial_tx_bits = m_serial_rx_buffer = m_serial_rx_data = m_serial_rx_bits = 0;
	m_kb_flag = IE_1;

	machine().device<beep_device>("beeper")->set_frequency(2400);
	machine().device<beep_device>("beeper")->set_state(0);

	m_serial_tx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ie15_state::serial_tx_callback),this));
}

void ie15_state::video_start()
{
	m_p_chargen = machine().root_device().memregion("chargen")->base();
	m_p_videoram = memregion("video")->base();
	m_videoptr = m_videoptr_2 = m_latch = 0;

	m_tmpbmp.allocate(IE15_DISP_HORZ, IE15_DISP_VERT);
}

UINT32 ie15_state::draw_scanline(UINT16 *p, UINT16 offset, UINT8 scanline, UINT8 y)
{
	UINT8 gfx,fg,bg,ra,blink,red;
	UINT16 x,chr;

	bg = 0; fg = 1; ra = scanline % 8;
	blink = (machine().first_screen()->frame_number() % 10) >= 5;
	red = m_io_keyboard->read() & 0x01;

	DBG_LOG(2,"draw_scanline",
		("addr %03x row %d-%d video %d\n", offset, y, scanline, m_video));

	for (x = offset; x < offset + 80; x++)
	{
		if (m_video) {
			chr = m_p_videoram[x] << 3;
			gfx = m_p_chargen[chr | ra];

			/*
			    Cursor is a character with only 3 scan lines, and is
			    not shown if flag 1 is not active on this scan line.
			    It always blinks if shown.

			    Control characters blink if RED mode is on and they
			    are not on status line; else they are blanked out.
			*/

			if (scanline > 7 && (!m_cursor || blink))
				gfx = 0;
			if (chr < (0x20<<3)) {
				if (!y || !red || blink)
					gfx = 0;
				else
					gfx = m_p_chargen[chr | 0x200 | ra];
			}
			/* Display a scanline of a character */
			*p++ = BIT(gfx, 7) ? fg : bg;
			*p++ = BIT(gfx, 6) ? fg : bg;
			*p++ = BIT(gfx, 5) ? fg : bg;
			*p++ = BIT(gfx, 4) ? fg : bg;
			*p++ = BIT(gfx, 3) ? fg : bg;
			*p++ = BIT(gfx, 2) ? fg : bg;
			*p++ = BIT(gfx, 1) ? fg : bg;
			*p++ = bg;
			*p++ = bg;
			*p++ = bg;
		} else {
			*p++ = bg;
			*p++ = bg;
			*p++ = bg;
			*p++ = bg;
			*p++ = bg;
			*p++ = bg;
			*p++ = bg;
			*p++ = bg;
			*p++ = bg;
			*p++ = bg;
		}
	}
	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(ie15_state::scanline_callback)
{
	UINT16 y = machine().first_screen()->vpos();
//  DBG_LOG(2,"scanline",
//      ("addr %03x frame %lld x %04d y %03d\n", m_videoptr_2, machine().first_screen()->frame_number(), machine().first_screen()->hpos(), y));
	if (y>=IE15_VERT_START) {
		y -= IE15_VERT_START;
		if (y < IE15_DISP_VERT) {
			draw_scanline(&m_tmpbmp.pix16(y), m_videoptr_2, y%11, y/11);
		}
	}
}

UINT32 ie15_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap( bitmap, m_tmpbmp, 0, 0, IE15_HORZ_START, IE15_VERT_START, cliprect );
	return 0;
}


/* F4 Character Displayer */
static const gfx_layout ie15_charlayout =
{
	7, 8,                   /* 7x8 pixels in 10x11 cell */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( ie15 )
	GFXDECODE_ENTRY( "chargen", 0x0000, ie15_charlayout, 0, 1 )
GFXDECODE_END

PALETTE_INIT_MEMBER(ie15_state, ie15)
{
	palette.set_pen_color(0, rgb_t::black); // black
	palette.set_pen_color(1, 0x00, 0xc0, 0x00); // green
}

static MACHINE_CONFIG_START( ie15, ie15_state )
	/* Basic machine hardware */
	MCFG_CPU_ADD("maincpu", IE15, XTAL_30_8MHz / 10)
	MCFG_CPU_PROGRAM_MAP(ie15_mem)
	MCFG_CPU_IO_MAP(ie15_io)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("scantimer", ie15_state, scanline_callback, attotime::from_hz(50*28*11))
	MCFG_TIMER_START_DELAY(attotime::from_hz(XTAL_30_8MHz/(2*IE15_HORZ_START)))

	/* Video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(ie15_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL_30_8MHz/2,IE15_TOTAL_HORZ,IE15_HORZ_START,
		IE15_HORZ_START+IE15_DISP_HORZ,IE15_TOTAL_VERT,IE15_VERT_START,
		IE15_VERT_START+IE15_DISP_VERT);
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ie15)
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(ie15_state, ie15)

	/* Devices */
	MCFG_ASCII_KEYBOARD_ADD(KEYBOARD_TAG, keyboard_intf)
	MCFG_BITBANGER_ADD(BITBANGER_TAG, ie15_state::ie15_bitbanger_config)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.15)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( ie15 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )
	ROM_DEFAULT_BIOS("5chip")
	ROM_SYSTEM_BIOS(0, "5chip", "5-chip firmware (newer)")
	ROMX_LOAD( "dump1.bin", 0x0000, 0x1000, CRC(14b82284) SHA1(5ac4159fbb1c3b81445605e26cd97a713ae12b5f),ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "6chip", "6-chip firmware (older)")
	ROMX_LOAD( "dump5.bin", 0x0000, 0x1000, CRC(01f2e065) SHA1(2b72dc0594e38a528400cd25aed0c47e0c432895),ROM_BIOS(2) )

	ROM_REGION( 0x1000, "video", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "chargen", ROMREGION_ERASE00 )
	ROM_LOAD( "chargen-15ie.bin", 0x0000, 0x0800, CRC(ed16bf6b) SHA1(6af9fb75f5375943d5c0ce9ed408e0fb4621b17e) )
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT                       INIT   COMPANY     FULLNAME       FLAGS */
COMP( 1980, ie15,     0,      0,       ie15,      ie15,    driver_device,     0,     "USSR",     "15IE-00-013", 0)
