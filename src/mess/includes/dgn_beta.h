// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*****************************************************************************
 *
 * includes/dgn_beta.h
 *
 ****************************************************************************/

#ifndef DGN_BETA_H_
#define DGN_BETA_H_

#include "video/mc6845.h"
#include "machine/wd_fdc.h"
#include "machine/6821pia.h"
#include "machine/ram.h"

/* Tags */

#define MAINCPU_TAG "maincpu"
#define DMACPU_TAG  "dmacpu"
#define PIA_0_TAG   "pia_0"
#define PIA_1_TAG   "pia_1"
#define PIA_2_TAG   "pia_2"
#define FDC_TAG     "wd2797"


#define DGNBETA_CPU_SPEED_HZ        2000000 /* 2MHz */
#define DGNBETA_FRAMES_PER_SECOND   50

#define RamSize             256         /* 256K by default */
#define RamPageSize         4096        /* ram pages are 4096 bytes */

#define MaxTasks            16          /* Tasks 0..15 */
#define MaxPage             16          /* 16 4K pages */
#define NoPagingTask        MaxTasks    /* Task registers to use when paging disabled 16 */

#define RAMPage             0           /* Page with RAM in at power on */
#define VideoPage           6           /* Page where video ram mapped */
#define IOPage              MaxPage-1   /* Page for I/O */
#define ROMPage             MaxPage-2   /* Page for ROM */
#define LastPage            MaxPage-1

#define RAMPageValue        0x00        /* page with RAM at power on */
#define VideoPageValue      0x1F        /* Default page for video ram */
#define NoMemPageValue      0xC0        /* Page guaranteed not to have memory in */
#define ROMPageValue        0xFE        /* Page with boot ROM */
#define IOPageValue         0xFF        /* Page with I/O & Boot ROM */

#define TextVidBasePage     0x18        /* Base page of text video ram */

/***** Keyboard stuff *****/
#define NoKeyrows           0x0a        /* Number of rows in keyboard */

/* From Dragon Beta OS9 keyboard driver */
#define KAny                0x04        /* Any key pressed mask PB2 */
#define KOutClk             0x08        /* Ouput shift register clock */
#define KInClk              0x10        /* Input shift register clock */
#define KOutDat             KInClk      /* Also used for data into output shifter */
#define KInDat              0x20        /* Keyboard data in from keyboard (serial stream) */

/***** Video Modes *****/

enum BETA_VID_MODES
{
	TEXT_40x25,             /* Text mode 40x25 */
	TEXT_80x25,             /* Text mode 80x25 */
	GRAPH_320x256x4,        /* Graphics 320x256x4 */
	GRAPH_320x256x16,       /* Graphics 320x256x16 */
	GRAPH_640x512x2         /* Graphics 640X512X2 */
};

#define iosize  (0xfEFF-0xfc00)

struct PageReg
{
	int     value;          /* Value of the page register */
	UINT8   *memory;        /* The memory it actually points to */
};


class dgn_beta_state : public driver_device
{
public:
	dgn_beta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_mc6845(*this, "crtc"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_fdc(*this, FDC_TAG),
		m_floppy0(*this, FDC_TAG ":0"),
		m_floppy1(*this, FDC_TAG ":1"),
		m_floppy2(*this, FDC_TAG ":2"),
		m_floppy3(*this, FDC_TAG ":3"),
		m_palette(*this, "palette") { }

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	required_device<mc6845_device> m_mc6845;
	required_shared_ptr<UINT8> m_videoram;

	UINT8 *m_system_rom;
	int m_LogDatWrites;
	int m_Keyboard[NoKeyrows];
	int m_RowShifter;
	int m_Keyrow;
	int m_d_pia0_pb_last;
	int m_d_pia0_cb2_last;
	int m_KInDat_next;
	int m_KAny_next;
	int m_d_pia1_pa_last;
	int m_d_pia1_pb_last;
	int m_DMA_NMI_LAST;
	int m_wd2797_written;
	int m_TaskReg;
	int m_PIATaskReg;
	int m_EnableMapRegs;
	PageReg m_PageRegs[MaxTasks+1][MaxPage+1];
	int m_beta_6845_RA;
	int m_beta_scr_x;
	int m_beta_scr_y;
	int m_beta_HSync;
	int m_beta_VSync;
	int m_beta_DE;
	int m_LogRegWrites;
	int m_BoxColour;
	int m_BoxMinX;
	int m_BoxMinY;
	int m_BoxMaxX;
	int m_BoxMaxY;
	int m_HSyncMin;
	int m_VSyncMin;
	int m_DEPos;
	int m_NoScreen;
	bitmap_ind16 *m_bit;
	int m_MinAddr;
	int m_MaxAddr;
	int m_MinX;
	int m_MaxX;
	int m_MinY;
	int m_MaxY;
	int m_VidAddr;
	int m_ClkMax;
	int m_GCtrl;
	int m_FlashCount;
	int m_FlashBit;
	int m_s_DoubleY;
	int m_DoubleHL;
	int m_ColourRAM[4];
	int m_Field;
	int m_DrawInterlace;
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_PALETTE_INIT(dgn);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_b0_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_b1_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_b2_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_b3_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_b4_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_b5_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_b6_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_b7_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_b8_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_b9_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_bA_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_bB_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_bC_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_bD_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_bE_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_bF_w);
	DECLARE_WRITE8_MEMBER(dgnbeta_ram_bG_w);
	DECLARE_READ8_MEMBER(d_pia0_pa_r);
	DECLARE_WRITE8_MEMBER(d_pia0_pa_w);
	DECLARE_READ8_MEMBER(d_pia0_pb_r);
	DECLARE_WRITE8_MEMBER(d_pia0_pb_w);
	DECLARE_WRITE_LINE_MEMBER(d_pia0_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(d_pia0_irq_a);
	DECLARE_WRITE_LINE_MEMBER(d_pia0_irq_b);
	DECLARE_READ8_MEMBER(d_pia1_pa_r);
	DECLARE_WRITE8_MEMBER(d_pia1_pa_w);
	DECLARE_READ8_MEMBER(d_pia1_pb_r);
	DECLARE_WRITE8_MEMBER(d_pia1_pb_w);
	DECLARE_WRITE_LINE_MEMBER(d_pia1_irq_a);
	DECLARE_WRITE_LINE_MEMBER(d_pia1_irq_b);
	DECLARE_READ8_MEMBER(d_pia2_pa_r);
	DECLARE_WRITE8_MEMBER(d_pia2_pa_w);
	DECLARE_READ8_MEMBER(d_pia2_pb_r);
	DECLARE_WRITE8_MEMBER(d_pia2_pb_w);
	DECLARE_WRITE_LINE_MEMBER(d_pia2_irq_a);
	DECLARE_WRITE_LINE_MEMBER(d_pia2_irq_b);
	DECLARE_WRITE_LINE_MEMBER(dgnbeta_fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(dgnbeta_fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(dgnbeta_vsync_changed);
	/* 74HC670 4x4bit colour ram */
	DECLARE_WRITE8_MEMBER(dgnbeta_colour_ram_w);
	// Page IO at FE00
	DECLARE_READ8_MEMBER(dgn_beta_page_r);
	DECLARE_WRITE8_MEMBER(dgn_beta_page_w);
	MC6845_UPDATE_ROW(crtc_update_row);

	/*  WD2797 FDC */
	DECLARE_READ8_MEMBER(dgnbeta_wd2797_r);
	DECLARE_WRITE8_MEMBER(dgnbeta_wd2797_w);
	required_device<cpu_device> m_maincpu;
	void dgnbeta_vid_set_gctrl(int data);
	void UpdateBanks(int first, int last);
	void SetDefaultTask();
	void dgn_beta_bank_memory(int offset, int data, int bank);
	int SelectedKeyrow(dgn_beta_state *state, int Rows);
	int GetKeyRow(dgn_beta_state *state, int RowNo);
	void cpu0_recalc_irq(int state);
	void cpu0_recalc_firq(int state);
	void cpu1_recalc_firq(int state);
	void ScanInKeyboard(void);
	void dgn_beta_frame_interrupt (int data);
	void dgn_beta_line_interrupt (int data);
	required_device<ram_device> m_ram;
	required_device<wd2797_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_device<palette_device> m_palette;
};

#endif /* DGN_BETA_H_ */
