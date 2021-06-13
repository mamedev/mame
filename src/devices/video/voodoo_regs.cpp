// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo_regs.cpp

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#include "emu.h"
#include "voodoo.h"

using namespace voodoo;


/*************************************
 *
 *  Alias map of the first 64
 *  registers when remapped
 *
 *************************************/

u8 const voodoo_regs::s_register_alias_map[0x40] =
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

u8 const voodoo_regs::s_register_access[0x100] =
{
	/* 0x000 */
	REG_RP,     0,          REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x040 */
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x080 */
	REG_WPF,    0,          REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x0c0 */
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x100 */
	REG_WPF,    REG_RWPF,   REG_RWPF,   REG_RWPF,
	REG_RWF,    REG_RWF,    REG_RWF,    REG_RWF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     0,          0,

	/* 0x140 */
	REG_RWF,    REG_RWF,    REG_RWF,    REG_R,
	REG_R,      REG_R,      REG_R,      REG_R,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x180 */
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x1c0 */
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	0,          0,          0,          0,
	0,          0,          0,          0,

	/* 0x200 */
	REG_RW,     REG_R,      REG_RW,     REG_RW,
	REG_RW,     REG_RW,     REG_RW,     REG_RW,
	REG_W,      REG_W,      REG_W,      REG_W,
	REG_W,      0,          0,          0,

	/* 0x240 */
	0,          0,          0,          0,
	0,          0,          0,          0,
	0,          0,          0,          0,
	0,          0,          0,          0,

	/* 0x280 */
	0,          0,          0,          0,
	0,          0,          0,          0,
	0,          0,          0,          0,
	0,          0,          0,          0,

	/* 0x2c0 */
	0,          0,          0,          0,
	0,          0,          0,          0,
	0,          0,          0,          0,
	0,          0,          0,          0,

	/* 0x300 */
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x340 */
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x380 */
	REG_WF
};

u8 const voodoo_regs::s_voodoo2_register_access[0x100] =
{
	/* 0x000 */
	REG_RP,     REG_RWPT,   REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x040 */
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x080 */
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x0c0 */
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x100 */
	REG_WPF,    REG_RWPF,   REG_RWPF,   REG_RWPF,
	REG_RWF,    REG_RWF,    REG_RWF,    REG_RWF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x140 */
	REG_RWF,    REG_RWF,    REG_RWF,    REG_R,
	REG_R,      REG_R,      REG_R,      REG_R,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x180 */
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x1c0 */
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_RWT,    REG_RWT,    REG_RWT,    REG_RWT,
	REG_RWT,    REG_RWT,    REG_RWT,    REG_RW,

	/* 0x200 */
	REG_RWT,    REG_R,      REG_RWT,    REG_RWT,
	REG_RWT,    REG_RWT,    REG_RWT,    REG_RWT,
	REG_WT,     REG_WT,     REG_WF,     REG_WT,
	REG_WT,     REG_WT,     REG_WT,     REG_WT,

	/* 0x240 */
	REG_R,      REG_RWT,    REG_RWT,    REG_RWT,
	0,          0,          REG_R,      REG_R,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x280 */
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    0,          0,
	0,          0,          0,          0,

	/* 0x2c0 */
	REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWPF,
	REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWPF,
	REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_RWPF,
	REG_RWPF,   REG_RWPF,   REG_RWPF,   REG_WPF,

	/* 0x300 */
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x340 */
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x380 */
	REG_WF
};

u8 const voodoo_regs::s_banshee_register_access[0x100] =
{
	/* 0x000 */
	REG_RP,     REG_RWPT,   REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x040 */
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x080 */
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x0c0 */
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x100 */
	REG_WPF,    REG_RWPF,   REG_RWPF,   REG_RWPF,
	REG_RWF,    REG_RWF,    REG_RWF,    REG_RWF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x140 */
	REG_RWF,    REG_RWF,    REG_RWF,    REG_R,
	REG_R,      REG_R,      REG_R,      REG_R,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x180 */
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x1c0 */
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	0,          0,          0,          REG_RWF,
	REG_RWF,    REG_RWF,    REG_RWF,    0,

	/* 0x200 */
	REG_RWF,    REG_RWF,    0,          0,
	0,          0,          0,          0,
	0,          0,          0,          0,
	0,          0,          0,          0,

	/* 0x240 */
	0,          0,          0,          REG_WT,
	REG_RWF,    REG_RWF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_R,      REG_R,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,

	/* 0x280 */
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    0,          0,
	0,          0,          0,          0,

	/* 0x2c0 */
	0,          0,          0,          0,
	0,          0,          0,          0,
	0,          0,          0,          0,
	0,          0,          0,          0,

	/* 0x300 */
	REG_WPF,    REG_WPF,    REG_WPF,    REG_WPF,
	REG_WPF,    REG_WPF,    REG_WPF,    0,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x340 */
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,
	REG_WF,     REG_WF,     REG_WF,     REG_WF,

	/* 0x380 */
	REG_WF
};


/*************************************
 *
 *  Register string table for debug
 *
 *************************************/

char const *const voodoo_regs::s_register_name[0x100] =
{
	/* 0x000 */
	"status",       "{intrCtrl}",   "vertexAx",     "vertexAy",
	"vertexBx",     "vertexBy",     "vertexCx",     "vertexCy",
	"startR",       "startG",       "startB",       "startZ",
	"startA",       "startS",       "startT",       "startW",
	/* 0x040 */
	"dRdX",         "dGdX",         "dBdX",         "dZdX",
	"dAdX",         "dSdX",         "dTdX",         "dWdX",
	"dRdY",         "dGdY",         "dBdY",         "dZdY",
	"dAdY",         "dSdY",         "dTdY",         "dWdY",
	/* 0x080 */
	"triangleCMD",  "reserved084",  "fvertexAx",    "fvertexAy",
	"fvertexBx",    "fvertexBy",    "fvertexCx",    "fvertexCy",
	"fstartR",      "fstartG",      "fstartB",      "fstartZ",
	"fstartA",      "fstartS",      "fstartT",      "fstartW",
	/* 0x0c0 */
	"fdRdX",        "fdGdX",        "fdBdX",        "fdZdX",
	"fdAdX",        "fdSdX",        "fdTdX",        "fdWdX",
	"fdRdY",        "fdGdY",        "fdBdY",        "fdZdY",
	"fdAdY",        "fdSdY",        "fdTdY",        "fdWdY",
	/* 0x100 */
	"ftriangleCMD", "fbzColorPath", "fogMode",      "alphaMode",
	"fbzMode",      "lfbMode",      "clipLeftRight","clipLowYHighY",
	"nopCMD",       "fastfillCMD",  "swapbufferCMD","fogColor",
	"zaColor",      "chromaKey",    "{chromaRange}","{userIntrCMD}",
	/* 0x140 */
	"stipple",      "color0",       "color1",       "fbiPixelsIn",
	"fbiChromaFail","fbiZfuncFail", "fbiAfuncFail", "fbiPixelsOut",
	"fogTable160",  "fogTable164",  "fogTable168",  "fogTable16c",
	"fogTable170",  "fogTable174",  "fogTable178",  "fogTable17c",
	/* 0x180 */
	"fogTable180",  "fogTable184",  "fogTable188",  "fogTable18c",
	"fogTable190",  "fogTable194",  "fogTable198",  "fogTable19c",
	"fogTable1a0",  "fogTable1a4",  "fogTable1a8",  "fogTable1ac",
	"fogTable1b0",  "fogTable1b4",  "fogTable1b8",  "fogTable1bc",
	/* 0x1c0 */
	"fogTable1c0",  "fogTable1c4",  "fogTable1c8",  "fogTable1cc",
	"fogTable1d0",  "fogTable1d4",  "fogTable1d8",  "fogTable1dc",
	"{cmdFifoBaseAddr}","{cmdFifoBump}","{cmdFifoRdPtr}","{cmdFifoAMin}",
	"{cmdFifoAMax}","{cmdFifoDepth}","{cmdFifoHoles}","reserved1fc",
	/* 0x200 */
	"fbiInit4",     "vRetrace",     "backPorch",    "videoDimensions",
	"fbiInit0",     "fbiInit1",     "fbiInit2",     "fbiInit3",
	"hSync",        "vSync",        "clutData",     "dacData",
	"maxRgbDelta",  "{hBorder}",    "{vBorder}",    "{borderColor}",
	/* 0x240 */
	"{hvRetrace}",  "{fbiInit5}",   "{fbiInit6}",   "{fbiInit7}",
	"reserved250",  "reserved254",  "{fbiSwapHistory}","{fbiTrianglesOut}",
	"{sSetupMode}", "{sVx}",        "{sVy}",        "{sARGB}",
	"{sRed}",       "{sGreen}",     "{sBlue}",      "{sAlpha}",
	/* 0x280 */
	"{sVz}",        "{sWb}",        "{sWtmu0}",     "{sS/Wtmu0}",
	"{sT/Wtmu0}",   "{sWtmu1}",     "{sS/Wtmu1}",   "{sT/Wtmu1}",
	"{sDrawTriCMD}","{sBeginTriCMD}","reserved2a8", "reserved2ac",
	"reserved2b0",  "reserved2b4",  "reserved2b8",  "reserved2bc",
	/* 0x2c0 */
	"{bltSrcBaseAddr}","{bltDstBaseAddr}","{bltXYStrides}","{bltSrcChromaRange}",
	"{bltDstChromaRange}","{bltClipX}","{bltClipY}","reserved2dc",
	"{bltSrcXY}",   "{bltDstXY}",   "{bltSize}",    "{bltRop}",
	"{bltColor}",   "reserved2f4",  "{bltCommand}", "{bltData}",
	/* 0x300 */
	"textureMode",  "tLOD",         "tDetail",      "texBaseAddr",
	"texBaseAddr_1","texBaseAddr_2","texBaseAddr_3_8","trexInit0",
	"trexInit1",    "nccTable0.0",  "nccTable0.1",  "nccTable0.2",
	"nccTable0.3",  "nccTable0.4",  "nccTable0.5",  "nccTable0.6",
	/* 0x340 */
	"nccTable0.7",  "nccTable0.8",  "nccTable0.9",  "nccTable0.A",
	"nccTable0.B",  "nccTable1.0",  "nccTable1.1",  "nccTable1.2",
	"nccTable1.3",  "nccTable1.4",  "nccTable1.5",  "nccTable1.6",
	"nccTable1.7",  "nccTable1.8",  "nccTable1.9",  "nccTable1.A",
	/* 0x380 */
	"nccTable1.B"
};

char const *const voodoo_regs::s_banshee_register_name[0x100] =
{
	/* 0x000 */
	"status",       "intrCtrl",     "vertexAx",     "vertexAy",
	"vertexBx",     "vertexBy",     "vertexCx",     "vertexCy",
	"startR",       "startG",       "startB",       "startZ",
	"startA",       "startS",       "startT",       "startW",
	/* 0x040 */
	"dRdX",         "dGdX",         "dBdX",         "dZdX",
	"dAdX",         "dSdX",         "dTdX",         "dWdX",
	"dRdY",         "dGdY",         "dBdY",         "dZdY",
	"dAdY",         "dSdY",         "dTdY",         "dWdY",
	/* 0x080 */
	"triangleCMD",  "reserved084",  "fvertexAx",    "fvertexAy",
	"fvertexBx",    "fvertexBy",    "fvertexCx",    "fvertexCy",
	"fstartR",      "fstartG",      "fstartB",      "fstartZ",
	"fstartA",      "fstartS",      "fstartT",      "fstartW",
	/* 0x0c0 */
	"fdRdX",        "fdGdX",        "fdBdX",        "fdZdX",
	"fdAdX",        "fdSdX",        "fdTdX",        "fdWdX",
	"fdRdY",        "fdGdY",        "fdBdY",        "fdZdY",
	"fdAdY",        "fdSdY",        "fdTdY",        "fdWdY",
	/* 0x100 */
	"ftriangleCMD", "fbzColorPath", "fogMode",      "alphaMode",
	"fbzMode",      "lfbMode",      "clipLeftRight","clipLowYHighY",
	"nopCMD",       "fastfillCMD",  "swapbufferCMD","fogColor",
	"zaColor",      "chromaKey",    "chromaRange",  "userIntrCMD",
	/* 0x140 */
	"stipple",      "color0",       "color1",       "fbiPixelsIn",
	"fbiChromaFail","fbiZfuncFail", "fbiAfuncFail", "fbiPixelsOut",
	"fogTable160",  "fogTable164",  "fogTable168",  "fogTable16c",
	"fogTable170",  "fogTable174",  "fogTable178",  "fogTable17c",
	/* 0x180 */
	"fogTable180",  "fogTable184",  "fogTable188",  "fogTable18c",
	"fogTable190",  "fogTable194",  "fogTable198",  "fogTable19c",
	"fogTable1a0",  "fogTable1a4",  "fogTable1a8",  "fogTable1ac",
	"fogTable1b0",  "fogTable1b4",  "fogTable1b8",  "fogTable1bc",
	/* 0x1c0 */
	"fogTable1c0",  "fogTable1c4",  "fogTable1c8",  "fogTable1cc",
	"fogTable1d0",  "fogTable1d4",  "fogTable1d8",  "fogTable1dc",
	"reserved1e0",  "reserved1e4",  "reserved1e8",  "colBufferAddr",
	"colBufferStride","auxBufferAddr","auxBufferStride","reserved1fc",
	/* 0x200 */
	"clipLeftRight1","clipTopBottom1","reserved208","reserved20c",
	"reserved210",  "reserved214",  "reserved218",  "reserved21c",
	"reserved220",  "reserved224",  "reserved228",  "reserved22c",
	"reserved230",  "reserved234",  "reserved238",  "reserved23c",
	/* 0x240 */
	"reserved240",  "reserved244",  "reserved248",  "swapPending",
	"leftOverlayBuf","rightOverlayBuf","fbiSwapHistory","fbiTrianglesOut",
	"sSetupMode",   "sVx",          "sVy",          "sARGB",
	"sRed",         "sGreen",       "sBlue",        "sAlpha",
	/* 0x280 */
	"sVz",          "sWb",          "sWtmu0",       "sS/Wtmu0",
	"sT/Wtmu0",     "sWtmu1",       "sS/Wtmu1",     "sT/Wtmu1",
	"sDrawTriCMD",  "sBeginTriCMD", "reserved2a8",  "reserved2ac",
	"reserved2b0",  "reserved2b4",  "reserved2b8",  "reserved2bc",
	/* 0x2c0 */
	"reserved2c0",  "reserved2c4",  "reserved2c8",  "reserved2cc",
	"reserved2d0",  "reserved2d4",  "reserved2d8",  "reserved2dc",
	"reserved2e0",  "reserved2e4",  "reserved2e8",  "reserved2ec",
	"reserved2f0",  "reserved2f4",  "reserved2f8",  "reserved2fc",
	/* 0x300 */
	"textureMode",  "tLOD",         "tDetail",      "texBaseAddr",
	"texBaseAddr_1","texBaseAddr_2","texBaseAddr_3_8","reserved31c",
	"trexInit1",    "nccTable0.0",  "nccTable0.1",  "nccTable0.2",
	"nccTable0.3",  "nccTable0.4",  "nccTable0.5",  "nccTable0.6",
	/* 0x340 */
	"nccTable0.7",  "nccTable0.8",  "nccTable0.9",  "nccTable0.A",
	"nccTable0.B",  "nccTable1.0",  "nccTable1.1",  "nccTable1.2",
	"nccTable1.3",  "nccTable1.4",  "nccTable1.5",  "nccTable1.6",
	"nccTable1.7",  "nccTable1.8",  "nccTable1.9",  "nccTable1.A",
	/* 0x380 */
	"nccTable1.B"
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
