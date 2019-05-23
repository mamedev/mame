// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/***************************************************************************

    drivers/genpc.c

    Driver file for generic PC machines

***************************************************************************/

#include "emu.h"
#include "machine/genpc.h"
#include "cpu/nec/nec.h"
#include "cpu/i86/i86.h"
#include "bus/pc_kbd/keyboards.h"
#include "softlist.h"

class genpc_state : public driver_device
{
public:
	genpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	void pcega(machine_config &config);
	void pcvga(machine_config &config);
	void pccga(machine_config &config);
	void pcherc(machine_config &config);
	void pcmda(machine_config &config);
	void pc8_io(address_map &map);
	void pc8_map(address_map &map);

private:
	required_device<cpu_device> m_maincpu;
};

void genpc_state::pc8_map(address_map &map)
{
	map.unmap_value_high();
	map(0xfe000, 0xfffff).rom().region("bios", 0);
}

void genpc_state::pc8_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m("mb", FUNC(ibm5160_mb_device::map));
}

static DEVICE_INPUT_DEFAULTS_START(cga)
	DEVICE_INPUT_DEFAULTS("DSW0",0x30, 0x20)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(vga)
	DEVICE_INPUT_DEFAULTS("DSW0",0x30, 0x00)
DEVICE_INPUT_DEFAULTS_END

void genpc_state::pcmda(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, 4772720);
	m_maincpu->set_addrmap(AS_PROGRAM, &genpc_state::pc8_map);
	m_maincpu->set_addrmap(AS_IO, &genpc_state::pc8_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	IBM5160_MOTHERBOARD(config, "mb", 0).set_cputag(m_maincpu);

	ISA8_SLOT(config, "isa1", 0, "mb:isa", pc_isa8_cards, "mda", false); // FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa2", 0, "mb:isa", pc_isa8_cards, "com", false);
	ISA8_SLOT(config, "isa3", 0, "mb:isa", pc_isa8_cards, "fdc_xt", false);
	ISA8_SLOT(config, "isa4", 0, "mb:isa", pc_isa8_cards, "hdc", false);
	ISA8_SLOT(config, "isa5", 0, "mb:isa", pc_isa8_cards, "adlib", false);
	ISA8_SLOT(config, "isa6", 0, "mb:isa", pc_isa8_cards, nullptr, false);

	/* keyboard */
	PC_KBDC_SLOT(config, "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83).set_pc_kbdc_slot(subdevice("mb:pc_kbdc"));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("640K").set_extra_options("64K, 128K, 256K, 512K");

	/* software lists */
	SOFTWARE_LIST(config, "disk_list").set_original("ibm5150");
}


void genpc_state::pcherc(machine_config &config)
{
	pcmda(config);
	subdevice<isa8_slot_device>("isa1")->set_default_option("hercules");
}


void genpc_state::pccga(machine_config &config)
{
	pcmda(config);
	subdevice<ibm5160_mb_device>("mb")->set_input_default(DEVICE_INPUT_DEFAULTS_NAME(cga));
	subdevice<isa8_slot_device>("isa1")->set_default_option("cga");
}


void genpc_state::pcega(machine_config &config)
{
	pccga(config);
	subdevice<isa8_slot_device>("isa1")->set_default_option("ega");
	subdevice<ibm5160_mb_device>("mb")->set_input_default(DEVICE_INPUT_DEFAULTS_NAME(vga));
}


void genpc_state::pcvga(machine_config &config)
{
	pcega(config);
	subdevice<isa8_slot_device>("isa1")->set_default_option("vga");
}

ROM_START(pc)
	ROM_REGION(0x02000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v31", "Turbo XT BIOS 3.1")
	ROMX_LOAD("pcxtbios31.rom", 0x00000, 0x02000, CRC(8ede74c6) SHA1(5848065f9b40e504c02e6930d3602965b2b1bfad),ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v30", "Turbo XT BIOS 3.0")
	ROMX_LOAD("pcxtbios30.rom", 0x00000, 0x02000, CRC(4e1fd77a) SHA1(36873971c47b242db7edad7a0c1ea2f7f8d43b87),ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v26", "Turbo XT BIOS 2.6")
	ROMX_LOAD("pcxtbios26.rom", 0x00000, 0x02000, CRC(a7505acd) SHA1(1fdd80b09feed0ac59401fd6d8dae6250cb56054),ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v25", "Turbo XT BIOS 2.5")
	ROMX_LOAD("pcxtbios25.rom", 0x00000, 0x02000, CRC(1ab22db6) SHA1(e681acec93c79b08ec06fd26d3be4cccd28f7a45),ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v24", "Turbo XT BIOS 2.4")
	ROMX_LOAD("pcxtbios24.rom", 0x00000, 0x02000, CRC(80e3c43f) SHA1(3f623cf12f3375aa0fa59da84b5137b9fc86c0ce),ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v23", "Turbo XT BIOS 2.3")
	ROMX_LOAD("pcxtbios23.rom", 0x00000, 0x02000, CRC(f397485a) SHA1(777826be2feadb3a8cf7a28ed2245dddef8e1d23),ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v22", "Turbo XT BIOS 2.2")
	ROMX_LOAD("pcxtbios22.rom", 0x00000, 0x02000, CRC(00967678) SHA1(2dd7f6c8236673e471dd456be009dcc43e28a09f),ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "v21", "Turbo XT BIOS 2.1")
	ROMX_LOAD("pcxtbios21.rom", 0x00000, 0x02000, CRC(017f8f61) SHA1(d9696ba16b56685eb51612eddf1a75364acae7af),ROM_BIOS(7))
	ROM_SYSTEM_BIOS(8, "v20", "Turbo XT BIOS 2.0")
	ROMX_LOAD("xtbios2.rom",    0x00000, 0x02000, CRC(1d7bd86c) SHA1(33a500f599b4dad2fe6d7a5c3e89b13bd5dd2987),ROM_BIOS(8))
	ROM_SYSTEM_BIOS(9, "v10", "XT Anonymous Generic Turbo BIOS")
	ROMX_LOAD("pcxt.rom",       0x00000, 0x02000, CRC(031aafad) SHA1(a641b505bbac97b8775f91fe9b83d9afdf4d038f),ROM_BIOS(9))

	// List of bioses to go to separate drivers
	ROM_SYSTEM_BIOS(10, "ami", "XT AMI")
	ROMX_LOAD( "ami.bin", 0x00000, 0x2000, CRC(b381eb22) SHA1(9735193de119270c946a17ed58c3ab9554e0852e),ROM_BIOS(10))
	ROM_SYSTEM_BIOS(11, "award", "XT Award 2.05")
	ROMX_LOAD( "award2.05.bin", 0x00000, 0x2000, CRC(5b3953e5) SHA1(4a36171aa8d993008187f39f732b9296401b7b6c),ROM_BIOS(11))
	ROM_SYSTEM_BIOS(12, "dtk226", "XT DTK Erso bios 2.26")
	ROMX_LOAD( "dtk-ers0.rom", 0x00000, 0x2000, CRC(85fd5e10) SHA1(2ae152f042e7e43e27621f071af763e3f9dc68d2),ROM_BIOS(12))
	ROM_SYSTEM_BIOS(13, "dtk240", "XT DTK Erso bios 2.40") // 8 MHz Turbo
	ROMX_LOAD( "dtk2.40.bin", 0x00000, 0x2000, CRC(a4ed27c3) SHA1(66b67540d94c0d049ebc14ee14eadd2ab7304818),ROM_BIOS(13))
	ROM_SYSTEM_BIOS(14, "dtk242", "XT DTK Erso bios 2.42") // 10 MHz Turbo
	ROMX_LOAD( "dtk2.42.bin", 0x00000, 0x2000, CRC(3f2d2a76) SHA1(02fa057f2c22ab199a8d9795ab1ae570f2b13a36),ROM_BIOS(14))
	ROM_SYSTEM_BIOS(15, "peter", "XT Peter Kohlman 3.75") // V20 Rom only
	ROMX_LOAD( "peterv203.75.bin", 0x00000, 0x2000, CRC(b053a6a4) SHA1(f53218ad3d725f12d9149b22d8afcf6a8869a3bd),ROM_BIOS(15))
	ROM_SYSTEM_BIOS(16, "pho2271", "XT Phoenix Bios 2.27 #1")
	ROMX_LOAD( "phoenix2.27.bin", 0x00000, 0x2000, CRC(168ffef0) SHA1(69465db2f9246a614044d1f433d374506a13a07f),ROM_BIOS(16))
	ROM_SYSTEM_BIOS(17, "pho2272", "XT Phoenix Bios 2.27 #2") // V20 installed on board
	ROMX_LOAD( "compatibility_software_phoenix_technologies_1985_1986_1121277.bin", 0x00000, 0x2000, CRC(33ceb81a) SHA1(7c7db75e61e19025938f30798d9d0f8b4f6ab0ee),ROM_BIOS(17))
	ROM_SYSTEM_BIOS(18, "pho251", "XT Phoenix Bios 2.51")
	ROMX_LOAD( "phoenix2.51.bin", 0x00000, 0x2000, CRC(9b7e9c40) SHA1(c948a8d3d715e469105c6e2acd8b46ec274b25a8),ROM_BIOS(18))
	ROM_SYSTEM_BIOS(19, "turbo", "XT Turbo Bios 3.10")
	ROMX_LOAD( "turbo3.10.bin", 0x00000, 0x2000, CRC(8aaca1e3) SHA1(9c03da16713e08c0112a04c8bdfa394e7341c1fc),ROM_BIOS(19))
	ROM_SYSTEM_BIOS(20, "nestv200", "JUKO NEST v2.00") // use keytronic keyboard
	ROMX_LOAD( "jukoa.bin", 0x00000, 0x2000, CRC(7d78707e) SHA1(8b09a32658a850e7f03254d1328fe6e336e91871),ROM_BIOS(20))
	ROM_SYSTEM_BIOS(21, "nest230", "JUKO NEST v2.30")
	ROMX_LOAD( "juko_st_v2.30.bin", 0x00000, 0x2000, CRC(7a1c6dfa) SHA1(0b343f3028ca06c9e6dc69427d1b15a47c74b9fc),ROM_BIOS(21))
	ROM_SYSTEM_BIOS(22, "nest232", "JUKO NEST v2.32")
	ROMX_LOAD( "xt-juko-st-2.32.bin", 0x00000, 0x2000, CRC(0768524e) SHA1(259520bb7a6796e5b987c2b9bef1acd501df1670),ROM_BIOS(22))
	ROM_SYSTEM_BIOS(23, "sid41", "SID BIOS v4.1") // from X'GOLDEN mainboard
	ROMX_LOAD( "sid_bios_version_v4.1.bin", 0x00000, 0x2000, CRC(c58daf4d) SHA1(7066f8f993500383b99103a9fa1e6c125c89581b),ROM_BIOS(23))
	ROM_SYSTEM_BIOS(24, "txt310", "T U R B O  XT Version 3.10") // from X'GOLDEN Turbo mainboard, computer can operate in 8MHz mode, source mentions possible corruption
	ROMX_LOAD( "turbo_xt_3.10_2764.bin", 0x00000, 0x2000,  BAD_DUMP CRC(8aaca1e3) SHA1(9c03da16713e08c0112a04c8bdfa394e7341c1fc),ROM_BIOS(24))
	ROM_SYSTEM_BIOS(25, "alco", "ALCO 8MHz") // another Phoenix v2.27 variant, probably overdumped, therefore BAD_DUMP
	ROMX_LOAD( "alco8mhz.bin", 0x00000, 0x2000, BAD_DUMP CRC(96a56814) SHA1(7f752cbe1a25ed6ea5f77fed79cfbf608c667dc3),ROM_BIOS(25))
	ROM_SYSTEM_BIOS(26, "american", "American XT 1.32")
	ROMX_LOAD( "americxt.rom", 0x00000, 0x2000, CRC(4c6e23f3) SHA1(6e16f42da9c3d7bd408cf885caf93de9aa02ebe4),ROM_BIOS(26))
	ROM_SYSTEM_BIOS(27, "excel214", "Excel-Turbo Computer Version 2.14") // EXCEL-TURBO SPEEDY SYSTEM / Excel-Turbo Computer 9/20/1985 Version 2.14
	ROMX_LOAD( "excelturbobios.bin", 0x00000, 0x2000, CRC(8ef472a6) SHA1(8f3d512e23ecffb6d9a650d126b11270ff5cf175), ROM_BIOS(27))
	ROM_SYSTEM_BIOS(28, "excel31", "Excel-Turbo Computer Version 3.1") // EXCEL-TURBO SPEEDY SYSTEM / Excel-Turbo Computer 9/20/1985 Version 3.1
	ROMX_LOAD( "excel-turbo_computer_3.1_2764.bin", 0x00000, 0x2000, CRC(d319fea7) SHA1(5b4b0eb35889602aa7f18de82800599528690e15),ROM_BIOS(28))
	ROM_SYSTEM_BIOS(29, "s10b1", "Super 10-B1") // another Phoenix 2.51 variant
	ROMX_LOAD( "super_10-b1_27c64.bin", 0x00000, 0x2000, CRC(ba7797db) SHA1(2ee8863640b860a1807cc41e1ac9d94f73a087aa),ROM_BIOS(29))
	ROM_SYSTEM_BIOS(30, "td386", "TD 3.86")
	ROMX_LOAD( "td3.86_id_75102637.bin", 0x00000, 0x2000, CRC(aec96e13) SHA1(6e3143418f439a0373fba626cf69df34e41815e5),ROM_BIOS(30))
	ROM_SYSTEM_BIOS(31, "td391", "TD 3.91")
	ROMX_LOAD( "tbios-ii.rom", 0x00000, 0x2000, CRC(508b1bad) SHA1(ee9f51423f4cccfdc160c565ecd95fabbcb8a4d4),ROM_BIOS(31))
	ROM_SYSTEM_BIOS(32, "td393", "TD 3.93")
	ROMX_LOAD( "td3.93.bin", 0x00000, 0x2000, CRC(807620d9) SHA1(3f0ca24e33feb32051de9e819b962df1528a0403),ROM_BIOS(32))
	ROM_SYSTEM_BIOS(33, "yangp227", "YANGTECH.INC Phoenix 2.27")
	ROMX_LOAD( "000p001.bin", 0x00000, 0x2000, CRC(16f4fdc8) SHA1(8e73e9d1456aadd65bb89cc813d1aa1354c90d68),ROM_BIOS(33))
	ROM_SYSTEM_BIOS(34, "ethom11f", "ETHOM Associates Version 1.1f") // 8 MHz
	ROMX_LOAD( "ethom_associates_version_1.1f.bin", 0x00000, 0x02000, CRC(bbe7dc12) SHA1(195989a43e6701ff247329524622f1d6f41db7b4),ROM_BIOS(34))
	ROM_SYSTEM_BIOS(35, "arc20", "ARC BIOS 2.0") // ABC Turbo Board - X Turbo System
	ROMX_LOAD( "ibm-artb.rom", 0x00000, 0x2000, CRC(0ae5bf8e) SHA1(79b043070c92f9b2f6f9ca25fe61b4c1fcdf1bc8),ROM_BIOS(35))
	ROM_SYSTEM_BIOS(36, "pho252", "XT Phoenix Bios 2.52")
	ROMX_LOAD( "ibm-phxt.rom", 0x00000, 0x2000, CRC(c0bc9482) SHA1(a527403c92b6bf4fd876f516c18ca499cb7d4b13),ROM_BIOS(36))
	ROM_SYSTEM_BIOS(37, "com33", "IBM Compatible BIOS v3.3")
	ROMX_LOAD( "ibm3-3.rom", 0x00000, 0x02000, CRC(bf6dde1a) SHA1(e63456a888b887b8c0f77f35261ff067f0e2020d),ROM_BIOS(37))
	ROM_SYSTEM_BIOS(38, "xt16", "Turbo BIOS for PC XT-16")
	ROMX_LOAD( "ibmturb.rom", 0x00000, 0x2000, CRC(ba4a711e) SHA1(82fe2f76fd6668d2b38f8e6552a605d70c822792),ROM_BIOS(38))
	ROM_SYSTEM_BIOS(39, "znix", "Z-NIX PC-1600")
	ROMX_LOAD( "ibmzen.rom", 0x00000, 0x2000, CRC(c5468172) SHA1(499a7813f870b04003e246cc90d4a591d043c6bb),ROM_BIOS(39))
	ROM_SYSTEM_BIOS(40, "pcpi", "PC/88 BIOS Ver1.92") // use pcega
	ROMX_LOAD( "pcpi-192.rom", 0x00000, 0x2000, CRC(ef2da5ce) SHA1(95376440be1276e6f1c16fe49c847056bb1e4d5c),ROM_BIOS(40))
	ROM_SYSTEM_BIOS(41, "fday17", "Faraday 5 slot PC") // use pcherc
	ROMX_LOAD( "fdaypc17.rom", 0x00000, 0x2000, CRC(26bb29ac) SHA1(5a58680b9193f4323db3e7894f853dc82d17f4ee),ROM_BIOS(41))
	ROM_SYSTEM_BIOS(42, "tava238", "Tava DTK Erso V2.38")
	ROMX_LOAD( "tava_dtk_erso_bios_2.38_u87.bin", 0x00000, 0x2000, CRC(34f5c0e5) SHA1(5a1590f948670a5ef85a1ee7cbb40387fced8a1f), ROM_BIOS(42))
ROM_END

#define rom_pcmda    rom_pc

#define rom_pcherc   rom_pc

#define rom_pcega    rom_pc

#define rom_pcvga    rom_pc

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME    PARENT   COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME
COMP( 1987, pc,     ibm5150, 0,      pccga,   0,     genpc_state, empty_init, "<generic>", "PC (CGA)",      0 )
COMP( 1987, pcega,  ibm5150, 0,      pcega,   0,     genpc_state, empty_init, "<generic>", "PC (EGA)",      0 )
COMP( 1987, pcmda,  ibm5150, 0,      pcmda,   0,     genpc_state, empty_init, "<generic>", "PC (MDA)",      0 )
COMP( 1987, pcherc, ibm5150, 0,      pcherc,  0,     genpc_state, empty_init, "<generic>", "PC (Hercules)", 0 )
COMP( 1987, pcvga,  ibm5150, 0,      pcvga,   0,     genpc_state, empty_init, "<generic>", "PC (VGA)",      0 )
