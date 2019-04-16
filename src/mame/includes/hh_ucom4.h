// license:BSD-3-Clause
// copyright-holders:hap
/*

  NEC uCOM4 MCU tabletops/handhelds or other simple devices.

*/

#ifndef MAME_INCLUDES_HH_UCOM4_H
#define MAME_INCLUDES_HH_UCOM4_H

#include "cpu/ucom4/ucom4.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"


class hh_ucom4_state : public driver_device
{
public:
	hh_ucom4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_out_x(*this, "%u.%u", 0U, 0U),
		m_out_a(*this, "%u.a", 0U),
		m_out_digit(*this, "digit%u", 0U),
		m_speaker(*this, "speaker"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices
	required_device<ucom4_cpu_device> m_maincpu;
	optional_ioport_array<6> m_inp_matrix; // max 6
	output_finder<0x20, 0x20> m_out_x;
	output_finder<0x20> m_out_a;
	output_finder<0x20> m_out_digit;
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	u8 m_port[9];                   // MCU port A-I write data (optional)
	u8 m_int;                       // MCU INT pin state
	u16 m_inp_mux;                  // multiplexed inputs mask

	u8 read_inputs(int columns);
	void refresh_interrupts(void);
	void set_interrupt(int state);
	DECLARE_INPUT_CHANGED_MEMBER(single_interrupt_line);

	// display common
	int m_display_wait;             // led/lamp off-delay in milliseconds (default 33ms)
	int m_display_maxy;             // display matrix number of rows
	int m_display_maxx;             // display matrix number of columns (max 31 for now)

	u32 m_grid;                     // VFD current row data
	u32 m_plate;                    // VFD current column data

	u32 m_display_state[0x20];      // display matrix rows data (last bit is used for always-on)
	u16 m_display_segmask[0x20];    // if not 0, display matrix row is a digit, mask indicates connected segments
	u8 m_display_decay[0x20][0x20]; // (internal use)

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void set_display_segmask(u32 digits, u32 mask);
	void display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update = true);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


#endif // MAME_INCLUDES_HH_UCOM4_H
