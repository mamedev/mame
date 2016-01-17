// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Parodius

*************************************************************************/

#include "machine/bankdev.h"
#include "video/k053244_k053245.h"
#include "video/k052109.h"
#include "video/k053251.h"
#include "video/konami_helper.h"

class parodius_state : public driver_device
{
public:
	enum
	{
		TIMER_NMI
	};

	parodius_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bank0000(*this, "bank0000"),
		m_bank2000(*this, "bank2000"),
		m_k052109(*this, "k052109"),
		m_k053245(*this, "k053245"),
		m_k053251(*this, "k053251") { }

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_layerpri[3];

	/* misc */
	//int        m_nmi_enabled;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<address_map_bank_device> m_bank0000;
	required_device<address_map_bank_device> m_bank2000;
	required_device<k052109_device> m_k052109;
	required_device<k05324x_device> m_k053245;
	required_device<k053251_device> m_k053251;
	DECLARE_WRITE8_MEMBER(parodius_videobank_w);
	DECLARE_WRITE8_MEMBER(parodius_3fc0_w);
	DECLARE_WRITE8_MEMBER(parodius_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(sound_arm_nmi_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_parodius(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(parodius_interrupt);
	K05324X_CB_MEMBER(sprite_callback);
	K052109_CB_MEMBER(tile_callback);
	DECLARE_WRITE8_MEMBER(banking_callback);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
