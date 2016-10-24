// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves

#include "machine/watchdog.h"
#include "video/tc0100scn.h"
#include "video/tc0480scp.h"

struct gfx_tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int pri;
};

class groundfx_state : public driver_device
{
public:
	enum
	{
		TIMER_GROUNDFX_INTERRUPT5
	};

	groundfx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ram(*this,"ram"),
		m_spriteram(*this,"spriteram") ,
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_tc0100scn(*this, "tc0100scn"),
		m_tc0480scp(*this, "tc0480scp"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<uint32_t> m_ram;
	required_shared_ptr<uint32_t> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<tc0100scn_device> m_tc0100scn;
	required_device<tc0480scp_device> m_tc0480scp;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint16_t m_coin_word;
	uint16_t m_frame_counter;
	uint16_t m_port_sel;
	std::unique_ptr<gfx_tempsprite[]> m_spritelist;
	uint16_t m_rotate_ctrl[8];
	rectangle m_hack_cliprect;

	void groundfx_input_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t groundfx_adc_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void groundfx_adc_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void rotate_control_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void motor_control_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t irq_speedup_r_groundfx(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	ioport_value frame_counter_r(ioport_field &field, void *param);
	ioport_value coin_word_r(ioport_field &field, void *param);
	void init_groundfx();
	virtual void video_start() override;
	uint32_t screen_update_groundfx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void groundfx_interrupt(device_t &device);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,int do_hack,int x_offs,int y_offs);


protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
