// license:BSD-3-Clause
// copyright-holders:Angelo Salese,Carl
#ifndef MAME_INCLUDES_PC9801_H
#define MAME_INCLUDES_PC9801_H

#pragma once

#include "cpu/i386/i386.h"
#include "cpu/i86/i286.h"
#include "cpu/i86/i86.h"
#include "cpu/nec/nec.h"

#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "machine/bankdev.h"
#include "machine/buffer.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/output_latch.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/upd1990a.h"
#include "machine/upd765.h"

#include "bus/scsi/pc9801_sasi.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"

#include "sound/2608intf.h"
#include "sound/beep.h"
#include "sound/spkrdev.h"

#include "video/upd7220.h"

#include "bus/cbus/pc9801_26.h"
#include "bus/cbus/pc9801_86.h"
#include "bus/cbus/pc9801_118.h"
#include "bus/cbus/pc9801_amd98.h"
#include "bus/cbus/mpu_pc98.h"
#include "bus/cbus/pc9801_cbus.h"
#include "machine/pc9801_kbd.h"
#include "machine/pc9801_cd.h"

#include "machine/idectrl.h"
#include "machine/idehd.h"

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


#define UPD1990A_TAG "upd1990a"
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

class pc9801_state : public driver_device
{
public:
	pc9801_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dmac(*this, "i8237"),
		m_pit8253(*this, "pit8253"),
		m_pic1(*this, "pic8259_master"),
		m_pic2(*this, "pic8259_slave"),
		m_ppi_sys(*this, "ppi8255_sys"),
		m_ppi_prn(*this, "ppi8255_prn"),
		m_fdc_2hd(*this, "upd765_2hd"),
		m_fdc_2dd(*this, "upd765_2dd"),
		m_rtc(*this, UPD1990A_TAG),
		m_keyb(*this, "keyb"),
		m_sio(*this, UPD8251_TAG),
		m_hgdc1(*this, "upd7220_chr"),
		m_hgdc2(*this, "upd7220_btm"),
		m_sasibus(*this, SASIBUS_TAG),
		m_sasi_data_out(*this, "sasi_data_out"),
		m_sasi_data_in(*this, "sasi_data_in"),
		m_sasi_ctrl_in(*this, "sasi_ctrl_in"),
		m_ide(*this, "ide%u", 1U),
		m_video_ram_1(*this, "video_ram_1"),
		m_video_ram_2(*this, "video_ram_2"),
		m_ext_gvram(*this, "ext_gvram"),
		m_beeper(*this, "beeper"),
		m_ram(*this, RAM_TAG),
		m_ipl(*this, "ipl_bank"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{
	}

	void pc9821v20(machine_config &config);
	void pc9801ux(machine_config &config);
	void pc9801vm(machine_config &config);
	void pc9801(machine_config &config);
	void pc9801bx2(machine_config &config);
	void pc9801rs(machine_config &config);
	void pc9821(machine_config &config);
	void pc9821as(machine_config &config);
	void pc9821ap2(machine_config &config);
	DECLARE_CUSTOM_INPUT_MEMBER(system_type_r);
	void init_pc9801_kanji();
	void init_pc9801vm_kanji();

protected:
	virtual void video_start() override;

private:
	static void cdrom_headphones(device_t *device);

	required_device<cpu_device> m_maincpu;
	required_device<am9517a_device> m_dmac;
	required_device<pit8253_device> m_pit8253;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<i8255_device> m_ppi_sys;
	required_device<i8255_device> m_ppi_prn;
	required_device<upd765a_device> m_fdc_2hd;
	optional_device<upd765a_device> m_fdc_2dd;
	required_device<upd1990a_device> m_rtc;
	required_device<pc9801_kbd_device> m_keyb;
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
	required_device<beep_device> m_beeper;
	optional_device<ram_device> m_ram;
	optional_device<address_map_bank_device> m_ipl;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	DECLARE_WRITE8_MEMBER(rtc_w);
	DECLARE_WRITE8_MEMBER(dmapg4_w);
	DECLARE_WRITE8_MEMBER(dmapg8_w);
	DECLARE_WRITE8_MEMBER(nmi_ctrl_w);
	DECLARE_WRITE8_MEMBER(vrtc_clear_w);
	DECLARE_WRITE8_MEMBER(pc9801_video_ff_w);
	DECLARE_READ8_MEMBER(txt_scrl_r);
	DECLARE_WRITE8_MEMBER(txt_scrl_w);
	DECLARE_READ8_MEMBER(grcg_r);
	DECLARE_WRITE8_MEMBER(grcg_w);
	DECLARE_WRITE16_MEMBER(egc_w);
	DECLARE_READ8_MEMBER(pc9801_a0_r);
	DECLARE_WRITE8_MEMBER(pc9801_a0_w);
	DECLARE_READ8_MEMBER(fdc_2hd_ctrl_r);
	DECLARE_WRITE8_MEMBER(fdc_2hd_ctrl_w);
	DECLARE_READ8_MEMBER(fdc_2dd_ctrl_r);
	DECLARE_WRITE8_MEMBER(fdc_2dd_ctrl_w);
	DECLARE_READ16_MEMBER(tvram_r);
	DECLARE_WRITE16_MEMBER(tvram_w);
	DECLARE_READ8_MEMBER(gvram_r);
	DECLARE_WRITE8_MEMBER(gvram_w);
	DECLARE_WRITE8_MEMBER(pc9801rs_mouse_freq_w);
	DECLARE_READ16_MEMBER(grcg_gvram_r);
	DECLARE_WRITE16_MEMBER(grcg_gvram_w);
	DECLARE_READ16_MEMBER(grcg_gvram0_r);
	DECLARE_WRITE16_MEMBER(grcg_gvram0_w);

	DECLARE_READ16_MEMBER(pc9821_grcg_gvram_r);
	DECLARE_WRITE16_MEMBER(pc9821_grcg_gvram_w);
	DECLARE_READ16_MEMBER(pc9821_grcg_gvram0_r);
	DECLARE_WRITE16_MEMBER(pc9821_grcg_gvram0_w);

	DECLARE_READ16_MEMBER(upd7220_grcg_r);
	DECLARE_WRITE16_MEMBER(upd7220_grcg_w);

	DECLARE_READ8_MEMBER(ide_ctrl_r);
	DECLARE_WRITE8_MEMBER(ide_ctrl_w);
	DECLARE_READ16_MEMBER(ide_cs0_r);
	DECLARE_WRITE16_MEMBER(ide_cs0_w);
	DECLARE_READ16_MEMBER(ide_cs1_r);
	DECLARE_WRITE16_MEMBER(ide_cs1_w);

	DECLARE_WRITE8_MEMBER(sasi_data_w);
	DECLARE_READ8_MEMBER(sasi_data_r);
	DECLARE_WRITE_LINE_MEMBER(write_sasi_io);
	DECLARE_WRITE_LINE_MEMBER(write_sasi_req);
	DECLARE_READ8_MEMBER(sasi_status_r);
	DECLARE_WRITE8_MEMBER(sasi_ctrl_w);

	DECLARE_READ8_MEMBER(pc9801rs_knjram_r);
	DECLARE_WRITE8_MEMBER(pc9801rs_knjram_w);
	DECLARE_WRITE8_MEMBER(pc9801rs_bank_w);
	DECLARE_READ8_MEMBER(f0_r);

	DECLARE_READ8_MEMBER(a20_ctrl_r);
	DECLARE_WRITE8_MEMBER(a20_ctrl_w);
	DECLARE_READ8_MEMBER(fdc_mode_ctrl_r);
	DECLARE_WRITE8_MEMBER(fdc_mode_ctrl_w);
//  DECLARE_READ8_MEMBER(pc9801rs_2dd_r);
//  DECLARE_WRITE8_MEMBER(pc9801rs_2dd_w);
	DECLARE_WRITE8_MEMBER(pc9801rs_video_ff_w);
	DECLARE_WRITE8_MEMBER(pc9801rs_a0_w);
	DECLARE_WRITE8_MEMBER(pc9821_video_ff_w);
	DECLARE_READ8_MEMBER(pc9821_a0_r);
	DECLARE_WRITE8_MEMBER(pc9821_a0_w);
	DECLARE_READ8_MEMBER(access_ctrl_r);
	DECLARE_WRITE8_MEMBER(access_ctrl_w);
	DECLARE_READ8_MEMBER(midi_r);
//  DECLARE_READ8_MEMBER(winram_r);
//  DECLARE_WRITE8_MEMBER(winram_w);
	DECLARE_READ8_MEMBER(pic_r);
	DECLARE_WRITE8_MEMBER(pic_w);

	DECLARE_READ8_MEMBER(sdip_0_r);
	DECLARE_READ8_MEMBER(sdip_1_r);
	DECLARE_READ8_MEMBER(sdip_2_r);
	DECLARE_READ8_MEMBER(sdip_3_r);
	DECLARE_READ8_MEMBER(sdip_4_r);
	DECLARE_READ8_MEMBER(sdip_5_r);
	DECLARE_READ8_MEMBER(sdip_6_r);
	DECLARE_READ8_MEMBER(sdip_7_r);
	DECLARE_READ8_MEMBER(sdip_8_r);
	DECLARE_READ8_MEMBER(sdip_9_r);
	DECLARE_READ8_MEMBER(sdip_a_r);
	DECLARE_READ8_MEMBER(sdip_b_r);

	DECLARE_WRITE8_MEMBER(sdip_0_w);
	DECLARE_WRITE8_MEMBER(sdip_1_w);
	DECLARE_WRITE8_MEMBER(sdip_2_w);
	DECLARE_WRITE8_MEMBER(sdip_3_w);
	DECLARE_WRITE8_MEMBER(sdip_4_w);
	DECLARE_WRITE8_MEMBER(sdip_5_w);
	DECLARE_WRITE8_MEMBER(sdip_6_w);
	DECLARE_WRITE8_MEMBER(sdip_7_w);
	DECLARE_WRITE8_MEMBER(sdip_8_w);
	DECLARE_WRITE8_MEMBER(sdip_9_w);
	DECLARE_WRITE8_MEMBER(sdip_a_w);
	DECLARE_WRITE8_MEMBER(sdip_b_w);
	
	DECLARE_READ8_MEMBER(as_unkdev_data_r);
	DECLARE_WRITE8_MEMBER(as_unkdev_data_w);
	DECLARE_WRITE8_MEMBER(as_unkdev_addr_w);

	DECLARE_READ8_MEMBER(window_bank_r);
	DECLARE_WRITE8_MEMBER(window_bank_w);
	DECLARE_READ16_MEMBER(timestamp_r);
	DECLARE_READ8_MEMBER(ext2_video_ff_r);
	DECLARE_WRITE8_MEMBER(ext2_video_ff_w);

	DECLARE_FLOPPY_FORMATS( floppy_formats );
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
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE_LINE_MEMBER(dma_hrq_changed);
	DECLARE_WRITE_LINE_MEMBER(tc_w);
	DECLARE_READ8_MEMBER(dma_read_byte);
	DECLARE_WRITE8_MEMBER(dma_write_byte);
	DECLARE_WRITE_LINE_MEMBER(dack0_w);
	DECLARE_WRITE_LINE_MEMBER(dack1_w);
	DECLARE_WRITE_LINE_MEMBER(dack2_w);
	DECLARE_WRITE_LINE_MEMBER(dack3_w);
	DECLARE_WRITE8_MEMBER(ppi_sys_portc_w);

	DECLARE_WRITE_LINE_MEMBER(fdc_2dd_irq);
	DECLARE_WRITE_LINE_MEMBER(pc9801rs_fdc_irq);
	DECLARE_WRITE_LINE_MEMBER(pc9801rs_fdc_drq);

	DECLARE_READ8_MEMBER(ppi_mouse_porta_r);
	DECLARE_WRITE8_MEMBER(ppi_mouse_porta_w);
	DECLARE_WRITE8_MEMBER(ppi_mouse_portb_w);
	DECLARE_WRITE8_MEMBER(ppi_mouse_portc_w);
	TIMER_DEVICE_CALLBACK_MEMBER( mouse_irq_cb );
	DECLARE_READ8_MEMBER(unk_r);


	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t a20_286(bool state);

	void pc9801_keyboard(machine_config &config);
	void pc9801_mouse(machine_config &config);
	void pc9801_cbus(machine_config &config);
	void pc9801_sasi(machine_config &config);
	void pc9801_ide(machine_config &config);
	void pc9801_common(machine_config &config);
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
		uint8_t lx;
		uint8_t ly;
		uint8_t freq_reg;
		uint8_t freq_index;
	}m_mouse;

	uint8_t m_ide_sel;

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
	uint8_t m_sys_type;

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

	uint8_t m_sdip_read(uint16_t port, uint8_t sdip_offset);
	void m_sdip_write(uint16_t port, uint8_t sdip_offset,uint8_t data);
	uint16_t egc_do_partial_op(int plane, uint16_t src, uint16_t pat, uint16_t dst) const;
	uint16_t egc_shift(int plane, uint16_t val);
};

#endif // MAME_INCLUDES_PC9801_H
