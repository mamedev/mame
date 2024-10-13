// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/****************************************************************************

    Skeleton driver for Sony BETACAM-SP Videocassette Players and Recorders

    List of major ICs:
     - IC202 - H8/534 (Hitachi Single-Chip Microcomputer)
     - IC227 - MB88201 (MB88200 Series CMOS Low end Single-Chip 4-Bit Microprocessor)
     - IC211 - CXD1095BQ (C-MOS I/O PORT EXPANDER)
     - IC226 - CXD1095BQ (C-MOS I/O PORT EXPANDER)
     - IC100 - CXD8384Q (C-MOS LTC READER/GENERATOR)
     - IC219 - CXD2202Q (SERVO IC)
     - IC213 - LC3564BM-10 (Sanyo 64Kbit SRAM (8192-word x8-bit))
     - IC1 - D70320GJ-8 (CPU NEC V25)
     - IC3 - LC3564BM-10 (Sanyo 64Kbit SRAM (8192-word x8-bit))
     - IC5 - CXD8176AQ (C-MOS DUAL PORT RAM CONTROLLER)
     - IC15 - D6453GT (MOS INTEGRATED CIRCUIT CMOS LSI FOR 12 lines X 24 columns CHARACTER DISPLAY ON SCREEN)

****************************************************************************/

#include "emu.h"
#include "cpu/nec/v25.h"
#include "cpu/h8500/h8534.h"
#include "machine/cxd1095.h"


namespace {

class betacam_state : public driver_device
{
public:
	betacam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_systemcpu(*this, "systemcpu")
		, m_servocpu(*this, "servocpu")
	{
	}

	void betacam(machine_config &config);

private:
	void system_mem_map(address_map &map) ATTR_COLD;
	void servo_mem_map(address_map &map) ATTR_COLD;

	required_device<v25_device> m_systemcpu;
	required_device<h8534_device> m_servocpu;
};


void betacam_state::system_mem_map(address_map &map)
{
	map(0x17f00, 0x17fff).ram();                        //   256b ?
	map(0x18000, 0x189fe).ram();                        //   8kb SRAM at IC3
	map(0x189ff, 0x189ff).noprw();                      //   ZERO
	map(0x18a00, 0x19fff).ram();                        //   8kb SRAM at IC3 (init SS:SP)
	map(0x20800, 0x20fff).ram().share("svram");         //   2kb servo dual-port SRAM
	map(0x21800, 0x2183f).ram().share("ltcram");        //   64b LTC SRAM
	map(0xe0000, 0xfffff).rom().region("systemcpu", 0); // 128kb EPROM at IC4
}


void betacam_state::servo_mem_map(address_map &map)
{
	map(0x00000, 0x3ffff).rom().region("servocpu", 0); //guessed
	map(0x40000, 0x41fff).ram(); //guessed
	//map(0x?????, 0x?????).rw("cxdio0", FUNC(cxd1095_device::read), FUNC(cxd1095_device::write));
	//map(0x?????, 0x?????).rw("cxdio1", FUNC(cxd1095_device::read), FUNC(cxd1095_device::write));
}


static INPUT_PORTS_START(betacam)
	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW1:1" )
	PORT_DIPNAME( 0x02, 0x00, "RGB Output Sel" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "RGB Input Sel" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x08, 0x00, "Wide" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x10, 0x00, "R/P E/F" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x20, 0x20, "Rec/Player" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x40, 0x00, "J U/C" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x80, 0x00, "NTSC / PAL" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
INPUT_PORTS_END

void betacam_state::betacam(machine_config &config)
{
	V25(config, m_systemcpu, 16_MHz_XTAL); // NEC upD70320GJ-8
	m_systemcpu->set_addrmap(AS_PROGRAM, &betacam_state::system_mem_map);
	m_systemcpu->p2_in_cb().set_ioport("DSW1");

	HD6435348(config, m_servocpu, 20_MHz_XTAL); //Actual chip is marked "H8/534 6435348F 10"
	m_servocpu->set_mode(3);
	m_servocpu->set_addrmap(AS_PROGRAM, &betacam_state::servo_mem_map);

	//CXD1095(config, "cxdio0");
	//CXD1095(config, "cxdio1");
}

ROM_START(uvw1200)
	ROM_REGION(0x20000, "systemcpu", 0)
	ROM_LOAD("75932697_uvw-1000_sy_v2.00_f8b4.ic4", 0x00000, 0x20000, CRC(08e9b891) SHA1(3e01f0e037e83825dcb4a745ddc9148cf3cc7674))

	ROM_REGION(0x40000, "servocpu", 0)
	ROM_LOAD("75953491_uvw-1000_sv_v2.04_f024.ic212", 0x00000, 0x40000, CRC(dc2b8d4b) SHA1(f10a3dc0c317582e3dcb6f3dcc741c0d55c6fd22))
ROM_END

ROM_START(uvw1600)
	ROM_REGION(0x20000, "systemcpu", 0)
	ROM_LOAD("75925907_uvw-1000_sy_v1.03_e3b4.ic4", 0x00000, 0x20000, CRC(f9e575ce) SHA1(2f802c5685f7ce00586079ad5bc456083c595d66))

	ROM_REGION(0x40000, "servocpu", 0)
	ROM_LOAD("75927098_uvw-1000_sv_v1.04_150c.ic212", 0x00000, 0x40000, CRC(b4cb9c02) SHA1(92ae5ce303b9f67977b960047bac7f6bb337b8c0))
ROM_END

ROM_START(uvw1800)
	ROM_REGION(0x20000, "systemcpu", 0)
	ROM_LOAD("75925907_uvw-1000_sy_v1.03_e3b4.ic4", 0x00000, 0x20000, CRC(f9e575ce) SHA1(2f802c5685f7ce00586079ad5bc456083c595d66))

	ROM_REGION(0x40000, "servocpu", 0)
	ROM_LOAD("75927098_uvw-1000_sv_v1.04_150c.ic212", 0x00000, 0x40000, CRC(b4cb9c02) SHA1(92ae5ce303b9f67977b960047bac7f6bb337b8c0))
ROM_END

} // anonymous namespace


//   YEAR  NAME   PARENT/COMPAT MACHINE  INPUT    CLASS          INIT       COMPANY  FULLNAME                                                FLAGS
SYST(199?, uvw1200,   0, 0,     betacam, betacam, betacam_state, empty_init, "Sony", "BETACAM-SP Videocassette Player UVW-1200 RGB",          MACHINE_IS_SKELETON)
SYST(199?, uvw1600,   0, 0,     betacam, betacam, betacam_state, empty_init, "Sony", "BETACAM-SP Videocassette Player/Recorder UVW-1600 RGB", MACHINE_IS_SKELETON)
SYST(199?, uvw1800,   0, 0,     betacam, betacam, betacam_state, empty_init, "Sony", "BETACAM-SP Videocassette Player/Recorder UVW-1800 RGB", MACHINE_IS_SKELETON)
