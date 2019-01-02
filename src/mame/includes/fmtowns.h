// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#ifndef MAME_INCLUDES_FMTOWNS_H
#define MAME_INCLUDES_FMTOWNS_H

#pragma once

#include "cpu/i386/i386.h"
#include "imagedev/chd_cd.h"
#include "imagedev/floppy.h"
#include "machine/fm_scsi.h"
#include "machine/fmt_icmem.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/upd71071.h"
#include "machine/wd_fdc.h"
#include "machine/i8251.h"
#include "machine/msm58321.h"
#include "sound/2612intf.h"
#include "sound/cdda.h"
#include "sound/rf5c68.h"
#include "sound/spkrdev.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/rs232/rs232.h"

#include "formats/fmtowns_dsk.h"

#include "emupal.h"


#define IRQ_LOG 0  // set to 1 to log IRQ line activity

struct towns_cdrom_controller
{
	uint8_t command;
	uint8_t status;
	uint8_t cmd_status[4];
	uint8_t cmd_status_ptr;
	uint8_t extra_status;
	uint8_t parameter[8];
	uint8_t mpu_irq_enable;
	uint8_t dma_irq_enable;
	uint8_t buffer[2048];
	int32_t buffer_ptr;
	uint32_t lba_current;
	uint32_t lba_last;
	uint32_t cdda_current;
	uint32_t cdda_length;
	bool software_tx;
	emu_timer* read_timer;
};

struct towns_video_controller
{
	uint8_t towns_vram_wplane;
	uint8_t towns_vram_rplane;
	uint8_t towns_vram_page_sel;
	uint8_t towns_palette_select;
	uint8_t towns_palette_r[256];
	uint8_t towns_palette_g[256];
	uint8_t towns_palette_b[256];
	uint8_t towns_degipal[8];
	uint8_t towns_dpmd_flag;
	uint8_t towns_crtc_mix;
	uint8_t towns_crtc_sel;  // selected CRTC register
	uint16_t towns_crtc_reg[32];
	uint8_t towns_video_sel;  // selected video register
	uint8_t towns_video_reg[2];
	uint8_t towns_sprite_sel;  // selected sprite register
	uint8_t towns_sprite_reg[8];
	uint8_t towns_sprite_flag;  // sprite drawing flag
	uint8_t towns_sprite_page;  // VRAM page (not layer) sprites are drawn to
	uint8_t towns_tvram_enable;
	uint16_t towns_kanji_offset;
	uint8_t towns_kanji_code_h;
	uint8_t towns_kanji_code_l;
	rectangle towns_crtc_layerscr[2];  // each layer has independent sizes
	uint8_t towns_display_plane;
	uint8_t towns_display_page_sel;
	uint8_t towns_vblank_flag;
	uint8_t towns_layer_ctrl;
	emu_timer* sprite_timer;
};

class towns_state : public driver_device
{
	public:
	towns_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ram(*this, RAM_TAG)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_pic_master(*this, "pic8259_master")
		, m_pic_slave(*this, "pic8259_slave")
		, m_pit(*this, "pit")
		, m_dma(*this, "dma_%u", 1U)
		, m_palette(*this, "palette256")
		, m_palette16(*this, "palette16_%u", 0U)
		, m_fdc(*this, "fdc")
		, m_flop(*this, "fdc:%u", 0U)
		, m_icmemcard(*this, "icmemcard")
		, m_i8251(*this, "i8251")
		, m_rs232(*this, "rs232c")
		, m_screen(*this, "screen")
		, m_rtc(*this, "rtc58321")
		, m_dma_1(*this, "dma_1")
		, m_cdrom(*this, "cdrom")
		, m_cdda(*this, "cdda")
		, m_scsi(*this, "fmscsi")
		, m_bank_cb000_r(*this, "bank_cb000_r")
		, m_bank_cb000_w(*this, "bank_cb000_w")
		, m_bank_f8000_r(*this, "bank_f8000_r")
		, m_bank_f8000_w(*this, "bank_f8000_w")
		, m_nvram(*this, "nvram")
		, m_nvram16(*this, "nvram16")
		, m_ctrltype(*this, "ctrltype")
		, m_kb_ports(*this, "key%u", 1U)
		, m_joy1(*this, "joy1")
		, m_joy2(*this, "joy2")
		, m_joy1_ex(*this, "joy1_ex")
		, m_joy2_ex(*this, "joy2_ex")
		, m_6b_joy1(*this, "6b_joy1")
		, m_6b_joy2(*this, "6b_joy2")
		, m_6b_joy1_ex(*this, "6b_joy1_ex")
		, m_6b_joy2_ex(*this, "6b_joy2_ex")
		, m_mouse1(*this, "mouse1")
		, m_mouse2(*this, "mouse2")
		, m_mouse3(*this, "mouse3")
		, m_user(*this,"user")
		, m_serial(*this,"serial")
	{ }

	void towns_base(machine_config &config);
	void towns(machine_config &config);
	void townsftv(machine_config &config);
	void townshr(machine_config &config);
	void townssj(machine_config &config);

	INTERRUPT_GEN_MEMBER(towns_vsync_irq);

protected:
	uint16_t m_towns_machine_id;  // default is 0x0101

	void marty_mem(address_map &map);
	void pcm_mem(address_map &map);
	void towns16_io(address_map &map);
	void towns_io(address_map &map);
	void towns_mem(address_map &map);
	void ux_mem(address_map &map);

	virtual void driver_start() override;

	required_device<ram_device> m_ram;
	required_device<cpu_device> m_maincpu;

private:
	/* devices */
	required_device<speaker_sound_device> m_speaker;
	required_device<pic8259_device> m_pic_master;
	required_device<pic8259_device> m_pic_slave;
	required_device<pit8253_device> m_pit;
	required_device_array<upd71071_device, 2> m_dma;
	required_device<palette_device> m_palette;
	required_device_array<palette_device, 2> m_palette16;
	required_device<mb8877_device> m_fdc;
	required_device_array<floppy_connector, 2> m_flop;
	required_device<fmt_icmem_device> m_icmemcard;
	required_device<i8251_device> m_i8251;
	required_device<rs232_port_device> m_rs232;
	required_device<screen_device> m_screen;
	required_device<msm58321_device> m_rtc;
	required_device<upd71071_device> m_dma_1;
	required_device<cdrom_image_device> m_cdrom;
	required_device<cdda_device> m_cdda;
	required_device<fmscsi_device> m_scsi;

	required_memory_bank m_bank_cb000_r;
	required_memory_bank m_bank_cb000_w;
	required_memory_bank m_bank_f8000_r;
	required_memory_bank m_bank_f8000_w;

	uint16_t m_ftimer;
	uint16_t m_freerun_timer;
	emu_timer* m_towns_freerun_counter;
	uint16_t m_intervaltimer2_period;
	uint8_t m_intervaltimer2_irqmask;
	uint8_t m_intervaltimer2_timeout_flag;
	uint8_t m_intervaltimer2_timeout_flag2;
	emu_timer* m_towns_intervaltimer2;
	uint8_t m_nmi_mask;
	uint8_t m_compat_mode;
	uint8_t m_towns_system_port;
	uint32_t m_towns_ankcg_enable;
	uint32_t m_towns_mainmem_enable;
	uint32_t m_towns_ram_enable;
	std::unique_ptr<uint32_t[]> m_towns_vram;
	std::unique_ptr<uint8_t[]> m_towns_gfxvram;
	std::unique_ptr<uint8_t[]> m_towns_txtvram;
	int m_towns_selected_drive;
	uint8_t m_towns_fdc_irq6mask;
	std::unique_ptr<uint8_t[]> m_towns_serial_rom;
	int m_towns_srom_position;
	uint8_t m_towns_srom_clk;
	uint8_t m_towns_srom_reset;
	uint8_t m_towns_rtc_select;
	uint8_t m_towns_rtc_data;
	uint8_t m_towns_timer_mask;
	uint8_t m_towns_kb_status;
	uint8_t m_towns_kb_irq1_enable;
	uint8_t m_towns_kb_output;  // key output
	uint8_t m_towns_kb_extend;  // extended key output
	emu_timer* m_towns_kb_timer;
	emu_timer* m_towns_mouse_timer;
	uint8_t m_towns_fm_irq_flag;
	uint8_t m_towns_pcm_irq_flag;
	uint8_t m_towns_pcm_channel_flag;
	uint8_t m_towns_pcm_channel_mask;
	uint8_t m_towns_pad_mask;
	uint8_t m_towns_mouse_output;
	uint8_t m_towns_mouse_x;
	uint8_t m_towns_mouse_y;
	uint8_t m_towns_volume[8];  // volume ports
	uint8_t m_towns_volume_select;
	uint8_t m_towns_scsi_control;
	uint8_t m_towns_scsi_status;
	uint8_t m_towns_spkrdata;
	uint8_t m_pit_out0;
	uint8_t m_pit_out1;
	uint8_t m_pit_out2;
	uint8_t m_timer0;
	uint8_t m_timer1;

	uint8_t m_serial_irq_source;
	uint8_t m_serial_irq_enable;  // RS232 interrupt control

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

	emu_timer* m_towns_wait_timer;
	emu_timer* m_towns_status_timer;
	emu_timer* m_towns_cdda_timer;
	struct towns_cdrom_controller m_towns_cd;
	struct towns_video_controller m_video;

	uint32_t m_kb_prev[4];
	uint8_t m_prev_pad_mask;
	uint8_t m_prev_x;
	uint8_t m_prev_y;

	optional_shared_ptr<uint32_t> m_nvram;
	optional_shared_ptr<uint16_t> m_nvram16;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	DECLARE_READ8_MEMBER(towns_system_r);
	DECLARE_WRITE8_MEMBER(towns_system_w);
	DECLARE_READ8_MEMBER(towns_intervaltimer2_r);
	DECLARE_WRITE8_MEMBER(towns_intervaltimer2_w);
	DECLARE_READ8_MEMBER(towns_sys6c_r);
	DECLARE_WRITE8_MEMBER(towns_sys6c_w);
	template<int Chip> DECLARE_READ8_MEMBER(towns_dma_r);
	template<int Chip> DECLARE_WRITE8_MEMBER(towns_dma_w);
	DECLARE_READ8_MEMBER(towns_floppy_r);
	DECLARE_WRITE8_MEMBER(towns_floppy_w);
	DECLARE_READ8_MEMBER(towns_keyboard_r);
	DECLARE_WRITE8_MEMBER(towns_keyboard_w);
	DECLARE_READ8_MEMBER(towns_port60_r);
	DECLARE_WRITE8_MEMBER(towns_port60_w);
	DECLARE_READ8_MEMBER(towns_sys5e8_r);
	DECLARE_WRITE8_MEMBER(towns_sys5e8_w);
	DECLARE_READ8_MEMBER(towns_sound_ctrl_r);
	DECLARE_WRITE8_MEMBER(towns_sound_ctrl_w);
	DECLARE_READ8_MEMBER(towns_padport_r);
	DECLARE_WRITE8_MEMBER(towns_pad_mask_w);
	DECLARE_READ8_MEMBER(towns_cmos_low_r);
	DECLARE_WRITE8_MEMBER(towns_cmos_low_w);
	DECLARE_READ8_MEMBER(towns_cmos_r);
	DECLARE_WRITE8_MEMBER(towns_cmos_w);
	DECLARE_READ8_MEMBER(towns_sys480_r);
	DECLARE_WRITE8_MEMBER(towns_sys480_w);
	DECLARE_READ8_MEMBER(towns_video_404_r);
	DECLARE_WRITE8_MEMBER(towns_video_404_w);
	DECLARE_READ8_MEMBER(towns_cdrom_r);
	DECLARE_WRITE8_MEMBER(towns_cdrom_w);
	DECLARE_READ8_MEMBER(towns_rtc_r);
	DECLARE_WRITE8_MEMBER(towns_rtc_w);
	DECLARE_WRITE8_MEMBER(towns_rtc_select_w);
	DECLARE_READ8_MEMBER(towns_volume_r);
	DECLARE_WRITE8_MEMBER(towns_volume_w);
	DECLARE_READ8_MEMBER(unksnd_r);
	DECLARE_READ8_MEMBER(towns_41ff_r);

	DECLARE_READ8_MEMBER(towns_gfx_high_r);
	DECLARE_WRITE8_MEMBER(towns_gfx_high_w);
	DECLARE_READ8_MEMBER(towns_gfx_packed_r);
	DECLARE_WRITE8_MEMBER(towns_gfx_packed_w);
	DECLARE_READ8_MEMBER(towns_gfx_r);
	DECLARE_WRITE8_MEMBER(towns_gfx_w);
	DECLARE_READ8_MEMBER(towns_video_cff80_r);
	DECLARE_WRITE8_MEMBER(towns_video_cff80_w);
	DECLARE_READ8_MEMBER(towns_video_cff80_mem_r);
	DECLARE_WRITE8_MEMBER(towns_video_cff80_mem_w);
	DECLARE_READ8_MEMBER(towns_video_440_r);
	DECLARE_WRITE8_MEMBER(towns_video_440_w);
	DECLARE_READ8_MEMBER(towns_video_5c8_r);
	DECLARE_WRITE8_MEMBER(towns_video_5c8_w);
	DECLARE_READ8_MEMBER(towns_video_fd90_r);
	DECLARE_WRITE8_MEMBER(towns_video_fd90_w);
	DECLARE_READ8_MEMBER(towns_video_ff81_r);
	DECLARE_READ8_MEMBER(towns_video_unknown_r);
	DECLARE_WRITE8_MEMBER(towns_video_ff81_w);
	DECLARE_READ8_MEMBER(towns_spriteram_low_r);
	DECLARE_WRITE8_MEMBER(towns_spriteram_low_w);
	DECLARE_READ8_MEMBER(towns_spriteram_r);
	DECLARE_WRITE8_MEMBER(towns_spriteram_w);

	DECLARE_WRITE_LINE_MEMBER(mb8877a_irq_w);
	DECLARE_WRITE_LINE_MEMBER(mb8877a_drq_w);
	DECLARE_WRITE_LINE_MEMBER(pit_out2_changed);

	DECLARE_WRITE_LINE_MEMBER(towns_serial_irq);
	DECLARE_WRITE_LINE_MEMBER(towns_rxrdy_irq);
	DECLARE_WRITE_LINE_MEMBER(towns_txrdy_irq);
	DECLARE_WRITE_LINE_MEMBER(towns_syndet_irq);
	DECLARE_READ8_MEMBER(towns_serial_r);
	DECLARE_WRITE8_MEMBER(towns_serial_w);

	DECLARE_WRITE_LINE_MEMBER(rtc_d0_w);
	DECLARE_WRITE_LINE_MEMBER(rtc_d1_w);
	DECLARE_WRITE_LINE_MEMBER(rtc_d2_w);
	DECLARE_WRITE_LINE_MEMBER(rtc_d3_w);
	DECLARE_WRITE_LINE_MEMBER(rtc_busy_w);

	RF5C68_SAMPLE_END_CB_MEMBER(towns_pcm_irq);

	void towns_update_video_banks(address_space&);
	void init_serial_rom();
	void kb_sendcode(uint8_t scancode, int release);
	uint8_t speaker_get_spk();
	void speaker_set_spkrdata(uint8_t data);
	uint8_t towns_cdrom_read_byte_software();

	required_ioport m_ctrltype;
	required_ioport_array<4> m_kb_ports;
	required_ioport m_joy1;
	required_ioport m_joy2;
	required_ioport m_joy1_ex;
	required_ioport m_joy2_ex;
	required_ioport m_6b_joy1;
	required_ioport m_6b_joy2;
	required_ioport m_6b_joy1_ex;
	required_ioport m_6b_joy2_ex;
	required_ioport m_mouse1;
	required_ioport m_mouse2;
	required_ioport m_mouse3;
	required_memory_region m_user;
	optional_memory_region m_serial;

	static const device_timer_id TIMER_FREERUN = 1;
	static const device_timer_id TIMER_INTERVAL2 = 2;
	static const device_timer_id TIMER_KEYBOARD = 3;
	static const device_timer_id TIMER_MOUSE = 4;
	static const device_timer_id TIMER_WAIT = 5;
	static const device_timer_id TIMER_CDSTATUS = 6;
	static const device_timer_id TIMER_CDDA = 7;
	void freerun_inc();
	void intervaltimer2_timeout();
	void poll_keyboard();
	void mouse_timeout();
	void wait_end();
	void towns_cd_set_status(uint8_t st0, uint8_t st1, uint8_t st2, uint8_t st3);
	void towns_cdrom_execute_command(cdrom_image_device* device);
	void towns_cdrom_play_cdda(cdrom_image_device* device);
	void towns_cdrom_read(cdrom_image_device* device);
	void towns_cd_status_ready();
	void towns_delay_cdda(cdrom_image_device* dev);

	u8 m_rtc_d;
	bool m_rtc_busy;
	u8 m_vram_mask[4];
	u8 m_vram_mask_addr;

	TIMER_CALLBACK_MEMBER(towns_cdrom_read_byte);
	TIMER_CALLBACK_MEMBER(towns_sprite_done);
	TIMER_CALLBACK_MEMBER(towns_vblank_end);
	DECLARE_WRITE_LINE_MEMBER(towns_scsi_irq);
	DECLARE_WRITE_LINE_MEMBER(towns_scsi_drq);
	DECLARE_WRITE_LINE_MEMBER(towns_pit_out0_changed);
	DECLARE_WRITE_LINE_MEMBER(towns_pit_out1_changed);
	DECLARE_WRITE_LINE_MEMBER(pit2_out1_changed);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE_LINE_MEMBER(towns_fm_irq);
	DECLARE_FLOPPY_FORMATS(floppy_formats);
	void towns_crtc_refresh_mode();
	void towns_update_kanji_offset();
	void towns_update_palette();
	void render_sprite_4(uint32_t poffset, uint32_t coffset, uint16_t x, uint16_t y, bool xflip, bool yflip, bool xhalfsize, bool yhalfsize, bool rotation, const rectangle* rect);
	void render_sprite_16(uint32_t poffset, uint16_t x, uint16_t y, bool xflip, bool yflip, bool xhalfsize, bool yhalfsize, bool rotation, const rectangle* rect);
	void draw_sprites(const rectangle* rect);
	void towns_crtc_draw_scan_layer_hicolour(bitmap_rgb32 &bitmap,const rectangle* rect,int layer,int line,int scanline);
	void towns_crtc_draw_scan_layer_256(bitmap_rgb32 &bitmap,const rectangle* rect,int line,int scanline);
	void towns_crtc_draw_scan_layer_16(bitmap_rgb32 &bitmap,const rectangle* rect,int layer,int line,int scanline);
	void towns_crtc_draw_layer(bitmap_rgb32 &bitmap,const rectangle* rect,int layer);
	void render_text_char(uint8_t x, uint8_t y, uint8_t ascii, uint16_t jis, uint8_t attr);
	void draw_text_layer();
	inline uint8_t byte_to_bcd(uint8_t val);
	inline uint8_t bcd_to_byte(uint8_t val);
	inline uint32_t msf_to_lbafm(uint32_t val);  // because the CDROM core doesn't provide this;
	DECLARE_READ16_MEMBER(towns_fdc_dma_r);
	DECLARE_WRITE16_MEMBER(towns_fdc_dma_w);
	void towns_cdrom_set_irq(int line,int state);
	uint8_t towns_cd_get_track();
	DECLARE_READ16_MEMBER(towns_cdrom_dma_r);
	DECLARE_READ16_MEMBER(towns_scsi_dma_r);
	DECLARE_WRITE16_MEMBER(towns_scsi_dma_w);
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

	virtual void driver_start() override;
	void marty(machine_config &config);
};

#endif // MAME_INCLUDES_FMTOWNS_H
