// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Video Emulation for Midway T-unit, W-unit, and X-unit games.

**************************************************************************/

#include "emu.h"
#include "midtunit.h"
#include "midtunit.ipp"
#include "screen.h"
#include "midtview.ipp"

#include "debug/debugcon.h"
#include "debug/debugcmd.h"
#include "debugger.h"

#include "emuopts.h" // Used by PNG logging
#include "fileio.h" // Used by PNG logging
#include "png.h" // Used by PNG logging

#include <rapidjson/prettywriter.h> // Used by JSON logging
#include <rapidjson/stringbuffer.h> // Used by JSON logging

DEFINE_DEVICE_TYPE(MIDTUNIT_VIDEO, midtunit_video_device, "tunitvid", "Midway T-Unit Video")
DEFINE_DEVICE_TYPE(MIDWUNIT_VIDEO, midwunit_video_device, "wunitvid", "Midway W-Unit Video")
DEFINE_DEVICE_TYPE(MIDXUNIT_VIDEO, midxunit_video_device, "xunitvid", "Midway X-Unit Video")

/* compile-time options */
#define LOG_DMA             0       /* DMAs are logged if the 'L' key is pressed */

midtunit_video_device::midtunit_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_palette(*this, finder_base::DUMMY_TAG)
	, m_gfxrom(*this, finder_base::DUMMY_TAG)
#if DEBUG_MIDTUNIT_BLITTER
	, m_debug_palette(*this, "debugpalette")
#endif
{
}

midtunit_video_device::midtunit_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: midtunit_video_device(mconfig, MIDTUNIT_VIDEO, tag, owner, clock)
{
}

midwunit_video_device::midwunit_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: midtunit_video_device(mconfig, type, tag, owner, clock)
{
}

midwunit_video_device::midwunit_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: midtunit_video_device(mconfig, MIDWUNIT_VIDEO, tag, owner, clock)
{
}

midxunit_video_device::midxunit_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: midwunit_video_device(mconfig, MIDXUNIT_VIDEO, tag, owner, clock)
{
}

/*************************************
 *
 *  Debugger commands
 *
 *************************************/

void midtunit_video_device::debug_init()
{
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("midblit", CMDFLAG_CUSTOM_HELP, 1, 4, std::bind(&midtunit_video_device::debug_commands, this, _1));
	}
}

void midtunit_video_device::debug_commands(const std::vector<std::string> &params)
{
	if (params.size() < 1)
		return;

	if (params[0] == "pngdma")
		debug_png_dma_command(params);
	else
		debug_help_command(params);
}

void midtunit_video_device::debug_help_command(const std::vector<std::string> &params)
{
	debugger_console &con = machine().debugger().console();

	con.printf("Available Midway Blitter commands:\n");
	con.printf("  midblit pngdma,<enable>[,<path>][,<logjson>] -- Enable or disable dumping of DMA-drawn sprite PNGs to <path>, with or without JSON metadata\n");
	con.printf("  midblit help -- this list\n");
}

void midtunit_video_device::debug_png_dma_command(const std::vector<std::string> &params)
{
	debugger_console &con = machine().debugger().console();

	if (params.size() < 2)
	{
		con.printf("Error: not enough parameters for midblit pngdma command\n");
		return;
	}

	if (params.size() > 4)
	{
		con.printf("Error: too many parameters for midblit pngdma command\n");
		return;
	}

	bool old_state = m_log_png;
	bool new_state = false;
	if (!machine().debugger().commands().validate_boolean_parameter(params[1], new_state))
		return;

	if (!new_state)
	{
		if (params.size() > 2)
		{
			con.printf("Error: too many parameters for midblit pngdma command\n");
			return;
		}
		m_log_png = false;
		return;
	}

	if (params.size() < 3)
	{
		con.printf("Error: not enough parameters for midblit pngdma command\n");
		return;
	}

	if (params[2].empty() || params[2].length() > 2047)
	{
		con.printf("Error: invalid path parameter for midblit pngdma command\n");
		return;
	}

	strncpy(m_log_path, params[2].c_str(), 2047);

	if (params.size() == 4)
	{
		if (!machine().debugger().commands().validate_boolean_parameter(params[3], m_log_json))
			return;
	}

	m_log_png = new_state;

	if (!old_state && new_state)
	{
		m_logged_rom = make_unique_clear<uint64_t[]>(0x4000000);
	}
	else if (old_state && !new_state)
	{
		m_logged_rom.reset();
	}
}


/*************************************
 *
 *  Video startup
 *
 *************************************/

void midtunit_video_device::device_start()
{
	debug_init();

	/* allocate memory */
	m_local_videoram = std::make_unique<uint16_t[]>(0x100000/2);

#if DEBUG_MIDTUNIT_BLITTER
	m_debug_videoram = std::make_unique<uint16_t[]>(0x100000/2);
#endif

	m_logged_rom.reset();
	m_log_png = false;

	m_dma_timer = timer_alloc(TIMER_DMA);

	/* reset all the globals */
	m_gfxbank_offset[0] = 0x000000;
	m_gfxbank_offset[1] = 0x400000;

	memset(m_dma_register, 0, sizeof(m_dma_register));
	memset(&m_dma_state, 0, sizeof(dma_state));
	m_dma_state.gfxrom = m_gfxrom->base();

	/* register for state saving */
	save_item(NAME(m_midtunit_control));
	save_item(NAME(m_gfxbank_offset));
	save_pointer(NAME(m_local_videoram), 0x100000/sizeof(m_local_videoram[0]));
	save_item(NAME(m_videobank_select));
	save_item(NAME(m_dma_register));

	INIT_TEMPLATED_DMA_DRAW_GROUP(m_dma_draw_skip_scale,     true,  true);
	INIT_TEMPLATED_DMA_DRAW_GROUP(m_dma_draw_noskip_scale,   false, true);
	INIT_TEMPLATED_DMA_DRAW_GROUP(m_dma_draw_skip_noscale,   true,  false);
	INIT_TEMPLATED_DMA_DRAW_GROUP(m_dma_draw_noskip_noscale, false, false);

	m_gfx_rom_large = false;
}

void midwunit_video_device::device_start()
{
	midtunit_video_device::device_start();
	m_gfx_rom_large = true;
}

void midxunit_video_device::device_start()
{
	midtunit_video_device::device_start();
	m_gfx_rom_large = true;
	m_videobank_select = 1;
}



/*************************************
 *
 *  Banked graphics ROM access
 *
 *************************************/

uint16_t midtunit_video_device::midtunit_gfxrom_r(offs_t offset)
{
	uint8_t *base = m_gfxrom->base() + m_gfxbank_offset[(offset >> 21) & 1];
	offset = (offset & 0x01fffff) * 2;
	return base[offset] | (base[offset + 1] << 8);
}


uint16_t midwunit_video_device::midwunit_gfxrom_r(offs_t offset)
{
	uint8_t *base = m_gfxrom->base() + m_gfxbank_offset[0];
	offset *= 2;
	return base[offset] | (base[offset + 1] << 8);
}



/*************************************
 *
 *  Video RAM read/write
 *
 *************************************/

void midtunit_video_device::midtunit_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset *= 2;
	if (m_videobank_select)
	{
		if (ACCESSING_BITS_0_7)
			m_local_videoram[offset] = (data & 0xff) | ((m_dma_register[DMA_PALETTE] & 0xff) << 8);
		if (ACCESSING_BITS_8_15)
			m_local_videoram[offset + 1] = ((data >> 8) & 0xff) | (m_dma_register[DMA_PALETTE] & 0xff00);
	}
	else
	{
		if (ACCESSING_BITS_0_7)
			m_local_videoram[offset] = (m_local_videoram[offset] & 0xff) | ((data & 0xff) << 8);
		if (ACCESSING_BITS_8_15)
			m_local_videoram[offset + 1] = (m_local_videoram[offset + 1] & 0xff) | (data & 0xff00);
	}
}


void midtunit_video_device::midtunit_vram_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset *= 2;
	if (ACCESSING_BITS_0_7)
		m_local_videoram[offset] = (data & 0xff) | ((m_dma_register[DMA_PALETTE] & 0xff) << 8);
	if (ACCESSING_BITS_8_15)
		m_local_videoram[offset + 1] = ((data >> 8) & 0xff) | (m_dma_register[DMA_PALETTE] & 0xff00);
}


void midtunit_video_device::midtunit_vram_color_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset *= 2;
	if (ACCESSING_BITS_0_7)
		m_local_videoram[offset] = (m_local_videoram[offset] & 0xff) | ((data & 0xff) << 8);
	if (ACCESSING_BITS_8_15)
		m_local_videoram[offset + 1] = (m_local_videoram[offset + 1] & 0xff) | (data & 0xff00);
}


uint16_t midtunit_video_device::midtunit_vram_r(offs_t offset)
{
	offset *= 2;
	if (m_videobank_select)
		return (m_local_videoram[offset] & 0x00ff) | (m_local_videoram[offset + 1] << 8);
	else
		return (m_local_videoram[offset] >> 8) | (m_local_videoram[offset + 1] & 0xff00);
}


uint16_t midtunit_video_device::midtunit_vram_data_r(offs_t offset)
{
	offset *= 2;
	return (m_local_videoram[offset] & 0x00ff) | (m_local_videoram[offset + 1] << 8);
}


uint16_t midtunit_video_device::midtunit_vram_color_r(offs_t offset)
{
	offset *= 2;
	return (m_local_videoram[offset] >> 8) | (m_local_videoram[offset + 1] & 0xff00);
}



/*************************************
 *
 *  Shift register read/write
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(midtunit_video_device::to_shiftreg)
{
	memcpy(shiftreg, &m_local_videoram[address >> 3], 2 * 512 * sizeof(uint16_t));
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(midtunit_video_device::from_shiftreg)
{
	memcpy(&m_local_videoram[address >> 3], shiftreg, 2 * 512 * sizeof(uint16_t));
}



/*************************************
 *
 *  Control register
 *
 *************************************/

void midtunit_video_device::midtunit_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	    other important bits:
	        bit 2 (0x0004) is toggled periodically
	*/
	logerror("T-unit control = %04X\n", data);

	COMBINE_DATA(&m_midtunit_control);

	/* gfx bank select is bit 7 */
	if (!(m_midtunit_control & 0x0080) || !m_gfx_rom_large)
		m_gfxbank_offset[0] = 0x000000;
	else
		m_gfxbank_offset[0] = 0x800000;

	/* video bank select is bit 5 */
	m_videobank_select = (m_midtunit_control >> 5) & 1;
}


void midwunit_video_device::midwunit_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	    other important bits:
	        bit 2 (0x0004) is toggled periodically
	*/
	logerror("Wolf-unit control = %04X\n", data);

	COMBINE_DATA(&m_midtunit_control);

	/* gfx bank select is bits 8-9 */
	m_gfxbank_offset[0] = 0x800000 * ((m_midtunit_control >> 8) & 3);

	/* video bank select is unknown */
	m_videobank_select = (m_midtunit_control >> 11) & 1;
}


uint16_t midwunit_video_device::midwunit_control_r()
{
	return m_midtunit_control;
}



/*************************************
 *
 *  Palette handlers
 *
 *************************************/

void midxunit_video_device::midxunit_paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!(offset & 1))
		m_palette->write16(offset / 2, data, mem_mask);
}


uint16_t midxunit_video_device::midxunit_paletteram_r(offs_t offset)
{
	return m_palette->read16(offset / 2);
}



/*************************************
 *
 *  DMA drawing routines
 *
 *************************************/

/*** fast pixel extractors ***/
#if !defined(ALIGN_SHORTS) && defined(LSB_FIRST)
#define EXTRACTGEN(m)   ((*(uint16_t *)&base[o >> 3] >> (o & 7)) & (m))
#elif defined(powerc)
#define EXTRACTGEN(m)   ((__lhbrx(base, o >> 3) >> (o & 7)) & (m))
#else
#define EXTRACTGEN(m)   (((base[o >> 3] | (base[(o >> 3) + 1] << 8)) >> (o & 7)) & (m))
#endif

/*** core blitter routine macro ***/
template <int BitsPerPixel, bool XFlip, bool Skip, bool Scale, midtunit_video_device::op_type_t Zero, midtunit_video_device::op_type_t NonZero>
void midtunit_video_device::dma_draw()
{
	int height = m_dma_state.height << 8;
	uint8_t *base = m_dma_state.gfxrom;
	uint32_t offset = m_dma_state.offset;
	uint16_t pal = m_dma_state.palette;
	uint16_t color = pal | m_dma_state.color;
	int sy = m_dma_state.ypos;
	int iy = 0;
	int ty;
	int mask = (1 << BitsPerPixel) - 1;
	int xstep = Scale ? m_dma_state.xstep : 0x100;

	/* loop over the height */
	while (iy < height)
	{
		int startskip = m_dma_state.startskip << 8;
		[[maybe_unused]] int endskip = m_dma_state.endskip << 8;
		int width = m_dma_state.width << 8;
		int sx = m_dma_state.xpos;
		int ix = 0;
		int tx;
		uint32_t o = offset;
		int pre, post;
		uint16_t *d;

		/* handle skipping */
		if (Skip)
		{
			uint8_t value = EXTRACTGEN(0xff);
			o += 8;

			/* adjust for preskip */
			pre = (value & 0x0f) << (m_dma_state.preskip + 8);
			tx = pre / xstep;
			if (XFlip)
				sx = (sx - tx) & XPOSMASK;
			else
				sx = (sx + tx) & XPOSMASK;
			ix += tx * xstep;

			/* adjust for postskip */
			post = ((value >> 4) & 0x0f) << (m_dma_state.postskip + 8);
			width -= post;
			endskip -= post;
		}

		/* handle Y clipping */
		if (sy < m_dma_state.topclip || sy > m_dma_state.botclip)
			goto clipy;

		/* handle start skip */
		if (ix < startskip)
		{
			tx = ((startskip - ix) / xstep) * xstep;
			ix += tx;
			o += (tx >> 8) * BitsPerPixel;
		}

		/* handle end skip */
		if ((width >> 8) > m_dma_state.width - m_dma_state.endskip)
			width = (m_dma_state.width - m_dma_state.endskip) << 8;

		/* determine destination pointer */
#if DEBUG_MIDTUNIT_BLITTER
		d = m_doing_debug_dma ? &m_debug_videoram[sy * 512] : &m_local_videoram[sy * 512];
#else
		d = &m_local_videoram[sy * 512];
#endif

		/* loop until we draw the entire width */
		while (ix < width)
		{
			/* only process if not clipped */
			if (sx >= m_dma_state.leftclip && sx <= m_dma_state.rightclip)
			{
				/* special case similar handling of zero/non-zero */
				if (Zero == NonZero)
				{
					if (Zero == PIXEL_COLOR)
						d[sx] = color;
					else if (Zero == PIXEL_COPY)
						d[sx] = (EXTRACTGEN(mask)) | pal;
				}

				/* otherwise, read the pixel and look */
				else
				{
					int pixel = (EXTRACTGEN(mask));

					/* non-zero pixel case */
					if (pixel)
					{
						if (NonZero == PIXEL_COLOR)
							d[sx] = color;
						else if (NonZero == PIXEL_COPY)
							d[sx] = pixel | pal;
					}

					/* zero pixel case */
					else
					{
						if (Zero == PIXEL_COLOR)
							d[sx] = color;
						else if (Zero == PIXEL_COPY)
							d[sx] = pal;
					}
				}
			}

			/* update pointers */
			if (XFlip)
				sx = (sx - 1) & XPOSMASK;
			else
				sx = (sx + 1) & XPOSMASK;

			/* advance to the next pixel */
			if (!Scale)
			{
				ix += 0x100;
				o += BitsPerPixel;
			}
			else
			{
				tx = ix >> 8;
				ix += xstep;
				tx = (ix >> 8) - tx;
				o += BitsPerPixel * tx;
			}
		}

	clipy:
		/* advance to the next row */
		if (m_dma_state.yflip)
			sy = (sy - 1) & YPOSMASK;
		else
			sy = (sy + 1) & YPOSMASK;
		if (!Scale)
		{
			iy += 0x100;
			width = m_dma_state.width;
			if (Skip)
			{
				offset += 8;
				width -= (pre + post) >> 8;
				if (width > 0) offset += width * BitsPerPixel;
			}
			else
			{
				offset += width * BitsPerPixel;
			}
		}
		else
		{
			ty = iy >> 8;
			iy += m_dma_state.ystep;
			ty = (iy >> 8) - ty;
			if (!Skip)
			{
				offset += ty * m_dma_state.width * BitsPerPixel;
			}
			else if (ty--)
			{
				o = offset + 8;
				width = m_dma_state.width - ((pre + post) >> 8);
				if (width > 0) o += width * BitsPerPixel;
				while (ty--)
				{
					uint8_t value = EXTRACTGEN(0xff);
					o += 8;
					pre = (value & 0x0f) << m_dma_state.preskip;
					post = ((value >> 4) & 0x0f) << m_dma_state.postskip;
					width = m_dma_state.width - pre - post;
					if (width > 0) o += width * BitsPerPixel;
				}
				offset = o;
			}
		}
	}
}

DEFINE_TEMPLATED_DMA_DRAW_GROUP(true,  true);
DEFINE_TEMPLATED_DMA_DRAW_GROUP(false, true);
DEFINE_TEMPLATED_DMA_DRAW_GROUP(true,  false);
DEFINE_TEMPLATED_DMA_DRAW_GROUP(false, false);


/*************************************
 *
 *  DMA finished callback
 *
 *************************************/

void midtunit_video_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_DMA:
		m_dma_register[DMA_COMMAND] &= ~0x8000; /* tell the cpu we're done */
		m_maincpu->set_input_line(0, ASSERT_LINE);
		break;
	default:
		throw emu_fatalerror("Unknown id in midtunit_video_device::device_timer");
	}
}



/*************************************
 *
 *  DMA reader
 *
 *************************************/

uint16_t midtunit_video_device::midtunit_dma_r(offs_t offset)
{
	/* rmpgwt sometimes reads register 0, expecting it to return the */
	/* current DMA status; thus we map register 0 to register 1 */
	/* openice does it as well */
	if (offset == 0)
		offset = 1;
	return m_dma_register[offset];
}



/*************************************
 *
 *  DMA write handler
 *
 *************************************/

/*
 * DMA registers
 * ------------------
 *
 *  Register | Bit              | Use
 * ----------+-FEDCBA9876543210-+------------
 *     0     | xxxxxxxx-------- | pixels to drop at the start of each row
 *           | --------xxxxxxxx | pixels to drop at the end of each row
 *     1     | x--------------- | trigger write (or clear if zero)
 *           | -421------------ | image bpp (0=8)
 *           | ----84---------- | post skip size = (1<<x)
 *           | ------21-------- | pre skip size = (1<<x)
 *           | --------8------- | pre/post skip enable
 *           | ---------4------ | clipping enable
 *           | ----------2----- | flip y
 *           | -----------1---- | flip x
 *           | ------------8--- | blit nonzero pixels as color
 *           | -------------4-- | blit zero pixels as color
 *           | --------------2- | blit nonzero pixels
 *           | ---------------1 | blit zero pixels
 *     2     | xxxxxxxxxxxxxxxx | source address low word
 *     3     | xxxxxxxxxxxxxxxx | source address high word
 *     4     | -------xxxxxxxxx | detination x
 *     5     | -------xxxxxxxxx | destination y
 *     6     | ------xxxxxxxxxx | image columns
 *     7     | ------xxxxxxxxxx | image rows
 *     8     | xxxxxxxxxxxxxxxx | palette
 *     9     | xxxxxxxxxxxxxxxx | color
 *    10     | ---xxxxxxxxxxxxx | scale x
 *    11     | ---xxxxxxxxxxxxx | scale y
 *    12     | -------xxxxxxxxx | top/left clip
 *    13     | -------xxxxxxxxx | bottom/right clip
 *    14     | ---------------- | test
 *    15     | xxxxxxxx-------- | zero detect byte
 *           | --------8------- | extra page
 *           | ---------4------ | destination size
 *           | ----------2----- | select top/bottom or left/right for reg 12/13
 */

void midtunit_video_device::midtunit_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	static const uint8_t register_map[2][16] =
	{
		{ 0,1,2,3,4,5,6,7,8,9,10,11,16,17,14,15 },
		{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 }
	};
	int regbank = (m_dma_register[DMA_CONFIG] >> 5) & 1;
	int pixels = 0;

	/* blend with the current register contents */
	int regnum = register_map[regbank][offset];
	COMBINE_DATA(&m_dma_register[regnum]);

	/* only writes to DMA_COMMAND actually cause actions */
	if (regnum != DMA_COMMAND)
		return;

	/* high bit triggers action */
	int command = m_dma_register[DMA_COMMAND];
	m_maincpu->set_input_line(0, CLEAR_LINE);
	if (!(command & 0x8000))
		return;

	g_profiler.start(PROFILER_USER1);

	/* determine bpp */
	int bpp = (command >> 12) & 7;

	/* fill in the basic data */
	m_dma_state.xpos = m_dma_register[DMA_XSTART] & XPOSMASK;
	m_dma_state.ypos = m_dma_register[DMA_YSTART] & YPOSMASK;
	m_dma_state.width = m_dma_register[DMA_WIDTH] & 0x3ff;
	m_dma_state.height = m_dma_register[DMA_HEIGHT] & 0x3ff;
	m_dma_state.palette = m_dma_register[DMA_PALETTE] & 0x7f00;
	m_dma_state.color = m_dma_register[DMA_COLOR] & 0xff;

	/* fill in the rev 2 data */
	m_dma_state.yflip = (command & 0x20) >> 5;
	m_dma_state.preskip = (command >> 8) & 3;
	m_dma_state.postskip = (command >> 10) & 3;
	m_dma_state.xstep = m_dma_register[DMA_SCALE_X] ? m_dma_register[DMA_SCALE_X] : 0x100;
	m_dma_state.ystep = m_dma_register[DMA_SCALE_Y] ? m_dma_register[DMA_SCALE_Y] : 0x100;

	/* clip the clippers */
	m_dma_state.topclip = m_dma_register[DMA_TOPCLIP] & 0x1ff;
	m_dma_state.botclip = m_dma_register[DMA_BOTCLIP] & 0x1ff;
	m_dma_state.leftclip = m_dma_register[DMA_LEFTCLIP] & 0x3ff;
	m_dma_state.rightclip = m_dma_register[DMA_RIGHTCLIP] & 0x3ff;

	/* determine the offset */
	uint32_t gfxoffset = m_dma_register[DMA_OFFSETLO] | (m_dma_register[DMA_OFFSETHI] << 16);

	/* special case: drawing mode C doesn't need to know about any pixel data */
	if ((command & 0x0f) == 0x0c)
		gfxoffset = 0;

	/* determine the location */
	if (!m_gfx_rom_large && gfxoffset >= 0x2000000)
		gfxoffset -= 0x2000000;
	if (gfxoffset >= 0xf8000000)
		gfxoffset -= 0xf8000000;
	if (gfxoffset < 0x10000000)
		m_dma_state.offset = gfxoffset;
	else
	{
		logerror("DMA source out of range: %08X\n", gfxoffset);
		goto skipdma;
	}

	if (LOG_DMA || DEBUG_MIDTUNIT_BLITTER)
	{
		if (machine().input().code_pressed(KEYCODE_COLON))
		{
			logerror("DMA command %04X: (bpp=%d skip=%d xflip=%d yflip=%d preskip=%d postskip=%d)\n",
					command, (command >> 12) & 7, (command >> 7) & 1, (command >> 4) & 1, (command >> 5) & 1, (command >> 8) & 3, (command >> 10) & 3);
			logerror("  offset=%08X pos=(%d,%d) w=%d h=%d clip=(%d,%d)-(%d,%d)\n", gfxoffset, m_dma_state.xpos, m_dma_state.ypos,
					m_dma_state.width, m_dma_state.height, m_dma_state.leftclip, m_dma_state.topclip, m_dma_state.rightclip, m_dma_state.botclip);
			logerror("  palette=%04X color=%04X lskip=%02X rskip=%02X xstep=%04X ystep=%04X test=%04X config=%04X\n",
					m_dma_register[DMA_PALETTE], m_dma_register[DMA_COLOR],
					m_dma_register[DMA_LRSKIP] >> 8, m_dma_register[DMA_LRSKIP] & 0xff,
					m_dma_register[DMA_SCALE_X], m_dma_register[DMA_SCALE_Y], m_dma_register[DMA_UNKNOWN_E],
					m_dma_register[DMA_CONFIG]);
			logerror("----\n");
		}
	}

	/* there seems to be two types of behavior for the DMA chip */
	/* for MK1 and MK2, the upper byte of the LRSKIP is the     */
	/* starting skip value, and the lower byte is the ending    */
	/* skip value; for the NBA Jam, Hangtime, and Open Ice, the */
	/* full word seems to be the starting skip value.           */
	if (command & 0x40)
	{
		m_dma_state.startskip = m_dma_register[DMA_LRSKIP] & 0xff;
		m_dma_state.endskip = m_dma_register[DMA_LRSKIP] >> 8;
	}
	else
	{
		m_dma_state.startskip = 0;
		m_dma_state.endskip = m_dma_register[DMA_LRSKIP];
	}

	if (m_log_png)
	{
		if (command & 0x80)
		{
			log_bitmap(command, bpp ? bpp : 8, true);
		}
		else
		{
			log_bitmap(command, bpp ? bpp : 8, false);
		}
	}

	/* then draw */
	if (m_dma_state.xstep == 0x100 && m_dma_state.ystep == 0x100)
	{
		if (command & 0x80)
			((this)->*(m_dma_draw_skip_noscale[(command & 0x1f)*8 + bpp]))();
		else
			((this)->*(m_dma_draw_noskip_noscale[(command & 0x1f)*8 + bpp]))();

		pixels = m_dma_state.width * m_dma_state.height;
	}
	else
	{
		if (command & 0x80)
			((this)->*(m_dma_draw_skip_scale[(command & 0x1f)*8 + bpp]))();
		else
			((this)->*(m_dma_draw_noskip_scale[(command & 0x1f)*8 + bpp]))();

		if (m_dma_state.xstep && m_dma_state.ystep)
			pixels = ((m_dma_state.width << 8) / m_dma_state.xstep) * ((m_dma_state.height << 8) / m_dma_state.ystep);
		else
			pixels = 0;
	}

	/* signal we're done */
skipdma:
	m_dma_timer->adjust(attotime::from_nsec(41 * pixels));

	g_profiler.stop();
}



/*************************************
 *
 *  Core refresh routine
 *
 *************************************/

TMS340X0_SCANLINE_IND16_CB_MEMBER(midtunit_video_device::scanline_update)
{
	uint16_t const *const src = &m_local_videoram[(params->rowaddr << 9) & 0x3fe00];
	uint16_t *const dest = &bitmap.pix(scanline);
	int coladdr = params->coladdr << 1;

	/* copy the non-blanked portions of this scanline */
	for (int x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = src[coladdr++ & 0x1ff] & 0x7fff;
}

TMS340X0_SCANLINE_IND16_CB_MEMBER(midxunit_video_device::scanline_update)
{
	uint32_t fulladdr = ((params->rowaddr << 16) | params->coladdr) >> 3;
	uint16_t const *const src = &m_local_videoram[fulladdr & 0x3fe00];
	uint16_t *const dest = &bitmap.pix(scanline);

	/* copy the non-blanked portions of this scanline */
	for (int x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = src[fulladdr++ & 0x1ff] & 0x7fff;
}

void midtunit_video_device::log_bitmap(int command, int bpp, bool Skip)
{
	const uint32_t raw_offset = m_dma_register[DMA_OFFSETLO] | (m_dma_register[DMA_OFFSETHI] << 16);
	if (m_logged_rom[raw_offset >> 6] & (1ULL << (raw_offset & 0x3f)))
		return;

	int Zero = PIXEL_SKIP;
	int NonZero = PIXEL_SKIP;
	switch (command & 0xf)
	{
	case 1:  Zero = PIXEL_COPY;  NonZero = PIXEL_SKIP;  break;
	case 2:  Zero = PIXEL_SKIP;  NonZero = PIXEL_COPY;  break;
	case 3:  Zero = PIXEL_COPY;  NonZero = PIXEL_COPY;  break;
	case 4:  Zero = PIXEL_COLOR; NonZero = PIXEL_SKIP;  break;
	case 5:  Zero = PIXEL_COLOR; NonZero = PIXEL_SKIP;  break;
	case 6:  Zero = PIXEL_COLOR; NonZero = PIXEL_COPY;  break;
	case 7:  Zero = PIXEL_COLOR; NonZero = PIXEL_COPY;  break;
	case 8:  Zero = PIXEL_SKIP;  NonZero = PIXEL_COLOR; break;
	case 9:  Zero = PIXEL_COPY;  NonZero = PIXEL_COLOR; break;
	case 10: Zero = PIXEL_SKIP;  NonZero = PIXEL_COLOR; break;
	case 11: Zero = PIXEL_COPY;  NonZero = PIXEL_COLOR; break;
	case 12: Zero = PIXEL_COLOR; NonZero = PIXEL_COLOR; break;
	case 13: Zero = PIXEL_COLOR; NonZero = PIXEL_COLOR; break;
	case 14: Zero = PIXEL_COLOR; NonZero = PIXEL_COLOR; break;
	case 15: Zero = PIXEL_COLOR; NonZero = PIXEL_COLOR; break;
	default: return;
	}

	emu_file file(m_log_path, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);

	char name_buf[256];
	snprintf(name_buf, 255, "0x%08x.png", raw_offset);
	auto const filerr = file.open(name_buf);
	if (filerr)
	{
		return;
	}

	m_logged_rom[raw_offset >> 6] |= 1ULL << (raw_offset & 0x3f);

	m_log_bitmap.allocate(m_dma_state.width, m_dma_state.height);
	m_log_bitmap.fill(0);

	uint8_t *base = m_dma_state.gfxrom;
	uint32_t offset = m_dma_state.offset;
	uint16_t pal = m_dma_state.palette;
	uint16_t color = pal | m_dma_state.color;
	int mask = (1 << bpp) - 1;

	/* loop over the height */
	for (int y = 0; y < m_dma_state.height; y++)
	{
		int startskip = m_dma_state.startskip;
		[[maybe_unused]] int endskip = m_dma_state.endskip;
		int width = m_dma_state.width;
		int ix = 0;
		int tx;
		uint32_t o = offset;
		int pre = 0, post = 0;

		/* handle skipping */
		if (Skip)
		{
			uint8_t value = EXTRACTGEN(0xff);
			o += 8;

			/* adjust for preskip */
			pre = (value & 0x0f) << m_dma_state.preskip;
			tx = pre;
			ix += tx;

			/* adjust for postskip */
			post = ((value >> 4) & 0x0f) << m_dma_state.postskip;
			width -= post;
			endskip -= post;
		}

		/* handle start skip */
		if (ix < startskip)
		{
			tx = (startskip - ix);
			ix += tx;
			o += tx * bpp;
		}

		/* handle end skip */
		if (width > m_dma_state.width - m_dma_state.endskip)
			width = m_dma_state.width - m_dma_state.endskip;

		bitmap_rgb32::pixel_t *d = &m_log_bitmap.pix(y, ix);

		/* determine destination pointer */

		/* loop until we draw the entire width */
		while (ix < width)
		{
			/* special case similar handling of zero/non-zero */
			if (Zero == NonZero)
			{
				if (Zero == PIXEL_COLOR)
					*d = m_palette->palette()->entry_list_raw()[color];
				else if (Zero == PIXEL_COPY)
					*d = m_palette->palette()->entry_list_raw()[(EXTRACTGEN(mask)) | pal];
			}

			/* otherwise, read the pixel and look */
			else
			{
				int pixel = (EXTRACTGEN(mask));

				/* non-zero pixel case */
				if (pixel)
				{
					if (NonZero == PIXEL_COLOR)
						*d = m_palette->palette()->entry_list_raw()[color];
					else if (NonZero == PIXEL_COPY)
						*d = m_palette->palette()->entry_list_raw()[pixel | pal];
				}

				/* zero pixel case */
				else
				{
					if (Zero == PIXEL_COLOR)
						*d = m_palette->palette()->entry_list_raw()[color];
					else if (Zero == PIXEL_COPY)
						*d = m_palette->palette()->entry_list_raw()[pal];
				}
			}

			/* advance to the next pixel */
			ix++;
			d++;
			o += bpp;
		}

		/* advance to the next row */
		width = m_dma_state.width;
		if (Skip)
		{
			offset += 8;
			width -= pre + post;
			if (width > 0) offset += width * bpp;
		}
		else
		{
			offset += width * bpp;
		}
	}

	util::png_write_bitmap(file, nullptr, m_log_bitmap, 0, nullptr);

	if (m_log_json)
	{
		char hex_buf[11];
		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
		emu_file json(m_log_path, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);

		snprintf(name_buf, 255, "0x%08x.json", raw_offset);
		auto const jsonerr = json.open(name_buf);
		if (jsonerr)
		{
			return;
		}

		writer.StartObject();
		writer.Key("DMAState");
		writer.StartObject();

		sprintf(hex_buf, "0x%08x", raw_offset);
		writer.Key("MemoryAddress");
		writer.String(hex_buf);

		sprintf(hex_buf, "0x%08x", m_dma_state.offset >> 3);
		writer.Key("ROMSourceOffsetByte");
		writer.String(hex_buf);

		writer.Key("ROMSourceOffsetBit");
		writer.Int(m_dma_state.offset & 7);

		writer.Key("Size");
		writer.StartArray();
		writer.Int(m_dma_state.width);
		writer.Int(m_dma_state.height);
		writer.EndArray();

		writer.Key("BitsPerPixel");
		writer.Uint(bpp);

		writer.Key("PaletteBank");
		writer.Uint(m_dma_state.palette >> 8);

		writer.Key("FGColor");
		writer.Uint(m_dma_state.color);

		writer.Key("YFlip");
		writer.Bool(m_dma_state.yflip ? true : false);

		writer.Key("PreSkipScale");
		writer.Uint(m_dma_state.preskip);

		writer.Key("PostSkipScale");
		writer.Uint(m_dma_state.postskip);

		writer.Key("RowSkipBits");
		writer.Int(m_dma_state.rowbits);

		writer.Key("StartPixelsToSkip");
		writer.Int(m_dma_state.startskip);

		writer.Key("EndPixelsToSkip");
		writer.Int(m_dma_state.endskip);

		writer.EndObject();
		writer.EndObject();

		json.puts(s.GetString());
		json.close();
	}
}
