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
		, m_ipl(*this, "ipl_bank")
		, m_dmac(*this, "i8237")
		, m_pit(*this, "pit")
		, m_fdc_2hd(*this, "upd765_2hd")
		, m_fdc_2dd(*this, "upd765_2dd")
		, m_ram(*this, RAM_TAG)
		, m_hgdc(*this, "hgdc%d", 1)
		, m_video_ram(*this, "video_ram_%d", 1)
		, m_cbus(*this, "cbus%d", 0)
		, m_pic1(*this, "pic8259_master")
		, m_pic2(*this, "pic8259_slave")
		, m_memsw(*this, "memsw")
		, m_sio(*this, UPD8251_TAG)
		, m_sasibus(*this, SASIBUS_TAG)
		, m_sasi_data_out(*this, "sasi_data_out")
		, m_sasi_data_in(*this, "sasi_data_in")
		, m_sasi_ctrl_in(*this, "sasi_ctrl_in")
	{
	}

	void pc9801(machine_config &config);

	void init_pc9801_kanji();

//  Device declarations
protected:
	required_device<cpu_device> m_maincpu;
	optional_device<address_map_bank_device> m_ipl;
	required_device<am9517a_device> m_dmac;
	required_device<pit8253_device> m_pit;
	// TODO: should really be one FDC
	// (I/O $90-$93 is a "simplified" version)
	required_device<upd765a_device> m_fdc_2hd;
	optional_device<upd765a_device> m_fdc_2dd;
	optional_device<ram_device> m_ram;
	required_device_array<upd7220_device, 2> m_hgdc;
	required_shared_ptr_array<uint16_t, 2> m_video_ram;
	required_device_array<pc9801_slot_device, 2> m_cbus;
private:
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<pc9801_memsw_device> m_memsw;
	required_device<i8251_device> m_sio;
	optional_device<scsi_port_device> m_sasibus;
	optional_device<output_latch_device> m_sasi_data_out;
	optional_device<input_buffer_device> m_sasi_data_in;
	optional_device<input_buffer_device> m_sasi_ctrl_in;

//  Infrastructure declaration
protected:
	DECLARE_MACHINE_START(pc9801_common);
	DECLARE_MACHINE_RESET(pc9801_common);

	void pc9801_keyboard(machine_config &config);
	void pc9801_mouse(machine_config &config);
	void pc9801_cbus(machine_config &config);
	void pc9801_sasi(machine_config &config);
	void pc9801_common(machine_config &config);
	void config_floppy_525hd(machine_config &config);
	void config_floppy_35hd(machine_config &config);

	void pit_clock_config(machine_config &config, const XTAL clock);

	void pc9801_common_io(address_map &map);
	void ipl_bank(address_map &map);

	uint8_t pc9801_a0_r(offs_t offset);
	void pc9801_a0_w(offs_t offset, uint8_t data);
	u8 unk_r(offs_t offset);
	uint8_t f0_r(offs_t offset);

	uint8_t m_nmi_ff;

private:
	void pc9801_io(address_map &map);
	void pc9801_map(address_map &map);

	void nmi_ctrl_w(offs_t offset, uint8_t data);

	void sasi_data_w(uint8_t data);
	uint8_t sasi_data_r();
	DECLARE_WRITE_LINE_MEMBER(write_sasi_io);
	DECLARE_WRITE_LINE_MEMBER(write_sasi_req);
	uint8_t sasi_status_r();
	void sasi_ctrl_w(uint8_t data);

//  uint8_t winram_r();
//  void winram_w(uint8_t data);

	DECLARE_MACHINE_START(pc9801f);
	DECLARE_MACHINE_RESET(pc9801f);

//  PIC
protected:
	uint8_t pic_r(offs_t offset);
	void pic_w(offs_t offset, uint8_t data);
private:
	uint8_t get_slave_ack(offs_t offset);

//  FDC
protected:
	uint8_t fdc_2hd_ctrl_r();
	void fdc_2hd_ctrl_w(uint8_t data);

	u8 m_fdc_ctrl;
	uint8_t m_fdc_2dd_ctrl, m_fdc_2hd_ctrl;
private:
	DECLARE_WRITE_LINE_MEMBER(fdc_2dd_irq);

	uint8_t fdc_2dd_ctrl_r();
	void fdc_2dd_ctrl_w(uint8_t data);

//	DMA
protected:
	uint8_t m_dma_offset[4];
	uint8_t m_dma_autoinc[4];
	int m_dack;
	
private:
	void dmapg4_w(offs_t offset, uint8_t data);

	inline void set_dma_channel(int channel, int state);

	DECLARE_WRITE_LINE_MEMBER(dma_hrq_changed);
	DECLARE_WRITE_LINE_MEMBER(tc_w);
	uint8_t dma_read_byte(offs_t offset);
	void dma_write_byte(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(dack0_w);
	DECLARE_WRITE_LINE_MEMBER(dack1_w);
	DECLARE_WRITE_LINE_MEMBER(dack2_w);
	DECLARE_WRITE_LINE_MEMBER(dack3_w);

//	Video
protected:
	void upd7220_1_map(address_map &map);
	void upd7220_2_map(address_map &map);

	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );

	virtual void video_start() override;
	void pc9801_palette(palette_device &palette) const;

	uint8_t *m_char_rom;
	uint8_t *m_kanji_rom;

	struct {
		uint8_t mode;
		uint8_t tile[4], tile_index;
	}m_grcg;

	enum
	{
		TIMER_VBIRQ
	};

	uint16_t tvram_r(offs_t offset, uint16_t mem_mask = ~0);
	void tvram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t gvram_r(offs_t offset);
	void gvram_w(offs_t offset, uint8_t data);
	uint8_t grcg_r(offs_t offset);
	void grcg_w(offs_t offset, uint8_t data);
	
	void pc9801_video_ff_w(uint8_t data);

	uint16_t m_font_addr;
	uint16_t m_font_lr;
	uint8_t m_video_ff[8];
	// TODO: move to derived state
	uint8_t m_ex_video_ff[128];
	u8 m_vram_bank;
	u8 m_vram_disp;

private:
	UPD7220_DRAW_TEXT_LINE_MEMBER( hgdc_draw_text );

	DECLARE_WRITE_LINE_MEMBER(vrtc_irq);
	void vrtc_clear_w(uint8_t data);
	uint8_t txt_scrl_r(offs_t offset);
	void txt_scrl_w(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t m_font_line;
	std::unique_ptr<uint16_t[]> m_tvram;
	uint8_t m_gfx_ff;
	uint8_t m_txt_scroll_reg[8];
	uint8_t m_pal_clut[4];

//  SASI
	uint8_t m_sasi_data;
	int m_sasi_data_enable;
	uint8_t m_sasi_ctrl;

//	Mouse
protected:
	struct{
		uint8_t control;
		uint8_t lx, ly;
		uint8_t dx, dy;
		uint8_t prev_dx, prev_dy;
		uint8_t freq_reg;
		uint8_t freq_index;
	}m_mouse;

private:
	uint8_t ppi_mouse_porta_r();
	void ppi_mouse_porta_w(uint8_t data);
	void ppi_mouse_portb_w(uint8_t data);
	void ppi_mouse_portc_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER( mouse_irq_cb );
};

/**********************************************************
 *
 * VM class
 *
 **********************************************************/

class pc9801vm_state : public pc9801_state
{
public:
	pc9801vm_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9801_state(mconfig, type, tag)
		, m_ide(*this, "ide%u", 1U)
		, m_dac1bit(*this, "dac1bit")
	{
	}

	void pc9801vm(machine_config &config);

	void pc9801ux(machine_config &config);
	void pc9801vx(machine_config &config);
	void pc9801rs(machine_config &config);

	void init_pc9801vm_kanji();

protected:
	void pc9801rs_io(address_map &map);
	void pc9801rs_map(address_map &map);

	uint16_t grcg_gvram_r(offs_t offset, uint16_t mem_mask = ~0);
	void grcg_gvram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t grcg_gvram0_r(offs_t offset, uint16_t mem_mask = ~0);
	void grcg_gvram0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t upd7220_grcg_r(offs_t offset, uint16_t mem_mask = ~0);
	void upd7220_grcg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void upd7220_grcg_2_map(address_map &map);

	void pc9801_ide(machine_config &config);
	static void cdrom_headphones(device_t *device);

	void pc9801rs_video_ff_w(offs_t offset, uint8_t data);
	void pc9801rs_a0_w(offs_t offset, uint8_t data);	

	uint8_t ide_ctrl_r();
	void ide_ctrl_w(uint8_t data);
	uint16_t ide_cs0_r(offs_t offset, uint16_t mem_mask = ~0);
	void ide_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ide_cs1_r(offs_t offset, uint16_t mem_mask = ~0);
	void ide_cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void dmapg8_w(offs_t offset, uint8_t data);

	uint16_t timestamp_r(offs_t offset);

	void ppi_sys_dac_portc_w(uint8_t data);

	DECLARE_MACHINE_START(pc9801rs);
	DECLARE_MACHINE_RESET(pc9801rs);

	u8 m_gate_a20;
	u8 m_dma_access_ctrl;
	u8 m_ide_sel;

	// starting from PC9801VF/U buzzer is substituted with a DAC1BIT
	bool m_dac1bit_disable;
private:
	optional_device_array<ata_interface_device, 2> m_ide;
//	optional_device<dac_1bit_device> m_dac1bit;
	required_device<speaker_sound_device> m_dac1bit;

	void pc9801ux_io(address_map &map);
	void pc9801ux_map(address_map &map);

	uint32_t a20_286(bool state);

	uint8_t pc9801rs_knjram_r(offs_t offset);
	void pc9801rs_knjram_w(offs_t offset, uint8_t data);
	void pc9801rs_bank_w(offs_t offset, uint8_t data);
	uint8_t midi_r();


	// 286-based machines except for PC98XA
	u8 dma_access_ctrl_r(offs_t offset);
	void dma_access_ctrl_w(offs_t offset, u8 data);

	uint8_t a20_ctrl_r(offs_t offset);
	void a20_ctrl_w(offs_t offset, uint8_t data);

	uint8_t fdc_mode_ctrl_r();
	void fdc_mode_ctrl_w(uint8_t data);

//	DECLARE_WRITE_LINE_MEMBER(pc9801rs_fdc_irq);
//	DECLARE_WRITE_LINE_MEMBER(pc9801rs_fdc_drq);

protected:
	struct {
		uint8_t pal_entry;
		uint8_t r[16],g[16],b[16];
	}m_analog16;

private:
	// EGC, PC9801VX onward
	struct {
		uint16_t regs[8];
		uint16_t pat[4];
		uint16_t src[4];
		int16_t count;
		uint16_t leftover[4];
		bool first;
		bool init;
	} m_egc;

protected:
	void egc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
private:
	void egc_blit_w(uint32_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t egc_blit_r(uint32_t offset, uint16_t mem_mask);

	uint16_t egc_do_partial_op(int plane, uint16_t src, uint16_t pat, uint16_t dst) const;
	uint16_t egc_shift(int plane, uint16_t val);
	uint16_t egc_color_pat(int plane) const;

protected:
	u8 mouse_freq_r(offs_t offset);
	void mouse_freq_w(offs_t offset, u8 data);
};

class pc9801us_state : public pc9801vm_state
{
public:
	pc9801us_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9801vm_state(mconfig, type, tag)
	{
	}
	void pc9801us(machine_config &config);

protected:
	void pc9801us_io(address_map &map);

	DECLARE_MACHINE_START(pc9801us);


	// SDIP, PC9801DA onward
protected:
	u8 m_sdip[24];
private:
	u8 m_sdip_bank;
	template<unsigned port> u8 sdip_r(offs_t offset);
	template<unsigned port> void sdip_w(offs_t offset, u8 data);
	void sdip_bank_w(offs_t offset, u8 data);
};

/**********************************************************
 *
 * BX class ("98Fellow")
 *
 **********************************************************/

class pc9801bx_state : public pc9801us_state
{
public:
	pc9801bx_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9801us_state(mconfig, type, tag)
	{
	}

	void pc9801bx2(machine_config &config);

protected:
	void pc9801bx2_io(address_map &map);
	void pc9801bx2_map(address_map &map);

	DECLARE_MACHINE_START(pc9801bx2);
	DECLARE_MACHINE_RESET(pc9801bx2);

private:
	u8 i486_cpu_mode_r(offs_t offset);
	u8 gdc_31kHz_r(offs_t offset);
	void gdc_31kHz_w(offs_t offset, u8 data);
};

/******************************************
 *
 * pc9821.cpp: PC-9821 base class
 *
 *****************************************/

class pc9821_state : public pc9801bx_state
{
public:
	pc9821_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9801bx_state(mconfig, type, tag)
		, m_ext_gvram(*this, "ext_gvram")
	{
	}

	void pc9821(machine_config &config);

protected:
	void pc9821_io(address_map &map);
	void pc9821_map(address_map &map);

	DECLARE_MACHINE_START(pc9821);
	DECLARE_MACHINE_RESET(pc9821);
private:
	required_shared_ptr<uint32_t> m_ext_gvram;

	uint16_t pc9821_grcg_gvram_r(offs_t offset, uint16_t mem_mask = ~0);
	void pc9821_grcg_gvram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pc9821_grcg_gvram0_r(offs_t offset, uint16_t mem_mask = ~0);
	void pc9821_grcg_gvram0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pc9821_video_ff_w(offs_t offset, uint8_t data);
	uint8_t pc9821_a0_r(offs_t offset);
	void pc9821_a0_w(offs_t offset, uint8_t data);
	uint8_t window_bank_r(offs_t offset);
	void window_bank_w(offs_t offset, uint8_t data);
	uint8_t ext2_video_ff_r();
	void ext2_video_ff_w(uint8_t data);

	uint8_t m_pc9821_window_bank;
	uint8_t m_ext2_ff;

	uint16_t m_pc9821_256vram_bank;
	
	struct {
		uint8_t pal_entry;
		uint8_t r[0x100],g[0x100],b[0x100];
		uint16_t bank[2];
	}m_analog256;
	
	void pc9821_egc_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	UPD7220_DISPLAY_PIXELS_MEMBER( pc9821_hgdc_display_pixels );
};

// MATE A

class pc9821_mate_a_state : public pc9821_state
{
public:
	pc9821_mate_a_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_state(mconfig, type, tag)
	{
	}

	void pc9821as(machine_config &config);
	void pc9821ap2(machine_config &config);

protected:
	void pc9821as_io(address_map &map);

private:
	DECLARE_MACHINE_START(pc9821ap2);

	// Ap, As, Ae only
	u8 ext_sdip_data_r(offs_t offset);
	void ext_sdip_data_w(offs_t offset, u8 data);
	void ext_sdip_address_w(offs_t offset, u8 data);
	void ext_sdip_access_w(offs_t offset, u8 data);

	uint8_t m_ext_sdip[0x100], m_ext_sdip_addr;
};

// CanBe

class pc9821_canbe_state : public pc9821_state
{
public:
	pc9821_canbe_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_state(mconfig, type, tag)
	{
	}

	void pc9821ce2(machine_config &config);
	void pc9821cx3(machine_config &config);

protected:
	void pc9821cx3_map(address_map &map);
	void pc9821cx3_io(address_map &map);

private:
	void remote_addr_w(offs_t offset, u8 data);
	u8 remote_data_r(offs_t offset);
	void remote_data_w(offs_t offset, u8 data);

	DECLARE_MACHINE_START(pc9821_canbe);

	struct {
		u8 index;
	}m_remote;
};

// class pc9821_cereb_state : public pc9821_canbe_state

// Mate B

// class pc9821_mate_b_state : public pc9821_state

// Mate X (NB: should be subclass of Mate B)

class pc9821_mate_x_state : public pc9821_state
{
public:
	pc9821_mate_x_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_state(mconfig, type, tag)
	{
	}

	void pc9821xa16(machine_config &config);
	void pc9821xs(machine_config &config);
};

// Mate R

class pc9821_mate_r_state : public pc9821_mate_x_state
{
public:
	pc9821_mate_r_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_mate_x_state(mconfig, type, tag)
	{
	}

	void pc9821ra20(machine_config &config);
	void pc9821ra266(machine_config &config);
	void pc9821ra333(machine_config &config);
};

class pc9821_valuestar_state : public pc9821_mate_x_state
{
public:
	pc9821_valuestar_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_mate_x_state(mconfig, type, tag)
	{
	}

	void pc9821v13(machine_config &config);
	void pc9821v20(machine_config &config);
};

// 9821NOTE

class pc9821_note_state : public pc9821_state
{
public:
	pc9821_note_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_state(mconfig, type, tag)
	{
	}

	void pc9821ne(machine_config &config);
};

class pc9821_note_lavie_state : public pc9821_note_state
{
public:
	pc9821_note_lavie_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9821_note_state(mconfig, type, tag)
	{
	}

	void pc9821nr15(machine_config &config);
	void pc9821nr166(machine_config &config);
	void pc9821nw150(machine_config &config);
};

/******************************************
 *
 * pc9801_epson.cpp: Epson clones
 *
 ******************************************/

class pc98_epson_state : public pc9801vm_state
{
public:
	pc98_epson_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9801vm_state(mconfig, type, tag)
		, m_shadow_ipl(*this, "shadow_ipl_%u", 0)
	{
	}

	void pc386m(machine_config &config);
	void pc486mu(machine_config &config);
	void pc486se(machine_config &config);

protected:
	void pc386m_ipl_bank(address_map &map);

	void pc386m_io(address_map &map);
	void pc386m_map(address_map &map);
	void pc486se_io(address_map &map);
	void pc486se_map(address_map &map);

//	virtual void machine_start() override;
//	virtual void machine_reset() override;

	DECLARE_MACHINE_START(pc98_epson);
	DECLARE_MACHINE_RESET(pc98_epson);

private:
	void epson_ipl_bank_w(offs_t offset, u8 data);
	void epson_itf_bank_w(offs_t offset, u8 data);
	void epson_a20_w(offs_t offset, u8 data);
	void epson_vram_bank_w(offs_t offset, u8 data);

	required_shared_ptr_array<uint16_t, 2> m_shadow_ipl;

	template <unsigned which> void shadow_ipl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	u8 m_shadow_ipl_bank;
	bool m_itf_bank_enable;
//	u8 m_itf_bank;
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
