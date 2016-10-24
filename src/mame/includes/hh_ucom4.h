// license:BSD-3-Clause
// copyright-holders:hap, Kevin Horton
/*

  NEC uCOM4 MCU tabletops/handhelds or other simple devices,

*/

#ifndef _HH_UCOM4_H_
#define _HH_UCOM4_H_


#include "emu.h"
#include "rendlay.h"
#include "cpu/ucom4/ucom4.h"
#include "sound/speaker.h"


class hh_ucom4_state : public driver_device
{
public:
	hh_ucom4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_speaker(*this, "speaker"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_ioport_array<5> m_inp_matrix; // max 5
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	uint8_t m_port[9];                    // MCU port A-I write data (optional)
	uint8_t m_int;                        // MCU INT pin state
	uint16_t m_inp_mux;                   // multiplexed inputs mask

	uint8_t read_inputs(int columns);
	void refresh_interrupts(void);
	void set_interrupt(int state);
	void single_interrupt_line(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	// display common
	int m_display_wait;                 // led/lamp off-delay in microseconds (default 33ms)
	int m_display_maxy;                 // display matrix number of rows
	int m_display_maxx;                 // display matrix number of columns (max 31 for now)

	uint32_t m_grid;                      // VFD current row data
	uint32_t m_plate;                     // VFD current column data

	uint32_t m_display_state[0x20];       // display matrix rows data (last bit is used for always-on)
	uint16_t m_display_segmask[0x20];     // if not 0, display matrix row is a digit, mask indicates connected segments
	uint32_t m_display_cache[0x20];       // (internal use)
	uint8_t m_display_decay[0x20][0x20];  // (internal use)

	void display_decay_tick(timer_device &timer, void *ptr, int32_t param);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void display_matrix(int maxx, int maxy, uint32_t setx, uint32_t sety, bool update = true);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


#endif /* _HH_UCOM4_H_ */
