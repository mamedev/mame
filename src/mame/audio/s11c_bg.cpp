// license:BSD-3-Clause
// copyright-holders:Barry Rodewald, Jonathan Gevaryahu
/*
 * s11c_bg.cpp - Williams D-11581 "Audio Board" PCB trace part number: 5766-12130-00 REV A.
 * (M68B09E + YM2151 + HC555xx + DAC)
 *
 * Used by Williams System 11A (F-14), all System 11B (except Jokerz) and all System 11C pinballs
 * Used by Midway Y-Unit Arcade Hardware (for Smash TV, Trog, and Strike Force; see below
 * for [Super] High Impact Football)
 * Used by Midway MCR68 Arcade Hardware (for Pigskin, Arch Rivals, and Tri-Sport)
 * The older D-11298 version is used by Williams Joust 2 Arcade hardware, see below
 *
 * The interface connector for this board is a 20 pin header J4 with the following pinout:
 *
 *            +--------+
 *        GND |  1   2 | NC
 *        PB0 |  3   4 | PB1
 *        PB2 |  5   6 | PB3
 *        PB4 |  7   8 | PB5
 *        PB6 |  9  10 | PB7
 *         NC | 11  12 | CB2 (/stbr)
 * (/stb) CB1 | 13  14 | NC
 *         NC | 15  16 | NC
 *         NC | 17  18 | /RESET
 *         NC | 19  20 | NC
 *            +--------+
 *
 * Technically:
 * CB1 Pin 13 is 'strobe in' and is asserted low to write to the sound board
 * CB2 Pin 12 is 'strobe out' and is asserted low to indicate the sound board
 * has written and needs the mainboard to read its bus
 *
 * The actual full pinout of the connector, from the System 11 end is:
 *        +--------+
 *    GND |  1   2 | BLANKING
 *    MD0 |  3   4 | MD1
 *    MD2 |  5   6 | MD3
 *    MD4 |  7   8 | MD5
 *    MD6 |  9  10 | MD7
 *   LCB1 | 11  12 | MCB1
 *   MCB2 | 13  14 | /RESET
 *   LCA1 | 15  16 | MCA1
 *    R/W | 17  18 | MCA2
 *      E | 19  20 | NC
 *        +--------+
 *
 *
 * The mixing resistors before the MC1458 differ between the D-11581,
 * D-11581-20xx (System 11C), and D-1129x board schematics:
 * (All resistors are 5% unless otherwise noted)
 * schematic P/N   Unknown    16-8999 Rev -  16-8999 Rev E      16-9138
 * newstyle P/N                              A-13971-43313      A-13971-500xx
 * oldstyle P/N    D-1129x    D-11581        D-11581-20xx/400xx
 * CPU_SOUND       R32 2.2k   R12 2.2k       R12 4.7k           R12 4.7k
 * YM2151 CH1      R33 10k    R14 10k        R14 20k            R14 20k
 * YM2151 CH2      R34 10k    R15 10k        R15 20k            R15 20k
 * MC1408 DAC      R35 10k    R16 6.3k       R16 13k            R16 13k
 * CVSD           [R30 10k]  [R13 4.99k 1%]  R13 4.99k 1%       R13 4.99k 1%
 * MC1458 Feedback R38 10k    R17 10k        R17 10k            R17 10k
 * MC1458 +-to-gnd R36 4.7k   R11 4.7k       R11 4.7k           R11 4.7k
 * [] - if CVSD section present

 * Note that some D-11581 boards have the D-11297/8 mixing resistors in place,
 * i.e. those intended for use as replacement parts for D-1129x boards.
 * See https://pinwiki.com/wiki/images/f/fc/System_11_Sound_Board.JPG for an
 * example, which may have originally been a D-11298 replacement, as the CVSD
 * section is unpopulated.

 * Note that the System11C D-11581-20xx/400xx boards have the mixing resistors
 * changed to make the CVSD sound roughly twice as loud as it typically
 * would be, by doubling the resistor values for the other mixer inputs,
 * see https://pinwiki.com/wiki/images/7/7e/System11CSoundBoard.jpg

 * D-11581 Jumpers:
 * W1 - exclusive w/W4, W6 : YM2151 reset comes from Board PIA CA2
 * W2 - exclusive w/W3     : EPROMS U4, U19, [U20] are 27512
 * W3 - exclusive w/W2     : EPROMS U4, U19, [U20] are 2764, 27128 or 27256 (where pin 1 must be high) [Pre-System 11C boards (i.e. D-11581) have a trace shorting W3 closed permanently which must be cut if 27512 EPROMs are to be used]
 * W4 -'exclusive'w/W4, W6 : YM2151 reset comes from PIA CB2/J4 P12 (internal or external)
 * W5                      : Board Reset comes from J4 P18 (if absent, Board Reset is generated from power up)
 * W6 - exclusive w/W4, W6 : YM2151 reset comes from Board Reset
 * W7                      : if present, the VCC/+5v rail is shorted to the +12v rail. (This is used in the case where the board is only run on +5v and -12v instead of +5v, +12v, and -12v)
 * W8 - exclusive w/W9     : U5 is a 6164 SRAM (pin 23/A11 is grounded)
 * W9 - exclusive w/W8     : U5 is a 6116 SRAM (pin 21, footprint pin 23, is /WE)

 * These two jumpers are only on the D-11581-20xx/400xx (System 11C) version,
 *   and W2 and W3 do not affect U20 on this PCB version:
 * W10 - linked w/W2+W3    : (only if W2 is also set) EPROM U20 is a 27512
 * W11 - linked w/W2+W3    : EPROM U20 is a 2764, 27128 or 27256 (where pin 1 must be high)


 * NOTE: A board called A-13971-50003 is used on Midway Y-Unit Arcade Hardware
 * High Impact Football and Super High Impact Football, and on the first 500 or
 * so Funhouse Pinball machines. (Later Funhouse pinballs use the A-12738-50003
 * WPC Sound board, mentioned in the CVSD filtering section below. Despite its
 * earlier part number, the A-12738-50003 WPC Sound Board is a newer design,
 * with the A-13971-50003 mentioned here seemingly produced later as a stopgap
 * due to development or production issues with the WPC Sound Board.)
 * The A-13971-50003 board is ALMOST the same as the D-11581-20xx System 11C
 * version, except it has 32 pin sockets for 27c010 chips, instead of 28 pin
 * sockets for smaller chips. Despite this, the board is fully backwards
 * compatible with the D-11581-20xx, including the mixing resistors.
 * The highest address bit (A16, pin 2) for all 3 EPROMs (as shown in the
 * High Impact Football schematics) is driven by the ROM banking register
 * 0x7800 bit 3, which is unused/unconnected on all older board revisions.
 * The 32 pin EPROM socket pins 1(VPP), 31(/PGM), 32(VCC) and 30(NC) are all
 * tied to VCC.
 * Jumpers W2, W3, W10 and W11 act the same as they do on D-11581-20xx, just
 * the pins they control are offset down in the socket by 2 pins.
 * This means this board is fully backwards compatible with D-11581-20xx.
 * (Note that the prototype Funhouse Schematics and the Super High Impact
 * Footall Kit Service manual both incorrectly have schematics for the
 * D-11581-20xx System 11C version of the board, and do not show the extra
 * banking bit and larger sockets that the A-13971-50003 board has.)



 * Williams D-11297/D-11298 "BG Music & Speech Board":
 * D-11297/D-11298 is the predecessor to D-11581, and is fully compatible with it.
 * It is a larger board, physically.
 * It is used on the following Williams System 11A games:
 * D-11297 (CVSD populated)  : PIN*BOT (and prototype F-14 Tomcat)
 * D-11298 (CVSD unpopulated): Millionaire!
 * It has different mixing resistors to the D-11581.

 * Unlike the later D-11581 board which has mono output only, the D-11297/8 board
 * has provisions for stereo output from the YM2151, but it is unclear if these were
 * ever populated on any shipping boards.

 * D-1129x Jumpers:
 * W1                      : Board Reset comes from J4 P18 (if absent, Board Reset is generated from power up or a manual test switch sw1)
 * W2 - exclusive w/W3     : EPROMS U4, U19, U20 are 27512
 * W3 - exclusive w/W2     : EPROMS U4, U19, U20 are 2764, 27128 or 27256 (where pin 1 must be high)
 * W4                      : if present, the VCC/+5v rail is shorted to the +12v rail. (This is used in the case where the board is only run on +5v and -12v instead of +5v, +12v, and -12v)
 * W5 - exclusive w/W6, W7 : YM2151 reset comes from Board Reset
 * W6 -'exclusive'w/W5, W7 : YM2151 reset comes from PIA CB2/J4 P12 (internal or external)
 * W7 - exclusive w/W5, W6 : YM2151 reset comes from Board PIA CA2



 * CVSD filter:
 * The CVSD filter on all of these boards has the same components:
 *
 *                                     .--------+---------.                      .--------+---------.
 *                                     |        |         |                      |        |         |
 *                                     Z       --- c2     |                      Z       --- c12    |
 *                                     Z r3    --- 180pf  |                      Z r13   --- 1200pf |
 *                                     Z 180k   |         |                      Z 27k    |         |
 *          + c10     r1               |   r2   |  |\     |     r11              |   r12  |  |\     |
 *   In >----|(------ZZZZ----+---------+--ZZZZ--+  | \    +----ZZZZ----+---------+--ZZZZ--+  | \    |
 *            1uf     43k    |             36k  '--|- \   |     27k    |             15k  '--|- \   |   + c20
 *                          ---  c1                |   >--'           ---  c11               |   >--+----|(----> to mix resistor
 *                          ---  0uf[1]         .--|+ /               ---  4700pf         .--|+ /         10uf
 *                           |             r4   |  | /                 |             r14  |  | /
 *                          gnd        .--ZZZZ--'  |/                 gnd        .--ZZZZ--'  |/
 *                                     |   220k     MC1458                       |   27k      MC1458
 *                                    gnd                                       gnd
 *
 *
 * [1] Logically there should be a capacitor to ground at c1 of value 1800uf,
 *     but this was omitted on the D-11297 board, possibly in error, and this
 *     omission carried over to future sound boards including all versions of the D-11581.
 *     The later WPC Sound board, A-12738-500xx, fixed this by completely redesigning the filters
 *     for the YM2151, DAC and CVSD.
 *
 * This circuit would be a 4th order (cascaded 2nd order) op-amp multifeedback lowpass filter,
 *  but because of the capacitor omitted, it is actually a first order-with-gain lowpass,
 *  cascaded into a 2nd order lowpass, forming a 3rd order filter.
 *
 * This same exact circuit, same component values (but including the capacitor missing here),
 * appears on the System 11 and System 11A mainboards, where it forms a 4th order filter.
 * System 11B changed the components slightly, presumably so the CVSD produced on the mainboard
 * sounded a bit different tone-wise to the CVSD produced from the Audio Board, or to reduce the
 * number of distinct part values to reduce cost per board.



 * Williams "BG Music Board" D-11197-542 16-8972
 *  * used on Williams System 11 'Road Kings' only
 *  * has DAC and YM2151/YM3012
 *  * PIA CA2 EPROM banking
 *  * This board, like D-1129x, has provisions for YM2151 stereo via two amplifiers to be
 *     populated on the board itself, but like D-1129x they and their supporting passives are not populated.

 * The mixing resistors on this board are the same as D-1129x, except the CPU_SOUND input resistor is 2.7k instead of 2.2k
 * There is no CVSD section at all.

 * D-11197 Jumpers:
 * W1                      : Board Reset comes from J4 P18 (if absent, Board Reset is generated from power up or a manual test switch sw1)
 * W2 - exclusive w/W3     : EPROMS U4 is a 27512
 * W3 - exclusive w/W2     : EPROMS U4 is a 2764, 27128 or 27256 (where pin 1 must be high)
 * W4                      : if present, the VCC/+5v rail is shorted to the +12v rail. (This is used in the case where the board is only run on +5v and -12v instead of +5v, +12v, and -12v)
 * W5 - exclusive w/W6, W7 : YM2151 reset comes from Board Reset
 * W6 -'exclusive'w/W5, W7 : YM2151 reset comes from PIA CB2/J4 P12 (internal or external)



 * Williams "BG Sound Board" C-11029/C-11030 5766-10929-00
 *  * used on Williams System 11 'High Speed' and 'Grand Lizard' only (as well as the "Wreck'n Ball" prototype, see https://www.ipdb.org/machine.cgi?id=6167 )
 *  * has DAC only
 *  * no EPROM banking hardware, 1 28-pin ROM socket (2764/27128/27256 only)
 *  * PIA CA1 and CA2 are labeled 'HAND1' and 'HAND2' on schematics and are tied high/unused.
 *  * This board has some provisions for what looks like incoming and outgoing octal latch
 *    for data to be read from/writen to by the PIA, but this is unpopulated/bypassed on the pcb

 * C-11029/C-11030 Jumpers:
 * W1                      : Board Reset comes from J4 P18 (if absent, Board Reset is generated from power up or a manual test switch sw1)



 *
 *  Created on: 2/10/2013
 *      Author: bsr
 *  Updated on: 7/6/2020
 *      Author: lord nightmare
 */

#include "emu.h"
#include "s11c_bg.h"


DEFINE_DEVICE_TYPE(S11C_BG, s11c_bg_device, "s11c_bg", "Williams System 11C Background Audio Board") // D-11581-20xx or D-11581-400xx or A-13971-50003
DEFINE_DEVICE_TYPE(S11_BG, s11_bg_device, "s11_bg", "Williams System 11 Background Audio Board") // D-11581 (without the W10/W11 jumpers)
DEFINE_DEVICE_TYPE(S11_OBG, s11_obg_device, "s11_obg", "Williams System 11 (Older) Background Audio Board") // D-11297 or D-11298
DEFINE_DEVICE_TYPE(S11_BGM, s11_bgm_device, "s11_bgm", "Williams System 11 Background Sound/Music Board") // D-11197
DEFINE_DEVICE_TYPE(S11_BGS, s11_bgs_device, "s11_bgs", "Williams System 11 Background Sound Board") // C-11029 or C-11030

s11c_bg_device::s11c_bg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig,S11C_BG,tag,owner,clock)
	, device_mixer_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_dac(*this, "dac")
	, m_ym2151(*this, "ym2151")
	, m_cvsd(*this, "hc55516")
	, m_cvsd_filter(*this, "cvsd_filter")
	, m_cvsd_filter2(*this, "cvsd_filter2")
	, m_pia40(*this, "pia40")
	, m_cpubank(*this, "bgbank")
	, m_cb2_cb(*this)
	, m_pb_cb(*this)
	, m_old_resetq_state(ASSERT_LINE)
{
}

// constructor with overridable type for subclass
s11c_bg_device::s11c_bg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig,type,tag,owner,clock)
	, device_mixer_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_dac(*this, "dac")
	, m_ym2151(*this, "ym2151")
	, m_cvsd(*this, "hc55516")
	, m_cvsd_filter(*this, "cvsd_filter")
	, m_cvsd_filter2(*this, "cvsd_filter2")
	, m_pia40(*this, "pia40")
	, m_cpubank(*this, "bgbank")
	, m_cb2_cb(*this)
	, m_pb_cb(*this)
	, m_old_resetq_state(ASSERT_LINE)
{
}

// subclass definitions
s11_bg_device::s11_bg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: s11c_bg_device(mconfig,S11_BG,tag,owner,clock)
{
}

s11_obg_device::s11_obg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: s11c_bg_device(mconfig,S11_OBG,tag,owner,clock)
{
}

s11_bgm_device::s11_bgm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: s11c_bg_device(mconfig,S11_BGM,tag,owner,clock)
{
}

s11_bgs_device::s11_bgs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: s11c_bg_device(mconfig,S11_BGS,tag,owner,clock)
{
}

void s11c_bg_device::s11c_bg_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x2001).mirror(0x1ffe).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x4000, 0x4003).mirror(0x1ffc).rw("pia40", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x6000, 0x6000).mirror(0x07ff).w(FUNC(s11c_bg_device::bg_cvsd_clock_set_w));
	map(0x6800, 0x6800).mirror(0x07ff).w(FUNC(s11c_bg_device::bg_cvsd_digit_clock_clear_w));
	map(0x7800, 0x7800).mirror(0x07ff).w(FUNC(s11c_bg_device::bgbank_w));
	map(0x8000, 0xffff).bankr("bgbank");
}

void s11c_bg_device::s11c_bgm_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x2001).mirror(0x1ffe).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x4000, 0x4003).mirror(0x1ffc).rw("pia40", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0xffff).bankr("bgbank");
}

void s11c_bg_device::s11c_bgs_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x4000, 0x4003).mirror(0x1ffc).rw("pia40", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0xffff).bankr("bgbank");
}

TIMER_CALLBACK_MEMBER(s11c_bg_device::deferred_cb2_w)
{
	if (!m_cb2_cb.isnull())
		m_cb2_cb(param);
	else
		logerror("S11C_BG CB2 writeback called with state %x, but callback is not registered!\n", param);
}

TIMER_CALLBACK_MEMBER(s11c_bg_device::deferred_pb_w)
{
	if (!m_pb_cb.isnull())
		m_pb_cb(param);
	else
		logerror("S11C_BG PB writeback called with state 0x%2X, but callback is not registered!\n", param);
}


WRITE_LINE_MEMBER( s11c_bg_device::pia40_cb2_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(s11c_bg_device::deferred_cb2_w),this), state);
}

void s11c_bg_device::pia40_pb_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(s11c_bg_device::deferred_pb_w),this), data);
}


WRITE_LINE_MEMBER( s11c_bg_device::extra_w )
{
	m_pia40->cb2_w(state);
}

WRITE_LINE_MEMBER( s11c_bg_device::ctrl_w )
{
	m_pia40->cb1_w(state);
}

void s11c_bg_device::data_w(uint8_t data)
{
	m_pia40->portb_w(data);
}

WRITE_LINE_MEMBER( s11c_bg_device::resetq_w )
{
	if ((m_old_resetq_state != CLEAR_LINE) && (state == CLEAR_LINE))
	{
		logerror("S11 bg device received reset request\n");
		common_reset();
	}
	m_old_resetq_state = state;
}


// just the 6809, the DAC and the PIA
void s11c_bg_device::s11_bg_base(machine_config &config)
{
	MC6809E(config, m_cpu, XTAL(8'000'000) / 4); // MC68B09E
	m_cpu->set_addrmap(AS_PROGRAM, &s11c_bg_device::s11c_bg_map); // override this as needed
	config.set_maximum_quantum(attotime::from_hz(50));

	MC1408(config, m_dac, 0);

	PIA6821(config, m_pia40, 0);
	m_pia40->writepa_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pia40->writepb_handler().set(FUNC(s11c_bg_device::pia40_pb_w));
	// ca2 handler is set in the s11_bg_ym function
	m_pia40->cb2_handler().set(FUNC(s11c_bg_device::pia40_cb2_w));
	m_pia40->irqa_handler().set_inputline(m_cpu, M6809_FIRQ_LINE);
	m_pia40->irqb_handler().set_inputline(m_cpu, INPUT_LINE_NMI);
}

// add a YM2151 for boards which have it
void s11c_bg_device::s11_bg_ym(machine_config &config)
{
	m_pia40->ca2_handler().set(m_ym2151, FUNC(ym2151_device::reset_w));

	YM2151(config, m_ym2151, XTAL(3'579'545)); // "3.58 MHz" on schematics and parts list
	m_ym2151->irq_handler().set(m_pia40, FUNC(pia6821_device::ca1_w)).invert(); // IRQ is not true state
}

// add a CVSD chip for boards which have it
void s11c_bg_device::s11_bg_cvsd(machine_config &config)
{
	// m_cvsd_filter is the first 'half' of U18(MC1458), with R32, R30, R29, no capacitor to ground(!?!) and C9, output feeding the second half
	// m_cvsd_filter2 is the second 'half' of U18(MC1458), with R20, R15, R19, C10 and C7, output feeding the final mixer
	// Note that the (intended 1800uf according to Sinistar/System 6) capacitor
	// to ground for m_cvsd_filter is COMPLETELY MISSING, and hence this
	// filter section behaves very oddly under simulation, retaining a
	// gain but having a first-order falloff response!
	// This was presumably a design error, and not intended, given the
	// strange filter response. We emulate this weird circuit as it existed.
	FILTER_BIQUAD(config, m_cvsd_filter2).opamp_mfb_lowpass_setup(RES_K(27), RES_K(15), RES_K(27), CAP_P(4700), CAP_P(1200));
	FILTER_BIQUAD(config, m_cvsd_filter).opamp_mfb_lowpass_setup(RES_K(43), RES_K(36), RES_K(180), CAP_P(0), CAP_P(180)); // note the first capacitor is 0pf meaning it doesn't exist
	m_cvsd_filter->add_route(ALL_OUTPUTS, m_cvsd_filter2, 1.0);
	HC55516(config, m_cvsd, 0).add_route(ALL_OUTPUTS, m_cvsd_filter, 1.0/4.0); // to prevent massive clipping issues, we divide the signal by 4 here before going into the filters, then multiply it by 4 after it comes out the other end
}


// D-11581-20xx or D-11581-400xx or A-13971-50003
void s11c_bg_device::device_add_mconfig(machine_config &config)
{
	s11_bg_base(config);
	s11_bg_ym(config);
	s11_bg_cvsd(config);
	// volume mixer stuff
	// the sum of all resistances is 13k + 20k + 20k + 4.99k = 57990
	// 1/resistance * 57990 is 4.460769, 2.8895, 2.8895, 11.62124
	// the sum of the previous 4 values is 21.88101; 100/21.88101 = 4.570173
	// the 4 (1/r)*rtotal numbers * 4.570173 are 20.38649, 13.25122, 13.25122 and 53.11108 respectively
	// NOTE: Multiply all numbers here or the final output by 1/0.6395 = 1.5638 to get the relative
	// volume values if there is no audio input used at all
	// audio passthrough resistor from the mainboard input is 4.7kohm, 0.3605 in files sending audio into this device
	m_dac->add_route(ALL_OUTPUTS, *this, 0.1304); // 13Kohm
	m_ym2151->add_route(1, *this, 0.08473); // 20kohm
	m_ym2151->add_route(0, *this, 0.08473); // 20kohm
	m_cvsd_filter2->add_route(ALL_OUTPUTS, *this, 0.3396*4.0); // 4.99kohm
}

// D-11581 (without the W10/W11 jumpers)
void s11_bg_device::device_add_mconfig(machine_config &config)
{
	s11_bg_base(config);
	s11_bg_ym(config);
	s11_bg_cvsd(config);
	// volume mixer stuff
	// the sum of all resistances is 6.3k + 10k + 10k + 4.99k = 31290
	// 1/resistance * 57990 is 4.9666, 3.129, 3.129, 6.2705
	// the sum of the previous 4 values is 17.49521; 100/17.49521 = 5.715851
	// the 4 (1/r)*rtotal numbers * 5.715851 are 28.38873, 17.8849, 17.8849, and 35.84148 respectively
	// NOTE: Multiply all numbers here or the final output by 1/0.5516 = 1.8129 to get the relative
	// volume values correct if there is no audio input used at all
	// NOTE: audio passthrough from the mainboard is 2.2kohm, 0.4484 in files sending audio into this device
	m_dac->add_route(ALL_OUTPUTS, *this, 0.1566); // 6.3Kohm
	m_ym2151->add_route(1, *this, 0.0987); // 10kohm
	m_ym2151->add_route(0, *this, 0.0987); // 10kohm
	m_cvsd_filter2->add_route(ALL_OUTPUTS, *this, 0.1977*4.0); // 4.99kohm
}

// D-11297 or D-11298
void s11_obg_device::device_add_mconfig(machine_config &config)
{
	s11_bg_base(config);
	s11_bg_ym(config);
	s11_bg_cvsd(config);
	// volume mixer stuff
	// the sum of all resistances is 10k + 10k + 10k + 10k = 40000
	// 1/resistance * 40000 is 4.0, 4.0, 4.0, 4.0
	// the sum of the previous 4 values is 16.0; 100/16 = 6.25
	// the 4 (1/r)*rtotal numbers * 6.25 are 25.0, 25.0, 25.0, 25.0 respectively
	// NOTE: Multiply all numbers here or the final output by 1/0.468 = 2.1368 to get the relative
	// volume values correct if there is no audio input used at all
	// NOTE: audio passthrough from the mainboard is 2.2kohm, 0.5319 in files sending audio into this device
	m_dac->add_route(ALL_OUTPUTS, *this, 0.1170); // 10Kohm
	m_ym2151->add_route(1, *this, 0.1170); // 10kohm
	m_ym2151->add_route(0, *this, 0.1170); // 10kohm
	m_cvsd_filter2->add_route(ALL_OUTPUTS, *this, 0.1170*4.0); // 10kohm
}

// D-11197
void s11_bgm_device::device_add_mconfig(machine_config &config)
{
	s11_bg_base(config);
	m_cpu->set_addrmap(AS_PROGRAM, &s11c_bg_device::s11c_bgm_map);
	s11_bg_ym(config);
	// volume mixer stuff
	// the sum of all resistances is 10k + 10k + 10k + 10k = 40000
	// 1/resistance * 40000 is 4.0, 4.0, 4.0, 4.0
	// the sum of the previous 4 values is 16.0; 100/16 = 6.25
	// the 4 (1/r)*rtotal numbers * 6.25 are 25.0, 25.0, 25.0, 25.0 respectively
	// NOTE: Multiply all numbers here or the final output by 1/0.468 = 2.1368 to get the relative
	// volume values correct if there is no audio input used at all
	// NOTE: audio passthrough from the mainboard is 2.2kohm, 0.5319 in files sending audio into this device
	m_dac->add_route(ALL_OUTPUTS, *this, 0.1170); // 10Kohm
	m_ym2151->add_route(1, *this, 0.1170); // 10kohm
	m_ym2151->add_route(0, *this, 0.1170); // 10kohm
	// interestingly, there is no cvsd, but a fourth 10k resistor here, but it is tied to ground. this makes the board quieter than it would otherwise be, presumably.
}

// C-11029 or C-11030
void s11_bgs_device::device_add_mconfig(machine_config &config)
{
	s11_bg_base(config);
	m_cpu->set_addrmap(AS_PROGRAM, &s11c_bg_device::s11c_bgs_map);
	// volume mixer stuff
	// the sum of all resistances is 10k + 10k = 20k
	// NOTE: audio passthrough from the mainboard is 10k, 0.50 in files sending audio to this device
	m_dac->add_route(ALL_OUTPUTS, *this, 0.50); // 10Kohm
}

void s11c_bg_device::device_start()
{
	u8 *rom = memregion("cpu")->base();
	m_cpubank->configure_entries(0, 16, &rom[0x0], 0x8000);
	m_cpubank->set_entry(0);
	/* resolve lines */
	m_cb2_cb.resolve();
	m_pb_cb.resolve();
	save_item(NAME(m_old_resetq_state));
}

void s11c_bg_device::common_reset()
{
	m_cpubank->set_entry(0);
	// reset the CPU again, so that the CPU are starting with the right vectors (otherwise sound may die on reset)
	m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void s11c_bg_device::device_reset()
{
	common_reset();
}

void s11c_bg_device::bg_cvsd_clock_set_w(uint8_t data)
{
	if (m_cvsd)
		m_cvsd->clock_w(1);
}

void s11c_bg_device::bg_cvsd_digit_clock_clear_w(uint8_t data)
{
	if (m_cvsd)
	{
		m_cvsd->clock_w(0);
		m_cvsd->digit_w(data&1);
	}
}

/*
    Rom mapping for the 4 banking bits:
    3 2 1 0
    r q 0 0 -  U4, A15 q, A16 r
    r q 0 1 - U19, A15 q, A16 r
    r q 1 0 - U20, A15 q, A16 r
    x x 1 1 - open bus
    for ease of loading the roms, we swap the bits to the order '1 0 3 2'
*/
void s11c_bg_device::bgbank_w(uint8_t data)
{
	uint8_t bank = bitswap<8>(data,7,6,5,4,1,0,3,2);
	m_cpubank->set_entry(bank&0xf);
}
