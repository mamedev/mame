// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle, Kevin Horton
/*

  TMS1000 MCU series tabletops/handhelds or other simple devices.

*/

#ifndef _HH_TMS1K_H_
#define _HH_TMS1K_H_

#include "emu.h"
#include "cpu/tms1000/tms1000.h"
#include "cpu/tms1000/tms1100.h"
#include "cpu/tms1000/tms1400.h"
#include "cpu/tms1000/tms0970.h"
#include "cpu/tms1000/tms0980.h"
#include "cpu/tms1000/tms0270.h"
#include "cpu/tms1000/tp0320.h"
#include "sound/speaker.h"


class hh_tms1k_state : public driver_device
{
public:
	hh_tms1k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_speaker(*this, "speaker"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices
	required_device<tms1k_base_device> m_maincpu;
	optional_ioport_array<18> m_inp_matrix; // max 18
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	uint16_t m_r;                         // MCU R-pins data
	uint16_t m_o;                         // MCU O-pins data
	uint32_t m_inp_mux;                   // multiplexed inputs mask
	bool m_power_on;
	bool m_power_led;

	uint8_t read_inputs(int columns);
	uint8_t read_rotated_inputs(int columns, uint8_t rowmask = 0xf);
	virtual void power_button(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	virtual void auto_power_off(int state);

	// display common
	int m_display_wait;                 // led/lamp off-delay in microseconds (default 33ms)
	int m_display_maxy;                 // display matrix number of rows
	int m_display_maxx;                 // display matrix number of columns (max 31 for now)

	uint32_t m_grid;                      // VFD/LED current row data
	uint32_t m_plate;                     // VFD/LED current column data

	uint32_t m_display_state[0x20];       // display matrix rows data (last bit is used for always-on)
	uint16_t m_display_segmask[0x20];     // if not 0, display matrix row is a digit, mask indicates connected segments
	uint32_t m_display_cache[0x20];       // (internal use)
	uint8_t m_display_decay[0x20][0x20];  // (internal use)

	void display_decay_tick(timer_device &timer, void *ptr, int32_t param);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void set_display_segmask(uint32_t digits, uint32_t mask);
	void display_matrix(int maxx, int maxy, uint32_t setx, uint32_t sety, bool update = true);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// LED segments
enum
{
	lA = 0x01,
	lB = 0x02,
	lC = 0x04,
	lD = 0x08,
	lE = 0x10,
	lF = 0x20,
	lG = 0x40,
	lDP = 0x80
};


#endif /* _HH_TMS1K_H_ */
