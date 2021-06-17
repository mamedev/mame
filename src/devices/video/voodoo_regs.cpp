// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo_regs.cpp

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#include "emu.h"
#include "voodoo.h"

using namespace voodoo;


char const *const *const voodoo_regs::s_names[] =
{
	&s_names_rev1[0],
	&s_names_rev1[0],	// same as rev 1
	&s_names_rev3[0]
};

u8 const *const voodoo_regs::s_access[] =
{
	&s_access_rev1[0],
	&s_access_rev2[0],
	&s_access_rev3[0]
};


/*************************************
 *
 *  Alias map of the first 64
 *  registers when remapped
 *
 *************************************/

u8 const voodoo_regs::s_alias_map[0x40] =
{
	voodoo_regs::reg_vdstatus,   0x004/4,                     voodoo_regs::reg_vertexAx,   voodoo_regs::reg_vertexAy,
	voodoo_regs::reg_vertexBx,   voodoo_regs::reg_vertexBy,   voodoo_regs::reg_vertexCx,   voodoo_regs::reg_vertexCy,
	voodoo_regs::reg_startR,     voodoo_regs::reg_dRdX,       voodoo_regs::reg_dRdY,       voodoo_regs::reg_startG,
	voodoo_regs::reg_dGdX,       voodoo_regs::reg_dGdY,       voodoo_regs::reg_startB,     voodoo_regs::reg_dBdX,
	voodoo_regs::reg_dBdY,       voodoo_regs::reg_startZ,     voodoo_regs::reg_dZdX,       voodoo_regs::reg_dZdY,
	voodoo_regs::reg_startA,     voodoo_regs::reg_dAdX,       voodoo_regs::reg_dAdY,       voodoo_regs::reg_startS,
	voodoo_regs::reg_dSdX,       voodoo_regs::reg_dSdY,       voodoo_regs::reg_startT,     voodoo_regs::reg_dTdX,
	voodoo_regs::reg_dTdY,       voodoo_regs::reg_startW,     voodoo_regs::reg_dWdX,       voodoo_regs::reg_dWdY,

	voodoo_regs::reg_triangleCMD,0x084/4,                     voodoo_regs::reg_fvertexAx,  voodoo_regs::reg_fvertexAy,
	voodoo_regs::reg_fvertexBx,  voodoo_regs::reg_fvertexBy,  voodoo_regs::reg_fvertexCx,  voodoo_regs::reg_fvertexCy,
	voodoo_regs::reg_fstartR,    voodoo_regs::reg_fdRdX,      voodoo_regs::reg_fdRdY,      voodoo_regs::reg_fstartG,
	voodoo_regs::reg_fdGdX,      voodoo_regs::reg_fdGdY,      voodoo_regs::reg_fstartB,    voodoo_regs::reg_fdBdX,
	voodoo_regs::reg_fdBdY,      voodoo_regs::reg_fstartZ,    voodoo_regs::reg_fdZdX,      voodoo_regs::reg_fdZdY,
	voodoo_regs::reg_fstartA,    voodoo_regs::reg_fdAdX,      voodoo_regs::reg_fdAdY,      voodoo_regs::reg_fstartS,
	voodoo_regs::reg_fdSdX,      voodoo_regs::reg_fdSdY,      voodoo_regs::reg_fstartT,    voodoo_regs::reg_fdTdX,
	voodoo_regs::reg_fdTdY,      voodoo_regs::reg_fstartW,    voodoo_regs::reg_fdWdX,      voodoo_regs::reg_fdWdY
};


/*************************************
 *
 *  Table of per-register access rights
 *
 *************************************/

u8 const voodoo_regs::s_access_rev1[0x100] =
{
	REG_RP,     0,          REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x000
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x020
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x040
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x060
	REG_WPF,    0,          REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x080
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x0a0
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x0c0
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x0e0
	REG_WPF,    REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWF,    REG_RWF,    REG_RWF,    REG_RWF,    // 0x100
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     0,          0,          // 0x120
	REG_RWF,    REG_RWF,    REG_RWF,    REG_R,      REG_R,      REG_R,      REG_R,      REG_R,      // 0x140
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x160
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x180
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x1a0
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x1c0
	0,          0,          0,          0,          0,          0,          0,          0,          // 0x1e0
	REG_RW,     REG_R,      REG_RW,     REG_RW,     REG_RW,     REG_RW,     REG_RW,     REG_RW,     // 0x200
	REG_W,      REG_W,      REG_W,      REG_W,      REG_W,      0,          0,          0,          // 0x220
	0,          0,          0,          0,          0,          0,          0,          0,          // 0x240
	0,          0,          0,          0,          0,          0,          0,          0,          // 0x260
	0,          0,          0,          0,          0,          0,          0,          0,          // 0x280
	0,          0,          0,          0,          0,          0,          0,          0,          // 0x2a0
	0,          0,          0,          0,          0,          0,          0,          0,          // 0x2c0
	0,          0,          0,          0,          0,          0,          0,          0,          // 0x2e0
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WF,     // 0x300
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x320
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x340
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x360
	REG_WF                                                                                          // 0x380
};

u8 const voodoo_regs::s_access_rev2[0x100] =
{
	REG_RP,     REG_RWPT,   REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x000
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x020
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x040
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x060
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x080
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x0a0
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x0c0
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x0e0
	REG_WPF,    REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWF,    REG_RWF,    REG_RWF,    REG_RWF,    // 0x100
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x120
	REG_RWF,    REG_RWF,    REG_RWF,    REG_R,      REG_R,      REG_R,      REG_R,      REG_R,      // 0x140
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x160
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x180
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x1a0
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x1c0
	REG_RWT,    REG_RWT,    REG_RWT,    REG_RWT,    REG_RWT,    REG_RWT,    REG_RWT,    REG_RW,     // 0x1e0
	REG_RWT,    REG_R,      REG_RWT,    REG_RWT,    REG_RWT,    REG_RWT,    REG_RWT,    REG_RWT,    // 0x200
	REG_WT,     REG_WT,     REG_WF,     REG_WT,     REG_WT,     REG_WT,     REG_WT,     REG_WT,     // 0x220
	REG_R,      REG_RWT,    REG_RWT,    REG_RWT,    0,          0,          REG_R,      REG_R,      // 0x240
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x260
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x280
	REG_WPF,    REG_WPF,    0,          0,          0,          0,          0,          0,          // 0x2a0
	REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWPF,   // 0x2c0
	REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_WPF,    // 0x2e0
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WF,     // 0x300
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x320
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x340
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x360
	REG_WF                                                                                          // 0x380
};

u8 const voodoo_regs::s_access_rev3[0x100] =
{
	REG_RP,     REG_RWPT,   REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x000
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x020
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x040
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x060
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x080
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x0a0
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x0c0
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x0e0
	REG_WPF,    REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWF,    REG_RWF,    REG_RWF,    REG_RWF,    // 0x100
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x120
	REG_RWF,    REG_RWF,    REG_RWF,    REG_R,      REG_R,      REG_R,      REG_R,      REG_R,      // 0x140
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x160
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x180
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x1a0
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x1c0
	0,          0,          0,          REG_RWF,    REG_RWF,    REG_RWF,    REG_RWF,    0,          // 0x1e0
	REG_RWF,    REG_RWF,    0,          0,          0,          0,          0,          0,          // 0x200
	0,          0,          0,          0,          0,          0,          0,          0,          // 0x220
	0,          0,          0,          REG_WT,     REG_RWF,    REG_RWF,    REG_WPF,    REG_WPF,    // 0x240
	REG_WPF,    REG_WPF,    REG_R,      REG_R,      REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x260
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    // 0x280
	REG_WPF,    REG_WPF,    0,          0,          0,          0,          0,          0,          // 0x2a0
	0,          0,          0,          0,          0,          0,          0,          0,          // 0x2c0
	0,          0,          0,          0,          0,          0,          0,          0,          // 0x2e0
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,    0,          // 0x300
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x320
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x340
	REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     REG_WF,     // 0x360
	REG_WF                                                                                          // 0x380
};


/*************************************
 *
 *  Register string table for debug
 *
 *************************************/

char const *const voodoo_regs::s_names_rev1[0x100] =
{
	"status",           "{intrCtrl}",       "vertexAx",         "vertexAy",         // 0x000
	"vertexBx",         "vertexBy",         "vertexCx",         "vertexCy",         // 0x010
	"startR",           "startG",           "startB",           "startZ",           // 0x020
	"startA",           "startS",           "startT",           "startW",           // 0x030
	"dRdX",             "dGdX",             "dBdX",             "dZdX",             // 0x040
	"dAdX",             "dSdX",             "dTdX",             "dWdX",             // 0x050
	"dRdY",             "dGdY",             "dBdY",             "dZdY",             // 0x060
	"dAdY",             "dSdY",             "dTdY",             "dWdY",             // 0x070
	"triangleCMD",      "reserved084",      "fvertexAx",        "fvertexAy",        // 0x080
	"fvertexBx",        "fvertexBy",        "fvertexCx",        "fvertexCy",        // 0x090
	"fstartR",          "fstartG",          "fstartB",          "fstartZ",          // 0x0a0
	"fstartA",          "fstartS",          "fstartT",          "fstartW",          // 0x0b0
	"fdRdX",            "fdGdX",            "fdBdX",            "fdZdX",            // 0x0c0
	"fdAdX",            "fdSdX",            "fdTdX",            "fdWdX",            // 0x0d0
	"fdRdY",            "fdGdY",            "fdBdY",            "fdZdY",            // 0x0e0
	"fdAdY",            "fdSdY",            "fdTdY",            "fdWdY",            // 0x0f0
	"ftriangleCMD",     "fbzColorPath",     "fogMode",          "alphaMode",        // 0x100
	"fbzMode",          "lfbMode",          "clipLeftRight",    "clipLowYHighY",    // 0x110
	"nopCMD",           "fastfillCMD",      "swapbufferCMD",    "fogColor",         // 0x120
	"zaColor",          "chromaKey",        "{chromaRange}",    "{userIntrCMD}",    // 0x130
	"stipple",          "color0",           "color1",           "fbiPixelsIn",      // 0x140
	"fbiChromaFail",    "fbiZfuncFail",     "fbiAfuncFail",     "fbiPixelsOut",     // 0x150
	"fogTable160",      "fogTable164",      "fogTable168",      "fogTable16c",      // 0x160
	"fogTable170",      "fogTable174",      "fogTable178",      "fogTable17c",      // 0x170
	"fogTable180",      "fogTable184",      "fogTable188",      "fogTable18c",      // 0x180
	"fogTable190",      "fogTable194",      "fogTable198",      "fogTable19c",      // 0x190
	"fogTable1a0",      "fogTable1a4",      "fogTable1a8",      "fogTable1ac",      // 0x1a0
	"fogTable1b0",      "fogTable1b4",      "fogTable1b8",      "fogTable1bc",      // 0x1b0
	"fogTable1c0",      "fogTable1c4",      "fogTable1c8",      "fogTable1cc",      // 0x1c0
	"fogTable1d0",      "fogTable1d4",      "fogTable1d8",      "fogTable1dc",      // 0x1d0
	"{cmdFifoBaseAddr}","{cmdFifoBump}",    "{cmdFifoRdPtr}",   "{cmdFifoAMin}",    // 0x1e0
	"{cmdFifoAMax}",    "{cmdFifoDepth}",   "{cmdFifoHoles}",   "reserved1fc",      // 0x1f0
	"fbiInit4",         "vRetrace",         "backPorch",        "videoDimensions",  // 0x200
	"fbiInit0",         "fbiInit1",         "fbiInit2",         "fbiInit3",         // 0x210
	"hSync",            "vSync",            "clutData",         "dacData",          // 0x220
	"maxRgbDelta",      "{hBorder}",        "{vBorder}",        "{borderColor}",    // 0x230
	"{hvRetrace}",      "{fbiInit5}",       "{fbiInit6}",       "{fbiInit7}",       // 0x240
	"reserved250",      "reserved254",      "{fbiSwapHistory}", "{fbiTrianglesOut}",// 0x250
	"{sSetupMode}",     "{sVx}",            "{sVy}",            "{sARGB}",          // 0x260
	"{sRed}",           "{sGreen}",         "{sBlue}",          "{sAlpha}",         // 0x270
	"{sVz}",            "{sWb}",            "{sWtmu0}",         "{sS/Wtmu0}",       // 0x280
	"{sT/Wtmu0}",       "{sWtmu1}",         "{sS/Wtmu1}",       "{sT/Wtmu1}",       // 0x290
	"{sDrawTriCMD}",    "{sBeginTriCMD}",   "reserved2a8",      "reserved2ac",      // 0x2a0
	"reserved2b0",      "reserved2b4",      "reserved2b8",      "reserved2bc",      // 0x2b0
	"{bltSrcBaseAddr}", "{bltDstBaseAddr}", "{bltXYStrides}",   "{bltSrcChromaRange}",// 0x2c0
	"{bltDstChromaRange}","{bltClipX}",     "{bltClipY}",       "reserved2dc",      // 0x2d0
	"{bltSrcXY}",       "{bltDstXY}",       "{bltSize}",        "{bltRop}",         // 0x2e0
	"{bltColor}",       "reserved2f4",      "{bltCommand}",     "{bltData}",        // 0x2f0
	"textureMode",      "tLOD",             "tDetail",          "texBaseAddr",      // 0x300
	"texBaseAddr_1",    "texBaseAddr_2",    "texBaseAddr_3_8",  "trexInit0",        // 0x310
	"trexInit1",        "nccTable0.0",      "nccTable0.1",      "nccTable0.2",      // 0x320
	"nccTable0.3",      "nccTable0.4",      "nccTable0.5",      "nccTable0.6",      // 0x330
	"nccTable0.7",      "nccTable0.8",      "nccTable0.9",      "nccTable0.A",      // 0x340
	"nccTable0.B",      "nccTable1.0",      "nccTable1.1",      "nccTable1.2",      // 0x350
	"nccTable1.3",      "nccTable1.4",      "nccTable1.5",      "nccTable1.6",      // 0x360
	"nccTable1.7",      "nccTable1.8",      "nccTable1.9",      "nccTable1.A",      // 0x370
	"nccTable1.B"                                                                   // 0x380
};

char const *const voodoo_regs::s_names_rev3[0x100] =
{
	"status",           "intrCtrl",         "vertexAx",         "vertexAy",         // 0x000
	"vertexBx",         "vertexBy",         "vertexCx",         "vertexCy",         // 0x010
	"startR",           "startG",           "startB",           "startZ",           // 0x020
	"startA",           "startS",           "startT",           "startW",           // 0x030
	"dRdX",             "dGdX",             "dBdX",             "dZdX",             // 0x040
	"dAdX",             "dSdX",             "dTdX",             "dWdX",             // 0x050
	"dRdY",             "dGdY",             "dBdY",             "dZdY",             // 0x060
	"dAdY",             "dSdY",             "dTdY",             "dWdY",             // 0x070
	"triangleCMD",      "reserved084",      "fvertexAx",        "fvertexAy",        // 0x080
	"fvertexBx",        "fvertexBy",        "fvertexCx",        "fvertexCy",        // 0x090
	"fstartR",          "fstartG",          "fstartB",          "fstartZ",          // 0x0a0
	"fstartA",          "fstartS",          "fstartT",          "fstartW",          // 0x0b0
	"fdRdX",            "fdGdX",            "fdBdX",            "fdZdX",            // 0x0c0
	"fdAdX",            "fdSdX",            "fdTdX",            "fdWdX",            // 0x0d0
	"fdRdY",            "fdGdY",            "fdBdY",            "fdZdY",            // 0x0e0
	"fdAdY",            "fdSdY",            "fdTdY",            "fdWdY",            // 0x0f0
	"ftriangleCMD",     "fbzColorPath",     "fogMode",          "alphaMode",        // 0x100
	"fbzMode",          "lfbMode",          "clipLeftRight",    "clipLowYHighY",    // 0x110
	"nopCMD",           "fastfillCMD",      "swapbufferCMD",    "fogColor",         // 0x120
	"zaColor",          "chromaKey",        "chromaRange",      "userIntrCMD",      // 0x130
	"stipple",          "color0",           "color1",           "fbiPixelsIn",      // 0x140
	"fbiChromaFail",    "fbiZfuncFail",     "fbiAfuncFail",     "fbiPixelsOut",     // 0x150
	"fogTable160",      "fogTable164",      "fogTable168",      "fogTable16c",      // 0x160
	"fogTable170",      "fogTable174",      "fogTable178",      "fogTable17c",      // 0x170
	"fogTable180",      "fogTable184",      "fogTable188",      "fogTable18c",      // 0x180
	"fogTable190",      "fogTable194",      "fogTable198",      "fogTable19c",      // 0x190
	"fogTable1a0",      "fogTable1a4",      "fogTable1a8",      "fogTable1ac",      // 0x1a0
	"fogTable1b0",      "fogTable1b4",      "fogTable1b8",      "fogTable1bc",      // 0x1b0
	"fogTable1c0",      "fogTable1c4",      "fogTable1c8",      "fogTable1cc",      // 0x1c0
	"fogTable1d0",      "fogTable1d4",      "fogTable1d8",      "fogTable1dc",      // 0x1d0
	"reserved1e0",      "reserved1e4",      "reserved1e8",      "colBufferAddr",    // 0x1e0
	"colBufferStride",  "auxBufferAddr",    "auxBufferStride",  "reserved1fc",      // 0x1f0
	"clipLeftRight1",   "clipTopBottom1",   "reserved208",      "reserved20c",      // 0x200
	"reserved210",      "reserved214",      "reserved218",      "reserved21c",      // 0x210
	"reserved220",      "reserved224",      "reserved228",      "reserved22c",      // 0x220
	"reserved230",      "reserved234",      "reserved238",      "reserved23c",      // 0x230
	"reserved240",      "reserved244",      "reserved248",      "swapPending",      // 0x240
	"leftOverlayBuf",   "rightOverlayBuf",  "fbiSwapHistory",   "fbiTrianglesOut",  // 0x250
	"sSetupMode",       "sVx",              "sVy",              "sARGB",            // 0x260
	"sRed",             "sGreen",           "sBlue",            "sAlpha",           // 0x270
	"sVz",              "sWb",              "sWtmu0",           "sS/Wtmu0",         // 0x280
	"sT/Wtmu0",         "sWtmu1",           "sS/Wtmu1",         "sT/Wtmu1",         // 0x290
	"sDrawTriCMD",      "sBeginTriCMD",     "reserved2a8",      "reserved2ac",      // 0x2a0
	"reserved2b0",      "reserved2b4",      "reserved2b8",      "reserved2bc",      // 0x2b0
	"reserved2c0",      "reserved2c4",      "reserved2c8",      "reserved2cc",      // 0x2c0
	"reserved2d0",      "reserved2d4",      "reserved2d8",      "reserved2dc",      // 0x2d0
	"reserved2e0",      "reserved2e4",      "reserved2e8",      "reserved2ec",      // 0x2e0
	"reserved2f0",      "reserved2f4",      "reserved2f8",      "reserved2fc",      // 0x2f0
	"textureMode",      "tLOD",             "tDetail",          "texBaseAddr",      // 0x300
	"texBaseAddr_1",    "texBaseAddr_2",    "texBaseAddr_3_8",  "reserved31c",      // 0x310
	"trexInit1",        "nccTable0.0",      "nccTable0.1",      "nccTable0.2",      // 0x320
	"nccTable0.3",      "nccTable0.4",      "nccTable0.5",      "nccTable0.6",      // 0x330
	"nccTable0.7",      "nccTable0.8",      "nccTable0.9",      "nccTable0.A",      // 0x340
	"nccTable0.B",      "nccTable1.0",      "nccTable1.1",      "nccTable1.2",      // 0x350
	"nccTable1.3",      "nccTable1.4",      "nccTable1.5",      "nccTable1.6",      // 0x360
	"nccTable1.7",      "nccTable1.8",      "nccTable1.9",      "nccTable1.A",      // 0x370
	"nccTable1.B"                                                                   // 0x380
};


/*************************************
 *
 *  Register string table for debug
 *
 *************************************/

char const *const voodoo_regs::s_banshee_io_reg_name[0x40] =
{
	/* 0x000 */
	"status",       "pciInit0",     "sipMonitor",   "lfbMemoryConfig",
	"miscInit0",    "miscInit1",    "dramInit0",    "dramInit1",
	"agpInit",      "tmuGbeInit",   "vgaInit0",     "vgaInit1",
	"dramCommand",  "dramData",     "reserved38",   "reserved3c",

	/* 0x040 */
	"pllCtrl0",     "pllCtrl1",     "pllCtrl2",     "dacMode",
	"dacAddr",      "dacData",      "rgbMaxDelta",  "vidProcCfg",
	"hwCurPatAddr", "hwCurLoc",     "hwCurC0",      "hwCurC1",
	"vidInFormat",  "vidInStatus",  "vidSerialParallelPort","vidInXDecimDeltas",

	/* 0x080 */
	"vidInDecimInitErrs","vidInYDecimDeltas","vidPixelBufThold","vidChromaMin",
	"vidChromaMax", "vidCurrentLine","vidScreenSize","vidOverlayStartCoords",
	"vidOverlayEndScreenCoord","vidOverlayDudx","vidOverlayDudxOffsetSrcWidth","vidOverlayDvdy",
	"vga[b0]",      "vga[b4]",      "vga[b8]",      "vga[bc]",

	/* 0x0c0 */
	"vga[c0]",      "vga[c4]",      "vga[c8]",      "vga[cc]",
	"vga[d0]",      "vga[d4]",      "vga[d8]",      "vga[dc]",
	"vidOverlayDvdyOffset","vidDesktopStartAddr","vidDesktopOverlayStride","vidInAddr0",
	"vidInAddr1",   "vidInAddr2",   "vidInStride",  "vidCurrOverlayStartAddr"
};


/*************************************
 *
 *  Register string table for debug
 *
 *************************************/

char const *const voodoo_regs::s_banshee_agp_reg_name[0x50] =
{
	/* 0x000 */
	"agpReqSize",   "agpHostAddressLow","agpHostAddressHigh","agpGraphicsAddress",
	"agpGraphicsStride","agpMoveCMD","reserved18",  "reserved1c",
	"cmdBaseAddr0", "cmdBaseSize0", "cmdBump0",     "cmdRdPtrL0",
	"cmdRdPtrH0",   "cmdAMin0",     "reserved38",   "cmdAMax0",

	/* 0x040 */
	"reserved40",   "cmdFifoDepth0","cmdHoleCnt0",  "reserved4c",
	"cmdBaseAddr1", "cmdBaseSize1", "cmdBump1",     "cmdRdPtrL1",
	"cmdRdPtrH1",   "cmdAMin1",     "reserved68",   "cmdAMax1",
	"reserved70",   "cmdFifoDepth1","cmdHoleCnt1",  "reserved7c",

	/* 0x080 */
	"cmdFifoThresh","cmdHoleInt",   "reserved88",   "reserved8c",
	"reserved90",   "reserved94",   "reserved98",   "reserved9c",
	"reserveda0",   "reserveda4",   "reserveda8",   "reservedac",
	"reservedb0",   "reservedb4",   "reservedb8",   "reservedbc",

	/* 0x0c0 */
	"reservedc0",   "reservedc4",   "reservedc8",   "reservedcc",
	"reservedd0",   "reservedd4",   "reservedd8",   "reserveddc",
	"reservede0",   "reservede4",   "reservede8",   "reservedec",
	"reservedf0",   "reservedf4",   "reservedf8",   "reservedfc",

	/* 0x100 */
	"yuvBaseAddress","yuvStride",   "reserved108",  "reserved10c",
	"reserved110",  "reserved114",  "reserved118",  "reserved11c",
	"crc1",         "reserved124",  "reserved128",  "reserved12c",
	"crc2",         "reserved134",  "reserved138",  "reserved13c"
};
