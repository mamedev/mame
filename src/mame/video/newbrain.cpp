// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "includes/newbrain.h"
#include "rendlay.h"
#include "newbrain.lh"

#define LOG 0

#define NEWBRAIN_VIDEO_RV               0x01
#define NEWBRAIN_VIDEO_FS               0x02
#define NEWBRAIN_VIDEO_32_40            0x04
#define NEWBRAIN_VIDEO_UCR              0x08
#define NEWBRAIN_VIDEO_80L              0x40

void newbrain_state::tvl(UINT8 data, int a6)
{
	/* latch video address counter bits A5-A0 */
	m_tvl = m_80l ? 0x04 : 0x02;

	/* latch video address counter bit A6 */
	m_tvl |= a6 << 6;

	/* latch data to video address counter bits A14-A7 */
	m_tvl |= (data << 7);

	if (LOG) logerror("%s %s TVL %04x\n", machine().time().as_string(), machine().describe_context(), m_tvl);
}

WRITE8_MEMBER( newbrain_state::tvtl_w )
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

	if (LOG) logerror("%s %s TVTL %02x\n", machine().time().as_string(), machine().describe_context(), data);

	m_rv = BIT(data, 0);
	m_fs = BIT(data, 1);
	m_32_40 = BIT(data, 2);
	m_ucr = BIT(data, 3);
	m_80l = BIT(data, 6);
}

void newbrain_state::video_start()
{
	// state saving
	save_item(NAME(m_rv));
	save_item(NAME(m_fs));
	save_item(NAME(m_32_40));
	save_item(NAME(m_ucr));
	save_item(NAME(m_80l));
	save_item(NAME(m_tvl));
}

void newbrain_state::screen_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int columns = m_80l ? 80 : 40;
	int excess = m_32_40 ? 4 : 24;
	int gr = 0;

	UINT16 videoram_addr = m_tvl;
	UINT8 rc = 0;

	for (int y = 0; y < 200; y++)
	{
		int x = 0;

		for (int sx = 0; sx < columns; sx++)
		{
			UINT8 videoram_data = m_ram->pointer()[(videoram_addr + sx) & 0x7fff];
			UINT8 charrom_data;

			if (gr)
			{
				/* render video ram data */
				charrom_data = videoram_data;
			}
			else
			{
				/* render character rom data */
				UINT16 charrom_addr = (rc << 8) | ((BIT(videoram_data, 7) && m_fs) << 7) | (videoram_data & 0x7f);
				charrom_data = m_char_rom->base()[charrom_addr & 0xfff];

				if ((videoram_data & 0x80) && !m_fs)
				{
					/* invert character */
					charrom_data ^= 0xff;
				}

				if ((videoram_data & 0x60) && !m_ucr)
				{
					/* strip bit D0 */
					charrom_data &= 0xfe;
				}
			}

			for (int bit = 0; bit < 8; bit++)
			{
				int color = BIT(charrom_data, 7) ^ m_rv;

				bitmap.pix32(y, x++) = m_palette->pen(color);

				if (columns == 40)
				{
					bitmap.pix32(y, x++) = m_palette->pen(color);
				}

				charrom_data <<= 1;
			}
		}

		if (gr)
		{
			/* get new data for each line */
			videoram_addr += columns;
			videoram_addr += excess;
		}
		else
		{
			/* increase row counter */
			rc++;

			if (rc == (m_ucr ? 8 : 10))
			{
				/* reset row counter */
				rc = 0;

				videoram_addr += columns;
				videoram_addr += excess;
			}
		}
	}
}

UINT32 newbrain_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_tvp)
	{
		screen_update(bitmap, cliprect);
	}
	else
	{
		bitmap.fill(rgb_t::black, cliprect);
	}

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

static GFXDECODE_START( newbrain )
	GFXDECODE_ENTRY( "chargen", 0x0000, newbrain_charlayout, 0, 1 )
GFXDECODE_END

/* Machine Drivers */

MACHINE_CONFIG_FRAGMENT( newbrain_video )
	MCFG_DEFAULT_LAYOUT(layout_newbrain)

	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, RASTER, rgb_t::green)
	MCFG_SCREEN_UPDATE_DRIVER(newbrain_state, screen_update)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 250)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 249)

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", newbrain)
MACHINE_CONFIG_END
