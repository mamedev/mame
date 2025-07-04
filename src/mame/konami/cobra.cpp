// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*  Konami Cobra System

    Driver by Ville Linde


    Games on this hardware
    ----------------------

    Game                                     ID        Year    Notes
    -----------------------------------------------------------------------
    Fighting Bujutsu / Fighting Wu-Shu     | GN645   | 1997  |
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



    GFX Registers:

        0x00090:        Viewport width / 2?
        0x0009c:        Viewport center X
        0x000a4:        Viewport height / 2?
        0x000ac:        Viewport center Y

        0x00114:        xxxxxxxx xxxxxxxx -------- --------             Framebuffer pixel read line count
                        -------- -------- xxxxxxxx xxxxxxxx             Framebuffer pixel read pixel count

        0x00118:        xxxxxxxx xxxxxxxx -------- --------             Framebuffer pixel read X pos
                        -------- -------- xxxxxxxx xxxxxxxx             Framebuffer pixel read Y pos

        0x0011c:        Same as above?

        0x00454:        (mask 0xff) 0x80000000                          Tex related

        0x00458:        Set to 0x02100000 (0xff) by texselect()

                        ------xx -------- -------- --------             Texture select (0-3)
                        -------- ---x---- -------- --------             ?

        0x02900:        -------- -------- -------- --------             Texture[0] ?

        0x02904:        -------- ------xx xx------ --------             Texture[0] mag filter?
                        -------- -------- --xxxx-- --------             Texture[0] min filter?

        0x02908:        -------- ----xxx- -------- --------             Texture[0] wrap S (0 = repeat, 1 = mirror, 2 = clamp)
                        -------- -------x xx------ --------             Texture[0] wrap T

        0x02910:        xxxx---- -------- -------- --------             Texture[0] width shift (size = 1 << X)
                        ----xxxx -------- -------- --------             Texture[0] height shift (size = 1 << X)
                        -------- ----x--- -------- --------             ?
                        -------- -------- -x------ --------             ?
                        -------- -------- -------- xxx-----             Texture[0] format (texel size, 2 = 4-bit, 3 = 8-bit, 4 = 16-bit)
                        -------- -------- -------- ---xxx--             Texture[0] format param

        0x02914:        xxxxxxxx xxxxxxxx xxxx---- --------             Texture[0] address

        0x02980:        Texture[1] ?
        0x02984:        Texture[1] min/mag filter
        0x02988:        Texture[1] wrap
        0x02990:        Texture[1] width/height/format
        0x02994:        Texture[1] address

        0x02a00:        Texture[2] ?
        0x02a04:        Texture[2] min/mag filter
        0x02a08:        Texture[2] wrap
        0x02a10:        Texture[2] width/height/format
        0x02a14:        Texture[2] address

        0x02a80:        Texture[3] ?
        0x02a84:        Texture[3] min/mag filter
        0x02a88:        Texture[3] wrap
        0x02a90:        Texture[3] width/height/format
        0x02a94:        Texture[3] address

        0x40018:        Set to 0x0001040a (0xc0) by mode_stipple()      (bits 24..27 = stipple pattern?)
        0x400d0:        Set to 0x80000000 (0x80) by mode_stipple()

        0x400f4:        xxx----- -------- -------- --------             Texture select (0-3)

        0x40114:        -------- ----x--- -------- --------             Scissor enable

        0x40138:        Set to 0x88800000 (0xe0) by mode_viewclip()

        0x40160:        xxxxxxxx xxxxxxxx -------- --------             Scissor Left
                        -------- -------- xxxxxxxx xxxxxxxx             Scissor Top

        0x40164:        xxxxxxxx xxxxxxxx -------- --------             Scissor Right
                        -------- -------- xxxxxxxx xxxxxxxx             Scissor Bottom

        0x40170:        xxxxxxxx xxxxxxxx -------- --------             Viewport Left
                        -------- -------- xxxxxxxx xxxxxxxx             Viewport Top

        0x40174:        xxxxxxxx xxxxxxxx -------- --------             Viewport Right
                        -------- -------- xxxxxxxx xxxxxxxx             Viewport Bottom

        0x40198:        x------- -------- -------- --------             Alpha test enable?
                        -------- xxx----- -------- --------             Alpha test function (0 = never, 1 = less, 2 = lequal, 3 = greater,
                                                                                             4 = gequal, 5 = equal, 6 = notequal, 7 = always)
                        -------- -------- xxxxxxxx xxxxxxxx             Alpha test reference value?

        0x4019c:        x------- -------- -------- --------             Fog enable
                        ----x--- -------- -------- --------             0 = table fog, 1 = linear fog

        0x401a8:        (mask 0xff): 0x2CAB34FD                         ?
        0x401ac:        (mask 0xf0): 0x48C70000                         ?
        0x401b8:        (mask 0x20): 0x00400000                         ?

        0x401bc:                                                        Texture env mode
                        xxx----- -------- -------- --------             ?
                        ---xxx-- -------- -------- --------             ?

        0x8001c:        (mask 0xfe) 0xc1d60060 = (not equal, 6)         Stencil register
                                    0xca9b0010 = (not equal, 1)
                                    0x037dfff0 = (never)
                                    0x2616fff0 = (greater)
                                    0x459cfff0 = (greater/equal)
                                    0x6672fff0 = (less)
                                    0x8827fff0 = (less/equal)
                                    0xa92b0100 = (equal, 16)

                        -------- -------- xxxxxxxx xxxx----             Stencil reference value?
                        ----xxxx xxxxxxxx -------- --------             Stencil fill value?
                        xxx----- -------- -------- --------             Stencil function?


        0x80020:        -------- ----xxx- -------- --------             Depth test function (7 = always?)

        0x80040:        (mask 0x0f) 0x00002CAB (same value as 0x401a8)
        0x80044:        (mask 0x0f) 0x000034FD (same value as 0x401a8)
        0x80048:        (mask 0x0f) 0x000048C7 (same value as 0x401ac)

        0x80050:        (mask 0x7c) 0x04445500 = (As, 1-As)             Blend register
                                    0x04111100 = (1, 1)
                                    0x04441100 = (As, 1)
                                    0x04114400 = (1, As)
                                    0x04681100 = (Cd, 1)
                                    0x04680000 = (Cd, 0)
                                    0x04002400 = (0, Cs)
                                    0x04e11100 = (Ad, 1-Ad)
                                    0x04889900 = (pseudo-fog)

                        -----x-- -------- -------- --------             Blend enable?
                        -------- xxxx---- -------- --------             source factor?
                        -------- ----xxxx -------- --------             source?
                        -------- -------- xxxx---- --------             dest factor?
                        -------- -------- ----xxxx --------             dest?


        0x80054:        Set to 0x02400000 (0xe0) by mode_stencilmod()

        0x800a4:        x------- -------- -------- --------             Logic op enable?
                        -----xxx x------- -------- --------             Logic op (0 = clear, 1 = and, 2 = and reverse, 3 = copy,
                                                                                  4 = and inverted, 5 = noop, 6 = xor, 7 = or inverted,
                                                                                  8 = nor, 9 = equiv, 10 = invert, 11 = or reverse,
                                                                                  12 = copy inverted, 13 = or inverted, 14 = nand, 15 = set)

        0x80114:        (0xcc) by mode_colormask()
        0x80118:        (0xcc) by mode_colormask()

        0x8011c:        xxxxxxxx xxxx---- -------- --------             Stencil mask

        0x80120:        -------- -------- xxxxxxxx xxxxxxxx             Depth mask register

        0xc0c00..fff:   Texture RAM readback
        0xc3020:        Start address for texram writes?
        0xc3028:        Reads address from texram?
        0xc4c00..fff:   Texture RAM readback
        0xc8c00..fff:   Texture RAM readback
        0xccc00..fff:   Texture RAM readback



        Bujutsu status:

        Main:
             0xe948 ->  0x1ea4(): Waiting for [0x168700] != 1  @ 0x1ebc

        Gfx:
            0x3b1a0 -> 0x3ad70(): Waiting for [0x132000] != 0  @ 0x3adf4

*/


#include "emu.h"

#include "k001604.h"
#include "konppc_jvshost.h"
#include "windy2.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/hdd.h"
#include "cpu/powerpc/ppc.h"
#include "machine/lpci.h"
#include "machine/timekpr.h"
#include "sound/dmadac.h"
#include "sound/rf5c400.h"
#include "video/poly.h"
#include "video/rgbutil.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_DEBUG_STATES   (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

#define LOG_GFX_RAM_WRITES (0)
#define LOG_DRAW_COMMANDS  (0)

namespace {

#define GFXFIFO_IN_VERBOSE          0
#define GFXFIFO_OUT_VERBOSE         0
#define M2SFIFO_VERBOSE             0
#define S2MFIFO_VERBOSE             0

#define ENABLE_BILINEAR             1

#define DMA_SOUND_BUFFER_SIZE       16000


/* Cobra Renderer class */

struct cobra_polydata
{
	uint32_t alpha_test;
	uint32_t zmode;
	uint32_t tex_format;
	uint32_t tex_address;
};

class cobra_renderer : public poly_manager<float, cobra_polydata, 8>
{
public:
	cobra_renderer(screen_device &screen)
		: poly_manager<float, cobra_polydata, 8>(screen.machine())
		, m_screen(screen)
		, m_framebuffer_size(0, 511, 0, 399)
	{
		m_texture_ram = std::make_unique<uint32_t[]>(0x100000);

		m_framebuffer = std::make_unique<bitmap_rgb32>( 1024, 1024);
		m_backbuffer = std::make_unique<bitmap_rgb32>( 1024, 1024);
		m_overlay = std::make_unique<bitmap_rgb32>( 1024, 1024);
		m_zbuffer = std::make_unique<bitmap_ind32>(1024, 1024);
		m_stencil = std::make_unique<bitmap_ind32>(1024, 1024);

		m_gfx_regmask = std::make_unique<uint32_t[]>(0x100);
		for (int i=0; i < 0x100; i++)
		{
			uint32_t mask = 0;
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

	screen_device &screen() const { return m_screen; }
	void render_texture_scan(int32_t scanline, const extent_t &extent, const cobra_polydata &extradata, int threadid);
	void render_color_scan(int32_t scanline, const extent_t &extent, const cobra_polydata &extradata, int threadid);
	void draw_point(const rectangle &visarea, vertex_t &v, uint32_t color);
	void draw_line(const rectangle &visarea, vertex_t &v1, vertex_t &v2);

	void gfx_init();
	void gfx_exit();
	void gfx_reset();
	void gfx_fifo_exec();
	uint32_t gfx_read_gram(uint32_t address);
	void gfx_write_gram(uint32_t address, uint32_t mask, uint32_t data);
	uint64_t gfx_read_reg();
	void gfx_write_reg(uint64_t data);

	void display(bitmap_rgb32 *bitmap, const rectangle &cliprect);
	inline rgb_t texture_fetch(uint32_t *texture, int u, int v, int width, int format);
private:
	screen_device &m_screen;
	std::unique_ptr<bitmap_rgb32> m_framebuffer;
	std::unique_ptr<bitmap_rgb32> m_backbuffer;
	std::unique_ptr<bitmap_rgb32> m_overlay;
	std::unique_ptr<bitmap_ind32> m_zbuffer;
	std::unique_ptr<bitmap_ind32> m_stencil;

	std::unique_ptr<uint32_t[]> m_texture_ram;

	std::unique_ptr<uint32_t[]> m_gfx_gram;
	std::unique_ptr<uint32_t[]> m_gfx_regmask;

	uint32_t m_gfx_register_select;
	std::unique_ptr<uint64_t[]> m_gfx_register;

	uint32_t m_texram_ptr;

	rectangle m_framebuffer_size;

	enum
	{
		RE_STATUS_IDLE              = 0,
		RE_STATUS_COMMAND           = 1
	};

	enum
	{
		POLY_Z      = 0,
		POLY_R      = 1,
		POLY_G      = 2,
		POLY_B      = 3,
		POLY_A      = 4,
		POLY_U      = 5,
		POLY_V      = 6,
		POLY_W      = 7
	};
};


/* FIFO class */
class cobra_fifo
{
public:
	enum EventType
	{
		EVENT_EMPTY,
		EVENT_HALF_FULL,
		EVENT_FULL
	};

	typedef delegate<void (EventType)> event_delegate;

	cobra_fifo(running_machine &machine, int capacity, const char *name, bool verbose, event_delegate event_callback)
	{
		m_data = std::make_unique<uint64_t[]>(capacity);

		m_name = name;
		m_size = capacity;
		m_wpos = 0;
		m_rpos = 0;
		m_num = 0;

		m_verbose = verbose;

		m_event_callback = event_callback;
	}

	void push(const device_t *cpu, uint64_t data);
	bool pop(const device_t *cpu, uint64_t *result);
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
	std::unique_ptr<uint64_t[]> m_data;
	event_delegate m_event_callback;
};

/* Cobra driver class */

class cobra_state : public driver_device
{
public:
	cobra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_gfxcpu(*this, "gfxcpu"),
		m_gfx_pagetable(*this, "pagetable"),
		m_k001604(*this, "k001604"),
		m_ata(*this, "ata"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_legacy_pci(*this, "pcibus"),
		m_jvs_host(*this, "jvs_host"),
		m_dmadac(*this, "dac%u", 1U),
		m_generic_paletteram_32(*this, "paletteram"),
		m_main_ram(*this, "main_ram"),
		m_sub_ram(*this, "sub_ram"),
		m_gfx_ram0(*this, "gfx_main_ram_0"),
		m_gfx_ram1(*this, "gfx_main_ram_1")
	{
	}

	required_device<ppc_device> m_maincpu;
	required_device<ppc4xx_device> m_subcpu;
	required_device<ppc_device> m_gfxcpu;
	required_shared_ptr<uint64_t> m_gfx_pagetable;
	required_device<k001604_device> m_k001604;
	required_device<ata_interface_device> m_ata;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<pci_bus_legacy_device> m_legacy_pci;
	required_device<konppc_jvs_host_device> m_jvs_host;
	required_device_array<dmadac_sound_device, 2> m_dmadac;
	required_shared_ptr<uint32_t> m_generic_paletteram_32;
	required_shared_ptr<uint64_t> m_main_ram;
	required_shared_ptr<uint32_t> m_sub_ram;
	required_shared_ptr<uint64_t> m_gfx_ram0;
	required_shared_ptr<uint64_t> m_gfx_ram1;

	uint64_t main_comram_r(offs_t offset, uint64_t mem_mask = ~0);
	void main_comram_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t main_fifo_r(offs_t offset, uint64_t mem_mask = ~0);
	void main_fifo_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void main_cpu_dc_store(offs_t offset, uint32_t data);

	uint32_t sub_comram_r(offs_t offset);
	void sub_comram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t sub_unk7e_r(offs_t offset);
	void sub_debug_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sub_unk1_r(offs_t offset, uint32_t mem_mask = ~0);
	void sub_unk1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sub_config_r(offs_t offset, uint32_t mem_mask = ~0);
	void sub_config_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sub_mainbd_r(offs_t offset, uint32_t mem_mask = ~0);
	void sub_mainbd_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sub_psac2_r();
	void sub_psac2_w(uint32_t data);
	void sub_psac_palette_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void sub_sound_dma_w(offs_t offset, uint32_t data);

	void gfx_fifo0_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void gfx_fifo1_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void gfx_fifo2_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void gfx_debug_state_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t gfx_unk1_r(offs_t offset, uint64_t mem_mask = ~0);
	void gfx_unk1_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t gfx_fifo_r(offs_t offset, uint64_t mem_mask = ~0);
	void gfx_buf_w(uint64_t data);
	void gfx_cpu_dc_store(offs_t offset, uint32_t data);

	void sub_jvs_w(uint8_t data);

	void ide_interrupt(int state);

	std::unique_ptr<cobra_renderer> m_renderer;

	std::unique_ptr<cobra_fifo> m_gfxfifo_in;
	std::unique_ptr<cobra_fifo> m_gfxfifo_out;
	std::unique_ptr<cobra_fifo> m_m2sfifo;
	std::unique_ptr<cobra_fifo> m_s2mfifo;

	void gfxfifo_in_event_callback(cobra_fifo::EventType event);
	void gfxfifo_out_event_callback(cobra_fifo::EventType event);
	void m2sfifo_event_callback(cobra_fifo::EventType event);
	void s2mfifo_event_callback(cobra_fifo::EventType event);

	enum
	{
		MAIN_INT_M2S = 0x01,
		MAIN_INT_S2M = 0x02
	};

	uint8_t m_m2s_int_enable = 0;
	uint8_t m_s2m_int_enable = 0;
	uint8_t m_vblank_enable = 0;

	uint8_t m_m2s_int_mode = 0;
	uint8_t m_s2m_int_mode = 0;

	uint8_t m_main_int_active = 0;


	std::unique_ptr<uint32_t[]> m_comram[2];
	int m_comram_page = 0;

	int m_main_debug_state = 0;
	int m_main_debug_state_wc = 0;
	int m_sub_debug_state = 0;
	int m_sub_debug_state_wc = 0;
	int m_gfx_debug_state = 0;
	int m_gfx_debug_state_wc = 0;

	uint32_t m_sub_psac_reg = 0;
	int m_sub_psac_count = 0;
	uint32_t m_sub_interrupt = 0;

	uint8_t m_gfx_unk_flag = 0;
	uint32_t m_gfx_re_command_word1 = 0;
	uint32_t m_gfx_re_command_word2 = 0;
	int m_gfx_re_word_count = 0;
	int m_gfx_re_status = 0;
	uint32_t m_gfx_unk_status = 0;

	uint64_t m_gfx_fifo_mem[256]{};
	int m_gfx_fifo_cache_addr = 0;
	int m_gfx_fifo_loopback = 0;
	int m_gfx_unknown_v1 = 0;
	int m_gfx_status_byte = 0;

	bool m_has_psac = false;

	std::unique_ptr<int16_t[]> m_sound_dma_buffer_l;
	std::unique_ptr<int16_t[]> m_sound_dma_buffer_r;
	uint32_t m_sound_dma_ptr = 0;

	void init_racjamdx();
	void init_bujutsu();
	void init_cobra();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_cobra(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cobra_vblank);
	void cobra_video_exit();
	int decode_debug_state_value(int v);
	void cobra(machine_config &config);
	void bujutsu(machine_config &config);
	void cobra_gfx_map(address_map &map) ATTR_COLD;
	void cobra_main_map(address_map &map) ATTR_COLD;
	void cobra_sub_map(address_map &map) ATTR_COLD;

	void rf5c400_map(address_map &map) ATTR_COLD;

	uint32_t mpc106_pci_r(int function, int reg, uint32_t mem_mask);
	void mpc106_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);

	uint32_t m_mpc106_regs[256/4];
};

void cobra_renderer::render_color_scan(int32_t scanline, const extent_t &extent, const cobra_polydata &extradata, int threadid)
{
	uint32_t *const fb = &m_backbuffer->pix(scanline);
	float *const zb = (float*)&m_zbuffer->pix(scanline);

	float z = extent.param[POLY_Z].start;
	float dz = extent.param[POLY_Z].dpdx;

	float gr = extent.param[POLY_R].start;
	float dgr = extent.param[POLY_R].dpdx;
	float gg = extent.param[POLY_G].start;
	float dgg = extent.param[POLY_G].dpdx;
	float gb = extent.param[POLY_B].start;
	float dgb = extent.param[POLY_B].dpdx;
	[[maybe_unused]] float ga = extent.param[POLY_A].start;
	[[maybe_unused]] float dga = extent.param[POLY_A].dpdx;

	uint32_t zmode = extradata.zmode;

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (z <= zb[x] || zmode == 7)
		{
			uint32_t r = (int)(gr);
			uint32_t g = (int)(gg);
			uint32_t b = (int)(gb);

			if (r > 255) r = 255;
			if (g > 255) g = 255;
			if (b > 255) b = 255;

			r <<= 16;
			g <<= 8;

			fb[x] = 0xff000000 | r | g | b;
			zb[x] = z;
		}

		z += dz;
		gr += dgr;
		gg += dgg;
		gb += dgb;
		ga += dga;
	}
}

rgb_t cobra_renderer::texture_fetch(uint32_t *texture, int u, int v, int width, int format)
{
	uint32_t texel = texture[((v * width) + u) / 2];

	if (u & 1)
	{
		texel &= 0xffff;
	}
	else
	{
		texel >>= 16;
	}

	rgb_t color;

	if (format == 6)
	{
		int r = (texel & 0xf000) >> 8;
		int g = (texel & 0x0f00) >> 4;
		int b = (texel & 0x00f0) >> 0;
		int a = (texel & 0x000f) | ((texel & 0x000f) << 4);
		color = rgb_t(a, r, g, b);
	}
	else
	{
		int r = (texel & 0xf800) >> 8;
		int g = (texel & 0x07c0) >> 3;
		int b = (texel & 0x003e) << 2;
		int a = (texel & 0x0001) ? 0xff : 0;
		color = rgb_t(a, r, g, b);
	}

	return color;
}

void cobra_renderer::render_texture_scan(int32_t scanline, const extent_t &extent, const cobra_polydata &extradata, int threadid)
{
	float u = extent.param[POLY_U].start;
	float v = extent.param[POLY_V].start;
	float du = extent.param[POLY_U].dpdx;
	float dv = extent.param[POLY_V].dpdx;

	float w = extent.param[POLY_W].start;
	float dw = extent.param[POLY_W].dpdx;

	float z = extent.param[POLY_Z].start;
	float dz = extent.param[POLY_Z].dpdx;

	float gr = extent.param[POLY_R].start;
	float dgr = extent.param[POLY_R].dpdx;
	float gg = extent.param[POLY_G].start;
	float dgg = extent.param[POLY_G].dpdx;
	float gb = extent.param[POLY_B].start;
	float dgb = extent.param[POLY_B].dpdx;
	[[maybe_unused]] float ga = extent.param[POLY_A].start;
	[[maybe_unused]] float dga = extent.param[POLY_A].dpdx;

	uint32_t *const fb = &m_backbuffer->pix(scanline);
	float *const zb = (float*)&m_zbuffer->pix(scanline);

	uint32_t texture_width  = 1 << ((extradata.tex_format >> 28) & 0xf);
	uint32_t texture_height = 1 << ((extradata.tex_format >> 24) & 0xf);
	uint32_t tex_address = extradata.tex_address;

	uint32_t alpha_test = extradata.alpha_test;
	uint32_t zmode = extradata.zmode;
	uint32_t tex_format = (extradata.tex_format >> 2) & 0x7;

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		int iu, iv;

		if (z <= zb[x] || zmode == 7)
		{
			float oow;

			if (w == 0)
				oow = 1.0f;
			else
				oow = 1.0f / w;

#if !ENABLE_BILINEAR

			iu = (int)((u * oow) * texture_width) & 0x7ff;
			iv = (int)((v * oow) * texture_height) & 0x7ff;

			rgb_t texel = texture_fetch(&m_texture_ram[tex_address], iu, iv, texture_width, tex_format);

#else

			float tex_u = (u * oow) * texture_width;
			float tex_v = (v * oow) * texture_height;
			iu = (int)(tex_u) & 0x7ff;
			iv = (int)(tex_v) & 0x7ff;

			float lerp_u = tex_u - (float)(iu);
			float lerp_v = tex_v - (float)(iv);

			rgb_t texel00 = texture_fetch(&m_texture_ram[tex_address], iu, iv, texture_width, tex_format);
			rgb_t texel01 = texture_fetch(&m_texture_ram[tex_address], iu+1, iv, texture_width, tex_format);
			rgb_t texel10 = texture_fetch(&m_texture_ram[tex_address], iu, iv+1, texture_width, tex_format);
			rgb_t texel11 = texture_fetch(&m_texture_ram[tex_address], iu+1, iv+1, texture_width, tex_format);

			rgb_t texel = rgbaint_t::bilinear_filter(texel00, texel01, texel10, texel11, (int)(lerp_u * 255), (int)(lerp_v * 255));

#endif

			int a = texel.a();

			if (a != 0 || !alpha_test)
			{
				uint32_t gour = (int)(gr);
				uint32_t goug = (int)(gg);
				uint32_t goub = (int)(gb);

				int r = (texel.r() * gour) >> 8;
				int g = (texel.g() * goug) >> 8;
				int b = (texel.b() * goub) >> 8;

				if (a != 0xff)
				{
					int fb_r = (fb[x] >> 16) & 0xff;
					int fb_g = (fb[x] >> 8) & 0xff;
					int fb_b = fb[x] & 0xff;

					r = ((r * a) >> 8) + ((fb_r * (0xff-a)) >> 8);
					g = ((g * a) >> 8) + ((fb_g * (0xff-a)) >> 8);
					b = ((b * a) >> 8) + ((fb_b * (0xff-a)) >> 8);
				}

				if (r > 255) r = 255;
				if (g > 255) g = 255;
				if (b > 255) b = 255;

				r <<= 16;
				g <<= 8;

				fb[x] = 0xff000000 | r | g | b;
				zb[x] = z;
			}
		}

		u += du;
		v += dv;
		w += dw;
		z += dz;

		gr += dgr;
		gg += dgg;
		gb += dgb;
		ga += dga;
	}
}

void cobra_renderer::draw_point(const rectangle &visarea, vertex_t &v, uint32_t color)
{
	int x = v.x;
	int y = v.y;

	if (x >= visarea.min_x && x <= visarea.max_x &&
		y >= visarea.min_y && y <= visarea.max_y)
	{
		uint32_t *const fb = &m_backbuffer->pix(y);
		fb[x] = color;
	}
}

void cobra_renderer::draw_line(const rectangle &visarea, vertex_t &v1, vertex_t &v2)
{
	int dx = (v2.x - v1.x);
	int dy = (v2.y - v1.y);

	int x1 = v1.x;
	int y1 = v1.y;

	uint32_t color = 0xffffffff;      // TODO: where does the color come from?

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

			uint32_t *const fb = &m_backbuffer->pix(y);
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

			uint32_t *const fb = &m_backbuffer->pix(y);
			fb[x] = color;

			y++;
		}
	}
}

void cobra_state::cobra_video_exit()
{
	m_renderer->gfx_exit();
}

void cobra_state::video_start()
{
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&cobra_state::cobra_video_exit, this));

	m_renderer = std::make_unique<cobra_renderer>(*m_screen);
	m_renderer->gfx_init();
}

uint32_t cobra_state::screen_update_cobra(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_has_psac)
		m_k001604->draw_back_layer(screen, bitmap, cliprect);

	m_renderer->display(&bitmap, cliprect);

	if (m_has_psac)
		m_k001604->draw_front_layer(screen, bitmap, cliprect);
	return 0;
}




/*****************************************************************************/

int cobra_state::decode_debug_state_value(int v)
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


void cobra_fifo::push(const device_t *cpu, uint64_t data)
{
	if (m_verbose)
	{
		printf("%s %s: push %08X%08X (%d)\n", cpu->machine().describe_context().c_str(), m_name, (uint32_t)(data >> 32), (uint32_t)(data), m_num);
	}

	if (m_num == m_size)
	{
		if (m_verbose)
		{
			int i, j;
			printf("%s %s overflow\n", cpu->machine().describe_context().c_str(), m_name);
			printf("%s dump:\n", m_name);

			for (j=0; j < 128; j+=4)
			{
				printf("    ");
				for (i=0; i < 4; i++)
				{
					uint64_t val = 0;
					pop(cpu, &val);
					printf("%08X ", (uint32_t)(val));
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

	if (m_num >= m_size)
		m_event_callback(EVENT_FULL);
	if (m_num == (m_size / 2))
		m_event_callback(EVENT_HALF_FULL);
}

bool cobra_fifo::pop(const device_t *cpu, uint64_t *result)
{
	uint64_t r;

	if (m_num == 0)
	{
		if (m_verbose)
		{
			printf("%s %s underflow\n", cpu->machine().describe_context().c_str(), m_name);
		}
		return false;
	}

	r = m_data[m_rpos];

	if (m_verbose)
	{
		printf("%s %s: pop %08X%08X (%d)\n", cpu->machine().describe_context().c_str(), m_name, (uint32_t)(r >> 32), (uint32_t)(r), m_num-1);
	}

	m_rpos++;

	if (m_rpos == m_size)
	{
		m_rpos = 0;
	}

	m_num--;

	if (m_num == 0)
		m_event_callback(EVENT_EMPTY);
	if (m_num == (m_size / 2))
		m_event_callback(EVENT_HALF_FULL);

	*result = r;

	return true;
}

bool cobra_fifo::pop(const device_t *cpu, float *result)
{
	uint64_t value = 0;
	bool status = pop(cpu, &value);
	*result = u2f((uint32_t)(value));
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

	m_event_callback(EVENT_EMPTY);
}


/*****************************************************************************/

void cobra_state::m2sfifo_event_callback(cobra_fifo::EventType event)
{
	switch (event)
	{
		case cobra_fifo::EVENT_EMPTY:
		{
			m_subcpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

			// give sub cpu a bit more time to stabilize on the current fifo status
			m_maincpu->spin_until_time(attotime::from_usec(1));

			if (m_m2s_int_enable & 0x80)
			{
				if (!m_m2s_int_mode)
					m_main_int_active |= MAIN_INT_M2S;

				m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
			}

			// EXISR needs to update for the *next* instruction during FIFO tests
			// TODO: try to abort the timeslice before the next instruction?
			m_subcpu->set_state_int(PPC_EXISR, m_subcpu->state_int(PPC_EXISR) & ~0x10);
			break;
		}

		case cobra_fifo::EVENT_HALF_FULL:
			break;

		case cobra_fifo::EVENT_FULL:
			break;
	}
}

void cobra_state::s2mfifo_event_callback(cobra_fifo::EventType event)
{
	switch (event)
	{
		case cobra_fifo::EVENT_EMPTY:
			m_main_int_active &= ~MAIN_INT_S2M;
			break;

		case cobra_fifo::EVENT_HALF_FULL:
			break;

		case cobra_fifo::EVENT_FULL:
			break;
	}
}

void cobra_state::gfxfifo_in_event_callback(cobra_fifo::EventType event)
{
}

void cobra_state::gfxfifo_out_event_callback(cobra_fifo::EventType event)
{
}

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

// Interrupts (0xFFFF0003):
// 0x01: M2S FIFO
// 0x02: S2M FIFO
// 0x04: Vblank?

uint32_t cobra_state::mpc106_pci_r(int function, int reg, uint32_t mem_mask)
{
	//printf("MPC106: PCI read %d, %02X, %08X\n", function, reg, mem_mask);

	switch (reg)
	{
	}

	return m_mpc106_regs[reg/4];
}

void cobra_state::mpc106_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	//printf("MPC106: PCI write %d, %02X, %08X, %08X\n", function, reg, data, mem_mask);
	COMBINE_DATA(&m_mpc106_regs[reg/4]);
}

uint64_t cobra_state::main_fifo_r(offs_t offset, uint64_t mem_mask)
{
	uint64_t r = 0;

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

		r |= (uint64_t)(value) << 56;
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

		uint64_t value;
		m_s2mfifo->pop(m_maincpu.target(), &value);

		r |= (uint64_t)(value & 0xff) << 40;
	}
	if (ACCESSING_BITS_32_39)
	{
		// Register 0xffff0003:
		//
		// 7 6 5 4 3 2 1 0
		//----------------
		//               x   Unknown, must be 0 for coins to be updated
		//             x     S2M FIFO interrupt active
		//           x       Graphics board/FIFO busy flag
		//         x         M2S FIFO interrupt active

		int value = 0;

		value |= (m_main_int_active & MAIN_INT_S2M) ? 0x00 : 0x02;
		value |= (m_main_int_active & MAIN_INT_M2S) ? 0x00 : 0x08;
		value |= (m_gfx_unk_flag & 0x80) ? 0x00 : 0x04;

		r |= (uint64_t)(value) << 32;
	}

	return r;
}

void cobra_state::main_fifo_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	if (ACCESSING_BITS_40_47)
	{
		// Register 0xffff0002:
		// Main-to-Sub FIFO write data

		m_m2sfifo->push(m_maincpu.target(), (uint8_t)(data >> 40));

		if (!m_m2s_int_mode)
			m_main_int_active &= ~MAIN_INT_M2S;

		m_subcpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);

		// EXISR needs to update for the *next* instruction during FIFO tests
		// TODO: try to abort the timeslice before the next instruction?
		m_subcpu->set_state_int(PPC_EXISR, m_subcpu->state_int(PPC_EXISR) | 0x10);
	}
	if (ACCESSING_BITS_32_39)
	{
		// Register 0xffff0003:
		// Main-to-Sub FIFO unknown
		//
		// 7 6 5 4 3 2 1 0
		//----------------
		//         x            M2S interrupt mode (0 = empty, 1 = half full)
		// x                    Comram page

		m_m2s_int_mode = ((data >> 32) & 0x8) ? 1 : 0;

		if (m_m2s_int_mode)
		{
			if (!m_m2sfifo->is_half_full())
			{
				m_main_int_active |= MAIN_INT_M2S;
			}
			else
			{
				m_main_int_active &= ~MAIN_INT_M2S;
			}
		}
		else
		{
			if (m_m2sfifo->is_empty())
			{
				m_main_int_active |= MAIN_INT_M2S;
			}
			else
			{
				m_main_int_active &= ~MAIN_INT_M2S;
			}
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

		m_vblank_enable = (uint8_t)(data >> 24);

		if ((m_vblank_enable & 0x80) == 0)
		{
			// clear the interrupt
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
		}
	}
	if (ACCESSING_BITS_16_23)
	{
		// Register 0xffff0005:
		// Interrupt enable for S2MFIFO
		//
		// 7 6 5 4 3 2 1 0
		//----------------
		// x                    ?

		m_s2m_int_enable = (uint8_t)(data >> 16);

		if ((m_s2m_int_enable & 0x80) == 0)
		{
			m_main_int_active &= ~MAIN_INT_S2M;

			// clear the interrupt
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
		}
	}
	if (ACCESSING_BITS_8_15)
	{
		// Register 0xffff0007:
		// ???

		printf("main_fifo_w: 0xffff0006: %02X\n", (uint8_t)(data >> 8));
	}
	if (ACCESSING_BITS_0_7)
	{
		// Register 0xffff0007:
		// Interrupt enable for M2SFIFO
		//
		// 7 6 5 4 3 2 1 0
		//----------------
		// x                    ?

		m_m2s_int_enable = (uint8_t)(data);

		if ((m_m2s_int_enable & 0x80) == 0)
		{
			m_main_int_active &= ~MAIN_INT_M2S;

			// clear the interrupt
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
		}
	}

	if (ACCESSING_BITS_56_63)
	{
		// Register 0xffff0000
		// Debug state write
		m_main_debug_state |= decode_debug_state_value((data >> 56) & 0xff) << 4;
		m_main_debug_state_wc++;
	}
	if (ACCESSING_BITS_48_55)
	{
		// Register 0xffff0001
		// Debug state write
		m_main_debug_state |= decode_debug_state_value((data >> 48) & 0xff);
		m_main_debug_state_wc++;
	}

	if (m_main_debug_state_wc >= 2)
	{
		if (m_main_debug_state != 0)
		{
			LOGMASKED(LOG_DEBUG_STATES, "MAIN: debug state %02X\n", m_main_debug_state);
		}

		if (m_main_debug_state == 0x6b)
		{
			// install HD patches for bujutsu
			if (strcmp(machine().system().name, "bujutsu") == 0)
			{
				uint32_t *main_ram = (uint32_t*)(uint64_t*)m_main_ram;
				uint32_t *sub_ram = (uint32_t*)m_sub_ram;
				uint32_t *gfx_ram = (uint32_t*)(uint64_t*)m_gfx_ram0;

				main_ram[(0x0005ac^4) / 4] = 0x60000000;        // skip IRQ fail
				main_ram[(0x001ec4^4) / 4] = 0x60000000;        // waiting for IRQ?
				main_ram[(0x001f00^4) / 4] = 0x60000000;        // waiting for IRQ?

				sub_ram[0x568 / 4] = 0x60000000;                // skip IRQ fail

				gfx_ram[(0x38632c^4) / 4] = 0x38600000;     // skip check_one_scene()
			}
			// racjamdx
			else if (strcmp(machine().system().name, "racjamdx") == 0)
			{
			}
		}

		m_main_debug_state = 0;
		m_main_debug_state_wc = 0;
	}
}

uint64_t cobra_state::main_comram_r(offs_t offset, uint64_t mem_mask)
{
	uint64_t r = 0;
	int page = m_comram_page;

	if (ACCESSING_BITS_32_63)
	{
		r |= (uint64_t)(m_comram[page][(offset << 1) + 0]) << 32;
	}
	if (ACCESSING_BITS_0_31)
	{
		r |= (uint64_t)(m_comram[page][(offset << 1) + 1]);
	}

	return r;
}

void cobra_state::main_comram_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	int page = m_comram_page;

	uint32_t w1 = m_comram[page][(offset << 1) + 0];
	uint32_t w2 = m_comram[page][(offset << 1) + 1];
	uint32_t d1 = (uint32_t)(data >> 32);
	uint32_t d2 = (uint32_t)(data);
	uint32_t m1 = (uint32_t)(mem_mask >> 32);
	uint32_t m2 = (uint32_t)(mem_mask);

	m_comram[page][(offset << 1) + 0] = (w1 & ~m1) | (d1 & m1);
	m_comram[page][(offset << 1) + 1] = (w2 & ~m2) | (d2 & m2);
}

void cobra_state::main_cpu_dc_store(offs_t offset, uint32_t data)
{
	if ((offset & 0xf0000000) == 0xc0000000)
	{
		// force sync when writing to GFX board main ram
		m_maincpu->spin_until_time(attotime::from_usec(80));
	}
}

void cobra_state::cobra_main_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share("main_ram");
	map(0x07c00000, 0x07ffffff).ram();
	map(0x80000cf8, 0x80000cff).rw(m_legacy_pci, FUNC(pci_bus_legacy_device::read_64be), FUNC(pci_bus_legacy_device::write_64be));
	map(0xc0000000, 0xc03fffff).ram().share("gfx_main_ram_0");              // GFX board main ram, bank 0
	map(0xc7c00000, 0xc7ffffff).ram().share("gfx_main_ram_1");              // GFX board main ram, bank 1
	map(0xfff00000, 0xfff7ffff).rom().region("user1", 0);                   /* Boot ROM */
	map(0xfff80000, 0xfffbffff).rw(FUNC(cobra_state::main_comram_r), FUNC(cobra_state::main_comram_w));
	map(0xffff0000, 0xffff0007).rw(FUNC(cobra_state::main_fifo_r), FUNC(cobra_state::main_fifo_w));
}


/*****************************************************************************/
// Sub board (PPC403)

// Interrupts:

// Serial Transmit      JVS
// DMA0:                DMA-driven DAC
// DMA2:                SCSI?
// DMA3:                JVS
// External IRQ0        M2SFIFO
// External IRQ1        S2MFIFO (mostly dummy, only disables the interrupt)
// External IRQ2        SCSI Interrupt?

//static int ucount = 0;

uint32_t cobra_state::sub_unk1_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t r = 0;

	if (ACCESSING_BITS_16_23)
	{
		r |= 0x10000;
	}

	return r;
}

void cobra_state::sub_unk1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	/*
	if (!ACCESSING_BITS_24_31)
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

uint32_t cobra_state::sub_mainbd_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t r = 0;

	if (ACCESSING_BITS_24_31)
	{
		// Register 0x7E380000
		// M2S FIFO read

		uint64_t value = 0;
		m_m2sfifo->pop(m_subcpu.target(), &value);

		r |= (value & 0xff) << 24;
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
		// x                  Comram page

		uint32_t value = 0x00;
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

void cobra_state::sub_mainbd_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		// Register 0x7E380000
		// Sub-to-Main FIFO data

		m_s2mfifo->push(m_subcpu.target(), (uint8_t)(data >> 24));

		m_main_int_active |= MAIN_INT_S2M;

		// fire off an interrupt if enabled
		if (m_s2m_int_enable & 0x80)
		{
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		}
	}
	if (ACCESSING_BITS_16_23)
	{
		// Register 0x7E380001
		//
		// 7 6 5 4 3 2 1 0
		//----------------
		//               x    S2M interrupt mode (0 = empty, 1 = half full)

		m_s2m_int_mode = (data & 0x10000) ? 1 : 0;

		if (m_s2m_int_mode)
		{
			if (!m_s2mfifo->is_half_full())
			{
				m_subcpu->set_state_int(PPC_EXISR, m_subcpu->state_int(PPC_EXISR) | 0x08);
			}
			else
			{
				m_subcpu->set_state_int(PPC_EXISR, m_subcpu->state_int(PPC_EXISR) & ~0x08);
			}
		}
		else
		{
			if (m_s2mfifo->is_empty())
			{
				m_subcpu->set_state_int(PPC_EXISR, m_subcpu->state_int(PPC_EXISR) | 0x08);
			}
			else
			{
				m_subcpu->set_state_int(PPC_EXISR, m_subcpu->state_int(PPC_EXISR) & ~0x08);
			}
		}
	}
}

uint8_t cobra_state::sub_unk7e_r(offs_t offset)
{
	uint8_t r = 0;

	if (offset == 3)
	{
		r |= !m_jvs_host->sense();
	}

	return ~r;
}

void cobra_state::sub_debug_w(offs_t offset, uint32_t data, uint32_t mem_mask)
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
			LOGMASKED(LOG_DEBUG_STATES, "SUB: debug state %02X\n", m_sub_debug_state);
		}

		m_sub_debug_state = 0;
		m_sub_debug_state_wc = 0;
	}
}

uint32_t cobra_state::sub_config_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t r = 0;

	if (ACCESSING_BITS_8_15)
	{
		r |= (0x2) << 8;        // if bit 0x2 is zero, maskrom boot
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= m_sub_interrupt;
	}

	return r;
}

void cobra_state::sub_config_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
}

uint32_t cobra_state::sub_comram_r(offs_t offset)
{
	int page = m_comram_page ^ 1;

	return m_comram[page][offset];
}

void cobra_state::sub_comram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	int page = m_comram_page ^ 1;

	COMBINE_DATA(m_comram[page].get() + offset);
}

void cobra_state::sub_psac_palette_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	data = m_generic_paletteram_32[offset];
	m_palette->set_pen_color(offset, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

uint32_t cobra_state::sub_psac2_r()
{
	m_sub_psac_count++;
	if (m_sub_psac_count >= 0x8000)
	{
		m_sub_psac_reg ^= 0xffffffff;
		m_sub_psac_count = 0;
	}
	return m_sub_psac_reg;
}

void cobra_state::sub_psac2_w(uint32_t data)
{
}

void cobra_state::sub_sound_dma_w(offs_t offset, uint32_t data)
{
	//printf("DMA write to unknown: size %d, data %08X\n", address, data);

	/*
	static FILE *out;
	if (out == nullptr)
	    out = fopen("sound.bin", "wb");

	fputc((data >> 24) & 0xff, out);
	fputc((data >> 16) & 0xff, out);
	fputc((data >> 8) & 0xff, out);
	fputc((data >> 0) & 0xff, out);
	*/

	int16_t ldata = (int16_t)(data >> 16);
	int16_t rdata = (int16_t)(data);

	m_sound_dma_buffer_l[m_sound_dma_ptr] = ldata;
	m_sound_dma_buffer_r[m_sound_dma_ptr] = rdata;
	m_sound_dma_ptr++;

	if (m_sound_dma_ptr >= DMA_SOUND_BUFFER_SIZE)
	{
		m_sound_dma_ptr = 0;

		m_dmadac[0]->transfer(0, 0, 1, DMA_SOUND_BUFFER_SIZE, m_sound_dma_buffer_l.get());
		m_dmadac[1]->transfer(0, 0, 1, DMA_SOUND_BUFFER_SIZE, m_sound_dma_buffer_r.get());
	}
}

void cobra_state::sub_jvs_w(uint8_t data)
{
	bool accepted = m_jvs_host->write(data);
	if (accepted)
		m_jvs_host->read();
}

void cobra_state::cobra_sub_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share("sub_ram");                       // Main RAM
	map(0x70000000, 0x7003ffff).rw(FUNC(cobra_state::sub_comram_r), FUNC(cobra_state::sub_comram_w));         // Double buffered shared RAM between Main and Sub
//  map(0x78000000, 0x780000ff).noprw();                                           // SCSI controller (unused)
	map(0x78040000, 0x7804ffff).rw("rfsnd", FUNC(rf5c400_device::rf5c400_r), FUNC(rf5c400_device::rf5c400_w));
	map(0x78080000, 0x7808000f).rw(m_ata, FUNC(ata_interface_device::cs0_swap_r), FUNC(ata_interface_device::cs0_swap_w));
	map(0x780c0010, 0x780c001f).rw(m_ata, FUNC(ata_interface_device::cs1_swap_r), FUNC(ata_interface_device::cs1_swap_w));
	map(0x78200000, 0x782000ff).rw(m_k001604, FUNC(k001604_device::reg_r), FUNC(k001604_device::reg_w));              // PSAC registers
	map(0x78210000, 0x78217fff).ram().w(FUNC(cobra_state::sub_psac_palette_w)).share("paletteram");                      // PSAC palette RAM
	map(0x78220000, 0x7823ffff).rw(m_k001604, FUNC(k001604_device::tile_r), FUNC(k001604_device::tile_w));            // PSAC tile RAM
	map(0x78240000, 0x7827ffff).rw(m_k001604, FUNC(k001604_device::char_r), FUNC(k001604_device::char_w));            // PSAC character RAM
	map(0x78280000, 0x7828000f).noprw();                                           // ???
	map(0x78300000, 0x7830000f).rw(FUNC(cobra_state::sub_psac2_r), FUNC(cobra_state::sub_psac2_w));           // PSAC
	map(0x7e000000, 0x7e000003).rw(FUNC(cobra_state::sub_unk7e_r), FUNC(cobra_state::sub_debug_w));
	map(0x7e040000, 0x7e041fff).rw("m48t58", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write));    /* M48T58Y RTC/NVRAM */
	map(0x7e180000, 0x7e180003).rw(FUNC(cobra_state::sub_unk1_r), FUNC(cobra_state::sub_unk1_w));             // TMS57002?
	map(0x7e200000, 0x7e200003).rw(FUNC(cobra_state::sub_config_r), FUNC(cobra_state::sub_config_w));
	map(0x7e280000, 0x7e28ffff).noprw();                                           // LANC
	map(0x7e300000, 0x7e30ffff).noprw();                                           // LANC
	map(0x7e380000, 0x7e380003).rw(FUNC(cobra_state::sub_mainbd_r), FUNC(cobra_state::sub_mainbd_w));
	map(0x7ff80000, 0x7fffffff).rom().region("user2", 0);                     /* Boot ROM */
}


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


void cobra_renderer::display(bitmap_rgb32 *bitmap, const rectangle &cliprect)
{
	if (m_gfx_register[0] & 0x4)
	{
		copybitmap_trans(*bitmap, *m_framebuffer, 0, 0, cliprect.min_x, cliprect.min_y, cliprect, 0);
	}
	else
	{
		copybitmap_trans(*bitmap, *m_backbuffer, 0, 0, cliprect.min_x, cliprect.min_y, cliprect, 0);
	}
}

void cobra_renderer::gfx_init()
{
	m_gfx_gram = std::make_unique<uint32_t[]>(0x40000);

	m_gfx_register = std::make_unique<uint64_t[]>(0x3000);
	m_gfx_register_select = 0;

	float zvalue = 10000000.0f;
	m_zbuffer->fill(*(int*)&zvalue, m_framebuffer_size);
}

void cobra_renderer::gfx_exit()
{
	/*
	FILE *file;
	file = fopen("texture_ram.bin","wb");
	for (int i=0; i < 0x100000; i++)
	{
	    fputc((uint8_t)(m_texture_ram[i] >> 24), file);
	    fputc((uint8_t)(m_texture_ram[i] >> 16), file);
	    fputc((uint8_t)(m_texture_ram[i] >> 8), file);
	    fputc((uint8_t)(m_texture_ram[i] >> 0), file);
	}
	fclose(file);
	*/
}

void cobra_renderer::gfx_reset()
{
	cobra_state *cobra = screen().machine().driver_data<cobra_state>();

	cobra->m_gfx_re_status = RE_STATUS_IDLE;
}

uint32_t cobra_renderer::gfx_read_gram(uint32_t address)
{
	if (!DWORD_ALIGNED(address))
	{
		printf("gfx_read_gram: %08X, not dword aligned!\n", address);
		return 0;
	}

	switch ((address >> 16) & 0xf)
	{
		case 0xc:       // 0xCxxxx
		{
			if ((address >= 0xc0c00 && address < 0xc1000) ||
				(address >= 0xc4c00 && address < 0xc5000) ||
				(address >= 0xc8c00 && address < 0xc9000) ||
				(address >= 0xccc00 && address < 0xcd000))
			{
				uint32_t a = (((address >> 2) & 0xff) * 2) + ((address & 0x4000) ? 1 : 0);
				uint32_t page = ((m_gfx_gram[0xc3028/4] >> 9) * 0x800) +
								((address & 0x8000) ? 0x400 : 0) +
								((m_gfx_gram[0xc3028/4] & 0x100) ? 0x200 : 0);

				return m_texture_ram[page + a];
			}
			break;
		}
	}

	return m_gfx_gram[address/4];
}

void cobra_renderer::gfx_write_gram(uint32_t address, uint32_t mask, uint32_t data)
{
	switch ((address >> 16) & 0xf)
	{
		case 0x4:       // 0x4xxxx
		{
			if (address == 0x40fff)
			{
				printf("gfx: reg 40fff = %d, %d\n", (uint16_t)(data >> 16), (uint16_t)(data));
			}
			break;
		}

		case 0xc:       // 0xCxxxx
		{
			switch (address & 0xffff)
			{
				case 0x3020:
				case 0x0020:
				{
					m_texram_ptr = (data & mask) * 4;
					break;
				}
			}

			break;
		}
	}

	if (!DWORD_ALIGNED(address))
	{
		printf("gfx_write_gram: %08X, %08X, not dword aligned!\n", address, data);
		return;
	}

	m_gfx_gram[address/4] &= ~mask;
	m_gfx_gram[address/4] |= data & mask;
}

uint64_t cobra_renderer::gfx_read_reg()
{
	return m_gfx_register[m_gfx_register_select];
}

void cobra_renderer::gfx_write_reg(uint64_t data)
{
	switch (m_gfx_register_select)
	{
		case 0x0000:
		{
			copybitmap_trans(*m_framebuffer, *m_backbuffer, 0, 0, 0, 0, m_framebuffer_size, 0);
			m_backbuffer->fill(0xff000000, m_framebuffer_size);

			float zvalue = 10000000.0f;
			m_zbuffer->fill(*(int*)&zvalue, m_framebuffer_size);
			break;
		}
	}

	m_gfx_register[m_gfx_register_select] = data;
}

void cobra_renderer::gfx_fifo_exec()
{
	cobra_state *cobra = screen().machine().driver_data<cobra_state>();

	if (cobra->m_gfx_fifo_loopback != 0)
		return;

	vertex_t vert[32];

	cobra_fifo *fifo_in = cobra->m_gfxfifo_in.get();
	cobra_fifo *fifo_out = cobra->m_gfxfifo_out.get();

	while (fifo_in->current_num() >= 2)
	{
		uint64_t in1, in2 = 0;
		uint32_t w1, w2;

		if (cobra->m_gfx_re_status == RE_STATUS_IDLE)
		{
			fifo_in->pop(nullptr, &in1);
			fifo_in->pop(nullptr, &in2);
			w1 = (uint32_t)(in1);
			w2 = (uint32_t)(in2);

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
				uint64_t param[6];
				uint32_t w[6];

				if (fifo_in->current_num() < 6)
				{
					// wait until there's enough data in FIFO
					memset(param, 0, sizeof(param));
					memset(w, 0, sizeof(w));
					return;
				}

				fifo_in->pop(nullptr, &param[0]);
				fifo_in->pop(nullptr, &param[1]);
				fifo_in->pop(nullptr, &param[2]);
				fifo_in->pop(nullptr, &param[3]);
				fifo_in->pop(nullptr, &param[4]);
				fifo_in->pop(nullptr, &param[5]);

				w[0] = (uint32_t)param[0];    w[1] = (uint32_t)param[1];    w[2] = (uint32_t)param[2];
				w[3] = (uint32_t)param[3];    w[4] = (uint32_t)param[4];    w[5] = (uint32_t)param[5];

				// mbuslib_pumpkin(): 0x00600000 0x10500010
				//                    0x00600000 0x10500018

				if (w2 == 0x10500010)
				{
					// GFX register select
					m_gfx_register_select = w[3];

				//  printf("GFX: register select %08X\n", m_gfx_register_select);
				}
				else if (w2 == 0x10500018)
				{
					// register write to the register selected above?
					// 64-bit registers, top 32-bits in word 2, low 32-bit in word 3
				//  printf("GFX: register write %08X: %08X %08X\n", m_gfx_register_select, w[2], w[3]);

					gfx_write_reg(((uint64_t)(w[2]) << 32) | w[3]);
				}
				else if (w2 == 0x10521000)
				{
					printf("gfxfifo_exec: unknown %08X %08X %08X %08X\n", w1, w2, w[0], w[1]);
					printf("                      %08X %08X %08X %08X\n", w[2], w[3], w[4], w[5]);
				}
				else
				{
					cobra->logerror("gfxfifo_exec: unknown %08X %08X\n", w1, w2);
				}

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}
			case 0x0f:
			case 0xf0:
			{
				uint64_t in3 = 0, in4 = 0, ignore;

				// check_mergebus_self(): 0x0F600000 0x10520C00

				if (fifo_in->current_num() < 6)
				{
					// wait until there's enough data in FIFO
					return;
				}

				if (w1 != 0x0f600000 && w1 != 0xf0600000)
				{
					cobra->logerror("gfxfifo_exec: unknown %08X %08X\n", w1, w2);
				}

				//printf("gfxfifo_exec: unhandled %08X %08X\n", w1, w2);

				fifo_in->pop(nullptr, &in3);
				fifo_in->pop(nullptr, &in4);
				fifo_in->pop(nullptr, &ignore);
				fifo_in->pop(nullptr, &ignore);
				fifo_in->pop(nullptr, &ignore);
				fifo_in->pop(nullptr, &ignore);

				if (w1 == 0x0f600000 && w2 == 0x10520c00)
				{
					fifo_out->push(nullptr, w1);
					fifo_out->push(nullptr, w2);
					fifo_out->push(nullptr, in3);
					fifo_out->push(nullptr, in4);
				}

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}

			case 0xf1:
			case 0xf4:
			{
				//printf("gfxfifo_exec: unhandled %08X %08X\n", w1, w2);

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}

			case 0xe0:
			case 0xe2:
			case 0xe3:
			{
				// Draw graphics primitives

				// 0x00: xxxx---- -------- -------- --------    Command
				// 0x00: ----xxxx -------- -------- --------    Primitive type (0 = triangle, 2 = point, 3 = line)
				// 0x00: -------- xx------ -------- --------    ?
				// 0x00: -------- -------- -------- xxxxxxxx    Number of units (amount of bits is uncertain)
				//
				// 0x01: -x------ -------- -------- --------    Has extra flags word (used by lines only)
				// 0x01: --x----- -------- -------- --------    Has extra unknown float (seems to be fog-related?)
				// 0x01: ---x---- -------- -------- --------    Always 1?
				// 0x01: -------- xx------ -------- --------    ?
				// 0x01: -------- --x----- -------- --------    Has texture coords
				// 0x01: -------- ---x---- -------- --------    ?
				// 0x01: -------- ----x--- -------- --------    ?
				// 0x01: -------- -------- ------xx xx------    ? (always set to 1s?)
				// 0x01: -------- -------- -------- -------x    Has extra unknown float

				int i;
				int num = 0;
				int units = w1 & 0xff;

				// determine the expected packet size to see if we can process it yet
				int unit_size = 8;
				if (w2 & 0x40000000)    unit_size += 1;     // lines only
				if (w2 & 0x20000000)    unit_size += 1;     // unknown float
				if (w2 & 0x00200000)    unit_size += 3;     // texture coords?
				if (w2 & 0x00000001)    unit_size += 1;     // ?

				num = unit_size * units;


				if (fifo_in->current_num() < num)
				{
					// wait until there's enough data in FIFO
					return;
				}



				float vp_width      = u2f(m_gfx_gram[0x00090/4]);
				float vp_height     = u2f(m_gfx_gram[0x000a4/4]);
				float vp_center_x   = u2f(m_gfx_gram[0x0009c/4]);
				float vp_center_y   = u2f(m_gfx_gram[0x000ac/4]);

				if (vp_width == 0.0f)
				{
					vp_width = 256.0f;
					vp_center_x = 256.0f;
				}

				if (LOG_DRAW_COMMANDS) cobra->logerror("--- Draw command %08X %08X ---\n", w1, w2);

				// extract vertex data
				for (int i=0; i < units; i++)
				{
					float x, y, z, w;
					float r, g, b, a;
					w = 1.0f;

					uint64_t in[4];
					if (w2 & 0x40000000)        // line flags
					{
						fifo_in->pop(nullptr, &in[0]);
					}

					if (w2 & 0x20000000)        // unknown float (0.0f ... 1.0f)
					{
						fifo_in->pop(nullptr, &in[1]);
					}

					fifo_in->pop(nullptr, &x);                     // X coord
					fifo_in->pop(nullptr, &y);                     // Y coord
					fifo_in->pop(nullptr, &in[2]);                 // coord?
					fifo_in->pop(nullptr, &z);                     // Z coord

					if (w2 & 0x00200000)        // texture coords
					{
						fifo_in->pop(nullptr, &w);                 // W coord (1 / Z)
						fifo_in->pop(nullptr, &vert[i].p[POLY_U]); // U/Z coord
						fifo_in->pop(nullptr, &vert[i].p[POLY_V]); // V/Z coord
					}

					fifo_in->pop(nullptr, &a);                     // Gouraud Color Alpha
					fifo_in->pop(nullptr, &r);                     // Gouraud Color R
					fifo_in->pop(nullptr, &g);                     // Gouraud Color G
					fifo_in->pop(nullptr, &b);                     // Gouraud Color B

					if (w2 & 0x00000001)        // unknown float (0.0f ... 1.0f)
					{
						fifo_in->pop(nullptr, &in[3]);
					}

					vert[i].x = ((x / z) * vp_width) + vp_center_x;
					vert[i].y = ((y / z) * vp_height) + vp_center_y;
					vert[i].p[POLY_Z] = z;
					vert[i].p[POLY_W] = w;
					vert[i].p[POLY_R] = r * 255.0f;
					vert[i].p[POLY_G] = g * 255.0f;
					vert[i].p[POLY_B] = b * 255.0f;
					vert[i].p[POLY_A] = a * 255.0f;

					if (w2 & 0x40000000)
					{
						if (LOG_DRAW_COMMANDS) cobra->logerror("    ?: %08X\n", (uint32_t)in[0]);
					}
					if (w2 & 0x20000000)
					{
						if (LOG_DRAW_COMMANDS) cobra->logerror("    ?: %08X\n", (uint32_t)in[1]);
					}

					if (LOG_DRAW_COMMANDS) cobra->logerror("    x: %f\n", x);
					if (LOG_DRAW_COMMANDS) cobra->logerror("    y: %f\n", y);
					if (LOG_DRAW_COMMANDS) cobra->logerror("    ?: %08X\n", (uint32_t)in[2]);
					if (LOG_DRAW_COMMANDS) cobra->logerror("    z: %f\n", z);

					if (w2 & 0x00200000)
					{
						if (LOG_DRAW_COMMANDS) cobra->logerror("    w: %f\n", w);
						if (LOG_DRAW_COMMANDS) cobra->logerror("    u: %f\n", vert[i].p[POLY_U]);
						if (LOG_DRAW_COMMANDS) cobra->logerror("    v: %f\n", vert[i].p[POLY_V]);
					}

					if (LOG_DRAW_COMMANDS) cobra->logerror("    a: %f\n", a);
					if (LOG_DRAW_COMMANDS) cobra->logerror("    r: %f\n", r);
					if (LOG_DRAW_COMMANDS) cobra->logerror("    g: %f\n", g);
					if (LOG_DRAW_COMMANDS) cobra->logerror("    b: %f\n", b);

					if (w2 & 0x00000001)
					{
						if (LOG_DRAW_COMMANDS) cobra->logerror("    ?: %08X\n", (uint32_t)in[3]);
					}

					if (LOG_DRAW_COMMANDS) cobra->logerror("\n");
				}


				cobra_polydata &extra = object_data().next();

				int texture = (m_gfx_gram[0x400f4/4] >> 29);

				extra.alpha_test = m_gfx_gram[0x40198/4] & 0x80000000;
				extra.zmode = (m_gfx_gram[0x80020/4] >> 17) & 0x7;
				extra.tex_format = m_gfx_gram[(0x2910 + (texture << 7)) / 4];
				extra.tex_address = (m_gfx_gram[(0x2914 + (texture << 7)) / 4] >> 12) & 0xfffff;


				// render
				switch ((w1 >> 24) & 0xf)
				{
					case 0x0:           // triangles
					{
						if (w2 & 0x00200000)
						{
							render_delegate rd = render_delegate(&cobra_renderer::render_texture_scan, this);
							for (int i=2; i < units; i++)
							{
								render_triangle<8>(m_framebuffer_size, rd, vert[i-2], vert[i-1], vert[i]);
							}
						}
						else
						{
							render_delegate rd = render_delegate(&cobra_renderer::render_color_scan, this);
							for (int i=2; i < units; i++)
							{
								render_triangle<5>(m_framebuffer_size, rd, vert[i-2], vert[i-1], vert[i]);
							}
						}
						break;
					}

					case 0x2:           // points
					{
						for (int i=0; i < units; i++)
						{
							draw_point(m_framebuffer_size, vert[i], 0xffffffff);
						}
						break;
					}

					case 0x3:           // lines
					{
						if ((units & 1) == 0)       // batches of lines
						{
							for (i=0; i < units; i+=2)
							{
								draw_line(m_framebuffer_size, vert[i], vert[i+1]);
							}
						}
						else                        // line strip
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

				x &= 0x3ff;
				y &= 0x3ff;

				for (i=0; i < num; i+=4)
				{
					uint32_t *buffer;
					switch (m_gfx_gram[0x80104/4])
					{
						case 0x800000:      buffer = &m_framebuffer->pix(y); break;
						case 0x200000:      buffer = &m_backbuffer->pix(y); break;
						case 0x0e0000:      buffer = &m_overlay->pix(y); break;
						case 0x000800:      buffer = &m_zbuffer->pix(y); break;
						case 0x000200:      buffer = &m_stencil->pix(y); break;

						default:
						{
							fatalerror("gfxfifo_exec: fb write to buffer %08X!\n", m_gfx_gram[0x80100/4]);
						}
					}

					uint64_t param[4];
					param[0] = param[1] = param[2] = param[3] = 0;
					fifo_in->pop(nullptr, &param[0]);
					fifo_in->pop(nullptr, &param[1]);
					fifo_in->pop(nullptr, &param[2]);
					fifo_in->pop(nullptr, &param[3]);

					buffer[x+0] = (uint32_t)(param[0]);
					buffer[x+1] = (uint32_t)(param[1]);
					buffer[x+2] = (uint32_t)(param[2]);
					buffer[x+3] = (uint32_t)(param[3]);

					//printf("gfx: fb write %d, %d: %08X %08X %08X %08X\n", x, y, (uint32_t)(param[0]), (uint32_t)(param[1]), (uint32_t)(param[2]), (uint32_t)(param[3]));

					y++;
				}

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}

			case 0xe9:
			{
				// Read a specified pixel position from a pixelbuffer

//              printf("GFX: FB read X: %d, Y: %d, %08X\n", (uint16_t)(m_gfx_gram[0x118/4] >> 16), (uint16_t)(m_gfx_gram[0x118/4]), m_gfx_gram[0x114/4]);

				int x = (m_gfx_gram[0x118/4] >> 16) & 0xffff;
				int y = m_gfx_gram[0x118/4] & 0xffff;

				int pix_count = m_gfx_gram[0x114/4] & 0xffff;
				int line_count = (m_gfx_gram[0x114/4] >> 16) & 0xffff;

				// flush fifo_out so we have fresh data at top
				fifo_out->flush();

				if (pix_count != 4)
					fatalerror("GFX: fb read line count %d, pix count %d\n", line_count, pix_count);

				for (int i=0; i < line_count; i++)
				{
					uint32_t *buffer;
					switch (m_gfx_gram[0x80104/4])
					{
						case 0x800000:      buffer = &m_framebuffer->pix(y+i); break;
						case 0x200000:      buffer = &m_backbuffer->pix(y+i); break;
						case 0x0e0000:      buffer = &m_overlay->pix(y+i); break;
						case 0x000800:      buffer = &m_zbuffer->pix(y+i); break;
						case 0x000200:      buffer = &m_stencil->pix(y+i); break;

						default:
						{
							fatalerror("gfxfifo_exec: fb read from buffer %08X!\n", m_gfx_gram[0x80100/4]);
						}
					}

					fifo_out->push(nullptr, buffer[x+0]);
					fifo_out->push(nullptr, buffer[x+1]);
					fifo_out->push(nullptr, buffer[x+2]);
					fifo_out->push(nullptr, buffer[x+3]);
				}

				cobra->m_gfx_re_status = RE_STATUS_IDLE;
				break;
			}

			case 0x8f:
			{
				// buf_flush(): 0x8FFF0000 0x00000000

				if (w1 != 0x8fff0000 || w2 != 0x00000000)
				{
					cobra->logerror("gfxfifo_exec: buf_flush: %08X %08X\n", w1, w2);
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
				uint32_t mask = m_gfx_regmask[w1 & 0xff];

				gfx_write_gram(reg, mask, w2);

				if (reg != 0x118 && reg != 0x114 && reg != 0x11c)
				{
					if (LOG_GFX_RAM_WRITES) cobra->logerror("gfxfifo_exec: ram write %05X (mask %08X): %08X (%f)\n", reg, mask, w2, u2f(w2));
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
					uint64_t value = 0;
					fifo_in->pop(nullptr, &value);

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

				uint32_t ret = gfx_read_gram(reg);
				fifo_out->push(nullptr, ret);

		//      printf("GFX: reg read %08X\n", reg);

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
					uint32_t value = gfx_read_gram(reg + (i*4));

					fifo_out->push(nullptr, value);
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

				if (cobra->m_gfx_re_word_count == 0 && num_left > 0)
					printf("gfxfifo_exec: tex_ints %d words left\n", num - cobra->m_gfx_re_word_count);

				for (int i=0; i < num_left; i++)
				{
					uint64_t param = 0;
					fifo_in->pop(nullptr, &param);
					cobra->m_gfx_re_word_count++;

					m_texture_ram[m_texram_ptr] = (uint32_t)(param);
					m_texram_ptr++;
				}


				if (cobra->m_gfx_re_word_count >= num)
				{
					cobra->m_gfx_re_status = RE_STATUS_IDLE;
				}
				break;
			}
			default:
			{
				int c = 0;
				printf("gfxfifo_exec: unknown command %08X %08X\n", w1, w2);

				if (fifo_in->current_num() < 0)
				{
					return;
				}

				while (fifo_in->current_num() > 0)
				{
					uint64_t param;
					fifo_in->pop(nullptr, &param);

					if (c == 0)
						printf("              ");
					printf("%08X ", (uint32_t)(param));

					c++;

					if (c == 4)
					{
						printf("\n");
						c = 0;
					}
				};
				cobra->logerror("\n");
			}
		}

//      printf("gfxfifo_exec: %08X %08X\n", w1, w2);
	};

	wait();
}

uint64_t cobra_state::gfx_fifo_r(offs_t offset, uint64_t mem_mask)
{
	uint64_t r = 0;

	m_renderer->gfx_fifo_exec();

	if (ACCESSING_BITS_32_63)
	{
		uint64_t data = 0;
		m_gfxfifo_out->pop(m_gfxcpu.target(), &data);

		data &= 0xffffffff;

		r |= (uint64_t)(data) << 32;
	}
	if (ACCESSING_BITS_0_31)
	{
		uint64_t data = 0;
		m_gfxfifo_out->pop(m_gfxcpu.target(), &data);

		data &= 0xffffffff;

		r |= (uint64_t)(data);
	}
//  printf("GFX FIFO read %08X%08X\n", (uint32_t)(r >> 32), (uint32_t)(r));

	return r;
}

void cobra_state::gfx_fifo0_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	m_gfx_fifo_cache_addr = 2;
	COMBINE_DATA(m_gfx_fifo_mem + offset);
}

void cobra_state::gfx_fifo1_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	m_gfx_fifo_cache_addr = 0;
	COMBINE_DATA(m_gfx_fifo_mem + offset);
}

void cobra_state::gfx_fifo2_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	m_gfx_fifo_cache_addr = 1;
	COMBINE_DATA(m_gfx_fifo_mem + offset);
}

uint64_t cobra_state::gfx_unk1_r(offs_t offset, uint64_t mem_mask)
{
	uint64_t r = 0;

	if (ACCESSING_BITS_56_63)
	{
		uint64_t v = 0;
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

		r |= (uint64_t) 0x7f << 40;
	}
	if (ACCESSING_BITS_24_31)           // this register returns FIFO number during check_fifo (see below)
	{
		r |= (m_gfx_unknown_v1 & 3) << 24;
	}

	return r;
}

void cobra_state::gfx_unk1_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
//  printf("gfx_unk1_w: %08X %08X, %08X%08X\n", (uint32_t)(data >> 32), (uint32_t)(data), (uint32_t)(mem_mask >> 32), (uint32_t)(mem_mask));

	if (ACCESSING_BITS_56_63)
	{
		if ((data >> 63) & 1)
		{
			m_gfx_fifo_loopback = 0;
		}
	}

	if (ACCESSING_BITS_24_31)
	{
		uint64_t in1, in2;
		int value = (data >> 24) & 0xff;
		// used in check_fifo(). fifo loopback or something?

		if (value == 0xc0)
		{
			m_gfxfifo_in->pop(m_gfxcpu.target(), &in1);
			m_gfxfifo_in->pop(m_gfxcpu.target(), &in2);
			m_gfx_unknown_v1 = (uint32_t)(in1 >> 32);         // FIFO number is read back from this same register

			m_gfxfifo_out->push(m_gfxcpu.target(), in1 & 0xffffffff);
			m_gfxfifo_out->push(m_gfxcpu.target(), in2 & 0xffffffff);
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

void cobra_state::gfx_buf_w(uint64_t data)
{
//  printf("buf_w: top = %08X\n", gfxfifo_get_top());

	// buf_prc_read: 0x00A00001 0x10520200
	//               0x00A00001 0x10500018

	// teximage_load() / mbuslib_prc_read():    0x00A00001 0x10520800

//  printf("prc_read %08X%08X at %08X\n", (uint32_t)(data >> 32), (uint32_t)(data), m_gfxcpu->pc());

	m_renderer->gfx_fifo_exec();

	if (data == 0x00a0000110500018U)
	{
		m_gfxfifo_out->flush();

		// reads back the register selected by gfx register select

		uint64_t regdata = m_renderer->gfx_read_reg();

		m_gfxfifo_out->push(m_gfxcpu.target(), (uint32_t)(regdata >> 32));
		m_gfxfifo_out->push(m_gfxcpu.target(), (uint32_t)(regdata));
	}
	else if (data == 0x00a0000110520800U)
	{
		// in teximage_load()
		// some kind of busy flag for mbuslib_tex_ints()...

		// mbuslib_tex_ints() waits for bit 0x400 to be set
		// memcheck_teximage() wants 0x400 cleared

		m_gfxfifo_out->push(m_gfxcpu.target(), m_gfx_unk_status);

		m_gfx_unk_status &= ~0x400;
	}
	else if (data != 0x00a0000110520200U)       // mbuslib_regread()
	{
		// prc_read always expects a value...

		m_gfxfifo_out->push(m_gfxcpu.target(), 0);
	}
}

void cobra_state::gfx_cpu_dc_store(offs_t offset, uint32_t data)
{
	uint32_t addr = offset >> 24;
	if (addr == 0x10 || addr == 0x18 || addr == 0x1e)
	{
		uint64_t i = (uint64_t)(m_gfx_fifo_cache_addr) << 32;
		cobra_fifo *fifo_in = m_gfxfifo_in.get();

		uint32_t a = (offset / 8) & 0xff;

		fifo_in->push(m_gfxcpu, (uint32_t)(m_gfx_fifo_mem[a+0] >> 32) | i);
		fifo_in->push(m_gfxcpu, (uint32_t)(m_gfx_fifo_mem[a+0] >>  0) | i);
		fifo_in->push(m_gfxcpu, (uint32_t)(m_gfx_fifo_mem[a+1] >> 32) | i);
		fifo_in->push(m_gfxcpu, (uint32_t)(m_gfx_fifo_mem[a+1] >>  0) | i);
		fifo_in->push(m_gfxcpu, (uint32_t)(m_gfx_fifo_mem[a+2] >> 32) | i);
		fifo_in->push(m_gfxcpu, (uint32_t)(m_gfx_fifo_mem[a+2] >>  0) | i);
		fifo_in->push(m_gfxcpu, (uint32_t)(m_gfx_fifo_mem[a+3] >> 32) | i);
		fifo_in->push(m_gfxcpu, (uint32_t)(m_gfx_fifo_mem[a+3] >>  0) | i);

		m_renderer->gfx_fifo_exec();
	}
	else
	{
		logerror("gfx: data cache store at %08X\n", offset);
	}
}

void cobra_state::gfx_debug_state_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	if (ACCESSING_BITS_40_47)
	{
		m_gfx_unk_flag = (uint8_t)(data >> 40);
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
			LOGMASKED(LOG_DEBUG_STATES, "GFX: debug state %02X\n", m_gfx_debug_state);
		}

		m_gfx_debug_state = 0;
		m_gfx_debug_state_wc = 0;
	}
}

void cobra_state::cobra_gfx_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share("gfx_main_ram_0");
	map(0x07c00000, 0x07ffffff).ram().share("gfx_main_ram_1");
	map(0x10000000, 0x100007ff).w(FUNC(cobra_state::gfx_fifo0_w));
	map(0x18000000, 0x180007ff).w(FUNC(cobra_state::gfx_fifo1_w));
	map(0x1e000000, 0x1e0007ff).w(FUNC(cobra_state::gfx_fifo2_w));
	map(0x20000000, 0x20000007).w(FUNC(cobra_state::gfx_buf_w));                            // this might really map to 0x1e000000, depending on the pagetable
	map(0x7f000000, 0x7f00ffff).ram().share("pagetable");
	map(0xfff00000, 0xfff7ffff).rom().region("user3", 0);                   /* Boot ROM */
	map(0xfff80000, 0xfff80007).w(FUNC(cobra_state::gfx_debug_state_w));
	map(0xffff0000, 0xffff0007).rw(FUNC(cobra_state::gfx_unk1_r), FUNC(cobra_state::gfx_unk1_w));
	map(0xffff0010, 0xffff001f).r(FUNC(cobra_state::gfx_fifo_r));
}


/*****************************************************************************/
INPUT_PORTS_START( cobra )
INPUT_PORTS_END

void cobra_state::ide_interrupt(int state)
{
	if (state == CLEAR_LINE)
	{
		m_sub_interrupt |= 0x80;
	}
	else
	{
		m_sub_interrupt &= ~0x80;
	}
}


INTERRUPT_GEN_MEMBER(cobra_state::cobra_vblank)
{
	if (m_vblank_enable & 0x80)
	{
		device.execute().set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		m_gfx_unk_flag = 0x80;
	}
}

void cobra_state::machine_start()
{
	/* configure fast RAM regions for DRC */
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x003fffff, false, m_main_ram);

	m_subcpu->ppcdrc_add_fastram(0x00000000, 0x003fffff, false, m_sub_ram);

	m_gfxcpu->ppcdrc_add_fastram(0x00000000, 0x003fffff, false, m_gfx_ram0);
	m_gfxcpu->ppcdrc_add_fastram(0x07c00000, 0x07ffffff, false, m_gfx_ram1);
}

void cobra_state::machine_reset()
{
	m_sub_interrupt = 0xff;

	ide_hdd_device *hdd = m_ata->subdevice<ata_slot_device>("0")->subdevice<ide_hdd_device>("hdd");
	uint16_t *identify_device = hdd->identify_device_buffer();

	// Cobra expects these settings or the BIOS fails
	identify_device[51] = 0x0200;        /* 51: PIO data transfer cycle timing mode */
	identify_device[67] = 0x01e0;        /* 67: minimum PIO transfer cycle time without flow control */

	m_renderer->gfx_reset();

	m_sound_dma_ptr = 0;

	m_dmadac[0]->enable(1);
	m_dmadac[1]->enable(1);
	m_dmadac[0]->set_frequency(44100);
	m_dmadac[1]->set_frequency(44100);
}

void cobra_state::cobra(machine_config &config)
{
	/* basic machine hardware */
	PPC603(config, m_maincpu, 100000000);      /* 603EV, 100? MHz */
	m_maincpu->set_bus_frequency(XTAL(66'666'700)); /* Multiplier 1.5, Bus = 66MHz, Core = 100MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &cobra_state::cobra_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(cobra_state::cobra_vblank));

	PPC403GA(config, m_subcpu, 32000000);      /* 403GA, 33? MHz */
	m_subcpu->set_serial_clock(XTAL(7'372'800)); // set serial clock to 7.3728MHz to allow for JVS comm at 115200 baud
	m_subcpu->set_addrmap(AS_PROGRAM, &cobra_state::cobra_sub_map);

	PPC604(config, m_gfxcpu, 100000000);       /* 604, 100? MHz */
	m_gfxcpu->set_bus_frequency(XTAL(66'666'700));   /* Multiplier 1.5, Bus = 66MHz, Core = 100MHz */
	m_gfxcpu->set_addrmap(AS_PROGRAM, &cobra_state::cobra_gfx_map);

	config.set_maximum_quantum(attotime::from_hz(55005));

	PCI_BUS_LEGACY(config, m_legacy_pci, 0, 0);
	m_legacy_pci->set_device(0, FUNC(cobra_state::mpc106_pci_r), FUNC(cobra_state::mpc106_pci_w));

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, true);
	m_ata->irq_handler().set(FUNC(cobra_state::ide_interrupt));

	/* video hardware */

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(1024, 1024);
	m_screen->set_visarea(40, 511+40, 31, 399+31);
	m_screen->set_screen_update(FUNC(cobra_state::screen_update_cobra));
	PALETTE(config, m_palette).set_entries(65536);

	SPEAKER(config, "speaker", 2).front();

	rf5c400_device &rfsnd(RF5C400(config, "rfsnd", XTAL(16'934'400)));
	rfsnd.set_addrmap(0, &cobra_state::rf5c400_map);
	rfsnd.add_route(0, "speaker", 1.0, 0);
	rfsnd.add_route(1, "speaker", 1.0, 1);

	DMADAC(config, m_dmadac[0]).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);

	DMADAC(config, m_dmadac[1]).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	M48T58(config, "m48t58", 0);

	K001604(config, m_k001604, 0);     // on the LAN board in Racing Jam DX
	m_k001604->set_palette(m_palette);

	KONPPC_JVS_HOST(config, m_jvs_host, 4000000);
	m_jvs_host->output_callback().set([this](uint8_t c) { m_subcpu->ppc4xx_spu_receive_byte(c); });
}

void cobra_state::bujutsu(machine_config &config)
{
	cobra(config);
	KONAMI_WINDY2_JVS_IO_2L12B_PANEL(config, "windy2_jvsio", 0, m_jvs_host);
}

void cobra_state::rf5c400_map(address_map& map)
{
	map(0x000000, 0xffffff).ram().share("rf5c400_ram");
}

/*****************************************************************************/

/*****************************************************************************/

void cobra_state::init_cobra()
{
	m_gfxfifo_in  = std::make_unique<cobra_fifo>(machine(),
								8192,
								"GFXFIFO_IN",
								GFXFIFO_IN_VERBOSE != 0,
								cobra_fifo::event_delegate(&cobra_state::gfxfifo_in_event_callback, this)
								);

	m_gfxfifo_out = std::make_unique<cobra_fifo>(machine(),
								8192,
								"GFXFIFO_OUT",
								GFXFIFO_OUT_VERBOSE != 0,
								cobra_fifo::event_delegate(&cobra_state::gfxfifo_out_event_callback, this)
								);

	m_m2sfifo     = std::make_unique<cobra_fifo>(machine(),
								2048,
								"M2SFIFO",
								M2SFIFO_VERBOSE != 0,
								cobra_fifo::event_delegate(&cobra_state::m2sfifo_event_callback, this)
								);

	m_s2mfifo     = std::make_unique<cobra_fifo>(machine(),
								2048,
								"S2MFIFO",
								S2MFIFO_VERBOSE != 0,
								cobra_fifo::event_delegate(&cobra_state::s2mfifo_event_callback, this)
								);

	m_maincpu->ppc_set_dcstore_callback(write32sm_delegate(*this, FUNC(cobra_state::main_cpu_dc_store)));

	m_gfxcpu->ppc_set_dcstore_callback(write32sm_delegate(*this, FUNC(cobra_state::gfx_cpu_dc_store)));

	m_subcpu->ppc4xx_set_dma_write_handler(0, write32sm_delegate(*this, FUNC(cobra_state::sub_sound_dma_w)), 44100);
	m_subcpu->ppc4xx_spu_set_tx_handler(write8smo_delegate(*this, FUNC(cobra_state::sub_jvs_w)));


	m_comram[0] = std::make_unique<uint32_t[]>(0x40000/4);
	m_comram[1] = std::make_unique<uint32_t[]>(0x40000/4);

	m_comram_page = 0;

	m_sound_dma_buffer_l = std::make_unique<int16_t[]>(DMA_SOUND_BUFFER_SIZE);
	m_sound_dma_buffer_r = std::make_unique<int16_t[]>(DMA_SOUND_BUFFER_SIZE);

	// setup fake pagetable until we figure out what really maps there...
	//m_gfx_pagetable[0x80 / 8] = 0x800001001e0001a8U;
	m_gfx_pagetable[0x80 / 8] = 0x80000100200001a8U;        // should this map to 0x1e000000?
}

void cobra_state::init_bujutsu()
{
	init_cobra();

	// rom hacks for sub board...
	{
		uint32_t *rom = (uint32_t*)memregion("user2")->base();

		rom[0x62094 / 4] = 0x60000000;          // skip hardcheck()...
	}


	// rom hacks for gfx board...
	{
		int i;
		uint32_t sum = 0;

		uint32_t *rom = (uint32_t*)memregion("user3")->base();

		rom[(0x022d4^4) / 4] = 0x60000000;      // skip init_raster() for now ...

		// calculate the checksum of the patched rom...
		for (i=0; i < 0x20000/4; i++)
		{
			sum += (uint8_t)((rom[i] >> 24) & 0xff);
			sum += (uint8_t)((rom[i] >> 16) & 0xff);
			sum += (uint8_t)((rom[i] >>  8) & 0xff);
			sum += (uint8_t)((rom[i] >>  0) & 0xff);
		}

		rom[(0x0001fff0^4) / 4] = sum;
		rom[(0x0001fff4^4) / 4] = ~sum;
	}



	// fill in M48T58 data for now...
	{
		uint8_t *rom = (uint8_t*)memregion("m48t58")->base();
		rom[0x00] = 0x47;       // G
		rom[0x01] = 0x4e;       // N        // N = 2-player, Q = 1-player?
		rom[0x02] = 0x36;       // 6
		rom[0x03] = 0x34;       // 4
		rom[0x04] = 0x35;       // 5
		rom[0x05] = 0x00;
		rom[0x06] = 0x00;
		rom[0x07] = 0x00;

		rom[0x08] = 0x00;
		rom[0x09] = 0x00;
		rom[0x0a] = 0x4a;       // J
		rom[0x0b] = 0x41;       // A
		rom[0x0c] = 0x41;       // A
		rom[0x0d] = 0x00;

		// calculate checksum
		uint16_t sum = 0;
		for (int i=0; i < 14; i+=2)
		{
			sum += ((uint16_t)(rom[i]) << 8) | (rom[i+1]);
		}
		sum ^= 0xffff;

		rom[0x0e] = (uint8_t)(sum >> 8);
		rom[0x0f] = (uint8_t)(sum);
	}

	// hd patches
	// (gfx)
	// 0x18932c = 0x38600000                    skips check_one_scene()

	// (sub)
	// 0x2d3568 = 0x60000000 [0x4082001c]       skip IRQ fail

	// (main)
	// 0x5025ac = 0x60000000 [0x4082055c]       skip IRQ fail...
	// 0x503ec4 = 0x60000000 [0x4186fff8]
	// 0x503f00 = 0x60000000 [0x4186fff8]

	m_has_psac = false;
}

void cobra_state::init_racjamdx()
{
	init_cobra();

	// rom hacks for sub board...
	{
		uint32_t *rom = (uint32_t*)memregion("user2")->base();

		rom[0x62094 / 4] = 0x60000000;          // skip hardcheck()...
		rom[0x62ddc / 4] = 0x60000000;          // skip lanc_hardcheck()


		// calculate the checksum of the patched rom...
		uint32_t sum = 0;
		for (int i=0; i < 0x20000/4; i++)
		{
			sum += (uint8_t)((rom[(0x60000/4)+i] >> 24) & 0xff);
			sum += (uint8_t)((rom[(0x60000/4)+i] >> 16) & 0xff);
			sum += (uint8_t)((rom[(0x60000/4)+i] >>  8) & 0xff);
			sum += (uint8_t)((rom[(0x60000/4)+i] >>  0) & 0xff);
		}

		rom[(0x0007fff0^4) / 4] = ~sum;
		rom[(0x0007fff4^4) / 4] = sum;
	}


	// rom hacks for gfx board...
	{
		uint32_t sum = 0;

		uint32_t *rom = (uint32_t*)memregion("user3")->base();

		rom[(0x02448^4) / 4] = 0x60000000;      // skip init_raster() for now ...

		rom[(0x02438^4) / 4] = 0x60000000;      // awfully long delay loop (5000000 * 166)

		// calculate the checksum of the patched rom...
		for (int i = 0; i < 0x20000/4; i++)
		{
			sum += (uint8_t)((rom[i] >> 24) & 0xff);
			sum += (uint8_t)((rom[i] >> 16) & 0xff);
			sum += (uint8_t)((rom[i] >>  8) & 0xff);
			sum += (uint8_t)((rom[i] >>  0) & 0xff);
		}

		rom[(0x0001fff0^4) / 4] = sum;
		rom[(0x0001fff4^4) / 4] = ~sum;
	}


	// fill in M48T58 data for now...
	{
		uint8_t *rom = (uint8_t*)memregion("m48t58")->base();
		rom[0x00] = 0x47;       // G
		rom[0x01] = 0x59;       // Y
		rom[0x02] = 0x36;       // 6
		rom[0x03] = 0x37;       // 7
		rom[0x04] = 0x36;       // 6
		rom[0x05] = 0x00;
		rom[0x06] = 0x00;
		rom[0x07] = 0x00;

		// calculate checksum
		uint16_t sum = 0;
		for (int i=0; i < 14; i+=2)
		{
			sum += ((uint16_t)(rom[i]) << 8) | (rom[i+1]);
		}
		sum ^= 0xffff;

		rom[0x0e] = (uint8_t)(sum >> 8);
		rom[0x0f] = (uint8_t)(sum);
	}

	// hd patches
	// (gfx)
	// 0x144354 = 0x38600000 [0x4bfffb91]       skips check_one_scene()

	// (sub)
	// 0x2a5394 = 0x4800001c [0x4182001c]       sound chip check?
	// 0x2a53f4 = 0x4800001c [0x4082001c]       ?
	// 0x2a546c = 0x60000000 [0x48001a0d]       ?
	// 0x2a5510 = 0x48000014 [0x419e0014]       ?

	// (main)
	// 0x14aa48 = 0x60000000 [0x4182fff4]       ?

	m_has_psac = true;
}

/*****************************************************************************/

ROM_START(bujutsu)
	ROM_REGION64_BE(0x80000, "user1", 0)        /* Main CPU program (PPC603) */
	ROM_LOAD("645a01.33d", 0x00000, 0x80000, CRC(cb1a8683) SHA1(77b7dece84dc17e9d63242347b7202e879b9a10e) )

	ROM_REGION32_BE(0x80000, "user2", 0)        /* Sub CPU program (PPC403) */
	ROM_LOAD("645a02.24r", 0x00000, 0x80000, CRC(7d1c31bd) SHA1(94907c4068a488a74b2fa9a486c832d380c5b184) )

	ROM_REGION64_BE(0x80000, "user3", 0)        /* Gfx CPU program (PPC604) */
	ROM_LOAD("645a03.u17", 0x00000, 0x80000, CRC(086abd0b) SHA1(24df439eb9828ed3842f43f5f4014a3fc746e1e3) )

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)
	ROM_LOAD( "m48t58-70pc1.17l", 0x000000, 0x002000, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "645c04", 0, SHA1(c0aabe69f6eb4e4cf748d606ae50674297af6a04) )
ROM_END

ROM_START(racjamdx)
	ROM_REGION64_BE(0x80000, "user1", 0)        /* Main CPU program (PPC603) */
	ROM_LOAD( "676a01.33d", 0x000000, 0x080000, CRC(1e6238f1) SHA1(d55949d98e9e290ceb8c018ed60ca090ec16c9dd) )

	ROM_REGION32_BE(0x80000, "user2", 0)        /* Sub CPU program (PPC403) */
	ROM_LOAD( "676a02.24r", 0x000000, 0x080000, CRC(371978ed) SHA1(c83f0cf04204212db00588df91b32122f37900f8) )

	ROM_REGION64_BE(0x80000, "user3", 0)        /* Gfx CPU program (PPC604) */
	ROM_LOAD( "676a03.u17", 0x000000, 0x080000, CRC(66f77cbd) SHA1(f1c7e50dbbfcc27ac011cbbb8ad2fd376c2e9056) )

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)
	ROM_LOAD( "m48t58-70pc1.17l", 0x000000, 0x002000, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "676a04", 0, SHA1(8e89d3e5099e871b99fccba13adaa3cf8a6b71f0) )
ROM_END

} // anonymous namespace


/*************************************************************************/

GAME( 1997, bujutsu,  0, bujutsu, cobra, cobra_state, init_bujutsu,  ROT0, "Konami", "Fighting Wu-Shu 2nd! (ver JAA)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_TIMING )
GAME( 1997, racjamdx, 0, cobra,   cobra, cobra_state, init_racjamdx, ROT0, "Konami", "Racing Jam DX", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_TIMING | MACHINE_NODEVICE_LAN )
