// license:BSD-3-Clause
// copyright-holders:Golden Child
#include "emu.h"
#include "etibuffer.h"
#include "etibuffer.lh"
#include <cstdlib>

#define ETI_CENTRONICS_TAG "prn"
#define SCREEN_WIDTH  16
#define SCREEN_HEIGHT 384

#define STROBE_DELAY 1  // in usec
#define ACK_DELAY 1     // in usec

//#define DEBUG_BUFFER_DISPLAY

ROM_START( etiprintbuffer_device )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "etibuffer.rom",  0x0000, 0x800, CRC(bd31d7b6) SHA1(cd76a9a53c6b9994c5721f8c393bc782143c6d3f))
ROM_END

static INPUT_PORTS_START ( etiprintbuffer_device )
	PORT_START("TEST")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Test Buffer") PORT_CODE(KEYCODE_2_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, etiprintbuffer_device, test_sw, 0)
	PORT_START("CLEAR")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clear Buffer") PORT_CODE(KEYCODE_5_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, etiprintbuffer_device, clear_sw, 0)

	PORT_START("CONFIG")
#ifdef DEBUG_BUFFER_DISPLAY
	PORT_CONFNAME(0x03, 0x01, "ETI Buffer Debug Display")
	PORT_CONFSETTING(0x00, "Off")
	PORT_CONFSETTING(0x01, "LEDs")
	PORT_CONFSETTING(0x02, "Buffer Head/Tail")
#endif
	PORT_CONFNAME(0x0c, 0x0c, "Ram Size")
	PORT_CONFSETTING(0x00, "0K (will not work, passes FF to printer)")  // ram size of 0k will pass only 00 on to printer, use for ram test failure
	PORT_CONFSETTING(0x04, "16K")
	PORT_CONFSETTING(0x08, "32K")
	PORT_CONFSETTING(0x0c, "48K")

INPUT_PORTS_END

DEFINE_DEVICE_TYPE(ETIPRINTBUFFER, etiprintbuffer_device, "etiprintbuffer", "Electronics Today International Print Buffer 48K")

//--------------------------------------------------
//  device constructor
//--------------------------------------------------


etiprintbuffer_device::etiprintbuffer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	etiprintbuffer_device(mconfig, ETIPRINTBUFFER, tag, owner, clock)
{
}

etiprintbuffer_device::etiprintbuffer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_centronics_peripheral_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_screen(*this, "screen")
	, m_ctx(*this, ETI_CENTRONICS_TAG)
	, m_ctx_data_out(*this, "ctx_data_out")
	, m_printerready_led(*this, "printerready_led")
	, m_bufferready_led(*this, "bufferready_led")
{
}

//--------------------------------------------------
//  mem map
//--------------------------------------------------

void etiprintbuffer_device::etiprintbuffer_device_memmap(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7ff).rom().region("maincpu", 0);
	map(0x1000, 0x1fff).r(FUNC(etiprintbuffer_device::eti_read_1000));
	map(0x2000, 0x2fff).w(FUNC(etiprintbuffer_device::eti_write_2000));
	map(0x3000, 0x3fff).w(FUNC(etiprintbuffer_device::eti_write_3000));
}

//--------------------------------------------------
//  io map
//--------------------------------------------------

void etiprintbuffer_device::etiprintbuffer_device_iomap(address_map &map)
{
	map.unmap_value_high();
	map(0x0, 0xffff).r(FUNC(etiprintbuffer_device::eti_status_r));
}

//--------------------------------------------------
//  memory and io functions
//--------------------------------------------------

uint8_t etiprintbuffer_device::eti_status_r(offs_t offset)
{
	return ((m_strobereceived ? 1 : 0 ) << 6) | // d6 is m_strobe status
			((m_busy          ? 0 : 1 ) << 7);  // d7 is not busy output
}

uint8_t etiprintbuffer_device::eti_read_1000(offs_t offset)
{
	output_ack(0);
	m_strobereceived = false;
	m_bufferready_led = !m_strobereceived;

	output_busy(m_strobereceived);
	m_ack_timer->adjust(attotime::from_usec(ACK_DELAY), TIMER_ACK);
	return m_datalatch;
}

void etiprintbuffer_device::eti_write_2000(offs_t offset, uint8_t data)
{
	m_ctx_data_out->write(data);
}

void etiprintbuffer_device::eti_write_3000(offs_t offset, uint8_t data)
{
	m_ctx->write_strobe(CLEAR_LINE);
	m_strobeout_timer->adjust(attotime::from_usec(STROBE_DELAY), TIMER_STROBEOUT);
}

//--------------------------------------------------
//  device init
//--------------------------------------------------


void etiprintbuffer_device::device_start()
{
	m_printerready_led.resolve();
	m_bufferready_led.resolve();
	m_ack_timer = timer_alloc(TIMER_ACK);
	m_strobeout_timer = timer_alloc(TIMER_STROBEOUT);

	save_item(NAME(m_ram));
	save_item(NAME(m_data));
	save_item(NAME(m_datalatch));
	save_item(NAME(m_strobereceived));
	save_item(NAME(m_strobe));
	save_item(NAME(m_busy));
}

void etiprintbuffer_device::device_reset()
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	u16 ram16kblocks = ioport("CONFIG")->read() >> 2;
	if (ram16kblocks) mem.install_ram(0x4000,0x4000+ram16kblocks*0x4000-1,m_ram);

	output_busy(0);
	output_fault(1);
	output_ack(1);
	output_select(1);
	output_perror(0);
	m_ctx->write_strobe(ASSERT_LINE);
}

//--------------------------------------------------
//  device mconfig
//--------------------------------------------------

void etiprintbuffer_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, 2_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &etiprintbuffer_device::etiprintbuffer_device_memmap);
	m_maincpu->set_addrmap(AS_IO,      &etiprintbuffer_device::etiprintbuffer_device_iomap);

	CENTRONICS(config, m_ctx, centronics_devices, "printer");
	m_ctx->busy_handler().set(FUNC(etiprintbuffer_device::busy_w));

	OUTPUT_LATCH(config, m_ctx_data_out);
	m_ctx->set_output_latch(*m_ctx_data_out);

#ifdef DEBUG_BUFFER_DISPLAY
	// video hardware
	screen_device &screen(SCREEN(config, m_screen, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(SCREEN_WIDTH, SCREEN_HEIGHT);
	screen.set_visarea(0, SCREEN_WIDTH-1, 0, SCREEN_HEIGHT-1);
	screen.set_screen_update(FUNC(etiprintbuffer_device::screen_update_etibuffer));
	screen.set_physical_aspect(1,40);
#endif

	config.set_default_layout(layout_etibuffer);
}

//--------------------------------------------------
//  tiny rom entry
//--------------------------------------------------


const tiny_rom_entry *etiprintbuffer_device::device_rom_region() const
{
	return ROM_NAME( etiprintbuffer_device );
}

//--------------------------------------------------
//  ioport
//--------------------------------------------------

ioport_constructor etiprintbuffer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( etiprintbuffer_device );
}

//--------------------------------------------------
//  INPUT_CHANGED_MEMBERS
//--------------------------------------------------

INPUT_CHANGED_MEMBER(etiprintbuffer_device::clear_sw)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(etiprintbuffer_device::test_sw)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, newval ? CLEAR_LINE : ASSERT_LINE);
}

//--------------------------------------------------
//  WRITE_LINE_MEMBERS
//--------------------------------------------------

WRITE_LINE_MEMBER( etiprintbuffer_device::busy_w )
{
	m_busy = (state == ASSERT_LINE) ? true : false;
	m_printerready_led = !m_busy;
}

WRITE_LINE_MEMBER( etiprintbuffer_device::input_strobe )
{
	if (m_strobe == true && state == false)
	{
		m_datalatch = m_data;
		m_strobereceived = true;
		output_busy(m_strobereceived);
		m_bufferready_led = !m_strobereceived;
	}
	m_strobe = state;
}

//--------------------------------------------------
//  TIMERS
//--------------------------------------------------

void etiprintbuffer_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_ACK:
		output_ack(1);
		output_busy(m_strobereceived);
		break;
	case TIMER_STROBEOUT:
		m_ctx->write_strobe(ASSERT_LINE);
	}
}

//**************************************************************************
//  FUNCTIONS FOR DEBUG DISPLAY
//**************************************************************************


void etiprintbuffer_device::drawbar(double xval1, double xval2, double x1, double x2, double y1, double y2, int width, bitmap_rgb32 &bitmap, u32 color)
{
#ifdef DEBUG_BUFFER_DISPLAY
	double pct1 = (xval1 - x1) / (x2-x1);
	double pct2 = (xval2 - x1) / (x2-x1);
	// clamp range to 0,1.0
	pct1 = std::min(std::max(0.0, pct1), 1.0);
	pct2 = std::min(std::max(0.0, pct2), 1.0);
	double yval1 = pct1 * (y2 - y1) + y1;
	double yval2 = pct2 * (y2 - y1) + y1;
	bitmap.plot_box(0, yval1, width, yval2-yval1+1, color);
#endif
}

void etiprintbuffer_device::draw7seg(u8 digit, int x0, int y0, int width, int height, int thick, bitmap_rgb32 &bitmap, u32 color)
{
#ifdef DEBUG_BUFFER_DISPLAY
	const u8 pat[] = { 0x3f, 0x06,  0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 };
	u8 seg = pat[digit & 0xf];

	if (BIT(seg,0)) bitmap.plot_box(x0,       y0,                  width, thick,  color);
	if (BIT(seg,1)) bitmap.plot_box(x0+width, y0+thick,            thick, height, color);
	if (BIT(seg,2)) bitmap.plot_box(x0+width, y0+2*thick+height,   thick, height, color);
	if (BIT(seg,3)) bitmap.plot_box(x0,       y0+2*thick+2*height, width, thick,  color);
	if (BIT(seg,4)) bitmap.plot_box(x0-thick, y0+2*thick+height,   thick, height, color);
	if (BIT(seg,5)) bitmap.plot_box(x0-thick, y0+thick,            thick, height, color);
	if (BIT(seg,6)) bitmap.plot_box(x0,       y0+thick+height,     width, thick,  color);
#endif
}

uint32_t etiprintbuffer_device::screen_update_etibuffer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
#ifdef DEBUG_BUFFER_DISPLAY
	u8 ledsize = 8;
	u32 ledoncolor  = 0xff0000;
	u32 ledoffcolor = 0x7f0000;
	u32 bufferheadcolor = 0x00ff00;
	u32 buffertailcolor = 0x00ffff;
	u32 datacolor  = 0x888888;
	u32 emptycolor = 0x000000;
	u32 fullcolor  = 0xbbbbdd;
	u32 barcolor;

	u8 display = ioport("CONFIG")->read() & 0x3;
	bitmap.plot_box(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,0x000000);  // clear screen

	if (display & 0x2) // display bars
	{
		u16 pc = m_maincpu->state_int(Z80_PC);
		if (!(pc >= 0x100 && pc <= 0x106) &&    // update if we are not initializing bc/de/hl to avoid flicker
			!(pc >= 0x11a && pc <= 0x11c) &&    // update if we are not in these ranges to avoid flicker because of EXX / EXX
			!(pc >= 0x134 && pc <= 0x137) &&
			!(pc >= 0x159 && pc <= 0x15b))
		{
			m_buffersize = m_maincpu->state_int(Z80_HL);    // HL is buffer size
			m_bufferhead = m_maincpu->state_int(Z80_BC);    // DE is buffer tail
			m_buffertail = m_maincpu->state_int(Z80_DE);    // BC is buffer head
		}

		drawbar(0, 1, 0, 1, ledsize, SCREEN_HEIGHT, 16, bitmap, emptycolor);

		barcolor = (m_buffersize > 0xbfc0 ? fullcolor : datacolor);
		bool nowrap = m_bufferhead >= m_buffertail;

		if (nowrap) // draw from tail to head
		{
			drawbar(m_buffertail, m_bufferhead, 0x4000, 0xffff, ledsize, SCREEN_HEIGHT, 16, bitmap, barcolor);
		}
		else // draw from tail to top of buffer (0xffff), then from bottom of buffer (0x4000) to head
		{
			drawbar(0x4000, m_bufferhead, 0x4000, 0xffff, ledsize, SCREEN_HEIGHT, 16, bitmap, barcolor);
			drawbar(m_buffertail, 0xffff, 0x4000, 0xffff, ledsize, SCREEN_HEIGHT, 16, bitmap, barcolor);
		}
		drawbar(m_buffertail, m_buffertail+256, 0x4000, 0xffff, ledsize, SCREEN_HEIGHT, 16, bitmap, buffertailcolor);
		drawbar(m_bufferhead, m_bufferhead+256, 0x4000, 0xffff, ledsize, SCREEN_HEIGHT, 16, bitmap, bufferheadcolor);

		// draw hex digits of buffersize
		u16 seg7w = 4;
		u16 seg7h = 3;
		u16 seg7t = 1;
		u32 seg7color = 0xff2222;

		draw7seg((m_buffersize & 0xf)    >> 0,  9,16, seg7w, seg7h, seg7t, bitmap, seg7color);
		draw7seg((m_buffersize & 0xf0)   >> 4,  2,16, seg7w, seg7h, seg7t, bitmap, seg7color);
		draw7seg((m_buffersize & 0xf00)  >> 8,  9,30, seg7w, seg7h, seg7t, bitmap, seg7color);
		draw7seg((m_buffersize & 0xf000) >> 12, 2,30, seg7w, seg7h, seg7t, bitmap, seg7color);
	}
	if (display) // draw leds
	{
		bool led1 = !m_busy;
		bool led2 = !m_strobereceived;

		bitmap.plot_box(0,         0, ledsize, ledsize, led1 ? ledoncolor : ledoffcolor);  // draw led1
		bitmap.plot_box(0+ledsize, 0, ledsize, ledsize, led2 ? ledoncolor : ledoffcolor);  // draw led2
	}
#endif
	return 0;
}

