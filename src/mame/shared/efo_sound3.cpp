// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    EFO Sound-3 speech synthesizer board

    The CPU is identified on schematics as a custom "90435" type, but
    board photos confirm it really is a CDP1802ACE as the pinout
    and support ICs suggest. Its XTAL value is specified as 2.95 MHz,
    but 3 MHz has also been seen on one set of Night Mare.

    The synthesizer IC is custom-marked as EFO90503, but it also bears
    a TI logo and is obviously some sort of TMS5220 variant. Its
    discrete oscillator circuit is tuned for a 160 kHz operating
    frequency, according to both schematics and board markings.
    (Unlike other EFO sound boards, Sound-3 has no PSG.)

    The PCB has space for two TMS2532 EPROMs, but normally they are
    both replaced by one TMS2564 EPROM.

    Either of two pin headers may be used for input. J2 allows 8-bit
    parallel data to be strobed in by a clock signal. J3 carries only
    D0-D2 (+ ground) and is used with the older IOS-II board.

**********************************************************************/

#include "emu.h"
#include "efo_sound3.h"

#include "cpu/cosmac/cosmac.h"
#include "speaker.h"


DEFINE_DEVICE_TYPE(EFO_SOUND3, efo_sound3_device, "efo_sound3", "EFO Sound-3 board")

efo_sound3_device::efo_sound3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, EFO_SOUND3, tag, owner, clock)
	, m_inputlatch(*this, "inputlatch")
	, m_intflatch(*this, "intflatch")
	, m_tms(*this, "tms")
	, m_input(0)
{
}

void efo_sound3_device::device_start()
{
	save_item(NAME(m_input));
}

void efo_sound3_device::input_w(u8 data)
{
	m_input = data;
}

void efo_sound3_device::clock_w(int state)
{
	m_inputlatch->clock_w(state);
}

u8 efo_sound3_device::input_r()
{
	return m_input;
}

void efo_sound3_device::intf_cs_w(int state)
{
	if (!m_intflatch->sr_r())
	{
		// CS1 enables data while READY is inactive high!
		if (m_tms->readyq_r())
			m_tms->data_w(m_intflatch->do_r());
		else
			(void)m_intflatch->read(); // reset SR (and end WSQ)
	}
}

void efo_sound3_device::efo90435_mem(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0xc000).rom().region("rom", 0);
	map(0x2000, 0x201f).mirror(0xdfe0).ram(); // CDP1824 @ IC6
}

void efo_sound3_device::efo90435_io(address_map &map)
{
	map(1, 1).mirror(4).w(m_intflatch, FUNC(cdp1852_device::write_strobe));
	map(2, 2).mirror(4).r(m_inputlatch, FUNC(cdp1852_device::read));
}

void efo_sound3_device::device_add_mconfig(machine_config &config)
{
	cdp1802_device &soundcpu(CDP1802(config, "soundcpu", 2.95_MHz_XTAL)); // IC3 90435 "Microprocesador"
	soundcpu.set_addrmap(AS_PROGRAM, &efo_sound3_device::efo90435_mem);
	soundcpu.set_addrmap(AS_IO, &efo_sound3_device::efo90435_io);
	soundcpu.ef3_cb().set(m_tms, FUNC(tms5220_device::status_r)).bit(6).invert();
	soundcpu.ef4_cb().set(m_tms, FUNC(tms5220_device::status_r)).bit(7).invert();
	soundcpu.q_cb().set(m_tms, FUNC(tms5220_device::rsq_w)).invert();

	CDP1852(config, m_inputlatch); // IC7 "Entradas"
	m_inputlatch->mode_cb().set_constant(0);
	m_inputlatch->sr_cb().set_inputline("soundcpu", COSMAC_INPUT_LINE_EF2).invert();
	m_inputlatch->di_cb().set(FUNC(efo_sound3_device::input_r));

	CDP1852(config, m_intflatch); // IC8 "Interface Sintetizador"
	m_intflatch->mode_cb().set_constant(0);
	m_intflatch->sr_cb().set(m_tms, FUNC(tms5220_device::wsq_w));
	m_intflatch->sr_cb().append(FUNC(efo_sound3_device::intf_cs_w));

	TMS5220(config, m_tms, 640000); // IC9 90503 "Sintetizador"
	m_tms->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_tms->irq_cb().set_inputline("soundcpu", COSMAC_INPUT_LINE_INT).invert();
	m_tms->ready_cb().set_inputline("soundcpu", COSMAC_INPUT_LINE_EF1).invert();
	m_tms->ready_cb().append(FUNC(efo_sound3_device::intf_cs_w));

	SPEAKER(config, "mono"); // J5 "Altavoz"
}
