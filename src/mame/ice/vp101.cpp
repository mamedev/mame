// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Play Mechanix / Right Hand Tech "VP100" and "VP101" platforms
    (PCBs are also marked "Raw Thrills" but all RT games appear to be on PC hardware)

    Boards:
        - VP101: Johnny Nero.  The original (?)
        - VP100: Special Forces Elite Training.  A not-quite-complete VP101; missing ATA DMA,
                 and also omits the PPC405 T&L frontend to the polygon engine.

    Emulation by R. Belmont

    TODO:
        - Actually emulate the PPC405.  Since it's a small fixed program it should be an ideal
          case for MAME's DRC.
        - specfrce has some incorrect rendering for the in-game scenes
        - specfrce HDD has freeplay and coins inserted by default, need to clear settings and
          merge the CHD

    To make the games go into a POST test, hold down START 1 while resetting.

    VP101 Features from http://web.archive.org/web/20041016000248/http://www.righthandtech.com/projects.htm

    MIPS VR5500 CPU
        The VR5500 operates at either at 300 or 400 MHz with 120MHz external bus
        MIPS 64-bit RISC architecture
        Two-way super-scalar super pipeline
        On-chip floating-point unit (FPU)
        High-speed translation look-aside buffer (TLB)(48 double-entries)
        On-chip primary cache memory (instruction/data: 32 KB each)
        2-way set associative, Supports line lock feature
        Conforms to MIPS I, II, III, and IV instruction sets. Also supports product-sum operation instruction, rotate instruction, register scan instruction
        Six execution units (ALU0, ALU1, FPU, FPU/MAC, BRU, and LSU)
        Employment of out-of-order execution mechanism
        Branch prediction mechanism - Branch history table with 4K entries
        Support for CPU emulator connection via JTAG/n-Wire port

    Unified Memory Architecture - DDR SDRAM bank
        Arbitrating DDR SDRAM Memory controller
        128Mbyte to 512Mbyte memory capacity
        120/240 MHz @ 64 bits - ~2GBytes/sec bandwidth

    3D Render Engine
        True color and 8-bit palette lookup textures
        8K byte texel cache for accelerated source texel selection.
        Perspective corrected rendering
        Bi-linear filter for source texel scaling
        256 Color Palette Lookup (888 RGB plus 8 bit Source Palette Alpha)
        True Color Source Textures (888 RGB plus 8 bit Alpha)
        24 bit Z-buffer structure in DDR SDRAM buffer
        Per-vertex colored lighting
        Alpha channel structure in DDR SDRAM buffer
        Pixel processing effects (fog, night, etc.)
        888 RGB Video DAC output section.
        Bitmap structure in DDR SDRAM with DMA for screen update
        Flexible CRT controller with X/Y gun interface counters

    Game I/O
        Standard JAMMA I/O interface, including player 3 and 4 connectors
        4 channel general purpose A to D interface (steering wheel and control pedals)
        100baseT Ethernet interface for debugging and/or inter game communications
        Force-feedback “Wheel Driver Interface” for driving games
        High-current drivers for lamps or solenoids
        Gun interface I/O tightly coupled to the CRT controller

    Sound System
        AC'97 codec for low cost of implementation and development
        TDA7375 40 Watt Integrated Amplifier
        Codec fed from the DDR bank via a 16 channel (8 channels of stereo) DMA engine.

    ATA/IDE Disk Drive Interface
        Standard ATA/IDE interface
        Ultra DMA 33/66/100/133 to the DDR SDRAM memory

    Video DAC
        RGB values at 8 bits per color
        RGB voltage level adjustable from 0-1.0 Vp-p to 0-4.0 Vp-p

    Flash Memory
        Minimum of 1MB of Flash memory – expandable to 4 MB
        Updateable Boot ROM
        Updateable FPGA configuration

    Battery Backed Up RAM
        32K bytes of non-volatile memory for static game configuration and high score table
        Non-volatile Real-Time clock

    Small Footprint
        Small outline design for easy kit retrofitting of existing cabinet
        12.2 in x 14.96 in

    Security Interface
        Security processor provides for a means to “unlock” the FPGA functions
        Enabled for software protection against piracy and unwarranted game updates

Full populated and tested board is less than $500, including IDE hard disk.
Small outline design for easy kit retrofitting of existing cabinets.

****************************************************************************/

#include "emu.h"

#include "bus/ata/ataintf.h"
#include "cpu/mips/mips3.h"
#include "machine/nvram.h"
#include "sound/dmadac.h"
#include "video/poly.h"

#include "screen.h"
#include "speaker.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define LOG_MBOX    (1U << 1)   // mailbox messages and payloads
#define LOG_CMD     (1U << 2)   // decoded render commands and engine state blocks
#define LOG_PRIM    (1U << 3)   // geometry blocks queued for the rasterizer
#define LOG_TTY     (1U << 4)   // the boot ROM's and the RSS's console

#define VERBOSE (0)
#include "logmacro.h"


namespace {

/***************************************************************************
    Render engine
***************************************************************************/

struct vp101_render_data
{
	uint32_t *target;           // render target line 0 (8 bytes per pixel)
	const uint32_t *source;     // copy source line 0, or nullptr
	const uint8_t *texture;     // Z-order texel base, or nullptr
	const uint32_t *texture32;  // linear 256-wide 0xAARRGGBB texels (VP100), or nullptr
	const uint32_t *palette;    // 0xiiRRGGBB entries, or nullptr
	int32_t x0, y0;             // rectangle origin in pixels
	int32_t u0, v0;             // 10.22 texel coordinates at the origin
	int32_t dudx, dvdy;         // 10.22 texel steps per pixel
	uint32_t color;             // 0xRRGGBBAA written for fills and textured pixels
	bool interpolated;          // texture coordinates come from the span parameters
	bool affine;                // and are plain 10.22 texels, not divided by w (VP100)
	bool keyed;                 // texels of the key colour are transparent (VP100)
	uint32_t key;               // that colour, 0xRRGGBB00
	bool lit;                   // texels are scaled by the vertex colour (VP100, mode bit 1)
	int32_t light[3];           // that colour's R, G and B at the origin, 0x7f8 being full intensity
	int32_t dldx[3], dldy[3];   // and its steps per pixel and per scanline
	uint32_t blend;             // source weight, 0x100 for an opaque primitive
	bool alpha_pass;            // the texture is a mask for the primitive that follows
	bool masked;                // the source weight is scaled by the pixel's alpha
	bool depth_test;            // test the pixel's depth word
	bool depth_write;           // and replace it (not for overlays and alpha passes)
	bool mask_texel0;           // index 0 is transparent (a masked texture)
	uint32_t depth;             // a rectangle's depth: its constant w (1/z)
};

class vp101_renderer : public poly_manager<float, vp101_render_data, 6>
{
public:
	vp101_renderer(running_machine &machine)
		: poly_manager<float, vp101_render_data, 6>(machine)
	{
	}

	void render_scanline(int32_t scanline, const extent_t &extent, const vp101_render_data &data, int threadid);
	static bool fetch_texel(const vp101_render_data &data, uint32_t tu, uint32_t tv, uint32_t &src, uint32_t &weight);

	// a texel scaled by a VP100 vertex colour, each level 0 to 0x7f8
	static uint32_t light_texel(uint32_t src, const uint32_t *level)
	{
		uint32_t result = src & 0xff;
		for (int i = 0; i < 3; i++)
		{
			const int shift = 24 - (i * 8);
			result |= ((((src >> shift) & 0xff) * level[i] + 0x3fc) / 0x7f8) << shift;
		}
		return result;
	}

	// a texture coordinate after the perspective divide, in 10.22 texels
	static uint32_t texel_coord(float p, float inv)
	{
		// the coordinate wraps: a surface can repeat its texture
		const double coord = std::clamp(double(p) * double(inv) * double(1 << 30), -4.0e18, 4.0e18);
		return uint32_t(uint64_t(int64_t(coord)));
	}

	// mix a source colour into the destination, both 0xRRGGBBAA
	static uint32_t blend_pixel(uint32_t src, uint32_t dst, uint32_t weight)
	{
		const uint32_t rest = 0x100 - weight;
		const uint32_t r = (((src >> 24) & 0xff) * weight + ((dst >> 24) & 0xff) * rest) >> 8;
		const uint32_t g = (((src >> 16) & 0xff) * weight + ((dst >> 16) & 0xff) * rest) >> 8;
		const uint32_t b = (((src >> 8) & 0xff) * weight + ((dst >> 8) & 0xff) * rest) >> 8;
		return (r << 24) | (g << 16) | (b << 8) | (src & 0xff);
	}

	// the Z-order spread of a 5-bit coordinate: bit n lands on bit 2n
	static constexpr uint32_t spread5(uint32_t v)
	{
		v = (v | (v << 4)) & 0x10f;
		v = (v | (v << 2)) & 0x133;
		return (v | (v << 1)) & 0x155;
	}

	static uint32_t texel_offset(uint32_t u, uint32_t v)
	{
		return ((v & 0x1e0) << 8) | ((u & 0xe0) << 5) | spread5(u & 0x1f) | (spread5(v & 0x1f) << 1);
	}
};

// a texel's 0xRRGGBBAA colour and weight, or false when it is transparent
inline bool vp101_renderer::fetch_texel(const vp101_render_data &data, uint32_t tu, uint32_t tv, uint32_t &src, uint32_t &weight)
{
	weight = data.blend;
	if (data.texture32)
	{
		// 32-bit texels are 0xAARRGGBB in a plain 256-wide image
		const uint32_t texel = data.texture32[((tv & 0xff) << 8) | (tu & 0xff)];
		src = (texel << 8) | (texel >> 24);
		weight = (weight * ((texel >> 24) + 1)) >> 8;
		if (weight == 0)
		{
			return false;
		}
	}
	else
	{
		const uint8_t texel = data.texture[texel_offset(tu, tv)];
		if (!texel && data.mask_texel0)
		{
			return false;
		}
		src = data.palette ? ((data.palette[texel] << 8) | 0xff) : data.color;
	}
	if (data.keyed && (src & 0xffffff00) == data.key)
	{
		return false;
	}
	return true;
}

void vp101_renderer::render_scanline(int32_t scanline, const extent_t &extent, const vp101_render_data &data, int threadid)
{
	uint32_t *const dest = data.target + (scanline * 0x1000) / 4;

	if (data.source)
	{
		// buffer copy: one texel per pixel, same layout as the target
		const uint32_t *const src = data.source + (scanline * 0x1000) / 4;
		for (int x = extent.startx; x < extent.stopx; x++)
		{
			dest[x * 2] = src[x * 2];
		}
	}
	else if (data.interpolated)
	{
		// u and v arrive multiplied by w, 2^30 being one texel a pixel; VP100 triangles carry plain texels
		float u = extent.param[0].start;
		float v = extent.param[1].start;
		float w = extent.param[2].start;
		float light[3] = { extent.param[3].start, extent.param[4].start, extent.param[5].start };
		for (int x = extent.startx; x < extent.stopx; x++)
		{
			const uint32_t depth = uint32_t(std::clamp(w, 0.0f, 4'294'967'040.0f));
			if (!data.depth_test || depth >= dest[x * 2 + 1])
			{
				uint32_t tu, tv;
				if (data.affine)
				{
					tu = uint32_t(int64_t(u)) >> 22;
					tv = uint32_t(int64_t(v)) >> 22;
				}
				else
				{
					const float inv = (w > 1.0f) ? (1.0f / w) : 1.0f;
					tu = texel_coord(u, inv) >> 22;
					tv = texel_coord(v, inv) >> 22;
				}

				if (data.alpha_pass)
				{
					// the mask becomes the pixel's alpha for the pass that follows
					dest[x * 2] = (dest[x * 2] & 0xffffff00) | data.texture[texel_offset(tu, tv)];
				}
				else
				{
					uint32_t src, weight;
					if (fetch_texel(data, tu, tv, src, weight))
					{
						if (data.lit)
						{
							uint32_t level[3];
							for (int i = 0; i < 3; i++)
							{
								level[i] = uint32_t(std::clamp(light[i], 0.0f, 2040.0f));
							}
							src = light_texel(src, level);
						}
						if (data.masked)
						{
							weight = (weight * ((dest[x * 2] & 0xff) + 1)) >> 8;
						}
						dest[x * 2] = (weight >= 0x100) ? src : blend_pixel(src, dest[x * 2], weight);
					}
				}

				if (data.depth_write)
				{
					dest[x * 2 + 1] = depth;
				}
			}
			u += extent.param[0].dpdx;
			v += extent.param[1].dpdx;
			w += extent.param[2].dpdx;
			for (int i = 0; i < 3; i++)
			{
				light[i] += extent.param[3 + i].dpdx;
			}
		}
	}
	else if (data.texture || data.texture32)
	{
		const int32_t v = data.v0 + (scanline - data.y0) * data.dvdy;
		int32_t u = data.u0 + (extent.startx - data.x0) * data.dudx;
		int64_t light[3];
		for (int i = 0; i < 3; i++)
		{
			light[i] = data.light[i] + int64_t(extent.startx - data.x0) * data.dldx[i] + int64_t(scanline - data.y0) * data.dldy[i];
		}
		for (int x = extent.startx; x < extent.stopx; x++)
		{
			if (!data.depth_test || data.depth >= dest[x * 2 + 1])
			{
				const uint32_t tu = uint32_t(u) >> 22, tv = uint32_t(v) >> 22;
				if (data.alpha_pass)
				{
					// the mask becomes the pixel's alpha for the pass that follows
					dest[x * 2] = (dest[x * 2] & 0xffffff00) | data.texture[texel_offset(tu, tv)];
				}
				else
				{
					uint32_t src, weight;
					if (fetch_texel(data, tu, tv, src, weight))
					{
						if (data.lit)
						{
							uint32_t level[3];
							for (int i = 0; i < 3; i++)
							{
								level[i] = uint32_t(std::clamp<int64_t>(light[i], 0, 0x7f8));
							}
							src = light_texel(src, level);
						}
						if (data.masked)
						{
							weight = (weight * ((dest[x * 2] & 0xff) + 1)) >> 8;
						}
						dest[x * 2] = (weight >= 0x100) ? src : blend_pixel(src, dest[x * 2], weight);
						if (data.depth_write)
						{
							dest[x * 2 + 1] = data.depth;
						}
					}
				}
			}
			u += data.dudx;
			for (int i = 0; i < 3; i++)
			{
				light[i] += data.dldx[i];
			}
		}
	}
	else
	{
		// the clear resets the depth words to infinitely far away
		for (int x = extent.startx; x < extent.stopx; x++)
		{
			dest[x * 2] = data.color;
			dest[x * 2 + 1] = 0;
		}
	}
}


class vp10x_state : public driver_device
{
public:
	vp10x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_mainram(*this, "mainram"),
			m_mbox(*this, "mbox", 0x4000, ENDIANNESS_LITTLE),
			m_ata(*this, "ata"),
			m_screen(*this, "screen"),
			m_gun_x(*this, "GUNX%u", 1U),
			m_gun_y(*this, "GUNY%u", 1U),
			m_recoil(*this, "recoil%u", 1U),
			m_dmadac(*this, "dac%u", 0U),
			m_pic_cmd(0),
			m_pic_state(0),
			m_dmarq_state(false),
			m_dma_ptr(0),
			m_unk_sound_toggle(0),
			m_sound_cmd(0),
			m_snd_key(0),
			m_snd_enable(0),
			m_snd_status(0),
			m_snd_timer(nullptr),
			m_snd_block{},
			m_fb_base(0),
			m_clut{},
			m_render_timer(nullptr),
			m_mbox_tx_rptr(0),
			m_mbox_rx_wptr(0),
			m_ppc_ctrl(0),
			m_int_enable(0),
			m_int_status(0),
			m_dispctl{},
			m_ide_irq(0),
			m_gun_timer{},
			m_gun_latch{},
			m_gun_hpos{},
			m_gun_lines{},
			m_doorbell(0),
			m_doorbell_pending(false),
			m_texture_count(0),
			m_native_list(false)
	{ }

	void vp101(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// render engine register file, written by state blocks
	struct fpga_regs
	{
		uint32_t mode = 0;          // low 9 bits of the command header, or a block-specific code
		uint32_t address = 0;       // render target, offset to a source buffer, or a tile table
		uint32_t texture = 0;
		uint32_t attr_b = 0;
		uint32_t attr_c = 0;
		uint32_t attr_d = 0;
		uint32_t window_right = 0;  // right edge of the window, 12.20 pixels
		uint32_t target = 0;        // render target (address of the last mode-0x100 block)
	};

	// one corner of a triangle as the render engine receives it
	struct prim_vertex
	{
		int32_t x, y;       // screen position, 24.8 fixed-point pixels
		int32_t u, v;       // texture coordinates, divided by w when sampled
		int32_t w;          // 1/z from the transform
		uint32_t color;     // (colour & 0x1ff) << 8
		int32_t light[3];   // VP100 vertex colour, R/G/B with 0x7f8 being full intensity
	};

	// one geometry block: an axis-aligned rectangle or a triangle
	struct render_prim
	{
		uint32_t type;      // 0 = fill, 1 = textured rectangle, 2 = triangle
		int32_t x0, y0;     // first corner, 12.20 fixed-point screen pixels
		int32_t x1, y1;     // second corner
		int32_t u0, v0;     // texel coordinates at (x0, y0), 10.22 fixed point
		int32_t dudx;       // texel step per pixel, 10.22
		int32_t dvdy;       // as written to the block: the firmware negates dv/dy
		uint32_t w;
		uint32_t color;     // (color & 0x1ff) << 8, only when mode bit 1 is set
		int32_t light[3];   // VP100 rectangle colour at (x0, y0), R/G/B with 0x7f8 being full intensity
		int32_t dldx[3];    // its step per pixel
		int32_t dldy[3];    // and per scanline, as written to the block
		prim_vertex vtx[5]; // triangle corners (up to five once clipped)
		int vertices;       // how many of them
		bool native;        // the block was built by the game (VP100), not the firmware HLE
		int32_t clip_l, clip_t, clip_r, clip_b;   // window, pixels
		fpga_regs regs;     // engine state when the block was queued
	};

	// one of the firmware's sixteen vertex slots, which a record can name instead of carrying a vertex
	struct fw_vertex
	{
		int32_t u = 0, v = 0;       // texture coordinates
		int32_t u2 = 0, v2 = 0;     // second-pass texture coordinates
		int32_t x = 0, y = 0;       // screen position, 24.8 fixed-point pixels
		int32_t w = 0;              // 1/z, the perspective divisor
		uint32_t color = 0;         // (colour & 0x1ff) << 8
	};

	// a texture load block
	struct fpga_texture
	{
		uint32_t address = 0;   // tile table in shared RAM
		uint32_t size = 0;
		uint32_t texture = 0;
	};

	// firmware working state (the SDA variables of the real firmware)
	struct ppc_fw_state
	{
		uint32_t pending = 0;       // state block header being accumulated
		uint32_t mode = 0;
		uint32_t texture = 0;
		uint32_t address = 0;
		uint32_t attr_b = 0;
		uint32_t attr_c = 0;
		uint32_t attr_d = 0;
		uint32_t last_address = 0;  // change detection for address parameters
		uint32_t last_attr_b = 0;   // change detection for attribute B parameters
		uint32_t target = 0;        // persistent render target
		uint32_t target_base = 0;   // base for relative addresses
		int32_t clip_l = 0;         // window, pixels << 8
		int32_t clip_t = 0;
		int32_t clip_r = 0;
		int32_t clip_b = 0;
		int32_t center_x = 0;       // window centre, pixels << 8
		int32_t center_y = 0;
		int32_t persp_x = 0x100;    // projection scale pair from the window message
		int32_t persp_y = 0x100;
		uint32_t msg_count = 0;     // render buffers received this frame
		uint32_t vertex_next = 0;   // next vertex slot to allocate
	};

	// a sample engine voice: 16-bit PCM from main RAM; the second address and count are the loop point
	struct snd_voice
	{
		uint32_t control = 0;   // bit 17 = half rate, bit 16 = loop, low 16 = volume
		uint32_t addr[2] = { 0, 0 };
		uint32_t count[2] = { 0, 0 };
		uint8_t phase = 0;
	};

	static constexpr uint32_t SND_RATE = 48000;      // control bit 17 picks this or half
	static constexpr uint32_t SND_BLOCK = 480;

	// mailbox window at 0x1a000000: header words, then the TX (MIPS to PPC) and RX areas
	static constexpr uint32_t MBOX_WORDS = 0x1000;      // 16K window
	static constexpr uint32_t MBOX_TX_START = 0x80;     // byte offsets
	static constexpr uint32_t MBOX_TX_END = 0x1b80;
	static constexpr uint32_t MBOX_RX_START = 0x1b80;
	static constexpr uint32_t MBOX_RX_END = 0x1f80;

	// control words the firmware maintains in the mailbox header
	static constexpr uint32_t MBOX_NOTIFY = 0x20;       // MIPS: frame ends after MBOX_MSG_COUNT buffers
	static constexpr uint32_t MBOX_MSG_COUNT = 0x24;
	static constexpr uint32_t MBOX_PROCESSED = 0x28;    // messages consumed
	static constexpr uint32_t MBOX_ERRORS = 0x34;
	static constexpr uint32_t MBOX_STATUS = 0x38;       // error/request flags
	static constexpr uint32_t MBOX_FLAT = 0x3c;         // triangles that clip to one scanline
	static constexpr uint32_t MBOX_REJECTED = 0x40;     // rectangles clipped away
	static constexpr uint32_t MBOX_PRIMS = 0x44;        // geometry blocks queued
	static constexpr uint32_t MBOX_OFFSCREEN = 0x48;    // triangles outside the window
	static constexpr uint32_t MBOX_CULLED = 0x4c;       // back-facing triangles
	static constexpr uint32_t MBOX_ERR_WORD = 0x50;
	static constexpr uint32_t MBOX_ERR_START = 0x54;    // start of the stream being parsed
	static constexpr uint32_t MBOX_ERR_PTR = 0x58;
	static constexpr uint32_t MBOX_ERR_OFS = 0x5c;

	// state block header bits
	static constexpr uint32_t STATE_VALID = 0x01000000;
	static constexpr uint32_t STATE_ADDRESS = 0x02000000;
	static constexpr uint32_t STATE_ATTR_C = 0x04000000;
	static constexpr uint32_t STATE_TEXTURE = 0x08000000;
	static constexpr uint32_t STATE_ATTR_D = 0x10000000;
	static constexpr uint32_t STATE_FRAME = 0x40000000;
	static constexpr uint32_t STATE_TEX_FLAG = 0x00800000;  // texture & 0xa0
	static constexpr uint32_t STATE_ATTR_B = 0x00000040;

	// VP100 descriptor lists: state block fields, batch end, link and last-block bits
	static constexpr uint32_t STATE_FIELDS = 0x1f000040;
	static constexpr uint32_t LIST_END = 0x00008830;
	static constexpr uint32_t LIST_LINK = 0x00080000;
	static constexpr uint32_t LIST_LAST = 0x40000000;
	static constexpr int MAX_LIST_BLOCKS = 16384;

	static constexpr size_t MAX_PRIMS = 65536;

	// a gun's sensor sees the beam on this many consecutive lines
	static constexpr int GUN_LINES = 11;

	required_device<mips3_device> m_maincpu;
	required_shared_ptr<uint32_t> m_mainram;
	memory_share_creator<uint32_t> m_mbox;
	required_device<ata_interface_device> m_ata;
	required_device<screen_device> m_screen;
	optional_ioport_array<2> m_gun_x;
	optional_ioport_array<2> m_gun_y;
	output_finder<2> m_recoil;
	required_device_array<dmadac_sound_device, 2> m_dmadac;

	// PIC security chip
	uint8_t m_pic_cmd;
	uint8_t m_pic_state;

	// ATA DMA
	bool m_dmarq_state;
	uint32_t m_dma_ptr;

	// codec control
	uint32_t m_unk_sound_toggle;
	uint32_t m_sound_cmd;

	// sample engine
	snd_voice m_voice[8];
	uint32_t m_snd_key;             // 0x1000: a bit per playing voice
	uint32_t m_snd_enable;          // 0x1008
	uint32_t m_snd_status;          // 0x100c: a bit per voice wanting a refill
	emu_timer *m_snd_timer;
	int16_t m_snd_block[SND_BLOCK * 2];

	// display, FPGA interrupt controller and mailbox
	uint32_t m_fb_base;
	uint32_t m_clut[0x400];
	emu_timer *m_render_timer;
	uint32_t m_mbox_tx_rptr;
	uint32_t m_mbox_rx_wptr;
	uint32_t m_ppc_ctrl;
	uint32_t m_int_enable;
	uint32_t m_int_status;
	uint32_t m_dispctl[4];
	uint32_t m_ide_irq;

	// light guns: the CRT controller latches its counters as the beam passes the sensor
	emu_timer *m_gun_timer[2];
	uint32_t m_gun_latch[2];
	int32_t m_gun_hpos[2];
	int32_t m_gun_lines[2];

	// VP100 doorbell
	uint32_t m_doorbell;                // last descriptor list address handed to the engine
	bool m_doorbell_pending;            // written since the engine was last started

	// firmware HLE and render engine
	ppc_fw_state m_fw;
	fpga_regs m_fpga;
	fpga_texture m_textures[16];
	fw_vertex m_vertices[16];
	uint8_t m_texture_count;
	std::vector<render_prim> m_prims;   // this frame's geometry, for the rasterizer (not saved)
	bool m_native_list;                 // the geometry being drawn came from a VP100 list
	std::unique_ptr<vp101_renderer> m_renderer;

	std::string m_tty;                  // the console line being assembled

	// console, PIC and codec
	uint32_t tty_ready_r();
	void tty_w(uint32_t data);
	uint32_t test_r();
	uint32_t pic_r();
	void pic_w(uint32_t data);
	uint32_t sound_r(offs_t offset);
	void sound_w(offs_t offset, uint32_t data);

	// sample engine
	uint32_t snd_r(offs_t offset);
	void snd_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	TIMER_CALLBACK_MEMBER(snd_tick);
	void snd_update_irq();

	// ATA
	void dmaaddr_w(uint32_t data);
	void dmarq_w(int state);
	void ata_irq_w(int state);
	uint32_t ide_irq_r();
	void ide_irq_w(uint32_t data);

	// FPGA interrupt controller, display and guns
	uint32_t int_enable_r();
	void int_enable_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t int_status_r();
	void int_ack_w(uint32_t data);
	void update_irqs();
	void fb_base_w(uint32_t data);
	uint32_t fb_base_r();
	uint32_t dispctl_r(offs_t offset);
	void dispctl_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t clut_r(offs_t offset);
	void clut_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void vblank_w(int state);
	uint32_t gun_latch_r(offs_t offset);
	void gun_out_w(uint32_t data);
	TIMER_CALLBACK_MEMBER(gun_latch);

	// mailbox to the PPC
	uint32_t mbox_r(offs_t offset);
	void mbox_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t ppc_ctrl_r();
	void ppc_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void ppc_boot();
	void mbox_process_tx();
	void mbox_post_reply(uint32_t type, const uint32_t *payload, uint32_t len);
	TIMER_CALLBACK_MEMBER(render_done);

	// PPC405 firmware HLE
	void ppc_render_buffer(const uint32_t *payload, uint32_t len);
	void ppc_frame_end();
	void ppc_set_window(const uint32_t *payload, uint32_t len);
	void ppc_texture_load(const uint32_t *payload, uint32_t len);
	void ppc_render_stream(const uint32_t *stream, const uint32_t *end);
	const uint32_t *ppc_render_control(uint32_t hdr, const uint32_t *p, const uint32_t *end);
	const uint32_t *ppc_render_rects(uint32_t hdr, const uint32_t *p, const uint32_t *end);
	const uint32_t *ppc_render_vertices(uint32_t hdr, const uint32_t *p, const uint32_t *end);
	void ppc_emit_triangles(const int *slot, int count, bool second_pass);
	uint32_t ppc_resolve_texture(uint32_t handle);
	void ppc_clear(uint32_t value);
	void ppc_send_state();
	void ppc_error(uint32_t flag, const uint32_t *where, uint32_t word);

	// VP100: the game hands the render engine its descriptor lists directly
	uint32_t fpga_doorbell_r();
	void fpga_doorbell_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t fpga_busy_r();
	void fpga_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t fpga_target_r();
	void fpga_target_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void fpga_submit(uint32_t address);
	void fpga_native_geometry(const uint32_t *blk);

	// VP100 texels are 32-bit 0xAARRGGBB for mode bit 2, or texture bits 0-1 = 2 with bit 10
	static bool native_32bit(const fpga_regs &regs) { return BIT(regs.mode, 2) || (regs.texture & 0x403) == 0x402; }
	// VP101 lighting: one channel of attribute D's 0xRRGGBB scaled by the firmware's intensity, 0x7f8 being full
	static int32_t fw_light(uint32_t attr_d, int channel, uint32_t color) { return int32_t(((attr_d >> (16 - (channel * 8))) & 0xff) * ((color >> 8) & 0x1ff) * 8 / 0xff); }

	// render engine model
	void fpga_state_block(uint32_t hdr, uint32_t mode, uint32_t address, uint32_t attr_c, uint32_t texture, uint32_t attr_d, uint32_t attr_b);
	void fpga_geometry_block(render_prim &prim);
	void fpga_triangle_block(const fw_vertex &v0, const fw_vertex &v1, const fw_vertex &v2, bool second_pass);
	void fpga_frame_end();
	void rasterize();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};

void vp10x_state::machine_reset()
{
	m_dmarq_state = false;
	m_pic_cmd = 0;
	m_pic_state = 0;
	m_dma_ptr = 0;
	m_unk_sound_toggle = 0;
	m_sound_cmd = 0;

	std::fill(std::begin(m_voice), std::end(m_voice), snd_voice());
	m_snd_key = 0;
	m_snd_enable = 0;
	m_snd_status = 0;

	m_fb_base = 0;
	m_int_enable = 0;
	m_int_status = 0;
	m_ide_irq = 0;
	m_doorbell = 0;
	m_doorbell_pending = false;
	std::fill(std::begin(m_dispctl), std::end(m_dispctl), 0);
	m_ppc_ctrl = 0;
	for (int gun = 0; gun < 2; gun++)
	{
		m_gun_timer[gun]->adjust(attotime::never);
		m_gun_latch[gun] = 0;
		m_gun_hpos[gun] = 0;
		m_gun_lines[gun] = 0;
		m_recoil[gun] = 0;
	}
	ppc_boot();
	update_irqs();
}

void vp10x_state::machine_start()
{
	m_snd_timer = timer_alloc(FUNC(vp10x_state::snd_tick), this);
	for (auto &dac : m_dmadac)
	{
		dac->set_frequency(SND_RATE);
		dac->enable(1);
	}
	const attotime period = attotime::from_ticks(SND_BLOCK, SND_RATE);
	m_snd_timer->adjust(period, 0, period);

	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS | MIPS3DRC_DISABLE_INTRABLOCK);
//  m_maincpu->add_fastram(0x00000000, 0x03ffffff, false, m_mainram);

	m_render_timer = timer_alloc(FUNC(vp10x_state::render_done), this);
	for (int gun = 0; gun < 2; gun++)
	{
		m_gun_timer[gun] = timer_alloc(FUNC(vp10x_state::gun_latch), this);
	}
	m_prims.reserve(4096);

	save_item(NAME(m_pic_cmd));
	save_item(NAME(m_pic_state));
	save_item(NAME(m_dmarq_state));
	save_item(NAME(m_dma_ptr));
	save_item(NAME(m_unk_sound_toggle));
	save_item(NAME(m_sound_cmd));
	for (int n = 0; n < 8; n++)
	{
		save_item(NAME(m_voice[n].control), n);
		save_item(NAME(m_voice[n].addr), n);
		save_item(NAME(m_voice[n].count), n);
		save_item(NAME(m_voice[n].phase), n);
	}
	save_item(NAME(m_snd_key));
	save_item(NAME(m_snd_enable));
	save_item(NAME(m_snd_status));
	save_item(NAME(m_clut));
	save_item(NAME(m_mbox_tx_rptr));
	save_item(NAME(m_mbox_rx_wptr));
	save_item(NAME(m_ppc_ctrl));
	save_item(NAME(m_int_enable));
	save_item(NAME(m_int_status));
	save_item(NAME(m_dispctl));
	save_item(NAME(m_ide_irq));
	save_item(NAME(m_gun_latch));
	save_item(NAME(m_gun_hpos));
	save_item(NAME(m_gun_lines));
	save_item(NAME(m_doorbell));
	save_item(NAME(m_doorbell_pending));
	save_item(NAME(m_fb_base));
	save_item(NAME(m_fw.pending));
	save_item(NAME(m_fw.mode));
	save_item(NAME(m_fw.texture));
	save_item(NAME(m_fw.address));
	save_item(NAME(m_fw.attr_b));
	save_item(NAME(m_fw.attr_c));
	save_item(NAME(m_fw.attr_d));
	save_item(NAME(m_fw.last_address));
	save_item(NAME(m_fw.last_attr_b));
	save_item(NAME(m_fw.target));
	save_item(NAME(m_fw.target_base));
	save_item(NAME(m_fw.clip_l));
	save_item(NAME(m_fw.clip_t));
	save_item(NAME(m_fw.clip_r));
	save_item(NAME(m_fw.clip_b));
	save_item(NAME(m_fw.center_x));
	save_item(NAME(m_fw.center_y));
	save_item(NAME(m_fw.persp_x));
	save_item(NAME(m_fw.persp_y));
	save_item(NAME(m_fw.msg_count));
	save_item(NAME(m_fw.vertex_next));
	save_item(NAME(m_fpga.mode));
	save_item(NAME(m_fpga.address));
	save_item(NAME(m_fpga.texture));
	save_item(NAME(m_fpga.attr_b));
	save_item(NAME(m_fpga.attr_c));
	save_item(NAME(m_fpga.attr_d));
	save_item(NAME(m_fpga.window_right));
	save_item(NAME(m_fpga.target));
	save_item(STRUCT_MEMBER(m_textures, address));
	save_item(STRUCT_MEMBER(m_textures, size));
	save_item(STRUCT_MEMBER(m_textures, texture));
	save_item(NAME(m_texture_count));
	save_item(STRUCT_MEMBER(m_vertices, u));
	save_item(STRUCT_MEMBER(m_vertices, v));
	save_item(STRUCT_MEMBER(m_vertices, u2));
	save_item(STRUCT_MEMBER(m_vertices, v2));
	save_item(STRUCT_MEMBER(m_vertices, x));
	save_item(STRUCT_MEMBER(m_vertices, y));
	save_item(STRUCT_MEMBER(m_vertices, w));
	save_item(STRUCT_MEMBER(m_vertices, color));
}

// "Boot" the PPC in the FPGA: set up the mailbox header and signal readiness.
void vp10x_state::ppc_boot()
{
	std::fill_n(m_mbox.target(), MBOX_WORDS, 0);
	m_mbox[0x00 / 4] = MBOX_TX_START;
	m_mbox[0x04 / 4] = MBOX_TX_END;
	m_mbox[0x08 / 4] = MBOX_RX_START;
	m_mbox[0x0c / 4] = MBOX_RX_END;
	m_mbox[0x10 / 4] = 0;   // ready (game syncs on this changing from -1)
	m_mbox_tx_rptr = MBOX_TX_START;
	m_mbox_rx_wptr = MBOX_RX_START;

	m_fw = ppc_fw_state();
	m_fpga = fpga_regs();
	std::fill(std::begin(m_textures), std::end(m_textures), fpga_texture());
	std::fill(std::begin(m_vertices), std::end(m_vertices), fw_vertex());
	m_texture_count = 0;
	m_prims.clear();
}

// FPGA interrupt controller; the routing is the table the game hands to the RSS installer
void vp10x_state::update_irqs()
{
	// which CPU IP line (2-6, i.e. MIPS3_IRQ0-4) each status bit is routed to
	static constexpr uint8_t ROUTING[11] = { 0, 1, 1, 0, 2, 3, 4, 3, 0, 3, 3 };

	uint32_t lines = 0;
	const uint32_t active = m_int_status & m_int_enable;
	for (int bit = 0; bit < 11; bit++)
	{
		if (BIT(active, bit))
		{
			lines |= 1 << ROUTING[bit];
		}
	}

	for (int line = 0; line < 5; line++)
	{
		m_maincpu->set_input_line(MIPS3_IRQ0 + line, BIT(lines, line) ? ASSERT_LINE : CLEAR_LINE);
	}
}

uint32_t vp10x_state::int_enable_r()
{
	return m_int_enable;
}

void vp10x_state::int_enable_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_int_enable);
	update_irqs();
}

uint32_t vp10x_state::int_status_r()
{
	const uint32_t data = m_int_status;
	if (!machine().side_effects_disabled())
	{
		// vblank and the gun latches are pulses cleared by the read; the mailbox waits for int_ack_w
		m_int_status &= ~0x680;
		update_irqs();
	}
	return data;
}

void vp10x_state::int_ack_w(uint32_t data)
{
	// write-zero-to-clear (the game's PPC interrupt handler writes 0 here)
	m_int_status &= data;
	update_irqs();
}

void vp10x_state::vblank_w(int state)
{
	if (state)
	{
		if (m_dispctl[2] & 0x80)
		{
			m_int_status |= 0x80;
			update_irqs();
		}

		// arm the gun sensors for the coming frame
		for (int gun = 0; gun < 2; gun++)
		{
			if (m_gun_x[gun].found() && m_gun_y[gun].found())
			{
				const int hpos = m_gun_x[gun]->read();
				const int vpos = m_gun_y[gun]->read();
				const int first = std::max(vpos - (GUN_LINES / 2), 0);
				const int last = std::min(vpos + (GUN_LINES / 2), m_screen->visible_area().bottom());
				m_gun_hpos[gun] = hpos;
				m_gun_lines[gun] = last - first + 1;
				m_gun_timer[gun]->adjust(m_screen->time_until_pos(first, hpos), gun);
			}
		}
	}
}

// the beam has reached a line the gun's sensor can see: latch the counters
TIMER_CALLBACK_MEMBER(vp10x_state::gun_latch)
{
	const int gun = param;
	const int vpos = m_screen->vpos();
	m_gun_latch[gun] = ((vpos & 0x3ff) << 16) | (m_gun_hpos[gun] & 0x7ff);
	m_int_status |= 0x200 << gun;
	update_irqs();

	if (--m_gun_lines[gun] > 0)
	{
		m_gun_timer[gun]->adjust(m_screen->time_until_pos(vpos + 1, m_gun_hpos[gun]), gun);
	}
}

uint32_t vp10x_state::gun_latch_r(offs_t offset)
{
	// 0x18000020 = gun 1, 0x18000028 = gun 2; bits 0-10 horizontal counter, bits 16-25 line
	return BIT(offset, 0) ? 0 : m_gun_latch[BIT(offset, 1)];
}

void vp10x_state::gun_out_w(uint32_t data)
{
	// bits 12/13 fire the gun recoil solenoids ("gun clackers")
	for (int gun = 0; gun < 2; gun++)
	{
		m_recoil[gun] = BIT(data, 12 + gun);
	}
}

uint32_t vp10x_state::dispctl_r(offs_t offset)
{
	return m_dispctl[offset & 3];
}

void vp10x_state::dispctl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dispctl[offset & 3]);
	LOG("%s: dispctl_w %08x = %08x\n", machine().describe_context(), 0x18000000 + (offset << 2), data);

	if ((offset & 3) == 0)
	{
		// bit 7 selects the 512x384 picture (VP101); the VP100 games run 400x256
		const bool large = BIT(m_dispctl[0], 7);
		m_screen->set_visible_area(0, large ? 511 : 399, 0, large ? 383 : 255);
	}
}

uint32_t vp10x_state::clut_r(offs_t offset)
{
	return m_clut[offset & 0x3ff];
}

void vp10x_state::clut_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_clut[offset & 0x3ff]);
}

uint32_t vp10x_state::test_r()
{
	LOG("%s: test_r\n", machine().describe_context());
	return 0xffffffff;
}

void vp10x_state::dmaaddr_w(uint32_t data)
{
	LOG("%s: dmaaddr_w: %08x\n", machine().describe_context(), data);
	m_dma_ptr = (data & 0x07ffffff);
}

void vp10x_state::dmarq_w(int state)
{
	if (bool(state) != m_dmarq_state)
	{
		m_dmarq_state = bool(state);

		if (state)
		{
			uint16_t *RAMbase = (uint16_t *)&m_mainram[0];
			uint16_t *RAM = &RAMbase[m_dma_ptr>>1];

			m_ata->write_dmack(ASSERT_LINE);

			while (m_dmarq_state)
			{
				*RAM++ = m_ata->read_dma();
				m_dma_ptr += 2; // pointer must advance
			}

			m_ata->write_dmack(CLEAR_LINE);
		}
	}
}

// the drive's interrupt is FPGA status bit 1 and bit 0 of the pending register at 0x1d000004
void vp10x_state::ata_irq_w(int state)
{
	if (state)
	{
		m_ide_irq |= 1;
		m_int_status |= 0x02;
	}
	else
	{
		m_ide_irq &= ~1;
		m_int_status &= ~0x02;
	}
	update_irqs();
}

uint32_t vp10x_state::ide_irq_r()
{
	return m_ide_irq;
}

void vp10x_state::ide_irq_w(uint32_t data)
{
	m_ide_irq &= data;
	if (!(m_ide_irq & 1))
	{
		m_int_status &= ~0x02;
		update_irqs();
	}
}

// VP100 has no PPC: the game hands the engine its descriptor lists through this doorbell
uint32_t vp10x_state::fpga_doorbell_r()
{
	return m_doorbell;
}

void vp10x_state::fpga_doorbell_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_doorbell);
	m_doorbell_pending = true;
}

// bit 16 is the engine's busy flag, polled before a list is handed over
uint32_t vp10x_state::fpga_busy_r()
{
	return m_render_timer->enabled() ? 0x10000 : 0;
}

// with bit 0 set, a pulse on bit 2 starts the doorbell's list; the enables are shared with 0x12000000
void vp10x_state::fpga_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const uint32_t previous = m_int_enable;
	int_enable_w(offset, data, mem_mask);

	if (BIT(previous, 2) && !BIT(m_int_enable, 2) && m_doorbell_pending)
	{
		m_doorbell_pending = false;
		fpga_submit(m_doorbell);
	}
}

void vp10x_state::fpga_submit(uint32_t address)
{
	LOGMASKED(LOG_CMD, "%s: fpga: descriptor list at %08x\n", machine().describe_context(), address);

	// batches of eight 0x80-byte blocks, each linked to the next through its last word
	uint32_t block = address & 0x07ffffff;
	for (int count = 0; count < MAX_LIST_BLOCKS && block + 0x80 <= 0x08000000; count++)
	{
		const uint32_t *const blk = &m_mainram[block / 4];
		const uint32_t hdr = blk[0];
		// a zero header is a geometry block whose long edge is A; only an empty block ends the list
		if (std::all_of(blk, blk + 0x20, [] (uint32_t word) { return word == 0; }))
		{
			break;
		}

		const bool batch_end = (hdr & LIST_END) == LIST_END;
		if (hdr & STATE_FIELDS)
		{
			const uint32_t attr_b = (!batch_end && (hdr & LIST_LINK)) ? blk[0x1f] : blk[6];
			fpga_state_block(hdr, blk[1], blk[2], blk[3], blk[4], blk[5], attr_b);
		}
		else if (!batch_end || (hdr & ~LIST_END) != 0)
		{
			fpga_native_geometry(blk);
			if (hdr & LIST_LAST)
			{
				break;
			}
		}

		if (batch_end)
		{
			const uint32_t next = blk[0x1f];
			if (next == 0)
			{
				break;
			}
			block = next & 0x07ffffff;
		}
		else
		{
			block += 0x80;
		}
	}

	// the engine draws the list into the target register's buffer as it goes
	m_native_list = true;
	fpga_frame_end();
	m_native_list = false;

	m_render_timer->adjust(attotime::from_usec(200));
}

// 0x10000010: the render target for lists the game hands over directly
uint32_t vp10x_state::fpga_target_r()
{
	return m_fpga.target;
}

void vp10x_state::fpga_target_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_fpga.target);
	LOGMASKED(LOG_CMD, "%s: fpga: render target %08x\n", machine().describe_context(), m_fpga.target);
}

// a geometry block: three scanlines, three edges, and w/u/v/colour with their gradients
void vp10x_state::fpga_native_geometry(const uint32_t *blk)
{
	const int32_t y_top = blk[1], y_mid = blk[2], y_bot = blk[3];
	const int32_t x_a = blk[5], x_b = blk[6], x_c = blk[7];
	const int32_t slope_a = blk[9], slope_b = blk[10], slope_c = blk[11];

	// which of the two edges leaving the top is the long one
	const bool long_b = BIT(blk[0], 0);
	const int32_t x_long = long_b ? x_b : x_a;
	const int32_t slope_long = long_b ? slope_b : slope_a;
	const int32_t x_short = long_b ? x_a : x_b;
	const int32_t slope_short = long_b ? slope_a : slope_b;

	render_prim prim = {};
	prim.native = true;
	prim.w = blk[12];

	if (slope_a == 0 && slope_b == 0 && slope_c == 0)
	{
		// a rectangle has zero slopes; texture bit 9 marks a fill
		const int32_t x_far = (y_mid > y_top) ? x_short : x_c;
		prim.type = (m_fpga.texture & 0x200) ? 0 : 1;
		prim.x0 = std::min(x_long, x_far);
		prim.x1 = std::max(x_long, x_far);
		prim.y0 = y_top;
		prim.y1 = y_bot;
		// the texel origin slides with the left edge, in the engine's wrapping 32-bit arithmetic
		const int32_t left = int32_t(uint32_t(prim.x0) - uint32_t(x_long)) >> 20;
		prim.u0 = blk[15] + uint32_t(left) * blk[17];
		prim.v0 = blk[18];
		prim.dudx = blk[17];
		prim.dvdy = blk[19];
		for (int i = 0; i < 3; i++)
		{
			prim.light[i] = blk[21 + (i * 3)] + uint32_t(left) * blk[23 + (i * 3)];
			prim.dldx[i] = blk[23 + (i * 3)];
			prim.dldy[i] = blk[22 + (i * 3)];
		}
		fpga_geometry_block(prim);
		return;
	}

	// x of an edge dy (12.20) below the point where its x is known
	auto edge_x = [] (int32_t x, int32_t slope, int32_t dy) { return x - int32_t((int64_t(dy) * slope) >> 20); };

	// interpolants are given at the top of the long edge, with per-scanline and per-pixel steps
	int count = 0;
	auto vertex = [&count, &prim, &edge_x, blk, x_long, slope_long, y_top] (int32_t x, int32_t y)
	{
		if (count > 0 && prim.vtx[count - 1].x == (x >> 12) && prim.vtx[count - 1].y == (y >> 12))
		{
			return;
		}
		const int32_t dy = y - y_top;
		const int32_t dx = x - edge_x(x_long, slope_long, dy);
		auto param = [blk, dy, dx] (int base) { return int32_t(int32_t(blk[base]) - ((int64_t(dy) * int32_t(blk[base + 1])) >> 20) + ((int64_t(dx) * int32_t(blk[base + 2])) >> 20)); };
		prim_vertex &v = prim.vtx[count++];
		v.x = x >> 12;
		v.y = y >> 12;
		v.w = param(12);
		v.u = param(15);
		v.v = param(18);
		for (int i = 0; i < 3; i++)
		{
			v.light[i] = param(21 + (i * 3));
		}
	};

	// around the outline, from the top of the long edge
	vertex(x_long, y_top);
	if (y_mid > y_top)
	{
		vertex(x_short, y_top);
	}
	if (y_mid < y_bot)
	{
		vertex(x_c, y_mid);
		vertex(edge_x(x_c, slope_c, y_bot - y_mid), y_bot);
	}
	else
	{
		vertex(edge_x(x_short, slope_short, y_bot - y_top), y_bot);
	}
	vertex(edge_x(x_long, slope_long, y_bot - y_top), y_bot);
	if (count < 3)
	{
		return;
	}

	prim.type = 2;
	prim.vertices = count;
	const rectangle &visible = m_screen->visible_area();
	prim.clip_l = visible.left();
	prim.clip_t = visible.top();
	prim.clip_r = visible.right() + 1;
	prim.clip_b = visible.bottom() + 1;
	fpga_geometry_block(prim);
}

uint32_t vp10x_state::pic_r()
{
	static constexpr uint8_t VERSION[5] = { 0x00, 0x01, 0x00, 0x00, 0x00 };
	// byte 5 is the product code (3 for jnero), bytes 6-7 a non-zero serial number
	static constexpr uint8_t SERIAL[10] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x03, 0x07, 0x08, 0x09, 0x0a };
	static constexpr uint8_t MAGIC[10] = { 0xaa, 0x55, 0x18, 0x18, 0xc0, 0x03, 0xf0, 0x0f, 0x09, 0x0a };

	const uint8_t index = m_pic_state;
	uint32_t data = 0;
	switch (m_pic_cmd)
	{
		case 0x20:
			data = (index < std::size(VERSION)) ? VERSION[index] : 0;
			break;

		case 0x21:
		case 0x22:
			data = (index < std::size(SERIAL)) ? SERIAL[index] : 0;
			break;

		case 0x23:  // this is the same for jnero and specfrce.  great security!
			data = (index < std::size(MAGIC)) ? MAGIC[index] : 0;
			break;

		default:
			return 0;
	}

	if (!machine().side_effects_disabled())
	{
		m_pic_state++;
	}
	return data;
}

void vp10x_state::pic_w(uint32_t data)
{
	LOG("%s: pic_w: %08x\n", machine().describe_context(), data);
	if ((data & 0xff) == 0)
	{
		return;
	}
	m_pic_cmd = data & 0xff;
	m_pic_state = 0;
}

void vp10x_state::video_start()
{
	m_renderer = std::make_unique<vp101_renderer>(machine());
}

uint32_t vp10x_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// make sure the frame's geometry has been drawn before it is scanned out
	m_renderer->wait();

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint32_t *line = &bitmap.pix(y, cliprect.left());
		const uint32_t *video_ram = &m_mainram[((m_fb_base & 0x07ffffff) / 4) + (y * (0x1000 / 4)) + (cliprect.left() * 2)];

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			// pixels are a 0xRRGGBBAA colour word followed by a Z word
			*line++ = *video_ram >> 8;
			video_ram += 2;
		}
	}
	return 0;
}

uint32_t vp10x_state::tty_ready_r()
{
	LOG("%s: tty_ready_r\n", machine().describe_context());
	return 0x60;    // must return &0x20 for output at tty_w to continue
}

// the boot ROM's and the RSS's console ("RAM OK", "EPI RSS Ver 4.5.1", "<RSS active>")
void vp10x_state::tty_w(uint32_t data)
{
	if (data == 0x0d || m_tty.length() >= 256)
	{
		LOGMASKED(LOG_TTY, "tty: %s\n", m_tty);
		m_tty.clear();
	}
	if (data >= 0x20 || data == 0x09)
	{
		m_tty += char(data);
	}
}

uint32_t vp10x_state::mbox_r(offs_t offset)
{
	return m_mbox[offset & (MBOX_WORDS - 1)];
}

void vp10x_state::mbox_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	offset &= MBOX_WORDS - 1;
	COMBINE_DATA(&m_mbox[offset]);

	const uint32_t byteoff = offset << 2;
	if (byteoff == 0x10 && m_mbox[offset] == 0xffffffff)
	{
		// sync: the game writes -1 and waits for the PPC to answer 0 (ready)
		m_mbox[offset] = 0;
		LOG("%s: mbox: sync request, signalling ready\n", machine().describe_context());
		return;
	}

	if (byteoff >= MBOX_TX_START && byteoff < MBOX_TX_END && (data & 0x80000000))
	{
		mbox_process_tx();
	}
}

// consume MIPS->PPC messages and post replies like the PPC firmware would
void vp10x_state::mbox_process_tx()
{
	// bound the scan in case of protocol confusion
	for (int sanity = 0; sanity < 256; sanity++)
	{
		const uint32_t hdr = m_mbox[m_mbox_tx_rptr / 4];
		if (!(hdr & 0x80000000))
		{
			break;
		}

		if (hdr & 0x40000000)
		{
			// wrap marker: consume it and resume at the start of the TX area
			m_mbox[m_mbox_tx_rptr / 4] = hdr & ~0x80000000;
			m_mbox_tx_rptr = MBOX_TX_START;
			continue;
		}

		const uint32_t type = (hdr >> 16) & 0xff;
		const uint32_t len = hdr & 0x3fff;
		const uint32_t *payload = &m_mbox[(m_mbox_tx_rptr + 4) / 4];

		if (VERBOSE & LOG_MBOX)
		{
			const uint32_t *const limit = m_mbox.target() + MBOX_WORDS;
			std::ostringstream dump;
			for (const uint32_t *word = payload; word < payload + ((len + 3) / 4) && word < limit; word++)
			{
				util::stream_format(dump, " %08x", *word);
			}
			LOGMASKED(LOG_MBOX, "%s: mbox: msg type %02x len %x:%s\n", machine().describe_context(), type, len, std::move(dump).str());
		}

		switch (type)
		{
			case 1:     // echo: reply with the same payload
				mbox_post_reply(1, payload, 4);
				break;

			case 2:     // version request: %d.%d.%d packed as b31-24.b23-16.b15-0
			{
				const uint32_t version = 0x01000000;
				mbox_post_reply(2, &version, 4);
				break;
			}

			case 3:     // end of frame: render engine signals completion
				ppc_frame_end();
				m_render_timer->adjust(attotime::from_usec(200));
				break;

			case 4:     // window setup ("ppc replied with geometry")
				ppc_set_window(payload, len);
				mbox_post_reply(4, nullptr, 0);
				break;

			case 9:     // texture load
				ppc_texture_load(payload, len);
				break;

			case 0x10:  // render command buffer
				ppc_render_buffer(payload, len);
				m_render_timer->adjust(attotime::from_usec(200));
				break;

			default:
				LOGMASKED(LOG_MBOX, "mbox: unhandled message type %02x\n", type);
				break;
		}

		// mark consumed and advance
		m_mbox[MBOX_PROCESSED / 4]++;
		m_mbox[m_mbox_tx_rptr / 4] = hdr & ~0x80000000;
		m_mbox_tx_rptr += 4 + ((len + 3) & ~3);
		if (m_mbox_tx_rptr >= MBOX_TX_END)
		{
			m_mbox_tx_rptr = MBOX_TX_START;
		}
	}
}

void vp10x_state::mbox_post_reply(uint32_t type, const uint32_t *payload, uint32_t len)
{
	const uint32_t words = (len + 3) / 4;
	if (m_mbox_rx_wptr + 4 + (words * 4) >= MBOX_RX_END)
	{
		m_mbox[m_mbox_rx_wptr / 4] = 0xc0000000;
		m_mbox_rx_wptr = MBOX_RX_START;
	}

	for (uint32_t i = 0; i < words; i++)
	{
		m_mbox[(m_mbox_rx_wptr + 4) / 4 + i] = payload[i];
	}
	m_mbox[m_mbox_rx_wptr / 4] = 0x80000000 | (type << 16) | len;
	m_mbox_rx_wptr += 4 + (words * 4);

	m_int_status |= 0x20;
	update_irqs();
}

TIMER_CALLBACK_MEMBER(vp10x_state::render_done)
{
	m_int_status |= 0x20;
	update_irqs();
}

/***************************************************************************
    PPC405 firmware HLE
***************************************************************************/

// the PPC405 firmware's message handlers and parsers, feeding the engine state and geometry blocks

// note an error the way the firmware does, in the mailbox status words
void vp10x_state::ppc_error(uint32_t flag, const uint32_t *where, uint32_t word)
{
	const uint32_t offset = uint32_t(where - m_mbox.target()) << 2;
	m_mbox[MBOX_STATUS / 4] |= flag;
	m_mbox[MBOX_ERRORS / 4]++;
	if (m_mbox[MBOX_ERR_WORD / 4] == 0)
	{
		m_mbox[MBOX_ERR_WORD / 4] = word;
	}
	m_mbox[MBOX_ERR_PTR / 4] = offset;
	m_mbox[MBOX_ERR_OFS / 4] = offset - m_mbox[MBOX_ERR_START / 4];
	LOGMASKED(LOG_CMD, "ppc: render stream error %03x at mailbox offset %04x (word %08x)\n", flag, offset, word);
}

// type 0x10: a render command buffer
void vp10x_state::ppc_render_buffer(const uint32_t *payload, uint32_t len)
{
	// the MIPS can end the frame by posting a buffer count in the header
	m_fw.msg_count++;
	if (m_mbox[MBOX_NOTIFY / 4] != 0 && m_fw.msg_count == m_mbox[MBOX_MSG_COUNT / 4])
	{
		m_mbox[MBOX_NOTIFY / 4] = 0;
		m_mbox[MBOX_MSG_COUNT / 4] = 0;
		m_fw.msg_count = 0;
		m_fw.pending |= STATE_FRAME | STATE_ATTR_D;
		ppc_send_state();
	}

	const uint32_t *const stream = payload;
	const uint32_t *const limit = m_mbox.target() + MBOX_WORDS;
	const uint32_t *const end = std::min(payload + ((len + 3) / 4), limit);
	m_mbox[MBOX_ERR_START / 4] = uint32_t(stream - m_mbox.target()) << 2;
	ppc_render_stream(stream, end);
}

// type 3: end of frame
void vp10x_state::ppc_frame_end()
{
	m_fw.msg_count = 0;
	m_mbox[MBOX_MSG_COUNT / 4] = 0;
	m_fw.pending |= STATE_FRAME | STATE_ATTR_D;
	ppc_send_state();
	m_fw.last_address = 0;
	m_fw.last_attr_b = 0;
}

// type 4: window (top << 16 | left), (bottom << 16 | right), optional scale (x << 16 | y)
void vp10x_state::ppc_set_window(const uint32_t *payload, uint32_t len)
{
	if (len < 8)
	{
		return;
	}

	const int32_t left = int32_t(int16_t(payload[0] & 0xffff)) << 8;
	const int32_t top = int32_t(int16_t(payload[0] >> 16)) << 8;
	const int32_t right = int32_t(int16_t(payload[1] & 0xffff)) << 8;
	const int32_t bottom = int32_t(int16_t(payload[1] >> 16)) << 8;
	LOGMASKED(LOG_CMD, "ppc: window (%d,%d)-(%d,%d)\n", left >> 8, top >> 8, right >> 8, bottom >> 8);

	m_fw.clip_t = top;
	m_fw.clip_b = bottom;
	m_fw.center_x = (left + right) >> 1;
	m_fw.center_y = (top + bottom) >> 1;
	if (left != m_fw.clip_l || right != m_fw.clip_r)
	{
		m_fw.clip_l = left;
		m_fw.clip_r = right;
		ppc_send_state();

		// the firmware also queues a block carrying the right edge
		m_fpga.window_right = right << 12;
		LOGMASKED(LOG_CMD, "fpga: window right edge %d\n", right >> 8);
	}

	if (len > 8)
	{
		m_fw.persp_x = int16_t(payload[2] & 0xffff);
		m_fw.persp_y = int16_t(payload[2] >> 16);
	}
}

// type 9: load a texture from a tile table in shared RAM
void vp10x_state::ppc_texture_load(const uint32_t *payload, uint32_t len)
{
	if (len < 12)
	{
		return;
	}

	const uint32_t source = payload[0];
	const uint32_t arg = payload[1];
	const uint32_t size = payload[2];
	LOGMASKED(LOG_CMD, "ppc: texture load source %08x arg %08x size %08x\n", source, arg, size);

	fpga_state_block(STATE_VALID | STATE_ADDRESS | STATE_TEXTURE, (size << 16) | 0xf020, source, 0, ((arg - 1) << 8) | 0x20, 0, 0);

	// the address and texture registers were borrowed; send the real ones again
	m_fw.pending |= STATE_VALID | STATE_ADDRESS | STATE_TEXTURE;
}

// texture handle lookup (0x780007c0): the engine-filled tables are not modelled, so handles pass through
uint32_t vp10x_state::ppc_resolve_texture(uint32_t handle)
{
	if (handle & 3)
	{
		// cache miss: ask the MIPS to upload the texture
		m_mbox[MBOX_STATUS / 4] |= 0x100;
	}
	if (handle & 0x100003)
	{
		LOGMASKED(LOG_CMD, "ppc: unresolved texture handle %08x\n", handle);
	}
	return handle;
}

// emit the accumulated state block (0x780004e0)
void vp10x_state::ppc_send_state()
{
	uint32_t hdr = m_fw.pending;
	if (hdr == 0)
	{
		return;
	}

	m_fw.pending = 0;
	if (m_fw.texture & 0xa0)
	{
		hdr |= STATE_TEX_FLAG;
	}

	fpga_state_block(hdr, m_fw.mode, m_fw.address, m_fw.attr_c, m_fw.texture, m_fw.attr_d, m_fw.attr_b);
}

// control command 1 (0x7800086c): fill the window with a value
void vp10x_state::ppc_clear(uint32_t value)
{
	LOGMASKED(LOG_CMD, "ppc: clear %08x\n", value);

	// the block names texture 0x308 with the value as attribute D
	fpga_state_block(STATE_VALID | STATE_TEXTURE | STATE_ATTR_D, 0, 0, 0, 0x308, value, 0);
	m_fw.pending |= STATE_VALID | STATE_TEXTURE | STATE_ATTR_D;

	render_prim prim = {};
	prim.type = 0;
	prim.x0 = m_fw.clip_l << 12;
	prim.y0 = m_fw.clip_t << 12;
	prim.x1 = m_fw.clip_r << 12;
	prim.y1 = m_fw.clip_b << 12;
	fpga_geometry_block(prim);
}

// control commands (0x78002218)
const uint32_t *vp10x_state::ppc_render_control(uint32_t hdr, const uint32_t *p, const uint32_t *end)
{
	switch (hdr & 0xff)
	{
		case 1:
			ppc_clear((p < end) ? *p : 0);
			p++;
			break;

		case 2:     // mode and texture straight from the stream
			if (p + 2 <= end)
			{
				m_fw.mode = p[0];
				m_fw.texture = p[1];
			}
			p += 2;
			m_fw.pending |= STATE_VALID | STATE_TEXTURE;
			ppc_send_state();
			break;

		default:
			m_mbox[MBOX_STATUS / 4] |= 0x400;
			m_mbox[MBOX_ERRORS / 4]++;
			LOGMASKED(LOG_CMD, "ppc: unknown control command %08x\n", hdr);
			break;
	}
	return p;
}

// the render command stream parser (0x780022b8)
void vp10x_state::ppc_render_stream(const uint32_t *stream, const uint32_t *end)
{
	const uint32_t *p = stream;

	// parameters are read past the end as the firmware does; the overrun is caught after the command
	auto param = [&p, end] () -> uint32_t { return (p < end) ? *p++ : (p++, 0U); };

	while (p < end)
	{
		const uint32_t hdr = *p++;
		if (hdr == 0)
		{
			return;
		}

		if (p >= end || (*p & 0xff) != 0x16)
		{
			ppc_error(0x10, p, (p < end) ? *p : 0);
			return;
		}
		const uint32_t marker = *p++;

		if (BIT(hdr, 31))
		{
			p = ppc_render_control(hdr, p, end);
		}
		else
		{
			uint32_t pending = m_fw.pending | STATE_VALID;
			m_fw.mode = hdr & 0x1ff;

			if (hdr & 0x400)
			{
				const uint32_t value = param();
				if (value != m_fw.last_address)
				{
					m_fw.address = m_fw.last_address = value;
					pending |= STATE_ADDRESS;
				}
			}
			if (hdr & 0x800)
			{
				const uint32_t value = param();
				if (value != m_fw.last_attr_b)
				{
					m_fw.attr_b = m_fw.last_attr_b = value;
					pending |= STATE_ATTR_B;
				}
			}
			if (hdr & 0x1000)
			{
				const uint32_t value = param();
				if ((hdr & 0x100000) && (value & 3))
				{
					m_fw.texture = (value & 0xfc) | 0x1400;
				}
				else
				{
					m_fw.texture = ppc_resolve_texture(value);
				}
				pending |= STATE_TEXTURE;
			}
			if (hdr & 0x2000)
			{
				m_fw.attr_c = param();
				pending |= STATE_ATTR_C;
			}
			if (hdr & 0x4000)
			{
				m_fw.attr_d = param();
				pending |= STATE_ATTR_D;
			}
			m_fw.pending = pending;

			LOGMASKED(LOG_CMD, "ppc: cmd %08x mode %03x addr %08x tex %08x b %08x c %08x d %08x\n",
					hdr, m_fw.mode, m_fw.address, m_fw.texture, m_fw.attr_b, m_fw.attr_c, m_fw.attr_d);

			if (hdr & 0x80)
			{
				// the address becomes an offset from the current render target
				const uint32_t address = m_fw.address ? m_fw.address : m_fw.target;
				m_fw.address = address - m_fw.target_base;
				m_fw.last_address = ~0U;
			}

			if (hdr & 0x100)
			{
				// set the render target; bit 0 marks a temporary one
				uint32_t address = m_fw.address;
				if (address == 0)
				{
					address = m_fw.target;
				}
				else if (address & 1)
				{
					address &= ~1;
				}
				else
				{
					m_fw.target = address;
				}
				m_fw.address = address;
				m_fw.target_base = address;
				m_fw.last_address = ~0U;
				ppc_send_state();
			}
			else if (hdr & 0x40000)
			{
				p = ppc_render_rects(hdr, p, end);
				if (!p)
				{
					return;
				}
			}
			else if (hdr & 0x200)
			{
				p = ppc_render_vertices(hdr, p, end);
				if (!p)
				{
					return;
				}
			}
		}

		if (p > end)
		{
			ppc_error(0x80, p, marker);
			return;
		}
	}
}

// the rectangle list parser (0x78001d34): records of 10 words, 11 with header bit 0x20000
const uint32_t *vp10x_state::ppc_render_rects(uint32_t hdr, const uint32_t *p, const uint32_t *end)
{
	const bool long_form = BIT(hdr, 17);
	const int length = long_form ? 11 : 10;

	while (p < end && *p != 0)
	{
		if ((*p & 0xff) != 0x12)
		{
			ppc_error(0x40, p, *p);
			return nullptr;
		}
		if (p + length > end)
		{
			ppc_error(0x80, p, *p);
			return nullptr;
		}

		const uint32_t *const rec = p;
		p += length;

		if (rec[0] & 0x100)
		{
			m_mbox[MBOX_STATUS / 4] |= 0x200;
		}

		const uint32_t *const corner = long_form ? &rec[3] : &rec[4];
		const uint32_t color = long_form ? rec[6] : rec[3];
		const uint32_t ctrl = corner[2];

		// pixels << 8 and texels << 8 while clipping
		int32_t x0 = corner[0] << 8;
		int32_t y0 = corner[1] << 8;
		int32_t x1 = rec[7] << 8;
		int32_t y1 = rec[8] << 8;
		uint32_t u0 = (rec[1] & 0xff) << 8;
		uint32_t v0 = rec[1] & 0xff00;
		const int32_t du = int32_t((rec[2] & 0xff) << 8) - int32_t(u0);
		const int32_t dv = int32_t(rec[2] & 0xff00) - int32_t(v0);
		int32_t dudx, dvdy;

		if (hdr & 0x8000)
		{
			// scaled: spread the texel span over the pixel span (no clipping)
			const int32_t width = (x1 - x0) >> 8;
			const int32_t height = (y1 - y0) >> 8;
			dudx = width ? int32_t(uint32_t(du) << 14) / width : 0;
			dvdy = height ? int32_t(uint32_t(dv) << 14) / height : 0;
		}
		else
		{
			// one texel per pixel: clip to the window, sliding the texture origin
			bool visible = true;
			if (x0 < m_fw.clip_l)
			{
				visible = x1 >= m_fw.clip_l;
				u0 += m_fw.clip_l - x0;
				x0 = m_fw.clip_l;
			}
			if (visible && x1 > m_fw.clip_r)
			{
				visible = x0 <= m_fw.clip_r;
				x1 = m_fw.clip_r;
			}
			if (visible && y0 < m_fw.clip_t)
			{
				visible = y1 >= m_fw.clip_t;
				v0 += m_fw.clip_t - y0;
				y0 = m_fw.clip_t;
			}
			if (visible && y1 > m_fw.clip_b)
			{
				visible = y0 <= m_fw.clip_b;
				y1 = m_fw.clip_b;
			}
			if (!visible)
			{
				m_mbox[MBOX_REJECTED / 4]++;
				continue;
			}
			dudx = dvdy = 0x00400000;
		}

		render_prim prim = {};
		prim.type = 1;
		prim.x0 = x0 << 12;
		prim.y0 = y0 << 12;
		prim.x1 = x1 << 12;
		prim.y1 = y1 << 12;
		prim.u0 = int32_t(u0 << 14);
		prim.v0 = int32_t(v0 << 14);
		prim.dudx = dudx;
		prim.dvdy = -dvdy;
		prim.w = ((ctrl & 3) << 30) - 1;
		prim.color = (m_fw.mode & 2) ? ((color & 0x1ff) << 8) : 0;

		if (hdr & 0x100000)
		{
			// two passes: first a copy at address + 0x10000 with the coordinates scaled by 0x7f7f7f7f / 2^32
			const uint32_t mode = m_fw.mode;
			const uint32_t texture = m_fw.texture;
			const uint32_t address = m_fw.address;
			m_fw.mode = (mode & 0x10) | 0xb008;
			m_fw.texture = texture & 0xfc;
			m_fw.address = address + 0x10000;
			m_fw.pending |= STATE_VALID | STATE_ADDRESS | STATE_TEXTURE;
			ppc_send_state();

			auto scale = [] (int32_t value) { return int32_t((int64_t(value) * 0x7f7f7f7f) >> 32); };
			render_prim pass = prim;
			pass.u0 = scale(prim.u0);
			pass.dudx = scale(prim.dudx);
			pass.v0 = scale(prim.v0);
			pass.dvdy = scale(prim.dvdy);
			pass.color = 0;
			fpga_geometry_block(pass);

			m_fw.mode = mode;
			m_fw.texture = texture;
			m_fw.address = address;
			m_fw.pending |= STATE_VALID | STATE_ADDRESS | STATE_TEXTURE;
		}

		ppc_send_state();
		fpga_geometry_block(prim);
	}

	// skip the terminator
	if (p < end)
	{
		p++;
	}
	return p;
}

// the vertex list parser (0x78001198): 0x12 records of three or four vertices
const uint32_t *vp10x_state::ppc_render_vertices(uint32_t hdr, const uint32_t *p, const uint32_t *end)
{
	// the high bits of the 64-bit projected coordinate, scaled in halves so nothing overflows
	auto project = [] (int32_t coord, int32_t w, int32_t scale)
	{
		const int64_t product = (int64_t(coord) * w) >> 5;
		return int32_t(((product >> 32) * scale) + ((int64_t(uint32_t(product)) * scale) >> 32));
	};
	// the perspective divide the firmware applies to a texture coordinate
	auto divide = [] (int32_t w, int32_t value)
	{
		return int32_t((int64_t(w) * int64_t(uint32_t(value))) >> 32);
	};
	auto shrink = [] (int32_t value)
	{
		return int32_t((uint64_t(uint32_t(value)) * 0x7f7f7f7f) >> 32);
	};

	const uint32_t count = (hdr >> 24) & 7;
	const bool perspective = BIT(hdr, 15);
	const bool vertex_color = BIT(hdr, 17);
	const uint32_t address = m_fw.address;
	bool second_pass;
	int32_t uv_base;
	int32_t uv_scale;

	if (!(hdr & 0x200000))
	{
		m_fw.vertex_next = 0;
	}

	if (count == 0)
	{
		second_pass = true;
		uv_base = 0;
		uv_scale = 0x00400000;
	}
	else
	{
		// byte coordinates cover 2^count times fewer texels; the first pass reads the following texture
		second_pass = BIT(hdr, 22);
		uv_base = 0x40000000 - int32_t(0xffffffffU << (31 - count));
		uv_scale = int32_t(((0xff >> count) * 0x10101) >> 2);
		if (perspective)
		{
			uv_base <<= 2;
		}
		m_fw.address = address + 0x10000;
	}
	ppc_send_state();

	const uint32_t *rec = p;
	while (rec < end && *rec != 0)
	{
		const uint32_t ctrl = *rec;
		if ((ctrl & 0xff) != 0x12)
		{
			ppc_error(0x20, rec, ctrl);
			m_fw.address = address;
			return nullptr;
		}
		if (rec + 4 > end)
		{
			ppc_error(0x80, rec, ctrl);
			m_fw.address = address;
			return nullptr;
		}

		// the packed byte texture coordinates, one (u, v) pair per vertex
		const uint32_t packed[4] = { rec[1] & 0xffff, rec[1] >> 16, rec[2] & 0xffff, rec[2] >> 16 };
		p = rec + (vertex_color ? 3 : 4);

		// allocate the vertices from the ring; a record can name earlier ones
		const int corners = (ctrl & 0x100) ? 4 : 3;
		int slot[4];
		for (int i = 0; i < 3; i++)
		{
			slot[i] = (m_fw.vertex_next + i) & 15;
		}
		m_fw.vertex_next += 3;
		if (corners == 4)
		{
			slot[3] = m_fw.vertex_next++ & 15;
		}

		for (int i = 0; i < corners; i++)
		{
			fw_vertex &vtx = m_vertices[slot[i]];
			if (BIT(ctrl, 12 + i))
			{
				const fw_vertex &cached = m_vertices[(ctrl >> (16 + i * 4)) & 15];
				vtx.x = cached.x;
				vtx.y = cached.y;
				vtx.w = cached.w;
				vtx.color = cached.color;
			}
			else if (p + 3 <= end)
			{
				const int32_t w = int32_t(p[2]);
				vtx.x = project(int32_t(p[0]), w, m_fw.persp_x) + m_fw.center_x;
				vtx.y = project(int32_t(p[1]), w, m_fw.persp_y) + m_fw.center_y;
				vtx.w = w;
				p += 3;
				if (vertex_color)
				{
					vtx.color = (((p < end) ? *p++ : 0) << 8) & 0x1ff00;
				}
			}
			else
			{
				ppc_error(0x80, p, 0);
				m_fw.address = address;
				return nullptr;
			}

			const uint32_t u = perspective ? ((packed[i] & 0xff) << 2) : (packed[i] & 0xff);
			const uint32_t v = perspective ? ((packed[i] & 0xff00) >> 6) : (packed[i] >> 8);
			vtx.u = int32_t(u * uint32_t(uv_scale));
			vtx.v = int32_t(v * uint32_t(uv_scale) + uint32_t(uv_base));
			if (perspective)
			{
				vtx.u = divide(vtx.w, vtx.u);
				vtx.v = divide(vtx.w, vtx.v);
			}
		}

		// one colour for the whole primitive unless the vertices carry it
		if (!vertex_color)
		{
			const uint32_t color = (rec[3] << 8) & 0x1ff00;
			for (int i = 0; i < corners; i++)
			{
				m_vertices[slot[i]].color = color;
			}
		}

		// drop primitives that fall entirely outside the window
		auto outside = [this, slot, corners] (int32_t fw_vertex::*field, int32_t low, int32_t high)
		{
			bool above = false;
			bool below = false;
			for (int i = 0; i < corners; i++)
			{
				above |= m_vertices[slot[i]].*field > low;
				below |= m_vertices[slot[i]].*field < high;
			}
			return !above || !below;
		};
		if (outside(&fw_vertex::x, 0, m_fw.clip_r) || outside(&fw_vertex::y, 0, m_fw.clip_b))
		{
			m_mbox[MBOX_OFFSCREEN / 4]++;
			rec = p;
			continue;
		}

		// and the back-facing ones, in the firmware's wrapping 32-bit arithmetic
		const fw_vertex &v0 = m_vertices[slot[0]];
		const fw_vertex &v1 = m_vertices[slot[1]];
		const fw_vertex &v2 = m_vertices[slot[2]];
		auto edge = [] (int32_t a, int32_t b) { return uint32_t(int32_t(uint32_t(a) - uint32_t(b)) >> 5); };
		if (int32_t(edge(v2.x, v1.x) * edge(v0.y, v1.y) - edge(v2.y, v1.y) * edge(v0.x, v1.x)) <= 0)
		{
			m_mbox[MBOX_CULLED / 4]++;
			rec = p;
			continue;
		}

		if (hdr & 0x100000)
		{
			// two passes: the first draws the following texture with the coordinates scaled by 0x7f7f7f7f / 2^32
			if (second_pass)
			{
				for (int i = 0; i < corners; i++)
				{
					fw_vertex &vtx = m_vertices[slot[i]];
					vtx.u2 = shrink(vtx.u);
					if (count == 0)
					{
						vtx.v2 = shrink(vtx.v);
					}
					else if (perspective)
					{
						vtx.v2 = shrink(vtx.v + vtx.w);
					}
					else
					{
						vtx.v2 = shrink(vtx.v) + 0x20000000;
					}
				}
			}

			const uint32_t mode = m_fw.mode;
			const uint32_t texture = m_fw.texture;
			m_fw.mode = (mode & 0x10) | 0xb008;
			m_fw.texture = texture & 0xfc;
			m_fw.pending |= STATE_VALID | STATE_TEXTURE;
			if (count == 0)
			{
				m_fw.address = address + 0x10000;
				m_fw.pending |= STATE_ADDRESS;
			}
			ppc_send_state();
			ppc_emit_triangles(slot, corners, second_pass);

			if (count == 0)
			{
				m_fw.address = address;
				m_fw.pending |= STATE_ADDRESS;
			}
			m_fw.mode = mode;
			m_fw.texture = texture;
			m_fw.pending |= STATE_VALID | STATE_TEXTURE;
			ppc_send_state();
		}

		if (count != 0)
		{
			for (int i = 0; i < corners; i++)
			{
				fw_vertex &vtx = m_vertices[slot[i]];
				vtx.u += perspective ? (vtx.w >> 1) : 0x20000000;
			}
		}
		ppc_emit_triangles(slot, corners, false);
		rec = p;
	}

	// the address goes back to the one the command set up
	if (m_fw.address != address)
	{
		m_fw.address = address;
		m_fw.pending |= STATE_VALID | STATE_ADDRESS;
	}

	// skip the terminator
	return (rec < end) ? rec + 1 : rec;
}

// walk the vertex chain (0x78000f48): each new vertex closes a triangle with the two kept ones
void vp10x_state::ppc_emit_triangles(const int *slot, int count, bool second_pass)
{
	int upper = slot[0];
	int lower = slot[1];
	bool lower_is_new = true;
	if (m_vertices[upper].y > m_vertices[lower].y)
	{
		std::swap(upper, lower);
		lower_is_new = false;
	}

	for (int i = 2; i < count; i++)
	{
		const int added = slot[i];
		const int32_t y = m_vertices[added].y;
		int top;
		int middle;
		int bottom;

		if (y <= m_vertices[upper].y)
		{
			top = added;
			middle = upper;
			bottom = lower;
			if (lower_is_new)
			{
				lower = upper;
			}
			upper = added;
			lower_is_new = false;
		}
		else if (y <= m_vertices[lower].y)
		{
			top = upper;
			middle = added;
			bottom = lower;
			if (lower_is_new)
			{
				lower = added;
			}
			else
			{
				upper = added;
			}
		}
		else
		{
			top = upper;
			middle = lower;
			bottom = added;
			if (!lower_is_new)
			{
				upper = lower;
			}
			lower = added;
			lower_is_new = true;
		}

		// reject against the window and drop spans that round to one scanline (0x78001048)
		const fw_vertex &first = m_vertices[top];
		const fw_vertex &last = m_vertices[bottom];
		const fw_vertex &mid = m_vertices[middle];
		if ((first.x < m_fw.clip_l && last.x < m_fw.clip_l && mid.x < m_fw.clip_l)
			|| (first.x >= m_fw.clip_r && last.x >= m_fw.clip_r && mid.x >= m_fw.clip_r))
		{
			m_mbox[MBOX_REJECTED / 4]++;
			continue;
		}

		int32_t span_t = first.y;
		int32_t span_b = last.y;
		if (span_t < m_fw.clip_t)
		{
			if (span_b <= m_fw.clip_t)
			{
				m_mbox[MBOX_REJECTED / 4]++;
				continue;
			}
			span_t = m_fw.clip_t;
		}
		if (span_b > m_fw.clip_b)
		{
			if (span_t >= m_fw.clip_b)
			{
				m_mbox[MBOX_REJECTED / 4]++;
				continue;
			}
			span_b = m_fw.clip_b;
		}
		if (((span_t + 0xff) & ~0xff) >= ((span_b + 0xff) & ~0xff))
		{
			m_mbox[MBOX_FLAT / 4]++;
			continue;
		}

		fpga_triangle_block(m_vertices[top], m_vertices[middle], m_vertices[bottom], second_pass);
	}
}

/***************************************************************************
    Render engine model
***************************************************************************/

void vp10x_state::fpga_state_block(uint32_t hdr, uint32_t mode, uint32_t address, uint32_t attr_c, uint32_t texture, uint32_t attr_d, uint32_t attr_b)
{
	LOGMASKED(LOG_CMD, "fpga: state %08x mode %08x addr %08x tex %08x b %08x c %08x d %08x\n",
			hdr, mode, address, texture, attr_b, attr_c, attr_d);

	if (hdr & STATE_VALID)
	{
		m_fpga.mode = mode;
	}
	if (hdr & STATE_ADDRESS)
	{
		m_fpga.address = address;
		if ((hdr & STATE_VALID) && (mode & 0x100))
		{
			m_fpga.target = address;
		}
	}
	if (hdr & STATE_TEXTURE)
	{
		m_fpga.texture = texture;
	}
	if (hdr & STATE_ATTR_B)
	{
		m_fpga.attr_b = attr_b;
	}
	if (hdr & STATE_ATTR_C)
	{
		m_fpga.attr_c = attr_c;
	}
	if (hdr & STATE_ATTR_D)
	{
		m_fpga.attr_d = attr_d;
	}

	if ((mode & 0xffff) == 0xf020)
	{
		// texture load: the address is a tile table in shared RAM
		if (m_texture_count < std::size(m_textures))
		{
			m_textures[m_texture_count++] = fpga_texture{ address, mode >> 16, texture };
		}
		else
		{
			LOGMASKED(LOG_CMD, "fpga: texture table full, load of %08x dropped\n", address);
		}
	}

	if (hdr & STATE_FRAME)
	{
		fpga_frame_end();
	}
}

void vp10x_state::fpga_geometry_block(render_prim &prim)
{
	prim.regs = m_fpga;
	if (prim.type == 2)
	{
		LOGMASKED(LOG_PRIM, "prim: triangle (%d,%d) (%d,%d) (%d,%d) uv %08x %08x / %08x %08x / %08x %08x w %08x %08x %08x mode %03x addr %08x tex %08x c %08x\n",
				prim.vtx[0].x >> 8, prim.vtx[0].y >> 8, prim.vtx[1].x >> 8, prim.vtx[1].y >> 8, prim.vtx[2].x >> 8, prim.vtx[2].y >> 8,
				prim.vtx[0].u, prim.vtx[0].v, prim.vtx[1].u, prim.vtx[1].v, prim.vtx[2].u, prim.vtx[2].v,
				prim.vtx[0].w, prim.vtx[1].w, prim.vtx[2].w,
				prim.regs.mode, prim.regs.address, prim.regs.texture, prim.regs.attr_c);

		if (m_prims.size() < MAX_PRIMS)
		{
			m_prims.push_back(prim);
		}
		m_mbox[MBOX_PRIMS / 4]++;
		return;
	}

	LOGMASKED(LOG_PRIM, "prim: type %d (%d,%d)-(%d,%d) uv %08x %08x step %08x %08x w %08x color %08x light %03x %03x %03x mode %03x addr %08x tex %08x b %08x c %08x d %08x\n",
			prim.type, prim.x0 >> 20, prim.y0 >> 20, prim.x1 >> 20, prim.y1 >> 20, prim.u0, prim.v0, prim.dudx, prim.dvdy, prim.w, prim.color,
			prim.light[0], prim.light[1], prim.light[2],
			prim.regs.mode, prim.regs.address, prim.regs.texture, prim.regs.attr_b, prim.regs.attr_c, prim.regs.attr_d);

	if (m_prims.size() < MAX_PRIMS)
	{
		m_prims.push_back(prim);
	}
	m_mbox[MBOX_PRIMS / 4]++;
}

void vp10x_state::fpga_triangle_block(const fw_vertex &v0, const fw_vertex &v1, const fw_vertex &v2, bool second_pass)
{
	const fw_vertex *const corner[3] = { &v0, &v1, &v2 };
	render_prim prim = {};
	prim.type = 2;
	prim.vertices = 3;
	for (int i = 0; i < 3; i++)
	{
		prim.vtx[i].x = corner[i]->x;
		prim.vtx[i].y = corner[i]->y;
		prim.vtx[i].u = second_pass ? corner[i]->u2 : corner[i]->u;
		prim.vtx[i].v = second_pass ? corner[i]->v2 : corner[i]->v;
		prim.vtx[i].w = corner[i]->w;
		prim.vtx[i].color = corner[i]->color;
	}
	prim.clip_l = m_fw.clip_l >> 8;
	prim.clip_t = m_fw.clip_t >> 8;
	prim.clip_r = m_fw.clip_r >> 8;
	prim.clip_b = m_fw.clip_b >> 8;
	fpga_geometry_block(prim);
}

void vp10x_state::fpga_frame_end()
{
	LOGMASKED(LOG_CMD, "fpga: end of frame, %d primitives\n", int(m_prims.size()));

	rasterize();
	m_prims.clear();
}

// draw the frame's geometry blocks into their render targets in RAM
void vp10x_state::rasterize()
{
	const rectangle cliprect(0, 511, 0, 383);

	// nothing in the stream clears the depth words, so clear them for each 3D render target
	uint32_t cleared[4];
	int cleared_count = 0;
	for (const render_prim &prim : m_prims)
	{
		const uint32_t target = prim.regs.target & 0x07ffffff;
		if (m_native_list || prim.type != 2 || target + 384 * 0x1000 > 0x08000000)
		{
			continue;
		}
		if (std::find(cleared, cleared + cleared_count, target) != cleared + cleared_count)
		{
			continue;
		}
		if (cleared_count == std::size(cleared))
		{
			break;
		}

		cleared[cleared_count++] = target;
		for (int y = 0; y < 384; y++)
		{
			uint32_t *const line = &m_mainram[(target + y * 0x1000) / 4];
			for (int x = 0; x < 512; x++)
			{
				line[x * 2 + 1] = 0;
			}
		}
	}

	for (const render_prim &prim : m_prims)
	{
		// the mode 0xb000 pass writes the alpha that weights the texture bit 18 pass after it
		const bool alpha_pass = !prim.native && (prim.regs.mode & 0xf000) == 0xb000;
		const bool masked = !prim.native && !alpha_pass && (prim.regs.texture & 0x40000) != 0;

		const uint32_t target = prim.regs.target & 0x07ffffff;
		if (target + 384 * 0x1000 > 0x08000000)
		{
			continue;
		}

		vp101_render_data &data = m_renderer->object_data().next();
		data.target = &m_mainram[target / 4];
		data.source = nullptr;
		data.texture = nullptr;
		data.texture32 = nullptr;
		data.interpolated = false;
		data.affine = prim.native;
		// VP100 attribute C is a 0xAARRGGBB colour key; mode bit 1 lights the texels with the vertex colour
		data.keyed = prim.native && (prim.regs.attr_c >> 24) != 0;
		data.key = prim.regs.attr_c << 8;
		// the firmware's rectangles are lit by attribute D; how its triangles use it is not known
		data.lit = BIT(prim.regs.mode, 1) && (prim.native || prim.type != 2);
		for (int i = 0; i < 3; i++)
		{
			data.light[i] = prim.native ? prim.light[i] : fw_light(prim.regs.attr_d, i, prim.color);
			data.dldx[i] = prim.dldx[i];
			data.dldy[i] = -prim.dldy[i];
		}

		// only the alpha passes blend; attribute C's top byte is not an alpha
		data.blend = 0x100;
		data.alpha_pass = alpha_pass;
		data.masked = masked;
		// mode bit 4 primitives (2D overlays, the sky strip) and alpha passes leave the depth alone
		data.depth_test = true;
		data.depth_write = !alpha_pass && !(prim.regs.mode & 0x10);
		data.depth = prim.w;
		// attribute B is a 0xiiRRGGBB palette; texture bit 15 makes index 0 (the key colour) transparent
		const uint32_t palette = prim.regs.attr_b & 0x07ffffff;
		data.mask_texel0 = !prim.native && (prim.regs.texture & 0x8000) != 0;
		data.palette = (palette != 0 && palette + 0x400 <= 0x08000000) ? &m_mainram[palette / 4] : nullptr;
		data.x0 = prim.x0 >> 20;
		data.y0 = prim.y0 >> 20;
		data.u0 = prim.u0;
		data.v0 = prim.v0;
		data.dudx = prim.dudx;
		data.dvdy = -prim.dvdy;
		// a fill's colour: attribute D from the firmware, attribute C from a VP100 game
		data.color = prim.native ? ((prim.regs.attr_c << 8) | 0xff) : prim.regs.attr_d;

		if (prim.type != 0)
		{
			if (prim.regs.mode & 0x80)
			{
				// copy from another buffer: the address is its offset from the target
				const uint32_t source = (prim.regs.target + prim.regs.address) & 0x07ffffff;
				if (source + 384 * 0x1000 > 0x08000000)
				{
					continue;
				}
				data.source = &m_mainram[source / 4];
			}
			else
			{
				const uint32_t texture = prim.regs.address & 0x07ffffff;
				if (texture + 0x40000 > 0x08000000)
				{
					continue;
				}
				if (prim.native && native_32bit(prim.regs))
				{
					data.texture32 = &m_mainram[texture / 4];
				}
				else
				{
					data.texture = reinterpret_cast<const uint8_t *>(&m_mainram[0]) + texture;
				}
				data.color = prim.native ? 0xffffffff : (prim.regs.attr_c << 8);
			}
		}

		if (prim.type == 2)
		{
			data.interpolated = true;

			rectangle clip(std::max(prim.clip_l, 0), std::min(prim.clip_r, 511), std::max(prim.clip_t, 0), std::min(prim.clip_b, 383));
			clip &= cliprect;

			vp101_renderer::vertex_t v[5];
			for (int i = 0; i < prim.vertices; i++)
			{
				v[i].x = float(prim.vtx[i].x) / 256.0f;
				v[i].y = float(prim.vtx[i].y) / 256.0f;
				v[i].p[0] = float(prim.vtx[i].u);
				v[i].p[1] = float(prim.vtx[i].v);
				v[i].p[2] = float(prim.vtx[i].w);
				for (int j = 0; j < 3; j++)
				{
					v[i].p[3 + j] = float(prim.vtx[i].light[j]);
				}
			}
			if (prim.vertices == 5)
			{
				m_renderer->render_polygon<5, 6>(clip, vp101_renderer::render_delegate(&vp101_renderer::render_scanline, m_renderer.get()), v);
			}
			else if (prim.vertices == 4)
			{
				m_renderer->render_polygon<4, 6>(clip, vp101_renderer::render_delegate(&vp101_renderer::render_scanline, m_renderer.get()), v);
			}
			else
			{
				m_renderer->render_triangle<6>(clip, vp101_renderer::render_delegate(&vp101_renderer::render_scanline, m_renderer.get()), v[0], v[1], v[2]);
			}
			continue;
		}

		vp101_renderer::vertex_t v0(float(prim.x0) / float(1 << 20), float(prim.y0) / float(1 << 20));
		vp101_renderer::vertex_t v1(float(prim.x1) / float(1 << 20), float(prim.y1) / float(1 << 20));
		m_renderer->render_tile<0>(cliprect, vp101_renderer::render_delegate(&vp101_renderer::render_scanline, m_renderer.get()), v0, v1);
	}

	m_renderer->wait();
}

uint32_t vp10x_state::ppc_ctrl_r()
{
	return m_ppc_ctrl;
}

void vp10x_state::ppc_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ppc_ctrl);
	LOG("%s: ppc_ctrl_w = %08x\n", machine().describe_context(), data);

	// the game boots/kicks the PPC with the sequence 70 -> 78 -> 79 -> 71
	if ((data & 0xff) == 0x71)
	{
		ppc_boot();
	}
}

void vp10x_state::fb_base_w(uint32_t data)
{
	m_fb_base = data & 0x07ffffff;
}

uint32_t vp10x_state::fb_base_r()
{
	return m_fb_base;
}

uint32_t vp10x_state::sound_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			LOG("%s: sound_r: hardware flags(?): 11000000 = %08x\n", machine().describe_context(), 1 << 4);
			return (1 << 4); // Flag that sound hardware is initialized
		case 3:
		{
			uint32_t cmd_return = 0;
			bool known = true;
			switch (m_sound_cmd)
			{
				case 0xa6:
					cmd_return = 0x0000000e;
					break;
				case 0xfc:
					cmd_return = 0x00004352; // Some sort of info request? Looks like ASCII, 'CR' - codec is from Crystal Semi?
					break;
				default:
					known = false;
					break;
			}
			if (known)
			{
				LOG("%s: sound_r: sound command return value(?): 11000004 = %08x for cmd %02x\n", machine().describe_context(), cmd_return, m_sound_cmd);
			}
			else
			{
				LOG("%s: sound_r: sound command return value(?): 11000004 = %08x for unknown cmd %02x\n", machine().describe_context(), cmd_return, m_sound_cmd);
			}
			return cmd_return;
	}
	case 4:
		if (!machine().side_effects_disabled())
		{
			m_unk_sound_toggle ^= 1;
		}
		LOG("%s: sound_r: unknown: 11000010 = %08x\n", machine().describe_context(), m_unk_sound_toggle);
		return m_unk_sound_toggle; // Unknown
	default:
		LOG("%s: sound_r: %08x\n", machine().describe_context(), 0x11000000 | (offset << 2));
		return 0;
	}
}

void vp10x_state::sound_w(offs_t offset, uint32_t data)
{
	switch (offset)
	{
		case 1:
			LOG("%s: sound_w: command(?): 11000004 = %08x\n", machine().describe_context(), data);
			m_sound_cmd = data;
			return;
		default:
			LOG("%s: sound_w: %08x = %08x\n", machine().describe_context(), 0x11000000 | (offset << 2), data);
			return;
	}
}

uint32_t vp10x_state::snd_r(offs_t offset)
{
	const uint32_t reg = offset << 2;

	if (reg < 0x800)
	{
		const snd_voice &voice = m_voice[(reg >> 8) & 7];
		switch (reg & 0xff)
		{
			case 0x00: return voice.control;
			case 0x04: return voice.addr[0];
			case 0x08: return voice.count[0];
			case 0x0c: return voice.addr[1];
			case 0x10: return voice.count[1];
		}
		return 0;
	}

	switch (reg)
	{
		case 0x1000: return m_snd_key;
		case 0x1008: return m_snd_enable;
		case 0x100c: return m_snd_status;
	}
	return 0;
}

void vp10x_state::snd_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const uint32_t reg = offset << 2;

	if (reg < 0x800)
	{
		snd_voice &voice = m_voice[(reg >> 8) & 7];
		switch (reg & 0xff)
		{
			case 0x00: COMBINE_DATA(&voice.control); break;
			case 0x04: COMBINE_DATA(&voice.addr[0]); voice.phase = 0; break;
			case 0x08: COMBINE_DATA(&voice.count[0]); break;
			case 0x0c: COMBINE_DATA(&voice.addr[1]); break;
			case 0x10: COMBINE_DATA(&voice.count[1]); break;
		}
		return;
	}

	switch (reg)
	{
		case 0x1000: COMBINE_DATA(&m_snd_key); break;
		case 0x1008: COMBINE_DATA(&m_snd_enable); break;

		case 0x100c:
			// the refill request is acknowledged by writing the voice's bit back
			m_snd_status &= ~data;
			snd_update_irq();
			break;
	}
}

// a voice that wants more data raises FPGA interrupt bit 6
void vp10x_state::snd_update_irq()
{
	if (m_snd_status)
	{
		m_int_status |= 0x40;
	}
	else
	{
		m_int_status &= ~0x40;
	}
	update_irqs();
}

TIMER_CALLBACK_MEMBER(vp10x_state::snd_tick)
{
	const uint8_t *const ram = reinterpret_cast<const uint8_t *>(&m_mainram[0]);

	for (uint32_t frame = 0; frame < SND_BLOCK; frame++)
	{
		int32_t mix[2] = { 0, 0 };

		for (int n = 0; n < 8; n++)
		{
			snd_voice &voice = m_voice[n];
			if (!BIT(m_snd_key, n))
			{
				continue;
			}

			// control bit 17 halves the rate, for sounds below 30 kHz
			voice.phase ^= 1;
			const bool step = !BIT(voice.control, 17) || voice.phase;
			const uint32_t volume = voice.control & 0xffff;

			// one stream into both outputs
			const uint32_t sample = voice.addr[0] & 0x07fffffe;
			const int32_t value = int16_t(ram[sample] | (ram[sample + 1] << 8));
			const int32_t scaled = (value * int32_t(volume)) >> 16;
			mix[0] += scaled;
			mix[1] += scaled;

			if (!step)
			{
				continue;
			}

			voice.addr[0] = (voice.addr[0] + 2) & 0x1fffffff;
			voice.count[0]++;

			if (voice.count[0] > 0x01000000)
			{
				if (BIT(voice.control, 16))
				{
					// a looping voice restarts at the loop point without telling the CPU
					voice.addr[0] = (voice.addr[1] + 2) & 0x1fffffff;
					voice.count[0] = voice.count[1];
				}
				else
				{
					// A one-shot stops itself and asks the CPU to look at it.
					m_snd_key &= ~(1U << n);
					m_snd_status |= 1U << n;
					snd_update_irq();
				}
			}
		}

		for (int channel = 0; channel < 2; channel++)
		{
			m_snd_block[(frame * 2) + channel] = int16_t(std::clamp(mix[channel], -32768, 32767));
		}
	}

	for (int channel = 0; channel < 2; channel++)
	{
		m_dmadac[channel]->flush();
		m_dmadac[channel]->transfer(channel, 1, 2, SND_BLOCK, m_snd_block);
	}
}

void vp10x_state::main_map(address_map &map)
{
	map(0x00000000, 0x07ffffff).ram().share("mainram");
	map(0x10000000, 0x10000003).rw(FUNC(vp10x_state::int_enable_r), FUNC(vp10x_state::fpga_ctrl_w));
	map(0x10000004, 0x10000007).rw(FUNC(vp10x_state::int_status_r), FUNC(vp10x_state::int_ack_w));
	map(0x10000008, 0x1000000b).rw(FUNC(vp10x_state::fpga_doorbell_r), FUNC(vp10x_state::fpga_doorbell_w));
	map(0x10000010, 0x10000013).rw(FUNC(vp10x_state::fpga_target_r), FUNC(vp10x_state::fpga_target_w));
	map(0x10000034, 0x10000037).r(FUNC(vp10x_state::fpga_busy_r));
	map(0x11000000, 0x11000013).rw(FUNC(vp10x_state::sound_r), FUNC(vp10x_state::sound_w));
	map(0x12000000, 0x12000003).rw(FUNC(vp10x_state::int_enable_r), FUNC(vp10x_state::int_enable_w));
	map(0x12000004, 0x12000007).r(FUNC(vp10x_state::int_status_r));
	map(0x14000000, 0x14000003).r(FUNC(vp10x_state::test_r));
	map(0x15000000, 0x15001fff).rw(FUNC(vp10x_state::snd_r), FUNC(vp10x_state::snd_w));
	map(0x18000000, 0x1800000f).rw(FUNC(vp10x_state::dispctl_r), FUNC(vp10x_state::dispctl_w));
	map(0x18000010, 0x18000013).rw(FUNC(vp10x_state::fb_base_r), FUNC(vp10x_state::fb_base_w));
	map(0x18000020, 0x1800002f).r(FUNC(vp10x_state::gun_latch_r));
	map(0x18800000, 0x18800fff).rw(FUNC(vp10x_state::clut_r), FUNC(vp10x_state::clut_w));
	map(0x1a000000, 0x1a003fff).rw(FUNC(vp10x_state::mbox_r), FUNC(vp10x_state::mbox_w));
	map(0x1a800000, 0x1a800003).rw(FUNC(vp10x_state::ppc_ctrl_r), FUNC(vp10x_state::ppc_ctrl_w));
	map(0x1c000000, 0x1c000003).w(FUNC(vp10x_state::tty_w));        // RSS OS code uses this one
	map(0x1c000014, 0x1c000017).r(FUNC(vp10x_state::tty_ready_r));
	map(0x1c400000, 0x1c400003).w(FUNC(vp10x_state::tty_w));        // boot ROM code uses this one
	map(0x1c400014, 0x1c400017).r(FUNC(vp10x_state::tty_ready_r));
	map(0x1ca00000, 0x1ca00003).portr("GUNX1");
	map(0x1ca00004, 0x1ca00007).portr("GUNY1");
	map(0x1ca00008, 0x1ca0000b).portr("GUNBUTTONS");
	map(0x1ca0000c, 0x1ca0000f).portr("BUTTONS");
	map(0x1ca00010, 0x1ca00013).portr("DIPS");
	map(0x1ce00000, 0x1ce00003).w(FUNC(vp10x_state::gun_out_w));
	map(0x1cf00000, 0x1cf00003).noprw().nopr();
	map(0x1d000004, 0x1d000007).rw(FUNC(vp10x_state::ide_irq_r), FUNC(vp10x_state::ide_irq_w));
	map(0x1d000030, 0x1d000033).w(FUNC(vp10x_state::dmaaddr_w));    // ATA DMA destination address
	map(0x1d000040, 0x1d00005f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w)).umask32(0x0000ffff);
	map(0x1d000060, 0x1d00007f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w)).umask32(0x0000ffff);

	map(0x1f200000, 0x1f200003).rw(FUNC(vp10x_state::pic_r), FUNC(vp10x_state::pic_w));
	map(0x1f800000, 0x1f807fff).ram().share("nvram");   // 32K battery-backed RAM; gun calibration record at 0x1f806000
	map(0x1fc00000, 0x1fffffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( jnero )
	PORT_START("GUNX1")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_X ) PORT_MINMAX(0x000, 0x1ff) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNY1")
	PORT_BIT( 0x1ff, 0x0c0, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x000, 0x17f) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNX2")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_X ) PORT_MINMAX(0x000, 0x1ff) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNY2")
	PORT_BIT( 0x1ff, 0x0c0, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x000, 0x17f) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNBUTTONS")
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Trigger")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Sense")
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Pump")
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Trigger")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Sense")
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("Pump")
	PORT_BIT( 0xffffff88, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x00000040, IP_ACTIVE_LOW )
	PORT_BIT (0x00000080, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT (0x00001000, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT (0x00002000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0xffffcf30, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DIPS")
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Test ) )
	PORT_DIPSETTING(          0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xffffefaf, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x00001040, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( specfrce )
	PORT_START("GUNX1")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_X ) PORT_MINMAX(0x000, 0x1ff) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("GUNY1")
	PORT_BIT( 0x1ff, 0x0c0, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x000, 0x17f) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("GUNBUTTONS")
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Trigger")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_NAME("Sense")
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Pump")
	PORT_BIT( 0xfffffff8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x00000040, IP_ACTIVE_LOW )
	PORT_BIT (0x00000080, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT (0x00001000, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT (0x00002000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0xffffcf38, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DIPS")
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Test ) )
	PORT_DIPSETTING(          0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xffffefaf, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x00001040, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END

void vp10x_state::vp101(machine_config &config)
{
	VR5500LE(config, m_maincpu, 400000000);
	m_maincpu->set_dcache_size(32768);
	m_maincpu->set_system_clock(100000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &vp10x_state::main_map);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(vp10x_state::screen_update));
	screen.set_size(512, 384);
	screen.set_visarea(0, 511, 0, 383);
	screen.screen_vblank().set(FUNC(vp10x_state::vblank_w));

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
	m_ata->dmarq_handler().set(FUNC(vp10x_state::dmarq_w));
	m_ata->irq_handler().set(FUNC(vp10x_state::ata_irq_w));

	SPEAKER(config, "speaker", 2).front();
	DMADAC(config, m_dmadac[0]).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	DMADAC(config, m_dmadac[1]).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

ROM_START(jnero)
	ROM_REGION(0x400000, "maincpu", 0)  // Boot ROM
	ROM_LOAD( "d710.05523.bin", 0x000000, 0x100000, CRC(6054a066) SHA1(58e68b7d86e6f24c79b99c8406e86e3c14387726) )

	ROM_REGION(0x80000, "pic", 0)       // PIC18c422 program - read-protected, need dumped
	ROM_LOAD( "8722a-1206.bin", 0x000000, 0x80000, NO_DUMP )

	ROM_REGION(0x8000, "nvram", 0)      // MAME-generated: both guns calibrated on the crosshair
	ROM_LOAD( "jnero_nvram.bin", 0x000000, 0x8000, CRC(d38c6d04) SHA1(ccb4b228c2dbac9da4db8e60541a85f2542bf9ac) )

	DISK_REGION( "ata:0:hdd" )    // ideally an IDENTIFY page from a real drive should be the IDTN metadata,
								  // but even factory-new boardsets came with a variety of HDD makes and models
	DISK_IMAGE("jn010108", 0, SHA1(5a27990478b65fca801c3a6518c519c5b4ca934d) )
ROM_END

ROM_START(specfrce)
	ROM_REGION(0x400000, "maincpu", 0)  // Boot ROM
	ROM_SYSTEM_BIOS(0, "default", "rev. 3.6")
	ROMX_LOAD( "boot 3.6.u4.27c801", 0x000000, 0x100000, CRC(b1628dd9) SHA1(5970d31b0cf3d0c1ab4b10ee8e54d2696fafde24), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "r35", "rev. 3.5")
	ROMX_LOAD( "special_forces_boot_v3.5.u4", 0x000000, 0x100000, CRC(ae8dfdf0) SHA1(d64130e710d0c70095ad8ebd4e2194b8c461be4a), ROM_BIOS(1) ) // Newer, but keep both in driver
	ROM_SYSTEM_BIOS(2, "r34", "rev. 3.4")
	ROMX_LOAD( "special_forces_boot_v3.4.u4", 0x000000, 0x100000, CRC(db4862ac) SHA1(a1e886d424cf7d26605e29d972d48e8d44ae2d58), ROM_BIOS(2) )

	ROM_REGION(0x80000, "pic", 0)       // PIC18c422 I/P program - read-protected, need dumped
	ROM_LOAD( "special_forces_et_u7_rev1.2.u7", 0x000000, 0x80000, NO_DUMP )

	ROM_REGION(0x8000, "nvram", 0)      // Gun calibrated
	ROM_LOAD( "specfrce_nvram.bin", 0x000000, 0x8000, CRC(101c924b) SHA1(911e9995f0fe9bd43467470e8c4699e6d816863d) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE("sf010200", 0, SHA1(33c35fd5e110ff06330e0f0313fcd75d5c64a090) )
ROM_END

ROM_START(specfrceo)
	ROM_REGION(0x400000, "maincpu", 0)  // Boot ROM
	ROM_SYSTEM_BIOS(0, "default", "rev. 3.6")
	ROMX_LOAD( "boot 3.6.u4.27c801", 0x000000, 0x100000, CRC(b1628dd9) SHA1(5970d31b0cf3d0c1ab4b10ee8e54d2696fafde24), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "r35", "rev. 3.5")
	ROMX_LOAD( "special_forces_boot_v3.5.u4", 0x000000, 0x100000, CRC(ae8dfdf0) SHA1(d64130e710d0c70095ad8ebd4e2194b8c461be4a), ROM_BIOS(1) ) /* Newer, but keep both in driver */
	ROM_SYSTEM_BIOS(2, "r34", "rev. 3.4")
	ROMX_LOAD( "special_forces_boot_v3.4.u4", 0x000000, 0x100000, CRC(db4862ac) SHA1(a1e886d424cf7d26605e29d972d48e8d44ae2d58), ROM_BIOS(2) )

	ROM_REGION(0x80000, "pic", 0)       // PIC18c422 I/P program - read-protected, need dumped
	ROM_LOAD( "special_forces_et_u7_rev1.2.u7", 0x000000, 0x80000, NO_DUMP )

	ROM_REGION(0x8000, "nvram", 0)      // Gun calibrated
	ROM_LOAD( "specfrce_nvram.bin", 0x000000, 0x8000, CRC(101c924b) SHA1(911e9995f0fe9bd43467470e8c4699e6d816863d) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE("sf010101", 0, SHA1(59b5e3d8e1d5537204233598830be2066aad0556) )
ROM_END

} // anonymous namespace


GAME( 2002, specfrce,  0,        vp101, specfrce, vp10x_state, empty_init, ROT0, "ICE / Play Mechanix", "Special Forces Elite Training (v01.02.00)", MACHINE_NOT_WORKING )
GAME( 2002, specfrceo, specfrce, vp101, specfrce, vp10x_state, empty_init, ROT0, "ICE / Play Mechanix", "Special Forces Elite Training (v01.01.01)", MACHINE_NOT_WORKING )
GAME( 2004, jnero,     0,        vp101, jnero,    vp10x_state, empty_init, ROT0, "ICE / Play Mechanix", "Johnny Nero Action Hero (v01.01.08)",       MACHINE_NOT_WORKING )
