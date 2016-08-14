// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz
#ifndef _GBA_H_
#define _GBA_H_

#include "sound/gb.h"
#include "machine/intelfsh.h"
#include "bus/gba/gba_slot.h"
#include "sound/dac.h"

#define WORD(x)              (m_io_regs[(x) / 4])              /* 32-bit Register */   
#define HWHI(x)              (m_io_regs[(x) / 4] >> 16)        /* 16-bit Register, Upper Half-Word */
#define HWLO(x)              (m_io_regs[(x) / 4] & 0x0000ffff) /* 16-bit Register, Lower Half-Word */

#define WORD_SET(x, y)       (m_io_regs[(x) / 4] |= (y))
#define HWHI_SET(x, y)      ((m_io_regs[(x) / 4] |= (y << 16)))
#define HWLO_SET(x, y)       (m_io_regs[(x) / 4] |= (y))

#define WORD_RESET(x, y)     (m_io_regs[(x) / 4] &= ~(y))
#define HWHI_RESET(x, y)    ((m_io_regs[(x) / 4] &= ~(y << 16)))
#define HWLO_RESET(x, y)     (m_io_regs[(x) / 4] &= ~(y))

/* LCD I/O Registers */
#define DISPCNT     HWLO(0x000)  /* 0x4000000  2  R/W   LCD Control */
#define GRNSWAP     HWHI(0x000)  /* 0x4000002  2  R/W   Undocumented - Green Swap */
#define DISPSTAT    HWLO(0x004)  /* 0x4000004  2  R/W   General LCD Status (STAT,LYC) */
#define VCOUNT      HWHI(0x004)  /* 0x4000006  2  R     Vertical Counter (LY) */
#define BG0CNT      HWLO(0x008)  /* 0x4000008  2  R/W   BG0 Control */
#define BG1CNT      HWHI(0x008)  /* 0x400000A  2  R/W   BG1 Control */
#define BG2CNT      HWLO(0x00C)  /* 0x400000C  2  R/W   BG2 Control */
#define BG3CNT      HWHI(0x00C)  /* 0x400000E  2  R/W   BG3 Control */
#define BG0HOFS     HWLO(0x010)  /* 0x4000010  2  W     BG0 X-Offset */
#define BG0VOFS     HWHI(0x010)  /* 0x4000012  2  W     BG0 Y-Offset */
#define BG1HOFS     HWLO(0x014)  /* 0x4000014  2  W     BG1 X-Offset */
#define BG1VOFS     HWHI(0x014)  /* 0x4000016  2  W     BG1 Y-Offset */
#define BG2HOFS     HWLO(0x018)  /* 0x4000018  2  W     BG2 X-Offset */
#define BG2VOFS     HWHI(0x018)  /* 0x400001A  2  W     BG2 Y-Offset */
#define BG3HOFS     HWLO(0x01C)  /* 0x400001C  2  W     BG3 X-Offset */
#define BG3VOFS     HWHI(0x01C)  /* 0x400001E  2  W     BG3 Y-Offset */
#define BG2PA       HWLO(0x020)  /* 0x4000020  2  W     BG2 Rotation/Scaling Parameter A (dx) */
#define BG2PB       HWHI(0x020)  /* 0x4000022  2  W     BG2 Rotation/Scaling Parameter B (dmx) */
#define BG2PC       HWLO(0x024)  /* 0x4000024  2  W     BG2 Rotation/Scaling Parameter C (dy) */
#define BG2PD       HWHI(0x024)  /* 0x4000026  2  W     BG2 Rotation/Scaling Parameter D (dmy) */
#define BG2X        WORD(0x028)  /* 0x4000028  4  W     BG2 Reference Point X-Coordinate */
#define BG2Y        WORD(0x02C)  /* 0x400002C  4  W     BG2 Reference Point Y-Coordinate */
#define BG3PA       HWLO(0x030)  /* 0x4000030  2  W     BG3 Rotation/Scaling Parameter A (dx) */
#define BG3PB       HWHI(0x030)  /* 0x4000032  2  W     BG3 Rotation/Scaling Parameter B (dmx) */
#define BG3PC       HWLO(0x034)  /* 0x4000034  2  W     BG3 Rotation/Scaling Parameter C (dy) */
#define BG3PD       HWHI(0x034)  /* 0x4000036  2  W     BG3 Rotation/Scaling Parameter D (dmy) */
#define BG3X        WORD(0x038)  /* 0x4000038  4  W     BG3 Reference Point X-Coordinate */
#define BG3Y        WORD(0x03C)  /* 0x400003C  4  W     BG3 Reference Point Y-Coordinate */
#define WIN0H       HWLO(0x040)  /* 0x4000040  2  W     Window 0 Horizontal Dimensions */
#define WIN1H       HWHI(0x040)  /* 0x4000042  2  W     Window 1 Horizontal Dimensions */
#define WIN0V       HWLO(0x044)  /* 0x4000044  2  W     Window 0 Vertical Dimensions */
#define WIN1V       HWHI(0x044)  /* 0x4000046  2  W     Window 1 Vertical Dimensions */
#define WININ       HWLO(0x048)  /* 0x4000048  2  R/W   Inside of Window 0 and 1 */
#define WINOUT      HWHI(0x048)  /* 0x400004A  2  R/W   Inside of OBJ Window & Outside of Windows */
#define MOSAIC      HWLO(0x04C)  /* 0x400004C  2  W     Mosaic Size */
                                 /* 0x400004E  2  -     Unused */
#define BLDCNT      HWLO(0x050)  /* 0x4000050  2  R/W   Color Special Effects Selection */
#define BLDALPHA    HWHI(0x050)  /* 0x4000052  2  W     Alpha Blending Coefficients */
#define BLDY        HWLO(0x054)  /* 0x4000054  2  W     Brightness (Fade-In/Out) Coefficient */
                                 /* 0x4000056  2  -     Unused */
                                                      
/* Sound Registers */
#define SOUNDCNT_L  HWLO(0x080)  /* 0x4000080  2  R/W   Control Stereo/Volume/Enable */
#define SOUNDCNT_H  HWHI(0x080)  /* 0x4000082  2  R/W   Control Mixing/DMA Control */
#define SOUNDCNT_X  HWLO(0x084)  /* 0x4000084  2  R/W   Control Sound on/off */
                                 /* 0x4000086  2  -     Unused */
#define SOUNDBIAS   HWLO(0x088)  /* 0x4000088  2  BIOS  Sound PWM Control */
                                 /* 0x400008A  2  -     Unused */

/* DMA Transfer Channels Registers */
#define DMA0SAD     WORD(0x0B0)  /* 0x40000B0  4  W     DMA 0 Source Address */
#define DMA0DAD     WORD(0x0B4)  /* 0x40000B4  4  W     DMA 0 Destination Address */
#define DMA0CNT_L   HWLO(0x0B8)  /* 0x40000B8  2  W     DMA 0 Word Count */
#define DMA0CNT_H   HWHI(0x0B8)  /* 0x40000BA  2  R/W   DMA 0 Control */
#define DMA1SAD     WORD(0x0BC)  /* 0x40000BC  4  W     DMA 1 Source Address */
#define DMA1DAD     WORD(0x0C0)  /* 0x40000C0  4  W     DMA 1 Destination Address */
#define DMA1CNT_L   HWLO(0x0C4)  /* 0x40000C4  2  W     DMA 1 Word Count */
#define DMA1CNT_H   HWHI(0x0C4)  /* 0x40000C6  2  R/W   DMA 1 Control */
#define DMA2SAD     WORD(0x0C8)  /* 0x40000C8  4  W     DMA 2 Source Address */
#define DMA2DAD     WORD(0x0CC)  /* 0x40000CC  4  W     DMA 2 Destination Address */
#define DMA2CNT_L   HWLO(0x0D0)  /* 0x40000D0  2  W     DMA 2 Word Count */
#define DMA2CNT_H   HWHI(0x0D0)  /* 0x40000D2  2  R/W   DMA 2 Control */
#define DMA3SAD     WORD(0x0D4)  /* 0x40000D4  4  W     DMA 3 Source Address */
#define DMA3DAD     WORD(0x0D8)  /* 0x40000D8  4  W     DMA 3 Destination Address */
#define DMA3CNT_L   HWLO(0x0DC)  /* 0x40000DC  2  W     DMA 3 Word Count */
#define DMA3CNT_H   HWHI(0x0DC)  /* 0x40000DE  2  R/W   DMA 3 Control */

#define DMASAD(c)   WORD(0x0B0 + (c * 0xC))
#define DMADAD(c)   WORD(0x0B4 + (c * 0xC))
#define DMACNT_L(c) HWLO(0x0B8 + (c * 0xC))
#define DMACNT_H(c) HWHI(0x0B8 + (c * 0xC))

/* Serial Communication (1) Registers */
#define SIODATA32   WORD(0x120)  /* 0x4000120  4  R/W   SIO Data (Normal-32bit Mode; shared with below) */
#define SIOMULTI0   HWLO(0x120)  /* 0x4000120  2  R/W   SIO Data 0 (Parent)    (Multi-Player Mode) */
#define SIOMULTI1   HWHI(0x120)  /* 0x4000122  2  R/W   SIO Data 1 (1st Child) (Multi-Player Mode) */
#define SIOMULTI2   HWLO(0x124)  /* 0x4000124  2  R/W   SIO Data 2 (2nd Child) (Multi-Player Mode) */
#define SIOMULTI3   HWHI(0x124)  /* 0x4000126  2  R/W   SIO Data 3 (3rd Child) (Multi-Player Mode) */
#define SIOCNT      HWLO(0x128)  /* 0x4000128  2  R/W   SIO Control Register */
#define SIOMLT_SEND HWHI(0x128)  /* 0x400012A  2  R/W   SIO Data (Local of MultiPlayer; shared below) */
#define SIODATA8    HWHI(0x128)  /* 0x400012A  2  R/W   SIO Data (Normal-8bit and UART Mode) */
                                 /* 0x400012C  2  -     Unused */

/* Keypad Input Registers */
#define KEYINPUT    HWLO(0x130)  /* 0x4000130  2  R     Key Status */
#define KEYCNT      HWHI(0x130)  /* 0x4000132  2  R/W   Key Interrupt Control */

/* Serial Communication (2) Registers */
#define RCNT        HWLO(0x134)  /* 0x4000134  2  R/W   SIO Mode Select/General Purpose Data */
#define IR          HWHI(0x134)  /* 0x4000136  2  R/W   Ancient - Infrared Register (Prototypes only) */
                                 /* 0x4000138  8  -     Unused */
#define JOYCNT      HWLO(0x140)  /* 0x4000140  2  R/W   SIO JOY Bus Control */
                                 /* 0x4000142  2  -     Unused */
#define JOY_RECV    WORD(0x150)  /* 0x4000150  4  R/W   SIO JOY Bus Receive Data */
#define JOY_TRANS   WORD(0x154)  /* 0x4000154  4  R/W   SIO JOY Bus Transmit Data */
#define JOYSTAT     HWLO(0x158)  /* 0x4000158  2  R/?   SIO JOY Bus Receive Status */
                                 /* 0x400015A  2  -     Unused */

/* Interrupt, Waitstate, and Power-Down Control Registers */
#define IE          HWLO(0x200)  /* 0x4000200  2  R/W   Interrupt Enable Register */
#define IF          HWHI(0x200)  /* 0x4000202  2  R/W   Interrupt Request Flags / IRQ Acknowledge */
#define WAITCNT     HWLO(0x204)  /* 0x4000204  2  R/W   Game Pak Waitstate Control */
                                 /* 0x4000206     -     Unused */
#define IME         HWLO(0x208)  /* 0x4000208  2  R/W   Interrupt Master Enable Register */
                                 /* 0x400020A     -     Unused */
                                 /* 0x4000300  1  R/W   Undocumented - Post Boot Flag */
                                 /* 0x4000301  1  W     Undocumented - Power Down Control */
                                 /* 0x4000302     -     Unused */
                                 /* 0x4000410  ?  ?     Undocumented - Purpose Unknown / Bug ??? 0FFh */
                                 /* 0x4000411     -     Unused */
                                 /* 0x4000800  4  R/W   Undocumented - Internal Memory Control (R/W) */
                                 /* 0x4000804     -     Unused */
                                 /* 0x4xx0800  4  R/W   Mirrors of 4000800h (repeated each 64K) */

#define DISPSTAT_SET(val)       HWLO_SET(0x004, val)
#define DISPSTAT_RESET(val)     HWLO_RESET(0x004, val)

#define SOUNDBIAS_SET(val)      HWLO_SET(0x088, val)

#define DMASAD_SET(c, val)      WORD_SET(0x0B0 + (c * 0xC), val)
#define DMADAD_SET(c, val)      WORD_SET(0x0B4 + (c * 0xC), val)
#define DMACNT_L_SET(c, val)    HWLO_SET(0x0B8 + (c * 0xC), val)
#define DMACNT_H_SET(c, val)    HWHI_SET(0x0B8 + (c * 0xC), val)
#define DMACNT_H_RESET(c, val)  HWHI_RESET(0x0B8 + (c * 0xC), val)

#define SIOMULTI0_SET(val)      HWLO_SET(0x120, val)
#define SIOMULTI1_SET(val)      HWHI_SET(0x120, val)
#define SIOMULTI2_SET(val)      HWLO_SET(0x124, val)
#define SIOMULTI3_SET(val)      HWHI_SET(0x124, val)

#define SIOCNT_RESET(val)       HWLO_RESET(0x128, val)

#define KEYCNT_SET(val)         HWHI_SET(0x130, val)

#define RCNT_SET(val)           HWLO_SET(0x134, val)

#define JOYSTAT_SET(val)        HWLO_SET(0x158, val)

#define IF_SET(val)             HWHI_SET(0x200, val)
#define IF_RESET(val)           HWHI_RESET(0x200, val)

#define DISPSTAT_VBL            0x0001
#define DISPSTAT_HBL            0x0002
#define DISPSTAT_VCNT           0x0004
#define DISPSTAT_VBL_IRQ_EN     0x0008
#define DISPSTAT_HBL_IRQ_EN     0x0010
#define DISPSTAT_VCNT_IRQ_EN    0x0020
#define DISPSTAT_VCNT_VALUE     0xff00

#define INT_VBL                 0x0001
#define INT_HBL                 0x0002
#define INT_VCNT                0x0004
#define INT_TM0_OVERFLOW        0x0008
#define INT_TM1_OVERFLOW        0x0010
#define INT_TM2_OVERFLOW        0x0020
#define INT_TM3_OVERFLOW        0x0040
#define INT_SIO                 0x0080
#define INT_DMA0                0x0100
#define INT_DMA1                0x0200
#define INT_DMA2                0x0400
#define INT_DMA3                0x0800
#define INT_KEYPAD              0x1000
#define INT_GAMEPAK             0x2000

#define DISPCNT_MODE            0x0007
#define DISPCNT_FRAMESEL        0x0010
#define DISPCNT_HBL_FREE        0x0020

#define DISPCNT_VRAM_MAP        0x0040
#define DISPCNT_VRAM_MAP_2D     0x0000
#define DISPCNT_VRAM_MAP_1D     0x0040

#define DISPCNT_BLANK           0x0080
#define DISPCNT_BG0_EN          0x0100
#define DISPCNT_BG1_EN          0x0200
#define DISPCNT_BG2_EN          0x0400
#define DISPCNT_BG3_EN          0x0800
#define DISPCNT_OBJ_EN          0x1000
#define DISPCNT_WIN0_EN         0x2000
#define DISPCNT_WIN1_EN         0x4000
#define DISPCNT_OBJWIN_EN       0x8000

#define OBJ_Y_COORD             0x00ff
#define OBJ_ROZMODE             0x0300
#define OBJ_ROZMODE_NONE        0x0000
#define OBJ_ROZMODE_ROZ         0x0100
#define OBJ_ROZMODE_DISABLE     0x0200
#define OBJ_ROZMODE_DBLROZ      0x0300

#define OBJ_MODE                0x0c00
#define OBJ_MODE_NORMAL         0x0000
#define OBJ_MODE_ALPHA          0x0400
#define OBJ_MODE_WINDOW         0x0800
#define OBJ_MODE_UNDEFINED      0x0c00

#define OBJ_MOSAIC              0x1000

#define OBJ_PALMODE             0x2000
#define OBJ_PALMODE_16          0x0000
#define OBJ_PALMODE_256         0x2000

#define OBJ_SHAPE               0xc000
#define OBJ_SHAPE_SQR           0x0000
#define OBJ_SHAPE_HORIZ         0x4000
#define OBJ_SHAPE_VERT          0x8000

#define OBJ_X_COORD             0x01ff
#define OBJ_SCALE_PARAM         0x3e00
#define OBJ_SCALE_PARAM_SHIFT   9
#define OBJ_HFLIP               0x1000
#define OBJ_VFLIP               0x2000
#define OBJ_SIZE                0xc000
#define OBJ_SIZE_8              0x0000
#define OBJ_SIZE_16             0x4000
#define OBJ_SIZE_32             0x8000
#define OBJ_SIZE_64             0xc000

#define OBJ_TILENUM             0x03ff
#define OBJ_PRIORITY            0x0c00
#define OBJ_PRIORITY_SHIFT      10
#define OBJ_PALNUM              0xf000
#define OBJ_PALNUM_SHIFT        12

#define BGCNT_SCREENSIZE        0xc000
#define BGCNT_SCREENSIZE_SHIFT  14
#define BGCNT_PALETTESET_WRAP   0x2000
#define BGCNT_SCREENBASE        0x1f00
#define BGCNT_SCREENBASE_SHIFT  8
#define BGCNT_PALETTE256        0x0080
#define BGCNT_MOSAIC            0x0040
#define BGCNT_CHARBASE          0x003c
#define BGCNT_CHARBASE_SHIFT    2
#define BGCNT_PRIORITY          0x0003

#define BLDCNT_BG0TP1           0x0001
#define BLDCNT_BG1TP1           0x0002
#define BLDCNT_BG2TP1           0x0004
#define BLDCNT_BG3TP1           0x0008
#define BLDCNT_OBJTP1           0x0010
#define BLDCNT_BDTP1            0x0020
#define BLDCNT_SFX              0x00c0
#define BLDCNT_SFX_NONE         0x0000
#define BLDCNT_SFX_ALPHA        0x0040
#define BLDCNT_SFX_LIGHTEN      0x0080
#define BLDCNT_SFX_DARKEN       0x00c0
#define BLDCNT_BG0TP2           0x0100
#define BLDCNT_BG1TP2           0x0200
#define BLDCNT_BG2TP2           0x0400
#define BLDCNT_BG3TP2           0x0800
#define BLDCNT_OBJTP2           0x1000
#define BLDCNT_BDTP2            0x2000
#define BLDCNT_TP2_SHIFT        8

#define TILEOBJ_TILE            0x03ff
#define TILEOBJ_HFLIP           0x0400
#define TILEOBJ_VFLIP           0x0800
#define TILEOBJ_PALETTE         0xf000

/* driver state */
class gba_state : public driver_device
{
public:
	gba_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gba_pram(*this, "gba_pram"),
		m_gba_vram(*this, "gba_vram"),
		m_gba_oam(*this, "gba_oam"),
		m_ladac(*this, "direct_a_left"),
		m_radac(*this, "direct_a_right"),
		m_lbdac(*this, "direct_b_left"),
		m_rbdac(*this, "direct_b_right"),
		m_gbsound(*this, "custom"),
		m_cart(*this, "cartslot"),
		m_region_maincpu(*this, "maincpu"),
		m_io_inputs(*this, "INPUTS"),
		m_bios_hack(*this, "SKIP_CHECK")
	{ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT32> m_gba_pram;
	required_shared_ptr<UINT32> m_gba_vram;
	required_shared_ptr<UINT32> m_gba_oam;
	required_device<dac_device> m_ladac;
	required_device<dac_device> m_radac;
	required_device<dac_device> m_lbdac;
	required_device<dac_device> m_rbdac;
	required_device<gameboy_sound_device> m_gbsound;
	required_device<gba_cart_slot_device> m_cart;

	void request_irq(UINT32 int_type);
	void dma_exec(int ch);
	void audio_tick(int ref);

	// video-related
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	bitmap_ind16 m_bitmap;

	UINT32 m_io_regs[0x400 / 4];

	UINT8  m_windowOn;
	UINT8  m_fxOn;
	UINT8  m_gfxBG2Changed;
	UINT8  m_gfxBG3Changed;
	INT32  m_gfxBG2X;
	INT32  m_gfxBG2Y;
	INT32  m_gfxBG3X;
	INT32  m_gfxBG3Y;

	// DMA
	emu_timer *m_dma_timer[4];
	UINT32 m_dma_src[4];
	UINT32 m_dma_dst[4];
	UINT16 m_dma_cnt[4];

	// Timers
	UINT32 m_timer_regs[4];
	UINT16 m_timer_reload[4];
	int m_timer_recalc[4];

	emu_timer *m_tmr_timer[4], *m_irq_timer;
	emu_timer *m_scan_timer, *m_hbl_timer;

	double m_timer_hz[4];

	int m_fifo_a_ptr;
	int m_fifo_b_ptr;
	int m_fifo_a_in;
	int m_fifo_b_in;
	UINT8 m_fifo_a[20];
	UINT8 m_fifo_b[20];
	UINT32 m_xferscan[7][240+2048];

	UINT32 m_bios_last_address;
	int m_bios_protected;

	DIRECT_UPDATE_MEMBER(gba_direct);
	DECLARE_READ32_MEMBER(gba_io_r);
	DECLARE_WRITE32_MEMBER(gba_io_w);
	DECLARE_WRITE32_MEMBER(gba_pram_w);
	DECLARE_WRITE32_MEMBER(gba_vram_w);
	DECLARE_WRITE32_MEMBER(gba_oam_w);
	DECLARE_READ32_MEMBER(gba_bios_r);
	DECLARE_READ32_MEMBER(gba_10000000_r);
	DECLARE_DRIVER_INIT(gbadv);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(gba);
	TIMER_CALLBACK_MEMBER(dma_complete);
	TIMER_CALLBACK_MEMBER(timer_expire);
	TIMER_CALLBACK_MEMBER(handle_irq);
	TIMER_CALLBACK_MEMBER(perform_hbl);
	TIMER_CALLBACK_MEMBER(perform_scan);

	// video related
	void draw_scanline(int y);

	void draw_roz_bitmap_scanline(UINT32 *scanline, int ypos, UINT32 enablemask, UINT32 ctrl, INT32 X, INT32 Y, INT32 PA, INT32 PB, INT32 PC, INT32 PD, INT32 *currentx, INT32 *currenty, int changed, int depth);
	void draw_roz_scanline(UINT32 *scanline, int ypos, UINT32 enablemask, UINT32 ctrl, INT32 X, INT32 Y, INT32 PA, INT32 PB, INT32 PC, INT32 PD, INT32 *currentx, INT32 *currenty, int changed);
	void draw_bg_scanline(UINT32 *scanline, int ypos, UINT32 enablemask, UINT32 ctrl, UINT32 hofs, UINT32 vofs);
	void draw_gba_oam_window(UINT32 *scanline, int y);
	void draw_gba_oam(UINT32 *scanline, int y);

	inline int is_in_window(int x, int window);

	inline void update_mask(UINT8* mask, int mode, int submode, UINT32* obj_win, UINT8 inwin0, UINT8 inwin1, UINT8 in0_mask, UINT8 in1_mask, UINT8 out_mask);
	void draw_modes(int mode, int submode, int y, UINT32* line0, UINT32* line1, UINT32* line2, UINT32* line3, UINT32* lineOBJ, UINT32* lineOBJWin, UINT32* lineMix, int bpp);

protected:
	required_region_ptr<UINT32> m_region_maincpu;
	required_ioport m_io_inputs;
	required_ioport m_bios_hack;
};

#endif
