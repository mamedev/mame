// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Fabio Priuli
/***************************************************************************

  nes.cpp

  Driver file to handle emulation of the Nintendo Entertainment System (Famicom).

  MESS driver by Brad Oliver (bradman@pobox.com), NES sound code by Matt Conte.
  Based in part on the old xNes code, by Nicolas Hamel, Chuck Mason, Brad Oliver,
  Richard Bannister and Jeff Mitchell.

***************************************************************************/

#include "emu.h"
#include "includes/nes.h"

#include "cpu/m6502/n2a03.h"
#include "softlist_dev.h"
#include "speaker.h"


void nes_state::nes_vh_sprite_dma_w(address_space &space, uint8_t data)
{
	m_ppu->spriteram_dma(space, data);
}

void nes_state::nes_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().mirror(0x1800).share("mainram");                              // RAM
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write)); // PPU registers
	map(0x4014, 0x4014).w(FUNC(nes_state::nes_vh_sprite_dma_w));                            // stupid address space hole
	map(0x4016, 0x4016).rw(FUNC(nes_state::nes_in0_r), FUNC(nes_state::nes_in0_w));         // IN0 - input port 1
	map(0x4017, 0x4017).r(FUNC(nes_state::nes_in1_r));                                      // IN1 - input port 2
	// 0x4100-0x5fff -> LOW HANDLER defined on a pcb base
	// 0x6000-0x7fff -> MID HANDLER defined on a pcb base
	// 0x8000-0xffff -> HIGH HANDLER defined on a pcb base
}

static INPUT_PORTS_START( nes )
	// input devices go through slot options
INPUT_PORTS_END

static INPUT_PORTS_START( famicom )
	// input devices go through slot options
	PORT_START("FLIPDISK") // fake key
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Change Disk Side") PORT_CODE(KEYCODE_SPACE)
INPUT_PORTS_END


void nes_state::nes(machine_config &config)
{
	// basic machine hardware
	n2a03_device &maincpu(N2A03(config, m_maincpu, NTSC_APU_CLOCK));
	maincpu.set_addrmap(AS_PROGRAM, &nes_state::nes_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.0988);
	// This isn't used so much to calulate the vblank duration (the PPU code tracks that manually) but to determine
	// the number of cycles in each scanline for the PPU scanline timer. Since the PPU has 20 vblank scanlines + 2
	// non-rendering scanlines, we compensate. This ends up being 2500 cycles for the non-rendering portion, 2273
	// cycles for the actual vblank period.
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66/(NTSC_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_NTSC-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(nes_state::screen_update_nes));
	m_screen->screen_vblank().set(FUNC(nes_state::screen_vblank_nes));

	PPU_2C02(config, m_ppu);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	maincpu.add_route(ALL_OUTPUTS, "mono", 0.90);

	NES_CONTROL_PORT(config, m_ctrl1, nes_control_port1_devices, "joypad");
	NES_CONTROL_PORT(config, m_ctrl2, nes_control_port2_devices, "joypad");
	m_ctrl1->set_screen_tag(m_screen);
	m_ctrl2->set_screen_tag(m_screen);

	NES_CART_SLOT(config, m_cartslot, NTSC_APU_CLOCK, nes_cart, nullptr).set_must_be_loaded(true);
	SOFTWARE_LIST(config, "cart_list").set_original("nes");
	SOFTWARE_LIST(config, "ade_list").set_original("nes_ade");         // Camerica/Codemasters Aladdin Deck Enhancer mini-carts
	SOFTWARE_LIST(config, "ntb_list").set_original("nes_ntbrom");      // Sunsoft Nantettate! Baseball mini-carts
	SOFTWARE_LIST(config, "kstudio_list").set_original("nes_kstudio"); // Bandai Karaoke Studio expansion carts
	SOFTWARE_LIST(config, "datach_list").set_original("nes_datach");   // Bandai Datach Joint ROM System mini-carts
}

void nes_state::nespal(machine_config &config)
{
	nes(config);

	// basic machine hardware
	m_maincpu->set_clock(PAL_APU_CLOCK);

	PPU_2C07(config.replace(), m_ppu);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	m_cartslot->set_clock(PAL_APU_CLOCK);

	// video hardware
	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((106.53/(PAL_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 312);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
}

void nes_state::famicom(machine_config &config)
{
	nes(config);

	NES_CONTROL_PORT(config.replace(), m_ctrl1, fc_control_port1_devices, "joypad");
	NES_CONTROL_PORT(config.replace(), m_ctrl2, fc_control_port2_devices, "joypad");
	NES_CONTROL_PORT(config, m_exp, fc_expansion_devices, nullptr);
	m_ctrl1->set_screen_tag(m_screen);
	m_ctrl2->set_screen_tag(m_screen);
	m_exp->set_screen_tag(m_screen);

	SOFTWARE_LIST(config, "flop_list").set_original("famicom_flop");
	SOFTWARE_LIST(config, "cass_list").set_original("famicom_cass");
}

void nes_state::nespalc(machine_config &config)
{
	nespal(config);

	m_maincpu->set_clock(PALC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_state::nes_map);

	// UMC 6538 and friends -- extends time for rendering dummy scanlines
	PPU_PALC(config.replace(), m_ppu);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline("maincpu", INPUT_LINE_NMI);

	m_cartslot->set_clock(PALC_APU_CLOCK);

	// video hardware
	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66/(PALC_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL-ppu2c0x_device::VBLANK_FIRST_SCANLINE_PALC+1+2)));
}

void nes_state::famipalc(machine_config &config)
{
	nespalc(config);

	NES_CONTROL_PORT(config.replace(), m_ctrl1, fc_control_port1_devices, "joypad");
	NES_CONTROL_PORT(config.replace(), m_ctrl2, fc_control_port2_devices, "joypad");
	NES_CONTROL_PORT(config, m_exp, fc_expansion_devices, nullptr);
	m_ctrl1->set_screen_tag(m_screen);
	m_ctrl2->set_screen_tag(m_screen);
	m_exp->set_screen_tag(m_screen);

	SOFTWARE_LIST(config, "cass_list").set_original("famicom_cass");
}

void nes_state::suborkbd(machine_config &config)
{
	famipalc(config);

	// TODO: emulate the parallel port bus!
	m_exp->set_default_option("subor_keyboard");
	m_exp->set_fixed(true);
}

void nes_state::setup_disk(nes_disksys_device *slot)
{
	if (slot)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);

		// Set up memory handlers
		space.install_read_handler(0x4020, 0x40ff, read8sm_delegate(*slot, FUNC(nes_disksys_device::read_ex)));
		space.install_write_handler(0x4020, 0x40ff, write8sm_delegate(*slot, FUNC(nes_disksys_device::write_ex)));
		space.install_read_handler(0x4100, 0x5fff, read8sm_delegate(*slot, FUNC(device_nes_cart_interface::read_l)));
		space.install_write_handler(0x4100, 0x5fff, write8sm_delegate(*slot, FUNC(device_nes_cart_interface::write_l)));
		space.install_read_handler(0x6000, 0x7fff, read8sm_delegate(*slot, FUNC(nes_disksys_device::read_m)));
		space.install_write_handler(0x6000, 0x7fff, write8sm_delegate(*slot, FUNC(nes_disksys_device::write_m)));
		space.install_read_handler(0x8000, 0xffff, read8sm_delegate(*slot, FUNC(nes_disksys_device::read_h)));
		space.install_write_handler(0x8000, 0xffff, write8sm_delegate(*slot, FUNC(nes_disksys_device::write_h)));

		slot->vram_alloc(0x2000);
		slot->prgram_alloc(0x8000);

		slot->pcb_start(machine(), m_ciram.get(), false);
		m_ppu->space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8sm_delegate(*slot, FUNC(device_nes_cart_interface::chr_r)), write8sm_delegate(*slot, FUNC(device_nes_cart_interface::chr_w)));
		m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8sm_delegate(*slot, FUNC(device_nes_cart_interface::nt_r)), write8sm_delegate(*slot, FUNC(device_nes_cart_interface::nt_w)));
		m_ppu->set_scanline_callback(*slot, FUNC(device_nes_cart_interface::scanline_irq));
		m_ppu->set_hblank_callback(*slot, FUNC(nes_disksys_device::hblank_irq));
		m_ppu->set_latch(*slot, FUNC(device_nes_cart_interface::ppu_latch));
	}
}

MACHINE_START_MEMBER( nes_state, fds )
{
	m_ciram = std::make_unique<uint8_t[]>(0x800);
	m_io_disksel = ioport("FLIPDISK");
	setup_disk(m_disk);

	// register saves
	save_item(NAME(m_last_frame_flip));
	save_pointer(NAME(m_ciram), 0x800);
}

MACHINE_RESET_MEMBER( nes_state, fds )
{
	// Reset the mapper variables
	m_disk->pcb_reset();

	// the rest is the same as for nes/famicom/dendy
	m_maincpu->reset();
}

void nes_state::fds(machine_config &config)
{
	famicom(config);

	MCFG_MACHINE_START_OVERRIDE(nes_state, fds)
	MCFG_MACHINE_RESET_OVERRIDE(nes_state, fds)

	config.device_remove("nes_slot");
	NES_DISKSYS(config, "disk", NTSC_APU_CLOCK);

	config.device_remove("cart_list");
	config.device_remove("cass_list");
	config.device_remove("ade_list");
	config.device_remove("ntb_list");
	config.device_remove("kstudio_list");
	config.device_remove("datach_list");
}

MACHINE_START_MEMBER( nes_state, famitwin )
{
	// start the base nes stuff
	machine_start();

	// if there is no cart inserted, setup the disk expansion instead
	if (!m_cartslot->exists())
	{
		setup_disk(m_disk);

		// replace the famicom disk ROM with the famicom twin one (until we modernize the floppy drive)
		m_maincpu->space(AS_PROGRAM).install_rom(0xe000, 0xffff, memregion("maincpu")->base() + 0xe000);
	}
}

MACHINE_RESET_MEMBER( nes_state, famitwin )
{
	// Reset the mapper variables. Will also mark the char-gen ram as dirty
	m_cartslot->pcb_reset();
	// if there is no cart inserted, initialize the disk expansion instead
	if (!m_cartslot->exists())
		m_disk->pcb_reset();

	// the rest is the same as for nes/famicom/dendy
	m_maincpu->reset();
}

void nes_state::famitwin(machine_config &config)
{
	famicom(config);

	MCFG_MACHINE_START_OVERRIDE( nes_state, famitwin )
	MCFG_MACHINE_RESET_OVERRIDE( nes_state, famitwin )

	m_cartslot->set_must_be_loaded(false);

	NES_DISKSYS(config, "disk", NTSC_APU_CLOCK);
}



ROM_START( nes )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( nespal )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( famicom )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

#define rom_fds rom_famicom

ROM_START( famitwin )
	ROM_REGION( 0x10000, "maincpu", 0 )  // Main RAM
	ROM_LOAD( "rp2c33a-02.bin", 0xe000, 0x2000, CRC(4df24a6c) SHA1(e4e41472c454f928e53eb10e0509bf7d1146ecc1) ) // "Famicom" logo instead of Nintendo logo
ROM_END

ROM_START( m82 )
	ROM_REGION( 0x14000, "maincpu", 0 )  // Main RAM + program banks
	// Banks to be mapped at 0xe000? More investigations needed...
	ROM_LOAD( "m82_v1_0.bin", 0x10000, 0x4000, CRC(7d56840a) SHA1(cbd2d14fa073273ba58367758f40d67fd8a9106d) )
ROM_END

ROM_START( m82p )
	// same as m82
	ROM_REGION( 0x14000, "maincpu", 0 )  // Main RAM + program banks
	// Banks to be mapped at 0xe000? More investigations needed...
	ROM_LOAD( "m82_v1_0.bin", 0x10000, 0x4000, CRC(7d56840a) SHA1(cbd2d14fa073273ba58367758f40d67fd8a9106d) )
ROM_END

// see http://www.disgruntleddesigner.com/chrisc/drpcjr/index.html
// and http://www.disgruntleddesigner.com/chrisc/drpcjr/DrPCJrMemMap.txt
ROM_START( drpcjr )
	ROM_REGION( 0x18000, "maincpu", 0 )  // Main RAM + program banks
	// 4 banks to be mapped in 0xe000-0xffff (or 8 banks to be mapped in 0xe000-0xefff & 0xf000-0xffff).
	// Banks selected by writing at 0x4180
	ROM_LOAD("drpcjr_bios.bin", 0x10000, 0x8000, CRC(c8fbef89) SHA1(2cb0a817b31400cdf27817d09bae7e69f41b062b) ) // bios vers. 1.0a
	// Not sure if we should support this: hacked version 1.5a by Chris Covell with bugfixes and GameGenie support
//  ROM_LOAD("drpcjr_v1_5_gg.bin", 0x10000, 0x8000, CRC(98f2033b) SHA1(93c114da787a19279d1a46667c2f69b49e25d4f1) )
ROM_END

ROM_START( iq501 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( iq502 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( dendy )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( dendy2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( gchinatv )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( sb486 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT    CLASS      INIT          COMPANY          FULLNAME

// Nintendo Entertainment System hardware
CONS( 1985, nes,      0,       0,      nes,      nes,     nes_state, empty_init,   "Nintendo",      "Nintendo Entertainment System / Famicom (NTSC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1987, nespal,   nes,     0,      nespal,   nes,     nes_state, empty_init,   "Nintendo",      "Nintendo Entertainment System (PAL)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
// M82 Display Unit
// supports up to twelve cartridge slots
CONS( 198?, m82,      nes,     0,      nes,      nes,     nes_state, empty_init,   "Nintendo",      "M82 Display Unit (NTSC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
CONS( 198?, m82p,     nes,     0,      nespal,   nes,     nes_state, empty_init,   "Nintendo",      "M82 Display Unit (PAL)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

// Famicom hardware
CONS( 1983, famicom,  0,       nes,    famicom,  famicom, nes_state, init_famicom, "Nintendo",      "Famicom",                         MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1983, fds,      famicom, 0,      fds,      famicom, nes_state, init_famicom, "Nintendo",      "Famicom (w/ Disk System add-on)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1986, famitwin, famicom, 0,      famitwin, famicom, nes_state, init_famicom, "Sharp",         "Famicom Twin",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

// Clone hardware
// Many knockoffs using derivatives of the UMC board design, later incorporated into single CMOS chips, were manufactured before and past the end of the Famicom's timeline.

// !! PAL clones documented here !!
// Famicom-based
CONS( 1992, iq501,    0,       nes,    famipalc, nes,     nes_state, init_famicom, "Micro Genius",  "IQ-501",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1992, iq502,    0,       nes,    famipalc, nes,     nes_state, init_famicom, "Micro Genius",  "IQ-502",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1992, dendy,    iq501,   0,      famipalc, nes,     nes_state, init_famicom, "Steepler",      "Dendy Classic 1",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1992, dendy2,   iq502,   0,      famipalc, nes,     nes_state, init_famicom, "Steepler",      "Dendy Classic 2",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 198?, gchinatv, 0,       nes,    famipalc, nes,     nes_state, init_famicom, "Golden China",  "Golden China TV Game", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

// Subor/Xiao Ba Wang hardware and derivatives
// These clones implement a keyboard and a parallel port for printing from a word processor. Later models have mice, PS/2 ports, serial ports and a floppy drive.
CONS( 1993, sb486,    0,       nes,    suborkbd, nes,     nes_state, init_famicom, "Subor",         "SB-486", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

// !! NTSC clones documented here !!
// Famicom-based
// Bung hardware
// Mice, keyboard, etc, including a floppy drive that allows you to run games with a selection of 4 internal "mappers" available on the system.
CONS( 1996, drpcjr,   0,       nes,    famicom,  famicom, nes_state, init_famicom, "Bung",          "Doctor PC Jr", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
