// license:BSD-3-Clause
// copyright-holders:Carl
// Commodore PC 10 / PC 20 / PC 30
/***************************************************************************
Commodore PC10 / PC20 / PC30
Links: http://www.zimmers.net/cbmpics/cpcs.html , https://de.wikipedia.org/wiki/Commodore_PC-10_bis_PC-60 , http://mingos-commodorepage.tumblr.com/post/123656301482/commodore-pc-20-beim-pc-20-handelt-es-sich-um
http://www.richardlagendijk.nl/cip/computer/item/pc20ii/de
Form Factor: Desktop
CPU: 8088 @ 4.77 MHz
RAM: 256K / 512K / 640K
BUS: 5x ISA
Video: MDA
Mass storage: PC10: 1 or 2x 5.25" 360K , PC20: 1x 360K + 10MB HD, PC30: 1x 360K + 20MB HD
On board ports: Floppy, serial, parallel, speaker
Options: 8087 FPU


Commodore PC-10 III
=============
Links: http://dostalgie.de/downloads/pc10III-20III/PC10III_OM_COMMODORE_EN_DE.pdf ; ftp://ftp.zimmers.net/pub/cbm-pc/documents/PC-8088-Information.txt
Info: PC10-III and PC20-III are the same machines - PC10 has two floppies, PC20 one floppy and one harddisk
Form Factor: Desktop
CPU: 8088 @ 4.77 MHz / 7.16 MHz / 9.54 MHz
RAM: 640K
Bus: 3x ISA
Video: On board: MDA/CGA/Hercules/Plantronics
Mass storage: 1x Floppy 5.25" 360K and (PC10) another 360K or (PC20) 3.5" harddisk
On board ports: Floppy, XTA(8-bit IDE) Harddisk, Mouse, serial, parallel, RTC, Speaker
Options: 8087 FPU
***************************************************************************/

#include "emu.h"

#include "cpu/i86/i86.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "machine/genpc.h"
#include "machine/nvram.h"
#include "softlist_dev.h"


namespace {

class compc_state : public driver_device
{
public:
	compc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_dsw0(*this, "DSW0")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pc_noppi_mb_device> m_mb;
	required_ioport m_dsw0;

	void machine_reset() override ATTR_COLD;

	u8 pioiii_portc_r();
	void pioiii_portc_w(u8 data);
	u8 pio_portc_r();

	void compc(machine_config &config);
	void pc10iii(machine_config &config);
	void compc1(machine_config &config);
	void compc_io(address_map &map) ATTR_COLD;
	void compc_map(address_map &map) ATTR_COLD;
	void compciii_io(address_map &map) ATTR_COLD;
private:
	u8 m_dips = 0;
};

void compc_state::machine_reset()
{
	m_dips = 0;
}


u8 compc_state::pio_portc_r()
{
	u8 data;
	if(BIT(m_mb->pc_ppi_portb_r(), 3))
	{
		/* read hi nibble of S2 */
		data = (m_dsw0->read() >> 4) & 0x0f;
	}
	else
	{
		/* read lo nibble of S2 */
		data = m_dsw0->read() & 0x0f;
	}
	if(m_mb->pit_out2())
		data |= 0x20;
	return data;
}

void compc_state::pioiii_portc_w(u8 data)
{
	m_dips = (data & ~0x30) | m_dsw0->read();
}


u8 compc_state::pioiii_portc_r()
{
	u8 data;
	if(!BIT(m_mb->pc_ppi_portb_r(), 2))
		data = (m_dips >> 4) & 0x0f;
	else
		data = m_dips & 0x0f;
	if(m_mb->pit_out2())
		data |= 0x30;
	return data;
}

static INPUT_PORTS_START(compciii)
	PORT_START("DSW0") /* IN1 */
	PORT_DIPNAME( 0x30, 0x30, "Graphics adapter")
	PORT_DIPSETTING(    0x00, "EGA/VGA" )
	PORT_DIPSETTING(    0x10, "Color 40x25" )
	PORT_DIPSETTING(    0x20, "Color 80x25" )
	PORT_DIPSETTING(    0x30, "Monochrome" )
INPUT_PORTS_END

static INPUT_PORTS_START(compc)
	PORT_START("DSW0") /* IN1 */
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPNAME( 0x30, 0x30, "Graphics adapter")
	PORT_DIPSETTING(    0x00, "EGA/VGA" )
	PORT_DIPSETTING(    0x10, "Color 40x25" )
	PORT_DIPSETTING(    0x20, "Color 80x25" )
	PORT_DIPSETTING(    0x30, "Monochrome" )
	PORT_DIPNAME( 0x0c, 0x0c, "RAM banks")
	PORT_DIPSETTING(    0x00, "1 - 16/ 64/256K" )
	PORT_DIPSETTING(    0x04, "2 - 32/128/512K" )
	PORT_DIPSETTING(    0x08, "3 - 48/192/576K" )
	PORT_DIPSETTING(    0x0c, "4 - 64/256/640K" )
	PORT_DIPNAME( 0x02, 0x00, "8087 installed")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x02, DEF_STR(Yes) )
	PORT_DIPNAME( 0x01, 0x01, "Boot from floppy")
	PORT_DIPSETTING(    0x01, DEF_STR(Yes) )
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
INPUT_PORTS_END

void compc_state::compc_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void compc_state::compc_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(pc_noppi_mb_device::map));
	map(0x0062, 0x0062).r(FUNC(compc_state::pio_portc_r));
}

void compc_state::compciii_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(pc_noppi_mb_device::map));
	map(0x0062, 0x0062).rw(FUNC(compc_state::pioiii_portc_r), FUNC(compc_state::pioiii_portc_w));
}

void compc_state::compc(machine_config &config)
{
	I8088(config, m_maincpu, 4772720*2);
	m_maincpu->set_addrmap(AS_PROGRAM, &compc_state::compc_map);
	m_maincpu->set_addrmap(AS_IO, &compc_state::compc_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	PCNOPPI_MOTHERBOARD(config, m_mb, 0).set_cputag(m_maincpu);
	m_mb->int_callback().set_inputline(m_maincpu, 0);
	m_mb->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_mb->kbdclk_callback().set("keyboard", FUNC(pc_kbdc_device::clock_write_from_mb));
	m_mb->kbddata_callback().set("keyboard", FUNC(pc_kbdc_device::data_write_from_mb));
	config.device_remove("mb:pit8253");
	fe2010_pit_device &pit(FE2010_PIT(config, "mb:pit8253", 0));
	pit.set_clk<0>(XTAL(14'318'181)/12.0); /* heartbeat IRQ */
	pit.out_handler<0>().set("mb:pic8259", FUNC(pic8259_device::ir0_w));
	pit.set_clk<1>(XTAL(14'318'181)/12.0); /* dram refresh */
	pit.out_handler<1>().set(m_mb, FUNC(ibm5160_mb_device::pc_pit8253_out1_changed));
	pit.set_clk<2>(XTAL(14'318'181)/12.0); /* pio port c pin 4, and speaker polling enough */
	pit.out_handler<2>().set(m_mb, FUNC(ibm5160_mb_device::pc_pit8253_out2_changed));

	// FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa1", 0, "mb:isa", pc_isa8_cards, "mda", false);
	ISA8_SLOT(config, "isa2", 0, "mb:isa", pc_isa8_cards, "lpt", false);
	ISA8_SLOT(config, "isa3", 0, "mb:isa", pc_isa8_cards, "com", false);
	ISA8_SLOT(config, "isa4", 0, "mb:isa", pc_isa8_cards, "fdc_xt", false);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "keyboard", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(pc_noppi_mb_device::keyboard_clock_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(pc_noppi_mb_device::keyboard_data_w));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("256K").set_extra_options("512K, 640K");

	/* software lists */
	SOFTWARE_LIST(config, "disk_list").set_original("ibm5150");
}

void compc_state::pc10iii(machine_config &config)
{
	compc(config);
	m_maincpu->set_addrmap(AS_IO, &compc_state::compciii_io);
}

ROM_START(compc10)
	ROM_REGION(0x10000, "bios", 0)
	ROM_DEFAULT_BIOS("v205")
	ROM_SYSTEM_BIOS(0, "v201", "v2.01")
	ROMX_LOAD("bios2.01-380258-01.bin", 0xc000, 0x4000, CRC(921de6aa) SHA1(eb6c3fe4200cb40da20131b264521ba9f82021b2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v203", "v2.03")
	ROMX_LOAD("380258-03", 0xc000, 0x4000, CRC(fbe53865) SHA1(a6d6433c055d1c328f71403a2ed2fd5908c23d40), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v205", "v2.05")
	ROMX_LOAD("380258-04", 0xc000, 0x4000, CRC(e61084da) SHA1(dfb360a6ec6cb1250d8a6243f12a0d702e8479cb), ROM_BIOS(2))
ROM_END

// Note: Commodore PC20-III, PC10-III and COLT share the same BIOS
ROM_START(pc10iii)
	ROM_DEFAULT_BIOS("v441")
	ROM_SYSTEM_BIOS(0, "v435", "v4.35")
	ROM_SYSTEM_BIOS(1, "v435c", "v4.35c")
	ROM_SYSTEM_BIOS(2, "v436", "v4.36")
	ROM_SYSTEM_BIOS(3, "v436b", "v4.36b")
	ROM_SYSTEM_BIOS(4, "v436c", "v4.36c")
	ROM_SYSTEM_BIOS(5, "v438", "v4.38")
	ROM_SYSTEM_BIOS(6, "v439", "v4.39")
	ROM_SYSTEM_BIOS(7, "v440", "v4.40")
	ROM_SYSTEM_BIOS(8, "v441", "v4.41")

	ROM_REGION(0x10000, "bios", 0)
	ROMX_LOAD("318085-01.u201", 0x8000, 0x8000, CRC(be752d1e) SHA1(5e5e63cd6d6269816cd691602e4c4d209fe3df67), ROM_BIOS(0))
	ROMX_LOAD("318085-01_pc-3_bios_4.35c.bin", 0x8000, 0x8000, CRC(adbc4ab7) SHA1(c157e5bc115906705849f185e4fe8e42ab28930c), ROM_BIOS(1))
	ROMX_LOAD("318085-02.u201", 0x8000, 0x8000, CRC(db0c9d04) SHA1(1314ce606840f4f78e58e5f78909e8971387f387), ROM_BIOS(2))
	ROMX_LOAD("318085-03.u201", 0x8000, 0x8000, CRC(559e6b76) SHA1(cdfd4781f3520db7f5111469ebb29c10b39ab587), ROM_BIOS(3))
	ROMX_LOAD("318085-04.u201", 0x8000, 0x8000, CRC(f81e67f0) SHA1(b46613cb5c6ac4beb769778bc35f81777ebe02e1), ROM_BIOS(4))
	ROMX_LOAD("318085-05.u201", 0x8000, 0x8000, CRC(ae9e6a31) SHA1(853ee251cf230818c407a8d13ef060a21c90a8c1), ROM_BIOS(5))
	ROMX_LOAD("318085-06.u201", 0x8000, 0x8000, CRC(1901993c) SHA1(f75060c1c442376bd42c61e74fa9eee053351791), ROM_BIOS(6))
	ROMX_LOAD("318085-07.u201", 0x8000, 0x8000, CRC(505d52b0) SHA1(f717c6ab791d51f35e1c38ffbc81a44075b5f2f8), ROM_BIOS(7))
	ROMX_LOAD("318085-08.u201", 0x8000, 0x8000, CRC(7e228dc8) SHA1(958dfdd637bd31c01b949fac729d6973a7e630bc), ROM_BIOS(8))

	ROM_REGION(0x8000, "gfx1", 0)
	ROMX_LOAD("318086-01_p10c_164a.bin", 0x0000, 0x4000, CRC(ee6c27f0) SHA1(e769cc3a49a1d708bd74eb4ac85bb6ea67220d38), ROM_BIOS(0)) // came with ROM 4.35c
	ROMX_LOAD("318086-01_p10c_164a.bin", 0x0000, 0x4000, CRC(ee6c27f0) SHA1(e769cc3a49a1d708bd74eb4ac85bb6ea67220d38), ROM_BIOS(1))
	ROMX_LOAD("318086-02.u607", 0x0000, 0x8000, CRC(b406651c) SHA1(856f58353391a74a06ebb8ec9f8333d7d69e5fd6), ROM_BIOS(2))
	ROMX_LOAD("318086-02.u607", 0x0000, 0x8000, CRC(b406651c) SHA1(856f58353391a74a06ebb8ec9f8333d7d69e5fd6), ROM_BIOS(3))
	ROMX_LOAD("318086-02.u607", 0x0000, 0x8000, CRC(b406651c) SHA1(856f58353391a74a06ebb8ec9f8333d7d69e5fd6), ROM_BIOS(4))
	ROMX_LOAD("318086-02.u607", 0x0000, 0x8000, CRC(b406651c) SHA1(856f58353391a74a06ebb8ec9f8333d7d69e5fd6), ROM_BIOS(5))
	ROMX_LOAD("318086-02.u607", 0x0000, 0x8000, CRC(b406651c) SHA1(856f58353391a74a06ebb8ec9f8333d7d69e5fd6), ROM_BIOS(6))
	ROMX_LOAD("318086-02.u607", 0x0000, 0x8000, CRC(b406651c) SHA1(856f58353391a74a06ebb8ec9f8333d7d69e5fd6), ROM_BIOS(7))
	ROMX_LOAD("318086-02.u607", 0x0000, 0x8000, CRC(b406651c) SHA1(856f58353391a74a06ebb8ec9f8333d7d69e5fd6), ROM_BIOS(8))
ROM_END


/*********************************************************** Commodore PC-1 ***

Links: http://www.amiga-stuff.com/hardware/pc-i.html , http://www.zimmers.net/cbmpics/cpci.html
Form Factor: Desktop
CPU: 8088 @ 4.77 MHz
RAM: 512K / 640K
Bus: Proprietary expansion slot, carrying almost all ISA signals
Video: On board, MDA/Hercules/CGA
Mass storage: 1x 5.25" 360K
On board ports: Floppy, floppy expansion (for Amiga A1010/1011 (720 KB, 3.5") or A1020 (360 KB, 5.25" drives), speaker (but no speaker fitted), mouse,
Options: 8087 FPU
Expansion: Expansion box: 2x ISA

******************************************************************************/

ROM_START( compc1 )
	ROM_DEFAULT_BIOS("bios12")
	ROM_REGION(0x10000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "bios11", "PC-1 BIOS Rev. 1.1")
	ROMX_LOAD("pc1_bios.bin", 0xc000, 0x4000, CRC(e37367c8) SHA1(9aac9c38b4ebdb9a740e393199c2eff75a0bde03), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "bios12", "PC-1 BIOS Rev. 1.2")
	ROMX_LOAD("cbm-pci-bios-v1.2-380270-02.bin", 0xc000, 0x4000, CRC(7f744f87) SHA1(07f94a7e8ca4ddd1c738b304d24358711b4cd2ca), ROM_BIOS(1))
	ROM_REGION(0x8000, "gfx1", 0)
	ROM_LOAD("pc1_char.bin", 0x0000, 0x4000, CRC(ee6c27f0) SHA1(e769cc3a49a1d708bd74eb4ac85bb6ea67220d38))
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT     CLASS        INIT        COMPANY                        FULLNAME               FLAGS
COMP( 1984, compc10, ibm5150, 0,      compc,   compc,    compc_state, empty_init, "Commodore Business Machines", "Commodore PC 10",     MACHINE_NOT_WORKING )
COMP( 1987, pc10iii, ibm5150, 0,      pc10iii, compciii, compc_state, empty_init, "Commodore Business Machines", "Commodore PC-10 III", MACHINE_NOT_WORKING )
COMP( 198?, compc1,  ibm5150, 0,      pc10iii, compciii, compc_state, empty_init, "Commodore Business Machines", "PC-1",                MACHINE_NOT_WORKING )
