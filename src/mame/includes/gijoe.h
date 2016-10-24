// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************

    GI Joe

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/k054539.h"
#include "video/k053251.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053246_k053247_k055673.h"
#include "video/konami_helper.h"

class gijoe_state : public driver_device
{
public:
	gijoe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_workram(*this, "workram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k054539(*this, "k054539"),
		m_k056832(*this, "k056832"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_workram;

	/* video-related */
	int         m_avac_bits[4];
	int         m_avac_occupancy[4];
	int         m_layer_colorbase[4];
	int         m_layer_pri[4];
	int         m_avac_vrc;
	int         m_sprite_colorbase;

	/* misc */
	uint16_t      m_cur_control2;
	emu_timer   *m_dmadelay_timer;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k054539_device> m_k054539;
	required_device<k056832_device> m_k056832;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

	uint16_t control2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void control2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_cmd_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_irq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sound_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_gijoe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gijoe_interrupt(device_t &device);
	void dmaend_callback(void *ptr, int32_t param);
	void gijoe_objdma();
	K056832_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);
};
