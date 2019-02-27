// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    KCGD (Kontroller Cvetnogo Graficheskogo Displeya = Colour Graphics
    Display Controller), a replacement for KSM (dvk_ksm.c) in later
    models of DVK desktops.

    MPI (Q-Bus clone) board. Interfaces with MS7004 (DEC LK201 workalike)
    keyboard, mouse, and monochrome or color CRT.

    To do:
    - K1801VM2 CPU core (interrupts and EVNT pin, full EIS set, other insns)
    - Everything else :-)

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/t11/t11.h"
#include "machine/clock.h"
#include "machine/ms7004.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"


#define KCGD_TOTAL_HORZ 1000    // XXX verify
#define KCGD_DISP_HORZ  800
#define KCGD_HORZ_START 200 // XXX verify

#define KCGD_TOTAL_VERT 600 // XXX verify
#define KCGD_DISP_VERT  480
#define KCGD_VERT_START 100 // XXX verify

#define KCGD_STATUS_PAGE    0
#define KCGD_STATUS_INTERLACE   1
#define KCGD_STATUS_TIMER_INT   5
#define KCGD_STATUS_MODE_INT    6
#define KCGD_STATUS_MODE_LAST   7
#define KCGD_STATUS_TIMER_VAL   15

#define KCGD_PAGE_0 015574
#define KCGD_PAGE_1 005574

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


class kcgd_state : public driver_device
{
public:
	kcgd_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
//      m_ms7004(*this, "ms7004"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{ }

	void kcgd(machine_config &config);

private:
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);
	void kcgd_palette(palette_device &palette) const;

	enum
	{
		TIMER_ID_VSYNC_ON,
		TIMER_ID_VSYNC_OFF,
		TIMER_ID_500HZ
	};
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	DECLARE_READ16_MEMBER(vram_addr_r);
	DECLARE_READ16_MEMBER(vram_data_r);
	DECLARE_READ16_MEMBER(vram_mmap_r);
	DECLARE_WRITE16_MEMBER(vram_addr_w);
	DECLARE_WRITE16_MEMBER(vram_data_w);
	DECLARE_WRITE16_MEMBER(vram_mmap_w);
	DECLARE_READ16_MEMBER(status_r);
	DECLARE_WRITE16_MEMBER(status_w);
	DECLARE_READ8_MEMBER(palette_index_r);
	DECLARE_READ8_MEMBER(palette_data_r);
	DECLARE_WRITE8_MEMBER(palette_index_w);
	DECLARE_WRITE8_MEMBER(palette_data_w);

	//emu_timer *m_vsync_on_timer;
	//emu_timer *m_vsync_off_timer;
	emu_timer *m_500hz_timer;

	void kcgd_mem(address_map &map);

	void draw_scanline(uint16_t *p, uint16_t offset);
	rectangle m_tmpclip;
	bitmap_ind16 m_tmpbmp;

	struct {
		uint16_t status;  // 167770
		uint8_t control;  // 167772
		int palette_index, vram_addr;
		uint8_t palette[16];
	} m_video;
	std::unique_ptr<uint32_t[]> m_videoram;

	required_device<k1801vm2_device> m_maincpu;
//  required_device<ms7004_device> m_ms7004;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};

void kcgd_state::kcgd_mem(address_map &map)
{
	map.unmap_value_high();
	map(0000000, 0077777).rw(FUNC(kcgd_state::vram_mmap_r), FUNC(kcgd_state::vram_mmap_w));
	map(0100000, 0157777).rom();
	map(0160000, 0160001).mirror(03774).rw(FUNC(kcgd_state::vram_addr_r), FUNC(kcgd_state::vram_addr_w));
	map(0160002, 0160003).mirror(03774).rw(FUNC(kcgd_state::vram_data_r), FUNC(kcgd_state::vram_data_w));
	map(0167770, 0167771).rw(FUNC(kcgd_state::status_r), FUNC(kcgd_state::status_w));
	map(0167772, 0167772).rw(FUNC(kcgd_state::palette_index_r), FUNC(kcgd_state::palette_index_w)); // reads always return 0
	map(0167773, 0167773).rw(FUNC(kcgd_state::palette_data_r), FUNC(kcgd_state::palette_data_w));
//  map(0176560, 0176567).ram();  // USART2 -- host
//  map(0177560, 0177567).ram();  // USART3 -- keyboard
}

void kcgd_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
/*
    case TIMER_ID_VSYNC_ON:
        m_maincpu->set_input_line(INPUT_LINE_EVNT, ASSERT_LINE);
        break;

    case TIMER_ID_VSYNC_OFF:
        m_maincpu->set_input_line(INPUT_LINE_EVNT, CLEAR_LINE);
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

void kcgd_state::video_start()
{
	// 64 kwords, word size is 17 bits
	m_videoram = std::make_unique<uint32_t[]>(65536);

	m_tmpclip = rectangle(0, KCGD_DISP_HORZ-1, 0, KCGD_DISP_VERT-1);
	m_tmpbmp.allocate(KCGD_DISP_HORZ, KCGD_DISP_VERT);
/*
    m_vsync_on_timer = timer_alloc(TIMER_ID_VSYNC_ON);
    m_vsync_on_timer->adjust(m_screen->time_until_pos(0, 0), 0, m_screen->frame_period());

    m_vsync_off_timer = timer_alloc(TIMER_ID_VSYNC_OFF);
    m_vsync_off_timer->adjust(m_screen->time_until_pos(16, 0), 0, m_screen->frame_period());
*/
	m_500hz_timer = timer_alloc(TIMER_ID_500HZ);
	m_500hz_timer->adjust(attotime::from_hz(500), 0, attotime::from_hz(500));
}

void kcgd_state::kcgd_palette(palette_device &palette) const
{
	// FIXME: this doesn't seem right at all - no actual black, and all the grey levels are very close to black
	// should it just initialise everything besides the first entry to black, or should it be a greyscale ramp?
	for (int i = 0; i < 16; i++)
		palette.set_pen_color(i, i ? i : 255, i ? i : 255, i ? i : 255);
}

/*
    VRAM is 128K and is word-addressable, so address fits into 16 bits.
    Low 32K of VRAM are not used to store pixel data -- XXX.
*/
WRITE16_MEMBER(kcgd_state::vram_addr_w)
{
	DBG_LOG(3,"VRAM WA", ("%06o\n", data));
	m_video.vram_addr = data;
}

READ16_MEMBER(kcgd_state::vram_addr_r)
{
	DBG_LOG(3,"VRAM RA", ("\n"));
	return m_video.vram_addr;
}

WRITE16_MEMBER(kcgd_state::vram_data_w)
{
	DBG_LOG(1,"VRAM W2", ("%06o <- %04XH\n", m_video.vram_addr, data));
	m_videoram[m_video.vram_addr] = data | (BIT(m_video.control, 7) << 16);
}

READ16_MEMBER(kcgd_state::vram_data_r)
{
	DBG_LOG(2,"VRAM R2", ("%06o\n", m_video.vram_addr));
	m_video.status = (m_video.status & 0xff7f) | (BIT(m_videoram[m_video.vram_addr], 16) << 7);
	return (uint16_t) (m_videoram[m_video.vram_addr] & 0xffff);
}

WRITE16_MEMBER(kcgd_state::vram_mmap_w)
{
	DBG_LOG(3,"VRAM W1", ("%06o <- %04XH\n", offset, data));
	m_videoram[offset] = data | (BIT(m_video.control, 7) << 16);
}

READ16_MEMBER(kcgd_state::vram_mmap_r)
{
	DBG_LOG(3,"VRAM R1", ("%06o\n", offset));
	return (uint16_t) (m_videoram[offset] & 0xffff);
}

WRITE16_MEMBER(kcgd_state::status_w)
{
	DBG_LOG(1,"Status W", ("data %04XH (useful %02XH)\n", data, data & 0x63));
	// bits 7 and 15 are read-only
	m_video.status = (m_video.status & 0x8080) | (data & 0x7f7f);
}

READ16_MEMBER(kcgd_state::status_r)
{
	uint16_t data = m_video.status ^ (BIT(m_video.control, 6) << 7);
	DBG_LOG(1,"Status R", ("data %04X index %d\n", data, m_video.palette_index));
	return data;
}

WRITE8_MEMBER(kcgd_state::palette_index_w)
{
	m_video.control = data;
	m_video.palette_index = ((data >> 2) & 15);
	DBG_LOG(1,"Palette index, Control W", ("data %02XH index %d\n", data, m_video.palette_index));
}

WRITE8_MEMBER(kcgd_state::palette_data_w)
{
	DBG_LOG(1,"Palette data W", ("data %02XH index %d\n", data, m_video.palette_index));
	m_video.palette[m_video.palette_index] = data;
	m_palette->set_pen_color(m_video.palette_index,
		85*(data & 3), 85*((data >> 2) & 3), 85*((data >> 4) & 3));
}

READ8_MEMBER(kcgd_state::palette_index_r)
{
	return 0;
}

READ8_MEMBER(kcgd_state::palette_data_r)
{
	DBG_LOG(1,"Palette data R", ("index %d\n", m_video.palette_index));
	return m_video.palette[m_video.palette_index];
}

/*
    Raster sizes are:
    - 800(400)x480 in hires(lores) 60 Hz interlaced mode
    - 800(400)x240 in hires(lores) 30 Hz progressive mode

    Video memory is 17 bits wide (bit 16 indicates hi/lo res mode for each word,
    host writes it separately (via bit 7 in 167772).
*/

void kcgd_state::draw_scanline(uint16_t *p, uint16_t offset)
{
	int i;

	for ( i = 0; i < 100; i++ )
	{
		uint32_t data = m_videoram[ offset++ ];
		if (BIT(data, 16)) {
			*p = ( data >> 12) & 0x0F; p++;
			*p = ( data >> 12) & 0x0F; p++;
			*p = ( data >> 8 ) & 0x0F; p++;
			*p = ( data >> 8 ) & 0x0F; p++;
			*p = ( data >> 4 ) & 0x0F; p++;
			*p = ( data >> 4 ) & 0x0F; p++;
			*p =   data        & 0x0F; p++;
			*p =   data        & 0x0F; p++;
		} else {
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
	uint16_t y = m_screen->vpos(), offset;

	if (y < KCGD_VERT_START) return;
	y -= KCGD_VERT_START;
	if (y >= KCGD_DISP_VERT) return;

	offset = BIT(m_video.status, KCGD_STATUS_PAGE) ? (KCGD_PAGE_1 >> 1) : (KCGD_PAGE_0 >> 1);

	DBG_LOG(2,"scanline_cb", ("frame %d y %.3d page %d offset %04X *offset %04X\n",
		m_screen->frame_number(), BIT(m_video.status, KCGD_STATUS_PAGE),
		y, offset + y, m_videoram[offset + y]));

	draw_scanline(&m_tmpbmp.pix16(y), m_videoram[offset + (KCGD_DISP_VERT-1) - y]);
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

MACHINE_CONFIG_START(kcgd_state::kcgd)
	K1801VM2(config, m_maincpu, XTAL(30'800'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &kcgd_state::kcgd_mem);
	m_maincpu->set_initial_mode(0100000);

	timer_device &scantimer(TIMER(config, "scantimer"));
	scantimer.configure_periodic(FUNC(kcgd_state::scanline_callback), attotime::from_hz(50*28*11)); // XXX verify
	scantimer.set_start_delay(attotime::from_hz(XTAL(30'800'000)/KCGD_HORZ_START));

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(kcgd_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL(30'800'000), KCGD_TOTAL_HORZ, KCGD_HORZ_START,
		KCGD_HORZ_START+KCGD_DISP_HORZ, KCGD_TOTAL_VERT, KCGD_VERT_START,
		KCGD_VERT_START+KCGD_DISP_VERT);
	MCFG_SCREEN_PALETTE(m_palette)

	PALETTE(config, m_palette, FUNC(kcgd_state::kcgd_palette), 16);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_kcgd);
#if 0
	MS7004(config, m_ms7004, 0);
	m_ms7004->tx_handler().set("i8251kbd", FUNC(i8251_device::write_rxd));

	clock_device &keyboard_clock(CLOCK(config, "keyboard_clock", 4800*16));
	keyboard_clock.signal_handler().set(FUNC(kcgd_state::write_keyboard_clock));
#endif
MACHINE_CONFIG_END

ROM_START( dvk_kcgd )
	ROM_REGION16_BE(0x100000,"maincpu", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("181")
	ROM_SYSTEM_BIOS(0, "181", "mask 181")
	ROMX_LOAD("kr1801re2-181.bin", 0100000, 020000, CRC(acac124f) SHA1(412c3eb71bece6f791fc5a9d707cf4692fd0b45b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "182", "mask 182")
	ROMX_LOAD("kr1801re2-182.bin", 0100000, 020000, CRC(3ca2921a) SHA1(389b30c40ed7e41dae71d58c7bff630359a48153), ROM_BIOS(1))
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME    FLAGS */
COMP( 1987, dvk_kcgd, 0,      0,      kcgd,    0,     kcgd_state, empty_init, "USSR",  "DVK KCGD", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
