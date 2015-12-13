// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Fabio Priuli
/***************************************************************************

  nes.c

  Driver file to handle emulation of the Nintendo Entertainment System (Famicom).

  MESS driver by Brad Oliver (bradman@pobox.com), NES sound code by Matt Conte.
  Based in part on the old xNes code, by Nicolas Hamel, Chuck Mason, Brad Oliver,
  Richard Bannister and Jeff Mitchell.

***************************************************************************/

#include "emu.h"
#include "includes/nes.h"
#include "cpu/m6502/n2a03.h"
#include "softlist.h"

WRITE8_MEMBER(nes_state::nes_vh_sprite_dma_w)
{
	m_ppu->spriteram_dma(space, data);
}

static ADDRESS_MAP_START( nes_map, AS_PROGRAM, 8, nes_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_MIRROR(0x1800)                   /* RAM */
	AM_RANGE(0x2000, 0x3fff) AM_DEVREADWRITE("ppu", ppu2c0x_device, read, write)        /* PPU registers */
	AM_RANGE(0x4014, 0x4014) AM_WRITE(nes_vh_sprite_dma_w)              /* stupid address space hole */
	AM_RANGE(0x4016, 0x4016) AM_READWRITE(nes_in0_r, nes_in0_w)         /* IN0 - input port 1 */
	AM_RANGE(0x4017, 0x4017) AM_READ(nes_in1_r)                         /* IN1 - input port 2 */
	// 0x4100-0x5fff -> LOW HANDLER defined on a pcb base
	// 0x6000-0x7fff -> MID HANDLER defined on a pcb base
	// 0x8000-0xffff -> HIGH HANDLER defined on a pcb base
ADDRESS_MAP_END

static INPUT_PORTS_START( nes )
	// input devices go through slot options
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "Draw Top/Bottom 8 Lines")
	PORT_CONFSETTING(    0x01, DEF_STR(No) )
	PORT_CONFSETTING(    0x00, DEF_STR(Yes) )
	PORT_CONFNAME( 0x02, 0x00, "Enforce 8 Sprites/line")
	PORT_CONFSETTING(    0x02, DEF_STR(No) )
	PORT_CONFSETTING(    0x00, DEF_STR(Yes) )
INPUT_PORTS_END

static INPUT_PORTS_START( famicom )
	// input devices go through slot options
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "Draw Top/Bottom 8 Lines")
	PORT_CONFSETTING(    0x01, DEF_STR(No) )
	PORT_CONFSETTING(    0x00, DEF_STR(Yes) )
	PORT_CONFNAME( 0x02, 0x00, "Enforce 8 Sprites/line")
	PORT_CONFSETTING(    0x02, DEF_STR(No) )
	PORT_CONFSETTING(    0x00, DEF_STR(Yes) )

	PORT_START("FLIPDISK") /* fake key */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Change Disk Side") PORT_CODE(KEYCODE_SPACE)
INPUT_PORTS_END


void nes_state::ppu_nmi(int *ppu_regs)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


static MACHINE_CONFIG_START( nes, nes_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", N2A03, NTSC_CLOCK)
	MCFG_CPU_PROGRAM_MAP(nes_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.098)
	// This isn't used so much to calulate the vblank duration (the PPU code tracks that manually) but to determine
	// the number of cycles in each scanline for the PPU scanline timer. Since the PPU has 20 vblank scanlines + 2
	// non-rendering scanlines, we compensate. This ends up being 2500 cycles for the non-rendering portion, 2273
	// cycles for the actual vblank period.
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC((113.66/(NTSC_CLOCK/1000000)) * (PPU_VBLANK_LAST_SCANLINE_NTSC-PPU_VBLANK_FIRST_SCANLINE+1+2)))
	MCFG_SCREEN_SIZE(32*8, 262)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(nes_state, screen_update_nes)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 4*16*8)
	MCFG_PALETTE_INIT_OWNER(nes_state, nes)

	MCFG_PPU2C02_ADD("ppu")
	MCFG_PPU2C0X_CPU("maincpu")
	MCFG_PPU2C0X_SET_NMI(nes_state, ppu_nmi)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	// note APU sound level here was specified as 0.90, not 0.50 like the others
	// not sure how to adjust it when it's inside the CPU?

	MCFG_NES_CONTROL_PORT_ADD("ctrl1", nes_control_port1_devices, "joypad")
	MCFG_NESCTRL_BRIGHTPIXEL_CB(nes_state, bright_pixel)
	MCFG_NES_CONTROL_PORT_ADD("ctrl2", nes_control_port2_devices, "joypad")
	MCFG_NESCTRL_BRIGHTPIXEL_CB(nes_state, bright_pixel)

	MCFG_NES_CARTRIDGE_ADD("nes_slot", nes_cart, nullptr)
	MCFG_SOFTWARE_LIST_ADD("cart_list", "nes")
	MCFG_SOFTWARE_LIST_ADD("ade_list", "nes_ade")         // Camerica/Codemasters Aladdin Deck Enhancer mini-carts
	MCFG_SOFTWARE_LIST_ADD("ntb_list", "nes_ntbrom")      // Sunsoft Nantettate! Baseball mini-carts
	MCFG_SOFTWARE_LIST_ADD("kstudio_list", "nes_kstudio") // Bandai Karaoke Studio expansion carts
	MCFG_SOFTWARE_LIST_ADD("datach_list", "nes_datach")   // Bandai Datach Joint ROM System mini-carts
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nespal, nes )

	/* basic machine hardware */
//  MCFG_CPU_MODIFY("maincpu")
//  MCFG_CPU_CLOCK(PAL_CLOCK) // this doesn't get inherited by the APU with DERIVED_CLOCK!

	MCFG_CPU_REPLACE("maincpu", N2A03, PAL_CLOCK)
	MCFG_CPU_PROGRAM_MAP(nes_map)


	MCFG_DEVICE_REMOVE("ppu")
	MCFG_PPU2C07_ADD("ppu")
	MCFG_PPU2C0X_CPU("maincpu")
	MCFG_PPU2C0X_SET_NMI(nes_state, ppu_nmi)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(53.355)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC((106.53/(PAL_CLOCK/1000000)) * (PPU_VBLANK_LAST_SCANLINE_PAL-PPU_VBLANK_FIRST_SCANLINE+1+2)))
	MCFG_SCREEN_SIZE(32*8, 312)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 30*8-1)


MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dendy, nes )

	/* basic machine hardware */
//  MCFG_CPU_MODIFY( "maincpu" )
//  MCFG_CPU_CLOCK( 26601712/15 )  // this doesn't get inherited by the APU with DERIVED_CLOCK!

	MCFG_CPU_REPLACE("maincpu", N2A03, 26601712/15 )/* 26.601712MHz / 15 == 1.77344746666... MHz */
	MCFG_CPU_PROGRAM_MAP(nes_map)

	MCFG_DEVICE_REMOVE("ppu")
	MCFG_PPU2C07_ADD("ppu")
	MCFG_PPU2C0X_CPU("maincpu")
	MCFG_PPU2C0X_SET_NMI(nes_state, ppu_nmi)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50.00697796827)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC((106.53/(PAL_CLOCK/1000000)) * (PPU_VBLANK_LAST_SCANLINE_PAL-PPU_VBLANK_FIRST_SCANLINE+1+2)))

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( famicom, nes )
	MCFG_DEVICE_REMOVE("ctrl1")
	MCFG_DEVICE_REMOVE("ctrl2")
	MCFG_NES_CONTROL_PORT_ADD("ctrl1", fc_control_port1_devices, "joypad")
	MCFG_NES_CONTROL_PORT_ADD("ctrl2", fc_control_port2_devices, "joypad")
	MCFG_FC_EXPANSION_PORT_ADD("exp", fc_expansion_devices, nullptr)
	MCFG_NESCTRL_BRIGHTPIXEL_CB(nes_state, bright_pixel)

	MCFG_SOFTWARE_LIST_ADD("flop_list", "famicom_flop")
	MCFG_SOFTWARE_LIST_ADD("cass_list", "famicom_cass")
MACHINE_CONFIG_END

void nes_state::setup_disk(nes_disksys_device *slot)
{
	if (slot)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);

		// Set up memory handlers
		space.install_read_handler(0x4020, 0x40ff, read8_delegate(FUNC(nes_disksys_device::read_ex), (nes_disksys_device *)slot));
		space.install_write_handler(0x4020, 0x40ff, write8_delegate(FUNC(nes_disksys_device::write_ex), (nes_disksys_device *)slot));
		space.install_read_handler(0x4100, 0x5fff, read8_delegate(FUNC(device_nes_cart_interface::read_l), (device_nes_cart_interface *)slot));
		space.install_write_handler(0x4100, 0x5fff, write8_delegate(FUNC(device_nes_cart_interface::write_l), (device_nes_cart_interface *)slot));
		space.install_read_handler(0x6000, 0x7fff, read8_delegate(FUNC(nes_disksys_device::read_m), (nes_disksys_device *)slot));
		space.install_write_handler(0x6000, 0x7fff, write8_delegate(FUNC(nes_disksys_device::write_m), (nes_disksys_device *)slot));
		space.install_read_handler(0x8000, 0xffff, read8_delegate(FUNC(nes_disksys_device::read_h), (nes_disksys_device *)slot));
		space.install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(nes_disksys_device::write_h), (nes_disksys_device *)slot));

		slot->vram_alloc(0x2000);
		slot->prgram_alloc(0x8000);

		slot->pcb_start(machine(), m_ciram, FALSE);
		m_ppu->space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8_delegate(FUNC(device_nes_cart_interface::chr_r),(device_nes_cart_interface *)slot), write8_delegate(FUNC(device_nes_cart_interface::chr_w),(device_nes_cart_interface *)slot));
		m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8_delegate(FUNC(device_nes_cart_interface::nt_r),(device_nes_cart_interface *)slot), write8_delegate(FUNC(device_nes_cart_interface::nt_w),(device_nes_cart_interface *)slot));
		m_ppu->set_scanline_callback(ppu2c0x_scanline_delegate(FUNC(device_nes_cart_interface::scanline_irq),(device_nes_cart_interface *)slot));
		m_ppu->set_hblank_callback(ppu2c0x_hblank_delegate(FUNC(nes_disksys_device::hblank_irq),(nes_disksys_device *)slot));
		m_ppu->set_latch(ppu2c0x_latch_delegate(FUNC(device_nes_cart_interface::ppu_latch),(device_nes_cart_interface *)slot));
	}
}


MACHINE_START_MEMBER( nes_state, fds )
{
	m_ciram = auto_alloc_array(machine(), UINT8, 0x800);
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

static MACHINE_CONFIG_DERIVED( fds, famicom )
	MCFG_MACHINE_START_OVERRIDE( nes_state, fds )
	MCFG_MACHINE_RESET_OVERRIDE( nes_state, fds )

	MCFG_DEVICE_REMOVE("nes_slot")
	MCFG_DEVICE_ADD("disk", NES_DISKSYS, 0)

	MCFG_DEVICE_REMOVE("cart_list")
	MCFG_DEVICE_REMOVE("cass_list")
MACHINE_CONFIG_END


MACHINE_START_MEMBER( nes_state, famitwin )
{
	// start the base nes stuff
	machine_start();

	// if there is no cart inserted, setup the disk expansion instead
	if (!m_cartslot->exists())
	{
		setup_disk(m_disk);

		// replace the famicom disk ROM with the famicom twin one (until we modernize the floppy drive)
		m_maincpu->space(AS_PROGRAM).install_read_bank(0xe000, 0xffff, "ftbios");
		membank("ftbios")->set_base(machine().root_device().memregion("maincpu")->base() + 0xe000);
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

static MACHINE_CONFIG_DERIVED( famitwin, famicom )

	MCFG_MACHINE_START_OVERRIDE( nes_state, famitwin )
	MCFG_MACHINE_RESET_OVERRIDE( nes_state, famitwin )

	MCFG_DEVICE_MODIFY("nes_slot")
	MCFG_NES_CARTRIDGE_NOT_MANDATORY

	MCFG_DEVICE_ADD("disk", NES_DISKSYS, 0)
MACHINE_CONFIG_END



ROM_START( nes )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  /* Main RAM */
ROM_END

ROM_START( nespal )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  /* Main RAM */
ROM_END

ROM_START( famicom )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  /* Main RAM */
ROM_END

#define rom_fds rom_famicom

ROM_START( famitwin )
	ROM_REGION( 0x10000, "maincpu", 0 )  /* Main RAM */
	ROM_LOAD( "rp2c33a-02.bin", 0xe000, 0x2000, CRC(4df24a6c) SHA1(e4e41472c454f928e53eb10e0509bf7d1146ecc1) ) // "Famicom" logo instead of Nintendo logo
ROM_END

ROM_START( m82 )
	ROM_REGION( 0x14000, "maincpu", 0 )  /* Main RAM + program banks */
	/* Banks to be mapped at 0xe000? More investigations needed... */
	ROM_LOAD( "m82_v1_0.bin", 0x10000, 0x4000, CRC(7d56840a) SHA1(cbd2d14fa073273ba58367758f40d67fd8a9106d) )
ROM_END

// see http://www.disgruntleddesigner.com/chrisc/drpcjr/index.html
// and http://www.disgruntleddesigner.com/chrisc/drpcjr/DrPCJrMemMap.txt
ROM_START( drpcjr )
	ROM_REGION( 0x18000, "maincpu", 0 )  /* Main RAM + program banks */
	/* 4 banks to be mapped in 0xe000-0xffff (or 8 banks to be mapped in 0xe000-0xefff & 0xf000-0xffff).
	Banks selected by writing at 0x4180 */
	ROM_LOAD("drpcjr_bios.bin", 0x10000, 0x8000, CRC(c8fbef89) SHA1(2cb0a817b31400cdf27817d09bae7e69f41b062b) ) // bios vers. 1.0a
	// Not sure if we should support this: hacked version 1.5a by Chris Covell with bugfixes and GameGenie support
//  ROM_LOAD("drpcjr_v1_5_gg.bin", 0x10000, 0x8000, CRC(98f2033b) SHA1(93c114da787a19279d1a46667c2f69b49e25d4f1) )
ROM_END

ROM_START( dendy )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  /* Main RAM */
ROM_END

ROM_START( gchinatv )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  /* Main RAM */
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR  NAME      PARENT  COMPAT MACHINE   INPUT    INIT    COMPANY       FULLNAME */
CONS( 1985, nes,       0,      0,     nes,      nes,     driver_device,     0,       "Nintendo",  "Nintendo Entertainment System / Famicom (NTSC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1987, nespal,    nes,    0,     nespal,   nes,     driver_device,     0,       "Nintendo",  "Nintendo Entertainment System (PAL)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1983, famicom,   nes,    0,     famicom,  famicom, nes_state,         famicom, "Nintendo",  "Famicom (NTSC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1983, fds,       nes,    0,     fds,      famicom, nes_state,         famicom, "Nintendo",  "Famicom (w/ Disk System add-on)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1986, famitwin,  nes,    0,     famitwin, famicom, nes_state,         famicom, "Sharp",     "Famicom Twin", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 198?, m82,       nes,    0,     nes,      nes,     driver_device,     0,       "Nintendo",  "M82 Display Unit", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
CONS( 1996, drpcjr,    nes,    0,     famicom,  famicom, nes_state,         famicom, "Bung",      "Doctor PC Jr", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1992, dendy,     nes,    0,     dendy,    nes,     driver_device,     0,       "Steepler",  "Dendy Classic", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 198?, gchinatv,  nes,    0,     nespal,   nes,     driver_device,     0,       "Golden China",  "Golden China TV Game", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
