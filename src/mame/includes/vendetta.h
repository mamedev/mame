// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    Vendetta

*************************************************************************/
#include "machine/bankdev.h"
#include "machine/k053252.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k054000.h"
#include "video/k052109.h"
#include "video/k053251.h"
#include "video/konami_helper.h"

class vendetta_state : public driver_device
{
public:
	enum
	{
		TIMER_Z80_NMI
	};

	vendetta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k052109(*this, "k052109"),
		m_sprites(*this, "sprites"),
		m_mixer(*this, "mixer"),
		m_video_timings(*this, "video_timings"),
		m_k054000(*this, "k054000"),
		m_palette(*this, "palette"),
		m_videobank0(*this, "videobank0"),
		m_videobank1(*this, "videobank1") { }

	/* memory pointers */
	std::vector<uint8_t> m_paletteram;

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_layerpri[3];

	/* misc */
	int        m_irq_enabled;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k052109_device> m_k052109;
	required_device<k053246_053247_device> m_sprites;
	required_device<k053251_device> m_mixer;
	optional_device<k053252_device> m_video_timings;
	optional_device<k054000_device> m_k054000;
	required_device<palette_device> m_palette;

	required_device<address_map_bank_device> m_videobank0;
	required_device<address_map_bank_device> m_videobank1;

	DECLARE_WRITE8_MEMBER(eeprom_w);
	DECLARE_READ8_MEMBER(K052109_r);
	DECLARE_WRITE8_MEMBER(K052109_w);
	DECLARE_WRITE8_MEMBER(_5fe0_w);
	DECLARE_WRITE8_MEMBER(z80_arm_nmi_w);
	DECLARE_WRITE8_MEMBER(z80_irq_w);
	DECLARE_READ8_MEMBER(z80_irq_r);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	INTERRUPT_GEN_MEMBER(vendetta_irq);
	void vendetta_video_banking( int select );
	K052109_CB_MEMBER(vendetta_tile_callback);
	K052109_CB_MEMBER(esckids_tile_callback);
	DECLARE_WRITE8_MEMBER(banking_callback);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
