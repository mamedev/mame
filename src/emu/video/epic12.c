/* emulation of Altera Cyclone EPIC12 FPGA programmed as a blitter */

#include "emu.h"
#include "epic12.h"



const device_type EPIC12 = &device_creator<epic12_device>;

epic12_device::epic12_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, EPIC12, "epic12_device", tag, owner, clock, "epic12", __FILE__),
		device_video_interface(mconfig, *this)
{
	m_is_unsafe = 0;
	m_delay_scale = 0;
	m_maincpu = 0;
	queue = 0;
	blitter_request = 0;
	epic12_device_blitter_delay_timer = 0;
	blitter_busy = 0;
	use_ram = 0;
	epic12_device_ram16 = 0;
	epic12_device_gfx_addr = 0;
	epic12_device_gfx_scroll_0_x = 0;
	epic12_device_gfx_scroll_0_y = 0;
	epic12_device_gfx_scroll_1_x = 0;
	epic12_device_gfx_scroll_1_y = 0;
	epic12_device_gfx_size = 0;
	epic12_device_gfx_addr_shadowcopy = 0;
	epic12_device_gfx_scroll_0_x_shadowcopy = 0;
	epic12_device_gfx_scroll_0_y_shadowcopy = 0;
	epic12_device_gfx_scroll_1_x_shadowcopy = 0;
	epic12_device_gfx_scroll_1_y_shadowcopy = 0;
	epic12_device_ram16_copy = 0;
	epic12_device_blit_delay = 0;

}

TIMER_CALLBACK_MEMBER( epic12_device::epic12_device_blitter_delay_callback )
{
	blitter_busy = 0;
}

// static
	void epic12_device::set_rambase(device_t &device, UINT16* rambase)
{
	epic12_device &dev = downcast<epic12_device &>(device);
	dev.epic12_device_ram16 = rambase;
}


void epic12_device::set_delay_scale(device_t &device, int delay_scale)
{
	epic12_device &dev = downcast<epic12_device &>(device);
	dev.m_delay_scale = delay_scale;
}

void epic12_device::set_is_unsafe(device_t &device, int is_unsafe)
{
	epic12_device &dev = downcast<epic12_device &>(device);
	dev.m_is_unsafe = is_unsafe;

}

void epic12_device::set_cpu_device(device_t &device, cpu_device* maincpu)
{
	epic12_device &dev = downcast<epic12_device &>(device);
	dev.m_maincpu = maincpu;
}


void epic12_device::device_start()
{
	epic12_device_gfx_size = 0x2000 * 0x1000;
	epic12_device_bitmaps = auto_bitmap_rgb32_alloc(machine(), 0x2000, 0x1000);
	epic12_device_clip = epic12_device_bitmaps->cliprect();

	epic12_device_ram16_copy = auto_alloc_array(machine(), UINT16, m_main_ramsize/2);



	epic12_device_blitter_delay_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(epic12_device::epic12_device_blitter_delay_callback),this));
	epic12_device_blitter_delay_timer->adjust(attotime::never);


}

void epic12_device::device_reset()
{
	if (m_is_unsafe)
	{
		use_ram = epic12_device_ram16;
		queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_HIGH_FREQ|WORK_QUEUE_FLAG_MULTI);
	}
	else
	{
		use_ram = epic12_device_ram16_copy; // slow mode
		queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_HIGH_FREQ);
	}


	// cache table to avoid divides in blit code, also pre-clamped
	int x,y;
	for (y=0;y<0x40;y++)
	{
		for (x=0;x<0x20;x++)
		{
			epic12_device_colrtable[x][y] = (x*y) / 0x1f;
			if (epic12_device_colrtable[x][y]>0x1f) epic12_device_colrtable[x][y] = 0x1f;

			epic12_device_colrtable_rev[x^0x1f][y] = (x*y) / 0x1f;
			if (epic12_device_colrtable_rev[x^0x1f][y]>0x1f) epic12_device_colrtable_rev[x^0x1f][y] = 0x1f;
		}
	}

	// preclamped add table
	for (y=0;y<0x20;y++)
	{
		for (x=0;x<0x20;x++)
		{
			epic12_device_colrtable_add[x][y] = (x+y);
			if (epic12_device_colrtable_add[x][y]>0x1f) epic12_device_colrtable_add[x][y] = 0x1f;
		}
	}

	blitter_busy = 0;

}

// todo, get these into the device class without ruining performance
UINT8 epic12_device_colrtable[0x20][0x40];
UINT8 epic12_device_colrtable_rev[0x20][0x40];
UINT8 epic12_device_colrtable_add[0x20][0x20];
UINT64 epic12_device_blit_delay;

inline UINT16 epic12_device::READ_NEXT_WORD(offs_t *addr)
{
//  UINT16 data = space.read_word(*addr); // going through the memory system is 'more correct' but noticably slower
	UINT16 data = use_ram[((*addr & m_main_rammask) >> 1) ^ NATIVE_ENDIAN_VALUE_LE_BE(3, 0)];

	*addr += 2;

//  printf("data %04x\n", data);
	return data;
}

inline UINT16 epic12_device::COPY_NEXT_WORD(address_space &space, offs_t *addr)
{
//  UINT16 data = space.read_word(*addr); // going through the memory system is 'more correct' but noticably slower
	UINT16 data = epic12_device_ram16[((*addr & m_main_rammask) >> 1) ^ NATIVE_ENDIAN_VALUE_LE_BE(3, 0)];
	epic12_device_ram16_copy[((*addr & m_main_rammask) >> 1) ^ NATIVE_ENDIAN_VALUE_LE_BE(3, 0)] = data;

	*addr += 2;

//  printf("data %04x\n", data);
	return data;
}


inline void epic12_device::epic12_device_gfx_upload_shadow_copy(address_space &space, offs_t *addr)
{
	UINT32 x,y, dimx,dimy;
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);

	dimx = (COPY_NEXT_WORD(space, addr) & 0x1fff) + 1;
	dimy = (COPY_NEXT_WORD(space, addr) & 0x0fff) + 1;

	for (y = 0; y < dimy; y++)
	{
		for (x = 0; x < dimx; x++)
		{
			COPY_NEXT_WORD(space, addr);
		}
	}
}

inline void epic12_device::epic12_device_gfx_upload(offs_t *addr)
{
	UINT32 x,y, dst_p,dst_x_start,dst_y_start, dimx,dimy;
	UINT32 *dst;

	// 0x20000000
	READ_NEXT_WORD(addr);
	READ_NEXT_WORD(addr);

	// 0x99999999
	READ_NEXT_WORD(addr);
	READ_NEXT_WORD(addr);

	dst_x_start = READ_NEXT_WORD(addr);
	dst_y_start = READ_NEXT_WORD(addr);

	dst_p = 0;
	dst_x_start &= 0x1fff;
	dst_y_start &= 0x0fff;

	dimx = (READ_NEXT_WORD(addr) & 0x1fff) + 1;
	dimy = (READ_NEXT_WORD(addr) & 0x0fff) + 1;

	logerror("GFX COPY: DST %02X,%02X,%03X DIM %02X,%03X\n", dst_p,dst_x_start,dst_y_start, dimx,dimy);

	for (y = 0; y < dimy; y++)
	{
		dst = &epic12_device_bitmaps->pix(dst_y_start + y, 0);
		dst += dst_x_start;

		for (x = 0; x < dimx; x++)
		{
			UINT16 pendat = READ_NEXT_WORD(addr);
			// real hw would upload the gfxword directly, but our VRAM is 32-bit, so convert it.
			//dst[dst_x_start + x] = pendat;
			*dst++ = ((pendat&0x8000)<<14) | ((pendat&0x7c00)<<9) | ((pendat&0x03e0)<<6) | ((pendat&0x001f)<<3);  // --t- ---- rrrr r--- gggg g--- bbbb b---  format
			//dst[dst_x_start + x] = ((pendat&0x8000)<<14) | ((pendat&0x7c00)<<6) | ((pendat&0x03e0)<<3) | ((pendat&0x001f)<<0);  // --t- ---- ---r rrrr ---g gggg ---b bbbb  format


		}
	}
}

#define draw_params epic12_device_bitmaps, &epic12_device_clip, &epic12_device_bitmaps->pix(0,0),src_x,src_y, x,y, dimx,dimy, flipy, s_alpha, d_alpha, &tint_clr



epic12_device_blitfunction epic12_device_f0_ti1_tr1_blit_funcs[] =
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

epic12_device_blitfunction epic12_device_f0_ti1_tr0_blit_funcs[] =
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

epic12_device_blitfunction epic12_device_f1_ti1_tr1_blit_funcs[] =
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

epic12_device_blitfunction epic12_device_f1_ti1_tr0_blit_funcs[] =
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



epic12_device_blitfunction epic12_device_f0_ti0_tr1_blit_funcs[] =
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

epic12_device_blitfunction epic12_device_f0_ti0_tr0_blit_funcs[] =
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

epic12_device_blitfunction epic12_device_f1_ti0_tr1_blit_funcs[] =
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

epic12_device_blitfunction epic12_device_f1_ti0_tr0_blit_funcs[] =
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



inline void epic12_device::epic12_device_gfx_draw_shadow_copy(address_space &space, offs_t *addr, int cliptype)
{
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr); // UINT16 dst_x_start  =   COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr); // UINT16 dst_y_start  =   COPY_NEXT_WORD(space, addr);
	UINT16 w        =   COPY_NEXT_WORD(space, addr);
	UINT16 h        =   COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);



	// todo, calcualte clipping.
	epic12_device_blit_delay += w*h;

}



inline void epic12_device::epic12_device_gfx_draw(offs_t *addr)
{
	int x,y, dimx,dimy, flipx,flipy;//, src_p;
	int trans,blend, s_mode, d_mode;
	clr_t tint_clr;
	int tinted = 0;

	UINT16 attr     =   READ_NEXT_WORD(addr);
	UINT16 alpha    =   READ_NEXT_WORD(addr);
	UINT16 src_x    =   READ_NEXT_WORD(addr);
	UINT16 src_y    =   READ_NEXT_WORD(addr);
	UINT16 dst_x_start  =   READ_NEXT_WORD(addr);
	UINT16 dst_y_start  =   READ_NEXT_WORD(addr);
	UINT16 w        =   READ_NEXT_WORD(addr);
	UINT16 h        =   READ_NEXT_WORD(addr);
	UINT16 tint_r   =   READ_NEXT_WORD(addr);
	UINT16 tint_gb  =   READ_NEXT_WORD(addr);

	// 0: +alpha
	// 1: +source
	// 2: +dest
	// 3: *
	// 4: -alpha
	// 5: -source
	// 6: -dest
	// 7: *

	d_mode  =    attr & 0x0007;
	s_mode  =   (attr & 0x0070) >> 4;

	trans   =    attr & 0x0100;
	blend   =      attr & 0x0200;

	flipy   =    attr & 0x0400;
	flipx   =    attr & 0x0800;

	const UINT8 d_alpha =   ((alpha & 0x00ff)       )>>3;
	const UINT8 s_alpha =   ((alpha & 0xff00) >> 8  )>>3;

//  src_p   =   0;
	src_x   =   src_x & 0x1fff;
	src_y   =   src_y & 0x0fff;


	x       =   (dst_x_start & 0x7fff) - (dst_x_start & 0x8000);
	y       =   (dst_y_start & 0x7fff) - (dst_y_start & 0x8000);

	dimx    =   (w & 0x1fff) + 1;
	dimy    =   (h & 0x0fff) + 1;

	// convert parameters to clr


	tint_to_clr(tint_r & 0x00ff, (tint_gb >>  8) & 0xff, tint_gb & 0xff, &tint_clr);

	/* interestingly this gets set to 0x20 for 'normal' not 0x1f */

	if (tint_clr.r!=0x20)
		tinted = 1;

	if (tint_clr.g!=0x20)
		tinted = 1;

	if (tint_clr.b!=0x20)
		tinted = 1;


	// surprisingly frequent, need to verify if it produces a worthwhile speedup tho.
	if ((s_mode==0 && s_alpha==0x1f) && (d_mode==4 && d_alpha==0x1f))
		blend = 0;

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
					epic12_device_f0_ti1_tr1_blit_funcs[s_mode | (d_mode<<3)](draw_params);
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
					epic12_device_f0_ti1_tr0_blit_funcs[s_mode | (d_mode<<3)](draw_params);
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
					epic12_device_f1_ti1_tr1_blit_funcs[s_mode | (d_mode<<3)](draw_params);
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
					epic12_device_f1_ti1_tr0_blit_funcs[s_mode | (d_mode<<3)](draw_params);
				}
			}
		}
	}
	else
	{
		if (blend==0 && tinted==0)
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
					epic12_device_f0_ti0_tr1_blit_funcs[s_mode | (d_mode<<3)](draw_params);
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
					epic12_device_f0_ti0_tr0_blit_funcs[s_mode | (d_mode<<3)](draw_params);
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
					epic12_device_f1_ti0_tr1_blit_funcs[s_mode | (d_mode<<3)](draw_params);
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
					epic12_device_f1_ti0_tr0_blit_funcs[s_mode | (d_mode<<3)](draw_params);
				}
			}
		}
	}



}


void epic12_device::epic12_device_gfx_create_shadow_copy(address_space &space)
{
	offs_t addr = epic12_device_gfx_addr & 0x1fffffff;
	UINT16 cliptype = 0;

	epic12_device_clip.min_x = epic12_device_gfx_scroll_1_x_shadowcopy;
	epic12_device_clip.min_y = epic12_device_gfx_scroll_1_y_shadowcopy;
	epic12_device_clip.max_x = epic12_device_clip.min_x + 320-1;
	epic12_device_clip.max_y = epic12_device_clip.min_y + 240-1;

	while (1)
	{
		UINT16 data = COPY_NEXT_WORD(space, &addr);

		switch( data & 0xf000 )
		{
			case 0x0000:
			case 0xf000:
				return;

			case 0xc000:
				data = COPY_NEXT_WORD(space, &addr);

				cliptype = data ? 1 : 0;

				if (cliptype)
				{
					epic12_device_clip.min_x = epic12_device_gfx_scroll_1_x_shadowcopy;
					epic12_device_clip.min_y = epic12_device_gfx_scroll_1_y_shadowcopy;
					epic12_device_clip.max_x = epic12_device_clip.min_x + 320-1;
					epic12_device_clip.max_y = epic12_device_clip.min_y + 240-1;
				}
				else
				{
					epic12_device_clip.min_x = 0;
					epic12_device_clip.min_y = 0;
					epic12_device_clip.max_x = 0x2000-1;
					epic12_device_clip.max_y = 0x1000-1;
				}

				break;

			case 0x2000:
				addr -= 2;
				epic12_device_gfx_upload_shadow_copy(space, &addr);
				break;

			case 0x1000:
				addr -= 2;
				epic12_device_gfx_draw_shadow_copy(space, &addr, cliptype);
				break;

			default:
				popmessage("GFX op = %04X", data);
				return;
		}
	}
}


void epic12_device::epic12_device_gfx_exec(void)
{
	UINT16 cliptype = 0;

	offs_t addr = epic12_device_gfx_addr_shadowcopy & 0x1fffffff;

//  logerror("GFX EXEC: %08X\n", addr);

	epic12_device_clip.min_x = epic12_device_gfx_scroll_1_x_shadowcopy;
	epic12_device_clip.min_y = epic12_device_gfx_scroll_1_y_shadowcopy;
	epic12_device_clip.max_x = epic12_device_clip.min_x + 320-1;
	epic12_device_clip.max_y = epic12_device_clip.min_y + 240-1;

	while (1)
	{
		UINT16 data = READ_NEXT_WORD(&addr);

		switch( data & 0xf000 )
		{
			case 0x0000:
			case 0xf000:
				return;

			case 0xc000:
				data = READ_NEXT_WORD(&addr);
				cliptype = data ? 1 : 0;

				if (cliptype)
				{
					epic12_device_clip.min_x = epic12_device_gfx_scroll_1_x_shadowcopy;
					epic12_device_clip.min_y = epic12_device_gfx_scroll_1_y_shadowcopy;
					epic12_device_clip.max_x = epic12_device_clip.min_x + 320-1;
					epic12_device_clip.max_y = epic12_device_clip.min_y + 240-1;
				}
				else
				{
					epic12_device_clip.min_x = 0;
					epic12_device_clip.min_y = 0;
					epic12_device_clip.max_x = 0x2000-1;
					epic12_device_clip.max_y = 0x1000-1;
				}
				break;

			case 0x2000:
				addr -= 2;
				epic12_device_gfx_upload(&addr);
				break;

			case 0x1000:
				addr -= 2;
				epic12_device_gfx_draw(&addr);
				break;

			default:
				popmessage("GFX op = %04X", data);
				return;
		}
	}
}


void epic12_device::epic12_device_gfx_exec_unsafe(void)
{
	UINT16 cliptype = 0;

	offs_t addr = epic12_device_gfx_addr & 0x1fffffff;

//  logerror("GFX EXEC: %08X\n", addr);

	epic12_device_clip.min_x = epic12_device_gfx_scroll_1_x;
	epic12_device_clip.min_y = epic12_device_gfx_scroll_1_y;
	epic12_device_clip.max_x = epic12_device_clip.min_x + 320-1;
	epic12_device_clip.max_y = epic12_device_clip.min_y + 240-1;

	while (1)
	{
		UINT16 data = READ_NEXT_WORD(&addr);

		switch( data & 0xf000 )
		{
			case 0x0000:
			case 0xf000:
				return;

			case 0xc000:
				data = READ_NEXT_WORD(&addr);
				cliptype = data ? 1 : 0;

				if (cliptype)
				{
					epic12_device_clip.min_x = epic12_device_gfx_scroll_1_x;
					epic12_device_clip.min_y = epic12_device_gfx_scroll_1_y;
					epic12_device_clip.max_x = epic12_device_clip.min_x  + 320-1;
					epic12_device_clip.max_y = epic12_device_clip.min_y + 240-1;
				}
				else
				{
					epic12_device_clip.min_x = 0;
					epic12_device_clip.min_y = 0;
					epic12_device_clip.max_x = 0x2000-1;
					epic12_device_clip.max_y = 0x1000-1;
				}
				break;

			case 0x2000:
				addr -= 2;
				epic12_device_gfx_upload(&addr);
				break;

			case 0x1000:
				addr -= 2;
				epic12_device_gfx_draw(&addr);
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

	object->epic12_device_gfx_exec();
	return NULL;
}



void *epic12_device::blit_request_callback_unsafe(void *param, int threadid)
{
	epic12_device *object = reinterpret_cast<epic12_device *>(param);

	epic12_device_blit_delay = 0;
	object->epic12_device_gfx_exec_unsafe();
	return NULL;
}


READ32_MEMBER( epic12_device::epic12_device_gfx_ready_r )
{
	return 0x00000010;
}

READ32_MEMBER( epic12_device::epic12_device_gfx_ready_r_unsafe )
{
	if (blitter_busy)
	{
		m_maincpu->spin_until_time(attotime::from_usec(10));
		return 0x00000000;
	}
	else
		return 0x00000010;
}

WRITE32_MEMBER( epic12_device::epic12_device_gfx_exec_w )
{
	if ( ACCESSING_BITS_0_7 )
	{
		if (data & 1)
		{
			//g_profiler.start(PROFILER_USER1);
			// make sure we've not already got a request running
			if (blitter_request)
			{
				int result;
				do
				{
					result = osd_work_item_wait(blitter_request, 1000);
				} while (result==0);
				osd_work_item_release(blitter_request);
			}

			epic12_device_blit_delay = 0;
			epic12_device_gfx_create_shadow_copy(space); // create a copy of the blit list so we can safely thread it.

			if (epic12_device_blit_delay)
			{
				blitter_busy = 1;
				epic12_device_blitter_delay_timer->adjust(attotime::from_nsec(epic12_device_blit_delay*8)); // NOT accurate timing (currently ignored anyway)
			}

			epic12_device_gfx_addr_shadowcopy = epic12_device_gfx_addr;
			epic12_device_gfx_scroll_0_x_shadowcopy =  epic12_device_gfx_scroll_0_x;
			epic12_device_gfx_scroll_0_y_shadowcopy = epic12_device_gfx_scroll_0_y;
			epic12_device_gfx_scroll_1_x_shadowcopy = epic12_device_gfx_scroll_1_x;
			epic12_device_gfx_scroll_1_y_shadowcopy = epic12_device_gfx_scroll_1_y;
			blitter_request = osd_work_item_queue(queue, blit_request_callback, (void*)this, 0);
			//g_profiler.stop();
		}
	}
}


WRITE32_MEMBER( epic12_device::epic12_device_gfx_exec_w_unsafe )
{
	if ( ACCESSING_BITS_0_7 )
	{
		if (data & 1)
		{
			//g_profiler.start(PROFILER_USER1);
			// make sure we've not already got a request running
			if (blitter_request)
			{
				int result;
				do
				{
					result = osd_work_item_wait(blitter_request, 1000);
				} while (result==0);
				osd_work_item_release(blitter_request);
			}

			if (epic12_device_blit_delay)
			{
				blitter_busy = 1;
				int delay = epic12_device_blit_delay*(15 * m_delay_scale / 50);
				//printf("delay %d\n", delay);
				epic12_device_blitter_delay_timer->adjust(attotime::from_nsec(delay));
			}
			else
			{
				blitter_busy = 0;
			}

			blitter_request = osd_work_item_queue(queue, blit_request_callback_unsafe, (void*)this, 0);
			//g_profiler.stop();
		}
	}
}


void epic12_device::draw_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	if (!m_is_unsafe)
	{
		if (blitter_request)
		{
			int result;
			do
			{
				result = osd_work_item_wait(blitter_request, 1000);
			} while (result==0);
			osd_work_item_release(blitter_request);
		}
	}

	int scroll_0_x, scroll_0_y;
//  int scroll_1_x, scroll_1_y;

	bitmap.fill(0, cliprect);

	scroll_0_x = -epic12_device_gfx_scroll_0_x;
	scroll_0_y = -epic12_device_gfx_scroll_0_y;
//  scroll_1_x = -epic12_device_gfx_scroll_1_x;
//  scroll_1_y = -epic12_device_gfx_scroll_1_y;

	//printf("SCREEN UPDATE\n %d %d %d %d\n", scroll_0_x, scroll_0_y, scroll_1_x, scroll_1_y);

	copyscrollbitmap(bitmap, *epic12_device_bitmaps, 1,&scroll_0_x, 1,&scroll_0_y, cliprect);
}






READ32_MEMBER( epic12_device::epic12_device_blitter_r )
{
	switch (offset*4)
	{
		case 0x10:
			return epic12_device::epic12_device_gfx_ready_r(space,offset,mem_mask);

		case 0x24:
			return 0xffffffff;

		case 0x28:
			return 0xffffffff;

		case 0x50:
			return space.machine().root_device().ioport(":DSW")->read();

		default:
			logerror("unknownepic12_device_blitter_r %08x %08x\n", offset*4, mem_mask);
			break;

	}
	return 0;
}

READ32_MEMBER( epic12_device::epic12_device_blitter_r_unsafe )
{
	switch (offset*4)
	{
		case 0x10:
			return epic12_device::epic12_device_gfx_ready_r_unsafe(space,offset,mem_mask);

		case 0x24:
			return 0xffffffff;

		case 0x28:
			return 0xffffffff;

		case 0x50:
			return space.machine().root_device().ioport(":DSW")->read();

		default:
			logerror("unknownepic12_device_blitter_r %08x %08x\n", offset*4, mem_mask);
			break;

	}
	return 0;
}


WRITE32_MEMBER( epic12_device::epic12_device_blitter_w )
{
	switch (offset*4)
	{
		case 0x04:
			epic12_device_gfx_exec_w(space,offset,data,mem_mask);
			break;

		case 0x08:
			COMBINE_DATA(&epic12_device_gfx_addr);
			break;

		case 0x14:
			COMBINE_DATA(&epic12_device_gfx_scroll_0_x);
			break;

		case 0x18:
			COMBINE_DATA(&epic12_device_gfx_scroll_0_y);
			break;

		case 0x40:
			COMBINE_DATA(&epic12_device_gfx_scroll_1_x);
			break;

		case 0x44:
			COMBINE_DATA(&epic12_device_gfx_scroll_1_y);
			break;

	}
}

WRITE32_MEMBER( epic12_device::epic12_device_blitter_w_unsafe )
{
	switch (offset*4)
	{
		case 0x04:
			epic12_device_gfx_exec_w_unsafe(space,offset,data,mem_mask);
			break;

		case 0x08:
			COMBINE_DATA(&epic12_device_gfx_addr);
			break;

		case 0x14:
			COMBINE_DATA(&epic12_device_gfx_scroll_0_x);
			break;

		case 0x18:
			COMBINE_DATA(&epic12_device_gfx_scroll_0_y);
			break;

		case 0x40:
			COMBINE_DATA(&epic12_device_gfx_scroll_1_x);
			break;

		case 0x44:
			COMBINE_DATA(&epic12_device_gfx_scroll_1_y);
			break;

	}
}

void epic12_device::install_handlers(int addr1, int addr2)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	read32_delegate read;
	write32_delegate write;

	if (m_is_unsafe)
	{
		printf("using unsafe blit code!\n");
		read = read32_delegate(FUNC(epic12_device::epic12_device_blitter_r_unsafe), this);
		write = write32_delegate(FUNC(epic12_device::epic12_device_blitter_w_unsafe), this);
	}
	else
	{
		read = read32_delegate(FUNC(epic12_device::epic12_device_blitter_r), this);
		write = write32_delegate(FUNC(epic12_device::epic12_device_blitter_w), this);
	}

	space.install_readwrite_handler(addr1, addr2, read , write, U64(0xffffffffffffffff));



}

READ64_MEMBER( epic12_device::epic12_device_fpga_r )
{
	return 0xff;
}

// todo, store what's written here and checksum it, different microcode probably leads to slightly different blitter timings
WRITE64_MEMBER( epic12_device::epic12_device_fpga_w )
{
	if (ACCESSING_BITS_24_31)
	{
		// data & 0x08 = CE
		// data & 0x10 = CLK
		// data & 0x20 = DATA
	}
}
