// license:BSD-3-Clause
// copyright-holders:hap
/*

  NEC uCOM4 MCU tabletops/handhelds or other simple devices.

*/

#ifndef MAME_INCLUDES_HH_UCOM4_H
#define MAME_INCLUDES_HH_UCOM4_H

#include "includes/screenless.h"

#include "cpu/ucom4/ucom4.h"
#include "sound/spkrdev.h"


class hh_ucom4_state : public screenless_state
{
public:
	hh_ucom4_state(const machine_config &mconfig, device_type type, const char *tag) :
		screenless_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_speaker(*this, "speaker")
	{ }

	// devices
	required_device<ucom4_cpu_device> m_maincpu;
	optional_ioport_array<6> m_inp_matrix; // max 6
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	u8 m_port[9];                   // MCU port A-I write data (optional)
	u8 m_int;                       // MCU INT pin state
	u16 m_inp_mux;                  // multiplexed inputs mask

	u32 m_grid;                     // VFD current row data
	u32 m_plate;                    // VFD current column data

	u8 read_inputs(int columns);
	void refresh_interrupts(void);
	void set_interrupt(int state);
	DECLARE_INPUT_CHANGED_MEMBER(single_interrupt_line);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


#endif // MAME_INCLUDES_HH_UCOM4_H
