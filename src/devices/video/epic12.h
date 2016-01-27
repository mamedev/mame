// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia, MetalliC
/* emulation of Altera Cyclone EPIC12 FPGA programmed as a blitter */

#define MCFG_EPIC12_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, EPIC12, 0)

#define MCFG_EPIC12_SET_MAINRAMSIZE( _rgn ) \
	epic12_device::set_mainramsize(*device, _rgn);


extern UINT8 epic12_device_colrtable[0x20][0x40];
extern UINT8 epic12_device_colrtable_rev[0x20][0x40];
extern UINT8 epic12_device_colrtable_add[0x20][0x20];
extern UINT64 epic12_device_blit_delay;

struct _clr_t
{
	UINT8 b,g,r,t;
};

typedef struct _clr_t clr_t;

union colour_t
{
	clr_t trgb;
	UINT32 u32;
};

typedef void (*epic12_device_blitfunction)(bitmap_rgb32 *,
						const rectangle *,
						UINT32 *, /* gfx */
						int , /* src_x */
						int , /* src_y */
						const int , /* dst_x_start */
						const int , /* dst_y_start */
						int , /* dimx */
						int , /* dimy */
						const int , /* flipy */
						const UINT8 , /* s_alpha */
						const UINT8 , /* d_alpha */
						//int , /* tint */
						const clr_t * );


class epic12_device : public device_t,
	public device_video_interface
{
public:
	epic12_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void set_rambase(UINT16* rambase) { m_ram16 = rambase; }
	void set_delay_scale(int delay_scale) { m_delay_scale = delay_scale; }
	void set_is_unsafe(int is_unsafe) { m_is_unsafe = is_unsafe; }
	void set_cpu_device(cpu_device* maincpu) { m_maincpu = maincpu; }

	inline UINT16 READ_NEXT_WORD(offs_t *addr);

	static void set_mainramsize(device_t &device, int ramsize)
	{
		epic12_device &dev = downcast<epic12_device &>(device);
		dev.m_main_ramsize = ramsize;
		dev.m_main_rammask = ramsize-1;
	}

	static void *blit_request_callback(void *param, int threadid);

	DECLARE_READ64_MEMBER( fpga_r );
	DECLARE_WRITE64_MEMBER( fpga_w );

	void draw_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	UINT16* m_ram16;
	UINT32 m_gfx_addr;
	UINT32 m_gfx_scroll_0_x, m_gfx_scroll_0_y;
	UINT32 m_gfx_scroll_1_x, m_gfx_scroll_1_y;

	int m_gfx_size;
	std::unique_ptr<bitmap_rgb32> m_bitmaps;
	rectangle m_clip;

	UINT16* m_use_ram;
	int m_main_ramsize; // type D has double the main ram
	int m_main_rammask;

	int m_is_unsafe;
	int m_delay_scale;
	cpu_device* m_maincpu;

	void install_handlers(int addr1, int addr2);

	// thread safe mode, with no delays & shadow ram copy
	DECLARE_READ32_MEMBER(blitter_r);
	DECLARE_WRITE32_MEMBER(blitter_w);
	UINT32 m_gfx_addr_shadowcopy;
	UINT32 m_gfx_scroll_0_x_shadowcopy, m_gfx_scroll_0_y_shadowcopy;
	UINT32 m_gfx_scroll_1_x_shadowcopy, m_gfx_scroll_1_y_shadowcopy;
	std::unique_ptr<UINT16[]> m_ram16_copy;
	inline void gfx_upload_shadow_copy(address_space &space, offs_t *addr);
	inline void gfx_create_shadow_copy(address_space &space);
	inline UINT16 COPY_NEXT_WORD(address_space &space, offs_t *addr);
	inline void gfx_draw_shadow_copy(address_space &space, offs_t *addr);
	inline void gfx_upload(offs_t *addr);
	inline void gfx_draw(offs_t *addr);
	void gfx_exec(void);
	DECLARE_READ32_MEMBER( gfx_ready_r );
	DECLARE_WRITE32_MEMBER( gfx_exec_w );

	// for thread unsafe mode with blitter delays, no shadow copy of RAM
	DECLARE_READ32_MEMBER(blitter_r_unsafe);
	DECLARE_WRITE32_MEMBER(blitter_w_unsafe);
	READ32_MEMBER( gfx_ready_r_unsafe );
	WRITE32_MEMBER( gfx_exec_w_unsafe );
	void gfx_exec_unsafe(void);
	static void *blit_request_callback_unsafe(void *param, int threadid);

#define BLIT_FUNCTION static void
#define BLIT_PARAMS bitmap_rgb32 *bitmap, const rectangle *clip, UINT32 *gfx, int src_x, int src_y, const int dst_x_start, const int dst_y_start, int dimx, int dimy, const int flipy, const UINT8 s_alpha, const UINT8 d_alpha, const clr_t *tint_clr

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



	static inline void pen_to_clr(UINT32 pen, clr_t *clr)
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
	static inline void tint_to_clr(UINT8 r, UINT8 g, UINT8 b, clr_t *clr)
	{
		clr->r  =   r>>2;
		clr->g  =   g>>2;
		clr->b  =   b>>2;
	};

	// clr_t to r5g5b5
	static inline UINT32 clr_to_pen(const clr_t *clr)
	{
	// --t- ---- rrrr r--- gggg g--- bbbb b---  format
		return (clr->r << (16+3)) | (clr->g << (8+3)) | (clr->b << 3);

	// --t- ---- ---r rrrr ---g gggg ---b bbbb  format
	//  return (clr->r << (16)) | (clr->g << (8)) | (clr->b);
	};


	static inline void clr_add_with_clr_mul_fixed(clr_t *clr, const clr_t *clr0, const UINT8 mulfixed_val, const clr_t *mulfixed_clr0)
	{
		clr->r = epic12_device_colrtable_add[clr0->r][epic12_device_colrtable[(mulfixed_clr0->r)][mulfixed_val]];
		clr->g = epic12_device_colrtable_add[clr0->g][epic12_device_colrtable[(mulfixed_clr0->g)][mulfixed_val]];
		clr->b = epic12_device_colrtable_add[clr0->b][epic12_device_colrtable[(mulfixed_clr0->b)][mulfixed_val]];
	}

	static inline  void clr_add_with_clr_mul_3param(clr_t *clr, const clr_t *clr0, const clr_t *clr1, const clr_t *clr2)
	{
		clr->r = epic12_device_colrtable_add[clr0->r][epic12_device_colrtable[(clr2->r)][(clr1->r)]];
		clr->g = epic12_device_colrtable_add[clr0->g][epic12_device_colrtable[(clr2->g)][(clr1->g)]];
		clr->b = epic12_device_colrtable_add[clr0->b][epic12_device_colrtable[(clr2->b)][(clr1->b)]];
	}

	static inline  void clr_add_with_clr_square(clr_t *clr, const clr_t *clr0, const clr_t *clr1)
	{
		clr->r = epic12_device_colrtable_add[clr0->r][epic12_device_colrtable[(clr1->r)][(clr1->r)]];
		clr->g = epic12_device_colrtable_add[clr0->r][epic12_device_colrtable[(clr1->g)][(clr1->g)]];
		clr->b = epic12_device_colrtable_add[clr0->r][epic12_device_colrtable[(clr1->b)][(clr1->b)]];
	}

	static inline  void clr_add_with_clr_mul_fixed_rev(clr_t *clr, const clr_t *clr0, const UINT8 val, const clr_t *clr1)
	{
		clr->r =  epic12_device_colrtable_add[clr0->r][epic12_device_colrtable_rev[val][(clr1->r)]];
		clr->g =  epic12_device_colrtable_add[clr0->g][epic12_device_colrtable_rev[val][(clr1->g)]];
		clr->b =  epic12_device_colrtable_add[clr0->b][epic12_device_colrtable_rev[val][(clr1->b)]];
	}

	static inline  void clr_add_with_clr_mul_rev_3param(clr_t *clr, const clr_t *clr0, const clr_t *clr1, const clr_t *clr2)
	{
		clr->r =  epic12_device_colrtable_add[clr0->r][epic12_device_colrtable_rev[(clr2->r)][(clr1->r)]];
		clr->g =  epic12_device_colrtable_add[clr0->g][epic12_device_colrtable_rev[(clr2->g)][(clr1->g)]];
		clr->b =  epic12_device_colrtable_add[clr0->b][epic12_device_colrtable_rev[(clr2->b)][(clr1->b)]];
	}

	static inline  void clr_add_with_clr_mul_rev_square(clr_t *clr, const clr_t *clr0, const clr_t *clr1)
	{
		clr->r =  epic12_device_colrtable_add[clr0->r][epic12_device_colrtable_rev[(clr1->r)][(clr1->r)]];
		clr->g =  epic12_device_colrtable_add[clr0->g][epic12_device_colrtable_rev[(clr1->g)][(clr1->g)]];
		clr->b =  epic12_device_colrtable_add[clr0->b][epic12_device_colrtable_rev[(clr1->b)][(clr1->b)]];
	}


	static inline  void clr_add(clr_t *clr, const clr_t *clr0, const clr_t *clr1)
	{
	/*
	    clr->r = clr0->r + clr1->r;
	    clr->g = clr0->g + clr1->g;
	    clr->b = clr0->b + clr1->b;
	*/
		// use pre-clamped lookup table
		clr->r =  epic12_device_colrtable_add[clr0->r][clr1->r];
		clr->g =  epic12_device_colrtable_add[clr0->g][clr1->g];
		clr->b =  epic12_device_colrtable_add[clr0->b][clr1->b];

	}


	static inline void clr_mul(clr_t *clr0, const clr_t *clr1)
	{
		clr0->r = epic12_device_colrtable[(clr0->r)][(clr1->r)];
		clr0->g = epic12_device_colrtable[(clr0->g)][(clr1->g)];
		clr0->b = epic12_device_colrtable[(clr0->b)][(clr1->b)];
	}

	static inline void clr_square(clr_t *clr0, const clr_t *clr1)
	{
		clr0->r = epic12_device_colrtable[(clr1->r)][(clr1->r)];
		clr0->g = epic12_device_colrtable[(clr1->g)][(clr1->g)];
		clr0->b = epic12_device_colrtable[(clr1->b)][(clr1->b)];
	}

	static inline void clr_mul_3param(clr_t *clr0, const clr_t *clr1, const clr_t *clr2)
	{
		clr0->r = epic12_device_colrtable[(clr2->r)][(clr1->r)];
		clr0->g = epic12_device_colrtable[(clr2->g)][(clr1->g)];
		clr0->b = epic12_device_colrtable[(clr2->b)][(clr1->b)];
	}

	static inline void clr_mul_rev(clr_t *clr0, const clr_t *clr1)
	{
		clr0->r = epic12_device_colrtable_rev[(clr0->r)][(clr1->r)];
		clr0->g = epic12_device_colrtable_rev[(clr0->g)][(clr1->g)];
		clr0->b = epic12_device_colrtable_rev[(clr0->b)][(clr1->b)];
	}

	static inline void clr_mul_rev_square(clr_t *clr0, const clr_t *clr1)
	{
		clr0->r = epic12_device_colrtable_rev[(clr1->r)][(clr1->r)];
		clr0->g = epic12_device_colrtable_rev[(clr1->g)][(clr1->g)];
		clr0->b = epic12_device_colrtable_rev[(clr1->b)][(clr1->b)];
	}


	static inline void clr_mul_rev_3param(clr_t *clr0, const clr_t *clr1, const clr_t *clr2)
	{
		clr0->r = epic12_device_colrtable_rev[(clr2->r)][(clr1->r)];
		clr0->g = epic12_device_colrtable_rev[(clr2->g)][(clr1->g)];
		clr0->b = epic12_device_colrtable_rev[(clr2->b)][(clr1->b)];
	}

	static inline void clr_mul_fixed(clr_t *clr, const UINT8 val, const clr_t *clr0)
	{
		clr->r = epic12_device_colrtable[val][(clr0->r)];
		clr->g = epic12_device_colrtable[val][(clr0->g)];
		clr->b = epic12_device_colrtable[val][(clr0->b)];
	}

	static inline void clr_mul_fixed_rev(clr_t *clr, const UINT8 val, const clr_t *clr0)
	{
		clr->r = epic12_device_colrtable_rev[val][(clr0->r)];
		clr->g = epic12_device_colrtable_rev[val][(clr0->g)];
		clr->b = epic12_device_colrtable_rev[val][(clr0->b)];
	}

	static inline void clr_copy(clr_t *clr, const clr_t *clr0)
	{
		clr->r = clr0->r;
		clr->g = clr0->g;
		clr->b = clr0->b;
	}



	// (1|s|d) * s_factor * s + (1|s|d) * d_factor * d
	// 0: +alpha
	// 1: +source
	// 2: +dest
	// 3: *
	// 4: -alpha
	// 5: -source
	// 6: -dest
	// 7: *


protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	osd_work_queue *m_work_queue;
	osd_work_item *m_blitter_request;

	// blit timing
	emu_timer *m_blitter_delay_timer;
	int m_blitter_busy;

	TIMER_CALLBACK_MEMBER( blitter_delay_callback );
};



extern const device_type EPIC12;
