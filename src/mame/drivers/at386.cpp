// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/***************************************************************************

IBM AT compatibles using a 386 class CPU
split from at.cpp

***************************************************************************/

#include "emu.h"

/* mingw-gcc defines this */
#ifdef i386
#undef i386
#endif /* i386 */

#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i386/i386.h"
#include "machine/at.h"
#include "machine/cs8221.h"
#include "machine/ds128x.h"
// #include "machine/idectrl.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "emupal.h"
#include "softlist_dev.h"
#include "speaker.h"

class at386_state : public driver_device
{
public:
	at386_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_ram(*this, RAM_TAG)
	{ }

	void at386(machine_config &config);
	void at386l(machine_config &config);
	void pg750(machine_config &config);

	void init_at386();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<at_mb_device> m_mb;
	required_device<ram_device> m_ram;

	void init_at386_common(int xmsbase);

	void at386_io(address_map &map);
	void at386_map(address_map &map);
	void at386l_map(address_map &map);
	void at386l_io(address_map &map);
};

void at386_state::at386_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0009ffff).bankrw("bank10");
	map(0x000e0000, 0x000fffff).rom().region("bios", 0);
	map(0x00800000, 0x00800bff).ram().share("nvram");
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);
}

void at386_state::at386_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
}

void at386_state::at386l_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0009ffff).bankrw("bank10");
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0x00800000, 0x00800bff).ram().share("nvram");
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0x20000);
}

void at386_state::at386l_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(at_mb_device::map));
}

/**********************************************************
 *
 * Init functions
 *
 **********************************************************/

void at386_state::init_at386_common(int xmsbase)
{
	address_space& space = m_maincpu->space(AS_PROGRAM);

	/* MESS managed RAM */
	membank("bank10")->set_base(m_ram->pointer());

	if (m_ram->size() > xmsbase)
	{
		offs_t ram_limit = 0x100000 + m_ram->size() - xmsbase;
		space.install_ram(0x100000,  ram_limit - 1, m_ram->pointer() + xmsbase);
	}
}

void at386_state::init_at386()
{
	init_at386_common(0xa0000);
}

/**********************************************************
 *
 * Machine configurations
 *
 **********************************************************/

void at386_state::at386(machine_config &config)
{
	i386_device &maincpu(I386(config, m_maincpu, 12'000'000));
	maincpu.set_addrmap(AS_PROGRAM, &at386_state::at386_map);
	maincpu.set_addrmap(AS_IO, &at386_state::at386_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// on-board devices
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "fdcsmc", true); // FIXME: deteremine ISA bus clock
	ISA16_SLOT(config, "board2", 0, "mb:isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "mb:isabus", pc_isa16_cards, "ide", true);
	ISA16_SLOT(config, "board4", 0, "mb:isabus", pc_isa16_cards, "lpt", true);
	// ISA cards
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("1664K").set_extra_options("2M,4M,8M,15M,16M,32M,64M");
}

// Amstrad PC2386
void at386_state::at386l(machine_config &config)
{
	i386_device &maincpu(I386(config, m_maincpu, 12'000'000));
	maincpu.set_addrmap(AS_PROGRAM, &at386_state::at386l_map);
	maincpu.set_addrmap(AS_IO, &at386_state::at386l_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// on-board devices
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "fdcsmc", true); // FIXME: deteremine ISA bus clock
	ISA16_SLOT(config, "board2", 0, "mb:isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "mb:isabus", pc_isa16_cards, "ide", true);
	ISA16_SLOT(config, "board4", 0, "mb:isabus", pc_isa16_cards, "lpt", true);
	// ISA cards
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("1664K").set_extra_options("2M,4M,8M,15M,16M,32M,64M");
}

// Siemens PG 750
void at386_state::pg750(machine_config &config)
{
	i386_device &maincpu(I386(config, m_maincpu, 12'000'000));
	maincpu.set_addrmap(AS_PROGRAM, &at386_state::at386_map);
	maincpu.set_addrmap(AS_IO, &at386_state::at386_io);
	maincpu.set_irq_acknowledge_callback("mb:pic8259_master", FUNC(pic8259_device::inta_cb));

	config.set_maximum_quantum(attotime::from_hz(60));

	AT_MB(config, m_mb).at_softlists(config);
	m_mb->kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	ds12885_device &rtc(DS12885(config.replace(), "mb:rtc")); // TODO: move this into the cs8221
	rtc.irq().set("mb:pic8259_slave", FUNC(pic8259_device::ir0_w)); // this is in :mb
	rtc.set_century_index(0x32);

	// on-board devices
	ISA16_SLOT(config, "board1", 0, "mb:isabus", pc_isa16_cards, "fdc", true); // .set_option_machine_config("fdc", cfg_dual_1440K); // FIXME: deteremine ISA bus clock
	ISA16_SLOT(config, "board2", 0, "mb:isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "mb:isabus", pc_isa16_cards, "lpt", true);
	// ISA cards
	ISA16_SLOT(config, "isa1", 0, "mb:isabus", pc_isa16_cards, "ega", false);
	ISA16_SLOT(config, "isa2", 0, "mb:isabus", pc_isa16_cards, "hdc", false);
	ISA16_SLOT(config, "isa3", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "mb:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "mb:isabus", pc_isa16_cards, nullptr, false);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(at_mb_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(at_mb_device::kbd_data_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("3712K");
}

/***************************************************************************
  ROM DEFINITIONS
***************************************************************************/

/***************************************************************************
  Apricot systems

  http://bbs.actapricot.org/files/ , http://insight.actapricot.org/insight/products/main.htm

***************************************************************************/

// Apricot XEN-S (Venus I Motherboard 386)
// BIOS-String: apricot / Phoenix ROM BIOS PLUS Version 3.10.17 / Apricot XEN-S 25th June 1992
// MAME message: char SEL checker, contact MAMEdev
ROM_START( xb42664 )
	/* actual VGA BIOS not dumped */
	ROM_REGION32_LE(0x20000, "bios", 0)
	// XEN-S (Venus I Motherboard)
	ROM_LOAD16_BYTE( "3-10-17i.lo", 0x10000, 0x8000, CRC(3786ca1e) SHA1(c682d7c76f234559d03bcf21010c13c4dbeafb69))
	ROM_LOAD16_BYTE( "3-10-17i.hi", 0x10001, 0x8000, CRC(d66710eb) SHA1(e8c1cd5f9ecfbd8825655e416d7ddf2ae362e69b))
ROM_END

// Apricot XEN-S (Venus II Motherboard 386)
// BIOS-String: apricot XEN-S Series Personal Computer / Phoenix ROM BIOS PLUS VERSION 3.10.04 / XEN-S II BIOS VR 1.2.17 16th October 1990
ROM_START( xb42664a )
	/* actual VGA BIOS not dumped*/
	ROM_REGION32_LE(0x20000, "bios", 0)
	// XEN-S (Venus II Motherboard)
	ROM_LOAD16_BYTE( "10217.lo", 0x10000, 0x8000, CRC(ea53406f) SHA1(2958dfdbda14de4e6b9d6a8c3781131ab1e32bef))
	ROM_LOAD16_BYTE( "10217.hi", 0x10001, 0x8000, CRC(111725cf) SHA1(f6018a45bda4476d40c5881fb0a506ff75ec1688))
ROM_END

// Apricot Qi 300 (Rev D,E & F Motherboard) - no display
ROM_START( xb42663 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "qi310223.lo", 0x00000, 0x10000, CRC(53047f49) SHA1(7b38e533f7f27295269549c63e5477d950239167))
	ROM_LOAD16_BYTE( "qi310223.hi", 0x00001, 0x10000, CRC(4852869f) SHA1(98599d4691d40b3fac2936034c70b386ce4caf77))
ROM_END

// Apricot Qi 600 (Neptune Motherboard) - no display, beep code L-1-1-3 (Extended CMOS RAM failure)
ROM_START( qi600 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "qi610223.lo", 0x00000, 0x10000, CRC(563114a9) SHA1(62932b3bf0b5502ff708f604c21773f00afda58e))
	ROM_LOAD16_BYTE( "qi610223.hi", 0x00001, 0x10000, CRC(0ae133f6) SHA1(6039c366f7fe0ebf60b34c1a7d6b2d781b664001))
ROM_END

// Apricot LANstation (Krypton Motherboard) - no display
ROM_START( aplanst )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "31024", "Bios 3-10-24")
	ROMX_LOAD( "31024.lo", 0x10000, 0x8000, CRC(e52b59e1) SHA1(cfcaa4d8d658df8df463108ef30695bd4ee7a617), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "31024.hi", 0x10001, 0x8000, CRC(7286aefa) SHA1(dfc0e3f4936780fa62ae9ec392ce17aa65e717cd), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "31025", "Bios 3-10-25")
	ROMX_LOAD( "31025.lo", 0x10000, 0x8000, CRC(1aec09bc) SHA1(51d56c97c7c1674554aa89b68945329ea967a8bc), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "31025.hi", 0x10001, 0x8000, CRC(0763caa5) SHA1(48510a933dcd6efea3b14d04444f584c3e6fefeb), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "31026", "Bios 3-10-26i")
	ROMX_LOAD( "31026i.lo", 0x10000, 0x8000, CRC(670b6ab4) SHA1(8d61a0edf187f99b67eb58f5e11276deee801d17), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "31026i.hi", 0x10001, 0x8000, CRC(ef01c54f) SHA1(911f95d65ab96878e5e7ebccfc4b329db47a1351), ROM_SKIP(1) | ROM_BIOS(2))
ROM_END

// Apricot LANstation (Novell Remote Boot) - no display
ROM_START( aplannb )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "lsl31025.lo", 0x00000, 0x10000, CRC(8bb7229b) SHA1(31449d12884ec4e7752e6c1ce7ce9e0d044eadf2))
	ROM_LOAD16_BYTE( "lsh31025.hi", 0x00001, 0x10000, CRC(09e5c1b9) SHA1(d42be83b4181d3733268c29df04a4d2918370f4e))
ROM_END

// Apricot XEN-i 386 (Leopard Motherboard)
ROM_START( apxeni )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: Phoenix 80386 ROM BIOS PLUS Version 1.10.01 / XEN-i 386 Business Microcomputer / VR 1.2.1 22nd July 1988
	ROM_SYSTEM_BIOS(0, "lep121", "Rom Bios 1.2.1")
	ROMX_LOAD( "lep121.bin", 0x18000, 0x8000, CRC(948c1927) SHA1(d06bdbd6292db73c815ad1060daf055293dfddf5), ROM_BIOS(0))
	// 1: Phoenix 80386 ROM BIOS PLUS Version 1.10.01 / XEN-i 386 Business Microcomputer / VR 1.2.1 22nd January 1991
	ROM_SYSTEM_BIOS(1, "lep121s", "SCSI-Enabling ROMs")
	ROMX_LOAD( "lep121s.bin", 0x18000, 0x8000, CRC(296118e4) SHA1(d1feaa9704e6ce3bc10c900bdd310d9494b02304), ROM_BIOS(1))
ROM_END

/***************************************************************************
  Commodore systems
***************************************************************************/

// Commodore DT386
ROM_START( dt386 )
	// BIOS-String: 40-0502-DG1112-00101111-070791-SOLUTION-0 / 386DX-33 BIOS V1.00 #391560-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "dt386vp10", "DT386 V.pre 1.0")
	ROMX_LOAD( "cbm-dt386dx-33c-bios-hi-vpre1.0-391560-01.bin", 0x10000, 0x10000, CRC(600472f4) SHA1(2513c8bdb24fe27f73c82cbca9e1a983e4a0ba10), ROM_BIOS(0))
	// BIOS-String: 40-0500-DG112-00101111-070791-SOLUTION-0 / 386DX-33 Rev.1E (091592)
	ROM_SYSTEM_BIOS(1, "dt386v1e", "DT386 V.1e")
	ROMX_LOAD( "cbm-dt386dx-33c-bios-hi-v-xxxxxx-xx.bin", 0x10000, 0x10000, CRC(dc1ca1b5) SHA1(7441cb9d5ad5ca6e6425de73295eb74d1281929f), ROM_BIOS(1))
	// BIOS-String: 40-0500-DG1112-00101111-070791-SOLUTION-0 / 386DX-33 Rev.1E (091592) / Commodore BIOS Version 1.0 391560-01
	ROM_SYSTEM_BIOS(2, "dt386v10", "DT386 V.1.0")
	ROMX_LOAD( "cbm-dt386dx-33c-bios-hi-v1.00-391560-01.bin", 0x10000, 0x10000, CRC(da1f7e6d) SHA1(b825fc015233e7eef93a3abbdfc3eeb0da096f50), ROM_BIOS(2))
	// BIOS-String: 40-0501-DG1112-00101111-070791-SOLUTION-0 / Commodore 386DX-33 BIOS Rev. 1.01 391560-02
	ROM_SYSTEM_BIOS(3, "dt386v101", "DT386 V.1.01")
	ROMX_LOAD( "cbm-dt386dx-33c-bios-hi-v1.01-391560-02.bin", 0x10000, 0x10000, CRC(b3157f57) SHA1(a1a96c8d111e3c1da8f655b4b7e1c5be4af140e9), ROM_BIOS(3))
ROM_END

// Commodore PC-60-III - complaining "time-of-day-clock stopped" - Phoenix P8242 '87 keyboard BIOS
ROM_START( pc60iii ) // onboard Paradise PVGA1A-JK, 2xRS232, 1xparallel, keyboard
	ROM_REGION32_LE(0x20000, "bios", 0) // Chipset: Chips P82C301C, P82B305, P82A303, P82A304, Chips F82307, 25531/390423-1 COMBO SMC B9016, Commodore CSG, four MOSEL MS82C308-35JC, ISA16: 7, RAM card: 2 (up to 8MB, extended ISA connector)
	// 0: Commodore PC60-III 80386 BIOS Rev. 1.2 - 390473-01/390474-01
	ROM_SYSTEM_BIOS(0, "pc60iiiv12", "PC60-III V1.2")
	ROMX_LOAD( "cbm-pc60c-bios-lo_u73-v1.2-390473-01.bin", 0x00000, 0x10000, CRC(ff2cd8b3) SHA1(62e95f818c5016f4be2741872dc644999dee33ce),ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "cbm-pc60c-bios-hi_u67-v1.2-390474-01.bin", 0x00001, 0x10000, CRC(690fff4b) SHA1(adc262d40da64354c7c76b61f46d2c7ed35e9df9),ROM_SKIP(1) | ROM_BIOS(0) )
	// 1: Commodore PC-60-III 80386/25MHz BIOS Rev. 1.3 390473-02/390474-02
	ROM_SYSTEM_BIOS(1, "pc60iiiv13", "PC60-III V1.3")
	ROMX_LOAD( "cbm-pc60c-bios-lo-v1.30-390473-02.bin", 0x00000, 0x10000, CRC(3edd83e0) SHA1(3ebf393d6c33d9b8600f56c7be9eedb5aefb2645),ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "cbm-pc60c-bios-hi-v1.30-390474-02.bin", 0x00001, 0x10000, CRC(12209ac4) SHA1(76f271944894c77dde735da2b2ba065e81a99564),ROM_SKIP(1) | ROM_BIOS(1) )
	// 2: Commodore PC60-III 80386/25MHz BIOS rev.1.33 390473-04/390474-04
	ROM_SYSTEM_BIOS(2, "pc60iiiv133", "PC60-III V1.33")
	ROMX_LOAD( "cbm-pc60-bios-lo-v1.33-390473-04.bin", 0x00000, 0x10000, CRC(afd0aae0) SHA1(7fa4388c939f30e603f0fc90f9512e500b282432),ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "cbm-pc60-bios-hi-v1.33-390474-04.bin", 0x00001, 0x10000, CRC(7b7958db) SHA1(d542c63ec0d17e1e87403ac01735e75ce58302a9),ROM_SKIP(1) | ROM_BIOS(2) )
	// 3: Commodore PC60-III 80386-25MHz BIOS Rev.1.3.5 - 390473-06/390474-06
	ROM_SYSTEM_BIOS(3, "pc60iiiv135", "PC60-III V1.3.5")
	ROMX_LOAD( "cbm-pc60c-bios-lo-v1.35-390473-06.bin", 0x00000, 0x10000, CRC(6ff4aea9) SHA1(3fcb3a5c275dbfb93c3e55224d731f1b52343d4b),ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "cbm-pc60c-bios-hi-v1.35-390474-06.bin", 0x00001, 0x10000, CRC(5a04e3f0) SHA1(311a3ff3e578ecbce0ecd9f3b006ab772623255a),ROM_SKIP(1) | ROM_BIOS(3) )
	// 4: Commodore 80386 BIOS Rev.1.36 - 390473-07/390474-07
	ROM_SYSTEM_BIOS(4, "c386v136", "Commodore 386 V1.3.6")
	ROMX_LOAD( "cbm-pc60c-bios-lo-v1.36-390473-07-9b0e.bin", 0x00000, 0x10000, CRC(be7504f8) SHA1(a45f7690a41d416bc10ca6f583b8fdd2219a3d8a),ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "cbm-pc60c-bios-hi-v1.36-390474-07-ddf2.bin", 0x00001, 0x10000, CRC(d8e08ffa) SHA1(fb5fb973b01df6e486d76076d3373583758b1d01),ROM_SKIP(1) | ROM_BIOS(4) )
	// 5: Commodore 80386 BIOS Rev.1.36.03 - 390473-07/390474-07
	ROM_SYSTEM_BIOS(5, "c386v13603", "Commodore 386 V1.3.603")
	ROMX_LOAD( "cbm-pc60c-bios-lo-v1.3603-390473-07.bin", 0x00000, 0x10000, CRC(2cda07c7) SHA1(01fd6260192541dd73f88d2cc0f99fe5603efc81),ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "cbm-pc60c-bios-hi-v1.3603-390474-07.bin", 0x00001, 0x10000, CRC(39845b9b) SHA1(9d3cbfde4b2acc1d576aafa80126b75a49d3d8df),ROM_SKIP(1) | ROM_BIOS(5) )
ROM_END

// Commodore Tower 386
ROM_START( comt386 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// Phoenix 80386 ROM BIOS PLUS Version 1.10 22 - Twinhead International Corporation
	ROM_LOAD16_BYTE( "cbm-t386-bios-lo-v1.1022c-.bin", 0x10000, 0x8000, CRC(6857777e) SHA1(e80dbffd3523c9a1b027f57138c55768fc8328a6))
	ROM_LOAD16_BYTE( "cbm-t386-bios-hi-v1.1022c-.bin", 0x10001, 0x8000, CRC(6a321a7e) SHA1(c350fb273522f742c6008deda00ed13947a269b7))
ROM_END


/***************************************************************************
  80386 DX BIOS
***************************************************************************/

ROM_START( at386 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: X0-0100-000000-00101111-060692-386SX-0 / AMIBIOS Ver 5.19a
	ROM_SYSTEM_BIOS(0, "ami386", "AMI 386")
	ROMX_LOAD( "ami386.bin",  0x10000, 0x10000, CRC(3a807d7f) SHA1(8289ba36a3dfc3324333b1a834bc6b0402b546f0), ROM_BIOS(0))
	// 1: Phoenix 80386 ROM BIOS PLUS Verson 1.10 (R22)
	ROM_SYSTEM_BIOS(1, "at386", "unknown 386")  // This dump possibly comes from a MITAC INC 386 board, given that the original driver had it as manufacturer
	ROMX_LOAD( "at386.bin",  0x10000, 0x10000, CRC(3df9732a) SHA1(def71567dee373dc67063f204ef44ffab9453ead), ROM_BIOS(1))
	// 2: BIOS-String: 30-0101-429999-00101111-050591-D90-0 / AMI TD60C BIOS VERSION 2.42B
	ROM_SYSTEM_BIOS(2, "amicg", "AMI CG")
	ROMX_LOAD( "amicg.1",        0x10000, 0x10000,CRC(8408965a) SHA1(9893d3ac851e01b06a68a67d3721df36ca2c96f5), ROM_BIOS(2))
	// 3:
	ROM_SYSTEM_BIOS(3, "msi386", "MSI 386") // MSI 386 mainboard, initializes graphics card, then hangs - Chipset: Chips P82A304, P82A303, P82A302C, 2xP82B305, P82C301C, P82A306A,
	ROMX_LOAD( "ami_386_msi_02297_even.bin", 0x10000, 0x8000, CRC(768590a0) SHA1(90c5203d78591a093fd4f54ceb8d9827f1e64f39), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "ami_386_msi_02297_odd.bin", 0x10001, 0x8000, CRC(7b1360dc) SHA1(552ccda9f90826621e88d9abdc47306b9c2b2b15), ROM_SKIP(1) | ROM_BIOS(3) )
	// 4
	ROM_SYSTEM_BIOS(4, "ami2939", "AMI2939") // no display
	ROMX_LOAD( "ami2939e.rom", 0x10000, 0x8000, CRC(65cbbd32) SHA1(d7d26b496f8e86f01722ad9f171a68f9fcdc477c), ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "ami2939o.rom", 0x10001, 0x8000, CRC(8db6e739) SHA1(cdd47709d6036fad4be40c15bff41752d831d4b8), ROM_SKIP(1) | ROM_BIOS(4) )
	// 5: NCR 386 slot CPU - Upgrade card for e.g. NCR PC-8 - set graphics card to CGA to see a "Timer One Error" message
	ROM_SYSTEM_BIOS(5, "ncr386", "NCR 386 CPU card") // Chipset: SN76LS612PN, 2xAM9517A-5JC, NCR 006-3500402PT M472018 8650A
	ROMX_LOAD( "ncr_386_card_04152_u44_ver5.0.bin", 0x10000, 0x10000, CRC(80e44318) SHA1(54e1d4d646a577c53c65b2292b383ed6d91b65b2), ROM_BIOS(5))
	// ROM_LOAD ("ncr_386_card_keyboard_04181_u27_ver5.6.bin", 0x0000, 0x800, CRC(6c9004e7) SHA1(0fe77f47ff77333d1ff9bfcf8d6d92193ab1f208))
	// 6: BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 22
	ROM_SYSTEM_BIOS(6, "cbm386", "Commodore 386")
	ROMX_LOAD( "cbm-386-bios-lo-v1.022e-8100.bin", 0x10000, 0x8000, CRC(a054a1b8) SHA1(d952b02cc10534325c1c5aaa8b6dfb77bc20a179), ROM_SKIP(1) | ROM_BIOS(6))
	ROMX_LOAD( "cbm-386-bios-hi-v1.022e-d100.bin", 0x10001, 0x8000, CRC(b9541f3d) SHA1(e37c704521e85b07369d21b0521f4d1871c318dd), ROM_SKIP(1) | ROM_BIOS(6))
	// 7: BIOS-String: X0-0100-000000-00101111-060692-RC2018A-0 / Ver 1.4b / Texas Instruments 486 DLC [S3Q]
	ROM_SYSTEM_BIOS(7, "ti486dlc", "TI 486DLC") // board is equipped with a TI486DLC
	ROMX_LOAD( "ti_486dlc_rev.s3q.bin", 0x10000, 0x10000, CRC(39b150ed) SHA1(5fc96c6232dd3a066349d8e707e938af55893297), ROM_BIOS(7))
	// 8: BIOS-String: 305-3.2 000-00 - Chipset: TACT82206FN; Intel A82385-33 - Keyboard Controller: P/N: 191106-2 C/S E4F4 Rev. 1.4
	// Board with Tandon and Micronics stickers - BIOS: 192475-305A V305 3.2
	// ISA8: 2, ISA16: 5 - OSC: 14.31818 MHz - 66.0000 MHz, CPU: Intel 80386DX-33, FPU: Intel 80387DX-33
	ROM_SYSTEM_BIOS(8, "tanmic385", "Tandon/Micronics with 385")
	ROMX_LOAD( "386-micronics-09-00021-even_32k.bin", 0x10000, 0x8000, CRC(0d4f0093) SHA1(f66364a82c957862a0e54afc3a2f85f911adfd49), ROM_SKIP(1) | ROM_BIOS(8))
	ROMX_LOAD( "386-micronics-09-00021-odd_32k.bin", 0x10001, 0x8000, CRC(54195986) SHA1(f3536340ef1697763e5cd70d0de7bb9b2a4ecde9), ROM_SKIP(1) | ROM_BIOS(8))
	// 9: Biostar MB1325PM - Chipset: Chips P82C206 µIC MI9382 MI9381A
	// BIOS: AMI 386 BIOS - BIOS-String: 30-0101-D61223-00101111-050591-OPBC-F / MB-1325PM. - Keyboard-BIOS: AMI
	// CPU: AMD 386DX/DXL-25 - ISA8: 1, ISA16: 6, ISA8/Memory: 1
	ROM_SYSTEM_BIOS(9, "mb1325pm", "MB1325PM")
	ROMX_LOAD( "386-mb1325pm ok.bin", 0x10000, 0x10000, CRC(768689c1) SHA1(ce46b3baf3cd2586ffaccdded789a54583b73a3b), ROM_BIOS(9))
	// 10: Intel SSBC 386AT - Chipset: Intel P8237A, P8254; P8259A, SN74LS612N - BIOS: 380892-01 Rev. 1.04 U53 L - 380892-02 Rev. 1.04 U52 H
	// Keyboard BIOS: Intel 453775-001 - ISA8: 2, ISA16: 4, Memory expansion: 2 - OSC: 1.8432 MHz - 32.000 - 14.31818
	ROM_SYSTEM_BIOS(10, "ssbc386at", "Intel SSBC 386AT" ) // no display
	ROMX_LOAD( "386-intel-u53-l_32k.bin", 0x10001, 0x8000, CRC(5198a767) SHA1(03dd494e3a218c59c82ebd7b1dd16905bca30773), ROM_SKIP(1) | ROM_BIOS(10))
	ROMX_LOAD( "386-intel-u52-h_32k.bin", 0x10000, 0x8000, CRC(cedbad7a) SHA1(e1365f5a183a342fe58205679a512c4ccd2a705a), ROM_SKIP(1) | ROM_BIOS(10))
	// 11: BEK P405 clone - BIOS-String: 30-0201-428029-00101111-070791-OPWB-0 -  was found as a stray ROM - possibly from a 486 board
	ROM_SYSTEM_BIOS(11, "opwb", "OPWB")
	ROMX_LOAD( "opwb.bin",  0x10000, 0x10000, CRC(e7597fb6) SHA1(2f1eb88138b400cc3ad554d03e532b5d3b0b11ad), ROM_BIOS(11))
	// 12: unknown manufacturer, Logo could be a "J7" -  COPYRIGHT (C) 89 REV.C MADE IN TAIWAN - Chipset: Chips P82C206
	// BIOS: AMI 386 BIOS PLUS 896818 - BIOS-String: DINT-6102-091589-K0 - Keyboard-BIOS: AMI KEYBOARD BIOS PLUS 896819
	// CPU: Intel 386DX-25 FPU: i386DX-33 - OSC: 54.0000MHz, 16.000MHz, 14.31818MHz
	ROM_SYSTEM_BIOS(12, "386atj7", "386AT J7" )
	ROMX_LOAD( "386-big_ami_896818_even.bin", 0x10000, 0x8000, CRC(096e99c4) SHA1(29ff718362af4f5d7c0173f4de84290cec60dded), ROM_SKIP(1) | ROM_BIOS(12))
	ROMX_LOAD( "386-big_ami_896818_odd.bin", 0x10001, 0x8000, CRC(6f92634d) SHA1(e36d401975690043c5d5cb1f781036b319e57f37), ROM_SKIP(1) | ROM_BIOS(12))
	// 13: BIOS-String: DAMI-0000-040990-K0 - silkscreen on board: 17:35-2495-02 - 702430D - H8010-30 - ISA8:2, ISA16: 8 - OSC: 14.31818, 50.000MHz
	// Chipset:Laser 27-2024-00 4L40F1028, Laser 27-2025-00 4L40F1026, 2xKS82C37A, KS92C59A, KS82C54-10P - CPU: Intel 386DX-33, FPU socket provided
	ROM_SYSTEM_BIOS(13, "vt386vt", "VT386VT" )
	ROMX_LOAD( "vt386vt-702430d-rom0_32k.bin", 0x10000, 0x8000, CRC(00013ee6) SHA1(7fed0b176911a94e8127b01bb77445c78f283ff7), ROM_SKIP(1) | ROM_BIOS(13))
	ROMX_LOAD( "vt386vt-702430d-rom1_32k.bin", 0x10001, 0x8000, CRC(c817ec57) SHA1(acdd0e28cb4798059c02e1342da7efe3eaf2c5cb), ROM_SKIP(1) | ROM_BIOS(13))
	// 14: Micronics I-Cache 09-00021-L8949 - Chipset: Chips P82C206 - RAM: M500/385 Memory Board, Cache: 8xCXK5863P-25
	// BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 10a - Keyboard-BIOS: Intel - CPU/FPU: sockets provided, empty
	// ISA8: 2, ISA16: 4, Memory: 1 - OSC: 66.0000MHz, unreadable
	ROM_SYSTEM_BIOS(14, "l8949", "L8949" )
	ROMX_LOAD( "386-micronics 09-00021-lo_32k.bin", 0x10000, 0x8000, CRC(3a8743e3) SHA1(42262f60cb655ab120d968dbf9eb03387424bf14), ROM_SKIP(1) | ROM_BIOS(14))
	ROMX_LOAD( "386-micronics 09-00021-hi_32k.bin", 0x10001, 0x8000, CRC(c7fce430) SHA1(e0d6e8dbb8b6d68bd92dab63a259d2c9293f5571), ROM_SKIP(1) | ROM_BIOS(14))
	// 15: BIOS-String: 30-0200-ZZ1453-00101111-050591-SCAT3.04-0
	ROM_SYSTEM_BIOS( 15, "kmxc02", "KMX-C-02" )
	ROMX_LOAD( "3ctm005.bin", 0x10000, 0x10000, CRC(cfba6b2a) SHA1(001642016a3c02b031d739bd8b0fcff9470e86d2), ROM_BIOS(15))
	// 16: BIOS: AMI; 11/11/92 - ISA16: 5 - CPU/FPU: Am386DX-40, IIT 3C87-40 - Chipset: FOREX FRX46C521, KS83C206Q
	// BIOS-String: 40-0G00-009999-00101111-111192-4X521-F
	ROM_SYSTEM_BIOS( 16, "frx521", "using the Forex FRX46C521" ) // no display
	ROMX_LOAD( "3fom001.bin", 0x10000, 0x10000, CRC(8fa851c8) SHA1(68ac21357558d98aee4e2ffb903791e4198e0dd0), ROM_BIOS(16))
	// 17: FOREX 386 Super DX System  S3B
	ROM_SYSTEM_BIOS( 17, "frxs3b", "Forex Super DX System S3B") // no display
	ROMX_LOAD( "3fom003.bin", 0x10000, 0x10000, CRC(4e164e0a) SHA1(dc2d08061c443a3e4ced3ab11f1fa094585cbbba), ROM_BIOS(17))
	// 18: BIOS: AMI; 06/06/92 - BIOS-String: 40-0101-001107-00001111-060692-OPWB4SXB-0 / OPTI-495SX (471WB) BIOS VER 1.0
	// cf. driver hot409 - BIOS is capable of detecting 386sx => 486DX2, this particular BIOS was sorted with the 386s on chukaev
	ROM_SYSTEM_BIOS( 18, "495sx", "OPTi 82C495SX")
	ROMX_LOAD( "3opm009.bin", 0x10000, 0x10000, CRC(2abe36eb) SHA1(d113527ebd06f0359f2decd4ac0c6202f982d45e), ROM_BIOS(18))
	// 19: BIOS-String: EEMI-0386-030891-K0 - Chipset: 88C311
	ROM_SYSTEM_BIOS( 19, "eemi", "EEMI")
	ROMX_LOAD( "3zzm002", 0x10000, 0x10000, CRC(c2a7ff22) SHA1(af2e321d3245ad839a41666917bb24cca0f7884d), ROM_BIOS(19))
	// 20: BIOS-String: 30-0300-ZZ1425-00101111-020291-ITOPDX / 23L-1-0000-00-00-0000-00-00-000-K0
	// 000-0-0000-00-00-0000-00-00-00-2
	ROM_SYSTEM_BIOS( 20, "topcat", "TOPCAT")
	ROMX_LOAD( "ami_386_vl82c330_even.bin", 0x10000, 0x8000, CRC(a6f3d881) SHA1(40672d58f79d232dbda9685b9aa20533029fbdfc), ROM_SKIP(1) | ROM_BIOS(20))
	ROM_CONTINUE( 0x10001, 0x8000 )
	// 21: BIOS-String: MR BIOS (tm) V1.35 - RAM Pattern Test Failed at 0F0000H
	ROM_SYSTEM_BIOS( 21, "mrv135", "MR BIOS V1.35")
	ROMX_LOAD( "mrbios_v1.35_opti324_4floppy.bin", 0x10000, 0x10000, CRC(9a21dcd3) SHA1(dcab673fd2df621839671ef8f6a2eff443de39df), ROM_BIOS(21))
	// 22: BIOS-String: SINT-1185-040990-K0 - Chipset: VIA SL9030, SL9010, SL9025, SL9020, SL9020, SL9350, SL9090A
	ROM_SYSTEM_BIOS( 22, "3vim002", "3VIM002")
	ROMX_LOAD( "3vim002l.bin", 0x10000, 0x8000, CRC(368b66df) SHA1(1bef1e8e1818513061f0c7cf3c731da360c8400b), ROM_SKIP(1) | ROM_BIOS(22))
	ROMX_LOAD( "3vim002h.bin", 0x10001, 0x8000, CRC(02dbb9fe) SHA1(cfce750a4a019c71e59011fb7a7d891b40f61c61), ROM_SKIP(1) | ROM_BIOS(22))
	// 23: BIOS-String: 30-0201-ZZ1343-00101111-050591-OPWB-0
	ROM_SYSTEM_BIOS( 23, "zz1343", "zz1343")
	ROMX_LOAD( "ami_zz1343.bin", 0x10000, 0x10000, CRC(f5464c1f) SHA1(cb069c3d1d322aa769d46749716a35259f78264a), ROM_BIOS(23))
	// 24: ID: AT046DX3-B2.1(PQFP) - CPU: AMD Am386DX/DXL-33, FPU socket provided - Keyboard BIOS: JETkey V3 - Chipset: ACC Micro 2168 F9217D103 SC00 333MHz
	// RAM: 8xSIMM30, Cache: 64KB - OSC: 14.31818, 66.660MHz - ISA8: 1, ISA16: 5
	// BIOS-String: 30-004-428002-00101111-070791-ACC2046-0
	ROM_SYSTEM_BIOS( 24, "acc386", "AT046DX3-B2.1")
	ROMX_LOAD( "acc386.bin", 0x10000, 0x10000, CRC(2177c9ac) SHA1(2e320dcd173137b9b0cbf92602e9b77398921aaf), ROM_BIOS(24))
ROM_END


/**************************************************************************
  80386 DX motherboard
**************************************************************************/

// PC-CHIPS M317 - Chipset: PC Chip CHIP 5 (14L40F2054) / CHIP 6 (14L50F2053), UMC 82C206F - CPU: 80386, FPU socket privided
// BIOS: AMI 386 BIOS PLUS - Keyboard BIOS: AMI Keyboard BIOS Plus - RAM: 8xSIMM30 - Cache: 64K - solder pads for a memory card connector
// ISA8: 2, ISA16: 6 - OSC: 66.6670MHz, 14.318MHz
ROM_START( pccm317 ) // 40-0300-ZZ1347-00101111-031591-UMCWB-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386_pcchips.bin", 0x10000, 0x10000, CRC(8a6046ea) SHA1(1e6c5dacdb5f9a36e558e44466bcd9182d849932))
ROM_END

// SOYO 386DX Baby-AT - Chipset: ETEQ ET82C4901 C, ET82C4903 A, Chips P82C206 H1 - CPU: Am386DX/DXL-40, FPU: Cyrix 387DX+
// RAM: 8xSIMM30, Cache: 8x5C6408-12, 1x ATT7C185P-15 - BIOS: AMI 386 BIOS 336468 - BIOS-String: 41-0100-001102-00101111-121291-BENGAUTO-F - REV: C *** 40MHZ ***
// Keyboard BIOS: AMI KB BIOS-VER-F - ISA8: 1, ISA16: 6 - OSC: 8.000MHz, 80.000MHz
ROM_START( sybaby386 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386dx.bin", 0x10000, 0x10000, CRC(9fcfd21f) SHA1(5e7b42be55a45b0b51d2a7e8bd8d6405dbe69129))
ROM_END

// AMI 386 BABY SCREAMER - BIOS: AMI MARK V BABY SCREAMER - Chipset: VLSI VL82C331-FC, VL82C332-FC, Megatrends MG-9275, Chips ??? - OSC: 14.31818, 66.666, 24.000MHz
// BIOS-String: 40-0301-000000-00101111-070791-SCREAMER-0 / BIOS RELEASE 42121691 - On board: 2xserial, parallel, floppy, 1xIDE
ROM_START( amibaby )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROMX_LOAD( "ami_mark_v_baby_screamer_even.bin", 0x00000, 0x10000, CRC(50baacb7) SHA1(c9cb6bc3ab23f35050a7f079109005331eb5de2c), ROM_SKIP(1))
	ROMX_LOAD( "ami_mark_v_baby_screamer_odd.bin", 0x00001, 0x10000, CRC(42050eed) SHA1(c5e1ed9717acb2e3adcb388ccecf90a74d495132), ROM_SKIP(1))
ROM_END

// AUVA TAM/25-P2 M31720P - Chipset: µC M19382, M19381A, Chips - CPU: 386DX 25Mhz - BIOS: DA058290 - Keyboard-BIOS: A179859
// BIOS-String: 30-0101-D81105-00101111-050591-OPBC-0 / AUVA 386 TAM/25/P2(P2,A1,A2), 01/10/1992- ISA8: 1, ISA16: 6, ISA16/Memory: 1
ROM_START( tam25p2 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "tam25-p2.bin", 0x10000, 0x10000, CRC(0ea69975) SHA1(cb7f071a36653cf4f00a8b158a4900efb8f8b8e8))
ROM_END

// ID: Morse KP920121523 V2.20 - Chipset: UMC UM82C206L MORSE 91A311 91A310 - CPU: AM386DX/DXL-40, FPU socket provided - RAM: 8xSIMM30, Cache: 32K/64K/128K
// BIOS: AMI 386DX BIOS Ver. 2.11 C-1216 - Keyboard BIOS: KB-BIOS-VER F MEGATRENDS - OSC: 14.31818MHz, 80.000MHz - ISA16: 8, solder pads for a memory extension card
ROM_START( mokp386 ) // BIOS-String: 30-0200-ZZ1216-00101111-050591-KP386DX-0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-morse-kp920121523.bin", 0x10000, 0x10000, CRC(9392e265) SHA1(1df163bef6cf73a1d2a40ac997b96f93d1f2f4d1))
ROM_END

// UNICHIP 386W 367C REV 1.0 - Chipset: UNIchip U4800-VLX/9351EAI/4L04F1914, HMC HM82C206 - CPU: AM386DX-40, FPU socket provided - ISA8: 1, ISA16: 5 - OSC: 14.31818
ROM_START( uni386w )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS: AMI 386C BIOS 2116295 -
	// BIOS-String: 40-0400-001107-00101111-111192-U4800VLX-0 / UNICHIP BIOS VER 2.0A 09/27/1993 - Keyboard-BIOS: AMI 386C BIOS KEYBOARD 2116295 -
	ROM_SYSTEM_BIOS(0, "ver20a", "Ver. 2.0A")
	ROMX_LOAD( "386-2116295.bin", 0x10000, 0x10000, CRC(7922a8f9) SHA1(785008e10edfd393dc39e921a12d1a07a14bac25), ROM_BIOS(0))
	// 1: AMI, 367 UNICHIP 386 BIOS VER 1.0 (1886636) / BIOS-String: 40-0300-001107-00101111-111192-U4800VLX-0
	ROM_SYSTEM_BIOS(1, "ver10", "Ver. 1.0")
	ROMX_LOAD( "3ucm002.bin", 0x10000, 0x10000, CRC(9f2e19da) SHA1(ef64c6ad9d02db849d29e3b998ca42b663656bad), ROM_BIOS(1))
ROM_END

// Alaris Cougar - Chipset: OPTi 82C499 - ISA16: 5, ISA16/VL: 2
// BIOS: MR BIOS (r) V1.65 - CPU: 75MHz IBM Blue Lightning
ROM_START( alacou )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "cougrmrb.bin", 0x10000, 0x10000, CRC(c018f1ff) SHA1(92c4689e31b367baf42b12cad8800a851cc3e828))
ROM_END

// DTK PEM 2530 - Chipset: VLSI 9032BT/217203/VL82C100-0C
// Board's original ROMs were damaged (Datatech dtk 386 V4.26 A1763), "original" ROMs came from another user, V3.10 ROMs from a different board
// ISA8: 2, ISA16: 5, Memory connector: 1 - OSC: 40.000 MHz - 14.31818 MHz
ROM_START( pem2530 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// Phoenix 80386 ROM BIOS PLUS Version 1.10 01 KENITEC TECHNOLOGIES
	ROM_SYSTEM_BIOS(0, "pem2530ori", "DTK PEM 2530 original")
	ROMX_LOAD( "386-dtk_pem-2530_bios-low.bin", 0x10000, 0x8000, CRC(d9aad218) SHA1(a7feaad2889820852e3543229b0b103288470732), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "386-dtk_pem-2530_bios-high.bin", 0x10001, 0x8000, CRC(550c4d77) SHA1(05aba1a98e738f9b706b5a8f09b5b6c86bd336e2), ROM_SKIP(1) | ROM_BIOS(0))
	// 80386 BIOS Version 3.10 Rev. 2.06 (BIOS not original, works in PEM 2530)
	// additional info from chukaev.ru54.com: CPUBT-S26361-D548 (Siemens?) - Chipset: VL82C320A, VL82C331, VL16C452B - CPU: NG80386SX-20, socket for 80387SX
	ROM_SYSTEM_BIOS(1, "pem2530", "DTK PEM 2530")
	ROMX_LOAD( "386-dtk pem-2530-high_32k.bin", 0x10000, 0x8000, CRC(56a822c0) SHA1(b65797c0f87a0815b393758af9c059e6d7172ae9), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "386-dtk pem-2530-low_32k.bin", 0x10001, 0x8000, CRC(8688d883) SHA1(c3034c8b343786cb89de48fb2f4992160414f89e), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

// SM 386-40F - MR BIOS (r) V1.40 - Ver: V1.40-FORX300
// Chipset: SIS 85C206, FOREX FRX36C200, FOREX FRX36C300
ROM_START( sm38640f )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "forex300.bin", 0x10000, 0x10000, CRC(8d6c20e6) SHA1(cd4944847d112a8d46612e28b97e7366aaee1eea))
ROM_END

// Soyo SY-019H and SY-019I BIOS-String: 30-0200-DH1102-00101111-070791-ETEQ386-0 / REV C3
// Chipset: SIS 85C206, ETEQ ET82C493, ET82C491
ROM_START ( sy019hi )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ami_soyo_sy-19i.bin", 0x10000, 0x10000, CRC(369a040f) SHA1(3dbcbcb8b8a50717cae3b17f44ca1b7c394b75fc))
ROM_END

// PC-Chips M321 - RAM: 8xSIMM30, Cache: 8 sockets, 4 sockets occupied by CY71C199-25PC - CPU: AM386-DX40, FPU: socket provided - OSC: 80.000MHz, 14.31818
// Chipset: PCChips C206/306, CHIP6/4L04F1666, CHIP5/4L04F1282 (rev. 2.3, 2.5 and 2.7 boards) - ISA8: 2, ISA16: 6
ROM_START( pccm321 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// BIOS-String: 30-0201-ZZ1347-00101111-050591-M320-0
	ROM_SYSTEM_BIOS(0, "m321_23", "PCChips M321 Rev.2.3") // also on a rev. 2.5 board with C&T J38600DX-33, ULSI MathCo-DX33
	ROMX_LOAD( "pcchips_m321_rev2.3.bin", 0x10000, 0x10000, CRC(ca0542e4) SHA1(8af9f88e022f8115708178c6c0b313ea0423a2b5), ROM_BIOS(0) )
	// BIOS-String: 30-0100-001437-00101111-060692-PC CHIP-0
	ROM_SYSTEM_BIOS(1, "m321_27_1", "PCChips M321 Rev.2.7 #1")
	ROMX_LOAD( "3pcm002.bin", 0x10000, 0x10000, CRC(0525220a) SHA1(5565daea1db67fb2e6f5e7f5ddf5333569e74ab3), ROM_BIOS(1) )
	// BIOS-String: 30-0100-001437-00101111-060692-PC CHIP-0 - TRANS-AMERITECH ENTERPRISES, Inc.
	ROM_SYSTEM_BIOS(2, "m321_27_2", "PCChips M321 Rev.2.7 #2")
	ROMX_LOAD( "3pcm004.bin", 0x10000, 0x10000, CRC(d7957833) SHA1(b512d9fc404c4282fb964444aa70a9760edad7db), ROM_BIOS(2) )
	// BIOS-String: 30-0100-ZZ1437-00101111-070791-PC CHIP-0
	ROM_SYSTEM_BIOS(3, "m321_27_3", "PCChips M321 Rev.2.7 #3")
	ROMX_LOAD( "pcchips_m321_rev2.7_2.bin", 0x10000, 0x10000, CRC(1c529364) SHA1(a0cb0dc31b34377024efb3124f4167a8e1d748e6), ROM_BIOS(3) )
ROM_END

// Morse M3 V3.00 - Chipset: Morse 4L04F1282 + 4L04F1666 + 92A206S / possibly a rebranded OPTi 82C391 chipset - CPU: Am386DX/DXL-33,
// CPU (80386DX) and FPU socket (W3167) provided - RAM: 8xSIMM30, Cache: 4xMOSEL MS62256-A-25NC, 1xT6C6408-20 - BIOS: AMI 386DX BIOS PLUS Ver.3.00 386DX C-1216
// BIOS-string: 30-0201-ZZ1216-00101111-050591-386DX-0 - Keyboard-BIOS: AMI KEYBOARD BIOS PLUS C-1216 - OSC: 66.667MHz, 14.31818MHz - ISA8: 1, ISA16: 6
ROM_START( mom3v3 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "morse_m3_v3.00.bin", 0x10000, 0x10000, CRC(4748a084) SHA1(169696ea08f25e05928c48cc8328c67c51789f0d))
ROM_END

// PC-Chips M326
// Chipset: SARC RC4018A4/9324 and SARC RC6206A4/9408-AHS or SARC RC4018A4/9324 and RC4919A4-9323 (v5.5 board) or SARC RC2016A4-9320 and RC4019A4-9324 (v5.3)
ROM_START( pccm326  )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: Award Modular BIOS 4.50
	ROM_SYSTEM_BIOS(0, "pccm326", "PCChips M326 V5.2") //  BIOS reports a 66MHz 386DX original board has a TI TX486DLC/E-40PCE and IIT 4C87DLC-40 CPU/FPU combo
	ROMX_LOAD( "m326_v5.2_m601-326.bin", 0x10000, 0x10000, CRC(cca6a443) SHA1(096c8bfa000c682d6c801da27c7fd14243ebb63b), ROM_BIOS(0) )
	// 1: BIOS-String: 40-0100-001437-001001111-080893-4386-0 / Release 10/01/93 - also on an "M601 Rev. 1.3" board with a i486DX-33 (BIOS AMI AB0077440 - Keyboard-BIOS: Regional HT6542)
	ROM_SYSTEM_BIOS(1, "m326r53", "PC-Chips M326 Rev. 5.3")
	ROMX_LOAD( "m326_rev.5.3.bin", 0x10000, 0x10000, CRC(6c156064) SHA1(362ce5a2333641083706a878b807ab87537ca1e6), ROM_BIOS(1) )
	// 2: BIOS: AMI; 08/08/93; Release 10/01/93
	ROM_SYSTEM_BIOS(2, "m326", "M326") // no display
	ROMX_LOAD( "3sam005.bin", 0x10000, 0x10000, CRC(f232cd4b) SHA1(e005aa3a7d160223fb2912cf2cd5cc8af49e79a5), ROM_BIOS(2) )
ROM_END

// Elitegroup UM386 Rev. 1.1 - UMC UM82C482AF, UM82C391A, UM82C206F - OSC: 80.000MHz, 14.31818MHz
// BIOS: AMI-1131 E-91844945 - Keyboard-BIOS: AMI KEYBOARD-BIOS-VER-F / Intel P8942AHP
// RAM: SIMM30x8, Cache: 9xW2464AK-20, 1x ISA8, 7xISA16 - CPU: AM386DX/DXL-40
ROM_START( ecsum386 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// BIOS-String: 30-0500-D01131-00101111-070791-UMCWB-0 / UM386 V1.1 03-06-92
	ROM_LOAD( "ami_um386_rev1.1.bin", 0x10000, 0x10000, CRC(81fe4297) SHA1(efb2ba2be6f08cb487ee1b867a2456ed6b5975ad))
ROM_END


/***** 386 motherboards using the ALi M1419 chipset *****/

ROM_START( alim1419 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: KMC-A419-8 VER 1.0 - Chipset: M5818 A1, ALi M1421 A1, M1419 A0 - OSC: 14.31818, 80.000MHz
	// BIOS-String: 40-0100-001453-00101111-121291-ALI1419-0 / 486DLC/386DX, ISA8: 1, ISA16: 6
	ROM_SYSTEM_BIOS ( 0, "kmca419", "KMC-A419-8 VER 1.0" )
	ROMX_LOAD( "3alm010.bin", 0x10000, 0x10000, CRC(733d0704) SHA1(b5724f98047e95ea41aaa396a0374357f20cf2de), ROM_BIOS(0))
	// 1: BIOS: Award Modular BIOS v4.50 - BIOS-String: 06/22/93-ACER-M1419-214k6000-00
	ROM_SYSTEM_BIOS( 1, "alim141901", "ALi M1419 #1" )
	ROMX_LOAD( "3alw001.bin", 0x10000, 0x10000, CRC(15bd5c90) SHA1(abe2a8613d2950b27701468144fe9de8063d6e57), ROM_BIOS(1))
	// 2: 386AC P102 - CPU: Am386DX-40, FPU socket provided - Chipset: ALi M1419 A1, M1421 A1 - BIOS: AMI 386DX ISA BIOS AA1226493 -
	// Keyboard-BIOS: JETkey V3 - RAM: 8xSIMM30, Cache: 9x28pin - ISA8: 8, ISA16: 7 - OSC: 14.31818, 80.000MHz
	// BIOS-String: 40-0102-001128-00101111-121291-ALI1419-0
	ROM_SYSTEM_BIOS( 2, "386acp102", "386AC P102")
	ROMX_LOAD( "386ac_p102_ami_aa1226493.bin", 0x10000, 0x10000, CRC(43ba9775) SHA1(9f80ebf1e7ef1d7e5b7c2aad5839b4f982db75d1), ROM_BIOS(2))
	//3: ID: Unknown C3404 Rev:B - Chipset: ALI M1419 A1, M5818 A1, another unreadable (silk screen: M1421) - RAM: 8xSIMM30, Cache: 8x28pin DIP, 64K or 256K
	// CPU: AMD Am386DX-40, solder pads for Intel 80386 socket provided, FPU socket provided - BIOS: AMI 386DX ISA BIOS AA1458354
	// BIOS Version: "386DX Subversion 13.19.02 3/23/1993" - BIOS String: 40-0100-001374-00101111-121291-ALI1419-F - Keyboard BIOS: JETkey V3
	// ISA8: 1, ISA16: 6 - OSC: 14.31818, 80.000 MHz
	ROM_SYSTEM_BIOS( 3, "c3404", "C3404") // displayed BIOS-String in MAME has -0 instead of -F: this denotes the keyboard controller version http://www.idhw.com/textual/guide/inst_mobo_ami.html
	ROMX_LOAD( "386c3404.bio", 0x10000, 0x10000, CRC(8005f1f6) SHA1(1c78a7a3f134fd1299b48cd12b08b013c212fa59), ROM_BIOS(3))
ROM_END


/***** 386 Motherboards using the Ali M1429 A1 and M1431 A2 chipsets ... they hang before initializing the graphics card *****/

ROM_START( alim1429 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS( 0, "386ali", "386 board with Ali chipset" )
	ROMX_LOAD( "386_ali_ami_511767.bin", 0x10000, 0x10000, CRC(3c218db4) SHA1(785ea7c36e8be5e7410524e90170d4985cbc9c24), ROM_BIOS(0))
	// 1: Seritech SER-386AD III (written on the underside of the board) - CPU: AMD Am386DX-40 - ISA16: 5
	// BIOS : AMIBIOS 04/04/1993 Ser.# 579092 - BIOS-String : 40-0212-001133-00101111-040493-ALI1429-F - Keyboard BIOS: Regional HT6542
	ROM_SYSTEM_BIOS( 1, "ser386ad", "SER-386AD III" )
	ROMX_LOAD( "ser386ad3.bin", 0x10000, 0x10000, CRC(d80d6deb) SHA1(9f889f7464255431c13ac91d7df31b325447fef5), ROM_BIOS(1))
	// 2: ISA8: 1, ISA16: 5, BIOS-String: 40-0103-001256-00101111-080893-ALI1429-H, BIOS: AMIBIOS, 08/08/93, 386DX ISA BIOS, AA2722981
	ROM_SYSTEM_BIOS( 2, "revb", "REV:B")
	ROMX_LOAD( "3alm001.bin", 0x10000, 0x10000, CRC(56ea4d9d) SHA1(0633f78a0013a62be974233a3cad6a5d3cbe90d1), ROM_BIOS(2))
	// 3: CPU: 386DX-40 -
	ROM_SYSTEM_BIOS( 3, "alim142901", "ALi M1429 #1" )
	ROMX_LOAD( "3alm007.bin", 0x10000, 0x10000, CRC(b72d754a) SHA1(364a976eac61bc923b76ccddd13f80e0727e5df5), ROM_BIOS(3))
	// 4: REV:8 - CPU: 386DX-40
	ROM_SYSTEM_BIOS( 4, "alim142902", "ALi M1429 #2" )
	ROMX_LOAD( "3alm008.bin", 0x10000, 0x10000, CRC(4cb1052d) SHA1(995a590beb0654c5e784f10019c10bd4b0278d9b), ROM_BIOS(4))
	// 5: ??? REV 2.3, Chipset: Asaki 3A029, 3A031 (= ALi 1429) - BIOS: AMI 386 BIOS S/N: 254468
	// BIOS-String: 40-0215-001926-00101111-040493-ALI1429 - ISA8: 1, ISA16: 5
	ROM_SYSTEM_BIOS( 5, "asaki", "386 with Asaki chipset" )
	ROMX_LOAD( "3asm001.bin", 0x10000, 0x10000, CRC(57c72c4d) SHA1(934223fcd39533bca2e7e57406b1800d9e900ef0), ROM_BIOS(5))
ROM_END

// Daewoo AL486V-D Rev:1.1 - BIOS/Version: AMI v299 08/08/93, BIOS-String: 40-0100-001131-00101111-080893-ALI1429 - Keyboard-BIOS: MEGAKEY
// BIOS: AMI v1.9 299 WinPro-d S/No. E-94237376 - OSC: 14.31818
ROM_START( al486vd ) // this is a 386 class board despite the name
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "al486v-d_v299.bin", 0x10000, 0x10000, CRC(75c75d58) SHA1(50e314cdefe39e8e6f74b9b045a15cc53b3f16ba))
ROM_END


/***** 386 motherboards using the Chips & Technologies P82C351, P82C355, P82C356 chipset *****/

// ABIT FU340 - 6x 16-bit ISA + 2x 8-bit ISA - RAM: SIMM30x8, Cache: 32/64/128/256KB with TAG (TULARC info)
// BIOS-String: 30-0200-D01247-00101111-050591-PEAKDM_B-0 / FU340 REV-B PAGE MODE BIOS
ROM_START( fu340 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ami_abit_fu340.bin", 0x10000, 0x10000, CRC(9ea90d90) SHA1(091bdae7b1e36ac5168823d80d5907af2a95e583))
ROM_END

// GES 9051N-386C VER -0.01 - CPU/FPU: i386DX-33, i387DX 16-33 - Chipset: Chips F82C351, F82C355, F82C356 - BIOS: AMI 386DX ISA BIOS (AA0365368)
// BIOS-String: 30-1113-002101-00001111-050591-PEAKDM_B-0 / GES 9051N BIOS VERSION 2.0 - ISA8: 3, ISA16: 5
ROM_START( ges9051n )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "3ctm001.bin", 0x10000, 0x10000, CRC(7f03f606) SHA1(d03d5b6541bc7f41d78159f82aa8057229516c37))
ROM_END


/***** 386 Motherboards using the Chips & Technologies CS8230 chip set: P82C301C, P82C302C, P82A303, P82A304, 2x P82B305, P82A306 A, P82C206 *****/

// ECS (Elitegroup) 386A - Chipset: CHIPS P82C301/2/5 + P82A303/4/6 + P82C206 - BIOS/Version: AMI 386-BIOS / 386A V1.1 05-19-90 (2 x 27C256)
// BIOS String: EC&T-1131-040990-K8 - Keyboard BIOS: AMI Keyboard BIOS PLUS - CPU: Intel 80386DX-25, FPU: 80387DX-25 - RAM: 8xSIMM30
// ISA8: 2, ISA8/RAM: 1, ISA16: 5 - OSC: 32.0000MHz, 50.0000MHz, 14.31818
ROM_START( ecs386a ) // hangs after initialising the graphics card
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROMX_LOAD( "386a012l.bin", 0x10000, 0x8000, CRC(8a067777) SHA1(aadc84867155e4e167a0380bd409dba62fd238a1), ROM_SKIP(1))
	ROMX_LOAD( "386a012h.bin", 0x10001, 0x8000, CRC(525fc3bd) SHA1(3ab8cb5989933edc6aa9f99fcab518307b60552b), ROM_SKIP(1))
ROM_END

ROM_START( cs8230 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: EC&T-1332-040990-K0
	ROM_SYSTEM_BIOS(0, "cs823001", "CS8230 #1")
	ROMX_LOAD( "ami_386_cs8230_chipset.bin", 0x10000, 0x10000, CRC(1ee766d0) SHA1(75dba3c9817dfe6caca46f5f4f2f1d76ba88d3c7), ROM_BIOS(0) )
	// 1: BIOS-String: EC&T-1197-022589-K0
	ROM_SYSTEM_BIOS(1, "cs823002", "CS8230 #2")
	ROMX_LOAD( "3ctm004l.bin", 0x10000, 0x8000, CRC(b6efc361) SHA1(88d89bf5e7c57ffe4751e14220ac82a2d0a12994), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "3ctm004h.bin", 0x10001, 0x8000, CRC(f26c2672) SHA1(1d3a2554bbf3dc554970e0d62d9c5fad24977f55), ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// ECS-386/32 - OSC: 40.000MHz, 32.000MHz, 14.318MHz - CPU: 386DX-20, FPU socket provided
// 8x SIMM, 5x 16-bit ISA, 2x 8-bit ISA, 1x 32-bit proprietary memory expansion slot
ROM_START( ecs38632 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: EC&T-1131-030389-K0
	ROM_SYSTEM_BIOS( 0, "030389", "030389")
	ROMX_LOAD( "ami_ecs-386_32_lo.bin", 0x10000, 0x8000, CRC(e119d6a4) SHA1(bcc6164173b44832b8ebfa1883e22efc167e2cd4), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "ami_ecs-386_32_hi.bin", 0x10001, 0x8000, CRC(e3072bf8) SHA1(74eec72e190f682cfd5ae5425ebdc854e0ba7bc9), ROM_SKIP(1) | ROM_BIOS(0) )
	// 1: BIOS-String: EC&T-1131-092588-K0
	ROM_SYSTEM_BIOS( 1, "092588", "092588")
	ROMX_LOAD( "ami_1131_bios_l.bin", 0x10000, 0x8000, CRC(145c3905) SHA1(9ee2615982e28082971bae8ef8a1f936313ac8c8), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "ami_1131_bios_h.bin", 0x10001, 0x8000, CRC(73d57778) SHA1(176c90134540d5054c99c077126c7c5a65199175), ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

// SY-012 16/25 386MB VER: 5.2 - Chipset: Chips P82C301C; P82A306; P82A303; P82C206; P82A304; P82C302; P82B305
// BIOS: AMI 386 BIOS 10084 - BIOS-String: DC&T-1102-082588-K0 - CPU: i386DX-33, ISA8: 2, ISA16: 5, Memory: 1
// OSC: 14.31818 - 20.000 MHz - 50.000 MHz - 32.000 MHz
ROM_START( sy012 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROMX_LOAD( "386-sy-012-l_32k.bin", 0x10000, 0x8000, CRC(6ab197f4) SHA1(7efd9033af3a0b36bc5be64cb28c6218cda4d13c), ROM_SKIP(1) )
	ROMX_LOAD( "386-sy-012-h_32k.bin", 0x10001, 0x8000, CRC(61aedfdb) SHA1(0f492dc8102386a1c475c5637fb7853d81d3efb6), ROM_SKIP(1) )
ROM_END

// Goldstar 611-606A - Chipset: CHIPS P82C206 P82C301 P82A303 P82C302 P82A304 2xP82A305 -
// OSC: 14.318 - 9.6000000 MHz - 40.000000 MHz - 16.000000 MHz
// BIOS: TI CMC3000 - BIOS-String: Phoenix 80386 ROM BIOS PLUS Version 1.10 01 - release 2.7B
ROM_START( gs611606a )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROMX_LOAD( "386-goldstar-e_32k.bin", 0x10000, 0x8000, CRC(3f358257) SHA1(1570f3de1955895c29c1c4240e1cd47aadff1be0), ROM_SKIP(1) )
	ROMX_LOAD( "386-goldstar-o_32k.bin", 0x10001, 0x8000, CRC(c5d75635) SHA1(70ceb4089bfd3af6853c3d6e28dbded0c43f6a40), ROM_SKIP(1) )
ROM_END

// DFI386-20.REV0 - Chipset: Chips 2xP82B305 P82A304, P82C302 P82C301 P82C206, two unreadable - initializes graphics card then hangs
// BIOS: AMI 386 BIOS PLUS Ser.#: 102856 - Keyboard BIOS: AMI 386 BIOS PLUS Ser.#:102856
// CPU: i386DX-20 - ISA8: 1, ISA16: 5, Memory: 1 - Memory card shown in photos
// OSC: OSC1: 14.31818, OSC2: 16.000MHz, OSC3: unreadable, OSC4: 40.000MHz
ROM_START( dfi386 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROMX_LOAD( "386-dfi-386-20_even_32k.bin", 0x10000, 0x8000, CRC(2d1309f8) SHA1(a75816b97d1f763dba39bdccf8e58729a58b0e56), ROM_SKIP(1) )
	ROMX_LOAD( "386-dfi-386-20_odd_32k.bin", 0x10001, 0x8000, CRC(1968fe11) SHA1(b5662daa57751859d2cfa7740f708277cbe35080), ROM_SKIP(1) )
ROM_END


/***** 386 Motherboards using the Forex FRX36C300 + FRX46C402; SiS 85C206 chipset *****/

// Chipset: FOREX FRX46C402 FRX36C300 SIS 85C206 SiS 85C206 - CPU: Intel 80386DX-16 - ISA16: 7, ISA16/Memory: 1 - OSC: 66.000MHz
ROM_START( frxc402 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS: AMI 386 BIOS PLUS - Ser. 006707 - BIOS-String: 30-0400-ZZ1266-00101111-070791-FORX-0 / FRX/386DX CACHE SYSTEM
	ROM_SYSTEM_BIOS(0, "frx386", "FRX/386")
	ROMX_LOAD( "386-forex.bin", 0x10000, 0x10000, CRC(4a883c14) SHA1(1c2de190ccd152ff894f9fd128e028d4fa63109a), ROM_BIOS(0))
	// 1: Chipset: Forex FRX36C300 + FRX46C402; IMP 82C206 - ISA16: 8, memory extension connector on board but not fitted
	// BIOS: AMI - BIOS-String: - 30-0400-ZZ1139-00101111-070791-FORX-0, FRX/386DX CACHE SYSTEM - Keyboard BIOS: Intel P8942HP with AMI KB-BIOS-VER-F - OSC: 14.31818MHz, 66,667MHz
	ROM_SYSTEM_BIOS(1, "frximp", "Forex 386 with IMP chip")
	ROMX_LOAD( "386-imp82c206p.bin", 0x10000, 0x10000, CRC(6f340961) SHA1(393720e1bfe3d323a34106992a65dd593284bf95), ROM_BIOS(1))
ROM_END

// RAM: 8xSIMM30, Cache: 10 sockets, 5xATT7C199P occupied - ISA8: 2, ISA16: 6 - CPU: AM386DX/DXL-40, FPU: IIT 3C87-25
// OSC: 80.000MHz, 14.318180MHz - Keyboard-BIOS: AMI 386 BIOS ZA902884
ROM_START( smih0107 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// BIOS-String: 30-0400-428005-00101111-070791-FORX-0 / BIOS ID SMIH0107 / IT9112
	ROM_LOAD( "ami_smih0107.bin", 0x10000, 0x10000, CRC(970bb0c0) SHA1(4a958887485f7239d25fa7b0c98569b97ce93800))
ROM_END


/***** 386 Motherboards using the Forex FRX46C402 + FRX46C411 + SiS 85C206 chipset */////

// PT-581392 - CPU: AMD 386DX-40 FPU: ULSI Advanced Math Coprocessor DX/DLC 40MHz US83C87
// BIOS : AMI 07/07/1991, on a 27C512 type EPROM (64KB) Ser.# 007139, BIOS-String : 30-0400-ZZ1101-00101111-070791-FORX-0 FRX/386DX CACHE SYSTEM
// Keyboard-BIOS: AMI, Ser.# 007139 - OSC: 14.31818, 80.000MHz - ISA16: 8
ROM_START( pt581392 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "pt-581392 386dx.bin", 0x10000, 0x10000, CRC(389a93de) SHA1(8f1320b1d163167272cfad073f58c355e31fcf6f))
ROM_END

// Micro-Express Inc. Forex 386 Cache - Chipset: Forex FRX46C402, FRX46C411, Morse 92A206S - Keyboard BIOS: Lance LT38C41
// BIOS: EPROM, AMI 386 BIOS, #ZA605315 - CPU: AM386DX-40 - OSC: 66.6670MHz - ISA8: 2, ISA16: 5
// BIOS-String: 30-0701-001585-00101111-121291-microtel-0 / Microtel Computer products present C3HF/09/92
ROM_START( frx386c )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "forex386.bin", 0x10000, 0x10000, CRC(007b5565) SHA1(cf749fe05cacebb2230cd7493523ae55e80eea8b))
ROM_END


/***** 386 Motherboards using the Macronix MX83C305(A)(FC), MX83C06(A)(FC) chipset *****/

// CACHING TECH CORPORATION C386MX - Chipset: MX83C306 MX - BIOS: AMI - BIOS-ID: 31-0100-001190-00101111-121291-MXIC
// CPU: 386DX, FPU: Cyrix - RAM: 8xSIMM30 - ISA8: 2, ISA16: 6
ROM_START( ctcc386mx ) // nine short beeps, no display
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "c386mx.bio", 0x10000, 0x10000, CRC(9b24ce11) SHA1(d027da188cbdfbe34b279bd3bd84eccda75b4a5a))
ROM_END

// TAM/33/40-MA0 (CM318R00,M31-R00) - Chipset: MX83C305, MX83C306 - CPU: AMD Am386DX-40 - ISA16:8
// OSC: 80.000MHz - 14.31818 - BIOS: AMI 386 BIOS PLUS S/N OA2050592 - BIOS-String: 31-0100-001105-00101111-121291-MXIC-0 - 386DX/Cx486DLX TAM/33,30-MA0/MA01, 09/10/1992
ROM_START( tam3340ma0 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "tam_33_40-ma0.bin", 0x10000, 0x10000, CRC(56411a9f) SHA1(a6c80ea531912b758fd5b573d4fa125172cacce7))
ROM_END

// Octek Jaguar V rev.1.4 - Chipset: MX83C: MX83C305FC, MX83C306FC
// CPU: AMD 386DX-40, FPU socket provided - OSC: 80.000MHz, 14.31818
ROM_START( ocjagv )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: MR BIOS (r) V1.40
	ROM_SYSTEM_BIOS(0, "jagvmr14", "Jaguar V MR-BIOS 1.40")
	ROMX_LOAD( "bios.bin", 0x10000, 0x10000, CRC(a552d6ad) SHA1(91bae14c3ec7edbc9ef240fec1be17f3582d7ec2), ROM_BIOS(0))
	//1: AMI BIOS// BIOS: AMI 386DX ISA BIOS AA0797325 - BIOS-String: 31-0100-426069-00101111-121291-MXIC-0 MX-DIR_001
	// Keyboard-BIOS: Intel
	ROM_SYSTEM_BIOS(1, "jagvami", "Jaguar V AMI BIOS")
	ROMX_LOAD( "octek_jaguar_v_ami_bios_isa386dx.bin", 0x10000, 0x10000, CRC(f8d14914) SHA1(14e8ecc4794920dc530fc6bd12ad64494e2544e5), ROM_BIOS(1))
ROM_END

// DTK MBA-032Q TK83305-4N-D-03 - Chipset: MACRONIX MX83C305FC, MX83C306FC, HM6818A - BIOS/Version: Award v4.20, MXIC 80386 CACHE SYSTEM, S/N 0089082653
// BIOS string: 08/07/92-MX83C305/306-113b0000-00 (113B00AC) - Keyboard BIOS: Award, S/N: 008902654 - CPU: i386DX-25, solder pads for 80386, FPU socket provided
// RAM: 8xSIMM39 - OSC: 14.31818M, 80.000MHz - ISA8: 2, ISA16: 6
ROM_START( mba032q )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "mba-032q.bin", 0x10000, 0x10000, CRC(23481187) SHA1(1b6d0f54ce73853fcdd43588196bb6072b39d068))
ROM_END

ROM_START( mx83c305 )
	// 0: AMI BIOS, BIOS-String:  31-0101-009999-00101111-121291-MXIC-0 / 09/02/1992 - Keyboard-BIOS: JETkey V5.0
	// Chipset MX83C05AFC, MX8306AFC - CPU: AMD AM386DX-40, OSC: 14.31818 - ISA8: 1, ISA16: 5
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS( 0, "mxic01", "MXIC #1")
	ROMX_LOAD( "mxic.bin", 0x10000, 0x10000, CRC(81853049) SHA1(d855b8d935417cfcfd6580fe3ed4ea393dd49b35), ROM_BIOS(0))
	// 1: BIOS-String: 30-0200-009999-00101111-111192-MXIC-0 / 12/15/1993
	ROM_SYSTEM_BIOS( 1, "mxic02", "MXIC #2")
	ROMX_LOAD( "3mxm001.bin", 0x10000, 0x10000, CRC(62fcd52b) SHA1(fa34c27be4627c68fe5c828451d86cbfad0ba358), ROM_BIOS(1))
ROM_END


/***** 386 motherboards using the OPTi 82C381/382 "HiD/386 AT chipset" *****/

// CPU: 386DX-25 - Chipset: OPTI 82C382 25MHz, 82C381P, 82C206 - BIOS: AMI; 09/15/90
// BIOS-String: EOX3-6069-083090-K0
ROM_START( op82c381 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "3opm004.bin", 0x10000, 0x10000, CRC(933c2c2b) SHA1(191a1a80c128430a0a461ff9202d27969a715d9d))
ROM_END

// Shuttle HOT-304 - Chipset: Opti F82C382, Opti (erased), UMC UM82C206L - OSC: 14.31818MHz, 50.000MHz
// BIOS: AMI, Ser.Nr. 150796 - BIOS-String: 30-0101-DK1343-00001111-050591-OPBC-0 - Keyboard BIOS: AMI Ser.Nr. 209210 - ISA8: 1, ISA16: 6, ISA16/Memory: 1
ROM_START( hot304 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-hot-304.bin", 0x10000, 0x10000, CRC(cd4ad4ec) SHA1(50f1b7a15096fff7442d575a47728ba4709b2f39))
ROM_END

// Arche Technologies Inc. KMA-300G-25 - Chipset: OPTi 82C381, 82C382, UMC206L - CPU: i386-25, FPU: socket provided - RAM: SIMM30x8, Cache: 8x5C6408-35
// ISA8: 1, ISA16: 6, ISA16/Memory: 1 - OSC: 50.000MHz, 14.31818 MHz
// BIOS-String: EOPG-3700-040990-K0
ROM_START( kma300g )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "bios.bin", 0x10000, 0x10000, CRC(3149a4a4) SHA1(6a027ed94568a89a800360119da0c568a2a29e19))
ROM_END


/***** 386 motherboards using the Opti F82C206, 82C391B, 82C392 chipset *****/

ROM_START( op82c391 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: CPU: 386DX - Chipset: OPTi 82C391 B2, 82C392 B - BIOS: AMI; 07/07/91; AA 0571504
	// BIOS-String: 30-0100-DK1343-00101111-070791-OPWB3/B-0
	ROM_SYSTEM_BIOS(0, "39101", "82C391 #1")
	ROMX_LOAD( "3opm001.bin", 0x10000, 0x10000, CRC(3cb65e60) SHA1(c91deaba1b34008449d6cc2aa94d115c47e0640a), ROM_BIOS(0))
	// 1: BIOS: AMI; 05/05/91; AMI 386C BIOS; #1023992
	ROM_SYSTEM_BIOS(1, "39102", "82C391 #2") // no display, nine beeps
	ROMX_LOAD( "3opm005.bin", 0x10000, 0x10000, CRC(ef3dcdde) SHA1(53a8d0af776362d5b92d1cce92d6ca8dbeb33398), ROM_BIOS(1))
	// 2: BIOS: AMI; 07/07/91 - no display
	ROM_SYSTEM_BIOS(2, "39103", "82C391 #3")
	ROMX_LOAD( "3opm011.bin", 0x10000, 0x10000, CRC(6706c85a) SHA1(70af6de83e59df3d9b74e904fde98d0b9cbdaae9), ROM_BIOS(2))
	// 3: BIOS-String: 30-0100-000000-00101111-070791-OPTi391-0
	ROM_SYSTEM_BIOS(3, "39104", "92C391 #4")
	ROMX_LOAD( "3opm12.bin", 0x10000, 0x10000, CRC(fa9592c5) SHA1(f9042163e7e2762e999687c3ec94d576f5b7c499), ROM_BIOS(3))
	// 4: BIOS-String: MR BIOS (tm) V1.30 - RAM Pattern Test Failed at 0F0000H
	ROM_SYSTEM_BIOS(4, "mrv130", "MR BIOS V1.30")
	ROMX_LOAD( "opti_82c391_386_mr_bios_v130.rom", 0x10000, 0x10000, CRC(458c9405) SHA1(845ea33e4e228aa98d1bd3b0148fa306754e3d78), ROM_BIOS(4))
ROM_END

ROM_START( op386wb )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// OPTi 386WB VER.1.0 - OSC: 66.6670MHz, 50.000MHz, 14,31818 - ISA8: 1, ISA16: 7
	// BIOS: 1006229 - Keyboard-BIOS: Intel P8942AHP
	// 0: BIOS-String: 30-0201-D41107-00101111-050591-OPWB-0
	ROM_SYSTEM_BIOS( 0, "1006229", "1006220")
	ROMX_LOAD( "386-opti-386wb-10.bin", 0x10000, 0x10000, CRC(1a5dd6b2) SHA1(9e6b556bfdf21d6f3cba6a05a3092887a71a24a8), ROM_BIOS(0))
	// 1: BIOS-String: 30-0201-D41107-00101111-050591-OPWB-0
	ROM_SYSTEM_BIOS( 1, "d41107", "D41107")
	ROMX_LOAD( "ami_d41107.bin", 0x10000, 0x10000, CRC(2b6cc50d) SHA1(69f69e0295abf5e331fdf01dc8e6b1c0c6591992), ROM_BIOS(1))

ROM_END

// Shuttle HOT-307H - BIOS-String: 30-0100-DK1343-00101111-070791-OPWB3/B-0 - CPU: 386DX - Chipset: OPTi 82C391 B2, 82C392 B - BIOS: AMI; 07/07/91; AA 0571504
ROM_START( hot307h )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "3opm001.bin", 0x10000, 0x10000, CRC(3cb65e60) SHA1(c91deaba1b34008449d6cc2aa94d115c47e0640a))
ROM_END


/***** 386 Motherboards using the OPTi495SLC chipset => "qdi" in the 486 BIOS section in at486.cpp uses that chipset too *****/

ROM_START( opti495slc )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: Chipset: OPTi 82C495SLC / F82C206, BIOS: AMI 486086 - BIOS-String: 40-040A-001102-00101111-111192-OP495SLC-0
	// Keyboard-BIOS: AMI - CPU: AM386DX-40, FPU socket provided - ISA8: 1, ISA16: 5 - OSC: 14.31818
	ROM_SYSTEM_BIOS(0, "op495slc01", "OP495SLC #1")
	ROMX_LOAD( "op495slc01.bin", 0x10000, 0x10000, CRC(0b25044b) SHA1(1b585f0d73ea963dcfbf421325e7da6dd3dd918f), ROM_BIOS(0))
	// 1: BIOS-String: 40-0200-001107-00101111-111192-OP495SLC-0 - OPTI 495SLC 80386 ONLY - BIOS: AMI 386C BIOS 1605865
	// Keyboard-BIOS: AMI 386C BIOS Keyboard  ISA8: 1, ISA16: 5 - CPU: AMD AM386DX-40 - OSC: 14.3
	ROM_SYSTEM_BIOS(1, "op495slc02", "OP495SLC #2")
	ROMX_LOAD( "op495slc02.bin", 0x10000, 0x10000, CRC(4ff251a2) SHA1(e8655217bd46d50af6b30184bf462376d0e388c6), ROM_BIOS(1))
	// 2: BIOS-String: - Same board exists with an OPTi495XLC chip, possibly from A-Trend
	ROM_SYSTEM_BIOS(2, "op495slc03", "OP495SLC #3") // no display
	ROMX_LOAD( "486dlc-unknown.bin", 0x10000, 0x10000, CRC(2799e876) SHA1(ce7b421ecb27d915585c1a98bebb17cc5c2463e7), ROM_BIOS(2))
	// 3: 364 Ver. 1.1 - Chipset: OPTi 82C495SLC, F82C206Q - CPU: AMD AM386DX-40 - RAM: 8xSIMM30, Cache: 5x14pin
	// ISA8: 1, ISA16: 5 - BIOS: AMI 386C BIOS 1684393 - Keyboard-BIOS: AMI 386C BIOS KEYBOARD 1684393
	// BIOS-String: 40-040A-001107-00101111-111192-OP495SLC-0 / OPTI-495SLC BIOS VER 2.1
	ROM_SYSTEM_BIOS(3, "op495slc04", "OP495SLC #4")
	ROMX_LOAD( "386c_mobo-364-v1.1.bin", 0x10000, 0x10000, CRC(826ec2d1) SHA1(cc13e385c6614eb654ee0669f67df51f1e1fa585), ROM_BIOS(3))
ROM_END


/***** 386 Motherboards using the OPTi495XLC chipset: OPTi 82C495XLC F82C206, BIOS: AMI 386DX BIOS Ser.#:AA2602776, Keyboard-BIOS: Lance LT38C41 - ISA8: 1, ISA16: 5 *****/

ROM_START( opti495xlc )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-081L-001343-00101111-080893-OP495XLC-F  / OPTi495XLC For 386
	ROM_SYSTEM_BIOS(0, "optimini", "OPTi Mini 82C495XLC")
	ROMX_LOAD( "386-opti-mini.bio", 0x10000, 0x10000, CRC(04c75e45) SHA1(d5bf92421dda3191c6da12ae2fa31c9ee7a831e1), ROM_BIOS(0) )
	// 1: MR BIOS (r) V1.60
	ROM_SYSTEM_BIOS(1, "mr495xlc", "MR BIOS for OPTi 82C495XLC") // use Hercules
	ROMX_LOAD( "mr-3dx94.rom", 0x10000, 0x10000, CRC(6925759c) SHA1(540177fe2c10e20037893c9763b0bf6e35163c9c), ROM_BIOS(1) )
	// 2: possibly from A-Trend (A1742X REV.C 94V-0), exists with an OPTi495SLC chip, see above section, ISA8: 2, ISA16: 4, ISA16/VL: 2
	// BIOS-String: X0-0804-001117-00101111-080893-OP395XLC-0 / OPTI 495XLC 3/486 BIOS VER 5.02_T 94/07/07
	ROM_SYSTEM_BIOS(2, "op82c495xlc", "82C495XLC") // this one could also be listed as a 486 board as it has solder pads and sockets for CPUs from 386 to true 486s
	ROMX_LOAD( "at080893.bin", 0x10000, 0x10000, CRC(6b49fdaa) SHA1(5b490d1d1216763ef89688c8e383c46470272005), ROM_BIOS(2) )
	// 3: BIOS: AMI; 08/08/93; AA2740000 - hangs
	ROM_SYSTEM_BIOS(3, "mao13", "MAO13 Rev. A")
	ROMX_LOAD( "3opm002.bin", 0x10000, 0x10000, CRC(2d9dcbd1) SHA1(d8b0d1411b09767e10e66b455ebc74295bd1b896), ROM_BIOS(3) )
ROM_END


/***** 386 motherboards using the SIS Rabbit : 85C310 / 85C320 / 85C330 / 85C206 chipset *****/

// ASUS ISA-386C - BIOS : AMI 05/05/1991, on a 27C512 type EPROM (64KB)
// BIOS-String : 30-0105-001292-00101111-050591-SISDFC-386 - // ISA8: 2, ISA16:5, ISA16/Memory: 1
ROM_START( isa386c )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "asus_isa-386c_bios.bin", 0x10000, 0x10000, CRC(55e6d1bb) SHA1(e1ac490a30f63b6e4d6d9d0fbaea3d132b8ff053))
ROM_END

// Chaintech 333SC - Chipset: UMC UM82C206L, three smaller SiS chips (unreadable, probably SiS Rabbit)
// CPU/FPU present - BIOS: AMI 386 BIOS - Keyboard-BIOS: AMI
// BIOS-String: ESIS-1128-040990-K0 - ISA8: 2, ISA16: 6 - OSC: 14.31818, 66.000MHz
ROM_START( chn333sc )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "bios.bin", 0x10000, 0x10000, CRC(f8b2b0bc) SHA1(2799cce621b93bf38b04deeb419d25a73f7416f4))
ROM_END

// Octek Jaguar II - Chipset: SiS 85C330, 85C320, 85C206, 85C310 - CPU: i386-33, FPU: i387-33 - RAM: 8xSIMM30, Cache: 8xIS61C64-25N
// BIOS: AMI - Keyboard-BIOS: AMI KB-BIOS-VER-F - ISA8: 2, ISA16: 5, ISA16/Memory: 1 - OSC: 66.000MHz, 14.31818
// BIOS-String: 30-0100-006069-00101111-020291-SISC / 0033P-002
ROM_START( ocjagii )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "ami_386_zz457511.bin", 0x10000, 0x10000, CRC(b5bc6a9a) SHA1(b74f592dd3bcd2978ed7d895d483f83413e0f8d5))
ROM_END


ROM_START( sisrabb )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 30-0000-D01128-00101111-070791-SISD-0
	ROM_SYSTEM_BIOS(0, "3sim001", "3sim001")
	ROMX_LOAD( "3sim001.bin", 0x10000, 0x10000, CRC(2982f552) SHA1(f1849c207d8c802faaf8ef628f88b28256e7fb31), ROM_BIOS(0))
	// 1: PLATO TECHNOLOGY CO., LTD. - Chipset: SiS 'rabbit': SIS 85C320, 85C330, 85C310, 85C206 - CPU: Intel 80386DX-33, FPU: i387DX-33
	// RAM: 16xSIMM30, Cache: 8xKM688B65P-25, 2xQS8888-25P - BIOS: AMI 386 BIOS Ser# Z403736 - Keyboard BIOS: AMI - BIOS-String: ESIS-1393-040990-K0 / SIS-B 386 BIOS
	// OSC: 50.000MHz, 66.0000MHz, 14.31818MHz - ISA8: 2, ISA16: 6
	ROM_SYSTEM_BIOS(1, "plato", "Plato")
	ROMX_LOAD( "386-sis-z403736.bin", 0x10000, 0x10000, CRC(8230f4c1) SHA1(952de1e16efe7e3a7514b68ea251b88192de3ac8), ROM_BIOS(1))
ROM_END


/***** 386 Motherboards  using the Symphony SL82C362 SL82C461 SL82C465 chipset */

// FIC 386 SC Rev A2 - Chipset: Symphony SL82C461, SL82C465, SL82C362
ROM_START( 386sc )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS: AMI 386 BIOS Ser.#: ZZ006975, BIOS-String:  30-0200-DF1211-00101111-042591-SYMP-0 / 386DX BIOS for SYMLABS SL82C360 - Keyboard-BIOS: AMI #Z357365
	// CPU: unreadable, FPU: Cyrix 387DX-25 - OSC: 40.000MHz, 14.31818 - ISA8: 1, ISA16: 7
	ROM_SYSTEM_BIOS(0, "386sc1", "386SC #1")
	ROMX_LOAD( "386_sc_symphony.bin", 0x10000, 0x10000, CRC(fabe369c) SHA1(211ff63dd874c273135d1427db3562d752c2bade), ROM_BIOS(0))
	// 1: ID: FIC 386SC REV A2 MBZ86418 - CPU: AMD 386DX/DXL-40, FPU socket provided - RAM: 8xSIMM30, Cache: 512KB
	// BIOS Version: AMI 386DX BIOS - ZZ000294 - BIOS String: 30-0200-ZZ1121-00101111-042591-SYMP-0 - 386DX BIOS FOR SYMLABS SL86C360 - ISA8: 1, ISA16: 7 - OSC: 14.31818, 80.000MHz
	ROM_SYSTEM_BIOS(1, "386sc2", "386SC #2")
	ROMX_LOAD("fic386sc.bio", 0x10000, 0x10000, CRC(6154adb7) SHA1(96c495d9a9975e1af9b42384712e609e3ffcff4e), ROM_BIOS(1))
ROM_END

ROM_START( 386sc2c )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 20-0200-DF1121-00101111-102591-SYM_386B-0 / 386DX/SX (S1A.P)
	ROM_SYSTEM_BIOS(0, "s1a", "S1A.P") // Chipset: SYMPHONY SL82C362, SL82C461, SL82C465
	ROMX_LOAD( "386-sc-2c_ami_za492668.bin", 0x10000, 0x10000, CRC(b408eeb7) SHA1(cf1974492119e1aae623fa366d5760343e827e52), ROM_BIOS(0))
	// 1: BIOS-String: 20-0200-DF1121-00001111-102591-SYM_386B-0 / 386SX/DX (S1B)
	ROM_SYSTEM_BIOS(1, "s1b", "S1B") // also on FIC 386-SC-HG
	ROMX_LOAD( "ami_386_za590821.bin", 0x10000, 0x10000, CRC(51a4c231) SHA1(4ad65408f2a401ff262934f886937a2615c08e21), ROM_BIOS(1))
ROM_END

// FIC 386-SC-HG - Chipset: SYMPHONY SL82C362, SL82C461, SL82C465 - CPU: AMD Am386DX/DXL-40, FPU socket provided - BIOS: AMI 386 BIOS ZA977287
// Keyboard BIOS: Intel/AMI - RAM: 8xSIMM30 - ISA16: 6 - OSC: 80.000MHz
ROM_START( 386schg ) // BIOS-String: 20-0200-DF1121-00101111-102591-SYM_386B-0 / 386DX/SX(S1B)
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-za877287-386-sc-hg.bin", 0x10000, 0x10000, CRC(fcaa06c0) SHA1(5ce6258a26311cec46e51cb16bcb66e9c68d16b2))
ROM_END

// Peacock P386DX-40 REV: 1.0 - Chipset: Symphony SL82C461, SL82C362, SL82C465, DS1287 - CPU: AMD Am386DX-40, FPU: IIT 3C87-40 - RAM: 8xSIMM30, Cache: 4xW4256AK-20, 1xTC5588P-20
// BIOS: AMI 386 BIOS #ZA22147 - OSC: 80.000MHz, 14.31818, 24.000 - on board: IDE, Floppy - ISA16: 6
ROM_START( p386dx40 ) // BIOS-String: 30-0200-DF1121-00101111-070791-SYM_AUTO-0 / Peacock 386 DX Ver. 1.0
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386_peacock-p386dx-40-za224147.bin", 0x10000, 0x10000, CRC(f97562cf) SHA1(354b6ffd345c9f50beca0f836b0d4a92df1e7c48))
ROM_END


/***** 386 Motherboards using the UM82C491F chipset *****/

ROM_START( um82c491f )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: TAM/33/40-U2 - BIOS: AMI S/NO. OA 242412 - BIOS-String: 40-0102-001105-00101111-040493-UMC491F-0 / TAM/33,40-U2, 08/11/1993
	// ISA8: 1, ISA16: 5 - OSC: 80.000MHz, 14.31818
	ROM_SYSTEM_BIOS(0, "tam3340u2", "TAM/33/40-U2")
	ROMX_LOAD( "tam_umc491f.bin", 0x10000, 0x10000, CRC(718890d5) SHA1(52336cfc7cd0f0f51799c999cefcfed2b2942211), ROM_BIOS(0))
	// 1: Board is only marked "rev.0.3, looks like 386GRN - CPU: AMD AM386DX-40 - OSC: 14.31818 - ISA8: 1, ISA16: 5
	// Chipset: UMC UM82C491F - BIOS-String: 08/30/93-UMC-491-214X2000-OO - BIOS: Award 386 D2026361 - Keyboard BIOS: JETkey V3.0
	// additional info from chukaev.ru54.com: REV:0.4 board has JETkey V5.0 keyboard BIOS, uses same motherboard BIOS
	ROM_SYSTEM_BIOS( 1, "386grn", "386GRN-like board rev.03")
	ROMX_LOAD( "386dx40-27c512.bin", 0x10000, 0x10000, CRC(692a4d52) SHA1(7970a05586eacfe4bfdc575b17bbbfb7ff1c86b0), ROM_BIOS(1))
	// 2: BIOS: AMI; 04/04/93 - CPU: 386DX-40 - BIOS-String: 40-0102-001277-00101111-040493-UMC491F-0
	ROM_SYSTEM_BIOS( 2, "491f01", "UM82C491F #1")
	ROMX_LOAD( "3umm005.bin", 0x10000, 0x10000, CRC(032e78f2) SHA1(5271c4362284ec87840b3fb23542506a72a328c2), ROM_BIOS(2))
	// 3: BIOS-String: 08/30/93-UMC-491-214X2000-OO / CACHE 386/486 SYSTEM BIOS
	ROM_SYSTEM_BIOS( 3, "491f02", "UM82C491F #2")
	ROMX_LOAD( "3umw007.bin", 0x10000, 0x10000, CRC(d82c9bef) SHA1(36e8d1c7629642cbcc337721eef1c73f1f0ed92c), ROM_BIOS(3))
ROM_END


/***** 386 Motherboards using the UMC UM82C493F/UM82C491F chipset or badge engineered varieties (BIOTEQ) *****/

// BIOS-String: 40-0100-001494-00101111-080893-UMC491F-0 / 11/26/93 - CPU: TX486DLC/E-40GA, IIT 4C87DLC-40 - ISA8: 1, ISA16: 5 - BIOS: AMI; 208808; 08/08/93
ROM_START( um82c493f )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS( 0, "493f01", "UM82C493F #1" )
	ROMX_LOAD( "3umm007.bin", 0x10000, 0x10000, CRC(8116555a) SHA1(8f056a83de60373ed26026a226eead19868abeca), ROM_BIOS(0))
ROM_END

// 0: 386-4N-D04A
ROM_START( 4nd04a )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// BIOS-String: 40-0102-428003-00101111-080893-UMC491F-0 - UMC 4913 386 IVN 1.0 1993.12.22
	// Chipset: UMC UM82C493F, UM82C491F
	ROM_SYSTEM_BIOS( 0, "ivn10", "386-4N-D04A IVN 1.0" )
	ROMX_LOAD( "386-4n-d04a.bin", 0x10000, 0x10000, CRC(cf386b9c) SHA1(6fd4303e4f0d2ed75d4e7f36dc855037b1779e64), ROM_BIOS(0))
	// 1: 386-4N-D04A PCB V2.0 - BIOS-String: 40-0103-428003-00101111-080893-UMC491F-0 / UMC 4913 386 IVN 1.1 1994.1.31
	ROM_SYSTEM_BIOS( 1, "ivn11", "386-4N-D04A IVN 1.1" )
	ROMX_LOAD( "3umm006.bin", 0x10000, 0x10000, CRC(4056104d) SHA1(5e639e6766dc9a19296358e9a64a76ad57fc733a), ROM_BIOS(1))
	// 2: TK-82C491/493/386-4N-D04 - BIOS-String: (2c4x2u01) U-BOARD / 11/09/93-UMC-491-2C4X2U01-00 - ISA8: 1, ISA16: 5
	ROM_SYSTEM_BIOS( 2, "awa110993", "AWARD 11/09/93") // BIOS: Award; 386 BIOS; A3384454
	ROMX_LOAD( "3umw002.bin", 0x10000, 0x10000, CRC(2c510e81) SHA1(a12c672ec418cc4cd14482901f8ba34c50f319f5), ROM_BIOS(2))
	// 3: TK-82C491/493/386-4N-D04 - BIOS-String: 01/14/94-UMC-491-2C4X2U01-00 / U-BOARD
	ROM_SYSTEM_BIOS( 3, "awa011494", "AWARD 01/14/94")
	ROMX_LOAD( "3umw003.bin", 0x10000, 0x10000, CRC(64067839) SHA1(4ae3462619ef8da67f74d85ee7ab44bdb49a5728), ROM_BIOS(3))
ROM_END

// Biostar MB-1333/40PMB-CH, rev B.3 - Chipset: "Bioteq" [Atmel] AT40391, "Bioteq" G392 [Atmel AT40392], C&T P82C206
// BIOS: AMI 386 BIOS PLUS - Keyboard-BIOS: AMI - CPU: AM386-DX40 - OSC: 14.31818, <unreadable>
ROM_START( mb133340 )
	ROM_REGION32_LE(0x20000, "bios", 0) // the OPWB3 string also exists in the BIOS versions meant for the OPTI 82C391/392 chipsets
	// 0: BIOS-String: 30-0100-D61223-00101111-050591-OPWB3/B-0 / MB-1340PMA-CH, MB-1340PMB-CH, MB-1340PMD-CH, MB-1340PME-CH for B version..
	ROM_SYSTEM_BIOS(0, "opwb3b", "MB-1333/40PMB-CH OPWB3-B")
	ROMX_LOAD( "opwb3b.bin", 0x10000, 0x10000, CRC(c9cf46dd) SHA1(c9e58cb6fed770d92892672d0a910d448c507ac1), ROM_BIOS(0))
	// 1: BIOS-String: 30-0201-D61223-00101111-050591-OPWB-0 / MB-1333PMA-CH, MB-1333PMB-CH, MB-1333PMD-CH, MB-1333PME-CH
	ROM_SYSTEM_BIOS(1, "opwb", "MB-1333/40PMB-CH OPWB")
	ROMX_LOAD( "opwb.bin", 0x10000, 0x10000, CRC(9532c6d1) SHA1(48e889ed61921643147fea95224bcf42bb6e82fa), ROM_BIOS(1))
	// 2: BIOS-String: 40-0100-001223-00101111-040493-UMC491F-0 / MB-1333/40UCG-A, MB-1333/40UCG-B / MB-1433-40UDV-A, MB-1433/50UCV-C, MB6433/50UPC-A for EXT. RTC
	ROM_SYSTEM_BIOS( 2, "m21", "M21" )
	ROMX_LOAD( "3bim001.bin", 0x10000, 0x10000, CRC(9ea0ce67) SHA1(cb55a61cd43705a54e4109d816924c8820f78ae5), ROM_BIOS(2))
ROM_END


/***** 386 motherboards using the UMC UM82C481AF, UM82C482A/B/F, 82C206F chipset *****/

// QD-U386DX VER 1.0 - CPU/FPU: i386DX-33, IIT 3C87-33 - ISA8:2, ISA16: 5 - BIOS: AMI 386DX ISA BIOS (AA0119183)
// BIOS-String: 30-0200-428003-10101111-070791-UMC480A-F
ROM_START( qdu386dx ) // three short beeps (base 64k RAM failure)
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "3umm001.bin", 0x10000, 0x10000, CRC(5b6a7d0b) SHA1(02696eaaa5dd21fe4b3b39629aa926ae87a9a2bd))
ROM_END

// ASUS ISA-386U30 REV.2.2 - Chipset: UMC UM82C481AF, UM82C482AF, 82C206F - CPU: AM386DX-40 - OSC: 14.31818MHz, 32.000MHz - ISA8: 1, ISA16: 6
// BIOS: AMI 386DX BIOS AA0974582 - BIOS-String: - Keyboard-BIOS: AMI U2518640 MEGA-KB-F-WP
ROM_START( isa386u30 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "386-isa-386u30.bin", 0x10000, 0x10000, CRC(6d45a044) SHA1(63c06568f9db5ce12dc8dd0fb1ad1009a9fb24f6))
ROM_END


// Elitegroup FX-3000 REV:1.0 - Chipset: UMC UM82C481BF, UM82C482AF, UM82C206F - ISA16: 6
ROM_START( ecsfx3000 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-0100-001131-00101111-121291-UMCAUTO-0 / FX3000 V1.3 12-17-92 - Keyboard BIOS: AMI/Intel - BIOS: FX3000-014 - CPU: AM386DX-40
	ROM_SYSTEM_BIOS(0, "v13", "V1.3")
	ROMX_LOAD( "fx-3000-bios.bin", 0x10000, 0x10000, CRC(f93c9563) SHA1(46a71e7fbc9238dd470d6d5ce3bc1e057f3aae24), ROM_BIOS(0))
	// 1: BIOS-String: 30-0500-D01131-00101111-070791-UMCWB-0 / FE386 V1.1 12-03-92 - Keyboard-BIOS: Lance LT38C41 - BIOS: AMI-1131 / S/NO. E-92488183 / FE 386-012 - CPU: Cyrix 486DLC-33GP FPU: Cyrix Cx87DLC-33QP - OSC: 66.667MHz, 14.31818
	ROM_SYSTEM_BIOS(1, "v11", "V1.1")
	ROMX_LOAD( "486-fx3000.bin", 0x10000, 0x10000, CRC(af303f08) SHA1(65dfa2541d2b08746f91012a2ae0121636402aac), ROM_BIOS(1))
ROM_END

ROM_START( um82c481af )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// 0: BIOS-String: 40-0100-001266-00101111-121291-UMCAUTO-0 - 3DIUD-1.2
	// Chipset: // UMC UM92C206F, UM82C482AF, UM82C481BF - MB manufacturer according to BIOS is Modula Tech Co
	// ISA8: 1, ISA16: 6 - RAM: 8xSIMM30, Cache: 9x28pin, used: 4xIS61C256AH-20N, 1xW2465AK-20, CPU: AMD 386DX-40
	// CPU and FPU sockets provided - BIOS: AMI 386 BIOS, Keyboard-BIOS: AMI
	ROM_SYSTEM_BIOS(0, "3diud", "386 UMC 3DIUD")
	ROMX_LOAD( "386-umc-3flud.bin", 0x10000, 0x10000, CRC(2e795a01) SHA1(02e9e2871c1c1a542f44ab5eef66aee4b04225c1), ROM_BIOS(0))
	// 1: BIOS: Microid Research; 02/26/93 - BIOS-String: MR BIOS (r) V1.44
	ROM_SYSTEM_BIOS(1, "mr1441", "MR BIOS V1.44 #1") // resets continuously
	ROMX_LOAD( "3umr001.bin", 0x10000, 0x10000, CRC(466a115e) SHA1(077d797c653528062f1c87b03c608427c35c5505), ROM_BIOS(1))
	// 2: BIOS : MR 386DX/86DLC BIOS V.1.44 - Keyboard BIOS: JETkey V3.0 - CPU: TI 486DLC-40BGA, FPU: IIT 4C87DLC-40 - RAM: 8xSIMM30, Cache
	// OSC: 14.31818, 80.000MHz - ISA8: 1, ISA16: 6
	ROM_SYSTEM_BIOS(2, "mr1442", "MR BIOS V1.44 #2") // resets continuously
	ROMX_LOAD( "um82481m.bin", 0x10000, 0x10000, CRC(f617e1ce) SHA1(73ee80cb9f50547f26adbe0bfd2435b01728dd09), ROM_BIOS(2))
ROM_END


/**************************************************************************
  80386 DX Desktop
**************************************************************************/

// Amstrad PC2386
ROM_START( pc2386 )
	ROM_REGION32_LE( 0x40000, "bios", 0 )
	ROM_LOAD( "c000.bin", 0x00000, 0x4000, CRC(33145bbf) SHA1(c49eaec19f656482e12c8bf282cd4ee5986d227d) )
	ROM_LOAD( "f000.bin", 0x30000, 0x10000, CRC(f54a063c) SHA1(ce70ec493053afab662f51199ef9c9304a209b8e) )
	ROM_FILL(0x3fff1, 1, 0x5b) // f000:e05b is the standard at reset vector jump address
	ROM_FILL(0x3fff2, 1, 0xe0) // why does this rom's point to nowhere sane?
	ROM_FILL(0x3fff3, 1, 0x00) // and why does the rest of the rom look okay?
	ROM_FILL(0x3fff4, 1, 0xf0)

	ROM_REGION( 0x1000, "keyboard", 0 ) // PC2286 / PC2386 102-key keyboard
	ROM_LOAD( "40211.ic801", 0x000, 0x1000, CRC(4440d981) SHA1(a76006a929f26c178e09908c66f28abc92e7744c) )
ROM_END

// Atari PC 5 - American Megatrends 386XT Series-4 motherboard - on board EGA
// screen remains blank, 1 beep repeated (DRAM refresh failure)
ROM_START( ataripc5 )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD32_BYTE("ami_pc5_1.00_rom0.bin", 0x00000, 0x8000, CRC(496149a6) SHA1(81033b22af830af8306abfde03a194739fe54355))
	ROM_LOAD32_BYTE("ami_pc5_1.00_rom1.bin", 0x00001, 0x8000, CRC(3c82fe66) SHA1(dd6c2c3c3635761b1d928912269b8937cbdc09ae))
	ROM_LOAD32_BYTE("ami_pc5_1.00_rom2.bin", 0x00002, 0x8000, CRC(7dc5b53b) SHA1(33e138baa84a8acc629bde5a6b54e47d0d4508f1))
	ROM_LOAD32_BYTE("ami_pc5_1.00_rom3.bin", 0x00003, 0x8000, CRC(b588b7a8) SHA1(2f2597b14e54d03cf957cce47536266f68d3aa66))
ROM_END


/**************************************************************************
  80386 DX Laptop/Notebook
**************************************************************************/

// Triumph-Adler Walkstation 386DX - German version of the Olivetti D33
// VLSI TOPCAT chipset: VL82C330 + VL82C331 + VL82C332 + VL82C106; Austek A38202C; DP8473V
// Video board: Cirrus Logic CL-GD610 + CL-GD620 + CL-GD63
ROM_START( walk386dx )
	ROM_REGION32_LE( 0x20000, "bios", 0 ) // contains Cirrus Logic VGA BIOS
	ROM_LOAD( "am28f010_ctaa060125rc.bin", 0x00000, 0x20000, CRC(6cc540fe) SHA1(9853793d5433bbc5efc09c7f31c4a8a8f78d4549) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "cthj02_03_76.bin", 0x0000, 0x2000, NO_DUMP )
ROM_END

// Siemens PG 750 - Luggable programmer ("Programmiergerät") for the Siemens S5 automation systems, a 486 EISA version exists as well
// Chipset: CHIPS P82C302, P82C301C, P82C206, four more "CHIPS" ICs
// ISA16: 7
// on board: V24/Mouse, V24/Modem, Printer
ROM_START( pg750 )
	ROM_REGION32_LE( 0x20000, "bios", 0 )
	// 0: Phoenix 80386 ROM BIOS PLUS Version 1.10 14 / SIEMENS PG-750
	// Time-of-day clock stopped
	// EGA/TIGA Graphics System "Highgraph II"
	ROM_DEFAULT_BIOS("v402")
	ROM_SYSTEM_BIOS(0, "v40", "v40")
	ROMX_LOAD( "pg750_4.0_386_l.bin", 0x10000, 0x8000, CRC(1e6dcd40) SHA1(2def9f729f43652a7b8b32a42e4c073f580d39ce), ROM_SKIP(1)|ROM_BIOS(0) )
	ROMX_LOAD( "pg750_4.0_386_h.bin", 0x10001, 0x8000, CRC(389c20dd) SHA1(eb0e86ba88ac9742868689b2aac9911ed7acac74), ROM_SKIP(1)|ROM_BIOS(0) )
	// 1: Phoenix 80386 ROM BIOS PLUS Version 1.10 17 / SIEMENS PG-750/730
	// Time-of-day clock stopped
	ROM_SYSTEM_BIOS(1, "v402", "v402")
	ROMX_LOAD( "pg750_4.02_386_l.bin", 0x10000, 0x8000, CRC(208aac51) SHA1(49d50b7ade8f56bda203375a9b138adf2cb5e500), ROM_SKIP(1)|ROM_BIOS(1) )
	ROMX_LOAD( "pg750_4.02_386_h.bin", 0x10001, 0x8000, CRC(c6e14fb6) SHA1(dad6ab5c5a18341ec0dce53fe712b8367340506b), ROM_SKIP(1)|ROM_BIOS(1) )
ROM_END


/***************************************************************************
  Game driver(s)
***************************************************************************/

//    YEAR  NAME       PARENT   COMPAT   MACHINE    INPUT  CLASS         INIT         COMPANY                           FULLNAME         FLAGS
COMP( 199?, 386sc,     ibm5170, 0,       at386,     0,     at386_state,  init_at386,  "First International Computer",   "386 SC Rev A2", MACHINE_NOT_WORKING )
COMP( 199?, 386sc2c,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "386 motherboard using the Symphony chipset", MACHINE_NOT_WORKING )
COMP( 199?, 386schg,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "386-SC-HG", MACHINE_NOT_WORKING )
COMP( 199?, 4nd04a,    ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "386-4N-D04A (UMC chipset)", MACHINE_NOT_WORKING )
COMP( 199?, al486vd,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Daewoo",      "AL486V-D Rev:1.1", MACHINE_NOT_WORKING )
COMP( 199?, alacou,    ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Alaris",      "Cougar", MACHINE_NOT_WORKING )
COMP( 199?, alim1419,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "386 motherboards using the ALi M1419 chipset", MACHINE_NOT_WORKING )
COMP( 199?, alim1429,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "386 motherboards using the ALi M1429 A1 and M1431 A2 chipset", MACHINE_NOT_WORKING )
COMP( 199?, amibaby,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "AMI",         "Mark V Baby Screamer", MACHINE_NOT_WORKING )
COMP( 1990, aplanst,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Apricot",     "Apricot LANstation (Krypton Motherboard)", MACHINE_NOT_WORKING )
COMP( 1990, aplannb,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Apricot",     "Apricot LANstation (Novell Remote Boot)", MACHINE_NOT_WORKING )
COMP( 1987, apxeni,    ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Apricot",     "Apricot XEN-i 386 (Leopard Motherboard)" , MACHINE_NOT_WORKING )
COMP( 1988, at386,     ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<generic>",   "PC/AT 386 (12 MHz, MF2 Keyboard)", MACHINE_NOT_WORKING )
COMP( 1988, ataripc5,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Atari", "PC5", MACHINE_NOT_WORKING )
COMP( 199?, chn333sc,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Chaintech",   "333SC", MACHINE_NOT_WORKING )
COMP( 199?, comt386,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Commodore Business Machines",  "Tower 386", MACHINE_NOT_WORKING )
COMP( 198?, cs8230,    ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "386 motherboards using the CS8230 chipset", MACHINE_NOT_WORKING )
COMP( 199?, ctcc386mx, ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Caching Tech Corporation", "C386MX", MACHINE_NOT_WORKING )
COMP( 198?, dfi386,    ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "DFI", "386-20.REV0", MACHINE_NOT_WORKING )
COMP( 199?, dt386,     ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Commodore Business Machines", "DT386", MACHINE_NOT_WORKING )
COMP( 1988, ecs38632,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Elitegroup Computer Systems",      "ECS-386/32", MACHINE_NOT_WORKING )
COMP( 1988, ecs386a,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Elitegroup Computer Systems",      "ECS-386A", MACHINE_NOT_WORKING )
COMP( 199?, ecsfx3000, ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Elitegroup Computer Systems", "FX-3000 REV1.0", MACHINE_NOT_WORKING )
COMP( 1992, ecsum386,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Elitegroup Computer Systems",      "UM386 (Rev 1.1)", MACHINE_NOT_WORKING )
COMP( 199?, frx386c,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Micro-Express Inc.", "Forex 386 Cache", MACHINE_NOT_WORKING )
COMP( 199?, frxc402,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>", "386 motherboards with a FOREX FRX46C402/FRX36C300/SIS85C206 chipset", MACHINE_NOT_WORKING )
COMP( 1991, fu340,     ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Abit",        "FU340", MACHINE_NOT_WORKING )
COMP( 199?, ges9051n,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "GES 9051N-386C VER -0.01", MACHINE_NOT_WORKING )
COMP( 198?, gs611606a, ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Goldstar",    "GOLDSTAR P/N 611-606A Rev 1.0A", MACHINE_NOT_WORKING )
COMP( 198?, hot304,    ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Shuttle Computer International", "HOT-304", MACHINE_NOT_WORKING )
COMP( 198?, hot307h,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Shuttle Computer International", "HOT-307H", MACHINE_NOT_WORKING )
COMP( 199?, isa386u30, ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Asus",        "ISA-386U30 REV.2.2", MACHINE_NOT_WORKING )
COMP( 1989, isa386c,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Asus",        "ISA-386C", MACHINE_NOT_WORKING )
COMP( 199?, kma300g,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Arche Technologies Inc.", "KMA-300G-25", MACHINE_NOT_WORKING )
COMP( 199?, mb133340,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Biostar",     "MB-1340UCQ-B", MACHINE_NOT_WORKING )
COMP( 100?, mba032q,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "DTK", "MBA-032Q TK83305-4N-D-03", MACHINE_NOT_WORKING )
COMP( 199?, mokp386,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Morse", "KP920121523 V2.20", MACHINE_NOT_WORKING )
COMP( 199?, mom3v3,    ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Morse", "M3 V3.0", MACHINE_NOT_WORKING )
COMP( 199?, mx83c305,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>", "386 motherboards using the MX83C305(A)(FC)/MX83C05(A)(FC) chipset", MACHINE_NOT_WORKING )
COMP( 1992, ocjagii,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Octek",       "Jaguar II", MACHINE_NOT_WORKING )
COMP( 1992, ocjagv,    ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Octek",       "Jaguar V v1.4", MACHINE_NOT_WORKING )
COMP( 199?, op82c381,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "386 motherboards using the OPTi 82C381 chipset", MACHINE_NOT_WORKING )
COMP( 199?, op82c391,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "386 motherboards using the OPTi 82C391 chipset", MACHINE_NOT_WORKING )
COMP( 199?, opti495slc,ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>", "386 motherboards using a OPTi 82C495SLC chipset", MACHINE_NOT_WORKING )
COMP( 199?, opti495xlc,ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>", "386 motherboards using a OPTi 82C495XLC chipset", MACHINE_NOT_WORKING )
COMP( 199?, op386wb,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "OPTi", "OPTi 386WB VER.1.0", MACHINE_NOT_WORKING )
COMP( 199?, p386dx40,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Peacock", "P386DX-40", MACHINE_NOT_WORKING )
COMP( 1989, pc2386,    ibm5170, 0,       at386l,    0,     at386_state,     init_at386,        "Amstrad plc", "PC2386", MACHINE_NOT_WORKING )
COMP( 198?, pc60iii,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Commodore Business Machines",  "PC 60-III", MACHINE_NOT_WORKING )
COMP( 199?, pccm317,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "PC-Chips", "M317", MACHINE_NOT_WORKING )
COMP( 199?, pccm321,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "PC-Chips", "M321", MACHINE_NOT_WORKING )
COMP( 199?, pccm326,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "PC-Chips", "M326", MACHINE_NOT_WORKING )
COMP( 198?, pem2530,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "DTK", "PEM 2539", MACHINE_NOT_WORKING )
COMP( 199?, pg750,     ibm5170, 0,       pg750,     0,     at386_state,     init_at386,        "Siemens", "PG 750", MACHINE_NOT_WORKING )
COMP( 199?, pt581392,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "386 motherboard using the Forex FRX46C402 + FRX46C411 + SiS 85C206 chipset", MACHINE_NOT_WORKING )
COMP( 199?, qdu386dx,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>", "QD-U386DX VER 1.0", MACHINE_NOT_WORKING )
COMP( 1988, qi600,     ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Apricot",     "Apricot Qi 600 (Neptune Motherboard)", MACHINE_NOT_WORKING )
COMP( 199?, sisrabb,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "386 motherboards using the SiS Rabbit chipset", MACHINE_NOT_WORKING )
COMP( 199?, sm38640f,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "SM 386-40F", MACHINE_NOT_WORKING )
COMP( 19??, smih0107,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Forex Computer Company", "unknown 386 AT clone with Forex chipset", MACHINE_NOT_WORKING )
COMP( 199?, sy012,     ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "SY-012 16/25 386MB VER: 5.2", MACHINE_NOT_WORKING )
COMP( 199?, sy019hi,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Soyo", "SY-019H and SY-019I", MACHINE_NOT_WORKING )
COMP( 199?, sybaby386, ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Soyo", "Baby AT 386", MACHINE_NOT_WORKING )
COMP( 199?, tam25p2,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "AUVA", "TAM/25-P2 M31720P", MACHINE_NOT_WORKING )
COMP( 199?, tam3340ma0,ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "TAM/33/40-MA0", MACHINE_NOT_WORKING )
COMP( 199?, um82c481af,ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "386 motherboards using the UMC UM82C481AF chipset", MACHINE_NOT_WORKING )
COMP( 199?, um82c491f, ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "386 motherboard using the UMC UM82C491F chipset", MACHINE_NOT_WORKING )
COMP( 199?, um82c493f, ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "<unknown>",   "386 motherboards using the UMC UM82C491F + UM82C493F chipset or BIOTEQ equivalents", MACHINE_NOT_WORKING )
COMP( 199?, uni386w,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "UNICHIP", "386W 367C REV 1.0", MACHINE_NOT_WORKING )
COMP( 1992, walk386dx, ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Triumph-Adler", "Walkstation 386DX", MACHINE_NOT_WORKING ) // screen remains blank
COMP( 1988, xb42663,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Apricot",     "Apricot Qi 300 (Rev D,E & F Motherboard)", MACHINE_NOT_WORKING )
COMP( 1989, xb42664,   ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Apricot",     "Apricot XEN-S (Venus I Motherboard 386)" , MACHINE_NOT_WORKING )
COMP( 1990, xb42664a,  ibm5170, 0,       at386,     0,     at386_state,     init_at386,        "Apricot",     "Apricot XEN-S (Venus II Motherboard 386)" , MACHINE_NOT_WORKING )
