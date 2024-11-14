// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Callan PM68K Unix mainframe.

2013-09-04 Skeleton driver

Status: Boots into monitor, some commands work, some freeze.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "bus/rs232/rs232.h"
#include "machine/am9513.h"
#include "machine/z80sio.h"


namespace {

class pm68k_state : public driver_device
{
public:
	pm68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_base(*this, "rambase")
		, m_maincpu(*this, "maincpu")
	{ }

	void pm68k(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_shared_ptr<uint16_t> m_p_base;
	required_device<cpu_device> m_maincpu;
};


void pm68k_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).ram().share("rambase");
	map(0x200000, 0x205fff).rom().region("roms", 0);
	map(0x600000, 0x600007).rw("mpsc", FUNC(i8274_device::ba_cd_r), FUNC(i8274_device::ba_cd_w)).umask16(0xff00);
	map(0x800000, 0x800003).rw("stc", FUNC(am9513_device::read16), FUNC(am9513_device::write16));
}


/* Input ports */
static INPUT_PORTS_START( pm68k )
INPUT_PORTS_END


void pm68k_state::machine_reset()
{
	uint8_t* ROM = memregion("roms")->base();
	memcpy(m_p_base, ROM, 8);
}

void pm68k_state::pm68k(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pm68k_state::mem_map);

	i8274_device& mpsc(I8274(config, "mpsc", 0));
	mpsc.out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	mpsc.out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	mpsc.out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	mpsc.out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	mpsc.out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	mpsc.out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));

	am9513_device &stc(AM9513(config, "stc", 4000000));
	stc.out4_cb().set("mpsc", FUNC(i8274_device::rxca_w));
	stc.out4_cb().append("mpsc", FUNC(i8274_device::txca_w));
	stc.out5_cb().set("mpsc", FUNC(i8274_device::rxcb_w));
	stc.out5_cb().append("mpsc", FUNC(i8274_device::txcb_w));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set("mpsc", FUNC(i8274_device::rxa_w));
	rs232a.dsr_handler().set("mpsc", FUNC(i8274_device::dcda_w));
	rs232a.cts_handler().set("mpsc", FUNC(i8274_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("mpsc", FUNC(i8274_device::rxb_w));
	rs232b.dsr_handler().set("mpsc", FUNC(i8274_device::dcdb_w));
	rs232b.cts_handler().set("mpsc", FUNC(i8274_device::ctsb_w));
}

/* ROM definition */
ROM_START( pm68k )
	ROM_REGION16_BE(0x6000, "roms", 0)
	ROM_LOAD16_BYTE("u103", 0x00000, 0x1000, CRC(86d32d6c) SHA1(ce9c54b62c64c37ae9106fb06b8a2b2152d1ddf6) )
	ROM_LOAD16_BYTE("u101", 0x00001, 0x1000, CRC(66607e54) SHA1(06f380fdeba13dc3aee826dd166f4bd3031febb9) )
	ROM_LOAD16_BYTE("u104", 0x02000, 0x2000, CRC(ccd2ba4d) SHA1(5cdcf875e136aa9af5f150e0102cd209c496885e) )
	ROM_LOAD16_BYTE("u102", 0x02001, 0x2000, CRC(48182abd) SHA1(a6e4fb62c5f04cb397c6c3294723ec1f7bc3b680) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE INPUT  CLASS        INIT        COMPANY                FULLNAME  FLAGS
COMP( 198?, pm68k, 0,      0,      pm68k,  pm68k, pm68k_state, empty_init, "Callan Data Systems", "PM68K",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
