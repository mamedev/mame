// license:BSD-3-Clause
// copyright-holders:Luca Elia
#include "sound/okim9810.h"
#include "machine/eepromser.h"
#include "sound/x1_010.h"
#include "machine/tmp68301.h"

class seta2_state : public driver_device
{
public:
	seta2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_tmp68301(*this, "tmp68301"),
		m_oki(*this, "oki"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_nvram(*this, "nvram"),
		m_spriteram(*this, "spriteram", 0),
		m_vregs(*this, "vregs", 0),
		m_funcube_outputs(*this, "funcube_outputs"),
		m_funcube_leds(*this, "funcube_leds") { }

	required_device<cpu_device> m_maincpu;
	optional_device<tmp68301_device> m_tmp68301;
	optional_device<okim9810_device> m_oki;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_shared_ptr<UINT16> m_nvram;
	optional_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_vregs;
	optional_shared_ptr<UINT16> m_funcube_outputs;
	optional_shared_ptr<UINT16> m_funcube_leds;

	int m_xoffset;
	int m_yoffset;
	int m_keyboard_row;
	UINT16 *m_buffered_spriteram;

	UINT64 m_funcube_coin_start_cycles;
	UINT8 m_funcube_hopper_motor;

	DECLARE_WRITE16_MEMBER(vregs_w);
	DECLARE_WRITE16_MEMBER(sound_bank_w);

	DECLARE_WRITE16_MEMBER(grdians_lockout_w);

	DECLARE_READ16_MEMBER(mj4simai_p1_r);
	DECLARE_READ16_MEMBER(mj4simai_p2_r);
	DECLARE_WRITE16_MEMBER(mj4simai_keyboard_w);

	DECLARE_READ16_MEMBER(pzlbowl_protection_r);
	DECLARE_READ16_MEMBER(pzlbowl_coins_r);
	DECLARE_WRITE16_MEMBER(pzlbowl_coin_counter_w);

	DECLARE_WRITE16_MEMBER(reelquak_leds_w);
	DECLARE_WRITE16_MEMBER(reelquak_coin_w);

	DECLARE_WRITE16_MEMBER(samshoot_coin_w);

	DECLARE_READ16_MEMBER(gundamex_eeprom_r);
	DECLARE_WRITE16_MEMBER(gundamex_eeprom_w);

	DECLARE_READ32_MEMBER(funcube_nvram_dword_r);
	DECLARE_WRITE32_MEMBER(funcube_nvram_dword_w);
	DECLARE_WRITE16_MEMBER(spriteram16_word_w);
	DECLARE_READ16_MEMBER(spriteram16_word_r);
	DECLARE_READ32_MEMBER(funcube_debug_r);
	DECLARE_READ16_MEMBER(funcube_coins_r);
	DECLARE_WRITE16_MEMBER(funcube_leds_w);
	DECLARE_READ16_MEMBER(funcube_outputs_r);
	DECLARE_WRITE16_MEMBER(funcube_outputs_w);
	DECLARE_READ16_MEMBER(funcube_battery_r);
	DECLARE_READ32_MEMBER(oki_read);
	DECLARE_WRITE32_MEMBER(oki_write);

	DECLARE_DRIVER_INIT(funcube3);
	DECLARE_DRIVER_INIT(funcube);
	DECLARE_DRIVER_INIT(funcube2);
	virtual void video_start() override;
	DECLARE_MACHINE_START(mj4simai);
	DECLARE_MACHINE_START(funcube);
	DECLARE_MACHINE_RESET(funcube);
	DECLARE_VIDEO_START(yoffset);
	DECLARE_VIDEO_START(xoffset);

	INTERRUPT_GEN_MEMBER(seta2_interrupt);
	INTERRUPT_GEN_MEMBER(samshoot_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(funcube_interrupt);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void funcube_debug_outputs();
};
