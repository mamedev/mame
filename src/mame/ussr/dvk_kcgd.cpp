// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    KCGD (Kontroller Cvetnogo Graficheskogo Displeya = Colour Graphics
    Display Controller), a replacement for KSM (dvk_ksm.c) in later
    models of DVK desktops.

    MPI (Q-Bus clone) board. Interfaces with MS7004 (DEC LK201 workalike)
    keyboard, mouse, and monochrome or color CRT.  Host interface is a
    serial port; there is no direct framebuffer access.

    VRAM is 128K and is word-addressable, so address fits into 16 bits.
    Lower 32K of VRAM are not used to store pixel data.

    To do:
    - K1801VM2 CPU core (interrupts and EVNT pin, full EIS set, other insns)
    - verify hsync/vsync frequencies
    - mouse
    - mono/color CRT
    - KeyGP ROM

    Hardware notes:
    - PBA3.660.259TO manual (1987) does not document mouse interface
    - PBA3.660.259E3 schematic (KCGD_E3-PE3.djvu) has mouse interface
        but specifies ROM "181" in parts list?
    - MPSS journal article (1988) does document mouse interface
    - ROM "181" does not use rx interrupts, has no setup menu
    - ROM "182" uses? 500 hz timer, rx interrupts, interlace mode
    - add-on "KeyGP" ROM supports "save settings" -- to where?

****************************************************************************/

#include "emu.h"

#include "1801vp033.h"
#include "bus/generic/slot.h"
#include "bus/rs232/rs232.h"
#include "cpu/t11/t11.h"
#include "machine/dl11.h"
#include "machine/timer.h"
#include "ms7004.h"

#include "emupal.h"
#include "screen.h"


#define LOG_VRAM      (1U << 1)
#define LOG_DEBUG     (1U << 2)

//#define VERBOSE (LOG_DEBUG | LOG_VRAM)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGVRAM(...) LOGMASKED(LOG_VRAM, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


namespace {

// these are unverified
static constexpr int KCGD_TOTAL_HORZ = 977;
static constexpr int KCGD_DISP_HORZ = 800;
static constexpr int KCGD_HORZ_START = 0;

static constexpr int KCGD_TOTAL_VERT = 525;
static constexpr int KCGD_DISP_VERT = 480;
static constexpr int KCGD_VERT_START = 0;

static constexpr int KCGD_PAGE_0 = 015574;
static constexpr int KCGD_PAGE_1 = 005574;


class kcgd_state : public driver_device
{
public:
	kcgd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic(*this, "pic")
		, m_dl11host(*this, "dl11host")
		, m_rs232(*this, "rs232")
		, m_dl11kbd(*this, "dl11kbd")
		, m_ms7004(*this, "ms7004")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_buttons(*this, "mouse_buttons")
	{ }

	void kcgd(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(mouse_x_changed);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_y_changed);
	DECLARE_INPUT_CHANGED_MEMBER(buttons_changed);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(vsync_tick);
	TIMER_CALLBACK_MEMBER(toggle_500hz);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);
	void kcgd_palette(palette_device &palette) const;

	void reset_w(int state);

	uint16_t vram_addr_r();
	uint16_t vram_data_r();
	uint16_t vram_mmap_r(offs_t offset);
	void vram_addr_w(uint16_t data);
	void vram_data_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void vram_mmap_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void palette_control(offs_t offset, uint16_t data, uint16_t mem_mask);

	emu_timer *m_vsync_timer = nullptr;
	emu_timer *m_500hz_timer = nullptr;

	void kcgd_mem(address_map &map) ATTR_COLD;

	void draw_scanline(uint16_t *p, uint16_t offset);
	rectangle m_tmpclip;
	bitmap_ind16 m_tmpbmp;

	int m_page, m_interlace, m_palette_index, m_vram_addr, m_500hz;
	uint8_t m_video_control;
	uint8_t m_palette_data[16]{};

	std::unique_ptr<uint32_t[]> m_videoram;

	int8_t m_x, m_y, m_sr;

protected:
	required_device<k1801vm2_device> m_maincpu;
	required_device<k1801vp033_device> m_pic;
	required_device<k1801vp065_device> m_dl11host;
	required_device<rs232_port_device> m_rs232;
	required_device<k1801vp065_device> m_dl11kbd;
	required_device<ms7004_device> m_ms7004;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_ioport m_buttons;
};


INPUT_CHANGED_MEMBER(kcgd_state::mouse_x_changed)
{
	m_x += newval - oldval;
	logerror("X %3d->%3d d %3d m_x %3d\n", oldval, newval, newval-oldval, m_x);
}

INPUT_CHANGED_MEMBER(kcgd_state::mouse_y_changed)
{
	m_y += newval - oldval;
	logerror("Y %3d->%3d d %3d m_y %3d\n", oldval, newval, newval-oldval, m_y);
}

INPUT_CHANGED_MEMBER(kcgd_state::buttons_changed)
{
	m_sr = m_buttons->read();
}

INPUT_PORTS_START(kcgd)
	PORT_START("mouse_x")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kcgd_state::mouse_x_changed), 0)

	PORT_START("mouse_y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kcgd_state::mouse_y_changed), 0)

	PORT_START("mouse_buttons")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Mouse Button") PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kcgd_state::buttons_changed), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right Mouse Button") PORT_CODE(MOUSECODE_BUTTON2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(kcgd_state::buttons_changed), 0)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void kcgd_state::kcgd_mem(address_map &map)
{
	map.unmap_value_high();
	map(0000000, 0177777).lrw16(
		NAME([this](offs_t offset) { if (!machine().side_effects_disabled()) m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero); return 0; }),
		NAME([this](offs_t offset, u16 data) { m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero); }));

	map(0000000, 0077777).rw(FUNC(kcgd_state::vram_mmap_r), FUNC(kcgd_state::vram_mmap_w));
	map(0100000, 0117777).rom().region("maincpu", 0);
	// 1802VV1 chips
	map(0160000, 0160001).mirror(03774).rw(FUNC(kcgd_state::vram_addr_r), FUNC(kcgd_state::vram_addr_w));
	map(0160002, 0160003).mirror(03774).rw(FUNC(kcgd_state::vram_data_r), FUNC(kcgd_state::vram_data_w));
	map(0167770, 0167777).rw(m_pic, FUNC(k1801vp033_device::pic_read), FUNC(k1801vp033_device::pic_write));
	map(0176560, 0176567).rw(m_dl11host, FUNC(k1801vp065_device::read), FUNC(k1801vp065_device::write));
	map(0177560, 0177567).rw(m_dl11kbd, FUNC(k1801vp065_device::read), FUNC(k1801vp065_device::write));
}

static const z80_daisy_config daisy_chain[] =
{
	{ "pic" },
	{ "dl11kbd" },
	{ "dl11host" },
	{ nullptr }
};

static DEVICE_INPUT_DEFAULTS_START( host_rs232_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_57600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_57600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
	DEVICE_INPUT_DEFAULTS( "FLOW_CONTROL", 0x07, 0x01 )
DEVICE_INPUT_DEFAULTS_END


TIMER_CALLBACK_MEMBER(kcgd_state::vsync_tick)
{
	m_maincpu->pulse_input_line(t11_device::CP2_LINE, m_maincpu->minimum_quantum_time());
}

TIMER_CALLBACK_MEMBER(kcgd_state::toggle_500hz)
{
	m_500hz ^= 1;
	m_pic->pic_write_reqb(m_500hz);
}

void kcgd_state::machine_reset()
{
	m_page = m_interlace = m_palette_index = m_vram_addr = m_500hz = 0;
	m_video_control = 0;
	m_x = m_y = 0;
	memset(&m_palette_data, 0, sizeof(m_palette_data));
}

void kcgd_state::machine_start()
{
	// 64 kwords, word size is 17 bits
	m_videoram = std::make_unique<uint32_t[]>(65536);

	m_tmpclip = rectangle(0, KCGD_DISP_HORZ - 1, 0, KCGD_DISP_VERT - 1);
	m_tmpbmp.allocate(KCGD_DISP_HORZ, KCGD_DISP_VERT);
	m_vsync_timer = timer_alloc(FUNC(kcgd_state::vsync_tick), this);
	m_vsync_timer->adjust(m_screen->time_until_pos(0, 0), 0, m_screen->frame_period());
	if (system_bios() > 0)
	{
		m_500hz_timer = timer_alloc(FUNC(kcgd_state::toggle_500hz), this);
		m_500hz_timer->adjust(attotime::from_hz(500), 0, attotime::from_hz(500));
	}
}

void kcgd_state::reset_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_pic->reset();
		m_dl11kbd->reset();
		m_dl11host->reset();
	}
}

void kcgd_state::kcgd_palette(palette_device &palette) const
{
	// for debugging only -- palette is overwritten by firmware
	for (int i = 0; i < 16; i++)
		palette.set_pen_color(i, i ? i : 255, i ? i : 255, i ? i : 255);
}

void kcgd_state::vram_addr_w(uint16_t data)
{
	LOGDBG("VRAM WA %06o\n", data);
	m_vram_addr = data;
}

uint16_t kcgd_state::vram_addr_r()
{
	LOGDBG("VRAM RA %06o\n", m_vram_addr);
	return m_vram_addr;
}

void kcgd_state::vram_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGVRAM("VRAM W2 %06o <- %04XH\n", m_vram_addr, data);
	COMBINE_DATA(&m_videoram[m_vram_addr]);
	m_videoram[m_vram_addr] &= ~(1 << 16);
	m_videoram[m_vram_addr] |= (BIT(m_video_control, 7) << 16);
}

uint16_t kcgd_state::vram_data_r()
{
	int hires = BIT(m_videoram[m_vram_addr], 16);
	LOGVRAM("VRAM R2 %06o\n", m_vram_addr);
	m_pic->pic_write_reqa(hires ^ BIT(m_video_control, 6));
	return m_videoram[m_vram_addr];
}

void kcgd_state::vram_mmap_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGVRAM("VRAM W1 %06o <- %04XH\n", offset, data);
	COMBINE_DATA(&m_videoram[offset]);
	m_videoram[offset] &= ~(1 << 16);
	m_videoram[offset] |= (BIT(m_video_control, 7) << 16);
}

uint16_t kcgd_state::vram_mmap_r(offs_t offset)
{
	LOGVRAM("VRAM R1 %06o\n", offset);
	return m_videoram[offset];
}

/*
 * 167770:
 *
 *  0   RW  CSR0    scanline table select (0 = table 0, 1 = table 1)
 *  1   RW  CSR1    progressive or interlaced mode (1 = 480i, 0 = 240p)
 *  5   RW  IEB     500 Hz timer interrupt enable (1 = enabled)
 *  6   RW  IEA     hires mode interrupt enable (1 = enabled)
 *  7   R   REQA    hires mode of last word read via data register
 *  15  R   REQB    500 Hz timer flipflop state
 *
 * RESET signal clears bits 0, 1, 5 and 6.
 *
 * 167772:
 *
 *  2   W   mouse coordinate and button select (0 = X and button 0, 1 = Y and button 1)
 *  2-5 W   palette index
 *  6   W   invert bit 7 of status register 167770
 *  7   W   hires mode on writes to vram (0 = hires)
 *
 * 167774:
 *
 * 0-2	R	mouse coordinate data
 * 3	R	mouse button state
 */

void kcgd_state::palette_control(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_video_control = data;
		m_palette_index = (data >> 2) & 15;
		if (BIT(data, 2))
		{
			m_pic->pic_write_rbuf((m_x & 7) | (BIT(m_sr, 0) << 3));
		}
		else
		{
			m_pic->pic_write_rbuf((m_y & 7) | (BIT(m_sr, 1) << 3));
		}
		LOGDBG("Palette/Control W mask %06o data %06o = index %d control 0x%x\n",
				mem_mask, data, m_palette_index, m_video_control);
	}
	if (ACCESSING_BITS_8_15)
	{
		m_palette_data[m_palette_index] = data >> 8;
		m_palette->set_pen_color(m_palette_index,
				85*((data >> 8) & 3), 85*((data >> 10) & 3), 85*((data >> 12) & 3));
		LOGDBG("Palette/Control W mask %06o data %06o = value 0x%x\n",
				mem_mask, data, m_palette_data[m_palette_index]);
	}
}

/*
    Raster sizes:
    - 800(400)x480 in hires(lores) 60 Hz interlaced mode
    - 800(400)x240 in hires(lores) 30 Hz progressive mode

    Video memory is 17 bits wide. Bit 16 indicates hi/lo res mode for each word,
    host writes it separately (via bit 7 in 167772).
*/

void kcgd_state::draw_scanline(uint16_t *p, uint16_t offset)
{
	for (int i = 0; i < 100; i++)
	{
		uint32_t data = m_videoram[offset++];
		if (BIT(data, 16))
		{
			*p = ( data >> 12) & 0x0F; p++;
			*p = ( data >> 12) & 0x0F; p++;
			*p = ( data >> 8 ) & 0x0F; p++;
			*p = ( data >> 8 ) & 0x0F; p++;
			*p = ( data >> 4 ) & 0x0F; p++;
			*p = ( data >> 4 ) & 0x0F; p++;
			*p =   data        & 0x0F; p++;
			*p =   data        & 0x0F; p++;
		}
		else
		{
			*p = 5*(( data >> 14) & 0x03); p++;
			*p = 5*(( data >> 12) & 0x03); p++;
			*p = 5*(( data >> 10) & 0x03); p++;
			*p = 5*(( data >> 8 ) & 0x03); p++;
			*p = 5*(( data >> 6 ) & 0x03); p++;
			*p = 5*(( data >> 4 ) & 0x03); p++;
			*p = 5*(( data >> 2 ) & 0x03); p++;
			*p = 5*(  data        & 0x03); p++;
		}
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(kcgd_state::scanline_callback)
{
	int y = m_screen->vpos();

	if (y < KCGD_VERT_START) return;
	y -= KCGD_VERT_START;
	if (y >= KCGD_DISP_VERT) return;

	uint16_t offset = m_page ? (KCGD_PAGE_1 >> 1) : (KCGD_PAGE_0 >> 1);

	LOGDBG("scanline_cb frame %d y %.3d page %d offset %04X *offset %04X\n",
		m_screen->frame_number(), y, m_page, offset + y, m_videoram[offset + y]);

	// in progressive mode, display only odd scanlines
	offset = offset + (KCGD_DISP_VERT - 1) - (m_interlace ? y : (y & 0xfffe));
	draw_scanline(&m_tmpbmp.pix(y), m_videoram[offset]);
}

uint32_t kcgd_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_tmpbmp, 0, 0, KCGD_HORZ_START, KCGD_VERT_START, cliprect);
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout kcgd_charlayout =
{
	8, 10,                  /* 8x10 pixels */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	8*10                    /* every char takes 10 bytes */
};

static GFXDECODE_START( gfx_kcgd )
	GFXDECODE_ENTRY("maincpu", 012236, kcgd_charlayout, 0, 1)
GFXDECODE_END

void kcgd_state::kcgd(machine_config &config)
{
	K1801VM2(config, m_maincpu, XTAL(30'800'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &kcgd_state::kcgd_mem);
	m_maincpu->set_initial_mode(0100000);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->out_reset().set(FUNC(kcgd_state::reset_w));

	timer_device &scantimer(TIMER(config, "scantimer"));
	scantimer.configure_periodic(FUNC(kcgd_state::scanline_callback), attotime::from_hz(50 * 28 * 11));
	scantimer.set_start_delay(attotime::from_ticks(KCGD_HORZ_START, XTAL(30'800'000)));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(kcgd_state::screen_update));
	m_screen->set_raw(XTAL(30'800'000), KCGD_TOTAL_HORZ, KCGD_HORZ_START,
		KCGD_HORZ_START+KCGD_DISP_HORZ, KCGD_TOTAL_VERT, KCGD_VERT_START,
		KCGD_VERT_START+KCGD_DISP_VERT);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(kcgd_state::kcgd_palette), 16);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_kcgd);

	K1801VP033(config, m_pic, 0);
	m_pic->set_vec_a(0300);
	m_pic->set_vec_b(0304);
	m_pic->pic_out_wr_callback().set(FUNC(kcgd_state::palette_control));
	m_pic->pic_csr0_wr_callback().set([this] (int state) { m_page = state; });
	m_pic->pic_csr1_wr_callback().set([this] (int state) { m_interlace = state; });

	K1801VP065(config, m_dl11host, XTAL(4'608'000));
	m_dl11host->set_rxc(57600);
	m_dl11host->set_txc(57600);
	m_dl11host->set_rxvec(0360);
	m_dl11host->set_txvec(0364);
	m_dl11host->txd_wr_callback().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_dl11host->rts_wr_callback().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_dl11host->txrdy_wr_callback().set_inputline(m_maincpu, t11_device::VEC_LINE);
	m_dl11host->rxrdy_wr_callback().set_inputline(m_maincpu, t11_device::VEC_LINE);

	RS232_PORT(config, m_rs232, default_rs232_devices, "null_modem");
	m_rs232->rxd_handler().set(m_dl11host, FUNC(k1801vp065_device::rx_w));
	m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(host_rs232_defaults));

	K1801VP065(config, m_dl11kbd, XTAL(4'608'000));
	m_dl11kbd->set_rxc(4800);
	m_dl11kbd->set_txc(4800);
	m_dl11kbd->set_rxvec(060);
	m_dl11kbd->set_txvec(064);
	m_dl11kbd->txd_wr_callback().set(m_ms7004, FUNC(ms7004_device::write_rxd));
	m_dl11kbd->txrdy_wr_callback().set_inputline(m_maincpu, t11_device::VEC_LINE);
	m_dl11kbd->rxrdy_wr_callback().set_inputline(m_maincpu, t11_device::VEC_LINE);

	MS7004(config, m_ms7004, 0);
	m_ms7004->tx_handler().set(m_dl11kbd, FUNC(k1801vp065_device::rx_w));
}

ROM_START( dvk_kcgd )
	ROM_REGION16_BE(060000, "maincpu", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("181")
	ROM_SYSTEM_BIOS(0, "181", "mask 181")
	ROMX_LOAD("kr1801re2-181.bin", 0, 020000, CRC(acac124f) SHA1(412c3eb71bece6f791fc5a9d707cf4692fd0b45b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "182", "mask 182")
	ROMX_LOAD("kr1801re2-182.bin", 0, 020000, CRC(3ca2921a) SHA1(389b30c40ed7e41dae71d58c7bff630359a48153), ROM_BIOS(1))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME    FLAGS */
COMP( 1987, dvk_kcgd, 0,      0,      kcgd,    kcgd,  kcgd_state, empty_init, "USSR",  "DVK KCGD", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
