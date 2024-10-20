// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    NEC PC-H[yper] 98

    TODO:
    - NESA bus in place of C-Bus plus a billion of overrides from the base PC-98 ...
    - needs a specific chargen "FONT24.ROM" for anything that isn't a H98S;

**************************************************************************************************/

#include "emu.h"
#include "pc9801.h"

class pc_hyper98_state : public pc9801bx_state
{
public:
	pc_hyper98_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc9801bx_state(mconfig, type, tag)
	{}

	void pc_h98s(machine_config &config);

protected:
	DECLARE_MACHINE_START(pc_h98);
	DECLARE_MACHINE_RESET(pc_h98);

	void pc_h98_map(address_map &map) ATTR_COLD;
	void pc_h98_io(address_map &map) ATTR_COLD;
};

void pc_hyper98_state::pc_h98_map(address_map &map)
{
	pc_hyper98_state::pc9801bx2_map(map);
//  map(0x080000, 0x0bffff).unmaprw(); // RAM window
	// TODO: bigger, needs fn mods
	map(0x0c0000, 0x0dffff).rw(FUNC(pc_hyper98_state::grcg_gvram0_r), FUNC(pc_hyper98_state::grcg_gvram0_w));
	map(0x0e0000, 0x0e3fff).rw(FUNC(pc_hyper98_state::tvram_r), FUNC(pc_hyper98_state::tvram_w));
	map(0x0e4000, 0x0e4fff).rw(FUNC(pc_hyper98_state::pc9801rs_knjram_r), FUNC(pc_hyper98_state::pc9801rs_knjram_w));
}

void pc_hyper98_state::pc_h98_io(address_map &map)
{
	pc_hyper98_state::pc9801bx2_io(map);
	// ...
}

// TODO: backported from pc9801_epson.cpp, needs mods
static INPUT_PORTS_START( pc_h98 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Monitor Type" )
	PORT_DIPSETTING(    0x00, "Normal Display (15KHz)" )
	PORT_DIPSETTING(    0x01, "Hi-Res Display (24KHz)" )
	PORT_DIPNAME( 0x02, 0x00, "DSW1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Display Type" )
	PORT_DIPSETTING(    0x04, "RGB" )
	PORT_DIPSETTING(    0x00, "Plasma" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Graphic Function" )
	PORT_DIPSETTING(    0x80, "Basic (8 Colors)" )
	PORT_DIPSETTING(    0x00, "Expanded (16/4096 Colors)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "GDC clock" )
	PORT_DIPSETTING(    0x80, "2.5 MHz" )
	PORT_DIPSETTING(    0x00, "5 MHz" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "FDD Fix Mode" )
	PORT_DIPSETTING(    0x00, "Auto-Detection" )
	PORT_DIPSETTING(    0x01, "Fixed" )
	PORT_DIPNAME( 0x02, 0x02, "FDD Density Select" )
	PORT_DIPSETTING(    0x00, "2DD" )
	PORT_DIPSETTING(    0x02, "2HD" )
	PORT_DIPNAME( 0x04, 0x04, "DSW3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Conventional RAM size" )
	PORT_DIPSETTING(    0x40, "640 KB" )
	PORT_DIPSETTING(    0x00, "512 KB" )
	PORT_DIPNAME( 0x80, 0x00, "CPU Type" )
	PORT_DIPSETTING(    0x80, "V30" )
	PORT_DIPSETTING(    0x00, "I386" )

	PORT_START("MOUSE_X")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30)

	PORT_START("MOUSE_Y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30)

	PORT_START("MOUSE_B")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Right Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(MOUSECODE_BUTTON3) PORT_NAME("Mouse Middle Button")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Left Button")

	PORT_START("ROM_LOAD")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x04, 0x04, "Load IDE BIOS" )
	PORT_CONFSETTING(    0x00, DEF_STR( Yes ) )
	PORT_CONFSETTING(    0x04, DEF_STR( No ) )
INPUT_PORTS_END

MACHINE_START_MEMBER(pc_hyper98_state,pc_h98)
{
	MACHINE_START_CALL_MEMBER(pc9801bx2);
}

MACHINE_RESET_MEMBER(pc_hyper98_state,pc_h98)
{
	MACHINE_RESET_CALL_MEMBER(pc9801bx2);

	// boots with DMA > 1MB on.
	m_dma_access_ctrl = 0xfb;
}

void pc_hyper98_state::pc_h98s(machine_config &config)
{
	pc9801bx2(config);
	const XTAL xtal = XTAL(20'000'000);
	I486(config.replace(), m_maincpu, xtal); // i486sx
	m_maincpu->set_addrmap(AS_PROGRAM, &pc_hyper98_state::pc_h98_map);
	m_maincpu->set_addrmap(AS_IO, &pc_hyper98_state::pc_h98_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	MCFG_MACHINE_START_OVERRIDE(pc_hyper98_state, pc_h98)
	MCFG_MACHINE_RESET_OVERRIDE(pc_hyper98_state, pc_h98)

	m_ram->set_default_size("14M");
	// TODO: extra options, 1.6MB up to 45.6MB
}

// Stolen from pc9821
#define LOAD_KANJI_ROMS \
	ROM_REGION( 0x80000, "raw_kanji", ROMREGION_ERASEFF ) \
	ROM_LOAD16_BYTE( "24256c-x01.bin", 0x00000, 0x4000, BAD_DUMP CRC(28ec1375) SHA1(9d8e98e703ce0f483df17c79f7e841c5c5cd1692) ) \
	ROM_CONTINUE(                      0x20000, 0x4000  ) \
	ROM_LOAD16_BYTE( "24256c-x02.bin", 0x00001, 0x4000, BAD_DUMP CRC(90985158) SHA1(78fb106131a3f4eb054e87e00fe4f41193416d65) ) \
	ROM_CONTINUE(                      0x20001, 0x4000  ) \
	ROM_LOAD16_BYTE( "24256c-x03.bin", 0x40000, 0x4000, BAD_DUMP CRC(d4893543) SHA1(eb8c1bee0f694e1e0c145a24152222d4e444e86f) ) \
	ROM_CONTINUE(                      0x60000, 0x4000  ) \
	ROM_LOAD16_BYTE( "24256c-x04.bin", 0x40001, 0x4000, BAD_DUMP CRC(5dec0fc2) SHA1(41000da14d0805ed0801b31eb60623552e50e41c) ) \
	ROM_CONTINUE(                      0x60001, 0x4000  ) \
	ROM_REGION( 0x100000, "kanji", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x80000, "new_chargen", ROMREGION_ERASEFF )

ROM_START( pc_h98s )
	ROM_REGION( 0x40000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD( "hcz7k_01.bin", 0x000000, 0x020000, CRC(e68834e8) SHA1(f57dfd67915715168e46907fd535277e30357742) )
	ROM_LOAD( "hcz8k_01.bin", 0x020000, 0x020000, CRC(39f82b02) SHA1(52e950f10faa0bedca3d6ea2ba6caceaeff66fc9) )

	// unconfirmed
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_COPY( "biosrom", 0x20000, 0x08000, 0x10000 )
	ROM_COPY( "biosrom", 0x00000, 0x18000, 0x18000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	// stolen from pc9821
	ROM_LOAD( "font.rom", 0x00000, 0x46800, BAD_DUMP CRC(a61c0649) SHA1(554b87377d176830d21bd03964dc71f8e98676b1) )

	LOAD_KANJI_ROMS
//  LOAD_IDE_ROM
ROM_END

COMP( 1991, pc_h98s, 0,   0, pc_h98s, pc_h98,   pc_hyper98_state, init_pc9801_kanji,   "NEC",   "PC-H98S model 8/U8", MACHINE_IS_SKELETON )
