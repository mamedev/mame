// license:BSD-3-Clause
// copyright-holders:Robyn Powell
/*********************************************************************

    greengate_mad.cpp

    Greengate MAD board, companion card to the DS:3 sampler (slot 4).

    Hardware (from the board): MC6850 ACIA for MIDI, ZN428 DAC,
    74LS259 latch, MC14016 analog switches and two LM13700 dual OTAs,
    i.e. four VCAs, one per DS:3 voice, with four trimmers. The DS:3
    voice outputs are routed through the board (CON3) and out to the
    audio box (CON2); the MIDI box hangs off CON4.

    Register use, from the DS:3 1.4M(1) MIDI master (DMS SYS):
      $C0nE/$C0nF  MC6850 control/status and data. Reset with $03, then
                   $95: /16 clock, 8N1, receive interrupt enabled.
                   Received bytes are read in the IRQ handler; $F8 MIDI
                   clock is sent while a sequence plays.
      $Cn00        write: DAC value
      $C0n0-$C0n3  read: strobe the value into a voice control (4 voices)
      $C0n8-$C0nB  read: second set of per-voice strobes

    TODO:
    - the VCA levels aren't applied to the DS:3 audio yet; they are
      only latched (see logging)

*********************************************************************/

#include "emu.h"
#include "greengate_mad.h"

#include "bus/midi/midi.h"
#include "machine/6850acia.h"
#include "machine/clock.h"

#define LOG_VCA (1U << 1)
//#define VERBOSE (LOG_VCA)
#include "logmacro.h"

#define LOGVCA(...) LOGMASKED(LOG_VCA, __VA_ARGS__)


namespace {

class a2bus_greengate_mad_device:
		public device_t,
		public device_a2bus_card_interface
{
public:
	a2bus_greengate_mad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, A2BUS_GREENGATE_MAD, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this),
		m_acia(*this, "acia"),
		m_dac_value(0)
	{
	}

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;

private:
	void strobe(uint8_t offset);
	void acia_clock_w(int state);

	required_device<acia6850_device> m_acia;

	uint8_t m_dac_value;
	uint8_t m_level[2][4];
};

void a2bus_greengate_mad_device::device_add_mconfig(machine_config &config)
{
	ACIA6850(config, m_acia);
	m_acia->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia->irq_handler().set([this] (int state) {
		if (state)
			raise_slot_irq();
		else
			lower_slot_irq();
	});

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	// /16 in the ACIA gives the MIDI rate of 31250 baud
	clock_device &acia_clock(CLOCK(config, "acia_clock", 31250 * 16));
	acia_clock.signal_handler().set(FUNC(a2bus_greengate_mad_device::acia_clock_w));
}

void a2bus_greengate_mad_device::device_start()
{
	save_item(NAME(m_dac_value));
	save_item(NAME(m_level));
}

void a2bus_greengate_mad_device::device_reset()
{
	m_dac_value = 0;
	for (auto &bank : m_level)
		std::fill(std::begin(bank), std::end(bank), 0);
}

void a2bus_greengate_mad_device::acia_clock_w(int state)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

void a2bus_greengate_mad_device::strobe(uint8_t offset)
{
	// latch the DAC value into one voice control
	unsigned const bank = BIT(offset, 3);
	unsigned const voice = offset & 3;
	if (m_level[bank][voice] != m_dac_value)
		LOGVCA("voice %u control %u = %02x\n", voice, bank, m_dac_value);
	m_level[bank][voice] = m_dac_value;
}

uint8_t a2bus_greengate_mad_device::read_c0nx(uint8_t offset)
{
	if (offset >= 0x0e)
		return m_acia->read(offset & 1);

	if (!machine().side_effects_disabled() && (offset & 7) < 4)
		strobe(offset);
	return get_open_bus();
}

void a2bus_greengate_mad_device::write_c0nx(uint8_t offset, uint8_t data)
{
	if (offset >= 0x0e)
		m_acia->write(offset & 1, data);
	else if ((offset & 7) < 4)
		strobe(offset);
}

uint8_t a2bus_greengate_mad_device::read_cnxx(uint8_t offset)
{
	return get_open_bus();
}

void a2bus_greengate_mad_device::write_cnxx(uint8_t offset, uint8_t data)
{
	m_dac_value = data;
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_GREENGATE_MAD, device_a2bus_card_interface, a2bus_greengate_mad_device, "a2ggmad", "Greengate MAD board (DS:3 MIDI)")
