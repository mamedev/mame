// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wpcsnd.c - A-12738-50003 Williams WPC Sound Board
 *   M6809E + YM2151 + HC55516 + DAC
 *
 * TODO: The sound and io interface here needs to be verified from the ASIC on a real PCB, and documented better.
 *       The WPC mainboard has two connectors used for IO/sound:
 *       HDR 17X2 "I/O SOUND"
 *         has the 68B09E signals:
 *           A0-A4, D0-D7, and /FIRQ (with pullup)
 *         it also has the WPC ASIC signals:
 *           BLANK, WDEN, RESET
 *       HDR 2X13 "I/O EXTEND"
 *         has the 68B09E signals:
 *           A5-A12, /FIRQ (with pullup (as above)), /IRQ (with pullup), /E, /Q
 *         it also has the WPC ASIC signal:
 *           /IO
 *
 *       The funhouse schematics show the A-12738-50003 WPC Sound Board to the HDR 17X2 "I/O SOUND" connector as such:
 *       A4 A3 A2 A1 A0 WDEN R/W
 *        0  x  x  x  x    x   x   open bus
 *        x  0  x  x  x    x   x   open bus
 *        x  x  0  x  x    x   x   open bus
 *        x  x  x  x  x    1   x   open bus
 *        1  1  1  1  *    0   *   open bus* (technically the 74LS138 is enabled here, with ** selecting between 4 unused outputs)
 *        1  1  1  0  *    0   *   The used registers, see below
 *        1  1  1  0  0    0   W   write to input gen_8_latch and set semamphore to assert /IRQ on the sound board 68B09E
 *        1  1  1  0  0    0   R   read the output gen_8_latch and clear semamphore to deassert /FIRQ on the main board 68B09E
 *        1  1  1  0  1    0   W   any write here causes the sound board to be reset
 *        1  1  1  0  1    0   R   read from here returns the sound->mainboard semaphore status on D7, all other pins are open bus
 *
 *      Exactly which addresses cause the WPC ASIC to assert low the WDEN pin is not clear,
 *      but presumably it is asserted in the 0x0x3fc0-3fdf area, meaning that the addresses
 *      actually used are 0x3fdc and 0x3fdd (offsets 0x2c and 0x2d).
 *      See /machine/wpc.h
 *
 * TODO: add generic_latch_8 devices for the two latch+semaphore pairs.
 *
 *  Created on: 4/10/2013
 */

#include "emu.h"
#include "wpcsnd.h"
#include "sound/dac.h"

//#define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(WPCSND, wpcsnd_device, "wpcsnd", "Williams WPC Sound")

wpcsnd_device::wpcsnd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, WPCSND, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_cpu(*this, "bgcpu")
	, m_ym2151(*this, "ym2151")
	, m_hc55516(*this, "hc55516")
	, m_cpubank(*this, "rombank")
	, m_fixedbank(*this, "fixed")
	, m_rom(*this, finder_base::DUMMY_TAG)
	, m_reply_cb(*this)
{
}

void wpcsnd_device::wpcsnd_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2000).mirror(0x03ff).w(FUNC(wpcsnd_device::rombank_w));
	map(0x2400, 0x2401).mirror(0x03fe).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x2800, 0x2800).mirror(0x03ff).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x2c00, 0x2c00).mirror(0x03ff).w(FUNC(wpcsnd_device::bg_cvsd_clock_set_w));
	map(0x3000, 0x3000).mirror(0x03ff).r(FUNC(wpcsnd_device::latch_r));
	map(0x3400, 0x3400).mirror(0x03ff).w(FUNC(wpcsnd_device::bg_cvsd_digit_clock_clear_w));
	map(0x3800, 0x3800).mirror(0x03ff).w(FUNC(wpcsnd_device::volume_w));
	map(0x3c00, 0x3c00).mirror(0x03ff).w(FUNC(wpcsnd_device::latch_w));
	map(0x4000, 0xbfff).bankr("rombank");
	map(0xc000, 0xffff).bankr("fixed");
}

void wpcsnd_device::ctrl_w(uint8_t data)
{
	m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void wpcsnd_device::data_w(uint8_t data)
{
	m_latch = data;
	m_cpu->set_input_line(M6809_IRQ_LINE,ASSERT_LINE);
}

uint8_t wpcsnd_device::ctrl_r()
{
	return (m_reply_available) ? 0x01 : 0x00;
}

uint8_t wpcsnd_device::data_r()
{
	m_reply_available = false;
	m_reply_cb(0);
	return m_reply;
}

void wpcsnd_device::device_add_mconfig(machine_config &config)
{
	MC6809E(config, m_cpu, XTAL(8'000'000) / 4); // MC68B09E
	m_cpu->set_addrmap(AS_PROGRAM, &wpcsnd_device::wpcsnd_map);
	config.set_maximum_quantum(attotime::from_hz(50));

	YM2151(config, m_ym2151, 3580000);
	m_ym2151->irq_handler().set(FUNC(wpcsnd_device::ym2151_irq_w));
	m_ym2151->add_route(ALL_OUTPUTS, *this, 0.25);

	AD7524(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.25);

	HC55516(config, m_hc55516, 0).add_route(ALL_OUTPUTS, *this, 0.5);
}


void wpcsnd_device::device_start()
{
	// resolve callback
	m_reply_cb.resolve_safe();
	// save states
	save_item(NAME(m_latch));
	save_item(NAME(m_reply));
	save_item(NAME(m_reply_available));
}

void wpcsnd_device::device_reset()
{
	uint8_t* ROM = m_rom->base();
	m_cpubank->configure_entries(0, 0x80, &ROM[0], 0x8000);
	m_cpubank->set_entry(0);
	m_fixedbank->configure_entries(0, 1, &ROM[0x17c000], 0x4000);
	m_fixedbank->set_entry(0);

	// reset the CPU again, so that the CPU is starting with the right vectors (otherwise sound may die on reset)
	m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

	m_reply_available = false;
}

WRITE_LINE_MEMBER( wpcsnd_device::ym2151_irq_w)
{
	m_cpu->set_input_line(M6809_FIRQ_LINE,state ? ASSERT_LINE : CLEAR_LINE);
}

//------------------------------------------------------
//  bg_cvsd_clock_set_w - set the clk pin on the HC555xx
//------------------------------------------------------

void wpcsnd_device::bg_cvsd_clock_set_w(uint8_t data)
{
	m_hc55516->clock_w(1);
}

//----------------------------------------------------
//  bg_cvsd_digit_clock_clear_w - clear the clk pin on
//  the HC555xx and clock the data latch
//----------------------------------------------------

void wpcsnd_device::bg_cvsd_digit_clock_clear_w(uint8_t data)
{
	m_hc55516->clock_w(0);
	m_hc55516->digit_w(data&1);
}

void wpcsnd_device::rombank_w(uint8_t data)
{
	uint8_t bank = data & 0x0f;

	switch((~data) & 0xe0)
	{
	case 0x80:
		bank |= 0x20;
		break;
	case 0x40:
		bank |= 0x10;
		break;
	case 0x20:
		break;
	}

	m_cpubank->set_entry(bank);

	LOG("WPCSND: Bank set to %02x\n",bank);
}

/// TODO: 74ls374@U39? plus 74LS74@U24B, replace this with a generic_latch_8 device
uint8_t wpcsnd_device::latch_r()
{
	m_cpu->set_input_line(M6809_IRQ_LINE,CLEAR_LINE);
	return m_latch;
}

/// TODO: 74ls374@U23? plus 74LS74@U24A, replace this with a generic_latch_8 device
void wpcsnd_device::latch_w(uint8_t data)
{
	m_reply_available = true;
	m_reply = data;
	m_reply_cb(1);
}

/// TODO: actually implement this
void wpcsnd_device::volume_w(uint8_t data)
{
}
