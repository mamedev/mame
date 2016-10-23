// license:BSD-3-Clause
// copyright-holders:Manuel Abadia

#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/k007232.h"
#include "video/k052109.h"
#include "video/k051960.h"
#include "video/k051316.h"
#include "video/konami_helper.h"

class ajax_state : public driver_device
{
public:
	ajax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_watchdog(*this, "watchdog"),
		m_k007232_1(*this, "k007232_1"),
		m_k007232_2(*this, "k007232_2"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_k051316(*this, "k051316"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* video-related */
	uint8_t      m_priority;

	/* misc */
	int        m_firq_enable;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<k007232_device> m_k007232_1;
	required_device<k007232_device> m_k007232_2;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<k051316_device> m_k051316;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void sound_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ls138_f10_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ls138_f10_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamps_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void k007232_extvol_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void volume_callback0(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void volume_callback1(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	K051316_CB_MEMBER(zoom_callback);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
};
