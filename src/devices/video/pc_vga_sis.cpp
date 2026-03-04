// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Implementation of SiS family (S)VGA chipset

SiS6326: VBE 2.0, Multi Buffering & Virtual Scrolling available
SiS630:  VBE 3.0, Multi Buffering & Virtual Scrolling available

Notes:
- In logging we omit "Extended", for searching purposes it's suggested to use the <reg_name>%X
  nomenclature anyway i.e. CR19 or SR7

TODO:
- Extended 4bpp modes don't work (cfr. SDD item);
- Refresh rate for extended modes;
- interlace scaling;
- linear addressing
\- currently hardwired in BAR0, which matches the setup done here. What happens when it don't?
- Interrupts;
- Verify single segment mode;
- AGP/HostBus/Turbo Queue i/f (as separate device, currently in sis6326 PCI);
- DDC;
- Bridge with a secondary TV out (SiS301 for '630);
- Verify matches with earlier SiS PCI cards, backport;

TODO (sis630):
- Output scaling, cfr. xubuntu 6.10 splash screen at 1024x768x32 (really interlace as above?);
- fails banked modes (different setup?), fails extended start addresses;

**************************************************************************************************/

#include "emu.h"
#include "pc_vga_sis.h"

#include "screen.h"

#define LOG_SEQ    (1U << 1) // extended sequencer register descriptions

#define LOG_LOCKED (1U << 4) // log lock/unlock sequences

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#define LOGSEQ(...)       LOGMASKED(LOG_SEQ, __VA_ARGS__)
#define LOGLOCKED(...)    LOGMASKED(LOG_LOCKED, __VA_ARGS__)

#include "logmacro.h"

#define DEBUG_VRAM_VIEWER 0

// NOTE: several of these are actually MB integrated with different names

// retroactively known as 6201 in drivers
//DEFINE_DEVICE_TYPE(SIS86C201_VGA, sis86c201_vga_device, "sis86c201_vga", "SiS 86C201 VGA i/f")
//DEFINE_DEVICE_TYPE(SIS6202_VGA,   sis6202_vga_device,   "sis6202_vga",   "SiS 6202 VGA i/f")
//DEFINE_DEVICE_TYPE(SIS6205_VGA,   sis6205_vga_device,   "sis6205_vga",   "SiS 6205 VGA i/f")
//DEFINE_DEVICE_TYPE(SIS6225_VGA,   sis6225_vga_device,   "sis6225_vga",   "SiS 6225 VGA i/f")
//DEFINE_DEVICE_TYPE(SIS6215_VGA,   sis6215_vga_device,   "sis6215_vga",   "SiS 6215 VGA i/f")

// NOTE: Everest Home app actually uses 86C326 as Video Adapter name, while GPU is 6326
DEFINE_DEVICE_TYPE(SIS6326_VGA,   sis6326_vga_device,   "sis6326_vga",   "SiS 6326 VGA i/f")
DEFINE_DEVICE_TYPE(SIS630_VGA,    sis630_vga_device,    "sis630_vga",    "SiS 630 VGA i/f")

sis6326_vga_device::sis6326_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sis6326_vga_device(mconfig, SIS6326_VGA, tag, owner, clock)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(sis6326_vga_device::crtc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(sis6326_vga_device::sequencer_map), this));
	m_tvout_space_config = address_space_config("tvout_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(sis6326_vga_device::tvout_map), this));
}

sis6326_vga_device::sis6326_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, type, tag, owner, clock)
	, m_md20_cb(*this, 0)
	, m_md21_cb(*this, 0)
	, m_md23_cb(*this, 0)
	, m_md27_cb(*this, 0)
{
}

device_memory_interface::space_config_vector sis6326_vga_device::memory_space_config() const
{
	auto r = svga_device::memory_space_config();
	r.emplace_back(std::make_pair(EXT_REG,     &m_tvout_space_config));
	return r;
}

void sis6326_vga_device::device_start()
{
	svga_device::device_start();
	zero();

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;

	// copy over interfaces
	vga.memory = std::make_unique<uint8_t []>(vga.svga_intf.vram_size);
	memset(&vga.memory[0], 0, vga.svga_intf.vram_size);

	save_item(NAME(m_crtc_unlock_reg));
	save_item(NAME(m_seq_unlock_reg));
	save_item(NAME(m_ramdac_mode));
	save_item(NAME(m_ext_sr07));
	save_item(NAME(m_crt_cpu_threshold));
	save_item(NAME(m_ext_sr0b));
	save_item(NAME(m_ext_sr0c));
	save_item(NAME(m_ext_ddc));
	save_item(NAME(m_ext_sr12));
	save_item(NAME(m_ext_sr13));
	save_item(NAME(m_suspend_time));
	save_item(NAME(m_standby_time));
	save_item(NAME(m_ext_sr23));
	save_item(NAME(m_mclk_int));
	save_item(NAME(m_vclk_int));
	save_item(NAME(m_turbo_queue_address));
	save_item(NAME(m_page_size_select));
	save_item(NAME(m_dram_fb_size));
	save_item(NAME(m_fast_page_address_latch));
	save_item(NAME(m_ext_sr33));
	save_item(NAME(m_ext_sr34));
	save_item(NAME(m_ext_sr35));
	save_item(NAME(m_ext_sr38));
	save_item(NAME(m_ext_sr39));
	save_item(NAME(m_mpeg_turbo_queue_address));
	save_item(NAME(m_ext_sr3c));
	save_item(NAME(m_mclk_gen));
	save_item(NAME(m_vclk_gen));
	save_item(NAME(m_ext_ge26));
	save_item(NAME(m_ext_ge27));
	save_item(NAME(m_linear_address));

	save_item(NAME(m_crtc_hcounter_latch));
	save_item(NAME(m_crtc_vcounter_latch));

	save_item(STRUCT_MEMBER(m_cursor, address_base));
	save_item(STRUCT_MEMBER(m_cursor, color_cache));
	save_item(STRUCT_MEMBER(m_cursor, color));
	save_item(STRUCT_MEMBER(m_cursor, x));
	save_item(STRUCT_MEMBER(m_cursor, y));
	save_item(STRUCT_MEMBER(m_cursor, x_preset));
	save_item(STRUCT_MEMBER(m_cursor, y_preset));
	save_item(STRUCT_MEMBER(m_cursor, pattern_select));
	save_item(STRUCT_MEMBER(m_cursor, side_pattern_enable));

	save_item(STRUCT_MEMBER(m_tv, pycin));
	save_item(STRUCT_MEMBER(m_tv, enyf));
	save_item(STRUCT_MEMBER(m_tv, encf));
	save_item(STRUCT_MEMBER(m_tv, tvsense));
}

void sis6326_vga_device::device_reset()
{
	svga_device::device_reset();

	m_crtc_unlock_reg = false;
	m_seq_unlock_reg = false;
	m_ramdac_mode = 0;
	m_crt_cpu_threshold[0] = m_crt_cpu_threshold[1] = 0;
	m_suspend_time = m_standby_time = 0;
	m_ext_sr07 = m_ext_sr0b = m_ext_sr0c = m_ext_sr23 = m_ext_sr33 = 0;
	m_ext_sr34 = m_ext_sr35 = m_ext_sr38 = m_ext_sr39 = m_ext_sr3c = 0;
	m_ext_ge26 = m_ext_ge27 = 0;
	m_mclk_int[0] = m_mclk_int[1] = 0;
	m_vclk_int[0] = m_vclk_int[1] = 0;
	m_page_size_select = 0;
	m_dram_fb_size = 0;
	m_fast_page_address_latch[0] = m_fast_page_address_latch[1] = m_fast_page_address_latch[2] = 0;
	// irrelevant really
	m_crtc_hcounter_latch = m_crtc_vcounter_latch = 0xffff;

	// everything else shouldn't matter for cursor
	// initialize fixed part here: HW cannot set any other bit beyond 21 ~ 18.
	// On win98se this will map at bottom of VRAM i.e. at $3f'fc00 on 4MiB cards
	m_cursor.address_base = 0x03'fc00;

	m_turbo_queue_address = 0;
	m_mpeg_turbo_queue_address = 0;
	m_linear_address[0] = 0;
	m_linear_address[1] = 0;
	m_ext_ddc = 0;
}

void sis6326_vga_device::io_3cx_map(address_map &map)
{
	svga_device::io_3cx_map(map);
	// TODO: for '630 it's always with dual segment enabled?
	// May be like trident_vga where there's a specific register

	// read by gamecstl Kontron BIOS
	map(0x0b, 0x0b).lrw8(
		NAME([this] (offs_t offset) {
			return svga.bank_r & 0x3f;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (BIT(m_ext_sr0b, 3))
				svga.bank_r = data & 0x3f;
		})
	);
	map(0x0d, 0x0d).lrw8(
		NAME([this] (offs_t offset) {
			if (BIT(m_ext_sr0b, 3))
				return svga.bank_w & 0x3f;

			return (svga.bank_w & 0xf) << 4 | (svga.bank_r & 0xf);
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (BIT(m_ext_sr0b, 3))
				svga.bank_w = data & 0x3f;
			else
			{
				svga.bank_w = (data >> 4) & 0xf;
				svga.bank_r = data & 0xf;
			}
		})
	);
}

// Direct Draw uses this
void sis6326_vga_device::crtc_strobe_latch()
{
	m_crtc_hcounter_latch = screen().hpos();
	m_crtc_vcounter_latch = screen().vpos();
}

void sis6326_vga_device::crtc_map(address_map &map)
{
	svga_device::crtc_map(map);
	// CR19/CR1A Extended Signature Read-Back 0/1

	// CR1B CRT horizontal counter (r/o)
	map(0x1b, 0x1b).lr8(NAME([this] (offs_t offset) { return m_crtc_hcounter_latch & 0xff; }));
	// CR1C CRT vertical counter (r/o)
	map(0x1c, 0x1c).lr8(NAME([this] (offs_t offset) { return m_crtc_vcounter_latch & 0xff; }));
	// CR1D CRT overflow counter (r/o)
	map(0x1d, 0x1d).lr8(NAME([this] (offs_t offset) {
		return (BIT(m_crtc_hcounter_latch, 8) << 4) | ((m_crtc_vcounter_latch & 0x700) >> 8);
	}));

	// CR1E Extended Signature Read-Back 2

	// CR20: CRT Counter Trigger Port
	// NOTE: doc claims "r/o" but Direct Draw end of test actually writes on it
	// assume just strobe address
	map(0x20, 0x20).lrw8(
		NAME([this] (offs_t offset) {
			if (!machine().side_effects_disabled())
			{
				LOG("CR20: Counter trigger read\n");
				crtc_strobe_latch();
			}
			return 0xff;
		}),
		NAME([this] (offs_t offset, u8 data) {
			(void)data;
			LOG("CR20: Counter trigger write\n");
			crtc_strobe_latch();
		})
	);
	// CR26 Attribute Controller Index read-back
	// TODO: bit 5 is "video enable" at least for '6326
	map(0x26, 0x26).lr8(
		NAME([this] (offs_t offset) { return vga.attribute.index; })
	);

	// Password/Identification Register
	map(0x80, 0x80).lrw8(
		NAME([this] (offs_t offset) {
			return m_crtc_unlock_reg ? 0xa1 : 0x21;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: reimplement me thru memory_view or direct handler override
			m_crtc_unlock_reg = (data == 0x86);
			LOGLOCKED("CR80: Unlock register write %02x (%s)\n", data, m_crtc_unlock_reg ? "unlocked" : "locked");
		})
	);
	// ...

	map(0xe0, 0xe0).lrw8(
		NAME([this] (offs_t offset) -> u8 {
			if (!m_crtc_unlock_reg)
			{
				LOGLOCKED("CRE0: attempt to read TV OUT index while locked\n");
				return 0xff;
			}
			return m_tvout_index;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (!m_crtc_unlock_reg)
			{
				LOGLOCKED("CRE0: attempt to write TV OUT index while locked %02x\n", data);
				return;
			}
			m_tvout_index = data;
		})
	);
	map(0xe1, 0xe1).lrw8(
		NAME([this] (offs_t offset) -> u8 {
			if (!m_crtc_unlock_reg)
			{
				LOGLOCKED("CRE0: attempt to read TV OUT data while locked [%02x]\n", m_tvout_index);
				return 0;
			}
			return space(EXT_REG).read_byte(m_tvout_index);
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (!m_crtc_unlock_reg)
			{
				LOGLOCKED("CRE0: attempt to write TV OUT data while locked [%02x] %02x\n", m_tvout_index, data);
				return;
			}
			space(EXT_REG).write_byte(m_tvout_index, data);
		})
	);
	// TODO: e2 / e3 accessed (alias of TV OUT?)
}

void sis6326_vga_device::sequencer_map(address_map &map)
{
	svga_device::sequencer_map(map);
	// Password/Identification register
	map(0x05, 0x05).lrw8(
		NAME([this] (offs_t offset) {
			return m_seq_unlock_reg ? 0xa1 : 0x21;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: reimplement me thru memory_view or direct handler override
			m_seq_unlock_reg = (data == 0x86);
			LOGLOCKED("SR5: Unlock register write %02x (%s)\n", data, m_seq_unlock_reg ? "unlocked" : "locked");
		})
	);
	/*
	 * x--- ---- GFX mode linear addressing enable
	 * -x-- ---- GFX hardware cursor display
	 * --x- ---- GFX mode interlace
	 * ---x ---- True Color enable (ties with index 0x07 bit 2)
	 * ---- x--- RGB16 enable
	 * ---- -x-- RGB15 enable
	 * ---- --x- enhanced GFX mode enable
	 * ---- ---x enhanced text mode enable
	 */
	map(0x06, 0x06).lrw8(
		NAME([this] (offs_t offset) {
			return m_ramdac_mode;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_ramdac_mode = data;
			// verbose in win98se
			LOGSEQ("SR06: RAMDAC mode %02x\n", data);

			svga.rgb8_en = svga.rgb15_en = svga.rgb16_en = svga.rgb24_en = svga.rgb32_en = 0;

			if (BIT(data, 1))
			{
				// TODO: who wins on multiple bits enable?
				if (BIT(data, 1))
					svga.rgb8_en = 1;
				if (BIT(data, 2))
					svga.rgb15_en = 1;
				if (BIT(data, 3))
					svga.rgb16_en = 1;
				std::tie(svga.rgb24_en, svga.rgb32_en) = flush_true_color_mode();
			}
		})
	);
	/*
	 * x--- ---- Merge video line buffer into CRT FIFO
	 * -x-- ---- Enable feature connector
	 * --x- ---- Internal RAMDAC power saving mode (TODO: active low or high?)
	 * ---x ---- Extended video clock frequency /2
	 * ---- x--- Multi-line pre-fetch (TODO: active low or high?)
	 * ---- -x-- Enable 24bpp true color (active low on SiS6326)
	 * ---- --x- High speed DAC
	 * ---- ---x External DAC reference voltage input
	 */
	map(0x07, 0x07).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_sr07;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR07: Misc. Control 0 %02x\n", data);
			LOGSEQ("\tMerge video line buffer %d feature conn. %d RAMDAC power saving %d video clock freq / 2 %d\n"
				, BIT(data, 7)
				, BIT(data, 6)
				, BIT(data, 5)
				, BIT(data, 4)
			);
			LOGSEQ("\tMulti-line prefetch %d 24-bit color palette %d High Speed DAC %d External DAC reference voltage %d\n"
				, BIT(data, 3)
				, BIT(data, 2)
				, BIT(data, 1)
				, BIT(data, 0)
			);
			m_ext_sr07 = data;
			std::tie(svga.rgb24_en, svga.rgb32_en) = flush_true_color_mode();
		})
	);

	// CRT/CPU Threshold Control
	// [0]
	// xxxx ---- CRT/CPU Arbitration Threshold Low
	// ---- xxxx CRT/Engine Threshold High
	// [1]
	// xxxx ---- ASCII/Attribute Threshold
	// ---- xxxx CRT/CPU Threshold High
	map(0x08, 0x09).lrw8(
		NAME([this] (offs_t offset) {
			return m_crt_cpu_threshold[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR%02X: CRT/CPU Threshold Control %d %02x\n", offset + 8, offset, data);
			m_crt_cpu_threshold[offset] = data;
		})
	);
	map(0x0a, 0x0a).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_vert_overflow;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR0A: CRT Overflow %02x\n", data);
			m_ext_vert_overflow = data;
			vga.crtc.offset = (vga.crtc.offset & 0x00ff) | ((data & 0xf0) << 4);
			vga.crtc.vert_retrace_start = (vga.crtc.vert_retrace_start & 0x03ff) | (BIT(data, 3) << 10);
			vga.crtc.vert_blank_start =   (vga.crtc.vert_blank_start & 0x03ff)   | (BIT(data, 2) << 10);
			vga.crtc.vert_disp_end =      (vga.crtc.vert_disp_end & 0x03ff)      | (BIT(data, 1) << 10);
			vga.crtc.vert_total =         (vga.crtc.vert_total & 0x03ff)         | (BIT(data, 0) << 10);
			recompute_params();
		})
	);

	// x--- ---- True Color RGB select (0) RGB (1) BGR
	// -xx- ---- MMIO select
	// -00- ---- Disable
	// -01- ---- Select A:0000 segment
	// -10- ---- Select B:0000 segment
	// -11- ---- Select PCI BAR1
	// ---x ---- True Color frame rate modulation
	// ---- x--- Dual Segment register
	// ---- -x-- I/O gating enable while write-buffer not empty
	// ---- --x- 16-color packed pixel
	// ---- ---x CPU driven BitBlt enable
	map(0x0b, 0x0b).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_sr0b;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// verbose in win98se
			LOGSEQ("SR0B: Misc. Control 1 %02x\n", data);
			LOGSEQ("\tTrue Color format %s MMIO space sel %d True Color frame rate modulation %d\n"
				, BIT(data, 7) ? "BGR" : "RGB"
				, (data >> 5) & 3
				, BIT(data, 4)
			);
			LOGSEQ("\tDual segment enable %d I/O gating enable %d 16-color packed pixel %d CPU-driven BITBLT enable %d\n"
				, BIT(data, 3)
				, BIT(data, 2)
				, BIT(data, 1)
				, BIT(data, 0)
			);
			m_ext_sr0b = data;
		})
	);

	// x--- ---- Graphic mode 32-bit memory access enable
	// -x-- ---- Text mode 16-bit memory access enable
	// --x- ---- Read-ahead cache operation enable
	// ---- x--- Test mode
	// ---- -xx- Memory configuration
	// ---- -00- 1MByte/1 bank
	// ---- -01- 2MByte/2 banks
	// ---- -10- 4MByte/2 or 4 banks
	// ---- -11- 1Mbyte/2 banks
	// ---- ---x Sync reset timing generator
	map(0x0c, 0x0c).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_sr0c;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR0C: Misc. Control 2 %02x\n", data);
			LOGSEQ("\tGraphics mode 32bit %d Text mode 16bit %d Read-ahead cache %d\n"
				, BIT(data, 7)
				, BIT(data, 6)
				, BIT(data, 5)
			);
			LOGSEQ("\tTest mode %d Memory config %d Sync reset timing gen %d\n"
				, BIT(data, 3)
				, (data >> 1) & 3
				, BIT(data, 0)
			);
			m_ext_sr0c = data;
		})
	);

	//map(0x0d, 0x0e) Ext. Config Status (r/o)
	map(0x0d, 0x0d).lr8(
		// x--- ---- MD23 Enable 64K ROM
		// -x-- ---- MD22 Clock Generator Select (0) internal (1) external (test only)
		// --x- ---- MD21 AGP 2X Transfer enable
		// ---x ---- MD20 AGP bus enable
		// ---- x--- MD19 <reserved>
		// ---- -x-- MD18 NTSC (0) PAL (1)
		// ---- --x- MD17 Video subsystem power-on disable
		// ---- ---x MD16 Video subsystem port (0) $3c3 (1) $46e8
		NAME([this] () {
			return (m_md23_cb() << 7) | (m_md21_cb() << 5) | (m_md20_cb() << 4) | 1;
		})
	);
	map(0x0e, 0x0e).lr8(
		// xxx- ---- MD31~MD29 DRAM speed setting (000) SGRAM 66 MHz
		// ---x ---- MD28 disable VMI interface
		// ---- x--- MD27 INTA# enable
		// ---- -x-- MD26 BIOS ROM disable
		// ---- --xx MD25~MD24 <reserved>
		NAME([this] () { return (m_md27_cb() << 3); })
	);
	map(0x0f, 0x10).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_scratch[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR%02X: Scratch %d %02x\n", offset + 0xf, offset, data);
			m_ext_scratch[offset] = data;
		})
	);

	// DDC register
	map(0x11, 0x11).lrw8(
		NAME([this] (offs_t offset) {
			//LOG("SR11: DDC and Power Control read (%02x)\n", m_ext_ddc);
			return m_ext_ddc;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR11: DDC and Power Control %02x\n", data);
			if ((m_ext_ddc & 0xfc) != (data & 0xfc))
			{
				LOGSEQ("\tForce suspend mode %d Force standby mode %d memory activation source %d keyboard/HW cursor activation source %d\n"
					, BIT(data, 7)
					, BIT(data, 6)
					, BIT(data, 5)
					, BIT(data, 4)
				);
			}
			m_ext_ddc = data;
		})
	);

	// Ext. Horizontal Overflow
	map(0x12, 0x12).lrw8(
		NAME([this] (offs_t offset) { return m_ext_sr12; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR12: Horizontal Overflow %02x\n", data);
			m_ext_sr12 = data;
			// TODO: bits 7-5 for horizontal retrace skew (overrides base VGA?)
			// unused by base core anyway
			vga.crtc.horz_blank_end =     (vga.crtc.horz_blank_end & ~0x40)      | (BIT(data, 4) << 6);
			vga.crtc.horz_retrace_start = (vga.crtc.horz_retrace_start & ~0x100) | (BIT(data, 3) << 8);
			vga.crtc.horz_blank_start =   (vga.crtc.horz_blank_start & ~0x100)   | (BIT(data, 2) << 8);
			vga.crtc.horz_disp_end =      (vga.crtc.horz_disp_end & ~0x100)      | (BIT(data, 1) << 8);
			vga.crtc.horz_total =         (vga.crtc.horz_total & ~0x100)         | (BIT(data, 0) << 8);
			recompute_params();
		})
	);

	// Ext. Clock Generator / 25MHz/28MHz Video Clock
	map(0x13, 0x13).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_sr13;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR13: Clock Generator %02x\n", data);
			m_ext_sr13 = data;
			recompute_params();
		})
	);

	// HW Cursor Color 0/1
	map(0x14, 0x19).lrw8(
		NAME([this] (offs_t offset) {
			return m_cursor.color_cache[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_cursor.color_cache[offset] = data;
			const u8 pen_color = offset / 3;
			const u8 pal_offset = pen_color * 3;
			// RGB555 format
			m_cursor.color[pen_color] = (
				  (pal5bit(m_cursor.color_cache[0 + pal_offset]) << 16)
				| (pal5bit(m_cursor.color_cache[1 + pal_offset]) << 8)
				| (pal5bit(m_cursor.color_cache[2 + pal_offset]) << 0)
			);
		})
	);
	// HW Cursor Horizontal Start 0/1
	map(0x1a, 0x1b).lrw8(
		NAME([this] (offs_t offset) { return (offset) ? m_cursor.x >> 8 : m_cursor.x & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			if (offset)
			{
				m_cursor.x &= 0x00ff;
				m_cursor.x |= (data & 0x07) << 8;
			}
			else
			{
				m_cursor.x &= 0xff00;
				m_cursor.x |= data & 0xff;
			}
		})
	);
	// HW Cursor Horizontal Preset
	map(0x1c, 0x1c).lrw8(
		NAME([this] (offs_t offset) { return m_cursor.x_preset; }),
		NAME([this] (offs_t offset, u8 data) {
			m_cursor.x_preset = data & 0x3f;
		})
	);
	// HW Cursor Vertical Start 0/1
	map(0x1d, 0x1e).lrw8(
		NAME([this] (offs_t offset) {
			if (offset)
				return (m_cursor.pattern_select << 4) | (m_cursor.side_pattern_enable << 3) | ((m_cursor.y >> 8) & 7);
			return m_cursor.y & 0xff;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (offset)
			{
				m_cursor.y &= 0x00ff;
				m_cursor.y |= (data & 0x07) << 8;
				m_cursor.side_pattern_enable = !!BIT(data, 3);
				m_cursor.pattern_select = (data >> 4) & 0x0f;
			}
			else
			{
				m_cursor.y &= 0xff00;
				m_cursor.y |= data & 0xff;
			}
		})
	);
	// HW Cursor Vertical Preset
	map(0x1f, 0x1f).lrw8(
		NAME([this] (offs_t offset) { return m_cursor.y_preset; }),
		NAME([this] (offs_t offset, u8 data) {
			m_cursor.y_preset = data & 0x3f;
		})
	);

	// Linear Addressing Base Address 0/1
	map(0x20, 0x21).lrw8(
		NAME([this] (offs_t offset) {
			return m_linear_address[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_linear_address[offset] = data;
			LOG("SR%02X: Linear Addressing Base %02x\n", offset + 0x20, data);
			LOG("\tBase %08x Size %d\n"
				, (m_linear_address[0] << 19) | ((m_linear_address[1] & 0xf) << 27)
				// 00 512 KiB
				// 01 1 MiB
				// 10 2 MiB
				// 11 4 MiB
				, (m_linear_address[1] >> 5) & 3
			);
		})
	);

	// Standby/Suspend Timer
	map(0x22, 0x22).lrw8(
		NAME([this] (offs_t offset) { return (m_suspend_time << 4) | (m_standby_time); }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR22: Standby/Suspend Timer %02x\n", data);
			m_suspend_time = (data >> 4) & 0xf;
			m_standby_time = data & 0xf;
			// TODO: doc doesn't mention it, but is it actually supposed to be +1 for both?
			LOGSEQ("\tSuspend time %d minutes Standby Timer %d minutes\n"
				, m_suspend_time * 2
				, m_standby_time * 2
			);
		})
	);
	map(0x23, 0x23).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_sr23;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR23: Misc. Control 3 %02x\n", data);
			LOGSEQ("\tCRC Generator %d ED DRAM %d Bypass SRAM %d\n"
				, BIT(data, 6)
				, BIT(data, 5)
				, BIT(data, 4)
			);
			LOGSEQ("\tCompatible HW cursor visibility %d DRAM Control Delay Compensation %d nsec\n"
				, BIT(data, 3)
				, (data & 7) + 4
			);
			m_ext_sr23 = data;
		})
	);
	//map(0x24, 0x24) <reserved>
	map(0x25, 0x25).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_scratch[2];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR25: Scratch 2 %02x\n", data);
			m_ext_scratch[2] = data;
		})
	);

	// -x-- ---- Power Down Internal RAMDAC
	// --x- ---- PCI Burst Write Mode Enable
	// ---x ---- Continous Memory Data Access Enable
	// ---- -x-- Slow DRAM RAS pre-charge time
	// ---- --x- Slow FP/EDO DRAM RAS to CAS Timing Enable
	map(0x26, 0x26).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_ge26;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR26: Graphics Engine Register 0 %02x\n", data);
			LOGSEQ("\tPower-down Internal RAMDAC %d PCI Burst-Write Mode %d Continous Memory Data Access %d\n"
				, BIT(data, 6)
				, BIT(data, 5)
				, BIT(data, 4)
			);
			LOGSEQ("\tSlow DRAM RAS pre-charge %d MCLK/DRAM Slow FP/ED DRAM RAS-CAS %d MCLK/DRAM\n"
				, BIT(data, 2) + 3
				, BIT(data, 1) + 7
			);
			m_ext_ge26 = data;
		})
	);
	// x--- ---- Turbo Queue Engine enable
	// -x-- ---- Graphics Engine Programming enable
	// --xx ---- Logical Screen Width and BPP Select (TODO: verify, doc written like garbage)
	// --00 ---- 1024 on 8bpp or 512 on 15bpp/16bpp
	// --01 ---- 2048 on 8bpp or 1024 on 15bpp/16bpp
	// --10 ---- 4096 on 8bpp or 2048 on 15bpp/16bpp
	// ---- xxxx Extended Screen Start Address
	map(0x27, 0x27).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_ge27;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR27: Graphics Engine Register 1 %02x\n", data);
			if ((m_ext_ge27 & 0xf0) != (data & 0xf0))
			{
				LOGSEQ("\tTurbo Queue %d Graphic Engine Prog %d Logical Screen Width %d\n"
					, BIT(data, 7)
					, BIT(data, 6)
					, ((data >> 4) & 3) * 1024
				);
			}
			m_ext_ge27 = data;
			vga.crtc.start_addr_latch &= ~0x0f0000;
			vga.crtc.start_addr_latch |= ((data & 0x0f) << 16);
		})
	);

	// Internal Memory Clock
	map(0x28, 0x29).lrw8(
		NAME([this] (offs_t offset) {
			return m_mclk_int[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR%02x: Internal Memory Clock %d %02x\n", offset + 0x28, offset, data);
			m_mclk_int[offset] = data;
		})
	);
	// Internal Video Clock / 25MHz/28MHz Video Clock 0/1
	map(0x2a, 0x2b).lrw8(
		NAME([this] (offs_t offset) {
			return m_vclk_int[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR%02x: Internal Video Clock %d %02x\n", offset + 0x2a, offset, data);
			m_vclk_int[offset] = data;
			recompute_params();
		})
	);

	// Turbo Queue Base Address
	map(0x2c, 0x2c).lrw8(
		NAME([this] (offs_t offset) {
			return m_turbo_queue_address;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR2C: Turbo Queue Base Address %02x\n", data);
			// TODO: "in the last 32K segment"
			m_turbo_queue_address = data & 0x7f;
		})
	);
	// Memory Start Controller
	map(0x2d, 0x2d).lrw8(
		NAME([this] (offs_t offset) { return m_page_size_select; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR2D: Memory Start Control %02x\n", data);
			m_page_size_select = data & 0xf;
		})
	);

	//map(0x2e, 0x2e) <reserved>

	// DRAM Frame Buffer Size
	map(0x2f, 0x2f).lrw8(
		NAME([this] (offs_t offset) {
			return m_dram_fb_size;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR2F: DRAM Frame Buffer Size %02x\n", data);
			LOGSEQ("\tFast Change Mode Timing %d Fast Page Flip %d\n"
				, BIT(data, 5)
				, BIT(data, 4)
			);
			m_dram_fb_size = data & 0x30;
		})
	);

	// Fast Page Flip Starting Address
	map(0x30, 0x32).lrw8(
		NAME([this] (offs_t offset) { return m_fast_page_address_latch[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			const bool latch_address = offset == 2;
			const u8 mask = latch_address ? 0x0f : 0xff;
			m_fast_page_address_latch[offset] = data & mask;
			LOG("SR30: Fast Page Flip Starting Address [%d] %02x\n", offset, data);
			// TODO: Direct Draw obviously uses this
			//if (BIT(m_dram_fb_size, 4) && latch_address)
			//{
			//}
		})
	);

	// -x-- ---- Select external TVCLK as MCLK
	// --x- ---- Relocated VGA I/O port
	// ---x ---- Standard VGA I/O port address enable
	// ---- x--- Enable one cycle EDO DRAM timing
	// ---- -x-- Select SGRAM Latency
	// ---- --x- Enable SGRAM Mode Write timing
	// ---- ---x Enable SGRAM timing
	map(0x33, 0x33).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_sr33;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR33: Misc. Control 4 %02x\n", data);
			LOGSEQ("\tExternal TVCLK as MCLK %d\n"
				, BIT(data, 6)
			);
			LOGSEQ("\tOne cycle EDO DRAM %d SGRAM latency %d SGRAM Mode Write %d SGRAM timing %d\n"
				, BIT(data, 3)
				, 3 - BIT(data, 2)
				, BIT(data, 1)
				, BIT(data, 0)
			);
			m_ext_sr33 = data;
			// TODO: needs exposing for PCI card(s)
			// bit 5 relocates $3b0-$3df thru PCI bar
			// bit 4 disables VGA I/O on standard location
			if (data & 0x30)
				popmessage("pc_vga_sis.cpp: Relocated VGA PCI %d Standard VGA I/O disable %d", BIT(data, 5), BIT(data, 4));
		})
	);

	// x--- ---- DRAM controller one cycle write enable
	// -x-- ---- DRAM controller one cycle read enable
	// ---- -x-- Enable DRAM output PAD low power
	// ---- ---x Enable HW Command Queue threshold low
	map(0x34, 0x34).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_sr34;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR34: Misc. Control 5 %02x\n", data);
			LOGSEQ("\tDRAM controller one cycle write %d read %d\n"
				, BIT(data, 7)
				, BIT(data, 6)
			);
			LOGSEQ("\tDRAM output PAD low power %d HW Command Queue threshold low %d\n"
				, BIT(data, 2)
				, BIT(data, 0)
			);
			m_ext_sr34 = data;
		})
	);

	// x--- ---- Enable HW MPEG
	// -x-- ---- MA delay compensation (0) 0 nsec (1) 2 nsec
	// --x- ---- SGRAM burst timing enable (0) disable
	// ---x ---- Enable PCI burst write zero wait
	// ---- xx-- DRAM CAS LOW period width compensation
	// ---- --x- Enable PCI bus Write Cycle Retry
	// ---- ---x Enable PCI bus Read Cycle Retry
	map(0x35, 0x35).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_sr35;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR35: Misc. Control 6 %02x\n", data);
			LOGSEQ("\tHW MPEG %d MA delay compensation %d nsec SGRAM burst timing %d PCI burst write zero-wait %d\n"
				, BIT(data, 7)
				, BIT(data, 6) * 2
				, !BIT(data, 5)
				, BIT(data, 4)
			);
			LOGSEQ("\tDRAM CAS LOW period width compensation %d nsec PCI bus write cycle retry %d read cycle %d\n"
				, ((data >> 2) & 3) * 2
				, BIT(data, 1)
				, BIT(data, 0)
			);
			m_ext_sr35 = data;
		})
	);
	map(0x36, 0x37).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_scratch[offset + 3];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR%02X: Scratch %d %02x\n", offset + 0x36, offset + 3, data);
			m_ext_scratch[offset + 3] = data;
		})
	);

	// xxxx ---- HW Cursor Starting Address bits 21-18
	// ---- -x-- Line Compare (0) disable
	// ---- --xx Video Clock Select
	// ---- --00 Internal
	// ---- --01 25 MHz
	// ---- --10 28 MHz
	// ---- --11 <reserved>
	map(0x38, 0x38).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_sr38;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR38: Misc. Control 7 %02x\n", data);
			//if ((m_ext_sr38 & 3) != (data & 3))
			//{
			//	recompute_params();
			//}
			m_ext_sr38 = data;
			m_cursor.address_base &= ~0x3c'0000;
			m_cursor.address_base |= (data >> 4) << 18;
			// TODO: doc claims to be line compare disable, may just be bit 10 really?
			// testable at 1600x1200, needs HW test
			vga.crtc.line_compare = (vga.crtc.line_compare & 0x3ff) | (BIT(data, 2) * 0xfc00);
			//vga.crtc.line_compare = (vga.crtc.line_compare & 0x3ff) | (BIT(data, 2) << 10);
		})
	);

	// ---x ---- Select external TVCLK as internal TVCLK enable
	// ---- x--- Select external REFCLK as internal TVCLK enable
	// ---- -x-- Enable 3D accelerator
	// ---- --x- MPEG IDCT command software compression mode
	// ---- ---x Enable MPEG2 video decoding mode
	map(0x39, 0x39).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_sr39;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR39: Misc. Control 8 %02x\n", data);
			LOGSEQ("\tExternal TVCLK as internal TVCLK %d REFCLK as internal TVCLK %d\n"
				, BIT(data, 4)
				, BIT(data, 3)
			);
			LOGSEQ("\t3D Accelerator %d MPEG IDCT %d MPEG2 decode %d\n"
				, BIT(data, 2)
				, BIT(data, 1)
				, BIT(data, 0)
			);
			m_ext_sr39 = data;
		})
	);

	// MPEG Turbo Queue Base Address
	map(0x3a, 0x3a).lrw8(
		NAME([this] (offs_t offset) {
			return m_mpeg_turbo_queue_address;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR3A: MPEG Turbo Queue Base Address %02x\n", data);
			// TODO: "in the last 32K segment"
			m_mpeg_turbo_queue_address = data & 0x7f;
		})
	);

	// Clock Generator Control
	// TODO: undocumented
	map(0x3b, 0x3b).lrw8(
		NAME([this] (offs_t offset) {
			return (m_vclk_gen << 4) | (m_mclk_gen);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_vclk_gen = (data >> 4) & 0xf;
			m_mclk_gen = data & 0xf;
			LOG("SR3B: Clock Generator Control VCLK %01x MCLK %01x\n", m_vclk_gen, m_mclk_gen);
		})
	);

	// -x-- ---- SCLK output enable
	// --x- ---- AGP request high priority
	// ---x ---- Enable Oscillator I/O PAD power down
	// ---- x--- Enable AGP Dynamic Power Saving
	// ---- -x-- PCI-66 MHz timing enable
	// ---- --xx Turbo Queue length 2D/3D configuration bits
	// ---- --00 2D 32KB | 3D 0KB
	// ---- --01 2D 16KB | 3D 16KB
	// ---- --10 2D 8KB  | 3D 24KB
	// ---- --11 2D 4KB  | 3D 28KB
	map(0x3c, 0x3c).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_sr3c;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR3C: Misc. Control 9 %02x\n", data);
			LOGSEQ("\tSCLK output %d AGP request high priority %d Oscillator I/O Power Down %d\n"
				, BIT(data, 6)
				, BIT(data, 5)
				, BIT(data, 4)
			);
			LOGSEQ("\tAGP Dynamic Power Saving %d PCI-66 MHz %d Turbo Queue 2D/3D length config %d\n"
				, BIT(data, 3)
				, BIT(data, 2)
				, data & 3
			);
			m_ext_sr3c = data;
		})
	);
}

void sis6326_vga_device::tvout_map(address_map &map)
{
	map(0x00, 0x00).lrw8(
		NAME([this] (offs_t offset) {
			// win98se reads this while moving mouse
			//LOG("VR00: Basic TV Function Control read (%02x)\n", m_tv.control);
			return m_tv.control;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_tv.control = data;
			LOG("VR00: Basic TV Function Control %02x\n", data);
			LOG("\tFSEL %d COMPN %d SVIDEON %d ENTV %d SHRINK %d REGODD %d\n"
				, (data >> 5) & 7
				, !BIT(data, 4)
				, !BIT(data, 3)
				, BIT(data, 2)
				, BIT(data, 1)
				, BIT(data, 0)
			);
		})
	);
	// ...
	map(0x42, 0x42).lrw8(
		NAME([this] (offs_t offset) { return m_tv.pycin & 0xff; }),
		NAME([this] (offs_t offset, u8 data) {
			m_tv.pycin &= 0x0300;
			m_tv.pycin |= (data & 0xff);
			LOG("VR42: TV DAC Sense Input Register 1 %02x\n", data);
		})
	);
	map(0x43, 0x43).lrw8(
		NAME([this] (offs_t offset) {
			return (m_tv.enyf << 4) | (m_tv.encf << 3) | (m_tv.tvsense << 2) | ((m_tv.pycin >> 8) & 0x3);
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("VR43: TV DAC Sense Input Register 2 %02x\n", data);
			m_tv.enyf = !!BIT(data, 4);
			m_tv.encf = !!BIT(data, 3);
			m_tv.tvsense = !!BIT(data, 2);
			m_tv.pycin &= 0x00ff;
			m_tv.pycin |= (data & 3) << 8;
		})
	);
	// ---- -x-- RSENY Y signal readback
	// ---- --x- RSENC Cb & Cr signal readback
	// ---- ---x RSENCO Composite signal readback
	map(0x44, 0x44).lr8(
		NAME([this] (offs_t offset) {
			LOG("VR44: TV DAC Sense Read-back\n");
			// Pull high to enable TV mode
			return 0;
		})
	);
}

// original SiS6326 seems unable to do 32-bit mode
std::tuple<u8, u8> sis6326_vga_device::flush_true_color_mode()
{
	// punt if extended or true color is off
	if ((m_ramdac_mode & 0x12) != 0x12)
		return std::make_tuple(0, 0);

	const u8 res = !BIT(m_ext_sr07, 2);

	return std::make_tuple(res, 0);
}

void sis6326_vga_device::recompute_params()
{
	u8 xtal_select = (vga.miscellaneous_output & 0x0c) >> 2;
	int xtal;

	switch(xtal_select & 3)
	{
		case 0: xtal = XTAL(25'174'800).value(); break;
		case 1: xtal = XTAL(28'636'363).value(); break;
		// TODO: stub, barely enough to make BeOS 5 to set ~60 Hz for 640x480x16
		case 2:
		default:
			xtal = XTAL(25'174'800).value();
			break;
	}

	recompute_params_clock(1, xtal);
}

uint16_t sis6326_vga_device::offset()
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
		return vga.crtc.offset << 3;
	return svga_device::offset();
}

u16 sis6326_vga_device::line_compare_mask()
{
	// trick to make line compare to never occur
	// (assuming it's true, cfr. above)
	return 0x3ff | (vga.crtc.line_compare & 0xfc00);
}

uint8_t sis6326_vga_device::mem_r(offs_t offset)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
		return svga_device::mem_linear_r(offset + svga.bank_r * 0x10000);
	return svga_device::mem_r(offset);
}

void sis6326_vga_device::mem_w(offs_t offset, uint8_t data)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
	{
		svga_device::mem_linear_w(offset + svga.bank_w * 0x10000, data);
		return;
	}
	svga_device::mem_w(offset, data);
}

// TODO: similar to S3 variant, is there an enable bit?
uint32_t sis6326_vga_device::latch_start_addr()
{
	return vga.crtc.start_addr_latch << (svga.rgb8_en ? 2 : 0);
}

// undocumented, win98se access this for X/Y positions to actually work
// [1]/[3] are probably X/Y preset registers (byte accesses)
void sis6326_vga_device::cursor_mmio_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch(offset)
	{
		case 0:
			COMBINE_DATA(&m_cursor.x);
			break;
		case 2:
			COMBINE_DATA(&m_cursor.y);
			break;
	}
}

uint32_t sis6326_vga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	svga_device::screen_update(screen, bitmap, cliprect);

	// HW cursor
	if (BIT(m_ramdac_mode, 6))
	{
		// TODO: preliminary, likely using pattern_select for switching modes
		// Drawing specifics aren't really documented beyond what the register does.

		const u32 base_offs = (m_cursor.address_base);
		const u8 transparent_pen = 2;

		for (int y = 0; y < 64; y ++)
		{
			int res_y = y + m_cursor.y;
			for (int x = 0; x < 64; x++)
			{
				int res_x = x + m_cursor.x;
				if (!cliprect.contains(res_x, res_y))
					continue;
				const u32 cursor_address = ((x >> 2) + y * 16) + base_offs;
				const int xi = (3 - (x & 3)) * 2;
				u8 cursor_gfx =  (vga.memory[(cursor_address) % vga.svga_intf.vram_size] >> (xi) & 3);

				if (cursor_gfx == transparent_pen)
					continue;

				// RMW invert (win98se NotePad "I" caret)
				if (cursor_gfx == 3)
				{
					u32 const *dot = &bitmap.pix(res_y, 0);
					bitmap.pix(res_y, res_x) = dot[res_x] ^ 0xffffff;
				}
				else
					bitmap.pix(res_y, res_x) = m_cursor.color[cursor_gfx & 1];
			}
		}
	}

#if DEBUG_VRAM_VIEWER
	static int m_test_x = 1024, m_start_offs;
	static int m_test_trigger = 1;
	const int m_test_y = cliprect.max_y;

	if(machine().input().code_pressed(JOYCODE_HAT1RIGHT))
		m_test_x += 1 << (machine().input().code_pressed(JOYCODE_BUTTON2) ? 4 : 0);

	if(machine().input().code_pressed(JOYCODE_HAT1LEFT))
		m_test_x -= 1 << (machine().input().code_pressed(JOYCODE_BUTTON2) ? 4 : 0);

	//if(machine().input().code_pressed(JOYCODE_HAT1DOWN))
	//  m_test_y++;

	//if(machine().input().code_pressed(JOYCODE_HAT1UP))
	//  m_test_y--;

	if(machine().input().code_pressed(JOYCODE_HAT1DOWN))
		m_start_offs+= 0x100 << (machine().input().code_pressed(JOYCODE_BUTTON2) ? 8 : 0);

	if(machine().input().code_pressed(JOYCODE_HAT1UP))
		m_start_offs-= 0x100 << (machine().input().code_pressed(JOYCODE_BUTTON2) ? 8 : 0);

	m_start_offs %= vga.svga_intf.vram_size;

	if(machine().input().code_pressed_once(JOYCODE_BUTTON1))
		m_test_trigger ^= 1;

	if (!m_test_trigger)
		return 0;

	popmessage("%d %d %04x", m_test_x, m_test_y, m_start_offs);

	bitmap.fill(0, cliprect);

	int count = m_start_offs;

	for(int y = 0; y < m_test_y; y++)
	{
		for(int x = 0; x < m_test_x; x ++)
		{
			u8 color = vga.memory[count % vga.svga_intf.vram_size];

			if(cliprect.contains(x, y))
			{
				//bitmap.pix(y, x) = pal565(color, 11, 5, 0);
				bitmap.pix(y, x) = pen(color);
			}

			count ++;
			// count += 2;
		}
	}
#endif


	return 0;
}


/*
 * SiS630 overrides
 */

 sis630_vga_device::sis630_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sis6326_vga_device(mconfig, SIS630_VGA, tag, owner, clock)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(sis630_vga_device::crtc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(sis630_vga_device::sequencer_map), this));
	m_tvout_space_config = address_space_config("tvout_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(sis630_vga_device::tvout_map), this));
}

// Page 144
void sis630_vga_device::crtc_map(address_map &map)
{
	sis6326_vga_device::crtc_map(map);

	// TODO: very preliminary, this section is undocumented in '630 doc
	map(0x30, 0xff).lrw8(
		NAME([this] (offs_t offset) {
			return vga.crtc.data[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: if one of these is 0xff then it enables a single port transfer to $b8000
			// Older style MMIO?
			vga.crtc.data[offset] = data;
		})
	);
	// make sure '301 CRT2 is not enabled for now
	// TODO: BeMAME (0.36b5) under BeOS 5.0 detects a secondary monitor by default anyway
	map(0x30, 0x30).lr8(
		NAME([] (offs_t offset) { return 0; })
	);
	map(0x31, 0x31).lr8(
		NAME([] (offs_t offset) { return 0x60; })
	);
	map(0x32, 0x32).lr8(
		NAME([] (offs_t offset) { return 0x20; })
	);
}

void sis630_vga_device::sequencer_map(address_map &map)
{
	sis6326_vga_device::sequencer_map(map);
	map(0x0a, 0x0a).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_vert_overflow;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR0A: Vertical Overflow %02x\n", data);
			m_ext_vert_overflow = data;
			vga.crtc.vert_retrace_end  =  (vga.crtc.vert_retrace_end & 0xf)      | ((data & 0x20) >> 1);
			vga.crtc.vert_blank_end  =    (vga.crtc.vert_blank_end & 0x00ff)     | ((data & 0x10) << 4);
			vga.crtc.vert_retrace_start = (vga.crtc.vert_retrace_start & 0x03ff) | ((data & 0x08) << 7);
			vga.crtc.vert_blank_start =   (vga.crtc.vert_blank_start & 0x03ff)   | ((data & 0x04) << 8);
			vga.crtc.vert_disp_end =      (vga.crtc.vert_disp_end & 0x03ff)      | ((data & 0x02) << 9);
			vga.crtc.vert_total =         (vga.crtc.vert_total & 0x03ff)         | ((data & 0x01) << 10);
			recompute_params();
		})
	);
	map(0x0b, 0x0c).lr8(
		NAME([this] (offs_t offset) {
			return m_ext_horz_overflow[offset];
		})
	);
	map(0x0b, 0x0b).lw8(
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR0B: Horizontal Overflow 1 %02x\n", data);
			m_ext_horz_overflow[0] = data;

			vga.crtc.horz_retrace_start = (vga.crtc.horz_retrace_start & 0x00ff) | ((data & 0xc0) << 2);
			vga.crtc.horz_blank_start =   (vga.crtc.horz_blank_start & 0x00ff)   | ((data & 0x30) << 4);
			vga.crtc.horz_disp_end =      (vga.crtc.horz_disp_end & 0x00ff)      | ((data & 0x0c) << 6);
			vga.crtc.horz_total =         (vga.crtc.horz_total & 0x00ff)         | ((data & 0x03) << 8);

			recompute_params();
		})
	);
	map(0x0c, 0x0c).lw8(
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR0C: Horizontal Overflow 2 %02x\n", data);
			m_ext_horz_overflow[1] = data;

			vga.crtc.horz_retrace_end =   (vga.crtc.horz_retrace_end & 0x001f) | ((data & 0x04) << 3);
			vga.crtc.horz_blank_end =     (vga.crtc.horz_blank_end & 0x003f)   | ((data & 0x03) << 6);
			recompute_params();
		})
	);
	map(0x0d, 0x0d).lrw8(
		NAME([this] (offs_t offset) {
			return vga.crtc.start_addr_latch >> 16;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR0D: Starting Address %02x\n", data);
			vga.crtc.start_addr_latch &= ~0xff0000;
			vga.crtc.start_addr_latch |= data << 16;
		})
	);
	map(0x0e, 0x0e).unmapr();
	map(0x0e, 0x0e).lw8(
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR0E: pitch register %02x\n", data);
			// sis_main.c implicitly sets this with bits 0-3 granularity, assume being right
			vga.crtc.offset = (vga.crtc.offset & 0x00ff) | ((data & 0x0f) << 8);
		})
	);
	//map(0x0f, 0x0f) CRT misc. control
	//map(0x10, 0x10) Display line width register
	//map(0x11, 0x11) DDC register
	map(0x14, 0x14).lrw8(
		NAME([this] (offs_t offset) {
			// sis_main.c calculates VRAM size in two ways:
			// 1. the legacy way ('300), by probing this register
			// 2. by reading '630 PCI host register $63 (as shared DRAM?)
			// Method 1 seems enough to enforce "64MB" message at POST,
			// 2 is probably more correct but unsure about how to change the shared area in BIOS
			// (shutms11 will always write a "0x41" on fresh CMOS then a "0x47"
			//  on successive boots no matter what)
			return (m_bus_width) | ((vga.svga_intf.vram_size / (1024 * 1024) - 1) & 0x3f);
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR14: <unknown> %02x\n", data);
			m_bus_width = data & 0xc0;
		})
	);
	//map(0x1d, 0x1d) Segment Selection Overflow
	map(0x15, 0x1d).unmaprw();
	map(0x1e, 0x1e).lw8(
		NAME([this] (offs_t offset, u8 data) {
			if (BIT(data, 6))
				popmessage("pc_vga_sis: enable 2d engine");
		})
	);
	//map(0x1f, 0x1f) Power management
	map(0x20, 0x20).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// GUI address decoder setting
			if (data & 0x81)
				popmessage("pc_vga_sis: SR20 %s %s", BIT(data, 7) ? "PCI address enabled" : "", BIT(data, 0) ? "memory map I/O enable" : "");
		})
	);
	map(0x21, 0x21).unmaprw();
	//map(0x21, 0x21) GUI HostBus state machine setting
	//map(0x22, 0x22) GUI HostBus controller timing
	//map(0x23, 0x23) GUI HostBus timer

	//map(0x26, 0x26) Turbo Queue base address
	//map(0x27, 0x27) Turbo Queue control

	map(0x2b, 0x2d).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_dclk[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR%02X: DCLK %02x\n", offset + 0x2b, data);
			m_ext_dclk[offset] = data;
			recompute_params();
		})
	);
	map(0x2e, 0x30).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_eclk[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR%02X: ECLK %02x\n", offset + 0x2e, data);
			m_ext_eclk[offset] = data;
			recompute_params();
		})
	);
	map(0x31, 0x31).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_clock_gen;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR31: clock generator misc. %02x\n", data);
			m_ext_clock_gen = data;
			recompute_params();
		})
	);
	map(0x32, 0x32).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_clock_source_select;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR32: clock source selection %02x\n", data);
			m_ext_clock_source_select = data;
			recompute_params();
		})
	);

	//map(0x34, 0x34) Interrupt status
	//map(0x35, 0x35) Interrupt enable
	//map(0x36, 0x36) Interrupt reset

	//map(0x38, 0x3a) Power on trapping
	//map(0x3c, 0x3c) Synchronous reset
	//map(0x3d, 0x3d) Test enable
}

std::tuple<u8, u8> sis630_vga_device::flush_true_color_mode()
{
	// punt if extended or true color is off
	if ((m_ramdac_mode & 0x12) != 0x12)
		return std::make_tuple(0, 0);

	const u8 res = BIT(m_ext_sr07, 2);

	return std::make_tuple(res, res ^ 1);
}
