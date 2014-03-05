/*************************************************************************

    Vendetta

*************************************************************************/
#include "sound/k053260.h"
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
		m_k053260(*this, "k053260"),
		m_k052109(*this, "k052109"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_k053252(*this, "k053252"),
		m_k054000(*this, "k054000"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	UINT8 *    m_ram;
	dynamic_array<UINT8> m_paletteram;

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
	required_device<k053260_device> m_k053260;
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
	DECLARE_READ8_MEMBER(vendetta_sound_interrupt_r);
	DECLARE_READ8_MEMBER(vendetta_sound_r);
	DECLARE_DRIVER_INIT(vendetta);
	DECLARE_DRIVER_INIT(esckids);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_vendetta(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vendetta_irq);
	void vendetta_video_banking( int select );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};

/*----------- defined in video/vendetta.c -----------*/

extern void vendetta_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void esckids_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void vendetta_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
