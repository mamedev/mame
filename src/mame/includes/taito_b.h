// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
#include "machine/mb87078.h"
#include "machine/taitoio.h"
#include "video/hd63484.h"
#include "video/tc0180vcu.h"

class taitob_state : public driver_device
{
public:
	enum
	{
		RSAGA2_INTERRUPT2,
		CRIMEC_INTERRUPT3,
		HITICE_INTERRUPT6,
		RAMBO3_INTERRUPT1,
		PBOBBLE_INTERRUPT5,
		VIOFIGHT_INTERRUPT1,
		MASTERW_INTERRUPT4,
		SILENTD_INTERRUPT4,
		SELFEENA_INTERRUPT4,
		SBM_INTERRUPT5,
		REALPUNC_INTERRUPT3
	};

	taitob_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_pixelram(*this, "pixelram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_hd63484(*this, "hd63484"),
		m_tc0180vcu(*this, "tc0180vcu"),
		m_tc0640fio(*this, "tc0640fio"),
		m_tc0220ioc(*this, "tc0220ioc"),
		m_tc0510nio(*this, "tc0510nio"),
		m_mb87078(*this, "mb87078"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_pixelram;

	/* video-related */
	/* framebuffer is a raw bitmap, remapped as a last step */
	std::unique_ptr<bitmap_ind16> m_framebuffer[2];
	std::unique_ptr<bitmap_ind16> m_pixel_bitmap;
	std::unique_ptr<bitmap_ind16> m_realpunc_bitmap;

	uint16_t        m_pixel_scroll[2];

	int           m_b_fg_color_base;
	int           m_b_sp_color_base;

	/* misc */
	uint16_t        m_eep_latch;
	uint16_t        m_coin_word;

	uint16_t        m_realpunc_video_ctrl;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	device_t *m_ym;
	optional_device<hd63484_device> m_hd63484;
	required_device<tc0180vcu_device> m_tc0180vcu;
	optional_device<tc0640fio_device> m_tc0640fio;
	optional_device<tc0220ioc_device> m_tc0220ioc;
	optional_device<tc0510nio_device> m_tc0510nio;
	optional_device<mb87078_device> m_mb87078;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t tracky1_hi_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t tracky1_lo_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t trackx1_hi_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t trackx1_lo_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t tracky2_hi_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t tracky2_lo_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t trackx2_hi_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t trackx2_lo_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void gain_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t eep_latch_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t player_34_coin_ctrl_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void player_34_coin_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t pbobble_input_bypass_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void spacedxo_tc0220ioc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void realpunc_output_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hitice_pixelram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hitice_pixel_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void realpunc_video_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t tc0180vcu_framebuffer_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tc0180vcu_framebuffer_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mb87078_gain_changed(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void realpunc_sensor(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void init_taito_b();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void video_start_taitob_color_order0();
	void video_start_taitob_color_order1();
	void video_start_taitob_color_order2();
	void video_start_hitice();
	void video_reset_hitice();
	void video_start_realpunc();
	void video_start_taitob_core();
	uint32_t screen_update_taitob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_realpunc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_taitob(screen_device &screen, bool state);
	void rastansaga2_interrupt(device_t &device);
	void crimec_interrupt(device_t &device);
	void hitice_interrupt(device_t &device);
	void rambo3_interrupt(device_t &device);
	void pbobble_interrupt(device_t &device);
	void viofight_interrupt(device_t &device);
	void masterw_interrupt(device_t &device);
	void silentd_interrupt(device_t &device);
	void selfeena_interrupt(device_t &device);
	void sbm_interrupt(device_t &device);
	void realpunc_interrupt(device_t &device);
	void hitice_clear_pixel_bitmap(  );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_framebuffer( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
