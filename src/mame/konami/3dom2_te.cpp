// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    3DO M2 BDA Triangle Engine

***************************************************************************/

#include "emu.h"
#include "video/3dom2_te.h"

#include "machine/3dom2.h"

#include <cmath>

/*
    TODO:

    VTX_FLAGS do not get passed to the TMAPPER: Should use master control
    bits to disable shading and texturing.

    - Evil Night attract mode sky is missing. Why?
    - What is 1/w when 0?
    - What do we do about RL flat-topped triangles?
    - Heat of 11 color check has dodgy pixels <- HAS IT?
    - Polystar blending is incorrect (intro)
*/

#define TEST_TIMING     1

#if TEST_TIMING
enum
{
	STAT_TRIANGLES_PROCESSED,
	STAT_TEXEL_LOADS,
	STAT_TEXEL_READS,
	STAT_PIXELS_PROCESSED,
	STAT_PIXEL_LOADS,
	STAT_PIXEL_STORES,
	STAT_TEXEL_BYTES,
	STAT_ZBUFFER_LOADS,
	STAT_ZBUFFER_STORES,
};
static uint32_t g_statistics[16];
#endif

static bool g_debug = false; // TODO

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

// Device type definition
DEFINE_DEVICE_TYPE(M2_TE, m2_te_device, "m2te", "BDA Triangle Engine")



//static const uint32_t fixed_bits  = 23;

static const uint32_t xy_bits       = 11;
static const uint32_t color_bits    = 8;
static const uint32_t text_bits[]   = { 10, 7, 4, 1 };
static const int32_t depth_bits[]   = { 0, -3, -6, -9 };

// For right-aligning
static const uint32_t xy_rshift     = 12;
static const uint32_t color_rshift  = 4;


// 11.0 format
static const uint32_t xy_one        = 1;


/***************************************************************************
 REGISTER DEFINITIONS
 ***************************************************************************/

// TODO: SUFFIX bits with _BIT?
// Move to another file?

//-------------------------------------------------
//  General Control
//-------------------------------------------------

// Triangle Engine Master Mode
#define TEMASTER_MODE_RESET             0x00000001
#define TEMASTER_MODE_DTEXT             0x00000002
#define TEMASTER_MODE_DSHADE            0x00000004
#define TEMASTER_MODE_DBLEND            0x00000008
#define TEMASTER_MODE_DZBUF             0x00000010
#define TEMASTER_MODE_DDITH             0x00000020

// Triangle Engine Immediate Control
#define TEICNTL_INT                     0x00000001
#define TEICNTL_STEP                    0x00000002
#define TEICNTL_STPL                    0x00000004
#define TEICNTL_STPI                    0x00000008
#define TEICNTL_RSTRT                   0x00000010
#define TEICNTL_STRT                    0x00000020

#define TEDCNTL_TLD                     0x00000001
#define TEDCNTL_JA                      0x00000002
#define TEDCNTL_JR                      0x00000004
#define TEDCNTL_INT                     0x00000008
#define TEDCNTL_PSE                     0x00000010
#define TEDCNTL_SYNC                    0x00000020

#define INTSTAT_DEFERRED_INSTR          0x00000100
#define INTSTAT_IMMEDIATE_INSTR         0x00000200
#define INTSTAT_LIST_END                0x00000400
#define INTSTAT_WINDOW_CLIP             0x00000800
#define INTSTAT_SPECIAL_INSTR           0x00001000
#define INTSTAT_UNIMPLEMENTED_INSTR     0x00002000
#define INTSTAT_SUPERVISOR              0x00004000
#define INTSTAT_ANY_RENDER              0x00008000
#define INTSTAT_Z_FUNC                  0x00010000
#define INTSTAT_ALU_STATUS              0x00020000
#define INTSTAT_FB_CLIP                 0x00040000
#define INTSTAT_IMMEDIATE               0x00080000

// IWP
// IRP
// Interrupt Enable
// Interrupt Status
// Vertex Control



//-------------------------------------------------
//  Setup Engine
//-------------------------------------------------

// Vertex State
#define VERTEXSTATE_TSORT_MASK              0x00000007
#define VERTEXSTATE_TSORT_OMN               0x00000001
#define VERTEXSTATE_TSORT_MNO               0x00000002
#define VERTEXSTATE_TSORT_ONM               0x00000003
#define VERTEXSTATE_TSORT_NOM               0x00000004
#define VERTEXSTATE_TSORT_MON               0x00000005
#define VERTEXSTATE_TSORT_NMO               0x00000006

#define VERTEXSTATE_VCNT_SHIFT              3
#define VERTEXSTATE_VCNT_MASK               0x00000018


//-------------------------------------------------
//  Edge Walker
//-------------------------------------------------

// Edge and Span Walker Control
#define ESCNTL_DSPOFF                       0x00000001
#define ESCNTL_DUSCAN                       0x00000002
#define ESCNTL_PERSPECTIVEOFF               0x00000004


//-------------------------------------------------
//  Texture Mapper
//-------------------------------------------------

// Texture Mapper Master Control (0x00046400)
#define TXTCNTL_MMDMA_TRAM_ON               0x00000004
#define TXTCNTL_MMDMA_PIP_ON                0x00000008
#define TXTCNTL_SNOOP_ON                    0x00000020

// Texture Load Control (0x00046404)
#define TXTLDCNTL_SRCBITOFFS                0x00000007
#define TXTLDCNTL_LDMODE_MASK               0x00000300
#define TXTLDCNTL_LDMODE_TEXLOAD            0x00000000
#define TXTLDCNTL_LDMODE_MMDMA              0x00000100
#define TXTLDCNTL_LDMODE_PIPLOAD            0x00000200
#define TXTLDCNTL_LDMODE_RESERVED           0x00000300
#define TXTLDCNTL_COMPRESSED                0x00000400

// Address Control (0x00046408)
#define TXTADDRCNTL_LODMAX_MASK             0x0000000f
#define TXTADDRCNTL_FILTERSEL_MASK          0x00000003
#define TXTADDRCNTL_FILTERSEL_POINT         0x00000000
#define TXTADDRCNTL_FILTERSEL_LINEAR        0x00000001
#define TXTADDRCNTL_FILTERSEL_BILINEAR      0x00000002
#define TXTADDRCNTL_FILTERSEL_QUASITRI      0x00000003
#define TXTADDRCNTL_R12FILTERSEL_SHIFT      4
#define TXTADDRCNTL_R3FILTERSEL_SHIFT       7
#define TXTADDRCNTL_R45FILTERSEL_SHIFT      10
#define TXTADDRCNTL_LOOKUP_EN               0x00002000

// PIP Control (0x0004640C)
#define TXTPIPCNTL_INDEX_OFFSET             0x000000ff
#define TXTPIPCNTL_COLORSEL_MASK            0x00000700
#define TXTPIPCNTL_COLORSEL_SHIFT           8
#define TXTPIPCNTL_ALPHASEL_MASK            0x00003800
#define TXTPIPCNTL_ALPHASEL_SHIFT           11
#define TXTPIPCNTL_SSBSEL_MASK              0x0001c000
#define TXTPIPCNTL_SSBSEL_SHIFT             14

#define TXTPIPCNTL_SEL_CONSTANT             0
#define TXTPIPCNTL_SEL_TRAM                 1
#define TXTPIPCNTL_SEL_PIP                  2

// Texture Application Control (0x00046410)
#define TXTTABCNTL_C_ASEL_MASK              0x00000007
#define TXTTABCNTL_C_ASEL_SHIFT             0
#define TXTTABCNTL_C_BSEL_MASK              0x00000038
#define TXTTABCNTL_C_BSEL_SHIFT             3
#define TXTTABCNTL_C_TSEL_MASK              0x000001c0
#define TXTTABCNTL_C_TSEL_SHIFT             6

#define TXTTABCNTL_C_ABTSEL_AITER           0
#define TXTTABCNTL_C_ABTSEL_CITER           1
#define TXTTABCNTL_C_ABTSEL_AT              2
#define TXTTABCNTL_C_ABTSEL_CT              3
#define TXTTABCNTL_C_ABTSEL_ACONST          4
#define TXTTABCNTL_C_ABTSEL_CCONST          5

#define TXTTABCNTL_C_OSEL_MASK              0x00000600
#define TXTTABCNTL_C_OSEL_SHIFT             9

#define TXTTABCNTL_CO_SEL_CITER             0
#define TXTTABCNTL_CO_SEL_CT                1
#define TXTTABCNTL_CO_SEL_BLEND             2
#define TXTTABCNTL_CO_SEL_RESERVED          3

#define TXTTABCNTL_A_ASEL_MASK              0x00001800
#define TXTTABCNTL_A_ASEL_SHIFT             11
#define TXTTABCNTL_A_BSEL_MASK              0x00006000
#define TXTTABCNTL_A_BSEL_SHIFT             13

#define TXTTABCNTL_A_ABSEL_AITER            0
#define TXTTABCNTL_A_ABSEL_AT               1
#define TXTTABCNTL_A_ABSEL_ACONST           2

#define TXTTABCNTL_A_OSEL_MASK              0x00018000
#define TXTTABCNTL_A_OSEL_SHIFT             15

#define TXTTABCNTL_AO_SEL_AITER             0
#define TXTTABCNTL_AO_SEL_AT                1
#define TXTTABCNTL_AO_SEL_BLEND             2
#define TXTTABCNTL_AO_SEL_RESERVED          3

#define TXTTABCNTL_BLENDOP_MASK             0x00020000
#define TXTTABCNTL_BLENDOP_SHIFT            17
#define TXTTABCNTL_BLENDOP_LERP             0
#define TXTTABCNTL_BLENDOP_MULT             1

// TAB Constants
#define TXTTABCONST_BLUE                    0x000000ff
#define TXTTABCONST_BLUE_SHIFT              0
#define TXTTABCONST_GREEN                   0x0000ff00
#define TXTTABCONST_GREEN_SHIFT             8
#define TXTTABCONST_RED                     0x00ff0000
#define TXTTABCONST_RED_SHIFT               16
#define TXTTABCONST_ALPHA                   0x7f000000
#define TXTTABCONST_ALPHA_SHIFT             24
#define TXTTABCONST_SSB                     0x80000000

// Texture Loader Destination Base (0x00046414)
#define TXTLDDSTBASE_ADDR                   0x00003ffc

// Texture Lod Base 0 (0x00046414)
// Texture Lod Base 1 (0x00046418)
// Texture Lod Base 2 (0x0004641C)
// Texture Lod Base 3 (0x00046420)
#define TXTLODBASE_MASK                     0x00003ffc

// Texture Loader Source Base (0x00046424)
#define TXTLDSRCBASE_ADDR                   0x00003fff

// Texture Loader Counts (0x00046428)
#define TXTLDBYTECNT_COUNT                  0x0fffffff
#define TXTLDROWCNT_COUNT                   0x0fffffff
#define TXTLDTEXCNT_COUNT                   0x0fffffff

// Texture Loader Width (0x00046428)
#define TxTLDWIDTH_SRCROW                   0x0000ffff
#define TxTLDWIDTH_DSTROW_SHIFT             16
#define TxTLDWIDTH_DSTROW                   0xffff0000

// Texture Size (0x0004642C)
#define TXTUVMAX_VMAX_MASK                  0x000003ff
#define TXTUVMAX_VMAX_SHIFT                 0
#define TXTUVMAX_UMAX_MASK                  0x03ff0000
#define TXTUVMAX_UMAX_SHIFT                 16

// Texture Mask (0x00046430)
#define TXTUVMASK_VMASK_MASK                0x000003ff
#define TXTUVMASK_VMASK_SHIFT               0
#define TXTUVMASK_UMASK_MASK                0x03ff0000
#define TXTUVMASK_UMASK_SHIFT               16

// TRAM Format (0x0004643C)
// TODO: Expansion formats
#define TXTEXPFORM_CDEPTH_MASK              0x0000000f
#define TXTEXPFORM_CDEPTH_SHIFT             0
#define TXTEXPFORM_IDEPTH_MASK              0x0000000f
#define TXTEXPFORM_IDEPTH_SHIFT             0
#define TXTEXPFORM_ADEPTH_MASK              0x000000f0
#define TXTEXPFORM_ADEPTH_SHIFT             4
#define TXTEXPFORM_TRANSPARENT              0x00000100
#define TXTEXPFORM_SSBON                    0x00000200
#define TXTEXPFORM_COLORON                  0x00000400
#define TXTEXPFORM_INDEXON                  0x00000400
#define TXTEXPFORM_ALPHAON                  0x00000800
#define TXTEXPFORM_LITERAL                  0x00001000


// Format Registers


//-------------------------------------------------
//  Destination Blender
//-------------------------------------------------

// Snoop (0x0048000)
#define DBSNOOP_DESTWRSNOOP             0x00000001
#define DBSNOOP_SRCRDSNOOP              0x00000002
#define DBSNOOP_ZWRSNOOP                0x00000004
#define DBSNOOP_ZRDSNOOP                0x00000008

// Supervisor General Control (0x0048004)
#define DBSUPERGENCTL_DESTOUTEN         0x00000001
#define DBSUPERGENCTL_DESTWR16BEN       0x00000002
#define DBSUPERGENCTL_ZWR16BEN          0x00000004

// User General Control (0x0048008)
#define DBUSERGENCTL_DESTOUT_MASK       0x0000000f
#define DBUSERGENCTL_DITHEREN           0x00000010
#define DBUSERGENCTL_SRCINEN            0x00000020
#define DBUSERGENCTL_BLENDEN            0x00000040
#define DBUSERGENCTL_WCLIPOUTEN         0x00000080
#define DBUSERGENCTL_WCLIPINEN          0x00000100
#define DBUSERGENCTL_ZOUTEN             0x00000200
#define DBUSERGENCTL_ZBUFEN             0x00000400

// Discard Control (0x004800C)
#define DBDISCARDCTL_ADISEN             0x00000001
#define DBDISCARDCTL_RGBDISEN           0x00000002
#define DBDISCARDCTL_SSBDISEN           0x00000004
#define DBDISCARDCTL_ZCLIPDISEN         0x00000008

// Status (0x0048010)
#define DBSTATUS_ANYREND                0x00000001
#define DBSTATUS_ZFUNC_GT               0x00000002
#define DBSTATUS_ZFUNC_EQ               0x00000004
#define DBSTATUS_ZFUNC_LT               0x00000008
#define DBSTATUS_ALUSTAT_BLUE_GT        0x00000010
#define DBSTATUS_ALUSTAT_BLUE_EQ        0x00000020
#define DBSTATUS_ALUSTAT_BLUE_LT        0x00000040
#define DBSTATUS_ALUSTAT_GREEN_GT       0x00000080
#define DBSTATUS_ALUSTAT_GREEN_EQ       0x00000100
#define DBSTATUS_ALUSTAT_GREEN_LT       0x00000200
#define DBSTATUS_ALUSTAT_RED_GT         0x00000400
#define DBSTATUS_ALUSTAT_RED_EQ         0x00000800
#define DBSTATUS_ALUSTAT_RED_LT         0x00001000
#define DBSTATUS_ZCLIP                  0x00002000
#define DBSTATUS_WINCLIP                0x00004000
#define DBSTATUS_FBCLIP                 0x00008000

// Interrupt Control (0x00048014)
#define DBINTCNTL_ZFUNCSTATINTEN_MASK   0x00000003
#define DBINTCNTL_ZFUNCSTATINTEN_MASK   0x00000003

// Framebuffer XY Clip Control (0x00048018)
#define DBFBCLIP_YFBCLIP_MASK           0x000007ff
#define DBFBCLIP_YFBCLIP_SHIFT          0
#define DBFBCLIP_XFBCLIP_MASK           0x07ff0000
#define DBFBCLIP_XFBCLIP_SHIFT          16

// Window X Clip Control (0x0004801C)
#define DBFBXWINCLIP_XMAX_MASK          0x000007ff
#define DBFBXWINCLIP_XMAX_SHIFT         0
#define DBFBXWINCLIP_XMIN_MASK          0x07ff0000
#define DBFBXWINCLIP_XMIN_SHIFT         16

// Window Y Clip Control (0x00048020)
#define DBFBYWINCLIP_YMAX_MASK          0x000007ff
#define DBFBYWINCLIP_YMAX_SHIFT         0
#define DBFBYWINCLIP_YMIN_MASK          0x07ff0000
#define DBFBYWINCLIP_YMIN_SHIFT         16

// Destination Write Control (0x0048024)
#define DBDESTCNTL_32BPP                0x00000001

// Destination Write Base Address (0x0048028)

// Destination X Stride (0x004802C)
#define DBDEST_XSTRIDE                  0x000007ff

// Source Read Control (0x00048030)
#define DBSRCCNTL_32BPP                 0x00000001
#define DBSRCCNTL_MSBREP                0x00000002

// Source Read Base Address (0x00048034)

// Source X Stride (0x00048038)
#define DBSRCXSTRIDE                    0x000007ff

// Source XY Offset (0x0004803C)
#define DBSRCOFFS_YOFFS_MASK            0x00000fff
#define DBSRCOFFS_YOFFS_SHIFT           0
#define DBSRCOFFS_XOFFS_MASK            0x0fff0000
#define DBSRCOFFS_XOFFS_SHIFT           16

// Z Buffer Control (0x00048040)
#define DBZCNTL_ZFUNCCNTL_MASK          0x0000003f
#define DBZCNTL_ZPIXOUT_LT              0x00000001
#define DBZCNTL_ZBUFOUT_LT              0x00000002
#define DBZCNTL_ZPIXOUT_EQ              0x00000004
#define DBZCNTL_ZBUFOUT_EQ              0x00000008
#define DBZCNTL_ZPIXOUT_GT              0x00000010
#define DBZCNTL_ZBUFOUT_GT              0x00000020

// Z Buffer Base Address (0x00048044)
#define DBZBASEADDR_MASK                0x00ffffff

// Z Buffer XY Offset (0x00048048)
#define DBZOFFS_YOFFS_MASK              0x00000fff
#define DBZOFFS_YOFFS_SHIFT             0
#define DBZOFFS_XOFFS_MASK              0x0fff0000
#define DBZOFFS_XOFFS_SHIFT             16

// Z Buffer Clip (0x0004804C)
#define DBZCLIP_YCLIP_MASK              0x000007ff
#define DBZCLIP_YCLIP_SHIFT             0
#define DBZCLIP_XCLIP_MASK              0x07ff0000
#define DBZCLIP_XCLIP_SHIFT             16

// SSB/DSB Control (0x00048050)
#define DBSSBDSBCNTL_DSBSELECT_MASK     0x00000007
#define DBSSBDSBCNTL_DSBSELECT_SHIFT    0
#define DBSSBDSBCNTL_DSBSELECT_SSB      0
#define DBSSBDSBCNTL_DSBSELECT_CONSTANT 1
#define DBSSBDSBCNTL_DSBSELECT_SRC      2

#define DBSSBDSBCNTL_DSBCONST           0x00000004

// RGB constants (0x00048054)
#define DBCONSTIN_B_MASK                0x000000ff
#define DBCONSTIN_B_SHIFT               0
#define DBCONSTIN_G_MASK                0x0000ff00
#define DBCONSTIN_G_SHIFT               8
#define DBCONSTIN_R_MASK                0x00ff0000
#define DBCONSTIN_R_SHIFT               16

// Texture Multiplication Control (0x00048058)
#define DBTXTMULTCNTL_TXTRJUST                  0x00000001
#define DBTXTMULTCNTL_TXTCOEFCMP                0x00000002

#define DBTXTMULTCNTL_TXTCONSTCNTL_MASK         0x0000000c
#define DBTXTMULTCNTL_TXTCONSTCNTL_SHIFT        2
#define DBTXTMULTCNTL_TXTCONSTCNTL_TEXSSB       0
#define DBTXTMULTCNTL_TXTCONSTCNTL_SRCDSB       1

#define DBTXTMULTCNTL_COEFSEL_MASK              0x00000030
#define DBTXTMULTCNTL_COEFSEL_SHIFT             4
#define DBTXTMULTCNTL_COEFSEL_ATI               0
#define DBTXTMULTCNTL_COEFSEL_ASRC              1
#define DBTXTMULTCNTL_COEFSEL_CONSTANT          2
#define DBTXTMULTCNTL_COEFSEL_CSRC              3

#define DBTXTMULTCNTL_INSEL_MASK                0x000000c0
#define DBTXTMULTCNTL_INSEL_SHIFT               6
#define DBTXTMULTCNTL_INSEL_CTI                 0
#define DBTXTMULTCNTL_INSEL_CONSTANT            1
#define DBTXTMULTCNTL_INSEL_COMPSRC             2
#define DBTXTMULTCNTL_INSEL_ATI                 3

// Source Multiplication Control (0x00048058)
#define DBSRCMULTCNTL_SRCRJUST                  0x00000001
#define DBSRCMULTCNTL_SRCCOEFCMP                0x00000002

#define DBSRCMULTCNTL_SRCCONSTCNTL_MASK         0x0000000c
#define DBSRCMULTCNTL_SRCCONSTCNTL_SHIFT        2
#define DBSRCMULTCNTL_SRCCONSTCNTL_TEXSSB       0
#define DBSRCMULTCNTL_SRCCONSTCNTL_SRCDSB       1

#define DBSRCMULTCNTL_COEFSEL_MASK              0x00000030
#define DBSRCMULTCNTL_COEFSEL_SHIFT             4
#define DBSRCMULTCNTL_COEFSEL_ATI               0
#define DBSRCMULTCNTL_COEFSEL_ASRC              1
#define DBSRCMULTCNTL_COEFSEL_CONSTANT          2
#define DBSRCMULTCNTL_COEFSEL_CTI               3

#define DBSRCMULTCNTL_INSEL_MASK                0x000000c0
#define DBSRCMULTCNTL_INSEL_SHIFT               6
#define DBSRCMULTCNTL_INSEL_SRC                 0
#define DBSRCMULTCNTL_INSEL_CONSTANT            1
#define DBSRCMULTCNTL_INSEL_COMPCTI             2
#define DBSRCMULTCNTL_INSEL_TEXALPHA            3

// ALU Control (0x00048070) TODO
#define DBALUCNTL_FINALDIVIDE_MASK              0x00000007
#define DBALUCNTL_FINALDIVIDE_SHIFT             0

#define DBALUCNTL_ALUOP_MASK                    0x000000f8
#define DBALUCNTL_ALUOP_SHIFT                   5

// Source Alpha Control (0x00048074)
#define DBDSTACNTL_ADESTSEL_MASK                0x00000003
#define DBDSTACNTL_ADESTSEL_SHIFT               0
#define DBDSTACNTL_ADESTCONSTCNTL_MASK          0x0000000c
#define DBDSTACNTL_ADESTCONSTCNTL_SHIFT         2

#define DBDSTALPHACONST_CONST1_MASK             0x000000ff
#define DBDSTALPHACONST_CONST1_SHIFT            0
#define DBDSTALPHACONST_CONST0_MASK             0x00ff0000
#define DBDSTALPHACONST_CONST0_SHIFT            16

#define DBSSBDSBCNTL_DSBSEL_MASK                0x00000003
#define DBSSBDSBCNTL_DSBSEL_SHIFT               0
#define DBSSBDSBCNTL_DSBCONST_MASK              0x00000004
#define DBSSBDSBCNTL_DSBCONST_SHIFT             2



//**************************************************************************
//  SUPPORT FUNCTIONS
//**************************************************************************

static inline float int_trunc(float x)
{
	return (float)(int)x;
}

static inline uint8_t extract_exp(float x)
{
	uint32_t u32 = *reinterpret_cast<uint32_t *>(&x);
	return (u32 >> 23) & 0xff;
}

// Convert a regular float to sign-magnitude fixed point
static int32_t ieee754_to_tefix(float f, int32_t bits)
{
	float format = powf(2.0f, bits);
	float tmp1 = format * 2.0f;

	// Check to see if number is within range
	float tmp2 = fmodf(fabsf(f), tmp1);
	float tmp3 = (tmp2 >= format) ? tmp2 : tmp2 + format;

	// Extract the result
	uint32_t res = *reinterpret_cast<uint32_t *>(&tmp3) & 0x007fffff;

	// What is this?
	if (tmp2 >= format || tmp3 == tmp1)
		res |= 0x00800000;

	// Negate
	if (f < 0.0f)
		res = -res;

	return res;
}

static void write_te_reg(uint32_t &reg, uint32_t data, m2_te_device::te_reg_wmode mode)
{
	switch (mode)
	{
		case m2_te_device::REG_WRITE:
			reg = data;
			break;
		case m2_te_device::REG_SET:
			reg |= data;
			break;
		case m2_te_device::REG_CLEAR:
			reg &= ~data;
			break;
		default:
			throw emu_fatalerror("write_te_reg: Bad register write mode");
	}
}

#if 0
static const char *get_reg_name(uint32_t unit, uint32_t reg)
{
	static const char *gc_regs[] =
	{
		"TEMasterMode",
		"Reserved",
		"TEICntlData",
		"TEICntl",
		"TEDCntlData",
		"TEDCntl",
		"IWP",
		"IRP",
		"IntEn",
		"IntStat",
		"Vertex Control",
	};

	static const char *db_regs[] =
	{
		"Snoop",
		"SuperGenControl",
		"usergen_ctrl",
		"Discard Control",
		"Status",
		"Int Cntl",
		"FBClip",
		"XWinClip",
		"YWinClip",
		"DestCntl",
		"DestBaseAddr",
		"DestXStride",
		"SrcCntl",
		"SrcBaseAddr",
		"SrcXStride",
		"SrcOffset",
		"ZCntl",
		"ZBaseAddr",
		"ZOffset",
		"ZClip",
		"SSBDSBCntl",
		"ConstIn",
		"DBTXTMULTCNTL",
		"TxtCoefConst0",
		"TxtCoefConst1",
		"src_mult_cntl",
		"SrcCoefConst0",
		"SrcCoefConst1",
		"src_mult_cntl",
		"SrcCoefconst0",
		"SrcCoefconst1",
		"ALUCntl",
		"SrcAlphaCntl",
		"DestAlphaCntl",
		"DestAlphaConst",
		"DitherMatA",
		"DitherMatB",
	};

	static const char *es_regs[] =
	{
		"ESCntl",
		"ESCapAddr",
		"ESCapData",
	};

	static char buffer[128];

	switch (unit)
	{
		case 0:
		{
			if (reg < sizeof(gc_regs))
			{
				sprintf(buffer, "GC:%s", gc_regs[reg]);
				return buffer;
			}
			break;
		}
		case 1:
		{
			sprintf(buffer, "SE:????");
			return buffer;
		}
		case 2:
		{
			if (reg < sizeof(es_regs))
			{
				sprintf(buffer, "ES:%s", es_regs[reg]);
				return buffer;
			}
			break;
		}
		case 3:
		{
//          if (reg < sizeof(tm_regs))
			{
				sprintf(buffer, "TM:????");
				return buffer;
			}
			break;
		}
		case 4:
		{
			if (reg < sizeof(db_regs))
			{
				sprintf(buffer, "DB:%s", db_regs[reg]);
				return buffer;
			}
			break;
		}
	}

	return "????";
}
#endif
//**************************************************************************
//  TRIANGLE ENGINE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m2_te_device - constructor
//-------------------------------------------------

m2_te_device::m2_te_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, M2_TE, tag, owner, clock),
	m_general_int_handler(*this),
	m_dfinstr_int_handler(*this),
	m_iminstr_int_handler(*this),
	m_listend_int_handler(*this),
	m_winclip_int_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m2_te_device::device_start()
{
	// Find our parent
	m_bda = downcast<m2_bda_device *>(owner());

	// Resolve callbacks
	m_general_int_handler.resolve_safe();
	m_dfinstr_int_handler.resolve_safe();
	m_iminstr_int_handler.resolve_safe();
	m_listend_int_handler.resolve_safe();
	m_winclip_int_handler.resolve_safe();

	// Allocate texture RAM
	m_tram = std::make_unique<uint32_t[]>(TEXTURE_RAM_WORDS);

	// Allocate PIP RAM
	m_pipram = std::make_unique<uint32_t[]>(PIP_RAM_WORDS);

	// Clear our state; TODO: Proper reset values
	memset(&m_gc, 0, sizeof(m_gc));
	memset(&m_se, 0, sizeof(m_se));
	memset(&m_es, 0, sizeof(m_es));
	memset(&m_tm, 0, sizeof(m_tm));
	memset(&m_db, 0, sizeof(m_db));

	// Register state for saving
	save_pointer(NAME(m_tram), TEXTURE_RAM_WORDS);
	save_pointer(NAME(m_pipram), PIP_RAM_WORDS);

	save_item(NAME(m_gc.m_regs));
	save_item(NAME(m_se.m_regs));
	save_item(NAME(m_es.m_regs));
	save_item(NAME(m_tm.m_regs));
	save_item(NAME(m_db.m_regs));

	// Allocate timers
	m_done_timer = timer_alloc(FUNC(m2_te_device::command_done), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m2_te_device::device_reset()
{
	m_state = TE_STOPPED;

	// TODO
}


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

uint32_t m2_te_device::read(offs_t offset)
{
	uint32_t unit = (offset >> 11) & 7;
	uint32_t reg = offset & 0x1ff;

	logerror("%s: TE R[%x]\n", machine().describe_context(), 0x00040000 + (offset << 2));

	switch (unit)
	{
		case 0:
		{
			if (reg < sizeof(m_gc) / 4)
				return m_gc.m_regs[reg];

			break;
		}
		case 1:
		{
			if (reg < sizeof(m_se) / 4)
				return m_se.m_regs[reg];

			break;
		}
		case 2:
		{
			if (reg < sizeof(m_es) / 4)
				return m_es.m_regs[reg];

			break;
		}
		case 3:
		{
			if (reg < 0x400/4)
			{
				return m_pipram[reg];
			}
			else if ((reg - 0x400) < sizeof(m_tm) / 4)
			{
				return m_tm.m_regs[reg];
			}

			break;
		}
		case 4:
		{
			if (reg < sizeof(m_db) / 4)
				return m_db.m_regs[reg];

			break;
		}
	}

	logerror("%s: Unknown TE register read from %d:[%x]", machine().describe_context(), unit, reg);
	return 0;
}

void m2_te_device::write(offs_t offset, uint32_t data)
{
	uint32_t unit = (offset >> 11) & 7;
	uint32_t reg = offset & 0x1ff;
	te_reg_wmode wmode = static_cast<te_reg_wmode>((offset >> 9) & 3);

//  logerror("%s: TE W[%.8x] (%s) %.8x\n", machine().describe_context(), 0x00040000 + (offset << 2), get_reg_name(unit, reg), data);

	switch (unit)
	{
		case 0:
		{
			switch (reg)
			{
				case 0:
				{
					master_mode_w(data, wmode);
					return;
				}
				case 0x0c/4:
				{
					teicntl_w(data, wmode);
					return;
				}
				case 0x14/4:
				{
					tedcntl_w(data, wmode);
					return;
				}
				case 0x24/4:
				{
					write_te_reg(m_gc.int_status, data, wmode);
					update_interrupts();
					return;
				}
				default:
				{
					if (reg < sizeof(m_gc) / 4)
					{
						write_te_reg(m_gc.m_regs[reg], data, wmode);
						return;
					}
				}
			}
			break;
		}
		case 1:
		{
			if (reg < sizeof(m_se) / 4)
			{
				write_te_reg(m_se.m_regs[reg], data, wmode);
				return;
			}
			break;
		}
		case 2:
		{
			if (reg < sizeof(m_es) / 4)
			{
				write_te_reg(m_es.m_regs[reg], data, wmode);
				return;
			}
			break;
		}
		case 3:
		{
			if (reg < 0x400 / 4)
			{
				m_pipram[reg] = data;
				return;
			}
			else if ((reg - 0x400 / 4) < sizeof(m_tm) / 4)
			{
				write_te_reg(m_tm.m_regs[reg - 0x400 / 4], data, wmode);
				return;
			}
			break;
		}
		case 4:
		{
			if (reg < sizeof(m_db) / 4)
			{
				write_te_reg(m_db.m_regs[reg], data, wmode);
				return;
			}
			break;
		}
	}

	logerror("%s: Unknown TE register write to %d:[%x] with %x\n", machine().describe_context(), unit, reg, data);
}



/***************************************************************************
    PRIVATE FUNCTIONS
***************************************************************************/

/***************************************************************************
    INTERRUPTS
***************************************************************************/

//-------------------------------------------------
//  set_interrupt -
//-------------------------------------------------

void m2_te_device::set_interrupt(uint32_t mask)
{
	m_gc.int_status |= mask;
	update_interrupts();
}


//-------------------------------------------------
//  update_interrupts -
//-------------------------------------------------

void m2_te_device::update_interrupts()
{
	bool gen_int = ((m_gc.int_status & m_gc.int_enable) & 0x0000ff00) != 0;

	// TODO: ALU status and FB clip are controlled by dest blender
	m_general_int_handler(gen_int);

	// From DBL (Page 265)
	// FBClip
	// WinClip
	// ZClip
	// ALUStat
	// ZFuncStat

	// Page 43

	// PAGE 26
	m_dfinstr_int_handler((m_gc.int_status >> 8) & 1);
	m_iminstr_int_handler((m_gc.int_status >> 9) & 1);
	m_listend_int_handler((m_gc.int_status >> 10) & 1);
	m_winclip_int_handler((m_gc.int_status >> 11) & 1);
}



/***************************************************************************
    SPECIAL REGISTER WRITES
***************************************************************************/

//-------------------------------------------------
//  master_mode_w -
//-------------------------------------------------

void m2_te_device::master_mode_w(uint32_t data, te_reg_wmode wmode)
{
	write_te_reg(m_gc.te_master_mode, data, wmode);

	if (m_gc.te_master_mode & TEMASTER_MODE_RESET)
		device_reset();
}


//-------------------------------------------------
//  teicntl_w -
//-------------------------------------------------

void m2_te_device::teicntl_w(uint32_t data, te_reg_wmode wmode)
{
	uint32_t newreg = 0;

	write_te_reg(newreg, data, wmode);
	write_te_reg(m_gc.te_master_mode, data, wmode);

	if (newreg & TEICNTL_STRT)
	{
		m_gc.irp = m_gc.teicntl_data;
		m_state = TE_RUNNING;
		execute();
	}
	else if (newreg & TEICNTL_RSTRT)
	{
		m_state = TE_RUNNING;
		execute();
	}
}


//-------------------------------------------------
//  tedcntl_w -
//-------------------------------------------------

void m2_te_device::tedcntl_w(uint32_t data, te_reg_wmode wmode)
{
	write_te_reg(m_gc.tedcntl, data, wmode);

	if (m_gc.tedcntl & TEDCNTL_SYNC)
	{
		// TODO
	}
	else if (m_gc.tedcntl & TEDCNTL_PSE)
	{
		m_state = TE_PAUSED;
	}
	else if (m_gc.tedcntl & TEDCNTL_INT)
	{
		set_interrupt(INTSTAT_IMMEDIATE);
	}
	else if (m_gc.tedcntl & TEDCNTL_JR)
	{
		m_gc.irp += m_gc.tedcntl_data;
	}
	else if (m_gc.tedcntl & TEDCNTL_JA)
	{
		m_gc.irp = m_gc.tedcntl_data;
	}
	else if (m_gc.tedcntl & TEDCNTL_TLD)
	{
		load_texture();
	}
}



/***************************************************************************
    VERTEX PROCESSING
***************************************************************************/

//-------------------------------------------------
//  add_vertex -
//-------------------------------------------------

void m2_te_device::add_vertex(const se_vtx &vtx, uint32_t flags)
{
	uint32_t tsort;
	uint32_t vcnt;

	if (flags & VTX_FLAG_NEW)
	{
		// Reset state for a new triangle
		tsort = 0;
		vcnt = 0;
	}
	else
	{
		// Continue current triangle
		tsort = m_se.vertex_state & VERTEXSTATE_TSORT_MASK;
		vcnt = (m_se.vertex_state & VERTEXSTATE_VCNT_MASK) >> VERTEXSTATE_VCNT_SHIFT;
	}

	if (vcnt < 3)
	{
		// Add vertex to incomplete triangle
		m_se.vertices[vcnt++] = vtx;

		// Are we now complete?
		if (vcnt == 3)
			tsort = VERTEXSTATE_TSORT_OMN;
	}
	else
	{
		// Replace an existing vertex
		if (flags & VTX_FLAG_RM)
		{
			// Triangle fan mode
			switch (tsort)
			{
				case VERTEXSTATE_TSORT_OMN: m_se.vertices[1] = vtx;  tsort = VERTEXSTATE_TSORT_ONM; break;
				case VERTEXSTATE_TSORT_MNO: m_se.vertices[0] = vtx;  tsort = VERTEXSTATE_TSORT_NMO; break;
				case VERTEXSTATE_TSORT_ONM: m_se.vertices[2] = vtx;  tsort = VERTEXSTATE_TSORT_OMN; break;
				case VERTEXSTATE_TSORT_NOM: m_se.vertices[2] = vtx;  tsort = VERTEXSTATE_TSORT_MON; break;
				case VERTEXSTATE_TSORT_MON: m_se.vertices[0] = vtx;  tsort = VERTEXSTATE_TSORT_NOM; break;
				case VERTEXSTATE_TSORT_NMO: m_se.vertices[1] = vtx;  tsort = VERTEXSTATE_TSORT_MNO; break;
			}
		}
		else
		{
			// Triangle strip mode
			switch (tsort)
			{
				case VERTEXSTATE_TSORT_OMN: m_se.vertices[0] = vtx;  tsort = VERTEXSTATE_TSORT_NOM; break;
				case VERTEXSTATE_TSORT_MNO: m_se.vertices[2] = vtx;  tsort = VERTEXSTATE_TSORT_OMN; break;
				case VERTEXSTATE_TSORT_ONM: m_se.vertices[0] = vtx;  tsort = VERTEXSTATE_TSORT_NMO; break;
				case VERTEXSTATE_TSORT_NOM: m_se.vertices[1] = vtx;  tsort = VERTEXSTATE_TSORT_MNO; break;
				case VERTEXSTATE_TSORT_MON: m_se.vertices[1] = vtx;  tsort = VERTEXSTATE_TSORT_ONM; break;
				case VERTEXSTATE_TSORT_NMO: m_se.vertices[2] = vtx;  tsort = VERTEXSTATE_TSORT_MON; break;
			}
		}

	}

	// Update fields
	m_se.vertex_state = (vcnt << VERTEXSTATE_VCNT_SHIFT) | tsort;

	// Do we have three valid vertices?
	if (vcnt == 3)
	{
		// Send the triangle to the setup engine
		setup_triangle(flags);
	}
}


//-------------------------------------------------
//  calculate_slope -
//-------------------------------------------------

void m2_te_device::calculate_slope(const slope_params &sp, float q1, float q2, float q3, float &slope_out, float &ddx_out)
{
	float detx = q1 * sp.y23 + q2 * sp.y31 + q3 * sp.y12;
	float dety = q1 * sp.x23 + q2 * sp.x31 + q3 * sp.x12;
	float detxy = sp.xstep_long * detx - dety;

	slope_out = detxy * sp.iAria;
	ddx_out = detx * sp.iAria;
}


//-------------------------------------------------
//  log_triangle -
//-------------------------------------------------

void m2_te_device::log_triangle(uint32_t flags)
{
	logerror("[Triangle]\n");

	for (uint32_t i = 0; i < 3; ++i)
	{
		char s[64];
		char t[64];
		char p[64];

		s[0] = '\0';
		t[0] = '\0';
		p[0] = '\0';

		const se_vtx &vtx = m_se.vertices[i];

		if (flags & VTX_FLAG_SHAD)
			sprintf(s, "COLR[R:%.6f G:%.6f B:%.6f A:%.6f]", vtx.r, vtx.g, vtx.b, vtx.a);

		if (flags & VTX_FLAG_TEXT)
			sprintf(t, "TEXT[UW:%.6f VW:%.6f]", vtx.uw, vtx.vw);

		if (flags & VTX_FLAG_PRSP)
			sprintf(p, "PRSP[W:%.6f]", vtx.w);

		logerror("V%d: X:%.6f Y:%.6f %s %s %s\n", i, vtx.x, vtx.y, s, t, p);
	}
}


//-------------------------------------------------
//  setup_triangle -
//-------------------------------------------------

void m2_te_device::setup_triangle(uint32_t flags)
{
//  log_triangle(flags);

	se_vtx va = m_se.vertices[0];
	se_vtx vb = m_se.vertices[1];
	se_vtx vc = m_se.vertices[2];

	// Truncate XY coordinates to integers
	va.x = int_trunc(va.x);
	vb.x = int_trunc(vb.x);
	vc.x = int_trunc(vc.x);

	va.y = int_trunc(va.y);
	vb.y = int_trunc(vb.y);
	vc.y = int_trunc(vc.y);


	// Scale and truncate colors
	if (flags & VTX_FLAG_SHAD)
	{
		va.r = int_trunc(va.r * 255.0f);
		va.g = int_trunc(va.g * 255.0f);
		va.b = int_trunc(va.b * 255.0f);
		va.a = int_trunc(va.a * 255.0f);

		vb.r = int_trunc(vb.r * 255.0f);
		vb.g = int_trunc(vb.g * 255.0f);
		vb.b = int_trunc(vb.b * 255.0f);
		vb.a = int_trunc(vb.a * 255.0f);

		vc.r = int_trunc(vc.r * 255.0f);
		vc.g = int_trunc(vc.g * 255.0f);
		vc.b = int_trunc(vc.b * 255.0f);
		vc.a = int_trunc(vc.a * 255.0f);
	}

	// Sort the vertices into top, middle and bottom
	uint32_t a = 0;

	if ((va.y - vb.y) < 0.0f) a |= 4;
	if ((vb.y - vc.y) < 0.0f) a |= 2;
	if ((vc.y - va.y) < 0.0f) a |= 1;

	const se_vtx *v1 = nullptr;
	const se_vtx *v2 = nullptr;
	const se_vtx *v3 = nullptr;

	switch (a)
	{
		case 1: v1 = &vc; v2 = &vb; v3 = &va; break;
		case 2: v1 = &vb; v2 = &va; v3 = &vc; break;
		case 3: v1 = &vb; v2 = &vc; v3 = &va; break;
		case 4: v1 = &va; v2 = &vc; v3 = &vb; break;
		case 5: v1 = &vc; v2 = &va; v3 = &vb; break;
		case 6: v1 = &va; v2 = &vb; v3 = &vc; break;
		default:
			return; // Reject degenerates
	}


	// Determine the W range for depth and texture values
	uint32_t wrange = 0;

	if (flags & VTX_FLAG_PRSP)
	{
		const uint8_t exp_zero = 0x7f;
		const uint8_t wexp0 = exp_zero - 3;
		const uint8_t wexp1 = exp_zero - 6;
		const uint8_t wexp2 = exp_zero - 9;

		uint8_t w1exp = extract_exp(v1->w);
		uint8_t w2exp = extract_exp(v2->w);
		uint8_t w3exp = extract_exp(v3->w);

		if (w1exp >= wexp0 || w2exp >= wexp0 || w3exp >= wexp0)
			wrange = 0;
		else if (w1exp >= wexp1 || w2exp >= wexp1 || w3exp >= wexp1)
			wrange = 1;
		else if (w1exp >= wexp2 || w2exp >= wexp2 || w3exp >= wexp2)
			wrange = 2;
		else
			wrange = 3;
	}

	// Calculate the coordinate deltas
	float x12 = v1->x - v2->x;
	float x23 = v2->x - v3->x;
	float x31 = v3->x - v1->x;
	float y12 = v1->y - v2->y;
	float y23 = v2->y - v3->y;
	float y31 = v3->y - v1->y;

	// Calculate the triangle area
	float aria = v1->x * y23 + v2->x * y31 + v3->x * y12;

	// Reject degenerates
	if (aria == 0.0f)
		return;

	float iaria = 1.0f / aria;

	// Determine scan direction
	bool scan_lr = aria > 0.0f;

	// Calculate vertex slopes
	float abs_y12 = fabs(y12);
	float abs_y23 = fabs(y23);
	float abs_y31 = fabs(y31);


	// Avoid division by zero
	float xslope_0 = (abs_y12 > 0.0f) ? (-x12 / abs_y12) : 0.0f;
	float xslope_1 = (abs_y23 > 0.0f) ? (-x23 / abs_y23) : 0.0f;
	float xslope_long = (abs_y31 > 0.0f) ? (x31 / abs_y31) : 0.0f;


	// Calculate x steps
	float xstep_long;
	float xstep_0;
	float xstep_1;

	if (scan_lr)
	{
		xstep_long = floor(xslope_long);
		xstep_0 = ceil(xslope_0);
		xstep_1 = ceil(xslope_1);
	}
	else
	{
		xstep_long = ceil(xslope_long);
		xstep_0 = floor(xslope_0);
		xstep_1 = floor(xslope_1);
	}

	float xystep_0 = x12 - xstep_0 * y12;
	float xystep_1 = x23 - xstep_1 * y23;
	float xystep_long = x31 - xstep_long * y31;

	float dy_0 = -y12;
	float dy_1 = -y23;
	float dy_long = -y31;


	// Calculate color and texture slopes
	slope_params sparams = { y23, y31, y12, x23, x31, x12, xstep_long, iaria };

	float r_slope = 0.0f, r_ddx = 0.0f;
	float g_slope = 0.0f, g_ddx = 0.0f;
	float b_slope = 0.0f, b_ddx = 0.0f;
	float a_slope = 0.0f, a_ddx = 0.0f;
	float w_slope = 0.0f, w_ddx = 0.0f;
	float uw_slope = 0.0f, uw_ddx = 0.0f;
	float vw_slope = 0.0f, vw_ddx = 0.0f;

	if (flags & VTX_FLAG_SHAD)
	{
		calculate_slope(sparams, v1->r, v2->r, v3->r, r_slope, r_ddx);
		calculate_slope(sparams, v1->g, v2->g, v3->g, g_slope, g_ddx);
		calculate_slope(sparams, v1->b, v2->b, v3->b, b_slope, b_ddx);
		calculate_slope(sparams, v1->a, v2->a, v3->a, a_slope, a_ddx);
	}

	if (flags & VTX_FLAG_TEXT)
	{
		calculate_slope(sparams, v1->uw, v2->uw, v3->uw, uw_slope, uw_ddx);
		calculate_slope(sparams, v1->vw, v2->vw, v3->vw, vw_slope, vw_ddx);
	}

	if (flags & VTX_FLAG_PRSP)
	{
		calculate_slope(sparams, v1->w, v2->w, v3->w, w_slope, w_ddx);
	}

	const uint32_t textbits = text_bits[wrange];
	const uint32_t depthbits = depth_bits[wrange];

	// Convert everything to fixed point and pass to the edge walker

	/*
	    NOT SURE THESE ARE RIGHT. See P209

	    XY 12.0
	    SL s12.0
	    RGBA 9.0
	    RGBA SLOPES = s9.11
	    TEX: 11.13
	    DEPTH: 1.23
	    TEX SLOPES: s11.13
	    DEPTH SLOPES: s1.23
	    AREA : s23.0

	    Numbers output to the edge walker are 24 bits

	    IEE: 1.8.23
	    BDA: 1.7.24 (includes hidden bit)

	    However, we're using 23 bits.
	*/


	// XY:     0000 0000 0iii iiii iiii 0000 0000 0000
	// Colors: 0000 0000 0iii iiii ifff ffff ffff 0000
	// UV:
	// W:      0000 0000 0iii iiii iiii iiii iiii iiii


	// Converted numbers are left-aligned into the mantissa
	// However, we want them right-aligned for convenience
	// TODO: Is there a better way to do this?
	m_es.x1 = ieee754_to_tefix(v1->x, xy_bits) >> xy_rshift;
	m_es.y1 = ieee754_to_tefix(v1->y, xy_bits) >> xy_rshift;
	m_es.x2 = ieee754_to_tefix(v2->x, xy_bits) >> xy_rshift;
	m_es.y2 = ieee754_to_tefix(v2->y, xy_bits) >> xy_rshift;
	m_es.y3 = ieee754_to_tefix(v3->y, xy_bits) >> xy_rshift;

	m_es.xstep_0 = ieee754_to_tefix(xstep_0, xy_bits) >> xy_rshift;
	m_es.xstep_1 = ieee754_to_tefix(xstep_1, xy_bits) >> xy_rshift;
	m_es.xstep_long = ieee754_to_tefix(xstep_long, xy_bits) >> xy_rshift;
	m_es.xystep_0 = ieee754_to_tefix(xystep_0, xy_bits) >> xy_rshift;
	m_es.xystep_1 = ieee754_to_tefix(xystep_1, xy_bits) >> xy_rshift;
	m_es.xystep_long = ieee754_to_tefix(xystep_long, xy_bits) >> xy_rshift;
	m_es.dy_0 = ieee754_to_tefix(dy_0, xy_bits) >> xy_rshift;
	m_es.dy_1 = ieee754_to_tefix(dy_1, xy_bits) >> xy_rshift;
	m_es.dy_long = ieee754_to_tefix(dy_long, xy_bits) >> xy_rshift;

	m_es.r1 = ieee754_to_tefix(v1->r, color_bits) >> color_rshift;
	m_es.g1 = ieee754_to_tefix(v1->g, color_bits) >> color_rshift;
	m_es.b1 = ieee754_to_tefix(v1->b, color_bits) >> color_rshift;
	m_es.a1 = ieee754_to_tefix(v1->a, color_bits) >> color_rshift;
	m_es.ddx_r = ieee754_to_tefix(r_ddx, color_bits) >> color_rshift;
	m_es.ddx_g = ieee754_to_tefix(g_ddx, color_bits) >> color_rshift;
	m_es.ddx_b = ieee754_to_tefix(b_ddx, color_bits) >> color_rshift;
	m_es.ddx_a = ieee754_to_tefix(a_ddx, color_bits) >> color_rshift;
	m_es.slope_r = ieee754_to_tefix(r_slope, color_bits) >> color_rshift;
	m_es.slope_g = ieee754_to_tefix(g_slope, color_bits) >> color_rshift;
	m_es.slope_b = ieee754_to_tefix(b_slope, color_bits) >> color_rshift;
	m_es.slope_a = ieee754_to_tefix(a_slope, color_bits) >> color_rshift;

	// No need to shift these
	m_es.uw1 = ieee754_to_tefix(v1->uw, textbits);
	m_es.vw1 = ieee754_to_tefix(v1->vw, textbits);
	m_es.w1 = ieee754_to_tefix(v1->w, depthbits);
	m_es.ddx_uw = ieee754_to_tefix(uw_ddx, textbits);
	m_es.ddx_vw = ieee754_to_tefix(vw_ddx, textbits);
	m_es.ddx_w = ieee754_to_tefix(w_ddx, depthbits);
	m_es.slope_uw = ieee754_to_tefix(uw_slope, textbits);
	m_es.slope_vw = ieee754_to_tefix(vw_slope, textbits);
	m_es.slope_w = ieee754_to_tefix(w_slope, depthbits);

	m_es.r2l = !scan_lr;

#if TEST_TIMING
	g_statistics[STAT_TRIANGLES_PROCESSED]++;
#endif

	// Proceed to walk the edges
	walk_edges(wrange);
}

static inline bool ins(bool lr, int32_t v)
{
	return lr ? v <= 0 : v >= 0;
}

static inline bool ein(bool lr, int32_t v)
{
	return lr ? v < 0 : v > 0;
}

static const uint8_t m_nr_rom[128] =
{
	127, 125, 123, 121, 119, 118,
	116, 114, 112, 110, 109, 107,
	105, 104, 102, 100, 99, 97,
	96, 94, 93, 91, 90, 88,
	87, 86, 84, 83, 81, 80,
	79, 78, 76, 75, 74, 73,
	71, 70, 69, 68, 67, 65,
	64, 63, 62, 61, 60, 59,
	58, 57, 56, 55, 54, 53,
	52, 51, 50, 49, 48, 47,
	46, 45, 44, 43, 42, 41,
	40, 40, 39, 38, 37, 36,
	35, 35, 34, 33, 32, 31,
	31, 30, 29, 28, 28, 27,
	26, 25, 25, 24, 23, 23,
	22, 21, 21,20, 19, 19,
	18, 17, 17, 16, 15, 15,
	14, 13, 13, 12, 12, 11,
	11, 10, 9, 9, 8, 8,
	7, 7, 6, 5, 5, 4,
	4, 3, 3, 2, 2, 1,
	1, 0
};


//-------------------------------------------------
//  nr_invert - Newton Rhapson reciprocal
//-------------------------------------------------

uint32_t nr_invert(uint32_t num, uint32_t & shift_amount)
{
	uint32_t original;
	uint32_t first_guess;
	uint32_t m;
	uint32_t shfnum;

	// TODO: Unconfirmed but seems sensible
	if (num == 0)
	{
		shift_amount = 0;
		return 0;
	}

	shift_amount = -1;
	while ((num << ++shift_amount) < 0x800000);

	shfnum = num << shift_amount;
	original = shfnum >> 8;
	first_guess = m_nr_rom[(shfnum >> 16) & ((1 << 7) - 1)];
	first_guess = first_guess +  128;
	m = first_guess * original;
	m = (1 << 24) - m;
	m = m >> 3;
	m = first_guess * m;
	m = m >> 8;

	return m;
}


//-------------------------------------------------
//  walk_edges -
//-------------------------------------------------

void m2_te_device::walk_edges(uint32_t wrange)
{
	bool dsp_off = m_es.es_cntl & ESCNTL_DSPOFF;
	bool du_scan = m_es.es_cntl & ESCNTL_DUSCAN;
	bool omit_right = true;
	bool scan_lr = m_es.r2l == false;

	uint32_t cf_long = 0;
	uint32_t cf_short = 0;
	uint32_t xystep;
	uint32_t xstep;
	uint32_t dy;
	uint32_t xs;
	uint32_t xe;

	int32_t r, g, b, a;
	int32_t uw, vw;
	int32_t w;

	// Determine starting values
	uint32_t y = m_es.y1;

	// Flat-top triangles are a special case
	if (m_es.y1 == m_es.y2)
	{
		xystep = m_es.xystep_1;
		xstep = m_es.xstep_1;
		dy = m_es.dy_1;

		if (scan_lr ^ (m_es.x1 < m_es.x2))
		{
			 // TODO: Is this possible?
			throw emu_fatalerror("m2_te_device::walk_edges: SPECIAL CASE: WHAT DO?");
			r = m_es.r1; // Where do the colors come from?
			g = m_es.g1;
			b = m_es.b1;
			a = m_es.a1;
			uw = m_es.uw1;
			vw = m_es.vw1;
			w = m_es.w1;

			xs = m_es.x2;
			xe = m_es.x1;
		}
		else
		{
			r = m_es.r1;
			g = m_es.g1;
			b = m_es.b1;
			a = m_es.a1;
			uw = m_es.uw1;
			vw = m_es.vw1;
			w = m_es.w1;

			xs = m_es.x1;
			xe = m_es.x2;
		}
	}
	else
	{
		// Regular triangles
		xystep = m_es.xystep_0;
		xstep = m_es.xstep_0;
		dy = m_es.dy_0;
		r = m_es.r1;
		g = m_es.g1;
		b = m_es.b1;
		a = m_es.a1;
		uw = m_es.uw1;
		vw = m_es.vw1;
		w = m_es.w1;

		xs = m_es.x1;
		xe = xs;
	}

	do
	{
		// Render the pixels from this span
		walk_span(wrange, omit_right, y, xs, xe, r, g, b, a, uw, vw, w);

		// Now update the values
		omit_right = false;
		cf_short += xystep;
		cf_long += m_es.xystep_long;

		bool step_back = false;

		if (scan_lr)
		{
			xs += m_es.xstep_long;
			xe += xstep;

			if (!ins(scan_lr, cf_long))
			{
				step_back = true;
				cf_long += m_es.dy_long;
				xs += xy_one;
			}

			if (dsp_off ? !ins(scan_lr, cf_short) : !ein(scan_lr, cf_short))
			{
				cf_short -= dy;
				xe -= xy_one;
			}
		}
		else
		{
			xe += xstep;
			xs += m_es.xstep_long;

			if (!ins(scan_lr, cf_short))
			{
				cf_short += dy;
				xe += xy_one;
			}

			if (dsp_off ? !ins(scan_lr, cf_long) : !ein(scan_lr, cf_long))
			{
				step_back = true;
				cf_long -= m_es.dy_long;
				xs -= xy_one;
			}
		}

		// Update the color values
		if (!(m_gc.te_master_mode & TEMASTER_MODE_DSHADE))
		{
			if (scan_lr)
			{
				r += step_back ? m_es.slope_r + m_es.ddx_r : m_es.slope_r;
				g += step_back ? m_es.slope_g + m_es.ddx_g : m_es.slope_g;
				b += step_back ? m_es.slope_b + m_es.ddx_b : m_es.slope_b;
				a += step_back ? m_es.slope_a + m_es.ddx_a : m_es.slope_a;
			}
			else
			{
				r += step_back ? m_es.slope_r - m_es.ddx_r : m_es.slope_r;
				g += step_back ? m_es.slope_g - m_es.ddx_g : m_es.slope_g;
				b += step_back ? m_es.slope_b - m_es.ddx_b : m_es.slope_b;
				a += step_back ? m_es.slope_a - m_es.ddx_a : m_es.slope_a;
			}

			// Clamp to 8.11
			r = std::clamp<int32_t>(r, 0, 0x0007ffff);
			g = std::clamp<int32_t>(g, 0, 0x0007ffff);
			b = std::clamp<int32_t>(b, 0, 0x0007ffff);
			a = std::clamp<int32_t>(a, 0, 0x0007ffff);
		}

		if (!(m_es.es_cntl & TEMASTER_MODE_DTEXT))
		{
			if (scan_lr)
			{
				uw += step_back ? m_es.slope_uw + m_es.ddx_uw : m_es.slope_uw;
				vw += step_back ? m_es.slope_vw + m_es.ddx_vw : m_es.slope_vw;
			}
			else
			{
				uw += step_back ? m_es.slope_uw - m_es.ddx_uw : m_es.slope_uw;
				vw += step_back ? m_es.slope_vw - m_es.ddx_vw : m_es.slope_vw;
			}

			// Clamp to 10.13
			uw = std::clamp<int32_t>(uw, 0, 0x007fffff);
			vw = std::clamp<int32_t>(vw, 0, 0x007fffff);
		}

		if (!(m_es.es_cntl & ESCNTL_PERSPECTIVEOFF))
		{
			if (scan_lr)
				w += step_back ? m_es.slope_w + m_es.ddx_w : m_es.slope_w;
			else
				w += step_back ? m_es.slope_w - m_es.ddx_w : m_es.slope_w;

			// Clamp to 0.23
			w = std::clamp<int32_t>(w, 0, 0x007fffff);
		}

		// Update Y
		uint32_t next_y = y + (du_scan ? -xy_one : xy_one);

		if (next_y == m_es.y2)
		{
			cf_short = 0;
			xystep = m_es.xystep_1;
			xstep = m_es.xstep_1;
			dy = m_es.dy_1;
			xe = m_es.x2;

			if (scan_lr)
				omit_right = true;
		}

		y = next_y;

	} while (y != m_es.y3);
}



//-------------------------------------------------
//  texcoord_gen -
//-------------------------------------------------

void m2_te_device::texcoord_gen(uint32_t wrange, uint32_t uw, uint32_t vw, uint32_t w,
								uint32_t & uo, uint32_t & vo, uint32_t & wo)
{

	// Perspective correction
	if (!(m_es.es_cntl & ESCNTL_PERSPECTIVEOFF))
	{
		uint32_t wshift;

		// Calculate the inverse of 1/w. Output is 0.20
		uint32_t winv = nr_invert(w, wshift);

		// 10.13 normalize then reduce to 10.6
		// Normalization occurs by detecting leading zeroes and
		// left-shifting until 1 in MSB

		uint32_t uwshift = 0;
		uint32_t vwshift = 0;

		while ((uw < 0x00400000) && uwshift < 7)
		{
			uw <<= 1;
			++uwshift;
		}

		while ((vw < 0x00400000) && vwshift < 7)
		{
			vw <<= 1;
			++vwshift;
		}

		uint32_t normalized_uw = uw >> 7;
		uint32_t normalized_vw = vw >> 7;

		// 10.6 * 0.20 = 10.26
		uint64_t ur = (uint64_t)normalized_uw * winv;
		uint64_t vr = (uint64_t)normalized_vw * winv;

		// De-normalize
		if (uwshift > wshift)
			ur >>= (uwshift - wshift);
		else
			ur <<= (wshift - uwshift);

		if (vwshift > wshift)
			vr >>= (vwshift - wshift);
		else
			vr <<= (wshift - vwshift);

		// Reduce from 10.26 to 10.6
		uo = ur >> 20;
		vo = vr >> 20;
	}
	else
	{
		// Reduce from 10.13 to 10.6
		// TODO: Is this actually correct?
		uo = uw >> (7 + (10 - text_bits[wrange]));
		vo = vw >> (7 + (10 - text_bits[wrange]));
	}

	// Output to texture mapper is 10.4
	uo = uo >> 2;
	vo = vo >> 2;
}


//-------------------------------------------------
//  lod_calc -
//-------------------------------------------------

uint32_t m2_te_device::lod_calc(uint32_t u0, uint32_t v0, uint32_t u1, uint32_t v1)
{
	return 0;
}


//-------------------------------------------------
//  get_tram_bitdepth
//-------------------------------------------------

uint32_t m2_te_device::get_tram_bitdepth()
{
	// TODO: Could cache this
	const uint32_t tex_type = m_tm.tex_exptype;

	uint32_t bitdepth = 0;

	if (tex_type & TXTEXPFORM_COLORON)
		bitdepth += (tex_type & TXTEXPFORM_CDEPTH_MASK) >> TXTEXPFORM_CDEPTH_SHIFT;
	if (tex_type & TXTEXPFORM_ALPHAON)
		bitdepth += (tex_type & TXTEXPFORM_ADEPTH_MASK) >> TXTEXPFORM_ADEPTH_SHIFT;
	if (tex_type & TXTEXPFORM_SSBON)
		++bitdepth;

	return bitdepth;
}


//-------------------------------------------------
//  get_texture_color -
//-------------------------------------------------
void m2_te_device::get_texture_color(uint32_t u, uint32_t v, uint32_t lod,
									uint32_t & r, uint32_t & g, uint32_t & b, uint32_t & a, uint32_t & s)
{
	uint32_t texaddr;
	uint32_t texbit;
	uint32_t texdepth = get_tram_bitdepth();

	// TODO: Multiple LODs
	uint32_t filter = (m_tm.tex_addr_cntl >> TXTADDRCNTL_R12FILTERSEL_SHIFT) & TXTADDRCNTL_FILTERSEL_MASK;

	switch (filter)
	{
		case TXTADDRCNTL_FILTERSEL_POINT:
		case TXTADDRCNTL_FILTERSEL_LINEAR: // TODO
		{
			addr_calc(u, v, lod, texaddr, texbit, texdepth);
			get_texel(texaddr, texbit, texdepth, r, g, b, a, s);
			break;
		}

		case TXTADDRCNTL_FILTERSEL_BILINEAR:
		case TXTADDRCNTL_FILTERSEL_QUASITRI: // TODO
		{
			// See p170 for implemenation
			uint32_t r0, g0, b0, a0, s0;
			uint32_t r1, g1, b1, a1, s1;
			uint32_t r2, g2, b2, a2, s2;
			uint32_t r3, g3, b3, a3, s3;

			addr_calc(u, v, lod, texaddr, texbit, texdepth);
			get_texel(texaddr, texbit, texdepth, r0, g0, b0, a0, s0);
			addr_calc(u + 0x10, v, lod, texaddr, texbit, texdepth);
			get_texel(texaddr, texbit, texdepth, r1, g1, b1, a1, s1);
			addr_calc(u, v + 0x10, lod, texaddr, texbit, texdepth);
			get_texel(texaddr, texbit, texdepth, r2, g2, b2, a2, s2);
			addr_calc(u + 0x10, v + 0x10, lod, texaddr, texbit, texdepth);
			get_texel(texaddr, texbit, texdepth, r3, g3, b3, a3, s3);

			// LERP
			uint32_t ufrac = u & 0xf;
			uint32_t vfrac = v & 0xf;
			uint32_t om_ufrac = 0x10 - ufrac;
			uint32_t om_vfrac = 0x10 - vfrac;

			// This is probably wrong
			s0 = s0 * 0xff;
			s1 = s1 * 0xff;
			s2 = s2 * 0xff;
			s3 = s3 * 0xff;

			// 8.0 * 1.4 + 8.0 * 1.4 = 9.4
			uint32_t rl = r0 * (om_ufrac) + r1 * ufrac;
			uint32_t gl = g0 * (om_ufrac) + g1 * ufrac;
			uint32_t bl = b0 * (om_ufrac) + b1 * ufrac;
			uint32_t al = a0 * (om_ufrac) + a1 * ufrac;
			uint32_t sl = s0 * (om_ufrac) + s1 * ufrac;

			uint32_t ru = r2 * (om_ufrac) + r3 * ufrac;
			uint32_t gu = g2 * (om_ufrac) + g3 * ufrac;
			uint32_t bu = b2 * (om_ufrac) + b3 * ufrac;
			uint32_t au = a2 * (om_ufrac) + a3 * ufrac;
			uint32_t su = s2 * (om_ufrac) + s3 * ufrac;

			// 9.4 * 0.4 + 9.4 * 0.4 = 10.8?
			r = (rl * om_vfrac + ru * vfrac) >> 8;
			g = (gl * om_vfrac + gu * vfrac) >> 8;
			b = (bl * om_vfrac + bu * vfrac) >> 8;
			a = (al * om_vfrac + au * vfrac) >> 8;

			s = ((sl * om_vfrac + su * vfrac) >> 10) ? 1 : 0;
			break;
		}
	}
}


//-------------------------------------------------
//  addr_calc -
//-------------------------------------------------
void m2_te_device::addr_calc(uint32_t u, uint32_t v, uint32_t lod,
							uint32_t & texaddr, uint32_t & texbit, uint32_t & tdepth)
{
	uint32_t u_mask = (m_tm.uv_mask & TXTUVMASK_UMASK_MASK) >> TXTUVMASK_UMASK_SHIFT;
	uint32_t v_mask = (m_tm.uv_mask & TXTUVMASK_VMASK_MASK) >> TXTUVMASK_VMASK_SHIFT;
	uint32_t u_max = (m_tm.uv_max & TXTUVMAX_UMAX_MASK) >> TXTUVMAX_UMAX_SHIFT;
	uint32_t v_max = (m_tm.uv_max & TXTUVMAX_VMAX_MASK) >> TXTUVMAX_VMAX_SHIFT;

	// Remove fractional parts (10.0)
	uint32_t u0 = u >> 4;
	uint32_t v0 = v >> 4;

	u0 = u0 & u_mask;
	v0 = v0 & v_mask;

	u_max &= u_mask;
	v_max &= v_mask;

	u0 = std::min(u0, u_max);
	v0 = std::min(v0, v_max);

	// LOD
	uint32_t lodmax = m_tm.tex_addr_cntl & TXTADDRCNTL_LODMAX_MASK;
	uint32_t lod_shift = (lodmax - lod) & 3;

	// 20.0
	uint32_t voffset = (v0 * (u_max + 1)) << lod_shift;
	uint32_t offset = (voffset + u0) >> lod_shift;

	offset = offset * tdepth;

	uint32_t base_addr = 0;

	switch (lod)
	{
		case 0: base_addr = m_tm.tex_lod_base0; break;
		case 1: base_addr = m_tm.tex_lod_base1; break;
		case 2: base_addr = m_tm.tex_lod_base2; break;
		case 3: base_addr = m_tm.tex_lod_base3; break;
	}

	uint32_t adsum = (base_addr << 3) + offset;

	texaddr = adsum / 32;
	texbit = adsum & 31;
}


void m2_te_device::get_texel(uint32_t tex_addr, uint32_t tex_bit, uint32_t tdepth,
							uint32_t & r, uint32_t & g, uint32_t & b, uint32_t & a, uint32_t & ssb)
{
	const uint32_t tex_type = m_tm.tex_exptype;

	uint8_t rtex, gtex, btex, atex;
	uint8_t ssbtex;

	uint8_t rpip, gpip, bpip, apip;
	uint8_t ssbpip;

	// Page 127
	// See M2_2_7_part3_clt_ocrc.pdf page 95 for better expl.

	uint32_t tram_data = m_tram[tex_addr];

#if TEST_TIMING
	g_statistics[STAT_TEXEL_READS]++;
#endif


	// Align the texel
	uint32_t texel = tram_data >> (tex_bit & ~7);

	// Align within byte (TODO: Fix for 32-bits!)
	texel >>= (tex_bit ^ (8 - tdepth)) & 7;

	uint32_t c_depth = (tex_type & TXTEXPFORM_CDEPTH_MASK) >> TXTEXPFORM_CDEPTH_SHIFT;
	uint32_t a_depth = (tex_type & TXTEXPFORM_ADEPTH_MASK) >> TXTEXPFORM_ADEPTH_SHIFT;
	uint32_t i_depth = c_depth;

	// Color is either sourced from tram or PIP
	if (tex_type & TXTEXPFORM_LITERAL)
	{
		if (tex_type & TXTEXPFORM_COLORON)
		{
			// Literal can only be 32-bit or 16bpp
			uint32_t mask = (1 << c_depth) - 1;

			btex = texel & mask;
			texel >>= c_depth;
			gtex = texel & mask;
			texel >>= c_depth;
			rtex = texel & mask;
			texel >>= c_depth;
		}
		else
		{
			rtex = gtex = btex = 0;
		}
	}
	else
	{
		rtex = gtex = btex = 0;
	}

	if (tex_type & TXTEXPFORM_INDEXON)
	{
		uint32_t pipidx = texel & ((1 << i_depth) - 1);
		uint32_t pipaddr = (m_tm.tex_pip_cntl & TXTPIPCNTL_INDEX_OFFSET) + pipidx;

		uint32_t pipval = m_pipram[pipaddr & 0xff];

		// Color, alpha and SSB always present in PIP
		bpip = pipval & 0xff;
		gpip = (pipval >> 8) & 0xff;
		rpip = (pipval >> 16) & 0xff;
		apip = (pipval >> 24) & 0x7f;
		ssbpip = pipval >> 31;

		texel >>= i_depth;
	}
	else
	{
		// Nothing coming from the PIP
		rpip = gpip = bpip = 0;
		apip = ssbpip = 0;
	}

	// Alpha and SSB could be present
	if (tex_type & TXTEXPFORM_ALPHAON)
	{
		uint32_t mask = (1 << a_depth) - 1;

		atex = texel & mask;

		// Need to expand alpha if 4-bits
		if (a_depth == 4)
			atex = (atex << 3) | (atex >> 1);

		texel >>= a_depth;
	}
	else
	{
		atex = 0;
	}

	if (tex_type & TXTEXPFORM_SSBON)
		ssbtex = texel & 1;
	else
		ssbtex = 0;

	// Select the SSB
	switch ((m_tm.tex_pip_cntl & TXTPIPCNTL_SSBSEL_MASK) >> TXTPIPCNTL_SSBSEL_SHIFT)
	{
		case TXTPIPCNTL_SEL_CONSTANT:
		{
			uint32_t cnst = ssbtex ? m_tm.tex_srcconst1 : m_tm.tex_srcconst0;
			ssb = cnst >> 31;
			break;
		}
		case TXTPIPCNTL_SEL_TRAM:
		{
			ssb = ssbtex;
			break;
		}
		case TXTPIPCNTL_SEL_PIP:
		{
			ssb = ssbpip;
			break;
		}
	}

	// PIP color control
	switch ((m_tm.tex_pip_cntl & TXTPIPCNTL_COLORSEL_MASK) >> TXTPIPCNTL_COLORSEL_SHIFT)
	{
		case TXTPIPCNTL_SEL_CONSTANT:
		{
			uint32_t cnst = ssb ? m_tm.tex_srcconst1 : m_tm.tex_srcconst0;
			r = (cnst >> 16) & 0xff;
			g = (cnst >>  8) & 0xff;
			b = (cnst >>  0) & 0xff;
			break;
		}
		case TXTPIPCNTL_SEL_TRAM:
		{
			r = rtex;
			g = gtex;
			b = btex;
			break;
		}
		case TXTPIPCNTL_SEL_PIP:
		{
			r = rpip;
			g = gpip;
			b = bpip;
			break;
		}
	}

	// Alpha control
	switch ((m_tm.tex_pip_cntl & TXTPIPCNTL_ALPHASEL_MASK) >> TXTPIPCNTL_ALPHASEL_SHIFT)
	{
		case TXTPIPCNTL_SEL_CONSTANT:
		{
			uint32_t cnst = ssb ? m_tm.tex_srcconst1 : m_tm.tex_srcconst0;
			a = (cnst >> 24) & 0x7f;
			break;
		}
		case TXTPIPCNTL_SEL_TRAM:
		{
			a = atex;
			break;
		}
		case TXTPIPCNTL_SEL_PIP:
		{
			a = apip;
			break;
		}
	}

	// Expand alpha to 8-bits
	a = (a << 1) | (a >> 6);
}


//-------------------------------------------------
//  lerp
//-------------------------------------------------

static inline uint8_t lerp(uint8_t a, uint8_t b, uint8_t t)
{
	if (t == 255)
		return b;
	if (t == 0)
		return a;

	return (((255 - t) * a) >> 8) + ((t * b) >> 8);
}


//-------------------------------------------------
//  multiply
//-------------------------------------------------

static inline uint8_t multiply(uint8_t a, uint8_t b)
{
	if (b == 255)
		return a;
	if (a == 255)
		return b;
	else
		return (a * b) >> 8;
}


//-------------------------------------------------
//  texture_blend -
//-------------------------------------------------

void m2_te_device::texture_blend(
	uint32_t ri, uint32_t gi, uint32_t bi, uint32_t ai,
	uint32_t rt, uint32_t gt, uint32_t bt, uint32_t at, uint32_t ssbt,
	uint32_t &ro, uint32_t &go, uint32_t &bo, uint32_t &ao, uint32_t &ssbo)
{
	uint32_t rbl = 0, gbl = 0, bbl = 0, abl = 0;

	{
		uint32_t ar, ab, ag, aa;
		uint32_t br, bb, bg, ba;
		uint32_t tr, tb, tg;

		select_lerp( (m_tm.tex_tab_cntl & TXTTABCNTL_C_ASEL_MASK) >> TXTTABCNTL_C_ASEL_SHIFT,
					ri, gi, bi, ai,
					rt, gt, bt, at, ssbt,
					ar, ag, ab );

		select_lerp( (m_tm.tex_tab_cntl & TXTTABCNTL_C_BSEL_MASK) >> TXTTABCNTL_C_BSEL_SHIFT,
					ri, gi, bi, ai,
					rt, gt, bt, at, ssbt,
					br, bg, bb );

		select_lerp( (m_tm.tex_tab_cntl & TXTTABCNTL_C_TSEL_MASK) >> TXTTABCNTL_C_TSEL_SHIFT,
					ri, gi, bi, ai,
					rt, gt, bt, at, ssbt,
					tr, tg, tb );

		if (((m_tm.tex_tab_cntl & TXTTABCNTL_BLENDOP_MASK) >> TXTTABCNTL_BLENDOP_SHIFT) == TXTTABCNTL_BLENDOP_LERP)
		{
			rbl = lerp(ar, br, tr);
			gbl = lerp(ag, bg, tg);
			bbl = lerp(ab, bb, tb);
		}
		else
		{
			// TODO: CHECK ME
			// Alpha is multiply only
			select_mul( (m_tm.tex_tab_cntl & TXTTABCNTL_A_ASEL_MASK) >> TXTTABCNTL_A_ASEL_SHIFT,
						ai, at, ssbt,
						aa);

			select_mul( (m_tm.tex_tab_cntl & TXTTABCNTL_A_BSEL_MASK) >> TXTTABCNTL_A_BSEL_SHIFT,
						ai, at, ssbt,
						ba);

			rbl = multiply(ar, br);
			gbl = multiply(ag, bg);
			bbl = multiply(ab, bb);
			abl = multiply(aa, ba);
		}
	}

	// Now select the output
	switch ((m_tm.tex_tab_cntl & TXTTABCNTL_C_OSEL_MASK) >> TXTTABCNTL_C_OSEL_SHIFT)
	{
		case TXTTABCNTL_CO_SEL_CITER:
		{
			ro = ri;
			go = gi;
			bo = bi;
			break;
		}
		case TXTTABCNTL_CO_SEL_CT:
		{
			ro = rt;
			go = gt;
			bo = bt;
			break;
		}
		case TXTTABCNTL_CO_SEL_BLEND:
		{
			ro = rbl;
			go = gbl;
			bo = bbl;
			break;
		}
		case TXTTABCNTL_CO_SEL_RESERVED:
		{
			ro = go = bo = 0;
			break;
		}
	}

	// Select alpha output
	switch ((m_tm.tex_tab_cntl & TXTTABCNTL_A_OSEL_MASK) >> TXTTABCNTL_A_OSEL_SHIFT)
	{
		case TXTTABCNTL_AO_SEL_AITER:
		{
			ao = ai;
			break;
		}
		case TXTTABCNTL_AO_SEL_AT:
		{
			ao = at;
			break;
		}
		case TXTTABCNTL_AO_SEL_BLEND:
		{
			ao = abl;
			break;
		}
		case TXTTABCNTL_AO_SEL_RESERVED:
		{
			ao = 0;
			break;
		}
	}
}

void m2_te_device::select_lerp( uint32_t sel,
								uint32_t ri, uint32_t gi, uint32_t bi, uint32_t ai,
								uint32_t rt, uint32_t gt, uint32_t bt, uint32_t at, uint32_t ssbt,
								uint32_t & ar, uint32_t & ag, uint32_t & ab )
{
	switch (sel)
	{
		case TXTTABCNTL_C_ABTSEL_AITER:
		{
			ar = ai;
			ag = ai;
			ab = ai;
			break;
		}
		case TXTTABCNTL_C_ABTSEL_CITER:
		{
			ar = ri;
			ag = gi;
			ab = bi;
			break;
		}
		case TXTTABCNTL_C_ABTSEL_AT:
		{
			ar = at;
			ag = at;
			ab = at;
			break;
		}
		case TXTTABCNTL_C_ABTSEL_CT:
		{
			ar = rt;
			ag = gt;
			ab = bt;
			break;
		}
		case TXTTABCNTL_C_ABTSEL_ACONST:
		{
			uint32_t cnst = ssbt ? m_tm.tex_srcconst3 : m_tm.tex_srcconst2;
			uint8_t aval = (cnst >> 24) & 0x7f;

			// Expand to 8-bit
			aval = (aval << 1) | (aval >> 6);

			ar = aval;
			ag = aval;
			ab = aval;
			break;
		}
		case TXTTABCNTL_C_ABTSEL_CCONST:
		{
			uint32_t cnst = ssbt ? m_tm.tex_srcconst3 : m_tm.tex_srcconst2;

			ar = (cnst >> 16) & 0xff;
			ag = (cnst >>  8) & 0xff;
			ab = (cnst >>  0) & 0xff;
			break;
		}
		default:
		{
			ar = 0;
			ag = 0;
			ab = 0;
			break;
		}
	}
}


void m2_te_device::select_mul(uint32_t sel, uint32_t ai, uint32_t at, uint32_t ssbt,
							   uint32_t & a )
{
	switch (sel)
	{
		case TXTTABCNTL_A_ABSEL_AITER:
		{
			a = ai;
			break;
		}
		case TXTTABCNTL_A_ABSEL_AT:
		{
			a = at;
			break;
		}
		case TXTTABCNTL_A_ABSEL_ACONST:
		{
			uint32_t cnst = ssbt ? m_tm.tex_srcconst3 : m_tm.tex_srcconst2;
			uint8_t aval = (cnst >> 24) & 0x7f;

			// Expand to 8-bit
			a = (aval << 1) | (aval >> 6);
			break;
		}
		default:
		{
			a = 0;
			break;
		}
	}
}


//-------------------------------------------------
//  write_dst_pixel - Write pixel to framebuffer
//-------------------------------------------------

void m2_te_device::write_dst_pixel()
{
	uint32_t mask = m_db.usergen_ctrl & DBUSERGENCTL_DESTOUT_MASK;

	if (m_db.dst_ctrl & DBDESTCNTL_32BPP)
	{
		uint32_t dstaddr = m_db.dst_baseaddr + (m_dbstate.y * m_db.dst_xstride + m_dbstate.x) * sizeof(uint32_t);

		uint32_t old = m_bda->read_bus32(dstaddr);
		uint32_t out = 0;

		uint8_t sdsb_a = (old >> 24) & 0xff;
		uint8_t sr = (old >> 16) & 0xff;
		uint8_t sg = (old >> 8) & 0xff;
		uint8_t sb = old & 0xff;

		out |= (mask & 8 ? (m_dbstate.dsb << 7) | (m_dbstate.dst.a >> 1) : sdsb_a) << 24;
		out |= (mask & 4 ? m_dbstate.dst.r : sr) << 16;
		out |= (mask & 2 ? m_dbstate.dst.g : sg) << 8;
		out |= mask & 1 ? m_dbstate.dst.b : sb;

		m_bda->write_bus32(dstaddr, out);
	}
	else
	{
		uint32_t dstaddr = m_db.dst_baseaddr + (m_dbstate.y * m_db.dst_xstride + m_dbstate.x) * sizeof(uint16_t);

		uint16_t old = m_bda->read_bus16(dstaddr);
		uint16_t out = 0;

		uint8_t sdsb_a = (old >> 15) & 0x1;
		uint8_t sr = (old >> 10) & 0x1f;
		uint8_t sg = (old >> 5) & 0x1f;
		uint8_t sb = old & 0x1f;

		out |= (mask & 8 ? m_dbstate.dsb : sdsb_a) << 15;
		out |= (mask & 4 ? m_dbstate.dst.r >> 3 : sr) << 10;
		out |= (mask & 2 ? m_dbstate.dst.g >> 3 : sg) << 5;
		out |= mask & 1 ? m_dbstate.dst.b >> 3 : sb;

		m_bda->write_bus16(dstaddr, out);
	}

#if TEST_TIMING
	g_statistics[STAT_PIXEL_STORES]++;
#endif
}


//-------------------------------------------------
//  destination_blend -
//-------------------------------------------------

void m2_te_device::destination_blend(uint32_t x, uint32_t y, uint32_t w, const rgba & ti_color, uint8_t ssb)
{
	m_dbstate.x = x;
	m_dbstate.y = y;
	m_dbstate.w = w;
	m_dbstate.ti = ti_color;
	m_dbstate.ssb = ssb;

	bool dis = false;
	bool zpixout = true;
	bool zbufout = true;
	bool zclip = false;
	bool zclipdis = false;
	bool fbclipdis = false;
	bool winclipdis = false;

	// Z Status
	[[maybe_unused]] uint32_t zgel = 0;

	uint32_t zaddr;

	// FBCLIP
	{
		uint32_t xclip = (m_db.fbclip & DBFBCLIP_XFBCLIP_MASK) >> DBFBCLIP_XFBCLIP_SHIFT;
		uint32_t yclip = (m_db.fbclip & DBFBCLIP_YFBCLIP_MASK) >> DBFBCLIP_YFBCLIP_SHIFT;

		fbclipdis = (x >= xclip) || (y >= yclip);

		if (fbclipdis)
			m_db.status |= DBSTATUS_FBCLIP;
	}

	// WINCLIP
	{
		uint32_t xmin = (m_db.x_winclip & DBFBXWINCLIP_XMIN_MASK) >> DBFBXWINCLIP_XMIN_SHIFT;
		uint32_t xmax = (m_db.x_winclip & DBFBXWINCLIP_XMAX_MASK) >> DBFBXWINCLIP_XMAX_SHIFT;
		uint32_t ymin = (m_db.y_winclip & DBFBYWINCLIP_YMIN_MASK) >> DBFBYWINCLIP_YMIN_SHIFT;
		uint32_t ymax = (m_db.y_winclip & DBFBYWINCLIP_YMAX_MASK) >> DBFBYWINCLIP_YMAX_SHIFT;

		bool inside = (x >= xmin) && (x < xmax) && (y >= ymin) && (y < ymax);

		winclipdis = ((m_db.usergen_ctrl & DBUSERGENCTL_WCLIPINEN) && inside) ||
					((m_db.usergen_ctrl & DBUSERGENCTL_WCLIPOUTEN) && !inside);

		if (winclipdis)
			m_db.status |= DBSTATUS_WINCLIP;
	}

	select_src_pixel();

	select_tex_pixel();

	select_alpha_dsb();


	{
		uint8_t dm10, dm11, dm20, dm21;
		uint32_t txtcnst0 = m_db.txt_coef_const0;
		uint32_t txtcnst1 = m_db.txt_coef_const1;
		uint32_t srccnst0 = m_db.src_coef_const0;
		uint32_t srccnst1 = m_db.src_coef_const1;

		if ((m_db.usergen_ctrl & DBUSERGENCTL_BLENDEN) && !(m_gc.te_master_mode & TEMASTER_MODE_DBLEND))
		{
			// Blue
			dm10 = txtcnst0 & 0xff; txtcnst0 >>= 8;
			dm11 = txtcnst1 & 0xff; txtcnst1 >>= 8;
			dm20 = srccnst0 & 0xff; srccnst0 >>= 8;
			dm21 = srccnst1 & 0xff; srccnst1 >>= 8;
			m_dbstate.blend.b = color_blend(m_dbstate.texpath.b, m_dbstate.ti.b, m_dbstate.srcpath.b, m_dbstate.src.b, dm10, dm11, dm20, dm21);

			// TODO: ALURGEL for each

			// Green
			dm10 = txtcnst0 & 0xff; txtcnst0 >>= 8;
			dm11 = txtcnst1 & 0xff; txtcnst1 >>= 8;
			dm20 = srccnst0 & 0xff; srccnst0 >>= 8;
			dm21 = srccnst1 & 0xff; srccnst1 >>= 8;
			m_dbstate.blend.g = color_blend(m_dbstate.texpath.g, m_dbstate.ti.g, m_dbstate.srcpath.g, m_dbstate.src.g, dm10, dm11, dm20, dm21);

			// Red
			dm10 = txtcnst0 & 0xff;
			dm11 = txtcnst1 & 0xff;
			dm20 = srccnst0 & 0xff;
			dm21 = srccnst1 & 0xff;
			m_dbstate.blend.r = color_blend(m_dbstate.texpath.r, m_dbstate.ti.r, m_dbstate.srcpath.r, m_dbstate.src.r, dm10, dm11, dm20, dm21);
		}
		else
		{
			m_dbstate.blend.r = m_dbstate.ti.r;
			m_dbstate.blend.g = m_dbstate.ti.g;
			m_dbstate.blend.b = m_dbstate.ti.b;
		}

		// Dithering
		if (m_db.usergen_ctrl & DBUSERGENCTL_DITHEREN)
		{
			uint32_t dith_x = x & 3;
			uint32_t dith_y = y & 3;

			uint32_t dith_mtx = dith_y & 2 ? m_db.dither_mat_b : m_db.dither_mat_a;
			uint32_t idx = 7 ^ (((dith_y & 1) << 2) | dith_x);
			uint8_t val = (dith_mtx >> (idx * 4)) & 0xf;

			m_dbstate.dst.r = dither(m_dbstate.blend.r, val);
			m_dbstate.dst.g = dither(m_dbstate.blend.g, val);
			m_dbstate.dst.b = dither(m_dbstate.blend.b, val);
		}
		else
		{
			m_dbstate.dst.r = m_dbstate.blend.r;
			m_dbstate.dst.g = m_dbstate.blend.g;
			m_dbstate.dst.b = m_dbstate.blend.b;
		}
	}


	if (!(m_gc.te_master_mode & TEMASTER_MODE_DZBUF) &&
		(m_db.usergen_ctrl & DBUSERGENCTL_ZBUFEN))
	{
		int32_t x_offs = (m_db.z_offset & DBZOFFS_XOFFS_MASK) >> DBZOFFS_XOFFS_SHIFT;
		int32_t y_offs = (m_db.z_offset & DBZOFFS_YOFFS_MASK) >> DBZOFFS_YOFFS_SHIFT;

		// Sign extend
		x_offs = (x_offs << 20) >> 20;
		y_offs = (x_offs << 20) >> 20;

		x_offs += m_dbstate.x;
		y_offs += m_dbstate.y;

		uint32_t x_clip = (m_db.z_clip & DBZCLIP_XCLIP_MASK) >> DBZCLIP_XCLIP_SHIFT;
		uint32_t y_clip = (m_db.z_clip & DBZCLIP_YCLIP_MASK) >> DBZCLIP_YCLIP_SHIFT;

		zclip = (x_offs < 0) || (y_offs < 0) || (x_offs >= x_clip) || (y_offs >= y_clip);

		zaddr = m_db.z_baseaddr + (x_offs + y_offs * m_db.dst_xstride) * 2;
	}
	else
	{
		zaddr = 0;
		zclip = false;
	}

	zclipdis = zclip && (m_db.discard_ctrl & DBDISCARDCTL_ZCLIPDISEN);

	if (zclipdis)
		m_db.status |= DBSTATUS_ZCLIP;

	// Z-path
	if (!(m_gc.te_master_mode & TEMASTER_MODE_DZBUF) &&
		(m_db.usergen_ctrl & DBUSERGENCTL_ZBUFEN) &&
		!zclip)
	{
#if TEST_TIMING
		g_statistics[STAT_ZBUFFER_LOADS]++;
#endif

#if 1 // TODO: Why are we using this?
		int32_t zdiff = w - m_bda->read_bus16(zaddr);
#else
		// Shift W back to 0.23
		uint16_t oldw = m_bda->read_bus16(zaddr);
		uint32_t exp = (oldw & 0xc000) >> 14;
		uint32_t man = (oldw & 0x3fff) << 17;

		uint32_t oldz = man >> (exp * 3);

		exp = (w & 0xc000) >> 14;
		man = (w & 0x3fff) << 17;
		uint32_t curz = man >> (exp * 3);

		int32_t zdiff = curz - oldz;
#endif

		// TODO: Why isn't this working?
		// W vs Z?
		if (zdiff > 0)
		{
			zpixout = m_db.z_ctrl & 1;//DBZCNTL_ZPIXOUT_GT;
			zbufout = m_db.z_ctrl & 2;//DBZCNTL_ZBUFOUT_GT;
			zgel |= 4;
			m_db.status |= DBSTATUS_ZFUNC_GT;
		}
		else if (zdiff == 0)
		{
			zpixout = m_db.z_ctrl & DBZCNTL_ZPIXOUT_EQ;
			zbufout = m_db.z_ctrl & DBZCNTL_ZBUFOUT_EQ;
			zgel |= 2;
			m_db.status |= DBSTATUS_ZFUNC_EQ;
		}
		else
		{
			zpixout = m_db.z_ctrl & 0x10;//DBZCNTL_ZPIXOUT_LT;
			zbufout = m_db.z_ctrl & 0x20;//DBZCNTL_ZBUFOUT_LT;
			zgel |= 1;
			m_db.status |= DBSTATUS_ZFUNC_LT;
		}
	}
	else
	{
		zpixout = true;
		zbufout = false;
	}

	// Discard logic
	{
		bool ssbdis = (m_db.discard_ctrl & DBDISCARDCTL_SSBDISEN) && (m_dbstate.ssb == 0);

		bool adis = (m_db.discard_ctrl & DBDISCARDCTL_ADISEN) && (m_dbstate.dst.a == 0);

		bool rgbdis = (m_db.discard_ctrl & DBDISCARDCTL_RGBDISEN) &&
					(m_dbstate.dst.r == 0) && (m_dbstate.dst.g == 0) && (m_dbstate.dst.b == 0);

		dis = fbclipdis || winclipdis || zclipdis || ssbdis || adis || rgbdis;
	}

	// Write output depth and color
	if (!dis)
	{
		// Z-buffer
		if (zbufout && (m_db.usergen_ctrl & DBUSERGENCTL_ZOUTEN))
		{
#if TEST_TIMING
			g_statistics[STAT_ZBUFFER_STORES]++;
#endif
			m_bda->write_bus16(zaddr, w & 0xffff);
		}

		// Color
		if (zpixout && (m_db.supergen_ctrl & DBSUPERGENCTL_DESTOUTEN))
		{
			write_dst_pixel();
		}
	}

	// TODO: Status
/*
    {
        fbClipStat = fbClipDis
        winClipStat = winClipDis
        zClipStat = zClipDis
        alurstat
        alugstat
        alubstat
        zFuncStat
        anyRender
        set_interrupt
    }
*/
}

// Select between texture unit and source pixel
void m2_te_device::select_tex_pixel()
{
	uint32_t cntl;

	// TODO: REGBITS
	if (m_dbstate.ti.a == 0)
		cntl = (m_db.src_alpha_ctrl >> 4) & 3;
	else if (m_dbstate.ti.a == 255)
		cntl = m_db.src_alpha_ctrl & 3;
	else
		cntl = (m_db.src_alpha_ctrl >> 2) & 3;

	switch (cntl)
	{
		case 0: m_dbstate.texpath.a = m_dbstate.ti.a;   break;
		case 1: m_dbstate.texpath.a = 255;              break;
		case 2: m_dbstate.texpath.a = 0;                break;
	}

	switch ((m_db.txt_mult_cntl & DBTXTMULTCNTL_INSEL_MASK) >> DBTXTMULTCNTL_INSEL_SHIFT)
	{
		case DBTXTMULTCNTL_INSEL_CTI:
		{
			m_dbstate.texpath.r = m_dbstate.ti.r;
			m_dbstate.texpath.g = m_dbstate.ti.g;
			m_dbstate.texpath.b = m_dbstate.ti.b;
			break;
		}
		case DBTXTMULTCNTL_INSEL_CONSTANT:
		{
			uint32_t cnst = m_db.const_in;
			m_dbstate.texpath.r = (cnst >> 16) & 0xff;
			m_dbstate.texpath.g = (cnst >> 8) & 0xff;
			m_dbstate.texpath.b = cnst & 0xff;
			break;
		}
		case DBTXTMULTCNTL_INSEL_COMPSRC:
		{
			m_dbstate.texpath.r = ~m_dbstate.src.r;
			m_dbstate.texpath.g = ~m_dbstate.src.g;
			m_dbstate.texpath.b = ~m_dbstate.src.b;
			break;
		}
		case DBTXTMULTCNTL_INSEL_ATI:
		{
			m_dbstate.texpath.r = m_dbstate.ti.a;
			m_dbstate.texpath.g = m_dbstate.ti.a;
			m_dbstate.texpath.b = m_dbstate.ti.a;
			break;
		}
	}

	if (m_db.txt_mult_cntl & DBTXTMULTCNTL_TXTRJUST)
	{
		m_dbstate.texpath.r >>= 3;
		m_dbstate.texpath.g >>= 3;
		m_dbstate.texpath.b >>= 3;
	}
}

void m2_te_device::select_src_pixel()
{
	if ((m_db.usergen_ctrl & DBUSERGENCTL_SRCINEN)
		&& (m_db.usergen_ctrl & DBUSERGENCTL_BLENDEN)
		&& !(m_gc.te_master_mode & TEMASTER_MODE_DBLEND))
	{
		int32_t x_offs = (m_db.src_offset & DBSRCOFFS_XOFFS_MASK) >> DBSRCOFFS_YOFFS_SHIFT;
		int32_t y_offs = (m_db.src_offset & DBSRCOFFS_YOFFS_MASK) >> DBSRCOFFS_YOFFS_SHIFT;

		// Sign extend
		x_offs = (x_offs << 20) >> 20;
		y_offs = (y_offs << 20) >> 20;

		x_offs += m_dbstate.x;
		y_offs += m_dbstate.y;

		uint32_t addr = y_offs * m_db.src_xstride + x_offs;

		if (m_db.src_ctrl & DBSRCCNTL_32BPP)
		{
			uint32_t srcaddr = m_db.src_baseaddr + addr * sizeof(uint32_t);
			uint32_t srcval = m_bda->read_bus32(srcaddr);

			m_dbstate.src.a = ((srcval >> 24) & 0x7f) << 1;
			m_dbstate.src.r = (srcval >> 16) & 0xff;
			m_dbstate.src.g = (srcval >>  8) & 0xff;
			m_dbstate.src.b = (srcval >>  0) & 0xff;

			if (m_db.src_ctrl & DBSRCCNTL_MSBREP)
				m_dbstate.src.a |= (m_dbstate.src.a >> 6) & 1;

			m_dbstate.dsb = (srcval >> 31) & 1;
		}
		else
		{
			uint32_t srcaddr = m_db.src_baseaddr + addr * sizeof(uint16_t);
			uint32_t srcval = m_bda->read_bus16(srcaddr);

			m_dbstate.src.r = (srcval >> 10) & 0x1f;
			m_dbstate.src.g = (srcval >> 5) & 0x1f;
			m_dbstate.src.b = srcval & 0x1f;
			m_dbstate.src.a = 0;

			m_dbstate.dsb = srcval & 0x8000;
			m_dbstate.src.r <<= 3;
			m_dbstate.src.g <<= 3;
			m_dbstate.src.b <<= 3;

			if (m_db.src_ctrl & DBSRCCNTL_MSBREP)
			{
				m_dbstate.src.r |= (m_dbstate.src.r >> 5);
				m_dbstate.src.g |= (m_dbstate.src.g >> 5);
				m_dbstate.src.b |= (m_dbstate.src.b >> 5);
			}
		}
#if TEST_TIMING
		g_statistics[STAT_PIXEL_LOADS]++;
#endif
	}
	else
	{
		// Source input disabled
		m_dbstate.dsb = 0;
		m_dbstate.src.r = 0;
		m_dbstate.src.g = 0;
		m_dbstate.src.b = 0;
		m_dbstate.src.a = 0;
	}

	// Now
	switch ((m_db.src_mult_cntl & DBSRCMULTCNTL_INSEL_MASK) >> DBSRCMULTCNTL_INSEL_SHIFT)
	{
		case DBSRCMULTCNTL_INSEL_SRC:
		{
			m_dbstate.srcpath.r = m_dbstate.src.r;
			m_dbstate.srcpath.g = m_dbstate.src.g;
			m_dbstate.srcpath.b = m_dbstate.src.b;
			break;
		}
		case DBSRCMULTCNTL_INSEL_CONSTANT:
		{
			uint32_t cnst = m_db.const_in;
			m_dbstate.srcpath.r = (cnst >> 16) & 0xff;
			m_dbstate.srcpath.g = (cnst >> 8) & 0xff;
			m_dbstate.srcpath.b = (cnst >> 0) & 0xff;
			break;
		}
		case DBSRCMULTCNTL_INSEL_COMPCTI:
		{
			m_dbstate.srcpath.r = ~m_dbstate.ti.r;
			m_dbstate.srcpath.g = ~m_dbstate.ti.g;
			m_dbstate.srcpath.b = ~m_dbstate.ti.b;
			break;
		}
		case DBSRCMULTCNTL_INSEL_TEXALPHA:
		{
			m_dbstate.srcpath.r = m_dbstate.src.a;
			m_dbstate.srcpath.g = m_dbstate.src.a;
			m_dbstate.srcpath.b = m_dbstate.src.a;
			break;
		}
	}

	if (m_db.src_mult_cntl & DBSRCMULTCNTL_SRCRJUST)
	{
		m_dbstate.srcpath.r >>= 3;
		m_dbstate.srcpath.g >>= 3;
		m_dbstate.srcpath.b >>= 3;
	}

	m_dbstate.srcpath.a = m_dbstate.src.a;
}

uint8_t m2_te_device::dither(uint8_t in, uint8_t dithval)
{
	int32_t res;
	int32_t sgn_val;

	if (dithval & 8)
		sgn_val = -8 + (dithval & 7);
	else
		sgn_val = dithval;

	res = (int32_t)in + sgn_val;

	if (res > 255)
		res = 255;
	else if (res < 0)
		res = 0;

	return (uint8_t)res;
}

uint8_t m2_te_device::get_src_coef(uint8_t cti, uint8_t dm2const0, uint8_t dm2const1)
{
	uint32_t sel=0;
	uint8_t cnst, coef=0;

	switch ((m_db.src_mult_cntl & DBSRCMULTCNTL_SRCCONSTCNTL_MASK) >> DBSRCMULTCNTL_SRCCONSTCNTL_SHIFT)
	{
		case DBSRCMULTCNTL_SRCCONSTCNTL_TEXSSB: sel = m_dbstate.ssb;    break;
		case DBSRCMULTCNTL_SRCCONSTCNTL_SRCDSB: sel = m_dbstate.dsb;    break;
	}

	cnst = sel ? dm2const1 : dm2const0;

	switch ((m_db.src_mult_cntl & DBSRCMULTCNTL_COEFSEL_MASK) >> DBSRCMULTCNTL_COEFSEL_SHIFT)
	{
		case DBSRCMULTCNTL_COEFSEL_ATI:         coef = m_dbstate.texpath.a;     break;
		case DBSRCMULTCNTL_COEFSEL_ASRC:        coef = m_dbstate.srcpath.a;     break;
		case DBSRCMULTCNTL_COEFSEL_CONSTANT:    coef = cnst;                    break;
		case DBSRCMULTCNTL_COEFSEL_CTI:         coef = cti;                     break;
	}

	if (m_db.src_mult_cntl & DBSRCMULTCNTL_SRCCOEFCMP)
		return ~coef;
	else
		return coef;
}

uint8_t m2_te_device::get_tex_coef(uint8_t cs, uint8_t dm1const0, uint8_t dm1const1)
{
	uint32_t sel=0;
	uint8_t cnst, coef=0;

	// TODO: Make like src coefficient
	uint32_t cntl = ((m_db.txt_mult_cntl & DBTXTMULTCNTL_TXTCONSTCNTL_MASK) >> DBTXTMULTCNTL_TXTCONSTCNTL_SHIFT);

	if (cntl == DBTXTMULTCNTL_TXTCONSTCNTL_TEXSSB)
		sel = m_dbstate.ssb;
	else if (cntl == DBTXTMULTCNTL_TXTCONSTCNTL_SRCDSB)
		sel = m_dbstate.dsb;

	cnst = sel ? dm1const1 : dm1const0;

	switch ((m_db.txt_mult_cntl & DBTXTMULTCNTL_COEFSEL_MASK) >> DBTXTMULTCNTL_COEFSEL_SHIFT)
	{
		case DBTXTMULTCNTL_COEFSEL_ATI:         coef = m_dbstate.texpath.a;     break;
		case DBTXTMULTCNTL_COEFSEL_ASRC:        coef = m_dbstate.srcpath.a;     break;
		case DBTXTMULTCNTL_COEFSEL_CONSTANT:    coef = cnst;                    break;
		case DBTXTMULTCNTL_COEFSEL_CSRC:        coef = cs;                      break;
	}

	if (m_db.txt_mult_cntl & DBTXTMULTCNTL_TXTCOEFCMP)
		return ~coef;
	else
		return coef;
}

void m2_te_device::select_alpha_dsb()
{
	if ((m_db.usergen_ctrl & DBUSERGENCTL_BLENDEN) && !(m_gc.te_master_mode & TEMASTER_MODE_DBLEND))
	{
		uint32_t sel = 0;
		uint8_t aconst;

		switch ((m_db.dst_alpha_ctrl & DBDSTACNTL_ADESTCONSTCNTL_MASK) >> DBDSTACNTL_ADESTCONSTCNTL_SHIFT)
		{
			case 0: sel = m_dbstate.ssb;    break;
			case 1: sel = m_dbstate.dsb;    break;
		}

		if (sel)
			aconst = (m_db.dst_alpha_const & DBDSTALPHACONST_CONST1_MASK) >> DBDSTALPHACONST_CONST1_SHIFT;
		else
			aconst = (m_db.dst_alpha_const & DBDSTALPHACONST_CONST0_MASK) >> DBDSTALPHACONST_CONST0_SHIFT;

		switch ((m_db.dst_alpha_ctrl & DBDSTACNTL_ADESTSEL_MASK) >> DBDSTACNTL_ADESTSEL_SHIFT)
		{
			case 0: m_dbstate.dst.a = m_dbstate.texpath.a;  break;
			case 1: m_dbstate.dst.a = aconst;               break;
			case 2: m_dbstate.dst.a = m_dbstate.srcpath.a;  break;
			case 3: m_dbstate.dst.a = m_dbstate.blend.r;    break;
		}

		switch ((m_db.ssbdsb_ctrl & DBSSBDSBCNTL_DSBSEL_MASK) >> DBSSBDSBCNTL_DSBSEL_SHIFT)
		{
			case 0: m_dbstate.dsb = m_dbstate.ssb;      break;
			case 1: m_dbstate.dsb = (m_db.ssbdsb_ctrl & DBSSBDSBCNTL_DSBCONST_MASK) >> DBSSBDSBCNTL_DSBCONST_SHIFT; break;
			case 2: m_dbstate.dsb = m_dbstate.dsb;      break;
		}
	}
	else
	{
		m_dbstate.dst.a = m_dbstate.texpath.a;
		m_dbstate.dsb = m_dbstate.ssb;
	}
}


uint8_t m2_te_device::color_blend(uint8_t ct, uint8_t cti, uint8_t cs, uint8_t csrc,
								uint8_t dm10, uint8_t dm11,
								uint8_t dm20, uint8_t dm21)
{
	uint8_t tcoef, scoef;
	uint16_t tm, sm;

	tcoef = get_tex_coef(csrc, dm10, dm11);
	scoef = get_src_coef(cti, dm20, dm21);

	tm = (tcoef == 255) ? ct : ((ct == 255) ? tcoef : ((tcoef * ct) >> 8));
	sm = (scoef == 255) ? cs : ((cs == 255) ? scoef : ((scoef * cs) >> 8));

	return alu_calc(tm, sm);
}

#if 1

uint8_t m2_te_device::alu_calc(uint16_t a, uint16_t b)
{
	int32_t result = 0;
	uint32_t blendout;
	uint32_t carry = 0;
	uint32_t borrow = 0;
	uint32_t cntl = (m_db.alu_ctrl & DBALUCNTL_ALUOP_MASK) >> DBALUCNTL_ALUOP_SHIFT;

	// p271

	/* ALU */
	if ((cntl & 8) == 0)
	{
		if (!(cntl & 4))
		{
			result = a + b;
		}
		else if (cntl & 2)
		{
			result = b - a;
			if (result < 0)
				borrow = 1;
		}
		else
		{
			result = a - b;
			if (result < 0)
				borrow = 1;
		}
	}
	/* Boolean */
	else
	{
		int i, j;

		for (i = 0; i < 8; ++i)
		{
			j = (a & 1) *2 + (b & 1);

			result >>= 1;
			result |= (cntl >> j & 1) ? 0x80 : 0;

			a >>= 1;
			b >>= 1;
		}
	}

	result &= 0x1ff;

	switch ((m_db.alu_ctrl & DBALUCNTL_FINALDIVIDE_MASK) >> DBALUCNTL_FINALDIVIDE_SHIFT)
	{
		case 1:  blendout = result << 1;    break;
		case 2:  blendout = result << 2;    break;
		case 3:  blendout = result << 3;    break;
		case 7:  blendout = result >> 1;    break;
		case 6:  blendout = result >> 2;    break;
		case 5:  blendout = result >> 3;    break;
		default: blendout = result;         break;
	}

	if (blendout > 255)
		carry = 1;

#if 0
	int alugel;
	// TODO: FIX ALUGEL - Needs to be propagated to ALU status
	if (borrow == 1)
		alugel = 1;
	else if (blendout == 0)
		alugel = 2;
	else
		alugel = 4;
#endif

	if ((cntl & 8) == 0)
	{
		/* Clamp? */
		if (!(cntl & 1))
		{
			blendout = carry ? 255 : blendout;
			blendout = borrow ? 0 : blendout;
		}
		else
		{
			if (cntl & 2)
			{
				blendout = carry ? 255 : blendout;
				blendout = borrow ? 0 : blendout;
			}
		}
	}

	return blendout & 0xff;
}



#endif

//-------------------------------------------------
//  walk_span -
//-------------------------------------------------

void m2_te_device::walk_span(uint32_t wrange, bool omit_right,
							 uint32_t y, uint32_t xs, uint32_t xe,
							 int32_t r, int32_t g, int32_t b, int32_t a,
							 uint32_t uw, uint32_t vw,
							 uint32_t w)
{
	bool scan_lr = !m_es.r2l;

	// TODO: Is this correct?
	xe = scan_lr ? xe + 1 : xe - 1;

	/*
	    Edge to Span walker
	    X/Y         11.0
	    RGBA        8.11
	    U/W, V/W    10.13
	    1/W         0.23
	    RGBA DDX    s8.11
	    UV DDX      s10.13
	    1/W DDX     s0.23

	    To destination blender:
	    X/Y         11.0
	    W           0.16

	    To texture mapper:
	    RGBA        8.0
	    UV          10.4
	*/

	if (omit_right)
	{
		if (scan_lr)
		{
			xe -= 1;
		}
		else
		{
			xs -= 1;
			r -= m_es.ddx_r;
			g -= m_es.ddx_g;
			b -= m_es.ddx_b;
			a -= m_es.ddx_a;
			uw -= m_es.ddx_uw;
			vw -= m_es.ddx_vw;
			w -= m_es.ddx_w;
		}
	}

	if (g_debug)
	{
		g_debug = true;
	}

	while (xs != xe)
	{
		uint32_t sx = xs;
		uint32_t sy = y;

#if 1 // DEBUG
		if (sx == 320/2 && sy == 200)
		{
			;
		}
#endif

		// Fetch RGB and A
		uint32_t rt, gt, bt, at, ssbt;

		uint32_t w16 = 0;

		// W
		{
			// Undo shifting that took place during setup and reduce 0.23 to 0.16
			uint32_t normalized_w = w >> -depth_bits[wrange];
			w16 = (normalized_w >> 7) & 0xffff;
		}

		if (!(m_gc.te_master_mode & TEMASTER_MODE_DTEXT) && (m_tm.tex_addr_cntl & TXTADDRCNTL_LOOKUP_EN))
		{
			uint32_t u, v;

			// UV and W
			texcoord_gen(wrange, uw, vw, w, u, v, w16);

			// TODO: FIXME
			uint32_t lod = lod_calc(u, v, u, v);

			get_texture_color(u, v, lod, rt, gt, bt, at, ssbt);
		}
		else
		{
			rt = gt = bt = at = ssbt = 0;
		}

		uint32_t ri, gi, bi, ai;

		if (!(m_gc.te_master_mode & TEMASTER_MODE_DSHADE))
		{
			// Remove the fractional parts of iterated ARGB
			ri = r >> 11;
			gi = g >> 11;
			bi = b >> 11;
			ai = a >> 11;
		}
		else
		{
			ri = 0xff;
			gi = 0xff;
			bi = 0xff;
			ai = 0xff;
		}

		uint32_t ro, go, bo, ao;

		// Note: SSB may be overriden
		uint32_t ssbo = ssbt;

		// Blend iterated RGB with texel
		texture_blend(ri, gi, bi, ai,
					  rt, gt, bt, at, ssbt,
					  ro, go, bo, ao, ssbo);

		// Destination blend and write-out
		// Interface:
		// Span length
		// Left 2 Right
		// W0, W1

		rgba texout;
		texout.r = ro;
		texout.g = go;
		texout.b = bo;
		texout.a = ao;

		destination_blend(sx, sy, w16, texout, ssbo);

		// Update interpolated paramters
		if (scan_lr)
		{
			xs += 1;
			r += m_es.ddx_r;
			g += m_es.ddx_g;
			b += m_es.ddx_b;
			a += m_es.ddx_a;
			uw += m_es.ddx_uw;
			vw += m_es.ddx_vw;
			w += m_es.ddx_w;
		}
		else
		{
			xs -= 1;
			r -= m_es.ddx_r;
			g -= m_es.ddx_g;
			b -= m_es.ddx_b;
			a -= m_es.ddx_a;
			uw -= m_es.ddx_uw;
			vw -= m_es.ddx_vw;
			w -= m_es.ddx_w;
		}

		// Clamp to 11.8
		r = std::clamp<int32_t>(r, 0, 0x0007ffff);
		g = std::clamp<int32_t>(g, 0, 0x0007ffff);
		b = std::clamp<int32_t>(b, 0, 0x0007ffff);
		a = std::clamp<int32_t>(a, 0, 0x0007ffff);

		// Clamp to 10.13
		uw = std::clamp<int32_t>(uw, 0, 0x007fffff);
		vw = std::clamp<int32_t>(vw, 0, 0x007fffff);

		// Clamp to 0.23
		w = std::clamp<int32_t>(w, 0, 0x007fffff);

#if TEST_TIMING
		g_statistics[STAT_PIXELS_PROCESSED]++;
#endif
	}
}



/***************************************************************************
    INSTRUCTION PROCESSING
***************************************************************************/

//-------------------------------------------------
//  irp_fetch -
//-------------------------------------------------

uint32_t m2_te_device::irp_fetch()
{
	uint32_t data = m_bda->read_bus32(m_gc.irp);
	m_gc.irp += 4;

	return data;
}


//-------------------------------------------------
//  irp_fetch_float -
//-------------------------------------------------

float m2_te_device::irp_fetch_float()
{
	uint32_t data = m_bda->read_bus32(m_gc.irp);
	m_gc.irp += 4;

	return *reinterpret_cast<float*>(&data);
}


//-------------------------------------------------
//  illegal_inst -
//-------------------------------------------------

void m2_te_device::illegal_inst()
{
	set_interrupt(INTSTAT_UNIMPLEMENTED_INSTR);
	m_state = TE_STOPPED;
}


//-------------------------------------------------
//  execute -
//-------------------------------------------------

void m2_te_device::execute()
{
#if TEST_TIMING
	memset(g_statistics, 0, sizeof(g_statistics));
#endif

	while (m_state == TE_RUNNING)
	{
		uint32_t inst = irp_fetch();

		switch (inst & INST_MASK)
		{
			case INST_WRITE_REG:
			{
				uint32_t offs = inst & 0xffff;
				int32_t cnt = (inst >> 16) & 0xff;

				while (cnt-- >= 0)
				{
					write(offs >> 2, irp_fetch());
					offs += 4;

					if (m_state != TE_RUNNING)
						break;
				}
				break;
			}

			case INST_VTX_SHORT:
			{
				int32_t cnt = inst & 0xffff;
				uint32_t flags = inst & 0x001f0000;
				uint32_t ver = (inst >> 24) & 0xf;

				if (ver != 0)
				{
					illegal_inst();
					break;
				}

				while (cnt-- >= 0)
				{
					se_vtx vtx = { 0 };

					vtx.x = irp_fetch_float();
					vtx.y = irp_fetch_float();

					if (flags & VTX_FLAG_SHAD)
					{
						vtx.r = irp_fetch_float();
						vtx.g = irp_fetch_float();
						vtx.b = irp_fetch_float();
						vtx.a = irp_fetch_float();
					}
					if (flags & VTX_FLAG_PRSP)
					{
						vtx.w = irp_fetch_float();
					}
					if (flags & VTX_FLAG_TEXT)
					{
						vtx.uw = irp_fetch_float();
						vtx.vw = irp_fetch_float();
					}

					// Send this vertex for processing
					add_vertex(vtx, flags);

					// Clear the new triangle flag if set
					flags &= ~VTX_FLAG_NEW;
				}

				break;
			}
			case INST_VTX_LONG:
			{
				const uint32_t ver = (inst >> 24) & 0xf;

				if (ver != 0)
				{
					illegal_inst();
					break;
				}

				fatalerror("Long format unimplemented");

				break;
			}
			case INST_VTX_POINT:
			{
				const uint32_t ver = (inst >> 24) & 0xf;

				if (ver != 0)
				{
					illegal_inst();
					break;
				}

				fatalerror("Point format unimplemented");

				break;
			}
			default:
			{
				illegal_inst();
				break;
			}
		}

		// Stop or pause execution?
		if (m_gc.irp == m_gc.iwp)
		{
			m_state = m_gc.te_master_mode & TEICNTL_STPL ? TE_STOPPED : TE_PAUSED;
			set_interrupt(INTSTAT_LIST_END);
		}
		else if (m_gc.te_master_mode & (TEICNTL_STPI | TEICNTL_STEP))
		{
			m_state = TE_STOPPED;
		}
	};

#if TEST_TIMING
/*
    TESetup Engine: 600-700 triangles/sec (?)

    Pixel Rates:
    Point - 132Mpix/s
    Linear - 66Mpix/s
    Bilin - 33Mpix
    QTril - 22M

    No blend, no Z - 120MPix
    Zbuffer - 66-120M pix
    Bend - 66 M pix
*/

	uint32_t total_cycles = (g_statistics[STAT_TRIANGLES_PROCESSED] * 100) +
							(g_statistics[STAT_TEXEL_READS]) +
//                          (g_statistics[STAT_PIXELS_PROCESSED]) +
							(g_statistics[STAT_PIXEL_LOADS]) +
							(g_statistics[STAT_PIXEL_STORES]) +
							(g_statistics[STAT_TEXEL_BYTES]/4) +
							(g_statistics[STAT_ZBUFFER_LOADS]) +
							(g_statistics[STAT_ZBUFFER_STORES]);
#if 0
	logerror(">>> END OF LIST <<<\n");
	logerror("Triangles: %u\n", g_statistics[STAT_TRIANGLES_PROCESSED]);
	logerror("Texture samples: %u\n", g_statistics[STAT_TEXEL_READS]);
	logerror("Texture bytes loaded: %u\n", g_statistics[STAT_TEXEL_BYTES]);
	logerror("Pixels rasterized: %u\n", g_statistics[STAT_PIXELS_PROCESSED]);
	logerror("Pixel reads: %u\n", g_statistics[STAT_PIXEL_LOADS]);
	logerror("Pixel writes: %u\n", g_statistics[STAT_PIXEL_STORES]);
	logerror("Z reads: %u\n", g_statistics[STAT_ZBUFFER_LOADS]);
	logerror("Z writes: %u\n", g_statistics[STAT_ZBUFFER_STORES]);
	logerror("Total: %u cycles (%fusec)\n", total_cycles, clocks_to_attotime(total_cycles).as_double()*1.0e6);
#endif
	m_done_timer->adjust(clocks_to_attotime(total_cycles));
#else
	// Interrupt after stopping?
	if (m_gc.te_master_mode & TEICNTL_INT)
	{
		set_interrupt(INTSTAT_IMMEDIATE_INSTR);
	}
#endif
}



/***************************************************************************
    TEXTURE RAM ACCESSORS
***************************************************************************/

uint8_t m2_te_device::read_tram8(offs_t address) const
{
	address &= TEXTURE_RAM_BYTEMASK;

	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_tram[0]) + BYTE4_XOR_BE(address);
	return *ptr;
}

uint16_t m2_te_device::read_tram16(offs_t address) const
{
	address &= TEXTURE_RAM_BYTEMASK;

	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_tram[0]) + address;//WORD_XOR_BE(address);
	return *reinterpret_cast<uint16_t *>(ptr);
}

uint32_t m2_te_device::read_tram32(offs_t address) const
{
	address &= TEXTURE_RAM_BYTEMASK;

	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_tram[0]) + address;//DWORD_XOR_BE(address);
	return *reinterpret_cast<uint32_t *>(ptr);
}

void m2_te_device::write_tram8(offs_t address, uint8_t data)
{
	assert(address <= TEXTURE_RAM_WORDS * 4);

	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_tram[0]) + address;//BYTE8_XOR_BE(address);
	*ptr = data;
}

void m2_te_device::write_tram16(offs_t address, uint16_t data)
{
	assert(address < TEXTURE_RAM_WORDS * 4);

	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_tram[0]) + WORD2_XOR_BE(address);
	*reinterpret_cast<uint16_t *>(ptr) = data;
}

void m2_te_device::write_tram32(offs_t address, uint32_t data)
{
	address &= TEXTURE_RAM_BYTEMASK;

	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_tram[0]) + address;//DWORD_XOR_BE(address);
	*reinterpret_cast<uint32_t *>(ptr) = data;
}



/***************************************************************************
    PIP RAM ACCESSORS
***************************************************************************/

uint8_t m2_te_device::read_pipram8(offs_t address) const
{
	address &= PIP_RAM_BYTEMASK;

	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_pipram[0]) + address;//BYTE4_XOR_BE(address);
	return *ptr;
}

uint16_t m2_te_device::read_pipram16(offs_t address) const
{
	address &= PIP_RAM_BYTEMASK;

	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_pipram[0]) + address;//WORD_XOR_BE(address);
	return *reinterpret_cast<uint16_t *>(ptr);
}

uint32_t m2_te_device::read_pipram32(offs_t address) const
{
	address &= PIP_RAM_BYTEMASK;

	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_pipram[0]) + address;//DWORD_XOR_BE(address);
	return *reinterpret_cast<uint32_t *>(ptr);
}

void m2_te_device::write_pipram32(offs_t address, uint32_t data)
{
	address &= PIP_RAM_BYTEMASK;

	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_pipram[0]) + address;//DWORD_XOR_BE(address);
	*reinterpret_cast<uint32_t *>(ptr) = data;
}


// For extracting bit fields from RAM
uint32_t m2_te_device::readbits_from_ram(uint32_t & src_addr, uint32_t & bit_offs, uint32_t bits)
{
	uint32_t src_val = m_bda->read_bus32(src_addr);

	if (((bit_offs & 31) + bits) > 32)
	{
		uint32_t src_val2 = m_bda->read_bus32(src_addr + 4);

		uint32_t r_bits = ((bit_offs & 31) + bits) - 32;
		uint32_t l_bits = bits - r_bits;

		src_val &= (1 << l_bits) - 1;
		src_val <<= r_bits;

		// Position the right hand side
		src_val2 >>= (32 - r_bits);
		src_val2 &= (1 << r_bits) - 1;

		src_val |= src_val2;
	}
	else
	{
		src_val = src_val >> (32 - (bits + (bit_offs & 31)));
	}

	return src_val &= (1 << bits) - 1;
}

void m2_te_device::load_texture()
{
	switch (m_tm.texld_cntl & TXTLDCNTL_LDMODE_MASK)
	{
		case TXTLDCNTL_LDMODE_TEXLOAD:
		{
			if (m_tm.texld_cntl & TXTLDCNTL_COMPRESSED)
			{
				// Source address is byte aligned
				uint32_t src_addr = m_tm.texld_srcbase;
				uint32_t bit_offs = m_tm.texld_cntl & TXTLDCNTL_SRCBITOFFS;

				// Destination address is dword-aligned
				uint32_t dst_addr = m_tm.texld_dstbase << 3;
				int32_t texels = m_tm.tex_bytecnt;

				uint32_t dst_type = m_tm.tex_exptype;
				uint32_t dst_cdepth = (dst_type & TXTEXPFORM_CDEPTH_MASK) >> TXTEXPFORM_CDEPTH_SHIFT;
				uint32_t dst_adepth = (dst_type & TXTEXPFORM_ADEPTH_MASK) >> TXTEXPFORM_ADEPTH_SHIFT;
				uint32_t dst_bits = 0;

				if (dst_type & TXTEXPFORM_COLORON)
				{
					if (dst_type & TXTEXPFORM_LITERAL)
						dst_bits += dst_cdepth * 3;
					else
						dst_bits += dst_cdepth;
				}

				if (dst_type & TXTEXPFORM_SSBON)
					++dst_bits;

				if (dst_bits & TXTEXPFORM_ALPHAON)
					dst_bits += dst_adepth;

				while (texels > 0)
				{
					uint8_t cntl = readbits_from_ram(src_addr, bit_offs, 8);

					uint32_t type = (cntl >> 6) & 3;
					uint32_t cnst;

					// Select constant register
					if (type == 0)
						cnst = m_tm.tex_srcconst0;
					else if (type == 1)
						cnst = m_tm.tex_srcconst1;
					else if (type == 2)
						cnst = m_tm.tex_srcconst2;
					else
						cnst = m_tm.tex_srcconst3;

					// Select pixel type register
					uint32_t src_type = type & 2 ? m_tm.tex_srctype23 : m_tm.tex_srctype01;

					if (type & 1)
						src_type = src_type >> 16;

					const bool has_color = src_type & TXTEXPFORM_COLORON;
					const bool has_alpha = src_type & TXTEXPFORM_ALPHAON;
					const bool has_ssb = src_type & TXTEXPFORM_SSBON;
					const bool is_transparent = src_type & TXTEXPFORM_TRANSPARENT;
					const bool is_literal = src_type & TXTEXPFORM_LITERAL;

					uint32_t cnt = (cntl & (is_transparent ? 0x3f : 0x1f)) + 1;

					uint32_t src_cdepth = (src_type & TXTEXPFORM_CDEPTH_MASK) >> TXTEXPFORM_CDEPTH_SHIFT;
					uint32_t src_adepth = (src_type & TXTEXPFORM_ADEPTH_MASK) >> TXTEXPFORM_ADEPTH_SHIFT;
					uint32_t src_bits = 0;

					if (is_literal)
						src_cdepth *= 3;

					uint32_t src_val = 0;

					if (is_transparent)
					{
						src_val = cnst;
					}
					else
					{
						src_bits += src_cdepth;

						if (has_ssb)
							++src_bits;

						if (has_alpha)
							src_bits += src_adepth;
					}

					// String
					if (!is_transparent && (cntl & 0x20))
					{
						for (uint32_t i = 0; i < cnt; ++i)
						{
							uint32_t dst_val = 0;
							uint32_t src_ssb = 0;
							uint32_t src_color = 0;
							uint32_t src_alpha = 0;
							uint32_t src_val = readbits_from_ram(src_addr, bit_offs, src_bits);

#if TEST_TIMING // FIXME
							g_statistics[STAT_TEXEL_BYTES] += 4;
#endif

							if (has_color)
							{
								src_color = src_val & ((1 << src_cdepth) - 1);
								src_val >>= src_cdepth;

								// Indexed format - add an offset
								if (!(src_type & TXTEXPFORM_INDEXON))
									src_color += cnst & 0xff;
							}

							if (has_alpha)
							{
								if (is_transparent)
								{
									src_alpha = (cnst >> 24) & ((1 << src_adepth) - 1);
								}
								else
								{
									src_alpha = src_val & ((1 << src_adepth) - 1);
									src_val >>= src_adepth;
								}
							}

							if (has_ssb)
							{
								if (is_transparent)
									src_ssb = (cnst >> 31) & 1;
								else
									src_ssb = src_val & 1;
							}

							// Now create the destination value
							dst_val = src_color
									| (src_alpha << src_cdepth)
									| (src_ssb << (src_cdepth + src_adepth));

							uint32_t oldval = m_tram[(dst_addr >> 3) >> 2];

							// Clear out the old value
							uint32_t shift = (32 - (dst_bits + (dst_addr & 31)));
							oldval &= ~(((1 << dst_bits) - 1) << shift);
							oldval |= dst_val << shift;

							m_tram[(dst_addr >> 3) >> 2] = oldval;

							//src_addr += src_bits;
							dst_addr += dst_bits;
						}
					}
					else
					{
						uint32_t dst_val;

						if (!is_transparent)
						{
							// Read one texel
							src_val = readbits_from_ram(src_addr, bit_offs, src_bits);

#if TEST_TIMING // FIXME
							g_statistics[STAT_TEXEL_BYTES] += 4;
#endif

							// Indexed format - add an offset
							if (!is_literal)
								src_val += cnst & 0xff;
						}

						// HACK: Bits don't match?
						dst_val = src_val;

						for (uint32_t i = 0; i < cnt; ++i)
						{
							uint32_t oldval = m_tram[(dst_addr >> 3) & ~3];

							// Clear out the old value
							uint32_t shift = (32 - (dst_bits + (dst_addr & 31)));
							oldval &= ~(((1 << dst_bits) - 1) << shift);
							oldval |= dst_val << shift;

							m_tram[(dst_addr >> 3) >> 2] = oldval;

							dst_addr += dst_bits;
						}

						// Copy
						//src_addr += src_bits;
					}

					texels -= cnt;
					// assert if negative?
				}
			}
			else
			{
				// Regular
				fatalerror("REGULAR TEXTURE NOT SUPPORTED\n");
			}

			break;
		}

		case TXTLDCNTL_LDMODE_MMDMA:
		{
			uint32_t srcaddr = m_tm.tex_mm_srcbase;
			uint32_t dstaddr = m_tm.tex_mm_dstbase;
			uint32_t bytes = m_tm.tex_bytecnt;

			// TODO: Do TRAM and PIPRAM only allow 32-bit accesses?
			assert((bytes & 3) == 0);

			if (m_tm.tex_cntl & TXTCNTL_MMDMA_TRAM_ON)
			{
				// TRAM destination must be 32-bit aligned
				assert((dstaddr & 3) == 0);

				while (bytes > 0)
				{
#if 0
					uint32_t data = m_bda->read_bus32(srcaddr);
					write_tram32(dstaddr, data);
					dstaddr += 4;
					srcaddr += 4;
					bytes -= 4;
#else
					uint32_t data = m_bda->read_bus8(srcaddr);
					write_tram8(dstaddr, data);
					dstaddr ++;
					srcaddr ++;
					bytes--;

#if TEST_TIMING
					g_statistics[STAT_TEXEL_BYTES]++;
#endif

#endif
				}
			}
			else if (m_tm.tex_cntl & TXTCNTL_MMDMA_PIP_ON)
			{
				// TODO: Should PIP destination be 32-bit aligned?
				assert((dstaddr & 3) == 0);

				while (bytes > 0)
				{
					uint32_t data = m_bda->read_bus32(srcaddr);
					write_pipram32(dstaddr, data);
					dstaddr += 4;
					srcaddr += 4;
					bytes -= 4;

#if TEST_TIMING
					g_statistics[STAT_TEXEL_BYTES] += 4;
#endif
				}
			}

			break;
		}

		case TXTLDCNTL_LDMODE_PIPLOAD:
		{
			// TODO: What is the difference between this and the MMDMA?
			uint32_t srcaddr = m_tm.texld_srcbase;
			uint32_t dstaddr = m_tm.texld_dstbase;
			uint32_t bytes = m_tm.tex_bytecnt;

			// Assuming byte count must be aligned?
			assert((m_tm.tex_bytecnt & 3) == 0);

			while (bytes > 0)
			{
				uint32_t data = m_bda->read_bus32(srcaddr);
				write_pipram32(dstaddr, data);
				dstaddr += 4;
				srcaddr += 4;
				bytes -= 4;
#if TEST_TIMING
				g_statistics[STAT_TEXEL_BYTES] += 4;
#endif
			}

			break;
		}

		default:
		{
			logerror("UNHANDLED TEXTURE LOAD!\n");
			break;
		}
	}
}



/***************************************************************************
    TIMERS
***************************************************************************/

TIMER_CALLBACK_MEMBER(m2_te_device::command_done)
{
	if (m_gc.te_master_mode & TEICNTL_INT)
		set_interrupt(INTSTAT_IMMEDIATE_INSTR);
}
