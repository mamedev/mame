/******************************************************************************

    Atari 400/800

    ANTIC video controller
    GTIA  graphics television interface adapter

    Juergen Buchmueller, June 1998

******************************************************************************/

#ifndef ATARI_H
#define ATARI_H

/*----------- defined in machine/atari.c -----------*/

typedef struct {
	int  serout_count;
	int  serout_offs;
	UINT8 serout_buff[512];
	UINT8 serout_chksum;
	int  serout_delay;

	int  serin_count;
	int  serin_offs;
	UINT8 serin_buff[512];
	UINT8 serin_chksum;
	int  serin_delay;
}	ATARI_FDC;
extern ATARI_FDC atari_fdc;

MACHINE_START( a400 );
MACHINE_START( a800 );
MACHINE_START( a600xl );
MACHINE_START( a800xl );
MACHINE_START( a5200 );

#ifdef MESS
DEVICE_LOAD( a800_floppy );

DEVICE_LOAD( a800_cart );
DEVICE_UNLOAD( a800_cart );

DEVICE_LOAD( a800xl_cart );
DEVICE_UNLOAD( a800xl_cart );

DEVICE_LOAD( a5200_cart );
DEVICE_UNLOAD( a5200_cart );
#endif

 READ8_HANDLER ( atari_serin_r );
WRITE8_HANDLER ( atari_serout_w );
void atari_interrupt_cb(int mask);

void a800_handle_keyboard(void);
void a5200_handle_keypads(void);

/* video */

/* Enable this to make the video code use readmem16 to retrieve data */
/* Otherwise the RAM memory array is accessed and thus, if a display list would */
/* point to memory mapped IO, the result is different from a real machine */
#define ACCURATE_ANTIC_READMEM	1


#define CYCLES_PER_LINE 114 	/* total number of cpu cycles per scanline (incl. hblank) */
#define CYCLES_REFRESH	9		/* number of cycles lost for ANTICs RAM refresh using DMA */
#define CYCLES_HSTART	32		/* where does the ANTIC DMA fetch start */
#define CYCLES_DLI_NMI	7		/* number of cycles until the CPU recognizes a DLI */
#define CYCLES_HSYNC	104 	/* where does the HSYNC position of a scanline start */

#define VBL_END 		8		/* vblank ends in this scanline */
#define VDATA_START 	11		/* video display begins in this scanline */
#define VDATA_END		244 	/* video display ends in this scanline */
#define VBL_START		248 	/* vblank starts in this scanline */

/* total number of lines per frame (incl. vblank) */
#define TOTAL_LINES_60HZ 262
#define TOTAL_LINES_50HZ 312

/* frame rates */
#define FRAME_RATE_50HZ (double)1789790/114/TOTAL_LINES_50HZ
#define FRAME_RATE_60HZ (double)1789790/114/TOTAL_LINES_60HZ

#define HWIDTH			48		/* total characters per line */
#define HCHARS			44		/* visible characters per line */
#define VHEIGHT 		32
#define VCHARS			(VDATA_END-VDATA_START+7)/8
#define BUF_OFFS0		(HWIDTH-HCHARS)/2
#define MIN_X			((HWIDTH-42)/2)*8
#define MAX_X			MIN_X+42*8-1
#define MIN_Y			VDATA_START
#define MAX_Y			VDATA_END-8-1

#define PMOFFSET		32		/* # of pixels to adjust p/m hpos */

#define VPAGE			0xf000	/* 4K page mask for video data src */
#define VOFFS			0x0fff	/* 4K offset mask for video data src */
#define DPAGE			0xfc00	/* 1K page mask for display list */
#define DOFFS			0x03ff	/* 1K offset mask for display list */

#define DLI_NMI         0x80    /* 10000000b bit mask for display list interrupt */
#define VBL_NMI 		0x40	/* 01000000b bit mask for vertical blank interrupt */

#define ANTIC_DLI		0x80	/* 10000000b cmds with display list intr    */
#define ANTIC_LMS		0x40	/* 01000000b cmds with load memory scan     */
#define ANTIC_VSCR		0x20	/* 00100000b cmds with vertical scroll      */
#define ANTIC_HSCR		0x10	/* 00010000b cmds with horizontal scroll    */
#define ANTIC_MODE		0x0f	/* 00001111b cmd mode mask                  */

#define DMA_ANTIC		0x20	/* 00100000b ANTIC DMA enable               */
#define DMA_PM_DBLLINE	0x10	/* 00010000b double line player/missile     */
#define DMA_PLAYER		0x08	/* 00001000b player DMA enable              */
#define DMA_MISSILE 	0x04	/* 00000100b missile DMA enable             */

#define OFS_MIS_SINGLE	3*256	/* offset missiles single line DMA          */
#define OFS_PL0_SINGLE	4*256	/* offset player 0 single line DMA          */
#define OFS_PL1_SINGLE	5*256	/* offset player 1 single line DMA          */
#define OFS_PL2_SINGLE	6*256	/* offset player 2 single line DMA          */
#define OFS_PL3_SINGLE	7*256	/* offset player 3 single line DMA          */

#define OFS_MIS_DOUBLE  3*128   /* offset missiles double line DMA          */
#define OFS_PL0_DOUBLE	4*128	/* offset player 0 double line DMA          */
#define OFS_PL1_DOUBLE	5*128	/* offset player 1 double line DMA          */
#define OFS_PL2_DOUBLE	6*128	/* offset player 2 double line DMA          */
#define OFS_PL3_DOUBLE	7*128	/* offset player 3 double line DMA          */

#define PFD 	0x00	/* 00000000b playfield default color */

#define PBK 	0x00	/* 00000000b playfield background */
#define PF0 	0x01	/* 00000001b playfield color #0   */
#define PF1 	0x02	/* 00000010b playfield color #1   */
#define PF2 	0x04	/* 00000100b playfield color #2   */
#define PF3 	0x08	/* 00001000b playfield color #3   */
#define PL0 	0x11	/* 00010001b player #0            */
#define PL1 	0x12	/* 00010010b player #1            */
#define PL2 	0x14	/* 00010100b player #2            */
#define PL3 	0x18	/* 00011000b player #3            */
#define MI0 	0x21	/* 00100001b missile #0           */
#define MI1 	0x22	/* 00100010b missile #1           */
#define MI2 	0x24	/* 00100100b missile #2           */
#define MI3 	0x28	/* 00101000b missile #3           */
#define T00 	0x40	/* 01000000b text mode pixels 00  */
#define P000	0x48	/* 01001000b player #0 pixels 00  */
#define P100	0x4a	/* 01001010b player #1 pixels 00  */
#define P200	0x4c	/* 01001100b player #2 pixels 00  */
#define P300	0x4e	/* 01001110b player #3 pixels 00  */
#define P400	0x4f	/* 01001111b missiles  pixels 00  */
#define T01 	0x50	/* 01010000b text mode pixels 01  */
#define P001	0x58	/* 01011000b player #0 pixels 01  */
#define P101	0x5a	/* 01011010b player #1 pixels 01  */
#define P201	0x5c	/* 01011100b player #2 pixels 01  */
#define P301	0x5e	/* 01011110b player #3 pixels 01  */
#define P401	0x5f	/* 01011111b missiles  pixels 01  */
#define T10 	0x60	/* 01100000b text mode pixels 10  */
#define P010	0x68	/* 01101000b player #0 pixels 10  */
#define P110	0x6a	/* 01101010b player #1 pixels 10  */
#define P210	0x6c	/* 01101100b player #2 pixels 10  */
#define P310	0x6e	/* 01101110b player #3 pixels 10  */
#define P410	0x6f	/* 01101111b missiles  pixels 10  */
#define T11 	0x70	/* 01110000b text mode pixels 11  */
#define P011	0x78	/* 01111000b player #0 pixels 11  */
#define P111	0x7a	/* 01111010b player #1 pixels 11  */
#define P211	0x7c	/* 01111100b player #2 pixels 11  */
#define P311	0x7e	/* 01111110b player #3 pixels 11  */
#define P411	0x7f	/* 01111111b missiles  pixels 11  */
#define G00 	0x80	/* 10000000b hires gfx pixels 00  */
#define G01 	0x90	/* 10010000b hires gfx pixels 01  */
#define G10 	0xa0	/* 10100000b hires gfx pixels 10  */
#define G11 	0xb0	/* 10110000b hires gfx pixels 11  */
#define GT1 	0xc0	/* 11000000b gtia mode 1          */
#define GT2 	0xd0	/* 11010000b gtia mode 2          */
#define GT3 	0xe0	/* 11100000b gtia mode 3          */
#define ILL 	0xfe	/* 11111110b illegal priority     */
#define EOR 	0xff	/* 11111111b EOR mode color       */

#define LUM 	0x0f	/* 00001111b luminance bits       */
#define HUE 	0xf0	/* 11110000b hue bits             */

#define TRIGGER_VBLANK	64715
#define TRIGGER_STEAL	64716
#define TRIGGER_HSYNC	64717

#define GTIA_MISSILE	0x01
#define GTIA_PLAYER 	0x02
#define GTIA_TRIGGER	0x04

#ifdef  LSB_FIRST

/* make a UINT32 from four bytes */
#define MKD(b0,b1,b2,b3) ((b0)|((b1)<<8)|((b2)<<16)|((b3)<<24))

#define BYTE0	0x000000ff
#define BYTE1   0x0000ff00
#define BYTE2   0x00ff0000
#define BYTE3   0xff000000

#else

/* make a UINT32 from four bytes */
#define MKD(b0,b1,b2,b3) (((b0)<<24)|((b1)<<16)|((b2)<<8)|(b3))

#define BYTE0	0xff000000
#define BYTE1   0x00ff0000
#define BYTE2   0x0000ff00
#define BYTE3   0x000000ff

#endif

/* make a qUINT16 from eight bytes */
#define MKQ(b0,b1,b2,b3,b4,b5,b6,b7) MKD(b0,b1,b2,b3), MKD(b4,b5,b6,b7)

/* make two UINT32s from 4 bytes by doubling them */
#define MKDD(b0,b1,b2,b3) MKD(b0,b0,b1,b1), MKD(b2,b2,b3,b3)

/* make a four UINT32s from 4 bytes by quadrupling them */
#define MKQD(b0,b1,b2,b3) MKD(b0,b0,b0,b0), MKD(b1,b1,b1,b1), MKD(b2,b2,b2,b2), MKD(b3,b3,b3,b3)

/*****************************************************************************
 * If your memcpy does not expand too well if you use it with constant
 * size_t field, you might want to define these macros somehow different.
 * NOTE: dst is not necessarily UINT32 aligned (because of horz scrolling)!
 *****************************************************************************/
#define COPY4(dst,s1) *dst++ = s1
#define COPY8(dst,s1,s2) *dst++ = s1; *dst++ = s2
#define COPY16(dst,s1,s2,s3,s4) *dst++ = s1; *dst++ = s2; *dst++ = s3; *dst++ = s4

typedef struct {
	UINT8	antic00;	/* 00 nothing */
	UINT8	antic01;	/* 01 nothing */
	UINT8	antic02;	/* 02 nothing */
	UINT8	antic03;	/* 03 nothing */
	UINT8	antic04;	/* 04 nothing */
	UINT8	antic05;	/* 05 nothing */
	UINT8	antic06;	/* 06 nothing */
	UINT8	antic07;	/* 07 nothing */
	UINT8	antic08;	/* 08 nothing */
	UINT8	antic09;	/* 09 nothing */
	UINT8	antic0a;	/* 0a nothing */
	UINT8	vcount; 	/* 0b vertical (scanline) counter */
	UINT8	penh;		/* 0c light pen horizontal pos */
	UINT8	penv;		/* 0d light pen vertical pos */
	UINT8	antic0e;	/* 0e nothing */
	UINT8	nmist;		/* 0f NMI status */
}	ANTIC_R;  /* read registers */

typedef struct {
	UINT8	dmactl; 	/* 00 write DMA control */
	UINT8	chactl; 	/* 01 write character control */
	UINT8	dlistl; 	/* 02 display list low */
	UINT8	dlisth; 	/* 03 display list high */
	UINT8	hscrol; 	/* 04 horz scroll */
	UINT8	vscrol; 	/* 05 vert scroll */
	UINT8	pmbasl; 	/* 06 player/missile base addr low */
	UINT8	pmbash; 	/* 07 player/missile base addr high */
	UINT8	chbasl; 	/* 08 character generator base addr low */
	UINT8	chbash; 	/* 09 character generator base addr high */
	UINT8	wsync;		/* 0a wait for hsync */
	UINT8	antic0b;	/* 0b nothing */
	UINT8	antic0c;	/* 0c nothing */
	UINT8	antic0d;	/* 0d nothing */
	UINT8	nmien;		/* 0e NMI enable */
	UINT8	nmires; 	/* 0f NMI reset */
}	ANTIC_W;  /* write registers */

/* per scanline buffer for video data (and optimization variables) */
typedef struct {
    UINT32  cmd;                /* antic command for this scanline */
    UINT16  data[HWIDTH];       /* graphics data buffer (text through chargen) */
}   VIDEO;

typedef struct {
	UINT32	cmd;				/* currently executed display list command */
	UINT32	steal_cycles;		/* steal how many cpu cycles for this line ? */
	UINT32	vscrol_old; 		/* old vscrol value */
	UINT32	hscrol_old; 		/* old hscrol value */
	INT32	modelines;			/* number of lines for current ANTIC mode */
	UINT32	chbase; 			/* character mode source base */
	UINT32	chand;				/* character and mask (chactl) */
	UINT32	chxor;				/* character xor mask (chactl) */
    UINT32  scanline;           /* current scan line */
	UINT32	pfwidth;			/* playfield width */
	UINT32	dpage;				/* display list address page */
	UINT32	doffs;				/* display list offset into page */
	UINT32	vpage;				/* video data source page */
	UINT32	voffs;				/* video data offset into page */
	UINT32	pmbase_s;			/* p/m graphics single line source base */
	UINT32	pmbase_d;			/* p/m graphics double line source base */
	ANTIC_R r;					/* ANTIC read registers */
	ANTIC_W w;					/* ANTIC write registers */
	UINT8	cclock[256+32]; 	/* color clock buffer filled by ANTIC */
	UINT8	pmbits[256+32]; 	/* player missile buffer filled by GTIA */
	UINT16	color_lookup[256];	/* color lookup table */
	UINT8   *prio_table[64]; 	/* player/missile priority tables */
	VIDEO	*video[312];		/* video buffer */
	UINT32	*cclk_expand;		/* shared buffer for the following: */
	UINT32  *pf_21;				/* 1cclk 2 color txt 2,3 */
	UINT32  *pf_x10b;			/* 1cclk 4 color txt 4,5, gfx D,E */
	UINT32  *pf_3210b2;			/* 1cclk 5 color txt 6,7, gfx 9,B,C */
	UINT32  *pf_210b4;			/* 4cclk 4 color gfx 8 */
	UINT32  *pf_210b2;			/* 2cclk 4 color gfx A */
	UINT32  *pf_1b;				/* 1cclk hires gfx F */
	UINT32  *pf_gtia1;			/* 1cclk gtia mode 1 */
	UINT32  *pf_gtia2;			/* 1cclk gtia mode 2 */
	UINT32  *pf_gtia3;			/* 1cclk gtia mode 3 */
    UINT8   *used_colors;       /* shared buffer for the following: */
	UINT8   *uc_21;				/* used colors for txt (2,3) */
	UINT8   *uc_x10b;			/* used colors for txt 4,5, gfx D,E */
	UINT8   *uc_3210b2;			/* used colors for txt 6,7, gfx 9,B,C */
	UINT8   *uc_210b4;			/* used colors for gfx 8 */
	UINT8   *uc_210b2;			/* used colors for gfx A */
	UINT8   *uc_1b;				/* used colors for gfx F */
	UINT8   *uc_g1;				/* used colors for gfx GTIA 1 */
	UINT8   *uc_g2;				/* used colors for gfx GTIA 2 */
	UINT8   *uc_g3;				/* used colors for gfx GTIA 3 */
}   ANTIC;

#if ACCURATE_ANTIC_READMEM
#define RDANTIC()	cpunum_read_byte(0, antic.dpage+antic.doffs)
#define RDVIDEO(o)	cpunum_read_byte(0, antic.vpage+((antic.voffs+(o))&VOFFS))
#define RDCHGEN(o)	cpunum_read_byte(0, antic.chbase+(o))
#define RDPMGFXS(o) cpunum_read_byte(0, antic.pmbase_s+(o)+(antic.scanline>>1))
#define RDPMGFXD(o) cpunum_read_byte(0, antic.pmbase_d+(o)+antic.scanline)
#else
#define RDANTIC()	(memory_region(REGION_CPU1))[antic.dpage+antic.doffs]
#define RDVIDEO(o)	(memory_region(REGION_CPU1))[antic.vpage+((antic.voffs+(o))&VOFFS)]
#define RDCHGEN(o)	(memory_region(REGION_CPU1))[antic.chbase+(o)]
#define RDPMGFXS(o) (memory_region(REGION_CPU1))[antic.pmbase_s+(o)+(antic.scanline>>1)]
#define RDPMGFXD(o) (memory_region(REGION_CPU1))[antic.pmbase_d+(o)+antic.scanline]
#endif

#define PREPARE()												\
	UINT32 *dst = (UINT32 *)&antic.cclock[PMOFFSET]

#define PREPARE_TXT2(width) 									\
	UINT32 *dst = (UINT32 *)&antic.cclock[PMOFFSET];			\
    int i;                                                      \
	for( i = 0; i < width; i++ )								\
	{															\
		UINT16 ch = RDVIDEO(i) << 3;							\
		if( ch & 0x400 )										\
		{														\
			ch = RDCHGEN((ch & 0x3f8) + antic.w.chbasl);		\
			ch = (ch ^ antic.chxor) & antic.chand;				\
		}														\
		else													\
		{														\
			ch = RDCHGEN(ch + antic.w.chbasl);					\
		}														\
		video->data[i] = ch;									\
	}

#define PREPARE_TXT3(width) 									\
	UINT32 *dst = (UINT32 *)&antic.cclock[PMOFFSET];			\
    int i;                                                      \
	for( i = 0; i < width; i++ )								\
	{															\
		UINT16 ch = RDVIDEO(i) << 3;							\
		if( ch & 0x400 )										\
		{														\
			ch &= 0x3f8;										\
			if( (ch & 0x300) == 0x300 ) 						\
			{													\
				if (antic.w.chbasl < 2) /* first two lines empty */ \
					ch = 0x00;									\
				else /* lines 2..7 are standard, 8&9 are 0&1 */ \
					ch = RDCHGEN(ch + (antic.w.chbasl & 7));	\
			}													\
			else												\
			{													\
				if (antic.w.chbasl > 7) /* last two lines empty */	\
					ch = 0x00;									\
				else /* lines 0..7 are standard */				\
					ch = RDCHGEN(ch + antic.w.chbasl);			\
			}													\
			ch = (ch ^ antic.chxor) & antic.chand;				\
		}														\
		else													\
		{														\
			if( (ch & 0x300) == 0x300 ) 						\
			{													\
				if (antic.w.chbasl < 2) /* first two lines empty */ \
					ch = 0x00;									\
				else /* lines 2..7 are standard, 8&9 are 0&1 */ \
					ch = RDCHGEN(ch + (antic.w.chbasl & 7));	\
			}													\
			else												\
			{													\
				if (antic.w.chbasl > 7) /* last two lines empty */	\
					ch = 0x00;									\
				else /* lines 0..7 are standard */				\
					ch = RDCHGEN(ch + antic.w.chbasl);			\
			}													\
		}														\
        video->data[i] = ch;                                    \
	}

#define PREPARE_TXT45(width,shift)								\
	UINT32 *dst = (UINT32 *)&antic.cclock[PMOFFSET];			\
    int i;                                                      \
	for( i = 0; i < width; i++ )								\
	{															\
		UINT16 ch = RDVIDEO(i) << 3;							\
		ch = ((ch>>2)&0x100)|RDCHGEN((ch&0x3f8)+(antic.w.chbasl>>shift)); \
		video->data[i] = ch;									\
	}


#define PREPARE_TXT67(width,shift)								\
	UINT32 *dst = (UINT32 *)&antic.cclock[PMOFFSET];			\
    int i;                                                      \
	for( i = 0; i < width; i++ )								\
	{															\
		UINT16 ch = RDVIDEO(i) << 3;							\
		ch = (ch&0x600)|(RDCHGEN((ch&0x1f8)+(antic.w.chbasl>>shift))<<1); \
		video->data[i] = ch;									\
	}

#define PREPARE_GFX8(width)                                     \
	UINT32 *dst = (UINT32 *)&antic.cclock[PMOFFSET];			\
    int i;                                                      \
	for( i = 0; i < width; i++ )								\
		video->data[i] = RDVIDEO(i) << 2

#define PREPARE_GFX9BC(width)									\
	UINT32 *dst = (UINT32 *)&antic.cclock[PMOFFSET];			\
    int i;                                                      \
	for( i = 0; i < width; i++ )								\
		video->data[i] = RDVIDEO(i) << 1

#define PREPARE_GFXA(width) 									\
	UINT32 *dst = (UINT32 *)&antic.cclock[PMOFFSET];			\
    int i;                                                      \
	for( i = 0; i < width; i++ )								\
		video->data[i] = RDVIDEO(i) << 1

#define PREPARE_GFXDE(width)									\
	UINT32 *dst = (UINT32 *)&antic.cclock[PMOFFSET];			\
    int i;                                                      \
	for( i = 0; i < width; i++ )								\
		video->data[i] = RDVIDEO(i)

#define PREPARE_GFXF(width) 									\
	UINT32 *dst = (UINT32 *)&antic.cclock[PMOFFSET];			\
    int i;                                                      \
	for( i = 0; i < width; i++ )								\
		video->data[i] = RDVIDEO(i)

#define PREPARE_GFXG1(width)									\
	UINT32 *dst = (UINT32 *)&antic.cclock[PMOFFSET];			\
    int i;                                                      \
	for( i = 0; i < width; i++ )								\
		video->data[i] = RDVIDEO(i)

#define PREPARE_GFXG2(width)									\
	UINT32 *dst = (UINT32 *)&antic.cclock[PMOFFSET];			\
    int i;                                                      \
	for( i = 0; i < width; i++ )								\
		video->data[i] = RDVIDEO(i)

#define PREPARE_GFXG3(width)									\
	UINT32 *dst = (UINT32 *)&antic.cclock[PMOFFSET];			\
    int i;                                                      \
	for( i = 0; i < width; i++ )								\
		video->data[i] = RDVIDEO(i)

/******************************************************************
 * common end of a single antic/gtia mode emulation function
 ******************************************************************/
#define POST()													\
	--antic.modelines

#define POST_GFX(width) 										\
	antic.steal_cycles += width;								\
	if( --antic.modelines == 0 )								\
		antic.voffs = (antic.voffs + width) & VOFFS

#define POST_TXT(width) 										\
	antic.steal_cycles += width;								\
	if( --antic.modelines == 0 )								\
		antic.voffs = (antic.voffs + width) & VOFFS;			\
	else														\
	if( antic.w.chactl & 4 )									\
		antic.w.chbasl--;										\
	else														\
        antic.w.chbasl++

/* erase a number of color clocks to background color PBK */
#define ERASE4	{	\
	*dst++ = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK;	\
    *dst++ = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK;  \
    *dst++ = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK;  \
	*dst++ = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK;	\
	}

#define ZAP48() 												\
	dst = (UINT32 *)&antic.cclock[PMOFFSET];					\
	dst[ 0] = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK; 	\
	dst[ 1] = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK; 	\
	dst[ 2] = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK; 	\
	dst[45] = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK; 	\
	dst[46] = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK; 	\
	dst[47] = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK

#define ERASE8  \
	ERASE4; 	\
	ERASE4

#define REP08(FUNC) 						\
	ERASE8; 								\
	FUNC( 0); FUNC( 1); FUNC( 2); FUNC( 3); \
	FUNC( 4); FUNC( 5); FUNC( 6); FUNC( 7); \
	ERASE8

#define REP10(FUNC) 						\
    ERASE4;                                 \
    FUNC( 0); FUNC( 1); FUNC( 2); FUNC( 3); \
	FUNC( 4); FUNC( 5); FUNC( 6); FUNC( 7); \
	FUNC( 8); FUNC( 9); 					\
	ERASE4

#define REP12(FUNC) 						\
    FUNC( 0); FUNC( 1); FUNC( 2); FUNC( 3); \
	FUNC( 4); FUNC( 5); FUNC( 6); FUNC( 7); \
	FUNC( 8); FUNC( 9); FUNC(10); FUNC(11)

#define REP16(FUNC) 						\
    ERASE8;                                 \
	FUNC( 0); FUNC( 1); FUNC( 2); FUNC( 3); \
	FUNC( 4); FUNC( 5); FUNC( 6); FUNC( 7); \
	FUNC( 8); FUNC( 9); FUNC(10); FUNC(11); \
	FUNC(12); FUNC(13); FUNC(14); FUNC(15); \
	ERASE8

#define REP20(FUNC) 						\
    ERASE4;                                 \
	FUNC( 0); FUNC( 1); FUNC( 2); FUNC( 3); \
	FUNC( 4); FUNC( 5); FUNC( 6); FUNC( 7); \
	FUNC( 8); FUNC( 9); FUNC(10); FUNC(11); \
	FUNC(12); FUNC(13); FUNC(14); FUNC(15); \
	FUNC(16); FUNC(17); FUNC(18); FUNC(19); \
	ERASE4

#define REP24(FUNC) 						\
    FUNC( 0); FUNC( 1); FUNC( 2); FUNC( 3); \
	FUNC( 4); FUNC( 5); FUNC( 6); FUNC( 7); \
	FUNC( 8); FUNC( 9); FUNC(10); FUNC(11); \
	FUNC(12); FUNC(13); FUNC(14); FUNC(15); \
	FUNC(16); FUNC(17); FUNC(18); FUNC(19); \
	FUNC(20); FUNC(21); FUNC(22); FUNC(23)

#define REP32(FUNC) 						\
    ERASE8;                                 \
	FUNC( 0); FUNC( 1); FUNC( 2); FUNC( 3); \
	FUNC( 4); FUNC( 5); FUNC( 6); FUNC( 7); \
	FUNC( 8); FUNC( 9); FUNC(10); FUNC(11); \
	FUNC(12); FUNC(13); FUNC(14); FUNC(15); \
	FUNC(16); FUNC(17); FUNC(18); FUNC(19); \
	FUNC(20); FUNC(21); FUNC(22); FUNC(23); \
	FUNC(24); FUNC(25); FUNC(26); FUNC(27); \
	FUNC(28); FUNC(29); FUNC(30); FUNC(31); \
	ERASE8

#define REP40(FUNC) 						\
    ERASE4;                                 \
	FUNC( 0); FUNC( 1); FUNC( 2); FUNC( 3); \
	FUNC( 4); FUNC( 5); FUNC( 6); FUNC( 7); \
	FUNC( 8); FUNC( 9); FUNC(10); FUNC(11); \
	FUNC(12); FUNC(13); FUNC(14); FUNC(15); \
	FUNC(16); FUNC(17); FUNC(18); FUNC(19); \
	FUNC(20); FUNC(21); FUNC(22); FUNC(23); \
	FUNC(24); FUNC(25); FUNC(26); FUNC(27); \
	FUNC(28); FUNC(29); FUNC(30); FUNC(31); \
	FUNC(32); FUNC(33); FUNC(34); FUNC(35); \
	FUNC(36); FUNC(37); FUNC(38); FUNC(39); \
	ERASE4

#define REP48(FUNC) 						\
	FUNC( 0); FUNC( 1); FUNC( 2); FUNC( 3); \
	FUNC( 4); FUNC( 5); FUNC( 6); FUNC( 7); \
	FUNC( 8); FUNC( 9); FUNC(10); FUNC(11); \
	FUNC(12); FUNC(13); FUNC(14); FUNC(15); \
	FUNC(16); FUNC(17); FUNC(18); FUNC(19); \
	FUNC(20); FUNC(21); FUNC(22); FUNC(23); \
	FUNC(24); FUNC(25); FUNC(26); FUNC(27); \
	FUNC(28); FUNC(29); FUNC(30); FUNC(31); \
	FUNC(32); FUNC(33); FUNC(34); FUNC(35); \
	FUNC(36); FUNC(37); FUNC(38); FUNC(39); \
	FUNC(40); FUNC(41); FUNC(42); FUNC(43); \
	FUNC(44); FUNC(45); FUNC(46); FUNC(47);

typedef void (*renderer_function)(VIDEO *video);

/*----------- defined in video/antic.c -----------*/

extern ANTIC antic;

void antic_reset(void);

 READ8_HANDLER ( atari_antic_r );
WRITE8_HANDLER ( atari_antic_w );

void antic_mode_0_xx(VIDEO *video);
void antic_mode_2_32(VIDEO *video);
void antic_mode_2_40(VIDEO *video);
void antic_mode_2_48(VIDEO *video);
void antic_mode_3_32(VIDEO *video);
void antic_mode_3_40(VIDEO *video);
void antic_mode_3_48(VIDEO *video);
void antic_mode_4_32(VIDEO *video);
void antic_mode_4_40(VIDEO *video);
void antic_mode_4_48(VIDEO *video);
void antic_mode_5_32(VIDEO *video);
void antic_mode_5_40(VIDEO *video);
void antic_mode_5_48(VIDEO *video);
void antic_mode_6_32(VIDEO *video);
void antic_mode_6_40(VIDEO *video);
void antic_mode_6_48(VIDEO *video);
void antic_mode_7_32(VIDEO *video);
void antic_mode_7_40(VIDEO *video);
void antic_mode_7_48(VIDEO *video);
void antic_mode_8_32(VIDEO *video);
void antic_mode_8_40(VIDEO *video);
void antic_mode_8_48(VIDEO *video);
void antic_mode_9_32(VIDEO *video);
void antic_mode_9_40(VIDEO *video);
void antic_mode_9_48(VIDEO *video);
void antic_mode_a_32(VIDEO *video);
void antic_mode_a_40(VIDEO *video);
void antic_mode_a_48(VIDEO *video);
void antic_mode_b_32(VIDEO *video);
void antic_mode_b_40(VIDEO *video);
void antic_mode_b_48(VIDEO *video);
void antic_mode_c_32(VIDEO *video);
void antic_mode_c_40(VIDEO *video);
void antic_mode_c_48(VIDEO *video);
void antic_mode_d_32(VIDEO *video);
void antic_mode_d_40(VIDEO *video);
void antic_mode_d_48(VIDEO *video);
void antic_mode_e_32(VIDEO *video);
void antic_mode_e_40(VIDEO *video);
void antic_mode_e_48(VIDEO *video);
void antic_mode_f_32(VIDEO *video);
void antic_mode_f_40(VIDEO *video);
void antic_mode_f_48(VIDEO *video);

/*----------- defined in video/atari.c -----------*/

extern char atari_frame_message[64+1];
extern int atari_frame_counter;

extern VIDEO_START( atari );
extern VIDEO_UPDATE( atari );

void a400_interrupt(void);
void a800_interrupt(void);
void a800xl_interrupt(void);
void a5200_interrupt(void);

/*----------- defined in drivers/maxaflex.c -----------*/

int atari_input_disabled(void);

#endif /* ATARI_H */

