// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Angelo Salese

#include "emu.h"
#include "mga2064w.h"

#define LOG_WARN      (1U << 1)
#define LOG_ALIAS     (1U << 2) // log mgabase1 index setups thru the back door
#define LOG_DRAW      (1U << 3) // log drawing engine accesses
#define LOG_PIXELXFER (1U << 4) // log drawing pixel writes

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_DRAW)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGALIAS(...)           LOGMASKED(LOG_ALIAS, __VA_ARGS__)
#define LOGDRAW(...)            LOGMASKED(LOG_DRAW, __VA_ARGS__)
#define LOGPIXELXFER(...)       LOGMASKED(LOG_PIXELXFER, __VA_ARGS__)

DEFINE_DEVICE_TYPE(MGA2064W, mga2064w_device, "mga2064w", "Matrox Millennium \"IS-STORM / MGA-2064W\"")

mga2064w_device::mga2064w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, MGA2064W, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_svga(*this, "svga")
	, m_vga_rom(*this, "vga_rom")
{
	set_ids(0x102b0519, 0x01, 0x030000, 0x00000000);
	m_mgabase1_real_space_config = address_space_config("mgabase1_regs", ENDIANNESS_LITTLE, 32, 14, 0, address_map_constructor(FUNC(mga2064w_device::mgabase1_map), this));
}

ROM_START( mga2064w )
	ROM_REGION32_LE( 0x10000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "rev3", "Matrox Power Graphics Accelerator V2.4 IS-MGA-2064W R3" )
	ROMX_LOAD( "rev3.bin",     0x000000, 0x010000, CRC(cb623dab) SHA1(4dc10755613a8fa9599331d78995cfb15145440b), ROM_BIOS(0) )
	// TODO: verify label naming for these
	ROM_SYSTEM_BIOS( 1, "rev2", "Matrox Power Graphics Accelerator V1.9 IS-MGA-2064W R2 2MB" )
	ROMX_LOAD( "rev2_2mb.bin", 0x000000, 0x010000, CRC(253c352b) SHA1(a5cd7e1c4903fcc89ea04cc1911b8d010e6513d1), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "rev2_storm", "Matrox Power Graphics Accelerator V1.9 IS-STORM R2 (vbi)" )
	ROMX_LOAD( "rev2_storm4mb.vbi", 0x000000, 0x008000, CRC(35660abe) SHA1(36ec630507548e1ef5fa7fdd07852d936fb614e5), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "rev2_isstorm", "Matrox Power Graphics Accelerator V1.9 IS-STORM R2" )
	ROMX_LOAD( "matroxisstormr2.bin", 0x000000, 0x010000, CRC(0cfceda4) SHA1(26a4fe291c738b4b138b522beb37b7db1b639634), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "rev2_r2", "Matrox Power Graphics Accelerator V1.9 IS-MGA-2064W R2" )
	ROMX_LOAD( "matrox2064wr2.bin", 0x000000, 0x010000, CRC(79920e74) SHA1(d62d6a57c75f2266e3d0f85916f366d62ad56ce4), ROM_BIOS(4) )
ROM_END

const tiny_rom_entry *mga2064w_device::device_rom_region() const
{
	return ROM_NAME(mga2064w);
}

void mga2064w_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_svga, FUNC(matrox_vga_device::screen_update));

	MATROX_VGA(config, m_svga, 0);
	m_svga->set_screen("screen");
	m_svga->set_vram_size(8*1024*1024);
}

void mga2064w_device::device_start()
{
	pci_card_device::device_start();
	// NB: following is swapped on G400
	add_map(    16*1024, M_MEM, FUNC(mga2064w_device::mgabase1_map));
	add_map(8*1024*1024, M_MEM, FUNC(mga2064w_device::mgabase2_map));
	//  add_rom_from_region();

	add_rom((u8 *)m_vga_rom->base(), 0x10000);
}

void mga2064w_device::device_reset()
{
	pci_card_device::device_reset();

	// INTA#
	intr_pin = 1;
	m_mgabase1_real_index = 0;
	m_dwgreg.state = DRAW_IDLE;
}

device_memory_interface::space_config_vector mga2064w_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_IO, &m_mgabase1_real_space_config)
	};
}

void mga2064w_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
//  map(0x40, 0x43) OPTION
	map(0x44, 0x47).rw(FUNC(mga2064w_device::mga_index_r), FUNC(mga2064w_device::mga_index_w));
	map(0x48, 0x4b).rw(FUNC(mga2064w_device::mga_data_r), FUNC(mga2064w_device::mga_data_w));
}

void mga2064w_device::mgabase1_map(address_map &map)
{
	map(0x0000, 0x1bff).rw(FUNC(mga2064w_device::dmawin_idump_r), FUNC(mga2064w_device::dmawin_iload_w));
	map(0x1c00, 0x1dff).m(FUNC(mga2064w_device::dwgreg_map));
//  map(0x1e00, 0x1eff) HSTREG Host registers
	map(0x1e10, 0x1e13).r(FUNC(mga2064w_device::fifo_status_r));
	map(0x1e14, 0x1e17).r(FUNC(mga2064w_device::status_r));
//  map(0x1e18, 0x1e1b) ICLEAR
//  map(0x1e1c, 0x1e1f) IEN
	map(0x1e20, 0x1e23).r(m_svga, FUNC(matrox_vga_device::vcount_r));
//  map(0x1e40, 0x1e43) Reset
//  map(0x1e54, 0x1e57) OPMODE
//  map(0x1f00, 0x1fff) VGA CRTC linear I/O
	map(0x1fb0, 0x1fdf).m(m_svga, FUNC(matrox_vga_device::io_map));
	map(0x3c00, 0x3c1f).m(m_svga, FUNC(matrox_vga_device::ramdac_ext_map));
//  map(0x3e00, 0x3fff) EXPDEV Expansion bus
}

void mga2064w_device::mgabase2_map(address_map &map)
{
	map(0x000000, 0x7fffff).rw(m_svga, FUNC(matrox_vga_device::mem_linear_r), FUNC(matrox_vga_device::mem_linear_w));
}

// assume all registers to work with dword accesses only
// all signed registers are in two's complement
// TODO: accessing 0x1dxx starts the drawing engine
// will otherwise treat iload / idump access as register access,
// it's also necessary for anything like BMONOLEF to work at least.
void mga2064w_device::dwgreg_map(address_map &map)
{
	// DWGCTL
	map(0x0000, 0x0003).w(FUNC(mga2064w_device::dwgctl_w));
	// MACCESS
	map(0x0004, 0x0007).w(FUNC(mga2064w_device::maccess_w));
//  map(0x0008, 0x000b) <reserved> MCTLWTST
	// ZORG
	map(0x000c, 0x000f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// must be multiple of 512
			m_dwgreg.zorg = data & 0x7fffff;
			LOGDRAW("dwgreg: ZORG %08x & %08x\n", data, mem_mask);
		})
	);
	// PAT0 / PAT1
	map(0x0010, 0x0017).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGDRAW("dwgreg: PAT%d %08x & %08x\n", offset, data, mem_mask);
			// TODO: alternate way to load SRC registers, in 8x8 Windows format
		})
	);
	// PLNWT
	map(0x001c, 0x001f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGDRAW("dwgreg: PLNWT %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_dwgreg.plnwt);
		})
	);
	// BCOL / backcol
	map(0x0020, 0x0023).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGDRAW("dwgreg: BCOL %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_dwgreg.bcol);
		})
	);
	// FCOL / forcol
	map(0x0024, 0x0027).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGDRAW("dwgreg: FCOL %08x & %08x\n", data, mem_mask);
			COMBINE_DATA(&m_dwgreg.fcol);
		})
	);
//  map(0x002c, 0x002f) <reserved> SRCBLT
	// SRC0-3
	map(0x0030, 0x003f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGDRAW("dwgreg: SRC[%01d] -> %08x & %08x\n", offset, data, mem_mask);
			COMBINE_DATA(&m_dwgreg.src[offset]);
		})
	);
	// XYSTRT
	map(0x0040, 0x0043).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGDRAW("dwgreg: XYSTRT %08x & %08x\n", data, mem_mask);
			// TODO: alternate way to load AR5 / AR6 / XDST / YDST
		})
	);
	// XYEND
	map(0x0044, 0x0047).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGDRAW("dwgreg: XYEND %08x & %08x\n", data, mem_mask);
			// TODO: alternate way to load AR0 / AR2
		})
	);
	// SHIFT
	map(0x0050, 0x0053).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGDRAW("dwgreg: SHIFT %08x & %08x\n", data, mem_mask);
			LOGDRAW("\tfuncnt %d|x_off %d|y_off %d|stylelen %d|funoff %d\n"
				, data & 0x7f
				// funcnt and x_off / y_off are shared, x_off must really be with bit 3 off
				, data & 0x0f
				, (data & 0x30) >> 4
				// stylelen and funoff are shared
				, (data >> 16) & 0x7f
				, (data >> 16) & 0x3f
			);
			// TODO: related to PAT0 / PAT1 registers
		})
	);
	// SGN
	map(0x0058, 0x005b).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGDRAW("dwgreg: SGN %08x & %08x\n", data, mem_mask);
			LOGDRAW("\tsdydxl %s|scanleft %d|sdxl %s|sdy %s|sdxr %d\n"
				, BIT(data, 0) ? "x major axis" : "y major axis"
				// sdydxl and scanleft are shared
				, BIT(data, 0)
				, BIT(data, 1) ? "-x delta" : "+x delta"
				, BIT(data, 2) ? "-y delta" : "+y delta"
				, BIT(data, 5) ? "-x delta" : "+x delta"
			);
		})
	);
	// LEN
	map(0x005c, 0x005f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			m_dwgreg.len = data & 0xffff;
			LOGDRAW("dwgreg: LEN %08x & %08x %d\n", data, mem_mask, m_dwgreg.len);
		})
	);
	// AR0-6
	// TODO: documentation for each reg
	map(0x0060, 0x007b).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGDRAW("dwgreg: AR[%01d] -> %08x & %08x\n", offset, data, mem_mask);
			COMBINE_DATA(&m_dwgreg.ar[offset]);
		})
	);
	// CXBNDRY
	map(0x0080, 0x0083).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			m_dwgreg.cxleft = data & 0x7ff;
			m_dwgreg.cxright = (data >> 16) & 0x7ff;
			LOGDRAW("dwgreg: CXBNDRY %08x & %08x (CXLEFT %d|CXRIGHT %d)\n"
				, data, mem_mask
				, m_dwgreg.cxleft, m_dwgreg.cxright
			);
		})
	);
	// FXBNDRY
	map(0x0084, 0x0087).select(0x100).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// signed 16-bit
			m_dwgreg.fxleft = (s16)(data & 0xffff);
			m_dwgreg.fxright = (s16)(data >> 16);
			LOGDRAW("dwgreg: FXBNDRY %08x & %08x (FXLEFT %d|FXRIGHT %d)\n"
				, data, mem_mask
				, m_dwgreg.fxleft, m_dwgreg.fxright
			);
			if (BIT(offset, 6))
				draw_trigger();
		})
	);
	// YDSTLEN
	map(0x0088, 0x008b).select(0x100).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// alternative way to access YDST (bits 31-16) and LEN (15-0) with a single dword
			m_dwgreg.len = data & 0xffff;
			// TODO: YDST bits 31-16 with signed conversion
			m_dwgreg.ydst = util::sext(data >> 16, 15);
			LOGDRAW("dwgreg: YDSTLEN %08x & %08x (YDST %d|LEN %d)\n"
				, data, mem_mask
				, data >> 16, m_dwgreg.len
			);
			if (BIT(offset, 6))
				draw_trigger();
		})
	);
	// PITCH
	map(0x008c, 0x008f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			m_dwgreg.pitch = data & 0xfff;
			LOGDRAW("dwgreg: PITCH %08x & %08x %d|ylin %d %s\n"
				, data, mem_mask
				, m_dwgreg.pitch
				, BIT(data, 15), BIT(data, 15) ? "linear format" : "xy format"
			);
		})
	);
	// YDST
	map(0x0090, 0x0093).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// FIXME: signed 22-bits
			m_dwgreg.ydst = data & 0x3fffff;
			m_dwgreg.sellin = (data >> 29) & 7;
			// TODO: depends on ylin for bit meaning
			LOGDRAW("dwgreg: YDST %08x & %08x|ydst %08x|sellin %d\n"
				, data, mem_mask
				, m_dwgreg.ydst
				, m_dwgreg.sellin
			);
		})
	);
	// YDSTORG
	map(0x0094, 0x0097).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// unsigned 23-bit
			m_dwgreg.ydstorg = data & 0x7fffff;
			LOGDRAW("dwgreg: YDSTORG %08x & %08x %d\n"
				, data, mem_mask
				, m_dwgreg.ydstorg
			);
		})
	);
	// YTOP / cytop
	map(0x0098, 0x009b).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// unsigned 23-bit
			m_dwgreg.cytop = data & 0x7fffff;
			LOGDRAW("dwgreg: YTOP %08x & %08x\n", data, mem_mask);
		})
	);
	// YBOT / cybot
	map(0x009c, 0x009f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// unsigned 23-bit
			m_dwgreg.cybot = data & 0x7fffff;
			LOGDRAW("dwgreg: YBOT %08x & %08x\n", data, mem_mask);
		})
	);
	// CXLEFT
	map(0x00a0, 0x00a3).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			m_dwgreg.cxleft = data & 0x7ff;
			LOGDRAW("dwgreg: CXLEFT %08x & %08x %d\n"
				, data, mem_mask
				, m_dwgreg.cxleft
			);
		})
	);
	// CXRIGHT
	map(0x00a4, 0x00a7).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			m_dwgreg.cxright = data & 0x7ff;
			LOGDRAW("dwgreg: CXRIGHT %08x & %08x %d\n"
				, data, mem_mask
				, m_dwgreg.cxright
			);
		})
	);
	// FXLEFT
	map(0x00a8, 0x00ab).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// signed 16-bit
			m_dwgreg.fxleft = (s16)(data & 0xffff);
			LOGDRAW("dwgreg: FXLEFT %08x & %08x %d\n"
				, data, mem_mask
				, m_dwgreg.fxleft
			);
		})
	);
	// FXRIGHT
	map(0x00ac, 0x00af).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// signed 16-bit
			m_dwgreg.fxright = (s16)(data & 0xffff);
			LOGDRAW("dwgreg: FXRIGHT %08x & %08x %d\n"
				, data, mem_mask
				, m_dwgreg.fxright
			);
		})
	);
	// XDST
	map(0x00b0, 0x00b3).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// signed 16-bit
			m_dwgreg.xdst = (s16)(data & 0xffff);
			LOGDRAW("dwgreg: XDST %08x & %08x %d\n"
				, data, mem_mask
				, m_dwgreg.xdst
			);
		})
	);
	// DR0-DR15 (DR1-5-9-13 <reserved>)
	map(0x00c0, 0x00ff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			LOGDRAW("dwgreg: DR[%01d] -> %08x\n", offset, data, mem_mask);
			if ((offset & 3) == 1)
			{
				LOGWARN("dwgreg: attempt to setup reserved DR%01d (ignored)\n", offset);
				return;
			}
			COMBINE_DATA(&m_dwgreg.dr[offset]);
		})
	);
}

void mga2064w_device::dwgctl_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dwgreg.dwgctl);
	LOGDRAW("dwgreg: DWGCTL -> %08x & %08x\n", data, mem_mask);
	const char *const opcode_mnemonics[16] = {
		"LINE_OPEN", "AUTOLINE_OPEN", "LINE_CLOSE", "AUTOLINE_CLOSE",
		"TRAP",      "TEXTURE_TRAP",  "<reserved>", "<reserved>",
		"BITBLT",    "ILOAD",         "IDUMP",      "<reserved>",
		"FBITBLIT",  "ILOAD_SCALE",   "<reserved>", "ILOAD_FILTER"
	};
	const char *const atype_mnemonics[8] = {
		"RPL", "RSTR",       "<reserved>", "ZI",
		"BLK", "<reserved>", "<reserved>", "I"
	};
	const char *const zmode_mnemonics[8] = {
		"NOZCMP", "<reserved>", "ZE",  "ZNE",
		"ZLT",    "ZLTE",       "ZGT", "ZGTE"
	};
	const char *const bop_mnemonics[16] = {
		"0",        "~(D | S)", "D & ~S", "~S",
		"(~D) & S", "~D",       "D ^ S",  "~(D & S)",
		"D & S",    "~(D ^ S)", "D",      "D | ~S",
		"S",        "(~D) | S", "D | S",  "1"
	};
	const char *const bltmod_mnemonics[16] = {
		"BMONOLEF",   "BPLAN",      "BFCOL",      "BU32BGR",
		"BMONOWF",    "<reserved>", "<reserved>", "BU32RGB",
		"<reserved>", "<reserved>", "<reserved>", "BU24BGR",
		"<reserved>", "<reserved>", "BUYUV",      "BU24RGB"
	};
	LOGDRAW("\topcod %02x %s|atype %02x %s|%s mode|zmode %02x %s|\n"
		, m_dwgreg.dwgctl & 0xf, opcode_mnemonics[m_dwgreg.dwgctl & 0xf]
		, (m_dwgreg.dwgctl >> 4) & 7, atype_mnemonics[(m_dwgreg.dwgctl >> 4) & 7]
		, BIT(m_dwgreg.dwgctl, 7) ? "linear bitblt" : "xy bitblt"
		, (m_dwgreg.dwgctl >> 8) & 7, zmode_mnemonics[(m_dwgreg.dwgctl >> 8) & 7]
	);
	LOGDRAW("\tbop %02x %s|bltmod %02x %s|pattern %d|transc %d|\n"
		, (m_dwgreg.dwgctl >> 16) & 0xf, bop_mnemonics[(m_dwgreg.dwgctl >> 16) & 0xf]
		, (m_dwgreg.dwgctl >> 25) & 0xf, bltmod_mnemonics[(m_dwgreg.dwgctl >> 25) & 0xf]
		, BIT(m_dwgreg.dwgctl, 29)
		, BIT(m_dwgreg.dwgctl, 30)
	);
	LOGDRAW("\tsolid %d|arzero %d|sgnzero %d|shftzero %d|trans %02x|\n"
		, BIT(m_dwgreg.dwgctl, 11)
		, BIT(m_dwgreg.dwgctl, 12)
		, BIT(m_dwgreg.dwgctl, 13)
		, BIT(m_dwgreg.dwgctl, 14)
		, (m_dwgreg.dwgctl >> 20) & 0xf
	);
}

void mga2064w_device::maccess_w(offs_t offset, u32 data, u32 mem_mask)
{
	const char *const pwidth_mnemonics[4] = {
		"PW8", "PW16", "PW32", "PW16"
	};
	COMBINE_DATA(&m_dwgreg.maccess);
	LOGDRAW("dwgreg: MACCESS %08x & %08x\n", data, mem_mask);
	LOGDRAW("\tpwidth %d %s|memreset %d|dither %d|dit555 %d\n"
		, m_dwgreg.maccess & 3, pwidth_mnemonics[m_dwgreg.maccess & 3]
		, BIT(m_dwgreg.maccess, 15)
		// nodither, flipped for convenience
		, !BIT(m_dwgreg.maccess, 30)
		, BIT(m_dwgreg.maccess, 31)
	);
}
/*
 * MGABASE1 + 1e10h FIFO Status (r/o)
 *
 * ---- -x-- ---- ---- BEMPTY Bus FIFO empty
 * ---- --x- ---- ---- BFULL Bus FIFO full
 * ---- ---- ---x xxxx FIFOCOUNT free locations in FIFO (max: 32)
 */
u32 mga2064w_device::fifo_status_r()
{
	return (1 << 9) | 32;
}

/*
 * MGABASE1 + 1e14h Status (r/o)
 *
 * ---- ---- ---- ---x ---- ---- ---- ---- DWGENGSTS
 * ---- ---- ---- ---- ---- ---- -x-- ---- EXTPEN
 * ---- ---- ---- ---- ---- ---- --x- ---- VLINEPEN
 * ---- ---- ---- ---- ---- ---- ---x ---- VSYNCPEN
 * ---- ---- ---- ---- ---- ---- ---- x--- VSYNCSTS
 * ---- ---- ---- ---- ---- ---- ---- -x-- PICKPEN
 */
u32 mga2064w_device::status_r()
{
	return m_svga->vsync_status() << 3;
}

void mga2064w_device::draw_trigger()
{
	LOGDRAW("\tstart trigger\n");
	const u8 opcod = m_dwgreg.dwgctl & 0xf;
	const u8 bop = (m_dwgreg.dwgctl >> 16) & 0xf;
	const u8 bltmod = (m_dwgreg.dwgctl >> 25) & 0xf;

	if (bop != 0xc)
		return;

	const s32 ystart = m_dwgreg.ydst;
	const s32 yend = ystart + m_dwgreg.len;
	const s32 xstart = m_dwgreg.fxleft;
	const s32 xend = m_dwgreg.fxright;

	if (m_dwgreg.state != DRAW_IDLE)
		LOGWARN("\t(in-flight! %d)\n", m_dwgreg.state);

	m_dwgreg.state = DRAW_IDLE;

	switch(opcod)
	{
		// TRAP / RECT
		case 4:
		{
			if (BIT(m_dwgreg.dwgctl, 7))
			{
				LOGWARN("\tTRAP in linear mode (unemulated)\n");
				return;
			}
			for (int y = ystart; y < yend; y++)
			{
				for (int x = xstart; x < xend; x++)
				{
					m_svga->write_memory(x + (y * m_dwgreg.pitch), m_dwgreg.fcol & 0xff);
				}
			}
			break;
		}
		// BITBLT
		case 8:
		{
			if (BIT(m_dwgreg.dwgctl, 7))
			{
				// BMONOLEF only for now
				if (bltmod)
					return;

				const u32 source_base = m_dwgreg.ar[3] & 0xffffff;
				const s32 source_pitch = util::sext(m_dwgreg.ar[5], 17);
				int src_x = 0;
				int src_y = 0;
				for (int y = ystart; y < yend; y ++, src_y ++)
				{
					for (int x = xstart; x < xend; x+=8, src_x ++)
					{
						const u32 char_position = (src_x + src_y * source_pitch);
						const u8 char_data = m_svga->read_memory(source_base + char_position);
						for (int xi = 0; xi < 8; xi ++)
						{
							const u8 pen_dot = (char_data >> (7-xi)) & 1;

							u8 color_pen = (pen_dot ? m_dwgreg.fcol : m_dwgreg.bcol) & 0xff;
							m_svga->write_memory((x + xi) + m_dwgreg.pitch * y, color_pen);
						}

					}
				}
			}
			else
			{
				const u32 source_base = m_dwgreg.ar[3] & 0xffffff;
				const s32 source_pitch = util::sext(m_dwgreg.ar[5], 17);
				int src_x = 0;
				int src_y = 0;
				for (int y = ystart; y < yend; y++, src_y++)
				{
					for (int x = xstart; x < xend; x++, src_x++)
					{
						u8 color_pen = m_svga->read_memory(source_base + (source_pitch * src_y) + src_x);
						m_svga->write_memory(x + y * m_dwgreg.pitch, color_pen);
					}
				}
			}
			break;
		}
		// ILOAD
		case 9:
			m_dwgreg.state = DRAW_ILOAD;
			m_dwgreg.current_x = 0;
			m_dwgreg.current_y = 0;
			break;
		// IDUMP
		case 0xa:
			m_dwgreg.state = DRAW_IDUMP;
			m_dwgreg.current_x = 0;
			m_dwgreg.current_y = 0;
			break;
	}

	LOGDRAW("\n");
}

u32 mga2064w_device::dmawin_idump_r(offs_t offset, u32 mem_mask)
{
	u32 res = 0;
	LOGPIXELXFER("dmawin_idump_r [%08x] & %08x %d %d (state %d)\n", offset * 4, mem_mask, m_dwgreg.current_x, m_dwgreg.current_y, m_dwgreg.state);
	switch(m_dwgreg.state)
	{
		case DRAW_IDUMP:
		{
			const u32 y_base = m_dwgreg.pitch * (m_dwgreg.current_y + m_dwgreg.ydst);
			const u32 x_base = m_dwgreg.current_x + m_dwgreg.fxleft;

			for (int xi = 0; xi < 4; xi ++)
				res |= m_svga->read_memory((y_base) + (x_base + xi)) << (xi * 8);
			m_dwgreg.current_x += 4;
			if (m_dwgreg.current_x > m_dwgreg.ar[0])
			{
				m_dwgreg.current_x = 0;
				m_dwgreg.current_y ++;
				if (m_dwgreg.current_y > m_dwgreg.len)
					m_dwgreg.state = DRAW_IDLE;
			}

			return res;
		}
		default:
			LOGWARN("Unemulated IDUMP read state [%08x] mem_mask %08x\n", offset * 4, mem_mask);
			res = 0xdeadbeef;
			break;
	}
	return res;
}

void mga2064w_device::dmawin_iload_w(offs_t offset, u32 data, u32 mem_mask)
{
	// assume dword accesses unless something dull proves otherwise
	LOGPIXELXFER("dmawin_iload_w [%08x] %08x & %08x %d %d (state %d)\n", offset * 4, data, mem_mask, m_dwgreg.current_x, m_dwgreg.current_y, m_dwgreg.state);
	switch(m_dwgreg.state)
	{
		case DRAW_ILOAD:
		{
			const u32 y_base = m_dwgreg.pitch * (m_dwgreg.current_y + m_dwgreg.ydst);
			const u32 x_base = m_dwgreg.current_x + m_dwgreg.fxleft;

			for (int xi = 0; xi < 4; xi++)
			{
				u8 color_pen = (data >> (8 * xi)) & 0xff;
				m_svga->write_memory((y_base) + (x_base + xi), color_pen);
			}
			m_dwgreg.current_x += 4;
			if (m_dwgreg.current_x > m_dwgreg.ar[0])
			{
				m_dwgreg.current_x = 0;
				m_dwgreg.current_y ++;
				// Note: this is unnecessary, really changes with OPMODE
				if (m_dwgreg.current_y > m_dwgreg.len)
					m_dwgreg.state = DRAW_IDLE;
			}

			break;
		}
		default:
			LOGWARN("Unemulated ILOAD write state [%08x] %08x & %08x\n", offset * 4, data, mem_mask);
			break;
	}
}

// TODO: this should really be a subclass of VGA
void mga2064w_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(mga2064w_device::vram_r), FUNC(mga2064w_device::vram_w));
}

void mga2064w_device::legacy_io_map(address_map &map)
{
	map(0, 0x02f).m(m_svga, FUNC(matrox_vga_device::io_map));
}

uint8_t mga2064w_device::vram_r(offs_t offset)
{
	return downcast<matrox_vga_device *>(m_svga.target())->mem_r(offset);
}

void mga2064w_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<matrox_vga_device *>(m_svga.target())->mem_w(offset, data);
}

void mga2064w_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// TODO: both can be disabled thru config options
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(mga2064w_device::vram_r)), write8sm_delegate(*this, FUNC(mga2064w_device::vram_w)));

		io_space->install_device(0x03b0, 0x03df, *this, &mga2064w_device::legacy_io_map);
	}
}

/*
 * MGA_INDEX / MGA_DATA
 * aliases for accessing mgabase1 thru PCI config space
 * i.e. a backdoor for x86 in real mode
 */

u32 mga2064w_device::mga_index_r()
{
	LOGALIAS("MGA_INDEX read\n");
	return m_mgabase1_real_index & 0x3ffc;
}

void mga2064w_device::mga_index_w(offs_t offset, u32 data, u32 mem_mask)
{
	// VESA BIOS sets up $3c0a while accessing with mask 0x00ff0000
	// bits 0-1 are reserved and don't respond, assume mistake
	LOGALIAS("MGA_INDEX write %08x %08x\n", data, mem_mask);
	COMBINE_DATA(&m_mgabase1_real_index);
	m_mgabase1_real_index &= 0x3ffc;
}

u32 mga2064w_device::mga_data_r(offs_t offset, u32 mem_mask)
{
	return space(AS_IO).read_dword(m_mgabase1_real_index, mem_mask);
}

void mga2064w_device::mga_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	space(AS_IO).write_dword(m_mgabase1_real_index, data, mem_mask);
}
