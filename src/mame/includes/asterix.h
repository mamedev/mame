// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************

    Asterix

*************************************************************************/
#include "video/k053251.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053244_k053245.h"
#include "video/kvideodac.h"

class asterix_state : public driver_device
{
public:
	enum
	{
		TIMER_NMI
	};

	asterix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_tilemap(*this, "tilemap"),
		m_sprites(*this, "sprites"),
		m_mixer(*this, "mixer"),
		m_videodac(*this, "videodac") { }

	/* video-related */
	int         m_sprite_colorbase;
	int         m_layer_colorbase[4];
	int         m_layerpri[3];
	uint16_t      m_spritebank;
	int         m_tilebanks[4];
	int         m_spritebanks[4];

	/* misc */
	uint8_t       m_cur_control2;
	uint16_t      m_prot[2];

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k054156_054157_device> m_tilemap;
	required_device<k05324x_device> m_sprites;
	required_device<k053251_device> m_mixer;
	required_device<kvideodac_device> m_videodac;
	DECLARE_WRITE16_MEMBER(control2_w);
	DECLARE_WRITE8_MEMBER(sound_arm_nmi_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_WRITE16_MEMBER(protection_w);
	DECLARE_WRITE16_MEMBER(asterix_spritebank_w);
	DECLARE_DRIVER_INIT(asterix);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	INTERRUPT_GEN_MEMBER(asterix_interrupt);
	void reset_spritebank();


	void videodac_update(bitmap_ind16 **bitmaps, const rectangle &cliprect);
	void mixer_init(bitmap_ind16 **bitmaps);
	void mixer_update(bitmap_ind16 **bitmaps, const rectangle &cliprect);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
