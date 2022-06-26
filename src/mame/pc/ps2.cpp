// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic, Carl, Katherine Rohl

/*****************************************************
 * The IBM Personal System/2
 * MCA systems
 * 
 * Status:
 * 
 * Model 50/50Z boots DOS and the reference disk. Memory ID doesn't work right.
 * Model 80 boots DOS and the reference disk but the memory split stuff doesn't work right so it won't do diagnostics.
 * 
 * 16-bit MCA seems to work fine.
 * 32-bit MCA is a work-in-progress.
 *****************************************************/

#include "emu.h"
#include "cpu/i86/i286.h"
#include "cpu/i386/i386.h"
#include "machine/at.h"
#include "machine/ibmps2.h"
#include "machine/ram.h"
#include "bus/isa/isa_cards.h"
#include "bus/mca/mca.h"
#include "bus/mca/mca_cards.h"
#include "softlist_dev.h"

#include "ps2m50.h"
#include "ps2m80.h"

// According to http://nerdlypleasures.blogspot.com/2014/04/the-original-8-bit-ide-interface.html
// the IBM PS/2 Model 25-286 and Model 30-286 use a customised version of the XTA (8-bit IDE) harddisk interface


namespace {
	
#define PLANAR_ID_M50		0xFBFF
#define PLANAR_ID_M80_TYPE1 0xFEFF
#define PLANAR_ID_M80_TYPE2 0xFDFF
#define PLANAR_ID_M80_TYPE3 0xFFF9

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
	required_device<ps2_mb_device> m_mb;
	required_device<ram_device> m_ram;

	// Model 35
	void ibm8535_043(machine_config &config);

	// Model 50
	void ibm8550_021(machine_config &config);
	void ibm8550_061(machine_config &config);
	
	// Model 55
	void ibm8555_081(machine_config &config);
	void ibm8555_x61(machine_config &config);

	// Model 60
	void ibm8560(machine_config &config);

	// Model 80
	void ibm8580_071(machine_config &config);
	void ibm8580_111(machine_config &config);
	void ibm8580_a21(machine_config &config);

protected:
	void machine_start() override;
	void machine_reset() override;

	void at_softlists(machine_config &config);
	void ps2_io(address_map &map);
	void ps2_286_map(address_map &map);
	void ps2_386_map(address_map &map);
};

void ps2_state::at_softlists(machine_config &config)
{
	/* software lists */
	SOFTWARE_LIST(config, "pc_disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "at_disk_list").set_original("ibm5170");
	SOFTWARE_LIST(config, "at_cdrom_list").set_original("ibm5170_cdrom");
	SOFTWARE_LIST(config, "at_hdd_list").set_original("ibm5170_hdd");
	SOFTWARE_LIST(config, "midi_disk_list").set_compatible("midi_flop");
}

void ps2_state::ps2_286_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x09ffff).bankrw("bank10");
	map(0x0e0000, 0x0fffff).rom().region("bios", 0);
	map(0xfe0000, 0xffffff).rom().region("bios", 0);
}

void ps2_state::ps2_386_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0009ffff).bankrw("bank10");
	map(0x000e0000, 0x000fffff).rom().region("bios", 0);
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);
}

void ps2_state::ps2_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).m(m_mb, FUNC(ps2_mb_device::map));
}

void ps2_state::machine_reset()
{
	
}

void ps2_state::machine_start()
{
	/* managed RAM */
	membank("bank10")->set_base(m_ram->pointer());
}

/* 
	PS/2 Model 35SX

	80386SX-based second-generation PS/2s. 
	
	Based on the VLSI 82C300 chipset.

	Skeleton.
 */

void ps2_state::ibm8535_043(machine_config &config)
{
	I386SX(config, m_maincpu, 20'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ps2_state::ps2_386_map);
	m_maincpu->set_addrmap(AS_IO, &ps2_state::ps2_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("1M").set_extra_options("2M,4M,8M,15M,16M,32M,64M,128M,256M");

	PS2_M50_T1_MB(config, m_mb, 20'000'000); // wrong
	m_mb->set_cpu_tag(m_maincpu);
	m_mb->set_ram_tag(m_ram);

	config.set_maximum_quantum(attotime::from_hz(60));
	at_softlists(config);
}

/*
	PS/2 Model 50

	286-based first-generation PS/2.
 */

void ps2_state::ibm8550_021(machine_config &config)
{
	// Model 50 (Type 1)

	I80286(config, m_maincpu, 10'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ps2_state::ps2_286_map);
	m_maincpu->set_addrmap(AS_IO, &ps2_state::ps2_io);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("1M");

	PS2_M50_T1_MB(config, m_mb, 10_MHz_XTAL);
	m_mb->set_cpu_tag(m_maincpu);
	m_mb->set_ram_tag(m_ram);

	m_maincpu->set_irq_acknowledge_callback(m_mb, FUNC(ps2_mb_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));
	at_softlists(config);
}

void ps2_state::ibm8550_061(machine_config &config)
{
	// Model 50Z (Type 2)

	I80286(config, m_maincpu, 10'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ps2_state::ps2_286_map);
	m_maincpu->set_addrmap(AS_IO, &ps2_state::ps2_io);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("1M").set_extra_options("2M,4M,8M,15M,16M,32M,64M,128M,256M");

	PS2_M50_T1_MB(config, m_mb, 10_MHz_XTAL);
	m_mb->set_cpu_tag(m_maincpu);
	m_mb->set_ram_tag(m_ram);

	m_maincpu->set_irq_acknowledge_callback(m_mb, FUNC(ps2_mb_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));
	at_softlists(config);
}

/*
	PS/2 Model 55SX

	80386SX-based first-generation PS/2.

	Status: Skeleton.
 */

void ps2_state::ibm8555_x61(machine_config &config)
{
	I386SX(config, m_maincpu, 16'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ps2_state::ps2_386_map);
	m_maincpu->set_addrmap(AS_IO, &ps2_state::ps2_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("1M").set_extra_options("2M,4M,8M,15M,16M,32M,64M,128M,256M");

	//PS2_MB(config, m_mb, 16000000);
	//m_mb->set_cpu_tag(m_maincpu);
	//m_mb->set_ram_tag(m_ram);

	config.set_maximum_quantum(attotime::from_hz(60));
	at_softlists(config);
}

void ps2_state::ibm8555_081(machine_config &config)
{
	I386SX(config, m_maincpu, 16'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ps2_state::ps2_386_map);
	m_maincpu->set_addrmap(AS_IO, &ps2_state::ps2_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("1M").set_extra_options("2M,4M,8M,15M,16M,32M,64M,128M,256M");

	PS2_M50_T1_MB(config, m_mb, 16000000); // wrong
	m_mb->set_cpu_tag(m_maincpu);
	m_mb->set_ram_tag(m_ram);

	config.set_maximum_quantum(attotime::from_hz(60));
	at_softlists(config);
}

/* 	
	PS/2 Model 60

	Model 50, but in a tower case.
*/

void ps2_state::ibm8560(machine_config &config)
{
	I80286(config, m_maincpu, 10'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ps2_state::ps2_286_map);
	m_maincpu->set_addrmap(AS_IO, &ps2_state::ps2_io);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("1M");

	PS2_M60_MB(config, m_mb, 10_MHz_XTAL);
	m_mb->set_cpu_tag(m_maincpu);
	m_mb->set_ram_tag(m_ram);

	m_maincpu->set_irq_acknowledge_callback(m_mb, FUNC(ps2_mb_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));
	at_softlists(config);
}


/*	
	PS/2 Model 80

	The Model 80 is a little weird even compared to other PS/2 systems.
	Planar RAM is provided by two Eurocard-connector daughtercards of 1MB, 2MB, or 4MB.

	The PS/2 Model 80 had three different planars over the course of its life.
	Type 1 - 16MHz 386, max of 4MB of planar RAM, supports matched memory cycles but not 4MB cards.
	Type 2 - 20MHz 386, max of 8MB of planar RAM.
	Type 3 - 25MHz 386, 64K of L2 cache, max of 8MB of planar RAM, does not support 1MB memory cards.

	32-bit IBM MCA memory cards are capable of operating with Matched Memory Cycles.
	On the Type 1 planar, RAM accesses will take two 16MHz bus cycles plus one wait state.
	On the other planars, RAM accesses will always take two 10MHz bus cycles.

	Status:
	Type 1: Memory split isn't hooked up properly, so it doesn't work right outside of DOS.
	Type 2: Memory hardware not implemented.
	Type 3: Totally different planar, not started yet.	
	*/


void ps2_state::ibm8580_071(machine_config &config)
{
	// IBM 8580 + Type 1 Planar = Model 071
	I386(config, m_maincpu, 32_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ps2_state::ps2_386_map);
	m_maincpu->set_addrmap(AS_IO, &ps2_state::ps2_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("1M");

	PS2_M80_T1_MB(config, m_mb, 32_MHz_XTAL);
	m_mb->set_cpu_tag(m_maincpu);
	m_mb->set_ram_tag(m_ram);

	config.set_maximum_quantum(attotime::from_hz(60));
	at_softlists(config);
}

void ps2_state::ibm8580_111(machine_config &config)
{
	// IBM 8580 + Type 2 Planar = Model 111
	I386(config, m_maincpu, 40_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ps2_state::ps2_386_map);
	m_maincpu->set_addrmap(AS_IO, &ps2_state::ps2_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("1M,2M,8M");

	PS2_M80_T2_MB(config, m_mb, 40_MHz_XTAL);
	m_mb->set_cpu_tag(m_maincpu);
	m_mb->set_ram_tag(m_ram);

	config.set_maximum_quantum(attotime::from_hz(60));
	at_softlists(config);
}

void ps2_state::ibm8580_a21(machine_config &config)
{
	// IBM 8580 + Type 3 Planar = Model A21
	I386(config, m_maincpu, 50_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ps2_state::ps2_386_map);
	m_maincpu->set_addrmap(AS_IO, &ps2_state::ps2_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("2M,4M,8M");

	PS2_M80_T3_MB(config, m_mb, 50_MHz_XTAL);
	m_mb->set_cpu_tag(m_maincpu);
	m_mb->set_ram_tag(m_ram);

	config.set_maximum_quantum(attotime::from_hz(60));
	at_softlists(config);
}

/*
8535-043 (Model 35SX)
===================
  P/N    Checksum     Date
04G2021    C26C       1991    ODD
04G2022    9B94       1991    EVEN
*/

ROM_START( ibm8535_043 )
	ROM_REGION16_LE(0x20000, "bios", 0)
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
ROM_START( ibm8550_021 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "90x7423.zm14", 0x00000, 0x8000, CRC(2c1633e0) SHA1(1af7faa526585a7cfb69e71d90a75e1f1c541586))
	ROM_LOAD16_BYTE( "90x7426.zm16", 0x00001, 0x8000, CRC(e7c762ce) SHA1(228f67dc915d84519da7fc1a59b7f9254278f3a0))
	ROM_LOAD16_BYTE( "90x7420.zm13", 0x10000, 0x8000, CRC(19a57cc1) SHA1(5b31ba66cd3690e651a450619a32b7210769945d))
	ROM_LOAD16_BYTE( "90x7429.zm18", 0x10001, 0x8000, CRC(6f0120f6) SHA1(e112c291ac3d9f6507c93ac49ad26f9fd2245fd2))
ROM_END

/*
8550-061 (Model 50Z)
===================
                  P/N              Date
AMI 8935MKN     15F8365    S63512  1988
AMI 8948MML     15F8366    S63512  1988

http://ps-2.kev009.com:8081/ohlandl/8550/8550z_Planar.html
*/
ROM_START( ibm8550_061 )
	ROM_REGION16_LE(0x20000, "bios", 0)
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
ROM_START( ibm8555_081 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE("33f8145.zm40", 0x00001, 0x10000, CRC(0895894c) SHA1(7cee77828867ad1bdbe0ac223bc25d23c65b28a0))
	ROM_LOAD16_BYTE("33f8146.zm41", 0x00000, 0x10000, CRC(c6020680) SHA1(b25a64e4b2dca07c567648401100e04e89bbcddb))
ROM_END

ROM_START( ibm8555_x61 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE("33f8145.zm40", 0x00001, 0x10000, CRC(0895894c) SHA1(7cee77828867ad1bdbe0ac223bc25d23c65b28a0))
	ROM_LOAD16_BYTE("33f8146.zm41", 0x00000, 0x10000, CRC(c6020680) SHA1(b25a64e4b2dca07c567648401100e04e89bbcddb))
ROM_END

/*
8550-021 (Model 60)
===================
 Code     Date       Internal
90X7420  4/12/87 --> 90X6815
90X7423  8/12/87 --> 90X6816
90X7426  8/12/87 --> 90X6817
90X7429 18/12/87 --> 90X6818
*/
ROM_START( ibm8560 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "90x6816.bin", 0x00000, 0x8000, CRC(2c1633e0) SHA1(1af7faa526585a7cfb69e71d90a75e1f1c541586))
	ROM_LOAD16_BYTE( "90x6817.bin", 0x00001, 0x8000, CRC(e7c762ce) SHA1(228f67dc915d84519da7fc1a59b7f9254278f3a0))
	ROM_LOAD16_BYTE( "90x6815.bin", 0x10000, 0x8000, CRC(19a57cc1) SHA1(5b31ba66cd3690e651a450619a32b7210769945d))
	ROM_LOAD16_BYTE( "90x6818.bin", 0x10001, 0x8000, CRC(6f0120f6) SHA1(e112c291ac3d9f6507c93ac49ad26f9fd2245fd2))
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
ROM_START( ibm8580_071 )
	ROM_REGION32_LE(0x20000, "bios", 0)
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
ROM_START( ibm8580_111 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "15f6637.bin", 0x00000, 0x10000, CRC(76c36d1a) SHA1(c68d52a2e5fbd303225ebb006f91869b29ef700a))
	ROM_LOAD16_BYTE( "15f6639.bin", 0x00001, 0x10000, CRC(82cf0f7d) SHA1(13bb39225757b89749af70e881af0228673dbe0c))
ROM_END

/*
8580-A21 (Model 80)
===================
                 Code    Date    Internal
EVEN		    64F3084  1989 --> 64F3084
ODD			    64F3085  1989 --> 64F3085
*/
ROM_START( ibm8580_a21 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "64f3084.bin", 0x00000, 0x10000, CRC(76c36d1a) SHA1(c68d52a2e5fbd303225ebb006f91869b29ef700a))
	ROM_LOAD16_BYTE( "64f3085.bin", 0x00001, 0x10000, CRC(82cf0f7d) SHA1(13bb39225757b89749af70e881af0228673dbe0c))
ROM_END
} // anonymous namespace

COMP( 1991, ibm8535_043, 0,        		ibm5170, 	ibm8535_043, 0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8535-043 (Model 35SX)", MACHINE_IS_SKELETON )
COMP( 1987, ibm8550_021, ibm8550_061, 	0,       	ibm8550_021, 0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8550-021 (Model 50)", MACHINE_NOT_WORKING )
COMP( 1988, ibm8550_061, 0,        		ibm5170, 	ibm8550_061, 0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8550-061 (Model 50Z)", MACHINE_NOT_WORKING )
COMP( 1990, ibm8555_x61, 0,				ibm5170,	ibm8555_x61, 0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8555-081 (Model 55SX)", MACHINE_IS_SKELETON);
COMP( 1988, ibm8555_081, ibm8555_x61,	ibm5170,	ibm8555_081, 0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8555-X61 (Model 55SX)", MACHINE_IS_SKELETON);
COMP( 1988, ibm8560, 	 ibm8550_021,	ibm5170,	ibm8560,     0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8560 (Model 60)", MACHINE_NOT_WORKING);
COMP( 1987, ibm8580_071, 0,           	ibm5170, 	ibm8580_071, 0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8580 (Model 80, Type 1)", MACHINE_NOT_WORKING )
COMP( 1987, ibm8580_111, ibm8580_071, 	0,       	ibm8580_111, 0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8580 (Model 80, Type 2)", MACHINE_NOT_WORKING )
COMP( 1989, ibm8580_a21, ibm8580_071, 	0, 	   		ibm8580_a21, 0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8580 (Model 80, Type 3)", MACHINE_NOT_WORKING )
