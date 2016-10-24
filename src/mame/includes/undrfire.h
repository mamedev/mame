// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves

#include "machine/eepromser.h"
#include "machine/watchdog.h"
#include "video/tc0100scn.h"
#include "video/tc0480scp.h"

struct uf_tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class undrfire_state : public driver_device
{
public:
	enum
	{
		TIMER_INTERRUPT5
	};

	undrfire_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_tc0100scn(*this, "tc0100scn"),
		m_tc0480scp(*this, "tc0480scp"),
		m_eeprom(*this, "eeprom"),
		m_watchdog(*this, "watchdog"),
		m_ram(*this, "ram"),
		m_shared_ram(*this, "shared_ram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<tc0100scn_device> m_tc0100scn;
	required_device<tc0480scp_device> m_tc0480scp;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<watchdog_timer_device> m_watchdog;
	optional_shared_ptr<uint32_t> m_ram;
	optional_shared_ptr<uint32_t> m_shared_ram;
	uint16_t m_coin_word;
	uint16_t m_port_sel;
	int m_frame_counter;
	std::unique_ptr<uf_tempsprite[]> m_spritelist;
	uint16_t m_rotate_ctrl[8];
	uint8_t m_dislayer[6];
	required_shared_ptr<uint32_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint32_t undrfire_input_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void undrfire_input_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t shared_ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void shared_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t unknown_hardware_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void unknown_int_req_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t undrfire_lightgun_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void rotate_control_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void motor_control_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void cbombers_cpua_ctrl_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t cbombers_adc_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void cbombers_adc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value frame_counter_r(ioport_field &field, void *param);
	void init_undrfire();
	void init_cbombers();
	virtual void video_start() override;
	uint32_t screen_update_undrfire(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cbombers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void undrfire_interrupt(device_t &device);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const int *primasks,int x_offs,int y_offs);
	void draw_sprites_cbombers(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const int *primasks,int x_offs,int y_offs);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
