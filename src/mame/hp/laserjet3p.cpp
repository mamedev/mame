// license:BSD-3-Clause
// copyright-holders:rfka01
/***************************************************************************

    HP LaserJet IIIP on its own

    The printer (machine/hp_lj3p.cpp) with a computer's RS-232 port on its
    serial interface; by default a null modem takes the data from a file or
    a network socket, e.g.

        mame laserjet3p -bitb page.pcl
        mame laserjet3p -bitb socket.127.0.0.1:2000

    Select I/O SERIAL in the control panel once (Menu, DEVICE CONFIG; the
    setting stays in the EEPROM) and the same baud rate in the printer and
    in the null modem (9600 by default).  The null modem stops on the
    printer's DTR.  Printed pages are saved as PNG files in the snapshot
    directory.

***************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "machine/hp_lj3p.h"


namespace {

class laserjet3p_state : public driver_device
{
public:
	laserjet3p_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_printer(*this, "printer"),
		m_host(*this, "host")
	{
	}

	void laserjet3p(machine_config &config) ATTR_COLD;

private:
	required_device<hp_lj3p_device> m_printer;
	required_device<rs232_port_device> m_host;
};

static DEVICE_INPUT_DEFAULTS_START( null_modem )
	DEVICE_INPUT_DEFAULTS( "FLOW_CONTROL", 0x07, 0x02 )  // DTR
DEVICE_INPUT_DEFAULTS_END

void laserjet3p_state::laserjet3p(machine_config &config)
{
	HP_LJ3P(config, m_printer);

	RS232_PORT(config, m_host, default_rs232_devices, "null_modem");
	m_host->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(null_modem));
	m_host->rxd_handler().set(m_printer, FUNC(hp_lj3p_device::rxd_w));

	m_printer->txd_cb().set(m_host, FUNC(rs232_port_device::write_txd));
	m_printer->dtr_cb().set(m_host, FUNC(rs232_port_device::write_dtr));
}


ROM_START( laserjet3p )
ROM_END

} // anonymous namespace


//    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT  CLASS             INIT        COMPANY            FULLNAME            FLAGS
COMP( 1991, laserjet3p, 0,      0,      laserjet3p, 0,     laserjet3p_state, empty_init, "Hewlett-Packard", "LaserJet IIIP",    MACHINE_NO_SOUND_HW )
