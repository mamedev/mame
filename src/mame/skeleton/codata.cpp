// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Contel Codata Corporation Codata

        2010-01-11 Skeleton driver.
        2013-08-26 Connected to a terminal.

        Chips: uPD7201C, AM9513, SCN68000. Crystal: 16 MHz

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "bus/rs232/rs232.h"
#include "machine/am9513.h"
#include "machine/z80sio.h"


namespace {

class codata_state : public driver_device
{
public:
	codata_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ram(*this, "mainram")
		, m_maincpu(*this, "maincpu")
	{ }

	void codata(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_shared_ptr<u16> m_ram;
	required_device<cpu_device> m_maincpu;
};

void codata_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).ram().share("mainram");
	map(0x200000, 0x203fff).rom().region("bios", 0);
	map(0x400000, 0x403fff).rom().region("bios", 0x4000);
	map(0x600000, 0x600007).mirror(0x1ffff8).rw("uart", FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask16(0xff00);
	map(0x800000, 0x800003).mirror(0x1ffffc).rw("timer", FUNC(am9513_device::read16), FUNC(am9513_device::write16));
	map(0xe00000, 0xe00001).mirror(0x1ffffe).portr("INPUT");
	//map(0xa00000, 0xbfffff) page map (rw)
	//map(0xc00000, 0xdfffff) segment map (rw), context register (r)
	//map(0xe00000, 0xffffff) context register (w), 16-bit parallel input port (r)
}

/* Input ports */
static INPUT_PORTS_START( codata )
	PORT_START("INPUT")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void codata_state::machine_reset()
{
	uint8_t* bios = memregion("bios")->base();
	memcpy(m_ram, bios, 16);
}

void codata_state::codata(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &codata_state::mem_map);

	upd7201_device &uart(UPD7201(config, "uart", 16_MHz_XTAL / 4));
	uart.out_txda_callback().set("rs423a", FUNC(rs232_port_device::write_txd));
	uart.out_dtra_callback().set("rs423a", FUNC(rs232_port_device::write_dtr));
	uart.out_rtsa_callback().set("rs423a", FUNC(rs232_port_device::write_rts));
	uart.out_txdb_callback().set("rs423b", FUNC(rs232_port_device::write_txd));
	uart.out_int_callback().set_inputline(m_maincpu, M68K_IRQ_5);

	am9513_device &timer(AM9513A(config, "timer", 16_MHz_XTAL / 4));
	timer.out1_cb().set_nop(); // Timer 1 = "Abort/Reset" (watchdog)
	timer.out2_cb().set_inputline(m_maincpu, M68K_IRQ_6); // Timer 2
	timer.out3_cb().set_inputline(m_maincpu, M68K_IRQ_7); // Refresh
	timer.out4_cb().set("uart", FUNC(upd7201_device::rxca_w));
	timer.out4_cb().append("uart", FUNC(upd7201_device::txca_w));
	timer.out5_cb().set("uart", FUNC(upd7201_device::rxcb_w));
	timer.out5_cb().append("uart", FUNC(upd7201_device::txcb_w));

	rs232_port_device &rs423a(RS232_PORT(config, "rs423a", default_rs232_devices, "terminal"));
	rs423a.rxd_handler().set("uart", FUNC(upd7201_device::rxa_w));
	rs423a.dsr_handler().set("uart", FUNC(upd7201_device::dcda_w));
	rs423a.cts_handler().set("uart", FUNC(upd7201_device::ctsa_w));

	rs232_port_device &rs423b(RS232_PORT(config, "rs423b", default_rs232_devices, nullptr));
	rs423b.rxd_handler().set("uart", FUNC(upd7201_device::rxb_w));
}

/* ROM definition */
ROM_START( codata )
	ROM_REGION16_BE( 0x8000, "bios", 0 )
	ROM_LOAD16_BYTE( "27-0042-01a boot 00 u101 rev 3.6.2 9-28-83.u101", 0x0001, 0x2000, CRC(70014b16) SHA1(19a82000894d79817358d40ae520200e976be310))
	ROM_LOAD16_BYTE( "27-0043-01a boot 01 u102 rev 3.6.2 9-28-83.u102", 0x4001, 0x2000, CRC(fca9c314) SHA1(2f8970fad479000f28536003867066d6df9e33d9))
	ROM_LOAD16_BYTE( "27-0044-01a boot e0 u103 rev 3.6.2 9-28-83.u103", 0x0000, 0x2000, CRC(dc5d5cea) SHA1(b3e9248abf89d674c463d21d2f7be34508cf16c2))
	ROM_LOAD16_BYTE( "27-0045-01a boot e1 u104 rev 3.6.2 9-28-83.u104", 0x4000, 0x2000, CRC(a937e7b3) SHA1(d809bbd437fe7d925325958072b9e0dc33dd36a6))

	ROM_REGION( 0x240, "proms", 0 )
	ROM_LOAD( "p0.u502", 0x0000, 0x0020, CRC(20eb1183) SHA1(9b268792b28d858d6b6a1b6c4148af88a8d6b735) )
	ROM_LOAD( "p1.u602", 0x0020, 0x0020, CRC(ee1e5a14) SHA1(0d3346cb3b647fa2475bd7b4fa36ea6ecfdaf805) )
	ROM_LOAD( "p2.u503", 0x0040, 0x0200, CRC(12d9a6be) SHA1(fca99f9c5afc630ac67cbd4e5ba4e5242b826848) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                      FULLNAME  FLAGS
COMP( 1982, codata, 0,      0,      codata,  codata, codata_state, empty_init, "Contel Codata Corporation", "Codata", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
