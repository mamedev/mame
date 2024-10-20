// license:BSD-3-Clause
// copyright-holders:Angelo Salese,Carl
/******************************************
 *
 * NEC PC-9801
 *
 ******************************************/

#ifndef MAME_NEC_PC9801_H
#define MAME_NEC_PC9801_H

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
#include "pc9801_memsw.h"
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
#include "bus/cbus/pc9801_55.h"
#include "bus/cbus/pc9801_86.h"
#include "bus/cbus/pc9801_118.h"
#include "bus/cbus/pc9801_amd98.h"
#include "bus/cbus/mpu_pc98.h"
#include "bus/cbus/pc9801_cbus.h"
#include "pc9801_kbd.h"
#include "pc9801_cd.h"

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

	ioport_value system_type_r();

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

	u8 m_sys_type = 0;
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
		, m_dsw1(*this, "DSW1")
		, m_dsw2(*this, "DSW2")
		, m_ppi_mouse(*this, "ppi_mouse")
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
	required_ioport m_dsw1;
	required_ioport m_dsw2;
	required_device<i8255_device> m_ppi_mouse;
	// TODO: should really be one FDC
	// (I/O $90-$93 is a "simplified" version)
	required_device<upd765a_device> m_fdc_2hd;
	optional_device<upd765a_device> m_fdc_2dd;
	optional_device<ram_device> m_ram;
	required_device_array<upd7220_device, 2> m_hgdc;
	required_shared_ptr_array<uint16_t, 2> m_video_ram;
	required_device_array<pc9801_slot_device, 2> m_cbus;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
private:
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

	void pc9801_common_io(address_map &map) ATTR_COLD;
	void ipl_bank(address_map &map) ATTR_COLD;

	uint8_t pc9801_a0_r(offs_t offset);
	void pc9801_a0_w(offs_t offset, uint8_t data);
	u8 unk_r(offs_t offset);
	uint8_t f0_r(offs_t offset);

	uint8_t m_nmi_ff = 0;

	virtual u8 ppi_prn_portb_r();

private:
	void pc9801_io(address_map &map) ATTR_COLD;
	void pc9801_map(address_map &map) ATTR_COLD;

	void nmi_ctrl_w(offs_t offset, uint8_t data);

	u8 ppi_sys_portb_r();

	void sasi_data_w(uint8_t data);
	uint8_t sasi_data_r();
	void write_sasi_io(int state);
	void write_sasi_req(int state);
	uint8_t sasi_status_r();
	void sasi_ctrl_w(uint8_t data);
	void draw_text(bitmap_rgb32 &bitmap, uint32_t addr, int y, int wd, int pitch, int lr, int cursor_on, int cursor_addr, bool lower);

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

	u8 m_fdc_2hd_ctrl = 0;

	bool fdc_drive_ready_r(upd765a_device *fdc);
private:
	void fdc_2dd_irq(int state);

	uint8_t fdc_2dd_ctrl_r();
	void fdc_2dd_ctrl_w(uint8_t data);

	u8 m_fdc_2dd_ctrl = 0;

//  DMA
protected:
	uint8_t m_dma_offset[4];
	uint8_t m_dma_autoinc[4];
	int m_dack;

	virtual uint8_t dma_read_byte(offs_t offset);
	virtual void dma_write_byte(offs_t offset, uint8_t data);

private:
	void dmapg4_w(offs_t offset, uint8_t data);

	inline void set_dma_channel(int channel, int state);

	void dma_hrq_changed(int state);
	void tc_w(int state);

	void dack0_w(int state);
	void dack1_w(int state);
	void dack2_w(int state);
	void dack3_w(int state);

//  Video
protected:
	void upd7220_1_map(address_map &map) ATTR_COLD;
	void upd7220_2_map(address_map &map) ATTR_COLD;

	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );

	virtual void video_start() override ATTR_COLD;
	void pc9801_palette(palette_device &palette) const;

	uint8_t *m_char_rom = nullptr;
	uint8_t *m_kanji_rom = nullptr;

	struct {
		uint8_t mode = 0;
		uint8_t tile[4]{}, tile_index = 0;
	} m_grcg;

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
	u8 m_vram_bank = 0;
	u8 m_vram_disp = 0;

private:
	UPD7220_DRAW_TEXT_LINE_MEMBER( hgdc_draw_text );

	void vrtc_irq(int state);
	void vrtc_clear_w(uint8_t data);
	uint8_t txt_scrl_r(offs_t offset);
	void txt_scrl_w(offs_t offset, uint8_t data);

	// (virtual is necessary for H98 high-reso mode, PC9821-E02, SVGA binds)
	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t m_font_line = 0;
	std::unique_ptr<uint16_t[]> m_tvram;
	uint8_t m_gfx_ff = 0;
	uint8_t m_txt_scroll_reg[8]{};
	uint8_t m_pal_clut[4]{};

//  SASI
	uint8_t m_sasi_data = 0;
	int m_sasi_data_enable = 0;
	uint8_t m_sasi_ctrl = 0;

//  Mouse
protected:
	struct{
		uint8_t control = 0;
		uint8_t lx = 0, ly = 0;
		uint8_t dx = 0, dy = 0;
		uint8_t prev_dx = 0, prev_dy = 0;
		uint8_t freq_reg = 0;
		uint8_t freq_index = 0;
	} m_mouse;

private:
	u8 ppi_mouse_porta_r();
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
		, m_dsw3(*this, "DSW3")
		, m_ide(*this, "ide%u", 1U)
		, m_dac1bit(*this, "dac1bit")
	{
	}

	void pc9801vm(machine_config &config);

	void pc9801ux(machine_config &config);
	void pc9801vx(machine_config &config);
	void pc9801rs(machine_config &config);
	void pc9801dx(machine_config &config);
	void pc9801fs(machine_config &config);

	void init_pc9801vm_kanji();

protected:
	TIMER_CALLBACK_MEMBER(fdc_trigger);

	void pc9801rs_io(address_map &map) ATTR_COLD;
	void pc9801rs_map(address_map &map) ATTR_COLD;
	void pc9801ux_io(address_map &map) ATTR_COLD;
	void pc9801ux_map(address_map &map) ATTR_COLD;

	uint16_t grcg_gvram_r(offs_t offset, uint16_t mem_mask = ~0);
	void grcg_gvram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t grcg_gvram0_r(offs_t offset, uint16_t mem_mask = ~0);
	void grcg_gvram0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t upd7220_grcg_r(offs_t offset, uint16_t mem_mask = ~0);
	void upd7220_grcg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void upd7220_grcg_2_map(address_map &map) ATTR_COLD;

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
	virtual u8 ppi_prn_portb_r() override;
	uint32_t a20_286(bool state);

	DECLARE_MACHINE_START(pc9801rs);
	DECLARE_MACHINE_RESET(pc9801rs);

	u8 m_gate_a20 = 0;
	u8 m_dma_access_ctrl = 0;
	u8 m_ide_sel = 0;

	virtual uint8_t dma_read_byte(offs_t offset) override;
	virtual void dma_write_byte(offs_t offset, uint8_t data) override;

	// starting from PC9801VF/U buzzer is substituted with a DAC1BIT
	bool m_dac1bit_disable;

	uint8_t pc9801rs_knjram_r(offs_t offset);
	void pc9801rs_knjram_w(offs_t offset, uint8_t data);

	required_ioport m_dsw3;
private:
	optional_device_array<ata_interface_device, 2> m_ide;
//  optional_device<dac_1bit_device> m_dac1bit;
	required_device<speaker_sound_device> m_dac1bit;

	void pc9801rs_bank_w(offs_t offset, uint8_t data);
	uint8_t midi_r();

	// 286-based machines except for PC98XA
	u8 dma_access_ctrl_r(offs_t offset);
	void dma_access_ctrl_w(offs_t offset, u8 data);

	uint8_t a20_ctrl_r(offs_t offset);
	void a20_ctrl_w(offs_t offset, uint8_t data);

	template <unsigned port> u8 fdc_2hd_2dd_ctrl_r();
	template <unsigned port> void fdc_2hd_2dd_ctrl_w(u8 data);

	void fdc_irq_w(int state);
	void fdc_drq_w(int state);

	emu_timer *m_fdc_timer = nullptr;

	u8 m_fdc_mode = 0;
	u8 fdc_mode_r();
	void fdc_mode_w(u8 data);
	void fdc_set_density_mode(bool is_2hd);

protected:
	struct {
		uint8_t pal_entry = 0;
		uint8_t r[16]{}, g[16]{}, b[16]{};
	} m_analog16;

private:
	// EGC, PC9801VX onward
	struct {
		uint16_t regs[8]{};
		uint16_t pat[4]{};
		uint16_t src[4]{};
		int16_t count = 0;
		uint16_t leftover[4]{};
		bool first = false;
		bool init = false;
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

	u8 ppi_mouse_portb_r();
	u8 ppi_mouse_portc_r();
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
	void pc9801us_io(address_map &map) ATTR_COLD;

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
	void pc9801bx2_io(address_map &map) ATTR_COLD;
	void pc9801bx2_map(address_map &map) ATTR_COLD;

	DECLARE_MACHINE_START(pc9801bx2);
	DECLARE_MACHINE_RESET(pc9801bx2);

private:
	u8 i486_cpu_mode_r(offs_t offset);
	u8 gdc_31kHz_r(offs_t offset);
	void gdc_31kHz_w(offs_t offset, u8 data);
};

#endif // MAME_NEC_PC9801_H
