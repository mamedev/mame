// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
// thanks-to:Kevin Horton
/******************************************************************************
 *
 *  Votrax/Phonic Mirror HandiVoice models HC-110 and HC-120
 *
 *****************************************************************************/
/*
    The HC-110 and HC-120 both consist of 3 boards:
    1. An 'embedded system' 6800 board with two output latches, one input
       latch, and two 'resume/reset' latches which pull the 6800 is out of
       reset on a rising edge, as a form of power saving. (pcb 1816?)
    2. A keyboard handler pcb; this is different on the HC-110 (where it is
       made by a 3rd party company for the 128-key input) and the HC-120
       (where it was made by votrax/phonic mirror, pcb 1817?)
    3. The voice synthesizer pcb 1818c, encased in epoxy. It is a discrete
       synthesizer roughly equivalent to an SC-01 or VSK, it has external
       pins to allow control of speech pitch and rate in addition to the
       typical 2 inflection pins.

    Notes: Electronic Arrays, Inc. who made the EA8316 CMOS mask ROMs was
    bought out by NEC in 1978.

    TODO: 1818c discrete speech synth device

    The 1818C SYNTHESIZER BOARD is mentioned as one of two speech
    synthesizers described in US Patent 4,130,730 in figures 3, 4a and 4b
    (the Votrax VSK/VSL is the other device, described in figures 1, 2a,
    and 2b)
    The 1818C uses three Motorola MCM14524 256x4 CMOS MASK ROMs to hold
    the phoneme parameters.
    (This is mentioned in 4,130,730 column 11 line 31.)

    Motorola MCM14524:
                +---..---+
        /CLK -> |  1  16 | -- VDD (up to 18v)
          CE -> |  2  15 | <- A0
          B0 <- |  3  14 | <- A1
          B1 <- |  4  13 | <- A7
          B2 <- |  5  12 | <- A6
          B3 <- |  6  11 | <- A5
          A2 -> |  7  10 | <- A4
     (0v)VSS -- |  8   9 | <- A3
                +--------+
    see http://bitsavers.org/components/motorola/_dataBooks/1978_Motorola_CMOS_Data_Book.pdf
    page 488
*/

/* Core includes */
#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/input_merger.h"
#include "sound/votrax.h"
#include "speaker.h"

#include "hc110.lh"

// defines

#define LOG_INPUT     (1U << 1)
#define LOG_LATCHX    (1U << 2)
#define LOG_LATCHY    (1U << 3)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

namespace {

#define LOGINP(...) LOGMASKED(LOG_INPUT, __VA_ARGS__)
#define LOGLTX(...) LOGMASKED(LOG_LATCHX, __VA_ARGS__)
#define LOGLTY(...) LOGMASKED(LOG_LATCHY, __VA_ARGS__)

class votrhv_state : public driver_device
{
public:
	votrhv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_reset(*this, "reset")
		, m_latchx(0)
		, m_latchy(0)
		, m_latcha_flop(false)
		, m_latchb_flop(false)
		, m_latcha_in(false)
		, m_latchb_in(false)
		, m_scanflag(false)
		, m_maincpu(*this, "maincpu")
		, m_votrax(*this, "votrax")
		, m_swarray(*this, "SW.%u", 0U)
		, m_leds(*this, "led_%u", 0U)
	{ }

	void votrhv(machine_config &config);
	void hc110(machine_config &config);

	void reset_counter(int state);
	void key_pressed(int state);
	void pho_done(int state);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<input_merger_device> m_reset;

	uint8_t m_latchx;
	uint8_t m_latchy;
	bool m_latcha_flop;
	bool m_latchb_flop;
	bool m_latcha_in;
	bool m_latchb_in;
	bool m_scanflag;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<m6800_cpu_device> m_maincpu;
	required_device<votrax_sc01_device> m_votrax;

	TIMER_CALLBACK_MEMBER(resume_tick);

	optional_ioport_array<16> m_swarray;
	output_finder<5> m_leds;

	virtual void key_check();
	virtual void latchx_w(uint8_t data);
	void latchy_w(uint8_t data);
	virtual uint8_t input_r();
	uint8_t latcha_rst_r();
	uint8_t latchb_rst_r();

	emu_timer* m_resume_timer = nullptr;
};

class hc120_state : public votrhv_state
{
public:
	hc120_state(const machine_config &mconfig, device_type type, const char *tag)
		: votrhv_state(mconfig, type, tag)
	{ }

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void key_check() override;
	virtual uint8_t input_r() override;
	virtual void latchx_w(uint8_t data) override;

	TIMER_CALLBACK_MEMBER(scan_keys);

	emu_timer* m_scan_timer = nullptr;
};

/******************************************************************************
 Address Maps
******************************************************************************/

/* This memory map is for the CPU board which is shared between HC-110 and HC-120
  15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
   x  0  0  0    0  x  x  x    *  *  *  *    *  *  *  *    RW RAM (2x Harris MI-6561-9 256x4 SRAM, wired in parallel)
   x  0  0  0    1  x  x  x    x  x  x  x    x  x  x  x    open bus
   x  0  0  1    0  x  x  x    x  x  x  x    x  x  x  x    R Input Latch
   x  0  0  1    1  x  x  x    x  x  x  x    x  x  x  x    open bus
   x  0  1  0    0  x  x  x    x  x  x  x    x  x  x  x    W Latch X out
   x  0  1  0    1  x  x  x    x  x  x  x    x  x  x  x    open bus
   x  0  1  1    0  x  x  x    x  x  x  x    x  x  x  x    W Latch Y out
   x  0  1  1    1  x  x  x    x  x  x  x    x  x  x  x    open bus
   x  1  0  0    0  *  *  *    *  *  *  *    *  *  *  *    R ROM0
   x  1  0  0    1  *  *  *    *  *  *  *    *  *  *  *    R ROM1
   x  1  0  1    0  *  *  *    *  *  *  *    *  *  *  *    R ROM2
   x  1  0  1    1  *  *  *    *  *  *  *    *  *  *  *    R ROM3
   x  1  1  0    0  x  x  x    x  x  x  x    x  x  x  x    * Reset Latch A clear
   x  1  1  0    1  x  x  x    x  x  x  x    x  x  x  x    * Reset Latch B clear
   x  1  1  1    0  x  x  x    x  x  x  x    x  x  x  x    open bus
   x  1  1  1    1  0  0  x    x  x  x  x    x  x  x  x    open bus
   x  1  1  1    1  0  1  *    *  *  *  *    *  *  *  *    R PROM1 (unpopulated)
   x  1  1  1    1  1  0  *    *  *  *  *    *  *  *  *    R PROM2
   x  1  1  1    1  1  1  *    *  *  *  *    *  *  *  *    R PROM3
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
	map(0x4000, 0x5fff).mirror(0x8000).rom().region("maskrom", 0);
	map(0x6000, 0x6000).mirror(0x87ff).r(FUNC(votrhv_state::latcha_rst_r));
	map(0x6800, 0x6800).mirror(0x87ff).r(FUNC(votrhv_state::latchb_rst_r));
	// 7000-77ff open bus
	// 7800-79ff open bus
	map(0x7a00, 0x7fff).mirror(0x8000).rom().region("bootrom", 0);
}


/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START(hc110)
	PORT_START("SW.0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Talk
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Master Clear
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Clear
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Talk Repeat
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Level 1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Level 2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Level 3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // Level 4

	PORT_START("SW.1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.5")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.6")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.7")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.8")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.9")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.10")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.11")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.12")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.13")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.14")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)

	PORT_START("SW.15")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
INPUT_PORTS_END

static INPUT_PORTS_START(hc120)
	PORT_START("KEYPAD")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M CLR") PORT_CODE(KEYCODE_SLASH_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CLR") PORT_CODE(KEYCODE_DEL_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SCAN") PORT_CODE(KEYCODE_ASTERISK) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed) // some units have this button labeled "SCROLL" instead of "SCAN"
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TALK REPT.") PORT_CODE(KEYCODE_MINUS_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TALK") PORT_CODE(KEYCODE_PLUS_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, votrhv_state, key_pressed)
INPUT_PORTS_END

/******************************************************************************
 Timer and machine/start/reset handlers
******************************************************************************/

TIMER_CALLBACK_MEMBER(votrhv_state::resume_tick)
{
	// pull the cpu out of reset
	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(hc120_state::scan_keys)
{
	// invert the scan state bit
	m_scanflag = !m_scanflag;
	m_scan_timer->adjust(attotime::from_seconds(1));
}

void votrhv_state::machine_start()
{
	m_resume_timer = timer_alloc(FUNC(votrhv_state::resume_tick), this);

	m_leds.resolve();

	save_item(NAME(m_latchx));
	save_item(NAME(m_latchy));
	save_item(NAME(m_latcha_flop));
	save_item(NAME(m_latchb_flop));
	save_item(NAME(m_latcha_in));
	save_item(NAME(m_latchb_in));
	save_item(NAME(m_scanflag));
}

void votrhv_state::machine_reset()
{
	m_resume_timer->adjust(attotime::never);
}

// hc-120 variant for scan_timer
void hc120_state::machine_start()
{
	votrhv_state::machine_start();

	m_scan_timer = timer_alloc(FUNC(hc120_state::scan_keys), this);
}

void hc120_state::machine_reset()
{
	m_scan_timer->adjust(attotime::from_seconds(1)); // hc-120 specific; adjustable, guessed 1hz to 2hz? needs measurement
}

/******************************************************************************
 Driver specific functions
******************************************************************************/

void votrhv_state::reset_counter(int state)
{
	if (state == CLEAR_LINE)
	{
		// if the timer is not already running, start it.
		if (m_resume_timer->remaining().is_never())
		{
			m_resume_timer->adjust(attotime::from_hz(2'000'000/2/0x20));
		}
	}
	else // state == ASSERT_LINE
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}
}

void votrhv_state::key_pressed(int state)
{
	// If we got called here, a key was pressed or released somewhere on the keyboard, and we don't know where.
	// If, regardless of whether a key was pressed or released, our currently selected column has a non-zero
	// number of keys closed in it, we need to assert m_latchb_in and if its edge was rising with that assertion, set m_latchb_flop.
	LOGINP("key %s event\n", state?"down":"up");
	key_check();
}

void votrhv_state::pho_done(int state)
{
	bool rising_edge = (!m_latcha_in && (state == ASSERT_LINE));
	m_latcha_in = (state == ASSERT_LINE);
	if (rising_edge) // HACK: SC-01-A is in /ready state now
	{
		// clock the pho_done flipflop which sets its state to 1
		m_latcha_flop = true;
		// HACK: we manually clock the same old phoneme into the SC-01-A here, this is a hack since on the 1818C the pho_done signal is a free running counter and doesn't stop like the A/R latch on the SC-01[-A] does
		//m_votrax->inflection_w((m_latchy&0xc0)>>6);
		m_votrax->write(m_latchy&0x3f);
		m_reset->in_w<0>(1);
	}
}

void votrhv_state::key_check()
{
	uint8_t keys = m_swarray[m_latchx&0xf]->read();
	bool m_latchb_in_old = m_latchb_in;
	m_latchb_in = (keys == 0); // If the currently selected column has a non-zero number of keys set in it, the latchb_in state is false (KPRESS is active LOW), otherwise true
	if (!m_latchb_in_old && m_latchb_in) // on the rising edge of m_latchb_in, set the flipflop
	{
		// clock the key flipflop which sets its state to 1
		m_latchb_flop = true;
		m_reset->in_w<1>(1);
	}
	LOGINP("column %x keys read as %02x, latchb_in was %d now %d, latchb flop is %d\n", m_latchx&0xf, keys, m_latchb_in_old, m_latchb_in, m_latchb_flop);
}


void votrhv_state::latchx_w(uint8_t data)
{
	/* latchx output:
	 *  76543210
	 *  |||||||\- \.
	 *  ||||||\--  \ key input column select
	 *  |||||\---  /
	 *  ||||\---- /
	 *  |||\----- \ LED select 1/2/3/4
	 *  ||\------ /
	 *  |\------- Green status LED
	 *  \-------- Phoneme silence (ties pitch input of 1818C high thru a diode)
	 */
	m_latchx = data;
	LOGLTX("latchx written with value of %02x\n", m_latchx);
	for (int i = 0; i < 4; i++)
	{
		m_leds[i] = (((data >> 4) & 0x03) == (3 - i)) ? 1 : 0;
	}
	m_leds[4] = !BIT(data, 6);
	key_check();
}

void votrhv_state::latchy_w(uint8_t data)
{
	/* latchy output:
	 *  76543210
	 *  |||||||\- \.
	 *  ||||||\--  \.
	 *  |||||\---   \ 1818c phoneme select
	 *  ||||\----   /
	 *  |||\-----  /
	 *  ||\------ /
	 *  |\------- \ 1818c inflection select
	 *  \-------- /
	 */
	m_latchy = data;
	LOGLTY("latchy written with value of %02x\n", m_latchy);
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
	uint8_t retval = 0;
	// now scan the currently selected column, emulating a CD4532 priority encoder where D7 beats D6 beats D5... etc
	uint8_t temp = m_swarray[m_latchx&0xf]->read();
	for (int i = 7; i >= 0; i--)
	{
		if (BIT(temp,i))
		{
			retval |= i;
			break;
		}
	}
	retval |= (m_latcha_flop?0x80:0x00) | (m_latchb_in?0x40:0x00) | 0x10;
	LOGINP("input_r read with latchx column set to %01x, returning value of %02x\n", m_latchx&0xf, retval);
	return retval;
}

uint8_t votrhv_state::latcha_rst_r()
{
	if(!machine().side_effects_disabled())
	{
		// reset the 0x80 flop
		m_latcha_flop = false;
		m_reset->in_w<0>(0);
	}
	return 0xff;
}

uint8_t votrhv_state::latchb_rst_r()
{
	if(!machine().side_effects_disabled())
	{
		// reset the 0x40 flop
		m_latchb_flop = false;
		m_reset->in_w<1>(0);
	}
	return 0xff;
}


// hc120 specific overrides
void hc120_state::key_check()
{
	uint16_t keys = ioport("KEYPAD")->read();
	bool m_latchb_in_old = m_latchb_in;
	m_latchb_in = (keys != 0); // If the input has a non-zero number of keys set in it, the latchb_in state is TRUE, on the HC-120.
	if (!m_latchb_in_old && m_latchb_in) // on the rising edge of m_latchb_in, set the flipflop
	{
		// clock the key flipflop which sets its state to 1
		m_latchb_flop = true;
		m_reset->in_w<1>(1);
	}
	LOGINP("keys read as %02x, latchb_in was %d now %d, latchb flop is %d\n", keys, m_latchb_in_old, m_latchb_in, m_latchb_flop);
}

void hc120_state::latchx_w(uint8_t data)
{
	/* latchx output:
	 *  76543210
	 *  |||||||\- LCD digit sel d0
	 *  ||||||\-- LCD digit sel d1
	 *  |||||\--- LCD digit sel d2
	 *  ||||\---- LCD digit sel d3
	 *  |||\----- LCD digit pos1 en
	 *  ||\------ LCD digit pos2 en
	 *  |\------- LCD digit pos3 en
	 *  \-------- LCD latch 4bit digitsel for extra io (colon and other segments on d2 d3)
	 */
	m_latchx = data;
	LOGLTX("latchx written with value of %02x\n", m_latchx);
}

uint8_t hc120_state::input_r()
{
	/* input:
	 *  76543210
	 *  |||||||\- kbd decoded d0
	 *  ||||||\-- kbd decoded d1
	 *  |||||\--- kbd decoded d2
	 *  ||||\---- kbd decoded d3
	 *  |||\----- /low_battery
	 *  ||\------ scan rate oscillator input
	 *  |\------- keyboard flipflop
	 *  \-------- phoneme flipflop
	 */
	uint8_t retval = 0x0f;
	// now scan the currently selected column, emulating a Harris HD-0165
	// keyboard encoder including its weird inverted outputs which presumably
	// mask each other when multiple keys are pressed (see the note on the
	// datasheet in the 1975 Harris Integrated Circuits catalog about
	// "Erroneous Data" when the /KRO output is active
	uint16_t temp = ioport("KEYPAD")->read();
	for (int i = 0; i < 16; i++)
	{
		if (BIT(temp,i))
		{
			retval &= (15-i);
			break; // HACK: this is wrong for the HD-0165, which produces mangled data if more than one key is pressed simultaneously; later HC-120 units had a grid/collimator 'guard' on top of the keypad to reduce the chance of multiple keys being hit. Here we just give priority to the lowest bit active.
		}
	}
	retval |= (m_latcha_flop?0x80:0x00) | (m_latchb_in?0x40:0x00) | (m_scanflag?0x20:0x00) | 0x10;
	LOGINP("input_r read, returning value of %02x\n", retval);
	return retval;
}

/******************************************************************************
 Machine Drivers
******************************************************************************/

void votrhv_state::votrhv(machine_config &config)
{
	/* basic machine hardware */
	// ~1MHz done using two 74L123 multivibrators with cap 100pf res 11k, which each oscillate at 1.8-2.5mhz
	// since you need two clock phases for the 6800, each multivibrator does one phase and the falling edge of one triggers the other, so the actual clock rate is half the rate of each
	M6800(config, m_maincpu, 2'000'000 / 2 );
	m_maincpu->set_addrmap(AS_PROGRAM, &votrhv_state::mem_map);

	INPUT_MERGER_ALL_LOW(config, m_reset).output_handler().set(FUNC(votrhv_state::reset_counter));

	/* video hardware */
	//config.set_default_layout(layout_votrhv);

	/* serial hardware */

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	// TEMPORARY HACK until 1818c device is done
	VOTRAX_SC01A(config, m_votrax, 720000);
	m_votrax->ar_callback().set(FUNC(votrhv_state::pho_done));
	m_votrax->add_route(ALL_OUTPUTS, "mono", 1.00);
}

void votrhv_state::hc110(machine_config &config)
{
	votrhv(config);

	config.set_default_layout(layout_hc110);
}


/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(hc110)
	ROM_REGION(0x8000, "maskrom", 0) // 4x EA8316 (2316 equivalent) CMOS Mask ROMs
	//ROM_LOAD("ea8316e030.ic9", 0x0000, 0x0800, CRC(fd8cbf7d) SHA1(a2e1406c498a1821cacfcda254534f8e8d6b8260)) // used on older firmware?
	ROM_LOAD("ea8316e144.ic9", 0x0000, 0x0800, CRC(636415ee) SHA1(9699ea75eed566447d8682f52665b01c1e876981))
	ROM_LOAD("ea8316e031.ic8", 0x0800, 0x0800, CRC(f2de4e3b) SHA1(0cdc71a4d01d73e403cdf283c6eeb53f97ca5623))
	ROM_LOAD("ea8316e032.ic7", 0x1000, 0x0800, CRC(5df1270c) SHA1(5c81fcb2bb2c0bf509aa9fc11a92071cd469e407))
	ROM_LOAD("ea8316e033.ic6", 0x1800, 0x0800, CRC(0d7e246c) SHA1(1454c6c7ef3743320443c7bd1f37df6a25ff7795))

	ROM_REGION(0x0600, "bootrom", 0) // 2x 512x8 SN74S472 PROMs
	// ic12 is unpopulated
	ROM_LOAD("7031r2.sn74s472.ic11", 0x0200, 0x0200, CRC(6ef744c9) SHA1(6a92e520adb3c47b849241648ec2ca4107edfd8f))
	ROM_LOAD("7031r3.sn74s472.ic10", 0x0400, 0x0200, CRC(0800b0e6) SHA1(9e0481bf6c5feaf6506ac241a2baf83fb9342033))

	ROM_REGION16_BE(0x200, "s1818c", 0) // 1818C SYNTHESIZER BOARD; MCM14524 CMOS 256x4 mask ROMs holding the phoneme data
	ROMX_LOAD("scm46109pk.mcm14524.u30", 0x000, 0x100, CRC(6a1292fe) SHA1(67c8ac71e22de134a651dd5cad06d26b27894154), ROM_SKIP(1) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO)
	ROMX_LOAD("scm46110pk.mcm14524.u23", 0x001, 0x100, CRC(edc7bf31) SHA1(24a585b67ce246de5ef6c2cc024d91052b473816), ROM_SKIP(1) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI)
	ROMX_LOAD("scm46111pk.mcm14524.u20", 0x001, 0x100, CRC(4227f04e) SHA1(4393b0a9960cbae7768a27b32688f640e822f13e), ROM_SKIP(1) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO)
ROM_END

ROM_START(hc120) // ic10 and ic11 are Rev B? is there an older revision undumped?
	ROM_REGION(0x2000, "maskrom", 0) // 4x EA8316 (2316 equivalent) CMOS Mask ROMs
	ROM_LOAD("ea8316e030.ic9", 0x0000, 0x0800, CRC(fd8cbf7d) SHA1(a2e1406c498a1821cacfcda254534f8e8d6b8260))
	ROM_LOAD("ea8316e031.ic8", 0x0800, 0x0800, CRC(f2de4e3b) SHA1(0cdc71a4d01d73e403cdf283c6eeb53f97ca5623))
	ROM_LOAD("ea8316e032.ic7", 0x1000, 0x0800, CRC(5df1270c) SHA1(5c81fcb2bb2c0bf509aa9fc11a92071cd469e407))
	ROM_LOAD("ea8316e033.ic6", 0x1800, 0x0800, CRC(0d7e246c) SHA1(1454c6c7ef3743320443c7bd1f37df6a25ff7795))

	ROM_REGION(0x0600, "bootrom", 0) // 2x 512x8 SN74S472 PROMs
	// ic12 is unpopulated
	ROM_LOAD("7037__r2b.sn74s472.ic11", 0x0200, 0x0200, CRC(44de1bb1) SHA1(53e6811baf37af5da0648e906fee6c6acf259b82))
	ROM_LOAD("7037__r3b.sn74s472.ic10", 0x0400, 0x0200, CRC(688be8c7) SHA1(c9bdc7472cabcdddc23e63f45afbfcc835bb8f69))

	ROM_REGION16_BE(0x200, "s1818c", 0) // 1818C SYNTHESIZER BOARD; MCM14524 CMOS 256x4 mask ROMs holding the phoneme data
	ROMX_LOAD("scm46109pk.mcm14524.u30", 0x000, 0x100, CRC(6a1292fe) SHA1(67c8ac71e22de134a651dd5cad06d26b27894154), ROM_SKIP(1) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO)
	ROMX_LOAD("scm46110pk.mcm14524.u23", 0x001, 0x100, CRC(edc7bf31) SHA1(24a585b67ce246de5ef6c2cc024d91052b473816), ROM_SKIP(1) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI)
	ROMX_LOAD("scm46111pk.mcm14524.u20", 0x001, 0x100, CRC(4227f04e) SHA1(4393b0a9960cbae7768a27b32688f640e822f13e), ROM_SKIP(1) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO)
ROM_END

} // anonymous namespace

/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE INPUT  CLASS         INIT        COMPANY                 FULLNAME             FLAGS
COMP( 1978, hc110,   0,      0,      hc110,  hc110, votrhv_state, empty_init, "Votrax/Phonic Mirror", "HandiVoice HC-110", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1978, hc120,   hc110,  0,      votrhv, hc120, hc120_state,  empty_init, "Votrax/Phonic Mirror", "HandiVoice HC-120", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
