// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#ifndef FMTOWNS_H_
#define FMTOWNS_H_


#include "emu.h"
#include "cpu/i386/i386.h"
#include "sound/2612intf.h"
#include "sound/rf5c68.h"
#include "sound/cdda.h"
#include "sound/speaker.h"
#include "imagedev/chd_cd.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "formats/fmtowns_dsk.h"
#include "machine/upd71071.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "machine/fm_scsi.h"

#define IRQ_LOG 0  // set to 1 to log IRQ line activity

struct towns_cdrom_controller
{
	UINT8 command;
	UINT8 status;
	UINT8 cmd_status[4];
	UINT8 cmd_status_ptr;
	UINT8 extra_status;
	UINT8 parameter[8];
	UINT8 mpu_irq_enable;
	UINT8 dma_irq_enable;
	UINT8 buffer[2048];
	INT32 buffer_ptr;
	UINT32 lba_current;
	UINT32 lba_last;
	UINT32 cdda_current;
	UINT32 cdda_length;
	bool software_tx;
	emu_timer* read_timer;
};

struct towns_video_controller
{
	UINT8 towns_vram_wplane;
	UINT8 towns_vram_rplane;
	UINT8 towns_vram_page_sel;
	UINT8 towns_palette_select;
	UINT8 towns_palette_r[256];
	UINT8 towns_palette_g[256];
	UINT8 towns_palette_b[256];
	UINT8 towns_degipal[8];
	UINT8 towns_dpmd_flag;
	UINT8 towns_crtc_mix;
	UINT8 towns_crtc_sel;  // selected CRTC register
	UINT16 towns_crtc_reg[32];
	UINT8 towns_video_sel;  // selected video register
	UINT8 towns_video_reg[2];
	UINT8 towns_sprite_sel;  // selected sprite register
	UINT8 towns_sprite_reg[8];
	UINT8 towns_sprite_flag;  // sprite drawing flag
	UINT8 towns_sprite_page;  // VRAM page (not layer) sprites are drawn to
	UINT8 towns_tvram_enable;
	UINT16 towns_kanji_offset;
	UINT8 towns_kanji_code_h;
	UINT8 towns_kanji_code_l;
	rectangle towns_crtc_layerscr[2];  // each layer has independent sizes
	UINT8 towns_display_plane;
	UINT8 towns_display_page_sel;
	UINT8 towns_vblank_flag;
	UINT8 towns_layer_ctrl;
	emu_timer* sprite_timer;
};

class towns_state : public driver_device
{
	public:
	towns_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_speaker(*this, "speaker"),
			m_pic_master(*this, "pic8259_master"),
			m_pic_slave(*this, "pic8259_slave"),
			m_pit(*this, "pit"),
			m_dma_1(*this, "dma_1"),
			m_dma_2(*this, "dma_2"),
			m_palette(*this, "palette"),
			m_ram(*this, RAM_TAG),
			m_fdc(*this, "fdc"),
			m_flop0(*this, "fdc:0"),
			m_flop1(*this, "fdc:1"),
			m_nvram(*this, "nvram"),
			m_nvram16(*this, "nvram16"),
			m_ctrltype(*this, "ctrltype"),
			m_key1(*this, "key1"),
			m_key2(*this, "key2"),
			m_key3(*this, "key3"),
			m_key4(*this, "key4"),
			m_joy1(*this, "joy1"),
			m_joy2(*this, "joy2"),
			m_joy1_ex(*this, "joy1_ex"),
			m_joy2_ex(*this, "joy2_ex"),
			m_6b_joy1(*this, "6b_joy1"),
			m_6b_joy2(*this, "6b_joy2"),
			m_6b_joy1_ex(*this, "6b_joy1_ex"),
			m_6b_joy2_ex(*this, "6b_joy2_ex"),
			m_mouse1(*this, "mouse1"),
			m_mouse2(*this, "mouse2"),
			m_mouse3(*this, "mouse3"),
			m_user(*this,"user"),
			m_serial(*this,"serial")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<pic8259_device> m_pic_master;
	required_device<pic8259_device> m_pic_slave;
	required_device<pit8253_device> m_pit;
	required_device<upd71071_device> m_dma_1;
	required_device<upd71071_device> m_dma_2;
	required_device<palette_device> m_palette;
	required_device<ram_device> m_ram;
	required_device<mb8877_t> m_fdc;
	required_device<floppy_connector> m_flop0;
	required_device<floppy_connector> m_flop1;
	ram_device* m_messram;
	cdrom_image_device* m_cdrom;
	cdda_device* m_cdda;
	class fmscsi_device* m_scsi;

	UINT16 m_ftimer;
	UINT16 m_freerun_timer;
	emu_timer* m_towns_freerun_counter;
	UINT16 m_intervaltimer2_period;
	UINT8 m_intervaltimer2_irqmask;
	UINT8 m_intervaltimer2_timeout_flag;
	UINT8 m_intervaltimer2_timeout_flag2;
	emu_timer* m_towns_intervaltimer2;
	UINT8 m_nmi_mask;
	UINT8 m_compat_mode;
	UINT8 m_towns_system_port;
	UINT32 m_towns_ankcg_enable;
	UINT32 m_towns_mainmem_enable;
	UINT32 m_towns_ram_enable;
	std::unique_ptr<UINT32[]> m_towns_vram;
	std::unique_ptr<UINT8[]> m_towns_gfxvram;
	std::unique_ptr<UINT8[]> m_towns_txtvram;
	int m_towns_selected_drive;
	UINT8 m_towns_fdc_irq6mask;
	std::unique_ptr<UINT8[]> m_towns_serial_rom;
	int m_towns_srom_position;
	UINT8 m_towns_srom_clk;
	UINT8 m_towns_srom_reset;
	UINT8 m_towns_rtc_select;
	UINT8 m_towns_rtc_data;
	UINT8 m_towns_rtc_reg[16];
	emu_timer* m_towns_rtc_timer;
	UINT8 m_towns_timer_mask;
	UINT16 m_towns_machine_id;  // default is 0x0101
	UINT8 m_towns_kb_status;
	UINT8 m_towns_kb_irq1_enable;
	UINT8 m_towns_kb_output;  // key output
	UINT8 m_towns_kb_extend;  // extended key output
	emu_timer* m_towns_kb_timer;
	emu_timer* m_towns_mouse_timer;
	UINT8 m_towns_fm_irq_flag;
	UINT8 m_towns_pcm_irq_flag;
	UINT8 m_towns_pcm_channel_flag;
	UINT8 m_towns_pcm_channel_mask;
	UINT8 m_towns_pad_mask;
	UINT8 m_towns_mouse_output;
	UINT8 m_towns_mouse_x;
	UINT8 m_towns_mouse_y;
	UINT8 m_towns_volume[8];  // volume ports
	UINT8 m_towns_volume_select;
	UINT8 m_towns_scsi_control;
	UINT8 m_towns_scsi_status;
	UINT8 m_towns_spkrdata;
	UINT8 m_pit_out0;
	UINT8 m_pit_out1;
	UINT8 m_pit_out2;
	UINT8 m_timer0;
	UINT8 m_timer1;

	emu_timer* m_towns_wait_timer;
	emu_timer* m_towns_status_timer;
	emu_timer* m_towns_cdda_timer;
	struct towns_cdrom_controller m_towns_cd;
	struct towns_video_controller m_video;

	UINT32 m_kb_prev[4];
	UINT8 m_prev_pad_mask;
	UINT8 m_prev_x;
	UINT8 m_prev_y;

	optional_shared_ptr<UINT32> m_nvram;
	optional_shared_ptr<UINT16> m_nvram16;

	virtual void driver_start() override;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	DECLARE_READ8_MEMBER(towns_system_r);
	DECLARE_WRITE8_MEMBER(towns_system_w);
	DECLARE_READ8_MEMBER(towns_intervaltimer2_r);
	DECLARE_WRITE8_MEMBER(towns_intervaltimer2_w);
	DECLARE_READ8_MEMBER(towns_sys6c_r);
	DECLARE_WRITE8_MEMBER(towns_sys6c_w);
	DECLARE_READ8_MEMBER(towns_dma1_r);
	DECLARE_WRITE8_MEMBER(towns_dma1_w);
	DECLARE_READ8_MEMBER(towns_dma2_r);
	DECLARE_WRITE8_MEMBER(towns_dma2_w);
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
	DECLARE_READ8_MEMBER(towns_cmos8_r);
	DECLARE_WRITE8_MEMBER(towns_cmos8_w);
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
	DECLARE_READ8_MEMBER(towns_41ff_r);

	DECLARE_READ8_MEMBER(towns_gfx_high_r);
	DECLARE_WRITE8_MEMBER(towns_gfx_high_w);
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

	RF5C68_SAMPLE_END_CB_MEMBER(towns_pcm_irq);

	void towns_update_video_banks(address_space&);
	void init_serial_rom();
	void init_rtc();
	void kb_sendcode(UINT8 scancode, int release);
	UINT8 speaker_get_spk();
	void speaker_set_spkrdata(UINT8 data);
	void speaker_set_input(UINT8 data);
	UINT8 towns_cdrom_read_byte_software();

	required_ioport m_ctrltype;
	required_ioport m_key1;
	required_ioport m_key2;
	required_ioport m_key3;
	required_ioport m_key4;
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

private:
	static const device_timer_id TIMER_RTC = 0;
	static const device_timer_id TIMER_FREERUN = 1;
	static const device_timer_id TIMER_INTERVAL2 = 2;
	static const device_timer_id TIMER_KEYBOARD = 3;
	static const device_timer_id TIMER_MOUSE = 4;
	static const device_timer_id TIMER_WAIT = 5;
	static const device_timer_id TIMER_CDSTATUS = 6;
	static const device_timer_id TIMER_CDDA = 7;
	void rtc_second();
	void freerun_inc();
	void intervaltimer2_timeout();
	void poll_keyboard();
	void mouse_timeout();
	void wait_end();
	void towns_cd_set_status(UINT8 st0, UINT8 st1, UINT8 st2, UINT8 st3);
	void towns_cdrom_execute_command(cdrom_image_device* device);
	void towns_cdrom_play_cdda(cdrom_image_device* device);
	void towns_cdrom_read(cdrom_image_device* device);
	void towns_cd_status_ready();
	void towns_delay_cdda(cdrom_image_device* dev);
public:
	INTERRUPT_GEN_MEMBER(towns_vsync_irq);
	TIMER_CALLBACK_MEMBER(towns_cdrom_read_byte);
	TIMER_CALLBACK_MEMBER(towns_sprite_done);
	TIMER_CALLBACK_MEMBER(towns_vblank_end);
	DECLARE_WRITE_LINE_MEMBER(towns_scsi_irq);
	DECLARE_WRITE_LINE_MEMBER(towns_scsi_drq);
	DECLARE_WRITE_LINE_MEMBER(towns_pit_out0_changed);
	DECLARE_WRITE_LINE_MEMBER(towns_pit_out1_changed);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE_LINE_MEMBER(towns_fm_irq);
	DECLARE_FLOPPY_FORMATS(floppy_formats);
	void towns_crtc_refresh_mode();
	void towns_update_kanji_offset();
	void towns_update_palette();
	void render_sprite_4(UINT32 poffset, UINT32 coffset, UINT16 x, UINT16 y, UINT16 xflip, UINT16 yflip, const rectangle* rect);
	void render_sprite_16(UINT32 poffset, UINT16 x, UINT16 y, UINT16 xflip, UINT16 yflip, const rectangle* rect);
	void draw_sprites(const rectangle* rect);
	void towns_crtc_draw_scan_layer_hicolour(bitmap_rgb32 &bitmap,const rectangle* rect,int layer,int line,int scanline);
	void towns_crtc_draw_scan_layer_256(bitmap_rgb32 &bitmap,const rectangle* rect,int layer,int line,int scanline);
	void towns_crtc_draw_scan_layer_16(bitmap_rgb32 &bitmap,const rectangle* rect,int layer,int line,int scanline);
	void towns_crtc_draw_layer(bitmap_rgb32 &bitmap,const rectangle* rect,int layer);
	void render_text_char(UINT8 x, UINT8 y, UINT8 ascii, UINT16 jis, UINT8 attr);
	void draw_text_layer();
	inline UINT8 byte_to_bcd(UINT8 val);
	inline UINT8 bcd_to_byte(UINT8 val);
	inline UINT32 msf_to_lbafm(UINT32 val);  // because the CDROM core doesn't provide this;
	DECLARE_READ16_MEMBER(towns_fdc_dma_r);
	DECLARE_WRITE16_MEMBER(towns_fdc_dma_w);
	void towns_cdrom_set_irq(int line,int state);
	UINT8 towns_cd_get_track();
	DECLARE_READ16_MEMBER(towns_cdrom_dma_r);
	void rtc_hour();
	void rtc_minute();
	DECLARE_READ16_MEMBER(towns_scsi_dma_r);
	DECLARE_WRITE16_MEMBER(towns_scsi_dma_w);
};

class towns16_state : public towns_state
{
	public:
	towns16_state(const machine_config &mconfig, device_type type, const char *tag)
		: towns_state(mconfig, type, tag)
	{ }
};

class marty_state : public towns_state
{
	public:
	marty_state(const machine_config &mconfig, device_type type, const char *tag)
		: towns_state(mconfig, type, tag)
	{ }

	virtual void driver_start() override;
};

#endif /*FMTOWNS_H_*/
