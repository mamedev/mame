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

class codata_state : public driver_device
{
public:
	codata_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_base(*this, "rambase")
		, m_maincpu(*this, "maincpu")
	{ }

	void codata(machine_config &config);
	void mem_map(address_map &map);
private:
	virtual void machine_reset() override;
	required_shared_ptr<uint16_t> m_p_base;
	required_device<cpu_device> m_maincpu;
};

void codata_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).ram().share("rambase");
	map(0x200000, 0x203fff).rom().region("user1", 0);
	map(0x400000, 0x403fff).rom().region("user1", 0x4000);
	map(0x600000, 0x600007).mirror(0x1ffff8).rw("uart", FUNC(upd7201_new_device::ba_cd_r), FUNC(upd7201_new_device::ba_cd_w)).umask16(0xff00);
	map(0x800000, 0x800003).mirror(0x1ffffc).rw("timer", FUNC(am9513_device::read16), FUNC(am9513_device::write16));
	map(0xe00000, 0xe00001).mirror(0x1ffffe).portr("INPUT");
	//AM_RANGE(0xa00000, 0xbfffff) page map (rw)
	//AM_RANGE(0xc00000, 0xdfffff) segment map (rw), context register (r)
	//AM_RANGE(0xe00000, 0xffffff) context register (w), 16-bit parallel input port (r)
}

/* Input ports */
static INPUT_PORTS_START( codata )
	PORT_START("INPUT")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void codata_state::machine_reset()
{
	uint8_t* RAM = memregion("user1")->base();
	memcpy(m_p_base, RAM, 16);
	m_maincpu->reset();
}

MACHINE_CONFIG_START(codata_state::codata)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu",M68000, XTAL(16'000'000) / 2)
	MCFG_DEVICE_PROGRAM_MAP(mem_map)

	MCFG_DEVICE_ADD("uart", UPD7201_NEW, XTAL(16'000'000) / 4)
	MCFG_Z80SIO_OUT_TXDA_CB(WRITELINE("rs423a", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(WRITELINE("rs423a", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(WRITELINE("rs423a", rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_TXDB_CB(WRITELINE("rs423b", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("maincpu", M68K_IRQ_5))

	MCFG_DEVICE_ADD("timer", AM9513A, XTAL(16'000'000) / 4)
	MCFG_AM9513_OUT1_CALLBACK(NOOP) // Timer 1 = "Abort/Reset" (watchdog)
	MCFG_AM9513_OUT2_CALLBACK(INPUTLINE("maincpu", M68K_IRQ_6)) // Timer 2
	MCFG_AM9513_OUT3_CALLBACK(INPUTLINE("maincpu", M68K_IRQ_7)) // Refresh
	MCFG_AM9513_OUT4_CALLBACK(WRITELINE("uart", upd7201_new_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("uart", upd7201_new_device, txca_w))
	MCFG_AM9513_OUT5_CALLBACK(WRITELINE("uart", upd7201_new_device, rxcb_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("uart", upd7201_new_device, txcb_w))

	MCFG_DEVICE_ADD("rs423a", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("uart", upd7201_new_device, rxa_w))
	MCFG_RS232_DSR_HANDLER(WRITELINE("uart", upd7201_new_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("uart", upd7201_new_device, ctsa_w))

	MCFG_DEVICE_ADD("rs423b", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("uart", upd7201_new_device, rxb_w))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( codata )
	ROM_REGION( 0x8000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "27-0042-01a boot 00 u101 rev 3.6.2 9-28-83.u101", 0x0000, 0x2000, CRC(70014b16) SHA1(19a82000894d79817358d40ae520200e976be310))
	ROM_LOAD16_BYTE( "27-0043-01a boot 01 u102 rev 3.6.2 9-28-83.u102", 0x4000, 0x2000, CRC(fca9c314) SHA1(2f8970fad479000f28536003867066d6df9e33d9))
	ROM_LOAD16_BYTE( "27-0044-01a boot e0 u103 rev 3.6.2 9-28-83.u103", 0x0001, 0x2000, CRC(dc5d5cea) SHA1(b3e9248abf89d674c463d21d2f7be34508cf16c2))
	ROM_LOAD16_BYTE( "27-0045-01a boot e1 u104 rev 3.6.2 9-28-83.u104", 0x4001, 0x2000, CRC(a937e7b3) SHA1(d809bbd437fe7d925325958072b9e0dc33dd36a6))

	ROM_REGION( 0x240, "proms", 0 )
	ROM_LOAD( "p0.u502", 0x0000, 0x0020, CRC(20eb1183) SHA1(9b268792b28d858d6b6a1b6c4148af88a8d6b735) )
	ROM_LOAD( "p1.u602", 0x0020, 0x0020, CRC(ee1e5a14) SHA1(0d3346cb3b647fa2475bd7b4fa36ea6ecfdaf805) )
	ROM_LOAD( "p2.u503", 0x0040, 0x0200, CRC(12d9a6be) SHA1(fca99f9c5afc630ac67cbd4e5ba4e5242b826848) )
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                      FULLNAME  FLAGS
COMP( 1982, codata, 0,      0,      codata,  codata, codata_state, empty_init, "Contel Codata Corporation", "Codata", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
