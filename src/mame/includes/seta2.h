// license:BSD-3-Clause
// copyright-holders:Luca Elia

#include "machine/tmp68301.h"
#include "machine/eepromser.h"
#include "machine/intelfsh.h"
#include "machine/upd4992.h"
#include "sound/okim9810.h"
#include "sound/x1_010.h"

class seta2_state : public driver_device
{
public:
	seta2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_tmp68301(*this, "tmp68301"),
		m_oki(*this, "oki"),
		m_eeprom(*this, "eeprom"),
		m_flash(*this, "flash"),
		m_rtc(*this, "rtc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_nvram(*this, "nvram"),
		m_spriteram(*this, "spriteram", 0),
		m_tileram(*this, "tileram", 0),
		m_rgbram(*this, "rgbram", 0),
		m_vregs(*this, "vregs", 0),
		m_funcube_outputs(*this, "funcube_outputs"),
		m_funcube_leds(*this, "funcube_leds")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<tmp68301_device> m_tmp68301;
	optional_device<okim9810_device> m_oki;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<intelfsh16_device> m_flash;
	optional_device<upd4992_device> m_rtc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint16_t> m_nvram;
	optional_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_tileram;
	optional_shared_ptr<uint16_t> m_rgbram;
	optional_shared_ptr<uint16_t> m_vregs;
	optional_shared_ptr<uint16_t> m_funcube_outputs;
	optional_shared_ptr<uint16_t> m_funcube_leds;

	int m_xoffset;
	int m_yoffset;
	int m_keyboard_row;
	std::unique_ptr<uint16_t[]> m_buffered_spriteram;

	uint64_t m_funcube_coin_start_cycles;
	uint8_t m_funcube_hopper_motor;

	uint16_t m_lamps1, m_lamps2, m_cam;

	void spriteram16_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t spriteram16_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void vregs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t oki_read(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void oki_write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void sound_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void grdians_lockout_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t mj4simai_p1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t mj4simai_p2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mj4simai_keyboard_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t pzlbowl_protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t pzlbowl_coins_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pzlbowl_coin_counter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void reelquak_leds_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void reelquak_coin_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void samshoot_coin_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t gundamex_eeprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void gundamex_eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint32_t funcube_nvram_dword_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void funcube_nvram_dword_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t funcube_debug_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint16_t funcube_coins_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void funcube_leds_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t funcube_outputs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void funcube_outputs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t funcube_battery_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void staraudi_camera_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void staraudi_lamps1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void staraudi_lamps2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t staraudi_tileram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void staraudi_tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void init_funcube3();
	void init_funcube();
	void init_funcube2();
	void init_staraudi();

	void machine_start_mj4simai();
	void machine_start_funcube();
	void machine_reset_funcube();

	virtual void video_start() override;
	void video_start_yoffset();
	void video_start_xoffset();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t staraudi_screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void draw_rgbram(bitmap_ind16 &bitmap);

	void seta2_interrupt(device_t &device);
	void samshoot_interrupt(device_t &device);
	void funcube_interrupt(timer_device &timer, void *ptr, int32_t param);

	void funcube_debug_outputs();
	void staraudi_debug_outputs();
};
