// license:BSD-3-Clause
// copyright-holders:wurthless-elektroniks, R. Belmont
/****************************************************************************

    Power Macintosh x200/x300 "Cordyceps" hardware
    Heavily based on maccquadra630.cpp by R. Belmont

    The bootrom calls this board "Cordyceps" ("Boot Cordyceps 6")
    but the Apple codenames "Crusader" and "Elixir" are better known.

    This machine is an Apple fan "favorite". Ostensibly positioned as the
    successor to the 6100, reviewers opened it up and quickly found that
    it was really a Quadra 630 with a PowerPC 603 grafted onto it,
    with the expected performance bottlenecks making it perform worse
    than its predecessor. Add to that the overall cheapness of the
    case, poor software support by Mac OS, and unstable clock generators
    causing freezes on early production boards, and this machine became
    a perennial contender in the discussion of "worst Mac ever made".

    The later 5260/100 and 5260/120 upgrade the CPU to a 603e and substitute
    the PrimeTime II for the PrimeTime III, which adds 16-bit audio.
    It is not to be confused with the 6360/160, which is a complete redesign
    and has nothing to do with the Cordyceps architecture.

    The Capella bridge chip, as well as the existence of other PowerPC-to-68k bridge chips,
    will warrant a cleanup/refactor of some other 68k Mac drivers and devices
    to fully support PowerPC accelerators. But we can hack around that for now.

    Driver status:
    Boots to Finder from the ATA hard disk.  TurboSCSI was not intended for PowerPC use and
    a CD-ROM boot unsurprisingly hangs.

    Machine IDs:
    pmac5200: 0x3258, 0x3259, 0x325C, 0x325D, 0x325E
    pmac6200: 0x3250, 0x3251, 0x3254, 0x3255, 0x3256

****************************************************************************/

#include "emu.h"

#include "capella.h"
#include "cuda.h"
#include "dfac2.h"
#include "f108.h"
#include "iosb.h"
#include "mactoolbox.h"
#include "valkyrie.h"

#include "bus/adb/adb.h"
#include "bus/adb/cards.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/nubus/cards.h"
#include "bus/nubus/nubus.h"
#include "cpu/powerpc/ppc.h"
#include "machine/input_merger.h"
#include "machine/ram.h"
#include "machine/timer.h"

#include "softlist_dev.h"

#define C32M 31.3344_MHz_XTAL

namespace { // anonymous namespace

class pmac6200_state : public driver_device
{
public:
	pmac6200_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_capella(*this, "capella"),
		m_f108(*this, "f108"),
		m_primetimeii(*this, "primetimeii"),
		m_dfac2(*this, "dfac2"),
		m_video(*this, "valkyrie"),
		m_adbbus(*this, "adb"),
		m_cuda(*this, "cuda"),
		m_ram(*this, RAM_TAG)
	{
	}

	void pmac6200(machine_config &config);

	void pmac6200_map(address_map &map) ATTR_COLD;

	void init_pmac6200();

private:
	required_device<ppc603_device> m_maincpu;
	required_device<capella_device> m_capella;
	required_device<f108_device> m_f108;
	required_device<primetimeii_device> m_primetimeii;
	required_device<dfac2_device> m_dfac2;
	required_device<valkyrie_device> m_video;
	required_device<adb_bus_device> m_adbbus;
	required_device<cuda_device> m_cuda;
	required_device<ram_device> m_ram;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void cuda_reset_w(int state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	}

	void nmi_irq(int state)
	{
		m_capella->nmi_w(state);
	}
};

void pmac6200_state::machine_start()
{
	m_f108->set_ram_info(m_ram->pointer<u32>(), m_ram->size());
}

void pmac6200_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void pmac6200_state::init_pmac6200()
{
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

// [info 20d28] Box PowerMac 6200 (was Carnation 16) decoder 14 (djMEMC/MEMCjr/F108)
// VIA mask 00000000 VIA match 00000000 ID 3250
// [decoder @ 000218de] Screen physical f9001000 logical 32-bit f9001000 logical 24-bit 00000000
// ROM @ 40800000
// diag ROM @ 58000000 VIA1 @ 50f00000 SCC Read @ 50f0c020
// SCC Write @ 50f0c020 IWM/SWIM @ 50f1e000 VIA2 @ 50f02000 ASC @ 50f14000 VDAC @ 50f24000
// SONIC @ 50f0a000 SCSI96 1 @ 50f10000 Patch ROM @ 5ff00000
void pmac6200_state::pmac6200_map(address_map &map)
{
	// 68040 bus includes main RAM, with the expected bottlenecks
	map(0x00000000, 0xffffffff).m(m_f108, FUNC(f108_device::map));
	map(0x00000000, 0xffffffff).m(m_video, FUNC(valkyrie_device::map));
	map(0x50000000, 0x53ffffff).m(m_primetimeii, FUNC(primetime_device::map));

	// SONIC ethernet is supposed to live here. for now, pretend it's not there
	map(0x50f0a000, 0x50f0bfff).noprw();

	// Capella-mapped devices in 64-bit address space
	map(0x40000000, 0x403fffff).rom().region("bootrom64", 0); // HACK: should be mirrored, but doing
	map(0x40800000, 0x40bfffff).rom().region("bootrom64", 0); // .mirror(0x0fc00000) crashes the bootrom

	map(0xffc00000, 0xffffffff).rom().region("bootrom64", 0);

	map(0x00000000, 0xffffffff).m(m_capella, FUNC(capella_device::map));

	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a3250; }));
}

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

void pmac6200_state::pmac6200(machine_config &config)
{
	PPC603(config, m_maincpu, 75_MHz_XTAL);
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);
	m_maincpu->set_bus_frequency(XTAL(75_MHz_XTAL)); // FSB freq to Capella
	m_maincpu->set_addrmap(AS_PROGRAM, &pmac6200_state::pmac6200_map);
	config.set_perfect_quantum(m_maincpu); // chimes of death without it

	CAPELLA(config, m_capella, 75_MHz_XTAL);
	m_capella->set_maincpu_tag("maincpu");

	F108(config, m_f108, 75_MHz_XTAL / 2); // 68040 bus speed is 37.5 MHz, half of FSB frequency
	m_f108->set_maincpu_tag("maincpu");
	m_f108->set_primetimeii_tag("primetimeii");
	m_f108->set_rom_tag("bootrom");
	m_f108->write_ata_irq().set(m_primetimeii, FUNC(primetimeii_device::ata_irq_w));

	NSCSI_CONNECTOR(config, "f108:scsi:0", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "f108:scsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "f108:scsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "f108:scsi:3", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "f108:scsi:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "f108:scsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "f108:scsi:6", mac_scsi_devices, nullptr);

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");

	PRIMETIMEII(config, m_primetimeii, 75_MHz_XTAL / 2); // guessed
	m_primetimeii->set_maincpu_tag("maincpu");
	m_primetimeii->set_scsi_tag("f108:ncr53c96");
	m_primetimeii->set_capella_tag("capella");

	VALKYRIE(config, m_video, C32M); // TODO: confirm on real hardware
	m_video->write_irq().set(m_primetimeii, FUNC(primetime_device::via2_irq_w<0x40>));

	ADB_BUS(config, m_adbbus);
	ADB_CONNECTOR(config, "adb:0", adb_devices, "hle_keyboard");
	ADB_CONNECTOR(config, "adb:1", adb_devices, "hle_mouse");

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->zero_default_pram();
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->reset_callback().set(FUNC(pmac6200_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_adbbus, FUNC(adb_bus_device::adb_host_line_w));
	m_cuda->via_clock_callback().set(m_primetimeii, FUNC(primetime_device::cb1_w));
	m_cuda->via_data_callback().set(m_primetimeii, FUNC(primetime_device::cb2_w));
	m_cuda->nmi_callback().set(FUNC(pmac6200_state::nmi_irq));
	m_adbbus->out_adb_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	m_adbbus->out_poweron_callback().set(m_cuda, FUNC(cuda_device::set_adb_power));


	input_merger_device &sda_merger(INPUT_MERGER_ALL_HIGH(config, "sda"));
	sda_merger.output_handler().append(m_cuda, FUNC(cuda_device::set_iic_sda));

	m_cuda->iic_sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<0>));
	m_cuda->iic_sda_callback().append(m_video, FUNC(valkyrie_device::sda_write));
	m_cuda->iic_scl_callback().set(m_video, FUNC(valkyrie_device::scl_write));

	m_video->sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<1>));

	APPLE_DFAC2(config, m_dfac2, 22257);
	m_dfac2->sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<2>));
	m_cuda->iic_scl_callback().append(m_dfac2, FUNC(dfac2_device::scl_write));
	m_cuda->iic_sda_callback().append(m_dfac2, FUNC(dfac2_device::sda_write));

	m_primetimeii->pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	m_primetimeii->pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	m_primetimeii->pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	m_primetimeii->write_cb2().set(m_cuda, FUNC(cuda_device::set_via_data));

	// per the Apple Developer Notes, the PDS expansion card only appears at fe.
	// note that this PDS implementation is broken on real hardware because
	// of the use of a PowerPC chip, so accelerators will cause problems.
	//
	// TODO: PDS is to be added later because of said 68k/PPC incompatibility.

	RAM(config, m_ram);
	m_ram->set_default_size("8M");
	m_ram->set_extra_options("8M,16M,32M,64M"); // per service manual
}

#define PPC_MAKE_BRANCH_ALWAYS(x) \
	ROM_FILL(x,   1, 0x48) \
	ROM_FILL(x+1, 1, 0x00) \
	// .

#define PPC_ASSEMBLE_NOP(x) \
	ROM_FILL(x,   1, 0x7f) \
	ROM_FILL(x+1, 1, 0xff) \
	ROM_FILL(x+2, 1, 0xfb) \
	ROM_FILL(x+3, 1, 0x78) \
	// .

ROM_START( pmac6200 )
	// bootrom is on the 64-bit PowerPC bus, so it should be loaded in a 64-bit space
	ROM_REGION64_BE(0x400000, "bootrom64", 0)
	ROM_LOAD( "63abfd3f.bin", 0x000000, 0x400000, CRC(2f47a6ea) SHA1(0b34d7c692594695b39719c3bf21808985f89f2c) )

	// HACK: the bootrom tells the Capella to map in cache/tag RAMs and then tests them, failing if they're bad.
	// we skip these tests for now; if it turns out these are never touched again,
	// then these hacks can probably stay here...
	PPC_MAKE_BRANCH_ALWAYS(0x3051b0) // skip checksum mismatch panic
	PPC_ASSEMBLE_NOP(0x30529c)       // NOP out call to cache/tag RAM tests
	PPC_MAKE_BRANCH_ALWAYS(0x3052a4) // avoid panic case after patched-out routine

	// this is here to keep the F108 happy
	// TODO: confirm on real hardware if the 68k side can see the bootrom
	ROM_REGION32_BE(0x400000, "bootrom", 0)
	ROM_FILL(0, 0x400000, 0)
ROM_END

} // anonymous namespace

COMP( 1995, pmac6200, 0, 0, pmac6200, macadb, pmac6200_state, init_pmac6200,  "Apple Computer", "Power Macintosh 6200/75", MACHINE_NOT_WORKING)
