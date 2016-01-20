// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    Vendetta

*************************************************************************/
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

	vendetta_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k052109(*this, "k052109"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_k053252(*this, "k053252"),
		m_k054000(*this, "k054000"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	std::vector<UINT8> m_paletteram;

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_layerpri[3];

	/* misc */
	int        m_irq_enabled;
	offs_t     m_video_banking_base;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k052109_device> m_k052109;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	optional_device<k053252_device> m_k053252;
	optional_device<k054000_device> m_k054000;
	required_device<palette_device> m_palette;
	DECLARE_WRITE8_MEMBER(vendetta_eeprom_w);
	DECLARE_READ8_MEMBER(vendetta_K052109_r);
	DECLARE_WRITE8_MEMBER(vendetta_K052109_w);
	DECLARE_WRITE8_MEMBER(vendetta_5fe0_w);
	DECLARE_WRITE8_MEMBER(z80_arm_nmi_w);
	DECLARE_WRITE8_MEMBER(z80_irq_w);
	DECLARE_READ8_MEMBER(z80_irq_r);
	DECLARE_DRIVER_INIT(vendetta);
	DECLARE_DRIVER_INIT(esckids);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_vendetta(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vendetta_irq);
	void vendetta_video_banking( int select );
	K052109_CB_MEMBER(vendetta_tile_callback);
	K052109_CB_MEMBER(esckids_tile_callback);
	DECLARE_WRITE8_MEMBER(banking_callback);
	K053246_CB_MEMBER(sprite_callback);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
