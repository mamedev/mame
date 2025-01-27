// license:BSD-3-Clause
// copyright-holders:AJR

/*

Quick & Crash by Namco (1999)

Hardware notes:
Main PCB has both 'Namco TY294-V-0 1331960101' and 'Tamura DNP0674A'
- KL5C80A12CFP
- Oki M9810
- NEC D71055C-10
- 93C66N
- 6264BLP10L
- 20.000 MHz XTAL
- lots of connectors

Namco H-5 CPU PCB (8830970101 - 8830960101)
- TMPZ84C015BF-12
- D24OP8I XTAL
- Epson SED1351F0A
- 2x TC55257DFL-70L

Cool Gunman is thought to run on similar hardware, it probably belongs here
*/

#include "emu.h"

#include "cpu/z80/kl5c80a12.h"
#include "cpu/z80/tmpz84c015.h"
#include "machine/eepromser.h"
#include "machine/i8255.h"
#include "sound/okim9810.h"

#include "speaker.h"


namespace {

class qncrash_state : public driver_device
{
public:
	qncrash_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void qncrash(machine_config &config) ATTR_COLD;

private:
	required_device<kl5c80a12_device> m_maincpu;

	void main_program_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	void dot_program_map(address_map &map) ATTR_COLD;
	void dot_io_map(address_map &map) ATTR_COLD;
};


void qncrash_state::main_program_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("maincpu", 0x0000);
	map(0xff800, 0xffdff).ram(); // 0xffe00 - 0xfffff is internal RAM
}

void qncrash_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).lw8(NAME([this] (u8 data) { logerror("%s: port40 out %02X\n", machine().describe_context(), data); }));
	//map(0x50, 0x50).rw("oki", FUNC(okim9810_device::read), FUNC(okim9810_device::write_command_or_tmp_register));
	map(0x58, 0x5b).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void qncrash_state::dot_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("dotcpu", 0x0000);
	map(0x8000, 0xffff).ram();
}

void qncrash_state::dot_io_map(address_map &map)
{
	map.global_mask(0xff);
	//map(0xc0, 0xcf).m("lcdc", FUNC(sed1351_device::register_map));
}


static INPUT_PORTS_START( qncrash ) // no DIPs on PCB
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) // configured as output
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void qncrash_state::qncrash(machine_config &config)
{
	// basic machine hardware
	KL5C80A12(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &qncrash_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &qncrash_state::main_io_map);
	m_maincpu->in_p0_callback().set_ioport("IN0");
	m_maincpu->out_p0_callback().set([this] (u8 data) { logerror("%s: p0 out %02X\n", machine().describe_context(), data); });
	m_maincpu->out_p1_callback().set("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(0);
	m_maincpu->out_p1_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(1);
	m_maincpu->out_p1_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(2);
	m_maincpu->out_p2_callback().set([this] (u8 data) { logerror("%s: p2 out %02X\n", machine().describe_context(), data); });
	m_maincpu->out_p3_callback().set([this] (u8 data) { logerror("%s: p3 out %02X\n", machine().describe_context(), data); });
	m_maincpu->out_p4_callback().set([this] (u8 data) { logerror("%s: p4 out %02X\n", machine().describe_context(), data); });

	tmpz84c015_device &dotcpu(TMPZ84C015(config, "dotcpu", 24'000'000 / 2)); // unknown D24OP8I XTAL, but TMPZ84C015BF-12 is rated for max 12 MHz, so guessed
	dotcpu.set_addrmap(AS_PROGRAM, &qncrash_state::dot_program_map);
	dotcpu.set_addrmap(AS_IO, &qncrash_state::dot_io_map);

	i8255_device &ppi(I8255(config, "ppi")); // NEC D71055C
	ppi.in_pa_callback().set_ioport("IN1");
	ppi.in_pb_callback().set_ioport("IN2");
	ppi.in_pc_callback().set_ioport("IN3");

	EEPROM_93C66_16BIT(config, "eeprom");

	// video hardware
	// TODO:
	// - a 96x16 dot matrix LED display
	// - a 5 digit 7 segment display for time

	// sound hardware
	SPEAKER(config, "gun_speaker").front_center();
	SPEAKER(config, "target_speaker").front_center();

	okim9810_device &oki(OKIM9810(config, "oki", 4'096'000)); // no evident XTAL on PCB
	// TODO: May need to be swapped. The announcer should come from gun_speaker
	oki.add_route(0, "gun_speaker", 1.00);
	oki.add_route(1, "target_speaker", 1.00);
}


ROM_START( qncrash )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qc1-mpr0c.ic3", 0x00000, 0x10000, CRC(8e3f605d) SHA1(69a0da6286e250b92e47b66c9423bc5c793b350b) )

	ROM_REGION( 0x20000, "dotcpu", 0 )
	ROM_LOAD( "qc1-dot0.ic7", 0x00000, 0x20000, CRC(45dda645) SHA1(27efcef0e3a09390eec0d7859465a13caf52f9df) )

	ROM_REGION( 0x800000, "oki", 0 )
	ROM_LOAD( "qc1-sound00.ic12", 0x000000, 0x400000, CRC(d72713d2) SHA1(556a0be2bb08fc9b4a2476b0ce8a23aa66858809) )
	ROM_LOAD( "qc1-sound01.ic13", 0x400000, 0x400000, CRC(70e472a1) SHA1(df06270cede1d00e2ec231276e5e5466ab549794) ) // 1xxxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "93c66n.ic5", 0x000, 0x200, CRC(1ba66a58) SHA1(adb7f1685cf81585ed30613adc39e9091e63af84) )

	ROM_REGION( 0x600, "dot_plds", ROMREGION_ERASE00 ) // all 18CV8P
	ROM_LOAD( "j4153.ic2", 0x000, 0x155, NO_DUMP )
	ROM_LOAD( "j4154.ic3", 0x200, 0x155, NO_DUMP )
	ROM_LOAD( "j4155.ic5", 0x400, 0x155, NO_DUMP )
ROM_END

ROM_START( qncrasha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qc3_mpr0.ic3", 0x00000, 0x10000, CRC(42c54dec) SHA1(0f6ca4bec7ae4f60b1943dad756933d02cd660c4) )

	ROM_REGION( 0x20000, "dotcpu", 0 )
	ROM_LOAD( "qc3_dot0.bin", 0x00000, 0x20000, CRC(97d8c117) SHA1(61a8b52c61abae8cbeccf6dd23e3ee0b4c2e443d) )

	ROM_REGION( 0x800000, "oki", 0 )
	ROM_LOAD( "qc1_snd0.ic12", 0x000000, 0x400000, CRC(d72713d2) SHA1(556a0be2bb08fc9b4a2476b0ce8a23aa66858809) )
	ROM_LOAD( "qc1_snd1.ic13", 0x400000, 0x400000, CRC(70e472a1) SHA1(df06270cede1d00e2ec231276e5e5466ab549794) ) // 1xxxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "93c66n.ic5", 0x000, 0x200, CRC(1ba66a58) SHA1(adb7f1685cf81585ed30613adc39e9091e63af84) )

	ROM_REGION( 0x600, "dot_plds", ROMREGION_ERASE00 ) // all 18CV8P
	ROM_LOAD( "j4153.ic2", 0x000, 0x155, NO_DUMP )
	ROM_LOAD( "j4154.ic3", 0x200, 0x155, NO_DUMP )
	ROM_LOAD( "j4155.ic5", 0x400, 0x155, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1999, qncrash,  0,       qncrash, qncrash, qncrash_state, empty_init, ROT0, "Namco", "Quick & Crash (Japan, V2.200)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK ) // version listed at 0xa97a in program ROM
GAME( 1999, qncrasha, qncrash, qncrash, qncrash, qncrash_state, empty_init, ROT0, "Namco", "Quick & Crash (US)",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
