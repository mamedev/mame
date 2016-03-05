// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic, Carl

#include "emu.h"
#include "cpu/i86/i286.h"
#include "cpu/i386/i386.h"
#include "machine/at.h"
#include "machine/ram.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"

class ps2_state : public driver_device
{
public:
	ps2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_ram(*this, RAM_TAG)
		{ }
	required_device<cpu_device> m_maincpu;
	required_device<at_mb_device> m_mb;
	required_device<ram_device> m_ram;

protected:
	void machine_start() override;
};

static ADDRESS_MAP_START( ps2_16_map, AS_PROGRAM, 16, ps2_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x09ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x0e0000, 0x0fffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0xfe0000, 0xffffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ps2_32_map, AS_PROGRAM, 32, ps2_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x000e0000, 0x000fffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(ps2_16_io, AS_IO, 16, ps2_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE("mb", at_mb_device, map)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ps2_32_io, AS_IO, 32, ps2_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE16("mb", at_mb_device, map, 0xffffffff)
ADDRESS_MAP_END

void ps2_state::machine_start()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);

	/* MESS managed RAM */
	membank("bank10")->set_base(m_ram->pointer());

	if (m_ram->size() > 0xa0000)
	{
		offs_t ram_limit = 0x100000 + m_ram->size() - 0xa0000;
		space.install_read_bank(0x100000,  ram_limit - 1, "bank1");
		space.install_write_bank(0x100000,  ram_limit - 1, "bank1");
		membank("bank1")->set_base(m_ram->pointer() + 0xa0000);
	}
}

static MACHINE_CONFIG_START( ps2m30286, ps2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, 10000000)
	MCFG_CPU_PROGRAM_MAP(ps2_16_map)
	MCFG_CPU_IO_MAP(ps2_16_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259_master", pic8259_device, inta_cb)
	MCFG_80286_SHUTDOWN(DEVWRITELINE("mb", at_mb_device, shutdown))

	MCFG_DEVICE_ADD("mb", AT_MB, 0)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))
	MCFG_FRAGMENT_ADD( at_softlists )

	MCFG_ISA16_SLOT_ADD("mb:isabus","isa1", pc_isa16_cards, "vga", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa2", pc_isa16_cards, "fdc", false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa3", pc_isa16_cards, "ide", false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa4", pc_isa16_cards, "comat", false)
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_IBM_PC_AT_84)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1664K")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M,15M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ps2386, ps2_state )
	MCFG_CPU_ADD("maincpu", I386, 12000000)
	MCFG_CPU_PROGRAM_MAP(ps2_32_map)
	MCFG_CPU_IO_MAP(ps2_32_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259_master", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("mb", AT_MB, 0)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))
	MCFG_FRAGMENT_ADD( at_softlists )

	// on board devices
	MCFG_ISA16_SLOT_ADD("mb:isabus","board1", pc_isa16_cards, "fdcsmc", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board2", pc_isa16_cards, "comat", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board3", pc_isa16_cards, "ide", true)
	MCFG_ISA16_SLOT_ADD("mb:isabus","board4", pc_isa16_cards, "lpt", true)
	// ISA cards
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa1", pc_isa16_cards, "svga_et4k", false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa2", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa3", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa4", pc_isa16_cards, nullptr, false)
	MCFG_ISA16_SLOT_ADD("mb:isabus","isa5", pc_isa16_cards, nullptr, false)
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1664K")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M,15M,16M,32M,64M,128M,256M")
MACHINE_CONFIG_END

ROM_START( i8530286 )
	ROM_REGION(0x20000,"bios", 0)
	// saved from running machine
	ROM_LOAD16_BYTE("ps2m30.0", 0x00000, 0x10000, CRC(9965a634) SHA1(c237b1760f8a4561ec47dc70fe2e9df664e56596))
	ROM_LOAD16_BYTE("ps2m30.1", 0x00001, 0x10000, CRC(1448d3cb) SHA1(13fa26d895ce084278cd5ab1208fc16c80115ebe))
ROM_END

/*

8530-H31 (Model 30/286)
======================
  P/N          Date
33F5381A EC C01446 1990

*/
ROM_START( i8530h31 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD( "33f5381a.bin", 0x00000, 0x20000, CRC(ff57057d) SHA1(d7f1777077a8df43c3c14d175b9709bd3969c4b1))
ROM_END

/*
8535-043 (Model 35)
===================
  P/N    Checksum     Date
04G2021    C26C       1991    ODD
04G2022    9B94       1991    EVEN
*/
ROM_START( i8535043 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "04g2021.bin", 0x00001, 0x10000, CRC(4069b2eb) SHA1(9855c84c81d1f07e1da66b1ca45c1c10c0717a90))
	ROM_LOAD16_BYTE( "04g2022.bin", 0x00000, 0x10000, CRC(35c1af65) SHA1(7d2445cc463969c808fdd78e0a27a03db5dfc698))
ROM_END

/*
8550-021 (Model 50)
===================
 Code     Date       Internal
90X7420  4/12/87 --> 90X6815
90X7423  8/12/87 --> 90X6816
90X7426  8/12/87 --> 90X6817
90X7429 18/12/87 --> 90X6818

Same ROMs used by : (According to http://www.ibmmuseum.com/ohlandl/8565/8560.html)

IBM Personal System/2 Model 60 (8560-041 and 8560-071)
IBM Personal System/2 Model 65 SX (8565-061 and 8565-121)

*/
ROM_START( i8550021 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "90x7423.zm14", 0x00000, 0x8000, CRC(2c1633e0) SHA1(1af7faa526585a7cfb69e71d90a75e1f1c541586))
	ROM_LOAD16_BYTE( "90x7426.zm16", 0x00001, 0x8000, CRC(e7c762ce) SHA1(228f67dc915d84519da7fc1a59b7f9254278f3a0))
	ROM_LOAD16_BYTE( "90x7420.zm13", 0x10000, 0x8000, CRC(19a57cc1) SHA1(5b31ba66cd3690e651a450619a32b7210769945d))
	ROM_LOAD16_BYTE( "90x7429.zm18", 0x10001, 0x8000, CRC(6f0120f6) SHA1(e112c291ac3d9f6507c93ac49ad26f9fd2245fd2))

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "72x8455.zm82", 0x000, 0x800, CRC(7da223d3) SHA1(54c52ff6c6a2310f79b2c7e6d1259be9de868f0e) )
ROM_END

/*
8550-061 (Model 50Z)
===================
                  P/N              Date
AMI 8935MKN     15F8365    S63512  1988
AMI 8948MML     15F8366    S63512  1988

http://ps-2.kev009.com:8081/ohlandl/8550/8550z_Planar.html


*/
ROM_START( i8550061 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "15f8365.zm5", 0x00001, 0x10000, CRC(35aa3ecf) SHA1(a122531092a9cb08600b276da9c9c3ce385aab7b))
	ROM_LOAD16_BYTE( "15f8366.zm6", 0x00000, 0x10000, CRC(11bf564d) SHA1(0dda6a7ca9294cfaab5bdf4c05973be13b2766fc))
ROM_END

/*
8555-X61 (Model 55SX)
===================
         Code     Date       Internal
ODD     33F8145  13/03/90 --> 33F8153
EVEN    33F8146  31/01/90 --> 33F8152

8555-081 (Model 55SX)
===================
                         Code          Date    Internal
ODD     AMI 9205MEN     92F0627 EC32680 88 --> 33F8153
EVEN    AMI 9203MGS     92F0626 EC32680 88 --> 33F8152

*/
ROM_START( i8555081 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE("33f8145.zm40", 0x00001, 0x10000, CRC(0895894c) SHA1(7cee77828867ad1bdbe0ac223bc25d23c65b28a0))
	ROM_LOAD16_BYTE("33f8146.zm41", 0x00000, 0x10000, CRC(c6020680) SHA1(b25a64e4b2dca07c567648401100e04e89bbcddb))
ROM_END

/*
8580-071 (Model 80)
===================
                  Code    Date      Internal
AMI 8924MBW     90X8548   1987  --> 72X7551
AMI 8924MBL     90X8549   1987  --> 72X7554
AMI 8924MBG     90X8550   1987  --> 72X7557
AMI 8921MBK     90X8551   1987  --> 72X7560
*/
ROM_START( i8580071 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD32_BYTE( "90x8548.bin", 0x00000, 0x8000, CRC(1f13eea5) SHA1(0bf53ad86f47db3825a713ea2e4ef23715cc4f79))
	ROM_LOAD32_BYTE( "90x8549.bin", 0x00001, 0x8000, CRC(9e0f4a99) SHA1(b8600f04159ed281a57416274390ba9302be541b))
	ROM_LOAD32_BYTE( "90x8550.bin", 0x00002, 0x8000, CRC(cb21df96) SHA1(0c2765f6becfa3f9171c4f13f7b74d19c4c9acc2))
	ROM_LOAD32_BYTE( "90x8551.bin", 0x00003, 0x8000, CRC(3d7e9868) SHA1(2928fe0e48a573cc2c0c41bd7f7188a54a908229))
ROM_END

/*
8580-111 (Model 80)
===================
                 Code    Date    Internal
AMI 8934MDL     15F6637  1987 --> 15F6597
AMI 8944MDI     15F6639  1987 --> 15F6600
*/
ROM_START( i8580111 )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD16_BYTE( "15f6637.bin", 0x00000, 0x10000, CRC(76c36d1a) SHA1(c68d52a2e5fbd303225ebb006f91869b29ef700a))
	ROM_LOAD16_BYTE( "15f6639.bin", 0x00001, 0x10000, CRC(82cf0f7d) SHA1(13bb39225757b89749af70e881af0228673dbe0c))
ROM_END

COMP ( 1990, i8530h31, ibm5170, 0,       ps2m30286, 0, driver_device, 0,  "International Business Machines",  "IBM PS/2 8530-H31 (Model 30/286)", MACHINE_NOT_WORKING )
COMP ( 1988, i8530286, ibm5170, 0,       ps2m30286, 0, driver_device, 0,  "International Business Machines",  "IBM PS/2 Model 30-286", MACHINE_NOT_WORKING )
COMP ( 198?, i8535043, ibm5170, 0,       ps2386,    0, driver_device, 0,  "International Business Machines",  "IBM PS/2 8535-043 (Model 35)", MACHINE_NOT_WORKING )
COMP ( 198?, i8550021, ibm5170, 0,       ps2386,    0, driver_device, 0,  "International Business Machines",  "IBM PS/2 8550-021 (Model 50)", MACHINE_NOT_WORKING )
COMP ( 198?, i8550061, ibm5170, 0,       ps2386,    0, driver_device, 0,  "International Business Machines",  "IBM PS/2 8550-061 (Model 50Z)", MACHINE_NOT_WORKING )
COMP ( 1989, i8555081, ibm5170, 0,       ps2386,    0, driver_device, 0,  "International Business Machines",  "IBM PS/2 8550-081 (Model 55SX)", MACHINE_NOT_WORKING )
COMP ( 198?, i8580071, ibm5170, 0,       ps2386,    0, driver_device, 0,  "International Business Machines",  "IBM PS/2 8580-071 (Model 80)", MACHINE_NOT_WORKING )
COMP ( 198?, i8580111, ibm5170, 0,       ps2386,    0, driver_device, 0,  "International Business Machines",  "IBM PS/2 8580-111 (Model 80)", MACHINE_NOT_WORKING )
