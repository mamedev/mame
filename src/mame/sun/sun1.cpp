// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Sun-1 Models
        ------------

    Sun-1

        Processor(s):   68000
        Notes:          Large black desktop boxes with 17" monitors.
                        Uses the original Stanford-designed video board
                        and a parallel microswitch keyboard (type 1) and
                        parallel mouse (Sun-1).

    100
        Processor(s):   68000 @ 10MHz
        Bus:            Multibus, serial
        Notes:          Uses a design similar to original SUN (Stanford
                        University Network) CPU. The version 1.5 CPU can
                        take larger RAMs.

    100U
        Processor(s):   68010 @ 10MHz
        CPU:            501-1007
        Bus:            Multibus, serial
        Notes:          "Brain transplant" for 100 series. Replaced CPU
                        and memory boards with first-generation Sun-2
                        CPU and memory boards so original customers
                        could run SunOS 1.x. Still has parallel kb/mouse
                        interface so type 1 keyboards and Sun-1 mice
                        could be connected.

    170
        Processor(s):   68010?
        Bus:            Multibus?
        Chassis type:   rackmount
        Notes:          Server. Slightly different chassis design than
                        2/170's


        Documentation:
            http://www.bitsavers.org/pdf/sun/sun1/800-0345_Sun-1_System_Ref_Man_Jul82.pdf
            (page 39,40 of pdf contain memory map)

        This "Draft Version 1.0" reference claims a 10MHz clock for the
        MC68000 and a 5MHz clock for the Am9513; though the original design
        may have specified a 10MHz CPU, and though this speed may have been
        realized in later models, schematics suggest the system's core
        devices actually run at 8/4MHz (divided from a 16MHz XTAL), which
        lets the 1.0 monitor ROM's Am9513 configuration generate a more
        plausible baud rate.

        04/12/2009 Skeleton driver.

        04/04/2011 Modernised, added terminal keyboard.

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/am9513.h"
#include "machine/z80sio.h"


namespace {

class sun1_state : public driver_device
{
public:
	sun1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_iouart(*this, "iouart")
		, m_p_ram(*this, "p_ram")
	{
	}

	void sun1(machine_config &config);

private:
	void sun1_mem(address_map &map) ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<upd7201_device> m_iouart;
	required_shared_ptr<uint16_t> m_p_ram;
};


void sun1_state::sun1_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x001fffff).ram().share("p_ram"); // 512 KB RAM / ROM at boot
	map(0x00200000, 0x00203fff).rom().region("user1", 0);
	map(0x00600000, 0x00600007).mirror(0x1ffff8).rw(m_iouart, FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask16(0xff00);
	map(0x00800000, 0x00800003).mirror(0x1ffffc).rw("timer", FUNC(am9513_device::read16), FUNC(am9513_device::write16));
	map(0x00a00000, 0x00bfffff).unmaprw(); // page map
	map(0x00c00000, 0x00dfffff).unmaprw(); // segment map
	map(0x00e00000, 0x00ffffff).unmaprw(); // context register
}

/* Input ports */
static INPUT_PORTS_START( sun1 )
INPUT_PORTS_END


void sun1_state::machine_reset()
{
	uint8_t* user1 = memregion("user1")->base();

	memcpy((uint8_t*)m_p_ram.target(),user1,0x4000);
}


void sun1_state::sun1(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &sun1_state::sun1_mem);

	am9513_device &timer(AM9513(config, "timer", 16_MHz_XTAL / 4));
	timer.fout_cb().set("timer", FUNC(am9513_device::gate1_w));
	timer.out1_cb().set_nop(); // Watchdog; generates BERR/Reset
	timer.out2_cb().set_inputline(m_maincpu, M68K_IRQ_6); // User timer
	timer.out3_cb().set_inputline(m_maincpu, M68K_IRQ_7); // Refresh timer (2 ms)
	timer.out4_cb().set(m_iouart, FUNC(upd7201_device::rxca_w));
	timer.out4_cb().append(m_iouart, FUNC(upd7201_device::txca_w));
	timer.out5_cb().set(m_iouart, FUNC(upd7201_device::rxcb_w));
	timer.out5_cb().append(m_iouart, FUNC(upd7201_device::txcb_w));

	UPD7201(config, m_iouart, 16_MHz_XTAL / 4);
	m_iouart->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_iouart->out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	m_iouart->out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	m_iouart->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_iouart->out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	m_iouart->out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));
	m_iouart->out_int_callback().set_inputline(m_maincpu, M68K_IRQ_5);

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set(m_iouart, FUNC(upd7201_device::rxa_w));
	rs232a.cts_handler().set(m_iouart, FUNC(upd7201_device::ctsa_w));
	rs232a.dcd_handler().set(m_iouart, FUNC(upd7201_device::dcda_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_iouart, FUNC(upd7201_device::rxb_w));
	rs232b.cts_handler().set(m_iouart, FUNC(upd7201_device::ctsb_w));
	rs232b.dcd_handler().set(m_iouart, FUNC(upd7201_device::dcdb_w));
}

/* ROM definition */
ROM_START( sun1 )
	ROM_REGION16_BE( 0x4000, "user1", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("1.0")

	ROM_SYSTEM_BIOS(0, "1.0", "Sun Monitor 1.0")
	ROMX_LOAD( "v10.8.bin", 0x0000, 0x2000, CRC(3528a0f8) SHA1(be437dd93d1a44eccffa6f5e05935119482beab0), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "v10.0.bin", 0x0001, 0x2000, CRC(1ad4c52a) SHA1(4bc1a19e8f202378d5d7baa8b95319275c040a6d), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "diag", "Interactive Tests")
	ROMX_LOAD( "8mhzdiag.8.bin", 0x0000, 0x2000, CRC(808a549e) SHA1(d2aba014a5507c1538f2c1a73e1d2524f28034f4), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "8mhzdiag.0.bin", 0x0001, 0x2000, CRC(7a92d506) SHA1(5df3800f7083293fc01bb6a7e7538ad425bbebfb), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_REGION( 0x10000, "gfx", ROMREGION_ERASEFF )
	ROM_LOAD( "gfxu605.g4.bin",  0x0000, 0x0200, CRC(274b7b3d) SHA1(40d8be2cfcbd03512a05925991bb5030d5d4b5e9))
	ROM_LOAD( "gfxu308.g21.bin", 0x0200, 0x0200, CRC(35a6eed8) SHA1(25cb2dd8e5343cd7927c3045eb4cb96dc9935a37))
	ROM_LOAD( "gfxu108.g20.bin", 0x0400, 0x0200, CRC(ecee335e) SHA1(5f4d32dc918af15872cd6e700a04720caeb6c657))
	ROM_LOAD( "gfxu105.g0.bin",  0x0600, 0x0200, CRC(8e1a24b3) SHA1(dad2821c3a3137ad69e78b6fc29ab582e5d78646))
	ROM_LOAD( "gfxu104.g1.bin",  0x0800, 0x0200, CRC(86f7a483) SHA1(8eb3778f5497741cd4345e81ff1a903c9a63c8bb))
	ROM_LOAD( "gfxu307.g61.bin", 0x0a00, 0x0020, CRC(b190f25d) SHA1(80fbdc843f1eb68a2d3713499f04d99dab88ce83))
	ROM_LOAD( "gfxu107.g60.bin", 0x0a20, 0x0020, CRC(425d3a98) SHA1(9ae4ce3761c2f995d00bed8d752c55224d274062))

	ROM_REGION( 0x10000, "cpu", ROMREGION_ERASEFF )
	ROM_LOAD( "cpuu503.p2.bin", 0x0000, 0x0200, CRC(12d9a6be) SHA1(fca99f9c5afc630ac67cbd4e5ba4e5242b826848))
	ROM_LOAD( "cpuu602.p1.bin", 0x0200, 0x0020, CRC(ee1e5a14) SHA1(0d3346cb3b647fa2475bd7b4fa36ea6ecfdaf805))
	ROM_LOAD( "cpuu502.p0.bin", 0x0220, 0x0020, CRC(20eb1183) SHA1(9b268792b28d858d6b6a1b6c4148af88a8d6b735))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY             FULLNAME  FLAGS
COMP( 1982, sun1, 0,      0,      sun1,    sun1,  sun1_state, empty_init, "Sun Microsystems", "Sun-1",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
