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
#include "bus/pc_kbd/pc_kbdc.h"
#include "softlist_dev.h"


namespace {

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
	void pcv20(machine_config &config);
	void pc8_io(address_map &map) ATTR_COLD;
	void pc8_map(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
};

void genpc_state::pc8_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf0000, 0xfffff).rom().region("bios", 0);
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

	ibm5160_mb_device &mb(IBM5160_MOTHERBOARD(config, "mb"));
	mb.set_cputag(m_maincpu);
	mb.int_callback().set_inputline(m_maincpu, 0);
	mb.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	mb.kbdclk_callback().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	mb.kbddata_callback().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	ISA8_SLOT(config, "isa1", 0, "mb:isa", pc_isa8_cards, "mda", false); // FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa2", 0, "mb:isa", pc_isa8_cards, "com", false);
	ISA8_SLOT(config, "isa3", 0, "mb:isa", pc_isa8_cards, "fdc_xt", false);
	ISA8_SLOT(config, "isa4", 0, "mb:isa", pc_isa8_cards, "hdc", false);
	ISA8_SLOT(config, "isa5", 0, "mb:isa", pc_isa8_cards, "adlib", false);
	ISA8_SLOT(config, "isa6", 0, "mb:isa", pc_isa8_cards, nullptr, false);

	/* keyboard */
	pc_kbdc_device &kbd(PC_KBDC(config, "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83));
	kbd.out_clock_cb().set("mb", FUNC(ibm5160_mb_device::keyboard_clock_w));
	kbd.out_data_cb().set("mb", FUNC(ibm5160_mb_device::keyboard_data_w));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("640K").set_extra_options("64K, 128K, 256K, 512K");

	/* software lists */
	SOFTWARE_LIST(config, "disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "hdd_list").set_original("ibm5150_hdd");
}

void genpc_state::pcv20(machine_config &config)
{
	pccga(config);

	v20_device &maincpu(V20(config.replace(), "maincpu", XTAL(14'318'181)/3)); /* 4.77 MHz */
	maincpu.set_addrmap(AS_PROGRAM, &genpc_state::pc8_map);
	maincpu.set_addrmap(AS_IO, &genpc_state::pc8_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));
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
	ROM_REGION(0x10000, "bios", 0)
	// 0: Turbo XT BIOS v3.1 - 10/28/2017
	ROM_SYSTEM_BIOS(0, "v31", "Turbo XT BIOS 3.1")
	ROMX_LOAD("pcxtbios31.rom", 0xe000, 0x02000, CRC(8ede74c6) SHA1(5848065f9b40e504c02e6930d3602965b2b1bfad),ROM_BIOS(0))
	// 1: Turbo XT BIOS v3.0 - 11/09/2016
	ROM_SYSTEM_BIOS(1, "v30", "Turbo XT BIOS 3.0")
	ROMX_LOAD("pcxtbios30.rom", 0xe000, 0x02000, CRC(4e1fd77a) SHA1(36873971c47b242db7edad7a0c1ea2f7f8d43b87),ROM_BIOS(1))
	// 2: Turbo XT BIOS v2.6 for 8088/V20
	ROM_SYSTEM_BIOS(2, "v26", "Turbo XT BIOS 2.6")
	ROMX_LOAD("pcxtbios26.rom", 0xe000, 0x02000, CRC(a7505acd) SHA1(1fdd80b09feed0ac59401fd6d8dae6250cb56054),ROM_BIOS(2))
	// 3: Turbo XT BIOS v2.5 for 8088/V20
	ROM_SYSTEM_BIOS(3, "v25", "Turbo XT BIOS 2.5")
	ROMX_LOAD("pcxtbios25.rom", 0xe000, 0x02000, CRC(1ab22db6) SHA1(e681acec93c79b08ec06fd26d3be4cccd28f7a45),ROM_BIOS(3))
	// 4: Turbo XT BIOS v2.4 for 8088/V20
	ROM_SYSTEM_BIOS(4, "v24", "Turbo XT BIOS 2.4")
	ROMX_LOAD("pcxtbios24.rom", 0xe000, 0x02000, CRC(80e3c43f) SHA1(3f623cf12f3375aa0fa59da84b5137b9fc86c0ce),ROM_BIOS(4))
	// 5: Turbo XT BIOS for 8088/V20
	ROM_SYSTEM_BIOS(5, "v23", "Turbo XT BIOS 2.3")
	ROMX_LOAD("pcxtbios23.rom", 0xe000, 0x02000, CRC(f397485a) SHA1(777826be2feadb3a8cf7a28ed2245dddef8e1d23),ROM_BIOS(5))
	// 6: Turbo XT BIOS 2.2 for 8088/V20
	ROM_SYSTEM_BIOS(6, "v22", "Turbo XT BIOS 2.2")
	ROMX_LOAD("pcxtbios22.rom", 0xe000, 0x02000, CRC(00967678) SHA1(2dd7f6c8236673e471dd456be009dcc43e28a09f),ROM_BIOS(6))
	// 7: Turbo XT BIOS 2.1 for 8088/V20
	ROM_SYSTEM_BIOS(7, "v21", "Turbo XT BIOS 2.1")
	ROMX_LOAD("pcxtbios21.rom", 0xe000, 0x02000, CRC(017f8f61) SHA1(d9696ba16b56685eb51612eddf1a75364acae7af),ROM_BIOS(7))
	// 8: Turbo XT BIOS for 8088/V20
	ROM_SYSTEM_BIOS(8, "v20", "Turbo XT BIOS 2.0")
	ROMX_LOAD("xtbios2.rom",    0xe000, 0x02000, CRC(1d7bd86c) SHA1(33a500f599b4dad2fe6d7a5c3e89b13bd5dd2987),ROM_BIOS(8))
	// 9: Generic Turbo XT BIOS 1987 for 8088 or V20 cpu (c)Anonymous
	ROM_SYSTEM_BIOS(9, "v10", "XT Anonymous Generic Turbo BIOS")
	ROMX_LOAD("pcxt.rom",       0xe000, 0x02000, CRC(031aafad) SHA1(a641b505bbac97b8775f91fe9b83d9afdf4d038f),ROM_BIOS(9))

	// List of bioses to go to separate drivers
	// 10: 8088-BIOS (C)AMI, 1985, 1986 / (C)AMI, 3000-100386
	ROM_SYSTEM_BIOS(10, "ami", "XT AMI")
	ROMX_LOAD( "ami.bin", 0xe000, 0x2000, CRC(b381eb22) SHA1(9735193de119270c946a17ed58c3ab9554e0852e),ROM_BIOS(10))
	// 11: XT BIOS V2.05 COPYRIGHT Award Software Inc. 1986 / SUPERWAVE ELECTRONIC CO., LTD.
	ROM_SYSTEM_BIOS(11, "award", "XT Award 2.05 #1")
	ROMX_LOAD( "award2.05.bin", 0xe000, 0x2000, CRC(5b3953e5) SHA1(4a36171aa8d993008187f39f732b9296401b7b6c),ROM_BIOS(11))
	// 12: Phoenix ROM BIOS Ver 2.27
	ROM_SYSTEM_BIOS(12, "pho2271", "XT Phoenix BIOS 2.27 #1")
	ROMX_LOAD( "phoenix2.27.bin", 0xe000, 0x2000, CRC(168ffef0) SHA1(69465db2f9246a614044d1f433d374506a13a07f),ROM_BIOS(12))
	// 13: Phoenix ROM BIOS Ver 2.27
	ROM_SYSTEM_BIOS(13, "pho2272", "XT Phoenix BIOS 2.27 #2") // V20 installed on board, 8 ISA8 slots
	ROMX_LOAD( "compatibility_software_phoenix_technologies_1985_1986_1121277.bin", 0xe000, 0x2000, CRC(33ceb81a) SHA1(7c7db75e61e19025938f30798d9d0f8b4f6ab0ee),ROM_BIOS(13))
	// 14: Phoenix ROM BIOS Ver 2.51 / Micro-Universe ver 1.0B
	ROM_SYSTEM_BIOS(14, "pho251", "XT Phoenix BIOS 2.51")
	ROMX_LOAD( "phoenix2.51.bin", 0xe000, 0x2000, CRC(9b7e9c40) SHA1(c948a8d3d715e469105c6e2acd8b46ec274b25a8),ROM_BIOS(14))
	// 15: T U R B O - XT 1986 / Version 3.10
	ROM_SYSTEM_BIOS(15, "turbo", "XT Turbo BIOS 3.10")
	ROMX_LOAD( "turbo3.10.bin", 0xe000, 0x2000, CRC(8aaca1e3) SHA1(9c03da16713e08c0112a04c8bdfa394e7341c1fc),ROM_BIOS(15))
	// 16: System 100 ! / S.pecial I.ntegrated D.esigns / BIOS For PC,XT-16 Version 4.1 / (C) 1986
	ROM_SYSTEM_BIOS(16, "sid41", "SID BIOS v4.1") // from X'GOLDEN mainboard
	ROMX_LOAD( "sid_bios_version_v4.1.bin", 0xe000, 0x2000, CRC(c58daf4d) SHA1(7066f8f993500383b99103a9fa1e6c125c89581b),ROM_BIOS(16))
	// 17: System Already !
	ROM_SYSTEM_BIOS(17, "scb12", "Super Computer BIOS 1.2" ) // from X'GOLDEN mainboard
	ROMX_LOAD( "super_computer_bios_1.2_1984.bin", 0xe000, 0x2000, CRC(0768a9ba) SHA1(d05c893e9dfc84a3c11c35f87859429f350571c3), ROM_BIOS(17))
	// 18: T U R B O - XT 1986 / Version 3.10
	ROM_SYSTEM_BIOS(18, "txt310", "T U R B O  XT Version 3.10") // from X'GOLDEN Turbo mainboard, computer can operate in 8MHz mode, source mentions possible corruption
	ROMX_LOAD( "turbo_xt_3.10_2764.bin", 0xe000, 0x2000,  BAD_DUMP CRC(8aaca1e3) SHA1(9c03da16713e08c0112a04c8bdfa394e7341c1fc),ROM_BIOS(18))
	// 19: Phoenix ROM BIOS Ver 2.27
	ROM_SYSTEM_BIOS(19, "alco", "ALCO 8MHz") // another Phoenix v2.27 variant, probably overdumped, therefore BAD_DUMP
	ROMX_LOAD( "alco8mhz.bin", 0xe000, 0x2000, BAD_DUMP CRC(96a56814) SHA1(7f752cbe1a25ed6ea5f77fed79cfbf608c667dc3),ROM_BIOS(19))
	// 20: System   Ready / American XT Computer / (C) 1986 For American XT BIOS V.1.32
	ROM_SYSTEM_BIOS(20, "american", "American XT 1.32")
	ROMX_LOAD( "americxt.rom", 0xe000, 0x2000, CRC(4c6e23f3) SHA1(6e16f42da9c3d7bd408cf885caf93de9aa02ebe4),ROM_BIOS(20))
	// 21: EXCEL-TURBO SPEEDY SYSTEM / EXCEL-TURBO Computer 9/20/1985 Version 2.14
	ROM_SYSTEM_BIOS(21, "excel214", "Excel-Turbo Computer Version 2.14")
	ROMX_LOAD( "excelturbobios.bin", 0xe000, 0x2000, CRC(8ef472a6) SHA1(8f3d512e23ecffb6d9a650d126b11270ff5cf175), ROM_BIOS(21))
	// 22: EXCEL-TURBO SPEEDY SYSTEM / Excel-Turbo Computer 9/20/1985 Version 3.1
	ROM_SYSTEM_BIOS(22, "excel31", "Excel-Turbo Computer Version 3.1")
	ROMX_LOAD( "excel-turbo_computer_3.1_2764.bin", 0xe000, 0x2000, CRC(d319fea7) SHA1(5b4b0eb35889602aa7f18de82800599528690e15),ROM_BIOS(22))
	// 23: Phoenix ROM BIOS Ver 2.51
	ROM_SYSTEM_BIOS(23, "s10b1", "Super 10-B1") // another Phoenix 2.51 variant
	ROMX_LOAD( "super_10-b1_27c64.bin", 0xe000, 0x2000, CRC(ba7797db) SHA1(2ee8863640b860a1807cc41e1ac9d94f73a087aa),ROM_BIOS(23))
	// 24: 86(C) TD3.86 ID: 75102637
	ROM_SYSTEM_BIOS(24, "td386", "TD 3.86")
	ROMX_LOAD( "td3.86_id_75102637.bin", 0xe000, 0x2000, CRC(aec96e13) SHA1(6e3143418f439a0373fba626cf69df34e41815e5),ROM_BIOS(24))
	// 25: 86(C) TD3.91 ID:
	ROM_SYSTEM_BIOS(25, "td391", "TD 3.91")
	ROMX_LOAD( "td391-td.rom", 0xe000, 0x2000, CRC(508b1bad) SHA1(ee9f51423f4cccfdc160c565ecd95fabbcb8a4d4),ROM_BIOS(25))
	// 26: 86(C) TD3.93 ID:
	ROM_SYSTEM_BIOS(26, "td393", "TD 3.93")
	ROMX_LOAD( "td3.93.bin", 0xe000, 0x2000, CRC(807620d9) SHA1(3f0ca24e33feb32051de9e819b962df1528a0403),ROM_BIOS(26))
	// 27: Phoenix ROM BIOS Ver 2.27 / YANGTECH.INC
	ROM_SYSTEM_BIOS(27, "yangp227", "YANGTECH.INC Phoenix 2.27")
	ROMX_LOAD( "000p001.bin", 0xe000, 0x2000, CRC(16f4fdc8) SHA1(8e73e9d1456aadd65bb89cc813d1aa1354c90d68),ROM_BIOS(27))
	// 28: ETHOM Associates Inc. Personal Computer Version 1.1F
	ROM_SYSTEM_BIOS(28, "ethom11f", "ETHOM Associates Version 1.1f") // 8 MHz
	ROMX_LOAD( "ethom_associates_version_1.1f.bin", 0xe000, 0x02000, CRC(bbe7dc12) SHA1(195989a43e6701ff247329524622f1d6f41db7b4),ROM_BIOS(28))
	// 29: ARC Turbo Board - X Turbo System
	ROM_SYSTEM_BIOS(29, "arc20", "ARC BIOS 2.0")
	ROMX_LOAD( "ibm-artb.rom", 0xe000, 0x2000, CRC(0ae5bf8e) SHA1(79b043070c92f9b2f6f9ca25fe61b4c1fcdf1bc8),ROM_BIOS(29))
	// 30:  Phoenix ROM BIOS Ver 2.52
	ROM_SYSTEM_BIOS(30, "pho252", "XT Phoenix BIOS 2.52")
	ROMX_LOAD( "ibm-phxt.rom", 0xe000, 0x2000, CRC(c0bc9482) SHA1(a527403c92b6bf4fd876f516c18ca499cb7d4b13),ROM_BIOS(30))
	// 31: System Already ! IBM COMPATIBLE BIOS v3.3  .......1985
	ROM_SYSTEM_BIOS(31, "com33", "IBM Compatible BIOS v3.3")
	ROMX_LOAD( "ibm3-3.rom", 0xe000, 0x02000, CRC(bf6dde1a) SHA1(e63456a888b887b8c0f77f35261ff067f0e2020d),ROM_BIOS(31))
	// 32: TURBO SYSTEM / Compatible Computer TURBO
	ROM_SYSTEM_BIOS(32, "xt16", "Turbo BIOS for PC XT-16")
	ROMX_LOAD( "ibmturb.rom", 0xe000, 0x2000, CRC(ba4a711e) SHA1(82fe2f76fd6668d2b38f8e6552a605d70c822792),ROM_BIOS(32))
	// 33: Z-NIX PC-1600
	ROM_SYSTEM_BIOS(33, "znix", "Z-NIX PC-1600")
	ROMX_LOAD( "ibmzen.rom", 0xe000, 0x2000, CRC(c5468172) SHA1(499a7813f870b04003e246cc90d4a591d043c6bb),ROM_BIOS(33))
	// 34: PC/88 BIOS Ver1.92
	ROM_SYSTEM_BIOS(34, "pcpi", "PC/88 BIOS Ver1.92") // use pcega
	ROMX_LOAD( "pcpi-192.rom", 0xe000, 0x2000, CRC(ef2da5ce) SHA1(95376440be1276e6f1c16fe49c847056bb1e4d5c),ROM_BIOS(34))
	// 35: no POST screen, takes a few seconds to beep, then boots
	ROM_SYSTEM_BIOS(35, "fday17", "Faraday 5 slot PC")
	ROMX_LOAD( "fdaypc17.rom", 0xe000, 0x2000, CRC(26bb29ac) SHA1(5a58680b9193f4323db3e7894f853dc82d17f4ee),ROM_BIOS(35))
	// 36: (c) E C D Computer GmbH 1985 - BIOS for ECD Professional Microcomputer - use pcherc
	ROM_SYSTEM_BIOS(36, "ecd", "ECD-Computer")
	ROMX_LOAD( "ecd_computer.bin", 0xe000, 0x2000, CRC(caab05f5) SHA1(060aa6c17ff9405c256684cec8a5165227c7c522), ROM_BIOS(36))
	// 37: Triple D International TD-20 - 8088/86 Modular BIOS Ver 3.1jk 06/19&/89 15:42 / Copyright Award Software  Inc.
	ROM_SYSTEM_BIOS(37, "td20", "TD-20")
	ROMX_LOAD( "td20bios.bin", 0xc000, 0x4000, CRC(dfce8cd5) SHA1(c4a9624f230ecdeeee606ee1d0bc685226938505), ROM_BIOS(37))
	// 38: B-190-B' P1.830 810.02, Chipset: Faraday FE2010A ICs: UM8272A, INS8250N-BT, MM58167AN-T
	// 86(C) CD3.98 ID:
	ROM_SYSTEM_BIOS(38, "b190b", "B-190-B")
	ROMX_LOAD( "b190bios.bin", 0xc000, 0x4000, CRC(4178d321) SHA1(a6b30c0805beabe3566b7d22984aa683fc62d7dc), ROM_BIOS(38))
	// 39: XT BIOS V2.05 COPYRIGHT Award Software Inc. 1986
	ROM_SYSTEM_BIOS(39, "kt10mb", "KT 10 M/B") // Award XT BIOS 2.05
	ROMX_LOAD( "kt10bios.bin", 0xe000, 0x2000, CRC(94e9836e) SHA1(793a9359ffd6f0964aa25edce31a3f37aa0dadc8), ROM_BIOS(39))
	// 40: // http://www.vcfed.org/forum/showthread.php?68214-Ruud-s-diagnostic-ROM-for-IBM-PC-XT-and-compatibles
	ROM_SYSTEM_BIOS(40, "diag", "Ruud Baltissen's Diagnostics")
	ROMX_LOAD( "diagrom.bin", 0xe000, 0x2000, CRC(747b1853) SHA1(204a484bc83b3607d5e1404a2dbe629f5f3044b1), ROM_BIOS(40))
	// 41:
	ROM_SYSTEM_BIOS(41, "081682", "08/16/1982")
	ROMX_LOAD( "xt_rom_1_081682_clone.bin", 0xe000, 0x2000, CRC(cfce9b2c) SHA1(14145acb0aca2baf8a6f3c7613f4521fdf0cbe92), ROM_BIOS(41))
	// 42: V20 NEC D70108C-8 - OSC: 14.31818, 24.000 MHz
	// XT BIOS V2.05 COPYRIGHT Award Software Inc. 1986
	ROM_SYSTEM_BIOS(42, "awxt205", "XT Award 2.05 #2")
	ROMX_LOAD( "rom7.u35", 0xe000, 0x2000, CRC(aa3def6b) SHA1(9fb88b6b522d939f7080a567f4a24279ca6c0928), ROM_BIOS(42))
	// 43: 8 MHz TURBO BOARD - ISA8: 8 -
	// American Research Corp., Copyright 1985, ARC TURBO BIOS VERSION 1.23 6/27/85
	ROM_SYSTEM_BIOS(43, "arc123", "ARC Turbo BIOS 1.23")
	ROMX_LOAD( "arcturbobios.bin", 0xe000, 0x2000, CRC(07692e7b) SHA1(27aa350dbc0d846cee8f9149bde0ef72d3862254), ROM_BIOS(43))
	// 44: XT-Faraday PAC - Chipset: Faraday FE2010A-ES, Faraday FE2100, MM58167AN, Z0765A08PSC, NS8250N - CPU: SONY CXQ70108P-8 (V20)
	// OSC: 28.6363, 18.4328.000 - ISA8: 6 - BIOS: PCBIOS 05017 / FARADAY'84'87 / 07017007 - on board: Floppy, ... (ser/par?)
	ROM_SYSTEM_BIOS(44, "pac", "XT-Faraday PAC")
	ROMX_LOAD( "xt-faraday_pac_32k.bin", 0x8000, 0x8000, CRC(d1edf110) SHA1(09570ef36dada08a6d3b97d17ad64814fe32d345), ROM_BIOS(44))
	// 45: AMI XT BIOS
	// 8088-BIOS (C) 1985,1986, AMI - (C)AMI, (1255-013189)
	ROM_SYSTEM_BIOS(45, "amixt", "AMI XT BIOS")
	ROMX_LOAD( "ami_8088_bios_31jan89.bin", 0xe000, 0x2000, CRC(0bcafd1f) SHA1(cb30f01c46dad83343999c609d6f82092e2e8f54), ROM_BIOS(45))
	// 46: From a motherboard marked VIP M X M/10
	// Phoenix ROM BIOS Ver 2.52
	ROM_SYSTEM_BIOS(46, "vipmxm10", "VIP M X M/10")
	ROMX_LOAD( "xt-vip-mxm-10.bin", 0x8000, 0x8000, CRC(6fd64a0a) SHA1(43808f758e9e92d8920e8c3590c3050ec68415aa), ROM_BIOS(46))
	// 47: JUKO BABY XT BXM/12
	ROM_SYSTEM_BIOS(47, "bxm12", "JUKO Baby XT BXM/12")
	ROMX_LOAD("juko_baby_xt_bxm_12.bin", 0xe000, 0x2000, CRC(22d29b06) SHA1(75a504f50e4779d7fc0f0e0b0b1c17d3705cab42), ROM_BIOS(47))
ROM_END

// BIOS versions specifically for NEC V20 CPUs, these don't run on plain 8088
ROM_START( pcv20 )
	ROM_REGION(0x10000, "bios", 0)
	// 0: V20-BIOS Version 3.75 c't // (C) Peter Köhlmann 1987
	ROM_SYSTEM_BIOS(0, "v375", "c't v3.75")
	ROMX_LOAD( "peterv203.75.bin", 0xe000, 0x2000, CRC(b053a6a4) SHA1(f53218ad3d725f12d9149b22d8afcf6a8869a3bd), ROM_BIOS(0))
	// 1: V20-BIOS Version 3.72 c't // (C) Peter Köhlmann 1987 => last known version is 3.82
	ROM_SYSTEM_BIOS(1, "v372", "c't v3.72")
	ROMX_LOAD( "v20xtbios.bin", 0xe000, 0x2000, CRC(b2dca2e4) SHA1(18b0cb90084723eae08cf6b27bfb3fec8e9fb11b), ROM_BIOS(1))
	// 2: Chipset: Vopl TM 215 8750KK - M1101 / M78H012A / 7723 - CPU: NEC 8805F5 V20 D70108C-10
	// OSC: 32.000000MHz, 14.31818, 16.000MHz
	ROM_SYSTEM_BIOS(2, "v365", "c't v3.65")
	ROMX_LOAD( "xt_ls-1720_u52.bin", 0xe000, 0x2000, CRC(7082371a) SHA1(9965dbae5fa4355bc6325ac27a9acc176cc454c3), ROM_BIOS(2))
	// ROM_LOAD( "xt_ls-1720_u8.bin", 0x0000, 0x2000, CRC(aa1d3916) SHA1(bb1723fc637d5d8a9af82b2bdd9e3b11689f0cb9)))
	ROM_SYSTEM_BIOS(3, "glabios_0.24", "GLaBIOS 0.24") // Open Source XT clone BIOS under GPL3 https://github.com/640-KB/GLaBIOS
	// Versions of this BIOS exist for 8088, V20, pure emulation, homebrew projects, XT chipsets and genuine IBM 5150/5160 machines
	ROMX_LOAD( "glabios_0.2.4_vt.rom", 0xe000, 0x2000, CRC(7c173fe3) SHA1(4ac6ca07453890e02c203617fbbdeefb53098cdb), ROM_BIOS(3))
ROM_END


#define rom_pcmda    rom_pc

#define rom_pcherc   rom_pc

#define rom_pcega    rom_pc

#define rom_pcvga    rom_pc

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME    PARENT   COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME
COMP( 1987, pc,     ibm5150, 0,      pccga,   0,     genpc_state, empty_init, "<generic>", "PC (CGA)",        0 )
COMP( 1987, pcega,  ibm5150, 0,      pcega,   0,     genpc_state, empty_init, "<generic>", "PC (EGA)",        0 )
COMP( 1987, pcmda,  ibm5150, 0,      pcmda,   0,     genpc_state, empty_init, "<generic>", "PC (MDA)",        0 )
COMP( 1987, pcherc, ibm5150, 0,      pcherc,  0,     genpc_state, empty_init, "<generic>", "PC (Hercules)",   0 )
COMP( 1987, pcvga,  ibm5150, 0,      pcvga,   0,     genpc_state, empty_init, "<generic>", "PC (VGA)",        0 )
COMP( 1987, pcv20,  ibm5150, 0,      pcv20,   0,     genpc_state, empty_init, "<generic>", "PC with V20 CPU", 0 )
