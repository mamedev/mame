/*  Konami Cobra System


    Games on this hardware
	----------------------

	Game                                     ID        Year    Notes
	-----------------------------------------------------------------------
	Fighting Bujutsu / Fighting Wu-Shu     | G?645   | 1997  | 
	Racing Jam DX                          | GY676   | 1997  | GY676-PWB(F) LAN board


	Hardware overview:
	
	COBRA/603 GN645-PWB(A)   CPU Board
	----------------------------------
	    IBM PowerPC 603EV
		Motorola XPC105ARX66CD

	COBRA/403 GN645-PWB(B)   SUB Board
	----------------------------------
		IBM PowerPC 403GA-JC33C1 (32MHz)
		TI TMS57002
		Ricoh RF5c400
		M48T58-70PC1 Timekeeper
		ADC1038CIN A/D Converter
		2x AM7203A(?) FIFO (2K x 9)

	COBRA/604 GN645-PWB(C)   GFX Board
	----------------------------------
		IBM PowerPC 604
		Motorola XPC105ARX66CD
		Toshiba TMP47P241VM MCU (internal ROM?)
		4x CY7C4231 FIFO (2K x 9)
		Xilinx XC4300E FPGA
		Xilinx XC9536 FPGA

	GN645-PWB(D) Video Board (Also labeled IBM 36H3800. Seems to be related to the RS/6000 OpenGL accelerators.)
	-----------------------------------
		89G9380 (Xilinx part with an IBM sticker)
		PKA0702
		PLA0702 (4 dies on the chip)
		RAA0800
		BTA0803
		4x TOA2602
		Bt121KPJ80 RAMDAC

	GY676-PWB(F) LAN Board
	----------------------
	    Xilinx XC5210 FPGA
		Xilinx XC5204 FPGA
		Konami K001604 (2D tilemaps + 2x ROZ)
		Bt121KPC80 RAMDAC
*/

/*
    check_color_buffer(): 0, 0
        gfxfifo_exec: ram write 00100: 00000800
        gfxfifo_exec: ram write 00104: 00000000
        gfxfifo_exec: ram write 00108: 00000080
        gfxfifo_exec: ram write 0010C: 00000080
        gfxfifo_exec: ram write 00110: 20200000
        gfxfifo_exec: ram write 00120: 08800800
        gfxfifo_exec: ram write 00124: 00081018
        gfxfifo_exec: ram write 00128: 08080808
        gfxfifo_exec: ram write 80100: 00800000
        gfxfifo_exec: ram write 80104: 00800000
        gfxfifo_exec: ram write 80110: 00800000
        gfxfifo_exec: ram write 800A8: 00000000
        gfxfifo_exec: ram write 80108: 00000000

    check_color_buffer(): 0, 1
        gfxfifo_exec: ram write 00120: 08800800
        gfxfifo_exec: ram write 00124: 00081018
        gfxfifo_exec: ram write 00128: 08080808
        gfxfifo_exec: ram write 80100: 00200000
        gfxfifo_exec: ram write 80104: 00200000
        gfxfifo_exec: ram write 80110: 00200000
        gfxfifo_exec: ram write 800A8: 00000000

    check_overlay_buffer():
        gfxfifo_exec: ram write 00120: 08800800
        gfxfifo_exec: ram write 00124: 00081018
        gfxfifo_exec: ram write 00128: 08080808
        gfxfifo_exec: ram write 80100: 000E0000
        gfxfifo_exec: ram write 80104: 000E0000
        gfxfifo_exec: ram write 80110: 000E0000
        gfxfifo_exec: ram write 800A8: 00000000

    check_z_buffer():
        gfxfifo_exec: ram write 00120: 08000800
        gfxfifo_exec: ram write 00124: 00000010
        gfxfifo_exec: ram write 00128: 00001010
        gfxfifo_exec: ram write 80100: 00000000
        gfxfifo_exec: ram write 80104: 00000800
        gfxfifo_exec: ram write 80110: 00000800
        gfxfifo_exec: ram write 800A8: 80000000

    check_stencil_buffer():
        gfxfifo_exec: ram write 00120: 08000800
        gfxfifo_exec: ram write 00124: 00000010
        gfxfifo_exec: ram write 00128: 00001010
        gfxfifo_exec: ram write 80100: 00000000
        gfxfifo_exec: ram write 80104: 00000200
        gfxfifo_exec: ram write 80110: 00000200
        gfxfifo_exec: ram write 800A8: 80000000



	Bujutsu GL functions (603 board):
	
	?         : glDebugSwitch
	0x0000b784: glBindTexture?
	0x0000d40c: glEnable?
	0x0000d7f4: glDisable?
	0x0000d840: glAlphaFunc
	0x0000d9a4: glNewList
	0x0000dab0: glMatrixMode
	?         : glOrtho
	?         : glViewport
	0x0000db6c: glIdentity?
	0x0000dbf0: glFrustum
	?         : glCullFace
	?         : glClear
	?         : glCallList
	?         : glLightfv
	?         : glMaterialfv
	0x0001b5c8: glBlendFunc
	0x0001b8b0: glStencilOp
	?         : glStencilFunc
	0x000471e0: glTexParameterf
	0x0004a228: glColorMaterial
	?         : glFogfv
	?         : glFogf
	0x0004a7a8: glDepthFunc
	0x000508a8: glMaterialf
	0x000508fc: glLightModelfv
	0x00050a50: glTexEnvf
	?         : glTexEnvfv
	0x00050cc8: ? (param 0xff) could be glStencilMask
	0x00050d54: glFogi
	0x00050da0: glHint
	0x000cba6c: glTexParameteri


	Main:
	-
	0x1b14():
		-> 0xa6a8():   loop at 0xa7e0, waiting for [0x1790e8] to != 1

	Sub:
	-
	State 0xc00: waiting for?

	Gfx:
	-
	Fails on drawcheck()
	0x3806dc():   Waits for [0x7f7ffc] to change (in main)


	GFX Registers:

		0x00090:	Floating-point register (seen values 1.0f and 128.0f)
		0x0009c:	Floating-point register (seen values 0.0f and 256.0f)
		0x000a4:	Floating-point register (seen values 1.0f and -100.0f)
		0x000ac:	Floating-point register (seen values 0.0f and 200.0f)

		maybe a matrix?
		0x90 0x94 0x98 0x9c
		0xa0 0xa4 0xa8 0xac
		0xb0 0xb4 0xb8 0xbc

		0x00114:		High word: framebuffer pitch?   Low word: framebuffer pixel size?
		0x00118:		High word: FB pixel read X pos,   Low word: FB pixel read Y pos
		0x0011c:		Same as above?
		0x00458:		Set to 0x02100000 (0xff) by texselect()

		0x40018:		Set to 0x0001040a (0xc0) by mode_stipple()
		0x400d0:		Set to 0x80000000 (0x80) by mode_stipple()
		0x400f4:		Set to 0x40000000 (0x80) by texselect()
		0x40114:		Set to 0x00080000 (0x10) by mode_scissor()
		0x40138:		Set to 0x88800000 (0xe0) by mode_viewclip()
		0x40160:		Some viewport register? (high word: 192, low word: 150)
		0x40164:		Some viewport register? (high word: 352, low word: 275)
		0x40170:		Some viewport register? (high word: 160, low word: 125)
		0x40174:		Some viewport register? (high word: 320, low word: 250)
		0x40198:		Set to 0x80800000 (0xa0) by mode_alphatest()
		0x4019c:		Set to 0x88000000 (0xc0) by mode_fog()
		0x8001c:		Set to 0xc1d60060 (0xfe) by mode_stenciltest()
		0x80020:		Set to 0x00020000 (0x10) by mode_depthtest()
		0x80050:		Set to 0x04445500 (0x7c) by mode_blend()
		0x80054:		Set to 0x02400000 (0xe0) by mode_stencilmod()
		0x80114:		(0xcc) by mode_colormask()
		0x80118:		(0xcc) by mode_colormask()
		0x8011c:		(0xe0) by mode_stencilmask()
		0xc0c00..fff:	Texture RAM readback
		0xc3020:		Start address for texram writes?
		0xc3028:		Reads address from texram?
		0xc4c00..fff:	Texture RAM readback
		0xc8c00..fff:	Texture RAM readback
		0xccc00..fff:	Texture RAM readback
*/


#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "machine/pci.h"
#include "machine/idectrl.h"
#include "machine/timekpr.h"
#include "video/polynew.h"
#include "sound/rf5c400.h"

#define GFXFIFO_IN_VERBOSE		0
#define GFXFIFO_OUT_VERBOSE		0
#define M2SFIFO_VERBOSE			0
#define S2MFIFO_VERBOSE			0

struct cobra_polydata
{
};

class cobra_renderer : public poly_manager<float, cobra_polydata, 6, 10000>
{
public:
	cobra_renderer(running_machine &machine)
		: poly_manager<float, cobra_polydata, 6, 10000>(machine)
	{
		m_texture_ram = auto_alloc_array(machine, UINT32, 0x100000);

		m_framebuffer = auto_bitmap_rgb32_alloc(machine, 1024, 1024);
		m_backbuffer = auto_bitmap_rgb32_alloc(machine, 1024, 1024);
		m_overlay = auto_bitmap_rgb32_alloc(machine, 1024, 1024);
		m_zbuffer = auto_bitmap_ind32_alloc(machine, 1024, 1024);
		m_stencil = auto_bitmap_ind32_alloc(machine, 1024, 1024);

		// TODO: these are probably set by some 3D registers
		m_texture_width = 128;
		m_texture_height = 8;

		m_gfx_regmask = auto_alloc_array(machine, UINT32, 0x100);
		for (int i=0; i < 0x100; i++)
		{
			UINT32 mask = 0;
			if (i & 0x01) mask |= 0x0000000f;
			if (i & 0x02) mask |= 0x000000f0;
			if (i & 0x04) mask |= 0x00000f00;
			if (i & 0x08) mask |= 0x0000f000;
			if (i & 0x10) mask |= 0x000f0000;
			if (i & 0x20) mask |= 0x00f00000;
			if (i & 0x40) mask |= 0x0f000000;
			if (i & 0x80) mask |= 0xf0000000;

			m_gfx_regmask[i] = mask;
		}
	}

	void render_texture_scan(INT32 scanline, const extent_t &extent, const cobra_polydata &extradata, int threadid);
	void render_color_scan(INT32 scanline, const extent_t &extent, const cobra_polydata &extradata, int threadid);
	void draw_point(const rectangle &visarea, vertex_t &v, UINT32 color);
	void draw_line(const rectangle &visarea, vertex_t &v1, vertex_t &v2);

	void gfx_init(running_machine &machine);
	void gfx_reset(running_machine &machine);
	void gfx_fifo_exec(running_machine &machine);
	UINT32 gfx_read_gram(UINT32 address);
	void gfx_write_gram(UINT32 address, UINT32 mask, UINT32 data);

	void display(bitmap_rgb32 *bitmap, const rectangle &cliprect);

private:
	bitmap_rgb32 *m_framebuffer;
	bitmap_rgb32 *m_backbuffer;
	bitmap_rgb32 *m_overlay;
	bitmap_ind32 *m_zbuffer;
	bitmap_ind32 *m_stencil;

	UINT32 *m_texture_ram;
	int m_texture_width;
	int m_texture_height;

	UINT32 *m_gfx_gram;
	UINT32 *m_gfx_regmask;
};

class cobra_fifo
{
public:
	cobra_fifo(running_machine &machine, int capacity, const char *name, bool verbose)
	{
		m_data = auto_alloc_array(machine, UINT64, capacity);

		m_name = name;
		m_size = capacity;
		m_wpos = 0;
		m_rpos = 0;
		m_num = 0;

		m_verbose = verbose;
	}

	void push(const device_t *cpu, UINT64 data);
	bool pop(const device_t *cpu, UINT64 *result);
	bool pop(const device_t *cpu, float *result);
	int current_num();
	int space_left();
	bool is_empty();
	bool is_half_full();
	bool is_full();
	void flush();

private:
	int m_size;
	int m_wpos;
	int m_rpos;
	int m_num;
	bool m_verbose;
	const char *m_name;
	UINT64 *m_data;
};

class cobra_state : public driver_device
{
public:
	cobra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_gfxcpu(*this, "gfxcpu"),
		m_gfx_pagetable(*this, "pagetable")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_gfxcpu;
	required_shared_ptr<UINT64> m_gfx_pagetable;

	DECLARE_READ64_MEMBER(main_comram_r);
	DECLARE_WRITE64_MEMBER(main_comram_w);
	DECLARE_READ64_MEMBER(main_fifo_r);
	DECLARE_WRITE64_MEMBER(main_fifo_w);
	DECLARE_READ64_MEMBER(main_mpc106_r);
	DECLARE_WRITE64_MEMBER(main_mpc106_w);

	DECLARE_READ32_MEMBER(sub_comram_r);
	DECLARE_WRITE32_MEMBER(sub_comram_w);
	DECLARE_READ32_MEMBER(sub_sound_r);
	DECLARE_WRITE32_MEMBER(sub_sound_w);
	DECLARE_READ32_MEMBER(sub_unk7e_r);
	DECLARE_WRITE32_MEMBER(sub_debug_w);
	DECLARE_READ32_MEMBER(sub_unk1_r);
	DECLARE_WRITE32_MEMBER(sub_unk1_w);
	DECLARE_READ32_MEMBER(sub_config_r);
	DECLARE_WRITE32_MEMBER(sub_config_w);
	DECLARE_READ32_MEMBER(sub_mainbd_r);
	DECLARE_WRITE32_MEMBER(sub_mainbd_w);
	DECLARE_READ32_MEMBER(sub_ata0_r);
	DECLARE_WRITE32_MEMBER(sub_ata0_w);
	DECLARE_READ32_MEMBER(sub_ata1_r);
	DECLARE_WRITE32_MEMBER(sub_ata1_w);
	DECLARE_READ32_MEMBER(sub_psac2_r);
	DECLARE_WRITE32_MEMBER(sub_psac2_w);

	DECLARE_WRITE64_MEMBER(gfx_fifo0_w);
	DECLARE_WRITE64_MEMBER(gfx_fifo1_w);
	DECLARE_WRITE64_MEMBER(gfx_fifo2_w);
	DECLARE_WRITE64_MEMBER(gfx_debug_state_w);
	DECLARE_READ64_MEMBER(gfx_unk1_r);
	DECLARE_WRITE64_MEMBER(gfx_unk1_w);
	DECLARE_READ64_MEMBER(gfx_fifo_r);
	DECLARE_WRITE64_MEMBER(gfx_buf_w);

	cobra_renderer *m_renderer;

	cobra_fifo *m_gfxfifo_in;
	cobra_fifo *m_gfxfifo_out;
	cobra_fifo *m_m2sfifo;
	cobra_fifo *m_s2mfifo;

	UINT8 m_m2s_int_enable;
	UINT8 m_s2m_int_enable;

	int m2sfifo_unk_flag;
	int s2mfifo_unk_flag;


	UINT32 *m_comram[2];
	int m_comram_page;

	int m_main_debug_state;
	int m_main_debug_state_wc;
	int m_sub_debug_state;
	int m_sub_debug_state_wc;
	int m_gfx_debug_state;
	int m_gfx_debug_state_wc;

	UINT32 m_sub_psac_reg;
	int m_sub_psac_count;
	UINT32 m_sub_interrupt;

	UINT8 m_gfx_unk_flag;
	UINT32 m_gfx_re_command_word1;
	UINT32 m_gfx_re_command_word2;
	int m_gfx_re_word_count;
	int m_gfx_re_status;
	UINT32 m_gfx_unk_status;

	int m_gfx_register_select;
	UINT64 *m_gfx_register;
	UINT64 m_gfx_fifo_mem[4];
	int m_gfx_fifo_cache_addr;
	int m_gfx_fifo_loopback;
	int m_gfx_unknown_v1;
	int m_gfx_status_byte;
};

void cobra_renderer::render_color_scan(INT32 scanline, const extent_t &extent, const cobra_polydata &extradata, int threadid)
{
	/*
	UINT32 *fb = &m_framebuffer->pix32(scanline);

	UINT32 color = 0xffff0000; // TODO

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		fb[x] = color;
	}
	*/
}

void cobra_renderer::render_texture_scan(INT32 scanline, const extent_t &extent, const cobra_polydata &extradata, int threadid)
{
	float u = extent.param[0].start;
	float v = extent.param[1].start;
	float du = extent.param[0].dpdx;
	float dv = extent.param[1].dpdx;
	UINT32 *fb = &m_framebuffer->pix32(scanline);

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		int iu, iv;
		UINT32 texel;

		iu = (int)(u * m_texture_width);
		iv = (int)(v * m_texture_height);

		texel = m_texture_ram[((iv * m_texture_width) + iu) / 2];

		if (iu & 1)
		{
			texel &= 0xffff;
		}
		else
		{
			texel >>= 16;
		}

		UINT32 r = (texel & 0x7c00) << 9;
		UINT32 g = (texel & 0x03e0) << 6;
		UINT32 b = (texel & 0x001f) << 3;

		fb[x] = 0xff000000 | r | g | b;

		u += du;
		v += dv;
	}
}

void cobra_renderer::draw_point(const rectangle &visarea, vertex_t &v, UINT32 color)
{
	int x = v.x;
	int y = v.y;

	if (x >= visarea.min_x && x <= visarea.max_x &&
		y >= visarea.min_y && y <= visarea.max_y)
	{
		UINT32 *fb = &m_framebuffer->pix32(y);
		fb[x] = color;
	}
}

void cobra_renderer::draw_line(const rectangle &visarea, vertex_t &v1, vertex_t &v2)
{
	int dx = (v2.x - v1.x);
	int dy = (v2.y - v1.y);

	int x1 = v1.x;
	int y1 = v1.y;

	UINT32 color = 0xffffffff;		// TODO: where does the color come from?

	if (v1.x < visarea.min_x || v1.x > visarea.max_x ||
		v1.y < visarea.min_y || v1.y > visarea.max_y ||
		v2.x < visarea.min_x || v2.x > visarea.max_x ||
		v2.y < visarea.min_y || v2.y > visarea.max_x)
		return;

	if (dx > dy)
	{
		int x = x1;
		for (int i=0; i < abs(dx); i++)
		{
			int y = y1 + (dy * (float)(x - x1) / (float)(dx));

			UINT32 *fb = &m_framebuffer->pix32(y);
			fb[x] = color;

			x++;
		}
	}
	else
	{
		int y = y1;
		for (int i=0; i < abs(dy); i++)
		{
			int x = x1 + (dx * (float)(y - y1) / (float)(dy));

			UINT32 *fb = &m_framebuffer->pix32(y);
			fb[x] = color;

			y++;
		}
	}
}

static void cobra_video_exit(running_machine *machine)
{
}

VIDEO_START( cobra )
{
	cobra_state *cobra = machine.driver_data<cobra_state>();

	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(cobra_video_exit), &machine));

	cobra->m_renderer = auto_alloc(machine, cobra_renderer(machine));
	cobra->m_renderer->gfx_init(machine);
}

SCREEN_UPDATE_RGB32( cobra )
{
	cobra_state *cobra = screen.machine().driver_data<cobra_state>();

	cobra->m_renderer->display(&bitmap, cliprect);
	return 0;
}









/*****************************************************************************/

static int decode_debug_state_value(int v)
{
	switch (v)
	{
		case 0x01: return 0;
		case 0xcf: return 1;
		case 0x92: return 2;
		case 0x86: return 3;
		case 0xcc: return 4;
		case 0xa4: return 5;
		case 0xa0: return 6;
		case 0x8d: return 7;
		case 0x80: return 8;
		case 0x84: return 9;
		case 0x88: return 10;
		case 0xe0: return 11;
		case 0xb1: return 12;
		case 0xc2: return 13;
		case 0xb0: return 14;
		case 0xb8: return 15;
		default: return 0;
	}
}


void cobra_fifo::push(const device_t *cpu, UINT64 data)
{
	if (m_verbose)
	{
		char accessor_location[50];
		if (cpu != NULL)
		{
			// cpu has a name and a PC
			sprintf(accessor_location, "(%s) %08X", cpu->tag(), cpu_get_pc((device_t *)cpu));
		}
		else
		{
			// non-cpu
			sprintf(accessor_location, "(non-cpu)");
		}

		printf("%s: push %08X%08X (%d) at %s\n", m_name, (UINT32)(data >> 32), (UINT32)(data), m_num, accessor_location);
	}

	if (m_num == m_size)
	{
		if (m_verbose)
		{
			int i, j;
			char accessor_location[50];
			if (cpu != NULL)
			{
				// cpu has a name and a PC
				sprintf(accessor_location, "(%s) %08X", cpu->tag(), cpu_get_pc((device_t *)cpu));
			}
			else
			{
				// non-cpu
				sprintf(accessor_location, "(non-cpu)");
			}

			printf("%s overflow at %s\n", m_name, accessor_location);
			printf("%s dump:\n", m_name);

			for (j=0; j < 128; j+=4)
			{
				printf("    ");
				for (i=0; i < 4; i++)
				{
					UINT64 val = 0;
					pop(cpu, &val);
					printf("%08X ", (UINT32)(val));
				}
				printf("\n");
			}
			printf("\n");
		}

		return;
	}

	m_data[m_wpos] = data;

	m_wpos++;

	if (m_wpos == m_size)
	{
		m_wpos = 0;
	}

	m_num++;
}

bool cobra_fifo::pop(const device_t *cpu, UINT64 *result)
{
	UINT64 r;

	if (m_num == 0)
	{
		if (m_verbose)
		{
			char accessor_location[50];
			if (cpu != NULL)
			{
				// cpu has a name and a PC
				sprintf(accessor_location, "(%s) %08X", cpu->tag(), cpu_get_pc((device_t *)cpu));
			}
			else
			{
				// non-cpu
				sprintf(accessor_location, "(non-cpu)");
			}

			printf("%s underflow at %s\n", m_name, accessor_location);
		}
		return false;
	}

	r = m_data[m_rpos];

	if (m_verbose)
	{
		char accessor_location[50];
		if (cpu != NULL)
		{
			// cpu has a name and a PC
			sprintf(accessor_location, "(%s) %08X", cpu->tag(), cpu_get_pc((device_t *)cpu));
		}
		else
		{
			// non-cpu
			sprintf(accessor_location, "(non-cpu)");
		}

		printf("%s: pop %08X%08X (%d) at %s\n", m_name, (UINT32)(r >> 32), (UINT32)(r), m_num-1, accessor_location);
	}

	m_rpos++;

	if (m_rpos == m_size)
	{
		m_rpos = 0;
	}

	m_num--;

	*result = r;

	return true;
}

bool cobra_fifo::pop(const device_t *cpu, float *result)
{
	UINT64 value = 0;
	bool status = pop(cpu, &value);
	*result = u2f((UINT32)(value));
	return status;
}

int cobra_fifo::current_num()
{
	return m_num;
}

int cobra_fifo::space_left()
{
	return m_size - m_num;
}

bool cobra_fifo::is_empty()
{
	return (m_num == 0);
}

bool cobra_fifo::is_half_full()
{
	return (m_num > (m_size / 2));
}

bool cobra_fifo::is_full()
{
	return (m_num >= m_size);
}

void cobra_fifo::flush()
{
	m_num = 0;
	m_rpos = 0;
	m_wpos = 0;
}


/*****************************************************************************/

/*****************************************************************************/
// Main board (PPC603)

// MPC106 mem settings:
// Bank 0: start 0x00, end 0x7f
// Bank 1: start 0x81, end 0x81
// Bank 2: start 0x82, end 0x82
// Bank 3: start 0x83, end 0x83
// Bank 4: start 0x84, end 0x84
// Bank 5: start 0x85, end 0x85
// Bank 6: start 0x86, end 0x86
// Bank 7: start 0x87, end 0x87

// IBAT0 U: 0xfff00003 L: 0xfff00001    (0xfff00000, 128K)
// IBAT1 U: 0x0000007f L: 0x00000001    (0x00000000, 4MB)
// IBAT2 U: 0x0040007f L: 0x07c00001    (0x07c00000, 4MB)
// IBAT3 U: 0x00000000 L: 0x00000001    unused
// DBAT0 U: 0xfff0001f L: 0xfff0002a    (0xfff00000, 1MB)
// DBAT1 U: 0x0000007f L: 0x00000002    (0x00000000, 4MB)
// DBAT2 U: 0x0040007f L: 0x07c00002    (0x07c00000, 4MB)
// DBAT3 U: 0xc0000fff L: 0xc0000002    (0xc0000000, 128MB)

// RPA: 0x8010C000

static UINT32 mpc106_regs[256/4];
static UINT32 mpc106_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
	//printf("MPC106: PCI read %d, %02X, %08X\n", function, reg, mem_mask);

	switch (reg)
	{
	}

	return mpc106_regs[reg/4];
}

static void mpc106_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	//printf("MPC106: PCI write %d, %02X, %08X, %08X\n", function, reg, data, mem_mask);
	COMBINE_DATA(mpc106_regs + (reg/4));
}

READ64_MEMBER(cobra_state::main_mpc106_r)
{
	pci_bus_legacy_device *device = machine().device<pci_bus_legacy_device>("pcibus");
	//return pci_64be_r(offset, mem_mask);
	return device->read_64be(space, offset, mem_mask);
}

WRITE64_MEMBER(cobra_state::main_mpc106_w)
{
	pci_bus_legacy_device *device = machine().device<pci_bus_legacy_device>("pcibus");
	//pci_64be_w(offset, data, mem_mask);
	device->write_64be(space, offset, data, mem_mask);
}

READ64_MEMBER(cobra_state::main_fifo_r)
{
	UINT64 r = 0;

	if (ACCESSING_BITS_56_63)
	{
		// Register 0xffff0000:
		// Main-to-Sub FIFO status register
		// Sub-to-Main FIFO status register
		//
		// 7 6 5 4 3 2 1 0
		//----------------
		//               x      M2S FIFO full flag
		//             x        M2S FIFO empty flag
		//           x          M2S FIFO half-full flag
		//       x              S2M FIFO full flag
		//     x                S2M FIFO empty flag
		//   x                  S2M FIFO half-full flag
		// x                    Comram page

		int value = 0x00;
		value |= m_m2sfifo->is_full() ? 0x00 : 0x01;
		value |= m_m2sfifo->is_empty() ? 0x00 : 0x02;
		value |= m_m2sfifo->is_half_full() ? 0x00 : 0x04;

		value |= m_s2mfifo->is_full() ? 0x00 : 0x10;
		value |= m_s2mfifo->is_empty() ? 0x00 : 0x20;
		value |= m_s2mfifo->is_half_full() ? 0x00 : 0x40;

		value |= m_comram_page ? 0x80 : 0x00;

		r |= (UINT64)(value) << 56;
	}
	if (ACCESSING_BITS_48_55)
	{
		// Register 0xffff0001:
		// Sub board FIFO unknown register
	}
	if (ACCESSING_BITS_40_47)
	{
		// Register 0xffff0002:
		// Sub-to-Main FIFO read data

		UINT64 value;
		m_s2mfifo->pop(&space.device(), &value);

		if (m_s2mfifo->is_empty())
		{
			s2mfifo_unk_flag = 1;
		}
		else if (m_s2mfifo->is_full())
		{
			s2mfifo_unk_flag = 0;
		}

		r |= (UINT64)(value & 0xff) << 40;


		/*
        if (fifo_is_empty(S2MFIFO))
        {
            device_spin_until_time(machine().device("subcpu"), attotime::from_usec(80));
        }
        */
	}
	if (ACCESSING_BITS_32_39)
	{
		// Register 0xffff0003:
		//
		// 7 6 5 4 3 2 1 0
		//----------------
		//             x     S2M FIFO unknown flag
		//           x       Graphics board/FIFO busy flag
		//         x         M2S FIFO unknown flag

		int value = 0x01;
		//value |= s2mfifo_unk_flag ? 0x2 : 0x0;

		if (m_s2mfifo->is_empty())
		{
			value |= 0x2;
		}
		else if (!m_s2mfifo->is_full() && !m_s2mfifo->is_half_full())
		{
			//value |= s2mfifo_unk_flag ? 0x2 : 0x0;
		}

		value |= m2sfifo_unk_flag ? 0x8 : 0x0;

		value |= (m_gfx_unk_flag & 0x80) ? 0x00 : 0x04;

		r |= (UINT64)(value) << 32;
	}

	return r;
}

WRITE64_MEMBER(cobra_state::main_fifo_w)
{
	if (ACCESSING_BITS_40_47)
	{
		// Register 0xffff0002:
		// Main-to-Sub FIFO write data

		//printf("MAIN: M2S FIFO data write %02X\n", (UINT8)(data >> 40) & 0xff);

		m_m2sfifo->push(&space.device(), (UINT8)(data >> 40));

	//	cputag_set_input_line(space.machine(), "subcpu", INPUT_LINE_IRQ0, ASSERT_LINE);

		// this is a hack...
		// MAME has a small interrupt latency, which prevents the IRQ bit from being set in
		// the EXISR at the same time as the FIFO status updates.....
		cpu_set_reg(m_subcpu, PPC_EXISR, cpu_get_reg(m_subcpu, PPC_EXISR) | 0x10);
	}
	if (ACCESSING_BITS_32_39)
	{
		// Register 0xffff0003:
		// Main-to-Sub FIFO unknown
		//
		// 7 6 5 4 3 2 1 0
		//----------------
		//         x            M2S unknown flag
		// x                    Comram page

		if (!m_m2sfifo->is_empty())
		{
			if (m_m2sfifo->is_half_full())
			{
				m2sfifo_unk_flag = 0x8;
			}
			else
			{
				m2sfifo_unk_flag = ((data >> 32) & 0x8) ? 0 : 1;
			}
		}
		else
		{
			m2sfifo_unk_flag = 0x0;
		}

		m_comram_page = ((data >> 32) & 0x80) ? 1 : 0;
	}
	if (ACCESSING_BITS_24_31)
	{
		// Register 0xffff0004:
		// Interrupt enable for ???
		//
		// 7 6 5 4 3 2 1 0
		//----------------
		// x                    ?

		printf("main_fifo_w: 0xffff0004: %02X\n", (UINT8)(data >> 24));
	}
	if (ACCESSING_BITS_16_23)
	{
		// Register 0xffff0005:
		// Interrupt enable for S2MFIFO
		//
		// 7 6 5 4 3 2 1 0
		//----------------
		// x                    ?

		m_s2m_int_enable = (UINT8)(data >> 16);

		if ((m_s2m_int_enable & 0x80) == 0)
		{
			// clear the interrupt
			cputag_set_input_line(space.machine(), "maincpu", INPUT_LINE_IRQ0, CLEAR_LINE);
		}
	}
	if (ACCESSING_BITS_0_7)
	{
		// Register 0xffff0007:
		// Interrupt enable for M2SFIFO
		//
		// 7 6 5 4 3 2 1 0
		//----------------
		// x                    ?

		m_m2s_int_enable = (UINT8)(data);

		printf("main_fifo_w: 0xffff0007: %02X\n", (UINT8)(data >> 0));
	}

	// Register 0xffff0000,1
	// Debug state write

	if (ACCESSING_BITS_56_63)
	{
		m_main_debug_state |= decode_debug_state_value((data >> 56) & 0xff) << 4;
		m_main_debug_state_wc++;
	}
	if (ACCESSING_BITS_48_55)
	{
		m_main_debug_state |= decode_debug_state_value((data >> 48) & 0xff);
		m_main_debug_state_wc++;
	}

	if (m_main_debug_state_wc >= 2)
	{
		if (m_main_debug_state != 0)
		{
			printf("MAIN: debug state %02X\n", m_main_debug_state);
		}

		m_main_debug_state = 0;
		m_main_debug_state_wc = 0;
	}
}

READ64_MEMBER(cobra_state::main_comram_r)
{
	UINT64 r = 0;
	int page = m_comram_page;

	if (ACCESSING_BITS_32_63)
	{
		r |= (UINT64)(m_comram[page][(offset << 1) + 0]) << 32;
	}
	if (ACCESSING_BITS_0_31)
	{
		r |= (UINT64)(m_comram[page][(offset << 1) + 1]);
	}

	return r;
}

WRITE64_MEMBER(cobra_state::main_comram_w)
{
	int page = m_comram_page;

	UINT32 w1 = m_comram[page][(offset << 1) + 0];
	UINT32 w2 = m_comram[page][(offset << 1) + 1];
	UINT32 d1 = (UINT32)(data >> 32);
	UINT32 d2 = (UINT32)(data);
	UINT32 m1 = (UINT32)(mem_mask >> 32);
	UINT32 m2 = (UINT32)(mem_mask);

	m_comram[page][(offset << 1) + 0] = (w1 & ~m1) | (d1 & m1);
	m_comram[page][(offset << 1) + 1] = (w2 & ~m2) | (d2 & m2);
}

static ADDRESS_MAP_START( cobra_main_map, AS_PROGRAM, 64, cobra_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM
	AM_RANGE(0x07c00000, 0x07ffffff) AM_RAM
	AM_RANGE(0x80000cf8, 0x80000cff) AM_READWRITE(main_mpc106_r, main_mpc106_w)
	AM_RANGE(0xc0000000, 0xc03fffff) AM_RAM AM_SHARE("gfx_main_ram_0")				// GFX board main ram, bank 0
	AM_RANGE(0xc7c00000, 0xc7ffffff) AM_RAM AM_SHARE("gfx_main_ram_1")				// GFX board main ram, bank 1
	AM_RANGE(0xfff00000, 0xfff7ffff) AM_ROM AM_REGION("user1", 0)					/* Boot ROM */
	AM_RANGE(0xfff80000, 0xfffbffff) AM_READWRITE(main_comram_r, main_comram_w)
	AM_RANGE(0xffff0000, 0xffff0007) AM_READWRITE(main_fifo_r, main_fifo_w)
ADDRESS_MAP_END


/*****************************************************************************/
// Sub board (PPC403)

//static int ucount = 0;

READ32_MEMBER(cobra_state::sub_unk1_r)
{
	UINT32 r = 0;

	if (ACCESSING_BITS_16_23)
	{
		r |= 0x10000;
	}

	return r;
}

WRITE32_MEMBER(cobra_state::sub_unk1_w)
{
	/*
    if (!(mem_mask & 0xff000000))
    {
        printf("%02X", data >> 24);
        ucount++;

        if (ucount >= 4)
        {
            ucount = 0;
            printf("\n");
        }
    }
    */
}

READ32_MEMBER(cobra_state::sub_mainbd_r)
{
	UINT32 r = 0;

	if (ACCESSING_BITS_24_31)
	{
		// Register 0x7E380000
		// M2S FIFO read

		UINT64 value;
		m_m2sfifo->pop(&space.device(), &value);

		if (m_m2sfifo->is_empty())
		{
	//      cputag_set_input_line(space.machine(), "subcpu", INPUT_LINE_IRQ0, CLEAR_LINE);

			// this is a hack...
			// MAME has a small interrupt latency, which prevents the IRQ bit from being cleared in
			// the EXISR at the same time as the FIFO status updates.....
			cpu_set_reg(m_subcpu, PPC_EXISR, cpu_get_reg(m_subcpu, PPC_EXISR) & ~0x10);
		}

		r |= (value & 0xff) << 24;

		/*
        if (fifo_is_empty(M2SFIFO))
        {
            device_spin_until_time(machine().device("maincpu"), attotime::from_usec(80));
        }
        */
	}
	if (ACCESSING_BITS_16_23)
	{
		// Register 0x7E380001
		// Main-to-sub FIFO status register
		// Sub-to-main FIFO status register
		//
		// 7 6 5 4 3 2 1 0
		//----------------
		//               x    S2M FIFO full flag
		//             x      S2M FIFO empty flag
		//           x        S2M FIFO half-full flag
		//       x            M2S FIFO full flag
		//     x              M2S FIFO empty flag
		//   x                M2S FIFO half-full flag

		UINT32 value = 0x00;
		value |= m_s2mfifo->is_full() ? 0x00 : 0x01;
		value |= m_s2mfifo->is_empty() ? 0x00 : 0x02;
		value |= m_s2mfifo->is_half_full() ? 0x00 : 0x04;

		value |= m_m2sfifo->is_full() ? 0x00 : 0x10;
		value |= m_m2sfifo->is_empty() ? 0x00 : 0x20;
		value |= m_m2sfifo->is_half_full() ? 0x00 : 0x40;

		value |= m_comram_page ? 0x80 : 0x00;

		r |= (value) << 16;
	}

	return r;
}

WRITE32_MEMBER(cobra_state::sub_mainbd_w)
{
	if (ACCESSING_BITS_24_31)
	{
		// Register 0x7E380000
		// Sub-to-Main FIFO data

		m_s2mfifo->push(&space.device(), (UINT8)(data >> 24));

		// fire off an interrupt if enabled
		if (m_s2m_int_enable & 0x80)
		{
			cputag_set_input_line(space.machine(), "maincpu", INPUT_LINE_IRQ0, ASSERT_LINE);
		}
	}
	if (ACCESSING_BITS_16_23)
	{
		// Register 0x7E380001

		if (!m_s2mfifo->is_empty())
		{
			if (m_s2mfifo->is_half_full())
			{
				s2mfifo_unk_flag = 1;
			}
			else
			{
				s2mfifo_unk_flag = (~(data >> 16) & 0x1);
			}
		}
		else
		{
			s2mfifo_unk_flag = 0;
		}

		if (!s2mfifo_unk_flag)
		{
//          cputag_set_input_line(space.machine(), "subcpu", INPUT_LINE_IRQ1, ASSERT_LINE);

			// this is a hack...
			// MAME has a small interrupt latency, which prevents the IRQ bit from being set in
			// the EXISR at the same time as the FIFO status updates.....
			cpu_set_reg(m_subcpu, PPC_EXISR, cpu_get_reg(m_subcpu, PPC_EXISR) | 0x08);
		}
		else
		{
//          cputag_set_input_line(space.machine(), "subcpu", INPUT_LINE_IRQ1, CLEAR_LINE);

			// this is a hack...
			// MAME has a small interrupt latency, which prevents the IRQ bit from being cleared in
			// the EXISR at the same time as the FIFO status updates.....
			cpu_set_reg(m_subcpu, PPC_EXISR, cpu_get_reg(m_subcpu, PPC_EXISR) & ~0x08);
		}
	}
}

READ32_MEMBER(cobra_state::sub_unk7e_r)
{
	return 0xffffffff;
}

WRITE32_MEMBER(cobra_state::sub_debug_w)
{
	if (ACCESSING_BITS_24_31)
	{
		m_sub_debug_state |= decode_debug_state_value((data >> 24) & 0xff) << 4;
		m_sub_debug_state_wc++;
	}
	if (ACCESSING_BITS_16_23)
	{
		m_sub_debug_state |= decode_debug_state_value((data >> 16) & 0xff);
		m_sub_debug_state_wc++;
	}

	if (m_sub_debug_state_wc >= 2)
	{
		if (m_sub_debug_state != 0)
		{
			printf("SUB: debug state %02X\n", m_sub_debug_state);
		}

		m_sub_debug_state = 0;
		m_sub_debug_state_wc = 0;
	}
}

READ32_MEMBER(cobra_state::sub_config_r)
{
	UINT32 r = 0;

	if (ACCESSING_BITS_8_15)
	{
		r |= (0x2) << 8;		// if bit 0x2 is zero, maskrom boot
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= m_sub_interrupt;
	}

	return r;
}

WRITE32_MEMBER(cobra_state::sub_config_w)
{

}

READ32_MEMBER(cobra_state::sub_ata0_r)
{
	device_t *device = machine().device("ide");
	UINT32 r = 0;

	if (ACCESSING_BITS_16_31)
	{
		UINT16 v = ide_bus_r(device, 0, (offset << 1) + 0);
		r |= ((v << 8) | (v >> 8)) << 16;
	}
	if (ACCESSING_BITS_0_15)
	{
		UINT16 v = ide_bus_r(device, 0, (offset << 1) + 1);
		r |= ((v << 8) | (v >> 8)) << 0;
	}

	return r;
}

WRITE32_MEMBER(cobra_state::sub_ata0_w)
{
	device_t *device = machine().device("ide");

	if (ACCESSING_BITS_16_31)
	{
		UINT16 d = ((data >> 24) & 0xff) | ((data >> 8) & 0xff00);
		ide_bus_w(device, 0, (offset << 1) + 0, d);
	}
	if (ACCESSING_BITS_0_15)
	{
		UINT16 d = ((data >> 8) & 0xff) | ((data << 8) & 0xff00);
		ide_bus_w(device, 0, (offset << 1) + 1, d);
	}
}

READ32_MEMBER(cobra_state::sub_ata1_r)
{
	device_t *device = machine().device("ide");
	UINT32 r = 0;

	if (ACCESSING_BITS_16_31)
	{
		UINT16 v = ide_bus_r(device, 1, (offset << 1) + 0);
		r |= ((v << 8) | (v >> 8)) << 16;
	}
	if (ACCESSING_BITS_0_15)
	{
		UINT16 v = ide_bus_r(device, 1, (offset << 1) + 1);
		r |= ((v << 8) | (v >> 8)) << 0;
	}

	return r;
}

WRITE32_MEMBER(cobra_state::sub_ata1_w)
{
	device_t *device = machine().device("ide");

	if (ACCESSING_BITS_16_31)
	{
		UINT16 d = ((data >> 24) & 0xff) | ((data >> 8) & 0xff00);
		ide_bus_w(device, 1, (offset << 1) + 0, d);
	}
	if (ACCESSING_BITS_0_15)
	{
		UINT16 d = ((data >> 8) & 0xff) | ((data << 8) & 0xff00);
		ide_bus_w(device, 1, (offset << 1) + 1, d);
	}
}

READ32_MEMBER(cobra_state::sub_comram_r)
{
	int page = m_comram_page ^ 1;

	return m_comram[page][offset];
}

WRITE32_MEMBER(cobra_state::sub_comram_w)
{
	int page = m_comram_page ^ 1;

	COMBINE_DATA(m_comram[page] + offset);
}

READ32_MEMBER(cobra_state::sub_psac2_r)
{
	m_sub_psac_count++;
	if (m_sub_psac_count >= 0x8000)
	{
		m_sub_psac_reg ^= 0xffffffff;
		m_sub_psac_count = 0;
	}
	return m_sub_psac_reg;
}

WRITE32_MEMBER(cobra_state::sub_psac2_w)
{

}

static UINT32 sub_unknown_dma_r(device_t *device, int width)
{
	printf("DMA read from unknown: size %d\n", width);
	return 0;
}

static void sub_unknown_dma_w(device_t *device, int width, UINT32 data)
{
	printf("DMA write to unknown: size %d, data %08X\n", width, data);
}

static ADDRESS_MAP_START( cobra_sub_map, AS_PROGRAM, 32, cobra_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_MIRROR(0x80000000) AM_RAM											// Main RAM
	AM_RANGE(0x70000000, 0x7003ffff) AM_MIRROR(0x80000000) AM_READWRITE(sub_comram_r, sub_comram_w)			// Double buffered shared RAM between Main and Sub
	AM_RANGE(0x78040000, 0x7804ffff) AM_MIRROR(0x80000000) AM_DEVREADWRITE16_LEGACY("rfsnd", rf5c400_r, rf5c400_w, 0xffffffff)
	AM_RANGE(0x78080000, 0x7808000f) AM_MIRROR(0x80000000) AM_READWRITE(sub_ata0_r, sub_ata0_w)
	AM_RANGE(0x780c0010, 0x780c001f) AM_MIRROR(0x80000000) AM_READWRITE(sub_ata1_r, sub_ata1_w)
	AM_RANGE(0x78220000, 0x7823ffff) AM_MIRROR(0x80000000) AM_RAM											// PSAC RAM
	AM_RANGE(0x78240000, 0x78241fff) AM_MIRROR(0x80000000) AM_RAM											// PSAC unknown
	AM_RANGE(0x78300000, 0x7830000f) AM_MIRROR(0x80000000) AM_READWRITE(sub_psac2_r, sub_psac2_w)			// PSAC
	AM_RANGE(0x7e000000, 0x7e000003) AM_MIRROR(0x80000000) AM_READWRITE(sub_unk7e_r, sub_debug_w)
	AM_RANGE(0x7e040000, 0x7e041fff) AM_MIRROR(0x80000000) AM_DEVREADWRITE8_LEGACY("m48t58", timekeeper_r, timekeeper_w, 0xffffffff)	/* M48T58Y RTC/NVRAM */
	AM_RANGE(0x7e180000, 0x7e180003) AM_MIRROR(0x80000000) AM_READWRITE(sub_unk1_r, sub_unk1_w)				// TMS57002?
	AM_RANGE(0x7e200000, 0x7e200003) AM_MIRROR(0x80000000) AM_READWRITE(sub_config_r, sub_config_w)
//  AM_RANGE(0x7e240000, 0x7e27ffff) AM_MIRROR(0x80000000) AM_RAM                                           // PSAC (ROZ) in Racing Jam.
//  AM_RANGE(0x7e280000, 0x7e28ffff) AM_MIRROR(0x80000000) AM_RAM                                           // LANC
//  AM_RANGE(0x7e300000, 0x7e30ffff) AM_MIRROR(0x80000000) AM_RAM                                           // LANC
	AM_RANGE(0x7e380000, 0x7e380003) AM_MIRROR(0x80000000) AM_READWRITE(sub_mainbd_r, sub_mainbd_w)
	AM_RANGE(0x7ff80000, 0x7fffffff) AM_MIRROR(0x80000000) AM_ROM AM_REGION("user2", 0)						/* Boot ROM */
ADDRESS_MAP_END


/*****************************************************************************/
// Graphics board (PPC604)

// MPC106 mem settings:
// Bank 0: start 0x00, end 0x7f
// Bank 1: start 0x81, end 0x81
// Bank 2: start 0x82, end 0x82
// Bank 3: start 0x83, end 0x83
// Bank 4: start 0x84, end 0x84
// Bank 5: start 0x100, end 0x13f
// Bank 6: start 0x180, end 0x1bf
// Bank 7: start 0x1e0, end 0x1ef

// IBAT0 U: 0xfff00003 L: 0xfff00001    (0xfff00000, 0xfff00000, 128KB)
// IBAT1 U: 0x0000007f L: 0x00000001    (0x00000000, 0x00000000, 4MB)
// IBAT2 U: 0x0040007f L: 0x07c00001    (0x00400000, 0x07c00000, 4MB)
// IBAT3 U: 0x00000000 L: 0x00000001    unused
// DBAT0 U: 0xfff0001f L: 0xfff0002a    (0xfff00000, 0xfff00000, 1MB)
// DBAT1 U: 0x0000007f L: 0x00000002    (0x00000000, 0x00000000, 4MB)
// DBAT2 U: 0x0040007f L: 0x07c00002    (0x00400000, 0x07c00000, 4MB)
// DBAT3 U: 0xf8fe0003 L: 0xf8fe002a    (0xf8fe0000, 0xf8fe0000, 128KB)

// DBAT3 U: 0x10001fff L: 0x1000000a    (0x10000000, 0x10000000, 256MB)

// SR0:  0x00000000  SR1:  0x00000001  SR2:  0x00000002  SR3:  0x00000003
// SR4:  0x00000004  SR5:  0x00000005  SR6:  0x00000006  SR7:  0x00000007
// SR8:  0x00000008  SR9:  0x00000009  SR10: 0x0000000a  SR11: 0x0000000b
// SR12: 0x0000000c  SR13: 0x0000000d  SR14: 0x0000000e  SR15: 0x0000000f


#define RE_STATUS_IDLE			0
#define RE_STATUS_COMMAND		1

void cobra_renderer::display(bitmap_rgb32 *bitmap, const rectangle &cliprect)
{
	copybitmap_trans(*bitmap, *m_framebuffer, 0, 0, 0, 0, cliprect, 0);
}

void cobra_renderer::gfx_init(running_machine &machine)
{
	cobra_state *cobra = machine.driver_data<cobra_state>();

	m_gfx_gram = auto_alloc_array(machine, UINT32, 0x40000);
	
	cobra->m_gfx_register = auto_alloc_array(machine, UINT64, 0x3000);
}

void cobra_renderer::gfx_reset(running_machine &machine)
{
	cobra_state *cobra = machine.driver_data<cobra_state>();

	cobra->m_gfx_re_status = RE_STATUS_IDLE;
}

UINT32 cobra_renderer::gfx_read_gram(UINT32 address)
{
	if (address & 3)
	{
		printf("gfx_read_gram: %08X, not dword aligned!\n", address);
		return 0;
	}

	switch ((address >> 16) & 0xf)
	{
		case 0xc:		// 0xCxxxx
		{
			if ((address >= 0xc0c00 && address < 0xc1000) ||
				(address >= 0xc4c00 && address < 0xc5000) ||
				(address >= 0xc8c00 && address < 0xc9000) ||
				(address >= 0xccc00 && address < 0xcd000))
			{
				UINT32 a = (((address >> 2) & 0xff) * 2) + ((address & 0x4000) ? 1 : 0);
				UINT32 page = ((m_gfx_gram[0xc3028/4] >> 9) * 0x800) +
							  ((address & 0x8000) ? 0x400 : 0) +
							  ((m_gfx_gram[0xc3028/4] & 0x100) ? 0x200 : 0);

				return m_texture_ram[page + a];
			}
			break;
		}
	}

	return m_gfx_gram[address/4];
}

void cobra_renderer::gfx_write_gram(UINT32 address, UINT32 mask, UINT32 data)
{
	if (address & 3)
	{
		printf("gfx_write_gram: %08X, %08X, not dword aligned!\n", address, data);
		return;
	}

	switch (address)
	{
	}

	m_gfx_gram[address/4] &= ~mask;
	m_gfx_gram[address/4] |= data & mask;
}

void cobra_renderer::gfx_fifo_exec(running_machine &machine)
{
	cobra_state *cobra = machine.driver_data<cobra_state>();

	if (cobra->m_gfx_fifo_loopback != 0)
		return;

	const rectangle visarea = machine.primary_screen->visible_area();
	vertex_t vert[8];

	cobra_fifo *fifo_in = cobra->m_gfxfifo_in;
	cobra_fifo *fifo_out = cobra->m_gfxfifo_out;

	while (fifo_in->current_num() >= 2)
	{
		UINT64 in1, in2 = 0;
		UINT32 w1, w2;

		if (cobra->m_gfx_re_status == RE_STATUS_IDLE)
		{
			fifo_in->pop(NULL, &in1);
			fifo_in->pop(NULL, &in2);
			w1 = (UINT32)(in1);
			w2 = (UINT32)(in2);

			cobra->m_gfx_re_command_word1 = w1;
			cobra->m_gfx_re_command_word2 = w2;
			cobra->m_gfx_re_word_count = 0;

			cobra->m_gfx_re_status = RE_STATUS_COMMAND;
		}
		else
		{
			w1 = cobra->m_gfx_re_command_word1;
			w2 = cobra->m_gfx_re_command_word2;
		}


		switch ((w1 >> 24) & 0xff)
		{
			case 0x00:
			{
				UINT64 param[6];
				UINT32 w[6];

				if (fifo_in->current_num() < 6)
				{
					// wait until there's enough data in FIFO
                    memset(param, 0, sizeof(param));
                    memset(w, 0, sizeof(w));
					return;
				}

				fifo_in->pop(NULL, &param[0]);
				fifo_in->pop(NULL, &param[1]);
				fifo_in->pop(NULL, &param[2]);
				fifo_in->pop(NULL, &param[3]);
				fifo_in->pop(NULL, &param[4]);
				fifo_in->pop(NULL, &param[5]);

				w[0] = (UINT32)param[0];	w[1] = (UINT32)param[1];	w[2] = (UINT32)param[2];
				w[3] = (UINT32)param[3];	w[4] = (UINT32)param[4];	w[5] = (UINT32)param[5];

				// mbuslib_pumpkin(): 0x00600000 0x10500010
				//                    0x00600000 0x10500018

				if (w2 == 0x10500010)
				{
					// GFX register select
					cobra->m_gfx_register_select = w[3];

					printf("GFX: register select %04X\n", cobra->m_gfx_register_select);
				}
				else if (w2 == 0x10500018)
				{
					// register write to the register selected above?
					// 64-bit registers, top 32-bits in word 2, low 32-bit in word 3
					printf("GFX: register write %04X: %08X %08X\n", cobra->m_gfx_register_select, w[2], w[3]);

					cobra->m_gfx_register[cobra->m_gfx_register_select] = ((UINT64)(w[2]) << 32) | w[3];
				}
				else if (w2 == 0x10521000)
				{
					printf("gfxfifo_exec: unknown %08X %08X %08X %08X\n", w1, w2, w[0], w[1]);
					printf("                      %08X %08X %08X %08X\n", w[2], w[3], w[4], w[5]);
				}
				else
				{
					logerror("gfxfifo_exec: unknown %08X %08X\n", w1, w2);
				}

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}
			case 0x0f:
			case 0xf0:
			{
				UINT64 in3 = 0, in4 = 0, ignore;

				// check_mergebus_self(): 0x0F600000 0x10520C00

				if (fifo_in->current_num() < 6)
				{
					// wait until there's enough data in FIFO
					return;
				}

				if (w1 != 0x0f600000 && w1 != 0xf0600000)
				{
					logerror("gfxfifo_exec: unknown %08X %08X\n", w1, w2);
				}

				printf("gfxfifo_exec: unhandled %08X %08X\n", w1, w2);

				fifo_in->pop(NULL, &in3);
				fifo_in->pop(NULL, &in4);
				fifo_in->pop(NULL, &ignore);
				fifo_in->pop(NULL, &ignore);
				fifo_in->pop(NULL, &ignore);
				fifo_in->pop(NULL, &ignore);

				if (w1 == 0x0f600000 && w2 == 0x10520c00)
				{
					fifo_out->push(NULL, w1);
					fifo_out->push(NULL, w2);
					fifo_out->push(NULL, in3);
					fifo_out->push(NULL, in4);
				}

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}

			case 0xf1:
			case 0xf4:
			{
				printf("gfxfifo_exec: unhandled %08X %08X\n", w1, w2);

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}

			case 0xe0:
			case 0xe2:
			case 0xe3:
			{
				// Draw graphics primitives

				// 0x00: xxxx---- -------- -------- --------    Command
				// 0x00: ----xxxx -------- -------- --------	Primitive type (0 = triangle, 2 = point, 3 = line)
				// 0x00: -------- xx------ -------- --------	?
				// 0x00: -------- -------- -------- xxxxxxxx	Number of units (amount of bits is uncertain)
				//
				// 0x01: -x------ -------- -------- --------    Has extra flags word (used by lines only)
				// 0x01: --x----- -------- -------- --------    Has extra unknown float
				// 0x01: ---x---- -------- -------- --------    Always 1?
				// 0x01: -------- xx------ -------- --------    ?
				// 0x01: -------- --x----- -------- --------    Has texture coords
				// 0x01: -------- ---x---- -------- --------	?
				// 0x01: -------- ----x--- -------- --------	?
				// 0x01: -------- -------- ------xx xx------	? (always set to 1s?)
				// 0x01: -------- -------- -------- -------x	Has extra unknown float

				int i;
				int num = 0;
				int units = w1 & 0xff;

				// determine the expected packet size to see if we can process it yet
				int unit_size = 8;
				if (w2 & 0x40000000)	unit_size += 1;		// lines only
				if (w2 & 0x20000000)	unit_size += 1;		// unknown float
				if (w2 & 0x00200000)	unit_size += 3;		// texture coords?
				if (w2 & 0x00000001)	unit_size += 1;		// ?

				num = unit_size * units;


				if (fifo_in->current_num() < num)
				{
					// wait until there's enough data in FIFO
					return;
				}


				// extract vertex data
				for (int i=0; i < units; i++)
				{
					UINT64 in;
					if (w2 & 0x40000000)		// line flags
					{
						fifo_in->pop(NULL, &in);
					}
					
					if (w2 & 0x20000000)		// unknown float (0.0f ... 1.0f)
					{
						fifo_in->pop(NULL, &in);
					}
					
					fifo_in->pop(NULL, &vert[i].x);				// X coord
					fifo_in->pop(NULL, &vert[i].y);				// Y coord
					fifo_in->pop(NULL, &in);					// coord?
					fifo_in->pop(NULL, &in);					// coord?

					if (w2 & 0x00200000)		// texture coords
					{
						fifo_in->pop(NULL, &in);				// ? (float 0.0f ... 1.0f)
						fifo_in->pop(NULL, &vert[i].p[0]);		// U coord
						fifo_in->pop(NULL, &vert[i].p[1]);		// V coord
					}

					fifo_in->pop(NULL, &in);					// ? (float 0.0f ... 1.0f)
					fifo_in->pop(NULL, &in);					// ? (float 0.0f ... 1.0f)
					fifo_in->pop(NULL, &in);					// ? (float 0.0f ... 1.0f)
					fifo_in->pop(NULL, &in);					// ? (float 0.0f ... 1.0f)

					if (w2 & 0x00000001)		// unknown float (0.0f ... 1.0f)
					{
						fifo_in->pop(NULL, &in);
					}
				}


				// render
				switch ((w1 >> 24) & 0xf)
				{
					case 0x0:			// triangles
					{
						if (w2 & 0x00200000)
						{
							render_delegate rd = render_delegate(FUNC(cobra_renderer::render_texture_scan), this);
							for (int i=2; i < units; i++)
							{
								render_triangle(visarea, rd, 6, vert[i-2], vert[i-1], vert[i]);
							}
						}
						else
						{
							//render_delegate rd = render_delegate(FUNC(cobra_renderer::render_color_scan), this);
							for (int i=2; i < units; i++)
							{
								//render_triangle(visarea, rd, 6, vert[i-2], vert[i-1], vert[i]);
								draw_point(visarea, vert[i-2], 0xffff0000);
								draw_point(visarea, vert[i-1], 0xffff0000);
								draw_point(visarea, vert[i], 0xffff0000);
							}
						}
						break;
					}

					case 0x2:			// points
					{
						for (int i=0; i < units; i++)
						{
							draw_point(visarea, vert[i], 0xffffffff);
						}
						break;
					}

					case 0x3:			// lines
					{
						if ((units & 1) == 0)		// batches of lines
						{
							for (i=0; i < units; i+=2)
							{
								draw_line(visarea, vert[i], vert[i+1]);
							}
						}
						else						// line strip
						{
							printf("GFX: linestrip %08X, %08X\n", w1, w2);
						}
						break;
					}

					default:
					{
						printf("gfxfifo_exec: unhandled %08X %08X\n", w1, w2);
						break;
					}
				}

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}

			case 0xe8:
			{
				// Write into a pixelbuffer

				int num = w2;
				int i;

				if (fifo_in->current_num() < num)
				{
					// wait until there's enough data in FIFO
					return;
				}

				if (num & 3)
					fatalerror("gfxfifo_exec: e8 with num %d\n", num);

				int x = (m_gfx_gram[0x118/4] >> 16) & 0xffff;
				int y = m_gfx_gram[0x118/4] & 0xffff;

				for (i=0; i < num; i+=4)
				{
					UINT32 *buffer;
					switch (m_gfx_gram[0x80104/4])
					{
						case 0x800000:		buffer = &m_framebuffer->pix32(y); break;
						case 0x200000:		buffer = &m_backbuffer->pix32(y); break;
						case 0x0e0000:		buffer = &m_overlay->pix32(y); break;
						case 0x000800:		buffer = &m_zbuffer->pix32(y); break;
						case 0x000200:		buffer = &m_stencil->pix32(y); break;
						
						default:
						{
							fatalerror("gfxfifo_exec: fb write to buffer %08X!\n", m_gfx_gram[0x80100/4]);
						}
					}

					UINT64 param[4];
					param[0] = param[1] = param[2] = param[3] = 0;
					fifo_in->pop(NULL, &param[0]);
					fifo_in->pop(NULL, &param[1]);
					fifo_in->pop(NULL, &param[2]);
					fifo_in->pop(NULL, &param[3]);

					buffer[x+0] = (UINT32)(param[0]);
					buffer[x+1] = (UINT32)(param[1]);
					buffer[x+2] = (UINT32)(param[2]);
					buffer[x+3] = (UINT32)(param[3]);

					//printf("gfx: fb write %d, %d: %08X %08X %08X %08X\n", x, y, (UINT32)(param[0]), (UINT32)(param[1]), (UINT32)(param[2]), (UINT32)(param[3]));

					y++;
				}

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}

			case 0xe9:
			{
				// Read a specified pixel position from a pixelbuffer

//				printf("GFX: FB read X: %d, Y: %d\n", (UINT16)(m_gfx_gram[0x118/4] >> 16), (UINT16)(m_gfx_gram[0x118/4]));

				int x = (m_gfx_gram[0x118/4] >> 16) & 0xffff;
				int y = m_gfx_gram[0x118/4] & 0xffff;

				UINT32 *buffer;
				switch (m_gfx_gram[0x80104/4])
				{
					case 0x800000:		buffer = &m_framebuffer->pix32(y); break;
					case 0x200000:		buffer = &m_backbuffer->pix32(y); break;
					case 0x0e0000:		buffer = &m_overlay->pix32(y); break;
					case 0x000800:		buffer = &m_zbuffer->pix32(y); break;
					case 0x000200:		buffer = &m_stencil->pix32(y); break;
						
					default:
					{
						fatalerror("gfxfifo_exec: fb read from buffer %08X!\n", m_gfx_gram[0x80100/4]);
					}
				}

				fifo_out->push(NULL, buffer[x+0]);
				fifo_out->push(NULL, buffer[x+1]);
				fifo_out->push(NULL, buffer[x+2]);
				fifo_out->push(NULL, buffer[x+3]);

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}

			case 0x8f:
			{
				// buf_flush(): 0x8FFF0000 0x00000000

				if (w1 != 0x8fff0000 || w2 != 0x00000000)
				{
					logerror("gfxfifo_exec: buf_flush: %08X %08X\n", w1, w2);
				}

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}

			case 0x80:
			case 0xa4:
			case 0xa8:
			case 0xac:
			{
				// 0xA80114CC           prm_flashcolor()
				// 0xA80118CC
				// 0xA80108FF

				// 0xA80108FF           prm_flashmisc()
				// 0xA80110FF
				// 0xA8011CE0

				// 0xA401BCC0           texenvmode()

				// 0xA4019CC0           mode_fog()

				// 0xA40018C0           mode_stipple()
				// 0xA400D080

				// 0xA40138E0           mode_viewclip()

				// 0xA4011410           mode_scissor()

				// 0xA40198A0           mode_alphatest()

				// 0xA8002010           mode_depthtest()

				// 0xA800507C           mode_blend()

				// 0xA8001CFE           mode_stenciltest()

				// 0xA8002010           mode_stencilmod()
				// 0xA80054E0
				// 0xA8001CFE

				// 0xA80118CC           mode_colormask()
				// 0xA80114CC

				// 0xAxxxxxxx is different form in mbuslib_regwrite()

				// mbuslib_regwrite(): 0x800000FF 0x00000001
				//                     0xa40000FF 0x00000001

				int reg = (w1 >> 8) & 0xfffff;
				UINT32 mask = m_gfx_regmask[w1 & 0xff];

				gfx_write_gram(reg, mask, w2);

				if (reg != 0x118 && reg != 0x114 && reg != 0x11c)
				{
					printf("gfxfifo_exec: ram write %05X (mask %08X): %08X (%f)\n", reg, mask, w2, u2f(w2));
				}

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}

			case 0xb0:
			{
				// write multiple registers

				// mbuslib_pip_ints(): 0xB0300800 0x000001FE

				int reg = (w1 >> 8) & 0xfffff;
				int num = w2;
				int i;

				if (fifo_in->current_num() < num)
				{
					return;
				}

				printf("gfxfifo_exec: pip_ints %d\n", num);

				// writes to n ram location starting from x?
				for (i = 0; i < num; i++)
				{
					UINT64 value = 0;
					fifo_in->pop(NULL, &value);

					gfx_write_gram(reg + (i*4), 0xffffffff, value);
				}

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}

			case 0xc0:
			case 0xc4:
			case 0xc8:
			case 0xcc:
			{
				// mbuslib_regread(): 0xC0300800 0x00000000

				// read from register

				int reg = (w1 >> 8) & 0xfffff;

				UINT32 ret = gfx_read_gram(reg);
				fifo_out->push(NULL, ret);

		//		printf("GFX: reg read %08X\n", reg);

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}
			case 0xd0:
			{
				// read multiple registers

				// 0xD0301000 0x000001FC

				int reg = (w1 >> 8) & 0xfffff;
				int num = w2;
				int i;

				if (fifo_out->space_left() < num)
				{
					return;
				}

				// reads back n ram locations starting from x?
				for (i=0; i < num; i++)
				{
					UINT32 value = gfx_read_gram(reg + (i*4));

					fifo_out->push(NULL, value);
				}

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}
			case 0xed:
			{
				// mbuslib_tex_ints()?

				//int reg = (w1 >> 8) & 0xff;
				int num = w2;

				int num_left = num - cobra->m_gfx_re_word_count;

				if (fifo_in->current_num() < num_left)
				{
					num_left = fifo_in->current_num();
				}

				cobra->m_gfx_unk_status |= 0x400;

				if (cobra->m_gfx_re_word_count == 0)
					printf("gfxfifo_exec: tex_ints %d words left\n", num - cobra->m_gfx_re_word_count);

				for (int i=0; i < num_left; i++)
				{
					UINT64 param = 0;
					fifo_in->pop(NULL, &param);
					cobra->m_gfx_re_word_count++;

					UINT32 addr = m_gfx_gram[0xc3020/4];
					m_texture_ram[addr] = (UINT32)(param);
					m_gfx_gram[0xc3020/4]++;
				}


				if (cobra->m_gfx_re_word_count >= num)
				{
					cobra->m_gfx_re_status = RE_STATUS_IDLE;
				}
				break;
			}
			default:
			{
				int k = 0;
				int c = 0;
				printf("gfxfifo_exec: unknown command %08X %08X\n", w1, w2);

				if (fifo_in->current_num() < 0)
				{
					return;
				}

				while (fifo_in->current_num() > 0)
				{
					UINT64 param;
					fifo_in->pop(NULL, &param);

					if (c == 0)
						printf("              ");
					printf("%08X ", (UINT32)(param));

					c++;

					if (c == 4)
					{
						printf("\n");
						c = 0;
					}
					k++;
				};
				logerror("\n");
			}
		}

//      printf("gfxfifo_exec: %08X %08X\n", w1, w2);
	};

	wait();
}

READ64_MEMBER(cobra_state::gfx_fifo_r)
{
	UINT64 r = 0;

	m_renderer->gfx_fifo_exec(space.machine());

	if (ACCESSING_BITS_32_63)
	{
		UINT64 data;
		m_gfxfifo_out->pop(&space.device(), &data);

		data &= 0xffffffff;

		r |= (UINT64)(data) << 32;
	}
	if (ACCESSING_BITS_0_31)
	{
		UINT64 data;
		m_gfxfifo_out->pop(&space.device(), &data);

		data &= 0xffffffff;

		r |= (UINT64)(data);
	}
//  printf("GFX FIFO read %08X%08X\n", (UINT32)(r >> 32), (UINT32)(r));

	return r;
}

WRITE64_MEMBER(cobra_state::gfx_fifo0_w)
{
	m_gfx_fifo_cache_addr = 2;
	COMBINE_DATA(m_gfx_fifo_mem + offset);
}

WRITE64_MEMBER(cobra_state::gfx_fifo1_w)
{
	m_gfx_fifo_cache_addr = 0;
	COMBINE_DATA(m_gfx_fifo_mem + offset);
}

WRITE64_MEMBER(cobra_state::gfx_fifo2_w)
{
	m_gfx_fifo_cache_addr = 1;
	COMBINE_DATA(m_gfx_fifo_mem + offset);
}

READ64_MEMBER(cobra_state::gfx_unk1_r)
{
	UINT64 r = 0;

	if (ACCESSING_BITS_56_63)
	{
		UINT64 v = 0;
		// mbuslib_init fails if bits 3-7 (0x78) are not set

		v |= 0x78;

		// the low 2 bits are vblank flags
		// bit 3 (0x8) may be graphics engine idle flag

		v |= m_gfx_status_byte;
		m_gfx_status_byte ^= 1;

		r |= v << 56;
	}
	if (ACCESSING_BITS_40_47)
	{
		// mbuslib_init fails if this is not 0x7f

		r |= (UINT64) 0x7f << 40;
	}
	if (ACCESSING_BITS_24_31)			// this register returns FIFO number during check_fifo (see below)
	{
		r |= (m_gfx_unknown_v1 & 3) << 24;
	}

	return r;
}

WRITE64_MEMBER(cobra_state::gfx_unk1_w)
{
//  printf("gfx_unk1_w: %08X %08X, %08X%08X\n", (UINT32)(data >> 32), (UINT32)(data), (UINT32)(mem_mask >> 32), (UINT32)(mem_mask));

	if (ACCESSING_BITS_56_63)
	{
		if ((data >> 63) & 1)
		{
			m_gfx_fifo_loopback = 0;
		}
	}

	if (ACCESSING_BITS_24_31)
	{
		UINT64 in1, in2;
		int value = (data >> 24) & 0xff;
		// used in check_fifo(). fifo loopback or something?

		if (value == 0xc0)
		{
			m_gfxfifo_in->pop(&space.device(), &in1);
			m_gfxfifo_in->pop(&space.device(), &in2);
			m_gfx_unknown_v1 = (UINT32)(in1 >> 32);			// FIFO number is read back from this same register

			m_gfxfifo_out->push(&space.device(), in1 & 0xffffffff);
			m_gfxfifo_out->push(&space.device(), in2 & 0xffffffff);
		}
		else if (value == 0x80)
		{
			// used in check_fifo() before the fifo test...
			m_gfx_fifo_loopback = 1;
		}
		else
		{
			printf("gfx_unk1_w: unknown value %02X\n", value);
		}
	}
}

WRITE64_MEMBER(cobra_state::gfx_buf_w)
{
//  printf("buf_w: top = %08X\n", gfxfifo_get_top());

	// buf_prc_read: 0x00A00001 0x10520200
	//               0x00A00001 0x10500018

	// teximage_load() / mbuslib_prc_read():    0x00A00001 0x10520800

//  printf("prc_read %08X%08X at %08X\n", (UINT32)(data >> 32), (UINT32)(data), activecpu_get_pc());

	m_renderer->gfx_fifo_exec(space.machine());

	if (data == U64(0x00a0000110500018))
	{
		m_gfxfifo_out->flush();

		// reads back the register selected by gfx register select

		m_gfxfifo_out->push(&space.device(), (UINT32)((m_gfx_register[m_gfx_register_select] >> 32)));
		m_gfxfifo_out->push(&space.device(), (UINT32)(m_gfx_register[m_gfx_register_select]));
	}
	else if (data == U64(0x00a0000110520800))
	{
		// in teximage_load()
		// some kind of busy flag for mbuslib_tex_ints()...

		// mbuslib_tex_ints() waits for bit 0x400 to be set
		// memcheck_teximage() wants 0x400 cleared

		m_gfxfifo_out->push(&space.device(), m_gfx_unk_status);

		m_gfx_unk_status &= ~0x400;
	}
	else if (data != U64(0x00a0000110520200))		// mbuslib_regread()
	{
		// prc_read always expects a value...

		m_gfxfifo_out->push(&space.device(), 0);
	}
}

static void gfx_cpu_dc_store(device_t *device, UINT32 address)
{
	cobra_state *cobra = device->machine().driver_data<cobra_state>();

	if (address == 0x10000000 || address == 0x18000000 || address == 0x1e000000)
	{
		UINT64 i = (UINT64)(cobra->m_gfx_fifo_cache_addr) << 32;
		cobra_fifo *fifo_in = cobra->m_gfxfifo_in;

		fifo_in->push(device, (UINT32)(cobra->m_gfx_fifo_mem[0] >> 32) | i);
		fifo_in->push(device, (UINT32)(cobra->m_gfx_fifo_mem[0] >>  0) | i);
		fifo_in->push(device, (UINT32)(cobra->m_gfx_fifo_mem[1] >> 32) | i);
		fifo_in->push(device, (UINT32)(cobra->m_gfx_fifo_mem[1] >>  0) | i);
		fifo_in->push(device, (UINT32)(cobra->m_gfx_fifo_mem[2] >> 32) | i);
		fifo_in->push(device, (UINT32)(cobra->m_gfx_fifo_mem[2] >>  0) | i);
		fifo_in->push(device, (UINT32)(cobra->m_gfx_fifo_mem[3] >> 32) | i);
		fifo_in->push(device, (UINT32)(cobra->m_gfx_fifo_mem[3] >>  0) | i);

		cobra->m_renderer->gfx_fifo_exec(device->machine());
	}
	else
	{
		logerror("gfx: data cache store at %08X\n", address);
	}
}

WRITE64_MEMBER(cobra_state::gfx_debug_state_w)
{
	if (ACCESSING_BITS_40_47)
	{
		m_gfx_unk_flag = (UINT8)(data >> 40);
	}

	if (ACCESSING_BITS_56_63)
	{
		m_gfx_debug_state |= decode_debug_state_value((data >> 56) & 0xff) << 4;
		m_gfx_debug_state_wc++;
	}
	if (ACCESSING_BITS_48_55)
	{
		m_gfx_debug_state |= decode_debug_state_value((data >> 48) & 0xff);
		m_gfx_debug_state_wc++;
	}

	if (m_gfx_debug_state_wc >= 2)
	{
		if (m_gfx_debug_state != 0)
		{
			printf("GFX: debug state %02X\n", m_gfx_debug_state);
		}

		m_gfx_debug_state = 0;
		m_gfx_debug_state_wc = 0;
	}
}

static ADDRESS_MAP_START( cobra_gfx_map, AS_PROGRAM, 64, cobra_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("gfx_main_ram_0")
	AM_RANGE(0x07c00000, 0x07ffffff) AM_RAM AM_SHARE("gfx_main_ram_1")
	AM_RANGE(0x10000000, 0x1000001f) AM_WRITE(gfx_fifo0_w)
	AM_RANGE(0x18000000, 0x1800001f) AM_WRITE(gfx_fifo1_w)
	AM_RANGE(0x1e000000, 0x1e00001f) AM_WRITE(gfx_fifo2_w)
	AM_RANGE(0x20000000, 0x20000007) AM_WRITE(gfx_buf_w)							// this might really map to 0x1e000000, depending on the pagetable
	AM_RANGE(0x7f000000, 0x7f00ffff) AM_RAM AM_SHARE("pagetable")
	AM_RANGE(0xfff00000, 0xfff7ffff) AM_ROM AM_REGION("user3", 0)					/* Boot ROM */
	AM_RANGE(0xfff80000, 0xfff80007) AM_WRITE(gfx_debug_state_w)
	AM_RANGE(0xffff0000, 0xffff0007) AM_READWRITE(gfx_unk1_r, gfx_unk1_w)
	AM_RANGE(0xffff0010, 0xffff001f) AM_READ(gfx_fifo_r)
ADDRESS_MAP_END


/*****************************************************************************/

INPUT_PORTS_START( cobra )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

INPUT_PORTS_END

static powerpc_config main_ppc_cfg =
{
    XTAL_66_6667MHz,		/* Multiplier 1.5, Bus = 66MHz, Core = 100MHz */
    NULL,
    NULL
};

static powerpc_config gfx_ppc_cfg =
{
    XTAL_66_6667MHz,		/* Multiplier 1.5, Bus = 66MHz, Core = 100MHz */
	NULL,
    NULL
};


static void ide_interrupt(device_t *device, int state)
{
	cobra_state *cobra = device->machine().driver_data<cobra_state>();

	if (state == CLEAR_LINE)
	{
		cobra->m_sub_interrupt |= 0x80;
	}
	else
	{
		cobra->m_sub_interrupt &= ~0x80;
	}
}


static INTERRUPT_GEN( cobra_vblank )
{
	///cpunum_set_input_line(MAIN_CPU_ID, INPUT_LINE_IRQ0, ASSERT_LINE);
}


static MACHINE_RESET( cobra )
{
	cobra_state *cobra = machine.driver_data<cobra_state>();

	cobra->m_sub_interrupt = 0xff;

	UINT8 *ide_features = ide_get_features(machine.device("ide"), 0);

	// Cobra expects these settings or the BIOS fails
	ide_features[51*2+0] = 0;			/* 51: PIO data transfer cycle timing mode */
	ide_features[51*2+1] = 2;
	ide_features[67*2+0] = 0xe0;		/* 67: minimum PIO transfer cycle time without flow control */
	ide_features[67*2+1] = 0x01;

	cobra->m_renderer->gfx_reset(machine);
}

static MACHINE_CONFIG_START( cobra, cobra_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC603, 100000000)		/* 603EV, 100? MHz */
	MCFG_CPU_CONFIG(main_ppc_cfg)
	MCFG_CPU_PROGRAM_MAP(cobra_main_map)
	MCFG_CPU_VBLANK_INT("screen", cobra_vblank)

	MCFG_CPU_ADD("subcpu", PPC403GA, 32000000)		/* 403GA, 33? MHz */
	MCFG_CPU_PROGRAM_MAP(cobra_sub_map)

	MCFG_CPU_ADD("gfxcpu", PPC604, 100000000)		/* 604, 100? MHz */
	MCFG_CPU_CONFIG(gfx_ppc_cfg)
	MCFG_CPU_PROGRAM_MAP(cobra_gfx_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(10000))

	MCFG_MACHINE_RESET( cobra )

	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, NULL, mpc106_pci_r, mpc106_pci_w)

	MCFG_IDE_CONTROLLER_ADD("ide", ide_interrupt, ide_devices, "hdd", NULL, true)

	/* video hardware */
	MCFG_VIDEO_START(cobra)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 384)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 383)
	MCFG_PALETTE_LENGTH(65536)
	MCFG_SCREEN_UPDATE_STATIC(cobra)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("rfsnd", RF5C400, XTAL_16_9344MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_M48T58_ADD( "m48t58" )

MACHINE_CONFIG_END

/*****************************************************************************/

/*****************************************************************************/

static DRIVER_INIT(cobra)
{
	cobra_state *cobra = machine.driver_data<cobra_state>();

	cobra->m_gfxfifo_in	 = auto_alloc(machine, cobra_fifo(machine, 8192, "GFXFIFO_IN", GFXFIFO_IN_VERBOSE != 0));
	cobra->m_gfxfifo_out = auto_alloc(machine, cobra_fifo(machine, 8192, "GFXFIFO_IN", GFXFIFO_OUT_VERBOSE != 0));
	cobra->m_m2sfifo     = auto_alloc(machine, cobra_fifo(machine, 2048, "M2SFIFO", M2SFIFO_VERBOSE != 0));
	cobra->m_s2mfifo     = auto_alloc(machine, cobra_fifo(machine, 2048, "S2MFIFO", S2MFIFO_VERBOSE != 0));


	ppc_set_dcstore_callback(cobra->m_gfxcpu, gfx_cpu_dc_store);


	ppc4xx_set_dma_read_handler(cobra->m_subcpu, 0, sub_unknown_dma_r);
	ppc4xx_set_dma_write_handler(cobra->m_subcpu, 0, sub_unknown_dma_w);



	cobra->m_comram[0] = auto_alloc_array(machine, UINT32, 0x40000/4);
	cobra->m_comram[1] = auto_alloc_array(machine, UINT32, 0x40000/4);

	cobra->m_comram_page = 0;


	// setup fake pagetable until we figure out what really maps there...
	//cobra->m_gfx_pagetable[0x80 / 8] = U64(0x800001001e0001a8);
	cobra->m_gfx_pagetable[0x80 / 8] = U64(0x80000100200001a8);		// should this map to 0x1e000000?
}

static DRIVER_INIT(bujutsu)
{
	DRIVER_INIT_CALL(cobra);

	// rom hacks for sub board...
	{
		UINT32 *rom = (UINT32*)machine.root_device().memregion("user2")->base();

		rom[0x62094 / 4] = 0x60000000;			// skip hardcheck()...
	}


	// rom hacks for gfx board...
	{
		int i;
		UINT32 sum = 0;

		UINT32 *rom = (UINT32*)machine.root_device().memregion("user3")->base();

		rom[(0x022d4^4) / 4] = 0x60000000;		// skip init_raster() for now ...

		// calculate the checksum of the patched rom...
		for (i=0; i < 0x20000/4; i++)
		{
			sum += (UINT8)((rom[i] >> 24) & 0xff);
			sum += (UINT8)((rom[i] >> 16) & 0xff);
			sum += (UINT8)((rom[i] >>  8) & 0xff);
			sum += (UINT8)((rom[i] >>  0) & 0xff);
		}

		rom[(0x0001fff0^4) / 4] = sum;
		rom[(0x0001fff4^4) / 4] = ~sum;
	}



	// fill in M48T58 data for now...
	{
		UINT8 *rom = (UINT8*)machine.root_device().memregion("m48t58")->base();
		rom[0x00] = 0x47;		// G
		rom[0x02] = 0x36;		// 6
		rom[0x03] = 0x34;		// 4
		rom[0x04] = 0x35;		// 5
		rom[0x05] = 0x00;
		rom[0x06] = 0x00;
		rom[0x07] = 0x00;

		// calculate checksum
		UINT16 sum = 0;
		for (int i=0; i < 14; i+=2)
		{
			sum += ((UINT16)(rom[i]) << 8) | (rom[i+1]);
		}
		sum ^= 0xffff;

		rom[0x0e] = (UINT8)(sum >> 8);
		rom[0x0f] = (UINT8)(sum);
	}
}

static DRIVER_INIT(racjamdx)
{
	DRIVER_INIT_CALL(cobra);

	// rom hacks for sub board...
	{
		UINT32 *rom = (UINT32*)machine.root_device().memregion("user2")->base();

		rom[0x62094 / 4] = 0x60000000;			// skip hardcheck()...
		rom[0x62ddc / 4] = 0x60000000;			// skip lanc_hardcheck()


		// calculate the checksum of the patched rom...
		UINT32 sum = 0;
		for (int i=0; i < 0x20000/4; i++)
		{
			sum += (UINT8)((rom[(0x60000/4)+i] >> 24) & 0xff);
			sum += (UINT8)((rom[(0x60000/4)+i] >> 16) & 0xff);
			sum += (UINT8)((rom[(0x60000/4)+i] >>  8) & 0xff);
			sum += (UINT8)((rom[(0x60000/4)+i] >>  0) & 0xff);
		}

		rom[(0x0007fff0^4) / 4] = ~sum;
		rom[(0x0007fff4^4) / 4] = sum;
	}


	// rom hacks for gfx board...
	{
		int i;
		UINT32 sum = 0;

		UINT32 *rom = (UINT32*)machine.root_device().memregion("user3")->base();

		rom[(0x02448^4) / 4] = 0x60000000;		// skip init_raster() for now ...

		rom[(0x02438^4) / 4] = 0x60000000;		// awfully long delay loop (5000000 * 166)

		// calculate the checksum of the patched rom...
		for (i=0; i < 0x20000/4; i++)
		{
			sum += (UINT8)((rom[i] >> 24) & 0xff);
			sum += (UINT8)((rom[i] >> 16) & 0xff);
			sum += (UINT8)((rom[i] >>  8) & 0xff);
			sum += (UINT8)((rom[i] >>  0) & 0xff);
		}

		rom[(0x0001fff0^4) / 4] = sum;
		rom[(0x0001fff4^4) / 4] = ~sum;
	}


	// fill in M48T58 data for now...
	{
		UINT8 *rom = (UINT8*)machine.root_device().memregion("m48t58")->base();
		rom[0x00] = 0x47;		// G
		rom[0x01] = 0x59;		// Y
		rom[0x02] = 0x36;		// 6
		rom[0x03] = 0x37;		// 7
		rom[0x04] = 0x36;		// 6
		rom[0x05] = 0x00;
		rom[0x06] = 0x00;
		rom[0x07] = 0x00;

		// calculate checksum
		UINT16 sum = 0;
		for (int i=0; i < 14; i+=2)
		{
			sum += ((UINT16)(rom[i]) << 8) | (rom[i+1]);
		}
		sum ^= 0xffff;

		rom[0x0e] = (UINT8)(sum >> 8);
		rom[0x0f] = (UINT8)(sum);
	}
}

/*****************************************************************************/

ROM_START(bujutsu)
	ROM_REGION64_BE(0x80000, "user1", 0)		/* Main CPU program (PPC603) */
	ROM_LOAD("645a01.33d", 0x00000, 0x80000, CRC(cb1a8683) SHA1(77b7dece84dc17e9d63242347b7202e879b9a10e) )

	ROM_REGION32_BE(0x80000, "user2", 0)		/* Sub CPU program (PPC403) */
	ROM_LOAD("645a02.24r", 0x00000, 0x80000, CRC(7d1c31bd) SHA1(94907c4068a488a74b2fa9a486c832d380c5b184) )

	ROM_REGION64_BE(0x80000, "user3", 0)		/* Gfx CPU program (PPC604) */
	ROM_LOAD("645a03.u17", 0x00000, 0x80000, CRC(086abd0b) SHA1(24df439eb9828ed3842f43f5f4014a3fc746e1e3) )

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)
	ROM_LOAD( "m48t58-70pc1.17l", 0x000000, 0x002000, NO_DUMP )

	DISK_REGION( "drive_0" )
	DISK_IMAGE_READONLY( "645c04", 0, SHA1(c0aabe69f6eb4e4cf748d606ae50674297af6a04) )
ROM_END

ROM_START(racjamdx)
	ROM_REGION64_BE(0x80000, "user1", 0)		/* Main CPU program (PPC603) */
	ROM_LOAD( "676a01.33d", 0x000000, 0x080000, CRC(1e6238f1) SHA1(d55949d98e9e290ceb8c018ed60ca090ec16c9dd) )

	ROM_REGION32_BE(0x80000, "user2", 0)		/* Sub CPU program (PPC403) */
	ROM_LOAD( "676a02.24r", 0x000000, 0x080000, CRC(371978ed) SHA1(c83f0cf04204212db00588df91b32122f37900f8) )

	ROM_REGION64_BE(0x80000, "user3", 0)		/* Gfx CPU program (PPC604) */
	ROM_LOAD( "676a03.u17", 0x000000, 0x080000, CRC(66f77cbd) SHA1(f1c7e50dbbfcc27ac011cbbb8ad2fd376c2e9056) )

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)
	ROM_LOAD( "m48t58-70pc1.17l", 0x000000, 0x002000, NO_DUMP )

	DISK_REGION( "drive_0" )
	DISK_IMAGE_READONLY( "676a04", 0, SHA1(8e89d3e5099e871b99fccba13adaa3cf8a6b71f0) )
ROM_END

/*************************************************************************/

GAME( 1997, bujutsu, 0, cobra, cobra, cobra_state, bujutsu, ROT0, "Konami", "Fighting Bujutsu", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 1997, racjamdx, 0, cobra, cobra, cobra_state, racjamdx, ROT0, "Konami", "Racing Jam DX", GAME_NOT_WORKING | GAME_NO_SOUND )

