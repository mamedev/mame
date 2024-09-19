// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia, MetalliC
/* emulation of Altera Cyclone EP1C12 FPGA programmed as a blitter */
#ifndef MAME_CAVE_EP1C12_H
#define MAME_CAVE_EP1C12_H

#pragma once

#define DEBUG_VRAM_VIEWER 0 // VRAM viewer for debug

class ep1c12_device : public device_t, public device_video_interface
{
public:
	ep1c12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_cpu(T &&maintag) { m_maincpu.set_tag(std::forward<T>(maintag)); }
	auto port_r_callback() { return m_port_r_cb.bind(); }
	void set_rambase(u16* rambase) { m_ram16 = rambase; }

	inline u16 READ_NEXT_WORD(offs_t *addr);

	void set_mainramsize(size_t ramsize)
	{
		m_main_ramsize = ramsize;
		m_main_rammask = ramsize - 1;
	}

	static void *blit_request_callback(void *param, int threadid);

	u64 fpga_r();
	void fpga_w(offs_t offset, u64 data, u64 mem_mask = ~0);

	void draw_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u16* m_ram16;
	u32 m_gfx_addr;
	u32 m_gfx_scroll_x, m_gfx_scroll_y;
	u32 m_gfx_clip_x, m_gfx_clip_y;

	int m_gfx_size;
	std::unique_ptr<bitmap_rgb32> m_bitmaps;
	rectangle m_clip;

	u16* m_use_ram;
	size_t m_main_ramsize; // type D has double the main ram
	size_t m_main_rammask;

	void install_handlers(int addr1, int addr2);

	u32 blitter_r(offs_t offset, u32 mem_mask = ~0);
	void blitter_w(address_space &space, offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 m_gfx_addr_shadowcopy;
	u32 m_gfx_clip_x_shadowcopy, m_gfx_clip_y_shadowcopy;
	std::unique_ptr<u16[]> m_ram16_copy;
	inline void gfx_upload_shadow_copy(address_space &space, offs_t *addr);
	inline void gfx_create_shadow_copy(address_space &space);
	inline u16 COPY_NEXT_WORD(address_space &space, offs_t *addr);
	inline void gfx_draw_shadow_copy(address_space &space, offs_t *addr);
	inline void gfx_upload(offs_t *addr);
	inline void gfx_draw(offs_t *addr);
	void gfx_exec();
	u32 gfx_ready_r();
	void gfx_exec_w(address_space &space, offs_t offset, u32 data, u32 mem_mask = ~0);

protected:
	// Number of bytes that are read each time Blitter fetches operations from SRAM.
	static inline constexpr int OPERATION_CHUNK_SIZE_BYTES = 64;

	// Approximate time it takes to fetch a chunk of operations.
	// This is composed of the time that the Blitter holds the Bus Request (BREQ) signal
	// of the SH-3, as well as the overhead between requests.
	static inline constexpr int OPERATION_READ_CHUNK_INTERVAL_NS = 700;

	// The firmware versions
	enum {
		// Used by ibara & mushisama
		FW_A, // Byte checksum 03

		// Used by espgal2
		FW_B, // Byte checksum 3e

		// Used by espgal2a, mushitama and mushisamb
		FW_C, // Byte checksum f9

		// Used by everything else
		FW_D, // Byte checksum e1
	};

	struct clr_t
	{
		// clr_t to r5g5b5
		u32 to_pen() const
		{
			// --t- ---- rrrr r--- gggg g--- bbbb b---  format
			return (r << (16 + 3)) | (g << (8 + 3)) | (b << 3);

			// --t- ---- ---r rrrr ---g gggg ---b bbbb  format
			//return (r << 16) | (g << 8) | b;
		}


		void add_with_clr_mul_fixed(const clr_t &clr0, u8 mulfixed_val, const clr_t &mulfixed_clr0)
		{
			r = colrtable_add[clr0.r][colrtable[mulfixed_clr0.r][mulfixed_val]];
			g = colrtable_add[clr0.g][colrtable[mulfixed_clr0.g][mulfixed_val]];
			b = colrtable_add[clr0.b][colrtable[mulfixed_clr0.b][mulfixed_val]];
		}

		void add_with_clr_mul_3param(const clr_t &clr0, const clr_t &clr1, const clr_t &clr2)
		{
			r = colrtable_add[clr0.r][colrtable[clr2.r][clr1.r]];
			g = colrtable_add[clr0.g][colrtable[clr2.g][clr1.g]];
			b = colrtable_add[clr0.b][colrtable[clr2.b][clr1.b]];
		}

		void add_with_clr_square(const clr_t &clr0, const clr_t &clr1)
		{
			r = colrtable_add[clr0.r][colrtable[clr1.r][clr1.r]];
			g = colrtable_add[clr0.r][colrtable[clr1.g][clr1.g]];
			b = colrtable_add[clr0.r][colrtable[clr1.b][clr1.b]];
		}

		void add_with_clr_mul_fixed_rev(const clr_t &clr0, u8 val, const clr_t &clr1)
		{
			r = colrtable_add[clr0.r][colrtable_rev[val][clr1.r]];
			g = colrtable_add[clr0.g][colrtable_rev[val][clr1.g]];
			b = colrtable_add[clr0.b][colrtable_rev[val][clr1.b]];
		}

		void add_with_clr_mul_rev_3param(const clr_t &clr0, const clr_t &clr1, const clr_t &clr2)
		{
			r = colrtable_add[clr0.r][colrtable_rev[clr2.r][clr1.r]];
			g = colrtable_add[clr0.g][colrtable_rev[clr2.g][clr1.g]];
			b = colrtable_add[clr0.b][colrtable_rev[clr2.b][clr1.b]];
		}

		void add_with_clr_mul_rev_square(const clr_t &clr0, const clr_t &clr1)
		{
			r = colrtable_add[clr0.r][colrtable_rev[(clr1.r)][(clr1.r)]];
			g = colrtable_add[clr0.g][colrtable_rev[(clr1.g)][(clr1.g)]];
			b = colrtable_add[clr0.b][colrtable_rev[(clr1.b)][(clr1.b)]];
		}


		void add(const clr_t &clr0, const clr_t &clr1)
		{
			//r = clr0.r + clr1.r;
			//g = clr0.g + clr1.g;
			//b = clr0.b + clr1.b;

			// use pre-clamped lookup table
			r =  colrtable_add[clr0.r][clr1.r];
			g =  colrtable_add[clr0.g][clr1.g];
			b =  colrtable_add[clr0.b][clr1.b];
		}


		void mul(const clr_t &clr1)
		{
			r = colrtable[r][clr1.r];
			g = colrtable[g][clr1.g];
			b = colrtable[b][clr1.b];
		}

		void square(const clr_t &clr1)
		{
			r = colrtable[clr1.r][clr1.r];
			g = colrtable[clr1.g][clr1.g];
			b = colrtable[clr1.b][clr1.b];
		}

		void mul_3param(const clr_t &clr1, const clr_t &clr2)
		{
			r = colrtable[clr2.r][clr1.r];
			g = colrtable[clr2.g][clr1.g];
			b = colrtable[clr2.b][clr1.b];
		}

		void mul_rev(const clr_t &clr1)
		{
			r = colrtable_rev[r][clr1.r];
			g = colrtable_rev[g][clr1.g];
			b = colrtable_rev[b][clr1.b];
		}

		void mul_rev_square(const clr_t &clr1)
		{
			r = colrtable_rev[clr1.r][clr1.r];
			g = colrtable_rev[clr1.g][clr1.g];
			b = colrtable_rev[clr1.b][clr1.b];
		}


		void mul_rev_3param(const clr_t &clr1, const clr_t &clr2)
		{
			r = colrtable_rev[clr2.r][clr1.r];
			g = colrtable_rev[clr2.g][clr1.g];
			b = colrtable_rev[clr2.b][clr1.b];
		}

		void mul_fixed(u8 val, const clr_t &clr0)
		{
			r = colrtable[val][clr0.r];
			g = colrtable[val][clr0.g];
			b = colrtable[val][clr0.b];
		}

		void mul_fixed_rev(u8 val, const clr_t &clr0)
		{
			r = colrtable_rev[val][clr0.r];
			g = colrtable_rev[val][clr0.g];
			b = colrtable_rev[val][clr0.b];
		}

		void copy(const clr_t &clr0)
		{
			r = clr0.r;
			g = clr0.g;
			b = clr0.b;
		}

#ifdef LSB_FIRST
		u8 b, g, r, t;
#else
		u8 t, r, g, b;
#endif
	};

	union colour_t
	{
		clr_t trgb;
		u32 data;
	};

	typedef void (*blitfunction)(
			bitmap_rgb32 *,
			const rectangle *,
			u32 *gfx,
			int src_x,
			int src_y,
			const int dst_x_start,
			const int dst_y_start,
			int dimx,
			int dimy,
			const bool flipy,
			const u8 s_alpha,
			const u8 d_alpha,
			//int tint,
			const clr_t *);


#define BLIT_FUNCTION static void
#define BLIT_PARAMS bitmap_rgb32 *bitmap, const rectangle *clip, u32 *gfx, int src_x, int src_y, const int dst_x_start, const int dst_y_start, int dimx, int dimy, const bool flipy, const u8 s_alpha, const u8 d_alpha, const clr_t *tint_clr

	BLIT_FUNCTION draw_sprite_f0_ti0_plain(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s0_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s1_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s2_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s3_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s4_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s5_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s6_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s7_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s0_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s1_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s2_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s3_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s4_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s5_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s6_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s7_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s0_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s1_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s2_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s3_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s4_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s5_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s6_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s7_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s0_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s1_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s2_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s3_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s4_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s5_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s6_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s7_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s0_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s1_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s2_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s3_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s4_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s5_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s6_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s7_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s0_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s1_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s2_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s3_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s4_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s5_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s6_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s7_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s0_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s1_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s2_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s3_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s4_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s5_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s6_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s7_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s0_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s1_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s2_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s3_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s4_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s5_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s6_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_s7_d7(BLIT_PARAMS);

	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_plain(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s0_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s1_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s2_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s3_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s4_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s5_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s6_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s7_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s0_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s1_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s2_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s3_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s4_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s5_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s6_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s7_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s0_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s1_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s2_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s3_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s4_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s5_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s6_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s7_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s0_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s1_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s2_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s3_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s4_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s5_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s6_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s7_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s0_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s1_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s2_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s3_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s4_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s5_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s6_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s7_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s0_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s1_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s2_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s3_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s4_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s5_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s6_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s7_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s0_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s1_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s2_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s3_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s4_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s5_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s6_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s7_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s0_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s1_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s2_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s3_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s4_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s5_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s6_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_s7_d7(BLIT_PARAMS);

	BLIT_FUNCTION draw_sprite_f1_ti0_plain(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s0_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s1_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s2_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s3_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s4_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s5_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s6_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s7_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s0_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s1_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s2_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s3_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s4_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s5_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s6_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s7_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s0_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s1_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s2_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s3_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s4_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s5_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s6_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s7_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s0_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s1_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s2_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s3_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s4_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s5_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s6_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s7_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s0_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s1_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s2_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s3_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s4_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s5_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s6_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s7_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s0_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s1_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s2_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s3_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s4_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s5_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s6_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s7_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s0_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s1_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s2_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s3_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s4_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s5_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s6_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s7_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s0_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s1_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s2_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s3_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s4_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s5_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s6_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_s7_d7(BLIT_PARAMS);

	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_plain(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s0_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s1_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s2_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s3_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s4_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s5_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s6_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s7_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s0_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s1_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s2_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s3_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s4_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s5_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s6_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s7_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s0_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s1_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s2_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s3_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s4_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s5_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s6_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s7_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s0_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s1_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s2_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s3_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s4_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s5_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s6_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s7_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s0_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s1_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s2_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s3_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s4_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s5_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s6_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s7_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s0_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s1_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s2_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s3_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s4_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s5_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s6_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s7_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s0_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s1_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s2_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s3_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s4_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s5_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s6_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s7_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s0_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s1_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s2_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s3_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s4_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s5_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s6_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_s7_d7(BLIT_PARAMS);

	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_plain(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s0_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s1_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s2_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s3_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s4_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s5_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s6_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s7_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s0_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s1_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s2_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s3_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s4_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s5_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s6_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s7_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s0_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s1_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s2_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s3_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s4_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s5_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s6_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s7_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s0_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s1_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s2_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s3_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s4_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s5_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s6_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s7_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s0_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s1_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s2_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s3_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s4_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s5_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s6_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s7_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s0_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s1_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s2_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s3_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s4_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s5_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s6_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s7_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s0_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s1_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s2_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s3_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s4_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s5_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s6_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s7_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s0_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s1_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s2_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s3_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s4_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s5_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s6_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr1_s7_d7(BLIT_PARAMS);

	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_plain(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s0_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s1_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s2_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s3_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s4_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s5_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s6_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s7_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s0_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s1_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s2_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s3_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s4_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s5_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s6_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s7_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s0_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s1_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s2_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s3_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s4_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s5_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s6_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s7_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s0_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s1_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s2_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s3_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s4_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s5_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s6_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s7_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s0_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s1_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s2_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s3_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s4_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s5_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s6_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s7_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s0_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s1_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s2_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s3_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s4_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s5_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s6_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s7_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s0_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s1_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s2_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s3_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s4_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s5_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s6_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s7_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s0_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s1_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s2_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s3_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s4_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s5_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s6_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti1_tr0_s7_d7(BLIT_PARAMS);

	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_plain(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s0_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s1_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s2_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s3_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s4_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s5_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s6_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s7_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s0_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s1_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s2_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s3_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s4_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s5_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s6_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s7_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s0_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s1_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s2_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s3_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s4_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s5_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s6_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s7_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s0_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s1_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s2_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s3_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s4_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s5_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s6_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s7_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s0_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s1_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s2_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s3_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s4_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s5_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s6_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s7_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s0_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s1_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s2_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s3_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s4_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s5_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s6_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s7_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s0_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s1_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s2_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s3_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s4_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s5_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s6_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s7_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s0_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s1_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s2_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s3_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s4_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s5_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s6_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr1_s7_d7(BLIT_PARAMS);

	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_plain(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s0_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s1_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s2_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s3_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s4_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s5_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s6_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s7_d0(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s0_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s1_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s2_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s3_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s4_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s5_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s6_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s7_d1(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s0_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s1_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s2_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s3_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s4_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s5_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s6_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s7_d2(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s0_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s1_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s2_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s3_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s4_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s5_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s6_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s7_d3(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s0_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s1_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s2_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s3_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s4_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s5_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s6_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s7_d4(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s0_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s1_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s2_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s3_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s4_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s5_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s6_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s7_d5(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s0_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s1_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s2_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s3_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s4_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s5_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s6_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s7_d6(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s0_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s1_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s2_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s3_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s4_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s5_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s6_d7(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti1_tr0_s7_d7(BLIT_PARAMS);

	BLIT_FUNCTION draw_sprite_f0_ti0_tr1_simple(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f0_ti0_tr0_simple(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr1_simple(BLIT_PARAMS);
	BLIT_FUNCTION draw_sprite_f1_ti0_tr0_simple(BLIT_PARAMS);

	static inline void pen_to_clr(u32 pen, clr_t *clr)
	{
	// --t- ---- rrrr r--- gggg g--- bbbb b---  format
		clr->r = (pen >> (16+3));// & 0x1f;
		clr->g = (pen >>  (8+3));// & 0x1f;
		clr->b = (pen >>   3);// & 0x1f;

	// --t- ---- ---r rrrr ---g gggg ---b bbbb  format
	//  clr->r = (pen >> 16) & 0x1f;
	//  clr->g = (pen >> 8) & 0x1f;
	//  clr->b = (pen >> 0) & 0x1f;

	};

	// convert separate r,g,b biases (0..80..ff) to clr_t (-1f..0..1f)
	void tint_to_clr(u8 r, u8 g, u8 b, clr_t *clr)
	{
		clr->r  =   r>>2;
		clr->g  =   g>>2;
		clr->b  =   b>>2;
	};

	// (1|s|d) * s_factor * s + (1|s|d) * d_factor * d
	// 0: +alpha
	// 1: +source
	// 2: +dest
	// 3: *
	// 4: -alpha
	// 5: -source
	// 6: -dest
	// 7: *

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// Called when a Blitter operation does not cause any draws/uploads to be performed.
	// If multiple draws in a row are performed outside of an active clipping area,
	// the Blitter will be reading operations from SRAM in 64 byte chunks, but not
	// actually performing any work.
	// This will still be visible from the CPU as Blitter being busy, until the
	// operation list has exited.
	//
	// TODO: Having 64 bytes of non-drawing operations in a row will only cause the Blitter
	//   to idle if the operations are read from the same 64 byte chunk (and not split between two).
	//   More proper handling of this would be to change the reads from SRAM to be done 64 bytes at the time
	//   into a FIFO, but that's a fair amount of work.
	void idle_blitter(u8 operation_size_bytes)
	{
		m_blit_idle_op_bytes += operation_size_bytes;
		if (m_blit_idle_op_bytes >= OPERATION_CHUNK_SIZE_BYTES)
		{
			m_blit_idle_op_bytes -= OPERATION_CHUNK_SIZE_BYTES;
			m_blit_delay_ns += OPERATION_READ_CHUNK_INTERVAL_NS;
		}
	}

	TIMER_CALLBACK_MEMBER(blitter_delay_callback);

	osd_work_queue *m_work_queue;
	osd_work_item *m_blitter_request;

	// blit timing
	emu_timer *m_blitter_delay_timer;
	int m_blitter_busy;
	u64 m_blit_delay_ns;
	u16 m_blit_idle_op_bytes;

	// fpga firmware
	std::vector<u8> m_firmware;
	int m_firmware_pos;
	u8 m_firmware_port;
	int m_firmware_version;

	// debug vram viewer
#ifdef DEBUG_VRAM_VIEWER
	bool m_debug_vram_view_en;
	int m_prev_screen_width;
	int m_prev_screen_height;
	rectangle m_prev_screen_visarea;
	int m_curr_screen_width;
	int m_curr_screen_height;
	rectangle m_curr_screen_visarea;
#endif

	static u8 colrtable[0x20][0x40];
	static u8 colrtable_rev[0x20][0x40];
	static u8 colrtable_add[0x20][0x20];

	static const blitfunction f0_ti1_tr1_blit_funcs[64];
	static const blitfunction f0_ti1_tr0_blit_funcs[64];
	static const blitfunction f1_ti1_tr1_blit_funcs[64];
	static const blitfunction f1_ti1_tr0_blit_funcs[64];
	static const blitfunction f0_ti0_tr1_blit_funcs[64];
	static const blitfunction f0_ti0_tr0_blit_funcs[64];
	static const blitfunction f1_ti0_tr1_blit_funcs[64];
	static const blitfunction f1_ti0_tr0_blit_funcs[64];

	// internal states
	required_device<cpu_device> m_maincpu;
	devcb_read32 m_port_r_cb;
};


DECLARE_DEVICE_TYPE(EP1C12, ep1c12_device)

#endif // MAME_CAVE_EP1C12_H
