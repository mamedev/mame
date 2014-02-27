/*************************************************************************

    88 Games

*************************************************************************/
#include "sound/upd7759.h"
#include "video/k052109.h"
#include "video/k051960.h"
#include "video/k051316.h"
#include "video/konami_helper.h"

class _88games_state : public driver_device
{
public:
	_88games_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_banked_rom(*this, "banked_rom"),
		m_paletteram_1000(*this, "palette"),
		m_ram(*this, "ram"),
		m_audiocpu(*this, "audiocpu"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_k051316(*this, "k051316"),
		m_upd7759_1(*this, "upd1"),
		m_upd7759_2(*this, "upd2"),
		m_maincpu(*this, "maincpu") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_banked_rom;
	required_shared_ptr<UINT8> m_paletteram_1000;
	required_shared_ptr<UINT8> m_ram;
//  UINT8 *      m_paletteram_1000;   // this currently uses generic palette handling
//  UINT8 *      m_nvram; // this currently uses generic nvram handling

	/* video-related */
	int          m_k88games_priority;
	int          m_layer_colorbase[3];
	int          m_sprite_colorbase;
	int          m_zoom_colorbase;
	int          m_videobank;
	int          m_zoomreadroms;
	int          m_speech_chip;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<k051316_device> m_k051316;
	required_device<upd7759_device> m_upd7759_1;
	required_device<upd7759_device> m_upd7759_2;
	DECLARE_READ8_MEMBER(bankedram_r);
	DECLARE_WRITE8_MEMBER(bankedram_w);
	DECLARE_WRITE8_MEMBER(k88games_5f84_w);
	DECLARE_WRITE8_MEMBER(k88games_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(speech_control_w);
	DECLARE_WRITE8_MEMBER(speech_msg_w);
	DECLARE_READ8_MEMBER(k052109_051960_r);
	DECLARE_WRITE8_MEMBER(k052109_051960_w);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_88games(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(k88games_interrupt);
	required_device<cpu_device> m_maincpu;
};

/*----------- defined in video/88games.c -----------*/
void _88games_sprite_callback(running_machine &machine, int *code, int *color, int *priority, int *shadow);
void _88games_tile_callback(running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority);
void _88games_zoom_callback(running_machine &machine, int *code, int *color, int *flags);
