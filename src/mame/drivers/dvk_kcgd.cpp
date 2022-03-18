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
    - interlace
    - scanline table selection
    - interrupts
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

#include "bus/rs232/rs232.h"
#include "cpu/t11/t11.h"
#include "machine/dl11.h"
#include "machine/ms7004.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"


// these are unverified
static constexpr int KCGD_TOTAL_HORZ = 977;
static constexpr int KCGD_DISP_HORZ = 800;
static constexpr int KCGD_HORZ_START = 0;

static constexpr int KCGD_TOTAL_VERT = 525;
static constexpr int KCGD_DISP_VERT = 480;
static constexpr int KCGD_VERT_START = 0;

static constexpr int KCGD_PAGE_0 = 015574;
static constexpr int KCGD_PAGE_1 = 005574;


//#define LOG_GENERAL (1U <<  0) //defined in logmacro.h already
#define LOG_VRAM      (1U <<  1)
#define LOG_DEBUG     (1U <<  2)

//#define VERBOSE (LOG_DEBUG)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGVRAM(...) LOGMASKED(LOG_VRAM, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


class kcgd_state : public driver_device
{
public:
	kcgd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dl11host(*this, "dl11host")
		, m_rs232(*this, "rs232")
		, m_dl11kbd(*this, "dl11kbd")
		, m_ms7004(*this, "ms7004")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
	{ }

	void kcgd(machine_config &config);

private:
	virtual void machine_reset() override;
	virtual void machine_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);
	void kcgd_palette(palette_device &palette) const;

	enum timer_ids : unsigned
	{
		TIMER_ID_VSYNC,
		TIMER_ID_500HZ
	};
	enum status_bits : unsigned
	{
		KCGD_STATUS_PAGE = 0,
		KCGD_STATUS_INTERLACE = 1,
		KCGD_STATUS_TIMER_INT = 5,
		KCGD_STATUS_MODE_INT = 6,
		KCGD_STATUS_MODE_LAST = 7,
		KCGD_STATUS_TIMER_VAL = 15
	};

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	uint16_t vram_addr_r();
	uint16_t vram_data_r();
	uint16_t vram_mmap_r(offs_t offset);
	void vram_addr_w(uint16_t data);
	void vram_data_w(uint16_t data);
	void vram_mmap_w(offs_t offset, uint16_t data);
	uint16_t status_r();
	void status_w(uint16_t data);
	uint8_t palette_index_r();
	uint8_t palette_data_r();
	void palette_index_w(uint8_t data);
	void palette_data_w(uint8_t data);

	//emu_timer *m_vsync_on_timer;
	emu_timer *m_500hz_timer;

	void kcgd_mem(address_map &map);

	void draw_scanline(uint16_t *p, uint16_t offset);
	rectangle m_tmpclip;
	bitmap_ind16 m_tmpbmp;

	struct
	{
		uint16_t status; // 167770
		uint8_t control; // 167772
		int palette_index, vram_addr;
		uint8_t palette[16];
	} m_video;
	std::unique_ptr<uint32_t[]> m_videoram;

protected:
	required_device<k1801vm2_device> m_maincpu;
	required_device<dl11_device> m_dl11host;
	required_device<rs232_port_device> m_rs232;
	required_device<dl11_device> m_dl11kbd;
	required_device<ms7004_device> m_ms7004;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};

void kcgd_state::kcgd_mem(address_map &map)
{
	map.unmap_value_high();
	map(0000000, 0077777).rw(FUNC(kcgd_state::vram_mmap_r), FUNC(kcgd_state::vram_mmap_w));
	map(0100000, 0157777).rom();
	// 1802VV1 chips
	map(0160000, 0160001).mirror(03774).rw(FUNC(kcgd_state::vram_addr_r), FUNC(kcgd_state::vram_addr_w));
	map(0160002, 0160003).mirror(03774).rw(FUNC(kcgd_state::vram_data_r), FUNC(kcgd_state::vram_data_w));
	// 1801VP1-033 "pic" chip
	map(0167770, 0167771).rw(FUNC(kcgd_state::status_r), FUNC(kcgd_state::status_w));
	map(0167772, 0167772).rw(FUNC(kcgd_state::palette_index_r), FUNC(kcgd_state::palette_index_w)); // reads always return 0
	map(0167773, 0167773).rw(FUNC(kcgd_state::palette_data_r), FUNC(kcgd_state::palette_data_w));
	// 1801VP1-065 chips, not 100% DL11 compatible (error bits are in RCSR, not RBUF)
	map(0176560, 0176567).rw(m_dl11host, FUNC(dl11_device::read), FUNC(dl11_device::write));
	map(0177560, 0177567).rw(m_dl11kbd, FUNC(dl11_device::read), FUNC(dl11_device::write));
}

// future
static const z80_daisy_config daisy_chain[] =
{
//  { "pic" },
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
	DEVICE_INPUT_DEFAULTS( "FLOW_CONTROL", 0x01, 0x01 )
DEVICE_INPUT_DEFAULTS_END


void kcgd_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
/*
    case TIMER_ID_VSYNC:
        m_maincpu->set_input_line(INPUT_LINE_EVNT, ASSERT_LINE);
        break;
*/
	case TIMER_ID_500HZ:
		m_video.status ^= (1 << KCGD_STATUS_TIMER_VAL);
		break;
	}
}

void kcgd_state::machine_reset()
{
	memset(&m_video, 0, sizeof(m_video));
}

void kcgd_state::machine_start()
{
	// 64 kwords, word size is 17 bits
	m_videoram = std::make_unique<uint32_t[]>(65536);

	m_tmpclip = rectangle(0, KCGD_DISP_HORZ - 1, 0, KCGD_DISP_VERT - 1);
	m_tmpbmp.allocate(KCGD_DISP_HORZ, KCGD_DISP_VERT);
// future
//  m_vsync_on_timer = timer_alloc(TIMER_ID_VSYNC);
//  m_vsync_on_timer->adjust(m_screen->time_until_pos(0, 0), 0, m_screen->frame_period());
	m_500hz_timer = timer_alloc(TIMER_ID_500HZ);
	m_500hz_timer->adjust(attotime::from_hz(500), 0, attotime::from_hz(500));
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
	m_video.vram_addr = data;
}

uint16_t kcgd_state::vram_addr_r()
{
	LOGDBG("VRAM RA %06o\n", m_video.vram_addr);
	return m_video.vram_addr;
}

void kcgd_state::vram_data_w(uint16_t data)
{
	LOGVRAM("VRAM W2 %06o <- %04XH\n", m_video.vram_addr, data);
	m_videoram[m_video.vram_addr] = data | (BIT(m_video.control, 7) << 16);
}

uint16_t kcgd_state::vram_data_r()
{
	LOGVRAM("VRAM R2 %06o\n", m_video.vram_addr);
	m_video.status = (m_video.status & 0xff7f) | (BIT(m_videoram[m_video.vram_addr], 16) << 7);
	return (uint16_t)(m_videoram[m_video.vram_addr] & 0xffff);
}

void kcgd_state::vram_mmap_w(offs_t offset, uint16_t data)
{
	LOGVRAM("VRAM W1 %06o <- %04XH\n", offset, data);
	m_videoram[offset] = data | (BIT(m_video.control, 7) << 16);
}

uint16_t kcgd_state::vram_mmap_r(offs_t offset)
{
	LOGVRAM("VRAM R1 %06o\n", offset);
	return (uint16_t)m_videoram[offset];
}

void kcgd_state::status_w(uint16_t data)
{
	LOG("Status W data %04XH (useful %02XH)\n", data, data & 0x63);
	// bits 7 and 15 are read-only
	m_video.status = (m_video.status & 0x8080) | (data & 0x7f7f);
}

uint16_t kcgd_state::status_r()
{
	uint16_t data = m_video.status ^ (BIT(m_video.control, 6) << 7);
	LOG("Status R data %04X index %d\n", data, m_video.palette_index);
	return data;
}

void kcgd_state::palette_index_w(uint8_t data)
{
	m_video.control = data;
	m_video.palette_index = ((data >> 2) & 15);
	LOG("Palette index, Control W data %02XH index %d\n", data, m_video.palette_index);
}

void kcgd_state::palette_data_w(uint8_t data)
{
	LOG("Palette data W data %02XH index %d\n", data, m_video.palette_index);
	m_video.palette[m_video.palette_index] = data;
	m_palette->set_pen_color(m_video.palette_index,
		85*(data & 3), 85*((data >> 2) & 3), 85*((data >> 4) & 3));
}

uint8_t kcgd_state::palette_index_r()
{
	return 0;
}

uint8_t kcgd_state::palette_data_r()
{
	LOG("Palette data R index %d\n", m_video.palette_index);
	return m_video.palette[m_video.palette_index];
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
	int i;

	for (i = 0; i < 100; i++)
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
	uint16_t y = m_screen->vpos();

	if (y < KCGD_VERT_START) return;
	y -= KCGD_VERT_START;
	if (y >= KCGD_DISP_VERT) return;

	uint16_t const offset = BIT(m_video.status, KCGD_STATUS_PAGE) ? (KCGD_PAGE_1 >> 1) : (KCGD_PAGE_0 >> 1);

	LOGDBG("scanline_cb frame %d y %.3d page %d offset %04X *offset %04X\n",
		m_screen->frame_number(), BIT(m_video.status, KCGD_STATUS_PAGE),
		y, offset + y, m_videoram[offset + y]);

	draw_scanline(&m_tmpbmp.pix(y), m_videoram[offset + (KCGD_DISP_VERT - 1) - y]);
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
	GFXDECODE_ENTRY("maincpu", 0112236, kcgd_charlayout, 0, 1)
GFXDECODE_END

void kcgd_state::kcgd(machine_config &config)
{
	K1801VM2(config, m_maincpu, XTAL(30'800'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &kcgd_state::kcgd_mem);
	m_maincpu->set_initial_mode(0100000);
// future
//  m_maincpu->set_daisy_config(daisy_chain);

	timer_device &scantimer(TIMER(config, "scantimer"));
	scantimer.configure_periodic(FUNC(kcgd_state::scanline_callback), attotime::from_hz(50 * 28 * 11));
	scantimer.set_start_delay(attotime::from_hz(XTAL(30'800'000) / KCGD_HORZ_START));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(kcgd_state::screen_update));
	m_screen->set_raw(XTAL(30'800'000), KCGD_TOTAL_HORZ, KCGD_HORZ_START,
		KCGD_HORZ_START+KCGD_DISP_HORZ, KCGD_TOTAL_VERT, KCGD_VERT_START,
		KCGD_VERT_START+KCGD_DISP_VERT);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(kcgd_state::kcgd_palette), 16);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_kcgd);

	DL11(config, m_dl11host, XTAL(4'608'000));
	m_dl11host->set_rxc(57600);
	m_dl11host->set_txc(57600);
	m_dl11host->set_rxvec(0360);
	m_dl11host->set_txvec(0364);
	m_dl11host->txd_wr_callback().set(m_rs232, FUNC(rs232_port_device::write_txd));
// future
//  m_dl11host->rts_wr_callback().set(m_rs232, FUNC(rs232_port_device::write_rts));
//  m_dl11host->txrdy_wr_callback().set_inputline(m_maincpu, T11_IRQ0);
//  m_dl11host->rxrdy_wr_callback().set_inputline(m_maincpu, T11_IRQ0);

	RS232_PORT(config, m_rs232, default_rs232_devices, "null_modem");
	m_rs232->rxd_handler().set(m_dl11host, FUNC(dl11_device::rx_w));
	m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(host_rs232_defaults));

	DL11(config, m_dl11kbd, XTAL(4'608'000));
	m_dl11kbd->set_rxc(4960);
	m_dl11kbd->set_txc(4960);
	m_dl11kbd->set_rxvec(060);
	m_dl11kbd->set_txvec(064);
	m_dl11kbd->txd_wr_callback().set(m_ms7004, FUNC(ms7004_device::write_rxd));
// future
//  m_dl11kbd->txrdy_wr_callback().set_inputline(m_maincpu, T11_IRQ0);
//  m_dl11kbd->rxrdy_wr_callback().set_inputline(m_maincpu, T11_IRQ0);

	MS7004(config, m_ms7004, 0);
	m_ms7004->tx_handler().set(m_dl11kbd, FUNC(dl11_device::rx_w));
}

ROM_START( dvk_kcgd )
	ROM_REGION16_BE(0x100000, "maincpu", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("181")
	ROM_SYSTEM_BIOS(0, "181", "mask 181")
	ROMX_LOAD("kr1801re2-181.bin", 0100000, 020000, CRC(acac124f) SHA1(412c3eb71bece6f791fc5a9d707cf4692fd0b45b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "182", "mask 182")
	ROMX_LOAD("kr1801re2-182.bin", 0100000, 020000, CRC(3ca2921a) SHA1(389b30c40ed7e41dae71d58c7bff630359a48153), ROM_BIOS(1))
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME    FLAGS */
COMP( 1987, dvk_kcgd, 0,      0,      kcgd,    0,     kcgd_state, empty_init, "USSR",  "DVK KCGD", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
