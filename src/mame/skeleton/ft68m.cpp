// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Forward Technology FT-68M Multibus card.

2013-09-26 Skeleton driver

Chips: HD68000-10, uPD7201C, AM9513APC. Crystal: 19.6608 MHz

Interrupts: INT6 is output of Timer 2, INT7 is output of Timer 3 (refresh),
            INT5 comes from SIO.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "bus/rs232/rs232.h"
#include "machine/am9513.h"
#include "machine/z80sio.h"


namespace {

class ft68m_state : public driver_device
{
public:
	ft68m_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_base(*this, "rambase")
		, m_maincpu(*this, "maincpu")
	{
	}

	void ft68m(machine_config &config);

private:
	uint16_t switches_r();

	void mem_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_shared_ptr<uint16_t> m_p_base;

	required_device<cpu_device> m_maincpu;
};

uint16_t ft68m_state::switches_r()
{
	return 0x7c00; // bypass self test
}


void ft68m_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xffffff);
	map(0x000000, 0x1fffff).ram().share("rambase");
	map(0x200000, 0x201fff).rom().region("roms", 0x0000);
	map(0x400000, 0x401fff).rom().region("roms", 0x2000);
	map(0x600000, 0x600007).mirror(0x1ffff8).rw("mpsc", FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask16(0xff00);
	map(0x800000, 0x800003).mirror(0x1ffffc).rw("stc", FUNC(am9513_device::read16), FUNC(am9513_device::write16));
	map(0xa00000, 0xbfffff).ram(); //Page Map
	map(0xc00000, 0xdfffff).ram(); //Segment Map
	map(0xe00000, 0xffffff).r(FUNC(ft68m_state::switches_r)); //Context Register
}


/* Input ports */
static INPUT_PORTS_START( ft68m )
INPUT_PORTS_END


void ft68m_state::machine_start()
{
	// GATE 1 is tied to Vcc; other GATE and SRC pins are all grounded
	subdevice<am9513_device>("stc")->gate1_w(1);
}

void ft68m_state::machine_reset()
{
	uint8_t* ROM = memregion("roms")->base();
	memcpy(m_p_base, ROM, 8);
}

void ft68m_state::ft68m(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(19'660'800) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ft68m_state::mem_map);

	upd7201_device& mpsc(UPD7201(config, "mpsc", 0));
	mpsc.out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	mpsc.out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	mpsc.out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	mpsc.out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	mpsc.out_int_callback().set_inputline(m_maincpu, M68K_IRQ_5);

	am9513_device &stc(AM9513A(config, "stc", XTAL(19'660'800) / 8));
	stc.out2_cb().set_inputline(m_maincpu, M68K_IRQ_6);
	stc.out3_cb().set_inputline(m_maincpu, M68K_IRQ_7);
	stc.out4_cb().set("mpsc", FUNC(upd7201_device::rxca_w));
	stc.out4_cb().append("mpsc", FUNC(upd7201_device::txca_w));
	stc.out5_cb().set("mpsc", FUNC(upd7201_device::rxcb_w));
	stc.out5_cb().append("mpsc", FUNC(upd7201_device::txcb_w));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set("mpsc", FUNC(upd7201_device::rxa_w));
	rs232a.dsr_handler().set("mpsc", FUNC(upd7201_device::dcda_w));
	rs232a.cts_handler().set("mpsc", FUNC(upd7201_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("mpsc", FUNC(upd7201_device::rxb_w));
}

/* ROM definition */
ROM_START( ft68m )
	ROM_REGION16_BE(0x4000, "roms", 0)
	ROM_LOAD16_BYTE("23-0009-01c.a4", 0x0000, 0x1000, CRC(0d45fc8d) SHA1(59587cb1c151bfd0d69e708716ed3b0a78aa85ea) )
	ROM_LOAD16_BYTE("23-0008-01c.a1", 0x0001, 0x1000, CRC(d1aa1164) SHA1(05e10f1c594e2acd369949b873a524a9cc37829f) )
	ROM_LOAD16_BYTE( "33-01.a6", 0x2000, 0x1000, CRC(53fe3c73) SHA1(ad15c74cd8edef9d9716ad0d16f7a95ff2af901f) )
	ROM_LOAD16_BYTE( "33-00.a3", 0x2001, 0x1000, CRC(06b1cc77) SHA1(12e3314e92f800b3c4ebdf55dcd5351230224788) )

	ROM_REGION(0x700, "proms", 0)
	ROM_LOAD("23-0010-00.a15", 0x000, 0x020, CRC(20eb1183) SHA1(9b268792b28d858d6b6a1b6c4148af88a8d6b735) )
	ROM_LOAD("23-0011-00.a14", 0x100, 0x200, CRC(12d9a6be) SHA1(fca99f9c5afc630ac67cbd4e5ba4e5242b826848) )
	ROM_LOAD("23-0012-00.a16", 0x300, 0x020, CRC(ee1e5a14) SHA1(0d3346cb3b647fa2475bd7b4fa36ea6ecfdaf805) )
	ROM_LOAD("23-0034-00.e4",  0x400, 0x100, CRC(1a573887) SHA1(459bd2d8dc8c4b1c0a529984ae8e38d0c81a084c) )
	ROM_LOAD("23-0037-00.e7",  0x500, 0x100, CRC(9ed4b7f6) SHA1(136a74567094d8462c3a4de1b7e6eb8f30fe71ca) )
	ROM_LOAD("23-0038-00.f1",  0x600, 0x100, CRC(3e56cce5) SHA1(f30a8d5d744bfc25493cd1e92961bbb75f9e0d05) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT   CLASS        INIT        COMPANY               FULLNAME  FLAGS
COMP( 198?, ft68m, 0,      0,      ft68m,   ft68m,  ft68m_state, empty_init, "Forward Technology", "FT-68M", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
