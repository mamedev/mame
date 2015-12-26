// license:???
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
	required_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_pixelram;

	/* video-related */
	/* framebuffer is a raw bitmap, remapped as a last step */
	std::unique_ptr<bitmap_ind16> m_framebuffer[2];
	std::unique_ptr<bitmap_ind16> m_pixel_bitmap;
	std::unique_ptr<bitmap_ind16> m_realpunc_bitmap;

	UINT16        m_pixel_scroll[2];

	int           m_b_fg_color_base;
	int           m_b_sp_color_base;

	/* misc */
	UINT16        m_eep_latch;
	UINT16        m_coin_word;

	UINT16        m_realpunc_video_ctrl;

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

	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_READ16_MEMBER(tracky1_hi_r);
	DECLARE_READ16_MEMBER(tracky1_lo_r);
	DECLARE_READ16_MEMBER(trackx1_hi_r);
	DECLARE_READ16_MEMBER(trackx1_lo_r);
	DECLARE_READ16_MEMBER(tracky2_hi_r);
	DECLARE_READ16_MEMBER(tracky2_lo_r);
	DECLARE_READ16_MEMBER(trackx2_hi_r);
	DECLARE_READ16_MEMBER(trackx2_lo_r);
	DECLARE_WRITE16_MEMBER(gain_control_w);
	DECLARE_READ16_MEMBER(eep_latch_r);
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_READ16_MEMBER(player_34_coin_ctrl_r);
	DECLARE_WRITE16_MEMBER(player_34_coin_ctrl_w);
	DECLARE_READ16_MEMBER(pbobble_input_bypass_r);
	DECLARE_WRITE16_MEMBER(spacedxo_tc0220ioc_w);
	DECLARE_WRITE16_MEMBER(realpunc_output_w);
	DECLARE_WRITE16_MEMBER(hitice_pixelram_w);
	DECLARE_WRITE16_MEMBER(hitice_pixel_scroll_w);
	DECLARE_WRITE16_MEMBER(realpunc_video_ctrl_w);
	DECLARE_READ16_MEMBER(tc0180vcu_framebuffer_word_r);
	DECLARE_WRITE16_MEMBER(tc0180vcu_framebuffer_word_w);
	DECLARE_WRITE8_MEMBER(mb87078_gain_changed);
	DECLARE_INPUT_CHANGED_MEMBER(realpunc_sensor);
	DECLARE_DRIVER_INIT(taito_b);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(taitob_color_order0);
	DECLARE_VIDEO_START(taitob_color_order1);
	DECLARE_VIDEO_START(taitob_color_order2);
	DECLARE_VIDEO_START(hitice);
	DECLARE_VIDEO_RESET(hitice);
	DECLARE_VIDEO_START(realpunc);
	DECLARE_VIDEO_START(taitob_core);
	UINT32 screen_update_taitob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_realpunc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_taitob(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(rastansaga2_interrupt);
	INTERRUPT_GEN_MEMBER(crimec_interrupt);
	INTERRUPT_GEN_MEMBER(hitice_interrupt);
	INTERRUPT_GEN_MEMBER(rambo3_interrupt);
	INTERRUPT_GEN_MEMBER(pbobble_interrupt);
	INTERRUPT_GEN_MEMBER(viofight_interrupt);
	INTERRUPT_GEN_MEMBER(masterw_interrupt);
	INTERRUPT_GEN_MEMBER(silentd_interrupt);
	INTERRUPT_GEN_MEMBER(selfeena_interrupt);
	INTERRUPT_GEN_MEMBER(sbm_interrupt);
	INTERRUPT_GEN_MEMBER(realpunc_interrupt);
	void hitice_clear_pixel_bitmap(  );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_framebuffer( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	DECLARE_WRITE_LINE_MEMBER(irqhandler);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
