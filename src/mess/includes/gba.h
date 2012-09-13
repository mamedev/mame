#ifndef _GBA_H_
#define _GBA_H_

#include "machine/intelfsh.h"

#define DISPSTAT_VBL			0x0001
#define DISPSTAT_HBL			0x0002
#define DISPSTAT_VCNT			0x0004
#define DISPSTAT_VBL_IRQ_EN		0x0008
#define DISPSTAT_HBL_IRQ_EN		0x0010
#define DISPSTAT_VCNT_IRQ_EN		0x0020
#define DISPSTAT_VCNT_VALUE		0xff00

#define INT_VBL				0x0001
#define INT_HBL				0x0002
#define INT_VCNT			0x0004
#define INT_TM0_OVERFLOW		0x0008
#define INT_TM1_OVERFLOW		0x0010
#define INT_TM2_OVERFLOW		0x0020
#define INT_TM3_OVERFLOW		0x0040
#define INT_SIO				0x0080
#define INT_DMA0			0x0100
#define INT_DMA1			0x0200
#define INT_DMA2			0x0400
#define INT_DMA3			0x0800
#define INT_KEYPAD			0x1000
#define INT_GAMEPAK			0x2000

#define DISPCNT_MODE			0x0007
#define DISPCNT_FRAMESEL		0x0010
#define DISPCNT_HBL_FREE		0x0020

#define DISPCNT_VRAM_MAP		0x0040
#define DISPCNT_VRAM_MAP_2D		0x0000
#define DISPCNT_VRAM_MAP_1D		0x0040

#define DISPCNT_BLANK			0x0080
#define DISPCNT_BG0_EN			0x0100
#define DISPCNT_BG1_EN			0x0200
#define DISPCNT_BG2_EN			0x0400
#define DISPCNT_BG3_EN			0x0800
#define DISPCNT_OBJ_EN			0x1000
#define DISPCNT_WIN0_EN			0x2000
#define DISPCNT_WIN1_EN			0x4000
#define DISPCNT_OBJWIN_EN		0x8000

#define OBJ_Y_COORD			0x00ff
#define OBJ_ROZMODE			0x0300
#define OBJ_ROZMODE_NONE	0x0000
#define OBJ_ROZMODE_ROZ		0x0100
#define OBJ_ROZMODE_DISABLE	0x0200
#define OBJ_ROZMODE_DBLROZ	0x0300

#define OBJ_MODE			0x0c00
#define OBJ_MODE_NORMAL			0x0000
#define OBJ_MODE_ALPHA			0x0400
#define OBJ_MODE_WINDOW			0x0800
#define OBJ_MODE_UNDEFINED		0x0c00

#define OBJ_MOSAIC			0x1000

#define OBJ_PALMODE			0x2000
#define OBJ_PALMODE_16			0x0000
#define OBJ_PALMODE_256			0x2000

#define OBJ_SHAPE			0xc000
#define OBJ_SHAPE_SQR			0x0000
#define OBJ_SHAPE_HORIZ			0x4000
#define OBJ_SHAPE_VERT			0x8000

#define OBJ_X_COORD			0x01ff
#define OBJ_SCALE_PARAM			0x3e00
#define OBJ_SCALE_PARAM_SHIFT		9
#define OBJ_HFLIP			0x1000
#define OBJ_VFLIP			0x2000
#define OBJ_SIZE			0xc000
#define OBJ_SIZE_8			0x0000
#define OBJ_SIZE_16			0x4000
#define OBJ_SIZE_32			0x8000
#define OBJ_SIZE_64			0xc000

#define OBJ_TILENUM 			0x03ff
#define OBJ_PRIORITY			0x0c00
#define OBJ_PRIORITY_SHIFT		10
#define OBJ_PALNUM			0xf000
#define OBJ_PALNUM_SHIFT		12

#define BGCNT_SCREENSIZE		0xc000
#define BGCNT_SCREENSIZE_SHIFT	14
#define BGCNT_PALETTESET_WRAP	0x2000
#define BGCNT_SCREENBASE		0x1f00
#define BGCNT_SCREENBASE_SHIFT	8
#define BGCNT_PALETTE256		0x0080
#define BGCNT_MOSAIC			0x0040
#define BGCNT_CHARBASE			0x003c
#define BGCNT_CHARBASE_SHIFT	2
#define BGCNT_PRIORITY			0x0003

#define BLDCNT_BG0TP1			0x0001
#define BLDCNT_BG1TP1			0x0002
#define BLDCNT_BG2TP1			0x0004
#define BLDCNT_BG3TP1			0x0008
#define BLDCNT_OBJTP1			0x0010
#define BLDCNT_BDTP1			0x0020
#define BLDCNT_SFX				0x00c0
#define BLDCNT_SFX_NONE			0x0000
#define BLDCNT_SFX_ALPHA		0x0040
#define BLDCNT_SFX_LIGHTEN		0x0080
#define BLDCNT_SFX_DARKEN		0x00c0
#define BLDCNT_BG0TP2			0x0100
#define BLDCNT_BG1TP2			0x0200
#define BLDCNT_BG2TP2			0x0400
#define BLDCNT_BG3TP2			0x0800
#define BLDCNT_OBJTP2			0x1000
#define BLDCNT_BDTP2			0x2000
#define BLDCNT_TP2_SHIFT		8

#define TILEOBJ_TILE			0x03ff
#define TILEOBJ_HFLIP			0x0400
#define TILEOBJ_VFLIP			0x0800
#define TILEOBJ_PALETTE			0xf000

enum
{
	EEP_IDLE,
	EEP_COMMAND,
	EEP_ADDR,
	EEP_AFTERADDR,
	EEP_READ,
	EEP_WRITE,
	EEP_AFTERWRITE,
	EEP_READFIRST
};

/* driver state */
class gba_state : public driver_device
{
public:
	gba_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_gba_pram(*this, "gba_pram"),
		m_gba_vram(*this, "gba_vram"),
		m_gba_oam(*this, "gba_oam") { }

	required_shared_ptr<UINT32> m_gba_pram;
	required_shared_ptr<UINT32> m_gba_vram;
	required_shared_ptr<UINT32> m_gba_oam;

	virtual void video_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	bitmap_ind16 m_bitmap;

	UINT32 m_DISPSTAT;
	UINT32 m_BG2X, m_BG2Y, m_BG3X, m_BG3Y;
	UINT16 m_DISPCNT,	m_GRNSWAP;
	UINT16 m_BG0CNT, m_BG1CNT, m_BG2CNT, m_BG3CNT;
	UINT16 m_BG0HOFS, m_BG0VOFS, m_BG1HOFS, m_BG1VOFS, m_BG2HOFS, m_BG2VOFS, m_BG3HOFS, m_BG3VOFS;
	UINT16 m_BG2PA, m_BG2PB, m_BG2PC, m_BG2PD, m_BG3PA, m_BG3PB, m_BG3PC, m_BG3PD;
	UINT16 m_WIN0H, m_WIN1H, m_WIN0V, m_WIN1V, m_WININ, m_WINOUT;
	UINT16 m_MOSAIC;
	UINT16 m_BLDCNT;
	UINT16 m_BLDALPHA;
	UINT16 m_BLDY;
	UINT8  m_SOUNDCNT_X;
	UINT16 m_SOUNDCNT_H;
	UINT16 m_SOUNDBIAS;
	UINT16 m_SIOMULTI0, m_SIOMULTI1, m_SIOMULTI2, m_SIOMULTI3;
	UINT16 m_SIOCNT, m_SIODATA8;
	UINT16 m_KEYCNT;
	UINT16 m_RCNT;
	UINT16 m_JOYCNT;
	UINT32 m_JOY_RECV, m_JOY_TRANS;
	UINT16 m_JOYSTAT;
	UINT16 m_IR, m_IE, m_IF, m_IME;
	UINT16 m_WAITCNT;
	UINT8  m_POSTFLG;
	UINT8  m_HALTCNT;

	UINT8  m_windowOn;
	UINT8  m_fxOn;
	UINT8  m_gfxBG2Changed;
	UINT8  m_gfxBG3Changed;
	INT32  m_gfxBG2X;
	INT32  m_gfxBG2Y;
	INT32  m_gfxBG3X;
	INT32  m_gfxBG3Y;



	UINT32 m_dma_regs[16];
	UINT32 m_dma_src[4], m_dma_dst[4], m_dma_cnt[4], m_dma_srcadd[4], m_dma_dstadd[4];
	UINT32 m_timer_regs[4];
	UINT16 m_timer_reload[4];
	int m_timer_recalc[4];

	UINT32 m_gba_sram[0x10000/4];
	UINT8 m_gba_eeprom[0x2000];
	UINT32 m_flash_size;
	UINT32 m_flash_mask;
	intelfsh8_device *m_mFlashDev;
	int m_eeprom_state, m_eeprom_command, m_eeprom_count, m_eeprom_addr, m_eeprom_bits, m_eeprom_addr_bits;
	UINT8 m_eep_data;

	/* nvram-specific for MESS */
	UINT8 *m_nvptr;
	UINT32 m_nvsize;
	device_t *m_nvimage;

	emu_timer *m_dma_timer[4], *m_tmr_timer[4], *m_irq_timer;
	emu_timer *m_scan_timer, *m_hbl_timer;

	double m_timer_hz[4];

	int m_fifo_a_ptr, m_fifo_b_ptr, m_fifo_a_in, m_fifo_b_in;
	UINT8 m_fifo_a[20], m_fifo_b[20];
	UINT32 m_xferscan[7][240+2048];

	UINT32 m_bios_last_address;
	int m_bios_protected;

	int m_flash_battery_load;

	DIRECT_UPDATE_MEMBER(gba_direct);
	DECLARE_READ32_MEMBER(gba_io_r);
	DECLARE_WRITE32_MEMBER(gba_io_w);
	DECLARE_WRITE32_MEMBER(gba_pram_w);
	DECLARE_WRITE32_MEMBER(gba_vram_w);
	DECLARE_WRITE32_MEMBER(gba_oam_w);
	DECLARE_READ32_MEMBER(gba_bios_r);
	DECLARE_READ32_MEMBER(gba_10000000_r);
	DECLARE_READ32_MEMBER(sram_r);
	DECLARE_WRITE32_MEMBER(sram_w);
	DECLARE_READ32_MEMBER(flash_r);
	DECLARE_WRITE32_MEMBER(flash_w);
	DECLARE_READ32_MEMBER(eeprom_r);
	DECLARE_WRITE32_MEMBER(eeprom_w);
	DECLARE_DRIVER_INIT(gbadv);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void palette_init();
};

/*----------- defined in video/gba.c -----------*/

void gba_draw_scanline(running_machine &machine, int y);

#endif
