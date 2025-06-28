// license:BSD-3-Clause
// copyright-holders: Jonathan Gevaryahu
/*
 * pinsnd88.h - D-12338-567 Williams Pin Sound '88 board (M68B09E + YM2151 + DAC, two channels)
 * PCB solder side trace P/N: 5766-12342-00 REV -
 *
 * Used only by the Williams System 11B pinball game "Jokerz!"
 *
 * The interface connector for this board is a 20 pin header J1 with the following pinout:
 *
 *            +--------+
 *        GND |  1   2 | NC
 *        PB0 |  3   4 | PB1
 *        PB2 |  5   6 | PB3
 *        PB4 |  7   8 | PB5
 *        PB6 |  9  10 | PB7
 *         NC | 11  12 | NC [/SYNC_PULSE] (normal system 11 bg sound board uses this pin)
 *    /STROBE | 13  14 | NC [/RESET_S11]
 *         NC | 15  16 | /SYNC_PULSE_ALT
 *         NC | 17  18 | /RESET
 *         NC | 19  20 | NC
 *            +--------+
 *
 * Technically:
 * Pin 13 is 'strobe in' and is asserted low to write to the sound board
 * Pin 12 is 'sync out' and is asserted low to indicate the sound board
 * has done some action. Unlike the D-11581 sound board, this board cannot
 * write response data back to the host device.
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
 * This PCB shows deisgn choices which place it between the design of the
 * D-11581 Williams System 11 Background Sound board (s11c_bg.cpp) and
 * the A-12738-500xx WPC Sound board (wpcsnd.cpp), notably that it does
 * not use a 68B21 PIA, and it has a fixed ROM bank from 0xC000-0xFFFF
 * with the vectors instead of requiring the 68B09E vectors to appear in
 * every rom bank.
 * It even has two unused I/O addresses for supporting a CVSD chip with latches
 * though the pcb is not configured for this.
 * The C-12350 NARC sound board's master CPU shares its memory map with this
 * board, but the NARC board was seemingly, based on its part number, designed
 * slightly later.
 *
 *
 * D-12338 Jumpers:
 * W1 : enables the 8MHz clock to the divider to the 68B09E. present.
 * W2 : enables /SYNC PULSE on J1 P16. present, despite being missing on the schematics.
 * W3 : enables /SYNC PULSE on J1 P12 (as the usual S11 BG sound board does). absent.
 * W4 : enables /RESET from J1 P18 (under cpu control, as the usual S11 BG sound board does). present.
 * W5 : enables /RESET from J1 P14 (this makes the board reset on S11 power up reset only). absent.
 * W6 : enables the 3.579545MHz clock to the YM2151. present.
 * W7 : ties J4 pin 1 and 2 to GND. absent, despite being present on the schematics.
 * W8 : ties J4 pin 5-through-inductor and the final audio power amp + pins to +12v. absent, as power is presumably delivered in through J4 pin 5 instead.

 * see https://a.allegroimg.com/s1024/0c2cfa/0433164f4bfa94aa99cec60874f5 re: W2 being connected on the real board. (also see undumped REV1 rom)
 * see https://a.allegroimg.com/s1024/0c3dce/74cdfa004e1dbac943986a94999b re: W8 being absent
 * see https://a.allegroimg.com/s1024/0c2979/0ffe7737466bb0ee363d4e127e33 re: W8 being absent

 * NOTE: The Jokerz! pinball cabinet is known to have some significant issues with hum in the audio,
 * Williams released a service bulletin which involved modifying the power pins on the CVSD chip
 * on the System 11B Mainboard, but even that didn't completely fix it. The exact cause of this
 * issue is not entirely clear at this point.


 * TODO: the 'reset twice on reset' thing seems necessary or else tempo is horrendously screwed up. why?
 * TODO: the 'treat reading the analog control port as if it is connected to the input latch and clear the input semaphore' seems necessary for sound to work. why? It doesn't match the schematics...


 * NOTE: The Pin Sound '88 board used by Jokerz! can handle up to two 27c010
 * EPROMs, but it only ever shipped with a single 27512 EPROM.
 * Because of the way that the 27512 sits in the lower 28 pins of the 32 pin socket,
 * this makes the mapping a little odd.
 *  Bank # and area         Banked space    If 2x 27c010    If 1x 27512
 *  Bank 0 4000-7fff        00000-03fff     U6 00000-03fff  open bus
 *  Bank 0 8000-bfff        04000-07fff     U6 04000-07fff  open bus
 *  Bank 1 4000-7fff        08000-0bfff     U6 08000-0bfff  open bus
 *  Bank 1 8000-bfff        0c000-0ffff     U6 0c000-0ffff  open bus
 *  Bank 2 4000-7fff        10000-13fff     U6 10000-13fff  open bus
 *  Bank 2 8000-bfff        14000-17fff     U6 14000-17fff  open bus
 *  Bank 3 4000-7fff        18000-1bfff     U6 18000-1bfff  open bus
 *  Bank 3 8000-bfff        1c000-1ffff     U6 1c000-1ffff  open bus
 *  Bank 4 4000-7fff        20000-23fff     U5 00000-03fff  U5 00000-03fff
 *  Bank 4 8000-bfff        24000-27fff     U5 04000-07fff  U5 04000-07fff
 *  Bank 5 4000-7fff        28000-2bfff     U5 08000-0bfff  U5 08000-0bfff
 *  Bank 5 8000-bfff        2c000-2ffff     U5 0c000-0ffff  U5 0c000-0ffff
 *  Bank 6 4000-7fff        30000-33fff     U5 10000-13fff  U5 00000-03fff
 *  Bank 6 8000-bfff        34000-37fff     U5 14000-17fff  U5 04000-07fff
 *  Bank 7 4000-7fff        38000-3bfff     U5 18000-1bfff  U5 08000-0bfff
 *  Bank 7 8000-bfff +fixed 3c000-3ffff     U5 1c000-1ffff  U5 0c000-0ffff
*/

#include "emu.h"
#include "pinsnd88.h"

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PINSND88, pinsnd88_device, "pinsnd88", "Williams Pin Sound '88 Audio Board")

pinsnd88_device::pinsnd88_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig,PINSND88,tag,owner,clock)
	, device_mixer_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_dac(*this, "dac")
	, m_ym2151(*this, "ym2151")
	, m_cpubank(*this, "bank")
	, m_syncq_cb(*this)
	, m_old_resetq_state(ASSERT_LINE)
	, m_data_in(0xff)
	, m_inputlatch(*this, "inputlatch")
{
}

/*
15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 RW
0  0  0  *  *  *  *  *  *  *  *  *  *  *  *  *  RW  - 0x0000 - SRAM @U7
0  0  1  0  0  0  x  x  x  x  x  x  x  x  x  *  RW  - 0x2000 - YM2151 @U16
0  0  1  0  0  1  x  x  x  x  x  x  x  x  x  x  x   - 0x2400 - open bus (hypothesis: leftover from cvsd_clock_set_w circuit)
0  0  1  0  1  0  x  x  x  x  x  x  x  x  x  x  W[1]- 0x2800 - ANALOG SWITCH, goes to active low inputs of an ADG201 quad analog switch; on NARC, latch to respond to main pcb
0  0  1  0  1  1  x  x  x  x  x  x  x  x  x  x  x   - 0x2C00 - open bus (hypothesis: leftover from cvsd_digit_clock_clear_w circuit; on NARC, secondary command_w)
0  0  1  1  0  0  x  x  x  x  x  x  x  x  x  x  W   - 0x3000 - 7224 DAC write
0  0  1  1  0  1  x  x  x  x  x  x  x  x  x  x  R[2]- 0x3400 - Latch Read and de-assert MC68B09E /IRQ
0  0  1  1  1  0  x  x  x  x  x  x  x  x  x  x  W[1]- 0x3800 - write ROM bank
0  0  1  1  1  1  x  x  x  x  x  x  x  x  x  x  *   - 0x3C00 - pulse the /SYNC_PULSE line low using a 74LS123 retriggerable monostable multivibrator[3]
0  1  *  *  *  *  *  *  *  *  *  *  *  *  *  *  R   - banked rom area (A14 low)
1  0  *  *  *  *  *  *  *  *  *  *  *  *  *  *  R   - banked rom area (A14 high)
1  1  *  *  *  *  *  *  *  *  *  *  *  *  *  *  R   - fixed rom area (permanently on the last bank, A14 high)
[1] reading this address writes open bus garbage to it
[2] writing this address clears the /IRQ state but doesn't read the latch/bus conflict
[3] R=4.7k and C=.001uf, which pulses the /sync_pulse pin output from the sound board
    low according to the datasheet formula:
    TS = KRC, where K is the constant ~0.37, R is 4700),
    and C is 0.000000001, for a result of 1.739us
*/
void pinsnd88_device::pinsnd88_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2001).mirror(0x03fe).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	// 2400 open bus
	// TODO: map(0x2800, 0x2800).mirror(0x03ff).w(FUNC(pinsnd88_device::analog_w));
	map(0x2800, 0x2800).mirror(0x03ff).r(m_inputlatch, FUNC(generic_latch_8_device::read)); // TODO: For some inexplicable reason, this has to be mapped here or you get no sound. The schematic shows this mapped at 0x3400, not 0x2800?
	// 2c00 open bus
	map(0x3000, 0x3000).mirror(0x03ff).w(m_dac, FUNC(dac_byte_interface::data_w));
	map(0x3400, 0x3400).mirror(0x03ff).r(m_inputlatch, FUNC(generic_latch_8_device::read));
	map(0x3800, 0x3800).mirror(0x03ff).w(FUNC(pinsnd88_device::bgbank_w));
	map(0x3c00, 0x3c00).mirror(0x03ff).w(FUNC(pinsnd88_device::sync_w));
	map(0x4000, 0xbfff).bankr("bank"); // banked rom
	map(0xc000, 0xffff).rom().region("cpu",0x3c000); // fixed bank
}

TIMER_CALLBACK_MEMBER(pinsnd88_device::sync_callback)
{
	LOG("pinsnd88 sync cleared!\n");
	m_syncq_cb(ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(pinsnd88_device::deferred_sync_w)
{
	LOG("pinsnd88 sync asserted!\n");
	m_syncq_cb(CLEAR_LINE);
	m_sync_timer->adjust(attotime::from_usec(2));
}

void pinsnd88_device::sync_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pinsnd88_device::deferred_sync_w),this), 0);
}

void pinsnd88_device::strobe_w(int state)
{
	m_inputlatch->write(m_data_in);
}

void pinsnd88_device::data_w(uint8_t data)
{
	m_data_in = data;
}

void pinsnd88_device::resetq_w(int state)
{
	if ((m_old_resetq_state != CLEAR_LINE) && (state == CLEAR_LINE))
	{
		LOG("PINSND88 device received reset request\n");
		common_reset();
	}
	m_old_resetq_state = state;
}


void pinsnd88_device::device_add_mconfig(machine_config &config)
{
	MC6809E(config, m_cpu, XTAL(8'000'000) / 4); // MC68B09E
	m_cpu->set_addrmap(AS_PROGRAM, &pinsnd88_device::pinsnd88_map);
	config.set_maximum_quantum(attotime::from_hz(50));

	// TODO: analog filters and "volume" controls for the two channels
	AD7224(config, m_dac, 0);
	m_dac->add_route(ALL_OUTPUTS, *this, 0.41/2.0, 0); // 470K
	m_dac->add_route(ALL_OUTPUTS, *this, 0.5/2.0, 1); // 330K

	GENERIC_LATCH_8(config, m_inputlatch);
	m_inputlatch->data_pending_callback().set_inputline(m_cpu, M6809_IRQ_LINE);

	YM2151(config, m_ym2151, XTAL(3'579'545)); // "3.58 MHz" on schematics and parts list
	m_ym2151->irq_handler().set_inputline(m_cpu, M6809_FIRQ_LINE); // IRQ is not true state, but neither is the M6809_FIRQ_LINE so we're fine.
	m_ym2151->add_route(ALL_OUTPUTS, *this, 0.59/2.0, 0); // 330K
	m_ym2151->add_route(ALL_OUTPUTS, *this, 0.5/2.0, 1); // 330K
}

void pinsnd88_device::device_start()
{
	u8 *rom = memregion("cpu")->base();
	m_cpubank->configure_entries(0, 8, &rom[0x0], 0x8000);
	m_cpubank->set_entry(0);

	/* timer */
	m_sync_timer = timer_alloc(FUNC(pinsnd88_device::sync_callback), this);
	m_sync_timer->adjust(attotime::never);
	save_item(NAME(m_old_resetq_state));
	save_item(NAME(m_data_in));
}

void pinsnd88_device::common_reset()
{
	// reset the CPU again, so that the CPU are starting with the right vectors (otherwise sound may die on reset)
	m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero); // TODO: WHY is this needed? this board uses a fixed set of vectors at a fixed rom bank! but it still doesn't work properly without this.
	m_ym2151->reset();
}

void pinsnd88_device::device_reset()
{
	common_reset();
}

/*
    Rom mapping for the 3 banking bits:
    2 1 0
    0 r q -  U6, A15 q, A16 r
    1 r q -  U5, A15 q, A16 r
*/
void pinsnd88_device::bgbank_w(uint8_t data)
{
	m_cpubank->set_entry(data&0x7);
}
