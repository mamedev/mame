// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Morrow Tricep

        12/05/2009 Skeleton driver.

        Several of the boards apparently used in this S-100 system were
        made by CompuPro/Viasyn, including the CPU 68K and Interfacer 3
        (2651 USART multiplexer).

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"
#include "cpu/m68000/m68000.h"
#include "machine/scn_pci.h"


namespace {

class tricep_state : public driver_device
{
public:
	tricep_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_usart(*this, "usart%u", 0U)
		, m_ram(*this, "mainram")
	{
	}

	void tricep(machine_config &config);
private:
	void usart_select_w(uint8_t data);
	uint8_t usart_r(offs_t offset);
	void usart_w(offs_t offset, uint8_t data);

	void mem_map(address_map &map) ATTR_COLD;

	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device_array<scn2651_device, 4> m_usart;
	required_shared_ptr<uint16_t> m_ram;

	uint8_t m_mux = 0;
};



void tricep_state::usart_select_w(uint8_t data)
{
	m_mux = data & 3;
}

uint8_t tricep_state::usart_r(offs_t offset)
{
	return m_usart[m_mux]->read(offset);
}

void tricep_state::usart_w(offs_t offset, uint8_t data)
{
	m_usart[m_mux]->write(offset, data);
}

void tricep_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0007ffff).ram().share("mainram");
	map(0x00fd0000, 0x00fd1fff).rom().region("maincpu", 0);
	map(0x00ff0028, 0x00ff002b).rw(FUNC(tricep_state::usart_r), FUNC(tricep_state::usart_w));
	map(0x00ff002e, 0x00ff002f).nopr();
	map(0x00ff002f, 0x00ff002f).w(FUNC(tricep_state::usart_select_w));
}

/* Input ports */
static INPUT_PORTS_START( tricep )
INPUT_PORTS_END


void tricep_state::machine_reset()
{
	uint8_t* bios = memregion("maincpu")->base();

	memcpy((uint8_t*)m_ram.target(),bios,0x2000);

	m_maincpu->reset();

	m_mux = 0;
}

void tricep_state::machine_start()
{
	save_item(NAME(m_mux));
}

static const input_device_default terminal_defaults[] =
{
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
	{ nullptr, 0, 0 }
};

void tricep_state::tricep(machine_config &config)
{
	M68000(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &tricep_state::mem_map);
	// TODO: MC68451 MMU

	SCN2651(config, m_usart[0], 5.0688_MHz_XTAL);
	m_usart[0]->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_usart[0]->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_usart[0]->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	SCN2651(config, m_usart[1], 5.0688_MHz_XTAL);
	SCN2651(config, m_usart[2], 5.0688_MHz_XTAL);
	SCN2651(config, m_usart[3], 5.0688_MHz_XTAL);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_usart[0], FUNC(scn2651_device::rxd_w));
	rs232.dsr_handler().set(m_usart[0], FUNC(scn2651_device::dsr_w));
	rs232.dcd_handler().set(m_usart[0], FUNC(scn2651_device::dcd_w));
	rs232.cts_handler().set(m_usart[0], FUNC(scn2651_device::cts_w));
	rs232.set_option_device_input_defaults("terminal", terminal_defaults);
}

/* ROM definition */
ROM_START( tricep )
	ROM_REGION16_BE( 0x2000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tri2.4_odd.u37",  0x0001, 0x1000, CRC(31eb2dcf) SHA1(2d9df9262ee1096d0398505e10d209201ac49a5d))
	ROM_LOAD16_BYTE( "tri2.4_even.u36", 0x0000, 0x1000, CRC(4414dcdc) SHA1(00a3d293617dc691748ae85b6ccdd6723daefc0a))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY           FULLNAME  FLAGS
COMP( 1985, tricep, 0,      0,      tricep,  tricep, tricep_state, empty_init, "Morrow Designs", "Tricep", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

