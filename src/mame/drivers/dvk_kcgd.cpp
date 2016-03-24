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

	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);
	DECLARE_PALETTE_INIT(kcgd);

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

	emu_timer *m_vsync_on_timer;
	emu_timer *m_vsync_off_timer;
	emu_timer *m_500hz_timer;

private:
	void draw_scanline(UINT16 *p, UINT16 offset);
	rectangle m_tmpclip;
	bitmap_ind16 m_tmpbmp;

	struct {
		UINT16 status;  // 167770
		UINT8 control;  // 167772
		int palette_index, vram_addr;
		UINT8 palette[16];
	} m_video;
	std::unique_ptr<UINT32[]> m_videoram;

protected:
	required_device<cpu_device> m_maincpu;
//  required_device<ms7004_device> m_ms7004;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};

static ADDRESS_MAP_START( kcgd_mem, AS_PROGRAM, 16, kcgd_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0000000, 0077777) AM_READWRITE(vram_mmap_r, vram_mmap_w)
	AM_RANGE (0100000, 0157777) AM_ROM
	AM_RANGE (0160000, 0160001) AM_MIRROR(03774) AM_READWRITE(vram_addr_r, vram_addr_w)
	AM_RANGE (0160002, 0160003) AM_MIRROR(03774) AM_READWRITE(vram_data_r, vram_data_w)
	AM_RANGE (0167770, 0167771) AM_READWRITE(status_r, status_w)
	AM_RANGE (0167772, 0167773) AM_READWRITE8(palette_index_r, palette_index_w, 0x00ff) // reads always return 0
	AM_RANGE (0167772, 0167773) AM_READWRITE8(palette_data_r, palette_data_w, 0xff00)
//  AM_RANGE (0176560, 0176567) AM_RAM  // USART2 -- host
//  AM_RANGE (0177560, 0177567) AM_RAM  // USART3 -- keyboard
ADDRESS_MAP_END

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
//  screen_device *screen = machine().device<screen_device>("screen");

	// 64 kwords, word size is 17 bits
	m_videoram = std::make_unique<UINT32[]>(65536);

	m_tmpclip = rectangle(0, KCGD_DISP_HORZ-1, 0, KCGD_DISP_VERT-1);
	m_tmpbmp.allocate(KCGD_DISP_HORZ, KCGD_DISP_VERT);
/*
    m_vsync_on_timer = timer_alloc(TIMER_ID_VSYNC_ON);
    m_vsync_on_timer->adjust(screen->time_until_pos(0, 0), 0, screen->frame_period());

    m_vsync_off_timer = timer_alloc(TIMER_ID_VSYNC_OFF);
    m_vsync_off_timer->adjust(screen->time_until_pos(16, 0), 0, screen->frame_period());
*/
	m_500hz_timer = timer_alloc(TIMER_ID_500HZ);
	m_500hz_timer->adjust(attotime::from_hz(500), 0, attotime::from_hz(500));
}

PALETTE_INIT_MEMBER(kcgd_state, kcgd)
{
	for (int i = 0; i < 16; i++)
	{
		palette.set_pen_color(i, i?i:255, i?i:255, i?i:255);
	}
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
	return (UINT16) (m_videoram[m_video.vram_addr] & 0xffff);
}

WRITE16_MEMBER(kcgd_state::vram_mmap_w)
{
	DBG_LOG(3,"VRAM W1", ("%06o <- %04XH\n", offset, data));
	m_videoram[offset] = data | (BIT(m_video.control, 7) << 16);
}

READ16_MEMBER(kcgd_state::vram_mmap_r)
{
	DBG_LOG(3,"VRAM R1", ("%06o\n", offset));
	return (UINT16) (m_videoram[offset] & 0xffff);
}

WRITE16_MEMBER(kcgd_state::status_w)
{
	DBG_LOG(1,"Status W", ("data %04XH (useful %02XH)\n", data, data & 0x63));
	// bits 7 and 15 are read-only
	m_video.status = (m_video.status & 0x8080) | (data & 0x7f7f);
}

READ16_MEMBER(kcgd_state::status_r)
{
	UINT16 data = m_video.status ^ (BIT(m_video.control, 6) << 7);
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

void kcgd_state::draw_scanline(UINT16 *p, UINT16 offset)
{
	int i;

	for ( i = 0; i < 100; i++ )
	{
		UINT32 data = m_videoram[ offset++ ];
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
	UINT16 y = m_screen->vpos(), offset;

	if (y < KCGD_VERT_START) return;
	y -= KCGD_VERT_START;
	if (y >= KCGD_DISP_VERT) return;

	offset = BIT(m_video.status, KCGD_STATUS_PAGE) ? (KCGD_PAGE_1 >> 1) : (KCGD_PAGE_0 >> 1);

	DBG_LOG(2,"scanline_cb", ("frame %d y %.3d page %d offset %04X *offset %04X\n",
		m_screen->frame_number(), BIT(m_video.status, KCGD_STATUS_PAGE),
		y, offset + y, m_videoram[offset + y]));

	draw_scanline(&m_tmpbmp.pix16(y), m_videoram[offset + (KCGD_DISP_VERT-1) - y]);
}

UINT32 kcgd_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

static GFXDECODE_START( kcgd )
	GFXDECODE_ENTRY("maincpu", 0112236, kcgd_charlayout, 0, 1)
GFXDECODE_END

static MACHINE_CONFIG_START( kcgd, kcgd_state )
	MCFG_CPU_ADD("maincpu", K1801VM2, XTAL_30_8MHz/4)
	MCFG_CPU_PROGRAM_MAP(kcgd_mem)
	MCFG_T11_INITIAL_MODE(0100000)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("scantimer", kcgd_state, scanline_callback, attotime::from_hz(50*28*11)) // XXX verify
	MCFG_TIMER_START_DELAY(attotime::from_hz(XTAL_30_8MHz/KCGD_HORZ_START))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(kcgd_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL_30_8MHz, KCGD_TOTAL_HORZ, KCGD_HORZ_START,
		KCGD_HORZ_START+KCGD_DISP_HORZ, KCGD_TOTAL_VERT, KCGD_VERT_START,
		KCGD_VERT_START+KCGD_DISP_VERT);

	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(kcgd_state, kcgd)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", kcgd)
#if 0
	MCFG_DEVICE_ADD("ms7004", MS7004, 0)
	MCFG_MS7004_TX_HANDLER(DEVWRITELINE("i8251kbd", i8251_device, write_rxd))

	MCFG_DEVICE_ADD("keyboard_clock", CLOCK, 4800*16)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(kcgd_state, write_keyboard_clock))
#endif
MACHINE_CONFIG_END

ROM_START( dvk_kcgd )
	ROM_REGION16_BE(0x100000,"maincpu", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("181")
	ROM_SYSTEM_BIOS(0, "181", "mask 181")
	ROMX_LOAD("kr1801re2-181.bin", 0100000, 020000, CRC(acac124f) SHA1(412c3eb71bece6f791fc5a9d707cf4692fd0b45b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "182", "mask 182")
	ROMX_LOAD("kr1801re2-182.bin", 0100000, 020000, CRC(3ca2921a) SHA1(389b30c40ed7e41dae71d58c7bff630359a48153), ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    INIT                      COMPANY     FULLNAME       FLAGS */
COMP( 1987, dvk_kcgd, 0,      0,       kcgd,      0,       driver_device,     0,     "USSR",     "DVK KCGD",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
