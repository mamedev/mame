// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#include "includes/nb1413m3.h"

class pastelg_state : public driver_device
{
public:
	enum
	{
		TIMER_BLITTER
	};

	pastelg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_nb1413m3(*this, "nb1413m3"),
		m_screen(*this, "screen"),
		m_clut(*this, "clut") { }

	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_clut;

	uint8_t m_mux_data;
	int m_blitter_destx;
	int m_blitter_desty;
	int m_blitter_sizex;
	int m_blitter_sizey;
	int m_blitter_src_addr;
	int m_gfxrom;
	int m_dispflag;
	int m_flipscreen;
	int m_blitter_direction_x;
	int m_blitter_direction_y;
	int m_palbank;
	std::unique_ptr<uint8_t[]> m_videoram;
	int m_flipscreen_old;

	uint8_t pastelg_sndrom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pastelg_irq_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t threeds_inputport1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t threeds_inputport2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void threeds_inputportsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pastelg_blitter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void threeds_romsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void threeds_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t threeds_rom_readback_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pastelg_romsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	ioport_value nb1413m3_busyflag_r(ioport_field &field, void *param);
	ioport_value nb1413m3_hackbusyflag_r(ioport_field &field, void *param);

	virtual void machine_start() override;
	virtual void video_start() override;

	void palette_init_pastelg(palette_device &palette);
	uint32_t screen_update_pastelg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int pastelg_blitter_src_addr_r();
	void pastelg_vramflip();
	void pastelg_gfxdraw();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
