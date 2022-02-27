// license:BSD-3-Clause
// copyright-holders:hap
/*

  Generic sensorboard device

*/

#ifndef MAME_MACHINE_SENSORBOARD_H
#define MAME_MACHINE_SENSORBOARD_H

#pragma once

class sensorboard_device : public device_t, public device_nvram_interface
{
public:
	sensorboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	enum sb_type
	{
		NOSENSORS = 0,
		BUTTONS,
		MAGNETS,
		INDUCTIVE
	};

	// configuration helpers
	sensorboard_device &set_type(sb_type type); // sensor type
	sensorboard_device &set_size(u8 width, u8 height) { m_width = width; m_height = height; return *this; } // board dimensions, max 12 * 10
	sensorboard_device &set_spawnpoints(u8 i) { m_maxspawn = i; m_maxid = i; return *this; } // number of piece spawnpoints, max 16
	sensorboard_device &set_max_id(u8 i) { m_maxid = i; return *this; } // maximum piece id (if larger than set_spawnpoints)
	sensorboard_device &set_delay(attotime delay) { m_sensordelay = delay; return *this; } // delay when activating a sensor (like PORT_IMPULSE), set to attotime::never to disable
	sensorboard_device &set_nvram_enable(bool b) { m_nvram_auto = b; return *this; } // load last board position on start
	sensorboard_device &set_ui_enable(bool b) { if (!b) m_maxspawn = 0; m_ui_enabled = (b) ? 7 : 0; return *this; } // enable UI inputs
	sensorboard_device &set_mod_enable(bool b) { if (b) m_ui_enabled |= 1; else m_ui_enabled &= ~1; return *this; } // enable modifier keys

	auto init_cb() { return m_custom_init_cb.bind(); } // for setting pieces starting position
	auto sensor_cb() { return m_custom_sensor_cb.bind(); } // x = offset & 0xf, y = offset >> 4 & 0xf
	auto spawn_cb() { return m_custom_spawn_cb.bind(); } // spawnpoint/piece = offset, retval = new piece id
	auto output_cb() { return m_custom_output_cb.bind(); } // pos = offset(A8 for ui/board, A9 for count), id = data

	void preset_chess(int state); // init_cb() preset for chessboards

	// read sensors
	u8 read_sensor(u8 x, u8 y);
	u16 read_file(u8 x, bool reverse = false);
	u16 read_rank(u8 y, bool reverse = false);

	bool is_inductive() { return m_inductive; }

	// handle board state
	u8 read_piece(u8 x, u8 y) { return m_curstate[y * m_width + x]; }
	void write_piece(u8 x, u8 y, u8 id) { m_curstate[y * m_width + x] = id; }
	void clear_board() { memset(m_curstate, 0, sizeof(m_curstate)); }

	void refresh();
	void cancel_sensor();

	// handle pieces
	void cancel_hand();
	void remove_hand();
	int get_handpos() { return m_handpos; }
	bool drop_piece(u8 x, u8 y);
	bool pickup_piece(u8 x, u8 y);

	// input handlers
	DECLARE_INPUT_CHANGED_MEMBER(sensor);
	DECLARE_INPUT_CHANGED_MEMBER(ui_spawn);
	DECLARE_INPUT_CHANGED_MEMBER(ui_hand);
	DECLARE_INPUT_CHANGED_MEMBER(ui_undo);
	DECLARE_INPUT_CHANGED_MEMBER(ui_init);
	DECLARE_INPUT_CHANGED_MEMBER(ui_refresh) { refresh(); }

	DECLARE_CUSTOM_INPUT_MEMBER(check_sensor_busy) { return (m_sensorpos == -1) ? 0 : 1; }
	DECLARE_CUSTOM_INPUT_MEMBER(check_bs_mask) { return m_bs_mask; }
	DECLARE_CUSTOM_INPUT_MEMBER(check_ss_mask) { return m_ss_mask; }
	DECLARE_CUSTOM_INPUT_MEMBER(check_ui_enabled) { return m_ui_enabled; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override { refresh(); }

	virtual ioport_constructor device_input_ports() const override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual bool nvram_can_write() override;

private:
	output_finder<0x10, 0x10> m_out_piece;
	output_finder<0x20+1> m_out_pui;
	output_finder<2> m_out_count;
	required_ioport_array<10> m_inp_rank;
	required_ioport m_inp_spawn;
	required_ioport m_inp_ui;
	required_ioport m_inp_conf;

	devcb_write_line m_custom_init_cb;
	devcb_read8 m_custom_sensor_cb;
	devcb_read8 m_custom_spawn_cb;
	devcb_write16 m_custom_output_cb;

	bool m_nosensors;
	bool m_magnets;
	bool m_inductive;
	u8 m_width;
	u8 m_height;
	u32 m_bs_mask;
	u32 m_ss_mask;
	u8 m_maxspawn;
	u8 m_maxid;
	u8 m_hand;
	int m_handpos;
	int m_droppos;
	int m_sensorpos;
	u8 m_ui_enabled;

	u8 m_curstate[0x100];
	u8 m_history[1000][0x100];

	u8 m_uselect;
	u32 m_upointer;
	u32 m_ufirst;
	u32 m_ulast;
	u32 m_usize;

	bool m_nvram_auto;
	bool nvram_on();

	emu_timer *m_undotimer;
	TIMER_CALLBACK_MEMBER(undo_tick);
	void undo_reset();

	attotime m_sensordelay;
	emu_timer *m_sensortimer;
	TIMER_CALLBACK_MEMBER(sensor_off) { cancel_sensor(); }
};


DECLARE_DEVICE_TYPE(SENSORBOARD, sensorboard_device)

#endif // MAME_MACHINE_SENSORBOARD_H
