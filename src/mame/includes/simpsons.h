#include "sound/k053260.h"
#include "video/k05324x.h"

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
		m_k053260(*this, "k053260"),
		m_k052109(*this, "k052109"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251") { }

	/* memory pointers */
	UINT8 *    m_ram;
	UINT8 *    m_xtraram;
	UINT16 *   m_spriteram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_sprite_colorbase;
	int        m_layer_colorbase[3];
	int        m_layerpri[3];

	/* misc */
	int        m_firq_enabled;
	int        m_video_bank;
	//int        m_nmi_enabled;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k053260_device> m_k053260;
	required_device<k052109_device> m_k052109;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	DECLARE_WRITE8_MEMBER(z80_bankswitch_w);
	DECLARE_WRITE8_MEMBER(z80_arm_nmi_w);
	DECLARE_WRITE8_MEMBER(simpsons_eeprom_w);
	DECLARE_WRITE8_MEMBER(simpsons_coin_counter_w);
	DECLARE_READ8_MEMBER(simpsons_sound_interrupt_r);
	DECLARE_READ8_MEMBER(simpsons_k052109_r);
	DECLARE_WRITE8_MEMBER(simpsons_k052109_w);
	DECLARE_READ8_MEMBER(simpsons_k053247_r);
	DECLARE_WRITE8_MEMBER(simpsons_k053247_w);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_simpsons(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(simpsons_irq);
	TIMER_CALLBACK_MEMBER(nmi_callback);
	TIMER_CALLBACK_MEMBER(dmaend_callback);
	DECLARE_READ8_MEMBER(simpsons_sound_r);
	void simpsons_postload();
	void simpsons_video_banking( int bank );
	void sound_nmi_callback( int param );
	void simpsons_objdma(  );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};


/*----------- defined in video/simpsons.c -----------*/
extern void simpsons_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void simpsons_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
