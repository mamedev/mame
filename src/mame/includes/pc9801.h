// license:BSD-3-Clause
// copyright-holders:Angelo Salese,Carl
#ifndef MAME_INCLUDES_PC9801_H
#define MAME_INCLUDES_PC9801_H

#pragma once

#include "cpu/i386/i386.h"
#include "cpu/i86/i286.h"
#include "cpu/i86/i86.h"
#include "cpu/nec/nec.h"
#include "cpu/nec/v5x.h"

#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "machine/bankdev.h"
#include "machine/buffer.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/output_latch.h"
#include "machine/pc9801_memsw.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/upd1990a.h"
#include "machine/upd4991a.h"
#include "machine/upd765.h"

#include "bus/scsi/pc9801_sasi.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"

#include "sound/beep.h"
//#include "sound/dac.h"
#include "sound/spkrdev.h"
#include "sound/ymopn.h"

#include "video/upd7220.h"

#include "bus/cbus/pc9801_26.h"
#include "bus/cbus/pc9801_86.h"
#include "bus/cbus/pc9801_118.h"
#include "bus/cbus/pc9801_amd98.h"
#include "bus/cbus/mpu_pc98.h"
#include "bus/cbus/pc9801_cbus.h"
#include "machine/pc9801_kbd.h"
#include "machine/pc9801_cd.h"

#include "bus/ata/atadev.h"
#include "bus/ata/ataintf.h"

#include "debugger.h"
#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "formats/pc98_dsk.h"
#include "formats/pc98fdi_dsk.h"
#include "formats/fdd_dsk.h"
#include "formats/dcp_dsk.h"
#include "formats/dip_dsk.h"
#include "formats/nfd_dsk.h"

#define RTC_TAG      "rtc"
#define UPD8251_TAG  "upd8251"
#define SASIBUS_TAG  "sasi"

#define ATTRSEL_REG 0
#define WIDTH40_REG 2
#define FONTSEL_REG 3
#define INTERLACE_REG 4
#define MEMSW_REG   6
#define DISPLAY_REG 7

#define ANALOG_16_MODE 0
#define ANALOG_256_MODE (0x20 >> 1)
#define GDC_IS_5MHz (0x84 >> 1)

class pc98_base_state : public driver_device
{
public:
	pc98_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
    	, m_keyb(*this, "keyb")
		, m_rtc(*this, RTC_TAG)
		, m_ppi_sys(*this, "ppi_sys")
		, m_ppi_prn(*this, "ppi_prn")
		, m_beeper(*this, "beeper")
	{
	}

	DECLARE_CUSTOM_INPUT_MEMBER(system_type_r);

protected:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<pc9801_kbd_device> m_keyb;
	optional_device<upd1990a_device> m_rtc;
	required_device<i8255_device> m_ppi_sys;
	required_device<i8255_device> m_ppi_prn;
	optional_device<beep_device> m_beeper;

	void rtc_w(uint8_t data);
	void ppi_sys_beep_portc_w(uint8_t data);
	
	static void floppy_formats(format_registration &fr);

	u8 m_sys_type;
};

class pc9801_state : public pc98_base_state
{
public:
	pc9801_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc98_base_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dmac(*this, "i8237")
		, m_pit(*this, "pit")
		, m_pic1(*this, "pic8259_master")
		, m_pic2(*this, "pic8259_slave")
		, m_fdc_2hd(*this, "upd765_2hd")
		, m_fdc_2dd(*this, "upd765_2dd")
		, m_memsw(*this, "memsw")
		, m_sio(*this, UPD8251_TAG)
		, m_hgdc1(*this, "upd7220_chr")
		, m_hgdc2(*this, "upd7220_btm")
		, m_sasibus(*this, SASIBUS_TAG)
		, m_sasi_data_out(*this, "sasi_data_out")
		, m_sasi_data_in(*this, "sasi_data_in")
		, m_sasi_ctrl_in(*this, "sasi_ctrl_in")
		, m_ide(*this, "ide%u", 1U)
		, m_video_ram_1(*this, "video_ram_1")
		, m_video_ram_2(*this, "video_ram_2")
		, m_ext_gvram(*this, "ext_gvram")
		, m_dac(*this, "dac")
		, m_ram(*this, RAM_TAG)
		, m_ipl(*this, "ipl_bank")
	{
	}

	void pc9801(machine_config &config);

	void pc9801vm(machine_config &config);

	void pc9801ux(machine_config &config);
	void pc9801rs(machine_config &config);

	void pc9801bx2(machine_config &config);

	void pc9821(machine_config &config);
	void pc9821as(machine_config &config);
	void pc9821ap2(machine_config &config);
	void pc9821cx3(machine_config &config);
	void pc9821xa16(machine_config &config);
	void pc9821ra20(machine_config &config);
	void pc9821ra266(machine_config &config);
	void pc9821ra333(machine_config &config);
	void pc9821v20(machine_config &config);
	
	void pc9821nr15(machine_config &config);
	void pc9821nr166(machine_config &config);

	void pc386m(machine_config &config);
	void pc486mu(machine_config &config);
	void pc486se(machine_config &config);

	void init_pc9801_kanji();
	void init_pc9801vm_kanji();

protected:
	virtual void video_start() override;

	void pc9801_keyboard(machine_config &config);
	void pc9801_mouse(machine_config &config);
	void pc9801_cbus(machine_config &config);
	void pc9801_sasi(machine_config &config);
	void pc9801_ide(machine_config &config);
	void pc9801_common(machine_config &config);

	void pit_clock_config(machine_config &config, const XTAL clock);

private:
	static void cdrom_headphones(device_t *device);

	required_device<cpu_device> m_maincpu;
	required_device<am9517a_device> m_dmac;
	required_device<pit8253_device> m_pit;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<upd765a_device> m_fdc_2hd;
	optional_device<upd765a_device> m_fdc_2dd;
	required_device<pc9801_memsw_device> m_memsw;
	required_device<i8251_device> m_sio;
	required_device<upd7220_device> m_hgdc1;
	required_device<upd7220_device> m_hgdc2;
	optional_device<scsi_port_device> m_sasibus;
	optional_device<output_latch_device> m_sasi_data_out;
	optional_device<input_buffer_device> m_sasi_data_in;
	optional_device<input_buffer_device> m_sasi_ctrl_in;
	optional_device_array<ata_interface_device, 2> m_ide;
	required_shared_ptr<uint16_t> m_video_ram_1;
	required_shared_ptr<uint16_t> m_video_ram_2;
	optional_shared_ptr<uint32_t> m_ext_gvram;
//	optional_device<dac_1bit_device> m_dac;
	optional_device<speaker_sound_device> m_dac;
	optional_device<ram_device> m_ram;
	optional_device<address_map_bank_device> m_ipl;

	void dmapg4_w(offs_t offset, uint8_t data);
	void dmapg8_w(offs_t offset, uint8_t data);
	void nmi_ctrl_w(offs_t offset, uint8_t data);
	void vrtc_clear_w(uint8_t data);
	void pc9801_video_ff_w(uint8_t data);
	uint8_t txt_scrl_r(offs_t offset);
	void txt_scrl_w(offs_t offset, uint8_t data);
	uint8_t grcg_r(offs_t offset);
	void grcg_w(offs_t offset, uint8_t data);
	void egc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t pc9801_a0_r(offs_t offset);
	void pc9801_a0_w(offs_t offset, uint8_t data);
	uint8_t fdc_2hd_ctrl_r();
	void fdc_2hd_ctrl_w(uint8_t data);
	uint8_t fdc_2dd_ctrl_r();
	void fdc_2dd_ctrl_w(uint8_t data);
	uint16_t tvram_r(offs_t offset, uint16_t mem_mask = ~0);
	void tvram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t gvram_r(offs_t offset);
	void gvram_w(offs_t offset, uint8_t data);
	void pc9801rs_mouse_freq_w(offs_t offset, uint8_t data);
	uint16_t grcg_gvram_r(offs_t offset, uint16_t mem_mask = ~0);
	void grcg_gvram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t grcg_gvram0_r(offs_t offset, uint16_t mem_mask = ~0);
	void grcg_gvram0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t pc9821_grcg_gvram_r(offs_t offset, uint16_t mem_mask = ~0);
	void pc9821_grcg_gvram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pc9821_grcg_gvram0_r(offs_t offset, uint16_t mem_mask = ~0);
	void pc9821_grcg_gvram0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t upd7220_grcg_r(offs_t offset, uint16_t mem_mask = ~0);
	void upd7220_grcg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint8_t ide_ctrl_r();
	void ide_ctrl_w(uint8_t data);
	uint16_t ide_cs0_r(offs_t offset, uint16_t mem_mask = ~0);
	void ide_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ide_cs1_r(offs_t offset, uint16_t mem_mask = ~0);
	void ide_cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void sasi_data_w(uint8_t data);
	uint8_t sasi_data_r();
	DECLARE_WRITE_LINE_MEMBER(write_sasi_io);
	DECLARE_WRITE_LINE_MEMBER(write_sasi_req);
	uint8_t sasi_status_r();
	void sasi_ctrl_w(uint8_t data);

	uint8_t pc9801rs_knjram_r(offs_t offset);
	void pc9801rs_knjram_w(offs_t offset, uint8_t data);
	void pc9801rs_bank_w(offs_t offset, uint8_t data);
	uint8_t f0_r(offs_t offset);

	uint8_t a20_ctrl_r(offs_t offset);
	void a20_ctrl_w(offs_t offset, uint8_t data);
	uint8_t fdc_mode_ctrl_r();
	void fdc_mode_ctrl_w(uint8_t data);
//  uint8_t pc9801rs_2dd_r();
//  void pc9801rs_2dd_w(uint8_t data);
	void pc9801rs_video_ff_w(offs_t offset, uint8_t data);
	void pc9801rs_a0_w(offs_t offset, uint8_t data);
	void pc9821_video_ff_w(offs_t offset, uint8_t data);
	uint8_t pc9821_a0_r(offs_t offset);
	void pc9821_a0_w(offs_t offset, uint8_t data);
	uint8_t access_ctrl_r(offs_t offset);
	void access_ctrl_w(offs_t offset, uint8_t data);
	uint8_t midi_r();
//  uint8_t winram_r();
//  void winram_w(uint8_t data);
	uint8_t pic_r(offs_t offset);
	void pic_w(offs_t offset, uint8_t data);

	uint8_t as_unkdev_data_r(offs_t offset);
	void as_unkdev_data_w(offs_t offset, uint8_t data);
	void as_unkdev_addr_w(offs_t offset, uint8_t data);

	uint8_t window_bank_r(offs_t offset);
	void window_bank_w(offs_t offset, uint8_t data);
	uint16_t timestamp_r(offs_t offset);
	uint8_t ext2_video_ff_r();
	void ext2_video_ff_w(uint8_t data);

	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	UPD7220_DRAW_TEXT_LINE_MEMBER( hgdc_draw_text );

	DECLARE_MACHINE_START(pc9801_common);
	DECLARE_MACHINE_START(pc9801f);
	DECLARE_MACHINE_START(pc9801rs);
	DECLARE_MACHINE_START(pc9801bx2);
	DECLARE_MACHINE_START(pc9821);
	DECLARE_MACHINE_START(pc9821ap2);
	DECLARE_MACHINE_RESET(pc9801_common);
	DECLARE_MACHINE_RESET(pc9801f);
	DECLARE_MACHINE_RESET(pc9801rs);
	DECLARE_MACHINE_RESET(pc9821);

	void pc9801_palette(palette_device &palette) const;
	DECLARE_WRITE_LINE_MEMBER(vrtc_irq);
	uint8_t get_slave_ack(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(dma_hrq_changed);
	DECLARE_WRITE_LINE_MEMBER(tc_w);
	uint8_t dma_read_byte(offs_t offset);
	void dma_write_byte(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(dack0_w);
	DECLARE_WRITE_LINE_MEMBER(dack1_w);
	DECLARE_WRITE_LINE_MEMBER(dack2_w);
	DECLARE_WRITE_LINE_MEMBER(dack3_w);
	void ppi_sys_dac_portc_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(fdc_2dd_irq);
	DECLARE_WRITE_LINE_MEMBER(pc9801rs_fdc_irq);
	DECLARE_WRITE_LINE_MEMBER(pc9801rs_fdc_drq);

	uint8_t ppi_mouse_porta_r();
	void ppi_mouse_porta_w(uint8_t data);
	void ppi_mouse_portb_w(uint8_t data);
	void ppi_mouse_portc_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER( mouse_irq_cb );
	uint8_t unk_r();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t a20_286(bool state);

	void ipl_bank(address_map &map);
	void pc9801_common_io(address_map &map);
	void pc9801_io(address_map &map);
	void pc9801_map(address_map &map);
	void pc9801rs_io(address_map &map);
	void pc9801rs_map(address_map &map);
	void pc9801ux_io(address_map &map);
	void pc9801ux_map(address_map &map);
	void pc9821_io(address_map &map);
	void pc9821_map(address_map &map);
	void pc9821as_io(address_map &map);
	void upd7220_1_map(address_map &map);
	void upd7220_2_map(address_map &map);
	void upd7220_grcg_2_map(address_map &map);

	enum
	{
		TIMER_VBIRQ
	};

	inline void set_dma_channel(int channel, int state);
	uint8_t *m_char_rom;
	uint8_t *m_kanji_rom;

	uint8_t m_dma_offset[4];
	uint8_t m_dma_autoinc[4];
	int m_dack;

	uint8_t m_video_ff[8],m_gfx_ff;
	uint8_t m_txt_scroll_reg[8];
	uint8_t m_pal_clut[4];

	std::unique_ptr<uint16_t[]> m_tvram;

	uint16_t m_font_addr;
	uint8_t m_font_line;
	uint16_t m_font_lr;

	uint8_t m_fdc_2dd_ctrl,m_fdc_2hd_ctrl;
	uint8_t m_nmi_ff;

	uint8_t m_vram_bank;
	uint8_t m_vram_disp;

	uint8_t m_sasi_data;
	int m_sasi_data_enable;
	uint8_t m_sasi_ctrl;

	struct{
		uint8_t control;
		uint8_t lx, ly;
		uint8_t dx, dy;
		uint8_t prev_dx, prev_dy;
		uint8_t freq_reg;
		uint8_t freq_index;
	}m_mouse;

	uint8_t m_ide_sel;

	// starting from PC9801VF/U buzzer is substituted with a DAC1BIT
	bool m_dac_disable;

	/* PC9801RS specific, move to specific state */
	uint8_t m_gate_a20; //A20 line
	uint8_t m_access_ctrl; // DMA related
	uint8_t m_fdc_ctrl;
	uint8_t m_ex_video_ff[128];
	struct {
		uint8_t pal_entry;
		uint8_t r[16],g[16],b[16];
	}m_analog16;
	struct {
		uint8_t pal_entry;
		uint8_t r[0x100],g[0x100],b[0x100];
		uint16_t bank[2];
	}m_analog256;
	struct {
		uint8_t mode;
		uint8_t tile[4], tile_index;
	}m_grcg;

	void egc_blit_w(uint32_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t egc_blit_r(uint32_t offset, uint16_t mem_mask);


	/* PC9821 specific */
	uint8_t m_sdip[24], m_sdip_bank;
	uint8_t m_unkdev0468[0x100], m_unkdev0468_addr;
	uint8_t m_pc9821_window_bank;
	uint8_t m_ext2_ff;

	struct {
		uint16_t regs[8];
		uint16_t pat[4];
		uint16_t src[4];
		int16_t count;
		uint16_t leftover[4];
		bool first;
		bool init;
	} m_egc;

	uint16_t m_pc9821_256vram_bank;

	template<unsigned port> u8 sdip_r(offs_t offset);
	template<unsigned port> void sdip_w(offs_t offset, u8 data);

	uint16_t egc_do_partial_op(int plane, uint16_t src, uint16_t pat, uint16_t dst) const;
	uint16_t egc_shift(int plane, uint16_t val);
	uint16_t egc_color_pat(int plane) const;
};





/******************************************
 *
 * pc98ha.cpp: "Handy98" 1st gen portables
 *
 ******************************************/

class pc98lt_state : public pc98_base_state
{
public:
	pc98lt_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc98_base_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "upd765")
		, m_gvram(*this, "gvram")
		, m_bram_bank(*this, "bram_bank")
		, m_dict_bank(*this, "dict_bank")
		, m_kanji_bank(*this, "kanji_bank")
		, m_romdrv_bank(*this, "romdrv_bank")
	{
	}
	
	void lt_config(machine_config &config);

protected:
	void lt_map(address_map &map);
	void lt_io(address_map &map);

	required_device<v50_device> m_maincpu;

	virtual void machine_start() override;
//	virtual void machine_reset() override;
private:
	required_device<upd765a_device> m_fdc;
	required_shared_ptr<uint16_t> m_gvram;
	std::unique_ptr<uint16_t[]> m_bram_ptr;
	required_memory_bank m_bram_bank;
	required_memory_bank m_dict_bank;
	required_memory_bank m_kanji_bank;
	required_memory_bank m_romdrv_bank;

	void lt_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 power_status_r();
	void power_control_w(offs_t offset, u8 data);
	u8 floppy_mode_r(offs_t offset);
	void floppy_mode_w(offs_t offset, u8 data);
	u8 fdc_ctrl_r(offs_t offset);
	void fdc_ctrl_w(offs_t offset, u8 data);

	u8 m_romdrv_bank_reg;
	u8 m_bram_banks;
	u8 m_bram_bank_reg;
	u8 m_dict_bank_reg;
	
	u8 m_floppy_mode;
	u8 m_fdc_ctrl;
};

class pc98ha_state : public pc98lt_state
{
public:
	pc98ha_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc98lt_state(mconfig, type, tag)
		, m_ems_banks(*this, "ems_bank%u", 1U)
		, m_ext_view(*this, "ext_io")
		, m_ramdrv_bank(*this, "ramdrv_bank")
		, m_rtc_pio(*this, "prtc")
	{
	}

	void ha_config(machine_config &config);
	
protected:
	void ha_map(address_map &map);
	void ha_io(address_map &map);

	virtual void machine_start() override;
private:
	required_memory_bank_array<4> m_ems_banks;
	memory_view m_ext_view;
	required_memory_bank m_ramdrv_bank;
	required_device<upd4991a_device> m_rtc_pio;

	std::unique_ptr<uint16_t[]> m_ems_ram;

	void ext_view_bank_w(offs_t offset, u8 data);
	void ext_view_sel_w(offs_t offset, u8 data);
	void ems_bank_w(offs_t offset, u8 data);
	u8 memcard_status_1_r(offs_t offset);
	u8 memcard_status_2_r(offs_t offset);
	u8 m_ext_view_sel;
};

#endif // MAME_INCLUDES_PC9801_H
