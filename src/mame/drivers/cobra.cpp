// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*  Konami Cobra System

    Driver by Ville Linde


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



    GFX Registers:

        0x00090:        Viewport width / 2?
        0x0009c:        Viewport center X
        0x000a4:        Viewport height / 2?
        0x000ac:        Viewport center Y

        0x00114:        High word: framebuffer pitch?   Low word: framebuffer pixel size?

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
#include "cpu/powerpc/ppc.h"
#include "machine/lpci.h"
#include "machine/ataintf.h"
#include "machine/idehd.h"
#include "machine/jvshost.h"
#include "machine/jvsdev.h"
#include "machine/timekpr.h"
#include "video/k001604.h"
#include "video/poly.h"
#include "video/rgbutil.h"
#include "sound/rf5c400.h"
#include "sound/dmadac.h"

#define GFXFIFO_IN_VERBOSE          0
#define GFXFIFO_OUT_VERBOSE         0
#define M2SFIFO_VERBOSE             0
#define S2MFIFO_VERBOSE             0

#define LOG_DEBUG_STATES            0
#define LOG_JVS                     0
#define LOG_GFX_RAM_WRITES          0
#define LOG_DRAW_COMMANDS           0

#define ENABLE_BILINEAR             1

#define DMA_SOUND_BUFFER_SIZE       16000


/* Cobra Renderer class */

struct cobra_polydata
{
	UINT32 alpha_test;
	UINT32 zmode;
	UINT32 tex_format;
	UINT32 tex_address;
};

class cobra_renderer : public poly_manager<float, cobra_polydata, 8, 10000>
{
public:
	cobra_renderer(screen_device &screen)
		: poly_manager<float, cobra_polydata, 8, 10000>(screen)
	{
		m_texture_ram = std::make_unique<UINT32[]>(0x100000);

		m_framebuffer = std::make_unique<bitmap_rgb32>( 1024, 1024);
		m_backbuffer = std::make_unique<bitmap_rgb32>( 1024, 1024);
		m_overlay = std::make_unique<bitmap_rgb32>( 1024, 1024);
		m_zbuffer = std::make_unique<bitmap_ind32>(1024, 1024);
		m_stencil = std::make_unique<bitmap_ind32>(1024, 1024);

		m_gfx_regmask = std::make_unique<UINT32[]>(0x100);
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

	void gfx_init();
	void gfx_exit();
	void gfx_reset();
	void gfx_fifo_exec();
	UINT32 gfx_read_gram(UINT32 address);
	void gfx_write_gram(UINT32 address, UINT32 mask, UINT32 data);
	UINT64 gfx_read_reg();
	void gfx_write_reg(UINT64 data);

	void display(bitmap_rgb32 *bitmap, const rectangle &cliprect);
	inline rgb_t texture_fetch(UINT32 *texture, int u, int v, int width, int format);
private:
	std::unique_ptr<bitmap_rgb32> m_framebuffer;
	std::unique_ptr<bitmap_rgb32> m_backbuffer;
	std::unique_ptr<bitmap_rgb32> m_overlay;
	std::unique_ptr<bitmap_ind32> m_zbuffer;
	std::unique_ptr<bitmap_ind32> m_stencil;

	std::unique_ptr<UINT32[]> m_texture_ram;

	std::unique_ptr<UINT32[]> m_gfx_gram;
	std::unique_ptr<UINT32[]> m_gfx_regmask;

	UINT32 m_gfx_register_select;
	std::unique_ptr<UINT64[]> m_gfx_register;

	UINT32 m_texram_ptr;

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
		m_data = std::make_unique<UINT64[]>(capacity);

		m_name = name;
		m_size = capacity;
		m_wpos = 0;
		m_rpos = 0;
		m_num = 0;

		m_verbose = verbose;

		m_event_callback = event_callback;
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
	std::unique_ptr<UINT64[]> m_data;
	event_delegate m_event_callback;
};


/* Cobra JVS Device class */

class cobra_jvs : public jvs_device
{
public:
	cobra_jvs(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER(coin_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_2_w);

protected:
	virtual bool switches(UINT8 *&buf, UINT8 count_players, UINT8 bytes_per_switch) override;
	virtual bool coin_counters(UINT8 *&buf, UINT8 count) override;
	virtual void function_list(UINT8 *&buf) override;

private:
	int m_coin_counter[2];
};

const device_type COBRA_JVS = &device_creator<cobra_jvs>;

cobra_jvs::cobra_jvs(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: jvs_device(mconfig, COBRA_JVS, "JVS (COBRA)", tag, owner, clock, "cobra_jvs", __FILE__)
{
	m_coin_counter[0] = 0;
	m_coin_counter[1] = 0;
}

WRITE_LINE_MEMBER(cobra_jvs::coin_1_w)
{
	if(state)
		m_coin_counter[0]++;
}

WRITE_LINE_MEMBER(cobra_jvs::coin_2_w)
{
	if(state)
		m_coin_counter[1]++;
}

void cobra_jvs::function_list(UINT8 *&buf)
{
	// SW input - 2 players, 13 bits
	*buf++ = 0x01; *buf++ = 2; *buf++ = 13; *buf++ = 0;

	// Coin input - 2 slots
	*buf++ = 0x02; *buf++ = 2; *buf++ = 0; *buf++ = 0;

	// Analog input - 8 channels
	*buf++ = 0x03; *buf++ = 8; *buf++ = 16; *buf++ = 0;

	// Driver out - 6 channels
	*buf++ = 0x12; *buf++ = 6; *buf++ = 0; *buf++ = 0;
}

bool cobra_jvs::switches(UINT8 *&buf, UINT8 count_players, UINT8 bytes_per_switch)
{
#if LOG_JVS
	printf("jvs switch read: num players %d, bytes %d\n", count_players, bytes_per_switch);
#endif

	if (count_players > 2 || bytes_per_switch > 2)
		return false;

	static const char* player_ports[2] = { ":P1", ":P2" };

	*buf++ = read_safe(ioport(":TEST"), 0);

	for (int i=0; i < count_players; i++)
	{
		UINT32 pval = read_safe(ioport(player_ports[i]), 0);
		for (int j=0; j < bytes_per_switch; j++)
		{
			*buf++ = (UINT8)(pval >> ((1-j) * 8));
		}
	}
	return true;
}

bool cobra_jvs::coin_counters(UINT8 *&buf, UINT8 count)
{
#if LOG_JVS
	printf("jvs coin counter read: count %d\n", count);
#endif

	if (count > 2)
		return false;

	*buf++ = m_coin_counter[0] >> 8; *buf++ = m_coin_counter[0];

	if(count > 1)
		*buf++ = m_coin_counter[1] >> 8; *buf++ = m_coin_counter[1];

	return true;
}


class cobra_jvs_host : public jvs_host
{
public:
	cobra_jvs_host(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void write(UINT8, const UINT8 *&rec_data, UINT32 &rec_size);

private:
	UINT8 m_send[512];
	int m_send_ptr;
};

const device_type COBRA_JVS_HOST = &device_creator<cobra_jvs_host>;

cobra_jvs_host::cobra_jvs_host(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: jvs_host(mconfig, COBRA_JVS_HOST, "JVS-HOST (COBRA)", tag, owner, clock, "cobra_jvs_host", __FILE__)
{
	m_send_ptr = 0;
}

void cobra_jvs_host::write(UINT8 data, const UINT8 *&rec_data, UINT32 &rec_size)
{
	m_send[m_send_ptr++] = data;
	push(data);

	if (m_send[0] == 0xe0)
	{
		if (m_send_ptr > 2)
		{
			UINT8 length = m_send[2];
			if (length == 0xff)
				length = 4;
			else
				length = length + 3;

			if (m_send_ptr >= length)
			{
				commit_encoded();

				get_encoded_reply(rec_data, rec_size);

				m_send_ptr = 0;
				return;
			}
		}
	}
	else
	{
		m_send_ptr = 0;
	}

	rec_data = nullptr;
	rec_size = 0;
}


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
	required_shared_ptr<UINT64> m_gfx_pagetable;
	required_device<k001604_device> m_k001604;
	required_device<ata_interface_device> m_ata;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT32> m_generic_paletteram_32;
	required_shared_ptr<UINT64> m_main_ram;
	required_shared_ptr<UINT32> m_sub_ram;
	required_shared_ptr<UINT64> m_gfx_ram0;
	required_shared_ptr<UINT64> m_gfx_ram1;

	DECLARE_READ64_MEMBER(main_comram_r);
	DECLARE_WRITE64_MEMBER(main_comram_w);
	DECLARE_READ64_MEMBER(main_fifo_r);
	DECLARE_WRITE64_MEMBER(main_fifo_w);
	DECLARE_READ64_MEMBER(main_mpc106_r);
	DECLARE_WRITE64_MEMBER(main_mpc106_w);
	DECLARE_WRITE32_MEMBER(main_cpu_dc_store);

	DECLARE_READ32_MEMBER(sub_comram_r);
	DECLARE_WRITE32_MEMBER(sub_comram_w);
	DECLARE_READ32_MEMBER(sub_unk7e_r);
	DECLARE_WRITE32_MEMBER(sub_debug_w);
	DECLARE_READ32_MEMBER(sub_unk1_r);
	DECLARE_WRITE32_MEMBER(sub_unk1_w);
	DECLARE_READ32_MEMBER(sub_config_r);
	DECLARE_WRITE32_MEMBER(sub_config_w);
	DECLARE_READ32_MEMBER(sub_mainbd_r);
	DECLARE_WRITE32_MEMBER(sub_mainbd_w);
	DECLARE_READ16_MEMBER(sub_ata0_r);
	DECLARE_WRITE16_MEMBER(sub_ata0_w);
	DECLARE_READ16_MEMBER(sub_ata1_r);
	DECLARE_WRITE16_MEMBER(sub_ata1_w);
	DECLARE_READ32_MEMBER(sub_psac2_r);
	DECLARE_WRITE32_MEMBER(sub_psac2_w);
	DECLARE_WRITE32_MEMBER(sub_psac_palette_w);
	DECLARE_WRITE32_MEMBER(sub_sound_dma_w);

	DECLARE_WRITE64_MEMBER(gfx_fifo0_w);
	DECLARE_WRITE64_MEMBER(gfx_fifo1_w);
	DECLARE_WRITE64_MEMBER(gfx_fifo2_w);
	DECLARE_WRITE64_MEMBER(gfx_debug_state_w);
	DECLARE_READ64_MEMBER(gfx_unk1_r);
	DECLARE_WRITE64_MEMBER(gfx_unk1_w);
	DECLARE_READ64_MEMBER(gfx_fifo_r);
	DECLARE_WRITE64_MEMBER(gfx_buf_w);
	DECLARE_WRITE32_MEMBER(gfx_cpu_dc_store);

	DECLARE_WRITE8_MEMBER(sub_jvs_w);

	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);

	std::unique_ptr<cobra_renderer> m_renderer;

	cobra_fifo *m_gfxfifo_in;
	cobra_fifo *m_gfxfifo_out;
	cobra_fifo *m_m2sfifo;
	cobra_fifo *m_s2mfifo;

	void gfxfifo_in_event_callback(cobra_fifo::EventType event);
	void gfxfifo_out_event_callback(cobra_fifo::EventType event);
	void m2sfifo_event_callback(cobra_fifo::EventType event);
	void s2mfifo_event_callback(cobra_fifo::EventType event);

	enum
	{
		MAIN_INT_M2S = 0x01,
		MAIN_INT_S2M = 0x02
	};

	UINT8 m_m2s_int_enable;
	UINT8 m_s2m_int_enable;
	UINT8 m_vblank_enable;

	UINT8 m_m2s_int_mode;
	UINT8 m_s2m_int_mode;

	UINT8 m_main_int_active;


	std::unique_ptr<UINT32[]> m_comram[2];
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

	UINT64 m_gfx_fifo_mem[256];
	int m_gfx_fifo_cache_addr;
	int m_gfx_fifo_loopback;
	int m_gfx_unknown_v1;
	int m_gfx_status_byte;

	bool m_has_psac;

	std::unique_ptr<INT16[]> m_sound_dma_buffer_l;
	std::unique_ptr<INT16[]> m_sound_dma_buffer_r;
	UINT32 m_sound_dma_ptr;

	dmadac_sound_device *m_dmadac[2];

	DECLARE_DRIVER_INIT(racjamdx);
	DECLARE_DRIVER_INIT(bujutsu);
	DECLARE_DRIVER_INIT(cobra);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_cobra(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cobra_vblank);
	void cobra_video_exit();
	int decode_debug_state_value(int v);
};

void cobra_renderer::render_color_scan(INT32 scanline, const extent_t &extent, const cobra_polydata &extradata, int threadid)
{
	UINT32 *fb = &m_backbuffer->pix32(scanline);
	float *zb = (float*)&m_zbuffer->pix32(scanline);

	float z = extent.param[POLY_Z].start;
	float dz = extent.param[POLY_Z].dpdx;

	float gr = extent.param[POLY_R].start;
	float dgr = extent.param[POLY_R].dpdx;
	float gg = extent.param[POLY_G].start;
	float dgg = extent.param[POLY_G].dpdx;
	float gb = extent.param[POLY_B].start;
	float dgb = extent.param[POLY_B].dpdx;
	float ga = extent.param[POLY_A].start;
	float dga = extent.param[POLY_A].dpdx;

	UINT32 zmode = extradata.zmode;

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (z <= zb[x] || zmode == 7)
		{
			UINT32 r = (int)(gr);
			UINT32 g = (int)(gg);
			UINT32 b = (int)(gb);

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

rgb_t cobra_renderer::texture_fetch(UINT32 *texture, int u, int v, int width, int format)
{
	UINT32 texel = texture[((v * width) + u) / 2];

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

void cobra_renderer::render_texture_scan(INT32 scanline, const extent_t &extent, const cobra_polydata &extradata, int threadid)
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
	float ga = extent.param[POLY_A].start;
	float dga = extent.param[POLY_A].dpdx;

	UINT32 *fb = &m_backbuffer->pix32(scanline);
	float *zb = (float*)&m_zbuffer->pix32(scanline);

	UINT32 texture_width  = 1 << ((extradata.tex_format >> 28) & 0xf);
	UINT32 texture_height = 1 << ((extradata.tex_format >> 24) & 0xf);
	UINT32 tex_address = extradata.tex_address;

	UINT32 alpha_test = extradata.alpha_test;
	UINT32 zmode = extradata.zmode;
	UINT32 tex_format = (extradata.tex_format >> 2) & 0x7;

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
				UINT32 gour = (int)(gr);
				UINT32 goug = (int)(gg);
				UINT32 goub = (int)(gb);

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

void cobra_renderer::draw_point(const rectangle &visarea, vertex_t &v, UINT32 color)
{
	int x = v.x;
	int y = v.y;

	if (x >= visarea.min_x && x <= visarea.max_x &&
		y >= visarea.min_y && y <= visarea.max_y)
	{
		UINT32 *fb = &m_backbuffer->pix32(y);
		fb[x] = color;
	}
}

void cobra_renderer::draw_line(const rectangle &visarea, vertex_t &v1, vertex_t &v2)
{
	int dx = (v2.x - v1.x);
	int dy = (v2.y - v1.y);

	int x1 = v1.x;
	int y1 = v1.y;

	UINT32 color = 0xffffffff;      // TODO: where does the color come from?

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

			UINT32 *fb = &m_backbuffer->pix32(y);
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

			UINT32 *fb = &m_backbuffer->pix32(y);
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
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(cobra_state::cobra_video_exit), this));

	m_renderer = std::make_unique<cobra_renderer>(*m_screen);
	m_renderer->gfx_init();
}

UINT32 cobra_state::screen_update_cobra(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_has_psac)
	{
		m_k001604->draw_back_layer(bitmap, cliprect);
		m_k001604->draw_front_layer(screen, bitmap, cliprect);
	}

	m_renderer->display(&bitmap, cliprect);
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


void cobra_fifo::push(const device_t *cpu, UINT64 data)
{
	if (m_verbose)
	{
		char accessor_location[50];
		if (cpu != nullptr)
		{
			// cpu has a name and a PC
			sprintf(accessor_location, "(%s) %08X", cpu->tag(), const_cast<device_t *>(cpu)->safe_pc());
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
			if (cpu != nullptr)
			{
				// cpu has a name and a PC
				sprintf(accessor_location, "(%s) %08X", cpu->tag(), const_cast<device_t *>(cpu)->safe_pc());
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

	if (m_num >= m_size)
		m_event_callback(EVENT_FULL);
	if (m_num == (m_size / 2))
		m_event_callback(EVENT_HALF_FULL);
}

bool cobra_fifo::pop(const device_t *cpu, UINT64 *result)
{
	UINT64 r;

	if (m_num == 0)
	{
		if (m_verbose)
		{
			char accessor_location[50];
			if (cpu != nullptr)
			{
				// cpu has a name and a PC
				sprintf(accessor_location, "(%s) %08X", cpu->tag(), const_cast<device_t *>(cpu)->safe_pc());
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
		if (cpu != nullptr)
		{
			// cpu has a name and a PC
			sprintf(accessor_location, "(%s) %08X", cpu->tag(), const_cast<device_t *>(cpu)->safe_pc());
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

	if (m_num == 0)
		m_event_callback(EVENT_EMPTY);
	if (m_num == (m_size / 2))
		m_event_callback(EVENT_HALF_FULL);

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

		r |= (UINT64)(value & 0xff) << 40;
	}
	if (ACCESSING_BITS_32_39)
	{
		// Register 0xffff0003:
		//
		// 7 6 5 4 3 2 1 0
		//----------------
		//             x     S2M FIFO interrupt active
		//           x       Graphics board/FIFO busy flag
		//         x         M2S FIFO interrupt active

		int value = 0x01;

		value |= (m_main_int_active & MAIN_INT_S2M) ? 0x00 : 0x02;
		value |= (m_main_int_active & MAIN_INT_M2S) ? 0x00 : 0x08;
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

		m_m2sfifo->push(&space.device(), (UINT8)(data >> 40));

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

		m_vblank_enable = (UINT8)(data >> 24);

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

		m_s2m_int_enable = (UINT8)(data >> 16);

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

		printf("main_fifo_w: 0xffff0006: %02X\n", (UINT8)(data >> 8));
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
#if LOG_DEBUG_STATES
		if (m_main_debug_state != 0)
		{
			printf("MAIN: debug state %02X\n", m_main_debug_state);
		}
#endif

		if (m_main_debug_state == 0x6b)
		{
			// install HD patches for bujutsu
			if (strcmp(space.machine().system().name, "bujutsu") == 0)
			{
				UINT32 *main_ram = (UINT32*)(UINT64*)m_main_ram;
				UINT32 *sub_ram = (UINT32*)m_sub_ram;
				UINT32 *gfx_ram = (UINT32*)(UINT64*)m_gfx_ram0;

				main_ram[(0x0005ac^4) / 4] = 0x60000000;        // skip IRQ fail
				main_ram[(0x001ec4^4) / 4] = 0x60000000;        // waiting for IRQ?
				main_ram[(0x001f00^4) / 4] = 0x60000000;        // waiting for IRQ?

				sub_ram[0x568 / 4] = 0x60000000;                // skip IRQ fail

				gfx_ram[(0x38632c^4) / 4] = 0x38600000;     // skip check_one_scene()
			}
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

WRITE32_MEMBER(cobra_state::main_cpu_dc_store)
{
	if ((offset & 0xf0000000) == 0xc0000000)
	{
		// force sync when writing to GFX board main ram
		m_maincpu->spin_until_time(attotime::from_usec(80));
	}
}

static ADDRESS_MAP_START( cobra_main_map, AS_PROGRAM, 64, cobra_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("main_ram")
	AM_RANGE(0x07c00000, 0x07ffffff) AM_RAM
	AM_RANGE(0x80000cf8, 0x80000cff) AM_READWRITE(main_mpc106_r, main_mpc106_w)
	AM_RANGE(0xc0000000, 0xc03fffff) AM_RAM AM_SHARE("gfx_main_ram_0")              // GFX board main ram, bank 0
	AM_RANGE(0xc7c00000, 0xc7ffffff) AM_RAM AM_SHARE("gfx_main_ram_1")              // GFX board main ram, bank 1
	AM_RANGE(0xfff00000, 0xfff7ffff) AM_ROM AM_REGION("user1", 0)                   /* Boot ROM */
	AM_RANGE(0xfff80000, 0xfffbffff) AM_READWRITE(main_comram_r, main_comram_w)
	AM_RANGE(0xffff0000, 0xffff0007) AM_READWRITE(main_fifo_r, main_fifo_w)
ADDRESS_MAP_END


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

		UINT64 value = 0;
		m_m2sfifo->pop(&space.device(), &value);

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
#if LOG_DEBUG_STATES
		if (m_sub_debug_state != 0)
		{
			printf("SUB: debug state %02X\n", m_sub_debug_state);
		}
#endif

		m_sub_debug_state = 0;
		m_sub_debug_state_wc = 0;
	}
}

READ32_MEMBER(cobra_state::sub_config_r)
{
	UINT32 r = 0;

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

WRITE32_MEMBER(cobra_state::sub_config_w)
{
}

READ16_MEMBER(cobra_state::sub_ata0_r)
{
	mem_mask = ( mem_mask << 8 ) | ( mem_mask >> 8 );

	UINT32 data = m_ata->read_cs0(space, offset, mem_mask);
	data = ( data << 8 ) | ( data >> 8 );

	return data;
}

WRITE16_MEMBER(cobra_state::sub_ata0_w)
{
	mem_mask = ( mem_mask << 8 ) | ( mem_mask >> 8 );
	data = ( data << 8 ) | ( data >> 8 );

	m_ata->write_cs0(space, offset, data, mem_mask);
}

READ16_MEMBER(cobra_state::sub_ata1_r)
{
	mem_mask = ( mem_mask << 8 ) | ( mem_mask >> 8 );

	UINT32 data = m_ata->read_cs1(space, offset, mem_mask);

	return ( data << 8 ) | ( data >> 8 );
}

WRITE16_MEMBER(cobra_state::sub_ata1_w)
{
	mem_mask = ( mem_mask << 8 ) | ( mem_mask >> 8 );
	data = ( data << 8 ) | ( data >> 8 );

	m_ata->write_cs1(space, offset, data, mem_mask);
}

READ32_MEMBER(cobra_state::sub_comram_r)
{
	int page = m_comram_page ^ 1;

	return m_comram[page][offset];
}

WRITE32_MEMBER(cobra_state::sub_comram_w)
{
	int page = m_comram_page ^ 1;

	COMBINE_DATA(m_comram[page].get() + offset);
}

WRITE32_MEMBER(cobra_state::sub_psac_palette_w)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	data = m_generic_paletteram_32[offset];
	m_palette->set_pen_color(offset, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
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

WRITE32_MEMBER(cobra_state::sub_sound_dma_w)
{
	//printf("DMA write to unknown: size %d, data %08X\n", address, data);

	/*
	static FILE *out;
	if (out == NULL)
	    out = fopen("sound.bin", "wb");

	fputc((data >> 24) & 0xff, out);
	fputc((data >> 16) & 0xff, out);
	fputc((data >> 8) & 0xff, out);
	fputc((data >> 0) & 0xff, out);
	*/

	INT16 ldata = (INT16)(data >> 16);
	INT16 rdata = (INT16)(data);

	m_sound_dma_buffer_l[m_sound_dma_ptr] = ldata;
	m_sound_dma_buffer_r[m_sound_dma_ptr] = rdata;
	m_sound_dma_ptr++;

	if (m_sound_dma_ptr >= DMA_SOUND_BUFFER_SIZE)
	{
		m_sound_dma_ptr = 0;

		dmadac_transfer(&m_dmadac[0], 1, 0, 1, DMA_SOUND_BUFFER_SIZE, m_sound_dma_buffer_l.get());
		dmadac_transfer(&m_dmadac[1], 1, 0, 1, DMA_SOUND_BUFFER_SIZE, m_sound_dma_buffer_r.get());
	}
}

WRITE8_MEMBER(cobra_state::sub_jvs_w)
{
	cobra_jvs_host *jvs = machine().device<cobra_jvs_host>("cobra_jvs_host");

#if LOG_JVS
	printf("sub_jvs_w: %02X\n", data);
#endif

	const UINT8 *rec_data;
	UINT32 rec_size;

	jvs->write(data, rec_data, rec_size);

	if (rec_size > 0)
	{
#if LOG_JVS
		printf("jvs reply ");
		for (int i=0; i < rec_size; i++)
		{
			printf("%02X ", rec_data[i]);
		}
		printf("\n");
#endif

		for (int i=0; i < rec_size; i++)
		{
			m_subcpu->ppc4xx_spu_receive_byte(rec_data[i]);
		}
	}
}

static ADDRESS_MAP_START( cobra_sub_map, AS_PROGRAM, 32, cobra_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_MIRROR(0x80000000) AM_RAM AM_SHARE("sub_ram")                       // Main RAM
	AM_RANGE(0x70000000, 0x7003ffff) AM_MIRROR(0x80000000) AM_READWRITE(sub_comram_r, sub_comram_w)         // Double buffered shared RAM between Main and Sub
//  AM_RANGE(0x78000000, 0x780000ff) AM_MIRROR(0x80000000) AM_NOP                                           // SCSI controller (unused)
	AM_RANGE(0x78040000, 0x7804ffff) AM_MIRROR(0x80000000) AM_DEVREADWRITE16("rfsnd", rf5c400_device, rf5c400_r, rf5c400_w, 0xffffffff)
	AM_RANGE(0x78080000, 0x7808000f) AM_MIRROR(0x80000000) AM_READWRITE16(sub_ata0_r, sub_ata0_w, 0xffffffff)
	AM_RANGE(0x780c0010, 0x780c001f) AM_MIRROR(0x80000000) AM_READWRITE16(sub_ata1_r, sub_ata1_w, 0xffffffff)
	AM_RANGE(0x78200000, 0x782000ff) AM_MIRROR(0x80000000) AM_DEVREADWRITE("k001604", k001604_device, reg_r, reg_w)              // PSAC registers
	AM_RANGE(0x78210000, 0x78217fff) AM_MIRROR(0x80000000) AM_RAM_WRITE(sub_psac_palette_w) AM_SHARE("paletteram")                      // PSAC palette RAM
	AM_RANGE(0x78220000, 0x7823ffff) AM_MIRROR(0x80000000) AM_DEVREADWRITE("k001604", k001604_device, tile_r, tile_w)            // PSAC tile RAM
	AM_RANGE(0x78240000, 0x7827ffff) AM_MIRROR(0x80000000) AM_DEVREADWRITE("k001604", k001604_device, char_r, char_w)            // PSAC character RAM
	AM_RANGE(0x78280000, 0x7828000f) AM_MIRROR(0x80000000) AM_NOP                                           // ???
	AM_RANGE(0x78300000, 0x7830000f) AM_MIRROR(0x80000000) AM_READWRITE(sub_psac2_r, sub_psac2_w)           // PSAC
	AM_RANGE(0x7e000000, 0x7e000003) AM_MIRROR(0x80000000) AM_READWRITE(sub_unk7e_r, sub_debug_w)
	AM_RANGE(0x7e040000, 0x7e041fff) AM_MIRROR(0x80000000) AM_DEVREADWRITE8("m48t58", timekeeper_device, read, write, 0xffffffff)    /* M48T58Y RTC/NVRAM */
	AM_RANGE(0x7e180000, 0x7e180003) AM_MIRROR(0x80000000) AM_READWRITE(sub_unk1_r, sub_unk1_w)             // TMS57002?
	AM_RANGE(0x7e200000, 0x7e200003) AM_MIRROR(0x80000000) AM_READWRITE(sub_config_r, sub_config_w)
	AM_RANGE(0x7e280000, 0x7e28ffff) AM_MIRROR(0x80000000) AM_NOP                                           // LANC
	AM_RANGE(0x7e300000, 0x7e30ffff) AM_MIRROR(0x80000000) AM_NOP                                           // LANC
	AM_RANGE(0x7e380000, 0x7e380003) AM_MIRROR(0x80000000) AM_READWRITE(sub_mainbd_r, sub_mainbd_w)
	AM_RANGE(0x7ff80000, 0x7fffffff) AM_MIRROR(0x80000000) AM_ROM AM_REGION("user2", 0)                     /* Boot ROM */
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


void cobra_renderer::display(bitmap_rgb32 *bitmap, const rectangle &cliprect)
{
	if (m_gfx_register[0] & 0x4)
	{
		copybitmap_trans(*bitmap, *m_framebuffer, 0, 0, 0, 0, cliprect, 0);
	}
	else
	{
		copybitmap_trans(*bitmap, *m_backbuffer, 0, 0, 0, 0, cliprect, 0);
	}
}

void cobra_renderer::gfx_init()
{
	const rectangle& visarea = screen().visible_area();

	m_gfx_gram = std::make_unique<UINT32[]>(0x40000);

	m_gfx_register = std::make_unique<UINT64[]>(0x3000);
	m_gfx_register_select = 0;

	float zvalue = 10000000.0f;
	m_zbuffer->fill(*(int*)&zvalue, visarea);
}

void cobra_renderer::gfx_exit()
{
	/*
	FILE *file;
	file = fopen("texture_ram.bin","wb");
	for (int i=0; i < 0x100000; i++)
	{
	    fputc((UINT8)(m_texture_ram[i] >> 24), file);
	    fputc((UINT8)(m_texture_ram[i] >> 16), file);
	    fputc((UINT8)(m_texture_ram[i] >> 8), file);
	    fputc((UINT8)(m_texture_ram[i] >> 0), file);
	}
	fclose(file);
	*/
}

void cobra_renderer::gfx_reset()
{
	cobra_state *cobra = machine().driver_data<cobra_state>();

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
		case 0xc:       // 0xCxxxx
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
	switch ((address >> 16) & 0xf)
	{
		case 0x4:       // 0x4xxxx
		{
			if (address == 0x40fff)
			{
				printf("gfx: reg 40fff = %d, %d\n", (UINT16)(data >> 16), (UINT16)(data));
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

	if (address & 3)
	{
		printf("gfx_write_gram: %08X, %08X, not dword aligned!\n", address, data);
		return;
	}

	m_gfx_gram[address/4] &= ~mask;
	m_gfx_gram[address/4] |= data & mask;
}

UINT64 cobra_renderer::gfx_read_reg()
{
	return m_gfx_register[m_gfx_register_select];
}

void cobra_renderer::gfx_write_reg(UINT64 data)
{
	switch (m_gfx_register_select)
	{
		case 0x0000:
		{
			const rectangle& visarea = screen().visible_area();

			copybitmap_trans(*m_framebuffer, *m_backbuffer, 0, 0, 0, 0, visarea, 0);
			m_backbuffer->fill(0xff000000, visarea);

			float zvalue = 10000000.0f;
			m_zbuffer->fill(*(int*)&zvalue, visarea);
			break;
		}
	}

	m_gfx_register[m_gfx_register_select] = data;
}

void cobra_renderer::gfx_fifo_exec()
{
	cobra_state *cobra = machine().driver_data<cobra_state>();

	if (cobra->m_gfx_fifo_loopback != 0)
		return;

	const rectangle& visarea = screen().visible_area();
	vertex_t vert[32];

	cobra_fifo *fifo_in = cobra->m_gfxfifo_in;
	cobra_fifo *fifo_out = cobra->m_gfxfifo_out;

	while (fifo_in->current_num() >= 2)
	{
		UINT64 in1, in2 = 0;
		UINT32 w1, w2;

		if (cobra->m_gfx_re_status == RE_STATUS_IDLE)
		{
			fifo_in->pop(nullptr, &in1);
			fifo_in->pop(nullptr, &in2);
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

				fifo_in->pop(nullptr, &param[0]);
				fifo_in->pop(nullptr, &param[1]);
				fifo_in->pop(nullptr, &param[2]);
				fifo_in->pop(nullptr, &param[3]);
				fifo_in->pop(nullptr, &param[4]);
				fifo_in->pop(nullptr, &param[5]);

				w[0] = (UINT32)param[0];    w[1] = (UINT32)param[1];    w[2] = (UINT32)param[2];
				w[3] = (UINT32)param[3];    w[4] = (UINT32)param[4];    w[5] = (UINT32)param[5];

				// mbuslib_pumpkin(): 0x00600000 0x10500010
				//                    0x00600000 0x10500018

				if (w2 == 0x10500010)
				{
					// GFX register select
					m_gfx_register_select = w[3];

				//	printf("GFX: register select %08X\n", m_gfx_register_select);
				}
				else if (w2 == 0x10500018)
				{
					// register write to the register selected above?
					// 64-bit registers, top 32-bits in word 2, low 32-bit in word 3
				//	printf("GFX: register write %08X: %08X %08X\n", m_gfx_register_select, w[2], w[3]);

					gfx_write_reg(((UINT64)(w[2]) << 32) | w[3]);
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
				UINT64 in3 = 0, in4 = 0, ignore;

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

#if LOG_DRAW_COMMANDS
				printf("--- Draw command %08X %08X ---\n", w1, w2);
#endif


				// extract vertex data
				for (int i=0; i < units; i++)
				{
					float x, y, z, w;
					float r, g, b, a;
					w = 1.0f;

					UINT64 in[4];
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


#if LOG_DRAW_COMMANDS
					if (w2 & 0x40000000)
					{
						printf("    ?: %08X\n", (UINT32)in[0]);
					}
					if (w2 & 0x20000000)
					{
						printf("    ?: %08X\n", (UINT32)in[1]);
					}

					printf("    x: %f\n", x);
					printf("    y: %f\n", y);
					printf("    ?: %08X\n", (UINT32)in[2]);
					printf("    z: %f\n", z);

					if (w2 & 0x00200000)
					{
						printf("    w: %f\n", w);
						printf("    u: %f\n", vert[i].p[POLY_U]);
						printf("    v: %f\n", vert[i].p[POLY_V]);
					}

					printf("    a: %f\n", a);
					printf("    r: %f\n", r);
					printf("    g: %f\n", g);
					printf("    b: %f\n", b);

					if (w2 & 0x00000001)
					{
						printf("    ?: %08X\n", (UINT32)in[3]);
					}

					printf("\n");
#endif
				}


				cobra_polydata &extra = object_data_alloc();

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
							render_delegate rd = render_delegate(FUNC(cobra_renderer::render_texture_scan), this);
							for (int i=2; i < units; i++)
							{
								render_triangle(visarea, rd, 8, vert[i-2], vert[i-1], vert[i]);
							}
						}
						else
						{
							render_delegate rd = render_delegate(FUNC(cobra_renderer::render_color_scan), this);
							for (int i=2; i < units; i++)
							{
								render_triangle(visarea, rd, 5, vert[i-2], vert[i-1], vert[i]);
							}
						}
						break;
					}

					case 0x2:           // points
					{
						for (int i=0; i < units; i++)
						{
							draw_point(visarea, vert[i], 0xffffffff);
						}
						break;
					}

					case 0x3:           // lines
					{
						if ((units & 1) == 0)       // batches of lines
						{
							for (i=0; i < units; i+=2)
							{
								draw_line(visarea, vert[i], vert[i+1]);
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
					UINT32 *buffer;
					switch (m_gfx_gram[0x80104/4])
					{
						case 0x800000:      buffer = &m_framebuffer->pix32(y); break;
						case 0x200000:      buffer = &m_backbuffer->pix32(y); break;
						case 0x0e0000:      buffer = &m_overlay->pix32(y); break;
						case 0x000800:      buffer = &m_zbuffer->pix32(y); break;
						case 0x000200:      buffer = &m_stencil->pix32(y); break;

						default:
						{
							fatalerror("gfxfifo_exec: fb write to buffer %08X!\n", m_gfx_gram[0x80100/4]);
						}
					}

					UINT64 param[4];
					param[0] = param[1] = param[2] = param[3] = 0;
					fifo_in->pop(nullptr, &param[0]);
					fifo_in->pop(nullptr, &param[1]);
					fifo_in->pop(nullptr, &param[2]);
					fifo_in->pop(nullptr, &param[3]);

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

//              printf("GFX: FB read X: %d, Y: %d\n", (UINT16)(m_gfx_gram[0x118/4] >> 16), (UINT16)(m_gfx_gram[0x118/4]));

				int x = (m_gfx_gram[0x118/4] >> 16) & 0xffff;
				int y = m_gfx_gram[0x118/4] & 0xffff;

				UINT32 *buffer;
				switch (m_gfx_gram[0x80104/4])
				{
					case 0x800000:      buffer = &m_framebuffer->pix32(y); break;
					case 0x200000:      buffer = &m_backbuffer->pix32(y); break;
					case 0x0e0000:      buffer = &m_overlay->pix32(y); break;
					case 0x000800:      buffer = &m_zbuffer->pix32(y); break;
					case 0x000200:      buffer = &m_stencil->pix32(y); break;

					default:
					{
						fatalerror("gfxfifo_exec: fb read from buffer %08X!\n", m_gfx_gram[0x80100/4]);
					}
				}

				// flush fifo_out so we have fresh data at top
				fifo_out->flush();

				fifo_out->push(nullptr, buffer[x+0]);
				fifo_out->push(nullptr, buffer[x+1]);
				fifo_out->push(nullptr, buffer[x+2]);
				fifo_out->push(nullptr, buffer[x+3]);

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
				UINT32 mask = m_gfx_regmask[w1 & 0xff];

				gfx_write_gram(reg, mask, w2);

#if LOG_GFX_RAM_WRITES
				if (reg != 0x118 && reg != 0x114 && reg != 0x11c)
				{
					printf("gfxfifo_exec: ram write %05X (mask %08X): %08X (%f)\n", reg, mask, w2, u2f(w2));
				}
#endif

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

				UINT32 ret = gfx_read_gram(reg);
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
					UINT32 value = gfx_read_gram(reg + (i*4));

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
					UINT64 param = 0;
					fifo_in->pop(nullptr, &param);
					cobra->m_gfx_re_word_count++;

					m_texture_ram[m_texram_ptr] = (UINT32)(param);
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
					fifo_in->pop(nullptr, &param);

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
				cobra->logerror("\n");
			}
		}

//      printf("gfxfifo_exec: %08X %08X\n", w1, w2);
	};

	wait();
}

READ64_MEMBER(cobra_state::gfx_fifo_r)
{
	UINT64 r = 0;

	m_renderer->gfx_fifo_exec();

	if (ACCESSING_BITS_32_63)
	{
		UINT64 data = 0;
		m_gfxfifo_out->pop(&space.device(), &data);

		data &= 0xffffffff;

		r |= (UINT64)(data) << 32;
	}
	if (ACCESSING_BITS_0_31)
	{
		UINT64 data = 0;
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
	if (ACCESSING_BITS_24_31)           // this register returns FIFO number during check_fifo (see below)
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
			m_gfx_unknown_v1 = (UINT32)(in1 >> 32);         // FIFO number is read back from this same register

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

	m_renderer->gfx_fifo_exec();

	if (data == U64(0x00a0000110500018))
	{
		m_gfxfifo_out->flush();

		// reads back the register selected by gfx register select

		UINT64 regdata = m_renderer->gfx_read_reg();

		m_gfxfifo_out->push(&space.device(), (UINT32)(regdata >> 32));
		m_gfxfifo_out->push(&space.device(), (UINT32)(regdata));
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
	else if (data != U64(0x00a0000110520200))       // mbuslib_regread()
	{
		// prc_read always expects a value...

		m_gfxfifo_out->push(&space.device(), 0);
	}
}

WRITE32_MEMBER(cobra_state::gfx_cpu_dc_store)
{
	UINT32 addr = offset >> 24;
	if (addr == 0x10 || addr == 0x18 || addr == 0x1e)
	{
		UINT64 i = (UINT64)(m_gfx_fifo_cache_addr) << 32;
		cobra_fifo *fifo_in = m_gfxfifo_in;

		UINT32 a = (offset / 8) & 0xff;

		fifo_in->push(&space.device(), (UINT32)(m_gfx_fifo_mem[a+0] >> 32) | i);
		fifo_in->push(&space.device(), (UINT32)(m_gfx_fifo_mem[a+0] >>  0) | i);
		fifo_in->push(&space.device(), (UINT32)(m_gfx_fifo_mem[a+1] >> 32) | i);
		fifo_in->push(&space.device(), (UINT32)(m_gfx_fifo_mem[a+1] >>  0) | i);
		fifo_in->push(&space.device(), (UINT32)(m_gfx_fifo_mem[a+2] >> 32) | i);
		fifo_in->push(&space.device(), (UINT32)(m_gfx_fifo_mem[a+2] >>  0) | i);
		fifo_in->push(&space.device(), (UINT32)(m_gfx_fifo_mem[a+3] >> 32) | i);
		fifo_in->push(&space.device(), (UINT32)(m_gfx_fifo_mem[a+3] >>  0) | i);

		m_renderer->gfx_fifo_exec();
	}
	else
	{
		logerror("gfx: data cache store at %08X\n", offset);
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
#if LOG_DEBUG_STATES
		if (m_gfx_debug_state != 0)
		{
			printf("GFX: debug state %02X\n", m_gfx_debug_state);
		}
#endif

		m_gfx_debug_state = 0;
		m_gfx_debug_state_wc = 0;
	}
}

static ADDRESS_MAP_START( cobra_gfx_map, AS_PROGRAM, 64, cobra_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("gfx_main_ram_0")
	AM_RANGE(0x07c00000, 0x07ffffff) AM_RAM AM_SHARE("gfx_main_ram_1")
	AM_RANGE(0x10000000, 0x100007ff) AM_WRITE(gfx_fifo0_w)
	AM_RANGE(0x18000000, 0x180007ff) AM_WRITE(gfx_fifo1_w)
	AM_RANGE(0x1e000000, 0x1e0007ff) AM_WRITE(gfx_fifo2_w)
	AM_RANGE(0x20000000, 0x20000007) AM_WRITE(gfx_buf_w)                            // this might really map to 0x1e000000, depending on the pagetable
	AM_RANGE(0x7f000000, 0x7f00ffff) AM_RAM AM_SHARE("pagetable")
	AM_RANGE(0xfff00000, 0xfff7ffff) AM_ROM AM_REGION("user3", 0)                   /* Boot ROM */
	AM_RANGE(0xfff80000, 0xfff80007) AM_WRITE(gfx_debug_state_w)
	AM_RANGE(0xffff0000, 0xffff0007) AM_READWRITE(gfx_unk1_r, gfx_unk1_w)
	AM_RANGE(0xffff0010, 0xffff001f) AM_READ(gfx_fifo_r)
ADDRESS_MAP_END


/*****************************************************************************/

INPUT_PORTS_START( cobra )
	PORT_START("TEST")
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_HIGH)            /* Test Button */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("P1 Service") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("P2 Service") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_PLAYER(2)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_PLAYER(2)

	PORT_START("COINS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_WRITE_LINE_DEVICE_MEMBER("cobra_jvs1", cobra_jvs, coin_1_w)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_WRITE_LINE_DEVICE_MEMBER("cobra_jvs1", cobra_jvs, coin_2_w)
INPUT_PORTS_END

WRITE_LINE_MEMBER(cobra_state::ide_interrupt)
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
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x003fffff, FALSE, m_main_ram);

	m_subcpu->ppcdrc_add_fastram(0x00000000, 0x003fffff, FALSE, m_sub_ram);

	m_gfxcpu->ppcdrc_add_fastram(0x00000000, 0x003fffff, FALSE, m_gfx_ram0);
	m_gfxcpu->ppcdrc_add_fastram(0x07c00000, 0x07ffffff, FALSE, m_gfx_ram1);
}

void cobra_state::machine_reset()
{
	m_sub_interrupt = 0xff;

	ide_hdd_device *hdd = m_ata->subdevice<ata_slot_device>("0")->subdevice<ide_hdd_device>("hdd");
	UINT16 *identify_device = hdd->identify_device_buffer();

	// Cobra expects these settings or the BIOS fails
	identify_device[51] = 0x0200;        /* 51: PIO data transfer cycle timing mode */
	identify_device[67] = 0x01e0;        /* 67: minimum PIO transfer cycle time without flow control */

	m_renderer->gfx_reset();

	m_sound_dma_ptr = 0;

	m_dmadac[0] = machine().device<dmadac_sound_device>("dac1");
	m_dmadac[1] = machine().device<dmadac_sound_device>("dac2");
	dmadac_enable(&m_dmadac[0], 1, 1);
	dmadac_enable(&m_dmadac[1], 1, 1);
	dmadac_set_frequency(&m_dmadac[0], 1, 44100);
	dmadac_set_frequency(&m_dmadac[1], 1, 44100);
}

static MACHINE_CONFIG_START( cobra, cobra_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC603, 100000000)      /* 603EV, 100? MHz */
	MCFG_PPC_BUS_FREQUENCY(XTAL_66_6667MHz)  /* Multiplier 1.5, Bus = 66MHz, Core = 100MHz */
	MCFG_CPU_PROGRAM_MAP(cobra_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cobra_state,  cobra_vblank)

	MCFG_CPU_ADD("subcpu", PPC403GA, 32000000)      /* 403GA, 33? MHz */
	MCFG_CPU_PROGRAM_MAP(cobra_sub_map)

	MCFG_CPU_ADD("gfxcpu", PPC604, 100000000)       /* 604, 100? MHz */
	MCFG_PPC_BUS_FREQUENCY(XTAL_66_6667MHz)   /* Multiplier 1.5, Bus = 66MHz, Core = 100MHz */
	MCFG_CPU_PROGRAM_MAP(cobra_gfx_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(15005))


	MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)
	MCFG_PCI_BUS_LEGACY_DEVICE(0, nullptr, mpc106_pci_r, mpc106_pci_w)

	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(cobra_state, ide_interrupt))

	/* video hardware */

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 399)
	MCFG_SCREEN_UPDATE_DRIVER(cobra_state, screen_update_cobra)
	MCFG_PALETTE_ADD("palette", 65536)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_RF5C400_ADD("rfsnd", XTAL_16_9344MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("dac1", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MCFG_SOUND_ADD("dac2", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_M48T58_ADD("m48t58")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)
	MCFG_DEVICE_ADD("k001604", K001604, 0)     // on the LAN board in Racing Jam DX
	MCFG_K001604_GFX_INDEX1(0)
	MCFG_K001604_GFX_INDEX2(1)
	MCFG_K001604_LAYER_SIZE(0)
	MCFG_K001604_ROZ_SIZE(1)
	MCFG_K001604_TXT_OFFSET(0)  // correct?
	MCFG_K001604_ROZ_OFFSET(0)  // correct?
	MCFG_K001604_GFXDECODE("gfxdecode")
	MCFG_K001604_PALETTE("palette")

	MCFG_DEVICE_ADD("cobra_jvs_host", COBRA_JVS_HOST, 4000000)
	MCFG_JVS_DEVICE_ADD("cobra_jvs1", COBRA_JVS, "cobra_jvs_host")
	MCFG_JVS_DEVICE_ADD("cobra_jvs2", COBRA_JVS, "cobra_jvs_host")
	MCFG_JVS_DEVICE_ADD("cobra_jvs3", COBRA_JVS, "cobra_jvs_host")

MACHINE_CONFIG_END

/*****************************************************************************/

/*****************************************************************************/

DRIVER_INIT_MEMBER(cobra_state, cobra)
{
	m_gfxfifo_in  = auto_alloc(machine(),
								cobra_fifo(machine(),
								8192,
								"GFXFIFO_IN",
								GFXFIFO_IN_VERBOSE != 0,
								cobra_fifo::event_delegate(FUNC(cobra_state::gfxfifo_in_event_callback), this))
								);

	m_gfxfifo_out = auto_alloc(machine(),
								cobra_fifo(machine(),
								8192,
								"GFXFIFO_OUT",
								GFXFIFO_OUT_VERBOSE != 0,
								cobra_fifo::event_delegate(FUNC(cobra_state::gfxfifo_out_event_callback), this))
								);

	m_m2sfifo     = auto_alloc(machine(),
								cobra_fifo(machine(),
								2048,
								"M2SFIFO",
								M2SFIFO_VERBOSE != 0,
								cobra_fifo::event_delegate(FUNC(cobra_state::m2sfifo_event_callback), this))
								);

	m_s2mfifo     = auto_alloc(machine(),
								cobra_fifo(machine(),
								2048,
								"S2MFIFO",
								S2MFIFO_VERBOSE != 0,
								cobra_fifo::event_delegate(FUNC(cobra_state::s2mfifo_event_callback), this))
								);

	m_maincpu->ppc_set_dcstore_callback(write32_delegate(FUNC(cobra_state::main_cpu_dc_store),this));

	m_gfxcpu->ppc_set_dcstore_callback(write32_delegate(FUNC(cobra_state::gfx_cpu_dc_store), this));

	m_subcpu->ppc4xx_set_dma_write_handler(0, write32_delegate(FUNC(cobra_state::sub_sound_dma_w), this), 44100);
	m_subcpu->ppc4xx_spu_set_tx_handler(write8_delegate(FUNC(cobra_state::sub_jvs_w), this));


	m_comram[0] = std::make_unique<UINT32[]>(0x40000/4);
	m_comram[1] = std::make_unique<UINT32[]>(0x40000/4);

	m_comram_page = 0;

	m_sound_dma_buffer_l = std::make_unique<INT16[]>(DMA_SOUND_BUFFER_SIZE);
	m_sound_dma_buffer_r = std::make_unique<INT16[]>(DMA_SOUND_BUFFER_SIZE);

	// setup fake pagetable until we figure out what really maps there...
	//m_gfx_pagetable[0x80 / 8] = U64(0x800001001e0001a8);
	m_gfx_pagetable[0x80 / 8] = U64(0x80000100200001a8);        // should this map to 0x1e000000?
}

DRIVER_INIT_MEMBER(cobra_state,bujutsu)
{
	DRIVER_INIT_CALL(cobra);

	// rom hacks for sub board...
	{
		UINT32 *rom = (UINT32*)memregion("user2")->base();

		rom[0x62094 / 4] = 0x60000000;          // skip hardcheck()...
	}


	// rom hacks for gfx board...
	{
		int i;
		UINT32 sum = 0;

		UINT32 *rom = (UINT32*)memregion("user3")->base();

		rom[(0x022d4^4) / 4] = 0x60000000;      // skip init_raster() for now ...

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
		UINT8 *rom = (UINT8*)memregion("m48t58")->base();
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
		UINT16 sum = 0;
		for (int i=0; i < 14; i+=2)
		{
			sum += ((UINT16)(rom[i]) << 8) | (rom[i+1]);
		}
		sum ^= 0xffff;

		rom[0x0e] = (UINT8)(sum >> 8);
		rom[0x0f] = (UINT8)(sum);
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

DRIVER_INIT_MEMBER(cobra_state,racjamdx)
{
	DRIVER_INIT_CALL(cobra);

	// rom hacks for sub board...
	{
		UINT32 *rom = (UINT32*)memregion("user2")->base();

		rom[0x62094 / 4] = 0x60000000;          // skip hardcheck()...
		rom[0x62ddc / 4] = 0x60000000;          // skip lanc_hardcheck()


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

		UINT32 *rom = (UINT32*)memregion("user3")->base();

		rom[(0x02448^4) / 4] = 0x60000000;      // skip init_raster() for now ...

		rom[(0x02438^4) / 4] = 0x60000000;      // awfully long delay loop (5000000 * 166)

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
		UINT8 *rom = (UINT8*)memregion("m48t58")->base();
		rom[0x00] = 0x47;       // G
		rom[0x01] = 0x59;       // Y
		rom[0x02] = 0x36;       // 6
		rom[0x03] = 0x37;       // 7
		rom[0x04] = 0x36;       // 6
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

	ROM_REGION16_LE(0x1000000, "rfsnd", ROMREGION_ERASE00)

	DISK_REGION( "ata:0:hdd:image" )
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

	ROM_REGION16_LE(0x1000000, "rfsnd", ROMREGION_ERASE00)

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "676a04", 0, SHA1(8e89d3e5099e871b99fccba13adaa3cf8a6b71f0) )
ROM_END

/*************************************************************************/

GAME( 1997, bujutsu, 0, cobra, cobra, cobra_state, bujutsu, ROT0, "Konami", "Fighting Bujutsu", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, racjamdx, 0, cobra, cobra, cobra_state, racjamdx, ROT0, "Konami", "Racing Jam DX", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
