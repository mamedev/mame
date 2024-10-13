// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Paradise / Western Digital (S)VGA chipsets

- PVGA1A
- PVGA1A-JK / WD90C90-JK (same as PVGA1A with extra connectors?)
- WD90C00-JK (extended CRTC regs)
- WD90C11-LR / WD90C11A-LR (extended sequencer regs)
- WD90C30-LR
- WD90C31-LR / WD90C31-ZS / WD90C31A-LR / WD90C31A-ZS
- WD90C33-ZZ
- WD90C24A-ZZ / WD90C24A2-ZZ / WD90C26A (cfr. video/wd90c26.cpp)

TODO:
- WD9710-MZ (PCI + MPEG-1, a.k.a. Pipeline 9710 / 9712, to be added in specific sub-file as well)

- 'C31A difference compared to 'C31 (just "reserved" PR35?);
- Emulate new features of 'C31 & 'C33;
- win95 can't draw with 'C33 properly when in VESA modes;
- Memory Data pins (MD) a.k.a. CNF (64 of them across the device tree)
- /EBROM signal (for enabling ROM readback)
- AIDA16 & UniVBE VESA suite detects 'C11 as 'C30, is the ROM mislabeled?
- CRTC group locks;

**************************************************************************************************/

#include "emu.h"
#include "pc_vga_paradise.h"

#include "screen.h"

#define LOG_WARN    (1U << 1)
#define LOG_BANK    (1U << 2) // log banking r/ws
#define LOG_LOCKED  (1U << 8) // log locking mechanism

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_LOCKED)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)       LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGBANK(...)       LOGMASKED(LOG_BANK,  __VA_ARGS__)
#define LOGLOCKED(...)     LOGMASKED(LOG_LOCKED,  __VA_ARGS__)


DEFINE_DEVICE_TYPE(PVGA1A,   pvga1a_vga_device,    "pvga1a_vga",   "Paradise Systems PVGA1A i/f")
DEFINE_DEVICE_TYPE(WD90C00,  wd90c00_vga_device,   "wd90c00_vga",  "Western Digital WD90C00 \"PVGA1B\" VGA i/f")
DEFINE_DEVICE_TYPE(WD90C11A, wd90c11a_vga_device,  "wd90c11a_vga", "Western Digital WD90C11A \"PVGA1C\" VGA i/f")
DEFINE_DEVICE_TYPE(WD90C30,  wd90c30_vga_device,   "wd90c30_vga",  "Western Digital WD90C30 \"PVGA1D\" VGA i/f")
DEFINE_DEVICE_TYPE(WD90C31,  wd90c31_vga_device,   "wd90c31_vga",  "Western Digital WD90C31 VGA i/f")
DEFINE_DEVICE_TYPE(WD90C33,  wd90c33_vga_device,   "wd90c33_vga",  "Western Digital WD90C33 VGA i/f")


pvga1a_vga_device::pvga1a_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, type, tag, owner, clock)
{
	m_gc_space_config = address_space_config("gc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(pvga1a_vga_device::gc_map), this));
}

pvga1a_vga_device::pvga1a_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pvga1a_vga_device(mconfig, PVGA1A, tag, owner, clock)
{
}

void pvga1a_vga_device::device_start()
{
	svga_device::device_start();
	zero();

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;

	// copy over interfaces
	vga.svga_intf.vram_size = 1*1024*1024;
	//vga.memory = std::make_unique<uint8_t []>(vga.svga_intf.vram_size);
}

void pvga1a_vga_device::device_reset()
{
	svga_device::device_reset();

	m_memory_size = 0;
	m_video_control = 0; // Really &= 0x8; at POR according to docs
	m_video_select = 0;
	m_ega_compatible_mode = false;
	m_crtc_lock = 0;
	m_ext_gc_unlock = false;
}

uint8_t pvga1a_vga_device::mem_r(offs_t offset)
{
	if (svga.rgb8_en)
		return svga_device::mem_linear_r(offset + (svga.bank_w * 0x1000));
	return svga_device::mem_r(offset);
}

void pvga1a_vga_device::mem_w(offs_t offset, uint8_t data)
{
	// TODO: Address Offset B, not extensively tested
	// Should also enable thru bits 5-4 of PR1 but instead SW seems to use 7-6!?
	if (svga.rgb8_en)
	{
		svga_device::mem_linear_w(offset + (svga.bank_w * 0x1000), data);
		return;
	}
	svga_device::mem_w(offset, data);
}

u8 pvga1a_vga_device::gc_data_r(offs_t offset)
{
	if (m_ega_compatible_mode && vga.gc.index >= 9 && vga.gc.index <= 0xe && !machine().side_effects_disabled())
	{
		LOGLOCKED("Attempt to read ext. GC register offset [%02x] while locked\n", vga.gc.index);
		return 0xff;
	}
	return svga_device::gc_data_r(offset);
}

void pvga1a_vga_device::gc_data_w(offs_t offset, u8 data)
{
	if (!m_ext_gc_unlock && vga.gc.index >= 9 && vga.gc.index <= 0xe && !machine().side_effects_disabled())
	{
		LOGLOCKED("Attempt to write ext. GC register offset [%02x] <- %02x while locked\n", vga.gc.index, data);
		return;
	}
	svga_device::gc_data_w(offset, data);
}

void pvga1a_vga_device::gc_map(address_map &map)
{
	svga_device::gc_map(map);
	map(0x09, 0x0a).rw(FUNC(pvga1a_vga_device::address_offset_r), FUNC(pvga1a_vga_device::address_offset_w));
	map(0x0b, 0x0b).rw(FUNC(pvga1a_vga_device::memory_size_r), FUNC(pvga1a_vga_device::memory_size_w));
	map(0x0c, 0x0c).rw(FUNC(pvga1a_vga_device::video_select_r), FUNC(pvga1a_vga_device::video_select_w));
	map(0x0d, 0x0d).rw(FUNC(pvga1a_vga_device::crtc_lock_r), FUNC(pvga1a_vga_device::crtc_lock_w));
	map(0x0e, 0x0e).rw(FUNC(pvga1a_vga_device::video_control_r), FUNC(pvga1a_vga_device::video_control_w));
	map(0x0f, 0x0f).rw(FUNC(pvga1a_vga_device::ext_gc_status_r), FUNC(pvga1a_vga_device::ext_gc_unlock_w));
}

/*
 * [0x09] PR0A Address Offset A
 * [0x0a] PR0B Address Offset B
 *
 * x--- ---- <reserved> according to docs, Win 95 uses this in 800x600 modes for systray
 * -xxx xxxx bank selects, in 4KB units
 */
u8 pvga1a_vga_device::address_offset_r(offs_t offset)
{
	if (!offset)
	{
		LOGBANK("PR0A read Address Offset A\n");
		return svga.bank_w & 0xff;
	}
	// Address Offset B, TBD
	LOGBANK("PR0A read Address Offset B\n");
	return 0;
}

void pvga1a_vga_device::address_offset_w(offs_t offset, u8 data)
{
	if (!offset)
	{
		LOG("PR0A write Address Offset A %02x\n", data);
		svga.bank_w = data & 0xff;
	}
	else
	{
		LOG("PR0B write Address Offset B %02x\n", data);
		// ...
	}
}

/*
 * [0x0b] PR1 Memory Size
 *
 * xx-- ---- Memory Size
 * 11-- ---- 1MB
 * 10-- ---- 512KB
 * 0x-- ---- 256KB
 * --xx ---- Memory Map Select
 * ---- x--- Enable PR0B
 * ---- -x-- Enable 16-bit memory bus
 * ---- --x- Enable 16-bit BIOS ROM reads (MD1)
 * ---- ---x BIOS ROM mapped (MD0)
 */
u8 pvga1a_vga_device::memory_size_r(offs_t offset)
{
	LOG("PR1 Memory Size R\n");
	return 0xc0 | (m_memory_size & 0x3f);
}

void pvga1a_vga_device::memory_size_w(offs_t offset, u8 data)
{
	LOG("PR1 Memory Size W %02x\n", data);
	m_memory_size = data;
}

/*
 * [0x0c] PR2 Video Select
 *
 * x--- ---- M24 Mode Enable
 * -x-- ---- 6845 Compatiblity Mode
 * --x- -x-- Character Map Select
 * ---- -1-- \- also enables special underline effect (?)
 * ---x x--- Character Clock Period Control
 * ---0 0--- VGA 8/9 dots
 * ---0 1--- 7 dots
 * ---1 0--- 9 dots
 * ---1 1--- 10 dots
 * ---- --x- external clock select 3
 * ---- ---x Set horizontal sync timing (0) doubled?
 */
u8 pvga1a_vga_device::video_select_r(offs_t offset)
{
	LOG("PR2 Video Select R\n");
	return m_video_select;
}

void pvga1a_vga_device::video_select_w(offs_t offset, u8 data)
{
	LOG("PR2 Video Select W %02x\n", data);
	m_video_select = data;
	recompute_params();
}

/*
 * [0x0d] PR3 CRT Control [locks groups in CRTC]
 *
 * x--- ---- Lock VSYNC polarity
 * -x-- ---- Lock HSYNC polarity
 * --x- ---- Lock horizontal timing (group 0 & 4)
 * ---x ---- bit 9 of CRTC Start Memory Address
 * ---- x--- bit 8 of CRTC Start Memory Address
 * ---- -x-- CRT Control cursor start, cursor stop, preset row scan, maximum scan line x2 (??)
 * ---- --x- Lock vertical display enable end (group 1)
 * ---- ---x Lock vertical total/retrace (group 2 & 3)
 */
u8 pvga1a_vga_device::crtc_lock_r(offs_t offset)
{
	LOG("PR3 CRTC lock R\n");
	return m_crtc_lock;
}

void pvga1a_vga_device::crtc_lock_w(offs_t offset, u8 data)
{
	LOG("PR3 CRTC lock W %02x\n", data);
	m_crtc_lock = data;
}

/*
 * [0x0e] PR4 Video Control
 *
 * x--- ---- BLNKN (0) enables external Video DAC
 * -x-- ---- Tristate HSYNC, VSYNC, BLNKN
 * --x- ---- Tristate VID7-VID0
 * ---x ---- Tristate Memory Control outputs
 * ---- x--- Disable CGA (unaffected by POR)
 * ---- -x-- Lock palette and overscan regs
 * ---- --x- Enable EGA compatible mode
 * ---- ---x Enable 640x400x8bpp
 */
u8 pvga1a_vga_device::video_control_r(offs_t offset)
{
	LOG("PR4 Video Control R\n");
	return m_video_control;
}

void pvga1a_vga_device::video_control_w(offs_t offset, u8 data)
{
	LOG("PR4 Video Control W %02x\n", data);
	m_video_control = data;
	svga.rgb8_en = BIT(data, 0);
	m_ega_compatible_mode = bool(BIT(data, 1));
}

/*
 * [0x0f] PR5 Lock/Status
 *
 * xxxx ---- CNF(7)-CNF(4) / MD7/MD4 config reads
 * ---- x--- CNF(8) / MD8 config read (on later chipsets)
 * ---- -xxx lock register
 * ---- -101 unlock, any other value locks r/w to the extensions
 */
u8 pvga1a_vga_device::ext_gc_status_r(offs_t offset)
{
	return m_ext_gc_unlock ? 0x05 : 0x00;
}

void pvga1a_vga_device::ext_gc_unlock_w(offs_t offset, u8 data)
{
	m_ext_gc_unlock = (data & 0x7) == 5;
	LOGLOCKED("PR5 %s state (%02x)\n", m_ext_gc_unlock ? "unlock" : "lock", data);
}

/**************************************
 *
 * Western Digital WD90C00
 *
 *************************************/

wd90c00_vga_device::wd90c00_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pvga1a_vga_device(mconfig, type, tag, owner, clock)
	, m_cnf15_read_cb(*this, 1)
	, m_cnf14_read_cb(*this, 1)
	, m_cnf13_read_cb(*this, 1)
	, m_cnf12_read_cb(*this, 1)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(wd90c00_vga_device::crtc_map), this));
}

wd90c00_vga_device::wd90c00_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd90c00_vga_device(mconfig, WD90C00, tag, owner, clock)
{
}

void wd90c00_vga_device::device_reset()
{
	pvga1a_vga_device::device_reset();

	m_pr10_scratch = 0;
	m_ext_crtc_read_unlock = false;
	m_ext_crtc_write_unlock = false;
	// egasw
	m_pr11 = (m_cnf15_read_cb() << 7) | (m_cnf14_read_cb() << 6) | m_cnf13_read_cb() << 5 | m_cnf12_read_cb() << 4;
	m_interlace_start = 0;
	m_interlace_end = 0;
	m_interlace_mode = false;
	m_pr15 = 0;
}

ioport_value wd90c00_vga_device::egasw1_r() { return BIT(m_pr11, 4); }
ioport_value wd90c00_vga_device::egasw2_r() { return BIT(m_pr11, 5); }
ioport_value wd90c00_vga_device::egasw3_r() { return BIT(m_pr11, 6); }
ioport_value wd90c00_vga_device::egasw4_r() { return BIT(m_pr11, 7); }

static INPUT_PORTS_START(paradise_vga_sense)
	PORT_START("VGA_SENSE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(wd90c00_vga_device, egasw1_r)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(wd90c00_vga_device, egasw2_r)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(wd90c00_vga_device, egasw3_r)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(wd90c00_vga_device, egasw4_r)
INPUT_PORTS_END

ioport_constructor wd90c00_vga_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(paradise_vga_sense);
}

u8 wd90c00_vga_device::crtc_data_r(offs_t offset)
{
	if (!m_ext_crtc_read_unlock && vga.crtc.index >= 0x2a && !machine().side_effects_disabled())
	{
		LOGLOCKED("Attempt to read ext. CRTC register offset %02x while locked\n", vga.crtc.index);
		return 0xff;
	}
	return svga_device::crtc_data_r(offset);
}

void wd90c00_vga_device::crtc_data_w(offs_t offset, u8 data)
{
	if (!m_ext_crtc_write_unlock && vga.crtc.index >= 0x2a && !machine().side_effects_disabled())
	{
		LOGLOCKED("Attempt to write ext. CRTC register offset [%02x] <- %02x while locked\n", vga.crtc.index, data);
		return;
	}
	svga_device::crtc_data_w(offset, data);
}

void wd90c00_vga_device::crtc_map(address_map &map)
{
	pvga1a_vga_device::crtc_map(map);
	map(0x29, 0x29).rw(FUNC(wd90c00_vga_device::ext_crtc_status_r), FUNC(wd90c00_vga_device::ext_crtc_unlock_w));
	map(0x2a, 0x2a).rw(FUNC(wd90c00_vga_device::egasw_r), FUNC(wd90c00_vga_device::egasw_w));
	map(0x2b, 0x2b).ram(); // PR12 scratch pad
	map(0x2c, 0x2d).rw(FUNC(wd90c00_vga_device::interlace_r), FUNC(wd90c00_vga_device::interlace_w));
	map(0x2e, 0x2e).rw(FUNC(wd90c00_vga_device::misc_control_1_r), FUNC(wd90c00_vga_device::misc_control_1_w));
//  map(0x2f, 0x2f) PR16 Misc Control 2
//  map(0x30, 0x30) PR17 Misc Control 3
//  map(0x31, 0x3f) <reserved>, may still read device ASCII ID like later variants?
}

void wd90c00_vga_device::recompute_params()
{
	u8 xtal_select = (vga.miscellaneous_output & 0x0c) >> 2;
	int xtal;
	// TODO: wd90c11a VTB disables video select bit 1 for 1024x768 mode (with interlace mode = false)
	// Uses VCLK1, without the bump will select 26.43 Hz as refresh rate
	const u8 multiplier = BIT(m_video_select, 1) ? 1 : 2;

	//printf("%d %d %02x\n", m_interlace_mode, xtal_select, m_video_select);

	switch(xtal_select & 3)
	{
		// VCLK0
		case 0: xtal = XTAL(25'174'800).value() * multiplier; break;
		// VCLK1
		case 1: xtal = XTAL(28'636'363).value() * multiplier; break;
		// VCLK2, selected in 800x600 modes
		case 2:
		// TODO: wd90c30 selects this for 1024x768 interlace mode
		// (~40 Hz, should be 43 according to defined video clocks in WD9710 driver .inf)
		default:
			xtal = XTAL(42'000'000).value();
			break;
	}

	recompute_params_clock(1, xtal);
}

/*
 * [0x29] PR10 Unlock PR11/PR17
 *
 * x--- x--- Read lock
 * 1--- 0--- Unlocks, any other write locks reading
 * -xxx ---- Scratch Pad
 * ---- -xxx Write lock
 * ---- -101 Unlocks, any other write locks writing
 */
u8 wd90c00_vga_device::ext_crtc_status_r(offs_t offset)
{
	return (m_ext_crtc_read_unlock ? 0x80 : 0x00) | m_pr10_scratch | (m_ext_crtc_write_unlock ? 0x05 : 0x00);
}

void wd90c00_vga_device::ext_crtc_unlock_w(offs_t offset, u8 data)
{
	m_ext_crtc_read_unlock = (data & 0x88) == 0x80;
	m_ext_crtc_write_unlock = (data & 0x7) == 5;
	LOGLOCKED("PR10 read %s write %s state (%02x)\n"
		, m_ext_crtc_read_unlock ? "unlock" : "lock"
		, m_ext_crtc_write_unlock ? "unlock" : "lock"
		, data
	);
	m_pr10_scratch = data & 0x70;
}

/*
 * [0x2a] PR11 EGA Switches
 *
 * xxxx ---- EGA switches (MD15-MD12), latches high if written to.
 *           CONF15-12 on 'C26 for panel support. Pulling up will latch high these pins.
 * ---- x--- EGA emulation on Analog Display
 * ---- -x-- Lock Clock Select (disables external chip select for VCLK1)
 * ---- --x- Locks GC $5 bits 6:5, sequencer $1 bits 5:2, sequencer $3 bits 5:0
 * ---- ---x Locks sequencer $1 bit 0 (8/9 dot mode)
 */
u8 wd90c00_vga_device::egasw_r(offs_t offset)
{
	LOG("PR11 EGA Switch R (%02x)\n", m_pr11);
	return m_pr11;
}

void wd90c00_vga_device::egasw_w(offs_t offset, u8 data)
{
	LOG("PR11 EGA Switch W %02x\n", data);
	m_pr11 = data & 0xff;
}

/*
 * [0x2c] PR13 Interlace H/2 Start
 *
 * xxxx xxxx Horizontal interlace start
 *
 * [0x2d] PR14 Interlace H/2 End
 *
 * x--- ---- Enable EGA IRQ in AT bus mode (N/A for VGA operation and MCA bus)
 * -x-- ---- EGA vertical double scan
 * --x- ---- Enable interlace mode
 * ---x xxxx Interlace H/2 End
 *
 */
u8 wd90c00_vga_device::interlace_r(offs_t offset)
{
	if (!offset)
	{
		LOG("PR13 Interlace H/2 Start R\n");
		return m_interlace_start;
	}

	LOG("PR14 Interlace H/2 End R\n");
	return (m_interlace_mode << 5) | (m_interlace_end);
}

void wd90c00_vga_device::interlace_w(offs_t offset, u8 data)
{
	if (!offset)
	{
		LOG("PR13 Interlace H/2 Start W %02x\n", data);
		m_interlace_start = data;
		return;
	}

	LOG("PR14 Interlace H/2 End W %02x\n", data);
	m_interlace_mode = bool(BIT(data, 5));
	m_interlace_end = data & 0x1f;
}

/*
 * [0x2e] PR15 Misc Control 1
 *
 * x--- ---- Read $46e8 enable
 * -x-- ---- <reserved>
 * --x- ---- VCLK1, VCLK2 Latched Outputs
 * ---x ---- Select MCLK as video clock
 * ---- x--- 8514/A Interlace compatibility
 * ---- -x-- Enable Page Mode
 * ---- --x- Select Display Enable
 * ---- ---x Disable Border
 */
u8 wd90c00_vga_device::misc_control_1_r(offs_t offset)
{
	LOG("PR15 Misc Control 1 R (%02x)\n", m_pr15);
	return m_pr15;
}

void wd90c00_vga_device::misc_control_1_w(offs_t offset, u8 data)
{
	LOG("PR15 Misc Control 1 W %02x\n", data);
	m_pr15 = data;
}

/*
 * [0x2f] PR16 Misc Control 2
 */

/*
 * [0x30] PR17 Misc Control 3
 */

/**************************************
 *
 * Western Digital WD90C11A
 *
 *************************************/

wd90c11a_vga_device::wd90c11a_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: wd90c00_vga_device(mconfig, type, tag, owner, clock)
	, m_ext_seq_view(*this, "ext_seq_view")
{
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(wd90c11a_vga_device::sequencer_map), this));
}

wd90c11a_vga_device::wd90c11a_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd90c11a_vga_device(mconfig, WD90C11A, tag, owner, clock)
{
}

void wd90c11a_vga_device::device_reset()
{
	wd90c00_vga_device::device_reset();

	m_ext_seq_unlock = false;
	m_ext_seq_view.select(0);
	m_pr31 = 0;
}

void wd90c11a_vga_device::sequencer_map(address_map &map)
{
	wd90c00_vga_device::sequencer_map(map);
	map(0x06, 0x06).rw(FUNC(wd90c11a_vga_device::ext_seq_status_r), FUNC(wd90c11a_vga_device::ext_seq_unlock_w));
	map(0x07, 0x1f).view(m_ext_seq_view);
	m_ext_seq_view[0](0x07, 0x1f).lr8(
		NAME([this] (offs_t offset) {
			if (!machine().side_effects_disabled())
				LOGLOCKED("Attempt to R ext. Sequencer register offset %02x while locked\n", offset + 0x07);
			return 0xff;
		})
	);
//  m_ext_seq_view[1](0x07, 0x07) PR21 Display Configuration and Scratch Pad
	m_ext_seq_view[1](0x08, 0x08).ram(); // PR22 'C11A only Scratch Pad
	m_ext_seq_view[1](0x09, 0x09).ram(); // PR23 'C11A only Scratch Pad
//  m_ext_seq_view[1](0x10, 0x10) PR30 Memory Interface and FIFO Control
	m_ext_seq_view[1](0x11, 0x11).rw(FUNC(wd90c11a_vga_device::sys_if_control_r), FUNC(wd90c11a_vga_device::sys_if_control_w));
//  m_ext_seq_view[1](0x12, 0x12) PR32 Miscellaneous Control 4
}

// unlock also dictates index mask
u8 wd90c11a_vga_device::sequencer_data_r(offs_t offset)
{
	const u8 seq_index = vga.sequencer.index & (m_ext_seq_unlock ? 0xff : 0x07);
	return space(SEQ_REG).read_byte(seq_index);
}

void wd90c11a_vga_device::sequencer_data_w(offs_t offset, u8 data)
{
	const u8 seq_index = vga.sequencer.index & (m_ext_seq_unlock ? 0xff : 0x07);
	vga.sequencer.data[seq_index] = data;
	space(SEQ_REG).write_byte(seq_index, data);
	recompute_params();
}

/*
 * [0x06] PR20 Unlock PR21/PR32
 *
 * -x-x x--- Ext. Sequencer unlock
 * -1-0 1--- Unlocks, any other value locks
 */
u8 wd90c11a_vga_device::ext_seq_status_r(offs_t offset)
{
	return (m_ext_seq_unlock ? 0x48 : 0x00);
}

void wd90c11a_vga_device::ext_seq_unlock_w(offs_t offset, u8 data)
{
	m_ext_seq_unlock = (data & 0x58) == 0x48;
	LOG("PR20 %s state (%02x)\n", m_ext_seq_unlock ? "unlock" : "lock", data);
	m_ext_seq_view.select(m_ext_seq_unlock);
}

/*
 * [0x11] PR31 System Interface Control
 *
 * Defaults to 0x6d on ct486
 *
 * x--- ---- Read/Write Offset Enable
 * -x-- ---- Turbo Mode for Blanked Lines
 * --x- ---- Turbo Mode for Text
 * ---x x--- CPU RDY release Control
 * ---- -x-- Enable Write Buffer
 * ---- --x- Enable 16-bit I/O for ATC
 * ---- ---x Enable 16-bit I/O for CRTC, Sequencer & GC
 */
u8 wd90c11a_vga_device::sys_if_control_r(offs_t offset)
{
	LOG("PR31 System Interface Control R (%02x)\n", m_pr31);
	return m_pr31;
}

void wd90c11a_vga_device::sys_if_control_w(offs_t offset, u8 data)
{
	LOG("PR31 System Interface Control W %02x\n", data);
	m_pr31 = data;
}

/**************************************
 *
 * Western Digital WD90C30
 *
 *************************************/

wd90c30_vga_device::wd90c30_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: wd90c11a_vga_device(mconfig, type, tag, owner, clock)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(wd90c30_vga_device::crtc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(wd90c30_vga_device::sequencer_map), this));
}

wd90c30_vga_device::wd90c30_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd90c30_vga_device(mconfig, WD90C30, tag, owner, clock)
{
}

void wd90c30_vga_device::device_reset()
{
	wd90c11a_vga_device::device_reset();

	m_pr18 = 0;
}

void wd90c30_vga_device::crtc_map(address_map &map)
{
	wd90c11a_vga_device::crtc_map(map);
//  map(0x20, 0x21) Signature read data
//  map(0x3d, 0x3d) PR1A CRTC Shadow Register Control
	map(0x3e, 0x3e).rw(FUNC(wd90c30_vga_device::vert_timing_overflow_r), FUNC(wd90c30_vga_device::vert_timing_overflow_w));
//  map(0x3f, 0x3f) PR19 Signature Analyzer Control
}

void wd90c30_vga_device::sequencer_map(address_map &map)
{
	wd90c11a_vga_device::sequencer_map(map);
//  m_ext_seq_view[1](0x13, 0x13) PR33 DRAM Timing and zero Wait State Control
//  m_ext_seq_view[1](0x14, 0x14) PR34 Video Memory Mapping
//  m_ext_seq_view[1](0x15, 0x15) PR35 USR0, USR1 Output Select, <reserved> on 'C31A
}

/*
 * [0x3e] PR18 CRTC Vertical Timing Overflow
 *
 * xxx- ---- <reserved>
 * ---x ---- Line Compare bit 10
 * ---- x--- Start vertical blank bit 10
 * ---- -x-- Start vertical retrace bit 10
 * ---- --x- Vertical display enable end bit 10
 * ---- ---x Vertical total bit 10
 */
u8 wd90c30_vga_device::vert_timing_overflow_r(offs_t offset)
{
	LOG("PR18 CRTC Vertical Timing Overflow R (%02x)\n", m_pr18);
	return m_pr18;
}

void wd90c30_vga_device::vert_timing_overflow_w(offs_t offset, u8 data)
{
	LOG("PR18 CRTC Vertical Timing Overflow W %02x\n", data);
	if (!BIT(m_crtc_lock, 1) && !vga.crtc.protect_enable)
	{
		vga.crtc.vert_disp_end = (vga.crtc.vert_disp_end & 0x03ff) | ((BIT(data, 1) << 10));
		m_pr18 &= ~2;
		m_pr18 |= (data & 2);
	}

	if (!BIT(m_crtc_lock, 0) || !vga.crtc.protect_enable)
	{
		vga.crtc.vert_blank_start = (vga.crtc.vert_blank_start & 0x03ff) | ((BIT(data, 3) << 10));
		//vga.crtc.vert_retrace =    (vga.crtc.vert_retrace & 0x03ff)    | ((BIT(data, 2) << 10));
		vga.crtc.vert_total =       (vga.crtc.vert_total & 0x03ff)       | ((BIT(data, 0) << 10));
		m_pr18 &= ~0x0d;
		m_pr18 |= (data & 0xd);
	}

	vga.crtc.line_compare = (vga.crtc.line_compare & 0x03ff) | ((BIT(data, 4) << 10));
	m_pr18 &= ~0xf0;
	m_pr18 |= data & 0xf0;
	recompute_params();
}

u16 wd90c30_vga_device::line_compare_mask()
{
	return svga.rgb8_en ? 0x7ff : 0x3ff;
}

/**************************************
 *
 * Western Digital WD90C31
 *
 *************************************/

wd90c31_vga_device::wd90c31_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: wd90c30_vga_device(mconfig, type, tag, owner, clock)
{
}

wd90c31_vga_device::wd90c31_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd90c31_vga_device(mconfig, WD90C31, tag, owner, clock)
{
}

// maps at $23c0 in normal conditions, 16-bit
void wd90c31_vga_device::ext_io_map(address_map &map)
{
//  map(0x00, 0x01) Index Control register
//  map(0x02, 0x03) Register Access port
//  map(0x04, 0x05) BITBLT I/O Port
//  map(0x06, 0x07) <reserved>
}

/*
 * Index Control register
 *
 * xx-- ---- ---- ---- <reserved>
 * --x- ---- ---- ---- (r/o) signals if an attempt to an unhandled device index is selected
 * ---x ---- ---- ---- Auto-increment disable
 * ---- xxxx ---- ---- Device register Index
 * ---- ---- xxxx xxxx Register block pointer
 * ---- ---- 0000 0000 System Control
 * ---- ---- 0000 0001 BITBLT
 * ---- ---- 0000 0010 HW Cursor
 *
 */

// System Control Register Block
// ext_io_view[0](0x00, 0x00) IRQ status
// BITBLT
// ext_io_view[1](0x00, 0x01) Control
// ext_io_view[1](0x02, 0x03) Source
// ext_io_view[1](0x04, 0x05) Destination
// ext_io_view[1](0x06, 0x07) Dimension X/Y
// ext_io_view[1](0x08, 0x08) Row pitch
// ext_io_view[1](0x09, 0x09) ROP type
// ext_io_view[1](0x0a, 0x0a) Foreground Color
// ext_io_view[1](0x0b, 0x0b) Background Color
// ext_io_view[1](0x0c, 0x0c) Transparency Color
// ext_io_view[1](0x0d, 0x0d) Transparency Mask
// ext_io_view[1](0x0e, 0x0e) Map and Plane Mask
// HW Cursor
// ext_io_view[2](0x00, 0x00) Control
// ext_io_view[2](0x01, 0x02) Pattern Address
// ext_io_view[2](0x03, 0x03) Primary Color
// ext_io_view[2](0x04, 0x04) Secondary Color
// ext_io_view[2](0x05, 0x05) Origin
// ext_io_view[2](0x06, 0x07) Display Position X/Y
// ext_io_view[2](0x08, 0x08) Auxiliary Color
// NOTE: on shutdown Win 95 will try to read HW Cursor Control even if disabled (?)

/**************************************
 *
 * Western Digital WD90C33
 *
 *************************************/

wd90c33_vga_device::wd90c33_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: wd90c31_vga_device(mconfig, type, tag, owner, clock)
{
}

wd90c33_vga_device::wd90c33_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd90c33_vga_device(mconfig, WD90C33, tag, owner, clock)
{
}

void wd90c33_vga_device::ext_io_map(address_map &map)
{
	wd90c31_vga_device::ext_io_map(map);
//  map(0x04, 0x07) Host Bit Block Transfer (HBLT), same as above but 32-bit?
//  map(0x08, 0x09) K1 Line Draw Constant 1
//  map(0x0a, 0x0b) K2 Line Draw Constant 2
//  map(0x0c, 0x0d) ET Line Draw Error Term
//  map(0x0e, 0x0f) Command Buffer and Interrupt
}

// maps at $23d0 in normal conditions, 8-bit
void wd90c33_vga_device::localbus_if_map(address_map &map)
{
//  map(0x00, 0x00) configuration
//  map(0x01, 0x01) wait state
//  map(0x02, 0x02) Video Memory Mapping Register (MMIO)
//  map(0x03, 0x03) (r/o) Status Register
}
