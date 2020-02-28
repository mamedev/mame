// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Exidy Car Polo hardware

    driver by Zsolt Vasvari

****************************************************************************/
#ifndef MAME_INCLUDES_CARPOLO_H
#define MAME_INCLUDES_CARPOLO_H

#pragma once

#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "machine/7474.h"
#include "machine/74148.h"
#include "machine/74153.h"
#include "emupal.h"

class carpolo_state : public driver_device
{
public:
	carpolo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ttl74148_3s(*this, "74148_3s")
		, m_ttl74153_1k(*this, "74153_1k")
		, m_ttl7474_2s_1(*this, "7474_2s_1")
		, m_ttl7474_2s_2(*this, "7474_2s_2")
		, m_ttl7474_2u_1(*this, "7474_2u_1")
		, m_ttl7474_2u_2(*this, "7474_2u_2")
		, m_ttl7474_1f_1(*this, "7474_1f_1")
		, m_ttl7474_1f_2(*this, "7474_1f_2")
		, m_ttl7474_1d_1(*this, "7474_1d_1")
		, m_ttl7474_1d_2(*this, "7474_1d_2")
		, m_ttl7474_1c_1(*this, "7474_1c_1")
		, m_ttl7474_1c_2(*this, "7474_1c_2")
		, m_ttl7474_1a_1(*this, "7474_1a_1")
		, m_ttl7474_1a_2(*this, "7474_1a_2")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_alpharam(*this, "alpharam")
		, m_spriteram(*this, "spriteram")
		, m_user1(*this, "user1")
		, m_dial(*this, "DIAL%u", 0U)
		, m_in(*this, "IN%u", 0U)
		, m_pedals(*this, "PEDALS")
	{ }

	void init_carpolo();
	void carpolo(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	uint8_t m_ball_screen_collision_cause;
	uint8_t m_car_ball_collision_x;
	uint8_t m_car_ball_collision_y;
	uint8_t m_car_car_collision_cause;
	uint8_t m_car_goal_collision_cause;
	uint8_t m_car_ball_collision_cause;
	uint8_t m_car_border_collision_cause;
	uint8_t m_priority_0_extension;
	uint8_t m_last_wheel_value[4];
	int m_ls153_za;
	int m_ls153_zb;
	required_device<m6502_device> m_maincpu;
	required_device<ttl74148_device> m_ttl74148_3s;
	required_device<ttl153_device> m_ttl74153_1k;
	required_device<ttl7474_device> m_ttl7474_2s_1;
	required_device<ttl7474_device> m_ttl7474_2s_2;
	required_device<ttl7474_device> m_ttl7474_2u_1;
	required_device<ttl7474_device> m_ttl7474_2u_2;
	required_device<ttl7474_device> m_ttl7474_1f_1;
	required_device<ttl7474_device> m_ttl7474_1f_2;
	required_device<ttl7474_device> m_ttl7474_1d_1;
	required_device<ttl7474_device> m_ttl7474_1d_2;
	required_device<ttl7474_device> m_ttl7474_1c_1;
	required_device<ttl7474_device> m_ttl7474_1c_2;
	required_device<ttl7474_device> m_ttl7474_1a_1;
	required_device<ttl7474_device> m_ttl7474_1a_2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_alpharam;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_region m_user1;
	required_ioport_array<4> m_dial;
	required_ioport_array<3> m_in;
	required_ioport m_pedals;

	std::unique_ptr<bitmap_ind16> m_sprite_sprite_collision_bitmap1;
	std::unique_ptr<bitmap_ind16> m_sprite_sprite_collision_bitmap2;
	std::unique_ptr<bitmap_ind16> m_sprite_goal_collision_bitmap1;
	std::unique_ptr<bitmap_ind16> m_sprite_goal_collision_bitmap2;
	std::unique_ptr<bitmap_ind16> m_sprite_border_collision_bitmap;

	DECLARE_READ8_MEMBER(ball_screen_collision_cause_r);
	DECLARE_READ8_MEMBER(car_ball_collision_x_r);
	DECLARE_READ8_MEMBER(car_ball_collision_y_r);
	DECLARE_READ8_MEMBER(car_car_collision_cause_r);
	DECLARE_READ8_MEMBER(car_goal_collision_cause_r);
	DECLARE_READ8_MEMBER(car_ball_collision_cause_r);
	DECLARE_READ8_MEMBER(car_border_collision_cause_r);
	DECLARE_READ8_MEMBER(interrupt_cause_r);
	DECLARE_WRITE8_MEMBER(ball_screen_interrupt_clear_w);
	DECLARE_WRITE8_MEMBER(car_car_interrupt_clear_w);
	DECLARE_WRITE8_MEMBER(car_goal_interrupt_clear_w);
	DECLARE_WRITE8_MEMBER(car_ball_interrupt_clear_w);
	DECLARE_WRITE8_MEMBER(car_border_interrupt_clear_w);
	DECLARE_WRITE8_MEMBER(timer_interrupt_clear_w);
	void carpolo_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	DECLARE_WRITE_LINE_MEMBER(coin1_interrupt_clear_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_interrupt_clear_w);
	DECLARE_WRITE_LINE_MEMBER(coin3_interrupt_clear_w);
	DECLARE_WRITE_LINE_MEMBER(coin4_interrupt_clear_w);
	DECLARE_WRITE8_MEMBER(pia_0_port_a_w);
	DECLARE_WRITE8_MEMBER(pia_0_port_b_w);
	DECLARE_READ8_MEMBER(pia_0_port_b_r);
	DECLARE_READ8_MEMBER(pia_1_port_a_r);
	DECLARE_READ8_MEMBER(pia_1_port_b_r);
	DECLARE_WRITE_LINE_MEMBER(ttl7474_2s_1_q_cb);
	DECLARE_WRITE_LINE_MEMBER(ttl7474_2s_2_q_cb);
	DECLARE_WRITE_LINE_MEMBER(ttl7474_2u_1_q_cb);
	DECLARE_WRITE_LINE_MEMBER(ttl7474_2u_2_q_cb);
	DECLARE_WRITE_LINE_MEMBER(ls153_za_w);
	DECLARE_WRITE_LINE_MEMBER(ls153_zb_w);

	DECLARE_WRITE8_MEMBER(ttl74148_3s_cb);

	void timer_tick();
	void remap_sprite_code(int bank, int code, int *remapped_code, int *flipy);
	void normalize_coordinates(int *x1, int *y1, int *x2, int *y2);
	int check_sprite_left_goal_collision(int x1, int y1, int code1, int flipy1, int goalpost_only);
	int check_sprite_right_goal_collision(int x1, int y1, int code1, int flipy1, int goalpost_only);
	int check_sprite_border_collision(uint8_t x1, uint8_t y1, int code1, int flipy1);
	void generate_ball_screen_interrupt(uint8_t cause);
	void generate_car_car_interrupt(int car1, int car2);
	void generate_car_goal_interrupt(int car, int right_goal);
	void generate_car_ball_interrupt(int car, int car_x, int car_y);
	void generate_car_border_interrupt(int car, int horizontal_border);
	void draw_alpha_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int alpha_line, int video_line);
	void draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t x, uint8_t y, int bank, int code, int col);
	int check_sprite_sprite_collision(int x1, int y1, int code1, int flipy1,
										int x2, int y2, int code2, int flipy2,
										int *col_x, int *col_y);
	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_CARPOLO_H
