// license:BSD-3-Clause
// copyright-holders:Angelo Salese,Carl
/**************************************************************************************************

    Epson PC98[01] class machine

    TODO (PC-286VS):
    - Verify A20 gate usage, seems to reuse the same hookup as later Epson variants;

    TODO (PC-386M):
    - Incomplete shadow IPL banking, we currently never bankswitch to the other ROM bank
      (which barely contains program code);
    - "ERR:VR" at POST (GFX VRAM)
      Sub-routine that throws this is at PC=0xfd9bc. Notice that you can actually skip this
      with eip=0x1bf in debugger and make the system to actually checkout memory installed.
      (Shorthand: "bp fd9bc,eip=0x1bf")
    - POST throws non-fatal "ERR:PA" (page fault, "Protected Address"?) after checking memory
      installed. Non-fatal as in POST will checkout bootable devices afterward.

    TODO: (PC-486SE/PC-486MU):
    - Verify ROM bankswitch;
      On PC-486SE sets up what is normally IPL bankswitch at PC=0xf5115, successive opcode
      is a jmp 0xf8000, pretty unlikely it delays bankswitch so assume it reloads
      the same bank.
    - Remove IDE regression hack at I/O $74e;
    - Regressed with a ERR:RA (conventional memory!?) when moving driver to
      stand-alone file;
    - Eventually errors with a ERR:VR (GFX VRAM);

    Notes:
    - A detailed list of Epson PC98s can be seen from here:
      http://www.pc-9800.net/db_epson/index.htm

    - Being these knockoffs means that there isn't 100% compatibility with all SWs.
      Additionally NEC introduced the so called "EPSON Protect" / "EPSON check" (エプソンチェック)
      starting with MS-DOS 3.3 onward, which checks the presence of NEC / Microsoft copyright
      string at E800:0DD8 and refuses to boot if not satisfied.
      cfr. https://github.com/joncampbell123/dosbox-x/issues/682
      Epson offcially provided PC "Software Installation Program" (SIP) floppy disks
      (the "epinstal*" in SW list?) that counteracts with the protection check.
      There's alternatively a freeware user released "Dispell!" program tool that can be used for
      the same purpose, which also works for 32-bit DOS/V machines.

**************************************************************************************************/

#include "emu.h"
#include "pc9801_epson.h"

template <unsigned which> void pc98_epson_state::shadow_ipl_w(offs_t offset, u16 data, u16 mem_mask)
{
	// TODO: shadow register 0x6a may actually be write deprotect
	COMBINE_DATA(&m_shadow_ipl[which][offset]);
}

/**************************************************************************************************

    Control port for Epson shadow IPL

    If any of these isn't right system throws "ERR:BR" at boot (BIOS loader error).
    Executes some code in text VRAM area (PC=$a006e), trying to setup a writeable RAM bank
    to IPL window area.

**************************************************************************************************/
void pc98_epson_state::epson_ipl_bank_w(offs_t offset, u8 data)
{
	m_shadow_ipl_bank = data;
	switch(m_shadow_ipl_bank)
	{
		case 0x2a:
			m_ipl->set_bank(2);
			break;
		case 0xe6:
			m_ipl->set_bank(0);
			break;
		default:
			// TODO: at least 0xa6 used, what for?
			logerror("%s: unknown Epson shadow IPL bank setting set %02x\n", machine().describe_context(), data);
			break;
	}
}

// overrides original PC98 $43c-$43f ports
void pc98_epson_state::epson_itf_bank_w(offs_t offset, u8 data)
{
	// $43f
	if (offset == 1)
	{
		switch(data)
		{
			case 0x40: m_itf_bank_enable = false; break;
			case 0x42: m_itf_bank_enable = true; break;
			default:
				logerror("%s: unknown ITF enable setting %02x\n", machine().describe_context(), data);
				break;
		}
	}

	// $43d
	if (offset == 0)
	{
		switch(data)
		{
			case 0x00:
			case 0x02:
				if (m_itf_bank_enable == true)
					m_ipl->set_bank((data & 2) >> 1);
				break;
			default:
				// TODO: 0x10 - 0x12 setting, which should be same as NEC PC98
				// (i.e. reversed compared to above)
				logerror("%s: unknown ITF bank setting %02x\n", machine().describe_context(), data);
				break;
		}
	}
}

void pc98_epson_state::epson_a20_w(offs_t offset, u8 data)
{
	// pc386m PC=0xfd9b3
	// if this isn't mapped then POST throws "ERR:RA" (conventional memory -> "Real Address"?)
	m_gate_a20 = data & 1;
	m_maincpu->set_input_line(INPUT_LINE_A20, m_gate_a20 ? ASSERT_LINE : CLEAR_LINE);
	logerror("%s: Epson gate a20 %02x\n", machine().describe_context(), data);
}

void pc98_epson_state::epson_vram_bank_w(offs_t offset, u8 data)
{
//  m_vram_bank = (data & 1) ^ 1;
	// accessed in the same routine that actually throws ERR:VR
	logerror("%s: Epson $c06 write %02x\n", machine().describe_context(), data);
}

void pc98_epson_state::pc286vs_map(address_map &map)
{
	pc9801ux_map(map);
	map(0x0e8000, 0x0fffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
	map(0xee8000, 0xefffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
	map(0xfe8000, 0xffffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
}

void pc98_epson_state::pc386m_map(address_map &map)
{
	pc9801rs_map(map);
	// TODO: is shadow RAM physically mapped here?
//  map(0xd0000, 0xd**ff).ram();
}

void pc98_epson_state::pc486se_map(address_map &map)
{
	pc386m_map(map);
	map(0x000e8000, 0x000fffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
	map(0xffee8000, 0xffefffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
	map(0xfffe8000, 0xffffffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
}


void pc98_epson_state::epson_base_io(address_map &map)
{
//  map(0x0c03, 0x0c03).r Epson CPU mode, 'R' for Real mode, 'P' for Protected mode (lolwut)
	map(0x0c05, 0x0c05).w(FUNC(pc98_epson_state::epson_a20_w));
	map(0x0c06, 0x0c06).w(FUNC(pc98_epson_state::epson_vram_bank_w));
	map(0x0c07, 0x0c07).w(FUNC(pc98_epson_state::epson_ipl_bank_w));
//  map(0x0c13, 0x0c13).r Epson <unknown> readback
//  map(0x0c14, 0x0c14).r Epson <unknown> readback
}

void pc98_epson_state::pc286vs_io(address_map &map)
{
	pc9801ux_io(map);
	epson_base_io(map);
}

void pc98_epson_state::pc386m_io(address_map &map)
{
	pc9801rs_io(map);
	epson_base_io(map);
}

void pc98_epson_state::pc486se_io(address_map &map)
{
	pc386m_io(map);

	map(0x0082, 0x0082).lr8(NAME([]() -> u8 { return 0x00; }));

	map(0x043c, 0x043f).w(FUNC(pc98_epson_state::epson_itf_bank_w)).umask16(0xff00);

	map(0x0c42, 0x0c43).lr16(NAME([]() -> u16 { return 0x0000; }));

//  map(0x0640, 0x064f).rw(FUNC(pc9801_state::ide_cs0_r), FUNC(pc9801_state::ide_cs0_w));
//  map(0x0740, 0x074f).rw(FUNC(pc9801_state::ide_cs1_r), FUNC(pc9801_state::ide_cs1_w));
	// HACK: avoid POST moaning for misconfigured HDDs (!?)
	map(0x074e, 0x074e).lr8(NAME([]() -> u8 { return 0xff; }));

	// (R) bit 0: expected to go off at PC=0xf8c52 (pc486se)
	// (W) commands for?
	map(0x0c09, 0x0c09).lr8(NAME([]() -> u8 { return 0x00; }));
	map(0x0c0b, 0x0c0b).lr8(NAME([]() -> u8 { return 0x00; }));
}

void pc98_epson_state::pc386m_ipl_bank(address_map &map)
{
	map(0x00000, 0x17fff).rom().region("ipl", 0x00000).w(FUNC(pc98_epson_state::shadow_ipl_w<0>));
	map(0x18000, 0x2ffff).rom().region("ipl", 0x18000).w(FUNC(pc98_epson_state::shadow_ipl_w<1>));
	map(0x30000, 0x47fff).ram().share("shadow_ipl_0");
	map(0x48000, 0x5ffff).ram().share("shadow_ipl_1");
}

static INPUT_PORTS_START( pc386m )
	// in tracer bullet fashion we intentionally separated all dips/mouse switches
	// since there's no real need to actually INPUT_PORTS_EXTERN here:
	// - mouse should really be a device interface anyway;
	// - dips may really diverge from NEC specs or even be unmapped (needs verification).
	//   In both cases those needs a major cleanup, haven't yet found actual documentation for these specifically.
	//   Notice that Epson eventually inherited SDIP chip as well.
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

MACHINE_START_MEMBER(pc98_epson_state, pc98_epson)
{
	MACHINE_START_CALL_MEMBER(pc9801rs);
}

MACHINE_RESET_MEMBER(pc98_epson_state, pc98_epson)
{
	MACHINE_RESET_CALL_MEMBER(pc9801rs);
	m_ipl->set_bank(0);
}

void pc98_epson_state::config_base_epson(machine_config &config)
{
	m_ipl->set_addrmap(AS_PROGRAM, &pc98_epson_state::pc386m_ipl_bank);
	// TODO: 19 or 20 address lines?
	// 20 may be used in case that mixed up ROM & shadow IPL loads are actually possible
	m_ipl->set_options(ENDIANNESS_LITTLE, 16, 19, 0x18000);

	MCFG_MACHINE_START_OVERRIDE(pc98_epson_state, pc98_epson)
	MCFG_MACHINE_RESET_OVERRIDE(pc98_epson_state, pc98_epson)
}

void pc98_epson_state::pc286vs(machine_config &config)
{
	pc9801vx(config);
	i80286_cpu_device &maincpu(I80286(config.replace(), m_maincpu, 10000000));
	maincpu.set_addrmap(AS_PROGRAM, &pc98_epson_state::pc286vs_map);
	maincpu.set_addrmap(AS_IO, &pc98_epson_state::pc286vs_io);
	maincpu.set_a20_callback(FUNC(pc98_epson_state::a20_286));
	maincpu.set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	config_base_epson(config);

	// TODO: DMA type & clock
}

void pc98_epson_state::pc286u(machine_config &config)
{
	pc9801vm(config);
	config_base_epson(config);

	// TODO: DMA type & clock
}


void pc98_epson_state::pc386m(machine_config &config)
{
	pc9801rs(config);
	I386SX(config.replace(), m_maincpu, 16000000); // i386SX 16MHz, switchable to 10/6 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &pc98_epson_state::pc386m_map);
	m_maincpu->set_addrmap(AS_IO, &pc98_epson_state::pc386m_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	config_base_epson(config);

	// RAM: 640KB + 14.6MB max
	// 2 3.5 floppy drives
	// ...
}

void pc98_epson_state::pc486se(machine_config &config)
{
	pc386m(config);
	const XTAL xtal = XTAL(25'000'000);
	I486(config.replace(), m_maincpu, xtal); // i486SX, switchable to 10/5 MHz, supports overdrive
	m_maincpu->set_addrmap(AS_PROGRAM, &pc98_epson_state::pc486se_map);
	m_maincpu->set_addrmap(AS_IO, &pc98_epson_state::pc486se_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	pit_clock_config(config, xtal/8); // unknown, passes "ERR:TM" test

	// RAM: 1.6 MB (!) + 17.6 max
	// "dedicated internal memory slot x 1"
	// "dedicated video board" slot
}

void pc98_epson_state::pc486mu(machine_config &config)
{
	pc386m(config);
	const XTAL xtal = XTAL(33'000'000);
	I486(config.replace(), m_maincpu, xtal); // i486SX, switchable to I386DX 10MHz/5MHz, Pentium ODP compatible
	m_maincpu->set_addrmap(AS_PROGRAM, &pc98_epson_state::pc486se_map);
	m_maincpu->set_addrmap(AS_IO, &pc98_epson_state::pc386m_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	pit_clock_config(config, xtal/8); // unknown, passes "ERR:TM" test

	// CL-GD5428
	// RAM: 5.6 + 61.6MB max
	// 2 x 3.5 floppy drives
}


// backported from pc98, of course both aren't 100% identical to the NEC counterpart
#define LOAD_IDE_ROM \
	ROM_REGION( 0x4000, "ide", ROMREGION_ERASEVAL(0xcb) ) \
	ROM_LOAD( "epson_ide_bios.rom", 0x0000, 0x2000, NO_DUMP ) \
	ROM_IGNORE( 0x2000 ) \
	ROM_IGNORE( 0x2000 ) \
	ROM_IGNORE( 0x2000 )

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

/*
Epson PC-286VS
i286 @ 10 (selectable between 6 and 10 MHz)
NB: pc-9801.net reports @ 16, assume mistake
640 KB conventional memory + 14.6 MB
5.25"2DD/2HDx2
CBus: 4slots
*/

ROM_START( pc286vs )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "a2_wvs.2e",    0x10000, 0x08000, CRC(318d6bbe) SHA1(f3ba85f3144e361257d6f7129e7f29fff17c2e1e) )
	ROM_CONTINUE(             0x00000, 0x10000 )
	ROM_CONTINUE(             0x28000, 0x08000 ) // bank 1, unconfirmed

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_286vs.rom", 0x0000, 0x46800, BAD_DUMP CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff))

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END


/*
Epson PC-286U
μPD70116 (V30) @ 10/8 MHz
640 KB conventional memory + 8.6 MB
3.5"2DD/2HDx2
CBus: 2 internal slots + 2 external slots

NOTE: was mislabeled as pc9801vm11, this is a best guess
(contains Epson strings in place of KBCRT identifier, at ipl address $23270)

*/

ROM_START( pc286u )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf.rom",     0x10000, 0x08000, NO_DUMP )
	ROM_LOAD( "bios_vm.rom", 0x18000, 0x18000, CRC(2e2d7cee) SHA1(159549f845dc70bf61955f9469d2281a0131b47f) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_vm.rom",     0x000000, 0x046800, BAD_DUMP CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff) )

	LOAD_KANJI_ROMS
//  LOAD_IDE_ROM
ROM_END


/*
Epson PC-386M

i386SX-16 @ 16
1MB
3.5"2DD/2HDx2
CBus: 3slots
*/

ROM_START( pc386m )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// bank 0: definitely wants this ROM mapping otherwise POST throws "ERR:R0" (BIOS checksum)
	ROM_LOAD( "cwma-a02.bin", 0x10000, 0x08000,  CRC(d2c357a4) SHA1(819c9a1fc92124a8d6a85339c74651add7efaf92) )
	ROM_CONTINUE(             0x00000, 0x10000 )
	ROM_CONTINUE(             0x28000, 0x08000 ) // bank 1, unconfirmed

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_486mu.rom", 0x0000, 0x46800, BAD_DUMP CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff))

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
Epson PC-486SE

i486SX @ 25 MHz
1.6 MB of conventional memory (???)
17.6 MB
CBus: 2slots
*/

ROM_START( pc486se )
	ROM_REGION16_LE( 0x20000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD( "1699ma_cw99-a03.bin", 0x00000, 0x20000,   CRC(f03df711) SHA1(88614746e01c7d3cff9f3b8ce0a598830a77d1dc) )

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// this is quite convoluted
	ROM_COPY( "biosrom", 0x08000, 0x00000, 0x08000 )
	ROM_COPY( "biosrom", 0x00000, 0x10000, 0x08000 )
	ROM_COPY( "biosrom", 0x10000, 0x08000, 0x08000 )
//  ROM_FILL(                     0x18000, 0x08000, 0x90) // untested by BIOS
	ROM_COPY( "biosrom", 0x10000, 0x20000, 0x08000 )
	ROM_COPY( "biosrom", 0x18000, 0x28000, 0x08000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_486mu.rom", 0x0000, 0x46800, BAD_DUMP CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff))

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
Epson PC-486MU
i486SX-33 @ 33
8MB RAM
3.5'2DD/2HDx2, 2xCD-ROM
CBus: 3 slots
*/

ROM_START( pc486mu )
	ROM_REGION16_LE( 0x20000, "biosrom", ROMREGION_ERASEFF )
	ROM_LOAD( "pc-486mu_hn27c1024.bin", 0x00000, 0x20000, CRC(113268e1) SHA1(2a630abc825b2808f9f8fb65c6cb1fb7e7f6c710))
//  ROM_LOAD( "bios_486mu.rom", 0x00000, 0x18000, BAD_DUMP CRC(57b5d701) SHA1(15029800842e93e07615b0fd91fb9f2bfe3e3c24))

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// backported from pc486se
	ROM_COPY( "biosrom", 0x08000, 0x00000, 0x08000 )
	ROM_COPY( "biosrom", 0x00000, 0x10000, 0x08000 )
	ROM_COPY( "biosrom", 0x10000, 0x08000, 0x08000 )
//  ROM_FILL(                     0x18000, 0x08000, 0x90) // untested by BIOS
	ROM_COPY( "biosrom", 0x10000, 0x20000, 0x08000 )
	ROM_COPY( "biosrom", 0x18000, 0x28000, 0x08000 )


	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_486mu.rom", 0x0000, 0x46800, BAD_DUMP CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff))

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

// Epson PC98 desktop line

// PC-286 (i286, first model released in Oct 1987)
COMP( 1989, pc286vs,     0,       0, pc286vs,    pc386m, pc98_epson_state, init_pc9801_kanji, "Epson", "PC-286VS", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// PC-286U (same as above except running on V30)
COMP( 1987, pc286u,     0,        0, pc286u,     pc386m, pc98_epson_state, init_pc9801_kanji, "Epson", "PC-286U", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // Revised BIOS 1988

// PC-286C "PC Club" (same as PC-286?)
// ...

// PC-386 (i386)
COMP( 1990, pc386m,     0,        0, pc386m,    pc386m, pc98_epson_state, init_pc9801_kanji, "Epson", "PC-386M",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// PC-486 (i486SX/DX)
COMP( 1994, pc486mu,    0,        0, pc486se,   pc386m, pc98_epson_state, init_pc9801_kanji, "Epson", "PC-486MU", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1993, pc486se,    pc486mu,  0, pc486se,   pc386m, pc98_epson_state, init_pc9801_kanji, "Epson", "PC-486SE", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
// PRO-486 (first actual version with i486dx? Supports High-reso)
// PC-486P/Win (same as a PC-486P but with Windows 3.0a + MS-DOS 3.3 HDD pre-installed?)

// PC-586 (Pentium/Pentium ODP compatibles)
// ...

// Epson PC98 L[aptop] line
// PC-286B (80C286)
// PC-286L* (V30 or 80C286)
// ...

// PC-386BL* (i386sx)
// PC-386LS* (just bigger version of above?)
// ...

// Epson PC98 NOTE[book] line
// PC-*86N* (runs on correlated CPU as above)
// PC-486PT ("P[or]T[able]" SL enhanced i486, wallet-like dimensions, supports light pen, runs under "PenDOS")
// ...
