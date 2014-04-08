/*************************************************************************

    Super Contra / Thunder Cross

*************************************************************************/
#include "sound/k007232.h"
#include "video/k052109.h"
#include "video/k051960.h"
#include "video/konami_helper.h"

class thunderx_state : public driver_device
{
public:
	enum
	{
		TIMER_THUNDERX_FIRQ
	};

	thunderx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007232(*this, "k007232"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_ram;
	UINT8      m_pmcram[0x800];
	dynamic_array<UINT8> m_paletteram;

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;

	/* misc */
	int        m_priority;
	UINT8      m_1f98_data;
	int        m_palette_selected;
	int        m_rambank;
	int        m_pmcbank;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<k007232_device> m_k007232;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<palette_device> m_palette;

	DECLARE_READ8_MEMBER(scontra_bankedram_r);
	DECLARE_WRITE8_MEMBER(scontra_bankedram_w);
	DECLARE_READ8_MEMBER(thunderx_bankedram_r);
	DECLARE_WRITE8_MEMBER(thunderx_bankedram_w);
	DECLARE_READ8_MEMBER(thunderx_1f98_r);
	DECLARE_WRITE8_MEMBER(thunderx_1f98_w);
	DECLARE_WRITE8_MEMBER(scontra_bankswitch_w);
	DECLARE_WRITE8_MEMBER(thunderx_videobank_w);
	DECLARE_WRITE8_MEMBER(thunderx_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(k052109_051960_r);
	DECLARE_WRITE8_MEMBER(k052109_051960_w);
	DECLARE_WRITE8_MEMBER(scontra_snd_bankswitch_w);
	virtual void video_start();
	DECLARE_MACHINE_START(scontra);
	DECLARE_MACHINE_RESET(scontra);
	DECLARE_MACHINE_START(thunderx);
	DECLARE_MACHINE_RESET(thunderx);
	UINT32 screen_update_scontra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(scontra_interrupt);
	void run_collisions( int s0, int e0, int s1, int e1, int cm, int hm );
	void calculate_collisions(  );
	DECLARE_WRITE8_MEMBER(volume_callback);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};


/*----------- defined in video/thunderx.c -----------*/

extern void thunderx_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void thunderx_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask,int *shadow);
