// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

    - GR is always 0

*/

#include "emu.h"
#include "includes/newbrain.h"
#include "screen.h"

#define VERBOSE 0
#include "logmacro.h"

#define NEWBRAIN_VIDEO_RV               0x01
#define NEWBRAIN_VIDEO_FS               0x02
#define NEWBRAIN_VIDEO_32_40            0x04
#define NEWBRAIN_VIDEO_UCR              0x08
#define NEWBRAIN_VIDEO_80L              0x40

void newbrain_state::tvl(uint8_t data, int a6)
{
	/* latch video address counter bits A5-A0 */
	m_tvl = m_80l ? 0x04 : 0x02;

	/* latch video address counter bit A6 */
	m_tvl |= a6 << 6;

	/* latch data to video address counter bits A14-A7 */
	m_tvl |= (data << 7);

	LOG("%s %s TVL %04x\n", machine().time().as_string(), machine().describe_context(), m_tvl);
}

void newbrain_state::tvtl_w(uint8_t data)
{
	/*

	    bit     signal      description

	    0       RV          1 reverses video over entire field, ie. black on white
	    1       FS          0 generates 128 characters and 128 reverse field characters from 8 bit character code. 1 generates 256 characters from 8 bit character code
	    2       32/_40      0 generates 320 or 640 horizontal dots in pixel graphics mode. 1 generates 256 or 512 horizontal dots in pixel graphics mode
	    3       UCR         0 selects 256 characters expressed in an 8x10 matrix, and 25 lines (max) displayed. 1 selects 256 characters in an 8x8 matrix, and 31 lines (max) displayed
	    4
	    5
	    6       80L         0 selects 40 character line length. 1 selects 80 character line length
	    7

	*/

	LOG("%s %s TVTL %02x\n", machine().time().as_string(), machine().describe_context(), data);

	m_rv = BIT(data, 0);
	m_fs = BIT(data, 1);
	m_32_40 = BIT(data, 2);
	m_ucr = BIT(data, 3);
	m_80l = BIT(data, 6);
}

void newbrain_state::video_start()
{
	// set timer
	m_clkint_timer = timer_alloc(TIMER_ID_CLKINT);
	m_clkint_timer->adjust(attotime::zero, 0, attotime::from_hz(50));

	// state saving
	save_item(NAME(m_rv));
	save_item(NAME(m_fs));
	save_item(NAME(m_32_40));
	save_item(NAME(m_ucr));
	save_item(NAME(m_80l));
	save_item(NAME(m_tvl));
}

void newbrain_state::do_screen_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int columns = m_80l ? 80 : 40;
	int excess = m_32_40 ? 4 : 24;
	int gr = 0;

	uint16_t videoram_addr = m_tvl;
	int rc = 0;
	uint8_t vsr = 0;
	uint8_t grsr = 0;

	for (int y = 0; y < 240; y++) {
		int x = 0;

		for (int sx = 0; sx < columns; sx++) {
			uint8_t rd = m_ram->pointer()[(videoram_addr + sx) & 0x7fff];

			bool rc3 = BIT(rc, 3);
			bool txt = !(BIT(rd, 6) || BIT(rd, 5));
			bool txtq = m_ucr || txt;

			int rc_ = rc & 0x07;
			if (rc3 && txt) {
				rc_ = 7;
			}

			uint16_t charrom_addr = (m_ucr << 11) | (rc_ << 8) | ((BIT(rd, 7) && m_fs) << 7) | (rd & 0x7f);
			uint8_t crd = m_char_rom->base()[charrom_addr & 0xfff];
			bool crd0 = BIT(crd, 0);

			bool ldvsr = !(((!rc3 ^ crd0) || gr) || txtq);

			if (!ldvsr && !gr) {
				vsr = (crd & 0xfe) | (crd0 && txtq);
			}

			if (!ldvsr && gr) {
				grsr = rd;
			}

			for (int i = 0; i < 8; i++) {
				uint8_t sr = gr ? grsr : vsr;
				int color = BIT(sr, 7) ^ m_rv;

				bitmap.pix(y, x++) = m_palette->pen(color);

				if (columns == 40) {
					bitmap.pix(y, x++) = m_palette->pen(color);
				}

				grsr <<= 1;
				vsr <<= 1;
			}
		}

		if (gr)
		{
			// get new data for each line
			videoram_addr += columns;
			videoram_addr += excess;
		}
		else
		{
			rc++;

			if (rc == (m_ucr ? 8 : 10)) {
				rc = 0;

				// get new data after each character row
				videoram_addr += columns;
				videoram_addr += excess;
			}
		}
	}
}

uint32_t newbrain_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_tvp)
		do_screen_update(bitmap, cliprect);
	else
		bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}

/* F4 Character Displayer */
static const gfx_layout newbrain_charlayout =
{
	8, 10,                  /* 8 x 10 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*256*8, 1*256*8, 2*256*8, 3*256*8, 4*256*8, 5*256*8, 6*256*8, 7*256*8, 8*256*8, 9*256*8, 10*256*8, 11*256*8, 12*256*8, 13*256*8, 14*256*8, 15*256*8 },
	8                   /* every char takes 16 x 1 bytes */
};

static GFXDECODE_START( gfx_newbrain )
	GFXDECODE_ENTRY( "chargen", 0x0000, newbrain_charlayout, 0, 1 )
GFXDECODE_END

/* Machine Drivers */

void newbrain_state::newbrain_video(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_screen_update(FUNC(newbrain_state::screen_update));
	screen.set_refresh_hz(50);
	screen.set_size(640, 250);
	screen.set_visarea(0, 639, 0, 249);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_newbrain);
}
