// license:BSD-3-Clause
// copyright-holders:

/*
  Bootleg PCB with 2 sub PCBs.

  Main PCB components:
  1x Z80 (timer CPU?)
  1x ROM
  1x RP2A03E (NTSC NES main CPU)
  1x RP2C02E-0 (NES PPU)
  3x GM76C28-10 RAMs
  1x 8-dip bank
  various TTL

  ROM PCB components:
  6x ROMs
  1x UM6264 RAM
  1x PAL
  various TTL

  NTSC PCB components:
  1x NEC uPC1352C chrominance and luminance processor for NTSC color TV
  1x 3'579'545 XTAL

  The Z80 ROM is really strange:
  * 1st half is 0xff filled;
  * 2nd half contains 8x 0x800 programs
  * 6 of them are identical
  * 2 others (0x4000 - 0x47ff and 0x6000 - 0x67ff) differ but are identical between themselves
*/


#include "emu.h"

#include "cpu/m6502/rp2a03.h"
#include "cpu/z80/z80.h"
#include "video/ppu2c0x.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class nes_arcade_bl_state : public driver_device
{
public:
	nes_arcade_bl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppu(*this, "ppu")
	{ }

	void smb3bl(machine_config &config);

private:
	required_device<rp2a03_device> m_maincpu;
	required_device<ppu2c0x_device> m_ppu;

	void nes_cpu_map(address_map &map);
	void timer_prg_map(address_map &map);
	void timer_io_map(address_map &map);
};


void nes_arcade_bl_state::nes_cpu_map(address_map &map)
{
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

void nes_arcade_bl_state::timer_prg_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x8000, 0x87ff).ram();
}

void nes_arcade_bl_state::timer_io_map(address_map &map)
{
}


static INPUT_PORTS_START( smb3bl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END


void nes_arcade_bl_state::smb3bl(machine_config &config)
{
	RP2A03G(config, m_maincpu, 3.579545_MHz_XTAL / 2); // TODO: verify divider, really RP2A03E
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_arcade_bl_state::nes_cpu_map);

	z80_device &timercpu(Z80(config, "timercpu", 3.579545_MHz_XTAL));
	timercpu.set_addrmap(AS_PROGRAM, &nes_arcade_bl_state::timer_prg_map);
	timercpu.set_addrmap(AS_IO, &nes_arcade_bl_state::timer_io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(32*8, 262);
	screen.set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	screen.set_screen_update("ppu", FUNC(ppu2c0x_device::screen_update));

	PPU_2C02(config, m_ppu);
	m_ppu->set_cpu_tag("maincpu");
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	SPEAKER(config, "mono").front_center();
	m_maincpu->add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( smb3bl )
	ROM_REGION( 0x40000, "maincpu", 0 ) // extremely similar to smb3h in nes.xml once split
	ROM_LOAD( "mario_3-1.bin", 0x00000, 0x10000, CRC(d28e54ee) SHA1(25b842aa091dd3e497b1ff34ef8cf4cda5025dc5) ) // 5601.rom-2 [1/2]      IDENTICAL
	ROM_LOAD( "mario_3-2.bin", 0x10000, 0x10000, CRC(1ed05c8d) SHA1(f78cfc827b5cde86f2aa4f9a9df82b923835a7a6) ) // 5601.rom-2 [2/2]      IDENTICAL
	ROM_LOAD( "mario_3-3.bin", 0x20000, 0x10000, CRC(0b2f4356) SHA1(bffb329b9d7387ededf779cf40c84906fc26cf05) ) // 5602.rom-1 [1/2]      IDENTICAL
	ROM_LOAD( "mario_3-4.bin", 0x30000, 0x10000, CRC(2abda7cc) SHA1(6e6c9b1f28a0d6eb7dc43dfeca1da458b7ddb89e) ) // 5602.rom-1 [2/2]      99.929810% (changes seem legit / not a result of a bad dump)

	ROM_REGION( 0x20000, "gfx", 0 ) // matches smb3j in nes.xml once split
	ROM_LOAD( "mario_3-5.bin", 0x00000, 0x10000, CRC(48d6ddce) SHA1(686793fcb9c3ba9d7280b40c9afdbd75860a290a) ) // hvc-um-0 chr [1/2]      IDENTICAL
	ROM_LOAD( "mario_3-6.bin", 0x10000, 0x10000, CRC(a88664e0) SHA1(327d246f198713f20adc7764ee539d18eb0b82ad) ) // hvc-um-0 chr [2/2]      IDENTICAL

	ROM_REGION( 0x8000, "timercpu", 0 )
	ROM_LOAD( "nes_jamma_base.bin", 0x0000, 0x4000, CRC(ea276bdd) SHA1(1cd5916e9a6ea9e40526a4fe55b846ca1818fd5f) ) // BADADDR x-xxxxxxxxxxxxx
	ROM_CONTINUE(                   0x0000, 0x4000 )
ROM_END

} // anonymous namespace


GAME( 1987, smb3bl, 0, smb3bl, smb3bl, nes_arcade_bl_state, empty_init, ROT0, "Sang Ho Soft", "Super Mario Bros. 3 (NES bootleg)", MACHINE_IS_SKELETON ) // 1987.10.01 in Z80 ROM
