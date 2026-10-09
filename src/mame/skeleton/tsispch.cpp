// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
// thanks-to:Kevin Horton
/******************************************************************************
*
*  Telesensory Systems Inc./Speech Plus 2000 series standalone
*  Prose 2000/2020
*
*  The Prose 2000 card is an IEEE 796 Multibus card, with additional connectors to facilitate power and serial input other than via multibus.
*  There are two hardware versions of the card:
*  - The (C)1981 Telesensory Systems Inc version
*    (has some rework on the power input to add a bypass capacitor and an extra power line to the 8086)
*  - The (C)1986 Speech Plus version
*    (integrates the greenwire fixes from above, adds U82 for (unused) ROM banking, rams are 6164 instead of 6116, slightly faster CPU Xtal)
*
*  Both versions encountered have been in non-multibus enclosures:
*  - The 1981 Version appeared on a 'Voice V4' Speech board scrapped from a "Kurzweil Reading Machine" Talking Scanner (predecessor to the TSI/Kurzweil/Xerox 'Reading Edge' scanner which is SPARC based)
*  - The 1986 Version appeared in a 'Prose 2020' under-monitor RS232 speech unit.

*  The multibus version of the card will have jumpers present setting the multibus interrupts,
*   and will be missing the power connector at J4
*
*  Differences between the 1981 (TSI) and 1986 (Speech Plus) PCBs:
*      U6 changed to HCT version (74xx273)
*      U7, 35 changed to HCT version (74xx04)
*      U8, 10, 11, 12, 52, 53, 54 changed to HCT version (74xx163)
*      U19, 38, 39 changed to HCT version (74xx373)
*      U42 changed to AS version (74xx04)
*      U55, 56 changed to UPB8284
*      U57 changed to HCT version (74xx244)
*      U58 changed to HCT version (74xx32)
*      U67 changed from 54368 to 74368
*      U82 added (74HCT273)
*      pin 1 (VPP) of U29 (DSP) connected to VCC
*      C28 changed from 5nF to 22nF
*      JP2-7 removed and jumped permanently
*      J26 removed and jumped permanently
*      C5 changed from 2.2uF to 0.1uF
*      C7 changed from 10nF to 47nF
*      CR7,8 changed from 1N4004 to 1N4005
*      S1 (reset) moved to top edge of PCB, and changed to right angle
*      Y2 changed from 23.04MHz to 24MHz
*      R4 added (unused designator)
*      R12 removed from CPU and moved
*      R4/12 both are on crystal Y2 now
*      C57 removed from crystal Y2
*      R27/28 removed from CPU and moved
*      R27/28 both are on crystal Y1 now
*      C58 removed from crystal Y1
*      C57 is now a bypass capacitor on the added U82
*      U58 section B is used now for the new chip U82
*      U79 A2 input changed from A14 to A15
*      U81A's select lines connected to the new chip U82 which appears to be bank selection
*      U81B's input lines changed to support entire address range
*      U21-28, 44-51 changed from 2764 to 27512 (there seems to be cuttable jumpers to change but the default is 27512)
*      U61-66 changed from 6116-ish to 6264
*
*
*  notes about the mapping PROMs:
*
*  All are am27s19 32x8 TriState PROMs (equivalent to 82s123/6331)
*  L - always low; H - always high
*  U77: Waitstate selector for 8086
*       input is A19 for I4, A18 for I3, A15 for I2, A13 for I1, A12 for I0
*       output bits 0bLLLLzyxH
*       bit - function
*       7,6,5,4 - NC
*       3,2,1,0 - Preload value for a counter for generating the RDY1 signal to the
*                  8284A clocking and generating the READY waitstate pin for the 8086
*                 Note there is a hardware bug here that the incorrect READY length
*                  will be applied when an interrupt on the 8086 happens!
*
*  U79: SRAM and peripheral mapping:
*       input is A19 for I4, A18 for I3, A15 for I2, A13 for I1, A12 for I0, same as U77
*       On the Prose 2000 later board dumped, only bits 3 and 0 are used;
*       bits 7-4 are always low, bits 2 and 1 are always high.
*       SRAMS are only populated in U61 and U64.
*       On the Prose 2000 earlier board dumped, bits 3,2,1,0 are all used;
*       bits 7-4 are always low. SRAM is in 6 6116s, mapped the same as the 2 6264s on the later board.
*       output bits 0bLLLL3210
*       7,6,5,4 - NC
*       3 - to /EN3 (pin 4) of 74S138N at U80
*           AND to EN1 (pin 6) of 74S138N at U78
*           i.e. one is activated when pin is high and other when pin is low
*           The 74S138N at U80: [*ENABLED ONLY WITHIN/CONTROLS THE 3000-3FFF AREA*]
*               /EN2 (pin 5) - pulled to GND
*               EN1 (pin 6) - pulled to VCC through resistor R5
*               inputs: S0 - A9; S1 - A10; S2 - A11
*               /Y0 - /CS (pin 11) of iP8251A at U15 [0x3000-0x31FF]
*               /Y1 - /CS (pin 1) of AMD 8259A at U4 [0x3200-0x33FF]
*               /Y2 - pins 1, 4, 9 (1A, 2A, 3A inputs) of 74HCT32 Quad OR gate at U58 [0x3400-0x35FF]
*               /Y3 - pin 26 (/CS) of UPD77P20 at U29 [0x3600-0x37FF]
*               /Y4 through /Y7 - seem unconnected? [0x3800-0x3FFF]
*           The 74S138N at U78: [*ENABLED IN ALL AREAS EXCEPT 3000-3FFF*] <wip>
*               /EN3 (pin 4) - /DEN_Q
*               /EN2 (pin 5) - /MEM
*               inputs: S0 - A18; S1 - A19; S2 - Pulled to GND
*               /Y0 - /RAM_OE for the 6116 or 6264 SRAMs
*               /Y1 - NC
*               /Y2 - NC
*               /Y3 - /ROM_OE for the EPROMS
*               /Y4,/Y5,/Y6,/Y7 - NC
*       2 - to /CS1 on 6116 or 6264 SRAMs at U63 and U66
*       1 - to /CS1 on 6116 or 6264 SRAMs at U62 and U65
*       0 - to /CS1 on 6116 or 6264 SRAMs at U61 and U64
*
*  U81B: (OPTIONAL) maps ROMS:
*       On older TSI hardware, input is A19, A17, A16, A15, A14 for I4,3,2,1,0
*        however, on this hardware, U81 is not present or used.
*       On newer Speech plus hardware, input is A19-A15 for I4,3,2,1,0
*       On the newer hardware dumps we have, only bits 6 and 5 are used,
*        the rest are always high; maps ROMs 0,1,2,3 to C0000-FFFFF.
*       The newer hardware has empty unpopulated sockets for ROMs 4-15;
*        if present these would be driven by a different PROM in this location.
*       bit - function
*        7 - to /CE of ROMs 14(U28) and 15(U51) (unused)
*        6 - to /CE of ROMs 0(U21) and 1(U44)   (0xE0000-0xFFFFF)
*        5 - to /CE of ROMs 2(U22) and 3(U45)   (0xC0000-0xDFFFF)
*        4 - to /CE of ROMs 4(U23) and 5(U46)   (unused)
*        3 - to /CE of ROMs 6(U24) and 7(U47)   (unused)
*        2 - to /CE of ROMs 8(U25) and 9(U48)   (unused)
*        1 - to /CE of ROMs 10(U26) and 11(U49) (unused)
*        0 - to /CE of ROMs 12(U27) and 13(U50) (unused)
*
*  Note U81B is optional; it can be replaced by a 74s138 (U81A) instead of a PROM.
*       On older TSI hardware, the inputs are A19-A17, for decoding the ROMs as:
*       bit - function
*        7 - to /CE of ROMs 0(U21) and 1(U44)   (0xE0000-0xE3FFF, unused)
*        6 - to /CE of ROMs 2(U22) and 3(U45)   (0xE4000-0xE7FFF, unused)
*        5 - to /CE of ROMs 4(U23) and 5(U46)   (0xE8000-0xEBFFF, unused)
*        4 - to /CE of ROMs 6(U24) and 7(U47)   (0xEC000-0xEFFFF)
*        3 - to /CE of ROMs 8(U25) and 9(U48)   (0xF0000-0xF3FFF)
*        2 - to /CE of ROMs 10(U26) and 11(U49) (0xF4000-0xF7FFF)
*        1 - to /CE of ROMs 12(U27) and 13(U50) (0xF8000-0xFBFFF)
*        0 - to /CE of ROMs 14(U28) and 15(U51) (0xFC000-0xFFFFF)
*       On newer Speech Plus hardware, the inputs are the three banking bits
*        (bits 7, 6, and 0 for C,B,A) from the 74HCT273@U82 to select the same
*        ROMs per bit:
*       bit - function
*        7 - to /CE of ROMs 0(U21) and 1(U44)
*        6 - to /CE of ROMs 2(U22) and 3(U45)
*        5 - to /CE of ROMs 4(U23) and 5(U46)
*        4 - to /CE of ROMs 6(U24) and 7(U47)
*        3 - to /CE of ROMs 8(U25) and 9(U48)
*        2 - to /CE of ROMs 10(U26) and 11(U49)
*        1 - to /CE of ROMs 12(U27) and 13(U50)
*        0 - to /CE of ROMs 14(U28) and 15(U51)
*       However, U81B is not populated so this banking functionality is not used.
*
*  DONE:
*  Skeleton Written
*  Load cpu and dsp ROMs and mapper PROMs
*  Successful compile
*  Successful run
*  Correctly Interleave 8086 CPU ROMs
*  Debug LEDs hooked to popmessage
*  Attached i8251a uart at u15
*  Added dipswitch array S4
*  Attached 8259 PIC
   * IR0 = upd7720 p0 pin, inverted
   * IR1 = i8251 rxrdy
   * IR2 = i8251 txempty
   * IR3 = i8251 txrdy
   * IR4,5,6,7 = NC
*  Hooked the terminal to the i8251a uart at u15
*  Hooked up upd7720 reset line
*  Verified CPU and DSP clocks
*  UPD7720: hook up serial output and SCK, and hook SO to the DAC; this requires fixing the upd7725 core to actually support SCK and serial output/SO!
*  The other i8251a uart is connected via multibus, just ignore it for now.

*  TODO:
*  Add other dipswitches (multibus related)
*  Make a multibus peripheral device which emulates solely the second 8751 UART and connects its serial
*   output to the main UART here. This is where most of the dipswitches and jumpers come into play.
*
*    When the unit is idle, leds 5 and 3 are on and upd7720 reset is low (write of 0b?1?0101?.
*    On all character writes from i8251, bit 8 is unset, then set again, possibly to avoid interrupt clashes?
*
*  Bootup notes v3.4.1:
*    D3109: checks if 0x80 (S4-8) is set: if set, continue, else jump to D3123
*    D3123: write 0x1C (0 0 0 [1 1 1 0] 0) to 3401
*    then jump to D32B0
*      D32B0: memory test routine:
*        This routine flood-fills memory from 0000-2FFF with 0xFF,
*        then, bytewise starting from 0000, shifts the value progresively
*        right by one, writes it and checks that it still matches,
*        i.e. read 0xFF, write 0x7f, read 0x7f, write 0x3f... etc.
*        Loop at D32E4.
*      D32E6: similar to D32B0, but rotates in 1 bits to 16 bit words,
*        though only the low byte is written, and only fills the 2BFF
*        down to 0000 region. (seems rather redundant, actually)
*        Loop at D3301.
*      D3311: write 0x0A (0 0 0 [0 1 0 1] 0) to 3401
*      then jump to D3330
*      D3330: jump back to D312E
*    D312E: this is some unknown conditional code, usually goes to D314E
*      if BP is not 1, go to D314E and don't update leds (usually taken?)
*      if BP is 1 and SI is 0, delay for 8*65536 cycles. no delay if si!=0
*      write 0x0C (0 0 0 [0 1 1 0] 0) to 3401
*    D314E: floodfill 0000-2BFF with 0x55 (rep at D315C)
*      check if bp was 1 and jump to D318F if it was
*      write 0x14 (0 0 0 [1 0 1 0] 0) to 3401
*      call E3987: initialize UPD7720, return
*    D33D2: checksum the ROMs in 5? passes, loop at D33DA, test at D33E6 (which passes)
*      if test DID fail: write 0x10 (0 0 0 [1 0 0 0] 0) to 3401
*        more stuff
*        write 0xFF to 3401
*        more stuff
*        set up word table? not sure what its doing here...
*      if test does NOT fail (and it doesn't):
*        D3414: write 0x08 (0 0 0 [0 1 0 0] 0) to 3400
*    D5E14: initialize PIC8259
*    <more stuff, wip>
*    D338A: write 0x12 0 0 0 [1 0 0 1] 0 to 3401
*
*  bpset d3414 to get past the memory test and rom checksum tests
*
*  F44B4: general in-operation LED status write
******************************************************************************/

/* Core includes */
#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "cpu/upd7725/upd7720.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/pic8259.h"
#include "machine/rescap.h"
#include "sound/dac.h"
#include "sound/flt_biquad.h"
#include "speaker.h"

// we need the M_SQRT2 constant
#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

// defines

#define LOG_PARAM     (1U << 1)
#define LOG_DSP       (1U << 2)
#define LOG_P01       (1U << 3)

//define VERBOSE (LOG_GENERAL | LOG_PARAM | LOG_DSP | LOG_P01)
#define VERBOSE (LOG_GENERAL)
//define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

// Hack: use PCM54HP 16-bit DAC instead of the correct AM6012 or UPC6012 12-bit DAC
#undef TOO_ACCURATE

namespace {

#define LOGPRM(...) LOGMASKED(LOG_PARAM, __VA_ARGS__)
#define LOGDSP(...) LOGMASKED(LOG_DSP, __VA_ARGS__)
#define LOGP01(...) LOGMASKED(LOG_P01, __VA_ARGS__)

// class definition
class tsispch_state : public driver_device
{
public:
	tsispch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dsp(*this, "dsp")
		, m_pic(*this, "pic8259")
		, m_uart(*this, "i8251a_u15")
		, m_dac(*this, "dac")
		, m_rs232(*this, "rs232")
		, m_prefilter(*this, "prefilter")
		, m_efilter(*this, "efilter%u", 0U)
	{
	}

	void prose2k(machine_config &config);

private:
	void serial_dcd_w(int state);
	uint16_t dsw_r();
	void peripheral_w(uint16_t data);
	uint16_t dsp_data_r();
	void dsp_data_w(uint16_t data);
	uint16_t dsp_status_r();
	void dsp_so16_cb(uint16_t data);
	void dsp_to_8086_p1_w(int state);
	TIMER_CALLBACK_MEMBER(dsp_int_timer_cb);
	emu_timer *m_dsp_int_timer = nullptr;

	void i8086_mem(address_map &map) ATTR_COLD;
	void i8086_io(address_map &map) ATTR_COLD;
	void dsp_prg_map(address_map &map) ATTR_COLD;
	void dsp_data_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<upd7720_device> m_dsp;
	required_device<pic8259_device> m_pic;
	required_device<i8251_device> m_uart;
	required_device<dac_word_interface> m_dac;
	required_device<rs232_port_device> m_rs232;
	required_device<filter_biquad_device> m_prefilter;
	required_device_array<filter_biquad_device, 4> m_efilter;

	//uint16_t m_dac_shifter = 0;
	uint16_t m_dac_latch = 0;
	int m_dsp_int_level = CLEAR_LINE;
	bool m_serial_dcd = true;
	uint8_t m_paramReg = 0;           // status leds and resets and etc

};

/*
   Devices and handlers
 */


/*****************************************************************************
 peripheral (LED/dipswitch/serial DCD/etc) stuff
*****************************************************************************/
void tsispch_state::serial_dcd_w(int state)
{
	logerror("dcd state written with %d\n", state);
	m_serial_dcd = state?false:true; // inverted because +12v = FALSE/SPACE state
}

uint16_t tsispch_state::dsw_r()
{
	return (ioport("s4")->read()&0x7f)|(m_serial_dcd?0x80:0);
}

void tsispch_state::peripheral_w(uint16_t data)
{
	/* peripheral_w bits
	fedcba9876543210
	||||||||||xxxxx|- bits 54321 are NC
	||||||||\\-----\- ROM banking bits (unused, newer hardware only)
	|||||||\--------- debug leftover bit, written to but unused, NC
	|||\\\\---------- LEDs, in order CR3,CR4,CR5,CR6 for bits 12, 11, 10, 9; low = on, high = off.
	||\-------------- USEPOT: if high, vol = (int pot || ext pot), if low, ext pot only. ignored if R1(int pot) is unpopulated.
	|\--------------- /DSP_RST, is run through an inverter to the active-high RST pin on the upd7720
	\---------------- debug leftover bit, written to but unused, NC
	*/
	m_paramReg = data;
	m_dsp->set_input_line(INPUT_LINE_RESET, BIT(data,0xe)?CLEAR_LINE:ASSERT_LINE);
	LOGPRM("8086: Parameter Reg written: UNK7: %d, DSPRST6: %d; VOLPOT: %d; LED4: %d; LED3: %d; LED2: %d; LED1: %d; UNK0: %d, lowbyte:%02x\n", BIT(data,0xf), BIT(data,0xe), BIT(data,0xd), BIT(data,0xc), BIT(data,0xb), BIT(data,0xa), BIT(data,0x9), BIT(data,0x8), data&0xff);
}

/*****************************************************************************
 UPD77P20 stuff
 TODO: The data_r, data_w and status_r can be plumbed past these trampolines
       entirely, at the cost of being unable to trace the cpu<->dsp comms.
*****************************************************************************/
uint16_t tsispch_state::dsp_data_r()
{
	uint8_t r = m_dsp->data_r();
	LOGDSP("dsp data read: %02x\n", r);
	return r;
}

void tsispch_state::dsp_data_w(uint16_t data)
{
	LOGDSP("dsp data write: %02x\n", data);
	m_dsp->data_w(data);
}

uint16_t tsispch_state::dsp_status_r()
{
	uint8_t r = m_dsp->status_r();
	LOGDSP("dsp status read: %02x\n", r);
	return r;
}

// TODO: this is a bit of a hack, until SOACK/SCLK/etc is emulated properly
void tsispch_state::dsp_so16_cb(uint16_t data)
{
	m_dac_latch = data;
}

TIMER_CALLBACK_MEMBER(tsispch_state::dsp_int_timer_cb)
{
	// writing the DSP INT pin at 10khz also latches the state of the external serial shifter to the DAC inputs on the rising edge
	if (m_dsp_int_level == ASSERT_LINE)
	{
		m_dsp_int_level = CLEAR_LINE;
	}
	else // m_dsp_int_level == CLEAR_LINE
	{
		m_dsp_int_level = ASSERT_LINE;
		//LOGMASKED(LOG_GENERAL,"dsp output raw is %04x\n", m_dac_latch);
#ifndef TOO_ACCURATE
		m_dac->write((m_dac_latch & 0x1ffe) >> 1); // DSP_SO taps bits 12 to 1; bits 15, 14, 13 and 0 are unconnected. bit 0 is used, just unconnected.
#else
		m_dac->write(((m_dac_latch & 0x1fff) << 3) | ((dsp_so & 0x1c00) >> 10));
#endif
	}
	m_dsp->set_input_line(UPD7720_INPUT_LINE_INT, m_dsp_int_level);
	m_dsp_int_timer->adjust(attotime::from_hz(20000));
}

/*****************************************************************************
 Reset and Driver Init
*****************************************************************************/
void tsispch_state::machine_start()
{
	m_dsp_int_timer = timer_alloc(FUNC(tsispch_state::dsp_int_timer_cb), this);
	m_dsp_int_timer->adjust(attotime::from_hz(20000)); // 10khz clock to dsp int pin, both edges
	//save_item(NAME(m_dac_shifter));
	save_item(NAME(m_dac_latch));
	save_item(NAME(m_dsp_int_level));
	save_item(NAME(m_serial_dcd));
	save_item(NAME(m_paramReg));

	m_dsp_int_level = CLEAR_LINE;
	m_dsp->set_input_line(UPD7720_INPUT_LINE_INT, CLEAR_LINE);
}

void tsispch_state::machine_reset()
{
	m_paramReg = 0x00; // on power up, all leds on, reset to upd7720 is high
	peripheral_w(m_paramReg);
}

/******************************************************************************
 Address Maps
******************************************************************************/
/* The address map of the prose 2020 is controlled by 2 PROMs, see the ROM section
   for details on those.
   Everything here is verified from tracing out the boards and handmade schematics.
   (x = ignored; * = selects address within this range; s = selects one of a pair of chips)
   A19 A18 A17 A16  A15 A14 A13 A12  A11 A10 A9 A8  A7 A6 A5 A4  A3 A2 A1 A0

on newer Speech Plus hardware:
     0   0   x   x    0   x   0   *    *   *  *  *   *  *  *  *   *  *  *  s  RW 6264*2 SRAM @U61/U64 first half
     0   0   x   x    0   x   1   0    *   *  *  *   *  *  *  *   *  *  *  s  RW 6264*2 SRAM @U61/U64 3rd quarter

on older TSI hardware:
     0   0   x   x    0   x   0   0    *   *  *  *   *  *  *  *   *  *  *  s  RW 6116*2 SRAM @U61/U64
     0   0   x   x    0   x   0   1    *   *  *  *   *  *  *  *   *  *  *  s  RW 6116*2 SRAM @U62/U65
     0   0   x   x    0   x   1   0    *   *  *  *   *  *  *  *   *  *  *  s  RW 6116*2 SRAM @U63/U66

Common:
     0   0   x   x    0   x   1   1    0   0  0  x   x  x  x  x   x  x  *  0  RW iP8251A @U15
     0   0   x   x    0   x   1   1    0   0  1  x   x  x  x  x   x  x  *  0  RW AMD P8259A PIC @U5

on newer Speech Plus hardware:
     0   0   x   x    0   x   1   1    0   1  0  x   x  x  x  x   x  x  x  0   W 74HCT273@U82 for ROM banking, *UNUSED* but populated;
                                                                                 bits 0,6,7 select a ROM bank but only if the 74S138@U81A is
                                                                                 populated, which it is NOT.

Common:
     0   0   x   x    0   x   1   1    0   1  0  x   x  x  x  x   x  x  x  0   R Dipswitch array S-4, as well as serial DCD
     0   0   x   x    0   x   1   1    0   1  0  x   x  x  x  x   x  x  x  1   W LEDS, USEPOT enable, and UPD77[P]20 RESET line
     0   0   x   x    0   x   1   1    0   1  1  x   x  x  x  x   x  x  *  0  RW UPD77[P]20 data/status
     0   0   x   x    0   x   1   1    1   x  x  x   x  x  x  x   x  x  x  x     Open bus
     0   0   x   x    1   x   x   x    x   x  x  x   x  x  x  x   x  x  x  x     Open bus
     0   1   x   x    x   x   x   x    x   x  x  x   x  x  x  x   x  x  x  x     Open bus
     1   0   x   x    x   x   x   x    x   x  x  x   x  x  x  x   x  x  x  x     Open bus

on newer Speech Plus hardware:
     1   1   0   *    *   *   *   *    *   *  *  *   *  *  *  *   *  *  *  s  R  ROMs 2/3
     1   1   1   *    *   *   *   *    *   *  *  *   *  *  *  *   *  *  *  s  R  ROMs 0/1

on older TSI hardware:
     1   1   0   *    *   *   *   *    *   *  *  *   *  *  *  *   *  *  *  *     Open bus
     1   1   1   0    a   a   *   *    *   *  *  *   *  *  *  *   *  *  *  s  R  ROMs aa=00:0/1, aa=01:2/3, aa=10:4/5, aa=11:6/7
     1   1   1   1    a   a   *   *    *   *  *  *   *  *  *  *   *  *  *  s  R  ROMs aa=00:8/9, aa=01:10/11, aa=10:12/13, aa=11:14/15
*/
void tsispch_state::i8086_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x02fff).mirror(0x34000).ram();
	map(0x03000, 0x03003).mirror(0x341fc).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x03200, 0x03203).mirror(0x341fc).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x03400, 0x03401).mirror(0x341fe).rw(FUNC(tsispch_state::dsw_r), FUNC(tsispch_state::peripheral_w));
	map(0x03600, 0x03601).mirror(0x341fc).rw(FUNC(tsispch_state::dsp_data_r), FUNC(tsispch_state::dsp_data_w));
	map(0x03602, 0x03603).mirror(0x341fc).r(FUNC(tsispch_state::dsp_status_r));
	map(0xc0000, 0xfffff).rom();
}

// Technically the IO line of the i8086 is completely ignored
// (it is running in 8086 MIN mode) and the IO/M pin is unconnected,
// so any IO accesses will mirror into the 000-3ff area where RAM lives
void tsispch_state::i8086_io(address_map &map)
{
	// TODO: in theory, map the low 0x400 bytes of the SRAM here;
	// the program does not actually access this area at all!
	map.unmap_value_high();
}

void tsispch_state::dsp_prg_map(address_map &map)
{
	map(0x0000, 0x01ff).rom().region("dsp:prg", 0);
}

void tsispch_state::dsp_data_map(address_map &map)
{
	map(0x0000, 0x01ff).rom().region("dsp:dat", 0);
}


/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( prose2k )
PORT_START("s4") // dipswitch array s4
	PORT_DIPNAME( 0x01, 0x00, "S4-1: Character Length") PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, "7 bit" )
	PORT_DIPSETTING(    0x00, "8 bit" )
	PORT_DIPNAME( 0x02, 0x00, "S4-2: Unknown") PORT_DIPLOCATION("SW4:2") // guess: stop bits 1 vs 2?
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "S4-3: Parity Enable") PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "S4-4: Parity Odd/Even") PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, "Odd" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "S4-5: Unknown") PORT_DIPLOCATION("SW4:5") // guess: xon/xoff vs rts/cts?
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "S4-6: Flow Control") PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, "RTS/CTS" )
	PORT_DIPSETTING(    0x00, "DSR/DTR" )
	PORT_DIPNAME( 0x40, 0x40, "S4-7: Self Test") PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "S4-8: Unused") PORT_DIPLOCATION("SW4:8") // this switch is entirely unused, but its value is instead driven by serial port DCD
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

// The other two dipswitch arrays set the address for the second 8751 UART into multibus space

/******************************************************************************
 Machine Drivers
******************************************************************************/
void tsispch_state::prose2k(machine_config &config)
{
	/* basic machine hardware */
	/* There are two crystals on the board: a 24MHz xtal at Y2 and a 16MHz xtal at Y1 */
	I8086(config, m_maincpu, 24_MHz_XTAL/3); /* VERIFIED clock, 24MHz (or 23.040MHz on older devices) xtal at Y2, divided by 3 in an 8284A */
	m_maincpu->set_addrmap(AS_PROGRAM, &tsispch_state::i8086_mem);
	m_maincpu->set_addrmap(AS_IO, &tsispch_state::i8086_io);
	m_maincpu->set_irq_acknowledge_callback(m_pic, FUNC(pic8259_device::inta_cb));

	/* TODO: the UPD7720 has a 2MHz clock to its SCK pin, currently hacked around in the cpu core */
	UPD7720(config, m_dsp, 16_MHz_XTAL/2); /* VERIFIED clock, 16MHz xtal at Y1, divided by 2 */
	m_dsp->set_addrmap(AS_PROGRAM, &tsispch_state::dsp_prg_map);
	m_dsp->set_addrmap(AS_DATA, &tsispch_state::dsp_data_map);
	m_dsp->p0().set(m_pic, FUNC(pic8259_device::ir0_w)).invert();
	// UPD7720 P1 is NC
	m_dsp->so16().set(FUNC(tsispch_state::dsp_so16_cb));

	/* PIC 8259 */
	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	/* uarts */
	I8251(config, m_uart);
	m_uart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_uart->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_uart->rxrdy_handler().set(m_pic, FUNC(pic8259_device::ir1_w));
	m_uart->txrdy_handler().set(m_pic, FUNC(pic8259_device::ir3_w));
	m_uart->txempty_handler().set(m_pic, FUNC(pic8259_device::ir2_w));

// UART clock: ((16MHz / 2) / 13) = 615.384KHz
//             615.384Khz / (12 / 3) = 153.846KHz as root UART clock
//             153846Hz / 16 (within UART) = 9615.384Hz ~= 9600 baud
// So the supported baud rates, selected by a jumper on array P?:
// 153846Hz / (16 * 1) = 9615.384615
// 153846Hz / (16 * 2) = 4807.692308
// 153846Hz / (16 * 4) = 2403.846154
// 153846Hz / (16 * 8) = 1201.923077
// 153846Hz / (16 * 16) = 600.9615385
// 153846Hz / (16 * 32) = 300.4807692

	clock_device &clock(CLOCK(config, "baudclock", ((16_MHz_XTAL/2)/13)/(12/3))); // ((16MHz / 2) / 13) = 615.384KHz; 615.384Khz / (12/3) = 153.846KHz as root UART clock (153.846KHz)
	clock.signal_handler().set(m_uart, FUNC(i8251_device::write_txc));
	clock.signal_handler().append(m_uart, FUNC(i8251_device::write_rxc));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	// 7th order elliptic filter using an RF5609A clocked at (16Mhz/3)/11 = 484.8484...Khz, yielding a cutoff of 4.848khz
	FILTER_BIQUAD(config, m_efilter[0]).setup(filter_biquad_device::biquad_type::LOWPASS1P1Z, 4848.4848, M_SQRT2/2, 1.0);
	m_efilter[0]->add_route(ALL_OUTPUTS, "speaker", 1.0);
	FILTER_BIQUAD(config, m_efilter[1]).setup(filter_biquad_device::biquad_type::LOWPASS, 4848.4848, M_SQRT2/2, 1.0);
	m_efilter[1]->add_route(ALL_OUTPUTS, m_efilter[0], 1.0);
	FILTER_BIQUAD(config, m_efilter[2]).setup(filter_biquad_device::biquad_type::LOWPASS, 4848.4848, M_SQRT2/2, 1.0);
	m_efilter[2]->add_route(ALL_OUTPUTS, m_efilter[1], 1.0);
	FILTER_BIQUAD(config, m_efilter[3]).setup(filter_biquad_device::biquad_type::LOWPASS, 4848.4848, M_SQRT2/2, 1.0);
	m_efilter[3]->add_route(ALL_OUTPUTS, m_efilter[2], 1.0);

	// prefilter, has a cutoff at 10khz, this serves as a prefilter for preventing aliasing of the input of the RF5609
	// as a 10Khz filter is not very useful seeing as the samplerate of the unit itself IS 10khz
	FILTER_BIQUAD(config, m_prefilter).opamp_sk_lowpass_setup(RES_K(24.3), RES_K(24.3), RES_M(999.99), RES_R(0.001), CAP_P(910), CAP_P(470)); // R36, R32, N/A, N/A, C39, C33
	m_prefilter->add_route(ALL_OUTPUTS, m_efilter[3], 1.0);

#ifndef TOO_ACCURATE
	AM6012(config, m_dac, 0).add_route(ALL_OUTPUTS, m_prefilter, 1.0); // AM6012 or UPC6012 12-bit DAC
#else
	PCM54HP(config, m_dac, 0).add_route(ALL_OUTPUTS, m_prefilter, 1.0); // hack for 13 bit output using the otherwise unconnected low bit
#endif

	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	m_rs232->rxd_handler().set("i8251a_u15", FUNC(i8251_device::write_rxd));
	m_rs232->dsr_handler().set("i8251a_u15", FUNC(i8251_device::write_dsr));
	m_rs232->cts_handler().set("i8251a_u15", FUNC(i8251_device::write_cts));
	m_rs232->dcd_handler().set(FUNC(tsispch_state::serial_dcd_w));
}

/******************************************************************************
 ROM Definitions
******************************************************************************/
ROM_START( prose2k )
	ROM_REGION(0x100000,"maincpu", 0)
	// prose 2000/2020 firmware version 3.4.1
	ROMX_LOAD( "v3.4.1__2000__2.u22",   0xc0000, 0x10000, CRC(201d3114) SHA1(549ef1aa28d5664d4198cbc1826b31020d6c4870),ROM_SKIP(1))
	ROMX_LOAD( "v3.4.1__2000__3.u45",   0xc0001, 0x10000, CRC(190c77b6) SHA1(2b90b3c227012f2085719e6283da08afb36f394f),ROM_SKIP(1))
	ROMX_LOAD( "v3.4.1__2000__0.u21",   0xe0000, 0x10000, CRC(3fae874a) SHA1(e1d3e7ba309b29a9c3edbe3d22becf82eae50a31),ROM_SKIP(1))
	ROMX_LOAD( "v3.4.1__2000__1.u44",   0xe0001, 0x10000, CRC(bdbb0785) SHA1(6512a8c2641e032ef6bb0889490d82f5d4399575),ROM_SKIP(1))

	// TSI/Speech plus DSP firmware v3.12 8/9/88, NEC UPD77P20
	ROM_REGION32_LE( 0x800, "dsp:prg", 0) // unpacked 32 bit le data, cpu data is in low 23 bits
	ROM_LOAD( "v3.12__8-9-88.prg.u29", 0x0000, 0x0800, CRC(6511df1e) SHA1(d898912bf6f630205340f0f5c17a8d88cf154787))

	ROM_REGION16_LE( 0x400, "dsp:dat", 0) // 512*13-bit words, left-justified
	ROM_LOAD( "v3.12__8-9-88.dat.u29", 0x0000, 0x0400, CRC(95e4d57a) SHA1(f6f6d9073677515fcb5b7e47244f05c6a6c874d0))

	ROM_REGION(0x1000, "proms", 0)
	ROM_LOAD( "am27s19.u77", 0x0000, 0x0020, CRC(a88757fc) SHA1(9066d6dbc009d7a126d75b8461ca464ddf134412))
	ROM_LOAD( "am27s19.u79", 0x0020, 0x0020, CRC(a165b090) SHA1(bfc413c79915c68906033741318c070ad5dd0f6b))
	ROM_LOAD( "am27s19.u81", 0x0040, 0x0020, CRC(62e1019b) SHA1(acade372edb08fd0dcb1fa3af806c22c47081880))
ROM_END

ROM_START( prose2ko )
	// 'Older' prose2k set, on the older "Telesensory Systems, Inc." (TSI) board with one less 74xx chip.
	ROM_REGION(0x100000,"maincpu", 0)
	// prose 2000 firmware version 1.1
	ROMX_LOAD( "v1.1__6__speech__plus__=c=1983.am2764.6.u24",   0xec000, 0x2000, CRC(c881f92d) SHA1(2d4eb96360adac54d4f0110595bfaf682280c1ca),ROM_SKIP(1))
	ROMX_LOAD( "v1.1__7__speech__plus__=c=1983.am2764.7.u47",   0xec001, 0x2000, CRC(4d5771cb) SHA1(55ed59ad1cad154804dbeeebed98f062783c33c3),ROM_SKIP(1))
	ROMX_LOAD( "v1.1__8__speech__plus__=c=1983.am2764.8.u25",   0xf0000, 0x2000, CRC(adf9bfb8) SHA1(0b73561b52b388b740fabf07ada2d70a52f22037),ROM_SKIP(1))
	ROMX_LOAD( "v1.1__9__speech__plus__=c=1983.am2764.9.u48",   0xf0001, 0x2000, CRC(355f97d2) SHA1(7655fc55b577821e0bd8bf81fb74b8a20b1df098),ROM_SKIP(1))
	ROMX_LOAD( "v1.1__10__speech__plus__=c=1983.am2764.10.u26", 0xf4000, 0x2000, CRC(949a0344) SHA1(8e33c69dfc413aea95f166b08902ad97b1e3e980),ROM_SKIP(1))
	ROMX_LOAD( "v1.1__11__speech__plus__=c=1983.am2764.11.u49", 0xf4001, 0x2000, CRC(ad9a0670) SHA1(769f2f8696c7b6907706466aa9ab7a897ed9f889),ROM_SKIP(1))
	ROMX_LOAD( "v1.1__12__speech__plus__=c=1983.am2764.12.u27", 0xf8000, 0x2000, CRC(9eaf9378) SHA1(d296b1d347c03e6123c38c208ead25b1f43b9859),ROM_SKIP(1))
	ROMX_LOAD( "v1.1__13__speech__plus__=c=1983.am2764.13.u50", 0xf8001, 0x2000, CRC(5e173667) SHA1(93230c2fede5095f56e10d20ea36a5a45a1e7356),ROM_SKIP(1))
	ROMX_LOAD( "v1.1__14__speech__plus__=c=1983.am2764.14.u28", 0xfc000, 0x2000, CRC(e616bd6e) SHA1(5dfae2c5079d89f791c9d7166f9504231a464203),ROM_SKIP(1))
	ROMX_LOAD( "v1.1__15__speech__plus__=c=1983.am2764.15.u51", 0xfc001, 0x2000, CRC(beb1fa19) SHA1(72130fe45c3fd3de7cf794936dc68ed2d4193daf),ROM_SKIP(1))

	// TSI/Speech plus DSP firmware v1.0? on a mask ROM ceramic uPD7720, with a
	//  "Speech // Plus // (C)1983" sticker covering the metal cover and silkscreen.
	// The mask silkscreen is "NEC JAPAN // 7720 052 // 8349X9" (the X9 after the date code may
	//  indicate the 7720 die stepping)
	// The chip also has "S14025" silkscreened on the ceramic portion.
	//  This is the TSI part number, much like S14001A for their older speech chip, and
	//  S14002/S14003 for its respective MCU and RAM, etc.
	// This DSP was dumped using the mask test mode.
	ROM_REGION32_LE( 0x800, "dsp:prg", 0) // unpacked 32 bit le data, cpu data is in low 23 bits
	ROM_LOAD( "7720-052__8349x9___s14025___speech__plus__=c=1983.prog.u29", 0x0000, 0x0800, CRC(27c6b163) SHA1(b5ad3ead3525723e091c5763ae9a768a595e71fa))

	ROM_REGION16_LE( 0x400, "dsp:dat", 0) // 512*13-bit words, left-justified
	ROM_LOAD( "7720-052__8349x9___s14025___speech__plus__=c=1983.data.u29", 0x0000, 0x0400, CRC(8d80db41) SHA1(d4d26893a42ba4e010918b84c84d331c5bde5249))
	// Note that the S14025 (unlike the later DSP) data ROM is not actually used by the DSP chip
	//  in normal operation, and only seems to be potentially accessed as some form of 'secret
	//  response' style copy protection which is unclear how to trigger from the 8086 CPU side,
	//  if it is present in the 8086 code at all!

	ROM_REGION(0x1000, "proms", 0)
	ROM_LOAD( "dm74s288n.u77", 0x0000, 0x0020, CRC(a88757fc) SHA1(9066d6dbc009d7a126d75b8461ca464ddf134412)) // == am27s19.u77
	ROM_LOAD( "dm74s288n.whitespot.u79", 0x0020, 0x0020, CRC(7faee6cb) SHA1(b6dd2a6909dac9e89e7317c006a013ff0866382d))
	// no third PROM in this set, a 74S138@U81A is used instead for e0000-fffff ROM mapping
ROM_END

// a version U2.0 firmware, also using the s14025 dsp, is known but not yet dumped. This is also on the older TSI board.

} // anonymous namespace

/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY                                FULLNAME                  FLAGS
COMP( 1987, prose2k,  0,       0,      prose2k, prose2k, tsispch_state, empty_init, "Speech Plus", "Prose 2000/2020 v3.4.1 (new HW)", MACHINE_SUPPORTS_SAVE )
COMP( 1982, prose2ko, prose2k, 0,      prose2k, prose2k, tsispch_state, empty_init, "Telesensory Systems Inc.", "Prose 2000/2020 v1.1 (old HW)", MACHINE_SUPPORTS_SAVE )
