// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "mb86292.h"

#include "screen.h"

#define LOG_WARN      (1U << 1)
#define LOG_REGS      (1U << 2)
#define LOG_CRTC      (1U << 3)
#define LOG_DASM      (1U << 4) // display list FIFO commands

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_REGS | LOG_CRTC | LOG_DASM)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGREGS(...)            LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGCRTC(...)            LOGMASKED(LOG_CRTC, __VA_ARGS__)
#define LOGDASM(...)            LOGMASKED(LOG_DASM, __VA_ARGS__)

//DEFINE_DEVICE_TYPE(MB86290A, mb86290a_device, "mb86290a", "Fujitsu MB86290A \"Cremson\" Graphics Controller")
//DEFINE_DEVICE_TYPE(MB86291, mb86291_device, "mb86291", "Fujitsu MB86291 \"Scarlet\" Graphics Controller")
DEFINE_DEVICE_TYPE(MB86292, mb86292_device, "mb86292", "Fujitsu MB86292 \"Orchid\" Graphics Controller")
//DEFINE_DEVICE_TYPE(MB86293, mb86293_device, "mb86293", "Fujitsu MB86293 \"Coral LQ\" Graphics Controller")
//DEFINE_DEVICE_TYPE(MB86294, mb86294_device, "mb86294", "Fujitsu MB86294 \"Coral LB\" Graphics Controller")
//DEFINE_DEVICE_TYPE(MB86294S, mb86294s_device, "mb86294s", "Fujitsu MB86294S \"Coral LB\" Graphics Controller")
// PCI, to move in sub-file
//DEFINE_DEVICE_TYPE(MB86295S, mb86295s_device, "mb86295s", "Fujitsu MB86295S \"Coral P\" Graphics Controller")
// set_ids(0x10cf2019, <rev>, 0x038000, <subvendor>);
// INTA#
//DEFINE_DEVICE_TYPE(MB86296S, mb86296s_device, "mb86296s", "Fujitsu MB86296S \"Coral PA\" Graphics Controller")
// set_ids(0x10cf201e, <rev>, 0x038000, <subvendor>);
//DEFINE_DEVICE_TYPE(MB86297A, mb86297a_device, "mb86297a", "Fujitsu MB86297A \"Carmine\" Graphics Controller")
// set_ids(0x10cf202b, <rev>, 0x038000, <subvendor>);


mb86292_device::mb86292_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_vram(*this, finder_base::DUMMY_TAG)
{
}

mb86292_device::mb86292_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: mb86292_device(mconfig, MB86292, tag, owner, clock)
{
}

void mb86292_device::device_start()
{
	save_item(NAME(m_dce));
	save_item(STRUCT_MEMBER(m_displaylist, lsa));
	save_item(STRUCT_MEMBER(m_displaylist, lco));
	save_item(STRUCT_MEMBER(m_displaylist, lreq));
	save_item(STRUCT_MEMBER(m_crtc, hdp));
	save_item(STRUCT_MEMBER(m_crtc, hdb));
	save_item(STRUCT_MEMBER(m_crtc, hsp));
	save_item(STRUCT_MEMBER(m_crtc, hsw));
	save_item(STRUCT_MEMBER(m_crtc, vtr));
	save_item(STRUCT_MEMBER(m_crtc, vsp));
	save_item(STRUCT_MEMBER(m_crtc, vdp));
	save_item(STRUCT_MEMBER(m_crtc, vsw));
	save_item(STRUCT_MEMBER(m_fb, xres));
	save_item(STRUCT_MEMBER(m_clayer, cm));
	save_item(STRUCT_MEMBER(m_clayer, cc));
	save_item(STRUCT_MEMBER(m_clayer, ch));
	save_item(STRUCT_MEMBER(m_clayer, cw));
	save_item(STRUCT_MEMBER(m_clayer, cda));
}

void mb86292_device::device_reset()
{
	m_dce = 0;
	m_displaylist.lsa = m_displaylist.lco = 0;
	m_displaylist.lreq = false;
}

void mb86292_device::vregs_map(address_map &map)
{
	// 0x1fc0000 Host interface HostBase
//  map(0x00000, 0x00003) DTC DMA Transfer Count
//  map(0x00004, 0x00004) DSU DMA Set Up
//  map(0x00005, 0x00005) DRM DMA Request Mask
//  map(0x00006, 0x00006) DST DMA STatus
//  map(0x00008, 0x00008) DTS DMA Transfer Stop
//  map(0x00009, 0x00009) LTS display [List] Transfer Stop
//  map(0x00010, 0x00010) LSTA display List Transfer STAtus
//  map(0x00018, 0x00018) DRQ DMA ReQuest
//  map(0x00020, 0x00023) IST Interrupt STatus
//  map(0x00024, 0x00027) MASK Interrupt MASK
//  map(0x0002c, 0x0002c) SRST Software ReSeT
	// LSA display List Source Address
	map(0x00040, 0x00043).lrw32(
		NAME([this] (offs_t offset) {
			return m_displaylist.lsa;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_displaylist.lsa);
			m_displaylist.lsa &= 0xffffff;
			LOGREGS("LSA %08x & %08x -> %08x\n", data, mem_mask, m_displaylist.lsa);
		})
	);
	// LCO display List COunt
	map(0x00044, 0x00047).lrw32(
		NAME([this] (offs_t offset) {
			return m_displaylist.lco;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_displaylist.lco);
			m_displaylist.lco &= 0xffffff;
			LOGREGS("LCO %08x & %08x -> %08x\n", data, mem_mask, m_displaylist.lco);
		})
	);
	// LREQ display List transfer REQuest
	map(0x00048, 0x00048).lrw8(
		NAME([this] (offs_t offset) {
			return m_displaylist.lreq;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_displaylist.lreq = bool(BIT(data, 0));
			LOGREGS("LREQ %02x\n", data, m_displaylist.lreq);
			process_display_list();
		})
	);
//  map(0x0fffc, 0x0ffff) MMR Memory I/F Mode Register

	// 0x1fd0000 Display engine DisplayBase
//  map(0x10000, 0x10001) DCM Display Control Mode
	// DCE Display Controller Enable
	map(0x10002, 0x10003).lrw16(
		NAME([this] (offs_t offset) {
			return m_dce;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_dce);
			LOGREGS("DCE %04x & %04x\n", data, mem_mask);
		})
	);
	// HTP Horizontal Total Pixels
	map(0x10006, 0x10007).lrw16(
		NAME([this] (offs_t offset) {
			return m_crtc.htp;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_crtc.htp);
			m_crtc.htp &= 0xfff;
			LOGCRTC("HTP %04x & %04x -> %d\n", data, mem_mask, m_crtc.htp + 1);
			reconfigure_screen();
		})
	);
	// HDP Horizontal Display Period
	map(0x10008, 0x10009).lrw16(
		NAME([this] (offs_t offset) {
			return m_crtc.hdp;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_crtc.hdp);
			m_crtc.hdp &= 0xfff;
			LOGCRTC("HDP %04x & %04x -> %d\n", data, mem_mask, m_crtc.hdp + 1);
			reconfigure_screen();
		})
	);
	// HDB Horizontal Display Boundary
	map(0x1000a, 0x1000b).lrw16(
		NAME([this] (offs_t offset) {
			return m_crtc.hdb;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_crtc.hdb);
			m_crtc.hdb &= 0xfff;
			LOGCRTC("HDB %04x & %04x -> %d\n", data, mem_mask, m_crtc.hdb + 1);
			reconfigure_screen();
		})
	);
	// HSP Horizontal Sync pulse Position
	map(0x1000c, 0x1000d).lrw16(
		NAME([this] (offs_t offset) {
			return m_crtc.hsp;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_crtc.hsp);
			m_crtc.hsp &= 0xfff;
			LOGCRTC("HSP %04x & %04x -> %d\n", data, mem_mask, m_crtc.hsp + 1);
			reconfigure_screen();
		})
	);
	// HSW Horizontal Sync pulse Width
	map(0x1000e, 0x1000e).lrw8(
		NAME([this] (offs_t offset) {
			return m_crtc.hsw;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_crtc.hsw = (data & 0x3f);
			LOGCRTC("HSW %04x -> %d\n", data, m_crtc.hsw + 1);
			reconfigure_screen();
		})
	);
	// VSW Vertical Sync pulse Width
	map(0x1000f, 0x1000f).lrw8(
		NAME([this] (offs_t offset) {
			return m_crtc.vsw;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_crtc.vsw = (data & 0x3f);
			LOGCRTC("VSW %04x -> %d\n", data, m_crtc.vsw + 1);
			reconfigure_screen();
		})
	);
	// VTR Vertical Total Rasters
	map(0x10012, 0x10013).lrw16(
		NAME([this] (offs_t offset) {
			return m_crtc.vtr;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_crtc.vtr);
			m_crtc.vtr &= 0xfff;
			// actually +1.5 & +3 for interlace mode fields
			LOGCRTC("VTR %04x & %04x -> %d\n", data, mem_mask, m_crtc.vtr + 1);
			reconfigure_screen();
		})
	);
	// VSP Vertical Sync pulse Position
	map(0x10014, 0x10015).lrw16(
		NAME([this] (offs_t offset) {
			return m_crtc.vsp;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_crtc.vsp);
			m_crtc.vsp &= 0xfff;
			LOGCRTC("VSP %04x & %04x -> %d\n", data, mem_mask, m_crtc.vsp + 1);
			reconfigure_screen();
		})
	);
	// VDP Vertical Display Period
	map(0x10016, 0x10017).lrw16(
		NAME([this] (offs_t offset) {
			return m_crtc.vdp;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_crtc.vdp);
			m_crtc.vdp &= 0xfff;
			LOGCRTC("VDP %04x & %04x -> %d\n", data, mem_mask, m_crtc.vdp + 1);
			reconfigure_screen();
		})
	);
//  map(0x10018, 0x10019) WX Window position X
//  map(0x1001a, 0x1001b) WX Window position Y
//  map(0x1001c, 0x1001d) WW Window Width
//  map(0x1001e, 0x1001f) WH Window Height
	// CM C[onsole] layer Mode
	map(0x10020, 0x10023).lrw32(
		NAME([this] (offs_t offset) {
			return m_clayer.cm;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_clayer.cm);
			m_clayer.ch = (m_clayer.cm & 0xfff) + 1;
			m_clayer.cw = ((m_clayer.cm >> 16) & 0x3f) * 64;
			m_clayer.cc = bool(BIT(m_clayer.cm, 31));
			LOGREGS("CM %08x & %08x -> CW %d CH %d CC %d\n"
				, data, mem_mask
				, m_clayer.cw
				, m_clayer.ch
				, m_clayer.cc
			);
		})
	);
//  map(0x10024, 0x10027) COA C layer Origin Address
	// CDA C layer Display Address
	map(0x10028, 0x1002b).lrw32(
		NAME([this] (offs_t offset) {
			return m_clayer.cda;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_clayer.cda);
			m_clayer.cda &= 0x3ffffff;
			LOGREGS("CDA %08x & %08x -> %08x\n"
				, data, mem_mask
				, m_clayer.cda
			);
		})
	);
//  map(0x1002c, 0x1002d) CDX C layer Display position X
//  map(0x1002e, 0x1002f) CDY C layer Display position Y
//  map(0x10030, 0x10033) WM W[indow] layer Mode
//  map(0x10034, 0x10037) WOA W layer Origin Address
//  map(0x10038, 0x1003b) WDA W layer Display Address
//  map(0x10040, 0x10043) MLM M[iddle] L[eft] layer Mode
//  map(0x10044, 0x10047) MLOA0 ML Origin Address 0
//  map(0x10048, 0x1004b) MLDA0 ML Display Address 0
//  map(0x1004c, 0x1004f) MLOA1 ML Origin Address 1
//  map(0x10050, 0x10053) MLDA1 ML Display Address 1
//  map(0x10054, 0x10055) MLDX ML Display position X
//  map(0x10056, 0x10057) MLDY ML Display position Y
//  map(0x10058, 0x1005b) MRM M[iddle] R[ight] layer Mode
//  map(0x1005c, 0x1005f) MROA0 MR Origin Address 0
//  map(0x10060, 0x10063) MRDA0 MR Display Address 0
//  map(0x10064, 0x10067) MROA1 MR Origin Address 1
//  map(0x10068, 0x1006b) MRDA1 MR Display Address 1
//  map(0x1006c, 0x1006d) MRDX MR Display position X
//  map(0x1006e, 0x1006f) MRDY MR Display position Y
//  map(0x10070, 0x10073) BLM B[ase] L[eft] layer Mode
//  map(0x10074, 0x10077) BLOA0 BL Origin Address 0
//  map(0x10078, 0x1007b) BLDA0 BL Display Address 0
//  map(0x1007c, 0x1007f) BLOA1 BL Origin Address 1
//  map(0x10080, 0x10083) BLDA1 BL Display Address 1
//  map(0x10084, 0x10085) BLDX BL Display position X
//  map(0x10086, 0x10087) BLDY BL Display position Y
//  map(0x10088, 0x1008b) BRM B[ase] R[ight] layer Mode
//  map(0x1008c, 0x1008f) BROA0 BR Origin Address 0
//  map(0x10090, 0x10093) BRDA0 BR Display Address 0
//  map(0x10094, 0x10097) BROA1 BR Origin Address 1
//  map(0x10098, 0x1009b) BRDA1 BR Display Address 1
//  map(0x1009c, 0x1009d) BRDX BR Display position X
//  map(0x1009e, 0x1009f) BRDY BR Display position Y
//  map(0x100a0, 0x100a1) CUTC Cursor Transparent Control
//  map(0x100a2, 0x100a2) CPM Cursor Priority Mode
//  map(0x100a4, 0x100a7) CUOA0 CUrsor 0 Origin Address
//  map(0x100a8, 0x100a9) CUX0 CUrsor 0 X position
//  map(0x100aa, 0x100ab) CUY0 CUrsor 0 Y position
//  map(0x100ac, 0x100af) CUOA1 CUrsor 1 Origin Address
//  map(0x100b0, 0x100b1) CUX1 CUrsor 1 X position
//  map(0x100b2, 0x100b3) CUY1 CUrsor 1 Y position
//  map(0x100b4, 0x100b5) BRATIO Blend RATIO
//  map(0x100b6, 0x100b7) BMODE Blend MODE
//  map(0x100bc, 0x100bd) CTC C layer Transparent Control
//  map(0x100c0, 0x100c1) MRTC MR layer Transparent Control
//  map(0x100c2, 0x100c3) MLTC ML layer Transparent Control
//  map(0x10400, 0x107ff) CPAL C layer PALette
//  map(0x10800, 0x10bff) MBPAL M & B layer PALette
	// 0x1fd8000 Video capture CaptureBase
//  map(0x18000, 0x18003) VCM Video Capture Mode
//  map(0x18004, 0x18007) CSC Capture SCale
//  map(0x18008, 0x1800b) VCS Video Capture Status
//  map(0x18010, 0x18013) CBM video Capture Buffer Mode
//  map(0x18014, 0x18017) CBOA video Capture Buffer Origin Address
//  map(0x18018, 0x1801b) CBLA video Capture Buffer Limit Address
//  map(0x1801c, 0x1801d) CIHSTR Capture Image Horizontal STaRt
//  map(0x1801e, 0x1801f) CIVSTR Capture Image Vertical STaRt
//  map(0x18020, 0x18021) CIHEND Capture Image Horizontal END
//  map(0x18022, 0x18023) CIVEND Capture Image Vertical END
//  map(0x18028, 0x1802b) CHP Capture Horizontal Pixel
//  map(0x1802c, 0x1802f) CVP Capture Vertical Pixel
//  map(0x1c000, 0x1c003) CDCN Capture Data Count NTSC
//  map(0x1c004, 0x1c007) CDCP Capture Data Count PAL

	// 0x1fe0000 Internal texture memory TextureBase
//  map(0x20000, ...)
	// 0x1ff0000 Drawing engine DrawBase
//  map(0x30400, 0x30403) CTR ConTrol Register
//  map(0x30404, 0x30407) IFSR Input FIFO Status Register (CTR bits 14-12 alias)
//  map(0x30408, 0x3040b) IFCNT Input FIFO CouNTer (CTR bits 19-15 alias)
//  map(0x3040c, 0x3040f) SST Setup engine STatus (CTR bits 9-8 alias)
//  map(0x30410, 0x30413) DST DDA STatus (CTR bits 5-4 alias)
//  map(0x30414, 0x30417) PST Pixel engine STatus (CTR bits 1-0 alias)
//  map(0x30418, 0x3041b) EST Error STatus (CTR bits 24-22 alias)
//  map(0x30420, 0x30423) MDR0 MoDe Register 0 (miscellaneous)
//  map(0x30424, 0x30427) MDR1 MoDe Register 1 (line)
//  map(0x30428, 0x3042b) MDR2 MoDe Register 2 (polygon)
//  map(0x3042c, 0x3042f) MDR3 MoDe Register 3 (texture)
//  map(0x30430, 0x30433) MDR4 MoDe Register 4 (BitBLT)
//  map(0x30440, 0x30443) FBR Frame Buffer Register base address
	// XRES X RESoultion
	map(0x30444, 0x30447).lrw32(
		NAME([this] (offs_t offset) {
			return m_fb.xres;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_fb.xres);
			m_fb.xres &= 0xfff;
			LOGCRTC("XRES %04x & %04x -> %d\n", data, mem_mask, m_fb.xres);
		})
	);
//  map(0x30448, 0x3044b) ZBR Z-Buffer Register base address
//  map(0x3044c, 0x3044f) TBR Texture memory Base address
//  map(0x30450, 0x30453) PFBR 2d Polygon Flag Buffer base address
//  map(0x30454, 0x30457) CXMIN Clip X MINimum
//  map(0x30458, 0x3045b) CXMAX Clip X MAXimum
//  map(0x3045c, 0x3045f) CYMIN Clip Y MINimum
//  map(0x30460, 0x30463) CYMAX Clip Y MAXimum
//  map(0x30464, 0x30467) TXS TeXture Size
//  map(0x30468, 0x3046b) TIle Size
//  map(0x3046c, 0x3046f) TOA Texture buffer Offset Address
//  map(0x30480, 0x30483) FC Foreground Color
//  map(0x30484, 0x30487) BC Background Color
//  map(0x30488, 0x3048b) ALF ALpha Factor
//  map(0x3048c, 0x3048f) BLP Broken Line Pattern
//  map(0x303e0?, 0x303e3?) BLPO Broken Line Pattern Offset <- assume doc mistake, 0x490 seems more realistic
//  map(0x30494, 0x30497) TBC Texture Border Color
//  Other stuff in the area are apparently r/o copies of the drawing engine internals.
	// 0x1ff8000 Geometry engine GeometryBase
//  map(0x38000, 0x38000) GCTR Geometry ConTrol Register
//  map(0x38040, 0x38043) GMDR0 Geometry MoDe Register 0 (vertex)
//  map(0x38044, 0x38047) GMDR1 Geometry MoDe Register 1 (line)
//  map(0x38048, 0x3804b) GMDR2 Geometry MoDe Register 2 (triangle)
//  map(0x38400, 0x38403) DFIFOG Display List FIFO for Geometry
}

/*
 * CRTC section
 */

void mb86292_device::reconfigure_screen()
{
	const u16 hdb = m_crtc.hdb + 1;
	const u16 hdp = m_crtc.hdp + 1;
	const u16 hsp = m_crtc.hsp + 1;
	const u16 hse = hsp + m_crtc.hsw + 1;
	const u16 htp = m_crtc.htp + 1;
	// 0 < m_crtc.hdb <= m_crtc.hdp < m_crtc.hsp < (m_crtc.hsp + m_crtc.hsw + 1) < m_crtc.htp
	std::array<bool, 5> horiz_assert = {
		0 < hdb,
		hdb <= hdp,
		hdp < hsp,
		hsp < hse,
		hse < htp
	};
	if (!std::all_of(horiz_assert.begin(), horiz_assert.end(), [](bool res) { return res; }))
	{
		LOGCRTC("\tScreen off (H)\n");
		return;
	}
	const u16 vdp = m_crtc.vdp + 1;
	const u16 vsp = m_crtc.vsp + 1;
	const u16 vse = (vsp + m_crtc.vsw + 1);
	const u16 vtr = m_crtc.vtr + 1;
	// 0 < m_crtc.vdp < m_crtc.vsp < (m_crtc.vsp + m_crtc.vsw + 1) < m_crtc.vtr
	std::array<bool, 4> vert_assert = {
		0 < vdp,
		vdp <= vsp,
		vsp < vse,
		vse < vtr
	};
	if (!std::all_of(vert_assert.begin(), vert_assert.end(), [](bool res) { return res; }))
	{
		LOGCRTC("\tScreen off (V)\n");
		return;
	}
	// TODO: refresh rate, interlace, sync
	// Supported resolutions:
	// 1024x768, 1024x600, 800x600, 854x480, 640x480, 480x234, 400x234, 320x234
	LOGCRTC("\tSetting screen to %d x %d (total: %d x %d)\n", hdp, vdp, htp, vtr);
	rectangle visarea(0, hdp - 1, 0, vdp - 1);
	screen().configure(htp, vtr, visarea, screen().frame_period().attoseconds());
}

/*
 *
 * Display list
 *
 */

// Quick and dirty snippet to have something drawn,
// this is all done in FIFO and requires a timer (and loads of profiling ...)
void mb86292_device::process_display_list()
{
	if (!m_displaylist.lreq)
		return;

	m_displaylist.cur_address = m_displaylist.lsa;
	const u32 count = m_displaylist.lco == 0 ? 0x1000000 : (m_displaylist.lco << 2);
	const u32 end_address = m_displaylist.lsa + count;
//  printf("%08x %08x %08x\n", m_displaylist.cur_address, end_address, count);

	while (m_displaylist.cur_address < end_address)
	{
		u32 opcode = vram_read_dword(m_displaylist.cur_address);
		LOGDASM("PC=%08x %08x ", m_displaylist.cur_address, opcode);
		const u8 op_type = opcode >> 24;
		const u8 op_command = (opcode >> 16) & 0xff;
		u16 param_list = 1;
		switch(op_type)
		{
			case 0x0f:
			{
				u32 saddr = vram_read_dword(m_displaylist.cur_address + 0x04);
				u32 sstride = vram_read_dword(m_displaylist.cur_address + 0x08);
				u16 sry = vram_read_word(m_displaylist.cur_address + 0x0e);
				u16 srx = vram_read_word(m_displaylist.cur_address + 0x0c);
				u32 daddr = vram_read_dword(m_displaylist.cur_address + 0x10);
				u32 dstride = vram_read_dword(m_displaylist.cur_address + 0x14);
				u16 dry = vram_read_word(m_displaylist.cur_address + 0x1a);
				u16 drx = vram_read_word(m_displaylist.cur_address + 0x18);
				u16 brsizey = vram_read_word(m_displaylist.cur_address + 0x1e);
				u16 brsizex = vram_read_word(m_displaylist.cur_address + 0x1c);
				param_list += 7;
				LOGDASM("BltCopyAlternateP (%s)\n", op_command == 0x44 ? "TopLeft" : "<unknown>");
				LOGDASM("\t%08x %08x %04x|%04x\n"
					, saddr
					, sstride
					, sry
					, srx
				);
				LOGDASM("\t%08x %08x %04x|%04x\n"
					, daddr
					, dstride
					, dry
					, drx
				);
				LOGDASM("\t%04x|%04x\n", brsizey, brsizex);
				for (u16 yi = 0; yi < brsizey; yi ++)
				{
					const u32 src_ptr = saddr + (((sry + yi) * sstride) << 1);
					const u32 dst_ptr = daddr + (((dry + yi) * dstride) << 1);
					for (u16 xi = 0; xi < brsizex; xi ++)
					{
						u16 src_pixel = vram_read_word(src_ptr + ((srx + xi) << 1));
						vram_write_word(dst_ptr + ((drx + xi) << 1), src_pixel);
					}
				}
				break;
			}
			default:
				LOGDASM("<unsupported>\n");
				break;
		}

		m_displaylist.cur_address += param_list * 4;
	}


	m_displaylist.lreq = false;
}


u32 mb86292_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	if (!BIT(m_dce, 15) || ((m_dce & 0x0f) == 0))
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const u32 clayer_addr = (m_clayer.cw * y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			u16 pixel = vram_read_word(clayer_addr + (x << 1));
			bitmap.pix(y, x) = pal555(pixel, 10, 5, 0);
		}
	}

// quick debug GFX viewer, to be moved as a debug switch
#if 0
	static int m_test_x = 128, m_test_y = 256, m_start_offs;

	if(machine().input().code_pressed(KEYCODE_Z))
		m_test_x+=4;

	if(machine().input().code_pressed(KEYCODE_X))
		m_test_x-=4;

	if(machine().input().code_pressed(KEYCODE_A))
		m_test_y++;

	if(machine().input().code_pressed(KEYCODE_S))
		m_test_y--;

	if(machine().input().code_pressed(KEYCODE_Q))
		m_start_offs+=0x2000;

	if(machine().input().code_pressed(KEYCODE_W))
		m_start_offs-=0x2000;

	if(machine().input().code_pressed_once(KEYCODE_E))
		m_start_offs+=0x1000;

	if(machine().input().code_pressed_once(KEYCODE_R))
		m_start_offs-=0x1000;

	popmessage("%d %d %04x", m_test_x, m_test_y, m_start_offs);

	bitmap.fill(0, cliprect);

	int count = m_start_offs;

	for(int y = 0; y < m_test_y; y++)
	{
		for(int x = 0; x < m_test_x; x ++)
		{
			uint16_t color = m_vram->read(count) | (m_vram->read(count + 1) << 8);

			u8 r = pal5bit((color >> 0) & 0x1f);
			u8 g = pal5bit((color >> 5) & 0x1f);
			u8 b = pal5bit((color >> 10) & 0x1f);

			if(cliprect.contains(x, y))
				bitmap.pix(y, x) = (r << 16) | (g << 8) | b;

			count +=2;
		}
	}
#endif
	return 0;
}
