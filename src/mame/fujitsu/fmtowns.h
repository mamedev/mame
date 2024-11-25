// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#ifndef MAME_FUJITSU_FMTOWNS_H
#define MAME_FUJITSU_FMTOWNS_H

#pragma once

#include "fmt_icmem.h"

#include "cpu/i386/i386.h"
#include "imagedev/cdromimg.h"
#include "imagedev/floppy.h"
#include "machine/fm_scsi.h"
#include "machine/i8251.h"
#include "machine/msm58321.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/upd71071.h"
#include "machine/wd_fdc.h"
#include "sound/cdda.h"
#include "sound/rf5c68.h"
#include "sound/spkrdev.h"
#include "sound/ymopn.h"

#include "bus/fmt_scsi/fmt121.h"
#include "bus/fmt_scsi/fmt_scsi.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/rs232/rs232.h"
#include "bus/msx/ctrl/ctrl.h"

#include "emupal.h"

#include "formats/fmtowns_dsk.h"


#define IRQ_LOG 0  // set to 1 to log IRQ line activity

struct towns_cdrom_controller
{
	uint8_t command = 0;
	uint8_t status = 0;
	uint8_t cmd_status[4]{};
	uint8_t cmd_status_ptr = 0;
	uint8_t extra_status = 0;
	uint8_t parameter[8]{};
	uint8_t mpu_irq_enable = 0;
	uint8_t dma_irq_enable = 0;
	uint8_t buffer[2048]{};
	int32_t buffer_ptr = 0;
	uint32_t lba_current = 0;
	uint32_t lba_last = 0;
	uint32_t cdda_current = 0;
	uint32_t cdda_length = 0;
	bool software_tx = false;
	emu_timer* read_timer = nullptr;
};

struct towns_video_controller
{
	uint8_t towns_vram_wplane = 0;
	uint8_t towns_vram_rplane = 0;
	uint8_t towns_vram_page_sel = 0;
	uint8_t towns_palette_select = 0;
	uint8_t towns_palette_r[256]{};
	uint8_t towns_palette_g[256]{};
	uint8_t towns_palette_b[256]{};
	uint8_t towns_degipal[8]{};
	uint8_t towns_dpmd_flag = 0;
	uint8_t towns_crtc_mix = 0;
	uint8_t towns_crtc_sel = 0;  // selected CRTC register
	uint16_t towns_crtc_reg[32]{};
	uint8_t towns_video_sel = 0;  // selected video register
	uint8_t towns_video_reg[2]{};
	uint8_t towns_sprite_sel = 0;  // selected sprite register
	uint8_t towns_sprite_reg[8]{};
	uint8_t towns_sprite_flag = 0;  // sprite drawing flag
	uint8_t towns_sprite_page = 0;  // VRAM page (not layer) sprites are drawn to
	uint8_t towns_tvram_enable = 0;
	uint16_t towns_kanji_offset = 0;
	uint8_t towns_kanji_code_h = 0;
	uint8_t towns_kanji_code_l = 0;
	rectangle towns_crtc_layerscr[2];  // each layer has independent sizes
	uint8_t towns_display_plane = 0;
	uint8_t towns_display_page_sel = 0;
	uint8_t towns_vblank_flag = 0;
	uint8_t towns_layer_ctrl = 0;
	emu_timer* sprite_timer = nullptr;
	emu_timer* vblank_end_timer = nullptr;
};

class towns_state : public driver_device
{
public:
	towns_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ram(*this, RAM_TAG)
		, m_maincpu(*this, "maincpu")
		, m_dma(*this, "dma_%u", 1U)
		, m_scsi(*this, "fmscsi")
		, m_flop(*this, "fdc:%u", 0U)
		, m_pad_ports(*this, "pad%u", 1U)
		, m_speaker(*this, "speaker")
		, m_pic_master(*this, "pic8259_master")
		, m_pic_slave(*this, "pic8259_slave")
		, m_pit(*this, "pit")
		, m_palette(*this, "palette256")
		, m_palette16(*this, "palette16_%u", 0U)
		, m_fdc(*this, "fdc")
		, m_icmemcard(*this, "icmemcard")
		, m_i8251(*this, "i8251")
		, m_rs232(*this, "rs232c")
		, m_screen(*this, "screen")
		, m_rtc(*this, "rtc58321")
		, m_dma_1(*this, "dma_1")
		, m_cdrom(*this, "cdrom")
		, m_cdda(*this, "cdda")
		, m_scsi_slot(*this, "scsislot")
		, m_bank_cb000_r(*this, "bank_cb000_r")
		, m_bank_cb000_w(*this, "bank_cb000_w")
		, m_bank_f8000_r(*this, "bank_f8000_r")
		, m_bank_f8000_w(*this, "bank_f8000_w")
		, m_nvram(*this, "nvram")
		, m_nvram16(*this, "nvram16")
		, m_kb_ports(*this, "key%u", 1U)
		, m_user(*this,"user")
		, m_serial(*this,"serial")
	{ }

	void towns_base(machine_config &config);
	void towns(machine_config &config);
	void townsftv(machine_config &config);
	void townshr(machine_config &config);
	void townsmx(machine_config &config);
	void townssj(machine_config &config);

	INTERRUPT_GEN_MEMBER(towns_vsync_irq);

protected:
	uint16_t m_towns_machine_id;  // default is 0x0101

	void marty_mem(address_map &map) ATTR_COLD;
	void pcm_mem(address_map &map) ATTR_COLD;
	void towns16_io(address_map &map) ATTR_COLD;
	void towns_io(address_map &map) ATTR_COLD;
	void towns_1g_io(address_map &map) ATTR_COLD;
	void towns2_io(address_map &map) ATTR_COLD;
	void townsux_io(address_map &map) ATTR_COLD;
	void towns_mem(address_map &map) ATTR_COLD;
	void ux_mem(address_map &map) ATTR_COLD;

	virtual void driver_start() override;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<ram_device> m_ram;
	required_device<cpu_device> m_maincpu;

	required_device_array<upd71071_device, 2> m_dma;
	optional_device<fmscsi_device> m_scsi;
	required_device_array<floppy_connector, 2> m_flop;

	required_device_array<msx_general_purpose_port_device, 2U> m_pad_ports;

	static void floppy_formats(format_registration &fr);

	void towns_scsi_irq(int state);
	void towns_scsi_drq(int state);

private:
	/* devices */
	required_device<speaker_sound_device> m_speaker;
	required_device<pic8259_device> m_pic_master;
	required_device<pic8259_device> m_pic_slave;
	required_device<pit8253_device> m_pit;
	required_device<palette_device> m_palette;
	required_device_array<palette_device, 2> m_palette16;
	required_device<mb8877_device> m_fdc;
	required_device<fmt_icmem_device> m_icmemcard;
	required_device<i8251_device> m_i8251;
	required_device<rs232_port_device> m_rs232;
	required_device<screen_device> m_screen;
	required_device<msm58321_device> m_rtc;
	required_device<upd71071_device> m_dma_1;
	required_device<cdrom_image_device> m_cdrom;
	required_device<cdda_device> m_cdda;
	optional_device<fmt_scsi_slot_device> m_scsi_slot;

	required_memory_bank m_bank_cb000_r;
	required_memory_bank m_bank_cb000_w;
	required_memory_bank m_bank_f8000_r;
	required_memory_bank m_bank_f8000_w;

	uint16_t m_ftimer = 0;
	uint16_t m_freerun_timer = 0;
	emu_timer* m_towns_freerun_counter = nullptr;
	uint16_t m_intervaltimer2_period = 0;
	uint8_t m_intervaltimer2_irqmask = 0;
	uint8_t m_intervaltimer2_timeout_flag = 0;
	uint8_t m_intervaltimer2_timeout_flag2 = 0;
	emu_timer* m_towns_intervaltimer2 = nullptr;
	uint8_t m_nmi_mask = 0;
	uint8_t m_compat_mode = 0;
	uint8_t m_towns_system_port = 0;
	uint32_t m_towns_ankcg_enable = 0;
	uint32_t m_towns_mainmem_enable = 0;
	uint32_t m_towns_ram_enable = 0;
	std::unique_ptr<uint32_t[]> m_towns_vram;
	std::unique_ptr<uint8_t[]> m_towns_gfxvram;
	std::unique_ptr<uint8_t[]> m_towns_txtvram;
	int m_towns_selected_drive = 0;
	uint8_t m_towns_fdc_irq6mask = 0;
	std::unique_ptr<uint8_t[]> m_towns_serial_rom;
	int m_towns_srom_position = 0;
	uint8_t m_towns_srom_clk = 0;
	uint8_t m_towns_srom_reset = 0;
	uint8_t m_towns_rtc_select = 0;
	uint8_t m_towns_rtc_data = 0;
	uint8_t m_towns_timer_mask = 0;
	uint8_t m_towns_kb_status = 0;
	uint8_t m_towns_kb_irq1_enable = 0;
	uint8_t m_towns_kb_output = 0;  // key output
	uint8_t m_towns_kb_extend = 0;  // extended key output
	emu_timer* m_towns_kb_timer = nullptr;
	uint8_t m_towns_fm_irq_flag = 0;
	uint8_t m_towns_pcm_irq_flag = 0;
	uint8_t m_towns_pcm_channel_flag = 0;
	uint8_t m_towns_pcm_channel_mask = 0;
	uint8_t m_towns_pad_mask = 0;
	uint8_t m_towns_volume[4]{};  // volume ports
	uint8_t m_towns_volume_select = 0;
	uint8_t m_towns_scsi_control = 0;
	uint8_t m_towns_scsi_status = 0;
	uint8_t m_towns_spkrdata = 0;
	uint8_t m_pit_out0 = 0;
	uint8_t m_pit_out1 = 0;
	uint8_t m_pit_out2 = 0;
	uint8_t m_timer0 = 0;
	uint8_t m_timer1 = 0;

	uint8_t m_serial_irq_source = 0;
	uint8_t m_serial_irq_enable = 0;  // RS232 interrupt control

	enum
	{
		TXC_EXTERNAL      = 0x80,
		RXC_EXTERNAL      = 0x40,
		ER_CONTROL        = 0x20,
		CI_IRQ_ENABLE     = 0x10,
		CS_IRQ_ENABLE     = 0x08,
		SYNDET_IRQ_ENABLE = 0x04,
		RXRDY_IRQ_ENABLE  = 0x02,
		TXRDY_IRQ_ENABLE  = 0x01
	};

	emu_timer* m_towns_wait_timer = nullptr;
	emu_timer* m_towns_status_timer = nullptr;
	emu_timer* m_towns_cdda_timer = nullptr;
	emu_timer* m_towns_seek_timer = nullptr;
	struct towns_cdrom_controller m_towns_cd;
	struct towns_video_controller m_video;

	uint32_t m_kb_prev[4]{};
	uint8_t m_prev_pad_mask = 0;
	uint8_t m_prev_x = 0;
	uint8_t m_prev_y = 0;

	optional_shared_ptr<uint32_t> m_nvram;
	optional_shared_ptr<uint16_t> m_nvram16;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t towns_system_r(offs_t offset);
	void towns_system_w(offs_t offset, uint8_t data);
	uint8_t towns_intervaltimer2_r(offs_t offset);
	void towns_intervaltimer2_w(offs_t offset, uint8_t data);
	uint8_t towns_sys6c_r();
	void towns_sys6c_w(uint8_t data);
	template<int Chip> uint8_t towns_dma_r(offs_t offset);
	template<int Chip> void towns_dma_w(offs_t offset, uint8_t data);
	uint8_t towns_floppy_r(offs_t offset);
	void towns_floppy_w(offs_t offset, uint8_t data);
	uint8_t towns_keyboard_r(offs_t offset);
	void towns_keyboard_w(offs_t offset, uint8_t data);
	uint8_t towns_port60_r();
	void towns_port60_w(uint8_t data);
	uint8_t towns_sys5e8_r(offs_t offset);
	void towns_sys5e8_w(offs_t offset, uint8_t data);
	uint8_t towns_sound_ctrl_r(offs_t offset);
	void towns_sound_ctrl_w(offs_t offset, uint8_t data);
	uint8_t towns_padport_r(offs_t offset);
	void towns_pad_mask_w(uint8_t data);
	uint8_t towns_cmos_low_r(offs_t offset);
	void towns_cmos_low_w(offs_t offset, uint8_t data);
	uint8_t towns_cmos_r(offs_t offset);
	void towns_cmos_w(offs_t offset, uint8_t data);
	uint8_t towns_sys480_r();
	void towns_sys480_w(uint8_t data);
	uint8_t towns_video_404_r();
	void towns_video_404_w(uint8_t data);
	uint8_t towns_cdrom_r(offs_t offset);
	void towns_cdrom_w(offs_t offset, uint8_t data);
	uint8_t towns_rtc_r();
	void towns_rtc_w(uint8_t data);
	void towns_rtc_select_w(uint8_t data);
	uint8_t towns_volume_r(offs_t offset);
	void towns_volume_w(offs_t offset, uint8_t data);
	uint8_t unksnd_r();
	uint8_t towns_41ff_r();

	uint8_t towns_gfx_high_r(offs_t offset);
	void towns_gfx_high_w(offs_t offset, uint8_t data);
	uint8_t towns_gfx_packed_r(offs_t offset);
	void towns_gfx_packed_w(offs_t offset, uint8_t data);
	uint8_t towns_gfx_r(offs_t offset);
	void towns_gfx_w(offs_t offset, uint8_t data);
	uint8_t towns_video_cff80_r(offs_t offset);
	void towns_video_cff80_w(offs_t offset, uint8_t data);
	uint8_t towns_video_cff80_mem_r(offs_t offset);
	void towns_video_cff80_mem_w(offs_t offset, uint8_t data);
	uint8_t towns_video_440_r(offs_t offset);
	void towns_video_440_w(offs_t offset, uint8_t data);
	uint8_t towns_video_5c8_r(offs_t offset);
	void towns_video_5c8_w(offs_t offset, uint8_t data);
	uint8_t towns_video_fd90_r(offs_t offset);
	void towns_video_fd90_w(offs_t offset, uint8_t data);
	uint8_t towns_video_ff81_r();
	uint8_t towns_video_unknown_r();
	void towns_video_ff81_w(uint8_t data);
	uint8_t towns_spriteram_low_r(offs_t offset);
	void towns_spriteram_low_w(offs_t offset, uint8_t data);
	uint8_t towns_spriteram_r(offs_t offset);
	void towns_spriteram_w(offs_t offset, uint8_t data);

	void mb8877a_irq_w(int state);
	void mb8877a_drq_w(int state);
	void pit_out2_changed(int state);

	void towns_serial_irq(int state);
	void towns_rxrdy_irq(int state);
	void towns_txrdy_irq(int state);
	void towns_syndet_irq(int state);
	uint8_t towns_serial_r(offs_t offset);
	void towns_serial_w(offs_t offset, uint8_t data);

	void rtc_d0_w(int state);
	void rtc_d1_w(int state);
	void rtc_d2_w(int state);
	void rtc_d3_w(int state);
	void rtc_busy_w(int state);

	RF5C68_SAMPLE_END_CB_MEMBER(towns_pcm_irq);

	void towns_update_video_banks();
	void init_serial_rom();
	void kb_sendcode(uint8_t scancode, int release);
	uint8_t speaker_get_spk();
	void speaker_set_spkrdata(uint8_t data);
	uint8_t towns_cdrom_read_byte_software();
	void cdda_db_to_gain(float db);

	required_ioport_array<4> m_kb_ports;
	required_memory_region m_user;
	optional_memory_region m_serial;

	TIMER_CALLBACK_MEMBER(freerun_inc);
	TIMER_CALLBACK_MEMBER(intervaltimer2_timeout);
	TIMER_CALLBACK_MEMBER(poll_keyboard);
	TIMER_CALLBACK_MEMBER(wait_end);
	void towns_cd_set_status(uint8_t st0, uint8_t st1, uint8_t st2, uint8_t st3);
	void towns_cdrom_execute_command(cdrom_image_device* device);
	void towns_cdrom_play_cdda(cdrom_image_device* device);
	void towns_cdrom_read(cdrom_image_device* device);
	TIMER_CALLBACK_MEMBER(towns_cd_status_ready);
	TIMER_CALLBACK_MEMBER(towns_delay_cdda);
	TIMER_CALLBACK_MEMBER(towns_delay_seek);

	u8 m_rtc_d = 0;
	bool m_rtc_busy = false;
	u8 m_vram_mask[4]{};
	u8 m_vram_mask_addr = 0;

	TIMER_CALLBACK_MEMBER(towns_cdrom_read_byte);
	TIMER_CALLBACK_MEMBER(towns_vblank_end);
	TIMER_CALLBACK_MEMBER(draw_sprites);
	void towns_pit_out0_changed(int state);
	void towns_pit_out1_changed(int state);
	void pit2_out1_changed(int state);
	uint8_t get_slave_ack(offs_t offset);
	void towns_fm_irq(int state);
	void towns_sprite_start();
	void towns_crtc_refresh_mode();
	void towns_update_kanji_offset();
	void towns_update_palette();
	void render_sprite_4(uint32_t poffset, uint32_t coffset, uint16_t x, uint16_t y, bool xflip, bool yflip, bool xhalfsize, bool yhalfsize, bool rotation, const rectangle* rect);
	void render_sprite_16(uint32_t poffset, uint16_t x, uint16_t y, bool xflip, bool yflip, bool xhalfsize, bool yhalfsize, bool rotation, const rectangle* rect);
	void towns_crtc_draw_scan_layer_hicolour(bitmap_rgb32 &bitmap,const rectangle* rect,int layer,int line,int scanline);
	void towns_crtc_draw_scan_layer_256(bitmap_rgb32 &bitmap,const rectangle* rect,int line,int scanline);
	void towns_crtc_draw_scan_layer_16(bitmap_rgb32 &bitmap,const rectangle* rect,int layer,int line,int scanline);
	void towns_crtc_draw_layer(bitmap_rgb32 &bitmap,const rectangle* rect,int layer);
	void render_text_char(uint8_t x, uint8_t y, uint8_t ascii, uint16_t jis, uint8_t attr);
	void draw_text_layer();
	inline uint8_t byte_to_bcd(uint8_t val);
	inline uint8_t bcd_to_byte(uint8_t val);
	inline uint32_t msf_to_lbafm(uint32_t val);  // because the CDROM core doesn't provide this;
	uint16_t towns_fdc_dma_r();
	void towns_fdc_dma_w(uint16_t data);
	void towns_cdrom_set_irq(int line,int state);
	uint8_t towns_cd_get_track();
	uint16_t towns_cdrom_dma_r();
};

class towns16_state : public towns_state
{
	public:
	towns16_state(const machine_config &mconfig, device_type type, const char *tag)
		: towns_state(mconfig, type, tag)
	{ }
	void townsux(machine_config &config);
};

class marty_state : public towns_state
{
	public:
	marty_state(const machine_config &mconfig, device_type type, const char *tag)
		: towns_state(mconfig, type, tag)
	{ }

	void marty(machine_config &config);

protected:
	virtual void driver_start() override;
};

#endif // MAME_FUJITSU_FMTOWNS_H
