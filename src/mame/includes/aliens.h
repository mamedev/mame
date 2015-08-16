// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Aliens

*************************************************************************/

#include "machine/bankdev.h"
#include "sound/k007232.h"
#include "video/k052109.h"
#include "video/k051960.h"

class aliens_state : public driver_device
{
public:
	aliens_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bank0000(*this, "bank0000"),
		m_k007232(*this, "k007232"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_rombank(*this, "rombank") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<address_map_bank_device> m_bank0000;
	required_device<k007232_device> m_k007232;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_memory_bank m_rombank;

	DECLARE_WRITE8_MEMBER(aliens_coin_counter_w);
	DECLARE_WRITE8_MEMBER(aliens_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(k052109_051960_r);
	DECLARE_WRITE8_MEMBER(k052109_051960_w);
	DECLARE_WRITE8_MEMBER(aliens_snd_bankswitch_w);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_aliens(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(volume_callback);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
	DECLARE_WRITE8_MEMBER(banking_callback);
};
