// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    15IE-00-013 Terminal

    A serial (RS232 or current loop) green-screen terminal, mostly VT52
    compatible (no Hold Screen mode and no graphics character set).

    Alternate character set (selected by SO/SI chars) is Cyrillic.

****************************************************************************/

#include "emu.h"
#include "machine/ie15.h"

#include "emupal.h"

#include "ie15.lh"


//#define LOG_GENERAL (1U << 0) //defined in logmacro.h already
#define LOG_RAM      (1U << 1)
#define LOG_CPU      (1U << 2)
#define LOG_KBD      (1U << 3)
#define LOG_SCANLINE (1U << 4)
#define LOG_SERIAL   (1U << 5)

#define VERBOSE (LOG_GENERAL|LOG_KBD)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGM(...)      LOGMASKED(LOG_RAM,  __VA_ARGS__)
#define LOGCPU(...)    LOGMASKED(LOG_CPU,  __VA_ARGS__)
#define LOGKBD(...)    LOGMASKED(LOG_KBD,  __VA_ARGS__)
#define LOGSERIAL(...) LOGMASKED(LOG_SERIAL,  __VA_ARGS__)


#define _PRINT(ch) (std::isprint(ch & 127)?(ch & 127):' ')


ie15_device::ie15_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_p_videoram(*this, "video")
	, m_p_chargen(*this, "chargen")
	, m_beeper(*this, "beeper")
	, m_screen(*this, "screen")
	, m_keyboard(*this, "keyboard")
	, m_io_keyboard(*this, "io_keyboard")
	, m_lat_led(*this, "lat_led")
	, m_nr_led(*this, "nr_led")
	, m_pch_led(*this, "pch_led")
	, m_dup_led(*this, "dup_led")
	, m_lin_led(*this, "lin_led")
	, m_red_led(*this, "red_led")
	, m_sdv_led(*this, "sdv_led")
	, m_prd_led(*this, "prd_led")
	, m_rs232_conn_txd_handler(*this)
	, m_rs232_conn_dtr_handler(*this)
	, m_rs232_conn_rts_handler(*this)
	  // Until the UART is implemented
	, m_rs232_txbaud(*this, "RS232_TXBAUD")
	, m_rs232_rxbaud(*this, "RS232_RXBAUD")
	, m_rs232_databits(*this, "RS232_DATABITS")
	, m_rs232_parity(*this, "RS232_PARITY")
	, m_rs232_stopbits(*this, "RS232_STOPBITS")
{
}

ie15_device::ie15_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ie15_device(mconfig, IE15, tag, owner, clock)
{
}


uint8_t ie15_device::mem_r()
{
	uint8_t ret;

	ret = m_p_videoram[m_video.ptr1];
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0 && m_video.ptr1 >= SCREEN_PAGE)
	{
		LOGM("memory R @ %03x == %02x\n", m_video.ptr1, ret);
	}
	m_video.ptr1++;
	m_video.ptr1 &= 0xfff;
	m_latch = 0;
	return ret;
}

void ie15_device::mem_w(uint8_t data)
{
	if ((m_latch ^= 1) == 0)
	{
		if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0 && m_video.ptr1 >= SCREEN_PAGE)
		{
			LOGM("memory W @ %03x <- %02x\n", m_video.ptr1, data);
		}
		m_p_videoram[m_video.ptr1++] = data;
		m_video.ptr1 &= 0xfff;
	}
}

void ie15_device::mem_addr_inc_w(uint8_t data)
{
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		LOGM("memory addr ++ %03x\n", m_video.ptr1);
	}
	m_video.ptr1++;
	m_video.ptr1 &= 0xfff;
	if (m_video.enable) m_video.ptr2 = m_video.ptr1;
}

void ie15_device::mem_addr_dec_w(uint8_t data)
{
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		LOGM("memory addr -- %03x\n", m_video.ptr1);
	}
	m_video.ptr1--;
	m_video.ptr1 &= 0xfff;
	if (m_video.enable) m_video.ptr2 = m_video.ptr1;
}

void ie15_device::mem_addr_lo_w(uint8_t data)
{
	uint16_t tmp = m_video.ptr1;

	tmp &= 0xff0;
	tmp |= ((data >> 4) & 0xf);
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		LOGM("memory addr lo %03x <- %02x = %03x\n", m_video.ptr1, data, tmp);
	}
	m_video.ptr1 = tmp;
	if (m_video.enable) m_video.ptr2 = tmp;
}

void ie15_device::mem_addr_hi_w(uint8_t data)
{
	uint16_t tmp = m_video.ptr1;

	tmp &= 0xf;
	tmp |= (data << 4);
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		LOGM("memory addr hi %03x <- %02x = %03x\n", m_video.ptr1, data, tmp);
	}
	m_video.ptr1 = tmp;
	if (m_video.enable) m_video.ptr2 = tmp;
}

TIMER_CALLBACK_MEMBER(ie15_device::ie15_beepoff)
{
	m_beeper->set_state(0);
}

void ie15_device::beep_w(uint8_t data)
{
	uint16_t length = (m_long_beep & IE_TRUE) ? 150 : 400;

	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		LOG("beep (%s)\n", m_long_beep ? "short" : "long");
	}
	m_beepoff_timer->adjust(attotime::from_msec(length));
	m_beeper->set_state(1);
}

/* keyboard */

// active high
uint8_t ie15_device::kb_r()
{
	LOGKBD("keyboard data R %02X '%c'\n", m_kb_data, _PRINT(m_kb_data));
	return m_kb_data;
}

// active low
uint8_t ie15_device::kb_ready_r()
{
	m_kb_flag &= IE_TRUE;
	if (m_kb_flag != m_kb_flag0)
	{
		LOGKBD("keyboard ready? %c\n", m_kb_flag ? 'n' : 'y');
		m_kb_flag0 = m_kb_flag;
	}
	return m_kb_flag;
}

// active low
void ie15_device::kb_ready_w(uint8_t data)
{
	LOGKBD("keyboard clear ready\n");
	m_kb_flag = IE_TRUE | ie15_keyboard_device::IE_KB_ACK;
}


// active high; active = interpret controls, inactive = display controls
uint8_t ie15_device::kb_s_red_r()
{
	return m_io_keyboard->read() & ie15_keyboard_device::IE_KB_RED ? IE_TRUE : 0;
}

// active high; active = setup mode
uint8_t ie15_device::kb_s_sdv_r()
{
	return m_kbd_sdv ? IE_TRUE : 0;
}

// active high; active = keypress detected on aux keypad
uint8_t ie15_device::kb_s_dk_r()
{
	return m_kb_control & ie15_keyboard_device::IE_KB_DK ? IE_TRUE : 0;
}

// active low; active = full duplex, inactive = half duplex
uint8_t ie15_device::kb_s_dupl_r()
{
	return m_io_keyboard->read() & ie15_keyboard_device::IE_KB_DUP ? IE_TRUE : 0;
}

// active high; active = on-line, inactive = local editing
uint8_t ie15_device::kb_s_lin_r()
{
	return m_io_keyboard->read() & ie15_keyboard_device::IE_KB_LIN ? IE_TRUE : 0;
}

TIMER_CALLBACK_MEMBER(ie15_device::hblank_onoff_tick)
{
	if (m_hblank) // Transitioning from in blanking to out of blanking
	{
		m_hblank = 0;
		m_hblank_timer->adjust(m_screen->time_until_pos((m_vpos + 1) % IE15_TOTAL_VERT, 0));
		scanline_callback();
	}
	else // Transitioning from out of blanking to in blanking
	{
		m_hblank = 1;
		m_hblank_timer->adjust(m_screen->time_until_pos(m_vpos, IE15_HORZ_START));
	}
}

/* serial port */

WRITE_LINE_MEMBER(ie15_device::rs232_conn_rxd_w)
{
	device_serial_interface::rx_w(state);
}

void ie15_device::rcv_complete()
{
	receive_register_extract();
	m_serial_rx_char = get_received_char();
	m_serial_rx_ready = IE_FALSE;
}

void ie15_device::tra_callback()
{
	uint8_t bit = transmit_register_get_data_bit();
	m_rs232_conn_txd_handler(bit);
}

void ie15_device::tra_complete()
{
	m_serial_tx_ready = IE_TRUE;
}

// active low
uint8_t ie15_device::serial_rx_ready_r()
{
	return m_serial_rx_ready;
}

// active high
uint8_t ie15_device::serial_tx_ready_r()
{
	return m_serial_tx_ready;
}

// not called unless data are ready
uint8_t ie15_device::serial_r()
{
	m_serial_rx_ready = IE_TRUE;
	LOGSERIAL("serial R %02X '%c'\n", m_serial_rx_char, _PRINT(m_serial_rx_char));
	return m_serial_rx_char;
}

void ie15_device::serial_w(uint8_t data)
{
	LOGSERIAL("serial W %02X '%c'\n", data, _PRINT(data));

	m_serial_tx_ready = IE_FALSE;
	transmit_register_setup(data);
}

void ie15_device::serial_speed_w(uint8_t data)
{
	return;
}

WRITE_LINE_MEMBER(ie15_device::update_serial)
{
	int startbits = 1;
	int databits = m_rs232_databits->read();
	parity_t parity_table[] = { PARITY_NONE, PARITY_ODD, PARITY_EVEN, PARITY_MARK, PARITY_SPACE };
	parity_t parity = parity_table[m_rs232_parity->read()];
	stop_bits_t stopbits_table[] = { STOP_BITS_1, STOP_BITS_2 };
	stop_bits_t stopbits = stopbits_table[m_rs232_stopbits->read()];

	set_data_frame(startbits, databits, parity, stopbits);

	int txbaud = m_rs232_txbaud->read();
	set_tra_rate(txbaud);

	int rxbaud = m_rs232_rxbaud->read();
	set_rcv_rate(rxbaud);
}

uint8_t ie15_device::flag_r(offs_t offset)
{
	switch (offset)
	{
	case 0: // hsync pulse (not hblank)
		return m_hblank;
	case 1: // marker scanline
		return m_marker_scanline;
	case 2: // vblank
		return !m_screen->vblank();
	case 4:
		return m_kb_ruslat;
	default:
		break;
	}
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		LOGCPU("flag read %d: ?\n", offset);
	}
	return 0;
}

void ie15_device::flag_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		m_video.enable = data;
		break;
	case 1:
		m_video.cursor = data;
		break;
	case 2:
		m_long_beep = data;
		break;
	case 3:
		m_video.line25 = data;
		break;
	case 4:
		m_kb_ruslat = data;
		m_keyboard->set_ruslat(!!data);
		break;
	default:
		break;
	}
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0 && !offset)
	{
		LOGCPU("flag %sset %d\n", data ? "" : "re", offset);
	}
}

void ie15_device::ie15_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
}

void ie15_device::ie15_io(address_map &map)
{
	map.unmap_value_high();
	map(000, 000).rw(FUNC(ie15_device::mem_r), FUNC(ie15_device::mem_w));   // 00h W: memory request, R: memory data [6.1.2.2]
	map(001, 001).r(FUNC(ie15_device::serial_rx_ready_r)).nopw();   // 01h W: memory latch [6.1.2.2]
	map(002, 002).w(FUNC(ie15_device::mem_addr_hi_w));      // 02h W: memory address high [6.1.2.2]
	map(003, 003).w(FUNC(ie15_device::mem_addr_lo_w));      // 03h W: memory address low [6.1.2.2]
	map(004, 004).w(FUNC(ie15_device::mem_addr_inc_w));     // 04h W: memory address counter + [6.1.2.2]
	map(005, 005).w(FUNC(ie15_device::mem_addr_dec_w));     // 05h W: memory address counter - [6.1.2.2]
	map(006, 006).rw(FUNC(ie15_device::serial_r), FUNC(ie15_device::serial_w));     // 06h W: serial port data [6.1.5.4]
// port 7 is handled in cpu core
	map(010, 010).rw(FUNC(ie15_device::serial_tx_ready_r), FUNC(ie15_device::beep_w));  // 08h W: speaker control [6.1.5.4]
	map(011, 011).r(FUNC(ie15_device::kb_r));            // 09h R: keyboard data [6.1.5.2]
	map(012, 012).r(FUNC(ie15_device::kb_s_red_r));          // 0Ah I: keyboard mode "RED" [6.1.5.2]
	map(013, 013).r(FUNC(ie15_device::kb_ready_r));          // 0Bh R: keyboard data ready [6.1.5.2]
	map(014, 014).rw(FUNC(ie15_device::kb_s_sdv_r), FUNC(ie15_device::serial_speed_w)); // 0Ch W: serial port speed [6.1.3.1], R: keyboard mode "SDV" [6.1.5.2]
	map(015, 015).rw(FUNC(ie15_device::kb_s_dk_r), FUNC(ie15_device::kb_ready_w));  // 0Dh I: keyboard mode "DK" [6.1.5.2]
	map(016, 016).r(FUNC(ie15_device::kb_s_dupl_r));         // 0Eh I: keyboard mode "DUPL" [6.1.5.2]
	map(017, 017).r(FUNC(ie15_device::kb_s_lin_r));          // 0Fh I: keyboard mode "LIN" [6.1.5.2]
// simulation of flag registers
	map(020, 027).rw(FUNC(ie15_device::flag_r), FUNC(ie15_device::flag_w));
}

/* Input ports */
INPUT_PORTS_START( ie15 )
	PORT_START("io_keyboard")
	PORT_DIPNAME(ie15_keyboard_device::IE_KB_RED, ie15_keyboard_device::IE_KB_RED, "RED (Interpret controls)")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(ie15_keyboard_device::IE_KB_RED, "On")
	PORT_DIPNAME(ie15_keyboard_device::IE_KB_DUP, ie15_keyboard_device::IE_KB_DUP, "DUP (Full duplex)")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(ie15_keyboard_device::IE_KB_DUP, "On")
	PORT_DIPNAME(ie15_keyboard_device::IE_KB_LIN, ie15_keyboard_device::IE_KB_LIN, "LIN (Online)")
	PORT_DIPSETTING(0x00, "Off")
	PORT_DIPSETTING(ie15_keyboard_device::IE_KB_LIN, "On")

	// Until the UART is implemented
	PORT_START("RS232_RXBAUD")
	PORT_CONFNAME(0xffff, 9600, "RX Baud") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, ie15_device, update_serial)
	PORT_CONFSETTING(300, "300")
	PORT_CONFSETTING(600, "600")
	PORT_CONFSETTING(1200, "1200")
	PORT_CONFSETTING(2400, "2400")
	PORT_CONFSETTING(4800, "4800")
	PORT_CONFSETTING(9600, "9600")
	PORT_CONFSETTING(19200, "19200")

	PORT_START("RS232_TXBAUD")
	PORT_CONFNAME(0xffff, 9600, "TX Baud") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, ie15_device, update_serial)
	PORT_CONFSETTING(300, "300")
	PORT_CONFSETTING(600, "600")
	PORT_CONFSETTING(1200, "1200")
	PORT_CONFSETTING(2400, "2400")
	PORT_CONFSETTING(4800, "4800")
	PORT_CONFSETTING(9600, "9600")
	PORT_CONFSETTING(19200, "19200")

	PORT_START("RS232_DATABITS")
	PORT_CONFNAME(0xf, 8, "Data Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, ie15_device, update_serial)
	PORT_CONFSETTING(7, "7")
	PORT_CONFSETTING(8, "8")

	PORT_START("RS232_PARITY")
	PORT_CONFNAME(0x7, 0, "Parity") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, ie15_device, update_serial)
	PORT_CONFSETTING(0, "None")
	PORT_CONFSETTING(1, "Odd")
	PORT_CONFSETTING(2, "Even")
	PORT_CONFSETTING(3, "Mark")
	PORT_CONFSETTING(4, "Space")

	PORT_START("RS232_STOPBITS")
	PORT_CONFNAME(0x3, 1, "Stop Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, ie15_device, update_serial)
	PORT_CONFSETTING(1, "1")
	PORT_CONFSETTING(2, "2")

INPUT_PORTS_END

void ie15_device::kbd_put(uint16_t data)
{
	LOGKBD("keyboard data W %02X<-%02X '%c' %02X (%c)\n", m_kb_data, data, _PRINT(data), m_kb_flag, m_kb_flag ? 'n' : 'y');
	m_kb_control = (data >> 8) & 255;
	// send new key only when firmware has processed previous one
	if (m_kb_flag == IE_TRUE)
	{
		m_kb_data = data & 255;
		m_kb_flag = 0;
	}
}

WRITE_LINE_MEMBER( ie15_device::kbd_sdv )
{
	m_kbd_sdv = state;
}

void ie15_device::device_resolve_objects()
{
	m_rs232_conn_dtr_handler.resolve_safe();
	m_rs232_conn_rts_handler.resolve_safe();
	m_rs232_conn_txd_handler.resolve_safe();
}

void ie15_device::device_start()
{
	m_lat_led.resolve();
	m_nr_led.resolve();
	m_pch_led.resolve();
	m_dup_led.resolve();
	m_lin_led.resolve();
	m_red_led.resolve();
	m_sdv_led.resolve();
	m_prd_led.resolve();

	m_hblank_timer = timer_alloc(FUNC(ie15_device::hblank_onoff_tick), this);
	m_hblank_timer->adjust(attotime::never);

	m_beepoff_timer = timer_alloc(FUNC(ie15_device::ie15_beepoff), this);
	m_video.ptr1 = m_video.ptr2 = m_latch = 0;

	m_tmpbmp = std::make_unique<uint32_t[]>(IE15_TOTAL_HORZ * IE15_TOTAL_VERT);
}

void ie15_device::device_reset()
{
	update_serial(0);

	memset(&m_video, 0, sizeof(m_video));
	m_kb_ruslat = m_long_beep = m_kb_control = m_kb_data = m_kb_flag0 = 0;
	m_kb_flag = IE_TRUE;
	m_kbd_sdv = false;

	m_hblank = 1;
	m_hblank_timer->adjust(m_screen->time_until_pos(0, IE15_HORZ_START));
	m_vpos = m_screen->vpos();
	m_marker_scanline = (m_vpos % 11) > 7;

	m_beeper->set_state(0);

	m_serial_tx_ready = m_serial_rx_ready = IE_TRUE;
	set_data_frame(1 /* start bits */, 8 /* data bits */, PARITY_NONE, STOP_BITS_1);
	// device supports rates from 150 to 9600 baud but null_modem has hardcoded 9600
	set_rate(9600);
}

/*
    Usable raster is 800 x 275 pixels (80 x 25 characters).  24 lines are
    available to the user and 25th (topmost) line is the status line.
    Status line, if enabled, displays current serial port speed, 16 setup
    bits, and clock.  There is no NVRAM, so setup bits are always 0 after
    reset and clock starts counting at 0 XXX.

    No character attributes are available, but in 'display controls' mode
    control characters stored in memory are shown as blinking chars.

    Character cell is 10 x 11; character generator provides 7 x 8 of that.
    3 extra horizontal pixels are always blank.  Blinking cursor may be
    displayed on 3 extra scan lines.

    On each scan line, video board draws 80 characters from any location
    in video memory; this is used by firmware to provide instant scroll
    and cursor, which is a character with code 0x7F stored in off-screen
    memory.

    Video board output is controlled by
    - control flag 0 "disable video": 0 == disable
    - control flag 1 "cursor": 0 == if this scan line is one of extra 3,
      enable video every 5 frames.
    - control flag 3 "status line": 0 == current scan line is part of status line
    - keyboard mode 'RED' ('display controls'): if character code is
      less than 0x20 and RED is set, enable video every 5 frames; if RED is
      unset, disable video.
*/

void ie15_device::draw_scanline(uint32_t *p, uint16_t offset, uint8_t scanline)
{
	static const uint32_t palette[2] = { 0xff000000, 0xff00c000 };

	uint8_t ra = scanline % 8;
	uint32_t ra_high = 0x200 | ra;
	bool blink((m_screen->frame_number() % 10) > 4);
	bool red(m_io_keyboard->read() & ie15_keyboard_device::IE_KB_RED);
	bool blink_red_line25 = blink && red && m_video.line25;
	bool cursor_blank = scanline > 7 && (!m_video.cursor || blink);

	if (cursor_blank)
	{
		for (uint16_t x = 0; x < 80 * 10; x++)
		{
			*p++ = palette[0];
		}
	}
	else
	{
		for (uint16_t x = offset; x < offset + 80; x++)
		{
			uint16_t chr = m_p_videoram[x] << 3;
			uint8_t gfx = m_p_chargen[chr | ra];

			if (chr < (0x20 << 3))
			{
				if (blink_red_line25)
					gfx = m_p_chargen[chr | ra_high];
				else
					gfx = 0;
			}

			*p++ = palette[BIT(gfx, 7)];
			*p++ = palette[BIT(gfx, 6)];
			*p++ = palette[BIT(gfx, 5)];
			*p++ = palette[BIT(gfx, 4)];
			*p++ = palette[BIT(gfx, 3)];
			*p++ = palette[BIT(gfx, 2)];
			*p++ = palette[BIT(gfx, 1)];
			*p++ = palette[0];
			*p++ = palette[0];
			*p++ = palette[0];
		}
	}
}

void ie15_device::update_leds()
{
	uint8_t data = m_io_keyboard->read();

	m_lat_led = m_kb_ruslat ^ 1;
	m_nr_led = BIT(m_kb_control, ie15_keyboard_device::IE_KB_NR_BIT) ^ 1;
	m_pch_led = BIT(data, ie15_keyboard_device::IE_KB_PCH_BIT) ^ 1;
	m_dup_led = BIT(data, ie15_keyboard_device::IE_KB_DUP_BIT) ^ 1;
	m_lin_led = BIT(data, ie15_keyboard_device::IE_KB_LIN_BIT) ^ 1;
	m_red_led = BIT(data, ie15_keyboard_device::IE_KB_RED_BIT) ^ 1;
	m_sdv_led = m_kbd_sdv ^ 1;
	m_prd_led = 1; // XXX
}

/*
    VBlank is active for 3 topmost on-screen rows and 1 at the bottom.
    However, control flag 3 overrides VBlank, allowing status line
    to be switched on and off.
*/
void ie15_device::scanline_callback()
{
	int y = m_vpos;

	m_vpos++;
	m_vpos %= IE15_TOTAL_VERT;
	m_marker_scanline = (m_vpos % 11) > 7;

	LOGMASKED(LOG_SCANLINE,
		"addr %03x frame %d x %.4d y %.3d row %.2d e:c:s %d:%d:%d\n",
		m_video.ptr2, (int)m_screen->frame_number(), m_screen->hpos(), y,
		y%11, m_video.enable, m_video.cursor, m_video.line25);

	if (y < IE15_VERT_START) return;
	y -= IE15_VERT_START;
	if (y >= IE15_DISP_VERT) return;

	if (!m_video.enable || (y < IE15_STATUSLINE && m_video.line25))
	{
		memset(&m_tmpbmp[(y + IE15_VERT_START) * IE15_TOTAL_HORZ], 0, sizeof(uint32_t) * IE15_TOTAL_HORZ);
	}
	else
	{
		draw_scanline(&m_tmpbmp[(y + IE15_VERT_START) * IE15_TOTAL_HORZ + IE15_HORZ_START],
					  m_video.ptr2, y % 11);
	}
}

uint32_t ie15_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	update_leds();
	for (int y = 0; y < IE15_TOTAL_VERT; y++)
		std::copy_n(&m_tmpbmp[y * IE15_TOTAL_HORZ], IE15_TOTAL_HORZ, &bitmap.pix(y));
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

static GFXDECODE_START( gfx_ie15 )
	GFXDECODE_ENTRY("chargen", 0x0000, ie15_charlayout, 0, 1)
GFXDECODE_END

void ie15_device::ie15core(machine_config &config)
{
	/* Basic machine hardware */
	IE15_CPU(config, m_maincpu, XTAL(30'800'000) / 10);
	m_maincpu->set_addrmap(AS_PROGRAM, &ie15_device::ie15_mem);
	m_maincpu->set_addrmap(AS_IO, &ie15_device::ie15_io);

	config.set_default_layout(layout_ie15);

	/* Devices */
	IE15_KEYBOARD(config, m_keyboard, 0);
	m_keyboard->keyboard_cb().set(FUNC(ie15_device::kbd_put));
	m_keyboard->sdv_cb().set(FUNC(ie15_device::kbd_sdv));

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 2400);
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.15);
}

/* ROM definition */
ROM_START( ie15 )
	ROM_REGION(0x1000, "maincpu", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("5chip")
	ROM_SYSTEM_BIOS(0, "5chip", "5-chip firmware (newer)")
	ROMX_LOAD("dump1.bin", 0x0000, 0x1000, CRC(14b82284) SHA1(5ac4159fbb1c3b81445605e26cd97a713ae12b5f), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "6chip", "6-chip firmware (older)")
	ROMX_LOAD("dump5.bin", 0x0000, 0x1000, CRC(01f2e065) SHA1(2b72dc0594e38a528400cd25aed0c47e0c432895), ROM_BIOS(1))

	ROM_REGION(0x1000, "video", ROMREGION_ERASE00)

	ROM_REGION(0x0800, "chargen", ROMREGION_ERASE00)
	ROM_LOAD("chargen-15ie.bin", 0x0000, 0x0800, CRC(ed16bf6b) SHA1(6af9fb75f5375943d5c0ce9ed408e0fb4621b17e))
ROM_END

void ie15_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_screen_update(FUNC(ie15_device::screen_update));
	m_screen->set_raw(XTAL(30'800'000)/2,
			IE15_TOTAL_HORZ, IE15_HORZ_START, IE15_HORZ_START+IE15_DISP_HORZ,
			IE15_TOTAL_VERT, IE15_VERT_START, IE15_VERT_START+IE15_DISP_VERT);

	ie15core(config);

	GFXDECODE(config, "gfxdecode", "palette", gfx_ie15);
	PALETTE(config, "palette", palette_device::MONOCHROME);
}

ioport_constructor ie15_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ie15);
}

const tiny_rom_entry *ie15_device::device_rom_region() const
{
	return ROM_NAME(ie15);
}

DEFINE_DEVICE_TYPE(IE15, ie15_device, "ie15_device", "IE15")
