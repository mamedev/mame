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
	ROM_SYSTEM_BIOS(0, "v30", "Turbo XT BIOS 3.0")
	ROMX_LOAD("pcxtbios30.rom", 0x00000, 0x02000, CRC(4e1fd77a) SHA1(36873971c47b242db7edad7a0c1ea2f7f8d43b87),ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v26", "Turbo XT BIOS 2.6")
	ROMX_LOAD("pcxtbios26.rom", 0x00000, 0x02000, CRC(a7505acd) SHA1(1fdd80b09feed0ac59401fd6d8dae6250cb56054),ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v25", "Turbo XT BIOS 2.5")
	ROMX_LOAD("pcxtbios25.rom", 0x00000, 0x02000, CRC(1ab22db6) SHA1(e681acec93c79b08ec06fd26d3be4cccd28f7a45),ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v24", "Turbo XT BIOS 2.4")
	ROMX_LOAD("pcxtbios24.rom", 0x00000, 0x02000, CRC(80e3c43f) SHA1(3f623cf12f3375aa0fa59da84b5137b9fc86c0ce),ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v23", "Turbo XT BIOS 2.3")
	ROMX_LOAD("pcxtbios23.rom", 0x00000, 0x02000, CRC(f397485a) SHA1(777826be2feadb3a8cf7a28ed2245dddef8e1d23),ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v22", "Turbo XT BIOS 2.2")
	ROMX_LOAD("pcxtbios22.rom", 0x00000, 0x02000, CRC(00967678) SHA1(2dd7f6c8236673e471dd456be009dcc43e28a09f),ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v21", "Turbo XT BIOS 2.1")
	ROMX_LOAD("pcxtbios21.rom", 0x00000, 0x02000, CRC(017f8f61) SHA1(d9696ba16b56685eb51612eddf1a75364acae7af),ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "v20", "Turbo XT BIOS 2.0")
	ROMX_LOAD("xtbios2.rom",    0x00000, 0x02000, CRC(1d7bd86c) SHA1(33a500f599b4dad2fe6d7a5c3e89b13bd5dd2987),ROM_BIOS(7))
	ROM_SYSTEM_BIOS(8, "v10", "XT Anonymous Generic Turbo BIOS")
	ROMX_LOAD("pcxt.rom",       0x00000, 0x02000, CRC(031aafad) SHA1(a641b505bbac97b8775f91fe9b83d9afdf4d038f),ROM_BIOS(8))

	// List of bioses to go to separate drivers
	ROM_SYSTEM_BIOS(9, "ami", "XT AMI")
	ROMX_LOAD( "ami.bin", 0x00000, 0x2000, CRC(b381eb22) SHA1(9735193de119270c946a17ed58c3ab9554e0852e),ROM_BIOS(9))
	ROM_SYSTEM_BIOS(10, "award", "XT Award 2.05")
	ROMX_LOAD( "award2.05.bin", 0x00000, 0x2000, CRC(5b3953e5) SHA1(4a36171aa8d993008187f39f732b9296401b7b6c),ROM_BIOS(10))
	ROM_SYSTEM_BIOS(11, "dtk", "XT DTK Erso bios 2.42")
	ROMX_LOAD( "dtk2.42.bin", 0x00000, 0x2000, CRC(3f2d2a76) SHA1(02fa057f2c22ab199a8d9795ab1ae570f2b13a36),ROM_BIOS(11))
	ROM_SYSTEM_BIOS(12, "peter", "XT Peter Kohlman 3.75") // V20 Rom only
	ROMX_LOAD( "peterv203.75.bin", 0x00000, 0x2000, CRC(b053a6a4) SHA1(f53218ad3d725f12d9149b22d8afcf6a8869a3bd),ROM_BIOS(12))
	ROM_SYSTEM_BIOS(13, "pho227", "XT Phoenix Bios 2.27")
	ROMX_LOAD( "phoenix2.27.bin", 0x00000, 0x2000, CRC(168ffef0) SHA1(69465db2f9246a614044d1f433d374506a13a07f),ROM_BIOS(13))
	ROM_SYSTEM_BIOS(14, "pho251", "XT Phoenix Bios 2.51")
	ROMX_LOAD( "phoenix2.51.bin", 0x00000, 0x2000, CRC(9b7e9c40) SHA1(c948a8d3d715e469105c6e2acd8b46ec274b25a8),ROM_BIOS(14))
	ROM_SYSTEM_BIOS(15, "turbo", "XT Turbo Bios 3.10")
	ROMX_LOAD( "turbo3.10.bin", 0x00000, 0x2000, CRC(8aaca1e3) SHA1(9c03da16713e08c0112a04c8bdfa394e7341c1fc),ROM_BIOS(15))
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
