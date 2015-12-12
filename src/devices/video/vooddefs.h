	// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    vooddefs.h

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/


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

// Use old macro style or newer SSE2 optimized functions
#define USE_OLD_RASTER  0

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
#define status          (0x000/4)   /* R  P  */
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
 *  Alias map of the first 64
 *  registers when remapped
 *
 *************************************/

static const UINT8 register_alias_map[0x40] =
{
	status,     0x004/4,    vertexAx,   vertexAy,
	vertexBx,   vertexBy,   vertexCx,   vertexCy,
	startR,     dRdX,       dRdY,       startG,
	dGdX,       dGdY,       startB,     dBdX,
	dBdY,       startZ,     dZdX,       dZdY,
	startA,     dAdX,       dAdY,       startS,
	dSdX,       dSdY,       startT,     dTdX,
	dTdY,       startW,     dWdX,       dWdY,

	triangleCMD,0x084/4,    fvertexAx,  fvertexAy,
	fvertexBx,  fvertexBy,  fvertexCx,  fvertexCy,
	fstartR,    fdRdX,      fdRdY,      fstartG,
	fdGdX,      fdGdY,      fstartB,    fdBdX,
	fdBdY,      fstartZ,    fdZdX,      fdZdY,
	fstartA,    fdAdX,      fdAdY,      fstartS,
	fdSdX,      fdSdY,      fstartT,    fdTdX,
	fdTdY,      fstartW,    fdWdX,      fdWdY
};



/*************************************
 *
 *  Table of per-register access rights
 *
 *************************************/

static const UINT8 voodoo_register_access[0x100] =
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


static const UINT8 voodoo2_register_access[0x100] =
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


static const UINT8 banshee_register_access[0x100] =
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

static const char *const voodoo_reg_name[] =
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


static const char *const banshee_reg_name[] =
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
 *  Register string table for debug
 *
 *************************************/

static const char *const banshee_io_reg_name[] =
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
 *  Register string table for debug
 *
 *************************************/

static const char *const banshee_agp_reg_name[] =
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



/*************************************
 *
 *  Dithering tables
 *
 *************************************/

static const UINT8 dither_matrix_4x4[16] =
{
		0,  8,  2, 10,
	12,  4, 14,  6,
		3, 11,  1,  9,
	15,  7, 13,  5
};

static const UINT8 dither_matrix_2x2[16] =
{
		2, 10,  2, 10,
	14,  6, 14,  6,
		2, 10,  2, 10,
	14,  6, 14,  6
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
	(a) = ((INT16)(val) >> 15) & 0xff;                      \
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


/*************************************
 *
 *  Core types
 *
 *************************************/

struct voodoo_state;
struct poly_extra_data;


struct rgba
{
#ifdef LSB_FIRST
	UINT8               b, g, r, a;
#else
	UINT8               a, r, g, b;
#endif
};


union voodoo_reg
{
	INT32               i;
	UINT32              u;
	float               f;
	rgba                rgb;
};


typedef voodoo_reg rgb_union;


struct voodoo_stats
{
	UINT8               lastkey;                /* last key state */
	UINT8               display;                /* display stats? */
	INT32               swaps;                  /* total swaps */
	INT32               stalls;                 /* total stalls */
	INT32               total_triangles;        /* total triangles */
	INT32               total_pixels_in;        /* total pixels in */
	INT32               total_pixels_out;       /* total pixels out */
	INT32               total_chroma_fail;      /* total chroma fail */
	INT32               total_zfunc_fail;       /* total z func fail */
	INT32               total_afunc_fail;       /* total a func fail */
	INT32               total_clipped;          /* total clipped */
	INT32               total_stippled;         /* total stippled */
	INT32               lfb_writes;             /* LFB writes */
	INT32               lfb_reads;              /* LFB reads */
	INT32               reg_writes;             /* register writes */
	INT32               reg_reads;              /* register reads */
	INT32               tex_writes;             /* texture writes */
	INT32               texture_mode[16];       /* 16 different texture modes */
	UINT8               render_override;        /* render override */
	char                buffer[1024];           /* string */
};


/* note that this structure is an even 64 bytes long */
struct stats_block
{
	INT32               pixels_in;              /* pixels in statistic */
	INT32               pixels_out;             /* pixels out statistic */
	INT32               chroma_fail;            /* chroma test fail statistic */
	INT32               zfunc_fail;             /* z function test fail statistic */
	INT32               afunc_fail;             /* alpha function test fail statistic */
	INT32               clip_fail;              /* clipping fail statistic */
	INT32               stipple_count;          /* stipple statistic */
	INT32               filler[64/4 - 7];       /* pad this structure to 64 bytes */
};


struct fifo_state
{
	UINT32 *            base;                   /* base of the FIFO */
	INT32               size;                   /* size of the FIFO */
	INT32               in;                     /* input pointer */
	INT32               out;                    /* output pointer */
};


struct cmdfifo_info
{
	UINT8               enable;                 /* enabled? */
	UINT8               count_holes;            /* count holes? */
	UINT32              base;                   /* base address in framebuffer RAM */
	UINT32              end;                    /* end address in framebuffer RAM */
	UINT32              rdptr;                  /* current read pointer */
	UINT32              amin;                   /* minimum address */
	UINT32              amax;                   /* maximum address */
	UINT32              depth;                  /* current depth */
	UINT32              holes;                  /* number of holes */
};


struct pci_state
{
	fifo_state          fifo;                   /* PCI FIFO */
	UINT32              init_enable;            /* initEnable value */
	UINT8               stall_state;            /* state of the system if we're stalled */
	UINT8               op_pending;             /* true if an operation is pending */
	attotime            op_end_time;            /* time when the pending operation ends */
	emu_timer *         continue_timer;         /* timer to use to continue processing */
	UINT32              fifo_mem[64*2];         /* memory backing the PCI FIFO */
};


struct ncc_table
{
	UINT8               dirty;                  /* is the texel lookup dirty? */
	voodoo_reg *        reg;                    /* pointer to our registers */
	INT32               ir[4], ig[4], ib[4];    /* I values for R,G,B */
	INT32               qr[4], qg[4], qb[4];    /* Q values for R,G,B */
	INT32               y[16];                  /* Y values */
	rgb_t *             palette;                /* pointer to associated RGB palette */
	rgb_t *             palettea;               /* pointer to associated ARGB palette */
	rgb_t               texel[256];             /* texel lookup */
};


struct tmu_state
{
	UINT8 *             ram;                    /* pointer to our RAM */
	UINT32              mask;                   /* mask to apply to pointers */
	voodoo_reg *        reg;                    /* pointer to our register base */
	UINT32              regdirty;               /* true if the LOD/mode/base registers have changed */

	UINT32              texaddr_mask;           /* mask for texture address */
	UINT8               texaddr_shift;          /* shift for texture address */

	INT64               starts, startt;         /* starting S,T (14.18) */
	INT64               startw;                 /* starting W (2.30) */
	INT64               dsdx, dtdx;             /* delta S,T per X */
	INT64               dwdx;                   /* delta W per X */
	INT64               dsdy, dtdy;             /* delta S,T per Y */
	INT64               dwdy;                   /* delta W per Y */

	INT32               lodmin, lodmax;         /* min, max LOD values */
	INT32               lodbias;                /* LOD bias */
	UINT32              lodmask;                /* mask of available LODs */
	UINT32              lodoffset[9];           /* offset of texture base for each LOD */
	INT32               detailmax;              /* detail clamp */
	INT32               detailbias;             /* detail bias */
	UINT8               detailscale;            /* detail scale */

	UINT32              wmask;                  /* mask for the current texture width */
	UINT32              hmask;                  /* mask for the current texture height */

	UINT32              bilinear_mask;          /* mask for bilinear resolution (0xf0 for V1, 0xff for V2) */

	ncc_table           ncc[2];                 /* two NCC tables */

	rgb_t *             lookup;                 /* currently selected lookup */
	rgb_t *             texel[16];              /* texel lookups for each format */

	rgb_t               palette[256];           /* palette lookup table */
	rgb_t               palettea[256];          /* palette+alpha lookup table */
};


struct tmu_shared_state
{
	rgb_t               rgb332[256];            /* RGB 3-3-2 lookup table */
	rgb_t               alpha8[256];            /* alpha 8-bit lookup table */
	rgb_t               int8[256];              /* intensity 8-bit lookup table */
	rgb_t               ai44[256];              /* alpha, intensity 4-4 lookup table */

	rgb_t               rgb565[65536];          /* RGB 5-6-5 lookup table */
	rgb_t               argb1555[65536];        /* ARGB 1-5-5-5 lookup table */
	rgb_t               argb4444[65536];        /* ARGB 4-4-4-4 lookup table */
};


struct setup_vertex
{
	float               x, y;                   /* X, Y coordinates */
	float               a, r, g, b;             /* A, R, G, B values */
	float               z, wb;                  /* Z and broadcast W values */
	float               w0, s0, t0;             /* W, S, T for TMU 0 */
	float               w1, s1, t1;             /* W, S, T for TMU 1 */
};


struct fbi_state
{
	UINT8 *             ram;                    /* pointer to frame buffer RAM */
	UINT32              mask;                   /* mask to apply to pointers */
	UINT32              rgboffs[3];             /* word offset to 3 RGB buffers */
	UINT32              auxoffs;                /* word offset to 1 aux buffer */

	UINT8               frontbuf;               /* front buffer index */
	UINT8               backbuf;                /* back buffer index */
	UINT8               swaps_pending;          /* number of pending swaps */
	UINT8               video_changed;          /* did the frontbuffer video change? */

	UINT32              yorigin;                /* Y origin subtract value */
	UINT32              lfb_base;               /* base of LFB in memory */
	UINT8               lfb_stride;             /* stride of LFB accesses in bits */

	UINT32              width;                  /* width of current frame buffer */
	UINT32              height;                 /* height of current frame buffer */
	UINT32              xoffs;                  /* horizontal offset (back porch) */
	UINT32              yoffs;                  /* vertical offset (back porch) */
	UINT32              vsyncscan;              /* vertical sync scanline */
	UINT32              rowpixels;              /* pixels per row */
	UINT32              tile_width;             /* width of video tiles */
	UINT32              tile_height;            /* height of video tiles */
	UINT32              x_tiles;                /* number of tiles in the X direction */

	emu_timer *         vblank_timer;           /* VBLANK timer */
	UINT8               vblank;                 /* VBLANK state */
	UINT8               vblank_count;           /* number of VBLANKs since last swap */
	UINT8               vblank_swap_pending;    /* a swap is pending, waiting for a vblank */
	UINT8               vblank_swap;            /* swap when we hit this count */
	UINT8               vblank_dont_swap;       /* don't actually swap when we hit this point */

	/* triangle setup info */
	UINT8               cheating_allowed;       /* allow cheating? */
	INT32               sign;                   /* triangle sign */
	INT16               ax, ay;                 /* vertex A x,y (12.4) */
	INT16               bx, by;                 /* vertex B x,y (12.4) */
	INT16               cx, cy;                 /* vertex C x,y (12.4) */
	INT32               startr, startg, startb, starta; /* starting R,G,B,A (12.12) */
	INT32               startz;                 /* starting Z (20.12) */
	INT64               startw;                 /* starting W (16.32) */
	INT32               drdx, dgdx, dbdx, dadx; /* delta R,G,B,A per X */
	INT32               dzdx;                   /* delta Z per X */
	INT64               dwdx;                   /* delta W per X */
	INT32               drdy, dgdy, dbdy, dady; /* delta R,G,B,A per Y */
	INT32               dzdy;                   /* delta Z per Y */
	INT64               dwdy;                   /* delta W per Y */

	stats_block         lfb_stats;              /* LFB-access statistics */

	UINT8               sverts;                 /* number of vertices ready */
	setup_vertex        svert[3];               /* 3 setup vertices */

	fifo_state          fifo;                   /* framebuffer memory fifo */
	cmdfifo_info        cmdfifo[2];             /* command FIFOs */

	UINT8               fogblend[64];           /* 64-entry fog table */
	UINT8               fogdelta[64];           /* 64-entry fog table */
	UINT8               fogdelta_mask;          /* mask for for delta (0xff for V1, 0xfc for V2) */

	rgb_t               pen[65536];             /* mapping from pixels to pens */
	rgb_t               clut[512];              /* clut gamma data */
	UINT8               clut_dirty;             /* do we need to recompute? */
};


struct dac_state
{
	UINT8               reg[8];                 /* 8 registers */
	UINT8               read_result;            /* pending read result */
};


struct raster_info
{
	raster_info *       next;                   /* pointer to next entry with the same hash */
	poly_draw_scanline_func callback;           /* callback pointer */
	UINT8               is_generic;             /* TRUE if this is one of the generic rasterizers */
	UINT8               display;                /* display index */
	UINT32              hits;                   /* how many hits (pixels) we've used this for */
	UINT32              polys;                  /* how many polys we've used this for */
	UINT32              eff_color_path;         /* effective fbzColorPath value */
	UINT32              eff_alpha_mode;         /* effective alphaMode value */
	UINT32              eff_fog_mode;           /* effective fogMode value */
	UINT32              eff_fbz_mode;           /* effective fbzMode value */
	UINT32              eff_tex_mode_0;         /* effective textureMode value for TMU #0 */
	UINT32              eff_tex_mode_1;         /* effective textureMode value for TMU #1 */
	UINT32              hash;
};


struct poly_extra_data
{
	voodoo_state *      state;                  /* pointer back to the voodoo state */
	raster_info *       info;                   /* pointer to rasterizer information */

	INT16               ax, ay;                 /* vertex A x,y (12.4) */
	INT32               startr, startg, startb, starta; /* starting R,G,B,A (12.12) */
	INT32               startz;                 /* starting Z (20.12) */
	INT64               startw;                 /* starting W (16.32) */
	INT32               drdx, dgdx, dbdx, dadx; /* delta R,G,B,A per X */
	INT32               dzdx;                   /* delta Z per X */
	INT64               dwdx;                   /* delta W per X */
	INT32               drdy, dgdy, dbdy, dady; /* delta R,G,B,A per Y */
	INT32               dzdy;                   /* delta Z per Y */
	INT64               dwdy;                   /* delta W per Y */

	INT64               starts0, startt0;       /* starting S,T (14.18) */
	INT64               startw0;                /* starting W (2.30) */
	INT64               ds0dx, dt0dx;           /* delta S,T per X */
	INT64               dw0dx;                  /* delta W per X */
	INT64               ds0dy, dt0dy;           /* delta S,T per Y */
	INT64               dw0dy;                  /* delta W per Y */
	INT32               lodbase0;               /* used during rasterization */

	INT64               starts1, startt1;       /* starting S,T (14.18) */
	INT64               startw1;                /* starting W (2.30) */
	INT64               ds1dx, dt1dx;           /* delta S,T per X */
	INT64               dw1dx;                  /* delta W per X */
	INT64               ds1dy, dt1dy;           /* delta S,T per Y */
	INT64               dw1dy;                  /* delta W per Y */
	INT32               lodbase1;               /* used during rasterization */

	UINT16              dither[16];             /* dither matrix, for fastfill */
};


struct banshee_info
{
	UINT32              io[0x40];               /* I/O registers */
	UINT32              agp[0x80];              /* AGP registers */
	UINT8               vga[0x20];              /* VGA registers */
	UINT8               crtc[0x27];             /* VGA CRTC registers */
	UINT8               seq[0x05];              /* VGA sequencer registers */
	UINT8               gc[0x05];               /* VGA graphics controller registers */
	UINT8               att[0x15];              /* VGA attribute registers */
	UINT8               attff;                  /* VGA attribute flip-flop */

	UINT32              blt_regs[0x20];         /* 2D Blitter registers */
	UINT32              blt_dst_base;
	UINT32              blt_dst_x;
	UINT32              blt_dst_y;
	UINT32              blt_dst_width;
	UINT32              blt_dst_height;
	UINT32              blt_dst_stride;
	UINT32              blt_dst_bpp;
	UINT32              blt_cmd;
	UINT32              blt_src_base;
	UINT32              blt_src_x;
	UINT32              blt_src_y;
	UINT32              blt_src_width;
	UINT32              blt_src_height;
	UINT32              blt_src_stride;
	UINT32              blt_src_bpp;
};


struct voodoo_state
{
	UINT8               index;                  /* index of board */
	voodoo_device *device;               /* pointer to our containing device */
	screen_device *screen;              /* the screen we are acting on */
	device_t *cpu;                  /* the CPU we interact with */
	UINT8               type;                   /* type of system */
	UINT8               chipmask;               /* mask for which chips are available */
	UINT32              freq;                   /* operating frequency */
	attoseconds_t       attoseconds_per_cycle;  /* attoseconds per cycle */
	UINT32              extra_cycles;           /* extra cycles not yet accounted for */
	int                 trigger;                /* trigger used for stalling */

	voodoo_reg          reg[0x400];             /* raw registers */
	const UINT8 *       regaccess;              /* register access array */
	const char *const * regnames;               /* register names array */
	UINT8               alt_regmap;             /* enable alternate register map? */

	pci_state           pci;                    /* PCI state */
	dac_state           dac;                    /* DAC state */

	fbi_state           fbi;                    /* FBI states */
	tmu_state           tmu[MAX_TMU];           /* TMU states */
	tmu_shared_state    tmushare;               /* TMU shared state */
	banshee_info        banshee;                /* Banshee state */

	legacy_poly_manager * poly;                 /* polygon manager */
	stats_block *       thread_stats;           /* per-thread statistics */

	voodoo_stats        stats;                  /* internal statistics */

	offs_t              last_status_pc;         /* PC of last status description (for logging) */
	UINT32              last_status_value;      /* value of last status read (for logging) */

	int                 next_rasterizer;        /* next rasterizer index */
	raster_info         rasterizer[MAX_RASTERIZERS]; /* array of rasterizers */
	raster_info *       raster_hash[RASTER_HASH_SIZE]; /* hash table of rasterizers */

	bool                send_config;
	UINT32              tmu_config;
};



/*************************************
 *
 *  Inline FIFO management
 *
 *************************************/

static inline void fifo_reset(fifo_state *f)
{
	f->in = f->out = 0;
}


static inline void fifo_add(fifo_state *f, UINT32 data)
{
	INT32 next_in;

	/* compute the value of 'in' after we add this item */
	next_in = f->in + 1;
	if (next_in >= f->size)
		next_in = 0;

	/* as long as it's not equal to the output pointer, we can do it */
	if (next_in != f->out)
	{
		f->base[f->in] = data;
		f->in = next_in;
	}
}


static inline UINT32 fifo_remove(fifo_state *f)
{
	UINT32 data = 0xffffffff;

	/* as long as we have data, we can do it */
	if (f->out != f->in)
	{
		INT32 next_out;

		/* fetch the data */
		data = f->base[f->out];

		/* advance the output pointer */
		next_out = f->out + 1;
		if (next_out >= f->size)
			next_out = 0;
		f->out = next_out;
	}
	return data;
}


static inline UINT32 fifo_peek(fifo_state *f)
{
	return f->base[f->out];
}


static inline int fifo_empty(fifo_state *f)
{
	return (f->in == f->out);
}


static inline int fifo_full(fifo_state *f)
{
	return (f->in + 1 == f->out || (f->in == f->size - 1 && f->out == 0));
}


static inline INT32 fifo_items(fifo_state *f)
{
	INT32 items = f->in - f->out;
	if (items < 0)
		items += f->size;
	return items;
}


static inline INT32 fifo_space(fifo_state *f)
{
	INT32 items = f->in - f->out;
	if (items < 0)
		items += f->size;
	return f->size - 1 - items;
}



/*************************************
 *
 *  Computes a fast 16.16 reciprocal
 *  of a 16.32 value; used for
 *  computing 1/w in the rasterizer.
 *
 *  Since it is trivial to also
 *  compute log2(1/w) = -log2(w) at
 *  the same time, we do that as well
 *  to 16.8 precision for LOD
 *  calculations.
 *
 *  On a Pentium M, this routine is
 *  20% faster than a 64-bit integer
 *  divide and also produces the log
 *  for free.
 *
 *************************************/

static inline INT32 fast_reciplog(INT64 value, INT32 *log2)
{
	extern UINT32 voodoo_reciplog[];
	UINT32 temp, recip, rlog;
	UINT32 interp;
	UINT32 *table;
	int neg = FALSE;
	int lz, exp = 0;

	/* always work with unsigned numbers */
	if (value < 0)
	{
		value = -value;
		neg = TRUE;
	}

	/* if we've spilled out of 32 bits, push it down under 32 */
	if (value & U64(0xffff00000000))
	{
		temp = (UINT32)(value >> 16);
		exp -= 16;
	}
	else
		temp = (UINT32)value;

	/* if the resulting value is 0, the reciprocal is infinite */
	if (UNEXPECTED(temp == 0))
	{
		*log2 = 1000 << LOG_OUTPUT_PREC;
		return neg ? 0x80000000 : 0x7fffffff;
	}

	/* determine how many leading zeros in the value and shift it up high */
	lz = count_leading_zeros(temp);
	temp <<= lz;
	exp += lz;

	/* compute a pointer to the table entries we want */
	/* math is a bit funny here because we shift one less than we need to in order */
	/* to account for the fact that there are two UINT32's per table entry */
	table = &voodoo_reciplog[(temp >> (31 - RECIPLOG_LOOKUP_BITS - 1)) & ((2 << RECIPLOG_LOOKUP_BITS) - 2)];

	/* compute the interpolation value */
	interp = (temp >> (31 - RECIPLOG_LOOKUP_BITS - 8)) & 0xff;

	/* do a linear interpolatation between the two nearest table values */
	/* for both the log and the reciprocal */
	rlog = (table[1] * (0x100 - interp) + table[3] * interp) >> 8;
	recip = (table[0] * (0x100 - interp) + table[2] * interp) >> 8;

	/* the log result is the fractional part of the log; round it to the output precision */
	rlog = (rlog + (1 << (RECIPLOG_LOOKUP_PREC - LOG_OUTPUT_PREC - 1))) >> (RECIPLOG_LOOKUP_PREC - LOG_OUTPUT_PREC);

	/* the exponent is the non-fractional part of the log; normally, we would subtract it from rlog */
	/* but since we want the log(1/value) = -log(value), we subtract rlog from the exponent */
	*log2 = ((exp - (31 - RECIPLOG_INPUT_PREC)) << LOG_OUTPUT_PREC) - rlog;

	/* adjust the exponent to account for all the reciprocal-related parameters to arrive at a final shift amount */
	exp += (RECIP_OUTPUT_PREC - RECIPLOG_LOOKUP_PREC) - (31 - RECIPLOG_INPUT_PREC);

	/* shift by the exponent */
	if (exp < 0)
		recip >>= -exp;
	else
		recip <<= exp;

	/* on the way out, apply the original sign to the reciprocal */
	return neg ? -recip : recip;
}



/*************************************
 *
 *  Float-to-int conversions
 *
 *************************************/

static inline INT32 float_to_int32(UINT32 data, int fixedbits)
{
	int exponent = ((data >> 23) & 0xff) - 127 - 23 + fixedbits;
	INT32 result = (data & 0x7fffff) | 0x800000;
	if (exponent < 0)
	{
		if (exponent > -32)
			result >>= -exponent;
		else
			result = 0;
	}
	else
	{
		if (exponent < 32)
			result <<= exponent;
		else
			result = 0x7fffffff;
	}
	if (data & 0x80000000)
		result = -result;
	return result;
}


static inline INT64 float_to_int64(UINT32 data, int fixedbits)
{
	int exponent = ((data >> 23) & 0xff) - 127 - 23 + fixedbits;
	INT64 result = (data & 0x7fffff) | 0x800000;
	if (exponent < 0)
	{
		if (exponent > -64)
			result >>= -exponent;
		else
			result = 0;
	}
	else
	{
		if (exponent < 64)
			result <<= exponent;
		else
			result = U64(0x7fffffffffffffff);
	}
	if (data & 0x80000000)
		result = -result;
	return result;
}



/*************************************
 *
 *  Rasterizer inlines
 *
 *************************************/

static inline UINT32 normalize_color_path(UINT32 eff_color_path)
{
	/* ignore the subpixel adjust and texture enable flags */
	eff_color_path &= ~((1 << 26) | (1 << 27));

	return eff_color_path;
}


static inline UINT32 normalize_alpha_mode(UINT32 eff_alpha_mode)
{
	/* always ignore alpha ref value */
	eff_alpha_mode &= ~(0xff << 24);

	/* if not doing alpha testing, ignore the alpha function and ref value */
	if (!ALPHAMODE_ALPHATEST(eff_alpha_mode))
		eff_alpha_mode &= ~(7 << 1);

	/* if not doing alpha blending, ignore the source and dest blending factors */
	if (!ALPHAMODE_ALPHABLEND(eff_alpha_mode))
		eff_alpha_mode &= ~((15 << 8) | (15 << 12) | (15 << 16) | (15 << 20));

	return eff_alpha_mode;
}


static inline UINT32 normalize_fog_mode(UINT32 eff_fog_mode)
{
	/* if not doing fogging, ignore all the other fog bits */
	if (!FOGMODE_ENABLE_FOG(eff_fog_mode))
		eff_fog_mode = 0;

	return eff_fog_mode;
}


static inline UINT32 normalize_fbz_mode(UINT32 eff_fbz_mode)
{
	/* ignore the draw buffer */
	eff_fbz_mode &= ~(3 << 14);

	return eff_fbz_mode;
}


static inline UINT32 normalize_tex_mode(UINT32 eff_tex_mode)
{
	/* ignore the NCC table and seq_8_downld flags */
	eff_tex_mode &= ~((1 << 5) | (1 << 31));

	/* classify texture formats into 3 format categories */
	if (TEXMODE_FORMAT(eff_tex_mode) < 8)
		eff_tex_mode = (eff_tex_mode & ~(0xf << 8)) | (0 << 8);
	else if (TEXMODE_FORMAT(eff_tex_mode) >= 10 && TEXMODE_FORMAT(eff_tex_mode) <= 12)
		eff_tex_mode = (eff_tex_mode & ~(0xf << 8)) | (10 << 8);
	else
		eff_tex_mode = (eff_tex_mode & ~(0xf << 8)) | (8 << 8);

	return eff_tex_mode;
}


static inline UINT32 compute_raster_hash(const raster_info *info)
{
	UINT32 hash;

	/* make a hash */
	hash = info->eff_color_path;
	hash = (hash << 1) | (hash >> 31);
	hash ^= info->eff_fbz_mode;
	hash = (hash << 1) | (hash >> 31);
	hash ^= info->eff_alpha_mode;
	hash = (hash << 1) | (hash >> 31);
	hash ^= info->eff_fog_mode;
	hash = (hash << 1) | (hash >> 31);
	hash ^= info->eff_tex_mode_0;
	hash = (hash << 1) | (hash >> 31);
	hash ^= info->eff_tex_mode_1;

	return hash % RASTER_HASH_SIZE;
}



/*************************************
 *
 *  Dithering macros
 *
 *************************************/

/* note that these equations and the dither matrixes have
   been confirmed to be exact matches to the real hardware */
#define DITHER_RB(val,dith) ((((val) << 1) - ((val) >> 4) + ((val) >> 7) + (dith)) >> 1)
#define DITHER_G(val,dith)  ((((val) << 2) - ((val) >> 4) + ((val) >> 6) + (dith)) >> 2)

#define DECLARE_DITHER_POINTERS                                                 \
	const UINT8 *dither_lookup = NULL;                                          \
	const UINT8 *dither4 = NULL;                                                \
	const UINT8 *dither = NULL
#define DECLARE_DITHER_POINTERS_NO_DITHER_VAR                                               \
	const UINT8 *dither_lookup = NULL;
#define COMPUTE_DITHER_POINTERS(FBZMODE, YY)                                    \
do                                                                              \
{                                                                               \
	/* compute the dithering pointers */                                        \
	if (FBZMODE_ENABLE_DITHERING(FBZMODE))                                      \
	{                                                                           \
		dither4 = &dither_matrix_4x4[((YY) & 3) * 4];                           \
		if (FBZMODE_DITHER_TYPE(FBZMODE) == 0)                                  \
		{                                                                       \
			dither = dither4;                                                   \
			dither_lookup = &dither4_lookup[(YY & 3) << 11];                    \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			dither = &dither_matrix_2x2[((YY) & 3) * 4];                        \
			dither_lookup = &dither2_lookup[(YY & 3) << 11];                    \
		}                                                                       \
	}                                                                           \
}                                                                               \
while (0)

#define COMPUTE_DITHER_POINTERS_NO_DITHER_VAR(FBZMODE, YY)                                  \
do                                                                              \
{                                                                               \
	/* compute the dithering pointers */                                        \
	if (FBZMODE_ENABLE_DITHERING(FBZMODE))                                      \
	{                                                                           \
		if (FBZMODE_DITHER_TYPE(FBZMODE) == 0)                                  \
		{                                                                       \
			dither_lookup = &dither4_lookup[(YY & 3) << 11];                    \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			dither_lookup = &dither2_lookup[(YY & 3) << 11];                    \
		}                                                                       \
	}                                                                           \
}                                                                               \
while (0)

#define APPLY_DITHER(FBZMODE, XX, DITHER_LOOKUP, RR, GG, BB)                    \
do                                                                              \
{                                                                               \
	/* apply dithering */                                                       \
	if (FBZMODE_ENABLE_DITHERING(FBZMODE))                                      \
	{                                                                           \
		/* look up the dither value from the appropriate matrix */              \
		const UINT8 *dith = &DITHER_LOOKUP[((XX) & 3) << 1];                    \
																				\
		/* apply dithering to R,G,B */                                          \
		(RR) = dith[((RR) << 3) + 0];                                           \
		(GG) = dith[((GG) << 3) + 1];                                           \
		(BB) = dith[((BB) << 3) + 0];                                           \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		(RR) >>= 3;                                                             \
		(GG) >>= 2;                                                             \
		(BB) >>= 3;                                                             \
	}                                                                           \
}                                                                               \
while (0)



/*************************************
 *
 *  Clamping macros
 *
 *************************************/

#define CLAMPED_ARGB(ITERR, ITERG, ITERB, ITERA, FBZCP, RESULT)                 \
do                                                                              \
{                                                                               \
	r = (INT32)(ITERR) >> 12;                                             \
	g = (INT32)(ITERG) >> 12;                                             \
	b = (INT32)(ITERB) >> 12;                                             \
	a = (INT32)(ITERA) >> 12;                                             \
																				\
	if (FBZCP_RGBZW_CLAMP(FBZCP) == 0)                                          \
	{                                                                           \
		r &= 0xfff;                                                             \
		RESULT.rgb.r = r;                                                       \
		if (r == 0xfff)                                                         \
			RESULT.rgb.r = 0;                                                   \
		else if (r == 0x100)                                                    \
			RESULT.rgb.r = 0xff;                                                \
																				\
		g &= 0xfff;                                                             \
		RESULT.rgb.g = g;                                                       \
		if (g == 0xfff)                                                         \
			RESULT.rgb.g = 0;                                                   \
		else if (g == 0x100)                                                    \
			RESULT.rgb.g = 0xff;                                                \
																				\
		b &= 0xfff;                                                             \
		RESULT.rgb.b = b;                                                       \
		if (b == 0xfff)                                                         \
			RESULT.rgb.b = 0;                                                   \
		else if (b == 0x100)                                                    \
			RESULT.rgb.b = 0xff;                                                \
																				\
		a &= 0xfff;                                                             \
		RESULT.rgb.a = a;                                                       \
		if (a == 0xfff)                                                         \
			RESULT.rgb.a = 0;                                                   \
		else if (a == 0x100)                                                    \
			RESULT.rgb.a = 0xff;                                                \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		RESULT.rgb.r = (r < 0) ? 0 : (r > 0xff) ? 0xff : r;                     \
		RESULT.rgb.g = (g < 0) ? 0 : (g > 0xff) ? 0xff : g;                     \
		RESULT.rgb.b = (b < 0) ? 0 : (b > 0xff) ? 0xff : b;                     \
		RESULT.rgb.a = (a < 0) ? 0 : (a > 0xff) ? 0xff : a;                     \
	}                                                                           \
}                                                                               \
while (0)

static inline rgbaint_t ATTR_FORCE_INLINE clampARGB(const rgbaint_t &iterargb, UINT32 FBZCP)
{
	rgbaint_t result(iterargb);
	//rgbaint_t colorint((INT32) (itera>>12), (INT32) (iterr>>12), (INT32) (iterg>>12), (INT32) (iterb>>12));
	result.shr_imm(12);

	if (FBZCP_RGBZW_CLAMP(FBZCP) == 0)
	{
		//r &= 0xfff;
		result.and_imm(0xfff);
		//if (r == 0xfff)
		rgbaint_t temp(result);
		temp.cmpeq_imm(0xfff);
		//  result.rgb.r = 0;
		result.andnot_reg(temp);
		//else if (r == 0x100)
		temp.set(result);
		temp.cmpeq_imm(0x100);
		// Shift by 1 so that INT32 result is not negative
		temp.shr_imm(1);
		//  result.rgb.r = 0xff;
		result.or_reg(temp);
	}
	else
	{
		//return colorint.to_rgba_clamp();
	}
	result.clamp_to_uint8();
	return result;
}

#define CLAMPED_Z(ITERZ, FBZCP, RESULT)                                         \
do                                                                              \
{                                                                               \
	(RESULT) = (INT32)(ITERZ) >> 12;                                            \
	if (FBZCP_RGBZW_CLAMP(FBZCP) == 0)                                          \
	{                                                                           \
		(RESULT) &= 0xfffff;                                                    \
		if ((RESULT) == 0xfffff)                                                \
			(RESULT) = 0;                                                       \
		else if ((RESULT) == 0x10000)                                           \
			(RESULT) = 0xffff;                                                  \
		else                                                                    \
			(RESULT) &= 0xffff;                                                 \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		CLAMP((RESULT), 0, 0xffff);                                             \
	}                                                                           \
}                                                                               \
while (0)


#define CLAMPED_W(ITERW, FBZCP, RESULT)                                         \
do                                                                              \
{                                                                               \
	(RESULT) = (INT16)((ITERW) >> 32);                                          \
	if (FBZCP_RGBZW_CLAMP(FBZCP) == 0)                                          \
	{                                                                           \
		(RESULT) &= 0xffff;                                                     \
		if ((RESULT) == 0xffff)                                                 \
			(RESULT) = 0;                                                       \
		else if ((RESULT) == 0x100)                                             \
			(RESULT) = 0xff;                                                    \
		(RESULT) &= 0xff;                                                       \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		CLAMP((RESULT), 0, 0xff);                                               \
	}                                                                           \
}                                                                               \
while (0)



/*************************************
 *
 *  Chroma keying macro
 *
 *************************************/

#define APPLY_CHROMAKEY(VV, STATS, FBZMODE, COLOR)                              \
do                                                                              \
{                                                                               \
	if (FBZMODE_ENABLE_CHROMAKEY(FBZMODE))                                      \
	{                                                                           \
		/* non-range version */                                                 \
		if (!CHROMARANGE_ENABLE((VV)->reg[chromaRange].u))                      \
		{                                                                       \
			if (((COLOR.u ^ (VV)->reg[chromaKey].u) & 0xffffff) == 0)           \
			{                                                                   \
				(STATS)->chroma_fail++;                                         \
				goto skipdrawdepth;                                             \
			}                                                                   \
		}                                                                       \
																				\
		/* tricky range version */                                              \
		else                                                                    \
		{                                                                       \
			INT32 low, high, test;                                              \
			int results = 0;                                                    \
																				\
			/* check blue */                                                    \
			low = (VV)->reg[chromaKey].rgb.b;                                   \
			high = (VV)->reg[chromaRange].rgb.b;                                \
			test = COLOR.rgb.b;                                                 \
			results = (test >= low && test <= high);                            \
			results ^= CHROMARANGE_BLUE_EXCLUSIVE((VV)->reg[chromaRange].u);    \
			results <<= 1;                                                      \
																				\
			/* check green */                                                   \
			low = (VV)->reg[chromaKey].rgb.g;                                   \
			high = (VV)->reg[chromaRange].rgb.g;                                \
			test = COLOR.rgb.g;                                                 \
			results |= (test >= low && test <= high);                           \
			results ^= CHROMARANGE_GREEN_EXCLUSIVE((VV)->reg[chromaRange].u);   \
			results <<= 1;                                                      \
																				\
			/* check red */                                                     \
			low = (VV)->reg[chromaKey].rgb.r;                                   \
			high = (VV)->reg[chromaRange].rgb.r;                                \
			test = COLOR.rgb.r;                                                 \
			results |= (test >= low && test <= high);                           \
			results ^= CHROMARANGE_RED_EXCLUSIVE((VV)->reg[chromaRange].u);     \
																				\
			/* final result */                                                  \
			if (CHROMARANGE_UNION_MODE((VV)->reg[chromaRange].u))               \
			{                                                                   \
				if (results != 0)                                               \
				{                                                               \
					(STATS)->chroma_fail++;                                     \
					goto skipdrawdepth;                                         \
				}                                                               \
			}                                                                   \
			else                                                                \
			{                                                                   \
				if (results == 7)                                               \
				{                                                               \
					(STATS)->chroma_fail++;                                     \
					goto skipdrawdepth;                                         \
				}                                                               \
			}                                                                   \
		}                                                                       \
	}                                                                           \
}                                                                               \
while (0)

static inline bool ATTR_FORCE_INLINE chromaKeyTest(voodoo_state *v, stats_block *stats, UINT32 fbzModeReg, rgbaint_t rgbaIntColor)
{
	if (FBZMODE_ENABLE_CHROMAKEY(fbzModeReg))
	{
		rgb_union color;
		color.u = (rgbaIntColor.get_a()<<24) | (rgbaIntColor.get_r()<<16) | (rgbaIntColor.get_g()<<8) | rgbaIntColor.get_b();
		/* non-range version */
		if (!CHROMARANGE_ENABLE(v->reg[chromaRange].u))
		{
			if (((color.u ^ v->reg[chromaKey].u) & 0xffffff) == 0)
			{
				stats->chroma_fail++;
				return false;
			}
		}

		/* tricky range version */
		else
		{
			INT32 low, high, test;
			int results = 0;

			/* check blue */
			low = v->reg[chromaKey].rgb.b;
			high = v->reg[chromaRange].rgb.b;
			test = color.rgb.b;
			results = (test >= low && test <= high);
			results ^= CHROMARANGE_BLUE_EXCLUSIVE(v->reg[chromaRange].u);
			results <<= 1;

			/* check green */
			low = v->reg[chromaKey].rgb.g;
			high = v->reg[chromaRange].rgb.g;
			test = color.rgb.g;
			results |= (test >= low && test <= high);
			results ^= CHROMARANGE_GREEN_EXCLUSIVE(v->reg[chromaRange].u);
			results <<= 1;

			/* check red */
			low = v->reg[chromaKey].rgb.r;
			high = v->reg[chromaRange].rgb.r;
			test = color.rgb.r;
			results |= (test >= low && test <= high);
			results ^= CHROMARANGE_RED_EXCLUSIVE(v->reg[chromaRange].u);

			/* final result */
			if (CHROMARANGE_UNION_MODE(v->reg[chromaRange].u))
			{
				if (results != 0)
				{
					stats->chroma_fail++;
					return false;
				}
			}
			else
			{
				if (results == 7)
				{
					stats->chroma_fail++;
					return false;
				}
			}
		}
	}
	return true;
}



/*************************************
 *
 *  Alpha masking macro
 *
 *************************************/

#define APPLY_ALPHAMASK(VV, STATS, FBZMODE, AA)                                 \
do                                                                              \
{                                                                               \
	if (FBZMODE_ENABLE_ALPHA_MASK(FBZMODE))                                     \
	{                                                                           \
		if (((AA) & 1) == 0)                                                    \
		{                                                                       \
			(STATS)->afunc_fail++;                                              \
			goto skipdrawdepth;                                                 \
		}                                                                       \
	}                                                                           \
}                                                                               \
while (0)

static inline bool alphaMaskTest(stats_block *stats, UINT32 fbzModeReg, UINT8 alpha)
{
	if (FBZMODE_ENABLE_ALPHA_MASK(fbzModeReg))
	{
		if ((alpha & 1) == 0)
		{
			stats->afunc_fail++;
			return false;
		}
	}
	return true;
}

/*************************************
 *
 *  Alpha testing macro
 *
 *************************************/

#define APPLY_ALPHATEST(VV, STATS, ALPHAMODE, AA)                               \
do                                                                              \
{                                                                               \
	if (ALPHAMODE_ALPHATEST(ALPHAMODE))                                         \
	{                                                                           \
		UINT8 alpharef = (VV)->reg[alphaMode].rgb.a;                            \
		switch (ALPHAMODE_ALPHAFUNCTION(ALPHAMODE))                             \
		{                                                                       \
			case 0:     /* alphaOP = never */                                   \
				(STATS)->afunc_fail++;                                          \
				goto skipdrawdepth;                                             \
																				\
			case 1:     /* alphaOP = less than */                               \
				if ((AA) >= alpharef)                                           \
				{                                                               \
					(STATS)->afunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 2:     /* alphaOP = equal */                                   \
				if ((AA) != alpharef)                                           \
				{                                                               \
					(STATS)->afunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 3:     /* alphaOP = less than or equal */                      \
				if ((AA) > alpharef)                                            \
				{                                                               \
					(STATS)->afunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 4:     /* alphaOP = greater than */                            \
				if ((AA) <= alpharef)                                           \
				{                                                               \
					(STATS)->afunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 5:     /* alphaOP = not equal */                               \
				if ((AA) == alpharef)                                           \
				{                                                               \
					(STATS)->afunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 6:     /* alphaOP = greater than or equal */                   \
				if ((AA) < alpharef)                                            \
				{                                                               \
					(STATS)->afunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 7:     /* alphaOP = always */                                  \
				break;                                                          \
		}                                                                       \
	}                                                                           \
}                                                                               \
while (0)

static inline bool ATTR_FORCE_INLINE alphaTest(voodoo_state *v, stats_block *stats, UINT32 alphaModeReg, UINT8 alpha)
{
	if (ALPHAMODE_ALPHATEST(alphaModeReg))
	{
		UINT8 alpharef = v->reg[alphaMode].rgb.a;
		switch (ALPHAMODE_ALPHAFUNCTION(alphaModeReg))
		{
			case 0:     /* alphaOP = never */
				stats->afunc_fail++;
				return false;

			case 1:     /* alphaOP = less than */
				if (alpha >= alpharef)
				{
					stats->afunc_fail++;
					return false;
				}
				break;

			case 2:     /* alphaOP = equal */
				if (alpha != alpharef)
				{
					stats->afunc_fail++;
					return false;
				}
				break;

			case 3:     /* alphaOP = less than or equal */
				if (alpha > alpharef)
				{
					stats->afunc_fail++;
					return false;
				}
				break;

			case 4:     /* alphaOP = greater than */
				if (alpha <= alpharef)
				{
					stats->afunc_fail++;
					return false;
				}
				break;

			case 5:     /* alphaOP = not equal */
				if (alpha == alpharef)
				{
					stats->afunc_fail++;
					return false;
				}
				break;

			case 6:     /* alphaOP = greater than or equal */
				if (alpha < alpharef)
				{
					stats->afunc_fail++;
					return false;
				}
				break;

			case 7:     /* alphaOP = always */
				break;
		}
	}
	return true;
}


/*************************************
 *
 *  Alpha blending macro
 *
 *************************************/

#define APPLY_ALPHA_BLEND(FBZMODE, ALPHAMODE, XX, DITHER, RR, GG, BB, AA)       \
do                                                                              \
{                                                                               \
	if (ALPHAMODE_ALPHABLEND(ALPHAMODE))                                        \
	{                                                                           \
		int dpix = dest[XX];                                                    \
		int dr, dg, db;                                                         \
		EXTRACT_565_TO_888(dpix, dr, dg, db);                                   \
		int da = FBZMODE_ENABLE_ALPHA_PLANES(FBZMODE) ? depth[XX] : 0xff;       \
		int sr = (RR);                                                          \
		int sg = (GG);                                                          \
		int sb = (BB);                                                          \
		int sa = (AA);                                                          \
		int ta;                                                                 \
																				\
		/* apply dither subtraction */                                          \
		if (FBZMODE_ALPHA_DITHER_SUBTRACT(FBZMODE))                             \
		{                                                                       \
			/* look up the dither value from the appropriate matrix */          \
			int dith = DITHER[(XX) & 3];                                        \
																				\
			/* subtract the dither value */                                     \
			dr = ((dr << 1) + 15 - dith) >> 1;                                  \
			dg = ((dg << 2) + 15 - dith) >> 2;                                  \
			db = ((db << 1) + 15 - dith) >> 1;                                  \
		}                                                                       \
																				\
		/* compute source portion */                                            \
		switch (ALPHAMODE_SRCRGBBLEND(ALPHAMODE))                               \
		{                                                                       \
			default:    /* reserved */                                          \
			case 0:     /* AZERO */                                             \
				(RR) = (GG) = (BB) = 0;                                         \
				break;                                                          \
																				\
			case 1:     /* ASRC_ALPHA */                                        \
				(RR) = (sr * (sa + 1)) >> 8;                                    \
				(GG) = (sg * (sa + 1)) >> 8;                                    \
				(BB) = (sb * (sa + 1)) >> 8;                                    \
				break;                                                          \
																				\
			case 2:     /* A_COLOR */                                           \
				(RR) = (sr * (dr + 1)) >> 8;                                    \
				(GG) = (sg * (dg + 1)) >> 8;                                    \
				(BB) = (sb * (db + 1)) >> 8;                                    \
				break;                                                          \
																				\
			case 3:     /* ADST_ALPHA */                                        \
				(RR) = (sr * (da + 1)) >> 8;                                    \
				(GG) = (sg * (da + 1)) >> 8;                                    \
				(BB) = (sb * (da + 1)) >> 8;                                    \
				break;                                                          \
																				\
			case 4:     /* AONE */                                              \
				break;                                                          \
																				\
			case 5:     /* AOMSRC_ALPHA */                                      \
				(RR) = (sr * (0x100 - sa)) >> 8;                                \
				(GG) = (sg * (0x100 - sa)) >> 8;                                \
				(BB) = (sb * (0x100 - sa)) >> 8;                                \
				break;                                                          \
																				\
			case 6:     /* AOM_COLOR */                                         \
				(RR) = (sr * (0x100 - dr)) >> 8;                                \
				(GG) = (sg * (0x100 - dg)) >> 8;                                \
				(BB) = (sb * (0x100 - db)) >> 8;                                \
				break;                                                          \
																				\
			case 7:     /* AOMDST_ALPHA */                                      \
				(RR) = (sr * (0x100 - da)) >> 8;                                \
				(GG) = (sg * (0x100 - da)) >> 8;                                \
				(BB) = (sb * (0x100 - da)) >> 8;                                \
				break;                                                          \
																				\
			case 15:    /* ASATURATE */                                         \
				ta = (sa < (0x100 - da)) ? sa : (0x100 - da);                   \
				(RR) = (sr * (ta + 1)) >> 8;                                    \
				(GG) = (sg * (ta + 1)) >> 8;                                    \
				(BB) = (sb * (ta + 1)) >> 8;                                    \
				break;                                                          \
		}                                                                       \
																				\
		/* add in dest portion */                                               \
		switch (ALPHAMODE_DSTRGBBLEND(ALPHAMODE))                               \
		{                                                                       \
			default:    /* reserved */                                          \
			case 0:     /* AZERO */                                             \
				break;                                                          \
																				\
			case 1:     /* ASRC_ALPHA */                                        \
				(RR) += (dr * (sa + 1)) >> 8;                                   \
				(GG) += (dg * (sa + 1)) >> 8;                                   \
				(BB) += (db * (sa + 1)) >> 8;                                   \
				break;                                                          \
																				\
			case 2:     /* A_COLOR */                                           \
				(RR) += (dr * (sr + 1)) >> 8;                                   \
				(GG) += (dg * (sg + 1)) >> 8;                                   \
				(BB) += (db * (sb + 1)) >> 8;                                   \
				break;                                                          \
																				\
			case 3:     /* ADST_ALPHA */                                        \
				(RR) += (dr * (da + 1)) >> 8;                                   \
				(GG) += (dg * (da + 1)) >> 8;                                   \
				(BB) += (db * (da + 1)) >> 8;                                   \
				break;                                                          \
																				\
			case 4:     /* AONE */                                              \
				(RR) += dr;                                                     \
				(GG) += dg;                                                     \
				(BB) += db;                                                     \
				break;                                                          \
																				\
			case 5:     /* AOMSRC_ALPHA */                                      \
				(RR) += (dr * (0x100 - sa)) >> 8;                               \
				(GG) += (dg * (0x100 - sa)) >> 8;                               \
				(BB) += (db * (0x100 - sa)) >> 8;                               \
				break;                                                          \
																				\
			case 6:     /* AOM_COLOR */                                         \
				(RR) += (dr * (0x100 - sr)) >> 8;                               \
				(GG) += (dg * (0x100 - sg)) >> 8;                               \
				(BB) += (db * (0x100 - sb)) >> 8;                               \
				break;                                                          \
																				\
			case 7:     /* AOMDST_ALPHA */                                      \
				(RR) += (dr * (0x100 - da)) >> 8;                               \
				(GG) += (dg * (0x100 - da)) >> 8;                               \
				(BB) += (db * (0x100 - da)) >> 8;                               \
				break;                                                          \
																				\
			case 15:    /* A_COLORBEFOREFOG */                                  \
				(RR) += (dr * (prefogr + 1)) >> 8;                              \
				(GG) += (dg * (prefogg + 1)) >> 8;                              \
				(BB) += (db * (prefogb + 1)) >> 8;                              \
				break;                                                          \
		}                                                                       \
																				\
		/* blend the source alpha */                                            \
		(AA) = 0;                                                               \
		if (ALPHAMODE_SRCALPHABLEND(ALPHAMODE) == 4)                            \
			(AA) = sa;                                                          \
																				\
		/* blend the dest alpha */                                              \
		if (ALPHAMODE_DSTALPHABLEND(ALPHAMODE) == 4)                            \
			(AA) += da;                                                         \
																				\
		/* clamp */                                                             \
		CLAMP((RR), 0x00, 0xff);                                                \
		CLAMP((GG), 0x00, 0xff);                                                \
		CLAMP((BB), 0x00, 0xff);                                                \
		CLAMP((AA), 0x00, 0xff);                                                \
	}                                                                           \
}                                                                               \
while (0)

static inline void ATTR_FORCE_INLINE alphaBlend(UINT32 FBZMODE, UINT32 ALPHAMODE, INT32 x, const UINT8 *dither, int dpix, UINT16 *depth, rgbaint_t &preFog, rgbaint_t &srcColor)
{
	if (ALPHAMODE_ALPHABLEND(ALPHAMODE))
	{
		//int dpix = dest[XX];
		int dr, dg, db;
		EXTRACT_565_TO_888(dpix, dr, dg, db);
		int da = FBZMODE_ENABLE_ALPHA_PLANES(FBZMODE) ? depth[x] : 0xff;
		//int sr = (RR);
		//int sg = (GG);
		//int sb = (BB);
		//int sa = (AA);
		int sa = srcColor.get_a();
		int ta;
		int srcAlphaScale, destAlphaScale;
		rgbaint_t srcScale, destScale;

		/* apply dither subtraction */
		if (FBZMODE_ALPHA_DITHER_SUBTRACT(FBZMODE))
		{
			/* look up the dither value from the appropriate matrix */
			//int dith = DITHER[(XX) & 3];

			/* subtract the dither value */
			dr += (15 - dither[x&3]) >> 1;
			dg += (15 - dither[x&3]) >> 2;
			db += (15 - dither[x&3]) >> 1;
		}

		/* blend the source alpha */
		srcAlphaScale = 0;
		if (ALPHAMODE_SRCALPHABLEND(ALPHAMODE) == 4)
			srcAlphaScale = 256;
			//(AA) = sa;

		/* compute source portion */
		switch (ALPHAMODE_SRCRGBBLEND(ALPHAMODE))
		{
			default:    /* reserved */
			case 0:     /* AZERO */
				srcScale.set(srcAlphaScale, 0, 0, 0);
				//(RR) = (GG) = (BB) = 0;
				break;

			case 1:     /* ASRC_ALPHA */
				srcScale.set(srcAlphaScale-1, sa, sa, sa);
				srcScale.add_imm(1);
				//(RR) = (sr * (sa + 1)) >> 8;
				//(GG) = (sg * (sa + 1)) >> 8;
				//(BB) = (sb * (sa + 1)) >> 8;
				break;

			case 2:     /* A_COLOR */
				srcScale.set(srcAlphaScale-1, dr, dg, db);
				srcScale.add_imm(1);
				//(RR) = (sr * (dr + 1)) >> 8;
				//(GG) = (sg * (dg + 1)) >> 8;
				//(BB) = (sb * (db + 1)) >> 8;
				break;

			case 3:     /* ADST_ALPHA */
				ta = da + 1;
				srcScale.set(srcAlphaScale, ta, ta, ta);
				//(RR) = (sr * (da + 1)) >> 8;
				//(GG) = (sg * (da + 1)) >> 8;
				//(BB) = (sb * (da + 1)) >> 8;
				break;

			case 4:     /* AONE */
				srcScale.set(srcAlphaScale, 256, 256, 256);
				break;

			case 5:     /* AOMSRC_ALPHA */
				ta = (0x100 - sa);
				srcScale.set(srcAlphaScale, ta, ta, ta);
				//(RR) = (sr * (0x100 - sa)) >> 8;
				//(GG) = (sg * (0x100 - sa)) >> 8;
				//(BB) = (sb * (0x100 - sa)) >> 8;
				break;

			case 6:     /* AOM_COLOR */
				srcScale.set(srcAlphaScale, (0x100 - dr), (0x100 - dg), (0x100 - db));
				//(RR) = (sr * (0x100 - dr)) >> 8;
				//(GG) = (sg * (0x100 - dg)) >> 8;
				//(BB) = (sb * (0x100 - db)) >> 8;
				break;

			case 7:     /* AOMDST_ALPHA */
				ta = (0x100 - da);
				srcScale.set(srcAlphaScale, ta, ta, ta);
				//(RR) = (sr * (0x100 - da)) >> 8;
				//(GG) = (sg * (0x100 - da)) >> 8;
				//(BB) = (sb * (0x100 - da)) >> 8;
				break;

			case 15:    /* ASATURATE */
				ta = (sa < (0x100 - da)) ? sa : (0x100 - da);
				ta++;
				srcScale.set(srcAlphaScale, ta, ta, ta);
				//(RR) = (sr * (ta + 1)) >> 8;
				//(GG) = (sg * (ta + 1)) >> 8;
				//(BB) = (sb * (ta + 1)) >> 8;
				break;
		}

		/* blend the dest alpha */
		destAlphaScale = 0;
		if (ALPHAMODE_DSTALPHABLEND(ALPHAMODE) == 4)
			destAlphaScale = 256;
			//(AA) += da;

		/* add in dest portion */
		switch (ALPHAMODE_DSTRGBBLEND(ALPHAMODE))
		{
			default:    /* reserved */
			case 0:     /* AZERO */
				destScale.set(destAlphaScale, 0, 0, 0);
				break;

			case 1:     /* ASRC_ALPHA */
				destScale.set(destAlphaScale-1, sa, sa, sa);
				destScale.add_imm(1);
				//(RR) += (dr * (sa + 1)) >> 8;
				//(GG) += (dg * (sa + 1)) >> 8;
				//(BB) += (db * (sa + 1)) >> 8;
				break;

			case 2:     /* A_COLOR */
				destScale.set(srcColor);
				destScale.add_imm(1);
				destScale.set_a(destAlphaScale);
				//(RR) += (dr * (sr + 1)) >> 8;
				//(GG) += (dg * (sg + 1)) >> 8;
				//(BB) += (db * (sb + 1)) >> 8;
				break;

			case 3:     /* ADST_ALPHA */
				ta = da + 1;
				destScale.set(destAlphaScale, ta, ta, ta);
				//(RR) += (dr * (da + 1)) >> 8;
				//(GG) += (dg * (da + 1)) >> 8;
				//(BB) += (db * (da + 1)) >> 8;
				break;

			case 4:     /* AONE */
				destScale.set(destAlphaScale, 256, 256, 256);
				//(RR) += dr;
				//(GG) += dg;
				//(BB) += db;
				break;

			case 5:     /* AOMSRC_ALPHA */
				ta = (0x100 - sa);
				destScale.set(destAlphaScale, ta, ta, ta);
				//(RR) += (dr * (0x100 - sa)) >> 8;
				//(GG) += (dg * (0x100 - sa)) >> 8;
				//(BB) += (db * (0x100 - sa)) >> 8;
				break;

			case 6:     /* AOM_COLOR */
				destScale.set(0x100, 0x100, 0x100, 0x100);
				destScale.sub(srcColor);
				destScale.set_a(destAlphaScale);
				//destScale.set(destAlphaScale, (0x100 - color.rgb.r), (0x100 - color.rgb.g), (0x100 - color.rgb.b));
				//(RR) += (dr * (0x100 - sr)) >> 8;
				//(GG) += (dg * (0x100 - sg)) >> 8;
				//(BB) += (db * (0x100 - sb)) >> 8;
				break;

			case 7:     /* AOMDST_ALPHA */
				ta = (0x100 - da);
				destScale.set(destAlphaScale, ta, ta, ta);
				//(RR) += (dr * (0x100 - da)) >> 8;
				//(GG) += (dg * (0x100 - da)) >> 8;
				//(BB) += (db * (0x100 - da)) >> 8;
				break;

			case 15:    /* A_COLORBEFOREFOG */
				destScale.set(preFog);
				destScale.add_imm(1);
				destScale.set_a(destAlphaScale);
				//destScale.set((rgb_t) (((destAlphaScale-1)<<24) | (preFog.u & 0x00ffffff)));
				//destScale.add_imm(1);
				//(RR) += (dr * (prefogr + 1)) >> 8;
				//(GG) += (dg * (prefogg + 1)) >> 8;
				//(BB) += (db * (prefogb + 1)) >> 8;
				break;
		}
		// Main blend
		rgbaint_t destColor(da, dr, dg, db);

		srcColor.scale2_add_and_clamp(srcScale, destColor, destScale);
		/* clamp */
		//CLAMP((RR), 0x00, 0xff);
		//CLAMP((GG), 0x00, 0xff);
		//CLAMP((BB), 0x00, 0xff);
		//CLAMP((AA), 0x00, 0xff);
	}
}


/*************************************
 *
 *  Fogging macro
 *
 *************************************/

#define APPLY_FOGGING(VV, FOGMODE, FBZCP, XX, DITHER4, RR, GG, BB, ITERZ, ITERW, ITERAXXX)  \
do                                                                              \
{                                                                               \
	if (FOGMODE_ENABLE_FOG(FOGMODE))                                            \
	{                                                                           \
		rgb_union fogcolor = (VV)->reg[fogColor];                               \
		INT32 fr, fg, fb;                                                       \
																				\
		/* constant fog bypasses everything else */                             \
		if (FOGMODE_FOG_CONSTANT(FOGMODE))                                      \
		{                                                                       \
			fr = fogcolor.rgb.r;                                                \
			fg = fogcolor.rgb.g;                                                \
			fb = fogcolor.rgb.b;                                                \
		}                                                                       \
																				\
		/* non-constant fog comes from several sources */                       \
		else                                                                    \
		{                                                                       \
			INT32 fogblend = 0;                                                 \
																				\
			/* if fog_add is zero, we start with the fog color */               \
			if (FOGMODE_FOG_ADD(FOGMODE) == 0)                                  \
			{                                                                   \
				fr = fogcolor.rgb.r;                                            \
				fg = fogcolor.rgb.g;                                            \
				fb = fogcolor.rgb.b;                                            \
			}                                                                   \
			else                                                                \
				fr = fg = fb = 0;                                               \
																				\
			/* if fog_mult is zero, we subtract the incoming color */           \
			if (FOGMODE_FOG_MULT(FOGMODE) == 0)                                 \
			{                                                                   \
				fr -= (RR);                                                     \
				fg -= (GG);                                                     \
				fb -= (BB);                                                     \
			}                                                                   \
																				\
			/* fog blending mode */                                             \
			switch (FOGMODE_FOG_ZALPHA(FOGMODE))                                \
			{                                                                   \
				case 0:     /* fog table */                                     \
				{                                                               \
					INT32 delta = (VV)->fbi.fogdelta[fogdepth >> 10];             \
					INT32 deltaval;                                             \
																				\
					/* perform the multiply against lower 8 bits of wfloat */   \
					deltaval = (delta & (VV)->fbi.fogdelta_mask) *              \
								((fogdepth >> 2) & 0xff);                         \
																				\
					/* fog zones allow for negating this value */               \
					if (FOGMODE_FOG_ZONES(FOGMODE) && (delta & 2))              \
						deltaval = -deltaval;                                   \
					deltaval >>= 6;                                             \
																				\
					/* apply dither */                                          \
					if (FOGMODE_FOG_DITHER(FOGMODE))                            \
						deltaval += DITHER4[(XX) & 3];                          \
					deltaval >>= 4;                                             \
																				\
					/* add to the blending factor */                            \
					fogblend = (VV)->fbi.fogblend[fogdepth >> 10] + deltaval;     \
					break;                                                      \
				}                                                               \
																				\
				case 1:     /* iterated A */                                    \
					fogblend = ITERAXXX.rgb.a;                                  \
					break;                                                      \
																				\
				case 2:     /* iterated Z */                                    \
					CLAMPED_Z((ITERZ), FBZCP, fogblend);                        \
					fogblend >>= 8;                                             \
					break;                                                      \
																				\
				case 3:     /* iterated W - Voodoo 2 only */                    \
					CLAMPED_W((ITERW), FBZCP, fogblend);                        \
					break;                                                      \
			}                                                                   \
																				\
			/* perform the blend */                                             \
			fogblend++;                                                         \
			fr = (fr * fogblend) >> 8;                                          \
			fg = (fg * fogblend) >> 8;                                          \
			fb = (fb * fogblend) >> 8;                                          \
		}                                                                       \
																				\
		/* if fog_mult is 0, we add this to the original color */               \
		if (FOGMODE_FOG_MULT(FOGMODE) == 0)                                     \
		{                                                                       \
			(RR) += fr;                                                         \
			(GG) += fg;                                                         \
			(BB) += fb;                                                         \
		}                                                                       \
																				\
		/* otherwise this just becomes the new color */                         \
		else                                                                    \
		{                                                                       \
			(RR) = fr;                                                          \
			(GG) = fg;                                                          \
			(BB) = fb;                                                          \
		}                                                                       \
																				\
		/* clamp */                                                             \
		CLAMP((RR), 0x00, 0xff);                                                \
		CLAMP((GG), 0x00, 0xff);                                                \
		CLAMP((BB), 0x00, 0xff);                                                \
	}                                                                           \
}                                                                               \
while (0)

static inline void ATTR_FORCE_INLINE applyFogging(voodoo_state *v, UINT32 fogModeReg, UINT32 fbzCpReg,  INT32 x, const UINT8 *dither4, INT32 fogDepth,
	rgbaint_t &color, INT32 iterz, INT64 iterw, UINT8 itera)
{
	if (FOGMODE_ENABLE_FOG(fogModeReg))
	{
		UINT32 color_alpha = color.get_a();

		/* constant fog bypasses everything else */
		rgbaint_t fogColorLocal(v->reg[fogColor].u);

		if (FOGMODE_FOG_CONSTANT(fogModeReg))
		{
			/* if fog_mult is 0, we add this to the original color */
			if (FOGMODE_FOG_MULT(fogModeReg) == 0)
			{
				fogColorLocal.add(color);
				fogColorLocal.clamp_to_uint8();
				//color += fog;
			}

			/* otherwise this just becomes the new color */
			else
			{
				//color = fogColorLocal;
				//color = fog;
			}
		}
		/* non-constant fog comes from several sources */
		else
		{
			INT32 fogblend = 0;

			/* if fog_add is zero, we start with the fog color */
			if (FOGMODE_FOG_ADD(fogModeReg))
				fogColorLocal.set(0, 0, 0, 0);
				//fr = fg = fb = 0;

			/* if fog_mult is zero, we subtract the incoming color */
			if (!FOGMODE_FOG_MULT(fogModeReg))
			{
				fogColorLocal.sub(color);
				//fog.rgb -= color.rgb;
				//fr -= (RR);
				//fg -= (GG);
				//fb -= (BB);
			}

			/* fog blending mode */
			switch (FOGMODE_FOG_ZALPHA(fogModeReg))
			{
				case 0:     /* fog table */
				{
					INT32 delta = v->fbi.fogdelta[fogDepth >> 10];
					INT32 deltaval;

					/* perform the multiply against lower 8 bits of wfloat */
					deltaval = (delta & v->fbi.fogdelta_mask) *
								((fogDepth >> 2) & 0xff);

					/* fog zones allow for negating this value */
					if (FOGMODE_FOG_ZONES(fogModeReg) && (delta & 2))
						deltaval = -deltaval;
					deltaval >>= 6;

					/* apply dither */
					if (FOGMODE_FOG_DITHER(fogModeReg))
						deltaval += dither4[x&3];
					deltaval >>= 4;

					/* add to the blending factor */
					fogblend = v->fbi.fogblend[fogDepth >> 10] + deltaval;
					break;
				}

				case 1:     /* iterated A */
					fogblend = itera;
					break;

				case 2:     /* iterated Z */
					CLAMPED_Z(iterz, fbzCpReg, fogblend);
					fogblend >>= 8;
					break;

				case 3:     /* iterated W - Voodoo 2 only */
					CLAMPED_W(iterw, fbzCpReg, fogblend);
					break;
			}

			/* perform the blend */
			fogblend++;

			//fr = (fr * fogblend) >> 8;
			//fg = (fg * fogblend) >> 8;
			//fb = (fb * fogblend) >> 8;
			/* if fog_mult is 0, we add this to the original color */
			if (FOGMODE_FOG_MULT(fogModeReg) == 0)
			{
				fogColorLocal.scale_imm_add_and_clamp(fogblend, color);
				//color += fog;
				//(RR) += fr;
				//(GG) += fg;
				//(BB) += fb;
			}

			/* otherwise this just becomes the new color */
			else
			{
				fogColorLocal.scale_imm_and_clamp(fogblend);
				//color = fog;
				//(RR) = fr;
				//(GG) = fg;
				//(BB) = fb;
			}
		}


		/* clamp */
		//CLAMP((RR), 0x00, 0xff);
		//CLAMP((GG), 0x00, 0xff);
		//CLAMP((BB), 0x00, 0xff);
		fogColorLocal.set_a(color_alpha);
		color.set(fogColorLocal);
	}
}


/*************************************
 *
 *  Texture pipeline macro
 *
 *************************************/

#define TEXTURE_PIPELINE(TT, XX, DITHER4, TEXMODE, COTHER, LOOKUP, LODBASE, ITERS, ITERT, ITERW, RESULT) \
do                                                                              \
{                                                                               \
	INT32 blendr, blendg, blendb, blenda;                                       \
	INT32 tr, tg, tb, ta;                                                       \
	INT32 s, t, lod, ilod;                                                 \
	INT32 smax, tmax;                                                           \
	UINT32 texbase;                                                             \
	rgb_union c_local;                                                          \
																				\
	/* determine the S/T/LOD values for this texture */                         \
	if (TEXMODE_ENABLE_PERSPECTIVE(TEXMODE))                                    \
	{                                                                           \
		if (USE_FAST_RECIP) {                                                     \
			const INT32 oow = fast_reciplog((ITERW), &lod);                         \
			s = ((INT64)oow * (ITERS)) >> 29;                                       \
			t = ((INT64)oow * (ITERT)) >> 29;                                       \
		} else {                                                                  \
				multi_reciplog(ITERS, ITERT, ITERW, lod, s, t);                      \
		}                                                                       \
		lod += (LODBASE);                                                       \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		s = (ITERS) >> 14;                                                      \
		t = (ITERT) >> 14;                                                      \
		lod = (LODBASE);                                                        \
	}                                                                           \
																				\
	/* clamp W */                                                               \
	if (TEXMODE_CLAMP_NEG_W(TEXMODE) && (ITERW) < 0)                            \
		s = t = 0;                                                              \
																				\
	/* clamp the LOD */                                                         \
	lod += (TT)->lodbias;                                                       \
	if (TEXMODE_ENABLE_LOD_DITHER(TEXMODE))                                     \
		lod += DITHER4[(XX) & 3] << 4;                                          \
	if (lod < (TT)->lodmin)                                                     \
		lod = (TT)->lodmin;                                                     \
	if (lod > (TT)->lodmax)                                                     \
		lod = (TT)->lodmax;                                                     \
																				\
	/* now the LOD is in range; if we don't own this LOD, take the next one */  \
	ilod = lod >> 8;                                                            \
	if (!(((TT)->lodmask >> ilod) & 1))                                         \
		ilod++;                                                                 \
																				\
	/* fetch the texture base */                                                \
	texbase = (TT)->lodoffset[ilod];                                            \
																				\
	/* compute the maximum s and t values at this LOD */                        \
	smax = (TT)->wmask >> ilod;                                                 \
	tmax = (TT)->hmask >> ilod;                                                 \
																				\
	/* determine whether we are point-sampled or bilinear */                    \
	if ((lod == (TT)->lodmin && !TEXMODE_MAGNIFICATION_FILTER(TEXMODE)) ||      \
		(lod != (TT)->lodmin && !TEXMODE_MINIFICATION_FILTER(TEXMODE)))         \
	{                                                                           \
		/* point sampled */                                                     \
																				\
		UINT32 texel0;                                                          \
																				\
		/* adjust S/T for the LOD and strip off the fractions */                \
		s >>= ilod + 18;                                                        \
		t >>= ilod + 18;                                                        \
																				\
		/* clamp/wrap S/T if necessary */                                       \
		if (TEXMODE_CLAMP_S(TEXMODE))                                           \
			CLAMP(s, 0, smax);                                                  \
		if (TEXMODE_CLAMP_T(TEXMODE))                                           \
			CLAMP(t, 0, tmax);                                                  \
		s &= smax;                                                              \
		t &= tmax;                                                              \
		t *= smax + 1;                                                          \
																				\
		/* fetch texel data */                                                  \
		if (TEXMODE_FORMAT(TEXMODE) < 8)                                        \
		{                                                                       \
			texel0 = *(UINT8 *)&(TT)->ram[(texbase + t + s) & (TT)->mask];      \
			c_local.u = (LOOKUP)[texel0];                                       \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			texel0 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t + s)) & (TT)->mask]; \
			if (TEXMODE_FORMAT(TEXMODE) >= 10 && TEXMODE_FORMAT(TEXMODE) <= 12) \
				c_local.u = (LOOKUP)[texel0];                                   \
			else                                                                \
				c_local.u = ((LOOKUP)[texel0 & 0xff] & 0xffffff) |              \
							((texel0 & 0xff00) << 16);                          \
		}                                                                       \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		/* bilinear filtered */                                                 \
																				\
		UINT32 texel0, texel1, texel2, texel3;                                  \
		UINT32 sfrac, tfrac;                                                    \
		INT32 s1, t1;                                                           \
																				\
		/* adjust S/T for the LOD and strip off all but the low 8 bits of */    \
		/* the fraction */                                                      \
		s >>= ilod + 10;                                                        \
		t >>= ilod + 10;                                                        \
																				\
		/* also subtract 1/2 texel so that (0.5,0.5) = a full (0,0) texel */    \
		s -= 0x80;                                                              \
		t -= 0x80;                                                              \
																				\
		/* extract the fractions */                                             \
		sfrac = s & (TT)->bilinear_mask;                                        \
		tfrac = t & (TT)->bilinear_mask;                                        \
																				\
		/* now toss the rest */                                                 \
		s >>= 8;                                                                \
		t >>= 8;                                                                \
		s1 = s + 1;                                                             \
		t1 = t + 1;                                                             \
																				\
		/* clamp/wrap S/T if necessary */                                       \
		if (TEXMODE_CLAMP_S(TEXMODE))                                           \
		{                                                                       \
			CLAMP(s, 0, smax);                                                  \
			CLAMP(s1, 0, smax);                                                 \
		}                                                                       \
		if (TEXMODE_CLAMP_T(TEXMODE))                                           \
		{                                                                       \
			CLAMP(t, 0, tmax);                                                  \
			CLAMP(t1, 0, tmax);                                                 \
		}                                                                       \
		s &= smax;                                                              \
		s1 &= smax;                                                             \
		t &= tmax;                                                              \
		t1 &= tmax;                                                             \
		t *= smax + 1;                                                          \
		t1 *= smax + 1;                                                         \
																				\
		/* fetch texel data */                                                  \
		if (TEXMODE_FORMAT(TEXMODE) < 8)                                        \
		{                                                                       \
			texel0 = *(UINT8 *)&(TT)->ram[(texbase + t + s) & (TT)->mask];      \
			texel1 = *(UINT8 *)&(TT)->ram[(texbase + t + s1) & (TT)->mask];     \
			texel2 = *(UINT8 *)&(TT)->ram[(texbase + t1 + s) & (TT)->mask];     \
			texel3 = *(UINT8 *)&(TT)->ram[(texbase + t1 + s1) & (TT)->mask];    \
			texel0 = (LOOKUP)[texel0];                                          \
			texel1 = (LOOKUP)[texel1];                                          \
			texel2 = (LOOKUP)[texel2];                                          \
			texel3 = (LOOKUP)[texel3];                                          \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			texel0 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t + s)) & (TT)->mask]; \
			texel1 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t + s1)) & (TT)->mask];\
			texel2 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t1 + s)) & (TT)->mask];\
			texel3 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t1 + s1)) & (TT)->mask];\
			if (TEXMODE_FORMAT(TEXMODE) >= 10 && TEXMODE_FORMAT(TEXMODE) <= 12) \
			{                                                                   \
				texel0 = (LOOKUP)[texel0];                                      \
				texel1 = (LOOKUP)[texel1];                                      \
				texel2 = (LOOKUP)[texel2];                                      \
				texel3 = (LOOKUP)[texel3];                                      \
			}                                                                   \
			else                                                                \
			{                                                                   \
				texel0 = ((LOOKUP)[texel0 & 0xff] & 0xffffff) |                 \
							((texel0 & 0xff00) << 16);                          \
				texel1 = ((LOOKUP)[texel1 & 0xff] & 0xffffff) |                 \
							((texel1 & 0xff00) << 16);                          \
				texel2 = ((LOOKUP)[texel2 & 0xff] & 0xffffff) |                 \
							((texel2 & 0xff00) << 16);                          \
				texel3 = ((LOOKUP)[texel3 & 0xff] & 0xffffff) |                 \
							((texel3 & 0xff00) << 16);                          \
			}                                                                   \
		}                                                                       \
																				\
		/* weigh in each texel */                                               \
		c_local.u = rgbaint_t::bilinear_filter(texel0, texel1, texel2, texel3, sfrac, tfrac); \
	}                                                                           \
																				\
	/* select zero/other for RGB */                                             \
	if (!TEXMODE_TC_ZERO_OTHER(TEXMODE))                                        \
	{                                                                           \
		tr = COTHER.rgb.r;                                                      \
		tg = COTHER.rgb.g;                                                      \
		tb = COTHER.rgb.b;                                                      \
	}                                                                           \
	else                                                                        \
		tr = tg = tb = 0;                                                       \
																				\
	/* select zero/other for alpha */                                           \
	if (!TEXMODE_TCA_ZERO_OTHER(TEXMODE))                                       \
		ta = COTHER.rgb.a;                                                      \
	else                                                                        \
		ta = 0;                                                                 \
																				\
	/* potentially subtract c_local */                                          \
	if (TEXMODE_TC_SUB_CLOCAL(TEXMODE))                                         \
	{                                                                           \
		tr -= c_local.rgb.r;                                                    \
		tg -= c_local.rgb.g;                                                    \
		tb -= c_local.rgb.b;                                                    \
	}                                                                           \
	if (TEXMODE_TCA_SUB_CLOCAL(TEXMODE))                                        \
		ta -= c_local.rgb.a;                                                    \
																				\
	/* blend RGB */                                                             \
	switch (TEXMODE_TC_MSELECT(TEXMODE))                                        \
	{                                                                           \
		default:    /* reserved */                                              \
		case 0:     /* zero */                                                  \
			blendr = blendg = blendb = 0;                                       \
			break;                                                              \
																				\
		case 1:     /* c_local */                                               \
			blendr = c_local.rgb.r;                                             \
			blendg = c_local.rgb.g;                                             \
			blendb = c_local.rgb.b;                                             \
			break;                                                              \
																				\
		case 2:     /* a_other */                                               \
			blendr = blendg = blendb = COTHER.rgb.a;                            \
			break;                                                              \
																				\
		case 3:     /* a_local */                                               \
			blendr = blendg = blendb = c_local.rgb.a;                           \
			break;                                                              \
																				\
		case 4:     /* LOD (detail factor) */                                   \
			if ((TT)->detailbias <= lod)                                        \
				blendr = blendg = blendb = 0;                                   \
			else                                                                \
			{                                                                   \
				blendr = ((((TT)->detailbias - lod) << (TT)->detailscale) >> 8);\
				if (blendr > (TT)->detailmax)                                   \
					blendr = (TT)->detailmax;                                   \
				blendg = blendb = blendr;                                       \
			}                                                                   \
			break;                                                              \
																				\
		case 5:     /* LOD fraction */                                          \
			blendr = blendg = blendb = lod & 0xff;                              \
			break;                                                              \
	}                                                                           \
																				\
	/* blend alpha */                                                           \
	switch (TEXMODE_TCA_MSELECT(TEXMODE))                                       \
	{                                                                           \
		default:    /* reserved */                                              \
		case 0:     /* zero */                                                  \
			blenda = 0;                                                         \
			break;                                                              \
																				\
		case 1:     /* c_local */                                               \
			blenda = c_local.rgb.a;                                             \
			break;                                                              \
																				\
		case 2:     /* a_other */                                               \
			blenda = COTHER.rgb.a;                                              \
			break;                                                              \
																				\
		case 3:     /* a_local */                                               \
			blenda = c_local.rgb.a;                                             \
			break;                                                              \
																				\
		case 4:     /* LOD (detail factor) */                                   \
			if ((TT)->detailbias <= lod)                                        \
				blenda = 0;                                                     \
			else                                                                \
			{                                                                   \
				blenda = ((((TT)->detailbias - lod) << (TT)->detailscale) >> 8);\
				if (blenda > (TT)->detailmax)                                   \
					blenda = (TT)->detailmax;                                   \
			}                                                                   \
			break;                                                              \
																				\
		case 5:     /* LOD fraction */                                          \
			blenda = lod & 0xff;                                                \
			break;                                                              \
	}                                                                           \
																				\
	/* reverse the RGB blend */                                                 \
	if (!TEXMODE_TC_REVERSE_BLEND(TEXMODE))                                     \
	{                                                                           \
		blendr ^= 0xff;                                                         \
		blendg ^= 0xff;                                                         \
		blendb ^= 0xff;                                                         \
	}                                                                           \
																				\
	/* reverse the alpha blend */                                               \
	if (!TEXMODE_TCA_REVERSE_BLEND(TEXMODE))                                    \
		blenda ^= 0xff;                                                         \
																				\
	/* do the blend */                                                          \
	tr = (tr * (blendr + 1)) >> 8;                                              \
	tg = (tg * (blendg + 1)) >> 8;                                              \
	tb = (tb * (blendb + 1)) >> 8;                                              \
	ta = (ta * (blenda + 1)) >> 8;                                              \
																				\
	/* add clocal or alocal to RGB */                                           \
	switch (TEXMODE_TC_ADD_ACLOCAL(TEXMODE))                                    \
	{                                                                           \
		case 3:     /* reserved */                                              \
		case 0:     /* nothing */                                               \
			break;                                                              \
																				\
		case 1:     /* add c_local */                                           \
			tr += c_local.rgb.r;                                                \
			tg += c_local.rgb.g;                                                \
			tb += c_local.rgb.b;                                                \
			break;                                                              \
																				\
		case 2:     /* add_alocal */                                            \
			tr += c_local.rgb.a;                                                \
			tg += c_local.rgb.a;                                                \
			tb += c_local.rgb.a;                                                \
			break;                                                              \
	}                                                                           \
																				\
	/* add clocal or alocal to alpha */                                         \
	if (TEXMODE_TCA_ADD_ACLOCAL(TEXMODE))                                       \
		ta += c_local.rgb.a;                                                    \
																				\
	/* clamp */                                                                 \
	RESULT.rgb.r = (tr < 0) ? 0 : (tr > 0xff) ? 0xff : tr;                      \
	RESULT.rgb.g = (tg < 0) ? 0 : (tg > 0xff) ? 0xff : tg;                      \
	RESULT.rgb.b = (tb < 0) ? 0 : (tb > 0xff) ? 0xff : tb;                      \
	RESULT.rgb.a = (ta < 0) ? 0 : (ta > 0xff) ? 0xff : ta;                      \
																				\
	/* invert */                                                                \
	if (TEXMODE_TC_INVERT_OUTPUT(TEXMODE))                                      \
		RESULT.u ^= 0x00ffffff;                                                 \
	if (TEXMODE_TCA_INVERT_OUTPUT(TEXMODE))                                     \
		RESULT.rgb.a ^= 0xff;                                                   \
}                                                                               \
while (0)



/*************************************
 *
 *  Pixel pipeline macros
 *
 *************************************/

#define PIXEL_PIPELINE_BEGIN(VV, STATS, XX, YY, FBZCOLORPATH, FBZMODE, ITERZ, ITERW)    \
do                                                                              \
{                                                                               \
	INT32 depthval, wfloat, fogdepth, biasdepth;                                  \
	INT32 r, g, b, a;                                                           \
																				\
	(STATS)->pixels_in++;                                                       \
																				\
	/* apply clipping */                                                        \
	/* note that for perf reasons, we assume the caller has done clipping */    \
																				\
	/* handle stippling */                                                      \
	if (FBZMODE_ENABLE_STIPPLE(FBZMODE))                                        \
	{                                                                           \
		/* rotate mode */                                                       \
		if (FBZMODE_STIPPLE_PATTERN(FBZMODE) == 0)                              \
		{                                                                       \
			(VV)->reg[stipple].u = ((VV)->reg[stipple].u << 1) | ((VV)->reg[stipple].u >> 31);\
			if (((VV)->reg[stipple].u & 0x80000000) == 0)                       \
			{                                                                   \
				(VV)->stats.total_stippled++;                                   \
				goto skipdrawdepth;                                             \
			}                                                                   \
		}                                                                       \
																				\
		/* pattern mode */                                                      \
		else                                                                    \
		{                                                                       \
			int stipple_index = (((YY) & 3) << 3) | (~(XX) & 7);                \
			if ((((VV)->reg[stipple].u >> stipple_index) & 1) == 0)             \
			{                                                                   \
				(VV)->stats.total_stippled++;                                   \
				goto skipdrawdepth;                                             \
			}                                                                   \
		}                                                                       \
	}                                                                           \
																				\
	/* compute "floating point" W value (used for depth and fog) */             \
	if ((ITERW) & U64(0xffff00000000))                                          \
		wfloat = 0x0000;                                                        \
	else                                                                        \
	{                                                                           \
		UINT32 temp = (UINT32)(ITERW);                                \
		if (!(temp & 0xffff0000))                                           \
			wfloat = 0xffff;                                                    \
		else                                                                    \
		{                                                                       \
			int exp = count_leading_zeros(temp);                                \
			wfloat = ((exp << 12) | ((~temp >> (19 - exp)) & 0xfff)) + 1;       \
		}                                                                       \
	}                                                                           \
	fogdepth = wfloat;                                                         \
	/* add the bias for fog selection*/                                         \
	if (FBZMODE_ENABLE_DEPTH_BIAS(FBZMODE))                                     \
	{                                                                           \
		fogdepth += (INT16)(VV)->reg[zaColor].u;                                \
		CLAMP(fogdepth, 0, 0xffff);                                             \
	}                                                                           \
																				\
	/* compute depth value (W or Z) for this pixel */                           \
	if (FBZMODE_WBUFFER_SELECT(FBZMODE) == 0)                                   \
	{                                                                           \
		CLAMPED_Z(ITERZ, FBZCOLORPATH, depthval);                               \
	}                                                                           \
	else if (FBZMODE_DEPTH_FLOAT_SELECT(FBZMODE) == 0)                          \
		depthval = wfloat;                                                      \
	else                                                                        \
	{                                                                           \
		if ((ITERZ) & 0xf0000000)                                               \
			depthval = 0x0000;                                                  \
		else                                                                    \
		{                                                                       \
			UINT32 temp = (ITERZ << 4);                             \
			if (!(temp & 0xffff0000))                                                           \
				depthval = 0xffff;                                              \
			else                                                                \
			{                                                                   \
				int exp = count_leading_zeros(temp);                            \
				depthval = ((exp << 12) | ((~temp >> (19 - exp)) & 0xfff)) + 1; \
			}                                                                   \
		}                                                                       \
	}                                                                            \
	/* add the bias */                                                          \
	biasdepth = depthval;                                                     \
	if (FBZMODE_ENABLE_DEPTH_BIAS(FBZMODE))                                     \
	{                                                                           \
		biasdepth += (INT16)(VV)->reg[zaColor].u;                                \
		CLAMP(biasdepth, 0, 0xffff);                                             \
	}


#define DEPTH_TEST(VV, STATS, XX, FBZMODE)    \
do                                                                              \
{                                                                               \
	/* handle depth buffer testing */                                           \
	if (FBZMODE_ENABLE_DEPTHBUF(FBZMODE))                                       \
	{                                                                           \
		INT32 depthsource;                                                      \
																				\
		/* the source depth is either the iterated W/Z+bias or a */             \
		/* constant value */                                                    \
		if (FBZMODE_DEPTH_SOURCE_COMPARE(FBZMODE) == 0)                         \
			depthsource = biasdepth;                                             \
		else                                                                    \
			depthsource = (UINT16)(VV)->reg[zaColor].u;                         \
																				\
		/* test against the depth buffer */                                     \
		switch (FBZMODE_DEPTH_FUNCTION(FBZMODE))                                \
		{                                                                       \
			case 0:     /* depthOP = never */                                   \
				(STATS)->zfunc_fail++;                                          \
				goto skipdrawdepth;                                             \
																				\
			case 1:     /* depthOP = less than */                               \
				if (depthsource >= depth[XX])                                   \
				{                                                               \
					(STATS)->zfunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 2:     /* depthOP = equal */                                   \
				if (depthsource != depth[XX])                                   \
				{                                                               \
					(STATS)->zfunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 3:     /* depthOP = less than or equal */                      \
				if (depthsource > depth[XX])                                    \
				{                                                               \
					(STATS)->zfunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 4:     /* depthOP = greater than */                            \
				if (depthsource <= depth[XX])                                   \
				{                                                               \
					(STATS)->zfunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 5:     /* depthOP = not equal */                               \
				if (depthsource == depth[XX])                                   \
				{                                                               \
					(STATS)->zfunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 6:     /* depthOP = greater than or equal */                   \
				if (depthsource < depth[XX])                                    \
				{                                                               \
					(STATS)->zfunc_fail++;                                      \
					goto skipdrawdepth;                                         \
				}                                                               \
				break;                                                          \
																				\
			case 7:     /* depthOP = always */                                  \
				break;                                                          \
		}                                                                       \
	}                                                                       \
}                                                                               \
while (0)

static inline bool ATTR_FORCE_INLINE depthTest(UINT16 zaColorReg, stats_block *stats, INT32 destDepth, UINT32 fbzModeReg, INT32 biasdepth)
{
	/* handle depth buffer testing */
	if (FBZMODE_ENABLE_DEPTHBUF(fbzModeReg))
	{
		INT32 depthsource;

		/* the source depth is either the iterated W/Z+bias or a */
		/* constant value */
		if (FBZMODE_DEPTH_SOURCE_COMPARE(fbzModeReg) == 0)
			depthsource = biasdepth;
		else
			depthsource = zaColorReg;

		/* test against the depth buffer */
		switch (FBZMODE_DEPTH_FUNCTION(fbzModeReg))
		{
			case 0:     /* depthOP = never */
				stats->zfunc_fail++;
				return false;

			case 1:     /* depthOP = less than */
				if (depthsource >= destDepth)
				{
					stats->zfunc_fail++;
					return false;
				}
				break;

			case 2:     /* depthOP = equal */
				if (depthsource != destDepth)
				{
					stats->zfunc_fail++;
					return false;
				}
				break;

			case 3:     /* depthOP = less than or equal */
				if (depthsource > destDepth)
				{
					stats->zfunc_fail++;
					return false;
				}
				break;

			case 4:     /* depthOP = greater than */
				if (depthsource <= destDepth)
				{
					stats->zfunc_fail++;
					return false;
				}
				break;

			case 5:     /* depthOP = not equal */
				if (depthsource == destDepth)
				{
					stats->zfunc_fail++;
					return false;
				}
				break;

			case 6:     /* depthOP = greater than or equal */
				if (depthsource < destDepth)
				{
					stats->zfunc_fail++;
					return false;
				}
				break;

			case 7:     /* depthOP = always */
				break;
		}
	}
	return true;
}

#if USE_OLD_RASTER == 1
#define PIXEL_PIPELINE_END(VV, STATS, DITHER, DITHER4, DITHER_LOOKUP, XX, dest, depth, FBZMODE, FBZCOLORPATH, ALPHAMODE, FOGMODE, ITERZ, ITERW, ITERAXXX) \
																				\
	/* perform fogging */                                                       \
	INT32 prefogr, prefogg, prefogb;                                            \
	prefogr = r;                                                                \
	prefogg = g;                                                                \
	prefogb = b;                                                                \
	APPLY_FOGGING(VV, FOGMODE, FBZCOLORPATH, XX, DITHER4, r, g, b,              \
					ITERZ, ITERW, ITERAXXX);                                    \
																					\
		/* perform alpha blending */                                                \
		APPLY_ALPHA_BLEND(FBZMODE, ALPHAMODE, XX, DITHER, r, g, b, a);              \
	/* modify the pixel for debugging purposes */                               \
	MODIFY_PIXEL(VV);                                                           \
																				\
	/* write to framebuffer */                                                  \
	if (FBZMODE_RGB_BUFFER_MASK(FBZMODE))                                       \
	{                                                                           \
		/* apply dithering */                                                   \
		APPLY_DITHER(FBZMODE, XX, DITHER_LOOKUP, r, g, b);                      \
		dest[XX] = (r << 11) | (g << 5) | b;                                    \
	}                                                                           \
																				\
	/* write to aux buffer */                                                   \
	if (depth && FBZMODE_AUX_BUFFER_MASK(FBZMODE))                              \
	{                                                                           \
		if (FBZMODE_ENABLE_ALPHA_PLANES(FBZMODE) == 0)                          \
			depth[XX] = biasdepth;                                               \
		else                                                                    \
			depth[XX] = a;                                                      \
	}                                                                           \
																				\
	/* track pixel writes to the frame buffer regardless of mask */             \
	(STATS)->pixels_out++;                                                      \
																				\
skipdrawdepth:                                                                  \
	;                                                                           \
}                                                                               \
while (0)
#else
#define PIXEL_PIPELINE_END(VV, STATS, DITHER, DITHER4, DITHER_LOOKUP, XX, dest, depth, FBZMODE, FBZCOLORPATH, ALPHAMODE, FOGMODE, ITERZ, ITERW, ITERAXXX) \
																				\
	/* perform fogging */                                                       \
	preFog.set(color); \
	applyFogging(VV, FOGMODE, FBZCOLORPATH, XX, DITHER4, fogdepth, color, ITERZ, ITERW, ITERAXXX.get_a()); \
	/* perform alpha blending */                                                \
	alphaBlend(FBZMODE, ALPHAMODE, XX, DITHER, dest[XX], depth, preFog, color); \
	a = color.get_a(); r = color.get_r(); g = color.get_g(); b = color.get_b();                     \
	/* modify the pixel for debugging purposes */                               \
	MODIFY_PIXEL(VV);                                                           \
																				\
	/* write to framebuffer */                                                  \
	if (FBZMODE_RGB_BUFFER_MASK(FBZMODE))                                       \
	{                                                                           \
		/* apply dithering */                                                   \
		APPLY_DITHER(FBZMODE, XX, DITHER_LOOKUP, r, g, b);                      \
		dest[XX] = (r << 11) | (g << 5) | b;                                    \
	}                                                                           \
																				\
	/* write to aux buffer */                                                   \
	if (depth && FBZMODE_AUX_BUFFER_MASK(FBZMODE))                              \
	{                                                                           \
		if (FBZMODE_ENABLE_ALPHA_PLANES(FBZMODE) == 0)                          \
			depth[XX] = biasdepth;                                               \
		else                                                                    \
			depth[XX] = a;                                                      \
	}                                                                           \
																				\
	/* track pixel writes to the frame buffer regardless of mask */             \
	(STATS)->pixels_out++;                                                      \
																				\
skipdrawdepth:                                                                  \
	;                                                                           \
}                                                                               \
while (0)
#endif


/*************************************
 *
 *  Colorpath pipeline macro
 *
 *************************************/

/*

    c_other_is_used:

        if (FBZMODE_ENABLE_CHROMAKEY(FBZMODE) ||
            FBZCP_CC_ZERO_OTHER(FBZCOLORPATH) == 0)

    c_local_is_used:

        if (FBZCP_CC_SUB_CLOCAL(FBZCOLORPATH) ||
            FBZCP_CC_MSELECT(FBZCOLORPATH) == 1 ||
            FBZCP_CC_ADD_ACLOCAL(FBZCOLORPATH) == 1)

    NEEDS_ITER_RGB:

        if ((c_other_is_used && FBZCP_CC_RGBSELECT(FBZCOLORPATH) == 0) ||
            (c_local_is_used && (FBZCP_CC_LOCALSELECT_OVERRIDE(FBZCOLORPATH) != 0 || FBZCP_CC_LOCALSELECT(FBZCOLORPATH) == 0))

    NEEDS_ITER_A:

        if ((a_other_is_used && FBZCP_CC_ASELECT(FBZCOLORPATH) == 0) ||
            (a_local_is_used && FBZCP_CCA_LOCALSELECT(FBZCOLORPATH) == 0))

    NEEDS_ITER_Z:

        if (FBZMODE_WBUFFER_SELECT(FBZMODE) == 0 ||
            FBZMODE_DEPTH_FLOAT_SELECT(FBZMODE) != 0 ||
            FBZCP_CCA_LOCALSELECT(FBZCOLORPATH) == 2)


*/

/*
    Expects the following declarations to be outside of this scope:

    INT32 r, g, b, a;
*/
#define COLORPATH_PIPELINE(VV, STATS, FBZCOLORPATH, FBZMODE, ALPHAMODE, TEXELARGB, ITERZ, ITERW, ITERARGB) \
do                                                                              \
{                                                                               \
	INT32 blendr, blendg, blendb, blenda;                                       \
	rgb_union c_other;                                                          \
	rgb_union c_local;                                                          \
																				\
	/* compute c_other */                                                       \
	switch (FBZCP_CC_RGBSELECT(FBZCOLORPATH))                                   \
	{                                                                           \
		case 0:     /* iterated RGB */                                          \
			c_other.u = ITERARGB.u;                                             \
			break;                                                              \
																				\
		case 1:     /* texture RGB */                                           \
			c_other.u = TEXELARGB.u;                                            \
			break;                                                              \
																				\
		case 2:     /* color1 RGB */                                            \
			c_other.u = (VV)->reg[color1].u;                                    \
			break;                                                              \
																				\
		default:    /* reserved - voodoo3 framebufferRGB */                   \
			c_other.u = 0;                                                      \
			break;                                                              \
	}                                                                           \
																				\
	/* handle chroma key */                                                     \
	APPLY_CHROMAKEY(VV, STATS, FBZMODE, c_other);                               \
																				\
	/* compute a_other */                                                       \
	switch (FBZCP_CC_ASELECT(FBZCOLORPATH))                                     \
	{                                                                           \
		case 0:     /* iterated alpha */                                        \
			c_other.rgb.a = ITERARGB.rgb.a;                                     \
			break;                                                              \
																				\
		case 1:     /* texture alpha */                                         \
			c_other.rgb.a = TEXELARGB.rgb.a;                                    \
			break;                                                              \
																				\
		case 2:     /* color1 alpha */                                          \
			c_other.rgb.a = (VV)->reg[color1].rgb.a;                            \
			break;                                                              \
																				\
		default:    /* reserved */                                              \
			c_other.rgb.a = 0;                                                  \
			break;                                                              \
	}                                                                           \
																				\
	/* handle alpha mask */                                                     \
	APPLY_ALPHAMASK(VV, STATS, FBZMODE, c_other.rgb.a);                         \
																				\
	/* compute c_local */                                                       \
	if (FBZCP_CC_LOCALSELECT_OVERRIDE(FBZCOLORPATH) == 0)                       \
	{                                                                           \
		if (FBZCP_CC_LOCALSELECT(FBZCOLORPATH) == 0)    /* iterated RGB */      \
			c_local.u = ITERARGB.u;                                             \
		else                                            /* color0 RGB */        \
			c_local.u = (VV)->reg[color0].u;                                    \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		if (!(TEXELARGB.rgb.a & 0x80))                  /* iterated RGB */      \
			c_local.u = ITERARGB.u;                                             \
		else                                            /* color0 RGB */        \
			c_local.u = (VV)->reg[color0].u;                                    \
	}                                                                           \
																				\
	/* compute a_local */                                                       \
	switch (FBZCP_CCA_LOCALSELECT(FBZCOLORPATH))                                \
	{                                                                           \
		default:                                                                \
		case 0:     /* iterated alpha */                                        \
			c_local.rgb.a = ITERARGB.rgb.a;                                     \
			break;                                                              \
																				\
		case 1:     /* color0 alpha */                                          \
			c_local.rgb.a = (VV)->reg[color0].rgb.a;                            \
			break;                                                              \
																				\
		case 2:     /* clamped iterated Z[27:20] */                             \
		{                                                                       \
			int temp;                                                           \
			CLAMPED_Z(ITERZ, FBZCOLORPATH, temp);                               \
			c_local.rgb.a = (UINT8)temp;                                        \
			break;                                                              \
		}                                                                       \
																				\
		case 3:     /* clamped iterated W[39:32] */                             \
		{                                                                       \
			int temp;                                                           \
			CLAMPED_W(ITERW, FBZCOLORPATH, temp);           /* Voodoo 2 only */ \
			c_local.rgb.a = (UINT8)temp;                                        \
			break;                                                              \
		}                                                                       \
	}                                                                           \
																				\
	/* select zero or a_other */                                                \
	if (!FBZCP_CCA_ZERO_OTHER(FBZCOLORPATH))                                \
		a = c_other.rgb.a;                                                      \
	else                                                                        \
		a = 0;                                                                  \
																				\
	/* subtract a_local */                                                      \
	if (FBZCP_CCA_SUB_CLOCAL(FBZCOLORPATH))                                     \
		a -= c_local.rgb.a;                                                     \
																				\
	/* blend alpha */                                                           \
	switch (FBZCP_CCA_MSELECT(FBZCOLORPATH))                                    \
	{                                                                           \
		default:    /* reserved */                                              \
		case 0:     /* 0 */                                                     \
			blenda = 0;                                                         \
			break;                                                              \
																				\
		case 1:     /* a_local */                                               \
			blenda = c_local.rgb.a;                                             \
			break;                                                              \
																				\
		case 2:     /* a_other */                                               \
			blenda = c_other.rgb.a;                                             \
			break;                                                              \
																				\
		case 3:     /* a_local */                                               \
			blenda = c_local.rgb.a;                                             \
			break;                                                              \
																				\
		case 4:     /* texture alpha */                                         \
			blenda = TEXELARGB.rgb.a;                                           \
			break;                                                              \
	}                                                                           \
																				\
	/* reverse the alpha blend */                                               \
	if (!FBZCP_CCA_REVERSE_BLEND(FBZCOLORPATH))                                 \
		blenda ^= 0xff;                                                         \
																				\
	/* do the blend */                                                          \
	a = (a * (blenda + 1)) >> 8;                                                \
																				\
	/* add clocal or alocal to alpha */                                         \
	if (FBZCP_CCA_ADD_ACLOCAL(FBZCOLORPATH))                                    \
		a += c_local.rgb.a;                                                     \
																				\
	/* clamp */                                                                 \
	CLAMP(a, 0x00, 0xff);                                                       \
																				\
	/* invert */                                                                \
	if (FBZCP_CCA_INVERT_OUTPUT(FBZCOLORPATH))                                  \
		a ^= 0xff;                                                              \
																				\
	/* handle alpha test */                                                     \
	APPLY_ALPHATEST(VV, STATS, ALPHAMODE, a);                                   \
																				\
																				\
	/* select zero or c_other */                                                \
	if (FBZCP_CC_ZERO_OTHER(FBZCOLORPATH) == 0)                                 \
	{                                                                           \
		r = c_other.rgb.r;                                                      \
		g = c_other.rgb.g;                                                      \
		b = c_other.rgb.b;                                                      \
	}                                                                           \
	else                                                                        \
		r = g = b = 0;                                                          \
																				\
	/* subtract c_local */                                                      \
	if (FBZCP_CC_SUB_CLOCAL(FBZCOLORPATH))                                      \
	{                                                                           \
		r -= c_local.rgb.r;                                                     \
		g -= c_local.rgb.g;                                                     \
		b -= c_local.rgb.b;                                                     \
	}                                                                           \
																				\
	/* blend RGB */                                                             \
	switch (FBZCP_CC_MSELECT(FBZCOLORPATH))                                     \
	{                                                                           \
		default:    /* reserved */                                              \
		case 0:     /* 0 */                                                     \
			blendr = blendg = blendb = 0;                                       \
			break;                                                              \
																				\
		case 1:     /* c_local */                                               \
			blendr = c_local.rgb.r;                                             \
			blendg = c_local.rgb.g;                                             \
			blendb = c_local.rgb.b;                                             \
			break;                                                              \
																				\
		case 2:     /* a_other */                                               \
			blendr = blendg = blendb = c_other.rgb.a;                           \
			break;                                                              \
																				\
		case 3:     /* a_local */                                               \
			blendr = blendg = blendb = c_local.rgb.a;                           \
			break;                                                              \
																				\
		case 4:     /* texture alpha */                                         \
			blendr = blendg = blendb = TEXELARGB.rgb.a;                         \
			break;                                                              \
																				\
		case 5:     /* texture RGB (Voodoo 2 only) */                           \
			blendr = TEXELARGB.rgb.r;                                           \
			blendg = TEXELARGB.rgb.g;                                           \
			blendb = TEXELARGB.rgb.b;                                           \
			break;                                                              \
	}                                                                           \
																				\
	/* reverse the RGB blend */                                                 \
	if (!FBZCP_CC_REVERSE_BLEND(FBZCOLORPATH))                                  \
	{                                                                           \
		blendr ^= 0xff;                                                         \
		blendg ^= 0xff;                                                         \
		blendb ^= 0xff;                                                         \
	}                                                                           \
																				\
	/* do the blend */                                                          \
	r = (r * (blendr + 1)) >> 8;                                                \
	g = (g * (blendg + 1)) >> 8;                                                \
	b = (b * (blendb + 1)) >> 8;                                                \
																				\
	/* add clocal or alocal to RGB */                                           \
	switch (FBZCP_CC_ADD_ACLOCAL(FBZCOLORPATH))                                 \
	{                                                                           \
		case 3:     /* reserved */                                              \
		case 0:     /* nothing */                                               \
			break;                                                              \
																				\
		case 1:     /* add c_local */                                           \
			r += c_local.rgb.r;                                                 \
			g += c_local.rgb.g;                                                 \
			b += c_local.rgb.b;                                                 \
			break;                                                              \
																				\
		case 2:     /* add_alocal */                                            \
			r += c_local.rgb.a;                                                 \
			g += c_local.rgb.a;                                                 \
			b += c_local.rgb.a;                                                 \
			break;                                                              \
	}                                                                           \
																				\
	/* clamp */                                                                 \
	CLAMP(r, 0x00, 0xff);                                                       \
	CLAMP(g, 0x00, 0xff);                                                       \
	CLAMP(b, 0x00, 0xff);                                                       \
																				\
	/* invert */                                                                \
	if (FBZCP_CC_INVERT_OUTPUT(FBZCOLORPATH))                                   \
	{                                                                           \
		r ^= 0xff;                                                              \
		g ^= 0xff;                                                              \
		b ^= 0xff;                                                              \
	}                                                                           \
}                                                                               \
while (0)

static inline bool ATTR_FORCE_INLINE combineColor(voodoo_state *VV, stats_block *STATS, UINT32 FBZCOLORPATH, UINT32 FBZMODE, UINT32 ALPHAMODE,
													rgbaint_t TEXELARGB, INT32 ITERZ, INT64 ITERW, rgbaint_t &srcColor)
{
	rgbaint_t c_other;
	rgbaint_t c_local;

	/* compute c_other */
	switch (FBZCP_CC_RGBSELECT(FBZCOLORPATH))
	{
		case 0:     /* iterated RGB */
			c_other.set(srcColor);
			break;

		case 1:     /* texture RGB */
			c_other.set(TEXELARGB);
			break;

		case 2:     /* color1 RGB */
			c_other.set((VV)->reg[color1].u);
			break;

		default:    /* reserved - voodoo3 framebufferRGB */
			c_other.set(0);
			break;
	}

	/* handle chroma key */
	if (!chromaKeyTest(VV, STATS, FBZMODE, c_other))
		return false;
	//APPLY_CHROMAKEY(VV, STATS, FBZMODE, c_other);

	/* compute a_other */
	switch (FBZCP_CC_ASELECT(FBZCOLORPATH))
	{
		case 0:     /* iterated alpha */
			c_other.merge_alpha(srcColor);
			break;

		case 1:     /* texture alpha */
			c_other.merge_alpha(TEXELARGB);
			break;

		case 2:     /* color1 alpha */
			c_other.set_a((VV)->reg[color1].rgb.a);
			break;

		default:    /* reserved */
			c_other.set_a(0);
			break;
	}

	/* handle alpha mask */
	if (!alphaMaskTest(STATS, FBZMODE, c_other.get_a()))
		return false;
	//APPLY_ALPHAMASK(VV, STATS, FBZMODE, c_other.rgb.a);


	/* compute c_local */
	if (FBZCP_CC_LOCALSELECT_OVERRIDE(FBZCOLORPATH) == 0)
	{
		if (FBZCP_CC_LOCALSELECT(FBZCOLORPATH) == 0)    /* iterated RGB */
			c_local.set(srcColor);
		else                                            /* color0 RGB */
			c_local.set((VV)->reg[color0].u);
	}
	else
	{
		if (!(TEXELARGB.get_a() & 0x80))                  /* iterated RGB */
			c_local.set(srcColor);
		else                                            /* color0 RGB */
			c_local.set((VV)->reg[color0].u);
	}

	/* compute a_local */
	switch (FBZCP_CCA_LOCALSELECT(FBZCOLORPATH))
	{
		default:
		case 0:     /* iterated alpha */
			c_local.merge_alpha(srcColor);
			break;

		case 1:     /* color0 alpha */
			c_local.set_a((VV)->reg[color0].rgb.a);
			break;

		case 2:     /* clamped iterated Z[27:20] */
		{
			int temp;
			CLAMPED_Z(ITERZ, FBZCOLORPATH, temp);
			c_local.set_a((UINT8) temp);
			break;
		}

		case 3:     /* clamped iterated W[39:32] */
		{
			int temp;
			CLAMPED_W(ITERW, FBZCOLORPATH, temp);           /* Voodoo 2 only */
			c_local.set_a((UINT8) temp);
			break;
		}
	}

	UINT8 a_other = c_other.get_a();
	UINT8 a_local = c_local.get_a();
	UINT8 tmp;
	rgbaint_t add_val(c_local);

	/* select zero or c_other */
	if (FBZCP_CC_ZERO_OTHER(FBZCOLORPATH))
		c_other.and_imm_rgba(-1, 0, 0, 0);
		//r = g = b = 0;

	/* select zero or a_other */
	if (FBZCP_CCA_ZERO_OTHER(FBZCOLORPATH))
		c_other.set_a(0);

	/* subtract a/c_local */
	if (FBZCP_CC_SUB_CLOCAL(FBZCOLORPATH) || (FBZCP_CCA_SUB_CLOCAL(FBZCOLORPATH)))
	{
		rgbaint_t sub_val = c_local;

		if (!FBZCP_CC_SUB_CLOCAL(FBZCOLORPATH))
			sub_val.set(a_local, 0, 0, 0);

		if (!FBZCP_CCA_SUB_CLOCAL(FBZCOLORPATH))
			sub_val.set_a(0);

		c_other.sub(sub_val);
	}

	/* blend RGB */
	switch (FBZCP_CC_MSELECT(FBZCOLORPATH))
	{
		default:    /* reserved */
		case 0:     /* 0 */
			c_local.and_imm_rgba(-1, 0, 0, 0);
			break;

		case 1:     /* c_local */
			break;

		case 2:     /* a_other */
			c_local.set(a_local, a_other, a_other, a_other);
			break;

		case 3:     /* a_local */
			c_local.set(a_local, a_local, a_local, a_local);
			break;

		case 4:     /* texture alpha */
			tmp = TEXELARGB.get_a();
			c_local.set(a_local, tmp, tmp, tmp);
			break;

		case 5:     /* texture RGB (Voodoo 2 only) */
			c_local.set(TEXELARGB);
			break;
	}

	/* blend alpha */
	switch (FBZCP_CCA_MSELECT(FBZCOLORPATH))
	{
		default:    /* reserved */
		case 0:     /* 0 */
			c_local.set_a(0);
			break;

		case 1:     /* a_local */
		case 3:     /* a_local */
			c_local.set_a(a_local);
			break;

		case 2:     /* a_other */
			c_local.set_a(a_other);
			break;

		case 4:     /* texture alpha */
				c_local.merge_alpha(TEXELARGB);
			break;
	}

	/* reverse the RGB blend */
	if (!FBZCP_CC_REVERSE_BLEND(FBZCOLORPATH))
		c_local.xor_imm_rgba(0, 0xff, 0xff, 0xff);

	/* reverse the alpha blend */
	if (!FBZCP_CCA_REVERSE_BLEND(FBZCOLORPATH))
		c_local.xor_imm_rgba(0xff, 0, 0, 0);

	/* do the blend */
	//color.rgb.a = (color.rgb.a * (blenda + 1)) >> 8;
	//color.rgb.r = (color.rgb.r * (blendr + 1)) >> 8;
	//color.rgb.g = (color.rgb.g * (blendg + 1)) >> 8;
	//color.rgb.b = (color.rgb.b * (blendb + 1)) >> 8;

	/* add clocal or alocal to RGB */
	switch (FBZCP_CC_ADD_ACLOCAL(FBZCOLORPATH))
	{
		case 3:     /* reserved */
		case 0:     /* nothing */
			add_val.set(a_local, 0, 0, 0);
			break;

		case 1:     /* add c_local */
			break;

		case 2:     /* add_alocal */
			add_val.set(a_local, a_local, a_local, a_local);
			break;
	}

	/* add clocal or alocal to alpha */
	if (!FBZCP_CCA_ADD_ACLOCAL(FBZCOLORPATH))
		add_val.set_a(0);
		//color.rgb.a += c_local.rgb.a;

	/* clamp */
	//CLAMP(color.rgb.a, 0x00, 0xff);
	//CLAMP(color.rgb.r, 0x00, 0xff);
	//CLAMP(color.rgb.g, 0x00, 0xff);
	//CLAMP(color.rgb.b, 0x00, 0xff);
	c_local.add_imm(1);
	c_other.scale_add_and_clamp(c_local, add_val);
	srcColor.set(c_other);

	/* invert */
	if (FBZCP_CCA_INVERT_OUTPUT(FBZCOLORPATH))
		srcColor.xor_imm_rgba(0xff, 0, 0, 0);
	/* invert */
	if (FBZCP_CC_INVERT_OUTPUT(FBZCOLORPATH))
		srcColor.xor_imm_rgba(0, 0xff, 0xff, 0xff);


	/* handle alpha test */
	if (!alphaTest(VV, STATS, ALPHAMODE, srcColor.get_a()))
		return false;
	//APPLY_ALPHATEST(VV, STATS, ALPHAMODE, color.rgb.a);

	return true;
}



/*************************************
 *
 *  Rasterizer generator macro
 *
 *************************************/
#if USE_OLD_RASTER == 1
#define RASTERIZER(name, TMUS, FBZCOLORPATH, FBZMODE, ALPHAMODE, FOGMODE, TEXMODE0, TEXMODE1) \
																				\
static void raster_##name(void *destbase, INT32 y, const poly_extent *extent, const void *extradata, int threadid) \
{                                                                               \
	const poly_extra_data *extra = (const poly_extra_data *)extradata;          \
	voodoo_state *v = extra->state;                                             \
	stats_block *stats = &v->thread_stats[threadid];                            \
	DECLARE_DITHER_POINTERS;                                                    \
	INT32 startx = extent->startx;                                              \
	INT32 stopx = extent->stopx;                                                \
	INT32 iterr, iterg, iterb, itera;                                           \
	INT32 iterz;                                                                \
	INT64 iterw, iterw0 = 0, iterw1 = 0;                                        \
	INT64 iters0 = 0, iters1 = 0;                                               \
	INT64 itert0 = 0, itert1 = 0;                                               \
	UINT16 *depth;                                                              \
	UINT16 *dest;                                                               \
	INT32 dx, dy;                                                               \
	INT32 scry;                                                                 \
	INT32 x;                                                                    \
																				\
	/* determine the screen Y */                                                \
	scry = y;                                                                   \
	if (FBZMODE_Y_ORIGIN(FBZMODE))                                              \
		scry = (v->fbi.yorigin - y) & 0x3ff;                                    \
																				\
	/* compute dithering */                                                     \
	COMPUTE_DITHER_POINTERS(FBZMODE, y);                                        \
																				\
	/* apply clipping */                                                        \
	if (FBZMODE_ENABLE_CLIPPING(FBZMODE))                                       \
	{                                                                           \
		INT32 tempclip;                                                         \
																				\
		/* Y clipping buys us the whole scanline */                             \
		if (scry < ((v->reg[clipLowYHighY].u >> 16) & 0x3ff) ||                 \
			scry >= (v->reg[clipLowYHighY].u & 0x3ff))                          \
		{                                                                       \
			stats->pixels_in += stopx - startx;                                 \
			stats->clip_fail += stopx - startx;                                 \
			return;                                                             \
		}                                                                       \
																				\
		/* X clipping */                                                        \
		tempclip = (v->reg[clipLeftRight].u >> 16) & 0x3ff;                     \
		if (startx < tempclip)                                                  \
		{                                                                       \
			stats->pixels_in += tempclip - startx;                              \
			v->stats.total_clipped += tempclip - startx;                        \
			startx = tempclip;                                                  \
		}                                                                       \
		tempclip = v->reg[clipLeftRight].u & 0x3ff;                             \
		if (stopx >= tempclip)                                                  \
		{                                                                       \
			stats->pixels_in += stopx - tempclip;                               \
			v->stats.total_clipped += stopx - tempclip;                         \
			stopx = tempclip - 1;                                               \
		}                                                                       \
	}                                                                           \
																				\
	/* get pointers to the target buffer and depth buffer */                    \
	dest = (UINT16 *)destbase + scry * v->fbi.rowpixels;                        \
	depth = (v->fbi.auxoffs != ~0) ? ((UINT16 *)(v->fbi.ram + v->fbi.auxoffs) + scry * v->fbi.rowpixels) : NULL; \
																				\
	/* compute the starting parameters */                                       \
	dx = startx - (extra->ax >> 4);                                             \
	dy = y - (extra->ay >> 4);                                                  \
	iterr = extra->startr + dy * extra->drdy + dx * extra->drdx;                \
	iterg = extra->startg + dy * extra->dgdy + dx * extra->dgdx;                \
	iterb = extra->startb + dy * extra->dbdy + dx * extra->dbdx;                \
	itera = extra->starta + dy * extra->dady + dx * extra->dadx;                \
	iterz = extra->startz + dy * extra->dzdy + dx * extra->dzdx;                \
	iterw = extra->startw + dy * extra->dwdy + dx * extra->dwdx;                \
	if (TMUS >= 1)                                                              \
	{                                                                           \
		iterw0 = extra->startw0 + dy * extra->dw0dy +   dx * extra->dw0dx;      \
		iters0 = extra->starts0 + dy * extra->ds0dy + dx * extra->ds0dx;        \
		itert0 = extra->startt0 + dy * extra->dt0dy + dx * extra->dt0dx;        \
	}                                                                           \
	if (TMUS >= 2)                                                              \
	{                                                                           \
		iterw1 = extra->startw1 + dy * extra->dw1dy +   dx * extra->dw1dx;      \
		iters1 = extra->starts1 + dy * extra->ds1dy + dx * extra->ds1dx;        \
		itert1 = extra->startt1 + dy * extra->dt1dy + dx * extra->dt1dx;        \
	}                                                                           \
	extra->info->hits++;                                                        \
	/* loop in X */                                                             \
	for (x = startx; x < stopx; x++)                                            \
	{                                                                           \
		rgb_union iterargb;                                             \
		rgb_union texel = { 0 };                                                \
																				\
		/* pixel pipeline part 1 handles depth setup and stippling */         \
		PIXEL_PIPELINE_BEGIN(v, stats, x, y, FBZCOLORPATH, FBZMODE, iterz, iterw); \
		DEPTH_TEST(v, stats, x, FBZMODE);   \
																				\
		/* run the texture pipeline on TMU1 to produce a value in texel */      \
		/* note that they set LOD min to 8 to "disable" a TMU */                \
		if (TMUS >= 2 && v->tmu[1].lodmin < (8 << 8))                           \
			TEXTURE_PIPELINE(&v->tmu[1], x, dither4, TEXMODE1, texel,           \
								v->tmu[1].lookup, extra->lodbase1,              \
								iters1, itert1, iterw1, texel);                 \
																				\
		/* run the texture pipeline on TMU0 to produce a final */               \
		/* result in texel */                                                   \
		/* note that they set LOD min to 8 to "disable" a TMU */                \
		if (TMUS >= 1 && v->tmu[0].lodmin < (8 << 8))                           \
		{                                                                     \
			if (!v->send_config)                                                \
				TEXTURE_PIPELINE(&v->tmu[0], x, dither4, TEXMODE0, texel,           \
									v->tmu[0].lookup, extra->lodbase0,              \
									iters0, itert0, iterw0, texel);               \
			else                                                                \
				texel.u = v->tmu_config;                                              \
		}                                                                     \
		/* colorpath pipeline selects source colors and does blending */        \
		CLAMPED_ARGB(iterr, iterg, iterb, itera, FBZCOLORPATH, iterargb);       \
		COLORPATH_PIPELINE(v, stats, FBZCOLORPATH, FBZMODE, ALPHAMODE, texel,   \
								iterz, iterw, iterargb);                            \
																				\
		/* pixel pipeline part 2 handles fog, alpha, and final output */        \
		PIXEL_PIPELINE_END(v, stats, dither, dither4, dither_lookup, x, dest, depth, \
							FBZMODE, FBZCOLORPATH, ALPHAMODE, FOGMODE,          \
							iterz, iterw, iterargb);                            \
																				\
		/* update the iterated parameters */                                    \
		iterr += extra->drdx;                                                   \
		iterg += extra->dgdx;                                                   \
		iterb += extra->dbdx;                                                   \
		itera += extra->dadx;                                                   \
		iterz += extra->dzdx;                                                   \
		iterw += extra->dwdx;                                                   \
		if (TMUS >= 1)                                                          \
		{                                                                       \
			iterw0 += extra->dw0dx;                                             \
			iters0 += extra->ds0dx;                                             \
			itert0 += extra->dt0dx;                                             \
		}                                                                       \
		if (TMUS >= 2)                                                          \
		{                                                                       \
			iterw1 += extra->dw1dx;                                             \
			iters1 += extra->ds1dx;                                             \
			itert1 += extra->dt1dx;                                             \
		}                                                                       \
	}                                                                           \
}
#else
// New rasterizer implementation
#define RASTERIZER(name, TMUS, FBZCOLORPATH, FBZMODE, ALPHAMODE, FOGMODE, TEXMODE0, TEXMODE1) \
																				\
static void raster_##name(void *destbase, INT32 y, const poly_extent *extent, const void *extradata, int threadid) \
{                                                                               \
	const poly_extra_data *extra = (const poly_extra_data *)extradata;          \
	voodoo_state *v = extra->state;                                             \
	stats_block *stats = &v->thread_stats[threadid];                            \
	DECLARE_DITHER_POINTERS;                                                    \
	INT32 startx = extent->startx;                                              \
	INT32 stopx = extent->stopx;                                                \
	rgbaint_t iterargb, iterargbDelta;                                           \
	INT32 iterz;                                                                \
	INT64 iterw, iterw0 = 0, iterw1 = 0;                                        \
	INT64 iters0 = 0, iters1 = 0;                                               \
	INT64 itert0 = 0, itert1 = 0;                                               \
	UINT16 *depth;                                                              \
	UINT16 *dest;                                                               \
	INT32 dx, dy;                                                               \
	INT32 scry;                                                                 \
	INT32 x;                                                                    \
																				\
	/* determine the screen Y */                                                \
	scry = y;                                                                   \
	if (FBZMODE_Y_ORIGIN(FBZMODE))                                              \
		scry = (v->fbi.yorigin - y) & 0x3ff;                                    \
																				\
	/* compute dithering */                                                     \
	COMPUTE_DITHER_POINTERS(FBZMODE, y);                                        \
																				\
	/* apply clipping */                                                        \
	if (FBZMODE_ENABLE_CLIPPING(FBZMODE))                                       \
	{                                                                           \
		INT32 tempclip;                                                         \
																				\
		/* Y clipping buys us the whole scanline */                             \
		if (scry < ((v->reg[clipLowYHighY].u >> 16) & 0x3ff) ||                 \
			scry >= (v->reg[clipLowYHighY].u & 0x3ff))                          \
		{                                                                       \
			stats->pixels_in += stopx - startx;                                 \
			stats->clip_fail += stopx - startx;                                 \
			return;                                                             \
		}                                                                       \
																				\
		/* X clipping */                                                        \
		tempclip = (v->reg[clipLeftRight].u >> 16) & 0x3ff;                     \
		if (startx < tempclip)                                                  \
		{                                                                       \
			stats->pixels_in += tempclip - startx;                              \
			v->stats.total_clipped += tempclip - startx;                        \
			startx = tempclip;                                                  \
		}                                                                       \
		tempclip = v->reg[clipLeftRight].u & 0x3ff;                             \
		if (stopx >= tempclip)                                                  \
		{                                                                       \
			stats->pixels_in += stopx - tempclip;                               \
			v->stats.total_clipped += stopx - tempclip;                         \
			stopx = tempclip - 1;                                               \
		}                                                                       \
	}                                                                           \
																				\
	/* get pointers to the target buffer and depth buffer */                    \
	dest = (UINT16 *)destbase + scry * v->fbi.rowpixels;                        \
	depth = (v->fbi.auxoffs != ~0) ? ((UINT16 *)(v->fbi.ram + v->fbi.auxoffs) + scry * v->fbi.rowpixels) : NULL; \
																				\
	/* compute the starting parameters */                                       \
	dx = startx - (extra->ax >> 4);                                             \
	dy = y - (extra->ay >> 4);                                                  \
	INT32 iterr = extra->startr + dy * extra->drdy + dx * extra->drdx;                \
	INT32 iterg = extra->startg + dy * extra->dgdy + dx * extra->dgdx;                \
	INT32 iterb = extra->startb + dy * extra->dbdy + dx * extra->dbdx;                \
	INT32 itera = extra->starta + dy * extra->dady + dx * extra->dadx;                \
	iterargb.set(itera, iterr, iterg, iterb); \
	iterargbDelta.set(extra->dadx, extra->drdx, extra->dgdx, extra->dbdx); \
	iterz = extra->startz + dy * extra->dzdy + dx * extra->dzdx;                \
	iterw = extra->startw + dy * extra->dwdy + dx * extra->dwdx;                \
	if (TMUS >= 1)                                                              \
	{                                                                           \
		iterw0 = extra->startw0 + dy * extra->dw0dy +   dx * extra->dw0dx;      \
		iters0 = extra->starts0 + dy * extra->ds0dy + dx * extra->ds0dx;        \
		itert0 = extra->startt0 + dy * extra->dt0dy + dx * extra->dt0dx;        \
	}                                                                           \
	if (TMUS >= 2)                                                              \
	{                                                                           \
		iterw1 = extra->startw1 + dy * extra->dw1dy +   dx * extra->dw1dx;      \
		iters1 = extra->starts1 + dy * extra->ds1dy + dx * extra->ds1dx;        \
		itert1 = extra->startt1 + dy * extra->dt1dy + dx * extra->dt1dx;        \
	}                                                                           \
	extra->info->hits++;                                                        \
	/* loop in X */                                                             \
	for (x = startx; x < stopx; x++)                                            \
	{                                                                           \
		rgbaint_t texel(0);                                                \
		rgbaint_t color, preFog;                                                \
																				\
		/* pixel pipeline part 1 handles depth setup and stippling */         \
		PIXEL_PIPELINE_BEGIN(v, stats, x, y, FBZCOLORPATH, FBZMODE, iterz, iterw); \
		/* depth testing */         \
		if (!depthTest((UINT16) v->reg[zaColor].u, stats, depth[x], FBZMODE, biasdepth)) \
			goto skipdrawdepth; \
																				\
		/* run the texture pipeline on TMU1 to produce a value in texel */      \
		/* note that they set LOD min to 8 to "disable" a TMU */                \
		if (TMUS >= 2 && v->tmu[1].lodmin < (8 << 8))                    {       \
			INT32 tmp; \
			const rgbaint_t texelZero(0);  \
			texel = genTexture(&v->tmu[1], x, dither4, TEXMODE1, v->tmu[1].lookup, extra->lodbase1, \
														iters1, itert1, iterw1, tmp); \
			texel = combineTexture(&v->tmu[1], TEXMODE1, texel, texelZero, tmp); \
		} \
		/* run the texture pipeline on TMU0 to produce a final */               \
		/* result in texel */                                                   \
		/* note that they set LOD min to 8 to "disable" a TMU */                \
		if (TMUS >= 1 && v->tmu[0].lodmin < (8 << 8))                           \
		{                                                                   \
			if (!v->send_config)                                                \
			{                                                                   \
				INT32 lod0; \
				rgbaint_t texelT0;                                                \
				texelT0 = genTexture(&v->tmu[0], x, dither4, TEXMODE0, v->tmu[0].lookup, extra->lodbase0, \
																iters0, itert0, iterw0, lod0); \
				texel = combineTexture(&v->tmu[0], TEXMODE0, texelT0, texel, lod0); \
			}                                                                   \
			else                                                                \
			{                                                                   \
				texel.set(v->tmu_config);                                              \
			}                                                                   \
		}                                                                   \
																				\
		/* colorpath pipeline selects source colors and does blending */        \
		color = clampARGB(iterargb, FBZCOLORPATH);           \
		if (!combineColor(v, stats, FBZCOLORPATH, FBZMODE, ALPHAMODE, texel, iterz, iterw, color)) \
			goto skipdrawdepth; \
																				\
		/* pixel pipeline part 2 handles fog, alpha, and final output */        \
		PIXEL_PIPELINE_END(v, stats, dither, dither4, dither_lookup, x, dest, depth, \
							FBZMODE, FBZCOLORPATH, ALPHAMODE, FOGMODE,          \
							iterz, iterw, iterargb);                            \
																				\
		/* update the iterated parameters */                                    \
		iterargb += iterargbDelta;                                              \
		iterz += extra->dzdx;                                                   \
		iterw += extra->dwdx;                                                   \
		if (TMUS >= 1)                                                          \
		{                                                                       \
			iterw0 += extra->dw0dx;                                             \
			iters0 += extra->ds0dx;                                             \
			itert0 += extra->dt0dx;                                             \
		}                                                                       \
		if (TMUS >= 2)                                                          \
		{                                                                       \
			iterw1 += extra->dw1dx;                                             \
			iters1 += extra->ds1dx;                                             \
			itert1 += extra->dt1dx;                                             \
		}                                                                       \
	}                                                                           \
}
#endif
// ******************************************************************************************************************************
// Computes a log2 of a 16.32 value to 2 fractional bits of precision.
// The return value is coded as a 24.8 value.
// The maximum error using a 4 bit lookup from the mantissa is 0.0875, which is less than 1/2 lsb (0.125) for 2 bits of fraction.
// ******************************************************************************************************************************
static inline INT32 ATTR_FORCE_INLINE new_log2(double &value)
{
	static const INT32 new_log2_table[16] = {0, 22, 44, 63, 82, 100, 118, 134, 150, 165, 179, 193, 207, 220, 232, 244};
	UINT64 ival = *((UINT64 *)&value);
	// We zero the result if negative so don't worry about the sign bit
	INT32 exp = (ival>>52);
	exp -= 1023+32;
	exp <<= 8;
	UINT32 addr = (UINT64)(ival>>48) & 0xf;
	exp += new_log2_table[addr];
	// Return 0 if negative
	return (ival & ((UINT64)1<<63)) ? 0 : exp;
}

// Computes A/C and B/C and returns log2 of 1/C
// A, B and C are 16.32 values.  The results are 24.8.
static inline void ATTR_FORCE_INLINE multi_reciplog(INT64 valueA, INT64 valueB, INT64 valueC, INT32 &log, INT32 &resA, INT32 &resB)
{
	double recip = double(1ULL<<(47-39))/valueC;
	double resAD = valueA * recip;
	double resBD = valueB * recip;
	log = new_log2(recip);
	log += 56<<8;
	resA = resAD;
	resB = resBD;
}


static inline rgbaint_t ATTR_FORCE_INLINE genTexture(tmu_state *TT, INT32 x, const UINT8 *dither4, const UINT32 TEXMODE, rgb_t *LOOKUP, INT32 LODBASE, INT64 ITERS, INT64 ITERT, INT64 ITERW, INT32 &lod)
{
	rgbaint_t result;
	INT32 s, t, ilod;

	/* determine the S/T/LOD values for this texture */
	lod = (LODBASE);
	if (TEXMODE_ENABLE_PERSPECTIVE(TEXMODE))
	{
		INT32 wLog;
		if (USE_FAST_RECIP) {
			const INT32 oow = fast_reciplog((ITERW), &wLog);
			s = ((INT64)oow * (ITERS)) >> (29+10);
			t = ((INT64)oow * (ITERT)) >> (29+10);
		} else {
			multi_reciplog(ITERS, ITERT, ITERW, wLog, s, t);
		}
		lod += wLog;
	}
	else
	{
		s = (ITERS) >> (14+10);
		t = (ITERT) >> (14+10);
	}

	/* clamp W */
	if (TEXMODE_CLAMP_NEG_W(TEXMODE) && (ITERW) < 0)
	{
		s = t = 0;
	}

	/* clamp the LOD */
	lod += (TT)->lodbias;
	if (TEXMODE_ENABLE_LOD_DITHER(TEXMODE))
		lod += dither4[x&3] << 4;
	if (lod < (TT)->lodmin)
		lod = (TT)->lodmin;
	else if (lod > (TT)->lodmax)
		lod = (TT)->lodmax;

	/* now the LOD is in range; if we don't own this LOD, take the next one */
	ilod = lod >> 8;
	if (!(((TT)->lodmask >> ilod) & 1))
		ilod++;

	/* fetch the texture base */
	UINT32 texbase = (TT)->lodoffset[ilod];

	/* compute the maximum s and t values at this LOD */
	INT32 smax = (TT)->wmask >> ilod;
	INT32 tmax = (TT)->hmask >> ilod;

	/* determine whether we are point-sampled or bilinear */
	if ((lod == (TT)->lodmin && !TEXMODE_MAGNIFICATION_FILTER(TEXMODE)) ||
		(lod != (TT)->lodmin && !TEXMODE_MINIFICATION_FILTER(TEXMODE)))
	{
		/* point sampled */

		UINT32 texel0;

		/* adjust S/T for the LOD and strip off the fractions */
		s >>= ilod + (18-10);
		t >>= ilod + (18-10);

		/* clamp/wrap S/T if necessary */
		if (TEXMODE_CLAMP_S(TEXMODE))
			CLAMP(s, 0, smax);
		if (TEXMODE_CLAMP_T(TEXMODE))
			CLAMP(t, 0, tmax);
		s &= smax;
		t &= tmax;
		t *= smax + 1;

		/* fetch texel data */
		if (TEXMODE_FORMAT(TEXMODE) < 8)
		{
			texel0 = *(UINT8 *)&(TT)->ram[(texbase + t + s) & (TT)->mask];
			result.set((LOOKUP)[texel0]);
		}
		else
		{
			texel0 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t + s)) & (TT)->mask];
			if (TEXMODE_FORMAT(TEXMODE) >= 10 && TEXMODE_FORMAT(TEXMODE) <= 12)
				result.set((LOOKUP)[texel0]);
			else
				result.set(((LOOKUP)[texel0 & 0xff] & 0xffffff) | ((texel0 & 0xff00) << 16));
		}
	}
	else
	{
		/* bilinear filtered */

		UINT32 texel0, texel1, texel2, texel3;
		UINT32 sfrac, tfrac;
		INT32 s1, t1;

		/* adjust S/T for the LOD and strip off all but the low 8 bits of */
		/* the fraction */
		s >>= ilod; // + (10-10);
		t >>= ilod; // + (10-10);

		/* also subtract 1/2 texel so that (0.5,0.5) = a full (0,0) texel */
		s -= 0x80;
		t -= 0x80;

		/* extract the fractions */
		sfrac = s & (TT)->bilinear_mask;
		tfrac = t & (TT)->bilinear_mask;

		/* now toss the rest */
		s >>= 8;
		t >>= 8;
		s1 = s + 1;
		t1 = t + 1;

		/* clamp/wrap S/T if necessary */
		if (TEXMODE_CLAMP_S(TEXMODE))
		{
			if (s < 0) {
				s = 0;
				s1 = 0;
			} else if (s >= smax) {
				s = smax;
				s1 = smax;
			}
			//CLAMP(s, 0, smax);
			//CLAMP(s1, 0, smax);
		} else {
			s &= smax;
			s1 &= smax;
		}

		if (TEXMODE_CLAMP_T(TEXMODE))
		{
			if (t < 0) {
				t = 0;
				t1 = 0;
			} else if (t >= tmax) {
				t = tmax;
				t1 = tmax;
			}
			//CLAMP(t, 0, tmax);
			//CLAMP(t1, 0, tmax);
		} else {
			t &= tmax;
			t1 &= tmax;
		}
		t *= smax + 1;
		t1 *= smax + 1;

		/* fetch texel data */
		if (TEXMODE_FORMAT(TEXMODE) < 8)
		{
			texel0 = *(UINT8 *)&(TT)->ram[(texbase + t + s)];
			texel1 = *(UINT8 *)&(TT)->ram[(texbase + t + s1)];
			texel2 = *(UINT8 *)&(TT)->ram[(texbase + t1 + s)];
			texel3 = *(UINT8 *)&(TT)->ram[(texbase + t1 + s1)];
			texel0 = (LOOKUP)[texel0];
			texel1 = (LOOKUP)[texel1];
			texel2 = (LOOKUP)[texel2];
			texel3 = (LOOKUP)[texel3];
		}
		else
		{
			texel0 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t + s))];
			texel1 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t + s1))];
			texel2 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t1 + s))];
			texel3 = *(UINT16 *)&(TT)->ram[(texbase + 2*(t1 + s1))];
			if (TEXMODE_FORMAT(TEXMODE) >= 10 && TEXMODE_FORMAT(TEXMODE) <= 12)
			{
				texel0 = (LOOKUP)[texel0];
				texel1 = (LOOKUP)[texel1];
				texel2 = (LOOKUP)[texel2];
				texel3 = (LOOKUP)[texel3];
			}
			else
			{
				texel0 = ((LOOKUP)[texel0 & 0xff] & 0xffffff) | ((texel0 & 0xff00) << 16);
				texel1 = ((LOOKUP)[texel1 & 0xff] & 0xffffff) | ((texel1 & 0xff00) << 16);
				texel2 = ((LOOKUP)[texel2 & 0xff] & 0xffffff) | ((texel2 & 0xff00) << 16);
				texel3 = ((LOOKUP)[texel3 & 0xff] & 0xffffff) | ((texel3 & 0xff00) << 16);
			}
		}

		/* weigh in each texel */

		result.bilinear_filter_rgbaint(texel0, texel1, texel2, texel3, sfrac, tfrac);
	}
	return result;
}

static inline rgbaint_t ATTR_FORCE_INLINE combineTexture(tmu_state *TT, const UINT32 TEXMODE, rgbaint_t c_local, rgbaint_t c_other, INT32 lod)
{
	INT32 a_other = c_other.get_a();
	INT32 a_local = c_local.get_a();
	rgbaint_t add_val = c_local;
	UINT8 tmp;

	/* select zero/other for RGB */
	if (TEXMODE_TC_ZERO_OTHER(TEXMODE))
		c_other.and_imm_rgba(-1, 0, 0, 0);

	/* select zero/other for alpha */
	if (TEXMODE_TCA_ZERO_OTHER(TEXMODE))
		c_other.set_a(0);

	if (TEXMODE_TC_SUB_CLOCAL(TEXMODE) || TEXMODE_TCA_SUB_CLOCAL(TEXMODE))
	{
		rgbaint_t sub_val = c_local;

		/* potentially subtract c_local */
		if (!TEXMODE_TC_SUB_CLOCAL(TEXMODE))
			sub_val.and_imm_rgba(-1, 0, 0, 0);

		if (!TEXMODE_TCA_SUB_CLOCAL(TEXMODE))
			sub_val.set_a(0);

		c_other.sub(sub_val);
	}

	/* blend RGB */
	switch (TEXMODE_TC_MSELECT(TEXMODE))
	{
		default:    /* reserved */
		case 0:     /* zero */
			c_local.and_imm_rgba(-1, 0, 0, 0);
			break;

		case 1:     /* c_local */
			break;

		case 2:     /* a_other */
			c_local.set(a_local, a_other, a_other, a_other);
			break;

		case 3:     /* a_local */
			c_local.set(a_local, a_local, a_local, a_local);
			break;

		case 4:     /* LOD (detail factor) */
			if ((TT)->detailbias <= lod)
				c_local.and_imm_rgba(-1, 0, 0, 0);
			else
			{
				tmp = ((((TT)->detailbias - lod) << (TT)->detailscale) >> 8);
				if (tmp > (TT)->detailmax)
					tmp = (TT)->detailmax;
				c_local.set(a_local, tmp, tmp, tmp);
			}
			break;

		case 5:     /* LOD fraction */
			tmp = lod & 0xff;
			c_local.set(a_local, tmp, tmp, tmp);
			break;
	}

	/* blend alpha */
	switch (TEXMODE_TCA_MSELECT(TEXMODE))
	{
		default:    /* reserved */
		case 0:     /* zero */
			c_local.set_a(0);
			break;

		case 1:     /* c_local */
			break;

		case 2:     /* a_other */
			c_local.set_a(a_other);
			break;

		case 3:     /* a_local */
			break;

		case 4:     /* LOD (detail factor) */
			if ((TT)->detailbias <= lod)
				c_local.set_a(0);
			else
			{
				tmp = ((((TT)->detailbias - lod) << (TT)->detailscale) >> 8);
				if (tmp > (TT)->detailmax)
					tmp = (TT)->detailmax;
				c_local.set_a(tmp);
			}
			break;

		case 5:     /* LOD fraction */
			c_local.set_a(lod & 0xff);
			break;
	}

	/* reverse the RGB blend */
	if (!TEXMODE_TC_REVERSE_BLEND(TEXMODE))
	{
		c_local.xor_imm_rgba(0, 0xff, 0xff, 0xff);
	}

	/* reverse the alpha blend */
	if (!TEXMODE_TCA_REVERSE_BLEND(TEXMODE))
		c_local.xor_imm_rgba(0xff, 0, 0, 0);

	/* do the blend */
	//tr = (tr * (blendr + 1)) >> 8;
	//tg = (tg * (blendg + 1)) >> 8;
	//tb = (tb * (blendb + 1)) >> 8;
	//ta = (ta * (blenda + 1)) >> 8;

	/* add clocal or alocal to RGB */
	switch (TEXMODE_TC_ADD_ACLOCAL(TEXMODE))
	{
		case 3:     /* reserved */
		case 0:     /* nothing */
			add_val.set(a_local, 0, 0, 0);
			break;

		case 1:     /* add c_local */
			break;

		case 2:     /* add_alocal */
			add_val.set(a_local, a_local , a_local , a_local);
			//tr += c_local.rgb.a;
			//tg += c_local.rgb.a;
			//tb += c_local.rgb.a;
			break;
	}

	/* add clocal or alocal to alpha */
	if (!TEXMODE_TCA_ADD_ACLOCAL(TEXMODE))
		add_val.set_a(0);
		//ta += c_local.rgb.a;

	/* clamp */
	//result.rgb.r = (tr < 0) ? 0 : (tr > 0xff) ? 0xff : tr;
	//result.rgb.g = (tg < 0) ? 0 : (tg > 0xff) ? 0xff : tg;
	//result.rgb.b = (tb < 0) ? 0 : (tb > 0xff) ? 0xff : tb;
	//result.rgb.a = (ta < 0) ? 0 : (ta > 0xff) ? 0xff : ta;
	c_local.add_imm(1);
	c_other.scale_add_and_clamp(c_local, add_val);
	rgbaint_t result(c_other);
	/* invert */
	if (TEXMODE_TC_INVERT_OUTPUT(TEXMODE))
		result.xor_imm_rgba(0, 0xff, 0xff, 0xff);
	if (TEXMODE_TCA_INVERT_OUTPUT(TEXMODE))
		result.xor_imm_rgba(0xff, 0, 0, 0);
	return result;
}
