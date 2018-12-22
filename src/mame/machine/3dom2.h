// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

	3DO M2

	TODO: Move reg defines out of classes and into source

***************************************************************************/

#pragma once

#ifndef _3DOM2_H_
#define _3DOM2_H_

#include "emu.h"
#include "cpu/dspp/dspp.h"
#include "cpu/powerpc/ppc.h"
#include "screen.h"

#define M2_BAD_TIMING		0		// HACK

/***************************************************************************
    ENUMERATIONS
***************************************************************************/

enum
{
	SYSCFG_VIDEO_NTSC			= 0x00000000,
	SYSCFG_VIDEO_PAL			= 0x00000001,

	SYSCFG_VIDEO_ENCODER_MEIENC	= 0x00000000,	// NTSC by default
	SYSCFG_VIDEO_ENCODER_VP536	= 0x00000004,	// NTSC by default
	SYSCFG_VIDEO_ENCODER_BT9103	= 0x00000008,	// PAL by default
	SYSCFG_VIDEO_ENCODER_DENC	= 0x0000000C,	// PAL by default

	SYSCFG_REGION_UK			= 0x00000800,
	SYSCFG_REGION_JAPAN			= 0x00001000,
	SYSCFG_REGION_US			= 0x00001800,

#if 0 // Console
	SYSCFG_AUDIO_CS4216			= 0xA0000000,
	SYSCFG_AUDIO_ASASHI			= 0xE0000000,
#else
	SYSCFG_AUDIO_CS4216			= 0x20000000,
	SYSCFG_AUDIO_ASASHI			= 0x60000000,
#endif
	SYSCFG_BOARD_AC_DEVCARD		= 0x00040000,
	SYSCFG_BOARD_AC_COREBOARD	= 0x00058000,
	SYSCFG_BOARD_DEVCARD		= 0x00060000,
	SYSCFG_BOARD_UPGRADE		= 0x00070000,
	SYSCFG_BOARD_MULTIPLAYER	= 0x00078000,

	SYSCONFIG_ARCADE = 0x03600000 | SYSCFG_BOARD_AC_COREBOARD | SYSCFG_AUDIO_ASASHI | SYSCFG_REGION_JAPAN | SYSCFG_VIDEO_ENCODER_MEIENC | SYSCFG_VIDEO_NTSC,
};

enum bdaint_line
{
	BDAINT_EXTD4_LINE		= 3,
	BDAINT_EXTD3_LINE		= 4,
	BDAINT_EXTD2_LINE		= 5,
	BDAINT_EXTD1_LINE		= 6,
	BDAINT_PVIOL_LINE		= 7,
	BDAINT_WVIOL_LINE		= 8,
	BDAINT_TO_LINE			= 9,

	BDAINT_CEL_LINE			= 21,
	BDAINT_MYSTERY_LINE		= 22,
	BDAINT_VINT1_LINE		= 23,
	BDAINT_VINT0_LINE		= 24,
	BDAINT_DSP_LINE			= 25,
	BDAINT_MPEG_LINE		= 26,
	BDAINT_TRIGEN_LINE		= 27,
	BDAINT_TRIDFINST_LINE	= 28,
	BDAINT_TRIDMINST_LINE	= 29,
	BDAINT_TRILISTEND_LINE	= 30,
	BDAINT_TRIWINCLIP_LINE	= 31,
};

enum reg_wmode
{
	REG_WRITE,
	REG_INVALID,
	REG_SET,
	REG_CLEAR,
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class m2_bda_device;
class m2_powerbus_device;
class m2_memctl_device;
class m2_vdu_device;
class m2_te_device;
class m2_ctrlport_device;
class m2_mpeg_device;
class m2_cde_device;



/***************************************************************************
    BDA ASIC DEVICE
***************************************************************************/

class m2_bda_device : public device_t
{
public:
	enum rambank_size // TODO: REMOVE ME
	{
		RAM_2MB		= 2,
		RAM_4MB		= 4,
		RAM_8MB		= 8,
		RAM_16MB	= 16
	};

	template <typename T, typename U>
	m2_bda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu1_tag, U &&cpu2_tag)
		: m2_bda_device(mconfig, tag, owner, clock)
	{
		m_cpu1.set_tag(std::forward<T>(cpu1_tag));
		m_cpu2.set_tag(std::forward<U>(cpu2_tag));
	}
	m2_bda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto videores_in() { return m_videores_in.bind(); }
	auto ldac_handler() { return m_dac_l.bind(); }
	auto rdac_handler() { return m_dac_r.bind(); }
	void set_ram_size(rambank_size bank1, rambank_size bank2)
	{
		m_rambank_size[0] = bank1;
		m_rambank_size[1] = bank2;
	}

	// Interface
	DECLARE_READ32_MEMBER( cpu_id_r );
	DECLARE_WRITE32_MEMBER( cpu_id_w );

	READ8_MEMBER( read_bus )
	{
		return read_bus8(offset);
	}

	WRITE8_MEMBER( write_bus )
	{
		write_bus8(offset, data);
	}

	uint8_t read_bus8(offs_t offset);
	uint16_t read_bus16(offs_t offset);
	uint32_t read_bus32(offs_t offset);
	void write_bus8(offs_t offset, uint8_t data);
	void write_bus16(offs_t offset, uint16_t data);
	void write_bus32(offs_t offset, uint32_t data);

	void * ram_ptr() { return m_ram; }
	offs_t ram_start() { return RAM_BASE; }
	offs_t ram_end() { return RAM_BASE + m_ram_mask; }
	uint32_t get_rambank_size(uint32_t bank) const { return m_rambank_size[bank]; }

	void set_interrupt(uint32_t state);

//	screen_device * get_screen() const { return m_screen; }

protected:
	// Device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum base_addr
	{
		POWERBUS_BASE	= 0x00010000,
		MEMCTL_BASE		= 0x00020000,
		VDU_BASE		= 0x00030000,
		TE_BASE			= 0x00040000,
		DSP_BASE		= 0x00060000,
		CTRLPORT_BASE	= 0x00070000,
		MPEG_BASE		= 0x00080000,
		TE_TRAM_BASE	= 0x000c0000,
		SLOT1_BASE		= 0x01000000,
		SLOT2_BASE		= 0x02000000,
		SLOT3_BASE		= 0x03000000,
		SLOT4_BASE		= 0x04000000,
		SLOT5_BASE		= 0x05000000,
		SLOT6_BASE		= 0x06000000,
		SLOT7_BASE		= 0x07000000,
		SLOT8_BASE		= 0x08000000,
		CPUID_BASE		= 0x10000000,
		RAM_BASE		= 0x40000000,
	};

	enum dev_mask
	{
		DEVICE_MASK		= 0x0000ffff,
		SLOT_MASK		= 0x00ffffff,
		TE_TRAM_MASK	= 0x00003fff,
	};


	void configure_ppc_address_map(address_space &space);

public: // TODO: THIS SHOULD NOT BE PUBLIC
	required_device<ppc_device> m_cpu1;
	required_device<ppc_device> m_cpu2;
	devcb_read_line	m_videores_in;

	// Sub-devices
	required_device<m2_memctl_device>	m_memctl;
	required_device<m2_powerbus_device>	m_powerbus;
	required_device<m2_vdu_device>		m_vdu;
	required_device<m2_ctrlport_device>	m_ctrlport;
	required_device<dspp_device>		m_dspp;
	required_device<m2_mpeg_device>		m_mpeg;
	required_device<m2_te_device>		m_te;

	// System RAM
	uint32_t				*m_ram;
	uint32_t				m_rambank_size[2];
	uint32_t				m_ram_mask;

	devcb_write16		m_dac_l;
	devcb_write16		m_dac_r;

	emu_timer *m_dac_timer;

	// TODO
	friend class m2_memctl_device;
	friend class dspp;
};



/***************************************************************************
    POWERBUS CONTROLLER DEVICE
***************************************************************************/

class m2_powerbus_device : public device_t
{
public:
	m2_powerbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Static configuration helpers
	auto int_handler() { return m_int_handler.bind(); }

	template<uint32_t line> WRITE_LINE_MEMBER( int_line )
	{
		if (state)
			m_int_status |= 1 << line;
		else
			m_int_status &= ~(1 << line);

		update_interrupts();
	}

	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum reg_offs
	{
		BDAPCTL_DEVID		= 0x00,
		BDAPCTL_PBCONTROL	= 0x10,
		BDAPCTL_PBINTENSET	= 0x40,
		BDAPCTL_PBINTSTAT	= 0x50,
		BDAPCTL_ERRSTAT		= 0x60,
		BDAPCTL_ERRADDR		= 0x70,
	};

	void update_interrupts();

	devcb_write_line	m_int_handler;

	// Registers
	uint32_t	m_ctrl;
	uint32_t	m_int_enable;
	uint32_t	m_int_status;
	uint32_t	m_err_status;
	uint32_t	m_err_address;
};



/***************************************************************************
    MEMORY CONTROLLER DEVICE
***************************************************************************/

class m2_memctl_device : public device_t
{
public:
	m2_memctl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <std::size_t Line> auto gpio_in_handler() { return m_gpio_in[Line].bind(); }
	template <std::size_t Line> auto gpio_out_handler() { return m_gpio_out[Line].bind(); }

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum reg_offs
	{
		MCTL_MCONFIG	= 0x0,
		MCTL_MREF		= 0x4,
		MCTL_MCNTL		= 0x8,
		MCTL_MRESET		= 0xc,
	};

	enum mcfg_reg
	{
		MCFG_LDIA_MASK	= 0x07000000,
		MCFG_LDIA_SHIFT	= 24,
		MCFG_LDOA_MASK	= 0x00c00000,
		MCFG_LDOA_SHIFT	= 22,
		MCFG_RC_MASK	= 0x003c0000,
		MCFG_RC_SHIFT	= 18,
		MCFG_RCD_MASK	= 0x00030000,
		MCFG_RCD_SHIFT	= 16,
		MCFG_SS1_MASK	= 0x0000e000,
		MCFG_SS1_SHIFT	= 13,
		MCFG_SS0_MASK	= 0x00001c00,
		MCFG_SS0_SHIFT	= 10,
		MCFG_CL_MASK	= 0x00000030,
		MCFG_CL_SHIFT	= 4,
	};

	enum mref_reg
	{
		MREF_DEBUGADDR		= 0x7F000000,  /* Selector if GPIOx_GP == 0 */
		MREF_GPIO3_GP		= 0x00800000,  /* General purpose or debug out */
		MREF_GPIO3_OUT		= 0x00400000,  /* Output or input */
		MREF_GPIO3_VALUE	= 0x00200000,  /* Value if GPIOx_GP == 1 */
		MREF_GPIO2_GP		= 0x00100000,  /* General purpose or debug out */
		MREF_GPIO2_OUT		= 0x00080000,  /* Output or input */
		MREF_GPIO2_VALUE	= 0x00040000,  /* Value if GPIOx_GP == 1 */
		MREF_GPIO1_GP		= 0x00020000,  /* General purpose or debug out */
		MREF_GPIO1_OUT		= 0x00010000,  /* Output or input */
		MREF_GPIO1_VALUE	= 0x00008000,  /* Value if GPIOx_GP == 1 */
		MREF_GPIO0_GP		= 0x00004000,  /* General purpose or debug out */
		MREF_GPIO0_OUT		= 0x00002000,  /* Output or input */
		MREF_GPIO0_VALUE	= 0x00001000,  /* Value if GPIOx_GP == 1 */
		MREF_REFRESH		= 0x00000FFF,  /* Memory refresh count */
	};


	uint32_t ramsize_to_mcfg_field(uint32_t size)
	{
		// 0:0MB  1:2MB  2:4MB  3:4MB  4:4MB  5:8MB  6:16MB  7:0MB
		switch (size)
		{
			case 0:		return 0;
			case 2:		return 1;
			case 4:		return 2;
			case 8:		return 5;
			case 16:	return 6;
		}
		return 0;
	}


	// GPIO
	devcb_read_line	m_gpio_in[4];
	devcb_write_line m_gpio_out[4];

	// Registers
	uint32_t	m_mcfg;
	uint32_t	m_mref;
	uint32_t	m_mcntl;
	uint32_t	m_reset;
};



/***************************************************************************
    VDU DEVICE
***************************************************************************/

class m2_vdu_device : public device_t
{
public:
	m2_vdu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Static configuration helpers
	auto vint0_int_handler() { return m_vint0_int_handler.bind(); }
	auto vint1_int_handler() { return m_vint1_int_handler.bind(); }
	template <typename T> void set_screen(T &&screen_tag) { m_screen.set_tag(std::forward<T>(screen_tag)); }

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum timer_id
	{
		TIMER_ID_VBLANK,
		TIMER_ID_VINT0,
		TIMER_ID_VINT1,
	};

	void set_vint_timer(uint32_t id);
	void parse_dc_word(uint32_t data);
	void parse_av_word(uint32_t data);
	void parse_lc_word(uint32_t data);
	void draw_scanline(uint32_t *dst, uint32_t srclower, uint32_t srcupper);
	void draw_scanline_double(uint32_t *dst, uint32_t srclower, uint32_t srcupper);

	// Internal stuff
	required_device<screen_device> m_screen;
	emu_timer			*m_vint0_timer;
	emu_timer			*m_vint1_timer;
	devcb_write_line	m_vint0_int_handler;
	devcb_write_line	m_vint1_int_handler;

	// Registers
	uint32_t	m_vint;
	uint32_t	m_vdc0;
	uint32_t	m_vdc1;
	uint32_t	m_fv0a;
	uint32_t	m_fv1a;
	uint32_t	m_avdi;
	uint32_t	m_vdli;
	uint32_t	m_vcfg;
	uint32_t	m_dmt0;
	uint32_t	m_dmt1;
	uint32_t	m_vrst;

	// Screen parameters
	uint32_t	m_hstart;
	uint32_t 	m_htotal;
	uint32_t 	m_vstart;
	uint32_t 	m_vtotal;
};



/***************************************************************************
    CONTROL PORT DEVICE
***************************************************************************/

class m2_ctrlport_device : public device_t
{
public:
	m2_ctrlport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:

};



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

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

	uint32_t *tram_ptr() const { return m_tram; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:

	enum te_state
	{
		TE_STOPPED,
		TE_PAUSED,
		TE_RUNNING,
	};

	enum misc
	{
		PIP_RAM_WORDS			= 256,
		TEXTURE_RAM_WORDS		= 4096,
		PIP_RAM_BYTEMASK		= PIP_RAM_WORDS * 4 - 1,
		TEXTURE_RAM_BYTEMASK	= TEXTURE_RAM_WORDS * 4 - 1,
	};

	enum inst_type
	{
		INST_WRITE_REG	= 0x10000000,
		INST_VTX_SHORT	= 0x20000000,
		INST_VTX_LONG	= 0x30000000,
		INST_VTX_POINT	= 0x40000000,
		INST_MASK		= 0xf0000000,
	};

	enum vtx_flag
	{
		VTX_FLAG_SHAD	= 0x00010000,
		VTX_FLAG_TEXT	= 0x00020000,
		VTX_FLAG_PRSP	= 0x00040000,
		VTX_FLAG_NEW	= 0x00080000,
		VTX_FLAG_RM		= 0x00100000,
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

	void teicntl_w(uint32_t data, reg_wmode wmode);
	void tedcntl_w(uint32_t data, reg_wmode wmode);
	void master_mode_w(uint32_t data, reg_wmode wmode);

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

	m2_bda_device		*m_bda;
	const address_space_config  m_space_config; // TODO: Why is this still here?

	devcb_write_line	m_general_int_handler;
	devcb_write_line	m_dfinstr_int_handler;
	devcb_write_line	m_iminstr_int_handler;
	devcb_write_line	m_listend_int_handler;
	devcb_write_line	m_winclip_int_handler;

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
			se_vtx	vertices[3];
			uint32_t	reserved[16];
			uint32_t	vertex_state;
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
		uint32_t	x;
		uint32_t	y;
		uint32_t	w;

		rgba	ti;
		uint8_t	ssb;

		rgba	src;
		uint8_t	dsb;

		rgba	srcpath;
		rgba	texpath;
		rgba	blend;
		rgba	dst;
	} m_dbstate;

	te_state	m_state;

	uint32_t		*m_pipram;
	uint32_t		*m_tram;
};



/***************************************************************************
    MPEG DEVICE
***************************************************************************/

class m2_mpeg_device : public device_t
{
public:
	m2_mpeg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:

};



/***************************************************************************
    CDE ASIC DEVICE
***************************************************************************/

class m2_cde_device : public device_t
{
public:
	template <typename T>
	m2_cde_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu1_tag)
		: m2_cde_device(mconfig, tag, owner, clock)
	{
		m_cpu1.set_tag(std::forward<T>(cpu1_tag));
	}
	m2_cde_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Static configuration helpers
	auto int_handler() { return m_int_handler.bind(); }
	void set_syscfg(uint32_t syscfg) { m_syscfg = syscfg; }
	auto sdbg_out() { return m_sdbg_out_handler.bind(); }

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

	DECLARE_WRITE32_MEMBER(sdbg_in);

	void set_external_interrupt(uint32_t which, uint32_t state)
	{
		set_interrupt(CDE_EXT_INT);
	}

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:

	enum timer_id
	{
		TIMER_ID_READY,
		TIMER_ID_CD_DMA1,
		TIMER_ID_CD_DMA2,
		TIMER_ID_DMA1,
		TIMER_ID_DMA2,
	};

	enum reg_offs
	{
		// Miscellaneous
		CDE_DEVICE_ID		= 0x000,
		CDE_VERSION			= 0x004,
		CDE_SDBG_CNTL		= 0x00C,		// Serial debug control register
		CDE_SDBG_RD			= 0x010,		// Serial debug read data
		CDE_SDBG_WRT		= 0x014,		// Serial debug write data
		CDE_INT_STS			= 0x018,		// offset for status reg
		CDE_INT_ENABLE		= 0x01C,
		CDE_RESET_CNTL		= 0x020,
		CDE_ROM_DISABLE		= 0x024,
		CDE_CD_CMD_WRT		= 0x028,
		CDE_CD_STS_RD		= 0x02C,
		CDE_GPIO1			= 0x030,		// GPIO1 control register (UART interrupt)
		CDE_GPIO2			= 0x034,		// GPIO1 control register

		// BIO Bus
		CDE_DEV_DETECT		= 0x200,
		CDE_BBLOCK			= 0x204,
		CDE_BBLOCK_EN		= 0x208,		// Blocking enable register
		CDE_DEV5_CONF		= 0x20C,
		CDE_DEV_STATE		= 0x210,
		CDE_DEV6_CONF		= 0x214,
		CDE_DEV5_VISA_CONF	= 0x218,
		CDE_DEV6_VISA_CONF	= 0x21C,
		CDE_UNIQ_ID_CMD		= 0x220,
		CDE_UNIQ_ID_RD		= 0x224,
		CDE_DEV_ERROR		= 0x228,
		CDE_DEV7_CONF		= 0x22C,
		CDE_DEV7_VISA_CONF	= 0x230,
		CDE_DEV0_SETUP		= 0x240,
		CDE_DEV0_CYCLE_TIME	= 0x244,
		CDE_DEV1_SETUP		= 0x248,
		CDE_DEV1_CYCLE_TIME	= 0x24C,
		CDE_DEV2_SETUP		= 0x250,
		CDE_DEV2_CYCLE_TIME	= 0x254,
		CDE_DEV3_SETUP		= 0x258,
		CDE_DEV3_CYCLE_TIME	= 0x25C,
		CDE_DEV4_SETUP		= 0x260,
		CDE_DEV4_CYCLE_TIME	= 0x264,
		CDE_DEV5_SETUP		= 0x268,
		CDE_DEV5_CYCLE_TIME	= 0x26C,
		CDE_DEV6_SETUP		= 0x270,
		CDE_DEV6_CYCLE_TIME	= 0x274,
		CDE_DEV7_SETUP		= 0x278,
		CDE_DEV7_CYCLE_TIME	= 0x27C,
		CDE_SYSTEM_CONF		= 0x280,
		CDE_VISA_DIS		= 0x284,
		CDE_MICRO_RWS		= 0x290,
		CDE_MICRO_WI		= 0x294,
		CDE_MICRO_WOB		= 0x298,
		CDE_MICRO_WO		= 0x29C,
		CDE_MICRO_STATUS	= 0x2A0,

		// CD DMA
		CDE_CD_DMA1_CNTL	= 0x300,
		CDE_CD_DMA1_CPAD	= 0x308,
		CDE_CD_DMA1_CCNT	= 0x30C,
		CDE_CD_DMA1_NPAD	= 0x318,
		CDE_CD_DMA1_NCNT	= 0x31C,
		CDE_CD_DMA2_CNTL	= 0x320,
		CDE_CD_DMA2_CPAD	= 0x328,
		CDE_CD_DMA2_CCNT	= 0x32C,
		CDE_CD_DMA2_NPAD	= 0x338,
		CDE_CD_DMA2_NCNT	= 0x33C,

		// BioBus DMA
		CDE_DMA1_CNTL		= 0x1000,
		CDE_DMA1_CBAD		= 0x1004,
		CDE_DMA1_CPAD		= 0x1008,
		CDE_DMA1_CCNT		= 0x100C,
		CDE_DMA1_NBAD		= 0x1014,
		CDE_DMA1_NPAD		= 0x1018,
		CDE_DMA1_NCNT		= 0x101C,
		CDE_DMA2_CNTL		= 0x1020,
		CDE_DMA2_CBAD		= 0x1024,
		CDE_DMA2_CPAD		= 0x1028,
		CDE_DMA2_CCNT		= 0x102C,
		CDE_DMA2_NBAD		= 0x1034,
		CDE_DMA2_NPAD		= 0x1038,
		CDE_DMA2_NCNT		= 0x103C,
	};

	enum cde_int
	{
		CDE_INT_SENT		= 0x80000000,
		CDE_SDBG_WRT_DONE	= 0x10000000,
		CDE_SDBG_RD_DONE	= 0x08000000,
		CDE_DIPIR			= 0x04000000,
		CDE_ARM_BOUNDS		= 0x01000000,
		CDE_DMA2_BLOCKED	= 0x00400000,
		CDE_DMA1_BLOCKED	= 0x00200000,
		CDE_ID_READY		= 0x00100000,
		CDE_ARM_FENCE		= 0x00080000,
		CDE_EXT_INT			= 0x00040000, // PJB: Used for SIO?
		CDE_3DO_CARD_INT	= 0x00020000,
		CDE_ARM_INT			= 0x00010000,
		CDE_CD_DMA2_OF		= 0x00004000,
		CDE_CD_DMA1_OF		= 0x00002000,
		CDE_ARM_ABORT		= 0x00001000,
		CDE_CD_DMA2_DONE	= 0x00000800,
		CDE_CD_DMA1_DONE	= 0x00000400,
		CDE_DMA2_DONE		= 0x00000100,
		CDE_DMA1_DONE		= 0x00000080,
		CDE_PBUS_ERROR		= 0x00000040,
		CDE_CD_CMD_WRT_DONE	= 0x00000020,
		CDE_CD_STS_RD_DONE	= 0x00000010,
		CDE_CD_STS_FL_DONE	= 0x00000008,
		CDE_GPIO1_INT		= 0x00000004,
		CDE_GPIO2_INT		= 0x00000002,
		CDE_BBUS_ERROR		= 0x00000001,
	};

	enum cde_dma_cntl
	{
		CDE_DMA_DIRECTION	= 0x00000400,	/* PowerBus to BioBus if set */
		CDE_DMA_RESET		= 0x00000200,	/* Reset engine if set */
		CDE_DMA_GLOBAL		= 0x00000100,	/* snoopable trans if set */
		CDE_DMA_CURR_VALID	= 0x00000080,	/* current setup valid if set */
		CDE_DMA_NEXT_VALID	= 0x00000040,	/* next setup valid if set */
		CDE_DMA_GO_FOREVER	= 0x00000020,	/* copy next to current if set*/
		CDE_PB_CHANNEL_MASK	= 0x0000001F,	/* powerbus channel to use */
	};

	enum cde_dev_setup
	{
		CDE_WRITEN_HOLD		= 0x00000003,
		CDE_WRITEN_SETUP	= 0x0000001C,
		CDE_READ_HOLD		= 0x00000060,
		CDE_READ_SETUP		= 0x00000380,
		CDE_PAGEMODE		= 0x00000400,
		CDE_DATAWIDTH		= 0x00001800,
		CDE_DATAWIDTH_8		= 0x00000000,
		CDE_DATAWIDTH_16	= 0x00000800,
		CDE_READ_SETUP_IO	= 0x0000E000,
		CDE_MODEA			= 0x00010000,
		CDE_HIDEA			= 0x00020000,
	};

	void write_reg(uint32_t &reg, uint32_t data, bool clear);
	void set_interrupt(uint32_t intmask);
	void update_interrupts();

	void reset_dma(uint32_t ch);
	void start_dma(uint32_t ch);
	void next_dma(uint32_t ch);

	uint32_t address_to_biobus_slot(uint32_t addr) const
	{
		assert_always(addr >= 0x20000000 && addr <= 0x3fffffff, "Address not within BioBus address range");
		return ((addr >> 24) >> 2) & 7;
	}

	required_device<ppc_device> m_cpu1;
	m2_bda_device		*m_bda; // todo

	devcb_write_line	m_int_handler;
	devcb_write32		m_sdbg_out_handler;


	// Registers
	uint32_t	m_sdbg_in;
	uint32_t	m_sdbg_out;
	uint32_t	m_sdbg_cntl;
	uint32_t	m_int_status;
	uint32_t	m_int_enable;
	uint32_t	m_bblock_en;
	uint32_t	m_syscfg;
	uint32_t	m_visa_dis;

	struct biobus_device
	{
		uint32_t	m_setup;
		uint32_t	m_cycle_time;
	} m_bio_device[8];

	struct dma_channel
	{
		uint32_t		m_cntl;
		uint32_t		m_cbad;
		uint32_t		m_cpad;
		uint32_t		m_ccnt;
		uint32_t		m_nbad;
		uint32_t		m_npad;
		uint32_t		m_ncnt;
		emu_timer	*m_timer;
	} m_dma[2];
};

// device type definition
DECLARE_DEVICE_TYPE(M2_BDA, m2_bda_device)
DECLARE_DEVICE_TYPE(M2_POWERBUS, m2_powerbus_device)
DECLARE_DEVICE_TYPE(M2_MEMCTL, m2_memctl_device)
DECLARE_DEVICE_TYPE(M2_VDU, m2_vdu_device)
DECLARE_DEVICE_TYPE(M2_CTRLPORT, m2_ctrlport_device)
DECLARE_DEVICE_TYPE(M2_MPEG, m2_mpeg_device)
DECLARE_DEVICE_TYPE(M2_CDE, m2_cde_device)
DECLARE_DEVICE_TYPE(M2_TE, m2_te_device)

#endif // _3DOM2_H_
