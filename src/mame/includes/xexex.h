// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************

    Xexex

*************************************************************************/

#include "video/k053250.h"
#include "sound/flt_vol.h"
#include "sound/k054539.h"
#include "machine/gen_latch.h"
#include "machine/k053252.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k054338.h"
#include "video/k053251.h"
#include "video/konami_helper.h"

class xexex_state : public driver_device
{
public:
	xexex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k054539(*this, "k054539"),
		m_filter1l(*this, "filter1l"),
		m_filter1r(*this, "filter1r"),
		m_filter2l(*this, "filter2l"),
		m_filter2r(*this, "filter2r"),
		m_k056832(*this, "k056832"),
		m_k053246(*this, "k053246"),
		m_k053250(*this, "k053250"),
		m_k053251(*this, "k053251"),
		m_k053252(*this, "k053252"),
		m_k054338(*this, "k054338"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_soundlatch3(*this, "soundlatch3") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_workram;
	required_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	int        m_layer_colorbase[4];
	int        m_sprite_colorbase;
	int        m_layerpri[4];
	int        m_cur_alpha;

	/* misc */
	uint16_t     m_cur_control2;
	int32_t      m_strip_0x1a;
	int        m_suspension_active;
	int        m_resume_trigger;
	emu_timer  *m_dmadelay_timer;
	int        m_frame;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k054539_device> m_k054539;
	required_device<filter_volume_device> m_filter1l;
	required_device<filter_volume_device> m_filter1r;
	required_device<filter_volume_device> m_filter2l;
	required_device<filter_volume_device> m_filter2r;
	required_device<k056832_device> m_k056832;
	required_device<k053247_device> m_k053246;
	required_device<k053250_device> m_k053250;
	required_device<k053251_device> m_k053251;
	required_device<k053252_device> m_k053252;
	required_device<k054338_device> m_k054338;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_device<generic_latch_8_device> m_soundlatch3;

	uint16_t spriteram_mirror_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void spriteram_mirror_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t xexex_waitskip_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t control2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void control2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_cmd1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_cmd2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_irq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sound_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_xexex();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_xexex(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void dmaend_callback(void *ptr, int32_t param);
	void xexex_interrupt(timer_device &timer, void *ptr, int32_t param);
	void xexex_postload();
	void xexex_objdma( int limiter );
	void parse_control2(  );
	K056832_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);
	K054539_CB_MEMBER(ym_set_mixing);
};
