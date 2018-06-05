// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Morrow Tricep

        12/05/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc2661.h"

class tricep_state : public driver_device
{
public:
	tricep_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pci(*this, "pci%u", 0)
		, m_p_ram(*this, "p_ram")
	{
	}

	void tricep(machine_config &config);
protected:
	DECLARE_WRITE8_MEMBER(pci_mux_w);
	DECLARE_READ8_MEMBER(pci_r);
	DECLARE_WRITE8_MEMBER(pci_w);

	void tricep_mem(address_map &map);

	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device_array<mc2661_device, 4> m_pci;
	required_shared_ptr<uint16_t> m_p_ram;

	uint8_t m_mux;
};



WRITE8_MEMBER(tricep_state::pci_mux_w)
{
	m_mux = data & 3;
}

READ8_MEMBER(tricep_state::pci_r)
{
	return m_pci[m_mux]->read(space, offset);
}

WRITE8_MEMBER(tricep_state::pci_w)
{
	m_pci[m_mux]->write(space, offset, data);
}

void tricep_state::tricep_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0007ffff).ram().share("p_ram");
	map(0x00fd0000, 0x00fd1fff).rom().region("user1", 0);
	map(0x00ff0028, 0x00ff002b).rw(this, FUNC(tricep_state::pci_r), FUNC(tricep_state::pci_w)).umask16(0xffff);
	map(0x00ff002f, 0x00ff002f).w(this, FUNC(tricep_state::pci_mux_w));
}

/* Input ports */
static INPUT_PORTS_START( tricep )
INPUT_PORTS_END


void tricep_state::machine_reset()
{
	uint8_t* user1 = memregion("user1")->base();

	memcpy((uint8_t*)m_p_ram.target(),user1,0x2000);

	m_maincpu->reset();

	m_mux = 0;
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 ) // FIXME: should be 19200 with SCN2661B
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

MACHINE_CONFIG_START(tricep_state::tricep)
	MCFG_DEVICE_ADD("maincpu", M68000, XTAL(8'000'000))
	MCFG_DEVICE_PROGRAM_MAP(tricep_mem)

	MCFG_DEVICE_ADD("pci0", MC2661, 4915200)
	MCFG_MC2661_TXD_HANDLER(WRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_MC2661_RTS_HANDLER(WRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_MC2661_DTR_HANDLER(WRITELINE("rs232", rs232_port_device, write_dtr))

	MCFG_DEVICE_ADD("pci1", MC2661, 4915200)
	MCFG_DEVICE_ADD("pci2", MC2661, 4915200)
	MCFG_DEVICE_ADD("pci3", MC2661, 4915200)

	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("pci0", mc2661_device, rx_w))
	MCFG_RS232_DSR_HANDLER(WRITELINE("pci0", mc2661_device, dsr_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("pci0", mc2661_device, dcd_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("pci0", mc2661_device, cts_w))
	MCFG_SLOT_OPTION_DEVICE_INPUT_DEFAULTS("terminal", terminal)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( tricep )
	ROM_REGION( 0x2000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "tri2.4_odd.u37",  0x0000, 0x1000, CRC(31eb2dcf) SHA1(2d9df9262ee1096d0398505e10d209201ac49a5d))
	ROM_LOAD16_BYTE( "tri2.4_even.u36", 0x0001, 0x1000, CRC(4414dcdc) SHA1(00a3d293617dc691748ae85b6ccdd6723daefc0a))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY           FULLNAME  FLAGS
COMP( 1985, tricep, 0,      0,      tricep,  tricep, tricep_state, empty_init, "Morrow Designs", "Tricep", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
