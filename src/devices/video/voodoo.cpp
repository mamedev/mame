// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo.c

    3dfx Voodoo Graphics SST-1/2 emulator.

****************************************************************************

//fix me -- blitz2k dies when starting a game with heavy fog (in DRC)

****************************************************************************

    3dfx Voodoo Graphics SST-1/2 emulator

    emulator by Aaron Giles

    --------------------------

    Specs:

    Voodoo 1 (SST1):
        2,4MB frame buffer RAM
        1,2,4MB texture RAM
        50MHz clock frequency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        64 entry PCI FIFO
        memory FIFO up to 65536 entries

    Voodoo 2:
        2,4MB frame buffer RAM
        2,4,8,16MB texture RAM
        90MHz clock frquency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        ultrafast clears @ 16 pixels/clock
        128 entry PCI FIFO
        memory FIFO up to 65536 entries

    Voodoo Banshee (h3):
        Integrated VGA support
        2,4,8MB frame buffer RAM
        90MHz clock frquency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        ultrafast clears @ 32 pixels/clock

    Voodoo 3 ("Avenger"/h4):
        Integrated VGA support
        4,8,16MB frame buffer RAM
        143MHz clock frquency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        ultrafast clears @ 32 pixels/clock

    --------------------------

    still to be implemented:
        * trilinear textures

    things to verify:
        * floating Z buffer


iterated RGBA = 12.12 [24 bits]
iterated Z    = 20.12 [32 bits]
iterated W    = 18.32 [48 bits]

>mamepm blitz
Stall PCI for HWM: 1
PCI FIFO Empty Entries LWM: D
LFB -> FIFO: 1
Texture -> FIFO: 1
Memory FIFO: 1
Memory FIFO HWM: 2000
Memory FIFO Write Burst HWM: 36
Memory FIFO LWM for PCI: 5
Memory FIFO row start: 120
Memory FIFO row rollover: 3FF
Video dither subtract: 0
DRAM banking: 1
Triple buffer: 0
Video buffer offset: 60
DRAM banking: 1

>mamepm wg3dh
Stall PCI for HWM: 1
PCI FIFO Empty Entries LWM: D
LFB -> FIFO: 1
Texture -> FIFO: 1
Memory FIFO: 1
Memory FIFO HWM: 2000
Memory FIFO Write Burst HWM: 36
Memory FIFO LWM for PCI: 5
Memory FIFO row start: C0
Memory FIFO row rollover: 3FF
Video dither subtract: 0
DRAM banking: 1
Triple buffer: 0
Video buffer offset: 40
DRAM banking: 1


As a point of reference, the 3D engine uses the following algorithm to calculate the linear memory address as a
function of the video buffer offset (fbiInit2 bits(19:11)), the number of 32x32 tiles in the X dimension (fbiInit1
bits(7:4) and bit(24)), X, and Y:

    tilesInX[4:0] = {fbiInit1[24], fbiInit1[7:4], fbiInit6[30]}
    rowBase = fbiInit2[19:11]
    rowStart = ((Y>>5) * tilesInX) >> 1

    if (!(tilesInX & 1))
    {
        rowOffset = (X>>6);
        row[9:0] = rowStart + rowOffset (for color buffer 0)
        row[9:0] = rowBase + rowStart + rowOffset (for color buffer 1)
        row[9:0] = (rowBase<<1) + rowStart + rowOffset (for depth/alpha buffer when double color buffering[fbiInit5[10:9]=0])
        row[9:0] = (rowBase<<1) + rowStart + rowOffset (for color buffer 2 when triple color buffering[fbiInit5[10:9]=1 or 2])
        row[9:0] = (rowBase<<1) + rowBase + rowStart + rowOffset (for depth/alpha buffer when triple color buffering[fbiInit5[10:9]=2])
        column[8:0] = ((Y % 32) <<4) + ((X % 32)>>1)
        ramSelect[1] = ((X&0x20) ? 1 : 0) (for color buffers)
        ramSelect[1] = ((X&0x20) ? 0 : 1) (for depth/alpha buffers)
    }
    else
    {
        rowOffset = (!(Y&0x20)) ? (X>>6) : ((X>31) ? (((X-32)>>6)+1) : 0)
        row[9:0] = rowStart + rowOffset (for color buffer 0)
        row[9:0] = rowBase + rowStart + rowOffset (for color buffer 1)
        row[9:0] = (rowBase<<1) + rowStart + rowOffset (for depth/alpha buffer when double color buffering[fbiInit5[10:9]=0])
        row[9:0] = (rowBase<<1) + rowStart + rowOffset (for color buffer 2 when triple color buffering[fbiInit5[10:9]=1 or 2])
        row[9:0] = (rowBase<<1) + rowBase + rowStart + rowOffset (for depth/alpha buffer when triple color buffering[fbiInit5[10:9]=2])
        column[8:0] = ((Y % 32) <<4) + ((X % 32)>>1)
        ramSelect[1] = (((X&0x20)^(Y&0x20)) ? 1 : 0) (for color buffers)
        ramSelect[1] = (((X&0x20)^(Y&0x20)) ? 0 : 1) (for depth/alpha buffers)
    }
    ramSelect[0] = X % 2
    pixelMemoryAddress[21:0] = (row[9:0]<<12) + (column[8:0]<<3) + (ramSelect[1:0]<<1)
    bankSelect = pixelMemoryAddress[21]

**************************************************************************/


#include "emu.h"
#include "voodoo.h"
#include "vooddefs.ipp"

#include "screen.h"

using namespace voodoo;


/*************************************
 *
 *  Debugging
 *
 *************************************/

#define DEBUG_DEPTH         (0)
#define DEBUG_BACKBUF       (0)
#define DEBUG_STATS         (0)

#define LOG_RASTERIZERS     (0)

#define LOG_VBLANK_SWAP     (0)
#define LOG_FIFO            (0)
#define LOG_FIFO_VERBOSE    (0)
#define LOG_REGISTERS       (0)
#define LOG_WAITS           (0)
#define LOG_LFB             (0)
#define LOG_TEXTURE_RAM     (0)
#define LOG_CMDFIFO         (0)
#define LOG_CMDFIFO_VERBOSE (0)
#define LOG_BANSHEE_2D      (0)

// Need to turn off cycle eating when debugging MIPS drc
// otherwise timer interrupts won't match nodrc debug mode.
#define EAT_CYCLES          (1)


namespace {

/*************************************
 *
 *  Alias map of the first 64
 *  registers when remapped
 *
 *************************************/

const u8 register_alias_map[0x40] =
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

const u8 voodoo_register_access[0x100] =
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

const u8 voodoo2_register_access[0x100] =
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

const u8 banshee_register_access[0x100] =
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

const char *const voodoo_reg_name[] =
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

const char *const banshee_reg_name[] =
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

const char *const banshee_io_reg_name[] =
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

const char *const banshee_agp_reg_name[] =
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

} // anonymous namespace


/*************************************
 *
 *  Statics
 *
 *************************************/

static const rectangle global_cliprect(-4096, 4095, -4096, 4095);

/* fast dither lookup */
std::unique_ptr<u8[]> voodoo::dither_helper::s_dither4_lookup;
std::unique_ptr<u8[]> voodoo::dither_helper::s_dither2_lookup;
std::unique_ptr<u8[]> voodoo::dither_helper::s_nodither_lookup;

u8 const voodoo::dither_helper::s_dither_matrix_4x4[16] =
{
	 0,  8,  2, 10,
	12,  4, 14,  6,
	 3, 11,  1,  9,
	15,  7, 13,  5
};
// Using this matrix allows iteagle video memory tests to pass
u8 const voodoo::dither_helper::s_dither_matrix_2x2[16] =
{
	 8, 10,  8, 10,
	11,  9, 11,  9,
	 8, 10,  8, 10,
	11,  9, 11,  9
};

u8 const voodoo::dither_helper::s_dither_matrix_4x4_subtract[16] =
{
	(15 -  0) >> 1, (15 -  8) >> 1, (15 -  2) >> 1, (15 - 10) >> 1,
	(15 - 12) >> 1, (15 -  4) >> 1, (15 - 14) >> 1, (15 -  6) >> 1,
	(15 -  3) >> 1, (15 - 11) >> 1, (15 -  1) >> 1, (15 -  9) >> 1,
	(15 - 15) >> 1, (15 -  7) >> 1, (15 - 13) >> 1, (15 -  5) >> 1
};
u8 const voodoo::dither_helper::s_dither_matrix_2x2_subtract[16] =
{
	(15 -  8) >> 1, (15 - 10) >> 1, (15 -  8) >> 1, (15 - 10) >> 1,
	(15 - 11) >> 1, (15 -  9) >> 1, (15 - 11) >> 1, (15 -  9) >> 1,
	(15 -  8) >> 1, (15 - 10) >> 1, (15 -  8) >> 1, (15 - 10) >> 1,
	(15 - 11) >> 1, (15 -  9) >> 1, (15 - 11) >> 1, (15 -  9) >> 1
};



/*************************************
 *
 *  Rasterizer table
 *
 *************************************/

#define RASTERIZER_ENTRY(fbzcp, alpha, fog, fbz, tex0, tex1) \
	{ &voodoo_device::rasterizer<fbzcp, fbz, alpha, fog, tex0, tex1>, { fbzcp, alpha, fog, fbz, tex0, tex1 } },

voodoo::static_raster_info voodoo_device::predef_raster_table[] =
{
#include "voodoo_rast.ipp"
	{ nullptr, { 0xffffffff } }
};

#undef RASTERIZER_ENTRY

voodoo::static_raster_info voodoo_device::generic_raster_table[3] =
{
	{ &voodoo_device::rasterizer<reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, 0xffffffff, 0xffffffff>, { 0 } },
	{ &voodoo_device::rasterizer<reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE, 0xffffffff>, { 0 } },
	{ &voodoo_device::rasterizer<reg_fbz_colorpath::DECODE_LIVE, reg_fbz_mode::DECODE_LIVE, reg_alpha_mode::DECODE_LIVE, reg_fog_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE, reg_texture_mode::DECODE_LIVE>, { 0 } },
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*************************************
 *
 *  Video update
 *
 *************************************/

int voodoo_device::voodoo_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int changed = m_fbi.video_changed;
	int drawbuf = m_fbi.frontbuf;
	int x, y;

	/* reset the video changed flag */
	m_fbi.video_changed = false;

	/* if we are blank, just fill with black */
	if (m_type <= TYPE_VOODOO_2 && m_reg.fbi_init1().software_blank())
	{
		bitmap.fill(0, cliprect);
		return changed;
	}

	/* if the CLUT is dirty, recompute the pens array */
	if (m_fbi.clut_dirty)
	{
		u8 rtable[32], gtable[64], btable[32];

		/* Voodoo/Voodoo-2 have an internal 33-entry CLUT */
		if (m_type <= TYPE_VOODOO_2)
		{
			/* kludge: some of the Midway games write 0 to the last entry when they obviously mean FF */
			if ((m_fbi.clut[32] & 0xffffff) == 0 && (m_fbi.clut[31] & 0xffffff) != 0)
				m_fbi.clut[32] = 0x20ffffff;

			/* compute the R/G/B pens first */
			for (x = 0; x < 32; x++)
			{
				/* treat X as a 5-bit value, scale up to 8 bits, and linear interpolate for red/blue */
				y = (x << 3) | (x >> 2);
				rtable[x] = (m_fbi.clut[y >> 3].r() * (8 - (y & 7)) + m_fbi.clut[(y >> 3) + 1].r() * (y & 7)) >> 3;
				btable[x] = (m_fbi.clut[y >> 3].b() * (8 - (y & 7)) + m_fbi.clut[(y >> 3) + 1].b() * (y & 7)) >> 3;

				/* treat X as a 6-bit value with LSB=0, scale up to 8 bits, and linear interpolate */
				y = (x * 2) + 0;
				y = (y << 2) | (y >> 4);
				gtable[x*2+0] = (m_fbi.clut[y >> 3].g() * (8 - (y & 7)) + m_fbi.clut[(y >> 3) + 1].g() * (y & 7)) >> 3;

				/* treat X as a 6-bit value with LSB=1, scale up to 8 bits, and linear interpolate */
				y = (x * 2) + 1;
				y = (y << 2) | (y >> 4);
				gtable[x*2+1] = (m_fbi.clut[y >> 3].g() * (8 - (y & 7)) + m_fbi.clut[(y >> 3) + 1].g() * (y & 7)) >> 3;
			}
		}

		/* Banshee and later have a 512-entry CLUT that can be bypassed */
		else
		{
			int which = (m_banshee.io[io_vidProcCfg] >> 13) & 1;
			int bypass = (m_banshee.io[io_vidProcCfg] >> 11) & 1;

			/* compute R/G/B pens first */
			for (x = 0; x < 32; x++)
			{
				/* treat X as a 5-bit value, scale up to 8 bits */
				y = (x << 3) | (x >> 2);
				rtable[x] = bypass ? y : m_fbi.clut[which * 256 + y].r();
				btable[x] = bypass ? y : m_fbi.clut[which * 256 + y].b();

				/* treat X as a 6-bit value with LSB=0, scale up to 8 bits */
				y = (x * 2) + 0;
				y = (y << 2) | (y >> 4);
				gtable[x*2+0] = bypass ? y : m_fbi.clut[which * 256 + y].g();

				/* treat X as a 6-bit value with LSB=1, scale up to 8 bits, and linear interpolate */
				y = (x * 2) + 1;
				y = (y << 2) | (y >> 4);
				gtable[x*2+1] = bypass ? y : m_fbi.clut[which * 256 + y].g();
			}
		}

		/* now compute the actual pens array */
		for (x = 0; x < 65536; x++)
		{
			int r = rtable[(x >> 11) & 0x1f];
			int g = gtable[(x >> 5) & 0x3f];
			int b = btable[x & 0x1f];
			m_fbi.pen[x] = rgb_t(r, g, b);
		}

		/* no longer dirty */
		m_fbi.clut_dirty = false;
		changed = true;
	}

	/* debugging! */
	if (DEBUG_BACKBUF && machine().input().code_pressed(KEYCODE_L))
		drawbuf = m_fbi.backbuf;

	/* copy from the current front buffer */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		if (y >= m_fbi.yoffs)
		{
			u16 const *const src = (u16 *)(m_fbi.ram + m_fbi.rgboffs[drawbuf]) + (y - m_fbi.yoffs) * m_fbi.rowpixels - m_fbi.xoffs;
			u32 *const dst = &bitmap.pix(y);
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
				dst[x] = m_fbi.pen[src[x]];
		}

	/* update stats display */
	if (DEBUG_STATS)
	{
		int statskey = (machine().input().code_pressed(KEYCODE_BACKSLASH));
		if (statskey && statskey != m_stats.lastkey)
			m_stats.display = !m_stats.display;
		m_stats.lastkey = statskey;

		/* display stats */
		if (m_stats.display)
			popmessage(m_stats.buffer, 0, 0);
	}

	/* update render override */
	if (DEBUG_DEPTH)
	{
		m_stats.render_override = machine().input().code_pressed(KEYCODE_ENTER);
		if (m_stats.render_override)
		{
			for (y = cliprect.min_y; y <= cliprect.max_y; y++)
			{
				u16 const *const src = (u16*)(m_fbi.ram + m_fbi.auxoffs) + (y - m_fbi.yoffs) * m_fbi.rowpixels - m_fbi.xoffs;
				u32 *const dst = &bitmap.pix(y);
				for (x = cliprect.min_x; x <= cliprect.max_x; x++)
					dst[x] = ((src[x] << 8) & 0xff0000) | ((src[x] >> 0) & 0xff00) | ((src[x] >> 8) & 0xff);
			}
		}
	}
	return changed;
}



/*************************************
 *
 *  Chip reset
 *
 *************************************/

void voodoo_device::voodoo_set_init_enable(u32 newval)
{
	m_pci.init_enable = voodoo::reg_init_en(newval);
	if (LOG_REGISTERS)
		logerror("VOODOO.REG:initEnable write = %08X\n", newval);
}



/*************************************
 *
 *  Common initialization
 *
 *************************************/

void voodoo_device::init_fbi(fbi_state *f, void *memory, int fbmem)
{
	/* allocate frame buffer RAM and set pointers */
	f->ram = (u8 *)memory;
	f->mask = fbmem - 1;
	f->rgboffs[0] = f->rgboffs[1] = f->rgboffs[2] = 0;
	f->auxoffs = ~0;

	/* default to 0x0 */
	f->frontbuf = 0;
	f->backbuf = 1;
	f->width = 512;
	f->height = 384;

	/* init the pens */
	f->clut_dirty = true;
	if (m_type <= TYPE_VOODOO_2)
	{
		for (int pen = 0; pen < 32; pen++)
			m_fbi.clut[pen] = rgb_t(pen, pal5bit(pen), pal5bit(pen), pal5bit(pen));
		m_fbi.clut[32] = rgb_t(32,0xff,0xff,0xff);
	}
	else
	{
		for (int pen = 0; pen < 512; pen++)
			m_fbi.clut[pen] = rgb_t(pen,pen,pen);
	}

	// build static 16-bit rgb565 to rgb888 conversion table
	for (int val = 0; val < 65536; val++)
		m_fbi.rgb565[val] = rgbexpand<5,6,5>(val, 11, 5, 0).set_a(0xff);

	/* allocate a VBLANK timer */
	f->vsync_stop_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(voodoo_device::vblank_off_callback), this), this);
	f->vsync_start_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(voodoo_device::vblank_callback),this), this);
	f->vblank = false;

	/* initialize the memory FIFO */
	f->fifo.configure(nullptr, 0);

	/* set the fog delta mask */
	f->fogdelta_mask = (m_type < TYPE_VOODOO_2) ? 0xff : 0xfc;
}


void voodoo_device::tmu_shared_state::init()
{
	/* build static 8-bit texel tables */
	for (int val = 0; val < 256; val++)
	{
		/* 8-bit RGB (3-3-2) */
		rgb332[val] = rgbexpand<3,3,2>(val, 5, 2, 0).set_a(0xff);

		/* 8-bit alpha */
		alpha8[val] = rgb_t(val, val, val, val);

		/* 8-bit intensity */
		int8[val] = rgb_t(0xff, val, val, val);

		/* 8-bit alpha, intensity */
		ai44[val] = argbexpand<4,4,4,4>(val, 4, 0, 0, 0);
	}

	/* build static 16-bit texel tables */
	for (int val = 0; val < 65536; val++)
	{
		/* table 10 = 16-bit RGB (5-6-5) */
		// Use frame buffer table

		/* table 11 = 16 ARGB (1-5-5-5) */
		argb1555[val] = argbexpand<1,5,5,5>(val, 15, 10, 5, 0);

		/* table 12 = 16-bit ARGB (4-4-4-4) */
		argb4444[val] = argbexpand<4,4,4,4>(val, 12, 8, 4, 0);
	}

	rgb565 = nullptr;
}


void voodoo_device::tmu_state::init(u8 vdt, tmu_shared_state &share, void *memory, int tmem)
{
	/* allocate texture RAM */
	m_ram = reinterpret_cast<u8 *>(memory);
	m_mask = tmem - 1;
	m_regdirty = false;
	m_bilinear_mask = (vdt >= TYPE_VOODOO_2) ? 0xff : 0xf0;

	/* mark the NCC tables dirty and configure their registers */
	m_ncc[0].dirty = m_ncc[1].dirty = true;
	m_ncc[0].m_reg = m_reg.subset(voodoo_regs::reg_nccTable + 0);
	m_ncc[1].m_reg = m_reg.subset(voodoo_regs::reg_nccTable + 12);

	/* create pointers to all the tables */
	m_texel[0] = share.rgb332;
	m_texel[1] = m_ncc[0].texel;
	m_texel[2] = share.alpha8;
	m_texel[3] = share.int8;
	m_texel[4] = share.ai44;
	m_texel[5] = m_palette;
	m_texel[6] = (vdt >= TYPE_VOODOO_2) ? m_palettea : nullptr;
	m_texel[7] = nullptr;
	m_texel[8] = share.rgb332;
	m_texel[9] = m_ncc[0].texel;
	m_texel[10] = share.rgb565;
	m_texel[11] = share.argb1555;
	m_texel[12] = share.argb4444;
	m_texel[13] = share.int8;
	m_texel[14] = m_palette;
	m_texel[15] = nullptr;
	m_lookup = m_texel[0];

	/* attach the palette to NCC table 0 */
	m_ncc[0].palette = m_palette;
	if (vdt >= TYPE_VOODOO_2)
		m_ncc[0].palettea = m_palettea;

	/* set up texture address calculations */
	if (vdt <= TYPE_VOODOO_2)
	{
		m_texaddr_mask = 0x0fffff;
		m_texaddr_shift = 3;
	}
	else
	{
		m_texaddr_mask = 0xfffff0;
		m_texaddr_shift = 0;
	}
}


void voodoo_device::voodoo_postload()
{
	m_fbi.clut_dirty = true;
	for (tmu_state &tm : m_tmu)
	{
		tm.m_regdirty = true;
		for (tmu_state::ncc_table &ncc : tm.m_ncc)
			ncc.dirty = true;
	}

	/* recompute video memory to get the FBI FIFO base recomputed */
	if (m_type <= TYPE_VOODOO_2)
		recompute_video_memory();
}


ALLOW_SAVE_TYPE(voodoo::reg_init_en);
ALLOW_SAVE_TYPE(voodoo::voodoo_regs::register_data);

void voodoo_device::init_save_state()
{
	machine().save().register_postload(save_prepost_delegate(FUNC(voodoo_device::voodoo_postload), this));

	/* register states: core */
	save_item(NAME(m_reg.m_regs));
	save_item(NAME(m_alt_regmap));

	/* register states: pci */
	save_item(NAME(m_pci.fifo.m_in));
	save_item(NAME(m_pci.fifo.m_out));
	save_item(NAME(m_pci.init_enable));
	save_item(NAME(m_pci.stall_state));
	save_item(NAME(m_pci.op_pending));
	save_item(NAME(m_pci.op_end_time));
	save_item(NAME(m_pci.fifo_mem));

	/* register states: dac */
	save_item(NAME(m_dac.m_reg));
	save_item(NAME(m_dac.read_result));

	/* register states: fbi */
	save_pointer(NAME(m_fbi.ram), m_fbi.mask + 1);
	save_item(NAME(m_fbi.rgboffs));
	save_item(NAME(m_fbi.auxoffs));
	save_item(NAME(m_fbi.frontbuf));
	save_item(NAME(m_fbi.backbuf));
	save_item(NAME(m_fbi.swaps_pending));
	save_item(NAME(m_fbi.video_changed));
	save_item(NAME(m_fbi.yorigin));
	save_item(NAME(m_fbi.lfb_base));
	save_item(NAME(m_fbi.lfb_stride));
	save_item(NAME(m_fbi.width));
	save_item(NAME(m_fbi.height));
	save_item(NAME(m_fbi.xoffs));
	save_item(NAME(m_fbi.yoffs));
	save_item(NAME(m_fbi.vsyncstart));
	save_item(NAME(m_fbi.vsyncstop));
	save_item(NAME(m_fbi.rowpixels));
	save_item(NAME(m_fbi.vblank));
	save_item(NAME(m_fbi.vblank_count));
	save_item(NAME(m_fbi.vblank_swap_pending));
	save_item(NAME(m_fbi.vblank_swap));
	save_item(NAME(m_fbi.vblank_dont_swap));
	save_item(NAME(m_fbi.sign));
	save_item(NAME(m_fbi.ax));
	save_item(NAME(m_fbi.ay));
	save_item(NAME(m_fbi.bx));
	save_item(NAME(m_fbi.by));
	save_item(NAME(m_fbi.cx));
	save_item(NAME(m_fbi.cy));
	save_item(NAME(m_fbi.startr));
	save_item(NAME(m_fbi.startg));
	save_item(NAME(m_fbi.startb));
	save_item(NAME(m_fbi.starta));
	save_item(NAME(m_fbi.startz));
	save_item(NAME(m_fbi.startw));
	save_item(NAME(m_fbi.drdx));
	save_item(NAME(m_fbi.dgdx));
	save_item(NAME(m_fbi.dbdx));
	save_item(NAME(m_fbi.dadx));
	save_item(NAME(m_fbi.dzdx));
	save_item(NAME(m_fbi.dwdx));
	save_item(NAME(m_fbi.drdy));
	save_item(NAME(m_fbi.dgdy));
	save_item(NAME(m_fbi.dbdy));
	save_item(NAME(m_fbi.dady));
	save_item(NAME(m_fbi.dzdy));
	save_item(NAME(m_fbi.dwdy));
	save_item(NAME(m_fbi.lfb_stats.pixels_in));
	save_item(NAME(m_fbi.lfb_stats.pixels_out));
	save_item(NAME(m_fbi.lfb_stats.chroma_fail));
	save_item(NAME(m_fbi.lfb_stats.zfunc_fail));
	save_item(NAME(m_fbi.lfb_stats.afunc_fail));
	save_item(NAME(m_fbi.lfb_stats.clip_fail));
	save_item(NAME(m_fbi.lfb_stats.stipple_count));
	save_item(NAME(m_fbi.sverts));
	save_item(STRUCT_MEMBER(m_fbi.svert, x));
	save_item(STRUCT_MEMBER(m_fbi.svert, y));
	save_item(STRUCT_MEMBER(m_fbi.svert, a));
	save_item(STRUCT_MEMBER(m_fbi.svert, r));
	save_item(STRUCT_MEMBER(m_fbi.svert, g));
	save_item(STRUCT_MEMBER(m_fbi.svert, b));
	save_item(STRUCT_MEMBER(m_fbi.svert, z));
	save_item(STRUCT_MEMBER(m_fbi.svert, wb));
	save_item(STRUCT_MEMBER(m_fbi.svert, w0));
	save_item(STRUCT_MEMBER(m_fbi.svert, s0));
	save_item(STRUCT_MEMBER(m_fbi.svert, t0));
	save_item(STRUCT_MEMBER(m_fbi.svert, w1));
	save_item(STRUCT_MEMBER(m_fbi.svert, s1));
	save_item(STRUCT_MEMBER(m_fbi.svert, t1));
	save_item(NAME(m_fbi.fifo.m_size));
	save_item(NAME(m_fbi.fifo.m_in));
	save_item(NAME(m_fbi.fifo.m_out));
	save_item(STRUCT_MEMBER(m_fbi.cmdfifo, enable));
	save_item(STRUCT_MEMBER(m_fbi.cmdfifo, count_holes));
	save_item(STRUCT_MEMBER(m_fbi.cmdfifo, base));
	save_item(STRUCT_MEMBER(m_fbi.cmdfifo, end));
	save_item(STRUCT_MEMBER(m_fbi.cmdfifo, rdptr));
	save_item(STRUCT_MEMBER(m_fbi.cmdfifo, amin));
	save_item(STRUCT_MEMBER(m_fbi.cmdfifo, amax));
	save_item(STRUCT_MEMBER(m_fbi.cmdfifo, depth));
	save_item(STRUCT_MEMBER(m_fbi.cmdfifo, holes));
	save_item(NAME(m_fbi.fogblend));
	save_item(NAME(m_fbi.fogdelta));
	save_item(NAME(m_fbi.clut));

	/* register states: tmu */
	for (int index = 0; index < std::size(m_tmu); index++)
	{
		tmu_state *tmu = &m_tmu[index];
		if (tmu->m_ram == nullptr)
			continue;
		save_item(NAME(tmu->m_reg.m_regs), index);
		if (tmu->m_ram != m_fbi.ram)
			save_pointer(NAME(tmu->m_ram), tmu->m_mask + 1, index);
		save_item(NAME(tmu->m_starts), index);
		save_item(NAME(tmu->m_startt), index);
		save_item(NAME(tmu->m_startw), index);
		save_item(NAME(tmu->m_dsdx), index);
		save_item(NAME(tmu->m_dtdx), index);
		save_item(NAME(tmu->m_dwdx), index);
		save_item(NAME(tmu->m_dsdy), index);
		save_item(NAME(tmu->m_dtdy), index);
		save_item(NAME(tmu->m_dwdy), index);
		save_item(STRUCT_MEMBER(tmu->m_ncc, ir), index);
		save_item(STRUCT_MEMBER(tmu->m_ncc, ig), index);
		save_item(STRUCT_MEMBER(tmu->m_ncc, ib), index);
		save_item(STRUCT_MEMBER(tmu->m_ncc, qr), index);
		save_item(STRUCT_MEMBER(tmu->m_ncc, qg), index);
		save_item(STRUCT_MEMBER(tmu->m_ncc, qb), index);
		save_item(STRUCT_MEMBER(tmu->m_ncc, y), index);
	}

	/* register states: banshee */
	if (m_type >= TYPE_VOODOO_BANSHEE)
	{
		save_item(NAME(m_banshee.io));
		save_item(NAME(m_banshee.agp));
		save_item(NAME(m_banshee.vga));
		save_item(NAME(m_banshee.crtc));
		save_item(NAME(m_banshee.seq));
		save_item(NAME(m_banshee.gc));
		save_item(NAME(m_banshee.att));
		save_item(NAME(m_banshee.attff));
	}
}



/*************************************
 *
 *  Statistics management
 *
 *************************************/

void voodoo_device::accumulate_statistics(const thread_stats_block &block)
{
	/* apply internal voodoo statistics */
	m_reg.add(voodoo_regs::reg_fbiPixelsIn, block.pixels_in);
	m_reg.add(voodoo_regs::reg_fbiPixelsOut, block.pixels_out);
	m_reg.add(voodoo_regs::reg_fbiChromaFail, block.chroma_fail);
	m_reg.add(voodoo_regs::reg_fbiZfuncFail, block.zfunc_fail);
	m_reg.add(voodoo_regs::reg_fbiAfuncFail, block.afunc_fail);

	/* apply emulation statistics */
	m_stats.total_pixels_in += block.pixels_in;
	m_stats.total_pixels_out += block.pixels_out;
	m_stats.total_chroma_fail += block.chroma_fail;
	m_stats.total_zfunc_fail += block.zfunc_fail;
	m_stats.total_afunc_fail += block.afunc_fail;
	m_stats.total_clipped += block.clip_fail;
	m_stats.total_stippled += block.stipple_count;
}


void voodoo_device::update_statistics(bool accumulate)
{
	/* accumulate/reset statistics from all units */
	for (int threadnum = 0; threadnum < WORK_MAX_THREADS; threadnum++)
	{
		if (accumulate)
			accumulate_statistics(m_thread_stats[threadnum]);
		memset(&m_thread_stats[threadnum], 0, sizeof(m_thread_stats[threadnum]));
	}

	/* accumulate/reset statistics from the LFB */
	if (accumulate)
		accumulate_statistics(m_fbi.lfb_stats);
	memset(&m_fbi.lfb_stats, 0, sizeof(m_fbi.lfb_stats));
}



/*************************************
 *
 *  VBLANK management
 *
 *************************************/

void voodoo_device::swap_buffers()
{
	if (LOG_VBLANK_SWAP) logerror("--- swap_buffers @ %d\n", m_screen->vpos());

	/* force a partial update */
	m_screen->update_partial(m_screen->vpos());
	m_fbi.video_changed = true;

	/* keep a history of swap intervals */
	m_reg.update_swap_history(std::min<u8>(m_fbi.vblank_count, 15));

	/* rotate the buffers */
	if (m_type <= TYPE_VOODOO_2)
	{
		if (m_type < TYPE_VOODOO_2 || !m_fbi.vblank_dont_swap)
		{
			if (m_fbi.rgboffs[2] == ~0)
			{
				m_fbi.frontbuf = 1 - m_fbi.frontbuf;
				m_fbi.backbuf = 1 - m_fbi.frontbuf;
			}
			else
			{
				m_fbi.frontbuf = (m_fbi.frontbuf + 1) % 3;
				m_fbi.backbuf = (m_fbi.frontbuf + 1) % 3;
			}
		}
	}
	else
		m_fbi.rgboffs[0] = m_reg.read(voodoo_regs::reg_leftOverlayBuf) & m_fbi.mask & ~0x0f;

	/* decrement the pending count and reset our state */
	if (m_fbi.swaps_pending)
		m_fbi.swaps_pending--;
	m_fbi.vblank_count = 0;
	m_fbi.vblank_swap_pending = false;

	/* reset the last_op_time to now and start processing the next command */
	if (m_pci.op_pending)
	{
		if (LOG_VBLANK_SWAP) logerror("---- swap_buffers flush begin\n");
		m_pci.op_end_time = machine().time();
		flush_fifos(m_pci.op_end_time);
		if (LOG_VBLANK_SWAP) logerror("---- swap_buffers flush end\n");
	}

	/* we may be able to unstall now */
	if (m_pci.stall_state != NOT_STALLED)
		check_stalled_cpu(machine().time());

	/* periodically log rasterizer info */
	m_stats.swaps++;
	if (LOG_RASTERIZERS && m_stats.swaps % 1000 == 0)
		dump_rasterizer_stats();

	/* update the statistics (debug) */
	if (m_stats.display)
	{
		const rectangle &visible_area = m_screen->visible_area();
		int screen_area = visible_area.width() * visible_area.height();
		char *statsptr = m_stats.buffer;
		int pixelcount;
		int i;

		update_statistics(true);
		pixelcount = m_stats.total_pixels_out;

		statsptr += sprintf(statsptr, "Swap:%6d\n", m_stats.swaps);
		statsptr += sprintf(statsptr, "Hist:%08X\n", m_reg.swap_history());
		statsptr += sprintf(statsptr, "Stal:%6d\n", m_stats.stalls);
		statsptr += sprintf(statsptr, "Rend:%6d%%\n", pixelcount * 100 / screen_area);
		statsptr += sprintf(statsptr, "Poly:%6d\n", m_stats.total_triangles);
		statsptr += sprintf(statsptr, "PxIn:%6d\n", m_stats.total_pixels_in);
		statsptr += sprintf(statsptr, "POut:%6d\n", m_stats.total_pixels_out);
		statsptr += sprintf(statsptr, "Clip:%6d\n", m_stats.total_clipped);
		statsptr += sprintf(statsptr, "Stip:%6d\n", m_stats.total_stippled);
		statsptr += sprintf(statsptr, "Chro:%6d\n", m_stats.total_chroma_fail);
		statsptr += sprintf(statsptr, "ZFun:%6d\n", m_stats.total_zfunc_fail);
		statsptr += sprintf(statsptr, "AFun:%6d\n", m_stats.total_afunc_fail);
		statsptr += sprintf(statsptr, "RegW:%6d\n", m_stats.reg_writes);
		statsptr += sprintf(statsptr, "RegR:%6d\n", m_stats.reg_reads);
		statsptr += sprintf(statsptr, "LFBW:%6d\n", m_stats.lfb_writes);
		statsptr += sprintf(statsptr, "LFBR:%6d\n", m_stats.lfb_reads);
		statsptr += sprintf(statsptr, "TexW:%6d\n", m_stats.tex_writes);
		statsptr += sprintf(statsptr, "TexM:");
		for (i = 0; i < 16; i++)
			if (m_stats.texture_mode[i])
				*statsptr++ = "0123456789ABCDEF"[i];
		*statsptr = 0;
	}

	/* update statistics */
	m_stats.stalls = 0;
	m_stats.total_triangles = 0;
	m_stats.total_pixels_in = 0;
	m_stats.total_pixels_out = 0;
	m_stats.total_chroma_fail = 0;
	m_stats.total_zfunc_fail = 0;
	m_stats.total_afunc_fail = 0;
	m_stats.total_clipped = 0;
	m_stats.total_stippled = 0;
	m_stats.reg_writes = 0;
	m_stats.reg_reads = 0;
	m_stats.lfb_writes = 0;
	m_stats.lfb_reads = 0;
	m_stats.tex_writes = 0;
	memset(m_stats.texture_mode, 0, sizeof(m_stats.texture_mode));
}


void voodoo_device::adjust_vblank_timer()
{
	attotime vblank_period = m_screen->time_until_pos(m_fbi.vsyncstart);
	if (LOG_VBLANK_SWAP) logerror("adjust_vblank_timer: period: %s\n", vblank_period.as_string());
	/* if zero, adjust to next frame, otherwise we may get stuck in an infinite loop */
	if (vblank_period == attotime::zero)
		vblank_period = m_screen->frame_period();
	m_fbi.vsync_start_timer->adjust(vblank_period);
}


TIMER_CALLBACK_MEMBER( voodoo_device::vblank_off_callback )
{
	if (LOG_VBLANK_SWAP) logerror("--- vblank end\n");

	/* set internal state and call the client */
	m_fbi.vblank = false;

	// PCI Vblank IRQ enable is VOODOO2 and up
	if (m_type >= TYPE_VOODOO_2)
	{
		if (m_reg.intr_ctrl().vsync_falling_enable())
		{
			m_reg.clear_set(voodoo_regs::reg_intrCtrl, reg_intr_ctrl::EXTERNAL_PIN_ACTIVE, reg_intr_ctrl::VSYNC_FALLING_GENERATED);
			if (!m_pciint.isnull())
				m_pciint(true);
		}
	}

	// External vblank handler
	if (!m_vblank.isnull())
		m_vblank(false);

	/* go to the end of the next frame */
	adjust_vblank_timer();
}


TIMER_CALLBACK_MEMBER( voodoo_device::vblank_callback )
{
	if (LOG_VBLANK_SWAP) logerror("--- vblank start\n");

	/* flush the pipes */
	if (m_pci.op_pending)
	{
		if (LOG_VBLANK_SWAP) logerror("---- vblank flush begin\n");
		flush_fifos(machine().time());
		if (LOG_VBLANK_SWAP) logerror("---- vblank flush end\n");
	}

	/* increment the count */
	m_fbi.vblank_count++;
	if (m_fbi.vblank_count > 250)
		m_fbi.vblank_count = 250;
	if (LOG_VBLANK_SWAP) logerror("---- vblank count = %u swap = %u pending = %u", m_fbi.vblank_count, m_fbi.vblank_swap, m_fbi.vblank_swap_pending);
	if (m_fbi.vblank_swap_pending)
		if (LOG_VBLANK_SWAP) logerror(" (target=%d)", m_fbi.vblank_swap);
	if (LOG_VBLANK_SWAP) logerror("\n");

	/* if we're past the swap count, do the swap */
	if (m_fbi.vblank_swap_pending && m_fbi.vblank_count >= m_fbi.vblank_swap)
		swap_buffers();

	/* set a timer for the next off state */
	m_fbi.vsync_stop_timer->adjust(m_screen->time_until_pos(m_fbi.vsyncstop));

	/* set internal state and call the client */
	m_fbi.vblank = true;

	// PCI Vblank IRQ enable is VOODOO2 and up
	if (m_type >= TYPE_VOODOO_2)
	{
		if (m_reg.intr_ctrl().vsync_rising_enable())
		{
			m_reg.clear_set(voodoo_regs::reg_intrCtrl, reg_intr_ctrl::EXTERNAL_PIN_ACTIVE, reg_intr_ctrl::VSYNC_RISING_GENERATED);
			if (!m_pciint.isnull())
				m_pciint(true);
		}
	}
	// External vblank handler
	if (!m_vblank.isnull())
		m_vblank(true);
}



/*************************************
 *
 *  Chip reset
 *
 *************************************/

void voodoo_device::reset_counters()
{
	update_statistics(false);
	m_reg.write(voodoo_regs::reg_fbiPixelsIn, 0);
	m_reg.write(voodoo_regs::reg_fbiChromaFail, 0);
	m_reg.write(voodoo_regs::reg_fbiZfuncFail, 0);
	m_reg.write(voodoo_regs::reg_fbiAfuncFail, 0);
	m_reg.write(voodoo_regs::reg_fbiPixelsOut, 0);
}


void voodoo_device::soft_reset()
{
	reset_counters();
	m_reg.write(voodoo_regs::reg_fbiTrianglesOut, 0);
	m_fbi.fifo.reset();
	m_pci.fifo.reset();
}



/*************************************
 *
 *  Recompute video memory layout
 *
 *************************************/

void voodoo_device::recompute_video_memory()
{
	u32 const buffer_pages = m_reg.fbi_init2().video_buffer_offset();
	u32 const fifo_start_page = m_reg.fbi_init4().memory_fifo_start_row();
	u32 fifo_last_page = m_reg.fbi_init4().memory_fifo_stop_row();
	u32 memory_config;

	/* memory config is determined differently between V1 and V2 */
	memory_config = m_reg.fbi_init2().enable_triple_buf();
	if (m_type == TYPE_VOODOO_2 && memory_config == 0)
		memory_config = m_reg.fbi_init5().buffer_allocation();

	/* tiles are 64x16/32; x_tiles specifies how many half-tiles */
	m_fbi.tile_width = (m_type == TYPE_VOODOO_1) ? 64 : 32;
	m_fbi.tile_height = (m_type == TYPE_VOODOO_1) ? 16 : 32;
	m_fbi.x_tiles = m_reg.fbi_init1().x_video_tiles();
	if (m_type == TYPE_VOODOO_2)
	{
		m_fbi.x_tiles = (m_fbi.x_tiles << 1) |
						(m_reg.fbi_init1().x_video_tiles_bit5() << 5) |
						(m_reg.fbi_init6().x_video_tiles_bit0());
	}
	m_fbi.rowpixels = m_fbi.tile_width * m_fbi.x_tiles;

//  logerror("VOODOO.VIDMEM: buffer_pages=%X  fifo=%X-%X  tiles=%X  rowpix=%d\n", buffer_pages, fifo_start_page, fifo_last_page, m_fbi.x_tiles, m_fbi.rowpixels);

	/* first RGB buffer always starts at 0 */
	m_fbi.rgboffs[0] = 0;

	/* second RGB buffer starts immediately afterwards */
	m_fbi.rgboffs[1] = buffer_pages * 0x1000;

	/* remaining buffers are based on the config */
	switch (memory_config)
	{
		case 3: /* reserved */
			logerror("VOODOO.ERROR:Unexpected memory configuration in recompute_video_memory!\n");
			[[fallthrough]];
		case 0: /* 2 color buffers, 1 aux buffer */
			m_fbi.rgboffs[2] = ~0;
			m_fbi.auxoffs = 2 * buffer_pages * 0x1000;
			break;

		case 1: /* 3 color buffers, 0 aux buffers */
			m_fbi.rgboffs[2] = 2 * buffer_pages * 0x1000;
			m_fbi.auxoffs = ~0;
			break;

		case 2: /* 3 color buffers, 1 aux buffers */
			m_fbi.rgboffs[2] = 2 * buffer_pages * 0x1000;
			m_fbi.auxoffs = 3 * buffer_pages * 0x1000;
			break;
	}

	/* clamp the RGB buffers to video memory */
	for (int buf = 0; buf < 3; buf++)
		if (m_fbi.rgboffs[buf] != ~0 && m_fbi.rgboffs[buf] > m_fbi.mask)
			m_fbi.rgboffs[buf] = m_fbi.mask;

	/* clamp the aux buffer to video memory */
	if (m_fbi.auxoffs != ~0 && m_fbi.auxoffs > m_fbi.mask)
		m_fbi.auxoffs = m_fbi.mask;

/*  osd_printf_debug("rgb[0] = %08X   rgb[1] = %08X   rgb[2] = %08X   aux = %08X\n",
            m_fbi.rgboffs[0], m_fbi.rgboffs[1], m_fbi.rgboffs[2], m_fbi.auxoffs);*/

	/* compute the memory FIFO location and size */
	if (fifo_last_page > m_fbi.mask / 0x1000)
		fifo_last_page = m_fbi.mask / 0x1000;

	/* is it valid and enabled? */
	if (fifo_start_page <= fifo_last_page && m_reg.fbi_init0().enable_memory_fifo())
	{
		u32 size = std::min<u32>((fifo_last_page + 1 - fifo_start_page) * 0x1000 / 4, 65536*2);
		m_fbi.fifo.configure((u32 *)(m_fbi.ram + fifo_start_page * 0x1000), size);
	}

	/* if not, disable the FIFO */
	else
		m_fbi.fifo.configure(nullptr, 0);

	/* reset the FIFO */
	m_fbi.fifo.reset();

	/* reset our front/back buffers if they are out of range */
	if (m_fbi.rgboffs[2] == ~0)
	{
		if (m_fbi.frontbuf == 2)
			m_fbi.frontbuf = 0;
		if (m_fbi.backbuf == 2)
			m_fbi.backbuf = 0;
	}
}



/*************************************
 *
 *  NCC table management
 *
 *************************************/

void voodoo_device::tmu_state::ncc_table::write(offs_t regnum, u32 data)
{
	/* I/Q entries reference the plaette if the high bit is set */
	if (regnum >= 4 && (data & 0x80000000) && palette)
	{
		int const index = ((data >> 23) & 0xfe) | (regnum & 1);

		/* set the ARGB for this palette index */
		palette[index] = 0xff000000 | data;

		/* if we have an ARGB palette as well, compute its value */
		if (palettea)
		{
			int a = ((data >> 16) & 0xfc) | ((data >> 22) & 0x03);
			int r = ((data >> 10) & 0xfc) | ((data >> 16) & 0x03);
			int g = ((data >>  4) & 0xfc) | ((data >> 10) & 0x03);
			int b = ((data <<  2) & 0xfc) | ((data >>  4) & 0x03);
			palettea[index] = rgb_t(a, r, g, b);
		}

		/* this doesn't dirty the table or go to the registers, so bail */
		return;
	}

	/* if the register matches, don't update */
	if (data == m_reg[regnum])
		return;
	m_reg[regnum] = data;

	/* first four entries are packed Y values */
	if (regnum < 4)
	{
		regnum *= 4;
		y[regnum+0] = (data >>  0) & 0xff;
		y[regnum+1] = (data >>  8) & 0xff;
		y[regnum+2] = (data >> 16) & 0xff;
		y[regnum+3] = (data >> 24) & 0xff;
	}

	/* the second four entries are the I RGB values */
	else if (regnum < 8)
	{
		regnum &= 3;
		ir[regnum] = (s32)(data <<  5) >> 23;
		ig[regnum] = (s32)(data << 14) >> 23;
		ib[regnum] = (s32)(data << 23) >> 23;
	}

	/* the final four entries are the Q RGB values */
	else
	{
		regnum &= 3;
		qr[regnum] = (s32)(data <<  5) >> 23;
		qg[regnum] = (s32)(data << 14) >> 23;
		qb[regnum] = (s32)(data << 23) >> 23;
	}

	/* mark the table dirty */
	dirty = true;
}


void voodoo_device::tmu_state::ncc_table::update()
{
	/* generte all 256 possibilities */
	for (int i = 0; i < 256; i++)
	{
		int vi = (i >> 2) & 0x03;
		int vq = (i >> 0) & 0x03;

		/* start with the intensity */
		int r, g, b;
		r = g = b = y[(i >> 4) & 0x0f];

		/* add the coloring */
		r = std::clamp(r + ir[vi] + qr[vq], 0, 255);
		g = std::clamp(g + ig[vi] + qg[vq], 0, 255);
		b = std::clamp(b + ib[vi] + qb[vq], 0, 255);

		/* fill in the table */
		texel[i] = rgb_t(0xff, r, g, b);
	}

	/* no longer dirty */
	dirty = false;
}



/*************************************
 *
 *  Faux DAC implementation
 *
 *************************************/

void voodoo_device::dac_state::data_w(u8 regnum, u8 data)
{
	m_reg[regnum] = data;
}


void voodoo_device::dac_state::data_r(u8 regnum)
{
	u8 result = 0xff;

	/* switch off the DAC register requested */
	switch (regnum)
	{
		case 5:
			/* this is just to make startup happy */
			switch (m_reg[7])
			{
				case 0x01:  result = 0x55; break;
				case 0x07:  result = 0x71; break;
				case 0x0b:  result = 0x79; break;
			}
			break;

		default:
			result = m_reg[regnum];
			break;
	}

	/* remember the read result; it is fetched elsewhere */
	read_result = result;
}



/*************************************
 *
 *  Texuture parameter computation
 *
 *************************************/

void voodoo_device::tmu_state::recompute_texture_params()
{
	// extract LOD parameters
	auto const texlod = m_reg.texture_lod();
	m_lodmin = texlod.lod_min() << 6;
	m_lodmax = texlod.lod_max() << 6;
	m_lodbias = s8(texlod.lod_bias() << 2) << 4;

	// determine which LODs are present
	m_lodmask = 0x1ff;
	if (texlod.lod_tsplit())
		m_lodmask = texlod.lod_odd() ? 0x0aa : 0x155;

	// determine base texture width/height
	m_wmask = m_hmask = 0xff;
	if (texlod.lod_s_is_wider())
		m_hmask >>= texlod.lod_aspect();
	else
		m_wmask >>= texlod.lod_aspect();

	// determine the bpp of the texture
	auto const texmode = m_reg.texture_mode();
	int bppscale = texmode.format() >> 3;

	// start with the base of LOD 0
	u32 base = m_reg.texture_baseaddr();
	if (m_texaddr_shift == 0 && BIT(base, 0) != 0)
		osd_printf_debug("Tiled texture\n");
	base = (base & m_texaddr_mask) << m_texaddr_shift;
	m_lodoffset[0] = base & m_mask;

	// LODs 1-3 are different depending on whether we are in multitex mode
	// Several Voodoo 2 games leave the upper bits of TLOD == 0xff, meaning we think
	// they want multitex mode when they really don't -- disable for now
	// Enable for Voodoo 3 or Viper breaks - VL.
	// Add check for upper nibble not equal to zero to fix funkball -- TG
	if (texlod.tmultibaseaddr() && texlod.magic() == 0)
	{
		base = (m_reg.texture_baseaddr_1() & m_texaddr_mask) << m_texaddr_shift;
		m_lodoffset[1] = base & m_mask;
		base = (m_reg.texture_baseaddr_2() & m_texaddr_mask) << m_texaddr_shift;
		m_lodoffset[2] = base & m_mask;
		base = (m_reg.texture_baseaddr_3_8() & m_texaddr_mask) << m_texaddr_shift;
		m_lodoffset[3] = base & m_mask;
	}
	else
	{
		if (m_lodmask & (1 << 0))
			base += (((m_wmask >> 0) + 1) * ((m_hmask >> 0) + 1)) << bppscale;
		m_lodoffset[1] = base & m_mask;
		if (m_lodmask & (1 << 1))
			base += (((m_wmask >> 1) + 1) * ((m_hmask >> 1) + 1)) << bppscale;
		m_lodoffset[2] = base & m_mask;
		if (m_lodmask & (1 << 2))
			base += (((m_wmask >> 2) + 1) * ((m_hmask >> 2) + 1)) << bppscale;
		m_lodoffset[3] = base & m_mask;
	}

	// remaining LODs make sense
	for (int lod = 4; lod <= 8; lod++)
	{
		if (m_lodmask & (1 << (lod - 1)))
		{
			u32 size = ((m_wmask >> (lod - 1)) + 1) * ((m_hmask >> (lod - 1)) + 1);
			if (size < 4) size = 4;
			base += size << bppscale;
		}
		m_lodoffset[lod] = base & m_mask;
	}

	// set the NCC lookup appropriately
	m_texel[1] = m_texel[9] = m_ncc[texmode.ncc_table_select()].texel;

	// pick the lookup table
	m_lookup = m_texel[texmode.format()];

	// compute the detail parameters
	auto const texdetail = m_reg.texture_detail();
	m_detailmax = texdetail.detail_max();
	m_detailbias = s8(texdetail.detail_bias() << 2) << 6;
	m_detailscale = texdetail.detail_scale();

	// ensure that the NCC tables are up to date
	if ((texmode.format() & 7) == 1)
	{
		ncc_table &n = m_ncc[texmode.ncc_table_select()];
		m_texel[1] = m_texel[9] = n.texel;
		if (n.dirty)
			n.update();
	}

	// no longer dirty
	m_regdirty = false;

	// check for separate RGBA filtering
	if (texdetail.separate_rgba_filter())
		fatalerror("Separate RGBA filters!\n");
}


inline s32 voodoo_device::tmu_state::prepare()
{
	// if the texture parameters are dirty, update them
	if (m_regdirty)
		recompute_texture_params();

	// compute (ds^2 + dt^2) in both X and Y as 28.36 numbers
	s64 texdx = s64(m_dsdx >> 14) * s64(m_dsdx >> 14) + s64(m_dtdx >> 14) * s64(m_dtdx >> 14);
	s64 texdy = s64(m_dsdy >> 14) * s64(m_dsdy >> 14) + s64(m_dtdy >> 14) * s64(m_dtdy >> 14);

	// pick whichever is larger and shift off some high bits -> 28.20
	if (texdx < texdy)
		texdx = texdy;
	texdx >>= 16;

	// use our fast reciprocal/log on this value; it expects input as a
	// 16.32 number, and returns the log of the reciprocal, so we have to
	// adjust the result: negative to get the log of the original value
	// plus 12 to account for the extra exponent, and divided by 2 to
	// get the log of the square root of texdx
	double texdx_fp = texdx;
	s32 lodbase = fast_log2(texdx_fp, 0);
	return (lodbase + (12 << 8)) / 2;
}



/*************************************
 *
 *  Command FIFO depth computation
 *
 *************************************/

int voodoo_device::cmdfifo_compute_expected_depth(cmdfifo_info &f)
{
	u32 *fifobase = (u32 *)m_fbi.ram;
	u32 readptr = f.rdptr;
	u32 command = fifobase[readptr / 4];
	int i, count = 0;

	/* low 3 bits specify the packet type */
	switch (command & 7)
	{
		/*
		    Packet type 0: 1 or 2 words

		      Word  Bits
		        0  31:29 = reserved
		        0  28:6  = Address [24:2]
		        0   5:3  = Function (0 = NOP, 1 = JSR, 2 = RET, 3 = JMP LOCAL, 4 = JMP AGP)
		        0   2:0  = Packet type (0)
		        1  31:11 = reserved (JMP AGP only)
		        1  10:0  = Address [35:25]
		*/
		case 0:
			if (((command >> 3) & 7) == 4)
				return 2;
			return 1;

		/*
		    Packet type 1: 1 + N words

		      Word  Bits
		        0  31:16 = Number of words
		        0    15  = Increment?
		        0  14:3  = Register base
		        0   2:0  = Packet type (1)
		        1  31:0  = Data word
		*/
		case 1:
			return 1 + (command >> 16);

		/*
		    Packet type 2: 1 + N words

		      Word  Bits
		        0  31:3  = 2D Register mask
		        0   2:0  = Packet type (2)
		        1  31:0  = Data word
		*/
		case 2:
			for (i = 3; i <= 31; i++)
				if (command & (1 << i)) count++;
			return 1 + count;

		/*
		    Packet type 3: 1 + N words

		      Word  Bits
		        0  31:29 = Number of dummy entries following the data
		        0   28   = Packed color data?
		        0   25   = Disable ping pong sign correction (0=normal, 1=disable)
		        0   24   = Culling sign (0=positive, 1=negative)
		        0   23   = Enable culling (0=disable, 1=enable)
		        0   22   = Strip mode (0=strip, 1=fan)
		        0   17   = Setup S1 and T1
		        0   16   = Setup W1
		        0   15   = Setup S0 and T0
		        0   14   = Setup W0
		        0   13   = Setup Wb
		        0   12   = Setup Z
		        0   11   = Setup Alpha
		        0   10   = Setup RGB
		        0   9:6  = Number of vertices
		        0   5:3  = Command (0=Independent tris, 1=Start new strip, 2=Continue strip)
		        0   2:0  = Packet type (3)
		        1  31:0  = Data word
		*/
		case 3:
			count = 2;      /* X/Y */
			if (command & (1 << 28))
			{
				if (command & (3 << 10)) count++;       /* ARGB */
			}
			else
			{
				if (command & (1 << 10)) count += 3;    /* RGB */
				if (command & (1 << 11)) count++;       /* A */
			}
			if (command & (1 << 12)) count++;           /* Z */
			if (command & (1 << 13)) count++;           /* Wb */
			if (command & (1 << 14)) count++;           /* W0 */
			if (command & (1 << 15)) count += 2;        /* S0/T0 */
			if (command & (1 << 16)) count++;           /* W1 */
			if (command & (1 << 17)) count += 2;        /* S1/T1 */
			count *= (command >> 6) & 15;               /* numverts */
			return 1 + count + (command >> 29);

		/*
		    Packet type 4: 1 + N words

		      Word  Bits
		        0  31:29 = Number of dummy entries following the data
		        0  28:15 = General register mask
		        0  14:3  = Register base
		        0   2:0  = Packet type (4)
		        1  31:0  = Data word
		*/
		case 4:
			for (i = 15; i <= 28; i++)
				if (command & (1 << i)) count++;
			return 1 + count + (command >> 29);

		/*
		    Packet type 5: 2 + N words

		      Word  Bits
		        0  31:30 = Space (0,1=reserved, 2=LFB, 3=texture)
		        0  29:26 = Byte disable W2
		        0  25:22 = Byte disable WN
		        0  21:3  = Num words
		        0   2:0  = Packet type (5)
		        1  31:30 = Reserved
		        1  29:0  = Base address [24:0]
		        2  31:0  = Data word
		*/
		case 5:
			return 2 + ((command >> 3) & 0x7ffff);

		default:
			osd_printf_debug("UNKNOWN PACKET TYPE %d\n", command & 7);
			return 1;
	}
}



/*************************************
 *
 *  Command FIFO execution
 *
 *************************************/

u32 voodoo_device::cmdfifo_execute(cmdfifo_info *f)
{
	u32 *fifobase = (u32 *)m_fbi.ram;
	u32 readptr = f->rdptr;
	u32 *src = &fifobase[readptr / 4];
	u32 command = *src++;
	int count, inc, code, i;
	fbi_state::setup_vertex svert = {0};
	offs_t target;
	int cycles = 0;

	switch (command & 7)
	{
		/*
		    Packet type 0: 1 or 2 words

		      Word  Bits
		        0  31:29 = reserved
		        0  28:6  = Address [24:2]
		        0   5:3  = Function (0 = NOP, 1 = JSR, 2 = RET, 3 = JMP LOCAL, 4 = JMP AGP)
		        0   2:0  = Packet type (0)
		        1  31:11 = reserved (JMP AGP only)
		        1  10:0  = Address [35:25]
		*/
		case 0:

			/* extract parameters */
			target = (command >> 4) & 0x1fffffc;

			/* switch off of the specific command */
			switch ((command >> 3) & 7)
			{
				case 0:     /* NOP */
					if (LOG_CMDFIFO) logerror("  NOP\n");
					break;

				case 1:     /* JSR */
					if (LOG_CMDFIFO) logerror("  JSR $%06X\n", target);
					osd_printf_debug("JSR in CMDFIFO!\n");
					src = &fifobase[target / 4];
					break;

				case 2:     /* RET */
					if (LOG_CMDFIFO) logerror("  RET $%06X\n", target);
					fatalerror("RET in CMDFIFO!\n");

				case 3:     /* JMP LOCAL FRAME BUFFER */
					if (LOG_CMDFIFO) logerror("  JMP LOCAL FRAMEBUF $%06X\n", target);
					src = &fifobase[target / 4];
					break;

				case 4:     /* JMP AGP */
					if (LOG_CMDFIFO) logerror("  JMP AGP $%06X\n", target);
					fatalerror("JMP AGP in CMDFIFO!\n");
					src = &fifobase[target / 4];
					break;

				default:
					osd_printf_debug("INVALID JUMP COMMAND!\n");
					fatalerror("  INVALID JUMP COMMAND\n");
			}
			break;

		/*
		    Packet type 1: 1 + N words

		      Word  Bits
		        0  31:16 = Number of words
		        0    15  = Increment?
		        0  14:3  = Register base
		        0   2:0  = Packet type (1)
		        1  31:0  = Data word
		*/
		case 1:

			/* extract parameters */
			count = command >> 16;
			inc = (command >> 15) & 1;
			target = (command >> 3) & 0xfff;

			if (LOG_CMDFIFO) logerror("  PACKET TYPE 1: count=%d inc=%d reg=%04X\n", count, inc, target);

			if (m_type >= TYPE_VOODOO_BANSHEE && (target & 0x800))
			{
				//  Banshee/Voodoo3 2D register writes

				/* loop over all registers and write them one at a time */
				for (i = 0; i < count; i++, target += inc)
				{
					cycles += banshee_2d_w(target & 0xff, *src);
					//logerror("    2d reg: %03x = %08X\n", target & 0x7ff, *src);
					src++;
				}
			}
			else
			{
				/* loop over all registers and write them one at a time */
				for (i = 0; i < count; i++, target += inc)
					cycles += register_w(target, *src++);
			}
			break;

		/*
		    Packet type 2: 1 + N words

		      Word  Bits
		        0  31:3  = 2D Register mask
		        0   2:0  = Packet type (2)
		        1  31:0  = Data word
		*/
		case 2:
			if (LOG_CMDFIFO) logerror("  PACKET TYPE 2: mask=%X\n", (command >> 3) & 0x1ffffff);

			/* loop over all registers and write them one at a time */
			for (i = 3; i <= 31; i++)
				if (command & (1 << i))
					cycles += register_w(banshee2D_clip0Min + (i - 3), *src++);
			break;

		/*
		    Packet type 3: 1 + N words

		      Word  Bits
		        0  31:29 = Number of dummy entries following the data
		        0   28   = Packed color data?
		        0   25   = Disable ping pong sign correction (0=normal, 1=disable)
		        0   24   = Culling sign (0=positive, 1=negative)
		        0   23   = Enable culling (0=disable, 1=enable)
		        0   22   = Strip mode (0=strip, 1=fan)
		        0   17   = Setup S1 and T1
		        0   16   = Setup W1
		        0   15   = Setup S0 and T0
		        0   14   = Setup W0
		        0   13   = Setup Wb
		        0   12   = Setup Z
		        0   11   = Setup Alpha
		        0   10   = Setup RGB
		        0   9:6  = Number of vertices
		        0   5:3  = Command (0=Independent tris, 1=Start new strip, 2=Continue strip)
		        0   2:0  = Packet type (3)
		        1  31:0  = Data word
		*/
		case 3:

			/* extract parameters */
			count = (command >> 6) & 15;
			code = (command >> 3) & 7;

			if (LOG_CMDFIFO) logerror("  PACKET TYPE 3: count=%d code=%d mask=%03X smode=%02X pc=%d\n", count, code, (command >> 10) & 0xfff, (command >> 22) & 0x3f, (command >> 28) & 1);

			/* copy relevant bits into the setup mode register */
			m_reg.write(voodoo_regs::reg_sSetupMode, ((command >> 10) & 0xff) | ((command >> 6) & 0xf0000));

			/* loop over triangles */
			for (i = 0; i < count; i++)
			{
				/* always extract X/Y */
				svert.x = *(float *)src++;
				svert.y = *(float *)src++;

				/* load ARGB values if packed */
				if (command & (1 << 28))
				{
					if (command & (3 << 10))
					{
						rgb_t argb = *src++;
						if (command & (1 << 10))
						{
							svert.r = argb.r();
							svert.g = argb.g();
							svert.b = argb.b();
						}
						if (command & (1 << 11))
							svert.a = argb.a();
					}
				}

				/* load ARGB values if not packed */
				else
				{
					if (command & (1 << 10))
					{
						svert.r = *(float *)src++;
						svert.g = *(float *)src++;
						svert.b = *(float *)src++;
					}
					if (command & (1 << 11))
						svert.a = *(float *)src++;
				}

				/* load Z and Wb values */
				if (command & (1 << 12))
					svert.z = *(float *)src++;
				if (command & (1 << 13))
					svert.wb = svert.w0 = svert.w1 = *(float *)src++;

				/* load W0, S0, T0 values */
				if (command & (1 << 14))
					svert.w0 = svert.w1 = *(float *)src++;
				if (command & (1 << 15))
				{
					svert.s0 = svert.s1 = *(float *)src++;
					svert.t0 = svert.t1 = *(float *)src++;
				}

				/* load W1, S1, T1 values */
				if (command & (1 << 16))
					svert.w1 = *(float *)src++;
				if (command & (1 << 17))
				{
					svert.s1 = *(float *)src++;
					svert.t1 = *(float *)src++;
				}

				/* if we're starting a new strip, or if this is the first of a set of verts */
				/* for a series of individual triangles, initialize all the verts */
				if ((code == 1 && i == 0) || (code == 0 && i % 3 == 0))
				{
					m_fbi.sverts = 1;
					m_fbi.svert[0] = m_fbi.svert[1] = m_fbi.svert[2] = svert;
				}

				/* otherwise, add this to the list */
				else
				{
					/* for strip mode, shuffle vertex 1 down to 0 */
					if (!(command & (1 << 22)))
						m_fbi.svert[0] = m_fbi.svert[1];

					/* copy 2 down to 1 and add our new one regardless */
					m_fbi.svert[1] = m_fbi.svert[2];
					m_fbi.svert[2] = svert;

					/* if we have enough, draw */
					if (++m_fbi.sverts >= 3)
						cycles += setup_and_draw_triangle();
				}
			}

			/* account for the extra dummy words */
			src += command >> 29;
			break;

		/*
		    Packet type 4: 1 + N words

		      Word  Bits
		        0  31:29 = Number of dummy entries following the data
		        0  28:15 = General register mask
		        0  14:3  = Register base
		        0   2:0  = Packet type (4)
		        1  31:0  = Data word
		*/
		case 4:

			/* extract parameters */
			target = (command >> 3) & 0xfff;

			if (LOG_CMDFIFO) logerror("  PACKET TYPE 4: mask=%X reg=%04X pad=%d\n", (command >> 15) & 0x3fff, target, command >> 29);

			if (m_type >= TYPE_VOODOO_BANSHEE && (target & 0x800))
			{
				//  Banshee/Voodoo3 2D register writes

				/* loop over all registers and write them one at a time */
				target &= 0xff;
				for (i = 15; i <= 28; i++)
				{
					if (command & (1 << i))
					{
						cycles += banshee_2d_w(target + (i - 15), *src);
						//logerror("    2d reg: %03x = %08X\n", target & 0x7ff, *src);
						src++;
					}
				}
			}
			else
			{
				/* loop over all registers and write them one at a time */
				for (i = 15; i <= 28; i++)
					if (command & (1 << i))
						cycles += register_w(target + (i - 15), *src++);
			}

			/* account for the extra dummy words */
			src += command >> 29;
			break;

		/*
		    Packet type 5: 2 + N words

		      Word  Bits
		        0  31:30 = Space (0,1=reserved, 2=LFB, 3=texture)
		        0  29:26 = Byte disable W2
		        0  25:22 = Byte disable WN
		        0  21:3  = Num words
		        0   2:0  = Packet type (5)
		        1  31:30 = Reserved
		        1  29:0  = Base address [24:0]
		        2  31:0  = Data word
		*/
		case 5:

			/* extract parameters */
			count = (command >> 3) & 0x7ffff;
			target = *src++ / 4;

			/* handle LFB writes */
			switch (command >> 30)
			{
				case 0:     // Linear FB
				{
					if (LOG_CMDFIFO) logerror("  PACKET TYPE 5: FB count=%d dest=%08X bd2=%X bdN=%X\n", count, target, (command >> 26) & 15, (command >> 22) & 15);

					u32 addr = target * 4;
					for (i=0; i < count; i++)
					{
						u32 data = *src++;

						m_fbi.ram[BYTE_XOR_LE(addr + 0)] = (u8)(data);
						m_fbi.ram[BYTE_XOR_LE(addr + 1)] = (u8)(data >> 8);
						m_fbi.ram[BYTE_XOR_LE(addr + 2)] = (u8)(data >> 16);
						m_fbi.ram[BYTE_XOR_LE(addr + 3)] = (u8)(data >> 24);

						addr += 4;
					}
					break;
				}
				case 2:     // 3D LFB
				{
					if (LOG_CMDFIFO) logerror("  PACKET TYPE 5: 3D LFB count=%d dest=%08X bd2=%X bdN=%X\n", count, target, (command >> 26) & 15, (command >> 22) & 15);

					/* loop over words */
					for (i = 0; i < count; i++)
						cycles += lfb_w(target++, *src++, 0xffffffff);

					break;
				}

				case 1:     // Planar YUV
				{
					// TODO
					if (LOG_CMDFIFO) logerror("  PACKET TYPE 5: Planar YUV count=%d dest=%08X bd2=%X bdN=%X\n", count, target, (command >> 26) & 15, (command >> 22) & 15);

					/* just update the pointers for now */
					for (i = 0; i < count; i++)
					{
						target++;
						src++;
					}

					break;
				}

				case 3:     // Texture Port
				{
					if (LOG_CMDFIFO) logerror("  PACKET TYPE 5: textureRAM count=%d dest=%08X bd2=%X bdN=%X\n", count, target, (command >> 26) & 15, (command >> 22) & 15);

					/* loop over words */
					for (i = 0; i < count; i++)
						cycles += texture_w(target++, *src++);

					break;
				}
			}

			break;

		default:
			fprintf(stderr, "PACKET TYPE %d\n", command & 7);
			break;
	}

	/* by default just update the read pointer past all the data we consumed */
	f->rdptr = 4 * (src - fifobase);
	return cycles;
}



/*************************************
 *
 *  Handle execution if we're ready
 *
 *************************************/

s32 voodoo_device::cmdfifo_execute_if_ready(cmdfifo_info &f)
{
	/* all CMDFIFO commands need at least one word */
	if (f.depth == 0)
		return -1;

	/* see if we have enough for the current command */
	int const needed_depth = cmdfifo_compute_expected_depth(f);
	if (f.depth < needed_depth)
		return -1;

	/* execute */
	int const cycles = cmdfifo_execute(&f);
	f.depth -= needed_depth;
	return cycles;
}



/*************************************
 *
 *  Handle writes to the CMD FIFO
 *
 *************************************/

void voodoo_device::cmdfifo_w(cmdfifo_info *f, offs_t offset, u32 data)
{
	u32 addr = f->base + offset * 4;
	u32 *fifobase = (u32 *)m_fbi.ram;

	if (LOG_CMDFIFO_VERBOSE) logerror("CMDFIFO_w(%04X) = %08X\n", offset, data);

	/* write the data */
	if (addr < f->end)
		fifobase[addr / 4] = data;

	/* count holes? */
	if (f->count_holes)
	{
		/* in-order, no holes */
		if (f->holes == 0 && addr == f->amin + 4)
		{
			f->amin = f->amax = addr;
			f->depth++;
		}

		/* out-of-order, below the minimum */
		else if (addr < f->amin)
		{
			if (f->holes != 0)
				logerror("Unexpected CMDFIFO: AMin=%08X AMax=%08X Holes=%d WroteTo:%08X\n",
						f->amin, f->amax, f->holes, addr);
			//f->amin = f->amax = addr;
			f->holes += (addr - f->base) / 4;
			f->amin = f->base;
			f->amax = addr;

			f->depth++;
		}

		/* out-of-order, but within the min-max range */
		else if (addr < f->amax)
		{
			f->holes--;
			if (f->holes == 0)
			{
				f->depth += (f->amax - f->amin) / 4;
				f->amin = f->amax;
			}
		}

		/* out-of-order, bumping max */
		else
		{
			f->holes += (addr - f->amax) / 4 - 1;
			f->amax = addr;
		}
	}

	/* execute if we can */
	if (!m_pci.op_pending)
	{
		s32 cycles = cmdfifo_execute_if_ready(*f);
		if (cycles > 0)
		{
			m_pci.op_pending = true;
			m_pci.op_end_time = machine().time() + attotime(0, (attoseconds_t)cycles * m_attoseconds_per_cycle);

			if (LOG_FIFO_VERBOSE)
			{
				logerror("VOODOO.FIFO:direct write start at %d.%018d end at %d.%018d\n",
						machine().time().seconds(), machine().time().attoseconds(),
						m_pci.op_end_time.seconds(), m_pci.op_end_time.attoseconds());
			}
		}
	}
}



/*************************************
 *
 *  Stall the active cpu until we are
 *  ready
 *
 *************************************/

TIMER_CALLBACK_MEMBER( voodoo_device::stall_cpu_callback )
{
	check_stalled_cpu(machine().time());
}


void voodoo_device::check_stalled_cpu(attotime current_time)
{
	int resume = false;

	/* flush anything we can */
	if (m_pci.op_pending)
		flush_fifos(current_time);

	/* if we're just stalled until the LWM is passed, see if we're ok now */
	if (m_pci.stall_state == STALLED_UNTIL_FIFO_LWM)
	{
		/* if there's room in the memory FIFO now, we can proceed */
		if (m_reg.fbi_init0().enable_memory_fifo())
		{
			if (m_fbi.fifo.items() < 2 * 32 * m_reg.fbi_init0().memory_fifo_hwm())
				resume = true;
		}
		else if (m_pci.fifo.space() > 2 * m_reg.fbi_init0().pci_fifo_lwm())
			resume = true;
	}

	/* if we're stalled until the FIFOs are empty, check now */
	else if (m_pci.stall_state == STALLED_UNTIL_FIFO_EMPTY)
	{
		if (m_reg.fbi_init0().enable_memory_fifo())
		{
			if (m_fbi.fifo.empty() && m_pci.fifo.empty())
				resume = true;
		}
		else if (m_pci.fifo.empty())
			resume = true;
	}

	/* resume if necessary */
	if (resume || !m_pci.op_pending)
	{
		if (LOG_FIFO) logerror("VOODOO.FIFO:Stall condition cleared; resuming\n");
		m_pci.stall_state = NOT_STALLED;

		/* either call the callback, or trigger the trigger */
		if (!m_stall.isnull())
			m_stall(false);
		else
			machine().scheduler().trigger(m_trigger);
	}

	/* if not, set a timer for the next one */
	else
	{
		m_pci.continue_timer->adjust(m_pci.op_end_time - current_time);
	}
}


void voodoo_device::stall_cpu(int state, attotime current_time)
{
	/* sanity check */
	if (!m_pci.op_pending) fatalerror("FIFOs not empty, no op pending!\n");

	/* set the state and update statistics */
	m_pci.stall_state = state;
	m_stats.stalls++;

	/* either call the callback, or spin the CPU */
	if (!m_stall.isnull())
		m_stall(true);
	else
		m_cpu->spin_until_trigger(m_trigger);

	/* set a timer to clear the stall */
	m_pci.continue_timer->adjust(m_pci.op_end_time - current_time);
}



/*************************************
 *
 *  Voodoo register writes
 *
 *************************************/

s32 voodoo_device::register_w(offs_t offset, u32 data)
{
	u32 origdata = data;
	s32 cycles = 0;
	s64 data64;
	u8 regnum;
	u8 chips;

	/* statistics */
	m_stats.reg_writes++;

	/* determine which chips we are addressing */
	chips = (offset >> 8) & 0xf;
	if (chips == 0)
		chips = 0xf;
	chips &= m_chipmask;

	/* the first 64 registers can be aliased differently */
	if ((offset & 0x800c0) == 0x80000 && m_alt_regmap)
		regnum = register_alias_map[offset & 0x3f];
	else
		regnum = offset & 0xff;

	/* first make sure this register is readable */
	if (!(m_regaccess[regnum] & REGISTER_WRITE))
	{
		logerror("VOODOO.ERROR:Invalid attempt to write %s\n", m_regnames[regnum]);
		return 0;
	}

	/* switch off the register */
	switch (regnum)
	{
		case voodoo_regs::reg_intrCtrl:
			m_reg.write(regnum, data);
			// Setting bit 31 clears the PCI interrupts
			if (data & 0x80000000) {
				// Clear pci interrupt
				if (!m_pciint.isnull())
					m_pciint(false);
			}
			break;
		/* Vertex data is 12.4 formatted fixed point */
		case voodoo_regs::reg_fvertexAx:
			data = float_to_int32(data, 4);
			[[fallthrough]];
		case voodoo_regs::reg_vertexAx:
			if (chips & 1) m_fbi.ax = (s16)data;
			break;

		case voodoo_regs::reg_fvertexAy:
			data = float_to_int32(data, 4);
			[[fallthrough]];
		case voodoo_regs::reg_vertexAy:
			if (chips & 1) m_fbi.ay = (s16)data;
			break;

		case voodoo_regs::reg_fvertexBx:
			data = float_to_int32(data, 4);
			[[fallthrough]];
		case voodoo_regs::reg_vertexBx:
			if (chips & 1) m_fbi.bx = (s16)data;
			break;

		case voodoo_regs::reg_fvertexBy:
			data = float_to_int32(data, 4);
			[[fallthrough]];
		case voodoo_regs::reg_vertexBy:
			if (chips & 1) m_fbi.by = (s16)data;
			break;

		case voodoo_regs::reg_fvertexCx:
			data = float_to_int32(data, 4);
			[[fallthrough]];
		case voodoo_regs::reg_vertexCx:
			if (chips & 1) m_fbi.cx = (s16)data;
			break;

		case voodoo_regs::reg_fvertexCy:
			data = float_to_int32(data, 4);
			[[fallthrough]];
		case voodoo_regs::reg_vertexCy:
			if (chips & 1) m_fbi.cy = (s16)data;
			break;

		/* RGB data is 12.12 formatted fixed point */
		case voodoo_regs::reg_fstartR:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_startR:
			if (chips & 1) m_fbi.startr = (s32)(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fstartG:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_startG:
			if (chips & 1) m_fbi.startg = (s32)(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fstartB:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_startB:
			if (chips & 1) m_fbi.startb = (s32)(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fstartA:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_startA:
			if (chips & 1) m_fbi.starta = (s32)(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdRdX:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dRdX:
			if (chips & 1) m_fbi.drdx = (s32)(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdGdX:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dGdX:
			if (chips & 1) m_fbi.dgdx = (s32)(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdBdX:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dBdX:
			if (chips & 1) m_fbi.dbdx = (s32)(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdAdX:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dAdX:
			if (chips & 1) m_fbi.dadx = (s32)(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdRdY:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dRdY:
			if (chips & 1) m_fbi.drdy = (s32)(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdGdY:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dGdY:
			if (chips & 1) m_fbi.dgdy = (s32)(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdBdY:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dBdY:
			if (chips & 1) m_fbi.dbdy = (s32)(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdAdY:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dAdY:
			if (chips & 1) m_fbi.dady = (s32)(data << 8) >> 8;
			break;

		/* Z data is 20.12 formatted fixed point */
		case voodoo_regs::reg_fstartZ:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_startZ:
			if (chips & 1) m_fbi.startz = (s32)data;
			break;

		case voodoo_regs::reg_fdZdX:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dZdX:
			if (chips & 1) m_fbi.dzdx = (s32)data;
			break;

		case voodoo_regs::reg_fdZdY:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dZdY:
			if (chips & 1) m_fbi.dzdy = (s32)data;
			break;

		/* S,T data is 14.18 formatted fixed point, converted to 16.32 internally */
		case voodoo_regs::reg_fstartS:
			data64 = float_to_int64(data, 32);
			if (chips & 2) m_tmu[0].m_starts = data64;
			if (chips & 4) m_tmu[1].m_starts = data64;
			break;
		case voodoo_regs::reg_startS:
			if (chips & 2) m_tmu[0].m_starts = (s64)(s32)data << 14;
			if (chips & 4) m_tmu[1].m_starts = (s64)(s32)data << 14;
			break;

		case voodoo_regs::reg_fstartT:
			data64 = float_to_int64(data, 32);
			if (chips & 2) m_tmu[0].m_startt = data64;
			if (chips & 4) m_tmu[1].m_startt = data64;
			break;
		case voodoo_regs::reg_startT:
			if (chips & 2) m_tmu[0].m_startt = (s64)(s32)data << 14;
			if (chips & 4) m_tmu[1].m_startt = (s64)(s32)data << 14;
			break;

		case voodoo_regs::reg_fdSdX:
			data64 = float_to_int64(data, 32);
			if (chips & 2) m_tmu[0].m_dsdx = data64;
			if (chips & 4) m_tmu[1].m_dsdx = data64;
			break;
		case voodoo_regs::reg_dSdX:
			if (chips & 2) m_tmu[0].m_dsdx = (s64)(s32)data << 14;
			if (chips & 4) m_tmu[1].m_dsdx = (s64)(s32)data << 14;
			break;

		case voodoo_regs::reg_fdTdX:
			data64 = float_to_int64(data, 32);
			if (chips & 2) m_tmu[0].m_dtdx = data64;
			if (chips & 4) m_tmu[1].m_dtdx = data64;
			break;
		case voodoo_regs::reg_dTdX:
			if (chips & 2) m_tmu[0].m_dtdx = (s64)(s32)data << 14;
			if (chips & 4) m_tmu[1].m_dtdx = (s64)(s32)data << 14;
			break;

		case voodoo_regs::reg_fdSdY:
			data64 = float_to_int64(data, 32);
			if (chips & 2) m_tmu[0].m_dsdy = data64;
			if (chips & 4) m_tmu[1].m_dsdy = data64;
			break;
		case voodoo_regs::reg_dSdY:
			if (chips & 2) m_tmu[0].m_dsdy = (s64)(s32)data << 14;
			if (chips & 4) m_tmu[1].m_dsdy = (s64)(s32)data << 14;
			break;

		case voodoo_regs::reg_fdTdY:
			data64 = float_to_int64(data, 32);
			if (chips & 2) m_tmu[0].m_dtdy = data64;
			if (chips & 4) m_tmu[1].m_dtdy = data64;
			break;
		case voodoo_regs::reg_dTdY:
			if (chips & 2) m_tmu[0].m_dtdy = (s64)(s32)data << 14;
			if (chips & 4) m_tmu[1].m_dtdy = (s64)(s32)data << 14;
			break;

		/* W data is 2.30 formatted fixed point, converted to 16.32 internally */
		case voodoo_regs::reg_fstartW:
			data64 = float_to_int64(data, 32);
			if (chips & 1) m_fbi.startw = data64;
			if (chips & 2) m_tmu[0].m_startw = data64;
			if (chips & 4) m_tmu[1].m_startw = data64;
			break;
		case voodoo_regs::reg_startW:
			if (chips & 1) m_fbi.startw = (s64)(s32)data << 2;
			if (chips & 2) m_tmu[0].m_startw = (s64)(s32)data << 2;
			if (chips & 4) m_tmu[1].m_startw = (s64)(s32)data << 2;
			break;

		case voodoo_regs::reg_fdWdX:
			data64 = float_to_int64(data, 32);
			if (chips & 1) m_fbi.dwdx = data64;
			if (chips & 2) m_tmu[0].m_dwdx = data64;
			if (chips & 4) m_tmu[1].m_dwdx = data64;
			break;
		case voodoo_regs::reg_dWdX:
			if (chips & 1) m_fbi.dwdx = (s64)(s32)data << 2;
			if (chips & 2) m_tmu[0].m_dwdx = (s64)(s32)data << 2;
			if (chips & 4) m_tmu[1].m_dwdx = (s64)(s32)data << 2;
			break;

		case voodoo_regs::reg_fdWdY:
			data64 = float_to_int64(data, 32);
			if (chips & 1) m_fbi.dwdy = data64;
			if (chips & 2) m_tmu[0].m_dwdy = data64;
			if (chips & 4) m_tmu[1].m_dwdy = data64;
			break;
		case voodoo_regs::reg_dWdY:
			if (chips & 1) m_fbi.dwdy = (s64)(s32)data << 2;
			if (chips & 2) m_tmu[0].m_dwdy = (s64)(s32)data << 2;
			if (chips & 4) m_tmu[1].m_dwdy = (s64)(s32)data << 2;
			break;

		/* setup bits */
		case voodoo_regs::reg_sARGB:
			if (chips & 1)
			{
				rgb_t rgbdata(data);
				m_reg.write_float(voodoo_regs::reg_sAlpha, rgbdata.a());
				m_reg.write_float(voodoo_regs::reg_sRed, rgbdata.r());
				m_reg.write_float(voodoo_regs::reg_sGreen, rgbdata.g());
				m_reg.write_float(voodoo_regs::reg_sBlue, rgbdata.b());
			}
			break;

		/* mask off invalid bits for different cards */
		case voodoo_regs::reg_fbzColorPath:
			if (m_type < TYPE_VOODOO_2)
				data &= 0x0fffffff;
			if (chips & 1) m_reg.write(regnum, data);
			break;

		case voodoo_regs::reg_fbzMode:
			if (m_type < TYPE_VOODOO_2)
				data &= 0x001fffff;
			if (chips & 1) m_reg.write(regnum, data);
			break;

		case voodoo_regs::reg_fogMode:
			if (m_type < TYPE_VOODOO_2)
				data &= 0x0000003f;
			if (chips & 1) m_reg.write(regnum, data);
			break;

		/* triangle drawing */
		case voodoo_regs::reg_triangleCMD:
			m_fbi.sign = data;
			cycles = triangle();
			break;

		case voodoo_regs::reg_ftriangleCMD:
			m_fbi.sign = data;
			cycles = triangle();
			break;

		case voodoo_regs::reg_sBeginTriCMD:
			cycles = begin_triangle();
			break;

		case voodoo_regs::reg_sDrawTriCMD:
			cycles = draw_triangle();
			break;

		/* other commands */
		case voodoo_regs::reg_nopCMD:
			m_poly->wait(m_regnames[regnum]);
			if (data & 1)
				reset_counters();
			if (data & 2)
				m_reg.write(voodoo_regs::reg_fbiTrianglesOut, 0);
			break;

		case voodoo_regs::reg_fastfillCMD:
			cycles = fastfill();
			break;

		case voodoo_regs::reg_swapbufferCMD:
			m_poly->wait(m_regnames[regnum]);
			cycles = swapbuffer(data);
			break;

		case voodoo_regs::reg_userIntrCMD:
			m_poly->wait(m_regnames[regnum]);

			// Bit 5 of intrCtrl enables user interrupts
			if (m_reg.intr_ctrl().user_interrupt_enable())
			{
				// Bits 19:12 are set to cmd 9:2, bit 11 is user interrupt flag
				m_reg.clear_set(voodoo_regs::reg_intrCtrl,
					reg_intr_ctrl::EXTERNAL_PIN_ACTIVE | reg_intr_ctrl::USER_INTERRUPT_TAG_MASK,
					((data << 10) & reg_intr_ctrl::USER_INTERRUPT_TAG_MASK) | reg_intr_ctrl::USER_INTERRUPT_GENERATED);

				// Signal pci interrupt handler
				if (!m_pciint.isnull())
					m_pciint(true);
			}
			break;

		/* gamma table access -- Voodoo/Voodoo2 only */
		case voodoo_regs::reg_clutData:
			if (m_type <= TYPE_VOODOO_2 && (chips & 1))
			{
				m_poly->wait(m_regnames[regnum]);
				if (m_reg.fbi_init1().video_timing_reset() == 0)
				{
					int index = data >> 24;
					if (index <= 32)
					{
						m_fbi.clut[index] = data;
						m_fbi.clut_dirty = true;
					}
				}
				else
					logerror("clutData ignored because video timing reset = 1\n");
			}
			break;

		/* external DAC access -- Voodoo/Voodoo2 only */
		case voodoo_regs::reg_dacData:
			if (m_type <= TYPE_VOODOO_2 && (chips & 1))
			{
				m_poly->wait(m_regnames[regnum]);
				if (!(data & 0x800))
					m_dac.data_w((data >> 8) & 7, data & 0xff);
				else
					m_dac.data_r((data >> 8) & 7);
			}
			break;

		/* vertical sync rate -- Voodoo/Voodoo2 only */
		case voodoo_regs::reg_hSync:
		case voodoo_regs::reg_vSync:
		case voodoo_regs::reg_backPorch:
		case voodoo_regs::reg_videoDimensions:
			if (m_type <= TYPE_VOODOO_2 && (chips & 1))
			{
				m_poly->wait(m_regnames[regnum]);
				m_reg.write(regnum, data);

				auto const hsync = m_reg.hsync();
				auto const vsync = m_reg.vsync();
				auto const back_porch = m_reg.back_porch();
				auto const video_dimensions = m_reg.video_dimensions();
				if (hsync.raw() != 0 && vsync.raw() != 0 && video_dimensions.raw() != 0 && back_porch.raw() != 0)
				{
					u32 htotal = hsync.hsync_off(m_type) + 1 + hsync.hsync_on(m_type) + 1;
					u32 vtotal = vsync.vsync_off(m_type) + vsync.vsync_on(m_type);
					u32 hvis = video_dimensions.xwidth(m_type);
					u32 vvis = video_dimensions.yheight(m_type);
					u32 hbp = back_porch.horizontal(m_type) + 2;
					u32 vbp = back_porch.vertical(m_type);

					/* create a new visarea */
					rectangle visarea(hbp, hbp + std::max(hvis - 1, 0), vbp, vbp + std::max(vvis - 1, 0));

					/* keep within bounds */
					visarea.max_x = std::min<s32>(visarea.max_x, htotal - 1);
					visarea.max_y = std::min<s32>(visarea.max_y, vtotal - 1);

					/* compute the new period for standard res, medium res, and VGA res */
					attoseconds_t stdperiod = HZ_TO_ATTOSECONDS(15750) * vtotal;
					attoseconds_t medperiod = HZ_TO_ATTOSECONDS(25000) * vtotal;
					attoseconds_t vgaperiod = HZ_TO_ATTOSECONDS(31500) * vtotal;

					/* compute a diff against the current refresh period */
					attoseconds_t refresh = m_screen->frame_period().attoseconds();
					attoseconds_t stddiff = std::abs(stdperiod - refresh);
					attoseconds_t meddiff = std::abs(medperiod - refresh);
					attoseconds_t vgadiff = std::abs(vgaperiod - refresh);

					osd_printf_debug("hSync=%08X  vSync=%08X  backPorch=%08X  videoDimensions=%08X\n",
						hsync.raw(), vsync.raw(), back_porch.raw(), video_dimensions.raw());
					osd_printf_debug("Horiz: %d-%d (%d total)  Vert: %d-%d (%d total) -- ", visarea.min_x, visarea.max_x, htotal, visarea.min_y, visarea.max_y, vtotal);

					/* configure the screen based on which one matches the closest */
					if (stddiff < meddiff && stddiff < vgadiff)
					{
						m_screen->configure(htotal, vtotal, visarea, stdperiod);
						osd_printf_debug("Standard resolution, %f Hz\n", ATTOSECONDS_TO_HZ(stdperiod));
					}
					else if (meddiff < vgadiff)
					{
						m_screen->configure(htotal, vtotal, visarea, medperiod);
						osd_printf_debug("Medium resolution, %f Hz\n", ATTOSECONDS_TO_HZ(medperiod));
					}
					else
					{
						m_screen->configure(htotal, vtotal, visarea, vgaperiod);
						osd_printf_debug("VGA resolution, %f Hz\n", ATTOSECONDS_TO_HZ(vgaperiod));
					}

					/* configure the new framebuffer info */
					m_fbi.width = hvis;
					m_fbi.height = vvis;
					m_fbi.xoffs = hbp;
					m_fbi.yoffs = vbp;
					m_fbi.vsyncstart = vsync.vsync_off(m_type);
					m_fbi.vsyncstop = vsync.vsync_on(m_type);
					osd_printf_debug("yoffs: %d vsyncstart: %d vsyncstop: %d\n", vbp, m_fbi.vsyncstart, m_fbi.vsyncstop);
					/* recompute the time of VBLANK */
					adjust_vblank_timer();

					/* if changing dimensions, update video memory layout */
					if (regnum == voodoo_regs::reg_videoDimensions)
						recompute_video_memory();
				}
			}
			break;

		/* fbiInit0 can only be written if initEnable says we can -- Voodoo/Voodoo2 only */
		case voodoo_regs::reg_fbiInit0:
			m_poly->wait(m_regnames[regnum]);
			if (m_type <= TYPE_VOODOO_2 && (chips & 1) && m_pci.init_enable.enable_hw_init())
			{
				m_reg.write(regnum, data);
				if (m_reg.fbi_init0().graphics_reset())
					soft_reset();
				if (m_reg.fbi_init0().fifo_reset())
					m_pci.fifo.reset();
				recompute_video_memory();
			}
			break;

		/* fbiInit5-7 are Voodoo 2-only; ignore them on anything else */
		case voodoo_regs::reg_fbiInit5:
		case voodoo_regs::reg_fbiInit6:
			if (m_type < TYPE_VOODOO_2)
				break;
			[[fallthrough]];

		/* fbiInitX can only be written if initEnable says we can -- Voodoo/Voodoo2 only */
		/* most of these affect memory layout, so always recompute that when done */
		case voodoo_regs::reg_fbiInit1:
		case voodoo_regs::reg_fbiInit2:
		case voodoo_regs::reg_fbiInit4:
			m_poly->wait(m_regnames[regnum]);
			if (m_type <= TYPE_VOODOO_2 && (chips & 1) && m_pci.init_enable.enable_hw_init())
			{
				m_reg.write(regnum, data);
				recompute_video_memory();
				m_fbi.video_changed = true;
			}
			break;

		case voodoo_regs::reg_fbiInit3:
			m_poly->wait(m_regnames[regnum]);
			if (m_type <= TYPE_VOODOO_2 && (chips & 1) && m_pci.init_enable.enable_hw_init())
			{
				m_reg.write(regnum, data);
				m_alt_regmap = m_reg.fbi_init3().tri_register_remap();
				m_fbi.yorigin = m_reg.fbi_init3().yorigin_subtract();
				recompute_video_memory();
			}
			break;

		case voodoo_regs::reg_fbiInit7:
/*      case voodoo_regs::reg_swapPending: -- Banshee */
			if (m_type == TYPE_VOODOO_2 && (chips & 1) && m_pci.init_enable.enable_hw_init())
			{
				m_poly->wait(m_regnames[regnum]);
				m_reg.write(regnum, data);
				m_fbi.cmdfifo[0].enable = m_reg.fbi_init7().cmdfifo_enable();
				m_fbi.cmdfifo[0].count_holes = !m_reg.fbi_init7().disable_cmdfifo_holes();
			}
			else if (m_type >= TYPE_VOODOO_BANSHEE)
				m_fbi.swaps_pending++;
			break;

		/* cmdFifo -- Voodoo2 only */
		case voodoo_regs::reg_cmdFifoBaseAddr:
			if (m_type == TYPE_VOODOO_2 && (chips & 1))
			{
				m_poly->wait(m_regnames[regnum]);
				m_reg.write(regnum, data);
				m_fbi.cmdfifo[0].base = (data & 0x3ff) << 12;
				m_fbi.cmdfifo[0].end = (((data >> 16) & 0x3ff) + 1) << 12;
			}
			break;

		case voodoo_regs::reg_cmdFifoBump:
			if (m_type == TYPE_VOODOO_2 && (chips & 1))
				fatalerror("cmdFifoBump\n");
			break;

		case voodoo_regs::reg_cmdFifoRdPtr:
			if (m_type == TYPE_VOODOO_2 && (chips & 1))
				m_fbi.cmdfifo[0].rdptr = data;
			break;

		case voodoo_regs::reg_cmdFifoAMin:
/*      case voodoo_regs::reg_colBufferAddr: -- Banshee */
			if (m_type == TYPE_VOODOO_2 && (chips & 1))
				m_fbi.cmdfifo[0].amin = data;
			else if (m_type >= TYPE_VOODOO_BANSHEE && (chips & 1))
				m_fbi.rgboffs[1] = data & m_fbi.mask & ~0x0f;
			break;

		case voodoo_regs::reg_cmdFifoAMax:
/*      case voodoo_regs::reg_colBufferStride: -- Banshee */
			if (m_type == TYPE_VOODOO_2 && (chips & 1))
				m_fbi.cmdfifo[0].amax = data;
			else if (m_type >= TYPE_VOODOO_BANSHEE && (chips & 1))
			{
				if (data & 0x8000)
					m_fbi.rowpixels = (data & 0x7f) << 6;
				else
					m_fbi.rowpixels = (data & 0x3fff) >> 1;
			}
			break;

		case voodoo_regs::reg_cmdFifoDepth:
/*      case voodoo_regs::reg_auxBufferAddr: -- Banshee */
			if (m_type == TYPE_VOODOO_2 && (chips & 1))
				m_fbi.cmdfifo[0].depth = data;
			else if (m_type >= TYPE_VOODOO_BANSHEE && (chips & 1))
				m_fbi.auxoffs = data & m_fbi.mask & ~0x0f;
			break;

		case voodoo_regs::reg_cmdFifoHoles:
/*      case voodoo_regs::reg_auxBufferStride: -- Banshee */
			if (m_type == TYPE_VOODOO_2 && (chips & 1))
				m_fbi.cmdfifo[0].holes = data;
			else if (m_type >= TYPE_VOODOO_BANSHEE && (chips & 1))
			{
				int rowpixels;

				if (data & 0x8000)
					rowpixels = (data & 0x7f) << 6;
				else
					rowpixels = (data & 0x3fff) >> 1;
				if (m_fbi.rowpixels != rowpixels)
					fatalerror("aux buffer stride differs from color buffer stride\n");
			}
			break;

		/* nccTable entries are processed and expanded immediately */
		case voodoo_regs::reg_nccTable + 0:
		case voodoo_regs::reg_nccTable + 1:
		case voodoo_regs::reg_nccTable + 2:
		case voodoo_regs::reg_nccTable + 3:
		case voodoo_regs::reg_nccTable + 4:
		case voodoo_regs::reg_nccTable + 5:
		case voodoo_regs::reg_nccTable + 6:
		case voodoo_regs::reg_nccTable + 7:
		case voodoo_regs::reg_nccTable + 8:
		case voodoo_regs::reg_nccTable + 9:
		case voodoo_regs::reg_nccTable + 10:
		case voodoo_regs::reg_nccTable + 11:
			m_poly->wait(m_regnames[regnum]);
			if (chips & 2) m_tmu[0].m_ncc[0].write(regnum - voodoo_regs::reg_nccTable, data);
			if (chips & 4) m_tmu[1].m_ncc[0].write(regnum - voodoo_regs::reg_nccTable, data);
			break;

		case voodoo_regs::reg_nccTable + 12:
		case voodoo_regs::reg_nccTable + 13:
		case voodoo_regs::reg_nccTable + 14:
		case voodoo_regs::reg_nccTable + 15:
		case voodoo_regs::reg_nccTable + 16:
		case voodoo_regs::reg_nccTable + 17:
		case voodoo_regs::reg_nccTable + 18:
		case voodoo_regs::reg_nccTable + 19:
		case voodoo_regs::reg_nccTable + 20:
		case voodoo_regs::reg_nccTable + 21:
		case voodoo_regs::reg_nccTable + 22:
		case voodoo_regs::reg_nccTable + 23:
			m_poly->wait(m_regnames[regnum]);
			if (chips & 2) m_tmu[0].m_ncc[1].write(regnum - (voodoo_regs::reg_nccTable + 12), data);
			if (chips & 4) m_tmu[1].m_ncc[1].write(regnum - (voodoo_regs::reg_nccTable + 12), data);
			break;

		/* fogTable entries are processed and expanded immediately */
		case voodoo_regs::reg_fogTable + 0:
		case voodoo_regs::reg_fogTable + 1:
		case voodoo_regs::reg_fogTable + 2:
		case voodoo_regs::reg_fogTable + 3:
		case voodoo_regs::reg_fogTable + 4:
		case voodoo_regs::reg_fogTable + 5:
		case voodoo_regs::reg_fogTable + 6:
		case voodoo_regs::reg_fogTable + 7:
		case voodoo_regs::reg_fogTable + 8:
		case voodoo_regs::reg_fogTable + 9:
		case voodoo_regs::reg_fogTable + 10:
		case voodoo_regs::reg_fogTable + 11:
		case voodoo_regs::reg_fogTable + 12:
		case voodoo_regs::reg_fogTable + 13:
		case voodoo_regs::reg_fogTable + 14:
		case voodoo_regs::reg_fogTable + 15:
		case voodoo_regs::reg_fogTable + 16:
		case voodoo_regs::reg_fogTable + 17:
		case voodoo_regs::reg_fogTable + 18:
		case voodoo_regs::reg_fogTable + 19:
		case voodoo_regs::reg_fogTable + 20:
		case voodoo_regs::reg_fogTable + 21:
		case voodoo_regs::reg_fogTable + 22:
		case voodoo_regs::reg_fogTable + 23:
		case voodoo_regs::reg_fogTable + 24:
		case voodoo_regs::reg_fogTable + 25:
		case voodoo_regs::reg_fogTable + 26:
		case voodoo_regs::reg_fogTable + 27:
		case voodoo_regs::reg_fogTable + 28:
		case voodoo_regs::reg_fogTable + 29:
		case voodoo_regs::reg_fogTable + 30:
		case voodoo_regs::reg_fogTable + 31:
			m_poly->wait(m_regnames[regnum]);
			if (chips & 1)
			{
				int base = 2 * (regnum - voodoo_regs::reg_fogTable);
				m_fbi.fogdelta[base + 0] = (data >> 0) & 0xff;
				m_fbi.fogblend[base + 0] = (data >> 8) & 0xff;
				m_fbi.fogdelta[base + 1] = (data >> 16) & 0xff;
				m_fbi.fogblend[base + 1] = (data >> 24) & 0xff;
			}
			break;

		/* texture modifications cause us to recompute everything */
		case voodoo_regs::reg_textureMode:
		case voodoo_regs::reg_tLOD:
		case voodoo_regs::reg_tDetail:
		case voodoo_regs::reg_texBaseAddr:
		case voodoo_regs::reg_texBaseAddr_1:
		case voodoo_regs::reg_texBaseAddr_2:
		case voodoo_regs::reg_texBaseAddr_3_8:
			if (chips & 2)
			{
				if (m_tmu[0].m_reg.read(regnum) != data)
				{
					m_poly->wait(m_regnames[regnum]);
					m_tmu[0].m_regdirty = true;
					m_tmu[0].m_reg.write(regnum, data);
				}
			}
			if (chips & 4)
			{
				if (m_tmu[1].m_reg.read(regnum) != data)
				{
					m_poly->wait(m_regnames[regnum]);
					m_tmu[1].m_regdirty = true;
					m_tmu[1].m_reg.write(regnum, data);
				}
			}
			break;

		case voodoo_regs::reg_trexInit1:
			logerror("VOODOO.REG:%s(%d) write = %08X\n", (regnum < 0x384 / 4) ? m_regnames[regnum] : "oob", chips, data);
			/* send tmu config data to the frame buffer */
			m_send_config = (TREXINIT_SEND_TMU_CONFIG(data) > 0);
			goto default_case;

		/* these registers are referenced in the renderer; we must wait for pending work before changing */
		case voodoo_regs::reg_chromaRange:
		case voodoo_regs::reg_chromaKey:
		case voodoo_regs::reg_fogColor:
		case voodoo_regs::reg_stipple:
		case voodoo_regs::reg_clipLowYHighY:
		case voodoo_regs::reg_clipLeftRight:
			m_poly->wait(m_regnames[regnum]);
			[[fallthrough]];
		/* by default, just feed the data to the chips */
		default:
default_case:
			if (chips & 1) m_reg.write(regnum, data);
			if (chips & 2) m_tmu[0].m_reg.write(regnum, data);
			if (chips & 4) m_tmu[1].m_reg.write(regnum, data);
			break;
	}

	if (LOG_REGISTERS)
	{
		if (regnum < voodoo_regs::reg_fvertexAx || regnum > voodoo_regs::reg_fdWdY)
			logerror("VOODOO.REG:%s(%d) write = %08X\n", (regnum < 0x384/4) ? m_regnames[regnum] : "oob", chips, origdata);
		else
			logerror("VOODOO.REG:%s(%d) write = %f\n", (regnum < 0x384/4) ? m_regnames[regnum] : "oob", chips, (double) u2f(origdata));
	}

	return cycles;
}



/*************************************
 *
 *  Voodoo LFB writes
 *
 *************************************/
s32 voodoo_device::lfb_direct_w(offs_t offset, u32 data, u32 mem_mask)
{
	/* statistics */
	m_stats.lfb_writes++;

	/* byte swizzling */
	auto const lfbmode = m_reg.lfb_mode();
	if (lfbmode.byte_swizzle_writes())
	{
		data = swapendian_int32(data);
		mem_mask = swapendian_int32(mem_mask);
	}

	/* word swapping */
	if (lfbmode.word_swap_writes())
	{
		data = (data << 16) | (data >> 16);
		mem_mask = (mem_mask << 16) | (mem_mask >> 16);
	}

	// TODO: This direct write is not verified.
	// For direct lfb access just write the data
	/* compute X,Y */
	offset <<= 1;
	int const x = offset & ((1 << m_fbi.lfb_stride) - 1);
	int const y = (offset >> m_fbi.lfb_stride);
	u16 *const dest = (u16 *)(m_fbi.ram + m_fbi.lfb_base*4);
	u32 const destmax = (m_fbi.mask + 1 - m_fbi.lfb_base*4) / 2;
	u32 const bufoffs = y * m_fbi.rowpixels + x;
	if (bufoffs >= destmax) {
		logerror("lfb_direct_w: Buffer offset out of bounds x=%i y=%i offset=%08X bufoffs=%08X data=%08X\n", x, y, offset, bufoffs, data);
		return 0;
	}
	if (ACCESSING_BITS_0_15)
		dest[bufoffs + 0] = data&0xffff;
	if (ACCESSING_BITS_16_31)
		dest[bufoffs + 1] = data>>16;
	// Need to notify that frame buffer has changed
	m_fbi.video_changed = true;
	if (LOG_LFB) logerror("VOODOO.LFB:write direct (%d,%d) = %08X & %08X\n", x, y, data, mem_mask);
	return 0;
}

s32 voodoo_device::lfb_w(offs_t offset, u32 data, u32 mem_mask)
{
	// statistics
	m_stats.lfb_writes++;

	// byte swizzling
	auto const lfbmode = m_reg.lfb_mode();
	if (lfbmode.byte_swizzle_writes())
	{
		data = swapendian_int32(data);
		mem_mask = swapendian_int32(mem_mask);
	}

	// word swapping
	if (lfbmode.word_swap_writes())
	{
		data = (data << 16) | (data >> 16);
		mem_mask = (mem_mask << 16) | (mem_mask >> 16);
	}

	// extract default depth value from low bits of zaColor
	u16 sz[2];
	sz[0] = sz[1] = m_reg.za_color() & 0xffff;

	// if not otherwise specified, alpha defaults to the upper bits of zaColor
	u32 src_alpha = m_reg.za_color() >> 24;

	// extract color information from the data
	rgb_t src_color[2];
	u32 mask = 0;
	switch (16 * lfbmode.rgba_lanes() + lfbmode.write_format())
	{
		case 16*0 + 0:      // ARGB, format 0: 16-bit RGB 5-6-5
		case 16*2 + 0:      // RGBA, format 0: 16-bit RGB 5-6-5
			src_color[0] = rgbexpand<5,6,5>(data, 11,  5,  0).set_a(src_alpha);
			src_color[1] = rgbexpand<5,6,5>(data, 27, 21, 16).set_a(src_alpha);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*1 + 0:      // ABGR, format 0: 16-bit RGB 5-6-5
		case 16*3 + 0:      // BGRA, format 0: 16-bit RGB 5-6-5
			src_color[0] = rgbexpand<5,6,5>(data,  0,  5, 11).set_a(src_alpha);
			src_color[1] = rgbexpand<5,6,5>(data, 16, 21, 27).set_a(src_alpha);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*0 + 1:      // ARGB, format 1: 16-bit RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data, 10,  5,  0).set_a(src_alpha);
			src_color[1] = rgbexpand<5,5,5>(data, 26, 21, 16).set_a(src_alpha);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*1 + 1:      // ABGR, format 1: 16-bit RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data,  0,  5, 10).set_a(src_alpha);
			src_color[1] = rgbexpand<5,5,5>(data, 16, 21, 26).set_a(src_alpha);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*2 + 1:      // RGBA, format 1: 16-bit RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data, 11,  6,  1).set_a(src_alpha);
			src_color[1] = rgbexpand<5,5,5>(data, 27, 22, 17).set_a(src_alpha);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*3 + 1:      // BGRA, format 1: 16-bit RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data,  1,  6, 11).set_a(src_alpha);
			src_color[1] = rgbexpand<5,5,5>(data, 17, 22, 27).set_a(src_alpha);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*0 + 2:      // ARGB, format 2: 16-bit ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data, 15, 10,  5,  0);
			src_color[1] = argbexpand<1,5,5,5>(data, 31, 26, 21, 16);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | ((LFB_RGB_PRESENT | LFB_ALPHA_PRESENT) << 4);
			offset <<= 1;
			break;

		case 16*1 + 2:      // ABGR, format 2: 16-bit ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data, 15,  0,  5, 10);
			src_color[1] = argbexpand<1,5,5,5>(data, 31, 16, 21, 26);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | ((LFB_RGB_PRESENT | LFB_ALPHA_PRESENT) << 4);
			offset <<= 1;
			break;

		case 16*2 + 2:      // RGBA, format 2: 16-bit ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data,  0, 11,  6,  1);
			src_color[1] = argbexpand<1,5,5,5>(data, 16, 27, 22, 17);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | ((LFB_RGB_PRESENT | LFB_ALPHA_PRESENT) << 4);
			offset <<= 1;
			break;

		case 16*3 + 2:      // BGRA, format 2: 16-bit ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data,  0,  1,  6, 11);
			src_color[1] = argbexpand<1,5,5,5>(data, 16, 17, 22, 27);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | ((LFB_RGB_PRESENT | LFB_ALPHA_PRESENT) << 4);
			offset <<= 1;
			break;

		case 16*0 + 4:      // ARGB, format 4: 32-bit RGB x-8-8-8
			src_color[0] = rgbexpand<8,8,8>(data, 16,  8,  0);
			mask = LFB_RGB_PRESENT;
			break;

		case 16*1 + 4:      // ABGR, format 4: 32-bit RGB x-8-8-8
			src_color[0] = rgbexpand<8,8,8>(data,  0,  8, 16);
			mask = LFB_RGB_PRESENT;
			break;

		case 16*2 + 4:      // RGBA, format 4: 32-bit RGB x-8-8-8
			src_color[0] = rgbexpand<8,8,8>(data, 24, 16,  8);
			mask = LFB_RGB_PRESENT;
			break;

		case 16*3 + 4:      // BGRA, format 4: 32-bit RGB x-8-8-8
			src_color[0] = rgbexpand<8,8,8>(data,  8, 16, 24);
			mask = LFB_RGB_PRESENT;
			break;

		case 16*0 + 5:      // ARGB, format 5: 32-bit ARGB 8-8-8-8
			src_color[0] = argbexpand<8,8,8,8>(data, 24, 16,  8,  0);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT;
			break;

		case 16*1 + 5:      // ABGR, format 5: 32-bit ARGB 8-8-8-8
			src_color[0] = argbexpand<8,8,8,8>(data, 24,  0,  8, 16);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT;
			break;

		case 16*2 + 5:      // RGBA, format 5: 32-bit ARGB 8-8-8-8
			src_color[0] = argbexpand<8,8,8,8>(data,  0, 24, 16,  8);
			break;

		case 16*3 + 5:      // BGRA, format 5: 32-bit ARGB 8-8-8-8
			src_color[0] = argbexpand<8,8,8,8>(data,  0,  8, 16, 24);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT;
			break;

		case 16*0 + 12:     // ARGB, format 12: 32-bit depth+RGB 5-6-5
		case 16*2 + 12:     // RGBA, format 12: 32-bit depth+RGB 5-6-5
			src_color[0] = rgbexpand<5,6,5>(data, 11,  5,  0).set_a(src_alpha);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*1 + 12:     // ABGR, format 12: 32-bit depth+RGB 5-6-5
		case 16*3 + 12:     // BGRA, format 12: 32-bit depth+RGB 5-6-5
			src_color[0] = rgbexpand<5,6,5>(data,  0,  5, 11).set_a(src_alpha);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*0 + 13:     // ARGB, format 13: 32-bit depth+RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data, 10,  5,  0).set_a(src_alpha);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*1 + 13:     // ABGR, format 13: 32-bit depth+RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data,  0,  5, 10).set_a(src_alpha);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*2 + 13:     // RGBA, format 13: 32-bit depth+RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data, 11,  6,  1).set_a(src_alpha);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*3 + 13:     // BGRA, format 13: 32-bit depth+RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data,  1,  6, 11).set_a(src_alpha);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*0 + 14:     // ARGB, format 14: 32-bit depth+ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data, 15, 10,  5,  0);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*1 + 14:     // ABGR, format 14: 32-bit depth+ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data, 15,  0,  5, 10);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*2 + 14:     // RGBA, format 14: 32-bit depth+ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data,  0, 11,  6,  1);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*3 + 14:     // BGRA, format 14: 32-bit depth+ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data,  0,  1,  6, 11);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*0 + 15:     // ARGB, format 15: 16-bit depth
		case 16*1 + 15:     // ARGB, format 15: 16-bit depth
		case 16*2 + 15:     // ARGB, format 15: 16-bit depth
		case 16*3 + 15:     // ARGB, format 15: 16-bit depth
			sz[0] = data & 0xffff;
			sz[1] = data >> 16;
			mask = LFB_DEPTH_PRESENT | (LFB_DEPTH_PRESENT << 4);
			offset <<= 1;
			break;

		default:            // reserved
			logerror("lfb_w: Unknown format\n");
			return 0;
	}

	// compute X,Y
	s32 x = offset & ((1 << m_fbi.lfb_stride) - 1);
	s32 y = (offset >> m_fbi.lfb_stride) & 0x3ff;

	// adjust the mask based on which half of the data is written
	if (!ACCESSING_BITS_0_15)
		mask &= ~(0x0f - LFB_DEPTH_PRESENT_MSW);
	if (!ACCESSING_BITS_16_31)
		mask &= ~(0xf0 + LFB_DEPTH_PRESENT_MSW);

	// select the target buffer
	int destbuf = (m_type >= TYPE_VOODOO_BANSHEE) ? 1 : lfbmode.write_buffer_select();
	u32 destmax;
	u16 *dest;
	switch (destbuf)
	{
		case 0:         // front buffer
			dest = (u16 *)(m_fbi.ram + m_fbi.rgboffs[m_fbi.frontbuf]);
			destmax = (m_fbi.mask + 1 - m_fbi.rgboffs[m_fbi.frontbuf]) / 2;
			m_fbi.video_changed = true;
			break;

		case 1:         // back buffer
			dest = (u16 *)(m_fbi.ram + m_fbi.rgboffs[m_fbi.backbuf]);
			destmax = (m_fbi.mask + 1 - m_fbi.rgboffs[m_fbi.backbuf]) / 2;
			break;

		default:        // reserved
			return 0;
	}
	u16 *depth = (u16 *)(m_fbi.ram + m_fbi.auxoffs);
	u32 depthmax = (m_fbi.mask + 1 - m_fbi.auxoffs) / 2;

	// simple case: no pipeline
	auto const fbzmode = m_reg.fbz_mode();
	if (!lfbmode.enable_pixel_pipeline())
	{
		if (LOG_LFB) logerror("VOODOO.LFB:write raw mode %X (%d,%d) = %08X & %08X\n", lfbmode.write_format(), x, y, data, mem_mask);

		// determine the screen Y
		s32 scry = y;
		if (lfbmode.y_origin())
			scry = m_fbi.yorigin - y;

		// advance pointers to the proper row
		u32 bufoffs = scry * m_fbi.rowpixels + x;

		// wait for any outstanding work to finish
		m_poly->wait("LFB Write");

		// loop over up to two pixels
		voodoo::dither_helper dither(scry, fbzmode);
		for (int pix = 0; mask != 0; pix++)
		{
			// make sure we care about this pixel
			if ((mask & 0x0f) != 0)
			{
				// write to the RGB buffer
				rgb_t pixel = src_color[pix];
				if ((mask & LFB_RGB_PRESENT) && bufoffs < destmax)
					dest[bufoffs] = dither.pixel(x, pixel.r(), pixel.g(), pixel.b());

				// make sure we have an aux buffer to write to
				if (depth != nullptr && bufoffs < depthmax)
				{
					// write to the alpha buffer
					if ((mask & LFB_ALPHA_PRESENT) && fbzmode.enable_alpha_planes())
						depth[bufoffs] = pixel.a();

					// write to the depth buffer
					if ((mask & (LFB_DEPTH_PRESENT | LFB_DEPTH_PRESENT_MSW)) && !fbzmode.enable_alpha_planes())
						depth[bufoffs] = sz[pix];
				}

				// track pixel writes to the frame buffer regardless of mask
				m_reg.add(voodoo_regs::reg_fbiPixelsOut, 1);
			}

			// advance our pointers
			bufoffs++;
			x++;
			mask >>= 4;
		}
	}

	// tricky case: run the full pixel pipeline on the pixel
	else
	{
		if (LOG_LFB) logerror("VOODOO.LFB:write pipelined mode %X (%d,%d) = %08X & %08X\n", lfbmode.write_format(), x, y, data, mem_mask);

		// determine the screen Y
		s32 scry = y;
		if (fbzmode.y_origin())
			scry = m_fbi.yorigin - y;

		// advance pointers to the proper row
		dest += scry * m_fbi.rowpixels;
		if (depth != nullptr)
			depth += scry * m_fbi.rowpixels;

		// make a dummy poly_extra_data structure with some cached values
		poly_extra_data extra;
		extra.color0 = m_reg.color0().argb();
		extra.color1 = m_reg.color1().argb();
		extra.zacolor = m_reg.za_color();

		// loop over up to two pixels
		auto const fbzcp = m_reg.fbz_colorpath();
		auto const alphamode = m_reg.alpha_mode();
		auto const fogmode = m_reg.fog_mode();
		voodoo::dither_helper dither(scry, fbzmode, fogmode);
		thread_stats_block &threadstats = m_fbi.lfb_stats;
		rgbaint_t iterargb(0);
		for (int pix = 0; mask != 0; pix++)
		{
			// make sure we care about this pixel
			if ((mask & 0x0f) != 0) do
			{
				threadstats.pixels_in++;

				// apply clipping
				if (fbzmode.enable_clipping())
				{
					if (x < m_reg.clip_left() || x >= m_reg.clip_right() || scry < m_reg.clip_top() || scry >= m_reg.clip_bottom())
					{
						threadstats.clip_fail++;
						break;
					}
				}

				// handle stippling
				if (fbzmode.enable_stipple() && !stipple_test(threadstats, fbzmode, x, scry))
					break;

				// Depth testing value for lfb pipeline writes is directly from write data, no biasing is used
				s32 depthval = u32(sz[pix]);

				// Perform depth testing
				if (fbzmode.enable_depthbuf() && !depth_test(threadstats, extra, fbzmode, depth[x], depthval))
					break;

				// use the RGBA we stashed above
				rgbaint_t color(src_color[pix]);

				// handle chroma key
				if (fbzmode.enable_chromakey() && !chroma_key_test(threadstats, color))
					break;

				// handle alpha mask
				if (fbzmode.enable_alpha_mask() && !alpha_mask_test(threadstats, color.get_a()))
					break;

				// handle alpha test
				if (alphamode.alphatest() && !alpha_test(threadstats, alphamode, color.get_a()))
					break;

				// perform fogging
				rgbaint_t prefog(color);
				if (fogmode.enable_fog())
				{
					s32 iterz = sz[pix] << 12;
					s64 iterw = lfbmode.write_w_select() ? u32(m_reg.za_color() << 16) : u32(sz[pix] << 16);
					apply_fogging(color, extra, fbzmode, fogmode, fbzcp, x, dither, depthval, iterz, iterw, iterargb);
				}

				// wait for any outstanding work to finish
				m_poly->wait("LFB Write");

				// perform alpha blending
				if (alphamode.alphablend())
					alpha_blend(color, fbzmode, alphamode, x, dither, dest[x], depth, prefog);

				// pixel pipeline part 2 handles final output
				write_pixel(threadstats, fbzmode, dither, dest, depth, x, color, depthval);
			} while (0);

			// advance our pointers
			x++;
			mask >>= 4;
		}
	}
	return 0;
}



/*************************************
 *
 *  Voodoo texture RAM writes
 *
 *************************************/

s32 voodoo_device::texture_w(offs_t offset, u32 data)
{
	int tmunum = (offset >> 19) & 0x03;
	tmu_state *t;

	/* statistics */
	m_stats.tex_writes++;

	/* point to the right TMU */
	if (!(m_chipmask & (2 << tmunum)))
		return 0;
	t = &m_tmu[tmunum];

	auto const texlod = t->m_reg.texture_lod();
	if (texlod.tdirect_write())
		fatalerror("Texture direct write!\n");

	/* wait for any outstanding work to finish */
	m_poly->wait("Texture write");

	/* update texture info if dirty */
	if (t->m_regdirty)
		t->recompute_texture_params();

	/* swizzle the data */
	if (texlod.tdata_swizzle())
		data = swapendian_int32(data);
	if (texlod.tdata_swap())
		data = (data >> 16) | (data << 16);

	/* 8-bit texture case */
	auto const texmode = t->m_reg.texture_mode();
	if (texmode.format() < 8)
	{
		int lod, tt, ts;
		u32 tbaseaddr;
		u8 *dest;

		/* extract info */
		if (m_type <= TYPE_VOODOO_2)
		{
			lod = (offset >> 15) & 0x0f;
			tt = (offset >> 7) & 0xff;

			/* old code has a bit about how this is broken in gauntleg unless we always look at TMU0 */
			if (m_tmu[0].m_reg.texture_mode().seq_8_downld())
				ts = (offset << 2) & 0xfc;
			else
				ts = (offset << 1) & 0xfc;

			/* validate parameters */
			if (lod > 8)
				return 0;

			/* compute the base address */
			tbaseaddr = t->m_lodoffset[lod];
			tbaseaddr += tt * ((t->m_wmask >> lod) + 1) + ts;

			if (LOG_TEXTURE_RAM) logerror("Texture 8-bit w: lod=%d s=%d t=%d data=%08X\n", lod, ts, tt, data);
		}
		else
		{
			tbaseaddr = t->m_lodoffset[0] + offset*4;

			if (LOG_TEXTURE_RAM) logerror("Texture 8-bit w: offset=%X data=%08X\n", offset*4, data);
		}

		/* write the four bytes in little-endian order */
		dest = t->m_ram;
		tbaseaddr &= t->m_mask;
		dest[BYTE4_XOR_LE(tbaseaddr + 0)] = (data >> 0) & 0xff;
		dest[BYTE4_XOR_LE(tbaseaddr + 1)] = (data >> 8) & 0xff;
		dest[BYTE4_XOR_LE(tbaseaddr + 2)] = (data >> 16) & 0xff;
		dest[BYTE4_XOR_LE(tbaseaddr + 3)] = (data >> 24) & 0xff;
	}

	/* 16-bit texture case */
	else
	{
		int lod, tt, ts;
		u32 tbaseaddr;
		u16 *dest;

		/* extract info */
		if (m_type <= TYPE_VOODOO_2)
		{
			lod = (offset >> 15) & 0x0f;
			tt = (offset >> 7) & 0xff;
			ts = (offset << 1) & 0xfe;

			/* validate parameters */
			if (lod > 8)
				return 0;

			/* compute the base address */
			tbaseaddr = t->m_lodoffset[lod];
			tbaseaddr += 2 * (tt * ((t->m_wmask >> lod) + 1) + ts);

			if (LOG_TEXTURE_RAM) logerror("Texture 16-bit w: lod=%d s=%d t=%d data=%08X\n", lod, ts, tt, data);
		}
		else
		{
			tbaseaddr = t->m_lodoffset[0] + offset*4;

			if (LOG_TEXTURE_RAM) logerror("Texture 16-bit w: offset=%X data=%08X\n", offset*4, data);
		}

		/* write the two words in little-endian order */
		dest = (u16 *)t->m_ram;
		tbaseaddr &= t->m_mask;
		tbaseaddr >>= 1;
		dest[BYTE_XOR_LE(tbaseaddr + 0)] = (data >> 0) & 0xffff;
		dest[BYTE_XOR_LE(tbaseaddr + 1)] = (data >> 16) & 0xffff;
	}

	return 0;
}



/*************************************
 *
 *  Flush data from the FIFOs
 *
 *************************************/

void voodoo_device::flush_fifos(attotime current_time)
{
	static u8 in_flush;

	/* check for recursive calls */
	if (in_flush)
		return;
	in_flush = true;

	if (!m_pci.op_pending) fatalerror("flush_fifos called with no pending operation\n");

	if (LOG_FIFO_VERBOSE)
	{
		logerror("VOODOO.FIFO:flush_fifos start -- pending=%d.%018d cur=%d.%018d\n",
				m_pci.op_end_time.seconds(), m_pci.op_end_time.attoseconds(),
				current_time.seconds(), current_time.attoseconds());
	}

	/* loop while we still have cycles to burn */
	while (m_pci.op_end_time <= current_time)
	{
		s32 cycles;

		/* loop over 0-cycle stuff; this constitutes the bulk of our writes */
		do
		{
			/* we might be in CMDFIFO mode */
			if (m_fbi.cmdfifo[0].enable)
			{
				/* if we don't have anything to execute, we're done for now */
				cycles = cmdfifo_execute_if_ready(m_fbi.cmdfifo[0]);
				if (cycles == -1)
				{
					m_pci.op_pending = false;
					in_flush = false;
					if (LOG_FIFO_VERBOSE) logerror("VOODOO.FIFO:flush_fifos end -- CMDFIFO empty\n");
					return;
				}
			}
			else if (m_fbi.cmdfifo[1].enable)
			{
				/* if we don't have anything to execute, we're done for now */
				cycles = cmdfifo_execute_if_ready(m_fbi.cmdfifo[1]);
				if (cycles == -1)
				{
					m_pci.op_pending = false;
					in_flush = false;
					if (LOG_FIFO_VERBOSE) logerror("VOODOO.FIFO:flush_fifos end -- CMDFIFO empty\n");
					return;
				}
			}

			/* else we are in standard PCI/memory FIFO mode */
			else
			{
				voodoo::fifo_state *fifo;

				/* choose which FIFO to read from */
				if (!m_fbi.fifo.empty())
					fifo = &m_fbi.fifo;
				else if (!m_pci.fifo.empty())
					fifo = &m_pci.fifo;
				else
				{
					m_pci.op_pending = false;
					in_flush = false;
					if (LOG_FIFO_VERBOSE) logerror("VOODOO.FIFO:flush_fifos end -- FIFOs empty\n");
					return;
				}

				/* extract address and data */
				u32 address = fifo->remove();
				u32 data = fifo->remove();

				/* target the appropriate location */
				if ((address & (0xc00000/4)) == 0)
					cycles = register_w(address, data);
				else if (address & (0x800000/4))
					cycles = texture_w(address, data);
				else
				{
					u32 mem_mask = 0xffffffff;

					/* compute mem_mask */
					if (address & 0x80000000)
						mem_mask &= 0x0000ffff;
					if (address & 0x40000000)
						mem_mask &= 0xffff0000;
					address &= 0xffffff;

					cycles = lfb_w(address, data, mem_mask);
				}
			}
		}
		while (cycles == 0);

		/* account for those cycles */
		m_pci.op_end_time += attotime(0, (attoseconds_t)cycles * m_attoseconds_per_cycle);

		if (LOG_FIFO_VERBOSE)
		{
			logerror("VOODOO.FIFO:update -- pending=%d.%018d cur=%d.%018d\n",
					m_pci.op_end_time.seconds(), m_pci.op_end_time.attoseconds(),
					current_time.seconds(), current_time.attoseconds());
		}
	}

	if (LOG_FIFO_VERBOSE)
	{
		logerror("VOODOO.FIFO:flush_fifos end -- pending command complete at %d.%018d\n",
				m_pci.op_end_time.seconds(), m_pci.op_end_time.attoseconds());
	}

	in_flush = false;
}



/*************************************
 *
 *  Handle a write to the Voodoo
 *  memory space
 *
 *************************************/

void voodoo_device::voodoo_w(offs_t offset, u32 data, u32 mem_mask)
{
	int stall = false;

	g_profiler.start(PROFILER_USER1);

	/* should not be getting accesses while stalled */
	if (m_pci.stall_state != NOT_STALLED)
		logerror("voodoo_w while stalled!\n");

	/* if we have something pending, flush the FIFOs up to the current time */
	if (m_pci.op_pending)
		flush_fifos(machine().time());

	/* special handling for registers */
	if ((offset & 0xc00000/4) == 0)
	{
		u8 access;

		/* some special stuff for Voodoo 2 */
		if (m_type >= TYPE_VOODOO_2)
		{
			/* we might be in CMDFIFO mode */
			if (m_reg.fbi_init7().cmdfifo_enable())
			{
				/* if bit 21 is set, we're writing to the FIFO */
				if (offset & 0x200000/4)
				{
					/* check for byte swizzling (bit 18) */
					if (offset & 0x40000/4)
						data = swapendian_int32(data);
					cmdfifo_w(&m_fbi.cmdfifo[0], offset & 0xffff, data);
					g_profiler.stop();
					return;
				}

				/* we're a register access; but only certain ones are allowed */
				access = m_regaccess[offset & 0xff];
				if (!(access & REGISTER_WRITETHRU))
				{
					/* track swap buffers regardless */
					if ((offset & 0xff) == voodoo_regs::reg_swapbufferCMD)
						m_fbi.swaps_pending++;

					logerror("Ignoring write to %s in CMDFIFO mode\n", m_regnames[offset & 0xff]);
					g_profiler.stop();
					return;
				}
			}

			/* if not, we might be byte swizzled (bit 20) */
			else if (offset & 0x100000/4)
				data = swapendian_int32(data);
		}

		/* check the access behavior; note that the table works even if the */
		/* alternate mapping is used */
		access = m_regaccess[offset & 0xff];

		/* ignore if writes aren't allowed */
		if (!(access & REGISTER_WRITE))
		{
			g_profiler.stop();
			return;
		}

		// if this is non-FIFO command, execute immediately
		if (!(access & REGISTER_FIFO)) {
			register_w(offset, data);
			g_profiler.stop();
			return;
		}

		/* track swap buffers */
		if ((offset & 0xff) == voodoo_regs::reg_swapbufferCMD)
			m_fbi.swaps_pending++;
	}

	/* if we don't have anything pending, or if FIFOs are disabled, just execute */
	if (!m_pci.op_pending || !m_pci.init_enable.enable_pci_fifo())
	{
		int cycles;

		/* target the appropriate location */
		if ((offset & (0xc00000/4)) == 0)
			cycles = register_w(offset, data);
		else if (offset & (0x800000/4))
			cycles = texture_w(offset, data);
		else
			cycles = lfb_w(offset, data, mem_mask);

		/* if we ended up with cycles, mark the operation pending */
		if (cycles)
		{
			m_pci.op_pending = true;
			m_pci.op_end_time = machine().time() + attotime(0, (attoseconds_t)cycles * m_attoseconds_per_cycle);

			if (LOG_FIFO_VERBOSE)
			{
				logerror("VOODOO.FIFO:direct write start at %d.%018d end at %d.%018d\n",
						machine().time().seconds(), machine().time().attoseconds(),
						m_pci.op_end_time.seconds(), m_pci.op_end_time.attoseconds());
			}
		}
		g_profiler.stop();
		return;
	}

	/* modify the offset based on the mem_mask */
	if (mem_mask != 0xffffffff)
	{
		if (!ACCESSING_BITS_16_31)
			offset |= 0x80000000;
		if (!ACCESSING_BITS_0_15)
			offset |= 0x40000000;
	}

	/* if there's room in the PCI FIFO, add there */
	if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:voodoo_w adding to PCI FIFO @ %08X=%08X\n", this, offset, data);
	if (!m_pci.fifo.full())
	{
		m_pci.fifo.add(offset);
		m_pci.fifo.add(data);
	}
	else
		fatalerror("PCI FIFO full\n");

	/* handle flushing to the memory FIFO */
	if (m_reg.fbi_init0().enable_memory_fifo() && m_pci.fifo.space() <= 2 * m_reg.fbi_init4().memory_fifo_lwm())
	{
		u8 valid[4];

		/* determine which types of data can go to the memory FIFO */
		valid[0] = true;
		valid[1] = m_reg.fbi_init0().lfb_to_memory_fifo();
		valid[2] = valid[3] = m_reg.fbi_init0().texmem_to_memory_fifo();

		/* flush everything we can */
		if (LOG_FIFO_VERBOSE) logerror("VOODOO.FIFO:voodoo_w moving PCI FIFO to memory FIFO\n");
		while (!m_pci.fifo.empty() && valid[(m_pci.fifo.peek() >> 22) & 3])
		{
			m_fbi.fifo.add(m_pci.fifo.remove());
			m_fbi.fifo.add(m_pci.fifo.remove());
		}

		/* if we're above the HWM as a result, stall */
		if (m_reg.fbi_init0().stall_pcie_for_hwm() && m_fbi.fifo.items() >= 2 * 32 * m_reg.fbi_init0().memory_fifo_hwm())
		{
			if (LOG_FIFO) logerror("VOODOO.FIFO:voodoo_w hit memory FIFO HWM -- stalling\n");
			stall_cpu(STALLED_UNTIL_FIFO_LWM, machine().time());
		}
	}

	/* if we're at the LWM for the PCI FIFO, stall */
	if (m_reg.fbi_init0().stall_pcie_for_hwm() && m_pci.fifo.space() <= 2 * m_reg.fbi_init0().pci_fifo_lwm())
	{
		if (LOG_FIFO) logerror("VOODOO.FIFO:voodoo_w hit PCI FIFO free LWM -- stalling\n");
		stall_cpu(STALLED_UNTIL_FIFO_LWM, machine().time());
	}

	/* if we weren't ready, and this is a non-FIFO access, stall until the FIFOs are clear */
	if (stall)
	{
		if (LOG_FIFO_VERBOSE) logerror("VOODOO.FIFO:voodoo_w wrote non-FIFO register -- stalling until clear\n");
		stall_cpu(STALLED_UNTIL_FIFO_EMPTY, machine().time());
	}

	g_profiler.stop();
}



/*************************************
 *
 *  Handle a register read
 *
 *************************************/

u32 voodoo_device::register_r(offs_t offset)
{
	int regnum = offset & 0xff;
	u32 result;

	/* statistics */
	m_stats.reg_reads++;

	/* first make sure this register is readable */
	if (!(m_regaccess[regnum] & REGISTER_READ))
	{
		logerror("VOODOO.ERROR:Invalid attempt to read %s\n", regnum < 225 ? m_regnames[regnum] : "unknown register");
		return 0xffffffff;
	}

	/* default result is the FBI register value */
	result = m_reg.read(regnum);

	/* some registers are dynamic; compute them */
	switch (regnum)
	{
		case voodoo_regs::reg_vdstatus:

			/* start with a blank slate */
			result = 0;

			/* bits 5:0 are the PCI FIFO free space */
			if (m_pci.fifo.empty())
				result |= 0x3f << 0;
			else
			{
				int temp = m_pci.fifo.space()/2;
				if (temp > 0x3f)
					temp = 0x3f;
				result |= temp << 0;
			}

			/* bit 6 is the vertical retrace */
			result |= m_fbi.vblank << 6;

			/* bit 7 is FBI graphics engine busy */
			if (m_pci.op_pending)
				result |= 1 << 7;

			/* bit 8 is TREX busy */
			if (m_pci.op_pending)
				result |= 1 << 8;

			/* bit 9 is overall busy */
			if (m_pci.op_pending)
				result |= 1 << 9;

			/* Banshee is different starting here */
			if (m_type < TYPE_VOODOO_BANSHEE)
			{
				/* bits 11:10 specifies which buffer is visible */
				result |= m_fbi.frontbuf << 10;

				/* bits 27:12 indicate memory FIFO freespace */
				if (m_reg.fbi_init0().enable_memory_fifo() == 0 || m_fbi.fifo.empty())
					result |= 0xffff << 12;
				else
				{
					int temp = m_fbi.fifo.space()/2;
					if (temp > 0xffff)
						temp = 0xffff;
					result |= temp << 12;
				}
			}
			else
			{
				/* bit 10 is 2D busy */

				/* bit 11 is cmd FIFO 0 busy */
				if (m_fbi.cmdfifo[0].enable && m_fbi.cmdfifo[0].depth > 0)
					result |= 1 << 11;

				/* bit 12 is cmd FIFO 1 busy */
				if (m_fbi.cmdfifo[1].enable && m_fbi.cmdfifo[1].depth > 0)
					result |= 1 << 12;
			}

			/* bits 30:28 are the number of pending swaps */
			if (m_fbi.swaps_pending > 7)
				result |= 7 << 28;
			else
				result |= m_fbi.swaps_pending << 28;

			/* bit 31 is not used */

			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) m_cpu->eat_cycles(1000);
			break;

		/* bit 2 of the initEnable register maps this to dacRead */
		case voodoo_regs::reg_fbiInit2:
			if (m_pci.init_enable.remap_init_to_dac())
				result = m_dac.read_result;
			break;

		/* return the current visible scanline */
		case voodoo_regs::reg_vRetrace:
			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) m_cpu->eat_cycles(10);
			// Return 0 if vblank is active
			if (m_fbi.vblank) {
				result = 0;
			}
			else {
				// Want screen position from vblank off
				result = m_screen->vpos();
			}
			break;

		/* return visible horizontal and vertical positions. Read by the Vegas startup sequence */
		case voodoo_regs::reg_hvRetrace:
			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) m_cpu->eat_cycles(10);
			//result = 0x200 << 16;   /* should be between 0x7b and 0x267 */
			//result |= 0x80;         /* should be between 0x17 and 0x103 */
			// Return 0 if vblank is active
			if (m_fbi.vblank) {
				result = 0;
			}
			else {
				// Want screen position from vblank off
				result = m_screen->vpos();
			}
			// Hpos
			result |= m_screen->hpos() << 16;
			break;

		/* cmdFifo -- Voodoo2 only */
		case voodoo_regs::reg_cmdFifoRdPtr:
			result = m_fbi.cmdfifo[0].rdptr;

			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) m_cpu->eat_cycles(1000);
			break;

		case voodoo_regs::reg_cmdFifoAMin:
			result = m_fbi.cmdfifo[0].amin;
			break;

		case voodoo_regs::reg_cmdFifoAMax:
			result = m_fbi.cmdfifo[0].amax;
			break;

		case voodoo_regs::reg_cmdFifoDepth:
			result = m_fbi.cmdfifo[0].depth;
			break;

		case voodoo_regs::reg_cmdFifoHoles:
			result = m_fbi.cmdfifo[0].holes;
			break;

		/* all counters are 24-bit only */
		case voodoo_regs::reg_fbiPixelsIn:
		case voodoo_regs::reg_fbiChromaFail:
		case voodoo_regs::reg_fbiZfuncFail:
		case voodoo_regs::reg_fbiAfuncFail:
		case voodoo_regs::reg_fbiPixelsOut:
			update_statistics(true);
			[[fallthrough]];
		case voodoo_regs::reg_fbiTrianglesOut:
			result = m_reg.read(regnum) & 0xffffff;
			break;
	}

	if (LOG_REGISTERS)
	{
		int logit = true;

		/* don't log multiple identical status reads from the same address */
		if (regnum == voodoo_regs::reg_vdstatus)
		{
			offs_t pc = m_cpu->pc();
			if (pc == m_last_status_pc && result == m_last_status_value)
				logit = false;
			m_last_status_pc = pc;
			m_last_status_value = result;
		}
		if (regnum == voodoo_regs::reg_cmdFifoRdPtr)
			logit = false;

		if (logit)
			logerror("VOODOO.REG:%s read = %08X\n", m_regnames[regnum], result);
	}

	return result;
}



/*************************************
 *
 *  Handle an LFB read
 *
 *************************************/

u32 voodoo_device::lfb_r(offs_t offset, bool lfb_3d)
{
	u16 *buffer;
	u32 bufmax;
	u32 bufoffs;
	u32 data;
	int x, y, scry, destbuf;

	/* statistics */
	m_stats.lfb_reads++;

	/* compute X,Y */
	offset <<= 1;
	x = offset & ((1 << m_fbi.lfb_stride) - 1);
	y = (offset >> m_fbi.lfb_stride);

	/* select the target buffer */
	auto const lfbmode = m_reg.lfb_mode();
	if (lfb_3d)
	{
		y &= 0x3ff;
		destbuf = (m_type >= TYPE_VOODOO_BANSHEE) ? 1 : lfbmode.read_buffer_select();
		switch (destbuf)
		{
			case 0:         /* front buffer */
				buffer = (u16 *)(m_fbi.ram + m_fbi.rgboffs[m_fbi.frontbuf]);
				bufmax = (m_fbi.mask + 1 - m_fbi.rgboffs[m_fbi.frontbuf]) / 2;
				break;

			case 1:         /* back buffer */
				buffer = (u16 *)(m_fbi.ram + m_fbi.rgboffs[m_fbi.backbuf]);
				bufmax = (m_fbi.mask + 1 - m_fbi.rgboffs[m_fbi.backbuf]) / 2;
				break;

			case 2:         /* aux buffer */
				if (m_fbi.auxoffs == ~0)
					return 0xffffffff;
				buffer = (u16 *)(m_fbi.ram + m_fbi.auxoffs);
				bufmax = (m_fbi.mask + 1 - m_fbi.auxoffs) / 2;
				break;

			default:        /* reserved */
				return 0xffffffff;
		}

		/* determine the screen Y */
		scry = y;
		if (lfbmode.y_origin())
			scry = (m_fbi.yorigin - y);
	} else {
		// Direct lfb access
		buffer = (u16 *)(m_fbi.ram + m_fbi.lfb_base*4);
		bufmax = (m_fbi.mask + 1 - m_fbi.lfb_base*4) / 2;
		scry = y;
	}

	/* advance pointers to the proper row */
	bufoffs = scry * m_fbi.rowpixels + x;
	if (bufoffs >= bufmax) {
		logerror("LFB_R: Buffer offset out of bounds x=%i y=%i lfb_3d=%i offset=%08X bufoffs=%08X\n", x, y, lfb_3d, offset, (u32) bufoffs);
		return 0xffffffff;
	}

	/* wait for any outstanding work to finish */
	m_poly->wait("LFB read");

	/* compute the data */
	data = buffer[bufoffs + 0] | (buffer[bufoffs + 1] << 16);

	/* word swapping */
	if (lfbmode.word_swap_reads())
		data = (data << 16) | (data >> 16);

	/* byte swizzling */
	if (lfbmode.byte_swizzle_reads())
		data = swapendian_int32(data);

	if (LOG_LFB) logerror("VOODOO.LFB:read (%d,%d) = %08X\n", x, y, data);
	return data;
}



/*************************************
 *
 *  Handle a read from the Voodoo
 *  memory space
 *
 *************************************/

u32 voodoo_device::voodoo_r(offs_t offset)
{
	/* if we have something pending, flush the FIFOs up to the current time */
	if (m_pci.op_pending)
		flush_fifos(machine().time());

	/* target the appropriate location */
	if (!(offset & (0xc00000/4)))
		return register_r(offset);
	else if (!(offset & (0x800000/4)))
		return lfb_r(offset, true);

	return 0xffffffff;
}



/*************************************
 *
 *  Handle a read from the Banshee
 *  I/O space
 *
 *************************************/

u32 voodoo_banshee_device::banshee_agp_r(offs_t offset)
{
	u32 result;

	offset &= 0x1ff/4;

	/* switch off the offset */
	switch (offset)
	{
		case cmdRdPtrL0:
			result = m_fbi.cmdfifo[0].rdptr;
			break;

		case cmdAMin0:
			result = m_fbi.cmdfifo[0].amin;
			break;

		case cmdAMax0:
			result = m_fbi.cmdfifo[0].amax;
			break;

		case cmdFifoDepth0:
			result = m_fbi.cmdfifo[0].depth;
			break;

		case cmdHoleCnt0:
			result = m_fbi.cmdfifo[0].holes;
			break;

		case cmdRdPtrL1:
			result = m_fbi.cmdfifo[1].rdptr;
			break;

		case cmdAMin1:
			result = m_fbi.cmdfifo[1].amin;
			break;

		case cmdAMax1:
			result = m_fbi.cmdfifo[1].amax;
			break;

		case cmdFifoDepth1:
			result = m_fbi.cmdfifo[1].depth;
			break;

		case cmdHoleCnt1:
			result = m_fbi.cmdfifo[1].holes;
			break;

		default:
			result = m_banshee.agp[offset];
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:banshee_r(AGP:%s)\n", machine().describe_context(), banshee_agp_reg_name[offset]);
	return result;
}


u32 voodoo_banshee_device::banshee_r(offs_t offset, u32 mem_mask)
{
	u32 result = 0xffffffff;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (m_pci.op_pending)
		flush_fifos(machine().time());

	if (offset < 0x80000/4)
		result = banshee_io_r(offset, mem_mask);
	else if (offset < 0x100000/4)
		result = banshee_agp_r(offset);
	else if (offset < 0x200000/4)
		logerror("%s:banshee_r(2D:%X)\n", machine().describe_context(), (offset*4) & 0xfffff);
	else if (offset < 0x600000/4)
		result = register_r(offset & 0x1fffff/4);
	else if (offset < 0x800000/4)
		logerror("%s:banshee_r(TEX0:%X)\n", machine().describe_context(), (offset*4) & 0x1fffff);
	else if (offset < 0xa00000/4)
		logerror("%s:banshee_r(TEX1:%X)\n", machine().describe_context(), (offset*4) & 0x1fffff);
	else if (offset < 0xc00000/4)
		logerror("%s:banshee_r(FLASH Bios ROM:%X)\n", machine().describe_context(), (offset*4) & 0x3fffff);
	else if (offset < 0x1000000/4)
		logerror("%s:banshee_r(YUV:%X)\n", machine().describe_context(), (offset*4) & 0x3fffff);
	else if (offset < 0x2000000/4)
	{
		result = lfb_r(offset & 0xffffff/4, true);
	} else {
			logerror("%s:banshee_r(%X) Access out of bounds\n", machine().describe_context(), offset*4);
	}
	return result;
}


u32 voodoo_banshee_device::banshee_fb_r(offs_t offset)
{
	u32 result = 0xffffffff;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (m_pci.op_pending)
		flush_fifos(machine().time());

	if (offset < m_fbi.lfb_base)
	{
#if LOG_LFB
		logerror("%s:banshee_fb_r(%X)\n", machine().describe_context(), offset*4);
#endif
		if (offset*4 <= m_fbi.mask)
			result = ((u32 *)m_fbi.ram)[offset];
		else
			logerror("%s:banshee_fb_r(%X) Access out of bounds\n", machine().describe_context(), offset*4);
	}
	else {
		if (LOG_LFB)
			logerror("%s:banshee_fb_r(%X) to lfb_r: %08X lfb_base=%08X\n", machine().describe_context(), offset*4, offset - m_fbi.lfb_base, m_fbi.lfb_base);
		result = lfb_r(offset - m_fbi.lfb_base, false);
	}
	return result;
}


u8 voodoo_banshee_device::banshee_vga_r(offs_t offset)
{
	u8 result = 0xff;

	offset &= 0x1f;

	/* switch off the offset */
	switch (offset + 0x3c0)
	{
		/* attribute access */
		case 0x3c0:
			if (m_banshee.vga[0x3c1 & 0x1f] < std::size(m_banshee.att))
				result = m_banshee.att[m_banshee.vga[0x3c1 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_att_r(%X)\n", machine().describe_context(), m_banshee.vga[0x3c1 & 0x1f]);
			break;

		/* Input status 0 */
		case 0x3c2:
			/*
			    bit 7 = Interrupt Status. When its value is ?1?, denotes that an interrupt is pending.
			    bit 6:5 = Feature Connector. These 2 bits are readable bits from the feature connector.
			    bit 4 = Sense. This bit reflects the state of the DAC monitor sense logic.
			    bit 3:0 = Reserved. Read back as 0.
			*/
			result = 0x00;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;

		/* Sequencer access */
		case 0x3c5:
			if (m_banshee.vga[0x3c4 & 0x1f] < std::size(m_banshee.seq))
				result = m_banshee.seq[m_banshee.vga[0x3c4 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_seq_r(%X)\n", machine().describe_context(), m_banshee.vga[0x3c4 & 0x1f]);
			break;

		/* Feature control */
		case 0x3ca:
			result = m_banshee.vga[0x3da & 0x1f];
			m_banshee.attff = 0;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;

		/* Miscellaneous output */
		case 0x3cc:
			result = m_banshee.vga[0x3c2 & 0x1f];
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;

		/* Graphics controller access */
		case 0x3cf:
			if (m_banshee.vga[0x3ce & 0x1f] < std::size(m_banshee.gc))
				result = m_banshee.gc[m_banshee.vga[0x3ce & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_gc_r(%X)\n", machine().describe_context(), m_banshee.vga[0x3ce & 0x1f]);
			break;

		/* CRTC access */
		case 0x3d5:
			if (m_banshee.vga[0x3d4 & 0x1f] < std::size(m_banshee.crtc))
				result = m_banshee.crtc[m_banshee.vga[0x3d4 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_crtc_r(%X)\n", machine().describe_context(), m_banshee.vga[0x3d4 & 0x1f]);
			break;

		/* Input status 1 */
		case 0x3da:
			/*
			    bit 7:6 = Reserved. These bits read back 0.
			    bit 5:4 = Display Status. These 2 bits reflect 2 of the 8 pixel data outputs from the Attribute
			                controller, as determined by the Attribute controller index 0x12 bits 4 and 5.
			    bit 3 = Vertical sync Status. A ?1? indicates vertical retrace is in progress.
			    bit 2:1 = Reserved. These bits read back 0x2.
			    bit 0 = Display Disable. When this bit is 1, either horizontal or vertical display end has occurred,
			                otherwise video data is being displayed.
			*/
			result = 0x04;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;

		default:
			result = m_banshee.vga[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;
	}
	return result;
}


u32 voodoo_banshee_device::banshee_io_r(offs_t offset, u32 mem_mask)
{
	u32 result;

	offset &= 0xff/4;

	/* switch off the offset */
	switch (offset)
	{
		case io_status:
			result = register_r(0);
			break;

		case io_dacData:
			result = m_fbi.clut[m_banshee.io[io_dacAddr] & 0x1ff] = m_banshee.io[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_dac_r(%X)\n", machine().describe_context(), m_banshee.io[io_dacAddr] & 0x1ff);
			break;

		case io_vgab0:  case io_vgab4:  case io_vgab8:  case io_vgabc:
		case io_vgac0:  case io_vgac4:  case io_vgac8:  case io_vgacc:
		case io_vgad0:  case io_vgad4:  case io_vgad8:  case io_vgadc:
			result = 0;
			if (ACCESSING_BITS_0_7)
				result |= banshee_vga_r(offset*4+0) << 0;
			if (ACCESSING_BITS_8_15)
				result |= banshee_vga_r(offset*4+1) << 8;
			if (ACCESSING_BITS_16_23)
				result |= banshee_vga_r(offset*4+2) << 16;
			if (ACCESSING_BITS_24_31)
				result |= banshee_vga_r(offset*4+3) << 24;
			break;

		default:
			result = m_banshee.io[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_r(%s)\n", machine().describe_context(), banshee_io_reg_name[offset]);
			break;
	}

	return result;
}


u32 voodoo_banshee_device::banshee_rom_r(offs_t offset)
{
	logerror("%s:banshee_rom_r(%X)\n", machine().describe_context(), offset*4);
	return 0xffffffff;
}

void voodoo_device::banshee_blit_2d(u32 data)
{
	switch (m_banshee.blt_cmd)
	{
		case 0:         // NOP - wait for idle
		{
			break;
		}

		case 1:         // Screen-to-screen blit
		{
			// TODO
#if LOG_BANSHEE_2D
			logerror("   blit_2d:screen_to_screen: src X %d, src Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			break;
		}

		case 2:         // Screen-to-screen stretch blit
		{
			fatalerror("   blit_2d:screen_to_screen_stretch: src X %d, src Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 3:         // Host-to-screen blit
		{
			u32 addr = m_banshee.blt_dst_base;

			addr += (m_banshee.blt_dst_y * m_banshee.blt_dst_stride) + (m_banshee.blt_dst_x * m_banshee.blt_dst_bpp);

#if LOG_BANSHEE_2D
			logerror("   blit_2d:host_to_screen: %08x -> %08x, %d, %d\n", data, addr, m_banshee.blt_dst_x, m_banshee.blt_dst_y);
#endif

			switch (m_banshee.blt_dst_bpp)
			{
				case 1:
					m_fbi.ram[addr+0] = data & 0xff;
					m_fbi.ram[addr+1] = (data >> 8) & 0xff;
					m_fbi.ram[addr+2] = (data >> 16) & 0xff;
					m_fbi.ram[addr+3] = (data >> 24) & 0xff;
					m_banshee.blt_dst_x += 4;
					break;
				case 2:
					m_fbi.ram[addr+1] = data & 0xff;
					m_fbi.ram[addr+0] = (data >> 8) & 0xff;
					m_fbi.ram[addr+3] = (data >> 16) & 0xff;
					m_fbi.ram[addr+2] = (data >> 24) & 0xff;
					m_banshee.blt_dst_x += 2;
					break;
				case 3:
					m_banshee.blt_dst_x += 1;
					break;
				case 4:
					m_fbi.ram[addr+3] = data & 0xff;
					m_fbi.ram[addr+2] = (data >> 8) & 0xff;
					m_fbi.ram[addr+1] = (data >> 16) & 0xff;
					m_fbi.ram[addr+0] = (data >> 24) & 0xff;
					m_banshee.blt_dst_x += 1;
					break;
			}

			if (m_banshee.blt_dst_x >= m_banshee.blt_dst_width)
			{
				m_banshee.blt_dst_x = 0;
				m_banshee.blt_dst_y++;
			}
			break;
		}

		case 5:         // Rectangle fill
		{
			fatalerror("blit_2d:rectangle_fill: src X %d, src Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 6:         // Line
		{
			fatalerror("blit_2d:line: end X %d, end Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 7:         // Polyline
		{
			fatalerror("blit_2d:polyline: end X %d, end Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 8:         // Polygon fill
		{
			fatalerror("blit_2d:polygon_fill\n");
		}

		default:
		{
			fatalerror("blit_2d: unknown command %d\n", m_banshee.blt_cmd);
		}
	}
}

s32 voodoo_device::banshee_2d_w(offs_t offset, u32 data)
{
	switch (offset)
	{
		case banshee2D_command:
#if LOG_BANSHEE_2D
			logerror("   2D:command: cmd %d, ROP0 %02X\n", data & 0xf, data >> 24);
#endif

			m_banshee.blt_src_x        = m_banshee.blt_regs[banshee2D_srcXY] & 0xfff;
			m_banshee.blt_src_y        = (m_banshee.blt_regs[banshee2D_srcXY] >> 16) & 0xfff;
			m_banshee.blt_src_base     = m_banshee.blt_regs[banshee2D_srcBaseAddr] & 0xffffff;
			m_banshee.blt_src_stride   = m_banshee.blt_regs[banshee2D_srcFormat] & 0x3fff;
			m_banshee.blt_src_width    = m_banshee.blt_regs[banshee2D_srcSize] & 0xfff;
			m_banshee.blt_src_height   = (m_banshee.blt_regs[banshee2D_srcSize] >> 16) & 0xfff;

			switch ((m_banshee.blt_regs[banshee2D_srcFormat] >> 16) & 0xf)
			{
				case 1: m_banshee.blt_src_bpp = 1; break;
				case 3: m_banshee.blt_src_bpp = 2; break;
				case 4: m_banshee.blt_src_bpp = 3; break;
				case 5: m_banshee.blt_src_bpp = 4; break;
				case 8: m_banshee.blt_src_bpp = 2; break;
				case 9: m_banshee.blt_src_bpp = 2; break;
				default: m_banshee.blt_src_bpp = 1; break;
			}

			m_banshee.blt_dst_x        = m_banshee.blt_regs[banshee2D_dstXY] & 0xfff;
			m_banshee.blt_dst_y        = (m_banshee.blt_regs[banshee2D_dstXY] >> 16) & 0xfff;
			m_banshee.blt_dst_base     = m_banshee.blt_regs[banshee2D_dstBaseAddr] & 0xffffff;
			m_banshee.blt_dst_stride   = m_banshee.blt_regs[banshee2D_dstFormat] & 0x3fff;
			m_banshee.blt_dst_width    = m_banshee.blt_regs[banshee2D_dstSize] & 0xfff;
			m_banshee.blt_dst_height   = (m_banshee.blt_regs[banshee2D_dstSize] >> 16) & 0xfff;

			switch ((m_banshee.blt_regs[banshee2D_dstFormat] >> 16) & 0x7)
			{
				case 1: m_banshee.blt_dst_bpp = 1; break;
				case 3: m_banshee.blt_dst_bpp = 2; break;
				case 4: m_banshee.blt_dst_bpp = 3; break;
				case 5: m_banshee.blt_dst_bpp = 4; break;
				default: m_banshee.blt_dst_bpp = 1; break;
			}

			m_banshee.blt_cmd = data & 0xf;
			break;

		case banshee2D_colorBack:
#if LOG_BANSHEE_2D
			logerror("   2D:colorBack: %08X\n", data);
#endif
			m_banshee.blt_regs[banshee2D_colorBack] = data;
			break;

		case banshee2D_colorFore:
#if LOG_BANSHEE_2D
			logerror("   2D:colorFore: %08X\n", data);
#endif
			m_banshee.blt_regs[banshee2D_colorFore] = data;
			break;

		case banshee2D_srcBaseAddr:
#if LOG_BANSHEE_2D
			logerror("   2D:srcBaseAddr: %08X, %s\n", data & 0xffffff, data & 0x80000000 ? "tiled" : "non-tiled");
#endif
			m_banshee.blt_regs[banshee2D_srcBaseAddr] = data;
			break;

		case banshee2D_dstBaseAddr:
#if LOG_BANSHEE_2D
			logerror("   2D:dstBaseAddr: %08X, %s\n", data & 0xffffff, data & 0x80000000 ? "tiled" : "non-tiled");
#endif
			m_banshee.blt_regs[banshee2D_dstBaseAddr] = data;
			break;

		case banshee2D_srcSize:
#if LOG_BANSHEE_2D
			logerror("   2D:srcSize: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			m_banshee.blt_regs[banshee2D_srcSize] = data;
			break;

		case banshee2D_dstSize:
#if LOG_BANSHEE_2D
			logerror("   2D:dstSize: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			m_banshee.blt_regs[banshee2D_dstSize] = data;
			break;

		case banshee2D_srcXY:
#if LOG_BANSHEE_2D
			logerror("   2D:srcXY: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			m_banshee.blt_regs[banshee2D_srcXY] = data;
			break;

		case banshee2D_dstXY:
#if LOG_BANSHEE_2D
			logerror("   2D:dstXY: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			m_banshee.blt_regs[banshee2D_dstXY] = data;
			break;

		case banshee2D_srcFormat:
#if LOG_BANSHEE_2D
			logerror("   2D:srcFormat: str %d, fmt %d, packing %d\n", data & 0x3fff, (data >> 16) & 0xf, (data >> 22) & 0x3);
#endif
			m_banshee.blt_regs[banshee2D_srcFormat] = data;
			break;

		case banshee2D_dstFormat:
#if LOG_BANSHEE_2D
			logerror("   2D:dstFormat: str %d, fmt %d\n", data & 0x3fff, (data >> 16) & 0xf);
#endif
			m_banshee.blt_regs[banshee2D_dstFormat] = data;
			break;

		case banshee2D_clip0Min:
#if LOG_BANSHEE_2D
			logerror("   2D:clip0Min: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			m_banshee.blt_regs[banshee2D_clip0Min] = data;
			break;

		case banshee2D_clip0Max:
#if LOG_BANSHEE_2D
			logerror("   2D:clip0Max: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			m_banshee.blt_regs[banshee2D_clip0Max] = data;
			break;

		case banshee2D_clip1Min:
#if LOG_BANSHEE_2D
			logerror("   2D:clip1Min: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			m_banshee.blt_regs[banshee2D_clip1Min] = data;
			break;

		case banshee2D_clip1Max:
#if LOG_BANSHEE_2D
			logerror("   2D:clip1Max: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
#endif
			m_banshee.blt_regs[banshee2D_clip1Max] = data;
			break;

		case banshee2D_rop:
#if LOG_BANSHEE_2D
			logerror("   2D:rop: %d, %d, %d\n",  data & 0xff, (data >> 8) & 0xff, (data >> 16) & 0xff);
#endif
			m_banshee.blt_regs[banshee2D_rop] = data;
			break;

		default:
			if (offset >= 0x20 && offset < 0x40)
			{
				banshee_blit_2d(data);
			}
			else if (offset >= 0x40 && offset < 0x80)
			{
				// TODO: colorPattern
			}
			break;
	}


	return 1;
}




void voodoo_banshee_device::banshee_agp_w(offs_t offset, u32 data, u32 mem_mask)
{
	offset &= 0x1ff/4;

	/* switch off the offset */
	switch (offset)
	{
		case cmdBaseAddr0:
			COMBINE_DATA(&m_banshee.agp[offset]);
			m_fbi.cmdfifo[0].base = (data & 0xffffff) << 12;
			m_fbi.cmdfifo[0].end = m_fbi.cmdfifo[0].base + (((m_banshee.agp[cmdBaseSize0] & 0xff) + 1) << 12);
			break;

		case cmdBaseSize0:
			COMBINE_DATA(&m_banshee.agp[offset]);
			m_fbi.cmdfifo[0].end = m_fbi.cmdfifo[0].base + (((m_banshee.agp[cmdBaseSize0] & 0xff) + 1) << 12);
			m_fbi.cmdfifo[0].enable = (data >> 8) & 1;
			m_fbi.cmdfifo[0].count_holes = (~data >> 10) & 1;
			break;

		case cmdBump0:
			fatalerror("cmdBump0\n");

		case cmdRdPtrL0:
			m_fbi.cmdfifo[0].rdptr = data;
			break;

		case cmdAMin0:
			m_fbi.cmdfifo[0].amin = data;
			break;

		case cmdAMax0:
			m_fbi.cmdfifo[0].amax = data;
			break;

		case cmdFifoDepth0:
			m_fbi.cmdfifo[0].depth = data;
			break;

		case cmdHoleCnt0:
			m_fbi.cmdfifo[0].holes = data;
			break;

		case cmdBaseAddr1:
			COMBINE_DATA(&m_banshee.agp[offset]);
			m_fbi.cmdfifo[1].base = (data & 0xffffff) << 12;
			m_fbi.cmdfifo[1].end = m_fbi.cmdfifo[1].base + (((m_banshee.agp[cmdBaseSize1] & 0xff) + 1) << 12);
			break;

		case cmdBaseSize1:
			COMBINE_DATA(&m_banshee.agp[offset]);
			m_fbi.cmdfifo[1].end = m_fbi.cmdfifo[1].base + (((m_banshee.agp[cmdBaseSize1] & 0xff) + 1) << 12);
			m_fbi.cmdfifo[1].enable = (data >> 8) & 1;
			m_fbi.cmdfifo[1].count_holes = (~data >> 10) & 1;
			break;

		case cmdBump1:
			fatalerror("cmdBump1\n");

		case cmdRdPtrL1:
			m_fbi.cmdfifo[1].rdptr = data;
			break;

		case cmdAMin1:
			m_fbi.cmdfifo[1].amin = data;
			break;

		case cmdAMax1:
			m_fbi.cmdfifo[1].amax = data;
			break;

		case cmdFifoDepth1:
			m_fbi.cmdfifo[1].depth = data;
			break;

		case cmdHoleCnt1:
			m_fbi.cmdfifo[1].holes = data;
			break;

		default:
			COMBINE_DATA(&m_banshee.agp[offset]);
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:banshee_w(AGP:%s) = %08X & %08X\n", machine().describe_context(), banshee_agp_reg_name[offset], data, mem_mask);
}


void voodoo_banshee_device::banshee_w(offs_t offset, u32 data, u32 mem_mask)
{
	/* if we have something pending, flush the FIFOs up to the current time */
	if (m_pci.op_pending)
		flush_fifos(machine().time());

	if (offset < 0x80000/4)
		banshee_io_w(offset, data, mem_mask);
	else if (offset < 0x100000/4)
		banshee_agp_w(offset, data, mem_mask);
	else if (offset < 0x200000/4)
		logerror("%s:banshee_w(2D:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0xfffff, data, mem_mask);
	else if (offset < 0x600000/4)
		register_w(offset & 0x1fffff/4, data);
	else if (offset < 0x800000/4)
		logerror("%s:banshee_w(TEX0:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0x1fffff, data, mem_mask);
	else if (offset < 0xa00000/4)
		logerror("%s:banshee_w(TEX1:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0x1fffff, data, mem_mask);
	else if (offset < 0xc00000/4)
		logerror("%s:banshee_r(FLASH Bios ROM:%X)\n", machine().describe_context(), (offset*4) & 0x3fffff);
	else if (offset < 0x1000000/4)
		logerror("%s:banshee_w(YUV:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0x3fffff, data, mem_mask);
	else if (offset < 0x2000000/4)
	{
		lfb_w(offset & 0xffffff/4, data, mem_mask);
	} else {
		logerror("%s:banshee_w Address out of range %08X = %08X & %08X\n", machine().describe_context(), (offset*4), data, mem_mask);
	}
}


void voodoo_banshee_device::banshee_fb_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 addr = offset*4;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (m_pci.op_pending)
		flush_fifos(machine().time());

	if (offset < m_fbi.lfb_base)
	{
		if (m_fbi.cmdfifo[0].enable && addr >= m_fbi.cmdfifo[0].base && addr < m_fbi.cmdfifo[0].end)
			cmdfifo_w(&m_fbi.cmdfifo[0], (addr - m_fbi.cmdfifo[0].base) / 4, data);
		else if (m_fbi.cmdfifo[1].enable && addr >= m_fbi.cmdfifo[1].base && addr < m_fbi.cmdfifo[1].end)
			cmdfifo_w(&m_fbi.cmdfifo[1], (addr - m_fbi.cmdfifo[1].base) / 4, data);
		else
		{
			if (offset*4 <= m_fbi.mask)
				COMBINE_DATA(&((u32 *)m_fbi.ram)[offset]);
			else
				logerror("%s:banshee_fb_w Out of bounds (%X) = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
#if LOG_LFB
			logerror("%s:banshee_fb_w(%X) = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
#endif
		}
	}
	else
		lfb_direct_w(offset - m_fbi.lfb_base, data, mem_mask);
}


void voodoo_banshee_device::banshee_vga_w(offs_t offset, u8 data)
{
	offset &= 0x1f;

	/* switch off the offset */
	switch (offset + 0x3c0)
	{
		/* attribute access */
		case 0x3c0:
		case 0x3c1:
			if (m_banshee.attff == 0)
			{
				m_banshee.vga[0x3c1 & 0x1f] = data;
				if (LOG_REGISTERS)
					logerror("%s:banshee_vga_w(%X) = %02X\n", machine().describe_context(), 0x3c0+offset, data);
			}
			else
			{
				if (m_banshee.vga[0x3c1 & 0x1f] < std::size(m_banshee.att))
					m_banshee.att[m_banshee.vga[0x3c1 & 0x1f]] = data;
				if (LOG_REGISTERS)
					logerror("%s:banshee_att_w(%X) = %02X\n", machine().describe_context(), m_banshee.vga[0x3c1 & 0x1f], data);
			}
			m_banshee.attff ^= 1;
			break;

		/* Sequencer access */
		case 0x3c5:
			if (m_banshee.vga[0x3c4 & 0x1f] < std::size(m_banshee.seq))
				m_banshee.seq[m_banshee.vga[0x3c4 & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_seq_w(%X) = %02X\n", machine().describe_context(), m_banshee.vga[0x3c4 & 0x1f], data);
			break;

		/* Graphics controller access */
		case 0x3cf:
			if (m_banshee.vga[0x3ce & 0x1f] < std::size(m_banshee.gc))
				m_banshee.gc[m_banshee.vga[0x3ce & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_gc_w(%X) = %02X\n", machine().describe_context(), m_banshee.vga[0x3ce & 0x1f], data);
			break;

		/* CRTC access */
		case 0x3d5:
			if (m_banshee.vga[0x3d4 & 0x1f] < std::size(m_banshee.crtc))
				m_banshee.crtc[m_banshee.vga[0x3d4 & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_crtc_w(%X) = %02X\n", machine().describe_context(), m_banshee.vga[0x3d4 & 0x1f], data);
			break;

		default:
			m_banshee.vga[offset] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_w(%X) = %02X\n", machine().describe_context(), 0x3c0+offset, data);
			break;
	}
}


void voodoo_banshee_device::banshee_io_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 old;

	offset &= 0xff/4;
	old = m_banshee.io[offset];

	/* switch off the offset */
	switch (offset)
	{
		case io_vidProcCfg:
			COMBINE_DATA(&m_banshee.io[offset]);
			if ((m_banshee.io[offset] ^ old) & 0x2800)
				m_fbi.clut_dirty = true;
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_dacData:
			COMBINE_DATA(&m_banshee.io[offset]);
			if (m_banshee.io[offset] != m_fbi.clut[m_banshee.io[io_dacAddr] & 0x1ff])
			{
				m_fbi.clut[m_banshee.io[io_dacAddr] & 0x1ff] = m_banshee.io[offset];
				m_fbi.clut_dirty = true;
			}
			if (LOG_REGISTERS)
				logerror("%s:banshee_dac_w(%X) = %08X & %08X\n", machine().describe_context(), m_banshee.io[io_dacAddr] & 0x1ff, data, mem_mask);
			break;

		case io_miscInit0:
			COMBINE_DATA(&m_banshee.io[offset]);
			m_fbi.yorigin = (data >> 18) & 0xfff;
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_vidScreenSize:
			if (data & 0xfff)
				m_fbi.width = data & 0xfff;
			if (data & 0xfff000)
				m_fbi.height = (data >> 12) & 0xfff;
			[[fallthrough]];
		case io_vidOverlayDudx:
		case io_vidOverlayDvdy:
		{
			COMBINE_DATA(&m_banshee.io[offset]);

			// Get horizontal total and vertical total from CRTC registers
			int htotal = (m_banshee.crtc[0] + 5) * 8;
			int vtotal = m_banshee.crtc[6];
			vtotal |= ((m_banshee.crtc[7] >> 0) & 0x1) << 8;
			vtotal |= ((m_banshee.crtc[7] >> 5) & 0x1) << 9;
			vtotal += 2;

			int vstart = m_banshee.crtc[0x10];
			vstart |= ((m_banshee.crtc[7] >> 2) & 0x1) << 8;
			vstart |= ((m_banshee.crtc[7] >> 7) & 0x1) << 9;

			int vstop = m_banshee.crtc[0x11] & 0xf;
			// Compare to see if vstop is before or after low 4 bits of vstart
			if (vstop < (vstart & 0xf))
				vstop |= (vstart + 0x10) & ~0xf;
			else
				vstop |= vstart & ~0xf;

			// Get pll k, m and n from pllCtrl0
			const u32 k = (m_banshee.io[io_pllCtrl0] >> 0) & 0x3;
			const u32 m = (m_banshee.io[io_pllCtrl0] >> 2) & 0x3f;
			const u32 n = (m_banshee.io[io_pllCtrl0] >> 8) & 0xff;
			const double video_clock = (XTAL(14'318'181) * (n + 2) / ((m + 2) << k)).dvalue();
			const double frame_period = vtotal * htotal / video_clock;
			//osd_printf_info("k: %d m: %d n: %d clock: %f period: %f rate: %.2f\n", k, m, n, video_clock, frame_period, 1.0 / frame_period);

			int width = m_fbi.width;
			int height = m_fbi.height;
			//m_fbi.xoffs = hbp;
			//m_fbi.yoffs = vbp;

			if (m_banshee.io[io_vidOverlayDudx] != 0)
				width = (m_fbi.width * m_banshee.io[io_vidOverlayDudx]) / 1048576;
			if (m_banshee.io[io_vidOverlayDvdy] != 0)
				height = (m_fbi.height * m_banshee.io[io_vidOverlayDvdy]) / 1048576;
			if (LOG_REGISTERS)
				logerror("configure screen: htotal: %d vtotal: %d vstart: %d vstop: %d width: %d height: %d refresh: %f\n",
					htotal, vtotal, vstart, vstop, width, height, 1.0 / frame_period);
			if (htotal > 0 && vtotal > 0) {
				rectangle visarea(0, width - 1, 0, height - 1);
				m_screen->configure(htotal, vtotal, visarea, DOUBLE_TO_ATTOSECONDS(frame_period));

				// Set the vsync start and stop
				m_fbi.vsyncstart = vstart;
				m_fbi.vsyncstop = vstop;
				adjust_vblank_timer();
			}
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;
		}

		case io_lfbMemoryConfig:
			m_fbi.lfb_base = (data & 0x1fff) << (12-2);
			m_fbi.lfb_stride = ((data >> 13) & 7) + 9;
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_vgab0:  case io_vgab4:  case io_vgab8:  case io_vgabc:
		case io_vgac0:  case io_vgac4:  case io_vgac8:  case io_vgacc:
		case io_vgad0:  case io_vgad4:  case io_vgad8:  case io_vgadc:
			if (ACCESSING_BITS_0_7)
				banshee_vga_w(offset*4+0, data >> 0);
			if (ACCESSING_BITS_8_15)
				banshee_vga_w(offset*4+1, data >> 8);
			if (ACCESSING_BITS_16_23)
				banshee_vga_w(offset*4+2, data >> 16);
			if (ACCESSING_BITS_24_31)
				banshee_vga_w(offset*4+3, data >> 24);
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;

		default:
			COMBINE_DATA(&m_banshee.io[offset]);
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), banshee_io_reg_name[offset], data, mem_mask);
			break;
	}
}



/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

void voodoo_device::device_resolve_objects()
{
	if (!m_screen)
		m_screen = m_screen_finder;
	else if (m_screen_finder)
		throw emu_fatalerror("%s: screen set by both configuration and direct reference (%s and %s)\n", tag(), m_screen_finder->tag(), m_screen->tag());
	else if (m_screen_finder.finder_tag() != finder_base::DUMMY_TAG)
		throw emu_fatalerror("%s: configured screen %s not found\n", tag(), m_screen_finder.finder_tag());

	if (!m_cpu)
		m_cpu = m_cpu_finder;
	else if (m_cpu_finder)
		throw emu_fatalerror("%s: CPU set by both configuration and direct reference (%s and %s)\n", tag(), m_cpu_finder->tag(), m_cpu->tag());
	else if (m_cpu_finder.finder_tag() != finder_base::DUMMY_TAG)
		throw emu_fatalerror("%s: configured CPU %s not found\n", tag(), m_cpu_finder.finder_tag());
}

void voodoo_device::device_start()
{
	if (!m_screen || !m_cpu)
		throw device_missing_dependencies();

	m_poly = std::make_unique<voodoo_renderer>(machine());

	/* validate configuration */
	assert(m_fbmem > 0);

	/* copy config data */
	m_freq = clock();
	m_vblank.resolve();
	m_stall.resolve();
	m_pciint.resolve();

	/* create a multiprocessor work queue */
	m_thread_stats = std::make_unique<thread_stats_block[]>(WORK_MAX_THREADS);

	voodoo::dither_helper::init_static();

	m_tmu_config = 0x11;   // revision 1

	/* configure type-specific values */
	switch (m_type)
	{
		case TYPE_VOODOO_1:
			m_regaccess = voodoo_register_access;
			m_regnames = voodoo_reg_name;
			m_alt_regmap = 0;
			m_fbi.lfb_stride = 10;
			break;

		case TYPE_VOODOO_2:
			m_regaccess = voodoo2_register_access;
			m_regnames = voodoo_reg_name;
			m_alt_regmap = 0;
			m_fbi.lfb_stride = 10;
			m_tmu_config |= 0x800;
			break;

		case TYPE_VOODOO_BANSHEE:
			m_regaccess = banshee_register_access;
			m_regnames = banshee_reg_name;
			m_alt_regmap = 1;
			m_fbi.lfb_stride = 11;
			break;

		case TYPE_VOODOO_3:
			m_regaccess = banshee_register_access;
			m_regnames = banshee_reg_name;
			m_alt_regmap = 1;
			m_fbi.lfb_stride = 11;
			break;

		default:
			fatalerror("Unsupported voodoo card in voodoo_start!\n");
	}

	/* set the type, and initialize the chip mask */
	m_index = 0;
	for (device_t &scan : device_enumerator(machine().root_device()))
		if (scan.type() == this->type())
		{
			if (&scan == this)
				break;
			m_index++;
		}

	if (m_tmumem1 != 0)
		m_tmu_config |= 0xc0;  // two TMUs

	m_chipmask = 0x01;
	m_attoseconds_per_cycle = ATTOSECONDS_PER_SECOND / m_freq;
	m_trigger = 51324 + m_index;

	/* build the rasterizer table */
	std::fill(std::begin(m_raster_hash), std::end(m_raster_hash), nullptr);
	for (const static_raster_info *info = predef_raster_table; info->params.m_fbzcp != 0xffffffff; info++)
		add_rasterizer(info->params, info->callback_mfp, false);
	for (int index = 0; index < std::size(generic_raster_table); index++)
		m_generic_rasterizer[index] = add_rasterizer(generic_raster_table[index].params, generic_raster_table[index].callback_mfp, true);

	/* set up the PCI FIFO */
	m_pci.fifo.configure(m_pci.fifo_mem, 64*2);
	m_pci.stall_state = NOT_STALLED;
	m_pci.continue_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(voodoo_device::stall_cpu_callback), this), nullptr);

	/* allocate memory */
	void *fbmem, *tmumem[2];
	u32 tmumem0 = m_tmumem0;
	u32 tmumem1 = m_tmumem1;
	if (m_type <= TYPE_VOODOO_2)
	{
		/* separate FB/TMU memory */
		fbmem = (m_fbmem_alloc = std::make_unique<u8[]>(m_fbmem << 20)).get();
		tmumem[0] = (m_tmumem_alloc[0] = std::make_unique<u8[]>(m_tmumem0 << 20)).get();
		tmumem[1] = (m_tmumem1 != 0) ? (m_tmumem_alloc[1] = std::make_unique<u8[]>(m_tmumem1 << 20)).get() : nullptr;
	}
	else
	{
		/* shared memory */
		tmumem[0] = tmumem[1] = fbmem = (m_fbmem_alloc = std::make_unique<u8[]>(m_fbmem << 20)).get();
		tmumem0 = m_fbmem;
		if (m_type == TYPE_VOODOO_3)
			tmumem1 = m_fbmem;
	}

	/* set up frame buffer */
	init_fbi(&m_fbi, fbmem, m_fbmem << 20);

	/* build shared TMU tables */
	m_tmushare.init();
	// Point the rgb565 table to the frame buffer table
	m_tmushare.rgb565 = m_fbi.rgb565;

	/* set up the TMUs */
	m_tmu[0].init(m_type, m_tmushare, tmumem[0], tmumem0 << 20);
	m_chipmask |= 0x02;
	if (tmumem1 != 0)
	{
		m_tmu[1].init(m_type, m_tmushare, tmumem[1], tmumem1 << 20);
		m_chipmask |= 0x04;
		m_tmu_config |= 0x40;
	}

	/* initialize some registers */
	m_pci.init_enable = 0;
	m_reg.write(voodoo_regs::reg_fbiInit0, (1 << 4) | (0x10 << 6));
	m_reg.write(voodoo_regs::reg_fbiInit1, (1 << 1) | (1 << 8) | (1 << 12) | (2 << 20));
	m_reg.write(voodoo_regs::reg_fbiInit2, (1 << 6) | (0x100 << 23));
	m_reg.write(voodoo_regs::reg_fbiInit3, (2 << 13) | (0xf << 17));
	m_reg.write(voodoo_regs::reg_fbiInit4, (1 << 0));

	/* initialize banshee registers */
	memset(m_banshee.io, 0, sizeof(m_banshee.io));
	m_banshee.io[io_pciInit0] = 0x01800040;
	m_banshee.io[io_sipMonitor] = 0x40000000;
	m_banshee.io[io_lfbMemoryConfig] = 0x000a2200;
	m_banshee.io[io_dramInit0] = 0x00579d29;
	if (m_fbmem == 16)
		m_banshee.io[io_dramInit0] |= 0x0c000000;      // Midway Vegas (denver) expects 2 banks of 16MBit SGRAMs
	else
		m_banshee.io[io_dramInit0] |= 0x08000000;      // Konami Viper expects 16MBit SGRAMs
	m_banshee.io[io_dramInit1] = 0x00f02200;
	m_banshee.io[io_tmuGbeInit] = 0x00000bfb;

	/* do a soft reset to reset everything else */
	soft_reset();

	/* register for save states */
	init_save_state();
}



/***************************************************************************
    COMMAND HANDLERS
***************************************************************************/

/*-------------------------------------------------
    fastfill - execute the 'fastfill'
    command
-------------------------------------------------*/

s32 voodoo_device::fastfill()
{
	/* if we're not clearing either, take no time */
	auto const fbzmode = m_reg.fbz_mode();
	if (!fbzmode.rgb_buffer_mask() && !fbzmode.aux_buffer_mask())
		return 0;

	/* are we clearing the RGB buffer? */
	u16 dithermatrix[16];
	u16 *drawbuf = nullptr;
	if (fbzmode.rgb_buffer_mask())
	{
		/* determine the draw buffer */
		int destbuf = (m_type >= TYPE_VOODOO_BANSHEE) ? 1 : fbzmode.draw_buffer();
		switch (destbuf)
		{
			case 0:     /* front buffer */
				drawbuf = (u16 *)(m_fbi.ram + m_fbi.rgboffs[m_fbi.frontbuf]);
				break;

			case 1:     /* back buffer */
				drawbuf = (u16 *)(m_fbi.ram + m_fbi.rgboffs[m_fbi.backbuf]);
				break;

			default:    /* reserved */
				break;
		}

		/* determine the dither pattern */
		for (int y = 0; y < 4; y++)
		{
			voodoo::dither_helper dither(y, fbzmode);
			for (int x = 0; x < 4; x++)
				dithermatrix[y * 4 + x] = dither.pixel(x, m_reg.color1().argb());
		}
	}

	/* fill in a block of extents */
	voodoo_renderer::extent_t extents[64];
	extents[0].startx = m_reg.clip_left();
	extents[0].stopx = m_reg.clip_right();
	std::fill(std::begin(extents) + 1, std::end(extents), extents[0]);

	/* iterate over blocks of extents */
	voodoo_renderer::render_delegate fastfill_cb(&voodoo_device::raster_fastfill, this);
	int ey = m_reg.clip_bottom();
	u32 pixels = 0;
	for (int y = m_reg.clip_top(); y < ey; y += std::size(extents))
	{
		poly_extra_data &extra = m_poly->object_data_alloc();
		int count = std::min(ey - y, int(std::size(extents)));

		extra.destbase = drawbuf;
		memcpy(extra.u.dither, dithermatrix, sizeof(extra.u.dither));

		pixels += m_poly->render_triangle_custom(global_cliprect, fastfill_cb, y, count, extents);
	}

	/* 2 pixels per clock */
	return pixels / 2;
}


/*-------------------------------------------------
    swapbuffer - execute the 'swapbuffer'
    command
-------------------------------------------------*/

s32 voodoo_device::swapbuffer(u32 data)
{
	/* set the don't swap value for Voodoo 2 */
	m_fbi.vblank_swap_pending = true;
	m_fbi.vblank_swap = (data >> 1) & 0xff;
	m_fbi.vblank_dont_swap = (data >> 9) & 1;

	/* if we're not syncing to the retrace, process the command immediately */
	if (!(data & 1))
	{
		swap_buffers();
		return 0;
	}

	/* determine how many cycles to wait; we deliberately overshoot here because */
	/* the final count gets updated on the VBLANK */
	return (m_fbi.vblank_swap + 1) * m_freq / 10;
}


/*-------------------------------------------------
    triangle - execute the 'triangle'
    command
-------------------------------------------------*/

s32 voodoo_device::triangle()
{
	int texcount;
	u16 *drawbuf;
	int destbuf;
	int pixels;

	g_profiler.start(PROFILER_USER2);

	/* determine the number of TMUs involved */
	texcount = 0;
	auto const fbzcp = m_reg.fbz_colorpath();
	auto const fbzmode = m_reg.fbz_mode();
	if (m_reg.fbi_init3().disable_tmus() == 0 && fbzcp.texture_enable())
	{
		texcount = 1;
		if (m_chipmask & 0x04)
			texcount = 2;
	}

	/* perform subpixel adjustments */
	if (fbzcp.cca_subpixel_adjust())
	{
		s32 dx = 8 - (m_fbi.ax & 15);
		s32 dy = 8 - (m_fbi.ay & 15);

		/* adjust iterated R,G,B,A and W/Z */
		m_fbi.startr += (dy * m_fbi.drdy + dx * m_fbi.drdx) >> 4;
		m_fbi.startg += (dy * m_fbi.dgdy + dx * m_fbi.dgdx) >> 4;
		m_fbi.startb += (dy * m_fbi.dbdy + dx * m_fbi.dbdx) >> 4;
		m_fbi.starta += (dy * m_fbi.dady + dx * m_fbi.dadx) >> 4;
		m_fbi.startw += (dy * m_fbi.dwdy + dx * m_fbi.dwdx) >> 4;
		m_fbi.startz += mul_32x32_shift(dy, m_fbi.dzdy, 4) + mul_32x32_shift(dx, m_fbi.dzdx, 4);

		/* adjust iterated W/S/T for TMU 0 */
		if (texcount >= 1)
		{
			m_tmu[0].m_startw += (dy * m_tmu[0].m_dwdy + dx * m_tmu[0].m_dwdx) >> 4;
			m_tmu[0].m_starts += (dy * m_tmu[0].m_dsdy + dx * m_tmu[0].m_dsdx) >> 4;
			m_tmu[0].m_startt += (dy * m_tmu[0].m_dtdy + dx * m_tmu[0].m_dtdx) >> 4;

			/* adjust iterated W/S/T for TMU 1 */
			if (texcount >= 2)
			{
				m_tmu[1].m_startw += (dy * m_tmu[1].m_dwdy + dx * m_tmu[1].m_dwdx) >> 4;
				m_tmu[1].m_starts += (dy * m_tmu[1].m_dsdy + dx * m_tmu[1].m_dsdx) >> 4;
				m_tmu[1].m_startt += (dy * m_tmu[1].m_dtdy + dx * m_tmu[1].m_dtdx) >> 4;
			}
		}
	}

	/* wait for any outstanding work to finish */
//  m_poly->wait("triangle");

	/* determine the draw buffer */
	destbuf = (m_type >= TYPE_VOODOO_BANSHEE) ? 1 : fbzmode.draw_buffer();
	switch (destbuf)
	{
		case 0:     /* front buffer */
			drawbuf = (u16 *)(m_fbi.ram + m_fbi.rgboffs[m_fbi.frontbuf]);
			m_fbi.video_changed = true;
			break;

		case 1:     /* back buffer */
			drawbuf = (u16 *)(m_fbi.ram + m_fbi.rgboffs[m_fbi.backbuf]);
			break;

		default:    /* reserved */
			return TRIANGLE_SETUP_CLOCKS;
	}

	/* find a rasterizer that matches our current state */
	pixels = triangle_create_work_item(drawbuf, texcount);

	/* update stats */
	m_reg.add(voodoo_regs::reg_fbiTrianglesOut, 1);

	/* update stats */
	m_stats.total_triangles++;

	g_profiler.stop();

	/* 1 pixel per clock, plus some setup time */
	if (LOG_REGISTERS) logerror("cycles = %d\n", TRIANGLE_SETUP_CLOCKS + pixels);
	return TRIANGLE_SETUP_CLOCKS + pixels;
}


/*-------------------------------------------------
    begin_triangle - execute the 'beginTri'
    command
-------------------------------------------------*/

s32 voodoo_device::begin_triangle()
{
	fbi_state::setup_vertex *sv = &m_fbi.svert[2];

	/* extract all the data from registers */
	sv->x  = m_reg.read_float(voodoo_regs::reg_sVx);
	sv->y  = m_reg.read_float(voodoo_regs::reg_sVy);
	sv->wb = m_reg.read_float(voodoo_regs::reg_sWb);
	sv->w0 = m_reg.read_float(voodoo_regs::reg_sWtmu0);
	sv->s0 = m_reg.read_float(voodoo_regs::reg_sS_W0);
	sv->t0 = m_reg.read_float(voodoo_regs::reg_sT_W0);
	sv->w1 = m_reg.read_float(voodoo_regs::reg_sWtmu1);
	sv->s1 = m_reg.read_float(voodoo_regs::reg_sS_Wtmu1);
	sv->t1 = m_reg.read_float(voodoo_regs::reg_sT_Wtmu1);
	sv->a  = m_reg.read_float(voodoo_regs::reg_sAlpha);
	sv->r  = m_reg.read_float(voodoo_regs::reg_sRed);
	sv->g  = m_reg.read_float(voodoo_regs::reg_sGreen);
	sv->b  = m_reg.read_float(voodoo_regs::reg_sBlue);

	/* spread it across all three verts and reset the count */
	m_fbi.svert[0] = m_fbi.svert[1] = m_fbi.svert[2];
	m_fbi.sverts = 1;

	return 0;
}


/*-------------------------------------------------
    draw_triangle - execute the 'DrawTri'
    command
-------------------------------------------------*/

s32 voodoo_device::draw_triangle()
{
	fbi_state::setup_vertex *sv = &m_fbi.svert[2];
	int cycles = 0;

	/* for strip mode, shuffle vertex 1 down to 0 */
	if (!m_reg.setup_mode().fan_mode())
		m_fbi.svert[0] = m_fbi.svert[1];

	/* copy 2 down to 1 regardless */
	m_fbi.svert[1] = m_fbi.svert[2];

	/* extract all the data from registers */
	sv->x  = m_reg.read_float(voodoo_regs::reg_sVx);
	sv->y  = m_reg.read_float(voodoo_regs::reg_sVy);
	sv->wb = m_reg.read_float(voodoo_regs::reg_sWb);
	sv->w0 = m_reg.read_float(voodoo_regs::reg_sWtmu0);
	sv->s0 = m_reg.read_float(voodoo_regs::reg_sS_W0);
	sv->t0 = m_reg.read_float(voodoo_regs::reg_sT_W0);
	sv->w1 = m_reg.read_float(voodoo_regs::reg_sWtmu1);
	sv->s1 = m_reg.read_float(voodoo_regs::reg_sS_Wtmu1);
	sv->t1 = m_reg.read_float(voodoo_regs::reg_sT_Wtmu1);
	sv->a  = m_reg.read_float(voodoo_regs::reg_sAlpha);
	sv->r  = m_reg.read_float(voodoo_regs::reg_sRed);
	sv->g  = m_reg.read_float(voodoo_regs::reg_sGreen);
	sv->b  = m_reg.read_float(voodoo_regs::reg_sBlue);

	/* if we have enough verts, go ahead and draw */
	if (++m_fbi.sverts >= 3)
		cycles = setup_and_draw_triangle();

	return cycles;
}



/***************************************************************************
    TRIANGLE HELPERS
***************************************************************************/

/*-------------------------------------------------
    setup_and_draw_triangle - process the setup
    parameters and render the triangle
-------------------------------------------------*/

s32 voodoo_device::setup_and_draw_triangle()
{
	/* compute the divisor */
	// Just need sign for now
	float divisor = ((m_fbi.svert[0].x - m_fbi.svert[1].x) * (m_fbi.svert[0].y - m_fbi.svert[2].y) -
					 (m_fbi.svert[0].x - m_fbi.svert[2].x) * (m_fbi.svert[0].y - m_fbi.svert[1].y));

	/* backface culling */
	auto const setup_mode = m_reg.setup_mode();
	if (setup_mode.enable_culling() & 0x20000)
	{
		int culling_sign = setup_mode.culling_sign();
		int divisor_sign = (divisor < 0);

		/* if doing strips and ping pong is enabled, apply the ping pong */
		if (!setup_mode.fan_mode() && !setup_mode.disable_ping_pong_correction())
			culling_sign ^= (m_fbi.sverts - 3) & 1;

		/* if our sign matches the culling sign, we're done for */
		if (divisor_sign == culling_sign)
			return TRIANGLE_SETUP_CLOCKS;
	}

	// Finish the divisor
	divisor = 1.0f / divisor;

	/* grab the X/Ys at least */
	m_fbi.ax = s16(m_fbi.svert[0].x * 16.0f);
	m_fbi.ay = s16(m_fbi.svert[0].y * 16.0f);
	m_fbi.bx = s16(m_fbi.svert[1].x * 16.0f);
	m_fbi.by = s16(m_fbi.svert[1].y * 16.0f);
	m_fbi.cx = s16(m_fbi.svert[2].x * 16.0f);
	m_fbi.cy = s16(m_fbi.svert[2].y * 16.0f);

	/* compute the dx/dy values */
	float dx1 = m_fbi.svert[0].y - m_fbi.svert[2].y;
	float dx2 = m_fbi.svert[0].y - m_fbi.svert[1].y;
	float dy1 = m_fbi.svert[0].x - m_fbi.svert[1].x;
	float dy2 = m_fbi.svert[0].x - m_fbi.svert[2].x;

	/* set up R,G,B */
	float tdiv = divisor * 4096.0f;
	if (setup_mode.setup_rgb())
	{
		m_fbi.startr = (s32)(m_fbi.svert[0].r * 4096.0f);
		m_fbi.drdx = (s32)(((m_fbi.svert[0].r - m_fbi.svert[1].r) * dx1 - (m_fbi.svert[0].r - m_fbi.svert[2].r) * dx2) * tdiv);
		m_fbi.drdy = (s32)(((m_fbi.svert[0].r - m_fbi.svert[2].r) * dy1 - (m_fbi.svert[0].r - m_fbi.svert[1].r) * dy2) * tdiv);
		m_fbi.startg = (s32)(m_fbi.svert[0].g * 4096.0f);
		m_fbi.dgdx = (s32)(((m_fbi.svert[0].g - m_fbi.svert[1].g) * dx1 - (m_fbi.svert[0].g - m_fbi.svert[2].g) * dx2) * tdiv);
		m_fbi.dgdy = (s32)(((m_fbi.svert[0].g - m_fbi.svert[2].g) * dy1 - (m_fbi.svert[0].g - m_fbi.svert[1].g) * dy2) * tdiv);
		m_fbi.startb = (s32)(m_fbi.svert[0].b * 4096.0f);
		m_fbi.dbdx = (s32)(((m_fbi.svert[0].b - m_fbi.svert[1].b) * dx1 - (m_fbi.svert[0].b - m_fbi.svert[2].b) * dx2) * tdiv);
		m_fbi.dbdy = (s32)(((m_fbi.svert[0].b - m_fbi.svert[2].b) * dy1 - (m_fbi.svert[0].b - m_fbi.svert[1].b) * dy2) * tdiv);
	}

	/* set up alpha */
	if (setup_mode.setup_alpha())
	{
		m_fbi.starta = (s32)(m_fbi.svert[0].a * 4096.0f);
		m_fbi.dadx = (s32)(((m_fbi.svert[0].a - m_fbi.svert[1].a) * dx1 - (m_fbi.svert[0].a - m_fbi.svert[2].a) * dx2) * tdiv);
		m_fbi.dady = (s32)(((m_fbi.svert[0].a - m_fbi.svert[2].a) * dy1 - (m_fbi.svert[0].a - m_fbi.svert[1].a) * dy2) * tdiv);
	}

	/* set up Z */
	if (setup_mode.setup_z())
	{
		m_fbi.startz = (s32)(m_fbi.svert[0].z * 4096.0f);
		m_fbi.dzdx = (s32)(((m_fbi.svert[0].z - m_fbi.svert[1].z) * dx1 - (m_fbi.svert[0].z - m_fbi.svert[2].z) * dx2) * tdiv);
		m_fbi.dzdy = (s32)(((m_fbi.svert[0].z - m_fbi.svert[2].z) * dy1 - (m_fbi.svert[0].z - m_fbi.svert[1].z) * dy2) * tdiv);
	}

	/* set up Wb */
	tdiv = divisor * 65536.0f * 65536.0f;
	if (setup_mode.setup_wb())
	{
		m_fbi.startw = m_tmu[0].m_startw = m_tmu[1].m_startw = (s64)(m_fbi.svert[0].wb * 65536.0f * 65536.0f);
		m_fbi.dwdx = m_tmu[0].m_dwdx = m_tmu[1].m_dwdx = ((m_fbi.svert[0].wb - m_fbi.svert[1].wb) * dx1 - (m_fbi.svert[0].wb - m_fbi.svert[2].wb) * dx2) * tdiv;
		m_fbi.dwdy = m_tmu[0].m_dwdy = m_tmu[1].m_dwdy = ((m_fbi.svert[0].wb - m_fbi.svert[2].wb) * dy1 - (m_fbi.svert[0].wb - m_fbi.svert[1].wb) * dy2) * tdiv;
	}

	/* set up W0 */
	if (setup_mode.setup_w0())
	{
		m_tmu[0].m_startw = m_tmu[1].m_startw = (s64)(m_fbi.svert[0].w0 * 65536.0f * 65536.0f);
		m_tmu[0].m_dwdx = m_tmu[1].m_dwdx = ((m_fbi.svert[0].w0 - m_fbi.svert[1].w0) * dx1 - (m_fbi.svert[0].w0 - m_fbi.svert[2].w0) * dx2) * tdiv;
		m_tmu[0].m_dwdy = m_tmu[1].m_dwdy = ((m_fbi.svert[0].w0 - m_fbi.svert[2].w0) * dy1 - (m_fbi.svert[0].w0 - m_fbi.svert[1].w0) * dy2) * tdiv;
	}

	/* set up S0,T0 */
	if (setup_mode.setup_st0())
	{
		m_tmu[0].m_starts = m_tmu[1].m_starts = (s64)(m_fbi.svert[0].s0 * 65536.0f * 65536.0f);
		m_tmu[0].m_dsdx = m_tmu[1].m_dsdx = ((m_fbi.svert[0].s0 - m_fbi.svert[1].s0) * dx1 - (m_fbi.svert[0].s0 - m_fbi.svert[2].s0) * dx2) * tdiv;
		m_tmu[0].m_dsdy = m_tmu[1].m_dsdy = ((m_fbi.svert[0].s0 - m_fbi.svert[2].s0) * dy1 - (m_fbi.svert[0].s0 - m_fbi.svert[1].s0) * dy2) * tdiv;
		m_tmu[0].m_startt = m_tmu[1].m_startt = (s64)(m_fbi.svert[0].t0 * 65536.0f * 65536.0f);
		m_tmu[0].m_dtdx = m_tmu[1].m_dtdx = ((m_fbi.svert[0].t0 - m_fbi.svert[1].t0) * dx1 - (m_fbi.svert[0].t0 - m_fbi.svert[2].t0) * dx2) * tdiv;
		m_tmu[0].m_dtdy = m_tmu[1].m_dtdy = ((m_fbi.svert[0].t0 - m_fbi.svert[2].t0) * dy1 - (m_fbi.svert[0].t0 - m_fbi.svert[1].t0) * dy2) * tdiv;
	}

	/* set up W1 */
	if (setup_mode.setup_w1())
	{
		m_tmu[1].m_startw = (s64)(m_fbi.svert[0].w1 * 65536.0f * 65536.0f);
		m_tmu[1].m_dwdx = ((m_fbi.svert[0].w1 - m_fbi.svert[1].w1) * dx1 - (m_fbi.svert[0].w1 - m_fbi.svert[2].w1) * dx2) * tdiv;
		m_tmu[1].m_dwdy = ((m_fbi.svert[0].w1 - m_fbi.svert[2].w1) * dy1 - (m_fbi.svert[0].w1 - m_fbi.svert[1].w1) * dy2) * tdiv;
	}

	/* set up S1,T1 */
	if (setup_mode.setup_st1())
	{
		m_tmu[1].m_starts = (s64)(m_fbi.svert[0].s1 * 65536.0f * 65536.0f);
		m_tmu[1].m_dsdx = ((m_fbi.svert[0].s1 - m_fbi.svert[1].s1) * dx1 - (m_fbi.svert[0].s1 - m_fbi.svert[2].s1) * dx2) * tdiv;
		m_tmu[1].m_dsdy = ((m_fbi.svert[0].s1 - m_fbi.svert[2].s1) * dy1 - (m_fbi.svert[0].s1 - m_fbi.svert[1].s1) * dy2) * tdiv;
		m_tmu[1].m_startt = (s64)(m_fbi.svert[0].t1 * 65536.0f * 65536.0f);
		m_tmu[1].m_dtdx = ((m_fbi.svert[0].t1 - m_fbi.svert[1].t1) * dx1 - (m_fbi.svert[0].t1 - m_fbi.svert[2].t1) * dx2) * tdiv;
		m_tmu[1].m_dtdy = ((m_fbi.svert[0].t1 - m_fbi.svert[2].t1) * dy1 - (m_fbi.svert[0].t1 - m_fbi.svert[1].t1) * dy2) * tdiv;
	}

	/* draw the triangle */
	return triangle();
}


/*-------------------------------------------------
    triangle_create_work_item - finish triangle
    setup and create the work item
-------------------------------------------------*/

s32 voodoo_device::triangle_create_work_item(u16 *drawbuf, int texcount)
{
	poly_extra_data &extra = m_poly->object_data_alloc();

	extra.u.raster.compute(m_type, texcount, m_reg, m_tmu[0].m_reg, m_tmu[1].m_reg);
	raster_info *info = find_rasterizer(texcount, extra.u.raster);
	voodoo_renderer::vertex_t vert[3];

	/* fill in the vertex data */
	vert[0].x = (float)m_fbi.ax * (1.0f / 16.0f);
	vert[0].y = (float)m_fbi.ay * (1.0f / 16.0f);
	vert[1].x = (float)m_fbi.bx * (1.0f / 16.0f);
	vert[1].y = (float)m_fbi.by * (1.0f / 16.0f);
	vert[2].x = (float)m_fbi.cx * (1.0f / 16.0f);
	vert[2].y = (float)m_fbi.cy * (1.0f / 16.0f);

	/* fill in the extra data */
	extra.info = info;
	extra.destbase = drawbuf;

	/* fill in triangle parameters */
	extra.ax = m_fbi.ax;
	extra.ay = m_fbi.ay;
	extra.startr = m_fbi.startr;
	extra.startg = m_fbi.startg;
	extra.startb = m_fbi.startb;
	extra.starta = m_fbi.starta;
	extra.startz = m_fbi.startz;
	extra.startw = m_fbi.startw;
	extra.drdx = m_fbi.drdx;
	extra.dgdx = m_fbi.dgdx;
	extra.dbdx = m_fbi.dbdx;
	extra.dadx = m_fbi.dadx;
	extra.dzdx = m_fbi.dzdx;
	extra.dwdx = m_fbi.dwdx;
	extra.drdy = m_fbi.drdy;
	extra.dgdy = m_fbi.dgdy;
	extra.dbdy = m_fbi.dbdy;
	extra.dady = m_fbi.dady;
	extra.dzdy = m_fbi.dzdy;
	extra.dwdy = m_fbi.dwdy;

	/* fill in texture 0 parameters */
	if (texcount > 0)
	{
		extra.starts0 = m_tmu[0].m_starts;
		extra.startt0 = m_tmu[0].m_startt;
		extra.startw0 = m_tmu[0].m_startw;
		extra.ds0dx = m_tmu[0].m_dsdx;
		extra.dt0dx = m_tmu[0].m_dtdx;
		extra.dw0dx = m_tmu[0].m_dwdx;
		extra.ds0dy = m_tmu[0].m_dsdy;
		extra.dt0dy = m_tmu[0].m_dtdy;
		extra.dw0dy = m_tmu[0].m_dwdy;
		extra.lodbase0 = m_tmu[0].prepare();
		m_stats.texture_mode[m_tmu[0].m_reg.texture_mode().format()]++;

		/* fill in texture 1 parameters */
		if (texcount > 1)
		{
			extra.starts1 = m_tmu[1].m_starts;
			extra.startt1 = m_tmu[1].m_startt;
			extra.startw1 = m_tmu[1].m_startw;
			extra.ds1dx = m_tmu[1].m_dsdx;
			extra.dt1dx = m_tmu[1].m_dtdx;
			extra.dw1dx = m_tmu[1].m_dwdx;
			extra.ds1dy = m_tmu[1].m_dsdy;
			extra.dt1dy = m_tmu[1].m_dtdy;
			extra.dw1dy = m_tmu[1].m_dwdy;
			extra.lodbase1 = m_tmu[1].prepare();
			m_stats.texture_mode[m_tmu[1].m_reg.texture_mode().format()]++;
		}
	}

	// fill in color parameters
	extra.color0 = m_reg.color0().argb();
	extra.color1 = m_reg.color1().argb();
	extra.zacolor = m_reg.za_color();

	// farm the rasterization out to other threads
	info->polys++;
	return m_poly->render_triangle(global_cliprect, info->callback, 0, vert[0], vert[1], vert[2]);
}



//**************************************************************************
//  RASTERIZER MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  add_rasterizer - add a rasterizer to our
//  hash table
//-------------------------------------------------

raster_info *voodoo_device::add_rasterizer(raster_params const &params, voodoo_renderer::mfp rasterizer, bool is_generic)
{
	raster_info &info = m_rasterizer[m_next_rasterizer++];

	if (m_next_rasterizer > MAX_RASTERIZERS)
		throw emu_fatalerror("voodoo_device::add_rasterizer: Out of space for new rasterizers!");

	// fill in the data
	info.next = nullptr;
	info.callback = voodoo_renderer::render_delegate(rasterizer, this);
	info.display = 0;
	info.is_generic = is_generic;
	info.hits = 0;
	info.polys = 0;
	info.hash = params.hash();
	info.params = params;

	// hook us into the hash table
	if (LOG_RASTERIZERS || !is_generic)
	{
		info.next = m_raster_hash[info.hash];
		m_raster_hash[info.hash] = &info;
	}

	if (LOG_RASTERIZERS)
		printf("Adding rasterizer : cp=%08X am=%08X fog=%08X fbz=%08X tm0=%08X tm1=%08X (hash=%d)\n",
				params.m_fbzcp, params.m_alphamode, params.m_fogmode, params.m_fbzmode,
				params.m_texmode0, params.m_texmode1, info.hash);

	return &info;
}


//-------------------------------------------------
//  find_rasterizer - find a rasterizer that
//  matches our current parameters and return
//  it, creating a new one if necessary
//-------------------------------------------------

voodoo::raster_info *voodoo_device::find_rasterizer(int texcount, raster_params const &params)
{
	// compute the hash
	u32 hash = params.hash();

	// find the appropriate hash entry
	raster_info *prev = nullptr;
	for (raster_info *info = m_raster_hash[hash]; info != nullptr; prev = info, info = info->next)
		if (info->params == params)
		{
			// got it, move us to the head of the list
			if (prev != nullptr)
			{
				prev->next = info->next;
				info->next = m_raster_hash[hash];
				m_raster_hash[hash] = info;
			}
			return info;
		}

	// add a new one if we're logging usage
	if (LOG_RASTERIZERS)
		return add_rasterizer(params, generic_raster_table[texcount].callback_mfp, true);

	// otherwise just return the generic one directly
	return m_generic_rasterizer[texcount];
}


/*-------------------------------------------------
    dump_rasterizer_stats - dump statistics on
    the current rasterizer usage patterns
-------------------------------------------------*/

void voodoo_device::dump_rasterizer_stats()
{
	static u8 display_index;
	raster_info *cur, *best;
	int hash;

	printf("----\n");
	display_index++;

	// loop until we've displayed everything
	while (1)
	{
		best = nullptr;

		// find the highest entry
		for (hash = 0; hash < RASTER_HASH_SIZE; hash++)
			for (cur = m_raster_hash[hash]; cur != nullptr; cur = cur->next)
				if (cur->display != display_index && (best == nullptr || cur->hits > best->hits))
					best = cur;

		// if we're done, we're done
		if (best == nullptr || best->hits == 0)
			break;

		// print it
		printf("%s RASTERIZER_ENTRY( 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X ) /* %2d %8d %10d */\n",
			best->is_generic ? "   " : "// ",
			best->params.m_fbzcp,
			best->params.m_alphamode,
			best->params.m_fogmode,
			best->params.m_fbzmode,
			best->params.m_texmode0,
			best->params.m_texmode1,
			best->hash,
			best->polys,
			best->hits);

		// reset
		best->display = display_index;
	}
}

voodoo_device::voodoo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 vdt) :
	device_t(mconfig, type, tag, owner, clock),
	m_type(vdt),
	m_alt_regmap(0),
	m_chipmask(0),
	m_index(0),
	m_screen(nullptr),
	m_cpu(nullptr),
	m_freq(0),
	m_attoseconds_per_cycle(0),
	m_trigger(0),
	m_regaccess(nullptr),
	m_regnames(nullptr),
	m_last_status_pc(0),
	m_last_status_value(0),
	m_next_rasterizer(0),
	m_send_config(false),
	m_tmu_config(0),
	m_fbmem(0),
	m_tmumem0(0),
	m_tmumem1(0),
	m_vblank(*this),
	m_stall(*this),
	m_pciint(*this),
	m_screen_finder(*this, finder_base::DUMMY_TAG),
	m_cpu_finder(*this, finder_base::DUMMY_TAG)
{
}

voodoo_device::~voodoo_device()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void voodoo_device::device_reset()
{
	soft_reset();
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void voodoo_device::device_stop()
{
}


DEFINE_DEVICE_TYPE(VOODOO_1, voodoo_1_device, "voodoo_1", "3dfx Voodoo Graphics")

voodoo_1_device::voodoo_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_device(mconfig, VOODOO_1, tag, owner, clock, TYPE_VOODOO_1)
{
}


DEFINE_DEVICE_TYPE(VOODOO_2, voodoo_2_device, "voodoo_2", "3dfx Voodoo 2")

voodoo_2_device::voodoo_2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_device(mconfig, VOODOO_2, tag, owner, clock, TYPE_VOODOO_2)
{
}


DEFINE_DEVICE_TYPE(VOODOO_BANSHEE, voodoo_banshee_device, "voodoo_banshee", "3dfx Voodoo Banshee")

voodoo_banshee_device::voodoo_banshee_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_banshee_device(mconfig, VOODOO_BANSHEE, tag, owner, clock, TYPE_VOODOO_BANSHEE)
{
}

voodoo_banshee_device::voodoo_banshee_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 vdt)
	: voodoo_device(mconfig, type, tag, owner, clock, vdt)
{
}


DEFINE_DEVICE_TYPE(VOODOO_3, voodoo_3_device, "voodoo_3", "3dfx Voodoo 3")

voodoo_3_device::voodoo_3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_banshee_device(mconfig, VOODOO_3, tag, owner, clock, TYPE_VOODOO_3)
{
}



/***************************************************************************
    GENERIC RASTERIZERS
***************************************************************************/

/*-------------------------------------------------
    raster_fastfill - per-scanline
    implementation of the 'fastfill' command
-------------------------------------------------*/

void voodoo_device::raster_fastfill(s32 y, const voodoo_renderer::extent_t &extent, const poly_extra_data &extra, int threadid)
{
	thread_stats_block &threadstats = m_thread_stats[threadid];
	auto const fbzmode = m_reg.fbz_mode();
	s32 startx = extent.startx;
	s32 stopx = extent.stopx;
	int x;

	/* determine the screen Y */
	s32 scry = y;
	if (fbzmode.y_origin())
		scry = m_fbi.yorigin - y;

	/* fill this RGB row */
	if (fbzmode.rgb_buffer_mask())
	{
		const u16 *ditherow = &extra.u.dither[(y & 3) * 4];
		u64 expanded = *(u64 *)ditherow;
		u16 *dest = extra.destbase + scry * m_fbi.rowpixels;

		for (x = startx; x < stopx && (x & 3) != 0; x++)
			dest[x] = ditherow[x & 3];
		for ( ; x < (stopx & ~3); x += 4)
			*(u64 *)&dest[x] = expanded;
		for ( ; x < stopx; x++)
			dest[x] = ditherow[x & 3];
		threadstats.pixels_out += stopx - startx;
	}

	/* fill this dest buffer row */
	if (fbzmode.aux_buffer_mask() && m_fbi.auxoffs != ~0)
	{
		u16 depth = m_reg.za_color();
		u64 expanded = (u64(depth) << 48) | (u64(depth) << 32) | (u64(depth) << 16) | u64(depth);
		u16 *dest = (u16 *)(m_fbi.ram + m_fbi.auxoffs) + scry * m_fbi.rowpixels;

		for (x = startx; x < stopx && (x & 3) != 0; x++)
			dest[x] = depth;
		for ( ; x < (stopx & ~3); x += 4)
			*(u64 *)&dest[x] = expanded;
		for ( ; x < stopx; x++)
			dest[x] = depth;
	}
}
