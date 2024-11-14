// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*****************************************************************************
 *
 * includes/dgn_beta.h
 *
 ****************************************************************************/

#ifndef MAME_TRS_DGN_BETA_H
#define MAME_TRS_DGN_BETA_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/6821pia.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"

#include "emupal.h"

/* Tags */

#define DMACPU_TAG  "dmacpu"
#define PIA_0_TAG   "pia_0"
#define PIA_1_TAG   "pia_1"
#define PIA_2_TAG   "pia_2"
#define FDC_TAG     "wd2797"


#define DGNBETA_CPU_SPEED_HZ        2000000 /* 2MHz */
#define DGNBETA_FRAMES_PER_SECOND   50


class dgn_beta_state : public driver_device
{
public:
	dgn_beta_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mc6845(*this, "crtc"),
		m_maincpu(*this, "maincpu"),
		m_dmacpu(*this, DMACPU_TAG),
		m_ram(*this, RAM_TAG),
		m_pia_0(*this, PIA_0_TAG),
		m_pia_1(*this, PIA_1_TAG),
		m_pia_2(*this, PIA_2_TAG),
		m_fdc(*this, FDC_TAG),
		m_floppy0(*this, FDC_TAG ":0"),
		m_floppy1(*this, FDC_TAG ":1"),
		m_floppy2(*this, FDC_TAG ":2"),
		m_floppy3(*this, FDC_TAG ":3"),
		m_palette(*this, "palette"),
		m_system_rom(*this, "maincpu")
	{ }

	void dgnbeta(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static constexpr unsigned RamSize           = 256;          // 256K by default
	static constexpr unsigned RamPageSize       = 4096;         // ram pages are 4096 bytes

	static constexpr unsigned MaxTasks          = 16;           // Tasks 0..15
	static constexpr unsigned MaxPage           = 16;           // 16 4K pages
	static constexpr unsigned NoPagingTask      = MaxTasks;     // Task registers to use when paging disabled 16

	static constexpr unsigned RAMPage           = 0;            // Page with RAM in at power on
	static constexpr unsigned VideoPage         = 6;            // Page where video ram mapped
	static constexpr unsigned IOPage            = MaxPage-1;    // Page for I/O
	static constexpr unsigned ROMPage           = MaxPage-2;    // Page for ROM
	static constexpr unsigned LastPage          = MaxPage-1;

	static constexpr uint8_t RAMPageValue       = 0x00;         // page with RAM at power on
	static constexpr uint8_t VideoPageValue     = 0x1f;         // Default page for video ram
	static constexpr uint8_t NoMemPageValue     = 0xc0;         // Page guaranteed not to have memory in
	static constexpr uint8_t ROMPageValue       = 0xfe;         // Page with boot ROM
	static constexpr uint8_t IOPageValue        = 0xff;         // Page with I/O & Boot ROM

	static constexpr uint8_t TextVidBasePage    = 0x18;         // Base page of text video RAM

	/***** Keyboard stuff *****/
	static constexpr uint8_t NoKeyrows          = 0x0a;         // Number of rows in keyboard

	/* From Dragon Beta OS9 keyboard driver */
	static constexpr uint8_t KAny               = 0x04;         // Any key pressed mask PB2
	static constexpr uint8_t KOutClk            = 0x08;         // Output shift register clock
	static constexpr uint8_t KInClk             = 0x10;         // Input shift register clock
	static constexpr uint8_t KOutDat            = KInClk;       // Also used for data into output shifter
	static constexpr uint8_t KInDat             = 0x20;         // Keyboard data in from keyboard (serial stream)

	/***** Video Modes *****/

	enum BETA_VID_MODES
	{
		TEXT_40x25,             /* Text mode 40x25 */
		TEXT_80x25,             /* Text mode 80x25 */
		GRAPH_320x256x4,        /* Graphics 320x256x4 */
		GRAPH_320x256x16,       /* Graphics 320x256x16 */
		GRAPH_640x512x2         /* Graphics 640X512X2 */
	};

	static constexpr unsigned iosize = 0xfeff-0xfc00;

	struct PageReg
	{
		int     value = 0;          /* Value of the page register */
		uint8_t *memory = nullptr;        /* The memory it actually points to */
	};

	static void floppy_formats(format_registration &fr);

	required_device<mc6845_device> m_mc6845;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_dmacpu;
	required_device<ram_device> m_ram;
	required_device<pia6821_device> m_pia_0;
	required_device<pia6821_device> m_pia_1;
	required_device<pia6821_device> m_pia_2;
	required_device<wd2797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_device<palette_device> m_palette;

	required_region_ptr<uint8_t> m_system_rom;
	int m_LogDatWrites = 0;
	int m_Keyboard[NoKeyrows]{};
	int m_RowShifter = 0;
	int m_Keyrow = 0;
	int m_d_pia0_pb_last = 0;
	int m_d_pia0_cb2_last = 0;
	int m_KInDat_next = 0;
	int m_KAny_next = 0;
	int m_d_pia1_pa_last = 0;
	int m_d_pia1_pb_last = 0;
	int m_DMA_NMI_LAST = 0;
	int m_wd2797_written = 0;
	int m_TaskReg = 0;
	int m_PIATaskReg = 0;
	int m_EnableMapRegs = 0;
	PageReg m_PageRegs[MaxTasks+1][MaxPage+1]{};
	int m_beta_6845_RA = 0; // TODO: most of the variables from here on aren't used anywhere. Left-over or reminder of things to be implemented?
	int m_beta_scr_x = 0;
	int m_beta_scr_y = 0;
	int m_beta_HSync = 0;
	int m_beta_VSync = 0;
	int m_beta_DE = 0;
	int m_LogRegWrites = 0;
	int m_BoxColour = 0;
	int m_BoxMinX = 0;
	int m_BoxMinY = 0;
	int m_BoxMaxX = 0;
	int m_BoxMaxY = 0;
	int m_HSyncMin = 0;
	int m_VSyncMin = 0;
	int m_DEPos = 0;
	int m_NoScreen = 0;
	bitmap_ind16 *m_bit = nullptr;
	int m_MinAddr = 0;
	int m_MaxAddr = 0;
	int m_MinX = 0;
	int m_MaxX = 0;
	int m_MinY = 0;
	int m_MaxY = 0;
	int m_VidAddr = 0;
	int m_ClkMax = 0;
	int m_GCtrl = 0;
	int m_FlashCount = 0;
	int m_FlashBit = 0;
	int m_s_DoubleY = 0;
	int m_DoubleHL = 0;
	int m_ColourRAM[4]{};
	int m_Field = 0;
	int m_DrawInterlace = 0;
	void dgn_beta_palette(palette_device &palette) const;

	uint8_t d_pia0_pa_r();
	void d_pia0_pa_w(uint8_t data);
	uint8_t d_pia0_pb_r();
	void d_pia0_pb_w(uint8_t data);
	void d_pia0_cb2_w(int state);
	void d_pia0_irq_a(int state);
	void d_pia0_irq_b(int state);
	uint8_t d_pia1_pa_r();
	void d_pia1_pa_w(uint8_t data);
	uint8_t d_pia1_pb_r();
	void d_pia1_pb_w(uint8_t data);
	void d_pia1_irq_a(int state);
	void d_pia1_irq_b(int state);
	uint8_t d_pia2_pa_r();
	void d_pia2_pa_w(uint8_t data);
	uint8_t d_pia2_pb_r();
	void d_pia2_pb_w(uint8_t data);
	void d_pia2_irq_a(int state);
	void d_pia2_irq_b(int state);
	void dgnbeta_fdc_intrq_w(int state);
	void dgnbeta_fdc_drq_w(int state);
	void dgnbeta_vsync_changed(int state);
	/* 74HC670 4x4bit colour ram */
	void dgnbeta_colour_ram_w(offs_t offset, uint8_t data);
	// Page IO at FE00
	uint8_t dgn_beta_page_r(offs_t offset);
	void dgn_beta_page_w(offs_t offset, uint8_t data);
	MC6845_UPDATE_ROW(crtc_update_row);

	/*  WD2797 FDC */
	uint8_t dgnbeta_wd2797_r(offs_t offset);
	void dgnbeta_wd2797_w(offs_t offset, uint8_t data);

	void dgnbeta_vid_set_gctrl(int data);
	void UpdateBanks(int first, int last);
	void SetDefaultTask();
	void dgn_beta_bank_memory(int offset, int data, int bank);
	int SelectedKeyrow(int Rows);
	int GetKeyRow(int RowNo);
	void cpu0_recalc_irq(int state);
	void cpu0_recalc_firq(int state);
	void cpu1_recalc_firq(int state);
	void ScanInKeyboard(void);
	void dgn_beta_frame_interrupt(int data);
	[[maybe_unused]] void dgn_beta_line_interrupt(int data);

	offs_t dgnbeta_dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);

	void dgnbeta_map(address_map &map) ATTR_COLD;

	void execute_beta_key_dump(const std::vector<std::string_view> &params);
	void execute_beta_dat_log(const std::vector<std::string_view> &params);
};

#endif // MAME_TRS_DGN_BETA_H
