// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    3DO M2 BDA Triangle Engine

***************************************************************************/
#ifndef MAME_KONAMI_3DOM2_TE_H
#define MAME_KONAMI_3DOM2_TE_H

#pragma once

/***************************************************************************
    FORWARD DECLARATIONS
***************************************************************************/

class m2_bda_device;

/***************************************************************************
    TRIANGLE ENGINE DEVICE
***************************************************************************/

class m2_te_device : public device_t
{
public:
	m2_te_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Static configuration helpers
	auto general_int_handler() { return m_general_int_handler.bind(); }
	auto dfinstr_int_handler() { return m_dfinstr_int_handler.bind(); }
	auto iminstr_int_handler() { return m_iminstr_int_handler.bind(); }
	auto listend_int_handler() { return m_listend_int_handler.bind(); }
	auto winclip_int_handler() { return m_winclip_int_handler.bind(); }

	uint32_t read(offs_t offset);
	void write(offs_t offset, uint32_t data);

	uint32_t *tram_ptr() const { return &m_tram[0]; }

	enum te_reg_wmode
	{
		REG_WRITE,
		REG_INVALID,
		REG_SET,
		REG_CLEAR,
	};

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(command_done);

private:

	enum te_state
	{
		TE_STOPPED,
		TE_PAUSED,
		TE_RUNNING,
	};

	enum misc
	{
		PIP_RAM_WORDS           = 256,
		TEXTURE_RAM_WORDS       = 4096,
		PIP_RAM_BYTEMASK        = PIP_RAM_WORDS * 4 - 1,
		TEXTURE_RAM_BYTEMASK    = TEXTURE_RAM_WORDS * 4 - 1,
	};

	enum inst_type
	{
		INST_WRITE_REG  = 0x10000000,
		INST_VTX_SHORT  = 0x20000000,
		INST_VTX_LONG   = 0x30000000,
		INST_VTX_POINT  = 0x40000000,
		INST_MASK       = 0xf0000000,
	};

	enum vtx_flag
	{
		VTX_FLAG_SHAD   = 0x00010000,
		VTX_FLAG_TEXT   = 0x00020000,
		VTX_FLAG_PRSP   = 0x00040000,
		VTX_FLAG_NEW    = 0x00080000,
		VTX_FLAG_RM     = 0x00100000,
	};

	struct se_vtx
	{
		float x, y;
		float r, g, b, a;
		float w;
		float uw, vw;
		float iv[8];
	};

	struct slope_params
	{
		float y23;
		float y31;
		float y12;
		float x23;
		float x31;
		float x12;
		float xstep_long;
		float iAria;
	};

	struct rgba { uint8_t r, g, b, a; };

	void set_interrupt(uint32_t mask);
	void update_interrupts();

	void teicntl_w(uint32_t data, te_reg_wmode wmode);
	void tedcntl_w(uint32_t data, te_reg_wmode wmode);
	void master_mode_w(uint32_t data, te_reg_wmode wmode);

	void execute();
	void illegal_inst();
	uint32_t irp_fetch();
	float irp_fetch_float();

	void add_vertex(const se_vtx &vtx, uint32_t flags);
	void log_triangle(uint32_t flags);
	void setup_triangle(uint32_t flags);
	void calculate_slope(const slope_params &sp, float q1, float q2, float q3, float &slope_out, float &ddx_out);
	void walk_edges(uint32_t wrange);
	void walk_span(uint32_t wrange, bool omit_right, uint32_t y, uint32_t xs, uint32_t xe, int32_t r, int32_t g, int32_t b, int32_t a, uint32_t uw, uint32_t vw, uint32_t w);

	void texcoord_gen(uint32_t wrange, uint32_t uw, uint32_t vw, uint32_t w,
						uint32_t & uo, uint32_t & vo, uint32_t & wo);

	uint32_t lod_calc(uint32_t u0, uint32_t v0, uint32_t u1, uint32_t v1);

	uint32_t get_tram_bitdepth();

	void addr_calc(uint32_t u, uint32_t v, uint32_t lod,
					uint32_t & texaddr, uint32_t & texbit, uint32_t & tdepth);

	void get_texture_color(uint32_t u, uint32_t v, uint32_t lod,
							uint32_t & r, uint32_t & g, uint32_t & b, uint32_t & a, uint32_t & s);

	void get_texel(uint32_t u, uint32_t v, uint32_t lod,
		uint32_t &r_ti, uint32_t &g_ti, uint32_t &b_ti, uint32_t &a_ti, uint32_t &ssb_ti);

	void texture_fetch(uint32_t texaddr, uint32_t texbit, uint32_t tdepth,
		uint32_t &r_ti, uint32_t &g_ti, uint32_t &b_ti, uint32_t &a_ti, uint32_t &ssb_ti);

	void select_lerp(uint32_t sel,
					uint32_t ri, uint32_t gi, uint32_t bi, uint32_t ai,
					uint32_t rt, uint32_t gt, uint32_t bt, uint32_t at, uint32_t ssbt,
					uint32_t & ar, uint32_t & ag, uint32_t & ab );

	void select_mul(uint32_t sel, uint32_t ai, uint32_t at, uint32_t ssbt,
					uint32_t & a );

	void texture_blend(uint32_t ri, uint32_t gi, uint32_t bi, uint32_t ai,
						uint32_t rt, uint32_t gt, uint32_t bt, uint32_t at, uint32_t ssbt,
						uint32_t &ro, uint32_t &go, uint32_t &bo, uint32_t &ao, uint32_t &ssbo);

	void destination_blend(uint32_t x, uint32_t y, uint32_t w, const rgba & ti_color, uint8_t ssb);

	uint8_t color_blend(uint8_t ct, uint8_t cti, uint8_t cs, uint8_t csrc,
						uint8_t dm10, uint8_t dm11,
						uint8_t dm20, uint8_t dm21);

	void select_alpha_dsb();
	uint8_t get_tex_coef(uint8_t cs, uint8_t dm1const0, uint8_t dm1const1);
	uint8_t get_src_coef(uint8_t cti, uint8_t dm2const0, uint8_t dm2const1);
	uint8_t dither(uint8_t in, uint8_t dithval);
	uint8_t alu_calc(uint16_t a, uint16_t b);
	void select_src_pixel();
	void select_tex_pixel();
	void write_dst_pixel();

	uint8_t read_tram8(offs_t address) const;
	uint16_t read_tram16(offs_t address) const;
	uint32_t read_tram32(offs_t address) const;
	void write_tram8(offs_t address, uint8_t data);
	void write_tram16(offs_t address, uint16_t data);
	void write_tram32(offs_t address, uint32_t data);

	uint8_t read_pipram8(offs_t address) const;
	uint16_t read_pipram16(offs_t address) const;
	uint32_t read_pipram32(offs_t address) const;
	void write_pipram32(offs_t address, uint32_t data);

	uint32_t readbits_from_ram(uint32_t & src_addr, uint32_t & bit_offs, uint32_t bits);
	void load_texture();

	emu_timer           *m_done_timer;
	m2_bda_device       *m_bda;

	devcb_write_line    m_general_int_handler;
	devcb_write_line    m_dfinstr_int_handler;
	devcb_write_line    m_iminstr_int_handler;
	devcb_write_line    m_listend_int_handler;
	devcb_write_line    m_winclip_int_handler;

	// Registers
	union
	{
		struct
		{
			uint32_t te_master_mode;
			uint32_t reserved;
			uint32_t teicntl_data;
			uint32_t teicntl;
			uint32_t tedcntl_data;
			uint32_t tedcntl;
			uint32_t iwp;
			uint32_t irp;
			uint32_t int_enable;
			uint32_t int_status;
			uint32_t vertex_ctrl;
		};
		uint32_t m_regs[11];
	} m_gc;

	union
	{
		struct
		{
			se_vtx  vertices[3];
			uint32_t    reserved[16];
			uint32_t    vertex_state;
		};
		uint32_t m_regs[65];
	} m_se;

	struct
	{
		union
		{
			struct
			{
				uint32_t es_cntl;
				uint32_t es_capaddr;
				uint32_t es_capdata;
			};
			uint32_t m_regs[3];
		};
		union
		{
			struct
			{
				uint32_t x1;
				uint32_t y1;
				uint32_t r1;
				uint32_t g1;
				uint32_t b1;
				uint32_t a1;
				uint32_t w1;
				uint32_t uw1;
				uint32_t vw1;
				uint32_t x2;
				uint32_t y2;
				uint32_t y3;
				uint32_t xstep_0;
				uint32_t xstep_1;
				uint32_t xstep_long;
				uint32_t xystep_0;
				uint32_t xystep_1;
				uint32_t xystep_long;
				uint32_t dy_0;
				uint32_t dy_1;
				uint32_t dy_long;
				uint32_t ddx_r;
				uint32_t ddx_g;
				uint32_t ddx_b;
				uint32_t ddx_a;
				uint32_t ddx_w;
				uint32_t ddx_uw;
				uint32_t ddx_vw;
				uint32_t slope_r;
				uint32_t slope_g;
				uint32_t slope_b;
				uint32_t slope_a;
				uint32_t slope_uw;
				uint32_t slope_vw;
				uint32_t slope_w;
				uint32_t r2l;
			};
			uint32_t m_buffer_regs[36];
		};
	} m_es;

	union
	{
		struct
		{
			uint32_t tex_cntl;
			uint32_t texld_cntl;
			uint32_t tex_addr_cntl;
			uint32_t tex_pip_cntl;
			uint32_t tex_tab_cntl;
			union
			{
				uint32_t tex_lod_base0;
				uint32_t texld_dstbase;
				uint32_t tex_mm_dstbase;
			};
			uint32_t tex_lod_base1;
			uint32_t tex_lod_base2;
			uint32_t tex_lod_base3;
			union
			{
				uint32_t texld_srcbase;
				uint32_t tex_mm_srcbase;
			};
			union
			{
				uint32_t tex_bytecnt;
				uint32_t tex_rowcnt;
				uint32_t tex_texcnt;
			};
			uint32_t uv_max;
			uint32_t uv_mask;
			uint32_t tex_srctype01;
			uint32_t tex_srctype23;
			uint32_t tex_exptype;
			union
			{
				uint32_t tex_srcconst0;
				uint32_t tex_clrconst;
				uint32_t tex_catconst0;
			};
			union
			{
				uint32_t tex_srcconst1;
				uint32_t tex_pipconst1;
			};
			union
			{
				uint32_t tex_srcconst2;
				uint32_t tex_catiConst0;
			};
			union
			{
				uint32_t tex_srcconst3;
				uint32_t tex_caticonst1;
			};
			uint32_t tex_srcexp;
		};
		uint32_t m_regs[21];
	} m_tm;

	union
	{
		struct
		{
			uint32_t snoop;
			uint32_t supergen_ctrl;
			uint32_t usergen_ctrl;
			uint32_t discard_ctrl;
			uint32_t status;
			uint32_t int_ctrl;
			uint32_t fbclip;
			uint32_t x_winclip;
			uint32_t y_winclip;
			uint32_t dst_ctrl;
			uint32_t dst_baseaddr;
			uint32_t dst_xstride;
			uint32_t src_ctrl;
			uint32_t src_baseaddr;
			uint32_t src_xstride;
			uint32_t src_offset;
			uint32_t z_ctrl;
			uint32_t z_baseaddr;
			uint32_t z_offset;
			uint32_t z_clip;
			uint32_t ssbdsb_ctrl;
			uint32_t const_in;
			uint32_t txt_mult_cntl;
			uint32_t txt_coef_const0;
			uint32_t txt_coef_const1;
			uint32_t src_mult_cntl;
			uint32_t src_coef_const0;
			uint32_t src_coef_const1;
			uint32_t alu_ctrl;
			uint32_t src_alpha_ctrl;
			uint32_t dst_alpha_ctrl;
			uint32_t dst_alpha_const;
			uint32_t dither_mat_a;
			uint32_t dither_mat_b;
		};
		uint32_t m_regs[34];
	} m_db;

	// Destination blender state
	struct
	{
		uint32_t    x;
		uint32_t    y;
		uint32_t    w;

		rgba    ti;
		uint8_t ssb;

		rgba    src;
		uint8_t dsb;

		rgba    srcpath;
		rgba    texpath;
		rgba    blend;
		rgba    dst;
	} m_dbstate;

	te_state    m_state;

	std::unique_ptr<uint32_t[]> m_pipram;
	std::unique_ptr<uint32_t[]> m_tram;
};

DECLARE_DEVICE_TYPE(M2_TE, m2_te_device)

#endif // MAME_KONAMI_3DOM2_TE_H
