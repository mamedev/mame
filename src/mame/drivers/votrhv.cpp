// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
// thanks-to:Kevin Horton
/******************************************************************************
*
*  Votrax/Phonic Mirror HandiVolce models HC-110 and HC-120
*
******************************************************************************/
/*
	The HC-110 and HC-120 both consist of 3 boards:
	1. An 'embedded system' 6800 board with	two output latches, one input
	   latch, and two 'resume/reset' latches which	pull the 6800 is out of
	   reset on a rising edge, as a form of power saving. (pcb 1816?)
	2. A keyboard handler pcb; this is different on the HC-110 (where it is
	   made by a 3rd party company for the 128-key input) and the HC-120
	   (where it was made by votrax/phonic mirror, pcb 1817?)
	3. The voice synthesizer pcb 1818c, encased in epoxy. It is a discrete
	   synthesizer roughly equivalent to an SC-01 or VSK, it has external
	   pins to allow control of speech pitch and rate in addition to the
	   typical 2 inflection pins.

	TODO: 1818c discrete speech synth device
	TODO: make the two rstflops combine together using input_merger
*/

/* Core includes */
#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "sound/votrax.h"
#include "speaker.h"


class votrhv_state : public driver_device
{
public:
	votrhv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_votrax(*this, "votrax")
		, m_swarray(*this, "SW.%u", 0U)
		, m_latchx(0)
		, m_latchy(0)
		, m_rstflops(0)
	{ }

	// overrides
	virtual void device_start() override;

	void votrhv(machine_config &config);
	DECLARE_WRITE_LINE_MEMBER(key_pressed);
	DECLARE_WRITE_LINE_MEMBER(pho_done);

	static const device_timer_id TIMER_RESUME = 0;
private:
	// overrides
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	//virtual void machine_reset() override;

	void mem_map(address_map &map);

	required_device<m6800_cpu_device> m_maincpu;
	required_device<votrax_sc01_device> m_votrax;
	optional_ioport_array<16> m_swarray;

	uint8_t input_r();
	void latchx_w(uint8_t data);
	void latchy_w(uint8_t data);
	uint8_t latcha_rst_r();
	uint8_t latchb_rst_r();

	uint8_t m_latchx;
	uint8_t m_latchy;
	uint8_t m_rstflops;
	emu_timer* m_resume_timer;

};


/******************************************************************************
 Address Maps
******************************************************************************/

/*15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
   x  0  0  0    0  x  x  x    *  *  *  *    *  *  *  *    RW RAM (2x 2114 1kx4 SRAM, wired in parallel)
   x  0  0  0    1  x  x  x    x  x  x  x    x  x  x  x    open bus
   x  0  0  1    0  x  x  x    x  x  x  x    x  x  x  x    R  Input Latch
   x  0  0  1    1  x  x  x    x  x  x  x    x  x  x  x    open bus
   x  0  1  0    0  x  x  x    x  x  x  x    x  x  x  x    W  Latch X out
   x  0  1  0    1  x  x  x    x  x  x  x    x  x  x  x    open bus
   x  0  1  1    0  x  x  x    x  x  x  x    x  x  x  x    W  Latch Y out
   x  0  1  1    1  x  x  x    x  x  x  x    x  x  x  x    open bus
   x  1  0  0    0  *  *  *    *  *  *  *    *  *  *  *    R ROM0
   x  1  0  0    1  *  *  *    *  *  *  *    *  *  *  *    R ROM1
   x  1  0  1    0  *  *  *    *  *  *  *    *  *  *  *    R ROM2
   x  1  0  1    1  *  *  *    *  *  *  *    *  *  *  *    R ROM3
   x  1  1  0    0  x  x  x    x  x  x  x    x  x  x  x    W Latch A reset
   x  1  1  0    1  x  x  x    x  x  x  x    x  x  x  x    W Latch B reset
   x  1  1  1    0  x  x  x    x  x  x  x    x  x  x  x    open bus
   x  1  1  1    1  0  0  x    x  x  x  x    x  x  x  x    open bus
   x  1  1  1    1  0  1  *    *  *  *  *    *  *  *  *    R PROM0 (unpopulated)
   x  1  1  1    1  1  0  *    *  *  *  *    *  *  *  *    R PROM1
   x  1  1  1    1  1  1  *    *  *  *  *    *  *  *  *    R PROM2
*/

void votrhv_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).mirror(0x8700).ram(); /* RAM */
	// 0800-0fff open bus
	map(0x1000, 0x1000).mirror(0x87ff).r(FUNC(votrhv_state::input_r));
	// 1800-1fff open bus
	map(0x2000, 0x2000).mirror(0x87ff).w(FUNC(votrhv_state::latchx_w));
	// 2800-2fff open bus
	map(0x3000, 0x3000).mirror(0x87ff).w(FUNC(votrhv_state::latchy_w));
	// 3800-3fff open bus
	map(0x4000, 0x5fff).mirror(0x8000).rom().region("maincpu",0x4000); // Mask ROMs
	map(0x6000, 0x6000).mirror(0x87ff).r(FUNC(votrhv_state::latcha_rst_r));
	map(0x6800, 0x6800).mirror(0x87ff).r(FUNC(votrhv_state::latchb_rst_r));
	// 7000-77ff open bus
	// 7800-79ff open bus
	map(0x7a00, 0x7fff).mirror(0x8000).rom().region("maincpu",0x7a00); // boot PROMs
}


/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START(hc110)
	PORT_START("SW.0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Talk
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Master Clear
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Clear
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Talk Repeat
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Level 1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Level 2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Level 3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Level 4

	PORT_START("SW.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_TILT ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.14")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.15")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
INPUT_PORTS_END

//static INPUT_PORTS_START(hc120)
//INPUT_PORTS_END

/******************************************************************************
 Timers, then machine/reset handlers, then driver specific functions
******************************************************************************/

void votrhv_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_RESUME:
			// shut the timer OFF
			m_resume_timer->adjust(attotime::never);
			m_resume_timer->enable(false);
			// pull the cpu out of reset
			m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		break;
	}
}

void votrhv_state::device_start()
{
	m_resume_timer = timer_alloc(TIMER_RESUME);
	m_resume_timer->adjust(attotime::never);
	m_resume_timer->enable(false);
	save_item(NAME(m_latchx));
	save_item(NAME(m_latchy));
	save_item(NAME(m_rstflops));
}

WRITE_LINE_MEMBER( votrhv_state::key_pressed )
{
	// clock the key flipflop which sets its state to 1
	m_rstflops |= 0x40;
	// is the timer already running/non-zero? if not, start it.
	if (!m_resume_timer->enabled())
	{
		m_resume_timer->adjust(attotime::from_hz((2'000'000/2)/0x20));
	}
}

WRITE_LINE_MEMBER( votrhv_state::pho_done )
{
	if (state == ASSERT_LINE) // HACK: sc-01 is in /ready state now
	{
		// clock the pho_done flipflop which sets its state to 1
		m_rstflops |= 0x80;
		// is the timer already running/non-zero? if not, start it.
		if (!m_resume_timer->enabled())
		{
			m_resume_timer->adjust(attotime::from_hz((2'000'000/2)/0x20));
		}
		// HACK: we manually clock the same old phoneme into the sc-01 here, this is a hack since the 1818c the pho_done signal is a free running counter and doesn't stop like the A/R latch on the sc-01 does
		//m_votrax->inflection_w((m_latchy&0xc0)>>6);
		m_votrax->write(m_latchy&0x3f);
	}
}

void votrhv_state::latchx_w(uint8_t data)
{
	/* latchx output:
	 *  76543210
	 *  |||||||\- \
	 *  ||||||\--  \ key input column select
	 *  |||||\---  /
	 *  ||||\---- /
	 *  |||\----- \ LED select 1/2/3/4
	 *  ||\------ /
	 *  |\------- Green status LED
	 *  \-------- Phoneme silence (ties pitch input of 1818c high thru a diode)
	 */
	m_latchx = data;
}

void votrhv_state::latchy_w(uint8_t data)
{
	/* latchy output:
	 *  76543210
	 *  |||||||\- \
	 *  ||||||\--  \
	 *  |||||\---   \ 1818c phoneme select
	 *  ||||\----   /
	 *  |||\-----  /
	 *  ||\------ /
	 *  |\------- \ 1818c inflection select
	 *  \-------- /
	 */
	m_latchy = data;
	m_votrax->inflection_w((m_latchy&0xc0)>>6);
	m_votrax->write(m_latchy&0x3f);
}

uint8_t votrhv_state::input_r()
{
	/* input:
	 *  76543210
	 *  |||||||\- kbd decoded d0
	 *  ||||||\-- kbd decoded d1
	 *  |||||\--- kbd decoded d2
	 *  ||||\---- GND/unused
	 *  |||\----- /low_battery
	 *  ||\------ GND/unused
	 *  |\------- keyboard flipflop
	 *  \-------- phoneme flipflop
	 */
	uint8_t retval = m_rstflops;
	// now scan the currently selected column, emulating a CD4532 priority encoder where D7 beats D6 beats D5... etc
	uint8_t temp = m_swarray[m_latchx&0xf]->read();
	for (int i = 7; i >= 0; i--)
	{
		if ((temp>>i)&1)
		{
			retval |= i;
			break;
		}
	}
	//logerror("input_r read, returning value of %02x\n", retval);
	return retval;
}

uint8_t votrhv_state::latcha_rst_r()
{
	if(!machine().side_effects_disabled())
	{
		// reset the 0x80 flop
		m_rstflops &= ~0x80;
		// if both flops are now clear, assert reset on the cpu
		if (!m_rstflops)
			m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}
	return 0xff;
}

uint8_t votrhv_state::latchb_rst_r()
{
	if(!machine().side_effects_disabled())
	{
		// reset the 0x40 flop
		m_rstflops &= ~0x40;
		// if both flops are now clear, assert reset on the cpu
		if (!m_rstflops)
			m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}
	return 0xff;
}

/******************************************************************************
 Machine Drivers
******************************************************************************/

void votrhv_state::votrhv(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, 2'000'000 / 2 );  // ~1'000'000 done using two 74L123 multivibrators with cap 100pf res 11k, which each oscillate at 1.8-2.5mhz, but since you need two clock phases for the 6800, each multivibrator does one phase and the falling edge of one triggers the other, so the actual clock rate is half the rate of each.
	m_maincpu->set_addrmap(AS_PROGRAM, &votrhv_state::mem_map);

	/* video hardware */
	//config.set_default_layout(layout_votrhv);

	/* serial hardware */

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	// TEMPORARY HACK until 1818c device is done
	VOTRAX_SC01(config, m_votrax, 720000);
	m_votrax->ar_callback().set(FUNC(votrhv_state::pho_done));
	m_votrax->add_route(ALL_OUTPUTS, "mono", 1.00);
}



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(hc110)
	ROM_REGION(0x8000, "maincpu", 0) // 4x EA8316 (2316 equivalent) CMOS Mask ROMs and 2x 512x8 PROMs
	//ROM_LOAD("ea8316e030.bin", 0x4000, 0x0800, CRC(fd8cbf7d) SHA1(a2e1406c498a1821cacfcda254534f8e8d6b8260)) // used on older firmware?
	ROM_LOAD("ea8316e144.bin", 0x4000, 0x0800, CRC(636415ee) SHA1(9699ea75eed566447d8682f52665b01c1e876981))
	ROM_LOAD("ea8316e031.bin", 0x4800, 0x0800, CRC(f2de4e3b) SHA1(0cdc71a4d01d73e403cdf283c6eeb53f97ca5623))
	ROM_LOAD("ea8316e032.bin", 0x5000, 0x0800, CRC(5df1270c) SHA1(5c81fcb2bb2c0bf509aa9fc11a92071cd469e407))
	ROM_LOAD("ea8316e033.bin", 0x5800, 0x0800, CRC(0d7e246c) SHA1(1454c6c7ef3743320443c7bd1f37df6a25ff7795))
	ROM_LOAD("7031r2b.bin",    0x7c00, 0x0200, CRC(6ef744c9) SHA1(6a92e520adb3c47b849241648ec2ca4107edfd8f)) // REV B?
	ROM_LOAD("7031r3b.bin",    0x7e00, 0x0200, CRC(0800b0e6) SHA1(9e0481bf6c5feaf6506ac241a2baf83fb9342033)) // REV B?
ROM_END

ROM_START(hc120)
	ROM_REGION(0x8000, "maincpu", 0) // 4x EA8316 (2316 equivalent) CMOS Mask ROMs and 2x 512x8 PROMs
	ROM_LOAD("ea8316e030.bin", 0x4000, 0x0800, CRC(fd8cbf7d) SHA1(a2e1406c498a1821cacfcda254534f8e8d6b8260))
	ROM_LOAD("ea8316e031.bin", 0x4800, 0x0800, CRC(f2de4e3b) SHA1(0cdc71a4d01d73e403cdf283c6eeb53f97ca5623))
	ROM_LOAD("ea8316e032.bin", 0x5000, 0x0800, CRC(5df1270c) SHA1(5c81fcb2bb2c0bf509aa9fc11a92071cd469e407))
	ROM_LOAD("ea8316e033.bin", 0x5800, 0x0800, CRC(0d7e246c) SHA1(1454c6c7ef3743320443c7bd1f37df6a25ff7795))
	ROM_LOAD("7037r2.bin",     0x7c00, 0x0200, CRC(44de1bb1) SHA1(53e6811baf37af5da0648e906fee6c6acf259b82))
	ROM_LOAD("7037r3.bin",     0x7e00, 0x0200, CRC(688be8c7) SHA1(c9bdc7472cabcdddc23e63f45afbfcc835bb8f69))
ROM_END

/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE INPUT  CLASS         INIT        COMPANY   FULLNAME             FLAGS
COMP( 1980, hc110,   0,      0,      votrhv, hc110, votrhv_state, empty_init, "Votrax/Phonic Mirror", "HandiVoice HC-110", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1980, hc120,   hc110,  0,      votrhv, hc110, votrhv_state, empty_init, "Votrax/Phonic Mirror", "HandiVoice HC-120", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
