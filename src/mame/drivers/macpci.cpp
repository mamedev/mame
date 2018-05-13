// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  macpci.c: second-generation Old World PowerMacs based on PCI instead of NuBus

  Preliminary driver by R. Belmont (based on pippin.c skeleton by Angelo Salese)

  Pippin:

  Apple ASICs identified:
  -----------------------
  343S1125    Grand Central (SWIM III, Sound, VIA)
  341S0060    Cuda (68HC05 MCU, handles ADB and parameter ("CMOS") RAM)
  343S1146    ??? (likely SCSI due to position on board)
  343S1191(x2) Athens Prime PLL Clock Generator

  Other chips
  -----------
  Z8530 SCC
  CS4217 audio DAC
  Bt856 video DAC

  Pippin-type map
  F3000000 : Grand Central DMA/IRQ controller
  F3012000 : SCC
  F3013000 : Grand Central system controller
  F3014000 : AWACS audio
  F3015000 : SWIM III
  F3016000 : VIA1
  F3018000 : SCSI (53C96)

  NOTE: the PowerPC starts off disabled; the Cuda 68HC05 starts it up once it's booted.

  NOTE 2: goes off into the weeds in the subroutine at fff05010

****************************************************************************/

#include "emu.h"
#include "includes/macpci.h"
#include "cpu/powerpc/ppc.h"
#include "imagedev/chd_cd.h"
#include "sound/cdda.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

READ64_MEMBER( macpci_state::unk1_r )
{
	m_unk1_test ^= 0x0400; //PC=ff808760

	return m_unk1_test << 16;
}

READ64_MEMBER( macpci_state::unk2_r )
{
	if (ACCESSING_BITS_32_47)
		return (uint64_t)0xe1 << 32; //PC=fff04810

	return 0;
}

void macpci_state::pippin_mem(address_map &map)
{
	map(0x00000000, 0x005fffff).ram();

	/* writes at 0x0*c01000 the string "Mr. Kesh" and wants it to be read back, true color VRAMs perhaps? */
	map(0x00c00000, 0x00c01007).ram();
	map(0x01c00000, 0x01c01007).ram();
	map(0x02c00000, 0x02c01007).ram();
	map(0x03c00000, 0x03c01007).ram();

	map(0x40000000, 0x403fffff).rom().region("bootrom", 0).mirror(0x0fc00000);   // mirror of ROM for 680x0 emulation

	map(0xf00dfff8, 0xf00dffff).r(this, FUNC(macpci_state::unk2_r));
	map(0xf3008800, 0xf3008807).r(this, FUNC(macpci_state::unk1_r));

	map(0xf3016000, 0xf3017fff).rw(this, FUNC(macpci_state::mac_via_r), FUNC(macpci_state::mac_via_w));

	map(0xffc00000, 0xffffffff).rom().region("bootrom", 0);
}

/* Input ports */
static INPUT_PORTS_START( pippin )
INPUT_PORTS_END


uint32_t macpci_state::screen_update_pippin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

MACHINE_CONFIG_START(macpci_state::pippin)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", PPC603, 66000000)
	MCFG_DEVICE_PROGRAM_MAP(pippin_mem)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(macpci_state, screen_update_pippin)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_DEVICE_ADD( "cdda", CDDA )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.00 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.00 )

	MCFG_CDROM_ADD("cdrom")
	MCFG_CDROM_INTERFACE("pippin_cdrom")
	MCFG_SOFTWARE_LIST_ADD("cd_list","pippin")

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32M")

	MCFG_DEVICE_ADD("via6522_0", VIA6522, C7M/10)
	MCFG_VIA6522_READPA_HANDLER(READ8(*this, macpci_state, mac_via_in_a))
	MCFG_VIA6522_READPB_HANDLER(READ8(*this, macpci_state, mac_via_in_b))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(*this, macpci_state,mac_via_out_a))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(*this, macpci_state,mac_via_out_b))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(*this, macpci_state, mac_adb_via_out_cb2))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(*this, macpci_state,mac_via_irq))

	//MCFG_DEVICE_ADD("scc", SCC8530, C7M)
	//MCFG_Z8530_INTRQ_CALLBACK(WRITELINE(*this, macpci_state, set_scc_interrupt))
	MCFG_CUDA_ADD(CUDA_341S0060)
	MCFG_CUDA_RESET_CALLBACK(WRITELINE(*this, macpci_state, cuda_reset_w))
	MCFG_CUDA_LINECHANGE_CALLBACK(WRITELINE(*this, macpci_state, cuda_adb_linechange_w))
	MCFG_CUDA_VIA_CLOCK_CALLBACK(WRITELINE("via6522_0", via6522_device, write_cb1))
	MCFG_CUDA_VIA_DATA_CALLBACK(WRITELINE("via6522_0", via6522_device, write_cb2))
	MCFG_QUANTUM_PERFECT_CPU("maincpu")
MACHINE_CONFIG_END

/* ROM definition */
/*

    BIOS versions

    dev
    monitor 341S0241 - 245,247,248,250
    1.0     341S0251..254               U1-U4
    1.2     341S0297..300               U15,U20,U31,U35
    1.3     341S0328..331               U1/U31, U2/U35, U3/U15 and U4/U20

*/

ROM_START( pippin )
	ROM_REGION( 0x400000, "bootrom",  ROMREGION_64BIT | ROMREGION_BE )
	ROM_SYSTEM_BIOS(0, "v13", "Kinka v 1.3")
	ROMX_LOAD( "bandai pippin,19960920 - kinka 1.3,- 3e6b3ee4-a52528e9ce8c.rom", 0x000000, 0x400000, CRC(87a1337d) SHA1(8e512af6e34dd823f3defec77d43ecbff1ecad54), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(1, "v12", "Kinka v 1.2")
	ROMX_LOAD( "bandai pippin,19960628 - kinka 1.2,- 3e10e14c-72c40c1af23a.rom", 0x000000, 0x400000, CRC(4fead4b3) SHA1(3fa02e9b0fa702ac6e02edc08911eac8b50e2d1f), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS(2, "v1", "Kinka v 1.0")
	ROMX_LOAD( "341s0251.u1", 0x000006, 0x100000, CRC(aaea2449) SHA1(2f63e215260a42fb7c5f2364682d5e8c0604646f),ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6) | ROM_BIOS(3))
	ROMX_LOAD( "341s0252.u2", 0x000004, 0x100000, CRC(3d584419) SHA1(e29c764816755662693b25f1fb3c24faef4e9470),ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6) | ROM_BIOS(3))
	ROMX_LOAD( "341s0253.u3", 0x000002, 0x100000, CRC(d8ae5037) SHA1(d46ce4d87ca1120dfe2cf2ba01451f035992b6f6),ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6) | ROM_BIOS(3))
	ROMX_LOAD( "341s0254.u4", 0x000000, 0x100000, CRC(3e2851ba) SHA1(7cbf5d6999e890f5e9ab2bc4b10ca897c4dc2016),ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6) | ROM_BIOS(3))

	ROM_SYSTEM_BIOS(3, "vgm", "Kinka GM version")
	ROMX_LOAD( "bandai pippin,19960128 - kinka gm flash,- 2bf65931-318e40f6a1f4.rom", 0x000000, 0x400000, CRC(4ff875e6) SHA1(eb8739cab1807c6c7c51acc7f4a3afc1f9c6ddbb), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS(4, "pre", "Kinka pre-release")
	ROMX_LOAD( "kinka-pre.rom", 0x000000, 0x400000, CRC(4ff875e6) SHA1(eb8739cab1807c6c7c51acc7f4a3afc1f9c6ddbb),ROM_BIOS(5) )

	ROM_REGION( 0x10000, "cdrom", 0 ) /* MATSUSHITA CR504-L OEM */
	ROM_LOAD( "504par4.0i.ic7", 0x0000, 0x10000, CRC(25f7dd46) SHA1(ec3b3031742807924c6259af865e701827208fec) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY           FULLNAME        FLAGS */
COMP( 1996, pippin, 0,      0,      pippin,  pippin, macpci_state, empty_init, "Apple / Bandai", "Pippin @mark", MACHINE_NOT_WORKING)
