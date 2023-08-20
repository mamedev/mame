// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "mb86292.h"

#include "screen.h"

//#define LOG_CONFIG  (1U << 1)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

//MB86290 / MB86290A
//DEFINE_DEVICE_TYPE(MB86291, mb86291_device, "mb86291", "Fujitsu MB86291 \"Scarlet\" Graphics Controller")
DEFINE_DEVICE_TYPE(MB86292, mb86292_device, "mb86292", "Fujitsu MB86292 \"Orchid\" Graphics Controller")

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
}

void mb86292_device::device_reset()
{
}

void mb86292_device::vregs_map(address_map &map)
{
	// 0x1fc0000 Host interface HostBase
//	map(0x00000, 0x00003) DTC DMA Transfer Count
//	map(0x00004, 0x00004) DSU DMA Set Up
//	map(0x00005, 0x00005) DRM DMA Request Mask
//	map(0x00006, 0x00006) DST DMA STatus
//	map(0x00008, 0x00008) DTS DMA Transfer Stop
//	map(0x00009, 0x00009) LTS display [List] Transfer Stop
//	map(0x00010, 0x00010) LSTA display List Transfer STAtus
//	map(0x00018, 0x00018) DRQ DMA ReQuest
//	map(0x00020, 0x00023) IST Interrupt STatus
//	map(0x00024, 0x00027) MASK Interrupt MASK
//	map(0x0002c, 0x0002c) SRST Software ReSeT
//	map(0x00040, 0x00043) LSA display List Source Address
//	map(0x00044, 0x00047) LCO display List COunt
//	map(0x00048, 0x00048) LREQ display List transfer REQuest
//	map(0x0fffc, 0x0ffff) MMR Memory I/F Mode Register

	// 0x1fd0000 Display engine DisplayBase
//	map(0x10000, 0x10001) DCM Display Control Mode
//	map(0x10002, 0x10003) DCE Display Controller Enable
//	map(0x10006, 0x10007) HTP Horizontal Total Pixels
//	map(0x10008, 0x10009) HDP Horizontal Display Period
//	map(0x1000a, 0x1000b) HDB Horizontal Display Boundary
//	map(0x1000c, 0x1000d) HSP Horizontal Sync pulse Position
//	map(0x1000e, 0x1000e) HSW Horizontal Sync pulse Width
//	map(0x1000f, 0x1000f) VSW Vertical Sync pulse Width
//	map(0x10012, 0x10013) VTR Vertical Total Rasters
//	map(0x10014, 0x10015) VSP Vertical Sync pulse Position
//	map(0x10016, 0x10017) VDP Vertical Display Period
//	map(0x10018, 0x10019) WX Window position X
//	map(0x1001a, 0x1001b) WX Window position Y
//	map(0x1001c, 0x1001d) WW Window Width
//	map(0x1001e, 0x1001f) WH Window Height
//	map(0x10020, 0x10023) CM C[onsole] layer Mode
//	map(0x10024, 0x10027) COA C layer Origin Address
//	map(0x10028, 0x1002b) CDA C layer Display Address
//	map(0x1002c, 0x1002d) CDX C layer Display position X
//	map(0x1002e, 0x1002f) CDY C layer Display position Y
//	map(0x10030, 0x10033) WM W[indow] layer Mode
//	map(0x10034, 0x10037) WOA W layer Origin Address
//	map(0x10038, 0x1003b) WDA W layer Display Address
//	map(0x10040, 0x10043) MLM M[iddle] L[eft] layer Mode
//	map(0x10044, 0x10047) MLOA0 ML Origin Address 0
//	map(0x10048, 0x1004b) MLDA0 ML Display Address 0
//	map(0x1004c, 0x1004f) MLOA1 ML Origin Address 1
//	map(0x10050, 0x10053) MLDA1 ML Display Address 1
//	map(0x10054, 0x10055) MLDX ML Display position X
//	map(0x10056, 0x10057) MLDY ML Display position Y
//	map(0x10058, 0x1005b) MRM M[iddle] R[ight] layer Mode
//	map(0x1005c, 0x1005f) MROA0 MR Origin Address 0
//	map(0x10060, 0x10063) MRDA0 MR Display Address 0
//	map(0x10064, 0x1004f) MROA1 MR Origin Address 1
//	map(0x10068, 0x10053) MRDA1 MR Display Address 1
//	map(0x1006c, 0x1006d) MRDX MR Display position X
//	map(0x1006e, 0x1006f) MRDY MR Display position Y
//	map(0x10070, 0x10073) BLM B[ase] L[eft] layer Mode
//	map(0x10074, 0x10077) BLOA0 BL Origin Address 0
//	map(0x10078, 0x1007b) BLDA0 BL Display Address 0
//	map(0x1007c, 0x1007f) BLOA1 BL Origin Address 1
//	map(0x10080, 0x10083) BLDA1 BL Display Address 1
//	map(0x10084, 0x10085) BLDX BL Display position X
//	map(0x10086, 0x10087) BLDY BL Display position Y
//	map(0x10088, 0x1008b) BRM B[ase] R[ight] layer Mode
//	map(0x1008c, 0x1008f) BROA0 BR Origin Address 0
//	map(0x10090, 0x10093) BRDA0 BR Display Address 0
//	map(0x10094, 0x10097) BROA1 BR Origin Address 1
//	map(0x10098, 0x1009b) BRDA1 BR Display Address 1
//	map(0x1009c, 0x1009d) BRDX BR Display position X
//	map(0x1009e, 0x1009f) BRDY BR Display position Y
//	map(0x100a0, 0x100a1) CUTC Cursor Transparent Control
//	map(0x100a2, 0x100a2) CPM Cursor Priority Mode
//	map(0x100a4, 0x100a7) CUOA0 CUrsor 0 Origin Address
//	map(0x100a8, 0x100a9) CUX0 CUrsor 0 X position
//	map(0x100aa, 0x100ab) CUY0 CUrsor 0 Y position
//	map(0x100ac, 0x100af) CUOA1 CUrsor 1 Origin Address
//	map(0x100b0, 0x100b1) CUX1 CUrsor 1 X position
//	map(0x100b2, 0x100b3) CUY1 CUrsor 1 Y position
//	map(0x100b4, 0x100b5) BRATIO Blend RATIO
//	map(0x100b6, 0x100b7) BMODE Blend MODE
//	map(0x100bc, 0x100bd) CTC C layer Transparent Control
//	map(0x100c0, 0x100c1) MRTC MR layer Transparent Control
//	map(0x100c2, 0x100c3) MLTC ML layer Transparent Control
//	map(0x10400, 0x107ff) CPAL C layer PALette
//	map(0x10800, 0x10bff) MBPAL M & B layer PALette
	// 0x1fd8000 Video capture CaptureBase
//	map(0x18000, 0x18003) VCM Video Capture Mode
//	map(0x18004, 0x18007) CSC Capture SCale
//	map(0x18008, 0x1800b) VCS Video Capture Status
//	map(0x18010, 0x18013) CBM video Capture Buffer Mode
//	map(0x18014, 0x18017) CBOA video Capture Buffer Origin Address
//	map(0x18018, 0x1801b) CBLA video Capture Buffer Limit Address
//	map(0x1801c, 0x1801d) CIHSTR Capture Image Horizontal STaRt
//	map(0x1801e, 0x1801f) CIVSTR Capture Image Vertical STaRt
//	map(0x18020, 0x18021) CIHEND Capture Image Horizontal END
//	map(0x18022, 0x18023) CIVEND Capture Image Vertical END
//	map(0x18028, 0x1802b) CHP Capture Horizontal Pixel
//	map(0x1802c, 0x1802f) CVP Capture Vertical Pixel
//	map(0x1c000, 0x1c003) CDCN Capture Data Count NTSC
//	map(0x1c004, 0x1c007) CDCP Capture Data Count PAL

	// 0x1fe0000 Internal texture memory TextureBase
//	map(0x20000, ...)
	// 0x1ff0000 Drawing engine DrawBase
//	map(0x30400, 0x30403) CTR ConTrol Register
//	map(0x30404, 0x30407) IFSR Input FIFO Status Register (CTR bits 14-12 alias)
//	map(0x30408, 0x3040b) IFCNT Input FIFO CouNTer (CTR bits 19-15 alias)
//	map(0x3040c, 0x3040f) SST Setup engine STatus (CTR bits 9-8 alias)
//	map(0x30410, 0x30413) DST DDA STatus (CTR bits 5-4 alias)
//	map(0x30414, 0x30417) PST Pixel engine STatus (CTR bits 1-0 alias)
//	map(0x30418, 0x3041b) EST Error STatus (CTR bits 24-22 alias)
//	map(0x30420, 0x30423) MDR0 MoDe Register 0 (miscellaneous)
//	map(0x30424, 0x30427) MDR1 MoDe Register 1 (line)
//	map(0x30428, 0x3042b) MDR2 MoDe Register 2 (polygon)
//	map(0x3042c, 0x3042f) MDR3 MoDe Register 3 (texture)
//	map(0x30430, 0x30433) MDR4 MoDe Register 4 (BitBLT)
//	map(0x30440, 0x30443) FBR Frame Buffer Register base address
//	map(0x30444, 0x30447) XRES X RESoultion
//	map(0x30448, 0x3044b) ZBR Z-Buffer Register base address
//	map(0x3044c, 0x3044f) TBR Texture memory Base address
//	map(0x30450, 0x30453) PFBR 2d Polygon Flag Buffer base address
//	map(0x30454, 0x30457) CXMIN Clip X MINimum
//	map(0x30458, 0x3045b) CXMAX Clip X MAXimum
//	map(0x3045c, 0x3045f) CYMIN Clip Y MINimum
//	map(0x30460, 0x30463) CYMAX Clip Y MAXimum
//	map(0x30464, 0x30467) TXS TeXture Size
//	map(0x30468, 0x3046b) TIle Size
//	map(0x3046c, 0x3046f) TOA Texture buffer Offset Address
//	map(0x30480, 0x30483) FC Foreground Color
//	map(0x30484, 0x30487) BC Background Color
//	map(0x30488, 0x3048b) ALF ALpha Factor
//	map(0x3048c, 0x3048f) BLP Broken Line Pattern
//	map(0x303e0?, 0x303e3?) BLPO Broken Line Pattern Offset <- assume doc mistake, 0x490 seems more realistic
//	map(0x30494, 0x30497) TBC Texture Border Color
//	Other stuff in the area are apparently r/o copies of the drawing engine internals.
	// 0x1ff8000 Geometry engine GeometryBase
//	map(0x38000, ...)
}

u32 mb86292_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
// mariojjl / mmaruchan has two RGB555 charsets at $36d80 & $150600 (pitch=128)
// pingu has one at $96000
// all Medalusion games also loads up a display list at $780000, which references above and
// (presumably) do clipped ROP transfers to whatever the framebuffer VRAM is.
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
