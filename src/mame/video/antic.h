// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/***************************************************************************

    Atari 400/800

    ANTIC video controller

    Juergen Buchmueller, June 1998

***************************************************************************/

#ifndef __ANTIC_H__
#define __ANTIC_H__

#include "video/gtia.h"


#define CYCLES_PER_LINE 114     /* total number of cpu cycles per scanline (incl. hblank) */
#define CYCLES_REFRESH  9       /* number of cycles lost for ANTICs RAM refresh using DMA */
#define CYCLES_HSTART   32      /* where does the ANTIC DMA fetch start */
#define CYCLES_DLI_NMI  7       /* number of cycles until the CPU recognizes a DLI */
#define CYCLES_HSYNC    104     /* where does the HSYNC position of a scanline start */

#define VBL_END         8       /* vblank ends in this scanline */
#define VDATA_START     11      /* video display begins in this scanline */
#define VDATA_END       244     /* video display ends in this scanline */
#define VBL_START       248     /* vblank starts in this scanline */

/* total number of lines per frame (incl. vblank) */
#define TOTAL_LINES_60HZ 262
#define TOTAL_LINES_50HZ 312

/* frame rates */
#define FRAME_RATE_50HZ (double)1789790/114/TOTAL_LINES_50HZ
#define FRAME_RATE_60HZ (double)1789790/114/TOTAL_LINES_60HZ

#define HWIDTH          48      /* total characters per line */
#define HCHARS          44      /* visible characters per line */
#define VHEIGHT         32
#define VCHARS          (VDATA_END-VDATA_START+7)/8
#define BUF_OFFS0       (HWIDTH-HCHARS)/2
#define MIN_X           ((HWIDTH-42)/2)*8
#define MAX_X           MIN_X+42*8-1
#define MIN_Y           VDATA_START
#define MAX_Y           VDATA_END-8-1

#define PMOFFSET        32      /* # of pixels to adjust p/m hpos */

#define VPAGE           0xf000  /* 4K page mask for video data src */
#define VOFFS           0x0fff  /* 4K offset mask for video data src */
#define DPAGE           0xfc00  /* 1K page mask for display list */
#define DOFFS           0x03ff  /* 1K offset mask for display list */

#define DLI_NMI         0x80    /* 10000000b bit mask for display list interrupt */
#define VBL_NMI         0x40    /* 01000000b bit mask for vertical blank interrupt */

#define ANTIC_DLI       0x80    /* 10000000b cmds with display list intr    */
#define ANTIC_LMS       0x40    /* 01000000b cmds with load memory scan     */
#define ANTIC_VSCR      0x20    /* 00100000b cmds with vertical scroll      */
#define ANTIC_HSCR      0x10    /* 00010000b cmds with horizontal scroll    */
#define ANTIC_MODE      0x0f    /* 00001111b cmd mode mask                  */

#define DMA_ANTIC       0x20    /* 00100000b ANTIC DMA enable               */
#define DMA_PM_DBLLINE  0x10    /* 00010000b double line player/missile     */
#define DMA_PLAYER      0x08    /* 00001000b player DMA enable              */
#define DMA_MISSILE     0x04    /* 00000100b missile DMA enable             */

#define OFS_MIS_SINGLE  3*256   /* offset missiles single line DMA          */
#define OFS_PL0_SINGLE  4*256   /* offset player 0 single line DMA          */
#define OFS_PL1_SINGLE  5*256   /* offset player 1 single line DMA          */
#define OFS_PL2_SINGLE  6*256   /* offset player 2 single line DMA          */
#define OFS_PL3_SINGLE  7*256   /* offset player 3 single line DMA          */

#define OFS_MIS_DOUBLE  3*128   /* offset missiles double line DMA          */
#define OFS_PL0_DOUBLE  4*128   /* offset player 0 double line DMA          */
#define OFS_PL1_DOUBLE  5*128   /* offset player 1 double line DMA          */
#define OFS_PL2_DOUBLE  6*128   /* offset player 2 double line DMA          */
#define OFS_PL3_DOUBLE  7*128   /* offset player 3 double line DMA          */

#define PFD     0x00    /* 00000000b playfield default color */

#define PBK     0x00    /* 00000000b playfield background */
#define PF0     0x01    /* 00000001b playfield color #0   */
#define PF1     0x02    /* 00000010b playfield color #1   */
#define PF2     0x04    /* 00000100b playfield color #2   */
#define PF3     0x08    /* 00001000b playfield color #3   */
#define PL0     0x11    /* 00010001b player #0            */
#define PL1     0x12    /* 00010010b player #1            */
#define PL2     0x14    /* 00010100b player #2            */
#define PL3     0x18    /* 00011000b player #3            */
#define MI0     0x21    /* 00100001b missile #0           */
#define MI1     0x22    /* 00100010b missile #1           */
#define MI2     0x24    /* 00100100b missile #2           */
#define MI3     0x28    /* 00101000b missile #3           */
#define T00     0x40    /* 01000000b text mode pixels 00  */
#define P000    0x48    /* 01001000b player #0 pixels 00  */
#define P100    0x4a    /* 01001010b player #1 pixels 00  */
#define P200    0x4c    /* 01001100b player #2 pixels 00  */
#define P300    0x4e    /* 01001110b player #3 pixels 00  */
#define P400    0x4f    /* 01001111b missiles  pixels 00  */
#define T01     0x50    /* 01010000b text mode pixels 01  */
#define P001    0x58    /* 01011000b player #0 pixels 01  */
#define P101    0x5a    /* 01011010b player #1 pixels 01  */
#define P201    0x5c    /* 01011100b player #2 pixels 01  */
#define P301    0x5e    /* 01011110b player #3 pixels 01  */
#define P401    0x5f    /* 01011111b missiles  pixels 01  */
#define T10     0x60    /* 01100000b text mode pixels 10  */
#define P010    0x68    /* 01101000b player #0 pixels 10  */
#define P110    0x6a    /* 01101010b player #1 pixels 10  */
#define P210    0x6c    /* 01101100b player #2 pixels 10  */
#define P310    0x6e    /* 01101110b player #3 pixels 10  */
#define P410    0x6f    /* 01101111b missiles  pixels 10  */
#define T11     0x70    /* 01110000b text mode pixels 11  */
#define P011    0x78    /* 01111000b player #0 pixels 11  */
#define P111    0x7a    /* 01111010b player #1 pixels 11  */
#define P211    0x7c    /* 01111100b player #2 pixels 11  */
#define P311    0x7e    /* 01111110b player #3 pixels 11  */
#define P411    0x7f    /* 01111111b missiles  pixels 11  */
#define G00     0x80    /* 10000000b hires gfx pixels 00  */
#define G01     0x90    /* 10010000b hires gfx pixels 01  */
#define G10     0xa0    /* 10100000b hires gfx pixels 10  */
#define G11     0xb0    /* 10110000b hires gfx pixels 11  */
#define GT1     0xc0    /* 11000000b gtia mode 1          */
#define GT2     0xd0    /* 11010000b gtia mode 2          */
#define GT3     0xe0    /* 11100000b gtia mode 3          */
#define ILL     0xfe    /* 11111110b illegal priority     */
#define EOR     0xff    /* 11111111b EOR mode color       */

#define LUM     0x0f    /* 00001111b luminance bits       */
#define HUE     0xf0    /* 11110000b hue bits             */

#define TRIGGER_VBLANK  64715
#define TRIGGER_STEAL   64716
#define TRIGGER_HSYNC   64717


/*****************************************************************************
 * If your memcpy does not expand too well if you use it with constant
 * size_t field, you might want to define these macros somehow different.
 * NOTE: dst is not necessarily uint32_t aligned (because of horz scrolling)!
 *****************************************************************************/
#define COPY4(dst,s1) *dst++ = s1
#define COPY8(dst,s1,s2) *dst++ = s1; *dst++ = s2
#define COPY16(dst,s1,s2,s3,s4) *dst++ = s1; *dst++ = s2; *dst++ = s3; *dst++ = s4

#define RDANTIC(space)      space.read_byte(m_dpage+m_doffs)
#define RDVIDEO(space,o)    space.read_byte(m_vpage+((m_voffs+(o))&VOFFS))
#define RDCHGEN(space,o)    space.read_byte(m_chbase+(o))
#define RDPMGFXS(space,o)   space.read_byte(m_pmbase_s+(o)+(m_scanline>>1))
#define RDPMGFXD(space,o)   space.read_byte(m_pmbase_d+(o)+m_scanline)

#define PREPARE()                                               \
	uint32_t *dst = (uint32_t *)&m_cclock[PMOFFSET]

#define PREPARE_TXT2(space,width)                               \
	uint32_t *dst = (uint32_t *)&m_cclock[PMOFFSET];            \
	for (int i = 0; i < width; i++)                             \
	{                                                           \
		uint16_t ch = RDVIDEO(space,i) << 3;                      \
		if (ch & 0x400)                                         \
		{                                                       \
			ch = RDCHGEN(space,(ch & 0x3f8) + m_w.chbasl);  \
			ch = (ch ^ m_chxor) & m_chand;              \
		}                                                       \
		else                                                    \
		{                                                       \
			ch = RDCHGEN(space,ch + m_w.chbasl);            \
		}                                                       \
		video->data[i] = ch;                                    \
	}

#define PREPARE_TXT3(space,width)                               \
	uint32_t *dst = (uint32_t *)&m_cclock[PMOFFSET];            \
	for (int i = 0; i < width; i++)                             \
	{                                                           \
		uint16_t ch = RDVIDEO(space,i) << 3;                      \
		if (ch & 0x400)                                         \
		{                                                       \
			ch &= 0x3f8;                                        \
			if ((ch & 0x300) == 0x300)                          \
			{                                                   \
				if (m_w.chbasl < 2) /* first two lines empty */ \
					ch = 0x00;                                  \
				else /* lines 2..7 are standard, 8&9 are 0&1 */ \
					ch = RDCHGEN(space,ch + (m_w.chbasl & 7));\
			}                                                   \
			else                                                \
			{                                                   \
				if (m_w.chbasl > 7) /* last two lines empty */  \
					ch = 0x00;                                  \
				else /* lines 0..7 are standard */              \
					ch = RDCHGEN(space,ch + m_w.chbasl);    \
			}                                                   \
			ch = (ch ^ m_chxor) & m_chand;              \
		}                                                       \
		else                                                    \
		{                                                       \
			if ((ch & 0x300) == 0x300)                          \
			{                                                   \
				if (m_w.chbasl < 2) /* first two lines empty */ \
					ch = 0x00;                                  \
				else /* lines 2..7 are standard, 8&9 are 0&1 */ \
					ch = RDCHGEN(space,ch + (m_w.chbasl & 7));\
			}                                                   \
			else                                                \
			{                                                   \
				if (m_w.chbasl > 7) /* last two lines empty */  \
					ch = 0x00;                                  \
				else /* lines 0..7 are standard */              \
					ch = RDCHGEN(space,ch + m_w.chbasl);    \
			}                                                   \
		}                                                       \
		video->data[i] = ch;                                    \
	}

#define PREPARE_TXT45(space,width,shift)                        \
	uint32_t *dst = (uint32_t *)&m_cclock[PMOFFSET];            \
	for (int i = 0; i < width; i++)                             \
	{                                                           \
		uint16_t ch = RDVIDEO(space,i) << 3;                      \
		ch = ((ch>>2)&0x100)|RDCHGEN(space,(ch&0x3f8)+(m_w.chbasl>>shift)); \
		video->data[i] = ch;                                    \
	}


#define PREPARE_TXT67(space,width,shift)                        \
	uint32_t *dst = (uint32_t *)&m_cclock[PMOFFSET];            \
	for (int i = 0; i < width; i++)                             \
	{                                                           \
		uint16_t ch = RDVIDEO(space,i) << 3;                      \
		ch = (ch&0x600)|(RDCHGEN(space,(ch&0x1f8)+(m_w.chbasl>>shift))<<1); \
		video->data[i] = ch;                                    \
	}

#define PREPARE_GFX8(space,width)                               \
	uint32_t *dst = (uint32_t *)&m_cclock[PMOFFSET];            \
	for (int i = 0; i < width; i++)                             \
		video->data[i] = RDVIDEO(space,i) << 2

#define PREPARE_GFX9BC(space,width)                             \
	uint32_t *dst = (uint32_t *)&m_cclock[PMOFFSET];            \
	for (int i = 0; i < width; i++)                             \
		video->data[i] = RDVIDEO(space,i) << 1

#define PREPARE_GFXA(space,width)                               \
	uint32_t *dst = (uint32_t *)&m_cclock[PMOFFSET];            \
	for (int i = 0; i < width; i++)                             \
		video->data[i] = RDVIDEO(space,i) << 1

#define PREPARE_GFXDE(space,width)                              \
	uint32_t *dst = (uint32_t *)&m_cclock[PMOFFSET];            \
	for (int i = 0; i < width; i++)                             \
		video->data[i] = RDVIDEO(space,i)

#define PREPARE_GFXF(space,width)                               \
	uint32_t *dst = (uint32_t *)&m_cclock[PMOFFSET];            \
	for (int i = 0; i < width; i++)                             \
		video->data[i] = RDVIDEO(space,i)

#define PREPARE_GFXG1(space,width)                              \
	uint32_t *dst = (uint32_t *)&m_cclock[PMOFFSET];            \
	for (int i = 0; i < width; i++)                             \
		video->data[i] = RDVIDEO(space,i)

#define PREPARE_GFXG2(space,width)                              \
	uint32_t *dst = (uint32_t *)&m_cclock[PMOFFSET];            \
	for (int i = 0; i < width; i++)                             \
		video->data[i] = RDVIDEO(space,i)

#define PREPARE_GFXG3(space,width)                              \
	uint32_t *dst = (uint32_t *)&m_cclock[PMOFFSET];            \
	for (int i = 0; i < width; i++)                             \
		video->data[i] = RDVIDEO(space,i)

/******************************************************************
 * common end of a single antic/gtia mode emulation function
 ******************************************************************/
#define POST()                                                  \
	--m_modelines

#define POST_GFX(width)                                         \
	m_steal_cycles += width;                                \
	if (--m_modelines == 0)                                 \
		m_voffs = (m_voffs + width) & VOFFS

#define POST_TXT(width)                                         \
	m_steal_cycles += width;                                \
	if (--m_modelines == 0)                                 \
		m_voffs = (m_voffs + width) & VOFFS;            \
	else if (m_w.chactl & 4)                                \
		m_w.chbasl--;                                       \
	else                                                        \
		m_w.chbasl++

/* erase a number of color clocks to background color PBK */
#define ERASE(size)                         \
	for (int i = 0; i < size; i++)          \
	{                                       \
		*dst++ = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK;  \
	}
#define ZAP48()                                                 \
	dst = (uint32_t *)&antic.cclock[PMOFFSET];                    \
	dst[ 0] = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK;     \
	dst[ 1] = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK;     \
	dst[ 2] = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK;     \
	dst[45] = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK;     \
	dst[46] = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK;     \
	dst[47] = (PBK << 24) | (PBK << 16) | (PBK << 8) | PBK

#define REP(FUNC, size)                     \
	for (int i = 0; i < size; i++)          \
	{                                       \
		FUNC(i);                            \
	}


struct ANTIC_R {
	uint8_t   antic00;    /* 00 nothing */
	uint8_t   antic01;    /* 01 nothing */
	uint8_t   antic02;    /* 02 nothing */
	uint8_t   antic03;    /* 03 nothing */
	uint8_t   antic04;    /* 04 nothing */
	uint8_t   antic05;    /* 05 nothing */
	uint8_t   antic06;    /* 06 nothing */
	uint8_t   antic07;    /* 07 nothing */
	uint8_t   antic08;    /* 08 nothing */
	uint8_t   antic09;    /* 09 nothing */
	uint8_t   antic0a;    /* 0a nothing */
	uint8_t   vcount;     /* 0b vertical (scanline) counter */
	uint8_t   penh;       /* 0c light pen horizontal pos */
	uint8_t   penv;       /* 0d light pen vertical pos */
	uint8_t   antic0e;    /* 0e nothing */
	uint8_t   nmist;      /* 0f NMI status */
};  /* read registers */

struct ANTIC_W {
	uint8_t   dmactl;     /* 00 write DMA control */
	uint8_t   chactl;     /* 01 write character control */
	uint8_t   dlistl;     /* 02 display list low */
	uint8_t   dlisth;     /* 03 display list high */
	uint8_t   hscrol;     /* 04 horz scroll */
	uint8_t   vscrol;     /* 05 vert scroll */
	uint8_t   pmbasl;     /* 06 player/missile base addr low */
	uint8_t   pmbash;     /* 07 player/missile base addr high */
	uint8_t   chbasl;     /* 08 character generator base addr low */
	uint8_t   chbash;     /* 09 character generator base addr high */
	uint8_t   wsync;      /* 0a wait for hsync */
	uint8_t   antic0b;    /* 0b nothing */
	uint8_t   antic0c;    /* 0c nothing */
	uint8_t   antic0d;    /* 0d nothing */
	uint8_t   nmien;      /* 0e NMI enable */
	uint8_t   nmires;     /* 0f NMI reset */
};  /* write registers */

/* per scanline buffer for video data (and optimization variables) */
struct VIDEO {
	uint32_t  cmd;                /* antic command for this scanline */
	uint16_t  data[HWIDTH];       /* graphics data buffer (text through chargen) */
};


class antic_device :  public device_t,
								public device_video_interface
{
public:
	// construction/destruction
	antic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static void set_gtia_tag(device_t &device, const char *tag) { downcast<antic_device &>(device).m_gtia_tag = tag; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void generic_interrupt(int button_count);

private:
	static const device_timer_id TIMER_CYCLE_STEAL = 0;
	static const device_timer_id TIMER_ISSUE_DLI = 1;
	static const device_timer_id TIMER_LINE_REND = 2;
	static const device_timer_id TIMER_LINE_DONE = 3;

	const char *m_gtia_tag;
	gtia_device  *m_gtia;
	required_device<cpu_device> m_maincpu;
	optional_ioport m_djoy_b;
	optional_ioport m_artifacts;

	uint32_t m_tv_artifacts;
	int m_render1, m_render2, m_render3;

	inline void mode_0(address_space &space, VIDEO *video);
	inline void mode_2(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_3(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_4(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_5(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_6(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_7(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_8(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_9(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_a(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_b(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_c(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_d(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_e(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_f(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_gtia1(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_gtia2(address_space &space, VIDEO *video, int bytes, int erase);
	inline void mode_gtia3(address_space &space, VIDEO *video, int bytes, int erase);

	uint32_t  m_cmd;                /* currently executed display list command */
	uint32_t  m_steal_cycles;       /* steal how many cpu cycles for this line ? */
	uint32_t  m_vscrol_old;         /* old vscrol value */
	uint32_t  m_hscrol_old;         /* old hscrol value */
	int32_t   m_modelines;          /* number of lines for current ANTIC mode */
	uint32_t  m_chbase;             /* character mode source base */
	uint32_t  m_chand;              /* character and mask (chactl) */
	uint32_t  m_chxor;              /* character xor mask (chactl) */
	uint32_t  m_scanline;           /* current scan line */
	uint32_t  m_pfwidth;            /* playfield width */
	uint32_t  m_dpage;              /* display list address page */
	uint32_t  m_doffs;              /* display list offset into page */
	uint32_t  m_vpage;              /* video data source page */
	uint32_t  m_voffs;              /* video data offset into page */
	uint32_t  m_pmbase_s;           /* p/m graphics single line source base */
	uint32_t  m_pmbase_d;           /* p/m graphics double line source base */
	ANTIC_R m_r;                  /* ANTIC read registers */
	ANTIC_W m_w;                  /* ANTIC write registers */
	uint8_t   m_cclock[256+32];     /* color clock buffer filled by ANTIC */
	uint8_t   m_pmbits[256+32];     /* player missile buffer filled by GTIA */
	std::unique_ptr<uint8_t[]>   m_prio_table[64];    /* player/missile priority tables */
	VIDEO   *m_video[312];        /* video buffer */
	std::unique_ptr<uint32_t[]>  m_cclk_expand;       /* shared buffer for the following: */
	uint32_t  *m_pf_21;             /* 1cclk 2 color txt 2,3 */
	uint32_t  *m_pf_x10b;           /* 1cclk 4 color txt 4,5, gfx D,E */
	uint32_t  *m_pf_3210b2;         /* 1cclk 5 color txt 6,7, gfx 9,B,C */
	uint32_t  *m_pf_210b4;          /* 4cclk 4 color gfx 8 */
	uint32_t  *m_pf_210b2;          /* 2cclk 4 color gfx A */
	uint32_t  *m_pf_1b;             /* 1cclk hires gfx F */
	uint32_t  *m_pf_gtia1;          /* 1cclk gtia mode 1 */
	uint32_t  *m_pf_gtia2;          /* 1cclk gtia mode 2 */
	uint32_t  *m_pf_gtia3;          /* 1cclk gtia mode 3 */
	std::unique_ptr<uint8_t[]>   m_used_colors;       /* shared buffer for the following: */
	uint8_t   *m_uc_21;             /* used colors for txt (2,3) */
	uint8_t   *m_uc_x10b;           /* used colors for txt 4,5, gfx D,E */
	uint8_t   *m_uc_3210b2;         /* used colors for txt 6,7, gfx 9,B,C */
	uint8_t   *m_uc_210b4;          /* used colors for gfx 8 */
	uint8_t   *m_uc_210b2;          /* used colors for gfx A */
	uint8_t   *m_uc_1b;             /* used colors for gfx F */
	uint8_t   *m_uc_g1;             /* used colors for gfx GTIA 1 */
	uint8_t   *m_uc_g2;             /* used colors for gfx GTIA 2 */
	uint8_t   *m_uc_g3;             /* used colors for gfx GTIA 3 */
	std::unique_ptr<bitmap_ind16> m_bitmap;

	void prio_init();
	void cclk_init();

	void artifacts_gfx(uint8_t *src, uint8_t *dst, int width);
	void artifacts_txt(uint8_t *src, uint8_t *dst, int width);

	void linerefresh();
	int cycle();

	TIMER_CALLBACK_MEMBER( issue_dli );
	TIMER_CALLBACK_MEMBER( line_done );
	TIMER_CALLBACK_MEMBER( steal_cycles );
	TIMER_CALLBACK_MEMBER( scanline_render );

	void render(address_space &space, int param1, int param2, int param3);

	inline void LMS(int new_cmd);
	void scanline_dma(int param);
};


// device type definition
extern const device_type ATARI_ANTIC;



#define MCFG_ANTIC_GTIA(_tag) \
	antic_device::set_gtia_tag(*device, _tag);


#endif /* __GTIA_H__ */
