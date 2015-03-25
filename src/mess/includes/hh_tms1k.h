// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle, Kevin Horton
/*

  TMS1000 MCU series tabletops/handhelds or other simple devices.

*/

#ifndef _HH_TMS1K_H_
#define _HH_TMS1K_H_


#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"


class hh_tms1k_state : public driver_device
{
public:
	hh_tms1k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN"),
		m_speaker(*this, "speaker"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_ioport_array<7> m_inp_matrix; // max 7
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	UINT16 m_r;                         // MCU R-pins data
	UINT16 m_o;                         // MCU O-pins data
	UINT16 m_inp_mux;                   // multiplexed inputs mask
	bool m_power_on;

	UINT8 read_inputs(int columns);
	DECLARE_INPUT_CHANGED_MEMBER(power_button);
	DECLARE_WRITE_LINE_MEMBER(auto_power_off);

	// display common
	int m_display_wait;                 // led/lamp off-delay in microseconds (default 33ms)
	int m_display_maxy;                 // display matrix number of rows
	int m_display_maxx;                 // display matrix number of columns

	UINT32 m_display_state[0x20];       // display matrix rows data
	UINT16 m_display_segmask[0x20];     // if not 0, display matrix row is a digit, mask indicates connected segments
	UINT32 m_display_cache[0x20];       // (internal use)
	UINT8 m_display_decay[0x20][0x20];  // (internal use)

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety);

	// game-specific handlers
	void mathmagi_display();
	DECLARE_WRITE16_MEMBER(mathmagi_write_r);
	DECLARE_WRITE16_MEMBER(mathmagi_write_o);
	DECLARE_READ8_MEMBER(mathmagi_read_k);

	void amaztron_display();
	DECLARE_WRITE16_MEMBER(amaztron_write_r);
	DECLARE_WRITE16_MEMBER(amaztron_write_o);
	DECLARE_READ8_MEMBER(amaztron_read_k);

	void tc4_display();
	DECLARE_WRITE16_MEMBER(tc4_write_r);
	DECLARE_WRITE16_MEMBER(tc4_write_o);
	DECLARE_READ8_MEMBER(tc4_read_k);

	void ebball_display();
	DECLARE_WRITE16_MEMBER(ebball_write_r);
	DECLARE_WRITE16_MEMBER(ebball_write_o);
	DECLARE_READ8_MEMBER(ebball_read_k);

	void ebball2_display();
	DECLARE_WRITE16_MEMBER(ebball2_write_r);
	DECLARE_WRITE16_MEMBER(ebball2_write_o);
	DECLARE_READ8_MEMBER(ebball2_read_k);

	void ebball3_display();
	DECLARE_WRITE16_MEMBER(ebball3_write_r);
	DECLARE_WRITE16_MEMBER(ebball3_write_o);
	DECLARE_READ8_MEMBER(ebball3_read_k);
	void ebball3_set_clock();
	DECLARE_INPUT_CHANGED_MEMBER(ebball3_difficulty_switch);
	DECLARE_MACHINE_RESET(ebball3);

	DECLARE_WRITE16_MEMBER(elecdet_write_r);
	DECLARE_WRITE16_MEMBER(elecdet_write_o);
	DECLARE_READ8_MEMBER(elecdet_read_k);

	void starwbc_display();
	DECLARE_WRITE16_MEMBER(starwbc_write_r);
	DECLARE_WRITE16_MEMBER(starwbc_write_o);
	DECLARE_READ8_MEMBER(starwbc_read_k);

	DECLARE_WRITE16_MEMBER(comp4_write_r);
	DECLARE_WRITE16_MEMBER(comp4_write_o);
	DECLARE_READ8_MEMBER(comp4_read_k);

	DECLARE_WRITE16_MEMBER(simon_write_r);
	DECLARE_WRITE16_MEMBER(simon_write_o);
	DECLARE_READ8_MEMBER(simon_read_k);

	DECLARE_WRITE16_MEMBER(ssimon_write_r);
	DECLARE_WRITE16_MEMBER(ssimon_write_o);
	DECLARE_READ8_MEMBER(ssimon_read_k);

	DECLARE_WRITE16_MEMBER(cnsector_write_r);
	DECLARE_WRITE16_MEMBER(cnsector_write_o);
	DECLARE_READ8_MEMBER(cnsector_read_k);

	DECLARE_WRITE16_MEMBER(merlin_write_r);
	DECLARE_WRITE16_MEMBER(merlin_write_o);
	DECLARE_READ8_MEMBER(merlin_read_k);

	DECLARE_WRITE16_MEMBER(stopthief_write_r);
	DECLARE_WRITE16_MEMBER(stopthief_write_o);
	DECLARE_READ8_MEMBER(stopthief_read_k);

	DECLARE_WRITE16_MEMBER(bankshot_write_r);
	DECLARE_WRITE16_MEMBER(bankshot_write_o);
	DECLARE_READ8_MEMBER(bankshot_read_k);

	DECLARE_WRITE16_MEMBER(splitsec_write_r);
	DECLARE_WRITE16_MEMBER(splitsec_write_o);
	DECLARE_READ8_MEMBER(splitsec_read_k);

	void tandy12_display();
	DECLARE_WRITE16_MEMBER(tandy12_write_r);
	DECLARE_WRITE16_MEMBER(tandy12_write_o);
	DECLARE_READ8_MEMBER(tandy12_read_k);

protected:
	virtual void machine_start();
	virtual void machine_reset();
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
