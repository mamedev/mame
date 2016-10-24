// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    The Game Room Lethal Justice hardware

**************************************************************************/

#include "machine/ticket.h"


class lethalj_state : public driver_device
{
public:
	enum
	{
		TIMER_GEN_EXT1_INT
	};

	lethalj_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_ticket(*this, "ticket"),
		m_paddle(*this, "PADDLE"),
		m_light0_x(*this, "LIGHT0_X"),
		m_light0_y(*this, "LIGHT0_Y"),
		m_light1_x(*this, "LIGHT1_X"),
		m_light1_y(*this, "LIGHT1_Y")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<ticket_dispenser_device> m_ticket;
	optional_ioport m_paddle;
	optional_ioport m_light0_x;
	optional_ioport m_light0_y;
	optional_ioport m_light1_x;
	optional_ioport m_light1_y;

	uint16_t m_blitter_data[8];
	std::unique_ptr<uint16_t[]> m_screenram;
	uint8_t m_vispage;
	uint16_t *m_blitter_base;
	int m_blitter_rows;
	uint16_t m_gunx;
	uint16_t m_guny;
	uint8_t m_blank_palette;
	void ripribit_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cfarm_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cclownz_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t lethalj_gun_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void lethalj_blitter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void do_blit();
	ioport_value cclownz_paddle(ioport_field &field, void *param);
	void init_cfarm();
	void init_ripribit();
	void init_cclownz();
	virtual void video_start() override;
	inline void get_crosshair_xy(int player, int *x, int *y);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

/*----------- defined in video/lethalj.c -----------*/
void lethalj_scanline_update(screen_device &screen, bitmap_ind16 &bitmap, int scanline, const tms34010_display_params *params);
