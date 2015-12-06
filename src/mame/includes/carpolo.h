// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Exidy Car Polo hardware

    driver by Zsolt Vasvari

****************************************************************************/

#include "machine/6821pia.h"
#include "machine/7474.h"
#include "machine/74148.h"
#include "machine/74153.h"

class carpolo_state : public driver_device
{
public:
	carpolo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_alpharam(*this, "alpharam"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_ttl74148_3s(*this, "74148_3s"),
		m_ttl74153_1k(*this, "74153_1k"),
		m_ttl7474_2s_1(*this, "7474_2s_1"),
		m_ttl7474_2s_2(*this, "7474_2s_2"),
		m_ttl7474_2u_1(*this, "7474_2u_1"),
		m_ttl7474_2u_2(*this, "7474_2u_2"),
		m_ttl7474_1f_1(*this, "7474_1f_1"),
		m_ttl7474_1f_2(*this, "7474_1f_2"),
		m_ttl7474_1d_1(*this, "7474_1d_1"),
		m_ttl7474_1d_2(*this, "7474_1d_2"),
		m_ttl7474_1c_1(*this, "7474_1c_1"),
		m_ttl7474_1c_2(*this, "7474_1c_2"),
		m_ttl7474_1a_1(*this, "7474_1a_1"),
		m_ttl7474_1a_2(*this, "7474_1a_2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
		{}

	required_shared_ptr<UINT8> m_alpharam;
	required_shared_ptr<UINT8> m_spriteram;
	UINT8 m_ball_screen_collision_cause;
	UINT8 m_car_ball_collision_x;
	UINT8 m_car_ball_collision_y;
	UINT8 m_car_car_collision_cause;
	UINT8 m_car_goal_collision_cause;
	UINT8 m_car_ball_collision_cause;
	UINT8 m_car_border_collision_cause;
	UINT8 m_priority_0_extension;
	UINT8 m_last_wheel_value[4];
	required_device<cpu_device> m_maincpu;
	required_device<ttl74148_device> m_ttl74148_3s;
	required_device<ttl74153_device> m_ttl74153_1k;
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

	bitmap_ind16 *m_sprite_sprite_collision_bitmap1;
	bitmap_ind16 *m_sprite_sprite_collision_bitmap2;
	bitmap_ind16 *m_sprite_goal_collision_bitmap1;
	bitmap_ind16 *m_sprite_goal_collision_bitmap2;
	bitmap_ind16 *m_sprite_border_collision_bitmap;
	DECLARE_READ8_MEMBER(carpolo_ball_screen_collision_cause_r);
	DECLARE_READ8_MEMBER(carpolo_car_ball_collision_x_r);
	DECLARE_READ8_MEMBER(carpolo_car_ball_collision_y_r);
	DECLARE_READ8_MEMBER(carpolo_car_car_collision_cause_r);
	DECLARE_READ8_MEMBER(carpolo_car_goal_collision_cause_r);
	DECLARE_READ8_MEMBER(carpolo_car_ball_collision_cause_r);
	DECLARE_READ8_MEMBER(carpolo_car_border_collision_cause_r);
	DECLARE_READ8_MEMBER(carpolo_interrupt_cause_r);
	DECLARE_WRITE8_MEMBER(carpolo_ball_screen_interrupt_clear_w);
	DECLARE_WRITE8_MEMBER(carpolo_car_car_interrupt_clear_w);
	DECLARE_WRITE8_MEMBER(carpolo_car_goal_interrupt_clear_w);
	DECLARE_WRITE8_MEMBER(carpolo_car_ball_interrupt_clear_w);
	DECLARE_WRITE8_MEMBER(carpolo_car_border_interrupt_clear_w);
	DECLARE_WRITE8_MEMBER(carpolo_timer_interrupt_clear_w);
	DECLARE_DRIVER_INIT(carpolo);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(carpolo);
	UINT32 screen_update_carpolo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_carpolo(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(carpolo_timer_interrupt);
	DECLARE_WRITE_LINE_MEMBER(coin1_interrupt_clear_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_interrupt_clear_w);
	DECLARE_WRITE_LINE_MEMBER(coin3_interrupt_clear_w);
	DECLARE_WRITE_LINE_MEMBER(coin4_interrupt_clear_w);
	DECLARE_WRITE8_MEMBER(pia_0_port_a_w);
	DECLARE_WRITE8_MEMBER(pia_0_port_b_w);
	DECLARE_READ8_MEMBER(pia_0_port_b_r);
	DECLARE_READ8_MEMBER(pia_1_port_a_r);
	DECLARE_READ8_MEMBER(pia_1_port_b_r);
	DECLARE_WRITE_LINE_MEMBER(carpolo_7474_2s_1_q_cb);
	DECLARE_WRITE_LINE_MEMBER(carpolo_7474_2s_2_q_cb);
	DECLARE_WRITE_LINE_MEMBER(carpolo_7474_2u_1_q_cb);
	DECLARE_WRITE_LINE_MEMBER(carpolo_7474_2u_2_q_cb);

	TTL74148_OUTPUT_CB(ttl74148_3s_cb);

	void remap_sprite_code(int bank, int code, int *remapped_code, int *flipy);
	void normalize_coordinates(int *x1, int *y1, int *x2, int *y2);
	int check_sprite_left_goal_collision(int x1, int y1, int code1, int flipy1, int goalpost_only);
	int check_sprite_right_goal_collision(int x1, int y1, int code1, int flipy1, int goalpost_only);
	int check_sprite_border_collision(UINT8 x1, UINT8 y1, int code1, int flipy1);
	void carpolo_generate_ball_screen_interrupt(UINT8 cause);
	void carpolo_generate_car_car_interrupt(int car1, int car2);
	void carpolo_generate_car_goal_interrupt(int car, int right_goal);
	void carpolo_generate_car_ball_interrupt(int car, int car_x, int car_y);
	void carpolo_generate_car_border_interrupt(int car, int horizontal_border);
	void draw_alpha_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int alpha_line, int video_line);
	void draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 x, UINT8 y, int bank, int code, int col);
	int check_sprite_sprite_collision(int x1, int y1, int code1, int flipy1,
										int x2, int y2, int code2, int flipy2,
										int *col_x, int *col_y);
};
