// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia, MetalliC
/* emulation of Altera Cyclone EPIC12 FPGA programmed as a blitter */

#include "emu.h"
#include "epic12.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(EPIC12, epic12_device, "epic12", "EPIC12 Blitter")

epic12_device::epic12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, EPIC12, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_ram16(nullptr), m_gfx_size(0), m_bitmaps(nullptr), m_use_ram(nullptr)
	, m_main_ramsize(0), m_main_rammask(0), m_ram16_copy(nullptr), m_work_queue(nullptr)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_port_r_cb(*this)
{
	m_is_unsafe = 0;
	m_delay_scale = 0;
	m_blitter_request = nullptr;
	m_blitter_delay_timer = nullptr;
	m_blitter_busy = 0;
	m_gfx_addr = 0;
	m_gfx_scroll_0_x = 0;
	m_gfx_scroll_0_y = 0;
	m_gfx_scroll_1_x = 0;
	m_gfx_scroll_1_y = 0;
	m_gfx_addr_shadowcopy = 0;
	m_gfx_scroll_0_x_shadowcopy = 0;
	m_gfx_scroll_0_y_shadowcopy = 0;
	m_gfx_scroll_1_x_shadowcopy = 0;
	m_gfx_scroll_1_y_shadowcopy = 0;
	blit_delay = 0;
}

TIMER_CALLBACK_MEMBER(epic12_device::blitter_delay_callback)
{
	m_blitter_busy = 0;
}


void epic12_device::device_start()
{
	m_port_r_cb.resolve_safe(0);

	m_gfx_size = 0x2000 * 0x1000;
	m_bitmaps = std::make_unique<bitmap_rgb32>(0x2000, 0x1000);
	m_clip = m_bitmaps->cliprect();
	m_clip.set(0, 0x2000 - 1, 0, 0x1000 - 1);

#ifdef DEBUG_VRAM_VIEWER
	m_debug_vram_view_en = false;
	m_prev_screen_width = m_curr_screen_width = screen().width();
	m_prev_screen_height = m_curr_screen_height = screen().height();
	m_prev_screen_visarea = m_curr_screen_visarea = screen().visible_area();
#endif

	m_ram16_copy = std::make_unique<u16[]>(m_main_ramsize / 2);

	m_blitter_delay_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(epic12_device::blitter_delay_callback),this));
	m_blitter_delay_timer->adjust(attotime::never);

	save_item(NAME(m_gfx_addr));
	save_item(NAME(m_gfx_scroll_0_x));
	save_item(NAME(m_gfx_scroll_0_y));
	save_item(NAME(m_gfx_scroll_1_x));
	save_item(NAME(m_gfx_scroll_1_y));
	save_item(NAME(m_delay_scale));
	save_item(NAME(m_gfx_addr_shadowcopy));
	save_item(NAME(m_gfx_scroll_0_x_shadowcopy));
	save_item(NAME(m_gfx_scroll_0_y_shadowcopy));
	save_item(NAME(m_gfx_scroll_1_x_shadowcopy));
	save_item(NAME(m_gfx_scroll_1_y_shadowcopy));
	save_pointer(NAME(m_ram16_copy), m_main_ramsize/2);
	save_item(NAME(*m_bitmaps));
}

void epic12_device::device_reset()
{
	if (m_is_unsafe)
	{
		m_use_ram = m_ram16;
		m_work_queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_HIGH_FREQ|WORK_QUEUE_FLAG_MULTI);
	}
	else
	{
		m_use_ram = m_ram16_copy.get(); // slow mode
		m_work_queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_HIGH_FREQ);
	}

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

// todo, get these into the device class without ruining performance
u8 epic12_device::colrtable[0x20][0x40];
u8 epic12_device::colrtable_rev[0x20][0x40];
u8 epic12_device::colrtable_add[0x20][0x20];
u64 epic12_device::blit_delay;

inline u16 epic12_device::READ_NEXT_WORD(offs_t *addr)
{
//  u16 data = space.read_word(*addr); // going through the memory system is 'more correct' but noticeably slower
	const u16 data = m_use_ram[((*addr & m_main_rammask) >> 1) ^ NATIVE_ENDIAN_VALUE_LE_BE(3, 0)];

	*addr += 2;

//  printf("data %04x\n", data);
	return data;
}

inline u16 epic12_device::COPY_NEXT_WORD(address_space &space, offs_t *addr)
{
//  u16 data = space.read_word(*addr); // going through the memory system is 'more correct' but noticeably slower
	const u16 data = m_ram16[((*addr & m_main_rammask) >> 1) ^ NATIVE_ENDIAN_VALUE_LE_BE(3, 0)];
	m_ram16_copy[((*addr & m_main_rammask) >> 1) ^ NATIVE_ENDIAN_VALUE_LE_BE(3, 0)] = data;

	*addr += 2;

//  printf("data %04x\n", data);
	return data;
}


inline void epic12_device::gfx_upload_shadow_copy(address_space &space, offs_t *addr)
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
}

inline void epic12_device::gfx_upload(offs_t *addr)
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


const epic12_device::blitfunction epic12_device::f0_ti1_tr1_blit_funcs[64] =
{
	epic12_device::draw_sprite_f0_ti1_tr1_s0_d0, epic12_device::draw_sprite_f0_ti1_tr1_s1_d0, epic12_device::draw_sprite_f0_ti1_tr1_s2_d0, epic12_device::draw_sprite_f0_ti1_tr1_s3_d0, epic12_device::draw_sprite_f0_ti1_tr1_s4_d0, epic12_device::draw_sprite_f0_ti1_tr1_s5_d0, epic12_device::draw_sprite_f0_ti1_tr1_s6_d0, epic12_device::draw_sprite_f0_ti1_tr1_s7_d0,
	epic12_device::draw_sprite_f0_ti1_tr1_s0_d1, epic12_device::draw_sprite_f0_ti1_tr1_s1_d1, epic12_device::draw_sprite_f0_ti1_tr1_s2_d1, epic12_device::draw_sprite_f0_ti1_tr1_s3_d1, epic12_device::draw_sprite_f0_ti1_tr1_s4_d1, epic12_device::draw_sprite_f0_ti1_tr1_s5_d1, epic12_device::draw_sprite_f0_ti1_tr1_s6_d1, epic12_device::draw_sprite_f0_ti1_tr1_s7_d1,
	epic12_device::draw_sprite_f0_ti1_tr1_s0_d2, epic12_device::draw_sprite_f0_ti1_tr1_s1_d2, epic12_device::draw_sprite_f0_ti1_tr1_s2_d2, epic12_device::draw_sprite_f0_ti1_tr1_s3_d2, epic12_device::draw_sprite_f0_ti1_tr1_s4_d2, epic12_device::draw_sprite_f0_ti1_tr1_s5_d2, epic12_device::draw_sprite_f0_ti1_tr1_s6_d2, epic12_device::draw_sprite_f0_ti1_tr1_s7_d2,
	epic12_device::draw_sprite_f0_ti1_tr1_s0_d3, epic12_device::draw_sprite_f0_ti1_tr1_s1_d3, epic12_device::draw_sprite_f0_ti1_tr1_s2_d3, epic12_device::draw_sprite_f0_ti1_tr1_s3_d3, epic12_device::draw_sprite_f0_ti1_tr1_s4_d3, epic12_device::draw_sprite_f0_ti1_tr1_s5_d3, epic12_device::draw_sprite_f0_ti1_tr1_s6_d3, epic12_device::draw_sprite_f0_ti1_tr1_s7_d3,
	epic12_device::draw_sprite_f0_ti1_tr1_s0_d4, epic12_device::draw_sprite_f0_ti1_tr1_s1_d4, epic12_device::draw_sprite_f0_ti1_tr1_s2_d4, epic12_device::draw_sprite_f0_ti1_tr1_s3_d4, epic12_device::draw_sprite_f0_ti1_tr1_s4_d4, epic12_device::draw_sprite_f0_ti1_tr1_s5_d4, epic12_device::draw_sprite_f0_ti1_tr1_s6_d4, epic12_device::draw_sprite_f0_ti1_tr1_s7_d4,
	epic12_device::draw_sprite_f0_ti1_tr1_s0_d5, epic12_device::draw_sprite_f0_ti1_tr1_s1_d5, epic12_device::draw_sprite_f0_ti1_tr1_s2_d5, epic12_device::draw_sprite_f0_ti1_tr1_s3_d5, epic12_device::draw_sprite_f0_ti1_tr1_s4_d5, epic12_device::draw_sprite_f0_ti1_tr1_s5_d5, epic12_device::draw_sprite_f0_ti1_tr1_s6_d5, epic12_device::draw_sprite_f0_ti1_tr1_s7_d5,
	epic12_device::draw_sprite_f0_ti1_tr1_s0_d6, epic12_device::draw_sprite_f0_ti1_tr1_s1_d6, epic12_device::draw_sprite_f0_ti1_tr1_s2_d6, epic12_device::draw_sprite_f0_ti1_tr1_s3_d6, epic12_device::draw_sprite_f0_ti1_tr1_s4_d6, epic12_device::draw_sprite_f0_ti1_tr1_s5_d6, epic12_device::draw_sprite_f0_ti1_tr1_s6_d6, epic12_device::draw_sprite_f0_ti1_tr1_s7_d6,
	epic12_device::draw_sprite_f0_ti1_tr1_s0_d7, epic12_device::draw_sprite_f0_ti1_tr1_s1_d7, epic12_device::draw_sprite_f0_ti1_tr1_s2_d7, epic12_device::draw_sprite_f0_ti1_tr1_s3_d7, epic12_device::draw_sprite_f0_ti1_tr1_s4_d7, epic12_device::draw_sprite_f0_ti1_tr1_s5_d7, epic12_device::draw_sprite_f0_ti1_tr1_s6_d7, epic12_device::draw_sprite_f0_ti1_tr1_s7_d7,
};

const epic12_device::blitfunction epic12_device::f0_ti1_tr0_blit_funcs[64] =
{
	epic12_device::draw_sprite_f0_ti1_tr0_s0_d0, epic12_device::draw_sprite_f0_ti1_tr0_s1_d0, epic12_device::draw_sprite_f0_ti1_tr0_s2_d0, epic12_device::draw_sprite_f0_ti1_tr0_s3_d0, epic12_device::draw_sprite_f0_ti1_tr0_s4_d0, epic12_device::draw_sprite_f0_ti1_tr0_s5_d0, epic12_device::draw_sprite_f0_ti1_tr0_s6_d0, epic12_device::draw_sprite_f0_ti1_tr0_s7_d0,
	epic12_device::draw_sprite_f0_ti1_tr0_s0_d1, epic12_device::draw_sprite_f0_ti1_tr0_s1_d1, epic12_device::draw_sprite_f0_ti1_tr0_s2_d1, epic12_device::draw_sprite_f0_ti1_tr0_s3_d1, epic12_device::draw_sprite_f0_ti1_tr0_s4_d1, epic12_device::draw_sprite_f0_ti1_tr0_s5_d1, epic12_device::draw_sprite_f0_ti1_tr0_s6_d1, epic12_device::draw_sprite_f0_ti1_tr0_s7_d1,
	epic12_device::draw_sprite_f0_ti1_tr0_s0_d2, epic12_device::draw_sprite_f0_ti1_tr0_s1_d2, epic12_device::draw_sprite_f0_ti1_tr0_s2_d2, epic12_device::draw_sprite_f0_ti1_tr0_s3_d2, epic12_device::draw_sprite_f0_ti1_tr0_s4_d2, epic12_device::draw_sprite_f0_ti1_tr0_s5_d2, epic12_device::draw_sprite_f0_ti1_tr0_s6_d2, epic12_device::draw_sprite_f0_ti1_tr0_s7_d2,
	epic12_device::draw_sprite_f0_ti1_tr0_s0_d3, epic12_device::draw_sprite_f0_ti1_tr0_s1_d3, epic12_device::draw_sprite_f0_ti1_tr0_s2_d3, epic12_device::draw_sprite_f0_ti1_tr0_s3_d3, epic12_device::draw_sprite_f0_ti1_tr0_s4_d3, epic12_device::draw_sprite_f0_ti1_tr0_s5_d3, epic12_device::draw_sprite_f0_ti1_tr0_s6_d3, epic12_device::draw_sprite_f0_ti1_tr0_s7_d3,
	epic12_device::draw_sprite_f0_ti1_tr0_s0_d4, epic12_device::draw_sprite_f0_ti1_tr0_s1_d4, epic12_device::draw_sprite_f0_ti1_tr0_s2_d4, epic12_device::draw_sprite_f0_ti1_tr0_s3_d4, epic12_device::draw_sprite_f0_ti1_tr0_s4_d4, epic12_device::draw_sprite_f0_ti1_tr0_s5_d4, epic12_device::draw_sprite_f0_ti1_tr0_s6_d4, epic12_device::draw_sprite_f0_ti1_tr0_s7_d4,
	epic12_device::draw_sprite_f0_ti1_tr0_s0_d5, epic12_device::draw_sprite_f0_ti1_tr0_s1_d5, epic12_device::draw_sprite_f0_ti1_tr0_s2_d5, epic12_device::draw_sprite_f0_ti1_tr0_s3_d5, epic12_device::draw_sprite_f0_ti1_tr0_s4_d5, epic12_device::draw_sprite_f0_ti1_tr0_s5_d5, epic12_device::draw_sprite_f0_ti1_tr0_s6_d5, epic12_device::draw_sprite_f0_ti1_tr0_s7_d5,
	epic12_device::draw_sprite_f0_ti1_tr0_s0_d6, epic12_device::draw_sprite_f0_ti1_tr0_s1_d6, epic12_device::draw_sprite_f0_ti1_tr0_s2_d6, epic12_device::draw_sprite_f0_ti1_tr0_s3_d6, epic12_device::draw_sprite_f0_ti1_tr0_s4_d6, epic12_device::draw_sprite_f0_ti1_tr0_s5_d6, epic12_device::draw_sprite_f0_ti1_tr0_s6_d6, epic12_device::draw_sprite_f0_ti1_tr0_s7_d6,
	epic12_device::draw_sprite_f0_ti1_tr0_s0_d7, epic12_device::draw_sprite_f0_ti1_tr0_s1_d7, epic12_device::draw_sprite_f0_ti1_tr0_s2_d7, epic12_device::draw_sprite_f0_ti1_tr0_s3_d7, epic12_device::draw_sprite_f0_ti1_tr0_s4_d7, epic12_device::draw_sprite_f0_ti1_tr0_s5_d7, epic12_device::draw_sprite_f0_ti1_tr0_s6_d7, epic12_device::draw_sprite_f0_ti1_tr0_s7_d7,
};

const epic12_device::blitfunction epic12_device::f1_ti1_tr1_blit_funcs[64] =
{
	epic12_device::draw_sprite_f1_ti1_tr1_s0_d0, epic12_device::draw_sprite_f1_ti1_tr1_s1_d0, epic12_device::draw_sprite_f1_ti1_tr1_s2_d0, epic12_device::draw_sprite_f1_ti1_tr1_s3_d0, epic12_device::draw_sprite_f1_ti1_tr1_s4_d0, epic12_device::draw_sprite_f1_ti1_tr1_s5_d0, epic12_device::draw_sprite_f1_ti1_tr1_s6_d0, epic12_device::draw_sprite_f1_ti1_tr1_s7_d0,
	epic12_device::draw_sprite_f1_ti1_tr1_s0_d1, epic12_device::draw_sprite_f1_ti1_tr1_s1_d1, epic12_device::draw_sprite_f1_ti1_tr1_s2_d1, epic12_device::draw_sprite_f1_ti1_tr1_s3_d1, epic12_device::draw_sprite_f1_ti1_tr1_s4_d1, epic12_device::draw_sprite_f1_ti1_tr1_s5_d1, epic12_device::draw_sprite_f1_ti1_tr1_s6_d1, epic12_device::draw_sprite_f1_ti1_tr1_s7_d1,
	epic12_device::draw_sprite_f1_ti1_tr1_s0_d2, epic12_device::draw_sprite_f1_ti1_tr1_s1_d2, epic12_device::draw_sprite_f1_ti1_tr1_s2_d2, epic12_device::draw_sprite_f1_ti1_tr1_s3_d2, epic12_device::draw_sprite_f1_ti1_tr1_s4_d2, epic12_device::draw_sprite_f1_ti1_tr1_s5_d2, epic12_device::draw_sprite_f1_ti1_tr1_s6_d2, epic12_device::draw_sprite_f1_ti1_tr1_s7_d2,
	epic12_device::draw_sprite_f1_ti1_tr1_s0_d3, epic12_device::draw_sprite_f1_ti1_tr1_s1_d3, epic12_device::draw_sprite_f1_ti1_tr1_s2_d3, epic12_device::draw_sprite_f1_ti1_tr1_s3_d3, epic12_device::draw_sprite_f1_ti1_tr1_s4_d3, epic12_device::draw_sprite_f1_ti1_tr1_s5_d3, epic12_device::draw_sprite_f1_ti1_tr1_s6_d3, epic12_device::draw_sprite_f1_ti1_tr1_s7_d3,
	epic12_device::draw_sprite_f1_ti1_tr1_s0_d4, epic12_device::draw_sprite_f1_ti1_tr1_s1_d4, epic12_device::draw_sprite_f1_ti1_tr1_s2_d4, epic12_device::draw_sprite_f1_ti1_tr1_s3_d4, epic12_device::draw_sprite_f1_ti1_tr1_s4_d4, epic12_device::draw_sprite_f1_ti1_tr1_s5_d4, epic12_device::draw_sprite_f1_ti1_tr1_s6_d4, epic12_device::draw_sprite_f1_ti1_tr1_s7_d4,
	epic12_device::draw_sprite_f1_ti1_tr1_s0_d5, epic12_device::draw_sprite_f1_ti1_tr1_s1_d5, epic12_device::draw_sprite_f1_ti1_tr1_s2_d5, epic12_device::draw_sprite_f1_ti1_tr1_s3_d5, epic12_device::draw_sprite_f1_ti1_tr1_s4_d5, epic12_device::draw_sprite_f1_ti1_tr1_s5_d5, epic12_device::draw_sprite_f1_ti1_tr1_s6_d5, epic12_device::draw_sprite_f1_ti1_tr1_s7_d5,
	epic12_device::draw_sprite_f1_ti1_tr1_s0_d6, epic12_device::draw_sprite_f1_ti1_tr1_s1_d6, epic12_device::draw_sprite_f1_ti1_tr1_s2_d6, epic12_device::draw_sprite_f1_ti1_tr1_s3_d6, epic12_device::draw_sprite_f1_ti1_tr1_s4_d6, epic12_device::draw_sprite_f1_ti1_tr1_s5_d6, epic12_device::draw_sprite_f1_ti1_tr1_s6_d6, epic12_device::draw_sprite_f1_ti1_tr1_s7_d6,
	epic12_device::draw_sprite_f1_ti1_tr1_s0_d7, epic12_device::draw_sprite_f1_ti1_tr1_s1_d7, epic12_device::draw_sprite_f1_ti1_tr1_s2_d7, epic12_device::draw_sprite_f1_ti1_tr1_s3_d7, epic12_device::draw_sprite_f1_ti1_tr1_s4_d7, epic12_device::draw_sprite_f1_ti1_tr1_s5_d7, epic12_device::draw_sprite_f1_ti1_tr1_s6_d7, epic12_device::draw_sprite_f1_ti1_tr1_s7_d7,
};

const epic12_device::blitfunction epic12_device::f1_ti1_tr0_blit_funcs[64] =
{
	epic12_device::draw_sprite_f1_ti1_tr0_s0_d0, epic12_device::draw_sprite_f1_ti1_tr0_s1_d0, epic12_device::draw_sprite_f1_ti1_tr0_s2_d0, epic12_device::draw_sprite_f1_ti1_tr0_s3_d0, epic12_device::draw_sprite_f1_ti1_tr0_s4_d0, epic12_device::draw_sprite_f1_ti1_tr0_s5_d0, epic12_device::draw_sprite_f1_ti1_tr0_s6_d0, epic12_device::draw_sprite_f1_ti1_tr0_s7_d0,
	epic12_device::draw_sprite_f1_ti1_tr0_s0_d1, epic12_device::draw_sprite_f1_ti1_tr0_s1_d1, epic12_device::draw_sprite_f1_ti1_tr0_s2_d1, epic12_device::draw_sprite_f1_ti1_tr0_s3_d1, epic12_device::draw_sprite_f1_ti1_tr0_s4_d1, epic12_device::draw_sprite_f1_ti1_tr0_s5_d1, epic12_device::draw_sprite_f1_ti1_tr0_s6_d1, epic12_device::draw_sprite_f1_ti1_tr0_s7_d1,
	epic12_device::draw_sprite_f1_ti1_tr0_s0_d2, epic12_device::draw_sprite_f1_ti1_tr0_s1_d2, epic12_device::draw_sprite_f1_ti1_tr0_s2_d2, epic12_device::draw_sprite_f1_ti1_tr0_s3_d2, epic12_device::draw_sprite_f1_ti1_tr0_s4_d2, epic12_device::draw_sprite_f1_ti1_tr0_s5_d2, epic12_device::draw_sprite_f1_ti1_tr0_s6_d2, epic12_device::draw_sprite_f1_ti1_tr0_s7_d2,
	epic12_device::draw_sprite_f1_ti1_tr0_s0_d3, epic12_device::draw_sprite_f1_ti1_tr0_s1_d3, epic12_device::draw_sprite_f1_ti1_tr0_s2_d3, epic12_device::draw_sprite_f1_ti1_tr0_s3_d3, epic12_device::draw_sprite_f1_ti1_tr0_s4_d3, epic12_device::draw_sprite_f1_ti1_tr0_s5_d3, epic12_device::draw_sprite_f1_ti1_tr0_s6_d3, epic12_device::draw_sprite_f1_ti1_tr0_s7_d3,
	epic12_device::draw_sprite_f1_ti1_tr0_s0_d4, epic12_device::draw_sprite_f1_ti1_tr0_s1_d4, epic12_device::draw_sprite_f1_ti1_tr0_s2_d4, epic12_device::draw_sprite_f1_ti1_tr0_s3_d4, epic12_device::draw_sprite_f1_ti1_tr0_s4_d4, epic12_device::draw_sprite_f1_ti1_tr0_s5_d4, epic12_device::draw_sprite_f1_ti1_tr0_s6_d4, epic12_device::draw_sprite_f1_ti1_tr0_s7_d4,
	epic12_device::draw_sprite_f1_ti1_tr0_s0_d5, epic12_device::draw_sprite_f1_ti1_tr0_s1_d5, epic12_device::draw_sprite_f1_ti1_tr0_s2_d5, epic12_device::draw_sprite_f1_ti1_tr0_s3_d5, epic12_device::draw_sprite_f1_ti1_tr0_s4_d5, epic12_device::draw_sprite_f1_ti1_tr0_s5_d5, epic12_device::draw_sprite_f1_ti1_tr0_s6_d5, epic12_device::draw_sprite_f1_ti1_tr0_s7_d5,
	epic12_device::draw_sprite_f1_ti1_tr0_s0_d6, epic12_device::draw_sprite_f1_ti1_tr0_s1_d6, epic12_device::draw_sprite_f1_ti1_tr0_s2_d6, epic12_device::draw_sprite_f1_ti1_tr0_s3_d6, epic12_device::draw_sprite_f1_ti1_tr0_s4_d6, epic12_device::draw_sprite_f1_ti1_tr0_s5_d6, epic12_device::draw_sprite_f1_ti1_tr0_s6_d6, epic12_device::draw_sprite_f1_ti1_tr0_s7_d6,
	epic12_device::draw_sprite_f1_ti1_tr0_s0_d7, epic12_device::draw_sprite_f1_ti1_tr0_s1_d7, epic12_device::draw_sprite_f1_ti1_tr0_s2_d7, epic12_device::draw_sprite_f1_ti1_tr0_s3_d7, epic12_device::draw_sprite_f1_ti1_tr0_s4_d7, epic12_device::draw_sprite_f1_ti1_tr0_s5_d7, epic12_device::draw_sprite_f1_ti1_tr0_s6_d7, epic12_device::draw_sprite_f1_ti1_tr0_s7_d7,
};


const epic12_device::blitfunction epic12_device::f0_ti0_tr1_blit_funcs[64] =
{
	epic12_device::draw_sprite_f0_ti0_tr1_s0_d0, epic12_device::draw_sprite_f0_ti0_tr1_s1_d0, epic12_device::draw_sprite_f0_ti0_tr1_s2_d0, epic12_device::draw_sprite_f0_ti0_tr1_s3_d0, epic12_device::draw_sprite_f0_ti0_tr1_s4_d0, epic12_device::draw_sprite_f0_ti0_tr1_s5_d0, epic12_device::draw_sprite_f0_ti0_tr1_s6_d0, epic12_device::draw_sprite_f0_ti0_tr1_s7_d0,
	epic12_device::draw_sprite_f0_ti0_tr1_s0_d1, epic12_device::draw_sprite_f0_ti0_tr1_s1_d1, epic12_device::draw_sprite_f0_ti0_tr1_s2_d1, epic12_device::draw_sprite_f0_ti0_tr1_s3_d1, epic12_device::draw_sprite_f0_ti0_tr1_s4_d1, epic12_device::draw_sprite_f0_ti0_tr1_s5_d1, epic12_device::draw_sprite_f0_ti0_tr1_s6_d1, epic12_device::draw_sprite_f0_ti0_tr1_s7_d1,
	epic12_device::draw_sprite_f0_ti0_tr1_s0_d2, epic12_device::draw_sprite_f0_ti0_tr1_s1_d2, epic12_device::draw_sprite_f0_ti0_tr1_s2_d2, epic12_device::draw_sprite_f0_ti0_tr1_s3_d2, epic12_device::draw_sprite_f0_ti0_tr1_s4_d2, epic12_device::draw_sprite_f0_ti0_tr1_s5_d2, epic12_device::draw_sprite_f0_ti0_tr1_s6_d2, epic12_device::draw_sprite_f0_ti0_tr1_s7_d2,
	epic12_device::draw_sprite_f0_ti0_tr1_s0_d3, epic12_device::draw_sprite_f0_ti0_tr1_s1_d3, epic12_device::draw_sprite_f0_ti0_tr1_s2_d3, epic12_device::draw_sprite_f0_ti0_tr1_s3_d3, epic12_device::draw_sprite_f0_ti0_tr1_s4_d3, epic12_device::draw_sprite_f0_ti0_tr1_s5_d3, epic12_device::draw_sprite_f0_ti0_tr1_s6_d3, epic12_device::draw_sprite_f0_ti0_tr1_s7_d3,
	epic12_device::draw_sprite_f0_ti0_tr1_s0_d4, epic12_device::draw_sprite_f0_ti0_tr1_s1_d4, epic12_device::draw_sprite_f0_ti0_tr1_s2_d4, epic12_device::draw_sprite_f0_ti0_tr1_s3_d4, epic12_device::draw_sprite_f0_ti0_tr1_s4_d4, epic12_device::draw_sprite_f0_ti0_tr1_s5_d4, epic12_device::draw_sprite_f0_ti0_tr1_s6_d4, epic12_device::draw_sprite_f0_ti0_tr1_s7_d4,
	epic12_device::draw_sprite_f0_ti0_tr1_s0_d5, epic12_device::draw_sprite_f0_ti0_tr1_s1_d5, epic12_device::draw_sprite_f0_ti0_tr1_s2_d5, epic12_device::draw_sprite_f0_ti0_tr1_s3_d5, epic12_device::draw_sprite_f0_ti0_tr1_s4_d5, epic12_device::draw_sprite_f0_ti0_tr1_s5_d5, epic12_device::draw_sprite_f0_ti0_tr1_s6_d5, epic12_device::draw_sprite_f0_ti0_tr1_s7_d5,
	epic12_device::draw_sprite_f0_ti0_tr1_s0_d6, epic12_device::draw_sprite_f0_ti0_tr1_s1_d6, epic12_device::draw_sprite_f0_ti0_tr1_s2_d6, epic12_device::draw_sprite_f0_ti0_tr1_s3_d6, epic12_device::draw_sprite_f0_ti0_tr1_s4_d6, epic12_device::draw_sprite_f0_ti0_tr1_s5_d6, epic12_device::draw_sprite_f0_ti0_tr1_s6_d6, epic12_device::draw_sprite_f0_ti0_tr1_s7_d6,
	epic12_device::draw_sprite_f0_ti0_tr1_s0_d7, epic12_device::draw_sprite_f0_ti0_tr1_s1_d7, epic12_device::draw_sprite_f0_ti0_tr1_s2_d7, epic12_device::draw_sprite_f0_ti0_tr1_s3_d7, epic12_device::draw_sprite_f0_ti0_tr1_s4_d7, epic12_device::draw_sprite_f0_ti0_tr1_s5_d7, epic12_device::draw_sprite_f0_ti0_tr1_s6_d7, epic12_device::draw_sprite_f0_ti0_tr1_s7_d7,
};

const epic12_device::blitfunction epic12_device::f0_ti0_tr0_blit_funcs[64] =
{
	epic12_device::draw_sprite_f0_ti0_tr0_s0_d0, epic12_device::draw_sprite_f0_ti0_tr0_s1_d0, epic12_device::draw_sprite_f0_ti0_tr0_s2_d0, epic12_device::draw_sprite_f0_ti0_tr0_s3_d0, epic12_device::draw_sprite_f0_ti0_tr0_s4_d0, epic12_device::draw_sprite_f0_ti0_tr0_s5_d0, epic12_device::draw_sprite_f0_ti0_tr0_s6_d0, epic12_device::draw_sprite_f0_ti0_tr0_s7_d0,
	epic12_device::draw_sprite_f0_ti0_tr0_s0_d1, epic12_device::draw_sprite_f0_ti0_tr0_s1_d1, epic12_device::draw_sprite_f0_ti0_tr0_s2_d1, epic12_device::draw_sprite_f0_ti0_tr0_s3_d1, epic12_device::draw_sprite_f0_ti0_tr0_s4_d1, epic12_device::draw_sprite_f0_ti0_tr0_s5_d1, epic12_device::draw_sprite_f0_ti0_tr0_s6_d1, epic12_device::draw_sprite_f0_ti0_tr0_s7_d1,
	epic12_device::draw_sprite_f0_ti0_tr0_s0_d2, epic12_device::draw_sprite_f0_ti0_tr0_s1_d2, epic12_device::draw_sprite_f0_ti0_tr0_s2_d2, epic12_device::draw_sprite_f0_ti0_tr0_s3_d2, epic12_device::draw_sprite_f0_ti0_tr0_s4_d2, epic12_device::draw_sprite_f0_ti0_tr0_s5_d2, epic12_device::draw_sprite_f0_ti0_tr0_s6_d2, epic12_device::draw_sprite_f0_ti0_tr0_s7_d2,
	epic12_device::draw_sprite_f0_ti0_tr0_s0_d3, epic12_device::draw_sprite_f0_ti0_tr0_s1_d3, epic12_device::draw_sprite_f0_ti0_tr0_s2_d3, epic12_device::draw_sprite_f0_ti0_tr0_s3_d3, epic12_device::draw_sprite_f0_ti0_tr0_s4_d3, epic12_device::draw_sprite_f0_ti0_tr0_s5_d3, epic12_device::draw_sprite_f0_ti0_tr0_s6_d3, epic12_device::draw_sprite_f0_ti0_tr0_s7_d3,
	epic12_device::draw_sprite_f0_ti0_tr0_s0_d4, epic12_device::draw_sprite_f0_ti0_tr0_s1_d4, epic12_device::draw_sprite_f0_ti0_tr0_s2_d4, epic12_device::draw_sprite_f0_ti0_tr0_s3_d4, epic12_device::draw_sprite_f0_ti0_tr0_s4_d4, epic12_device::draw_sprite_f0_ti0_tr0_s5_d4, epic12_device::draw_sprite_f0_ti0_tr0_s6_d4, epic12_device::draw_sprite_f0_ti0_tr0_s7_d4,
	epic12_device::draw_sprite_f0_ti0_tr0_s0_d5, epic12_device::draw_sprite_f0_ti0_tr0_s1_d5, epic12_device::draw_sprite_f0_ti0_tr0_s2_d5, epic12_device::draw_sprite_f0_ti0_tr0_s3_d5, epic12_device::draw_sprite_f0_ti0_tr0_s4_d5, epic12_device::draw_sprite_f0_ti0_tr0_s5_d5, epic12_device::draw_sprite_f0_ti0_tr0_s6_d5, epic12_device::draw_sprite_f0_ti0_tr0_s7_d5,
	epic12_device::draw_sprite_f0_ti0_tr0_s0_d6, epic12_device::draw_sprite_f0_ti0_tr0_s1_d6, epic12_device::draw_sprite_f0_ti0_tr0_s2_d6, epic12_device::draw_sprite_f0_ti0_tr0_s3_d6, epic12_device::draw_sprite_f0_ti0_tr0_s4_d6, epic12_device::draw_sprite_f0_ti0_tr0_s5_d6, epic12_device::draw_sprite_f0_ti0_tr0_s6_d6, epic12_device::draw_sprite_f0_ti0_tr0_s7_d6,
	epic12_device::draw_sprite_f0_ti0_tr0_s0_d7, epic12_device::draw_sprite_f0_ti0_tr0_s1_d7, epic12_device::draw_sprite_f0_ti0_tr0_s2_d7, epic12_device::draw_sprite_f0_ti0_tr0_s3_d7, epic12_device::draw_sprite_f0_ti0_tr0_s4_d7, epic12_device::draw_sprite_f0_ti0_tr0_s5_d7, epic12_device::draw_sprite_f0_ti0_tr0_s6_d7, epic12_device::draw_sprite_f0_ti0_tr0_s7_d7,
};

const epic12_device::blitfunction epic12_device::f1_ti0_tr1_blit_funcs[64] =
{
	epic12_device::draw_sprite_f1_ti0_tr1_s0_d0, epic12_device::draw_sprite_f1_ti0_tr1_s1_d0, epic12_device::draw_sprite_f1_ti0_tr1_s2_d0, epic12_device::draw_sprite_f1_ti0_tr1_s3_d0, epic12_device::draw_sprite_f1_ti0_tr1_s4_d0, epic12_device::draw_sprite_f1_ti0_tr1_s5_d0, epic12_device::draw_sprite_f1_ti0_tr1_s6_d0, epic12_device::draw_sprite_f1_ti0_tr1_s7_d0,
	epic12_device::draw_sprite_f1_ti0_tr1_s0_d1, epic12_device::draw_sprite_f1_ti0_tr1_s1_d1, epic12_device::draw_sprite_f1_ti0_tr1_s2_d1, epic12_device::draw_sprite_f1_ti0_tr1_s3_d1, epic12_device::draw_sprite_f1_ti0_tr1_s4_d1, epic12_device::draw_sprite_f1_ti0_tr1_s5_d1, epic12_device::draw_sprite_f1_ti0_tr1_s6_d1, epic12_device::draw_sprite_f1_ti0_tr1_s7_d1,
	epic12_device::draw_sprite_f1_ti0_tr1_s0_d2, epic12_device::draw_sprite_f1_ti0_tr1_s1_d2, epic12_device::draw_sprite_f1_ti0_tr1_s2_d2, epic12_device::draw_sprite_f1_ti0_tr1_s3_d2, epic12_device::draw_sprite_f1_ti0_tr1_s4_d2, epic12_device::draw_sprite_f1_ti0_tr1_s5_d2, epic12_device::draw_sprite_f1_ti0_tr1_s6_d2, epic12_device::draw_sprite_f1_ti0_tr1_s7_d2,
	epic12_device::draw_sprite_f1_ti0_tr1_s0_d3, epic12_device::draw_sprite_f1_ti0_tr1_s1_d3, epic12_device::draw_sprite_f1_ti0_tr1_s2_d3, epic12_device::draw_sprite_f1_ti0_tr1_s3_d3, epic12_device::draw_sprite_f1_ti0_tr1_s4_d3, epic12_device::draw_sprite_f1_ti0_tr1_s5_d3, epic12_device::draw_sprite_f1_ti0_tr1_s6_d3, epic12_device::draw_sprite_f1_ti0_tr1_s7_d3,
	epic12_device::draw_sprite_f1_ti0_tr1_s0_d4, epic12_device::draw_sprite_f1_ti0_tr1_s1_d4, epic12_device::draw_sprite_f1_ti0_tr1_s2_d4, epic12_device::draw_sprite_f1_ti0_tr1_s3_d4, epic12_device::draw_sprite_f1_ti0_tr1_s4_d4, epic12_device::draw_sprite_f1_ti0_tr1_s5_d4, epic12_device::draw_sprite_f1_ti0_tr1_s6_d4, epic12_device::draw_sprite_f1_ti0_tr1_s7_d4,
	epic12_device::draw_sprite_f1_ti0_tr1_s0_d5, epic12_device::draw_sprite_f1_ti0_tr1_s1_d5, epic12_device::draw_sprite_f1_ti0_tr1_s2_d5, epic12_device::draw_sprite_f1_ti0_tr1_s3_d5, epic12_device::draw_sprite_f1_ti0_tr1_s4_d5, epic12_device::draw_sprite_f1_ti0_tr1_s5_d5, epic12_device::draw_sprite_f1_ti0_tr1_s6_d5, epic12_device::draw_sprite_f1_ti0_tr1_s7_d5,
	epic12_device::draw_sprite_f1_ti0_tr1_s0_d6, epic12_device::draw_sprite_f1_ti0_tr1_s1_d6, epic12_device::draw_sprite_f1_ti0_tr1_s2_d6, epic12_device::draw_sprite_f1_ti0_tr1_s3_d6, epic12_device::draw_sprite_f1_ti0_tr1_s4_d6, epic12_device::draw_sprite_f1_ti0_tr1_s5_d6, epic12_device::draw_sprite_f1_ti0_tr1_s6_d6, epic12_device::draw_sprite_f1_ti0_tr1_s7_d6,
	epic12_device::draw_sprite_f1_ti0_tr1_s0_d7, epic12_device::draw_sprite_f1_ti0_tr1_s1_d7, epic12_device::draw_sprite_f1_ti0_tr1_s2_d7, epic12_device::draw_sprite_f1_ti0_tr1_s3_d7, epic12_device::draw_sprite_f1_ti0_tr1_s4_d7, epic12_device::draw_sprite_f1_ti0_tr1_s5_d7, epic12_device::draw_sprite_f1_ti0_tr1_s6_d7, epic12_device::draw_sprite_f1_ti0_tr1_s7_d7,
};

const epic12_device::blitfunction epic12_device::f1_ti0_tr0_blit_funcs[64] =
{
	epic12_device::draw_sprite_f1_ti0_tr0_s0_d0, epic12_device::draw_sprite_f1_ti0_tr0_s1_d0, epic12_device::draw_sprite_f1_ti0_tr0_s2_d0, epic12_device::draw_sprite_f1_ti0_tr0_s3_d0, epic12_device::draw_sprite_f1_ti0_tr0_s4_d0, epic12_device::draw_sprite_f1_ti0_tr0_s5_d0, epic12_device::draw_sprite_f1_ti0_tr0_s6_d0, epic12_device::draw_sprite_f1_ti0_tr0_s7_d0,
	epic12_device::draw_sprite_f1_ti0_tr0_s0_d1, epic12_device::draw_sprite_f1_ti0_tr0_s1_d1, epic12_device::draw_sprite_f1_ti0_tr0_s2_d1, epic12_device::draw_sprite_f1_ti0_tr0_s3_d1, epic12_device::draw_sprite_f1_ti0_tr0_s4_d1, epic12_device::draw_sprite_f1_ti0_tr0_s5_d1, epic12_device::draw_sprite_f1_ti0_tr0_s6_d1, epic12_device::draw_sprite_f1_ti0_tr0_s7_d1,
	epic12_device::draw_sprite_f1_ti0_tr0_s0_d2, epic12_device::draw_sprite_f1_ti0_tr0_s1_d2, epic12_device::draw_sprite_f1_ti0_tr0_s2_d2, epic12_device::draw_sprite_f1_ti0_tr0_s3_d2, epic12_device::draw_sprite_f1_ti0_tr0_s4_d2, epic12_device::draw_sprite_f1_ti0_tr0_s5_d2, epic12_device::draw_sprite_f1_ti0_tr0_s6_d2, epic12_device::draw_sprite_f1_ti0_tr0_s7_d2,
	epic12_device::draw_sprite_f1_ti0_tr0_s0_d3, epic12_device::draw_sprite_f1_ti0_tr0_s1_d3, epic12_device::draw_sprite_f1_ti0_tr0_s2_d3, epic12_device::draw_sprite_f1_ti0_tr0_s3_d3, epic12_device::draw_sprite_f1_ti0_tr0_s4_d3, epic12_device::draw_sprite_f1_ti0_tr0_s5_d3, epic12_device::draw_sprite_f1_ti0_tr0_s6_d3, epic12_device::draw_sprite_f1_ti0_tr0_s7_d3,
	epic12_device::draw_sprite_f1_ti0_tr0_s0_d4, epic12_device::draw_sprite_f1_ti0_tr0_s1_d4, epic12_device::draw_sprite_f1_ti0_tr0_s2_d4, epic12_device::draw_sprite_f1_ti0_tr0_s3_d4, epic12_device::draw_sprite_f1_ti0_tr0_s4_d4, epic12_device::draw_sprite_f1_ti0_tr0_s5_d4, epic12_device::draw_sprite_f1_ti0_tr0_s6_d4, epic12_device::draw_sprite_f1_ti0_tr0_s7_d4,
	epic12_device::draw_sprite_f1_ti0_tr0_s0_d5, epic12_device::draw_sprite_f1_ti0_tr0_s1_d5, epic12_device::draw_sprite_f1_ti0_tr0_s2_d5, epic12_device::draw_sprite_f1_ti0_tr0_s3_d5, epic12_device::draw_sprite_f1_ti0_tr0_s4_d5, epic12_device::draw_sprite_f1_ti0_tr0_s5_d5, epic12_device::draw_sprite_f1_ti0_tr0_s6_d5, epic12_device::draw_sprite_f1_ti0_tr0_s7_d5,
	epic12_device::draw_sprite_f1_ti0_tr0_s0_d6, epic12_device::draw_sprite_f1_ti0_tr0_s1_d6, epic12_device::draw_sprite_f1_ti0_tr0_s2_d6, epic12_device::draw_sprite_f1_ti0_tr0_s3_d6, epic12_device::draw_sprite_f1_ti0_tr0_s4_d6, epic12_device::draw_sprite_f1_ti0_tr0_s5_d6, epic12_device::draw_sprite_f1_ti0_tr0_s6_d6, epic12_device::draw_sprite_f1_ti0_tr0_s7_d6,
	epic12_device::draw_sprite_f1_ti0_tr0_s0_d7, epic12_device::draw_sprite_f1_ti0_tr0_s1_d7, epic12_device::draw_sprite_f1_ti0_tr0_s2_d7, epic12_device::draw_sprite_f1_ti0_tr0_s3_d7, epic12_device::draw_sprite_f1_ti0_tr0_s4_d7, epic12_device::draw_sprite_f1_ti0_tr0_s5_d7, epic12_device::draw_sprite_f1_ti0_tr0_s6_d7, epic12_device::draw_sprite_f1_ti0_tr0_s7_d7,
};


inline void epic12_device::gfx_draw_shadow_copy(address_space &space, offs_t *addr)
{
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr); // const u16 dst_x_start = COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr); // const u16 dst_y_start = COPY_NEXT_WORD(space, addr);
	const u16 w = COPY_NEXT_WORD(space, addr);
	const u16 h = COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);

	// todo, calcualte clipping.
	blit_delay += w * h;
}


inline void epic12_device::gfx_draw(offs_t *addr)
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


void epic12_device::gfx_create_shadow_copy(address_space &space)
{
	offs_t addr = m_gfx_addr & 0x1fffffff;
	m_clip.set(m_gfx_scroll_1_x_shadowcopy, m_gfx_scroll_1_x_shadowcopy + 320-1, m_gfx_scroll_1_y_shadowcopy, m_gfx_scroll_1_y_shadowcopy + 240-1);

	while (1)
	{
		const u16 data = COPY_NEXT_WORD(space, &addr);

		switch (data & 0xf000)
		{
			case 0x0000:
			case 0xf000:
				return;

			case 0xc000:
				if (COPY_NEXT_WORD(space, &addr)) // cliptype
					m_clip.set(m_gfx_scroll_1_x_shadowcopy, m_gfx_scroll_1_x_shadowcopy + 320 - 1, m_gfx_scroll_1_y_shadowcopy, m_gfx_scroll_1_y_shadowcopy + 240 - 1);
				else
					m_clip.set(0, 0x2000 - 1, 0, 0x1000 - 1);
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


void epic12_device::gfx_exec(void)
{
	offs_t addr = m_gfx_addr_shadowcopy & 0x1fffffff;
	m_clip.set(m_gfx_scroll_1_x_shadowcopy, m_gfx_scroll_1_x_shadowcopy + 320 - 1, m_gfx_scroll_1_y_shadowcopy, m_gfx_scroll_1_y_shadowcopy + 240 - 1);

//  logerror("GFX EXEC: %08X\n", addr);

	while (1)
	{
		const u16 data = READ_NEXT_WORD(&addr);

		switch (data & 0xf000)
		{
			case 0x0000:
			case 0xf000:
				return;

			case 0xc000:
				if (READ_NEXT_WORD(&addr)) // cliptype
					m_clip.set(m_gfx_scroll_1_x_shadowcopy, m_gfx_scroll_1_x_shadowcopy + 320 - 1, m_gfx_scroll_1_y_shadowcopy, m_gfx_scroll_1_y_shadowcopy + 240 - 1);
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


void epic12_device::gfx_exec_unsafe(void)
{
	offs_t addr = m_gfx_addr & 0x1fffffff;
	m_clip.set(m_gfx_scroll_1_x, m_gfx_scroll_1_x + 320 - 1, m_gfx_scroll_1_y, m_gfx_scroll_1_y + 240 - 1);

//  logerror("GFX EXEC: %08X\n", addr);

	while (1)
	{
		const u16 data = READ_NEXT_WORD(&addr);

		switch (data & 0xf000)
		{
			case 0x0000:
			case 0xf000:
				return;

			case 0xc000:
				if (READ_NEXT_WORD(&addr)) // cliptype
					m_clip.set(m_gfx_scroll_1_x, m_gfx_scroll_1_x + 320 - 1, m_gfx_scroll_1_y, m_gfx_scroll_1_y + 240 - 1);
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


void *epic12_device::blit_request_callback(void *param, int threadid)
{
	epic12_device *object = reinterpret_cast<epic12_device *>(param);

	object->gfx_exec();
	return nullptr;
}


void *epic12_device::blit_request_callback_unsafe(void *param, int threadid)
{
	epic12_device *object = reinterpret_cast<epic12_device *>(param);

	blit_delay = 0;
	object->gfx_exec_unsafe();
	return nullptr;
}


u32 epic12_device::gfx_ready_r()
{
	return 0x00000010;
}

u32 epic12_device::gfx_ready_r_unsafe()
{
	if (m_blitter_busy)
	{
		m_maincpu->spin_until_time(attotime::from_usec(10));
		return 0x00000000;
	}
	else
		return 0x00000010;
}

WRITE32_MEMBER(epic12_device::gfx_exec_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (data & 1)
		{
			//g_profiler.start(PROFILER_USER1);
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

			blit_delay = 0;
			gfx_create_shadow_copy(space); // create a copy of the blit list so we can safely thread it.

			if (blit_delay)
			{
				m_blitter_busy = 1;
				m_blitter_delay_timer->adjust(attotime::from_nsec(blit_delay*8)); // NOT accurate timing (currently ignored anyway)
			}

			m_gfx_addr_shadowcopy = m_gfx_addr;
			m_gfx_scroll_0_x_shadowcopy =  m_gfx_scroll_0_x;
			m_gfx_scroll_0_y_shadowcopy = m_gfx_scroll_0_y;
			m_gfx_scroll_1_x_shadowcopy = m_gfx_scroll_1_x;
			m_gfx_scroll_1_y_shadowcopy = m_gfx_scroll_1_y;
			m_blitter_request = osd_work_item_queue(m_work_queue, blit_request_callback, (void*)this, 0);
			//g_profiler.stop();
		}
	}
}


void epic12_device::gfx_exec_w_unsafe(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		if (data & 1)
		{
			//g_profiler.start(PROFILER_USER1);
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

			if (blit_delay)
			{
				m_blitter_busy = 1;
				int delay = blit_delay*(15 * m_delay_scale / 50);
				//printf("delay %d\n", delay);
				m_blitter_delay_timer->adjust(attotime::from_nsec(delay));
			}
			else
			{
				m_blitter_busy = 0;
			}

			m_blitter_request = osd_work_item_queue(m_work_queue, blit_request_callback_unsafe, (void*)this, 0);
			//g_profiler.stop();
		}
	}
}


void epic12_device::draw_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_is_unsafe)
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
	}

	int scroll_0_x, scroll_0_y;
//  int scroll_1_x, scroll_1_y;

	bitmap.fill(0, cliprect);

#ifdef DEBUG_VRAM_VIEWER
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

	scroll_0_x = -m_gfx_scroll_0_x;
	scroll_0_y = -m_gfx_scroll_0_y;
//  scroll_1_x = -m_gfx_scroll_1_x;
//  scroll_1_y = -m_gfx_scroll_1_y;

	//printf("SCREEN UPDATE\n %d %d %d %d\n", scroll_0_x, scroll_0_y, scroll_1_x, scroll_1_y);

#ifdef DEBUG_VRAM_VIEWER
	if (m_debug_vram_view_en)
		copybitmap(bitmap, *m_bitmaps, 0, 0, 0, 0, cliprect);
	else
#endif
		copyscrollbitmap(bitmap, *m_bitmaps, 1, &scroll_0_x, 1, &scroll_0_y, cliprect);
}


READ32_MEMBER(epic12_device::blitter_r)
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

READ32_MEMBER(epic12_device::blitter_r_unsafe)
{
	switch (offset * 4)
	{
		case 0x10:
			return gfx_ready_r_unsafe();

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


WRITE32_MEMBER(epic12_device::blitter_w)
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
			COMBINE_DATA(&m_gfx_scroll_0_x);
			break;

		case 0x18:
			COMBINE_DATA(&m_gfx_scroll_0_y);
			break;

		case 0x40:
			COMBINE_DATA(&m_gfx_scroll_1_x);
			break;

		case 0x44:
			COMBINE_DATA(&m_gfx_scroll_1_y);
			break;

	}
}

WRITE32_MEMBER(epic12_device::blitter_w_unsafe)
{
	switch (offset * 4)
	{
		case 0x04:
			gfx_exec_w_unsafe(offset, data, mem_mask);
			break;

		case 0x08:
			COMBINE_DATA(&m_gfx_addr);
			break;

		case 0x14:
			COMBINE_DATA(&m_gfx_scroll_0_x);
			break;

		case 0x18:
			COMBINE_DATA(&m_gfx_scroll_0_y);
			break;

		case 0x40:
			COMBINE_DATA(&m_gfx_scroll_1_x);
			break;

		case 0x44:
			COMBINE_DATA(&m_gfx_scroll_1_y);
			break;

	}
}

void epic12_device::install_handlers(int addr1, int addr2)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	read32_delegate read(*this);
	write32_delegate write(*this);

	if (m_is_unsafe)
	{
		printf("using unsafe blit code!\n");
		read = read32_delegate(*this, FUNC(epic12_device::blitter_r_unsafe));
		write = write32_delegate(*this, FUNC(epic12_device::blitter_w_unsafe));
	}
	else
	{
		read = read32_delegate(*this, FUNC(epic12_device::blitter_r));
		write = write32_delegate(*this, FUNC(epic12_device::blitter_w));
	}

	space.install_readwrite_handler(addr1, addr2, std::move(read), std::move(write), 0xffffffffffffffffU);
}

u64 epic12_device::fpga_r()
{
	return 0xff;
}

// todo, store what's written here and checksum it, different microcode probably leads to slightly different blitter timings
void epic12_device::fpga_w(offs_t offset, u64 data, u64 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		// data & 0x08 = CE
		// data & 0x10 = CLK
		// data & 0x20 = DATA
	}
}
