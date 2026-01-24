// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia, MetalliC
// emulation of Altera Cyclone EP1C12 FPGA programmed as a blitter

#include "emu.h"
#include "ep1c12.h"

#include "screen.h"

#define LOG_DEBUG     (1U << 1)

//#define VERBOSE (LOG_DEBUG)
#include "logmacro.h"

#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


DEFINE_DEVICE_TYPE(EP1C12, ep1c12_device, "ep1c12", "EP1C12 Blitter")

static constexpr int EP1C_VRAM_CLK_NANOSEC = 13;
static constexpr int EP1C_SRAM_CLK_NANOSEC = 20;
static constexpr int EP1C_VRAM_H_LINE_PERIOD_NANOSEC = 63600;
static constexpr int EP1C_VRAM_H_LINE_DURATION_NANOSEC = 2160;
static constexpr int EP1C_FRAME_DURATION_NANOSEC = 16666666;
static constexpr int EP1C_DRAW_OPERATION_SIZE_BYTES = 20;
static constexpr int EP1C_CLIP_OPERATION_SIZE_BYTES = 2;

// When looking at VRAM viewer in Special mode in Muchi Muchi Pork, draws 32 pixels outside of
// the "clip area" is visible. This is likely why the frame buffers will have at least a 32 pixel offset
// from the VRAM borders or other buffers in all games.
static constexpr int EP1C_CLIP_MARGIN = 32;

ep1c12_device::ep1c12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, EP1C12, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_ram16(nullptr), m_gfx_size(0), m_bitmaps(nullptr), m_use_ram(nullptr)
	, m_main_ramsize(0), m_main_rammask(0), m_ram16_copy(nullptr), m_work_queue(nullptr)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_port_r_cb(*this, 0)
{
	m_blitter_request = nullptr;
	m_blitter_delay_timer = nullptr;
	m_blitter_busy = 0;
	m_gfx_addr = 0;
	m_gfx_scroll_x = 0;
	m_gfx_scroll_y = 0;
	m_gfx_clip_x = 0;
	m_gfx_clip_y = 0;
	m_gfx_addr_shadowcopy = 0;
	m_gfx_clip_x_shadowcopy = 0;
	m_gfx_clip_y_shadowcopy = 0;
	m_blit_delay_ns = 0;
	m_blit_idle_op_bytes = 0;
}

TIMER_CALLBACK_MEMBER(ep1c12_device::blitter_delay_callback)
{
	m_blitter_busy = 0;
}


void ep1c12_device::device_start()
{
	m_gfx_size = 0x2000 * 0x1000;
	m_bitmaps = std::make_unique<bitmap_rgb32>(0x2000, 0x1000);
	m_clip = m_bitmaps->cliprect();
	m_clip.set(0, 0x2000 - 1, 0, 0x1000 - 1);

#if DEBUG_VRAM_VIEWER
	m_debug_vram_view_en = false;
	m_prev_screen_width = m_curr_screen_width = screen().width();
	m_prev_screen_height = m_curr_screen_height = screen().height();
	m_prev_screen_visarea = m_curr_screen_visarea = screen().visible_area();
#endif

	m_ram16_copy = std::make_unique<u16[]>(m_main_ramsize / 2);

	m_blitter_delay_timer = timer_alloc(FUNC(ep1c12_device::blitter_delay_callback), this);
	m_blitter_delay_timer->adjust(attotime::never);

	m_firmware_pos = 0;
	m_firmware.clear();
	m_firmware.resize(290405, 0);
	m_firmware_port = 0;
	m_firmware_version = -1;

	save_item(NAME(m_gfx_addr));
	save_item(NAME(m_gfx_scroll_x));
	save_item(NAME(m_gfx_scroll_y));
	save_item(NAME(m_gfx_clip_x));
	save_item(NAME(m_gfx_clip_y));
	save_item(NAME(m_gfx_addr_shadowcopy));
	save_item(NAME(m_gfx_clip_x_shadowcopy));
	save_item(NAME(m_gfx_clip_y_shadowcopy));
	save_pointer(NAME(m_ram16_copy), m_main_ramsize/2);
	save_item(NAME(*m_bitmaps));
	save_item(NAME(m_firmware_pos));
	save_item(NAME(m_firmware_port));
	save_item(NAME(m_firmware));
	save_item(NAME(m_firmware_version));
	save_item(NAME(m_blit_delay_ns));
	save_item(NAME(m_blit_idle_op_bytes));
}

void ep1c12_device::device_reset()
{
	m_use_ram = m_ram16_copy.get();
	m_work_queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_HIGH_FREQ);

	// cache table to avoid divides in blit code, also pre-clamped
	for (int y = 0; y < 0x40; y++)
	{
		for (int x = 0; x < 0x20; x++)
		{
			colrtable[x][y] = std::min((x * y) / 0x1f, 0x1f);
			colrtable_rev[x ^ 0x1f][y] = std::min((x * y) / 0x1f, 0x1f);
		}
	}

	// preclamped add table
	for (int y = 0; y < 0x20; y++)
	{
		for (int x = 0; x < 0x20; x++)
		{
			colrtable_add[x][y] = std::min((x + y), 0x1f);
		}
	}

	m_blitter_busy = 0;
}

// TODO: get these into the device class without ruining performance
u8 ep1c12_device::colrtable[0x20][0x40];
u8 ep1c12_device::colrtable_rev[0x20][0x40];
u8 ep1c12_device::colrtable_add[0x20][0x20];

inline u16 ep1c12_device::READ_NEXT_WORD(offs_t *addr)
{
//  u16 data = space.read_word(*addr); // going through the memory system is 'more correct' but noticeably slower
	const u16 data = m_use_ram[((*addr & m_main_rammask) >> 1) ^ NATIVE_ENDIAN_VALUE_LE_BE(3, 0)];

	*addr += 2;

	return data;
}

inline u16 ep1c12_device::COPY_NEXT_WORD(address_space &space, offs_t *addr)
{
//  u16 data = space.read_word(*addr); // going through the memory system is 'more correct' but noticeably slower
	const u16 data = m_ram16[((*addr & m_main_rammask) >> 1) ^ NATIVE_ENDIAN_VALUE_LE_BE(3, 0)];
	m_ram16_copy[((*addr & m_main_rammask) >> 1) ^ NATIVE_ENDIAN_VALUE_LE_BE(3, 0)] = data;

	*addr += 2;

	return data;
}

/*
    Upload command
    This command uploads gfx data to VRAM, from Main CPU RAM.

    Offset Bits              Description
           fedcba98 76543210
    00     0010---- -------- 0x2 for upload
           ----0000 00000000 Fixed for upload?
    02     00000000 00000000 ""
    04     10011001 10011001 ""
    06     10011001 10011001 ""
    08     ---xxxxx xxxxxxxx Destination X start position
    0a     ----xxxx xxxxxxxx Destination Y start position
    0c     ---xxxxx xxxxxxxx Source Width
    0e     ----xxxx xxxxxxxx Source Height
    10...10 + (Width * Height * 2) Source GFX data (ARGB1555 format)
*/

inline void ep1c12_device::gfx_upload_shadow_copy(address_space &space, offs_t *addr)
{
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);

	const u32 dimx = (COPY_NEXT_WORD(space, addr) & 0x1fff) + 1;
	const u32 dimy = (COPY_NEXT_WORD(space, addr) & 0x0fff) + 1;

	for (u32 y = 0; y < dimy; y++)
	{
		for (u32 x = 0; x < dimx; x++)
		{
			COPY_NEXT_WORD(space, addr);
		}
	}

	// Time spent on uploads is mostly due to Main RAM accesses.
	// The Blitter will send BREQ requests to the SH-3, to access Main RAM
	// and then write it to VRAM.
	// The number of bytes to read are the sum of a 16b fixed header and the pixel
	// data (2 byte per pixel). RAM accesses are 32bit, so divide by four for clocks.
	//
	// TODO: There's additional overhead to these request thats are not included. The BREQ
	// assertion also puts CPU into WAIT, if it needs uncached RAM accesses.
	int num_sram_clk = (16 + dimx * dimy * 2 ) / 4;
	m_blit_delay_ns += num_sram_clk * EP1C_SRAM_CLK_NANOSEC;
	m_blit_idle_op_bytes = 0;
}

inline void ep1c12_device::gfx_upload(offs_t *addr)
{
	// 0x20000000
	READ_NEXT_WORD(addr);
	READ_NEXT_WORD(addr);

	// 0x99999999
	READ_NEXT_WORD(addr);
	READ_NEXT_WORD(addr);

	u32 dst_x_start = READ_NEXT_WORD(addr);
	u32 dst_y_start = READ_NEXT_WORD(addr);

	u32 dst_p = 0;
	dst_x_start &= 0x1fff;
	dst_y_start &= 0x0fff;

	const u32 dimx = (READ_NEXT_WORD(addr) & 0x1fff) + 1;
	const u32 dimy = (READ_NEXT_WORD(addr) & 0x0fff) + 1;

	logerror("GFX COPY: DST %02X,%02X,%03X DIM %02X,%03X\n", dst_p,dst_x_start,dst_y_start, dimx,dimy);

	for (u32 y = 0; y < dimy; y++)
	{
		u32 *dst = &m_bitmaps->pix(dst_y_start + y, 0);
		dst += dst_x_start;

		for (u32 x = 0; x < dimx; x++)
		{
			const u16 pendat = READ_NEXT_WORD(addr);
			// real hw would upload the gfxword directly, but our VRAM is 32-bit, so convert it.
			//dst[dst_x_start + x] = pendat;
			*dst++ = ((pendat & 0x8000) << 14) | ((pendat & 0x7c00) << 9) | ((pendat & 0x03e0) << 6) | ((pendat & 0x001f) << 3);  // --t- ---- rrrr r--- gggg g--- bbbb b---  format
			//dst[dst_x_start + x] = ((pendat & 0x8000) << 14) | ((pendat & 0x7c00) << 6) | ((pendat & 0x03e0) << 3) | ((pendat & 0x001f) << 0);  // --t- ---- ---r rrrr ---g gggg ---b bbbb  format
		}
	}
}

#define draw_params m_bitmaps.get(), &m_clip, &m_bitmaps->pix(0,0),src_x,src_y, x,y, dimx,dimy, flipy, s_alpha, d_alpha, &tint_clr


const ep1c12_device::blitfunction ep1c12_device::f0_ti1_tr1_blit_funcs[64] =
{
	ep1c12_device::draw_sprite_f0_ti1_tr1_s0_d0, ep1c12_device::draw_sprite_f0_ti1_tr1_s1_d0, ep1c12_device::draw_sprite_f0_ti1_tr1_s2_d0, ep1c12_device::draw_sprite_f0_ti1_tr1_s3_d0, ep1c12_device::draw_sprite_f0_ti1_tr1_s4_d0, ep1c12_device::draw_sprite_f0_ti1_tr1_s5_d0, ep1c12_device::draw_sprite_f0_ti1_tr1_s6_d0, ep1c12_device::draw_sprite_f0_ti1_tr1_s7_d0,
	ep1c12_device::draw_sprite_f0_ti1_tr1_s0_d1, ep1c12_device::draw_sprite_f0_ti1_tr1_s1_d1, ep1c12_device::draw_sprite_f0_ti1_tr1_s2_d1, ep1c12_device::draw_sprite_f0_ti1_tr1_s3_d1, ep1c12_device::draw_sprite_f0_ti1_tr1_s4_d1, ep1c12_device::draw_sprite_f0_ti1_tr1_s5_d1, ep1c12_device::draw_sprite_f0_ti1_tr1_s6_d1, ep1c12_device::draw_sprite_f0_ti1_tr1_s7_d1,
	ep1c12_device::draw_sprite_f0_ti1_tr1_s0_d2, ep1c12_device::draw_sprite_f0_ti1_tr1_s1_d2, ep1c12_device::draw_sprite_f0_ti1_tr1_s2_d2, ep1c12_device::draw_sprite_f0_ti1_tr1_s3_d2, ep1c12_device::draw_sprite_f0_ti1_tr1_s4_d2, ep1c12_device::draw_sprite_f0_ti1_tr1_s5_d2, ep1c12_device::draw_sprite_f0_ti1_tr1_s6_d2, ep1c12_device::draw_sprite_f0_ti1_tr1_s7_d2,
	ep1c12_device::draw_sprite_f0_ti1_tr1_s0_d3, ep1c12_device::draw_sprite_f0_ti1_tr1_s1_d3, ep1c12_device::draw_sprite_f0_ti1_tr1_s2_d3, ep1c12_device::draw_sprite_f0_ti1_tr1_s3_d3, ep1c12_device::draw_sprite_f0_ti1_tr1_s4_d3, ep1c12_device::draw_sprite_f0_ti1_tr1_s5_d3, ep1c12_device::draw_sprite_f0_ti1_tr1_s6_d3, ep1c12_device::draw_sprite_f0_ti1_tr1_s7_d3,
	ep1c12_device::draw_sprite_f0_ti1_tr1_s0_d4, ep1c12_device::draw_sprite_f0_ti1_tr1_s1_d4, ep1c12_device::draw_sprite_f0_ti1_tr1_s2_d4, ep1c12_device::draw_sprite_f0_ti1_tr1_s3_d4, ep1c12_device::draw_sprite_f0_ti1_tr1_s4_d4, ep1c12_device::draw_sprite_f0_ti1_tr1_s5_d4, ep1c12_device::draw_sprite_f0_ti1_tr1_s6_d4, ep1c12_device::draw_sprite_f0_ti1_tr1_s7_d4,
	ep1c12_device::draw_sprite_f0_ti1_tr1_s0_d5, ep1c12_device::draw_sprite_f0_ti1_tr1_s1_d5, ep1c12_device::draw_sprite_f0_ti1_tr1_s2_d5, ep1c12_device::draw_sprite_f0_ti1_tr1_s3_d5, ep1c12_device::draw_sprite_f0_ti1_tr1_s4_d5, ep1c12_device::draw_sprite_f0_ti1_tr1_s5_d5, ep1c12_device::draw_sprite_f0_ti1_tr1_s6_d5, ep1c12_device::draw_sprite_f0_ti1_tr1_s7_d5,
	ep1c12_device::draw_sprite_f0_ti1_tr1_s0_d6, ep1c12_device::draw_sprite_f0_ti1_tr1_s1_d6, ep1c12_device::draw_sprite_f0_ti1_tr1_s2_d6, ep1c12_device::draw_sprite_f0_ti1_tr1_s3_d6, ep1c12_device::draw_sprite_f0_ti1_tr1_s4_d6, ep1c12_device::draw_sprite_f0_ti1_tr1_s5_d6, ep1c12_device::draw_sprite_f0_ti1_tr1_s6_d6, ep1c12_device::draw_sprite_f0_ti1_tr1_s7_d6,
	ep1c12_device::draw_sprite_f0_ti1_tr1_s0_d7, ep1c12_device::draw_sprite_f0_ti1_tr1_s1_d7, ep1c12_device::draw_sprite_f0_ti1_tr1_s2_d7, ep1c12_device::draw_sprite_f0_ti1_tr1_s3_d7, ep1c12_device::draw_sprite_f0_ti1_tr1_s4_d7, ep1c12_device::draw_sprite_f0_ti1_tr1_s5_d7, ep1c12_device::draw_sprite_f0_ti1_tr1_s6_d7, ep1c12_device::draw_sprite_f0_ti1_tr1_s7_d7,
};

const ep1c12_device::blitfunction ep1c12_device::f0_ti1_tr0_blit_funcs[64] =
{
	ep1c12_device::draw_sprite_f0_ti1_tr0_s0_d0, ep1c12_device::draw_sprite_f0_ti1_tr0_s1_d0, ep1c12_device::draw_sprite_f0_ti1_tr0_s2_d0, ep1c12_device::draw_sprite_f0_ti1_tr0_s3_d0, ep1c12_device::draw_sprite_f0_ti1_tr0_s4_d0, ep1c12_device::draw_sprite_f0_ti1_tr0_s5_d0, ep1c12_device::draw_sprite_f0_ti1_tr0_s6_d0, ep1c12_device::draw_sprite_f0_ti1_tr0_s7_d0,
	ep1c12_device::draw_sprite_f0_ti1_tr0_s0_d1, ep1c12_device::draw_sprite_f0_ti1_tr0_s1_d1, ep1c12_device::draw_sprite_f0_ti1_tr0_s2_d1, ep1c12_device::draw_sprite_f0_ti1_tr0_s3_d1, ep1c12_device::draw_sprite_f0_ti1_tr0_s4_d1, ep1c12_device::draw_sprite_f0_ti1_tr0_s5_d1, ep1c12_device::draw_sprite_f0_ti1_tr0_s6_d1, ep1c12_device::draw_sprite_f0_ti1_tr0_s7_d1,
	ep1c12_device::draw_sprite_f0_ti1_tr0_s0_d2, ep1c12_device::draw_sprite_f0_ti1_tr0_s1_d2, ep1c12_device::draw_sprite_f0_ti1_tr0_s2_d2, ep1c12_device::draw_sprite_f0_ti1_tr0_s3_d2, ep1c12_device::draw_sprite_f0_ti1_tr0_s4_d2, ep1c12_device::draw_sprite_f0_ti1_tr0_s5_d2, ep1c12_device::draw_sprite_f0_ti1_tr0_s6_d2, ep1c12_device::draw_sprite_f0_ti1_tr0_s7_d2,
	ep1c12_device::draw_sprite_f0_ti1_tr0_s0_d3, ep1c12_device::draw_sprite_f0_ti1_tr0_s1_d3, ep1c12_device::draw_sprite_f0_ti1_tr0_s2_d3, ep1c12_device::draw_sprite_f0_ti1_tr0_s3_d3, ep1c12_device::draw_sprite_f0_ti1_tr0_s4_d3, ep1c12_device::draw_sprite_f0_ti1_tr0_s5_d3, ep1c12_device::draw_sprite_f0_ti1_tr0_s6_d3, ep1c12_device::draw_sprite_f0_ti1_tr0_s7_d3,
	ep1c12_device::draw_sprite_f0_ti1_tr0_s0_d4, ep1c12_device::draw_sprite_f0_ti1_tr0_s1_d4, ep1c12_device::draw_sprite_f0_ti1_tr0_s2_d4, ep1c12_device::draw_sprite_f0_ti1_tr0_s3_d4, ep1c12_device::draw_sprite_f0_ti1_tr0_s4_d4, ep1c12_device::draw_sprite_f0_ti1_tr0_s5_d4, ep1c12_device::draw_sprite_f0_ti1_tr0_s6_d4, ep1c12_device::draw_sprite_f0_ti1_tr0_s7_d4,
	ep1c12_device::draw_sprite_f0_ti1_tr0_s0_d5, ep1c12_device::draw_sprite_f0_ti1_tr0_s1_d5, ep1c12_device::draw_sprite_f0_ti1_tr0_s2_d5, ep1c12_device::draw_sprite_f0_ti1_tr0_s3_d5, ep1c12_device::draw_sprite_f0_ti1_tr0_s4_d5, ep1c12_device::draw_sprite_f0_ti1_tr0_s5_d5, ep1c12_device::draw_sprite_f0_ti1_tr0_s6_d5, ep1c12_device::draw_sprite_f0_ti1_tr0_s7_d5,
	ep1c12_device::draw_sprite_f0_ti1_tr0_s0_d6, ep1c12_device::draw_sprite_f0_ti1_tr0_s1_d6, ep1c12_device::draw_sprite_f0_ti1_tr0_s2_d6, ep1c12_device::draw_sprite_f0_ti1_tr0_s3_d6, ep1c12_device::draw_sprite_f0_ti1_tr0_s4_d6, ep1c12_device::draw_sprite_f0_ti1_tr0_s5_d6, ep1c12_device::draw_sprite_f0_ti1_tr0_s6_d6, ep1c12_device::draw_sprite_f0_ti1_tr0_s7_d6,
	ep1c12_device::draw_sprite_f0_ti1_tr0_s0_d7, ep1c12_device::draw_sprite_f0_ti1_tr0_s1_d7, ep1c12_device::draw_sprite_f0_ti1_tr0_s2_d7, ep1c12_device::draw_sprite_f0_ti1_tr0_s3_d7, ep1c12_device::draw_sprite_f0_ti1_tr0_s4_d7, ep1c12_device::draw_sprite_f0_ti1_tr0_s5_d7, ep1c12_device::draw_sprite_f0_ti1_tr0_s6_d7, ep1c12_device::draw_sprite_f0_ti1_tr0_s7_d7,
};

const ep1c12_device::blitfunction ep1c12_device::f1_ti1_tr1_blit_funcs[64] =
{
	ep1c12_device::draw_sprite_f1_ti1_tr1_s0_d0, ep1c12_device::draw_sprite_f1_ti1_tr1_s1_d0, ep1c12_device::draw_sprite_f1_ti1_tr1_s2_d0, ep1c12_device::draw_sprite_f1_ti1_tr1_s3_d0, ep1c12_device::draw_sprite_f1_ti1_tr1_s4_d0, ep1c12_device::draw_sprite_f1_ti1_tr1_s5_d0, ep1c12_device::draw_sprite_f1_ti1_tr1_s6_d0, ep1c12_device::draw_sprite_f1_ti1_tr1_s7_d0,
	ep1c12_device::draw_sprite_f1_ti1_tr1_s0_d1, ep1c12_device::draw_sprite_f1_ti1_tr1_s1_d1, ep1c12_device::draw_sprite_f1_ti1_tr1_s2_d1, ep1c12_device::draw_sprite_f1_ti1_tr1_s3_d1, ep1c12_device::draw_sprite_f1_ti1_tr1_s4_d1, ep1c12_device::draw_sprite_f1_ti1_tr1_s5_d1, ep1c12_device::draw_sprite_f1_ti1_tr1_s6_d1, ep1c12_device::draw_sprite_f1_ti1_tr1_s7_d1,
	ep1c12_device::draw_sprite_f1_ti1_tr1_s0_d2, ep1c12_device::draw_sprite_f1_ti1_tr1_s1_d2, ep1c12_device::draw_sprite_f1_ti1_tr1_s2_d2, ep1c12_device::draw_sprite_f1_ti1_tr1_s3_d2, ep1c12_device::draw_sprite_f1_ti1_tr1_s4_d2, ep1c12_device::draw_sprite_f1_ti1_tr1_s5_d2, ep1c12_device::draw_sprite_f1_ti1_tr1_s6_d2, ep1c12_device::draw_sprite_f1_ti1_tr1_s7_d2,
	ep1c12_device::draw_sprite_f1_ti1_tr1_s0_d3, ep1c12_device::draw_sprite_f1_ti1_tr1_s1_d3, ep1c12_device::draw_sprite_f1_ti1_tr1_s2_d3, ep1c12_device::draw_sprite_f1_ti1_tr1_s3_d3, ep1c12_device::draw_sprite_f1_ti1_tr1_s4_d3, ep1c12_device::draw_sprite_f1_ti1_tr1_s5_d3, ep1c12_device::draw_sprite_f1_ti1_tr1_s6_d3, ep1c12_device::draw_sprite_f1_ti1_tr1_s7_d3,
	ep1c12_device::draw_sprite_f1_ti1_tr1_s0_d4, ep1c12_device::draw_sprite_f1_ti1_tr1_s1_d4, ep1c12_device::draw_sprite_f1_ti1_tr1_s2_d4, ep1c12_device::draw_sprite_f1_ti1_tr1_s3_d4, ep1c12_device::draw_sprite_f1_ti1_tr1_s4_d4, ep1c12_device::draw_sprite_f1_ti1_tr1_s5_d4, ep1c12_device::draw_sprite_f1_ti1_tr1_s6_d4, ep1c12_device::draw_sprite_f1_ti1_tr1_s7_d4,
	ep1c12_device::draw_sprite_f1_ti1_tr1_s0_d5, ep1c12_device::draw_sprite_f1_ti1_tr1_s1_d5, ep1c12_device::draw_sprite_f1_ti1_tr1_s2_d5, ep1c12_device::draw_sprite_f1_ti1_tr1_s3_d5, ep1c12_device::draw_sprite_f1_ti1_tr1_s4_d5, ep1c12_device::draw_sprite_f1_ti1_tr1_s5_d5, ep1c12_device::draw_sprite_f1_ti1_tr1_s6_d5, ep1c12_device::draw_sprite_f1_ti1_tr1_s7_d5,
	ep1c12_device::draw_sprite_f1_ti1_tr1_s0_d6, ep1c12_device::draw_sprite_f1_ti1_tr1_s1_d6, ep1c12_device::draw_sprite_f1_ti1_tr1_s2_d6, ep1c12_device::draw_sprite_f1_ti1_tr1_s3_d6, ep1c12_device::draw_sprite_f1_ti1_tr1_s4_d6, ep1c12_device::draw_sprite_f1_ti1_tr1_s5_d6, ep1c12_device::draw_sprite_f1_ti1_tr1_s6_d6, ep1c12_device::draw_sprite_f1_ti1_tr1_s7_d6,
	ep1c12_device::draw_sprite_f1_ti1_tr1_s0_d7, ep1c12_device::draw_sprite_f1_ti1_tr1_s1_d7, ep1c12_device::draw_sprite_f1_ti1_tr1_s2_d7, ep1c12_device::draw_sprite_f1_ti1_tr1_s3_d7, ep1c12_device::draw_sprite_f1_ti1_tr1_s4_d7, ep1c12_device::draw_sprite_f1_ti1_tr1_s5_d7, ep1c12_device::draw_sprite_f1_ti1_tr1_s6_d7, ep1c12_device::draw_sprite_f1_ti1_tr1_s7_d7,
};

const ep1c12_device::blitfunction ep1c12_device::f1_ti1_tr0_blit_funcs[64] =
{
	ep1c12_device::draw_sprite_f1_ti1_tr0_s0_d0, ep1c12_device::draw_sprite_f1_ti1_tr0_s1_d0, ep1c12_device::draw_sprite_f1_ti1_tr0_s2_d0, ep1c12_device::draw_sprite_f1_ti1_tr0_s3_d0, ep1c12_device::draw_sprite_f1_ti1_tr0_s4_d0, ep1c12_device::draw_sprite_f1_ti1_tr0_s5_d0, ep1c12_device::draw_sprite_f1_ti1_tr0_s6_d0, ep1c12_device::draw_sprite_f1_ti1_tr0_s7_d0,
	ep1c12_device::draw_sprite_f1_ti1_tr0_s0_d1, ep1c12_device::draw_sprite_f1_ti1_tr0_s1_d1, ep1c12_device::draw_sprite_f1_ti1_tr0_s2_d1, ep1c12_device::draw_sprite_f1_ti1_tr0_s3_d1, ep1c12_device::draw_sprite_f1_ti1_tr0_s4_d1, ep1c12_device::draw_sprite_f1_ti1_tr0_s5_d1, ep1c12_device::draw_sprite_f1_ti1_tr0_s6_d1, ep1c12_device::draw_sprite_f1_ti1_tr0_s7_d1,
	ep1c12_device::draw_sprite_f1_ti1_tr0_s0_d2, ep1c12_device::draw_sprite_f1_ti1_tr0_s1_d2, ep1c12_device::draw_sprite_f1_ti1_tr0_s2_d2, ep1c12_device::draw_sprite_f1_ti1_tr0_s3_d2, ep1c12_device::draw_sprite_f1_ti1_tr0_s4_d2, ep1c12_device::draw_sprite_f1_ti1_tr0_s5_d2, ep1c12_device::draw_sprite_f1_ti1_tr0_s6_d2, ep1c12_device::draw_sprite_f1_ti1_tr0_s7_d2,
	ep1c12_device::draw_sprite_f1_ti1_tr0_s0_d3, ep1c12_device::draw_sprite_f1_ti1_tr0_s1_d3, ep1c12_device::draw_sprite_f1_ti1_tr0_s2_d3, ep1c12_device::draw_sprite_f1_ti1_tr0_s3_d3, ep1c12_device::draw_sprite_f1_ti1_tr0_s4_d3, ep1c12_device::draw_sprite_f1_ti1_tr0_s5_d3, ep1c12_device::draw_sprite_f1_ti1_tr0_s6_d3, ep1c12_device::draw_sprite_f1_ti1_tr0_s7_d3,
	ep1c12_device::draw_sprite_f1_ti1_tr0_s0_d4, ep1c12_device::draw_sprite_f1_ti1_tr0_s1_d4, ep1c12_device::draw_sprite_f1_ti1_tr0_s2_d4, ep1c12_device::draw_sprite_f1_ti1_tr0_s3_d4, ep1c12_device::draw_sprite_f1_ti1_tr0_s4_d4, ep1c12_device::draw_sprite_f1_ti1_tr0_s5_d4, ep1c12_device::draw_sprite_f1_ti1_tr0_s6_d4, ep1c12_device::draw_sprite_f1_ti1_tr0_s7_d4,
	ep1c12_device::draw_sprite_f1_ti1_tr0_s0_d5, ep1c12_device::draw_sprite_f1_ti1_tr0_s1_d5, ep1c12_device::draw_sprite_f1_ti1_tr0_s2_d5, ep1c12_device::draw_sprite_f1_ti1_tr0_s3_d5, ep1c12_device::draw_sprite_f1_ti1_tr0_s4_d5, ep1c12_device::draw_sprite_f1_ti1_tr0_s5_d5, ep1c12_device::draw_sprite_f1_ti1_tr0_s6_d5, ep1c12_device::draw_sprite_f1_ti1_tr0_s7_d5,
	ep1c12_device::draw_sprite_f1_ti1_tr0_s0_d6, ep1c12_device::draw_sprite_f1_ti1_tr0_s1_d6, ep1c12_device::draw_sprite_f1_ti1_tr0_s2_d6, ep1c12_device::draw_sprite_f1_ti1_tr0_s3_d6, ep1c12_device::draw_sprite_f1_ti1_tr0_s4_d6, ep1c12_device::draw_sprite_f1_ti1_tr0_s5_d6, ep1c12_device::draw_sprite_f1_ti1_tr0_s6_d6, ep1c12_device::draw_sprite_f1_ti1_tr0_s7_d6,
	ep1c12_device::draw_sprite_f1_ti1_tr0_s0_d7, ep1c12_device::draw_sprite_f1_ti1_tr0_s1_d7, ep1c12_device::draw_sprite_f1_ti1_tr0_s2_d7, ep1c12_device::draw_sprite_f1_ti1_tr0_s3_d7, ep1c12_device::draw_sprite_f1_ti1_tr0_s4_d7, ep1c12_device::draw_sprite_f1_ti1_tr0_s5_d7, ep1c12_device::draw_sprite_f1_ti1_tr0_s6_d7, ep1c12_device::draw_sprite_f1_ti1_tr0_s7_d7,
};


const ep1c12_device::blitfunction ep1c12_device::f0_ti0_tr1_blit_funcs[64] =
{
	ep1c12_device::draw_sprite_f0_ti0_tr1_s0_d0, ep1c12_device::draw_sprite_f0_ti0_tr1_s1_d0, ep1c12_device::draw_sprite_f0_ti0_tr1_s2_d0, ep1c12_device::draw_sprite_f0_ti0_tr1_s3_d0, ep1c12_device::draw_sprite_f0_ti0_tr1_s4_d0, ep1c12_device::draw_sprite_f0_ti0_tr1_s5_d0, ep1c12_device::draw_sprite_f0_ti0_tr1_s6_d0, ep1c12_device::draw_sprite_f0_ti0_tr1_s7_d0,
	ep1c12_device::draw_sprite_f0_ti0_tr1_s0_d1, ep1c12_device::draw_sprite_f0_ti0_tr1_s1_d1, ep1c12_device::draw_sprite_f0_ti0_tr1_s2_d1, ep1c12_device::draw_sprite_f0_ti0_tr1_s3_d1, ep1c12_device::draw_sprite_f0_ti0_tr1_s4_d1, ep1c12_device::draw_sprite_f0_ti0_tr1_s5_d1, ep1c12_device::draw_sprite_f0_ti0_tr1_s6_d1, ep1c12_device::draw_sprite_f0_ti0_tr1_s7_d1,
	ep1c12_device::draw_sprite_f0_ti0_tr1_s0_d2, ep1c12_device::draw_sprite_f0_ti0_tr1_s1_d2, ep1c12_device::draw_sprite_f0_ti0_tr1_s2_d2, ep1c12_device::draw_sprite_f0_ti0_tr1_s3_d2, ep1c12_device::draw_sprite_f0_ti0_tr1_s4_d2, ep1c12_device::draw_sprite_f0_ti0_tr1_s5_d2, ep1c12_device::draw_sprite_f0_ti0_tr1_s6_d2, ep1c12_device::draw_sprite_f0_ti0_tr1_s7_d2,
	ep1c12_device::draw_sprite_f0_ti0_tr1_s0_d3, ep1c12_device::draw_sprite_f0_ti0_tr1_s1_d3, ep1c12_device::draw_sprite_f0_ti0_tr1_s2_d3, ep1c12_device::draw_sprite_f0_ti0_tr1_s3_d3, ep1c12_device::draw_sprite_f0_ti0_tr1_s4_d3, ep1c12_device::draw_sprite_f0_ti0_tr1_s5_d3, ep1c12_device::draw_sprite_f0_ti0_tr1_s6_d3, ep1c12_device::draw_sprite_f0_ti0_tr1_s7_d3,
	ep1c12_device::draw_sprite_f0_ti0_tr1_s0_d4, ep1c12_device::draw_sprite_f0_ti0_tr1_s1_d4, ep1c12_device::draw_sprite_f0_ti0_tr1_s2_d4, ep1c12_device::draw_sprite_f0_ti0_tr1_s3_d4, ep1c12_device::draw_sprite_f0_ti0_tr1_s4_d4, ep1c12_device::draw_sprite_f0_ti0_tr1_s5_d4, ep1c12_device::draw_sprite_f0_ti0_tr1_s6_d4, ep1c12_device::draw_sprite_f0_ti0_tr1_s7_d4,
	ep1c12_device::draw_sprite_f0_ti0_tr1_s0_d5, ep1c12_device::draw_sprite_f0_ti0_tr1_s1_d5, ep1c12_device::draw_sprite_f0_ti0_tr1_s2_d5, ep1c12_device::draw_sprite_f0_ti0_tr1_s3_d5, ep1c12_device::draw_sprite_f0_ti0_tr1_s4_d5, ep1c12_device::draw_sprite_f0_ti0_tr1_s5_d5, ep1c12_device::draw_sprite_f0_ti0_tr1_s6_d5, ep1c12_device::draw_sprite_f0_ti0_tr1_s7_d5,
	ep1c12_device::draw_sprite_f0_ti0_tr1_s0_d6, ep1c12_device::draw_sprite_f0_ti0_tr1_s1_d6, ep1c12_device::draw_sprite_f0_ti0_tr1_s2_d6, ep1c12_device::draw_sprite_f0_ti0_tr1_s3_d6, ep1c12_device::draw_sprite_f0_ti0_tr1_s4_d6, ep1c12_device::draw_sprite_f0_ti0_tr1_s5_d6, ep1c12_device::draw_sprite_f0_ti0_tr1_s6_d6, ep1c12_device::draw_sprite_f0_ti0_tr1_s7_d6,
	ep1c12_device::draw_sprite_f0_ti0_tr1_s0_d7, ep1c12_device::draw_sprite_f0_ti0_tr1_s1_d7, ep1c12_device::draw_sprite_f0_ti0_tr1_s2_d7, ep1c12_device::draw_sprite_f0_ti0_tr1_s3_d7, ep1c12_device::draw_sprite_f0_ti0_tr1_s4_d7, ep1c12_device::draw_sprite_f0_ti0_tr1_s5_d7, ep1c12_device::draw_sprite_f0_ti0_tr1_s6_d7, ep1c12_device::draw_sprite_f0_ti0_tr1_s7_d7,
};

const ep1c12_device::blitfunction ep1c12_device::f0_ti0_tr0_blit_funcs[64] =
{
	ep1c12_device::draw_sprite_f0_ti0_tr0_s0_d0, ep1c12_device::draw_sprite_f0_ti0_tr0_s1_d0, ep1c12_device::draw_sprite_f0_ti0_tr0_s2_d0, ep1c12_device::draw_sprite_f0_ti0_tr0_s3_d0, ep1c12_device::draw_sprite_f0_ti0_tr0_s4_d0, ep1c12_device::draw_sprite_f0_ti0_tr0_s5_d0, ep1c12_device::draw_sprite_f0_ti0_tr0_s6_d0, ep1c12_device::draw_sprite_f0_ti0_tr0_s7_d0,
	ep1c12_device::draw_sprite_f0_ti0_tr0_s0_d1, ep1c12_device::draw_sprite_f0_ti0_tr0_s1_d1, ep1c12_device::draw_sprite_f0_ti0_tr0_s2_d1, ep1c12_device::draw_sprite_f0_ti0_tr0_s3_d1, ep1c12_device::draw_sprite_f0_ti0_tr0_s4_d1, ep1c12_device::draw_sprite_f0_ti0_tr0_s5_d1, ep1c12_device::draw_sprite_f0_ti0_tr0_s6_d1, ep1c12_device::draw_sprite_f0_ti0_tr0_s7_d1,
	ep1c12_device::draw_sprite_f0_ti0_tr0_s0_d2, ep1c12_device::draw_sprite_f0_ti0_tr0_s1_d2, ep1c12_device::draw_sprite_f0_ti0_tr0_s2_d2, ep1c12_device::draw_sprite_f0_ti0_tr0_s3_d2, ep1c12_device::draw_sprite_f0_ti0_tr0_s4_d2, ep1c12_device::draw_sprite_f0_ti0_tr0_s5_d2, ep1c12_device::draw_sprite_f0_ti0_tr0_s6_d2, ep1c12_device::draw_sprite_f0_ti0_tr0_s7_d2,
	ep1c12_device::draw_sprite_f0_ti0_tr0_s0_d3, ep1c12_device::draw_sprite_f0_ti0_tr0_s1_d3, ep1c12_device::draw_sprite_f0_ti0_tr0_s2_d3, ep1c12_device::draw_sprite_f0_ti0_tr0_s3_d3, ep1c12_device::draw_sprite_f0_ti0_tr0_s4_d3, ep1c12_device::draw_sprite_f0_ti0_tr0_s5_d3, ep1c12_device::draw_sprite_f0_ti0_tr0_s6_d3, ep1c12_device::draw_sprite_f0_ti0_tr0_s7_d3,
	ep1c12_device::draw_sprite_f0_ti0_tr0_s0_d4, ep1c12_device::draw_sprite_f0_ti0_tr0_s1_d4, ep1c12_device::draw_sprite_f0_ti0_tr0_s2_d4, ep1c12_device::draw_sprite_f0_ti0_tr0_s3_d4, ep1c12_device::draw_sprite_f0_ti0_tr0_s4_d4, ep1c12_device::draw_sprite_f0_ti0_tr0_s5_d4, ep1c12_device::draw_sprite_f0_ti0_tr0_s6_d4, ep1c12_device::draw_sprite_f0_ti0_tr0_s7_d4,
	ep1c12_device::draw_sprite_f0_ti0_tr0_s0_d5, ep1c12_device::draw_sprite_f0_ti0_tr0_s1_d5, ep1c12_device::draw_sprite_f0_ti0_tr0_s2_d5, ep1c12_device::draw_sprite_f0_ti0_tr0_s3_d5, ep1c12_device::draw_sprite_f0_ti0_tr0_s4_d5, ep1c12_device::draw_sprite_f0_ti0_tr0_s5_d5, ep1c12_device::draw_sprite_f0_ti0_tr0_s6_d5, ep1c12_device::draw_sprite_f0_ti0_tr0_s7_d5,
	ep1c12_device::draw_sprite_f0_ti0_tr0_s0_d6, ep1c12_device::draw_sprite_f0_ti0_tr0_s1_d6, ep1c12_device::draw_sprite_f0_ti0_tr0_s2_d6, ep1c12_device::draw_sprite_f0_ti0_tr0_s3_d6, ep1c12_device::draw_sprite_f0_ti0_tr0_s4_d6, ep1c12_device::draw_sprite_f0_ti0_tr0_s5_d6, ep1c12_device::draw_sprite_f0_ti0_tr0_s6_d6, ep1c12_device::draw_sprite_f0_ti0_tr0_s7_d6,
	ep1c12_device::draw_sprite_f0_ti0_tr0_s0_d7, ep1c12_device::draw_sprite_f0_ti0_tr0_s1_d7, ep1c12_device::draw_sprite_f0_ti0_tr0_s2_d7, ep1c12_device::draw_sprite_f0_ti0_tr0_s3_d7, ep1c12_device::draw_sprite_f0_ti0_tr0_s4_d7, ep1c12_device::draw_sprite_f0_ti0_tr0_s5_d7, ep1c12_device::draw_sprite_f0_ti0_tr0_s6_d7, ep1c12_device::draw_sprite_f0_ti0_tr0_s7_d7,
};

const ep1c12_device::blitfunction ep1c12_device::f1_ti0_tr1_blit_funcs[64] =
{
	ep1c12_device::draw_sprite_f1_ti0_tr1_s0_d0, ep1c12_device::draw_sprite_f1_ti0_tr1_s1_d0, ep1c12_device::draw_sprite_f1_ti0_tr1_s2_d0, ep1c12_device::draw_sprite_f1_ti0_tr1_s3_d0, ep1c12_device::draw_sprite_f1_ti0_tr1_s4_d0, ep1c12_device::draw_sprite_f1_ti0_tr1_s5_d0, ep1c12_device::draw_sprite_f1_ti0_tr1_s6_d0, ep1c12_device::draw_sprite_f1_ti0_tr1_s7_d0,
	ep1c12_device::draw_sprite_f1_ti0_tr1_s0_d1, ep1c12_device::draw_sprite_f1_ti0_tr1_s1_d1, ep1c12_device::draw_sprite_f1_ti0_tr1_s2_d1, ep1c12_device::draw_sprite_f1_ti0_tr1_s3_d1, ep1c12_device::draw_sprite_f1_ti0_tr1_s4_d1, ep1c12_device::draw_sprite_f1_ti0_tr1_s5_d1, ep1c12_device::draw_sprite_f1_ti0_tr1_s6_d1, ep1c12_device::draw_sprite_f1_ti0_tr1_s7_d1,
	ep1c12_device::draw_sprite_f1_ti0_tr1_s0_d2, ep1c12_device::draw_sprite_f1_ti0_tr1_s1_d2, ep1c12_device::draw_sprite_f1_ti0_tr1_s2_d2, ep1c12_device::draw_sprite_f1_ti0_tr1_s3_d2, ep1c12_device::draw_sprite_f1_ti0_tr1_s4_d2, ep1c12_device::draw_sprite_f1_ti0_tr1_s5_d2, ep1c12_device::draw_sprite_f1_ti0_tr1_s6_d2, ep1c12_device::draw_sprite_f1_ti0_tr1_s7_d2,
	ep1c12_device::draw_sprite_f1_ti0_tr1_s0_d3, ep1c12_device::draw_sprite_f1_ti0_tr1_s1_d3, ep1c12_device::draw_sprite_f1_ti0_tr1_s2_d3, ep1c12_device::draw_sprite_f1_ti0_tr1_s3_d3, ep1c12_device::draw_sprite_f1_ti0_tr1_s4_d3, ep1c12_device::draw_sprite_f1_ti0_tr1_s5_d3, ep1c12_device::draw_sprite_f1_ti0_tr1_s6_d3, ep1c12_device::draw_sprite_f1_ti0_tr1_s7_d3,
	ep1c12_device::draw_sprite_f1_ti0_tr1_s0_d4, ep1c12_device::draw_sprite_f1_ti0_tr1_s1_d4, ep1c12_device::draw_sprite_f1_ti0_tr1_s2_d4, ep1c12_device::draw_sprite_f1_ti0_tr1_s3_d4, ep1c12_device::draw_sprite_f1_ti0_tr1_s4_d4, ep1c12_device::draw_sprite_f1_ti0_tr1_s5_d4, ep1c12_device::draw_sprite_f1_ti0_tr1_s6_d4, ep1c12_device::draw_sprite_f1_ti0_tr1_s7_d4,
	ep1c12_device::draw_sprite_f1_ti0_tr1_s0_d5, ep1c12_device::draw_sprite_f1_ti0_tr1_s1_d5, ep1c12_device::draw_sprite_f1_ti0_tr1_s2_d5, ep1c12_device::draw_sprite_f1_ti0_tr1_s3_d5, ep1c12_device::draw_sprite_f1_ti0_tr1_s4_d5, ep1c12_device::draw_sprite_f1_ti0_tr1_s5_d5, ep1c12_device::draw_sprite_f1_ti0_tr1_s6_d5, ep1c12_device::draw_sprite_f1_ti0_tr1_s7_d5,
	ep1c12_device::draw_sprite_f1_ti0_tr1_s0_d6, ep1c12_device::draw_sprite_f1_ti0_tr1_s1_d6, ep1c12_device::draw_sprite_f1_ti0_tr1_s2_d6, ep1c12_device::draw_sprite_f1_ti0_tr1_s3_d6, ep1c12_device::draw_sprite_f1_ti0_tr1_s4_d6, ep1c12_device::draw_sprite_f1_ti0_tr1_s5_d6, ep1c12_device::draw_sprite_f1_ti0_tr1_s6_d6, ep1c12_device::draw_sprite_f1_ti0_tr1_s7_d6,
	ep1c12_device::draw_sprite_f1_ti0_tr1_s0_d7, ep1c12_device::draw_sprite_f1_ti0_tr1_s1_d7, ep1c12_device::draw_sprite_f1_ti0_tr1_s2_d7, ep1c12_device::draw_sprite_f1_ti0_tr1_s3_d7, ep1c12_device::draw_sprite_f1_ti0_tr1_s4_d7, ep1c12_device::draw_sprite_f1_ti0_tr1_s5_d7, ep1c12_device::draw_sprite_f1_ti0_tr1_s6_d7, ep1c12_device::draw_sprite_f1_ti0_tr1_s7_d7,
};

const ep1c12_device::blitfunction ep1c12_device::f1_ti0_tr0_blit_funcs[64] =
{
	ep1c12_device::draw_sprite_f1_ti0_tr0_s0_d0, ep1c12_device::draw_sprite_f1_ti0_tr0_s1_d0, ep1c12_device::draw_sprite_f1_ti0_tr0_s2_d0, ep1c12_device::draw_sprite_f1_ti0_tr0_s3_d0, ep1c12_device::draw_sprite_f1_ti0_tr0_s4_d0, ep1c12_device::draw_sprite_f1_ti0_tr0_s5_d0, ep1c12_device::draw_sprite_f1_ti0_tr0_s6_d0, ep1c12_device::draw_sprite_f1_ti0_tr0_s7_d0,
	ep1c12_device::draw_sprite_f1_ti0_tr0_s0_d1, ep1c12_device::draw_sprite_f1_ti0_tr0_s1_d1, ep1c12_device::draw_sprite_f1_ti0_tr0_s2_d1, ep1c12_device::draw_sprite_f1_ti0_tr0_s3_d1, ep1c12_device::draw_sprite_f1_ti0_tr0_s4_d1, ep1c12_device::draw_sprite_f1_ti0_tr0_s5_d1, ep1c12_device::draw_sprite_f1_ti0_tr0_s6_d1, ep1c12_device::draw_sprite_f1_ti0_tr0_s7_d1,
	ep1c12_device::draw_sprite_f1_ti0_tr0_s0_d2, ep1c12_device::draw_sprite_f1_ti0_tr0_s1_d2, ep1c12_device::draw_sprite_f1_ti0_tr0_s2_d2, ep1c12_device::draw_sprite_f1_ti0_tr0_s3_d2, ep1c12_device::draw_sprite_f1_ti0_tr0_s4_d2, ep1c12_device::draw_sprite_f1_ti0_tr0_s5_d2, ep1c12_device::draw_sprite_f1_ti0_tr0_s6_d2, ep1c12_device::draw_sprite_f1_ti0_tr0_s7_d2,
	ep1c12_device::draw_sprite_f1_ti0_tr0_s0_d3, ep1c12_device::draw_sprite_f1_ti0_tr0_s1_d3, ep1c12_device::draw_sprite_f1_ti0_tr0_s2_d3, ep1c12_device::draw_sprite_f1_ti0_tr0_s3_d3, ep1c12_device::draw_sprite_f1_ti0_tr0_s4_d3, ep1c12_device::draw_sprite_f1_ti0_tr0_s5_d3, ep1c12_device::draw_sprite_f1_ti0_tr0_s6_d3, ep1c12_device::draw_sprite_f1_ti0_tr0_s7_d3,
	ep1c12_device::draw_sprite_f1_ti0_tr0_s0_d4, ep1c12_device::draw_sprite_f1_ti0_tr0_s1_d4, ep1c12_device::draw_sprite_f1_ti0_tr0_s2_d4, ep1c12_device::draw_sprite_f1_ti0_tr0_s3_d4, ep1c12_device::draw_sprite_f1_ti0_tr0_s4_d4, ep1c12_device::draw_sprite_f1_ti0_tr0_s5_d4, ep1c12_device::draw_sprite_f1_ti0_tr0_s6_d4, ep1c12_device::draw_sprite_f1_ti0_tr0_s7_d4,
	ep1c12_device::draw_sprite_f1_ti0_tr0_s0_d5, ep1c12_device::draw_sprite_f1_ti0_tr0_s1_d5, ep1c12_device::draw_sprite_f1_ti0_tr0_s2_d5, ep1c12_device::draw_sprite_f1_ti0_tr0_s3_d5, ep1c12_device::draw_sprite_f1_ti0_tr0_s4_d5, ep1c12_device::draw_sprite_f1_ti0_tr0_s5_d5, ep1c12_device::draw_sprite_f1_ti0_tr0_s6_d5, ep1c12_device::draw_sprite_f1_ti0_tr0_s7_d5,
	ep1c12_device::draw_sprite_f1_ti0_tr0_s0_d6, ep1c12_device::draw_sprite_f1_ti0_tr0_s1_d6, ep1c12_device::draw_sprite_f1_ti0_tr0_s2_d6, ep1c12_device::draw_sprite_f1_ti0_tr0_s3_d6, ep1c12_device::draw_sprite_f1_ti0_tr0_s4_d6, ep1c12_device::draw_sprite_f1_ti0_tr0_s5_d6, ep1c12_device::draw_sprite_f1_ti0_tr0_s6_d6, ep1c12_device::draw_sprite_f1_ti0_tr0_s7_d6,
	ep1c12_device::draw_sprite_f1_ti0_tr0_s0_d7, ep1c12_device::draw_sprite_f1_ti0_tr0_s1_d7, ep1c12_device::draw_sprite_f1_ti0_tr0_s2_d7, ep1c12_device::draw_sprite_f1_ti0_tr0_s3_d7, ep1c12_device::draw_sprite_f1_ti0_tr0_s4_d7, ep1c12_device::draw_sprite_f1_ti0_tr0_s5_d7, ep1c12_device::draw_sprite_f1_ti0_tr0_s6_d7, ep1c12_device::draw_sprite_f1_ti0_tr0_s7_d7,
};

/*
    Calculate number of VRAM row accesses a draw will perform.
    Source data will typically be aligned well with VRAM, but this is not the case for the destination.
    As an example, drawing a 64x32 pixel image will usually read from two VRAM rows for source data,
    but if the destination start coordinate is (x=10, y=10), each of the 32x32px chunks of source data will
    touch 4 rows of destination VRAM, leading to a total of 8 destination VRAM accesses.
*/
inline u16 calculate_vram_accesses(u16 start_x, u16 start_y, u16 dimx, u16 dimy)
{
	int x_rows = 0;
	int num_vram_rows = 0;
	for (int x_pixels = dimx; x_pixels > 0; x_pixels -= 32)
	{
		x_rows++;
		if (((start_x & 31) + std::min(32, x_pixels)) > 32)
			x_rows++;  // Drawing across multiple horizontal VRAM row boundaries.
	}
	for (int y_pixels = dimy; y_pixels > 0; y_pixels -= 32)
	{
		num_vram_rows += x_rows;
		if (((start_y & 31) + std::min(32, y_pixels)) > 32)
			num_vram_rows += x_rows;  // Drawing across multiple vertical VRAM row boundaries.
	}
	return num_vram_rows;
}

/*
    Draw command
    This command draws gfx data.

    Offset Bits              Description
           fedcba98 76543210
    00     0001---- -------- 0x1 for draw
           ----x--- -------- Flip X
           -----x-- -------- Flip Y
           ------x- -------- Enable Blending
           -------x -------- Enable Transparent
           -------- -xxx---- Source Blending mode
           -------- -----xxx Destination Blending mode
    02     xxxxxxxx -------- Source Alpha value
           -------- xxxxxxxx Destination Alpha value
    04     ---xxxxx xxxxxxxx Source X start position
    06     ----xxxx xxxxxxxx Source Y start position
    08     sxxxxxxx xxxxxxxx Destination X start position
    0a     sxxxxxxx xxxxxxxx Destination Y start position
    0c     ---xxxxx xxxxxxxx Source Width
    0e     ----xxxx xxxxxxxx Source Height
    10     -------- xxxxxxxx Source Red multiplication (0x80 = 100%)
    12     xxxxxxxx -------- Source Green multiplication (0x80 = 100%)
           -------- xxxxxxxx Source Blue multiplication (0x80 = 100%)

    Blending mode (description from ibara test mode)
    000 +alpha
    001 +source
    010 +destination
    100 -alpha
    101 -source
    110 -destination
    others are reserved/disable?
*/

inline void ep1c12_device::gfx_draw_shadow_copy(address_space &space, offs_t *addr)
{
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	u16 src_x_start = COPY_NEXT_WORD(space, addr);
	u16 src_y_start = COPY_NEXT_WORD(space, addr);
	u16 dst_x_start = COPY_NEXT_WORD(space, addr);
	u16 dst_y_start = COPY_NEXT_WORD(space, addr);
	u16 src_dimx = (COPY_NEXT_WORD(space, addr) & 0x1fff) + 1;
	u16 src_dimy = (COPY_NEXT_WORD(space, addr) & 0x0fff) + 1;
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);

	// Calculate Blitter delay for the Draw operation.
	// On real hardware, the Blitter will read operations into a FIFO queue
	// by asserting BREQ on the SH3 and then reading from Main RAM.
	// Since the reads are done concurrently to executions of operations, its
	// ok to estimate the delay all at once instead for emulation purposes.

	u16 dst_x_end = dst_x_start + src_dimx - 1;
	u16 dst_y_end = dst_y_start + src_dimy - 1;

	// Sprites fully outside of clipping area should not be drawn.
	if (dst_x_start > m_clip.max_x || dst_x_end < m_clip.min_x || dst_y_start > m_clip.max_y || dst_y_end < m_clip.min_y)
	{
		idle_blitter(EP1C_DRAW_OPERATION_SIZE_BYTES);
		return;
	}

	// Clip the blitter operations, to have the calculations only respect the area being written.
	// It's not 100% clear this is how this is performed, but it is clear that there should be some amount of clipping
	// applied here to match the hardware. This way seems most likely, and maps well to the delays seen on hardware.
	// One example of this being utilized heavily is the transparent fog in Mushihimesama Futari Stage 1. This is drawn as
	// 256x256 sprites, with large parts clipped away.
	dst_x_start = std::max(dst_x_start, (u16)m_clip.min_x);
	dst_y_start = std::max(dst_y_start, (u16)m_clip.min_y);
	dst_x_end = std::min(dst_x_end, (u16)m_clip.max_x);
	dst_y_end = std::min(dst_y_end, (u16)m_clip.max_y);
	src_dimx = dst_x_end - dst_x_start + 1;
	src_dimy = dst_y_end - dst_y_start + 1;

	m_blit_idle_op_bytes = 0;  // Blitter no longer idle.

	// VRAM data is laid out in 32x32 pixel rows. Calculate amount of rows accessed.
	int src_num_vram_rows = calculate_vram_accesses(src_x_start, src_y_start, src_dimx, src_dimy);
	int dst_num_vram_rows = calculate_vram_accesses(dst_x_start, dst_y_start, src_dimx, src_dimy);

	// Since draws are done 4 pixels at the time, extend the draw area to coordinates aligned for this.
	// Doing this after VRAM calculations simplify things a bit, and these extensions will never make the
	// destination area span additional VRAM rows.
	dst_x_start -= dst_x_start & 3;
	dst_x_end += (4 - ((dst_x_end + 1) & 3)) & 3;
	u16 dst_dimx = dst_x_end - dst_x_start + 1;
	u16 dst_dimy = dst_y_end - dst_y_start + 1;

	// Number of VRAM CLK cycles needed to draw a sprite is sum of:
	// - Number of pixels read from source divided by 4 (Each CLK reads 4 pixels, since 32bit DDR).
	// - Number of pixels read from destination divided by 4.
	// - Pixels written to destination divided by 4.
	// - VRAM access overhead:
	//   - 6 CLK of overhead after each read from a source VRAM row.
	//   - 20 CLK of overhead between read and write of each destination VRAM row.
	//   - 11 CLK of overhead after each write to a destination VRAM row.
	// - 12 CLK of additional overhead per sprite at the end of writing.
	// Note: Details are from https://buffis.com/docs/CV1000_Blitter_Research_by_buffi.pdf
	//       There may be mistakes.
	u32 num_vram_clk = src_dimx * src_dimy / 4 + dst_dimx * dst_dimy / 2 + src_num_vram_rows * 6 + dst_num_vram_rows * (20 + 11) + 12;
	m_blit_delay_ns += num_vram_clk * EP1C_VRAM_CLK_NANOSEC;
}

inline void ep1c12_device::gfx_draw(offs_t *addr)
{
	clr_t tint_clr;
	bool tinted = false;

	const u16 attr        = READ_NEXT_WORD(addr);
	const u16 alpha       = READ_NEXT_WORD(addr);
	u16 src_x             = READ_NEXT_WORD(addr);
	u16 src_y             = READ_NEXT_WORD(addr);
	const u16 dst_x_start = READ_NEXT_WORD(addr);
	const u16 dst_y_start = READ_NEXT_WORD(addr);
	const u16 w           = READ_NEXT_WORD(addr);
	const u16 h           = READ_NEXT_WORD(addr);
	const u16 tint_r      = READ_NEXT_WORD(addr);
	const u16 tint_gb     = READ_NEXT_WORD(addr);

	// 0: +alpha
	// 1: +source
	// 2: +dest
	// 3: *
	// 4: -alpha
	// 5: -source
	// 6: -dest
	// 7: *

	const u8 d_mode  =    attr & 0x0007;
	const u8 s_mode  =   (attr & 0x0070) >> 4;

	const bool trans =    attr & 0x0100;
	bool blend       =    attr & 0x0200;

	const bool flipy =    attr & 0x0400;
	const bool flipx =    attr & 0x0800;

	const u8 d_alpha =   ((alpha & 0x00ff)     ) >> 3;
	const u8 s_alpha =   ((alpha & 0xff00) >> 8) >> 3;

//  int src_p        =   0;
	src_x            =   src_x & 0x1fff;
	src_y            =   src_y & 0x0fff;


	const int x      =   (dst_x_start & 0x7fff) - (dst_x_start & 0x8000);
	const int y      =   (dst_y_start & 0x7fff) - (dst_y_start & 0x8000);

	const int dimx   =   (w & 0x1fff) + 1;
	const int dimy   =   (h & 0x0fff) + 1;

	// convert parameters to clr

	tint_to_clr(tint_r & 0x00ff, (tint_gb >> 8) & 0xff, tint_gb & 0xff, &tint_clr);

	/* interestingly this gets set to 0x20 for 'normal' not 0x1f */

	if (tint_clr.r != 0x20)
		tinted = true;

	if (tint_clr.g != 0x20)
		tinted = true;

	if (tint_clr.b != 0x20)
		tinted = true;

	// surprisingly frequent, need to verify if it produces a worthwhile speedup tho.
	if ((s_mode == 0 && s_alpha == 0x1f) && (d_mode == 4 && d_alpha == 0x1f))
		blend = false;

	if (tinted)
	{
		if (!flipx)
		{
			if (trans)
			{
				if (!blend)
				{
					draw_sprite_f0_ti1_tr1_plain(draw_params);
				}
				else
				{
					f0_ti1_tr1_blit_funcs[s_mode | (d_mode << 3)](draw_params);
				}
			}
			else
			{
			if (!blend)
				{
					draw_sprite_f0_ti1_tr0_plain(draw_params);
				}
				else
				{
					f0_ti1_tr0_blit_funcs[s_mode | (d_mode << 3)](draw_params);
				}
			}
		}
		else // flipx
		{
			if (trans)
			{
				if (!blend)
				{
					draw_sprite_f1_ti1_tr1_plain(draw_params);
				}
				else
				{
					f1_ti1_tr1_blit_funcs[s_mode | (d_mode << 3)](draw_params);
				}
			}
			else
			{
			if (!blend)
				{
					draw_sprite_f1_ti1_tr0_plain(draw_params);
				}
				else
				{
					f1_ti1_tr0_blit_funcs[s_mode | (d_mode << 3)](draw_params);
				}
			}
		}
	}
	else
	{
		if (blend == 0 && tinted == 0)
		{
			if (!flipx)
			{
				if (trans)
				{
					draw_sprite_f0_ti0_tr1_simple(draw_params);
				}
				else
				{
					draw_sprite_f0_ti0_tr0_simple(draw_params);
				}
			}
			else
			{
				if (trans)
				{
					draw_sprite_f1_ti0_tr1_simple(draw_params);
				}
				else
				{
					draw_sprite_f1_ti0_tr0_simple(draw_params);
				}

			}

			return;
		}

		//printf("smode %d dmode %d\n", s_mode, d_mode);

		if (!flipx)
		{
			if (trans)
			{
				if (!blend)
				{
					draw_sprite_f0_ti0_plain(draw_params);
				}
				else
				{
					f0_ti0_tr1_blit_funcs[s_mode | (d_mode << 3)](draw_params);
				}
			}
			else
			{
			if (!blend)
				{
					draw_sprite_f0_ti0_tr0_plain(draw_params);
				}
				else
				{
					f0_ti0_tr0_blit_funcs[s_mode | (d_mode << 3)](draw_params);
				}
			}
		}
		else // flipx
		{
			if (trans)
			{
				if (!blend)
				{
					draw_sprite_f1_ti0_plain(draw_params);
				}
				else
				{
					f1_ti0_tr1_blit_funcs[s_mode | (d_mode << 3)](draw_params);
				}
			}
			else
			{
			if (!blend)
				{
					draw_sprite_f1_ti0_tr0_plain(draw_params);
				}
				else
				{
					f1_ti0_tr0_blit_funcs[s_mode | (d_mode << 3)](draw_params);
				}
			}
		}
	}

}


void ep1c12_device::gfx_create_shadow_copy(address_space &space)
{
	offs_t addr = m_gfx_addr & 0x1fffffff;

	m_clip.set(m_gfx_clip_x_shadowcopy - EP1C_CLIP_MARGIN, m_gfx_clip_x_shadowcopy + 320 - 1 + EP1C_CLIP_MARGIN,
		m_gfx_clip_y_shadowcopy - EP1C_CLIP_MARGIN, m_gfx_clip_y_shadowcopy + 240 - 1 + EP1C_CLIP_MARGIN);
	while (1)
	{
		// request commands from main CPU RAM
		const u16 data = COPY_NEXT_WORD(space, &addr);

		switch (data & 0xf000)
		{
			case 0x0000:
			case 0xf000:
				return;

			case 0xc000:
				if (COPY_NEXT_WORD(space, &addr)) // cliptype
					m_clip.set(m_gfx_clip_x_shadowcopy - EP1C_CLIP_MARGIN, m_gfx_clip_x_shadowcopy + 320 - 1 + EP1C_CLIP_MARGIN,
						m_gfx_clip_y_shadowcopy - EP1C_CLIP_MARGIN, m_gfx_clip_y_shadowcopy + 240 - 1 + EP1C_CLIP_MARGIN);
				else
					m_clip.set(0, 0x2000 - 1, 0, 0x1000 - 1);
				idle_blitter(EP1C_CLIP_OPERATION_SIZE_BYTES);
				break;

			case 0x2000:
				addr -= 2;
				gfx_upload_shadow_copy(space, &addr);
				break;

			case 0x1000:
				addr -= 2;
				gfx_draw_shadow_copy(space, &addr);
				break;

			default:
				popmessage("GFX op = %04X", data);
				return;
		}
	}
}


void ep1c12_device::gfx_exec()
{
	offs_t addr = m_gfx_addr_shadowcopy & 0x1fffffff;
	m_clip.set(m_gfx_clip_x_shadowcopy - EP1C_CLIP_MARGIN, m_gfx_clip_x_shadowcopy + 320 - 1 + EP1C_CLIP_MARGIN,
		m_gfx_clip_y_shadowcopy - EP1C_CLIP_MARGIN, m_gfx_clip_y_shadowcopy + 240 - 1 + EP1C_CLIP_MARGIN);

//  logerror("GFX EXEC: %08X\n", addr);

	while (1)
	{
		// request commands from main CPU RAM
		const u16 data = READ_NEXT_WORD(&addr);

		switch (data & 0xf000)
		{
			case 0x0000:
			case 0xf000:
				return;

			case 0xc000:
				if (READ_NEXT_WORD(&addr)) // cliptype
					m_clip.set(m_gfx_clip_x_shadowcopy - EP1C_CLIP_MARGIN, m_gfx_clip_x_shadowcopy + 320 - 1 + EP1C_CLIP_MARGIN,
						m_gfx_clip_y_shadowcopy - EP1C_CLIP_MARGIN, m_gfx_clip_y_shadowcopy + 240 - 1 + EP1C_CLIP_MARGIN);
				else
					m_clip.set(0, 0x2000 - 1, 0, 0x1000 - 1);
				break;

			case 0x2000:
				addr -= 2;
				gfx_upload(&addr);
				break;

			case 0x1000:
				addr -= 2;
				gfx_draw(&addr);
				break;

			default:
				popmessage("GFX op = %04X", data);
				return;
		}
	}
}

void *ep1c12_device::blit_request_callback(void *param, int threadid)
{
	ep1c12_device *object = reinterpret_cast<ep1c12_device *>(param);
	object->gfx_exec();
	return nullptr;
}


u32 ep1c12_device::gfx_ready_r()
{
	return m_blitter_busy ? 0x00000000 : 0x00000010;
}

void ep1c12_device::gfx_exec_w(address_space &space, offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		if (data & 1)
		{
			// make sure we've not already got a request running
			if (m_blitter_request)
			{
				int result;
				do
				{
					result = osd_work_item_wait(m_blitter_request, 1000);
				} while (result == 0);
				osd_work_item_release(m_blitter_request);
			}

			m_gfx_clip_x_shadowcopy = m_gfx_clip_x;
			m_gfx_clip_y_shadowcopy = m_gfx_clip_y;

			// Create a copy of the blit list so we can safely thread it.
			// Copying the Blitter operations will also estimate the delay needed for processing.
			m_blit_delay_ns = 0;
			gfx_create_shadow_copy(space);

			// Every EP1C_VRAM_H_LINE_PERIOD_NANOSEC, the Blitter will block other operations, due
			// to fetching a horizontal line from VRAM for output.
			m_blit_delay_ns += std::floor( m_blit_delay_ns / EP1C_VRAM_H_LINE_PERIOD_NANOSEC ) * EP1C_VRAM_H_LINE_DURATION_NANOSEC;

			// Check if Blitter takes longer than a frame to render.
			// In practice, there's a bit less time than this to allow for lack of slowdown but
			// for debugging purposes this is an ok approximation.
			if (m_blit_delay_ns > EP1C_FRAME_DURATION_NANOSEC)
				LOGDBG("Blitter delay! Blit duration %lld ns.\n", m_blit_delay_ns);

			m_blitter_busy = 1;
			m_blitter_delay_timer->adjust(attotime::from_nsec(m_blit_delay_ns));

			m_gfx_addr_shadowcopy = m_gfx_addr;
			m_blitter_request = osd_work_item_queue(m_work_queue, blit_request_callback, (void*)this, 0);
		}
	}
}

void ep1c12_device::draw_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_blitter_request)
	{
		int result;
		do
		{
			result = osd_work_item_wait(m_blitter_request, 1000);
		} while (result == 0);
		osd_work_item_release(m_blitter_request);
	}

	bitmap.fill(0, cliprect);

#if DEBUG_VRAM_VIEWER
	int curr_width = m_curr_screen_width;
	int curr_height = m_curr_screen_height;
	rectangle curr_visarea = m_curr_screen_visarea;

	if (machine().input().code_pressed_once(KEYCODE_T)) m_debug_vram_view_en = !m_debug_vram_view_en;
	if (m_debug_vram_view_en)
	{
		curr_width = 8192;
		curr_height = 4096;
		curr_visarea.set(0, curr_width - 1, 0, curr_height - 1);
	}
	if ((m_prev_screen_height != curr_height) || (m_prev_screen_width != curr_width))
	{
		screen().configure(curr_width, curr_height, curr_visarea, screen().refresh_attoseconds());
		m_prev_screen_height = curr_height;
		m_prev_screen_width = curr_width;
	}
#endif

	int scroll_x = -m_gfx_scroll_x;
	int scroll_y = -m_gfx_scroll_y;

#if DEBUG_VRAM_VIEWER
	if (m_debug_vram_view_en)
		copybitmap(bitmap, *m_bitmaps, 0, 0, 0, 0, cliprect);
	else
#endif
		copyscrollbitmap(bitmap, *m_bitmaps, 1, &scroll_x, 1, &scroll_y, cliprect);
}


u32 ep1c12_device::blitter_r(offs_t offset, u32 mem_mask)
{
	switch (offset * 4)
	{
		case 0x10:
			return gfx_ready_r();

		case 0x24:
			return 0xffffffff;

		case 0x28:
			return 0xffffffff;

		case 0x50:
			return m_port_r_cb();

		default:
			logerror("unknownblitter_r %08x %08x\n", offset*4, mem_mask);
			break;

	}
	return 0;
}

void ep1c12_device::blitter_w(address_space &space, offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset * 4)
	{
		case 0x04:
			gfx_exec_w(space, offset, data, mem_mask);
			break;

		case 0x08:
			COMBINE_DATA(&m_gfx_addr);
			break;

		case 0x14:
			COMBINE_DATA(&m_gfx_scroll_x);
			break;

		case 0x18:
			COMBINE_DATA(&m_gfx_scroll_y);
			break;

		case 0x24:  // Some sort of handshake at start of IRQ's.
		case 0x28:  // Related to coins entered.
		case 0x30:  // Contrast (test menu).
		case 0x34:  // Brightness (test menu).
		case 0x38:  // V offset (test menu).
		case 0x3c:  // H offset (test menu).
			break;

		case 0x40:
			COMBINE_DATA(&m_gfx_clip_x);
			break;

		case 0x44:
			COMBINE_DATA(&m_gfx_clip_y);
			break;

	}
}

void ep1c12_device::install_handlers(int addr1, int addr2)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.install_read_handler(addr1, addr2, emu::rw_delegate(*this, FUNC(ep1c12_device::blitter_r)), 0xffffffffffffffffU);
	space.install_write_handler(addr1, addr2,  emu::rw_delegate(*this, FUNC(ep1c12_device::blitter_w)), 0xffffffffffffffffU);
}

u64 ep1c12_device::fpga_r()
{
	return 0xff;
}

void ep1c12_device::fpga_w(offs_t offset, u64 data, u64 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		// data & 0x08 = CE
		// data & 0x10 = CLK
		// data & 0x20 = DATA

		if ((data & 0x08) && !(m_firmware_port & 0x10) && (data & 0x10))
		{
			if (m_firmware_pos < 2323240 && (data & 0x20))
				m_firmware[m_firmware_pos >> 3] |= 1 << (m_firmware_pos & 7);
			m_firmware_pos++;
		}

		m_firmware_port = data;

		if (m_firmware_pos == 2323240)
		{
			u8 checksum = 0;
			for (u8 c : m_firmware)
				checksum += c;

			switch (checksum)
			{
				case 0x03: m_firmware_version = FW_A; break;
				case 0x3e: m_firmware_version = FW_B; break;
				case 0xf9: m_firmware_version = FW_C; break;
				case 0xe1: m_firmware_version = FW_D; break;
				default: m_firmware_version = -1; break;
			}

			if (m_firmware_version < 0)
				logerror("Unrecognized firmware version\n");
			else
				logerror("Detected firmware version %c\n", 'A' + m_firmware_version);
		}
	}
}
