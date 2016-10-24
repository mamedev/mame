// license:BSD-3-Clause
// copyright-holders:R. Belmont, Nicola Salmoria
/*************************************************************************

    Lethal Enforcers

*************************************************************************/

#include "machine/bankdev.h"
#include "sound/k054539.h"
#include "video/konami_helper.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053244_k053245.h"
#include "video/k054000.h"

class lethal_state : public driver_device
{
public:
	lethal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_bank4000(*this, "bank4000"),
		m_k056832(*this, "k056832"),
		m_k053244(*this, "k053244"),
		m_palette(*this, "palette") { }

	/* video-related */
	int        m_layer_colorbase[4];
	int        m_sprite_colorbase;
	int        m_back_colorbase;

	/* misc */
	uint8_t      m_cur_control2;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<address_map_bank_device> m_bank4000;
	required_device<k056832_device> m_k056832;
	required_device<k05324x_device> m_k053244;
	required_device<palette_device> m_palette;

	void control2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_irq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void le_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t guns_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t gunsaux_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lethalen_palette_control(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_lethalen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void lethalen_interrupt(device_t &device);
	K05324X_CB_MEMBER(sprite_callback);
	K056832_CB_MEMBER(tile_callback);
};
