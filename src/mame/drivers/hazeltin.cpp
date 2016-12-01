// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Hazeltine 1500
    original machine (c) 1977 Hazeltine Corporation

    perliminary driver by Ryan Holtz

TODO:
    - pretty much everything

References:
    [1]: Hazeltine_1500_Series_Maintenance_Manual_Dec77.pdf, on Bitsavers

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/7400.h"
#include "machine/7404.h"
#include "machine/7474.h"
#include "machine/74161.h"
#include "machine/74175.h"
#include "machine/82s129.h"
#include "machine/am2847.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/com8116.h"
#include "machine/dm9334.h"
#include "machine/kb3600.h"

#define CPU_TAG         "maincpu"
#define UART_TAG        "uart"
#define BAUDGEN_TAG     "baudgen"
#define KBDC_TAG        "ay53600"
#define CHARRAM_TAG     "chrram"
#define CHARROM_TAG     "chargen"
#define BAUDPORT_TAG    "baud"
#define MISCPORT_TAG    "misc"
#define MISCKEYS_TAG    "misc_keys"
#define SCREEN_TAG      "screen"
#define TMS3409A_TAG    "u67"
#define TMS3409B_TAG    "u57"
#define DOTCLK_TAG      "dotclk"
#define DOTCLK_DISP_TAG "dotclk_dispatch"
#define CHAR_CTR_CLK_TAG "ch_bucket_ctr_clk"
#define U58_TAG         "u58"
#define U59_TAG         "u59"
#define VID_PROM_ADDR_RESET_TAG "u59_y5"
#define U61_TAG         "u61"
#define U68_TAG         "u68"
#define U69_PROMMSB_TAG "u69"
#define U70_PROMLSB_TAG "u70"
#define U70_TC_LINE_TAG "u70_tc"
#define U71_PROM_TAG    "u71"
#define U72_PROMDEC_TAG "u72"
#define U81_TAG         "u81"
#define U83_TAG         "u83"
#define U84_DIV11_TAG   "u84"
#define U85_VERT_DR_UB_TAG "u85"
#define U87_TAG         "u87"
#define U88_DIV9_TAG    "u88"
#define U90_DIV14_TAG   "u90"
#define BAUD_PROM_TAG   "u39"

// Number of cycles to burn when fetching the next row of characters into the line buffer:
// CPU clock is 18MHz / 9
// Dot clock is 33.264MHz / 2
// 9 dots per character
// 80 visible characters per line
// Total duration of fetch: 1440 33.264MHz clock cycles
//
//     2*9*80                    1         1440 * XTAL_2MHz
// -------------- divided by ---------  =  ----------------  =  86.5 main CPU cycles per line fetch
// XTAL_33_264MHz            XTAL_2MHz      XTAL_33_264MHz
#define LINE_FETCH_CYCLES   (87)

#define SR2_FULL_DUPLEX (0x01)
#define SR2_UPPER_ONLY  (0x08)

#define SR3_PB_RESET    (0x04)

#define KBD_STATUS_KBDR     (0x01)
#define KBD_STATUS_TV_UB    (0x40)
#define KBD_STATUS_TV_INT   (0x80)

#define SCREEN_HTOTAL   (9*100)
#define SCREEN_HDISP    (9*80)
#define SCREEN_HSTART   (9*5)

#define SCREEN_VTOTAL   (28*11)
#define SCREEN_VDISP    (24*11)
#define SCREEN_VSTART   (0)

#define VERT_UB_LINE    (24*11+8)

class hazl1500_state : public driver_device
{
public:
	hazl1500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, CPU_TAG)
		, m_uart(*this, UART_TAG)
		, m_kbdc(*this, KBDC_TAG)
		, m_baud_dips(*this, BAUDPORT_TAG)
		, m_baud_prom(*this, BAUD_PROM_TAG)
		, m_misc_dips(*this, MISCPORT_TAG)
		, m_kbd_misc_keys(*this, MISCKEYS_TAG)
		, m_char_ram(*this, CHARRAM_TAG)
		, m_char_rom(*this, CHARROM_TAG)
		, m_line_buffer_lsb(*this, TMS3409A_TAG)
		, m_line_buffer_msb(*this, TMS3409B_TAG)
		, m_dotclk(*this, DOTCLK_TAG)
		, m_vid_prom_msb(*this, U69_PROMMSB_TAG)
		, m_vid_prom_lsb(*this, U70_PROMLSB_TAG)
		, m_vid_prom(*this, U71_PROM_TAG)
		, m_u59(*this, U59_TAG)
		, m_u83(*this, U83_TAG)
		, m_char_y(*this, U84_DIV11_TAG)
		, m_char_x(*this, U88_DIV9_TAG)
		, m_vid_div14(*this, U90_DIV14_TAG)
		, m_vid_decode(*this, U72_PROMDEC_TAG)
		, m_u58(*this, U58_TAG)
		, m_u68(*this, U68_TAG)
		, m_u81(*this, U81_TAG)
		, m_u87(*this, U87_TAG)
		, m_u61(*this, U61_TAG)
		, m_screen(*this, SCREEN_TAG)
		, m_hblank_timer(nullptr)
		, m_scanline_timer(nullptr)
		, m_status_reg_3(0)
		, m_kbd_status_latch(0)
		, m_refresh_address(0)
		, m_vpos(0)
		, m_hblank(false)
		, m_vblank(false)
		, m_delayed_vblank(false)
	{
	}

	//m_maincpu->adjust_icount(-14);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update_hazl1500(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(com5016_fr_w);

	DECLARE_READ8_MEMBER(system_test_r); // noted as "for use with auto test equip" in flowchart on pg. 30, ref[1], jumps to 0x8000 if bit 0 is unset
	DECLARE_READ8_MEMBER(status_reg_2_r);
	DECLARE_WRITE8_MEMBER(status_reg_3_w);

	DECLARE_READ8_MEMBER(uart_r);
	DECLARE_WRITE8_MEMBER(uart_w);

	DECLARE_READ8_MEMBER(kbd_status_latch_r);
	DECLARE_READ8_MEMBER(kbd_encoder_r);
	DECLARE_READ_LINE_MEMBER(ay3600_shift_r);
	DECLARE_READ_LINE_MEMBER(ay3600_control_r);
	DECLARE_WRITE_LINE_MEMBER(ay3600_data_ready_w);

	DECLARE_WRITE8_MEMBER(refresh_address_w);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static const device_timer_id TIMER_HBLANK = 0;
	static const device_timer_id TIMER_SCANLINE = 1;

private:
	void check_tv_interrupt();
	void update_tv_unblank();
	void scanline_tick();
	void draw_scanline(uint32_t *pix);

	required_device<cpu_device> m_maincpu;
	required_device<ay31015_device> m_uart;
	required_device<ay3600_device> m_kbdc;
	required_ioport m_baud_dips;
	required_region_ptr<uint8_t> m_baud_prom;
	required_ioport m_misc_dips;
	required_ioport m_kbd_misc_keys;

	required_shared_ptr<uint8_t> m_char_ram;
	required_region_ptr<uint8_t> m_char_rom;
	required_device<tms3409_device> m_line_buffer_lsb;
	required_device<tms3409_device> m_line_buffer_msb;
	required_device<clock_device> m_dotclk;
	required_device<ttl74161_device> m_vid_prom_msb;
	required_device<ttl74161_device> m_vid_prom_lsb;
	required_device<prom82s129_device> m_vid_prom;
	required_device<ttl7404_device> m_u59;
	required_device<ttl7400_device> m_u83;
	required_device<ttl74161_device> m_char_y;
	required_device<ttl74161_device> m_char_x;
	required_device<ttl74161_device> m_vid_div14;
	required_device<dm9334_device> m_vid_decode;
	required_device<ttl74175_device> m_u58;
	required_device<ttl74175_device> m_u68;
	required_device<ttl74175_device> m_u81;
	required_device<ttl7404_device> m_u87;
	required_device<ttl7404_device> m_u61;

	required_device<screen_device> m_screen;

	std::unique_ptr<uint32_t[]> m_screen_pixbuf;

	emu_timer *m_hblank_timer;
	emu_timer *m_scanline_timer;

	uint8_t m_status_reg_3;
	uint8_t m_kbd_status_latch;

	uint8_t m_refresh_address;
	uint16_t m_vpos;
	bool m_hblank;
	bool m_vblank;
	bool m_delayed_vblank;
};

void hazl1500_state::machine_start()
{
	m_hblank_timer = timer_alloc(TIMER_HBLANK);
	m_hblank_timer->adjust(attotime::never);

	m_scanline_timer = timer_alloc(TIMER_SCANLINE);
	m_scanline_timer->adjust(attotime::never);

	m_screen_pixbuf = std::make_unique<uint32_t[]>(SCREEN_HTOTAL * SCREEN_VTOTAL);

	save_item(NAME(m_status_reg_3));
	save_item(NAME(m_kbd_status_latch));
	save_item(NAME(m_refresh_address));
	save_item(NAME(m_vpos));
	save_item(NAME(m_hblank));
	save_item(NAME(m_vblank));
	save_item(NAME(m_delayed_vblank));
}

void hazl1500_state::machine_reset()
{
	m_status_reg_3 = 0;
	m_kbd_status_latch = 0;

	m_refresh_address = 0;
	m_screen->reset_origin(0, 0);
	m_vpos = m_screen->vpos();
	m_vblank = (m_vpos >= SCREEN_VDISP);
	m_delayed_vblank = m_vpos < VERT_UB_LINE;
	if (!m_vblank)
		m_kbd_status_latch |= KBD_STATUS_TV_UB;
	m_hblank = true;
	m_hblank_timer->adjust(m_screen->time_until_pos(m_vpos, SCREEN_HSTART));
	m_scanline_timer->adjust(m_screen->time_until_pos(m_vpos + 1, 0));

	m_vid_prom_lsb->p_w(generic_space(), 0, 0);
	m_vid_prom_msb->p_w(generic_space(), 0, 0);
}


WRITE_LINE_MEMBER( hazl1500_state::com5016_fr_w )
{
	m_uart->rx_process();
	m_uart->tx_process();
}

uint32_t hazl1500_state::screen_update_hazl1500(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	memcpy(&bitmap.pix32(0), &m_screen_pixbuf[0], sizeof(uint32_t) * SCREEN_HTOTAL * SCREEN_VTOTAL);
	return 0;
}

READ8_MEMBER( hazl1500_state::system_test_r )
{
	return 0xff;
}

READ8_MEMBER( hazl1500_state::status_reg_2_r )
{
	uint8_t misc_dips = m_misc_dips->read();
	uint8_t status = 0;

	if (misc_dips & 0x10)
		status |= SR2_FULL_DUPLEX;
	if (misc_dips & 0x40)
		status |= SR2_UPPER_ONLY;

	return status ^ 0xff;
}

WRITE8_MEMBER( hazl1500_state::status_reg_3_w )
{
	m_status_reg_3 = data;
}

READ8_MEMBER( hazl1500_state::uart_r )
{
	return m_uart->get_received_data();
}

WRITE8_MEMBER( hazl1500_state::uart_w )
{
	m_uart->set_transmit_data(data);
}

READ8_MEMBER( hazl1500_state::kbd_status_latch_r )
{
	//printf("m_kbd_status_latch r: %02x\n", m_kbd_status_latch);
	return m_kbd_status_latch;
}

READ8_MEMBER(hazl1500_state::kbd_encoder_r)
{
	return m_kbdc->b_r() & 0xff; // TODO: This should go through an 8048, but we have no dump of it currently.
}

READ_LINE_MEMBER(hazl1500_state::ay3600_shift_r)
{
	// either shift key
	if (m_kbd_misc_keys->read() & 0x06)
	{
		return 1;
	}

	return 0;
}

READ_LINE_MEMBER(hazl1500_state::ay3600_control_r)
{
	if (m_kbd_misc_keys->read() & 0x08)
	{
		return 1;
	}

	return 0;
}

WRITE_LINE_MEMBER(hazl1500_state::ay3600_data_ready_w)
{
	if (state)
		m_kbd_status_latch |= KBD_STATUS_KBDR;
	else
		m_kbd_status_latch &= ~KBD_STATUS_KBDR;
}

void hazl1500_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_HBLANK:
			if (m_hblank)
			{
				m_hblank_timer->adjust(m_screen->time_until_pos(m_vpos, SCREEN_HSTART + SCREEN_HDISP));
			}
			else
			{
				m_hblank_timer->adjust(m_screen->time_until_pos((m_vpos + 1) % SCREEN_VTOTAL, SCREEN_HSTART));
			}
			m_hblank ^= 1;
			break;

		case TIMER_SCANLINE:
		{
			scanline_tick();
			break;
		}
	}
}

WRITE8_MEMBER(hazl1500_state::refresh_address_w)
{
	m_refresh_address = data;
	//printf("m_refresh_address %x, vpos %d, screen vpos %d\n", m_refresh_address, m_vpos, m_screen->vpos());
}

void hazl1500_state::check_tv_interrupt()
{
	uint8_t char_row = m_vpos % 11;
	bool bit_match = char_row == 2 || char_row == 3;
	bool tv_interrupt = bit_match && !m_delayed_vblank;
	//printf("interrupt for line %d (%d): %s\n", m_vpos, char_row, tv_interrupt ? "yes" : "no");

	m_kbd_status_latch &= ~KBD_STATUS_TV_INT;
	m_kbd_status_latch |= tv_interrupt ? KBD_STATUS_TV_INT : 0;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, tv_interrupt ? ASSERT_LINE : CLEAR_LINE);
}

void hazl1500_state::update_tv_unblank()
{
	if (!m_vblank)
	{
		m_kbd_status_latch |= KBD_STATUS_TV_UB;
	}
	else
	{
		m_kbd_status_latch &= ~KBD_STATUS_TV_UB;
	}
}

void hazl1500_state::scanline_tick()
{
	uint16_t old_vpos = m_vpos;
	m_vpos = (m_vpos + 1) % SCREEN_VTOTAL;
	m_vblank = (m_vpos >= SCREEN_VDISP);
	m_delayed_vblank = m_vpos >= VERT_UB_LINE;

	check_tv_interrupt();
	update_tv_unblank();

	draw_scanline(&m_screen_pixbuf[old_vpos * SCREEN_HTOTAL + SCREEN_HSTART]);

	m_scanline_timer->adjust(m_screen->time_until_pos((m_vpos + 1) % SCREEN_VTOTAL, 0));
}

void hazl1500_state::draw_scanline(uint32_t *pix)
{
	static const uint32_t palette[4] = { 0xff000000, 0xff006000, 0xff000000, 0xff00c000 };

	uint16_t ram_offset = m_refresh_address << 4;
	uint8_t char_row = m_vpos % 11;
	uint8_t recycle = (char_row != 10 ? 0xff : 0x00);
	m_line_buffer_lsb->rc_w(recycle & 0xf);
	m_line_buffer_msb->rc_w(recycle >> 4);

	if (recycle == 0)
		m_maincpu->adjust_icount(-LINE_FETCH_CYCLES);

	for (uint16_t x = 0; x < 80; x++)
	{
		uint8_t in = 0;
		if (!m_vblank)
			in = m_char_ram[ram_offset + x];

		m_line_buffer_lsb->in_w(in & 0xf);
		m_line_buffer_lsb->cp_w(1);
		m_line_buffer_lsb->cp_w(0);

		m_line_buffer_msb->in_w(in >> 4);
		m_line_buffer_msb->cp_w(1);
		m_line_buffer_msb->cp_w(0);

		const uint8_t chr = (m_line_buffer_msb->out_r() << 4) | m_line_buffer_lsb->out_r();
		const uint16_t chr_addr = (chr & 0x7f) << 4;
		const uint8_t gfx = m_char_rom[chr_addr | char_row];
		const uint8_t bright = (chr & 0x80) >> 6;

		*pix++ = palette[0];
		*pix++ = palette[BIT(gfx, 6) | bright];
		*pix++ = palette[BIT(gfx, 5) | bright];
		*pix++ = palette[BIT(gfx, 4) | bright];
		*pix++ = palette[BIT(gfx, 3) | bright];
		*pix++ = palette[BIT(gfx, 2) | bright];
		*pix++ = palette[BIT(gfx, 1) | bright];
		*pix++ = palette[BIT(gfx, 0) | bright];
		*pix++ = palette[0];
	}
}

static ADDRESS_MAP_START(hazl1500_mem, AS_PROGRAM, 8, hazl1500_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x3000, 0x377f) AM_RAM AM_SHARE(CHARRAM_TAG)
	AM_RANGE(0x3780, 0x37ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(hazl1500_io, AS_IO, 8, hazl1500_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x7f, 0x7f) AM_READWRITE(status_reg_2_r, status_reg_3_w)
	AM_RANGE(0xbf, 0xbf) AM_READWRITE(uart_r, uart_w)
	AM_RANGE(0xdf, 0xdf) AM_READ(kbd_encoder_r)
	AM_RANGE(0xef, 0xef) AM_READWRITE(system_test_r, refresh_address_w)
	AM_RANGE(0xf7, 0xf7) AM_READ(kbd_status_latch_r)
ADDRESS_MAP_END

	/*
	  Hazeltine 1500 key matrix (from ref[1])

	      | Y0  | Y1  | Y2  | Y3  | Y4  | Y5  | Y6  | Y7  | Y8  | Y9  |
	      |     |     |     |     |     |     |     |     |     |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X1  | TAB |  1  |  3  |  5  |  7  |BREAK| ^~  |     |     |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X2  |BAKSP|  2  |  4  |  6  |  8  |  9  | -=  |     |     |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X3  |     | ESC |  W  |  R  |  Y  |  0  | `@  | CLR |NP , |NP 9 |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X4  |     |  Q  |  E  |  T  |  U  |  O  | {[  | |\  |NP 7 |NP 6 |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X5  |HOME |  A  |  D  |  G  |  I  |  P  | *:  | LF  |NP 8 |NP 5 |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X6  |     |  S  |  F  |  H  |  J  | +;  | }]  | DEL |NP 4 |NP 2 |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X7  |     |  Z  |  C  |  B  |  K  |  L  | <,  | CR  |NP 1 |NP 3 |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X8  |     |  X  |  V  |  N  |SPACE|  M  | >.  | ?/  |NP 0 |NP . |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	*/

static INPUT_PORTS_START( hazl1500 )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	// X1  | TAB |  1  |  3  |  5  |  7  |BREAK| ^~  |     |     |     |
	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t') PORT_NAME("Tab")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)  PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE) PORT_NAME("Break")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	// X2  |BAKSP|  2  |  4  |  6  |  8  |  9  | -=  |     |     |     |
	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_NAME("Backspace")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	// X3  |     | ESC |  W  |  R  |  Y  |  0  | `@  | CLR |NP , |NP 9 |
	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_NAME("Esc")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_NAME("Clr")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))

	// X4  |     |  Q  |  E  |  T  |  U  |  O  | {[  | |\  |NP 7 |NP 6 |
	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))

	// X5  |HOME |  A  |  D  |  G  |  I  |  P  | *:  | LF  |NP 8 |NP 5 |
	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)   PORT_NAME("Home")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)      PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)      PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)      PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)      PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)      PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)   PORT_NAME("Line Feed")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)  PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)  PORT_CHAR(UCHAR_MAMEKEY(5_PAD))

	// X6  |     |  S  |  F  |  H  |  J  | +;  | }]  | DEL |NP 4 |NP 2 |
	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)      PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)      PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)      PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)      PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)  PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)    PORT_NAME("Del")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)  PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)  PORT_CHAR(UCHAR_MAMEKEY(2_PAD))

	// X7  |     |  Z  |  C  |  B  |  K  |  L  | <,  | CR  |NP 1 |NP 3 |
	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)      PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)      PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)      PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)      PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)      PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)  PORT_CHAR(13)  PORT_NAME("Return")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)  PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)  PORT_CHAR(UCHAR_MAMEKEY(3_PAD))

	// X8  |     |  X  |  V  |  N  |SPACE|  M  | >.  | ?/  |NP 0 |NP . |
	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)      PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)      PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)      PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)      PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)  PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_START(MISCKEYS_TAG)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START(BAUDPORT_TAG)
	PORT_DIPNAME( 0xff, 0x08, "Baud Rate" )
	PORT_DIPSETTING( 0x01, "110" )
	PORT_DIPSETTING( 0x02, "300" )
	PORT_DIPSETTING( 0x04, "1200" )
	PORT_DIPSETTING( 0x08, "1800" )
	PORT_DIPSETTING( 0x10, "2400" )
	PORT_DIPSETTING( 0x20, "4800" )
	PORT_DIPSETTING( 0x40, "9600" )
	PORT_DIPSETTING( 0x80, "19.2K" )

	PORT_START(MISCPORT_TAG)
	PORT_DIPNAME( 0x0f, 0x01, "Parity" )
	PORT_DIPSETTING( 0x01, "Even" )
	PORT_DIPSETTING( 0x02, "Odd" )
	PORT_DIPSETTING( 0x04, "1" )
	PORT_DIPSETTING( 0x08, "0" )
	PORT_DIPNAME( 0x10, 0x10, "Duplex" )
	PORT_DIPSETTING( 0x00, "Half" )
	PORT_DIPSETTING( 0x10, "Full" )
	PORT_DIPNAME( 0x20, 0x20, "Auto" )
	PORT_DIPSETTING( 0x00, "LF" )
	PORT_DIPSETTING( 0x20, "CR" )
	PORT_DIPNAME( 0x40, 0x40, "Case" )
	PORT_DIPSETTING( 0x00, "Upper and Lower" )
	PORT_DIPSETTING( 0x40, "Upper" )
	PORT_DIPNAME( 0x80, 0x80, "Video" )
	PORT_DIPSETTING( 0x00, "Standard" )
	PORT_DIPSETTING( 0x80, "Reverse" )
INPUT_PORTS_END

/* F4 Character Displayer */
static const gfx_layout hazl1500_charlayout =
{
	8, 16,
	128,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

static GFXDECODE_START( hazl1500 )
	GFXDECODE_ENTRY( "chargen", 0x0000, hazl1500_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( hazl1500, hazl1500_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(CPU_TAG, I8080, XTAL_18MHz/9) // 18MHz crystal on schematics, using an i8224 clock gen/driver IC
	MCFG_CPU_PROGRAM_MAP(hazl1500_mem)
	MCFG_CPU_IO_MAP(hazl1500_io)
	MCFG_QUANTUM_PERFECT_CPU(CPU_TAG)

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, RASTER, rgb_t::green())
	MCFG_SCREEN_UPDATE_DRIVER(hazl1500_state, screen_update_hazl1500)
	MCFG_SCREEN_RAW_PARAMS(XTAL_33_264MHz/2,
		SCREEN_HTOTAL, SCREEN_HSTART, SCREEN_HSTART + SCREEN_HDISP,
		SCREEN_VTOTAL, SCREEN_VSTART, SCREEN_VSTART + SCREEN_VDISP);

	MCFG_PALETTE_ADD_MONOCHROME("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hazl1500)

	MCFG_DEVICE_ADD(BAUDGEN_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(WRITELINE(hazl1500_state, com5016_fr_w))

	MCFG_DEVICE_ADD(UART_TAG, AY51013, 0)

	MCFG_TMS3409_ADD(TMS3409A_TAG)
	MCFG_TMS3409_ADD(TMS3409B_TAG)

	MCFG_CLOCK_ADD(DOTCLK_TAG, XTAL_33_264MHz/2)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE(DOTCLK_DISP_TAG, devcb_line_dispatch_device<2>, in_w))

	MCFG_LINE_DISPATCH_ADD(DOTCLK_DISP_TAG, 2)
	MCFG_LINE_DISPATCH_FWD_CB(0, 2, DEVWRITELINE(U81_TAG, ttl74175_device, clock_w))
	MCFG_LINE_DISPATCH_FWD_CB(1, 2, DEVWRITELINE(U88_DIV9_TAG, ttl74161_device, clock_w))

	MCFG_74161_ADD(U70_PROMLSB_TAG)
	MCFG_7416x_QA_CB(DEVWRITELINE(U71_PROM_TAG, prom82s129_device, a0_w))
	MCFG_7416x_QB_CB(DEVWRITELINE(U71_PROM_TAG, prom82s129_device, a1_w))
	MCFG_7416x_QC_CB(DEVWRITELINE(U71_PROM_TAG, prom82s129_device, a2_w))
	MCFG_7416x_QD_CB(DEVWRITELINE(U71_PROM_TAG, prom82s129_device, a3_w))
	MCFG_7416x_TC_CB(DEVWRITELINE(U70_TC_LINE_TAG, devcb_line_dispatch_device<2>, in_w))

	MCFG_LINE_DISPATCH_ADD(U70_TC_LINE_TAG, 2)
	MCFG_LINE_DISPATCH_FWD_CB(0, 2, DEVWRITELINE(U69_PROMMSB_TAG, ttl74161_device, cet_w))
	MCFG_LINE_DISPATCH_FWD_CB(1, 2, DEVWRITELINE(U69_PROMMSB_TAG, ttl74161_device, cep_w))

	MCFG_74161_ADD(U69_PROMMSB_TAG)
	MCFG_7416x_QA_CB(DEVWRITELINE(U71_PROM_TAG, prom82s129_device, a4_w))
	MCFG_7416x_QB_CB(DEVWRITELINE(U71_PROM_TAG, prom82s129_device, a5_w))
	MCFG_7416x_QC_CB(DEVWRITELINE(U71_PROM_TAG, prom82s129_device, a6_w))

	//MCFG_LINE_DISPATCH_ADD(CHAR_LINE_CNT_CLK_TAG, 3)
	//MCFG_LINE_DISPATCH_FWD_CB(0, 3, DEVWRITELINE(U85_VERT_DR_UB_TAG, ttl7473_device, clk1_w))
	//MCFG_LINE_DISPATCH_FWD_CB(1, 3, DEVWRITELINE(U85_VERT_DR_UB_TAG, ttl7473_device, clk2_w))
	//MCFG_LINE_DISPATCH_FWD_CB(2, 3, DEVWRITELINE(U84_DIV11_TAG, ttl74161_device, clock_w))

	MCFG_7400_ADD(U83_TAG)
	//MCFG_7400_Y1_CB(DEVWRITELINE(CHAR_LINE_CNT_CLK_TAG, devcb_line_dispatch_device<4>, in_w))

	MCFG_74161_ADD(U84_DIV11_TAG)
	MCFG_74161_ADD(U90_DIV14_TAG)

	MCFG_74161_ADD(U88_DIV9_TAG)
	MCFG_7416x_QC_CB(DEVWRITELINE(U81_TAG, ttl74175_device, d4_w))
	MCFG_7416x_TC_CB(DEVWRITELINE(U81_TAG, ttl74175_device, d1_w))

	MCFG_LINE_DISPATCH_ADD(CHAR_CTR_CLK_TAG, 2)
	MCFG_LINE_DISPATCH_FWD_CB(0, 2, DEVWRITELINE(U70_PROMLSB_TAG, ttl74161_device, clock_w))
	MCFG_LINE_DISPATCH_FWD_CB(1, 2, DEVWRITELINE(U69_PROMMSB_TAG, ttl74161_device, clock_w))

	MCFG_74175_ADD(U58_TAG)
	MCFG_74175_ADD(U68_TAG)
	MCFG_74175_ADD(U81_TAG)
	MCFG_74175_Q1_CB(DEVWRITELINE(U81_TAG, ttl74175_device, d2_w))
	MCFG_74175_NOT_Q2_CB(DEVWRITELINE(CHAR_CTR_CLK_TAG, devcb_line_dispatch_device<2>, in_w))

	MCFG_DM9334_ADD(U72_PROMDEC_TAG)
	MCFG_DM9334_Q4_CB(DEVWRITELINE(U83_TAG, ttl7400_device, b1_w))

	MCFG_82S129_ADD(U71_PROM_TAG)
	MCFG_82S129_O1_CB(DEVWRITELINE(U72_PROMDEC_TAG, dm9334_device, a0_w))
	MCFG_82S129_O2_CB(DEVWRITELINE(U72_PROMDEC_TAG, dm9334_device, a1_w))
	MCFG_82S129_O3_CB(DEVWRITELINE(U72_PROMDEC_TAG, dm9334_device, a2_w))
	MCFG_82S129_O4_CB(DEVWRITELINE(U72_PROMDEC_TAG, dm9334_device, d_w))

	MCFG_7404_ADD(U61_TAG)
	MCFG_7404_ADD(U87_TAG)
	MCFG_7404_ADD(U59_TAG)
	MCFG_7404_Y5_CB(DEVWRITELINE(VID_PROM_ADDR_RESET_TAG, devcb_line_dispatch_device<2>, in_w))

	MCFG_LINE_DISPATCH_ADD(VID_PROM_ADDR_RESET_TAG, 2)
	MCFG_LINE_DISPATCH_FWD_CB(0, 2, DEVWRITELINE(U70_PROMLSB_TAG, ttl74161_device, pe_w))
	MCFG_LINE_DISPATCH_FWD_CB(1, 2, DEVWRITELINE(U69_PROMMSB_TAG, ttl74161_device, pe_w))

	/* keyboard controller */
	MCFG_DEVICE_ADD(KBDC_TAG, AY3600, 0)
	MCFG_AY3600_MATRIX_X0(IOPORT("X0"))
	MCFG_AY3600_MATRIX_X1(IOPORT("X1"))
	MCFG_AY3600_MATRIX_X2(IOPORT("X2"))
	MCFG_AY3600_MATRIX_X3(IOPORT("X3"))
	MCFG_AY3600_MATRIX_X4(IOPORT("X4"))
	MCFG_AY3600_MATRIX_X5(IOPORT("X5"))
	MCFG_AY3600_MATRIX_X6(IOPORT("X6"))
	MCFG_AY3600_MATRIX_X7(IOPORT("X7"))
	MCFG_AY3600_MATRIX_X8(IOPORT("X8"))
	MCFG_AY3600_SHIFT_CB(READLINE(hazl1500_state, ay3600_shift_r))
	MCFG_AY3600_CONTROL_CB(READLINE(hazl1500_state, ay3600_control_r))
	MCFG_AY3600_DATA_READY_CB(WRITELINE(hazl1500_state, ay3600_data_ready_w))
MACHINE_CONFIG_END


ROM_START( hazl1500 )
	ROM_REGION( 0x10000, CPU_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "h15s-00I-10-3.bin", 0x0000, 0x0800, CRC(a2015f72) SHA1(357cde517c3dcf693de580881add058c7b26dfaa))

	ROM_REGION( 0x800, CHARROM_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "u83_chr.bin", 0x0000, 0x0800, CRC(e0c6b734) SHA1(7c42947235c66c41059fd4384e09f4f3a17c9857))

	ROM_REGION( 0x100, BAUD_PROM_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "u43_702129_82s129.bin", 0x0000, 0x0100, CRC(b35aea2b) SHA1(4702620cdef72b32a397580c22b75df36e24ac74))

	ROM_REGION( 0x100, U71_PROM_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "u90_702128_82s129.bin", 0x0000, 0x0100, CRC(277bc424) SHA1(528a0de3b54d159bc14411961961706bf9ec41bf))
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT    COMPAT   MACHINE   INPUT     CLASS          INIT    COMPANY                     FULLNAME            FLAGS */
COMP( 1977, hazl1500, 0,        0,       hazl1500, hazl1500, driver_device, 0,      "Hazeltine Corporation",    "Hazeltine 1500",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
