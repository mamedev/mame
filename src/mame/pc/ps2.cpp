// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic, Carl

#include "emu.h"
#include "cpu/i86/i286.h"
#include "cpu/i386/i386.h"
#include "machine/at.h"
#include "machine/ram.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "softlist_dev.h"

// According to http://nerdlypleasures.blogspot.com/2014/04/the-original-8-bit-ide-interface.html
// the IBM PS/2 Model 25-286 and Model 30-286 use a customised version of the XTA (8-bit IDE) harddisk interface


namespace {

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

	void ps2m30286(machine_config &config);
	void ps2386(machine_config &config);
	void ps2386sx(machine_config &config);
	void at_softlists(machine_config &config);
	void ps2_16_io(address_map &map) ATTR_COLD;
	void ps2_16_map(address_map &map) ATTR_COLD;
	void ps2_32_io(address_map &map) ATTR_COLD;
	void ps2_32_map(address_map &map) ATTR_COLD;
protected:
	void machine_start() override ATTR_COLD;
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

void ps2_state::ps2_16_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x09ffff).bankrw("bank10");
	map(0x0e0000, 0x0fffff).rom().region("bios", 0);
	map(0xfe0000, 0xffffff).rom().region("bios", 0);
}

void ps2_state::ps2_32_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0009ffff).bankrw("bank10");
	map(0x000e0000, 0x000fffff).rom().region("bios", 0);
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);
}

void ps2_state::ps2_16_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
}

void ps2_state::ps2_32_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
}

void ps2_state::machine_start()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);

	/* managed RAM */
	membank("bank10")->set_base(m_ram->pointer());

	if (m_ram->size() > 0xa0000)
	{
		offs_t ram_limit = 0x100000 + m_ram->size() - 0xa0000;
		space.install_ram(0x100000,  ram_limit - 1, m_ram->pointer() + 0xa0000);
	}
}

void ps2_state::ps2m30286(machine_config &config)
{
	/* basic machine hardware */
	i80286_cpu_device &maincpu(I80286(config, m_maincpu, 10000000));
	maincpu.set_addrmap(AS_PROGRAM, &ps2_state::ps2_16_map);
	maincpu.set_addrmap(AS_IO, &ps2_state::ps2_16_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));
	maincpu.shutdown_callback().set("mb", FUNC(at_mb_device::shutdown));

	AT_MB(config, m_mb);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	config.set_maximum_quantum(attotime::from_hz(60));

	at_softlists(config);

	// FIXME: determine ISA bus clock
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "vga", true);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, "fdc", false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, "ide", false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, "comat", false);

	pc_kbdc_device &kbd(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_IBM_PC_AT_84));
	kbd.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	kbd.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("1664K").set_extra_options("2M,4M,8M,15M");
}

void ps2_state::ps2386(machine_config &config)
{
	I386(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ps2_state::ps2_32_map);
	m_maincpu->set_addrmap(AS_IO, &ps2_state::ps2_32_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	AT_MB(config, m_mb);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	config.set_maximum_quantum(attotime::from_hz(60));
	at_softlists(config);

	// on board devices
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "fdcsmc", true); // FIXME: determine ISA bus clock
	ISA16_SLOT(config, "board2", 0, "mb:isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "mb:isabus", pc_isa16_cards, "ide", true);
	ISA16_SLOT(config, "board4", 0, "mb:isabus", pc_isa16_cards, "lpt", true);
	// ISA cards
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false);

	pc_kbdc_device &kbd(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	kbd.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	kbd.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("1664K").set_extra_options("2M,4M,8M,15M,16M,32M,64M,128M,256M");
}

void ps2_state::ps2386sx(machine_config &config)
{
	ps2386(config);
	I386SX(config.replace(), m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ps2_state::ps2_16_map);
	m_maincpu->set_addrmap(AS_IO, &ps2_state::ps2_16_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));
}

ROM_START( i8530286 )
	ROM_REGION16_LE(0x20000, "bios", 0)
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
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "33f5381a.bin", 0x00000, 0x20000, CRC(ff57057d) SHA1(d7f1777077a8df43c3c14d175b9709bd3969c4b1))
ROM_END

/*
8535-043 (Model 35SX)
===================
  P/N    Checksum     Date
04G2021    C26C       1991    ODD
04G2022    9B94       1991    EVEN
*/
ROM_START( i8535043 )
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
ROM_START( i8550021 )
	ROM_REGION16_LE(0x20000, "bios", 0)
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
ROM_START( i8555081 )
	ROM_REGION16_LE(0x20000, "bios", 0)
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
ROM_START( i8580111 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "15f6637.bin", 0x00000, 0x10000, CRC(76c36d1a) SHA1(c68d52a2e5fbd303225ebb006f91869b29ef700a))
	ROM_LOAD16_BYTE( "15f6639.bin", 0x00001, 0x10000, CRC(82cf0f7d) SHA1(13bb39225757b89749af70e881af0228673dbe0c))
ROM_END

} // anonymous namespace


COMP( 1990, i8530h31, 0,        ibm5170, ps2m30286, 0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8530-H31 (Model 30/286)", MACHINE_NOT_WORKING )
COMP( 1988, i8530286, i8530h31, 0,       ps2m30286, 0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 Model 30-286", MACHINE_NOT_WORKING )
COMP( 198?, i8535043, 0,        ibm5170, ps2386sx,  0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8535-043 (Model 35SX)", MACHINE_NOT_WORKING )
COMP( 198?, i8550021, i8550061, 0,       ps2m30286, 0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8550-021 (Model 50)", MACHINE_NOT_WORKING )
COMP( 198?, i8550061, 0,        ibm5170, ps2m30286, 0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8550-061 (Model 50Z)", MACHINE_NOT_WORKING )
COMP( 1989, i8555081, 0,        ibm5170, ps2386sx,  0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8555-081 (Model 55SX)", MACHINE_NOT_WORKING )
COMP( 198?, i8580071, 0,        ibm5170, ps2386,    0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8580-071 (Model 80)", MACHINE_NOT_WORKING )
COMP( 198?, i8580111, 0,        ibm5170, ps2386,    0, ps2_state, empty_init, "International Business Machines", "IBM PS/2 8580-111 (Model 80)", MACHINE_NOT_WORKING )
