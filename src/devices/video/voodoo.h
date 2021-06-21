// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo.h

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#ifndef MAME_VIDEO_VOODOO_H
#define MAME_VIDEO_VOODOO_H

#pragma once

#include "video/polylgcy.h"
#include "video/rgbutil.h"
#include "screen.h"


/*************************************
 *
 *  Misc. constants
 *
 *************************************/

/* enumeration describing reasons we might be stalled */
enum
{
	NOT_STALLED = 0,
	STALLED_UNTIL_FIFO_LWM,
	STALLED_UNTIL_FIFO_EMPTY
};



// Use old table lookup versus straight double divide
#define USE_FAST_RECIP  0

/* maximum number of TMUs */
#define MAX_TMU                 2

/* accumulate operations less than this number of clocks */
#define ACCUMULATE_THRESHOLD    0

/* number of clocks to set up a triangle (just a guess) */
#define TRIANGLE_SETUP_CLOCKS   100

/* maximum number of rasterizers */
#define MAX_RASTERIZERS         1024

/* size of the rasterizer hash table */
#define RASTER_HASH_SIZE        97

/* flags for LFB writes */
#define LFB_RGB_PRESENT         1
#define LFB_ALPHA_PRESENT       2
#define LFB_DEPTH_PRESENT       4
#define LFB_DEPTH_PRESENT_MSW   8

/* flags for the register access array */
#define REGISTER_READ           0x01        /* reads are allowed */
#define REGISTER_WRITE          0x02        /* writes are allowed */
#define REGISTER_PIPELINED      0x04        /* writes are pipelined */
#define REGISTER_FIFO           0x08        /* writes go to FIFO */
#define REGISTER_WRITETHRU      0x10        /* writes are valid even for CMDFIFO */

/* shorter combinations to make the table smaller */
#define REG_R                   (REGISTER_READ)
#define REG_W                   (REGISTER_WRITE)
#define REG_WT                  (REGISTER_WRITE | REGISTER_WRITETHRU)
#define REG_RW                  (REGISTER_READ | REGISTER_WRITE)
#define REG_RWT                 (REGISTER_READ | REGISTER_WRITE | REGISTER_WRITETHRU)
#define REG_RP                  (REGISTER_READ | REGISTER_PIPELINED)
#define REG_WP                  (REGISTER_WRITE | REGISTER_PIPELINED)
#define REG_RWP                 (REGISTER_READ | REGISTER_WRITE | REGISTER_PIPELINED)
#define REG_RWPT                (REGISTER_READ | REGISTER_WRITE | REGISTER_PIPELINED | REGISTER_WRITETHRU)
#define REG_RF                  (REGISTER_READ | REGISTER_FIFO)
#define REG_WF                  (REGISTER_WRITE | REGISTER_FIFO)
#define REG_RWF                 (REGISTER_READ | REGISTER_WRITE | REGISTER_FIFO)
#define REG_RPF                 (REGISTER_READ | REGISTER_PIPELINED | REGISTER_FIFO)
#define REG_WPF                 (REGISTER_WRITE | REGISTER_PIPELINED | REGISTER_FIFO)
#define REG_RWPF                (REGISTER_READ | REGISTER_WRITE | REGISTER_PIPELINED | REGISTER_FIFO)

/* lookup bits is the log2 of the size of the reciprocal/log table */
#define RECIPLOG_LOOKUP_BITS    9

/* input precision is how many fraction bits the input value has; this is a 64-bit number */
#define RECIPLOG_INPUT_PREC     32

/* lookup precision is how many fraction bits each table entry contains */
#define RECIPLOG_LOOKUP_PREC    22

/* output precision is how many fraction bits the result should have */
#define RECIP_OUTPUT_PREC       15
#define LOG_OUTPUT_PREC         8



/*************************************
 *
 *  Register constants
 *
 *************************************/

/* Codes to the right:
    R = readable
    W = writeable
    P = pipelined
    F = goes to FIFO
*/

/* 0x000 */
#define vdstatus        (0x000/4)   /* R  P  */
#define intrCtrl        (0x004/4)   /* RW P   -- Voodoo2/Banshee only */
#define vertexAx        (0x008/4)   /*  W PF */
#define vertexAy        (0x00c/4)   /*  W PF */
#define vertexBx        (0x010/4)   /*  W PF */
#define vertexBy        (0x014/4)   /*  W PF */
#define vertexCx        (0x018/4)   /*  W PF */
#define vertexCy        (0x01c/4)   /*  W PF */
#define startR          (0x020/4)   /*  W PF */
#define startG          (0x024/4)   /*  W PF */
#define startB          (0x028/4)   /*  W PF */
#define startZ          (0x02c/4)   /*  W PF */
#define startA          (0x030/4)   /*  W PF */
#define startS          (0x034/4)   /*  W PF */
#define startT          (0x038/4)   /*  W PF */
#define startW          (0x03c/4)   /*  W PF */

/* 0x040 */
#define dRdX            (0x040/4)   /*  W PF */
#define dGdX            (0x044/4)   /*  W PF */
#define dBdX            (0x048/4)   /*  W PF */
#define dZdX            (0x04c/4)   /*  W PF */
#define dAdX            (0x050/4)   /*  W PF */
#define dSdX            (0x054/4)   /*  W PF */
#define dTdX            (0x058/4)   /*  W PF */
#define dWdX            (0x05c/4)   /*  W PF */
#define dRdY            (0x060/4)   /*  W PF */
#define dGdY            (0x064/4)   /*  W PF */
#define dBdY            (0x068/4)   /*  W PF */
#define dZdY            (0x06c/4)   /*  W PF */
#define dAdY            (0x070/4)   /*  W PF */
#define dSdY            (0x074/4)   /*  W PF */
#define dTdY            (0x078/4)   /*  W PF */
#define dWdY            (0x07c/4)   /*  W PF */

/* 0x080 */
#define triangleCMD     (0x080/4)   /*  W PF */
#define fvertexAx       (0x088/4)   /*  W PF */
#define fvertexAy       (0x08c/4)   /*  W PF */
#define fvertexBx       (0x090/4)   /*  W PF */
#define fvertexBy       (0x094/4)   /*  W PF */
#define fvertexCx       (0x098/4)   /*  W PF */
#define fvertexCy       (0x09c/4)   /*  W PF */
#define fstartR         (0x0a0/4)   /*  W PF */
#define fstartG         (0x0a4/4)   /*  W PF */
#define fstartB         (0x0a8/4)   /*  W PF */
#define fstartZ         (0x0ac/4)   /*  W PF */
#define fstartA         (0x0b0/4)   /*  W PF */
#define fstartS         (0x0b4/4)   /*  W PF */
#define fstartT         (0x0b8/4)   /*  W PF */
#define fstartW         (0x0bc/4)   /*  W PF */

/* 0x0c0 */
#define fdRdX           (0x0c0/4)   /*  W PF */
#define fdGdX           (0x0c4/4)   /*  W PF */
#define fdBdX           (0x0c8/4)   /*  W PF */
#define fdZdX           (0x0cc/4)   /*  W PF */
#define fdAdX           (0x0d0/4)   /*  W PF */
#define fdSdX           (0x0d4/4)   /*  W PF */
#define fdTdX           (0x0d8/4)   /*  W PF */
#define fdWdX           (0x0dc/4)   /*  W PF */
#define fdRdY           (0x0e0/4)   /*  W PF */
#define fdGdY           (0x0e4/4)   /*  W PF */
#define fdBdY           (0x0e8/4)   /*  W PF */
#define fdZdY           (0x0ec/4)   /*  W PF */
#define fdAdY           (0x0f0/4)   /*  W PF */
#define fdSdY           (0x0f4/4)   /*  W PF */
#define fdTdY           (0x0f8/4)   /*  W PF */
#define fdWdY           (0x0fc/4)   /*  W PF */

/* 0x100 */
#define ftriangleCMD    (0x100/4)   /*  W PF */
#define fbzColorPath    (0x104/4)   /* RW PF */
#define fogMode         (0x108/4)   /* RW PF */
#define alphaMode       (0x10c/4)   /* RW PF */
#define fbzMode         (0x110/4)   /* RW  F */
#define lfbMode         (0x114/4)   /* RW  F */
#define clipLeftRight   (0x118/4)   /* RW  F */
#define clipLowYHighY   (0x11c/4)   /* RW  F */
#define nopCMD          (0x120/4)   /*  W  F */
#define fastfillCMD     (0x124/4)   /*  W  F */
#define swapbufferCMD   (0x128/4)   /*  W  F */
#define fogColor        (0x12c/4)   /*  W  F */
#define zaColor         (0x130/4)   /*  W  F */
#define chromaKey       (0x134/4)   /*  W  F */
#define chromaRange     (0x138/4)   /*  W  F  -- Voodoo2/Banshee only */
#define userIntrCMD     (0x13c/4)   /*  W  F  -- Voodoo2/Banshee only */

/* 0x140 */
#define stipple         (0x140/4)   /* RW  F */
#define color0          (0x144/4)   /* RW  F */
#define color1          (0x148/4)   /* RW  F */
#define fbiPixelsIn     (0x14c/4)   /* R     */
#define fbiChromaFail   (0x150/4)   /* R     */
#define fbiZfuncFail    (0x154/4)   /* R     */
#define fbiAfuncFail    (0x158/4)   /* R     */
#define fbiPixelsOut    (0x15c/4)   /* R     */
#define fogTable        (0x160/4)   /*  W  F */

/* 0x1c0 */
#define cmdFifoBaseAddr (0x1e0/4)   /* RW     -- Voodoo2 only */
#define cmdFifoBump     (0x1e4/4)   /* RW     -- Voodoo2 only */
#define cmdFifoRdPtr    (0x1e8/4)   /* RW     -- Voodoo2 only */
#define cmdFifoAMin     (0x1ec/4)   /* RW     -- Voodoo2 only */
#define colBufferAddr   (0x1ec/4)   /* RW     -- Banshee only */
#define cmdFifoAMax     (0x1f0/4)   /* RW     -- Voodoo2 only */
#define colBufferStride (0x1f0/4)   /* RW     -- Banshee only */
#define cmdFifoDepth    (0x1f4/4)   /* RW     -- Voodoo2 only */
#define auxBufferAddr   (0x1f4/4)   /* RW     -- Banshee only */
#define cmdFifoHoles    (0x1f8/4)   /* RW     -- Voodoo2 only */
#define auxBufferStride (0x1f8/4)   /* RW     -- Banshee only */

/* 0x200 */
#define fbiInit4        (0x200/4)   /* RW     -- Voodoo/Voodoo2 only */
#define clipLeftRight1  (0x200/4)   /* RW     -- Banshee only */
#define vRetrace        (0x204/4)   /* R      -- Voodoo/Voodoo2 only */
#define clipTopBottom1  (0x204/4)   /* RW     -- Banshee only */
#define backPorch       (0x208/4)   /* RW     -- Voodoo/Voodoo2 only */
#define videoDimensions (0x20c/4)   /* RW     -- Voodoo/Voodoo2 only */
#define fbiInit0        (0x210/4)   /* RW     -- Voodoo/Voodoo2 only */
#define fbiInit1        (0x214/4)   /* RW     -- Voodoo/Voodoo2 only */
#define fbiInit2        (0x218/4)   /* RW     -- Voodoo/Voodoo2 only */
#define fbiInit3        (0x21c/4)   /* RW     -- Voodoo/Voodoo2 only */
#define hSync           (0x220/4)   /*  W     -- Voodoo/Voodoo2 only */
#define vSync           (0x224/4)   /*  W     -- Voodoo/Voodoo2 only */
#define clutData        (0x228/4)   /*  W  F  -- Voodoo/Voodoo2 only */
#define dacData         (0x22c/4)   /*  W     -- Voodoo/Voodoo2 only */
#define maxRgbDelta     (0x230/4)   /*  W     -- Voodoo/Voodoo2 only */
#define hBorder         (0x234/4)   /*  W     -- Voodoo2 only */
#define vBorder         (0x238/4)   /*  W     -- Voodoo2 only */
#define borderColor     (0x23c/4)   /*  W     -- Voodoo2 only */

/* 0x240 */
#define hvRetrace       (0x240/4)   /* R      -- Voodoo2 only */
#define fbiInit5        (0x244/4)   /* RW     -- Voodoo2 only */
#define fbiInit6        (0x248/4)   /* RW     -- Voodoo2 only */
#define fbiInit7        (0x24c/4)   /* RW     -- Voodoo2 only */
#define swapPending     (0x24c/4)   /*  W     -- Banshee only */
#define leftOverlayBuf  (0x250/4)   /*  W     -- Banshee only */
#define rightOverlayBuf (0x254/4)   /*  W     -- Banshee only */
#define fbiSwapHistory  (0x258/4)   /* R      -- Voodoo2/Banshee only */
#define fbiTrianglesOut (0x25c/4)   /* R      -- Voodoo2/Banshee only */
#define sSetupMode      (0x260/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sVx             (0x264/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sVy             (0x268/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sARGB           (0x26c/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sRed            (0x270/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sGreen          (0x274/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sBlue           (0x278/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sAlpha          (0x27c/4)   /*  W PF  -- Voodoo2/Banshee only */

/* 0x280 */
#define sVz             (0x280/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sWb             (0x284/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sWtmu0          (0x288/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sS_W0           (0x28c/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sT_W0           (0x290/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sWtmu1          (0x294/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sS_Wtmu1        (0x298/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sT_Wtmu1        (0x29c/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sDrawTriCMD     (0x2a0/4)   /*  W PF  -- Voodoo2/Banshee only */
#define sBeginTriCMD    (0x2a4/4)   /*  W PF  -- Voodoo2/Banshee only */

/* 0x2c0 */
#define bltSrcBaseAddr  (0x2c0/4)   /* RW PF  -- Voodoo2 only */
#define bltDstBaseAddr  (0x2c4/4)   /* RW PF  -- Voodoo2 only */
#define bltXYStrides    (0x2c8/4)   /* RW PF  -- Voodoo2 only */
#define bltSrcChromaRange (0x2cc/4) /* RW PF  -- Voodoo2 only */
#define bltDstChromaRange (0x2d0/4) /* RW PF  -- Voodoo2 only */
#define bltClipX        (0x2d4/4)   /* RW PF  -- Voodoo2 only */
#define bltClipY        (0x2d8/4)   /* RW PF  -- Voodoo2 only */
#define bltSrcXY        (0x2e0/4)   /* RW PF  -- Voodoo2 only */
#define bltDstXY        (0x2e4/4)   /* RW PF  -- Voodoo2 only */
#define bltSize         (0x2e8/4)   /* RW PF  -- Voodoo2 only */
#define bltRop          (0x2ec/4)   /* RW PF  -- Voodoo2 only */
#define bltColor        (0x2f0/4)   /* RW PF  -- Voodoo2 only */
#define bltCommand      (0x2f8/4)   /* RW PF  -- Voodoo2 only */
#define bltData         (0x2fc/4)   /*  W PF  -- Voodoo2 only */

/* 0x300 */
#define textureMode     (0x300/4)   /*  W PF */
#define tLOD            (0x304/4)   /*  W PF */
#define tDetail         (0x308/4)   /*  W PF */
#define texBaseAddr     (0x30c/4)   /*  W PF */
#define texBaseAddr_1   (0x310/4)   /*  W PF */
#define texBaseAddr_2   (0x314/4)   /*  W PF */
#define texBaseAddr_3_8 (0x318/4)   /*  W PF */
#define trexInit0       (0x31c/4)   /*  W  F  -- Voodoo/Voodoo2 only */
#define trexInit1       (0x320/4)   /*  W  F */
#define nccTable        (0x324/4)   /*  W  F */



// 2D registers
#define banshee2D_clip0Min          (0x008/4)
#define banshee2D_clip0Max          (0x00c/4)
#define banshee2D_dstBaseAddr       (0x010/4)
#define banshee2D_dstFormat         (0x014/4)
#define banshee2D_srcColorkeyMin    (0x018/4)
#define banshee2D_srcColorkeyMax    (0x01c/4)
#define banshee2D_dstColorkeyMin    (0x020/4)
#define banshee2D_dstColorkeyMax    (0x024/4)
#define banshee2D_bresError0        (0x028/4)
#define banshee2D_bresError1        (0x02c/4)
#define banshee2D_rop               (0x030/4)
#define banshee2D_srcBaseAddr       (0x034/4)
#define banshee2D_commandExtra      (0x038/4)
#define banshee2D_lineStipple       (0x03c/4)
#define banshee2D_lineStyle         (0x040/4)
#define banshee2D_pattern0Alias     (0x044/4)
#define banshee2D_pattern1Alias     (0x048/4)
#define banshee2D_clip1Min          (0x04c/4)
#define banshee2D_clip1Max          (0x050/4)
#define banshee2D_srcFormat         (0x054/4)
#define banshee2D_srcSize           (0x058/4)
#define banshee2D_srcXY             (0x05c/4)
#define banshee2D_colorBack         (0x060/4)
#define banshee2D_colorFore         (0x064/4)
#define banshee2D_dstSize           (0x068/4)
#define banshee2D_dstXY             (0x06c/4)
#define banshee2D_command           (0x070/4)


/*************************************
 *
 *  Voodoo Banshee I/O space registers
 *
 *************************************/

/* 0x000 */
#define io_status                       (0x000/4)   /*  */
#define io_pciInit0                     (0x004/4)   /*  */
#define io_sipMonitor                   (0x008/4)   /*  */
#define io_lfbMemoryConfig              (0x00c/4)   /*  */
#define io_miscInit0                    (0x010/4)   /*  */
#define io_miscInit1                    (0x014/4)   /*  */
#define io_dramInit0                    (0x018/4)   /*  */
#define io_dramInit1                    (0x01c/4)   /*  */
#define io_agpInit                      (0x020/4)   /*  */
#define io_tmuGbeInit                   (0x024/4)   /*  */
#define io_vgaInit0                     (0x028/4)   /*  */
#define io_vgaInit1                     (0x02c/4)   /*  */
#define io_dramCommand                  (0x030/4)   /*  */
#define io_dramData                     (0x034/4)   /*  */

/* 0x040 */
#define io_pllCtrl0                     (0x040/4)   /*  */
#define io_pllCtrl1                     (0x044/4)   /*  */
#define io_pllCtrl2                     (0x048/4)   /*  */
#define io_dacMode                      (0x04c/4)   /*  */
#define io_dacAddr                      (0x050/4)   /*  */
#define io_dacData                      (0x054/4)   /*  */
#define io_rgbMaxDelta                  (0x058/4)   /*  */
#define io_vidProcCfg                   (0x05c/4)   /*  */
#define io_hwCurPatAddr                 (0x060/4)   /*  */
#define io_hwCurLoc                     (0x064/4)   /*  */
#define io_hwCurC0                      (0x068/4)   /*  */
#define io_hwCurC1                      (0x06c/4)   /*  */
#define io_vidInFormat                  (0x070/4)   /*  */
#define io_vidInStatus                  (0x074/4)   /*  */
#define io_vidSerialParallelPort        (0x078/4)   /*  */
#define io_vidInXDecimDeltas            (0x07c/4)   /*  */

/* 0x080 */
#define io_vidInDecimInitErrs           (0x080/4)   /*  */
#define io_vidInYDecimDeltas            (0x084/4)   /*  */
#define io_vidPixelBufThold             (0x088/4)   /*  */
#define io_vidChromaMin                 (0x08c/4)   /*  */
#define io_vidChromaMax                 (0x090/4)   /*  */
#define io_vidCurrentLine               (0x094/4)   /*  */
#define io_vidScreenSize                (0x098/4)   /*  */
#define io_vidOverlayStartCoords        (0x09c/4)   /*  */
#define io_vidOverlayEndScreenCoord     (0x0a0/4)   /*  */
#define io_vidOverlayDudx               (0x0a4/4)   /*  */
#define io_vidOverlayDudxOffsetSrcWidth (0x0a8/4)   /*  */
#define io_vidOverlayDvdy               (0x0ac/4)   /*  */
#define io_vgab0                        (0x0b0/4)   /*  */
#define io_vgab4                        (0x0b4/4)   /*  */
#define io_vgab8                        (0x0b8/4)   /*  */
#define io_vgabc                        (0x0bc/4)   /*  */

/* 0x0c0 */
#define io_vgac0                        (0x0c0/4)   /*  */
#define io_vgac4                        (0x0c4/4)   /*  */
#define io_vgac8                        (0x0c8/4)   /*  */
#define io_vgacc                        (0x0cc/4)   /*  */
#define io_vgad0                        (0x0d0/4)   /*  */
#define io_vgad4                        (0x0d4/4)   /*  */
#define io_vgad8                        (0x0d8/4)   /*  */
#define io_vgadc                        (0x0dc/4)   /*  */
#define io_vidOverlayDvdyOffset         (0x0e0/4)   /*  */
#define io_vidDesktopStartAddr          (0x0e4/4)   /*  */
#define io_vidDesktopOverlayStride      (0x0e8/4)   /*  */
#define io_vidInAddr0                   (0x0ec/4)   /*  */
#define io_vidInAddr1                   (0x0f0/4)   /*  */
#define io_vidInAddr2                   (0x0f4/4)   /*  */
#define io_vidInStride                  (0x0f8/4)   /*  */
#define io_vidCurrOverlayStartAddr      (0x0fc/4)   /*  */



/*************************************
 *
 *  Voodoo Banshee AGP space registers
 *
 *************************************/

/* 0x000 */
#define agpReqSize              (0x000/4)   /*  */
#define agpHostAddressLow       (0x004/4)   /*  */
#define agpHostAddressHigh      (0x008/4)   /*  */
#define agpGraphicsAddress      (0x00c/4)   /*  */
#define agpGraphicsStride       (0x010/4)   /*  */
#define agpMoveCMD              (0x014/4)   /*  */
#define cmdBaseAddr0            (0x020/4)   /*  */
#define cmdBaseSize0            (0x024/4)   /*  */
#define cmdBump0                (0x028/4)   /*  */
#define cmdRdPtrL0              (0x02c/4)   /*  */
#define cmdRdPtrH0              (0x030/4)   /*  */
#define cmdAMin0                (0x034/4)   /*  */
#define cmdAMax0                (0x03c/4)   /*  */

/* 0x040 */
#define cmdFifoDepth0           (0x044/4)   /*  */
#define cmdHoleCnt0             (0x048/4)   /*  */
#define cmdBaseAddr1            (0x050/4)   /*  */
#define cmdBaseSize1            (0x054/4)   /*  */
#define cmdBump1                (0x058/4)   /*  */
#define cmdRdPtrL1              (0x05c/4)   /*  */
#define cmdRdPtrH1              (0x060/4)   /*  */
#define cmdAMin1                (0x064/4)   /*  */
#define cmdAMax1                (0x06c/4)   /*  */
#define cmdFifoDepth1           (0x074/4)   /*  */
#define cmdHoleCnt1             (0x078/4)   /*  */

/* 0x080 */
#define cmdFifoThresh           (0x080/4)   /*  */
#define cmdHoleInt              (0x084/4)   /*  */

/* 0x100 */
#define yuvBaseAddress          (0x100/4)   /*  */
#define yuvStride               (0x104/4)   /*  */
#define crc1                    (0x120/4)   /*  */
#define crc2                    (0x130/4)   /*  */



/*************************************
 *
 *  Dithering tables
 *
 *************************************/

static const uint8_t dither_matrix_4x4[16] =
{
	 0,  8,  2, 10,
	12,  4, 14,  6,
	 3, 11,  1,  9,
	15,  7, 13,  5
};

//static const uint8_t dither_matrix_2x2[16] =
//{
//      2, 10,  2, 10,
//  14,  6, 14,  6,
//      2, 10,  2, 10,
//  14,  6, 14,  6
//};
// Using this matrix allows iteagle video memory tests to pass
static const uint8_t dither_matrix_2x2[16] =
{
	8, 10, 8, 10,
	11, 9, 11, 9,
	8, 10, 8, 10,
	11, 9, 11, 9
};

// Dither 4x4 subtraction matrix used in alpha blending
static const uint8_t dither_subtract_4x4[16] =
{
	(15 - 0) >> 1,  (15 - 8) >> 1,  (15 - 2) >> 1, (15 - 10) >> 1,
	(15 - 12) >> 1,  (15 - 4) >> 1, (15 - 14) >> 1,  (15 - 6) >> 1,
	(15 - 3) >> 1, (15 - 11) >> 1,  (15 - 1) >> 1,  (15 - 9) >> 1,
	(15 - 15) >> 1,  (15 - 7) >> 1, (15 - 13) >> 1,  (15 - 5) >> 1
};

// Dither 2x2 subtraction matrix used in alpha blending
static const uint8_t dither_subtract_2x2[16] =
{
	(15 - 8) >> 1, (15 - 10) >> 1, (15 - 8) >> 1, (15 - 10) >> 1,
	(15 - 11) >> 1, (15 - 9) >> 1, (15 - 11) >> 1, (15 - 9) >> 1,
	(15 - 8) >> 1, (15 - 10) >> 1, (15 - 8) >> 1, (15 - 10) >> 1,
	(15 - 11) >> 1, (15 - 9) >> 1, (15 - 11) >> 1, (15 - 9) >> 1
};

/*************************************
 *
 *  Macros for extracting pixels
 *
 *************************************/

#define EXTRACT_565_TO_888(val, a, b, c)                    \
	(a) = (((val) >> 8) & 0xf8) | (((val) >> 13) & 0x07);   \
	(b) = (((val) >> 3) & 0xfc) | (((val) >> 9) & 0x03);    \
	(c) = (((val) << 3) & 0xf8) | (((val) >> 2) & 0x07);
#define EXTRACT_x555_TO_888(val, a, b, c)                   \
	(a) = (((val) >> 7) & 0xf8) | (((val) >> 12) & 0x07);   \
	(b) = (((val) >> 2) & 0xf8) | (((val) >> 7) & 0x07);    \
	(c) = (((val) << 3) & 0xf8) | (((val) >> 2) & 0x07);
#define EXTRACT_555x_TO_888(val, a, b, c)                   \
	(a) = (((val) >> 8) & 0xf8) | (((val) >> 13) & 0x07);   \
	(b) = (((val) >> 3) & 0xf8) | (((val) >> 8) & 0x07);    \
	(c) = (((val) << 2) & 0xf8) | (((val) >> 3) & 0x07);
#define EXTRACT_1555_TO_8888(val, a, b, c, d)               \
	(a) = ((int16_t)(val) >> 15) & 0xff;                      \
	EXTRACT_x555_TO_888(val, b, c, d)
#define EXTRACT_5551_TO_8888(val, a, b, c, d)               \
	EXTRACT_555x_TO_888(val, a, b, c)                       \
	(d) = ((val) & 0x0001) ? 0xff : 0x00;
#define EXTRACT_x888_TO_888(val, a, b, c)                   \
	(a) = ((val) >> 16) & 0xff;                             \
	(b) = ((val) >> 8) & 0xff;                              \
	(c) = ((val) >> 0) & 0xff;
#define EXTRACT_888x_TO_888(val, a, b, c)                   \
	(a) = ((val) >> 24) & 0xff;                             \
	(b) = ((val) >> 16) & 0xff;                             \
	(c) = ((val) >> 8) & 0xff;
#define EXTRACT_8888_TO_8888(val, a, b, c, d)               \
	(a) = ((val) >> 24) & 0xff;                             \
	(b) = ((val) >> 16) & 0xff;                             \
	(c) = ((val) >> 8) & 0xff;                              \
	(d) = ((val) >> 0) & 0xff;
#define EXTRACT_4444_TO_8888(val, a, b, c, d)               \
	(a) = (((val) >> 8) & 0xf0) | (((val) >> 12) & 0x0f);   \
	(b) = (((val) >> 4) & 0xf0) | (((val) >> 8) & 0x0f);    \
	(c) = (((val) >> 0) & 0xf0) | (((val) >> 4) & 0x0f);    \
	(d) = (((val) << 4) & 0xf0) | (((val) >> 0) & 0x0f);
#define EXTRACT_332_TO_888(val, a, b, c)                    \
	(a) = (((val) >> 0) & 0xe0) | (((val) >> 3) & 0x1c) | (((val) >> 6) & 0x03); \
	(b) = (((val) << 3) & 0xe0) | (((val) >> 0) & 0x1c) | (((val) >> 3) & 0x03); \
	(c) = (((val) << 6) & 0xc0) | (((val) << 4) & 0x30) | (((val) << 2) & 0x0c) | (((val) << 0) & 0x03);


/*************************************
 *
 *  Misc. macros
 *
 *************************************/

/* macro for clamping a value between minimum and maximum values */
#define CLAMP(val,min,max)      do { if ((val) < (min)) { (val) = (min); } else if ((val) > (max)) { (val) = (max); } } while (0)

/* macro to compute the base 2 log for LOD calculations */
#define LOGB2(x)                (log((double)(x)) / log(2.0))



/*************************************
 *
 *  Macros for extracting bitfields
 *
 *************************************/

#define INITEN_ENABLE_HW_INIT(val)          (((val) >> 0) & 1)
#define INITEN_ENABLE_PCI_FIFO(val)         (((val) >> 1) & 1)
#define INITEN_REMAP_INIT_TO_DAC(val)       (((val) >> 2) & 1)
#define INITEN_ENABLE_SNOOP0(val)           (((val) >> 4) & 1)
#define INITEN_SNOOP0_MEMORY_MATCH(val)     (((val) >> 5) & 1)
#define INITEN_SNOOP0_READWRITE_MATCH(val)  (((val) >> 6) & 1)
#define INITEN_ENABLE_SNOOP1(val)           (((val) >> 7) & 1)
#define INITEN_SNOOP1_MEMORY_MATCH(val)     (((val) >> 8) & 1)
#define INITEN_SNOOP1_READWRITE_MATCH(val)  (((val) >> 9) & 1)
#define INITEN_SLI_BUS_OWNER(val)           (((val) >> 10) & 1)
#define INITEN_SLI_ODD_EVEN(val)            (((val) >> 11) & 1)
#define INITEN_SECONDARY_REV_ID(val)        (((val) >> 12) & 0xf)   /* voodoo 2 only */
#define INITEN_MFCTR_FAB_ID(val)            (((val) >> 16) & 0xf)   /* voodoo 2 only */
#define INITEN_ENABLE_PCI_INTERRUPT(val)    (((val) >> 20) & 1)     /* voodoo 2 only */
#define INITEN_PCI_INTERRUPT_TIMEOUT(val)   (((val) >> 21) & 1)     /* voodoo 2 only */
#define INITEN_ENABLE_NAND_TREE_TEST(val)   (((val) >> 22) & 1)     /* voodoo 2 only */
#define INITEN_ENABLE_SLI_ADDRESS_SNOOP(val) (((val) >> 23) & 1)    /* voodoo 2 only */
#define INITEN_SLI_SNOOP_ADDRESS(val)       (((val) >> 24) & 0xff)  /* voodoo 2 only */

#define FBZCP_CC_RGBSELECT(val)             (((val) >> 0) & 3)
#define FBZCP_CC_ASELECT(val)               (((val) >> 2) & 3)
#define FBZCP_CC_LOCALSELECT(val)           (((val) >> 4) & 1)
#define FBZCP_CCA_LOCALSELECT(val)          (((val) >> 5) & 3)
#define FBZCP_CC_LOCALSELECT_OVERRIDE(val)  (((val) >> 7) & 1)
#define FBZCP_CC_ZERO_OTHER(val)            (((val) >> 8) & 1)
#define FBZCP_CC_SUB_CLOCAL(val)            (((val) >> 9) & 1)
#define FBZCP_CC_MSELECT(val)               (((val) >> 10) & 7)
#define FBZCP_CC_REVERSE_BLEND(val)         (((val) >> 13) & 1)
#define FBZCP_CC_ADD_ACLOCAL(val)           (((val) >> 14) & 3)
#define FBZCP_CC_INVERT_OUTPUT(val)         (((val) >> 16) & 1)
#define FBZCP_CCA_ZERO_OTHER(val)           (((val) >> 17) & 1)
#define FBZCP_CCA_SUB_CLOCAL(val)           (((val) >> 18) & 1)
#define FBZCP_CCA_MSELECT(val)              (((val) >> 19) & 7)
#define FBZCP_CCA_REVERSE_BLEND(val)        (((val) >> 22) & 1)
#define FBZCP_CCA_ADD_ACLOCAL(val)          (((val) >> 23) & 3)
#define FBZCP_CCA_INVERT_OUTPUT(val)        (((val) >> 25) & 1)
#define FBZCP_CCA_SUBPIXEL_ADJUST(val)      (((val) >> 26) & 1)
#define FBZCP_TEXTURE_ENABLE(val)           (((val) >> 27) & 1)
#define FBZCP_RGBZW_CLAMP(val)              (((val) >> 28) & 1)     /* voodoo 2 only */
#define FBZCP_ANTI_ALIAS(val)               (((val) >> 29) & 1)     /* voodoo 2 only */

#define ALPHAMODE_ALPHATEST(val)            (((val) >> 0) & 1)
#define ALPHAMODE_ALPHAFUNCTION(val)        (((val) >> 1) & 7)
#define ALPHAMODE_ALPHABLEND(val)           (((val) >> 4) & 1)
#define ALPHAMODE_ANTIALIAS(val)            (((val) >> 5) & 1)
#define ALPHAMODE_SRCRGBBLEND(val)          (((val) >> 8) & 15)
#define ALPHAMODE_DSTRGBBLEND(val)          (((val) >> 12) & 15)
#define ALPHAMODE_SRCALPHABLEND(val)        (((val) >> 16) & 15)
#define ALPHAMODE_DSTALPHABLEND(val)        (((val) >> 20) & 15)
#define ALPHAMODE_ALPHAREF(val)             (((val) >> 24) & 0xff)

#define FOGMODE_ENABLE_FOG(val)             (((val) >> 0) & 1)
#define FOGMODE_FOG_ADD(val)                (((val) >> 1) & 1)
#define FOGMODE_FOG_MULT(val)               (((val) >> 2) & 1)
#define FOGMODE_FOG_ZALPHA(val)             (((val) >> 3) & 3)
#define FOGMODE_FOG_CONSTANT(val)           (((val) >> 5) & 1)
#define FOGMODE_FOG_DITHER(val)             (((val) >> 6) & 1)      /* voodoo 2 only */
#define FOGMODE_FOG_ZONES(val)              (((val) >> 7) & 1)      /* voodoo 2 only */

#define FBZMODE_ENABLE_CLIPPING(val)        (((val) >> 0) & 1)
#define FBZMODE_ENABLE_CHROMAKEY(val)       (((val) >> 1) & 1)
#define FBZMODE_ENABLE_STIPPLE(val)         (((val) >> 2) & 1)
#define FBZMODE_WBUFFER_SELECT(val)         (((val) >> 3) & 1)
#define FBZMODE_ENABLE_DEPTHBUF(val)        (((val) >> 4) & 1)
#define FBZMODE_DEPTH_FUNCTION(val)         (((val) >> 5) & 7)
#define FBZMODE_ENABLE_DITHERING(val)       (((val) >> 8) & 1)
#define FBZMODE_RGB_BUFFER_MASK(val)        (((val) >> 9) & 1)
#define FBZMODE_AUX_BUFFER_MASK(val)        (((val) >> 10) & 1)
#define FBZMODE_DITHER_TYPE(val)            (((val) >> 11) & 1)
#define FBZMODE_STIPPLE_PATTERN(val)        (((val) >> 12) & 1)
#define FBZMODE_ENABLE_ALPHA_MASK(val)      (((val) >> 13) & 1)
#define FBZMODE_DRAW_BUFFER(val)            (((val) >> 14) & 3)
#define FBZMODE_ENABLE_DEPTH_BIAS(val)      (((val) >> 16) & 1)
#define FBZMODE_Y_ORIGIN(val)               (((val) >> 17) & 1)
#define FBZMODE_ENABLE_ALPHA_PLANES(val)    (((val) >> 18) & 1)
#define FBZMODE_ALPHA_DITHER_SUBTRACT(val)  (((val) >> 19) & 1)
#define FBZMODE_DEPTH_SOURCE_COMPARE(val)   (((val) >> 20) & 1)
#define FBZMODE_DEPTH_FLOAT_SELECT(val)     (((val) >> 21) & 1)     /* voodoo 2 only */

#define LFBMODE_WRITE_FORMAT(val)           (((val) >> 0) & 0xf)
#define LFBMODE_WRITE_BUFFER_SELECT(val)    (((val) >> 4) & 3)
#define LFBMODE_READ_BUFFER_SELECT(val)     (((val) >> 6) & 3)
#define LFBMODE_ENABLE_PIXEL_PIPELINE(val)  (((val) >> 8) & 1)
#define LFBMODE_RGBA_LANES(val)             (((val) >> 9) & 3)
#define LFBMODE_WORD_SWAP_WRITES(val)       (((val) >> 11) & 1)
#define LFBMODE_BYTE_SWIZZLE_WRITES(val)    (((val) >> 12) & 1)
#define LFBMODE_Y_ORIGIN(val)               (((val) >> 13) & 1)
#define LFBMODE_WRITE_W_SELECT(val)         (((val) >> 14) & 1)
#define LFBMODE_WORD_SWAP_READS(val)        (((val) >> 15) & 1)
#define LFBMODE_BYTE_SWIZZLE_READS(val)     (((val) >> 16) & 1)

#define CHROMARANGE_BLUE_EXCLUSIVE(val)     (((val) >> 24) & 1)
#define CHROMARANGE_GREEN_EXCLUSIVE(val)    (((val) >> 25) & 1)
#define CHROMARANGE_RED_EXCLUSIVE(val)      (((val) >> 26) & 1)
#define CHROMARANGE_UNION_MODE(val)         (((val) >> 27) & 1)
#define CHROMARANGE_ENABLE(val)             (((val) >> 28) & 1)

#define FBIINIT0_VGA_PASSTHRU(val)          (((val) >> 0) & 1)
#define FBIINIT0_GRAPHICS_RESET(val)        (((val) >> 1) & 1)
#define FBIINIT0_FIFO_RESET(val)            (((val) >> 2) & 1)
#define FBIINIT0_SWIZZLE_REG_WRITES(val)    (((val) >> 3) & 1)
#define FBIINIT0_STALL_PCIE_FOR_HWM(val)    (((val) >> 4) & 1)
#define FBIINIT0_PCI_FIFO_LWM(val)          (((val) >> 6) & 0x1f)
#define FBIINIT0_LFB_TO_MEMORY_FIFO(val)    (((val) >> 11) & 1)
#define FBIINIT0_TEXMEM_TO_MEMORY_FIFO(val) (((val) >> 12) & 1)
#define FBIINIT0_ENABLE_MEMORY_FIFO(val)    (((val) >> 13) & 1)
#define FBIINIT0_MEMORY_FIFO_HWM(val)       (((val) >> 14) & 0x7ff)
#define FBIINIT0_MEMORY_FIFO_BURST(val)     (((val) >> 25) & 0x3f)

#define FBIINIT1_PCI_DEV_FUNCTION(val)      (((val) >> 0) & 1)
#define FBIINIT1_PCI_WRITE_WAIT_STATES(val) (((val) >> 1) & 1)
#define FBIINIT1_MULTI_SST1(val)            (((val) >> 2) & 1)      /* not on voodoo 2 */
#define FBIINIT1_ENABLE_LFB(val)            (((val) >> 3) & 1)
#define FBIINIT1_X_VIDEO_TILES(val)         (((val) >> 4) & 0xf)
#define FBIINIT1_VIDEO_TIMING_RESET(val)    (((val) >> 8) & 1)
#define FBIINIT1_SOFTWARE_OVERRIDE(val)     (((val) >> 9) & 1)
#define FBIINIT1_SOFTWARE_HSYNC(val)        (((val) >> 10) & 1)
#define FBIINIT1_SOFTWARE_VSYNC(val)        (((val) >> 11) & 1)
#define FBIINIT1_SOFTWARE_BLANK(val)        (((val) >> 12) & 1)
#define FBIINIT1_DRIVE_VIDEO_TIMING(val)    (((val) >> 13) & 1)
#define FBIINIT1_DRIVE_VIDEO_BLANK(val)     (((val) >> 14) & 1)
#define FBIINIT1_DRIVE_VIDEO_SYNC(val)      (((val) >> 15) & 1)
#define FBIINIT1_DRIVE_VIDEO_DCLK(val)      (((val) >> 16) & 1)
#define FBIINIT1_VIDEO_TIMING_VCLK(val)     (((val) >> 17) & 1)
#define FBIINIT1_VIDEO_CLK_2X_DELAY(val)    (((val) >> 18) & 3)
#define FBIINIT1_VIDEO_TIMING_SOURCE(val)   (((val) >> 20) & 3)
#define FBIINIT1_ENABLE_24BPP_OUTPUT(val)   (((val) >> 22) & 1)
#define FBIINIT1_ENABLE_SLI(val)            (((val) >> 23) & 1)
#define FBIINIT1_X_VIDEO_TILES_BIT5(val)    (((val) >> 24) & 1)     /* voodoo 2 only */
#define FBIINIT1_ENABLE_EDGE_FILTER(val)    (((val) >> 25) & 1)
#define FBIINIT1_INVERT_VID_CLK_2X(val)     (((val) >> 26) & 1)
#define FBIINIT1_VID_CLK_2X_SEL_DELAY(val)  (((val) >> 27) & 3)
#define FBIINIT1_VID_CLK_DELAY(val)         (((val) >> 29) & 3)
#define FBIINIT1_DISABLE_FAST_READAHEAD(val) (((val) >> 31) & 1)

#define FBIINIT2_DISABLE_DITHER_SUB(val)    (((val) >> 0) & 1)
#define FBIINIT2_DRAM_BANKING(val)          (((val) >> 1) & 1)
#define FBIINIT2_ENABLE_TRIPLE_BUF(val)     (((val) >> 4) & 1)
#define FBIINIT2_ENABLE_FAST_RAS_READ(val)  (((val) >> 5) & 1)
#define FBIINIT2_ENABLE_GEN_DRAM_OE(val)    (((val) >> 6) & 1)
#define FBIINIT2_ENABLE_FAST_READWRITE(val) (((val) >> 7) & 1)
#define FBIINIT2_ENABLE_PASSTHRU_DITHER(val) (((val) >> 8) & 1)
#define FBIINIT2_SWAP_BUFFER_ALGORITHM(val) (((val) >> 9) & 3)
#define FBIINIT2_VIDEO_BUFFER_OFFSET(val)   (((val) >> 11) & 0x1ff)
#define FBIINIT2_ENABLE_DRAM_BANKING(val)   (((val) >> 20) & 1)
#define FBIINIT2_ENABLE_DRAM_READ_FIFO(val) (((val) >> 21) & 1)
#define FBIINIT2_ENABLE_DRAM_REFRESH(val)   (((val) >> 22) & 1)
#define FBIINIT2_REFRESH_LOAD_VALUE(val)    (((val) >> 23) & 0x1ff)

#define FBIINIT3_TRI_REGISTER_REMAP(val)    (((val) >> 0) & 1)
#define FBIINIT3_VIDEO_FIFO_THRESH(val)     (((val) >> 1) & 0x1f)
#define FBIINIT3_DISABLE_TMUS(val)          (((val) >> 6) & 1)
#define FBIINIT3_FBI_MEMORY_TYPE(val)       (((val) >> 8) & 7)
#define FBIINIT3_VGA_PASS_RESET_VAL(val)    (((val) >> 11) & 1)
#define FBIINIT3_HARDCODE_PCI_BASE(val)     (((val) >> 12) & 1)
#define FBIINIT3_FBI2TREX_DELAY(val)        (((val) >> 13) & 0xf)
#define FBIINIT3_TREX2FBI_DELAY(val)        (((val) >> 17) & 0x1f)
#define FBIINIT3_YORIGIN_SUBTRACT(val)      (((val) >> 22) & 0x3ff)

#define FBIINIT4_PCI_READ_WAITS(val)        (((val) >> 0) & 1)
#define FBIINIT4_ENABLE_LFB_READAHEAD(val)  (((val) >> 1) & 1)
#define FBIINIT4_MEMORY_FIFO_LWM(val)       (((val) >> 2) & 0x3f)
#define FBIINIT4_MEMORY_FIFO_START_ROW(val) (((val) >> 8) & 0x3ff)
#define FBIINIT4_MEMORY_FIFO_STOP_ROW(val)  (((val) >> 18) & 0x3ff)
#define FBIINIT4_VIDEO_CLOCKING_DELAY(val)  (((val) >> 29) & 7)     /* voodoo 2 only */

#define FBIINIT5_DISABLE_PCI_STOP(val)      (((val) >> 0) & 1)      /* voodoo 2 only */
#define FBIINIT5_PCI_SLAVE_SPEED(val)       (((val) >> 1) & 1)      /* voodoo 2 only */
#define FBIINIT5_DAC_DATA_OUTPUT_WIDTH(val) (((val) >> 2) & 1)      /* voodoo 2 only */
#define FBIINIT5_DAC_DATA_17_OUTPUT(val)    (((val) >> 3) & 1)      /* voodoo 2 only */
#define FBIINIT5_DAC_DATA_18_OUTPUT(val)    (((val) >> 4) & 1)      /* voodoo 2 only */
#define FBIINIT5_GENERIC_STRAPPING(val)     (((val) >> 5) & 0xf)    /* voodoo 2 only */
#define FBIINIT5_BUFFER_ALLOCATION(val)     (((val) >> 9) & 3)      /* voodoo 2 only */
#define FBIINIT5_DRIVE_VID_CLK_SLAVE(val)   (((val) >> 11) & 1)     /* voodoo 2 only */
#define FBIINIT5_DRIVE_DAC_DATA_16(val)     (((val) >> 12) & 1)     /* voodoo 2 only */
#define FBIINIT5_VCLK_INPUT_SELECT(val)     (((val) >> 13) & 1)     /* voodoo 2 only */
#define FBIINIT5_MULTI_CVG_DETECT(val)      (((val) >> 14) & 1)     /* voodoo 2 only */
#define FBIINIT5_SYNC_RETRACE_READS(val)    (((val) >> 15) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_RHBORDER_COLOR(val) (((val) >> 16) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_LHBORDER_COLOR(val) (((val) >> 17) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_BVBORDER_COLOR(val) (((val) >> 18) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_TVBORDER_COLOR(val) (((val) >> 19) & 1)     /* voodoo 2 only */
#define FBIINIT5_DOUBLE_HORIZ(val)          (((val) >> 20) & 1)     /* voodoo 2 only */
#define FBIINIT5_DOUBLE_VERT(val)           (((val) >> 21) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_16BIT_GAMMA(val)    (((val) >> 22) & 1)     /* voodoo 2 only */
#define FBIINIT5_INVERT_DAC_HSYNC(val)      (((val) >> 23) & 1)     /* voodoo 2 only */
#define FBIINIT5_INVERT_DAC_VSYNC(val)      (((val) >> 24) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_24BIT_DACDATA(val)  (((val) >> 25) & 1)     /* voodoo 2 only */
#define FBIINIT5_ENABLE_INTERLACING(val)    (((val) >> 26) & 1)     /* voodoo 2 only */
#define FBIINIT5_DAC_DATA_18_CONTROL(val)   (((val) >> 27) & 1)     /* voodoo 2 only */
#define FBIINIT5_RASTERIZER_UNIT_MODE(val)  (((val) >> 30) & 3)     /* voodoo 2 only */

#define FBIINIT6_WINDOW_ACTIVE_COUNTER(val) (((val) >> 0) & 7)      /* voodoo 2 only */
#define FBIINIT6_WINDOW_DRAG_COUNTER(val)   (((val) >> 3) & 0x1f)   /* voodoo 2 only */
#define FBIINIT6_SLI_SYNC_MASTER(val)       (((val) >> 8) & 1)      /* voodoo 2 only */
#define FBIINIT6_DAC_DATA_22_OUTPUT(val)    (((val) >> 9) & 3)      /* voodoo 2 only */
#define FBIINIT6_DAC_DATA_23_OUTPUT(val)    (((val) >> 11) & 3)     /* voodoo 2 only */
#define FBIINIT6_SLI_SYNCIN_OUTPUT(val)     (((val) >> 13) & 3)     /* voodoo 2 only */
#define FBIINIT6_SLI_SYNCOUT_OUTPUT(val)    (((val) >> 15) & 3)     /* voodoo 2 only */
#define FBIINIT6_DAC_RD_OUTPUT(val)         (((val) >> 17) & 3)     /* voodoo 2 only */
#define FBIINIT6_DAC_WR_OUTPUT(val)         (((val) >> 19) & 3)     /* voodoo 2 only */
#define FBIINIT6_PCI_FIFO_LWM_RDY(val)      (((val) >> 21) & 0x7f)  /* voodoo 2 only */
#define FBIINIT6_VGA_PASS_N_OUTPUT(val)     (((val) >> 28) & 3)     /* voodoo 2 only */
#define FBIINIT6_X_VIDEO_TILES_BIT0(val)    (((val) >> 30) & 1)     /* voodoo 2 only */

#define FBIINIT7_GENERIC_STRAPPING(val)     (((val) >> 0) & 0xff)   /* voodoo 2 only */
#define FBIINIT7_CMDFIFO_ENABLE(val)        (((val) >> 8) & 1)      /* voodoo 2 only */
#define FBIINIT7_CMDFIFO_MEMORY_STORE(val)  (((val) >> 9) & 1)      /* voodoo 2 only */
#define FBIINIT7_DISABLE_CMDFIFO_HOLES(val) (((val) >> 10) & 1)     /* voodoo 2 only */
#define FBIINIT7_CMDFIFO_READ_THRESH(val)   (((val) >> 11) & 0x1f)  /* voodoo 2 only */
#define FBIINIT7_SYNC_CMDFIFO_WRITES(val)   (((val) >> 16) & 1)     /* voodoo 2 only */
#define FBIINIT7_SYNC_CMDFIFO_READS(val)    (((val) >> 17) & 1)     /* voodoo 2 only */
#define FBIINIT7_RESET_PCI_PACKER(val)      (((val) >> 18) & 1)     /* voodoo 2 only */
#define FBIINIT7_ENABLE_CHROMA_STUFF(val)   (((val) >> 19) & 1)     /* voodoo 2 only */
#define FBIINIT7_CMDFIFO_PCI_TIMEOUT(val)   (((val) >> 20) & 0x7f)  /* voodoo 2 only */
#define FBIINIT7_ENABLE_TEXTURE_BURST(val)  (((val) >> 27) & 1)     /* voodoo 2 only */

#define TEXMODE_ENABLE_PERSPECTIVE(val)     (((val) >> 0) & 1)
#define TEXMODE_MINIFICATION_FILTER(val)    (((val) >> 1) & 1)
#define TEXMODE_MAGNIFICATION_FILTER(val)   (((val) >> 2) & 1)
#define TEXMODE_CLAMP_NEG_W(val)            (((val) >> 3) & 1)
#define TEXMODE_ENABLE_LOD_DITHER(val)      (((val) >> 4) & 1)
#define TEXMODE_NCC_TABLE_SELECT(val)       (((val) >> 5) & 1)
#define TEXMODE_CLAMP_S(val)                (((val) >> 6) & 1)
#define TEXMODE_CLAMP_T(val)                (((val) >> 7) & 1)
#define TEXMODE_FORMAT(val)                 (((val) >> 8) & 0xf)
#define TEXMODE_TC_ZERO_OTHER(val)          (((val) >> 12) & 1)
#define TEXMODE_TC_SUB_CLOCAL(val)          (((val) >> 13) & 1)
#define TEXMODE_TC_MSELECT(val)             (((val) >> 14) & 7)
#define TEXMODE_TC_REVERSE_BLEND(val)       (((val) >> 17) & 1)
#define TEXMODE_TC_ADD_ACLOCAL(val)         (((val) >> 18) & 3)
#define TEXMODE_TC_INVERT_OUTPUT(val)       (((val) >> 20) & 1)
#define TEXMODE_TCA_ZERO_OTHER(val)         (((val) >> 21) & 1)
#define TEXMODE_TCA_SUB_CLOCAL(val)         (((val) >> 22) & 1)
#define TEXMODE_TCA_MSELECT(val)            (((val) >> 23) & 7)
#define TEXMODE_TCA_REVERSE_BLEND(val)      (((val) >> 26) & 1)
#define TEXMODE_TCA_ADD_ACLOCAL(val)        (((val) >> 27) & 3)
#define TEXMODE_TCA_INVERT_OUTPUT(val)      (((val) >> 29) & 1)
#define TEXMODE_TRILINEAR(val)              (((val) >> 30) & 1)
#define TEXMODE_SEQ_8_DOWNLD(val)           (((val) >> 31) & 1)

#define TEXLOD_LODMIN(val)                  (((val) >> 0) & 0x3f)
#define TEXLOD_LODMAX(val)                  (((val) >> 6) & 0x3f)
#define TEXLOD_LODBIAS(val)                 (((val) >> 12) & 0x3f)
#define TEXLOD_LOD_ODD(val)                 (((val) >> 18) & 1)
#define TEXLOD_LOD_TSPLIT(val)              (((val) >> 19) & 1)
#define TEXLOD_LOD_S_IS_WIDER(val)          (((val) >> 20) & 1)
#define TEXLOD_LOD_ASPECT(val)              (((val) >> 21) & 3)
#define TEXLOD_LOD_ZEROFRAC(val)            (((val) >> 23) & 1)
#define TEXLOD_TMULTIBASEADDR(val)          (((val) >> 24) & 1)
#define TEXLOD_TDATA_SWIZZLE(val)           (((val) >> 25) & 1)
#define TEXLOD_TDATA_SWAP(val)              (((val) >> 26) & 1)
#define TEXLOD_TDIRECT_WRITE(val)           (((val) >> 27) & 1)     /* Voodoo 2 only */

#define TEXDETAIL_DETAIL_MAX(val)           (((val) >> 0) & 0xff)
#define TEXDETAIL_DETAIL_BIAS(val)          (((val) >> 8) & 0x3f)
#define TEXDETAIL_DETAIL_SCALE(val)         (((val) >> 14) & 7)
#define TEXDETAIL_RGB_MIN_FILTER(val)       (((val) >> 17) & 1)     /* Voodoo 2 only */
#define TEXDETAIL_RGB_MAG_FILTER(val)       (((val) >> 18) & 1)     /* Voodoo 2 only */
#define TEXDETAIL_ALPHA_MIN_FILTER(val)     (((val) >> 19) & 1)     /* Voodoo 2 only */
#define TEXDETAIL_ALPHA_MAG_FILTER(val)     (((val) >> 20) & 1)     /* Voodoo 2 only */
#define TEXDETAIL_SEPARATE_RGBA_FILTER(val) (((val) >> 21) & 1)     /* Voodoo 2 only */

#define TREXINIT_SEND_TMU_CONFIG(val)       (((val) >> 18) & 1)



struct rgba
{
#ifdef LSB_FIRST
	uint8_t               b, g, r, a;
#else
	uint8_t               a, r, g, b;
#endif
};


union voodoo_reg
{
	int32_t               i;
	uint32_t              u;
	float               f;
	rgba                rgb;
};






/***************************************************************************
    CONSTANTS
***************************************************************************/
/* enumeration specifying which model of Voodoo we are emulating */
enum
{
	TYPE_VOODOO_1,
	TYPE_VOODOO_2,
	TYPE_VOODOO_BANSHEE,
	TYPE_VOODOO_3
};

#define STD_VOODOO_1_CLOCK          50000000
#define STD_VOODOO_2_CLOCK          90000000
#define STD_VOODOO_BANSHEE_CLOCK    90000000
#define STD_VOODOO_3_CLOCK          132000000


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- device interface ----- */

class voodoo_device : public device_t
{
public:
	~voodoo_device();

	void set_fbmem(int value) { m_fbmem = value; }
	void set_tmumem(int value1, int value2) { m_tmumem0 = value1; m_tmumem1 = value2; }
	template <typename T> void set_screen_tag(T &&tag) { m_screen_finder.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_cpu_tag(T &&tag) { m_cpu_finder.set_tag(std::forward<T>(tag)); }
	auto vblank_callback() { return m_vblank.bind(); }
	auto stall_callback() { return m_stall.bind(); }
	auto pciint_callback() { return m_pciint.bind(); }

	void set_screen(screen_device &screen) { assert(!m_screen); m_screen = &screen; }
	void set_cpu(cpu_device &cpu) { assert(!m_cpu); m_cpu = &cpu; }

	u32 voodoo_r(offs_t offset);
	void voodoo_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	uint8_t             m_fbmem;
	uint8_t             m_tmumem0;
	uint8_t             m_tmumem1;
	devcb_write_line    m_vblank;
	devcb_write_line    m_stall;
	// This is for internally generated PCI interrupts in Voodoo3
	devcb_write_line    m_pciint;

	TIMER_CALLBACK_MEMBER( vblank_off_callback );
	TIMER_CALLBACK_MEMBER( stall_cpu_callback );
	TIMER_CALLBACK_MEMBER( vblank_callback );

	void voodoo_postload();

	int voodoo_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int voodoo_get_type();
	int voodoo_is_stalled();
	void voodoo_set_init_enable(uint32_t newval);

protected:
	voodoo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t vdt);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	struct tmu_shared_state;

	struct voodoo_stats
	{
		voodoo_stats()
		{
			std::fill(std::begin(texture_mode), std::end(texture_mode), 0);
			buffer[0] = 0;
		}

		uint8_t             lastkey = 0;            // last key state
		uint8_t             display = 0;            // display stats?
		int32_t             swaps = 0;              // total swaps
		int32_t             stalls = 0;             // total stalls
		int32_t             total_triangles = 0;    // total triangles
		int32_t             total_pixels_in = 0;    // total pixels in
		int32_t             total_pixels_out = 0;   // total pixels out
		int32_t             total_chroma_fail = 0;  // total chroma fail
		int32_t             total_zfunc_fail = 0;   // total z func fail
		int32_t             total_afunc_fail = 0;   // total a func fail
		int32_t             total_clipped = 0;      // total clipped
		int32_t             total_stippled = 0;     // total stippled
		int32_t             lfb_writes = 0;         // LFB writes
		int32_t             lfb_reads = 0;          // LFB reads
		int32_t             reg_writes = 0;         // register writes
		int32_t             reg_reads = 0;          // register reads
		int32_t             tex_writes = 0;         // texture writes
		int32_t             texture_mode[16];       // 16 different texture modes
		uint8_t             render_override = 0;    // render override
		char                buffer[1024];           // string
	};


	/* note that this structure is an even 64 bytes long */
	struct stats_block
	{
		int32_t             pixels_in = 0;          // pixels in statistic
		int32_t             pixels_out = 0;         // pixels out statistic
		int32_t             chroma_fail = 0;        // chroma test fail statistic
		int32_t             zfunc_fail = 0;         // z function test fail statistic
		int32_t             afunc_fail = 0;         // alpha function test fail statistic
		int32_t             clip_fail = 0;          // clipping fail statistic
		int32_t             stipple_count = 0;      // stipple statistic
		int32_t             filler[64/4 - 7];       // pad this structure to 64 bytes
	};


	struct fifo_state
	{
		void reset() { in = out = 0; }
		void add(uint32_t data);
		uint32_t remove();
		uint32_t peek() { return base[out]; }
		bool empty() const { return in == out; }
		bool full() const { return ((in + 1) == out) || ((in == (size - 1)) && (out == 0)); }
		int32_t items() const;
		int32_t space() const { return size - 1 - items(); }

		uint32_t *          base = nullptr;         // base of the FIFO
		int32_t             size = 0;               // size of the FIFO
		int32_t             in = 0;                 // input pointer
		int32_t             out = 0;                // output pointer
	};


	struct cmdfifo_info
	{
		uint8_t             enable = 0;             // enabled?
		uint8_t             count_holes = 0;        // count holes?
		uint32_t            base = 0;               // base address in framebuffer RAM
		uint32_t            end = 0;                // end address in framebuffer RAM
		uint32_t            rdptr = 0;              // current read pointer
		uint32_t            amin = 0;               // minimum address
		uint32_t            amax = 0;               // maximum address
		uint32_t            depth = 0;              // current depth
		uint32_t            holes = 0;              // number of holes
	};


	struct pci_state
	{
		fifo_state          fifo;                   // PCI FIFO
		uint32_t            init_enable = 0;        // initEnable value
		uint8_t             stall_state = 0;        // state of the system if we're stalled
		uint8_t             op_pending = 0;         // true if an operation is pending
		attotime            op_end_time = attotime::zero; // time when the pending operation ends
		emu_timer *         continue_timer = nullptr; // timer to use to continue processing
		uint32_t            fifo_mem[64*2];         // memory backing the PCI FIFO
	};


	struct tmu_state
	{
		class stw_t;
		void recompute_texture_params();
		void init(uint8_t vdt, tmu_shared_state &share, voodoo_reg *r, void *memory, int tmem);
		int32_t prepare();
		static int32_t new_log2(double &value, const int &offset);
		rgbaint_t genTexture(int32_t x, const uint8_t *dither4, const uint32_t TEXMODE, rgb_t *LOOKUP, int32_t LODBASE, const stw_t &iterstw, int32_t &lod);
		rgbaint_t combineTexture(const uint32_t TEXMODE, const rgbaint_t& c_local, const rgbaint_t& c_other, int32_t lod);

		struct ncc_table
		{
			void write(offs_t regnum, uint32_t data);
			void update();

			uint8_t             dirty = 0;              // is the texel lookup dirty?
			voodoo_reg *        reg = nullptr;          // pointer to our registers
			int32_t             ir[4], ig[4], ib[4];    // I values for R,G,B
			int32_t             qr[4], qg[4], qb[4];    // Q values for R,G,B
			int32_t             y[16];                  // Y values
			rgb_t *             palette = nullptr;      // pointer to associated RGB palette
			rgb_t *             palettea = nullptr;     // pointer to associated ARGB palette
			rgb_t               texel[256];             // texel lookup
		};

		uint8_t *           ram = nullptr;          // pointer to our RAM
		uint32_t            mask = 0;               // mask to apply to pointers
		voodoo_reg *        reg = nullptr;          // pointer to our register base
		uint32_t            regdirty = 0;           // true if the LOD/mode/base registers have changed

		uint32_t            texaddr_mask = 0;       // mask for texture address
		uint8_t             texaddr_shift = 0;      // shift for texture address

		int64_t             starts = 0, startt = 0; // starting S,T (14.18)
		int64_t             startw = 0;             // starting W (2.30)
		int64_t             dsdx = 0, dtdx = 0;     // delta S,T per X
		int64_t             dwdx = 0;               // delta W per X
		int64_t             dsdy = 0, dtdy = 0;     // delta S,T per Y
		int64_t             dwdy = 0;               // delta W per Y

		int32_t             lodmin = 0, lodmax = 0; // min, max LOD values
		int32_t             lodbias = 0;            // LOD bias
		uint32_t            lodmask = 0;            // mask of available LODs
		uint32_t            lodoffset[9];           // offset of texture base for each LOD
		int32_t             detailmax = 0;          // detail clamp
		int32_t             detailbias = 0;         // detail bias
		uint8_t             detailscale = 0;        // detail scale

		uint32_t            wmask = 0;              // mask for the current texture width
		uint32_t            hmask = 0;              // mask for the current texture height

		uint32_t            bilinear_mask = 0;      // mask for bilinear resolution (0xf0 for V1, 0xff for V2)

		ncc_table           ncc[2];                 // two NCC tables

		rgb_t *             lookup = nullptr;       // currently selected lookup
		rgb_t *             texel[16];              // texel lookups for each format

		rgb_t               palette[256];           // palette lookup table
		rgb_t               palettea[256];          // palette+alpha lookup table
	};


	struct tmu_shared_state
	{
		void init();

		rgb_t               rgb332[256];            // RGB 3-3-2 lookup table
		rgb_t               alpha8[256];            // alpha 8-bit lookup table
		rgb_t               int8[256];              // intensity 8-bit lookup table
		rgb_t               ai44[256];              // alpha, intensity 4-4 lookup table

		rgb_t*              rgb565;                 // RGB 5-6-5 lookup table
		rgb_t               argb1555[65536];        // ARGB 1-5-5-5 lookup table
		rgb_t               argb4444[65536];        // ARGB 4-4-4-4 lookup table
	};


	struct fbi_state
	{
		struct setup_vertex
		{
			float               x, y;                   // X, Y coordinates
			float               z, wb;                  // Z and broadcast W values
			float               r, g, b, a;             // A, R, G, B values
			float               s0, t0, w0;             // W, S, T for TMU 0
			float               s1, t1, w1;             // W, S, T for TMU 1
		};

		uint8_t *           ram = nullptr;          // pointer to frame buffer RAM
		uint32_t            mask = 0;               // mask to apply to pointers
		uint32_t            rgboffs[3] = { 0, 0, 0 }; // word offset to 3 RGB buffers
		uint32_t            auxoffs = 0;            // word offset to 1 aux buffer

		uint8_t             frontbuf = 0;           // front buffer index
		uint8_t             backbuf = 0;            // back buffer index
		uint8_t             swaps_pending = 0;      // number of pending swaps
		uint8_t             video_changed = 0;      // did the frontbuffer video change?

		uint32_t            yorigin = 0;            // Y origin subtract value
		uint32_t            lfb_base = 0;           // base of LFB in memory
		uint8_t             lfb_stride = 0;         // stride of LFB accesses in bits

		uint32_t            width = 0;              // width of current frame buffer
		uint32_t            height = 0;             // height of current frame buffer
		uint32_t            xoffs = 0;              // horizontal offset (back porch)
		uint32_t            yoffs = 0;              // vertical offset (back porch)
		uint32_t            vsyncstart = 0;         // vertical sync start scanline
		uint32_t            vsyncstop = 0;          // vertical sync stop
		uint32_t            rowpixels = 0;          // pixels per row
		uint32_t            tile_width = 0;         // width of video tiles
		uint32_t            tile_height = 0;        // height of video tiles
		uint32_t            x_tiles = 0;            // number of tiles in the X direction

		emu_timer *         vsync_stop_timer = nullptr; // VBLANK End timer
		emu_timer *         vsync_start_timer = nullptr; // VBLANK timer
		uint8_t             vblank = 0;             // VBLANK state
		uint8_t             vblank_count = 0;       // number of VBLANKs since last swap
		uint8_t             vblank_swap_pending = 0;// a swap is pending, waiting for a vblank
		uint8_t             vblank_swap = 0;        // swap when we hit this count
		uint8_t             vblank_dont_swap = 0;   // don't actually swap when we hit this point

		/* triangle setup info */
		uint8_t             cheating_allowed = 0;   // allow cheating?
		int32_t             sign;                   // triangle sign
		int16_t             ax, ay;                 // vertex A x,y (12.4)
		int16_t             bx, by;                 // vertex B x,y (12.4)
		int16_t             cx, cy;                 // vertex C x,y (12.4)
		int32_t             startr, startg, startb, starta; // starting R,G,B,A (12.12)
		int32_t             startz;                 // starting Z (20.12)
		int64_t             startw;                 // starting W (16.32)
		int32_t             drdx, dgdx, dbdx, dadx; // delta R,G,B,A per X
		int32_t             dzdx;                   // delta Z per X
		int64_t             dwdx;                   // delta W per X
		int32_t             drdy, dgdy, dbdy, dady; // delta R,G,B,A per Y
		int32_t             dzdy;                   // delta Z per Y
		int64_t             dwdy;                   // delta W per Y

		stats_block         lfb_stats;              // LFB-access statistics

		uint8_t             sverts = 0;             // number of vertices ready */
		setup_vertex        svert[3];               // 3 setup vertices */

		fifo_state          fifo;                   // framebuffer memory fifo */
		cmdfifo_info        cmdfifo[2];             // command FIFOs */

		uint8_t             fogblend[64];           // 64-entry fog table */
		uint8_t             fogdelta[64];           // 64-entry fog table */
		uint8_t             fogdelta_mask;          // mask for for delta (0xff for V1, 0xfc for V2) */

		rgb_t               pen[65536];             // mapping from pixels to pens */
		rgb_t               clut[512];              // clut gamma data */
		uint8_t             clut_dirty = 1;         // do we need to recompute? */
		rgb_t               rgb565[65536];          // RGB 5-6-5 lookup table */
	};


	struct dac_state
	{
		void data_w(uint8_t regum, uint8_t data);
		void data_r(uint8_t regnum);

		uint8_t             reg[8];                 // 8 registers
		uint8_t             read_result;            // pending read result
	};


	struct raster_info
	{
		uint32_t compute_hash() const;

		raster_info *       next = nullptr;         // pointer to next entry with the same hash
		poly_draw_scanline_func callback = nullptr; // callback pointer
		bool                is_generic = false;     // true if this is one of the generic rasterizers
		uint8_t             display;                // display index
		uint32_t            hits;                   // how many hits (pixels) we've used this for
		uint32_t            polys;                  // how many polys we've used this for
		uint32_t            eff_color_path;         // effective fbzColorPath value
		uint32_t            eff_alpha_mode;         // effective alphaMode value
		uint32_t            eff_fog_mode;           // effective fogMode value
		uint32_t            eff_fbz_mode;           // effective fbzMode value
		uint32_t            eff_tex_mode_0;         // effective textureMode value for TMU #0
		uint32_t            eff_tex_mode_1;         // effective textureMode value for TMU #1
		uint32_t            hash = 0U;
	};


	struct poly_extra_data;


	struct banshee_info
	{
		uint32_t            io[0x40];               // I/O registers
		uint32_t            agp[0x80];              // AGP registers
		uint8_t             vga[0x20];              // VGA registers
		uint8_t             crtc[0x27];             // VGA CRTC registers
		uint8_t             seq[0x05];              // VGA sequencer registers
		uint8_t             gc[0x05];               // VGA graphics controller registers
		uint8_t             att[0x15];              // VGA attribute registers
		uint8_t             attff;                  // VGA attribute flip-flop

		uint32_t            blt_regs[0x20];         // 2D Blitter registers
		uint32_t            blt_dst_base = 0;
		uint32_t            blt_dst_x = 0;
		uint32_t            blt_dst_y = 0;
		uint32_t            blt_dst_width = 0;
		uint32_t            blt_dst_height = 0;
		uint32_t            blt_dst_stride = 0;
		uint32_t            blt_dst_bpp = 0;
		uint32_t            blt_cmd = 0;
		uint32_t            blt_src_base = 0;
		uint32_t            blt_src_x = 0;
		uint32_t            blt_src_y = 0;
		uint32_t            blt_src_width = 0;
		uint32_t            blt_src_height = 0;
		uint32_t            blt_src_stride = 0;
		uint32_t            blt_src_bpp = 0;
	};


	static const raster_info predef_raster_table[];

	// not all of these need to be static, review.

	void check_stalled_cpu(attotime current_time);
	static void flush_fifos( voodoo_device* vd, attotime current_time);
	static void init_fbi(voodoo_device *vd, fbi_state *f, void *memory, int fbmem);
	static int32_t register_w(voodoo_device *vd, offs_t offset, uint32_t data);
	static int32_t swapbuffer(voodoo_device *vd, uint32_t data);
	static int32_t lfb_w(voodoo_device *vd, offs_t offset, uint32_t data, uint32_t mem_mask);
	static int32_t texture_w(voodoo_device *vd, offs_t offset, uint32_t data);
	int32_t lfb_direct_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	static int32_t banshee_2d_w(voodoo_device *vd, offs_t offset, uint32_t data);
	void stall_cpu(int state, attotime current_time);
	void soft_reset();
	void recompute_video_memory();
	void adjust_vblank_timer();
	static int32_t fastfill(voodoo_device *vd);
	static int32_t triangle(voodoo_device *vd);
	static int32_t begin_triangle(voodoo_device *vd);
	static int32_t draw_triangle(voodoo_device *vd);
	static int32_t setup_and_draw_triangle(voodoo_device *vd);
	static int32_t triangle_create_work_item(voodoo_device* vd,uint16_t *drawbuf, int texcount);
	static raster_info *add_rasterizer(voodoo_device *vd, const raster_info *cinfo);
	static raster_info *find_rasterizer(voodoo_device *vd, int texcount);
	static void dump_rasterizer_stats(voodoo_device *vd);

	void accumulate_statistics(const stats_block &block);
	void update_statistics(bool accumulate);
	void reset_counters();

	static uint32_t register_r(voodoo_device *vd, offs_t offset);

	static void swap_buffers(voodoo_device *vd);
	int cmdfifo_compute_expected_depth(cmdfifo_info &f);
	static uint32_t cmdfifo_execute(voodoo_device *vd, cmdfifo_info *f);
	int32_t cmdfifo_execute_if_ready(cmdfifo_info &f);
	static void cmdfifo_w(voodoo_device *vd, cmdfifo_info *f, offs_t offset, uint32_t data);

	static void init_save_state(voodoo_device *vd);

	static void raster_fastfill(void *dest, int32_t scanline, const poly_extent *extent, const void *extradata, int threadid);
	static void raster_generic_0tmu(void *dest, int32_t scanline, const poly_extent *extent, const void *extradata, int threadid);
	static void raster_generic_1tmu(void *dest, int32_t scanline, const poly_extent *extent, const void *extradata, int threadid);
	static void raster_generic_2tmu(void *dest, int32_t scanline, const poly_extent *extent, const void *extradata, int threadid);

#define RASTERIZER_HEADER(name) \
	static void raster_##name(void *destbase, int32_t y, const poly_extent *extent, const void *extradata, int threadid);
#define RASTERIZER_ENTRY(fbzcp, alpha, fog, fbz, tex0, tex1) \
	RASTERIZER_HEADER(fbzcp##_##alpha##_##fog##_##fbz##_##tex0##_##tex1)
#include "voodoo_rast.ipp"

#undef RASTERIZER_ENTRY

	static bool chromaKeyTest(voodoo_device *vd, stats_block *stats, uint32_t fbzModeReg, rgbaint_t rgaIntColor);
	static bool alphaMaskTest(stats_block *stats, uint32_t fbzModeReg, uint8_t alpha);
	static bool alphaTest(uint8_t alpharef, stats_block *stats, uint32_t alphaModeReg, uint8_t alpha);
	static bool depthTest(uint16_t zaColorReg, stats_block *stats, int32_t destDepth, uint32_t fbzModeReg, int32_t biasdepth);
	static bool combineColor(voodoo_device *vd, stats_block *STATS, uint32_t FBZCOLORPATH, uint32_t FBZMODE, rgbaint_t TEXELARGB, int32_t ITERZ, int64_t ITERW, rgbaint_t &srcColor);

// FIXME: this stuff should not be public
public:
	optional_device<screen_device> m_screen_finder; // the screen we are acting on
	optional_device<cpu_device> m_cpu_finder;   // the CPU we interact with

	std::unique_ptr<uint8_t[]> m_fbmem_alloc;
	std::unique_ptr<uint8_t[]> m_tmumem_alloc[2];

	uint8_t             index;                  // index of board
	screen_device *     m_screen;               // the screen we are acting on
	cpu_device *        m_cpu;                  // the CPU we interact with
	const uint8_t       vd_type;                // type of system
	uint8_t             chipmask;               // mask for which chips are available
	uint32_t            freq;                   // operating frequency
	attoseconds_t       attoseconds_per_cycle;  // attoseconds per cycle
	uint32_t            extra_cycles;           // extra cycles not yet accounted for
	int                 trigger;                // trigger used for stalling

	voodoo_reg          reg[0x400];             // raw registers
	const uint8_t *     regaccess;              // register access array
	const char *const * regnames;               // register names array
	uint8_t             alt_regmap;             // enable alternate register map?

	pci_state           pci;                    // PCI state
	dac_state           dac;                    // DAC state

	fbi_state           fbi;                    // FBI states
	tmu_state           tmu[MAX_TMU];           // TMU states
	tmu_shared_state    tmushare;               // TMU shared state
	banshee_info        banshee;                // Banshee state

	legacy_poly_manager_owner poly;              // polygon manager
	std::unique_ptr<stats_block[]> thread_stats; // per-thread statistics

	voodoo_stats        stats;                  // internal statistics

	offs_t              last_status_pc;         // PC of last status description (for logging)
	uint32_t            last_status_value;      // value of last status read (for logging)

	int                 next_rasterizer;        // next rasterizer index
	raster_info         rasterizer[MAX_RASTERIZERS]; // array of rasterizers
	raster_info *       raster_hash[RASTER_HASH_SIZE]; // hash table of rasterizers

	bool                send_config;
	uint32_t            tmu_config;
};

class voodoo_1_device : public voodoo_device
{
public:
	voodoo_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class voodoo_2_device : public voodoo_device
{
public:
	voodoo_2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class voodoo_banshee_device : public voodoo_device
{
public:
	voodoo_banshee_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u32 banshee_r(offs_t offset, u32 mem_mask = ~0);
	void banshee_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 banshee_fb_r(offs_t offset);
	void banshee_fb_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 banshee_io_r(offs_t offset, u32 mem_mask = ~0);
	void banshee_io_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 banshee_rom_r(offs_t offset);
	u8 banshee_vga_r(offs_t offset);
	void banshee_vga_w(offs_t offset, u8 data);

protected:
	voodoo_banshee_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t vdt);

	// device-level overrides
	u32 banshee_agp_r(offs_t offset);
	void banshee_agp_w(offs_t offset, u32 data, u32 mem_mask = ~0);
};


class voodoo_3_device : public voodoo_banshee_device
{
public:
	voodoo_3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(VOODOO_1,       voodoo_1_device)
DECLARE_DEVICE_TYPE(VOODOO_2,       voodoo_2_device)
DECLARE_DEVICE_TYPE(VOODOO_BANSHEE, voodoo_banshee_device)
DECLARE_DEVICE_TYPE(VOODOO_3,       voodoo_3_device)

// use SSE on 64-bit implementations, where it can be assumed
#if 1 && ((!defined(MAME_DEBUG) || defined(__OPTIMIZE__)) && (defined(__SSE2__) || defined(_MSC_VER)) && defined(PTR64))
#include <emmintrin.h>
#ifdef __SSE4_1__
#include <smmintrin.h>
#endif
class voodoo_device::tmu_state::stw_t
{
public:
	stw_t() { }
	stw_t(const stw_t& other) = default;
	stw_t &operator=(const stw_t& other) = default;

	void set(s64 s, s64 t, s64 w) { m_st = _mm_set_pd(s << 8, t << 8); m_w = _mm_set1_pd(w); }
	int is_w_neg() const { return _mm_comilt_sd(m_w, _mm_set1_pd(0.0)); }
	void get_st_shiftr(s32 &s, s32 &t, const s32 &shift) const
	{
		s64 tmpS = _mm_cvtsd_si64(_mm_shuffle_pd(m_st, _mm_setzero_pd(), 1));
		s = tmpS >> shift;
		s64 tmpT = _mm_cvtsd_si64(m_st);
		t = tmpT >> shift;
	}
	void add(const stw_t& other)
	{
		m_st = _mm_add_pd(m_st, other.m_st);
		m_w = _mm_add_pd(m_w, other.m_w);
	}
	void calc_stow(s32 &sow, s32 &tow, int32_t &oowlog) const
	{
		__m128d tmp = _mm_div_pd(m_st, m_w);
		// Allow for 8 bits of decimal in integer
		//tmp = _mm_mul_pd(tmp, _mm_set1_pd(256.0));
		__m128i tmp2 = _mm_cvttpd_epi32(tmp);
#ifdef __SSE4_1__
		sow = _mm_extract_epi32(tmp2, 1);
		tow = _mm_extract_epi32(tmp2, 0);
#else
		sow = _mm_cvtsi128_si32(_mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 0, 1)));
		tow = _mm_cvtsi128_si32(tmp2);
#endif
		double dW = _mm_cvtsd_f64(m_w);
		oowlog = -new_log2(dW, 0);
	}
private:
	__m128d m_st;
	__m128d m_w;
};
#else
class voodoo_device::tmu_state::stw_t
{
public:
	stw_t() {}
	stw_t(const stw_t& other) = default;
	stw_t &operator=(const stw_t& other) = default;

	void set(s64 s, s64 t, s64 w) { m_s = s; m_t = t; m_w = w; }
	int is_w_neg() const { return (m_w < 0) ? 1 : 0; }
	void get_st_shiftr(s32 &s, s32 &t, const s32 &shift) const
	{
		s = m_s >> shift;
		t = m_t >> shift;
	}
	inline void add(const stw_t& other)
	{
		m_s += other.m_s;
		m_t += other.m_t;
		m_w += other.m_w;
	}
	// Computes s/w and t/w and returns log2 of 1/w
	// s, t and c are 16.32 values.  The results are 24.8.
	inline void calc_stow(s32 &sow, s32 &tow, int32_t &oowlog) const
	{
		double recip = double(1ULL << (47 - 39)) / m_w;
		double resAD = m_s * recip;
		double resBD = m_t * recip;
		oowlog = new_log2(recip, 56);
		sow = resAD;
		tow = resBD;
	}
private:
	s64 m_s, m_t, m_w;
};
#endif

#endif // MAME_VIDEO_VOODOO_H
