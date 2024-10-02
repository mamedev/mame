// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  pippin.cpp: Apple/Bandai Pippin
  Preliminary driver by R. Belmont (based on pippin.c skeleton by Angelo Salese)

  Apple ASICs identified:
  -----------------------
  343S0152    Aspen (Bandit derived PCI host bridge and RAM controller)
  343S0153    Taos (640x480 framebuffer with double-buffering and convolution to fix interlace flicker)
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
  00000000 : RAM
  80000000 : PCI memory (to EFFFFFFF)
  F0000000 : 1 MB video RAM
  F0100000 : Space for second 1 MB of video RAM (unused in Pippin)
  (2 MB VRAM space mirrors every 0x00200000 until F0800000)
  F0800000 : Taos registers
  F1000000 : Palette for 256 color mode
  F3000000 : Grand Central I/O controller
  F3008000 : DMA channel registers
  F3010000 : SCSI0
  F3011000 : MACE Ethernet
  F3012000 : SCC (68K Mac style addressing)
  F3013000 : SCC (MacRISC addressing)
  F3014000 : AWACS audio
  F3015000 : SWIM III floppy
  F3016000 : VIA1 (interface to Cuda)
  F3018000 : SCSI1

  NOTE: the PowerPC starts off disabled; the Cuda 68HC05 starts it up once it's booted.

  NOTE 2: goes off into the weeds in the subroutine at fff05010

****************************************************************************/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/mn1880/mn1880.h"
#include "imagedev/cdromimg.h"
#include "machine/ram.h"
#include "sound/cdda.h"

#include "awacs_macrisc.h"
#include "bandit.h"
#include "cuda.h"
#include "heathrow.h"
#include "macadb.h"
#include "softlist.h"
#include "speaker.h"

class pippin_state : public driver_device
{
public:
	void pippin(machine_config &config);

	pippin_state(const machine_config &mconfig, device_type type, const char *tag);

	required_device<ppc_device> m_maincpu;
	required_device<aspen_host_device> m_aspen;
	required_device<cuda_device> m_cuda;
	required_device<macadb_device> m_macadb;
	required_device<ram_device> m_ram;

private:
	void pippin_map(address_map &map) ATTR_COLD;
	void cdmcu_mem(address_map &map) ATTR_COLD;
	void cdmcu_data(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void cuda_reset_w(int state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	}

	void irq_w(int state)
	{
		m_maincpu->set_input_line(PPC_IRQ, state);
	}
};

pippin_state::pippin_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_aspen(*this, "pci:00.0"),
	m_cuda(*this, "cuda"),
	m_macadb(*this, "macadb"),
	m_ram(*this, RAM_TAG)
{
}

void pippin_state::machine_start()
{
}

void pippin_state::machine_reset()
{
	// the PPC can't run until Cuda's ready
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void pippin_state::pippin_map(address_map &map)
{
	map(0x00000000, 0x005fffff).ram();

	/* writes at 0x0*c01000 the string "Mr. Kesh" and wants it to be read back, true color VRAMs perhaps? */
	map(0x00c00000, 0x00c01007).ram();
	map(0x01c00000, 0x01c01007).ram();
	map(0x02c00000, 0x02c01007).ram();
	map(0x03c00000, 0x03c01007).ram();

	map(0x40000000, 0x403fffff).rom().region("bootrom", 0).mirror(0x0fc00000);   // mirror of ROM for 680x0 emulation
	map(0x40000000, 0x40000000).lr8(NAME([]() { return 0x80; }));   // hack to make flash ROM status check pass (causes 0xe1 to be written to VRAM, which is important later)
	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a7001; }));

	map(0xf0000000, 0xf00fffff).ram();  // VRAM

	map(0xffc00000, 0xffffffff).rom().region("bootrom", 0);
}

void pippin_state::cdmcu_mem(address_map &map)
{
	map(0x0000, 0xffff).rom().region("cdrom", 0);
}

void pippin_state::cdmcu_data(address_map &map)
{
	map(0x0000, 0x0001).noprw();
	map(0x0003, 0x0003).noprw();
	map(0x0004, 0x0008).nopw();
	map(0x0008, 0x0008).nopr();
	map(0x0009, 0x0009).noprw();
	map(0x000f, 0x000f).noprw();
	map(0x001f, 0x0021).nopw();
	map(0x0031, 0x0031).noprw();
	map(0x0033, 0x0033).nopw();
	map(0x0036, 0x0036).noprw();
	map(0x0060, 0x031f).ram();
	map(0x802a, 0x802a).nopw();
	map(0x802f, 0x8034).nopw();
	map(0x8032, 0x8032).nopr();
	map(0x8035, 0x8035).lr8(NAME([]() { return 0x40; }));
	map(0x8037, 0x8037).noprw();
	map(0x8038, 0x8039).nopw();
}

/* Input ports */
static INPUT_PORTS_START( pippin )
INPUT_PORTS_END

void pippin_state::pippin(machine_config &config)
{
	/* basic machine hardware */
	PPC603(config, m_maincpu, 66000000);
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);
	m_maincpu->set_addrmap(AS_PROGRAM, &pippin_state::pippin_map);

	PCI_ROOT(config, "pci", 0);
	ASPEN(config, m_aspen, 66000000, "maincpu").set_dev_offset(1);

	cdrom_image_device &cdrom(CDROM(config, "cdrom", 0));
	cdrom.set_interface("pippin_cdrom");
	SOFTWARE_LIST(config, "cd_list").set_original("pippin");

	RAM(config, m_ram);
	m_ram->set_default_size("32M");

	grandcentral_device &grandcentral(GRAND_CENTRAL(config, "pci:0d.0", 0));
	grandcentral.set_maincpu_tag("maincpu");
	grandcentral.irq_callback().set(FUNC(pippin_state::irq_w));

	awacs_macrisc_device &awacs(AWACS_MACRISC(config, "codec", 45.1584_MHz_XTAL / 2));
	awacs.dma_output().set(grandcentral, FUNC(heathrow_device::codec_dma_read));

	grandcentral.codec_r_callback().set(awacs, FUNC(awacs_macrisc_device::read_macrisc));
	grandcentral.codec_w_callback().set(awacs, FUNC(awacs_macrisc_device::write_macrisc));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	awacs.add_route(0, "lspeaker", 1.0);
	awacs.add_route(1, "rspeaker", 1.0);

	MACADB(config, m_macadb, 15.6672_MHz_XTAL);

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->reset_callback().set(FUNC(pippin_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_cuda->via_clock_callback().set(grandcentral, FUNC(heathrow_device::cb1_w));
	m_cuda->via_data_callback().set(grandcentral, FUNC(heathrow_device::cb2_w));
	m_macadb->adb_data_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);

	grandcentral.pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	grandcentral.pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	grandcentral.pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	grandcentral.cb2_callback().set(m_cuda, FUNC(cuda_device::set_via_data));

	mn1880_device &cdmcu(MN1880(config, "cdmcu", 8388608)); // type and clock unknown
	cdmcu.set_addrmap(AS_PROGRAM, &pippin_state::cdmcu_mem);
	cdmcu.set_addrmap(AS_DATA, &pippin_state::cdmcu_data);
	cdmcu.set_disable();
}

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
	ROMX_LOAD( "bandai pippin,19960920 - kinka 1.3,- 3e6b3ee4-a52528e9ce8c.rom", 0x000000, 0x400000, CRC(87a1337d) SHA1(8e512af6e34dd823f3defec77d43ecbff1ecad54), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "v12", "Kinka v 1.2")
	ROMX_LOAD( "bandai pippin,19960628 - kinka 1.2,- 3e10e14c-72c40c1af23a.rom", 0x000000, 0x400000, CRC(4fead4b3) SHA1(3fa02e9b0fa702ac6e02edc08911eac8b50e2d1f), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "v1", "Kinka v 1.0")
	ROMX_LOAD( "341s0251.u1", 0x000006, 0x100000, CRC(aaea2449) SHA1(2f63e215260a42fb7c5f2364682d5e8c0604646f),ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6) | ROM_BIOS(2))
	ROMX_LOAD( "341s0252.u2", 0x000004, 0x100000, CRC(3d584419) SHA1(e29c764816755662693b25f1fb3c24faef4e9470),ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6) | ROM_BIOS(2))
	ROMX_LOAD( "341s0253.u3", 0x000002, 0x100000, CRC(d8ae5037) SHA1(d46ce4d87ca1120dfe2cf2ba01451f035992b6f6),ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6) | ROM_BIOS(2))
	ROMX_LOAD( "341s0254.u4", 0x000000, 0x100000, CRC(3e2851ba) SHA1(7cbf5d6999e890f5e9ab2bc4b10ca897c4dc2016),ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6) | ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "vgm", "Kinka GM version")
	ROMX_LOAD( "bandai pippin,19960128 - kinka gm flash,- 2bf65931-318e40f6a1f4.rom", 0x000000, 0x400000, CRC(4ff875e6) SHA1(eb8739cab1807c6c7c51acc7f4a3afc1f9c6ddbb), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS(4, "pre", "Kinka pre-release")
	ROMX_LOAD( "kinka-pre.rom", 0x000000, 0x400000, CRC(4ff875e6) SHA1(eb8739cab1807c6c7c51acc7f4a3afc1f9c6ddbb),ROM_BIOS(4) )

	ROM_REGION( 0x10000, "cdrom", 0 ) /* MATSUSHITA CR504-L OEM */
	ROM_LOAD( "504par4.0i.ic7", 0x0000, 0x10000, CRC(25f7dd46) SHA1(ec3b3031742807924c6259af865e701827208fec) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY           FULLNAME        FLAGS */
COMP( 1996, pippin, 0,      0,      pippin,  pippin, pippin_state, empty_init, "Apple / Bandai", "Pippin @mark", MACHINE_NOT_WORKING)
