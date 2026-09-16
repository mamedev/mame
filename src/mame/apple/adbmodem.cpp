// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple ADB Modem 342S0440-B
    Emulation by R. Belmont

    This is a PIC1654S that converts ADB bit serial to and from bit serial
    for the VIA shifter.

    Connections:
    Pin 1 - RA2 - ADB out (inverted)
    Pin 2 - RA3 - ADB in
    Pin 3 - RTCC - ADB in
    Pin 13 - RB2 - to VIA CB1 (shift clock)
    Pin 14 - RB3 - to VIA CB2 (shift data)
    Pin 16 - RB4 - to VIA PB3 (IRQ)
    Pin 27 - RA0 - to VIA PB4 (newaction bit 0)
    Pin 28 - RA1 - to VIA PB5 (newaction bit 1)
*/

#include "emu.h"
#include "adbmodem.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADBMODEM, adbmodem_device, "adbmodem", "Apple ADB Modem")

ROM_START( adbmodem )
	ROM_REGION(0x400, "adbmodem", 0)
	ROM_LOAD( "342s0440-b.bin", 0x0000, 0x0400, CRC(cffb33eb) SHA1(4a35a44605073ae6076a0292e2056ee4d938d1bd) )
ROM_END

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void adbmodem_device::adbmodem_map(address_map &map)
{
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void adbmodem_device::device_add_mconfig(machine_config &config)
{
	PIC1654S(config, m_maincpu, 3.6864_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &adbmodem_device::adbmodem_map);
	m_maincpu->read_a().set(FUNC(adbmodem_device::porta_r));
	m_maincpu->write_a().set(FUNC(adbmodem_device::porta_w));
	m_maincpu->read_b().set(FUNC(adbmodem_device::portb_r));
	m_maincpu->write_b().set(FUNC(adbmodem_device::portb_w));

#if USE_BUS_ADB
	ADB_CONNECTOR(config, "adb1", adb_device::default_devices, "a9m0330", false);
#endif
}

const tiny_rom_entry *adbmodem_device::device_rom_region() const
{
	return ROM_NAME( adbmodem );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************
#if USE_BUS_ADB
void adbmodem_device::adb_w(int id, int state)
{
	m_adb_device_out[id] = state;
	adb_change();
}

void adbmodem_device::adb_poweron_w(int id, int state)
{
	m_adb_device_poweron[id] = state;
}

void adbmodem_device::adb_change()
{
	bool adb = m_adb_out & m_adb_device_out[0] & m_adb_device_out[1];
	logerror("adb c:%d 1:%d 2:%d -> %d (%02x %02x)\n", m_adb_out, m_adb_device_out[0], m_adb_device_out[1], adb, ddrs[0], ports[0]);
	for (int i = 0; i != 2; i++)
		if (m_adb_device[i])
		{
			m_adb_device[i]->adb_w(adb);
		}
}
#endif

//-------------------------------------------------
//  adbmodem_device - constructor
//-------------------------------------------------

adbmodem_device::adbmodem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ADBMODEM, tag, owner, clock),
	write_reset(*this),
	write_linechange(*this),
	write_via_clock(*this),
	write_via_data(*this),
	write_irq(*this),
	m_maincpu(*this, "adbmodem"),
	m_via_data(0),
	m_via_clock(0),
	m_last_adb(0),
	m_via_state(0),
	m_last_adb_time(0),
	m_adb_in(false),
	m_reset_line(0),
	m_adb_dtime(0),
	m_last_via_clock(1)
#if USE_BUS_ADB
	, m_adb_connector{{*this, "adb1"}, {*this, finder_base::DUMMY_TAG}}
#endif
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adbmodem_device::device_start()
{
#if USE_BUS_ADB
	for (int i = 0; i < 2; i++)
	{
		m_adb_device[i] = m_adb_connector[i] ? m_adb_connector[i]->get_device() : nullptr;
		if (m_adb_device[i])
		{
			m_adb_device[i]->adb_r().set([this, i](int state) { adb_w(i, state); });
			m_adb_device[i]->poweron_r().set([this, i](int state) { adb_poweron_w(i, state); });
		}
	}
#endif

	save_item(NAME(m_via_data));
	save_item(NAME(m_via_clock));
	save_item(NAME(m_adb_in));
	save_item(NAME(m_reset_line));
	save_item(NAME(m_adb_dtime));

#if USE_BUS_ADB
	save_item(NAME(m_adb_out));
	save_item(NAME(m_adb_device_out));
	save_item(NAME(m_adb_device_poweron));
#endif
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void adbmodem_device::device_reset()
{
	#if USE_BUS_ADB
	m_adb_device_out[0] = m_adb_device_out[1] = true;
	m_adb_device_poweron[0] = m_adb_device_poweron[1] = true;
	#endif

	m_adb_in = true;  // line is pulled up to +5v, so nothing plugged in would read as "1"
	m_reset_line = 0;
	m_via_data = 0;
	m_via_clock = 0;
	m_last_adb_time = m_maincpu->total_cycles();
	m_last_adb = 0;
	m_last_via_clock = 1;
}

u8 adbmodem_device::porta_r()
{
	return (m_via_state & 3) | (m_adb_in << 3);
}

void adbmodem_device::porta_w(u8 data)
{
	write_linechange(BIT(data, 2) ^ 1);
}

u8 adbmodem_device::portb_r()
{
	return (m_via_data << 3);
}

void adbmodem_device::portb_w(u8 data)
{
	m_last_via_clock = BIT(data, 2);
	write_via_data(BIT(data, 3));
	write_via_clock(BIT(data, 2));
	write_irq(BIT(data, 4) ^ 1);
}

void adbmodem_device::set_via_data(u8 dat)
{
	/*
	    There is a race condition we sometimes lose on fast CPUs like the IIci where
	    the final bit shifts out of the VIA and the 68k writes the VIA ACR (causing the
	    data line to go high) before the PIC has sampled it to get the final bit state.
	    This manifested as ADB TALK mouse commands going to register 1 instead of 0, which
	    due to implementation details of macadb returned 00 00 for no mouse data available
	    instead of 80 80, and therefore the mouse button state in MacOS went nuts.

	    We solve this by only accepting ADB data line state changes when the last VIA clock
	    we wrote was 0; the VIA is configured with this device to transmit on the falling
	    edge of the clock.

	    Thanks to Tashtari for his extensively commmented disassembly and port of this PIC
	    program to the modern PIC16F87 and PIC16F88, which are pin-compatible with the
	    PIC1654S originally used.  https://github.com/lampmerchant/macseadb88
	*/
	if (!m_last_via_clock)
	{
		m_via_data = dat;
	}
}
