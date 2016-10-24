// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
#include "machine/eepromser.h"
#include "machine/watchdog.h"
#include "video/tc0480scp.h"

struct gb_tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class gunbustr_state : public driver_device
{
public:
	enum
	{
		TIMER_GUNBUSTR_INTERRUPT5
	};

	gunbustr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_watchdog(*this, "watchdog"),
		m_tc0480scp(*this, "tc0480scp"),
		m_ram(*this,"ram"),
		m_spriteram(*this,"spriteram"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{
		m_coin_lockout = true;
	}

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<tc0480scp_device> m_tc0480scp;
	required_shared_ptr<uint32_t> m_ram;
	required_shared_ptr<uint32_t> m_spriteram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	bool m_coin_lockout;
	uint16_t m_coin_word;
	std::unique_ptr<gb_tempsprite[]> m_spritelist;
	uint32_t m_mem[2];

	void gunbustr_input_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void motor_control_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t gunbustr_gun_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void gunbustr_gun_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t main_cycle_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	ioport_value coin_word_r(ioport_field &field, void *param);
	void init_gunbustrj();
	void init_gunbustr();
	virtual void video_start() override;
	uint32_t screen_update_gunbustr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gunbustr_interrupt(device_t &device);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const int *primasks,int x_offs,int y_offs);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
