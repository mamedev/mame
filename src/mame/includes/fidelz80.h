// license:BSD-3-Clause
// copyright-holders:Kevin Horton,Jonathan Gevaryahu,Sandro Ronco,hap
/******************************************************************************
*
*  Fidelity Electronics Z80 based board driver
*
******************************************************************************/

#include "emu.h"
#include "sound/speaker.h"
#include "sound/s14001a_new.h"

class fidelz80base_state : public driver_device
{
public:
	fidelz80base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN"),
		m_speech(*this, "speech"),
		m_speech_rom(*this, "speech"),
		m_speaker(*this, "speaker"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	optional_ioport_array<10> m_inp_matrix; // max 10
	optional_device<s14001a_new_device> m_speech;
	optional_region_ptr<UINT8> m_speech_rom;
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	UINT16 m_inp_mux;                   // multiplexed keypad/leds mask
	UINT16 m_led_select;             // 5 bit selects for 7 seg leds and for common other leds, bits are (7seg leds are 0 1 2 3, common other leds are C) 0bxx3210xc
	UINT16 m_7seg_data;            // data for seg leds
	UINT16 m_led_data;
	UINT8 m_speech_bank;

	UINT16 read_inputs(int columns);

	// display common
	int m_display_wait;                 // led/lamp off-delay in microseconds (default 33ms)
	int m_display_maxy;                 // display matrix number of rows
	int m_display_maxx;                 // display matrix number of columns (max 31 for now)

	UINT32 m_display_state[0x20];       // display matrix rows data (last bit is used for always-on)
	UINT16 m_display_segmask[0x20];     // if not 0, display matrix row is a digit, mask indicates connected segments
	UINT32 m_display_cache[0x20];       // (internal use)
	UINT8 m_display_decay[0x20][0x20];  // (internal use)

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};
