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
	lethal_state(const machine_config &mconfig, device_type type, std::string tag)
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
	UINT8      m_cur_control2;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<address_map_bank_device> m_bank4000;
	required_device<k056832_device> m_k056832;
	required_device<k05324x_device> m_k053244;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(control2_w);
	DECLARE_WRITE8_MEMBER(sound_cmd_w);
	DECLARE_WRITE8_MEMBER(sound_irq_w);
	DECLARE_READ8_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(le_bankswitch_w);
	DECLARE_READ8_MEMBER(guns_r);
	DECLARE_READ8_MEMBER(gunsaux_r);
	DECLARE_WRITE8_MEMBER(lethalen_palette_control);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_lethalen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(lethalen_interrupt);
	K05324X_CB_MEMBER(sprite_callback);
	K056832_CB_MEMBER(tile_callback);
};
