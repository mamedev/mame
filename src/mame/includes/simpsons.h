// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "machine/bankdev.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k052109.h"
#include "video/k053251.h"
#include "video/konami_helper.h"

class simpsons_state : public driver_device
{
public:
	enum
	{
		TIMER_NMI,
		TIMER_DMAEND
	};

	simpsons_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bank0000(*this, "bank0000"),
		m_bank2000(*this, "bank2000"),
		m_k052109(*this, "k052109"),
		m_sprites(*this, "sprites"),
		m_mixer(*this, "mixer") { }

	/* memory pointers */
	std::unique_ptr<uint16_t[]>   m_spriteram;

	/* video-related */
	int        m_sprite_colorbase;
	int        m_layer_colorbase[3];
	int        m_layerpri[3];

	/* misc */
	int        m_firq_enabled;
	//int        m_nmi_enabled;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<address_map_bank_device> m_bank0000;
	required_device<address_map_bank_device> m_bank2000;
	required_device<k052109_device> m_k052109;
	required_device<k053246_053247_device> m_sprites;
	required_device<k053251_device> m_mixer;
	DECLARE_WRITE8_MEMBER(z80_bankswitch_w);
	DECLARE_WRITE8_MEMBER(z80_arm_nmi_w);
	DECLARE_WRITE8_MEMBER(simpsons_eeprom_w);
	DECLARE_WRITE8_MEMBER(simpsons_coin_counter_w);
	DECLARE_READ8_MEMBER(simpsons_sound_interrupt_r);
	DECLARE_READ8_MEMBER(simpsons_k052109_r);
	DECLARE_WRITE8_MEMBER(simpsons_k052109_w);
	DECLARE_READ8_MEMBER(simpsons_k053247_r);
	DECLARE_WRITE8_MEMBER(simpsons_k053247_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_simpsons(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(simpsons_irq);
	void simpsons_video_banking(int bank);
	K052109_CB_MEMBER(tile_callback);
	DECLARE_WRITE8_MEMBER(banking_callback);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
