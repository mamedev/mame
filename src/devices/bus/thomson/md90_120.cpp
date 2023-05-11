// license:BSD-3-Clause
// copyright-holders:Antoine Mine

#include "emu.h"
#include "md90_120.h"

#include "machine/6821pia.h"
#include "machine/clock.h"

#define VERBOSE 0
#include "logmacro.h"

/* ------------  MD 90-120 MODEM extension (not functional) ------------ */

/* Features:
   - 6850 ACIA
   - 6821 PIA
   - asymetric 1200/ 75 bauds (reversable)

   TODO!
 */

// device type definition
DEFINE_DEVICE_TYPE(MD90_120, md90_120_device, "md90_120", "Thomson MD 90-120 Modem")

//-------------------------------------------------
//  md90_120_device - constructor
//-------------------------------------------------

md90_120_device::md90_120_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MD90_120, tag, owner, clock),
	thomson_extension_interface(mconfig, *this),
	m_acia(*this, "acia")
{
}

void md90_120_device::rom_map(address_map &map)
{
}

void md90_120_device::io_map(address_map &map)
{
	map(0x38, 0x3b).rw("pia", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0x3e, 0x3f).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
}

void md90_120_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, "pia");

	ACIA6850(config, m_acia);
	m_acia->txd_handler().set(FUNC(md90_120_device::modem_tx_w));
	m_acia->irq_handler().set(FUNC(md90_120_device::modem_cb));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 1200)); /* 1200 bauds, might be divided by 16 */
	acia_clock.signal_handler().set(FUNC(md90_120_device::write_acia_clock));
}


WRITE_LINE_MEMBER( md90_120_device::modem_cb )
{
	LOG( "modem_cb: called %i\n", state );
}



WRITE_LINE_MEMBER( md90_120_device::modem_tx_w )
{
	m_modem_tx = state;
}


WRITE_LINE_MEMBER( md90_120_device::write_acia_clock )
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

void md90_120_device::device_reset()
{
	m_acia->write_rxd(0);
	m_modem_tx = 0;
	/* pia_reset() is called in machine_reset */
	/* acia_6850 has no reset (?) */
}



void md90_120_device::device_start()
{
	LOG ( " MODEM not implemented!\n" );
	save_item(NAME(m_modem_tx));
}

