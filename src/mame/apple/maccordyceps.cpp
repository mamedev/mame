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

    The PrimeTime II I/O ASIC with 8-bit audio is in the 5200/6200 developer
    note, but Apple says only a limited number of early 5200s shipped with it.
    PrimeTime III, 343S1189, which adds 16-bit audio, is in the rest of the
    family, including the measured Performa 6200CD. The later 5260/100 and
    5260/120 upgrade the CPU to a 603e.
    It is not to be confused with the 6360/160, which is a complete redesign
    and has nothing to do with the Cordyceps architecture.

    The Capella bridge chip, as well as the existence of other PowerPC-to-68k bridge chips,
    will warrant a cleanup/refactor of some other 68k Mac drivers and devices
    to fully support PowerPC accelerators. But we can hack around that for now.

    Driver status:
    Boots to Finder from the ATA hard disk and from the internal SCSI CD-ROM.

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
#include "bus/maccomm/cards.h"
#include "bus/maccomm/maccomm.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/nubus/cards.h"
#include "bus/nubus/nubus.h"
#include "cpu/powerpc/ppc.h"
#include "machine/input_merger.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/flt_rc.h"

#include "softlist_dev.h"
#include "speaker.h"

#include <numbers>

#define C32M 31.3344_MHz_XTAL

namespace { // anonymous namespace

constexpr XTAL CPU_BCLK = C32M * 43 / 36;

class pmac6200_state : public driver_device
{
public:
	pmac6200_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_capella(*this, "capella"),
		m_f108(*this, "f108"),
		m_primetimeiii(*this, "primetimeiii"),
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

	void pmac5200(machine_config &config);

private:
	required_device<ppc603_device> m_maincpu;
	required_device<capella_device> m_capella;
	required_device<f108_device> m_f108;
	required_device<primetimeiii_device> m_primetimeiii;
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

	void cuda_power_w(int state)
	{
		if (state)
			machine().schedule_exit();
	}

	void nmi_irq(int state)
	{
		m_capella->nmi_w(state);
	}


	u32 id_r(offs_t offset, u32 mem_mask);
	u32 comm_r();
	void comm_w(u32 data);

	u32 m_model_id;
	u32 m_wait_comm_load, m_wait_comm_store;
};

void pmac6200_state::machine_start()
{
	m_f108->set_ram_info(m_ram->pointer<u32>(), m_ram->size());

	m_wait_comm_load = m_maincpu->attotime_to_cycles(attotime::from_usec(72));
	m_wait_comm_store = m_maincpu->attotime_to_cycles(attotime::from_usec(92));

	m_maincpu->space(AS_PROGRAM).install_read_tap(0x41000000, 0x410fffff, "overlay_off",
			[this] (offs_t offset, u64 &data, u64 mem_mask) { m_f108->overlay_off(); });
}

void pmac6200_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void pmac6200_state::init_pmac6200()
{
}

uint32_t pmac6200_state::id_r(offs_t offset, uint32_t mem_mask)
{
	return m_model_id;
}

u32 pmac6200_state::comm_r()
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu->bus_error();
		m_maincpu->adjust_icount(-int(m_wait_comm_load));
	}
	return 0;
}

void pmac6200_state::comm_w(u32 data)
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu->bus_error();
		m_maincpu->adjust_icount(-int(m_wait_comm_store));
	}
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
	map(0x50000000, 0x53ffffff).m(m_primetimeiii, FUNC(primetime_device::map));
	map(0x5ffffffc, 0x5fffffff).r(FUNC(pmac6200_state::id_r));

	map(0x50f08000, 0x50f09fff).lr32(NAME([] () { return ~u32(0); }));
	map(0x50f0a000, 0x50f0bfff).rw(FUNC(pmac6200_state::comm_r), FUNC(pmac6200_state::comm_w));

	// Capella-mapped devices in 64-bit address space
	map(0x40000000, 0x403fffff).mirror(0x0fc00000).rom().region("bootrom64", 0);
	map(0xff000000, 0xff3fffff).mirror(0x00c00000).rom().region("bootrom64", 0);

	map(0x00000000, 0xffffffff).m(m_capella, FUNC(capella_device::map));
}

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

void pmac6200_state::pmac6200(machine_config &config)
{
	m_model_id = 0xa55a2250;

	PPC603(config, m_maincpu, CPU_BCLK * 2);
	// BUS_RETRY: TurboSCSI stalls the CPU until DRQ
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS | PPCDRC_BUS_RETRY | PPCDRC_BUS_ERROR);
	m_maincpu->set_bus_frequency(CPU_BCLK); // FSB freq to Capella
	m_maincpu->set_addrmap(AS_PROGRAM, &pmac6200_state::pmac6200_map);
	config.set_perfect_quantum(m_maincpu); // chimes of death without it

	CAPELLA(config, m_capella, CPU_BCLK * 2);
	m_capella->set_maincpu_tag("maincpu");

	F108(config, m_f108, CPU_BCLK); // 68040 bus speed, same as the FSB
	m_f108->set_maincpu_tag("maincpu");
	m_f108->set_primetimeii_tag(m_primetimeiii);
	m_f108->write_ata_irq().set(m_primetimeiii, FUNC(primetimeii_device::ata_irq_w));
	m_f108->set_scsi_clock(CPU_BCLK / ((15.0 / 8) + (1.0 / 2048)));

	NSCSI_CONNECTOR(config, "f108:scsi:0", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "f108:scsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "f108:scsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "f108:scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config(
		[](device_t *device)
		{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^^dfac2", 1.0, 0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^^dfac2", 1.0, 1);
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^^dfac2:rec_0", 1.0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^^dfac2:rec_1", 1.0);
		});
	NSCSI_CONNECTOR(config, "f108:scsi:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "f108:scsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "f108:scsi:6", mac_scsi_devices, nullptr);

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("PPC603");

	PRIMETIMEIII(config, m_primetimeiii, CPU_BCLK);
	m_primetimeiii->set_maincpu_tag("maincpu");
	m_primetimeiii->set_scsi_tag("f108:ncr53c96");
	m_primetimeiii->set_capella_tag("capella");

	VALKYRIE(config, m_video, C32M);
	m_video->write_irq().set(m_primetimeiii, FUNC(primetime_device::via2_irq_w<0x40>));

	ADB_BUS(config, m_adbbus);
	ADB_CONNECTOR(config, "adb:0", adb_devices, "hle_keyboard");
	ADB_CONNECTOR(config, "adb:1", adb_devices, "hle_mouse");

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->zero_default_pram();
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->poweroff_callback().set(FUNC(pmac6200_state::cuda_power_w));
	m_cuda->reset_callback().set(FUNC(pmac6200_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_adbbus, FUNC(adb_bus_device::adb_host_line_w));
	m_cuda->via_clock_callback().set(m_primetimeiii, FUNC(primetime_device::cb1_w));
	m_cuda->via_data_callback().set(m_primetimeiii, FUNC(primetime_device::cb2_w));
	m_cuda->nmi_callback().set(FUNC(pmac6200_state::nmi_irq));
	m_adbbus->out_adb_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	m_adbbus->out_poweron_callback().set(m_cuda, FUNC(cuda_device::set_adb_power));


	input_merger_device &sda_merger(INPUT_MERGER_ALL_HIGH(config, "sda"));
	sda_merger.output_handler().append(m_cuda, FUNC(cuda_device::set_iic_sda));

	m_cuda->iic_sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<0>));
	m_cuda->iic_sda_callback().append(m_video, FUNC(valkyrie_device::sda_write));
	m_cuda->iic_scl_callback().set(m_video, FUNC(valkyrie_device::scl_write));

	m_video->sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<1>));

	APPLE_DFAC2(config, m_dfac2);
	m_dfac2->sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<2>));
	m_cuda->iic_scl_callback().append(m_dfac2, FUNC(dfac2_device::scl_write));
	m_cuda->iic_sda_callback().append(m_dfac2, FUNC(dfac2_device::sda_write));

	asc_base_device &asc = *m_primetimeiii->subdevice<asc_base_device>("asc");
	asc.reset_routes();
	asc.add_route(0, m_dfac2->dac(0), 1.0);
	asc.add_route(1, m_dfac2->dac(1), 1.0);
	MICROPHONE(config, "sound_in", 2).front().add_route(0, m_dfac2->rec(2), 1.0).add_route(1, m_dfac2->rec(3), 1.0);
	for (int i = 0; i < 4; i++)
		m_dfac2->rec(i).lookup()->add_route(0, asc, 1.0);
	// The board's reconstruction low-pass is modeled as two RC stages, with corners fitted to the line output of a Performa 6200CD
	static constexpr const char *const LPF_TAGS[2][2] = { { "lpf1_l", "lpf_l" }, { "lpf1_r", "lpf_r" } };
	for (int ch = 0; ch < 2; ch++)
	{
		FILTER_RC(config, LPF_TAGS[ch][0]).set_lowpass(RES_K(1), 1.0 / (2.0 * std::numbers::pi * RES_K(1) * 2270.0))
			.add_route(0, LPF_TAGS[ch][1], 1.0);
		FILTER_RC(config, LPF_TAGS[ch][1]).set_lowpass(RES_K(1), 1.0 / (2.0 * std::numbers::pi * RES_K(1) * 11723.0))
			.add_route(0, "primetimeiii:speaker", 1.0, ch);
		m_dfac2->add_route(ch, LPF_TAGS[ch][0], 1.0);
	}

	m_primetimeiii->pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	m_primetimeiii->pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	m_primetimeiii->pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	m_primetimeiii->write_cb2().set(m_cuda, FUNC(cuda_device::set_via_data));

	// per the Apple Developer Notes, the PDS expansion card only appears at fe.
	// note that this PDS implementation is broken on real hardware because
	// of the use of a PowerPC chip, so accelerators will cause problems.
	//
	// TODO: PDS is to be added later because of said 68k/PPC incompatibility.

	MAC_COMM_SLOT(config, "comm", m_maincpu, AS_PROGRAM, mac_comm_cards, nullptr)
		.irq_cb().set(m_primetimeiii, FUNC(primetime_device::via2_irq_w<0x01>));

	RAM(config, m_ram);
	m_ram->set_default_size("8M");
	m_ram->set_extra_options("8M,16M,32M,64M"); // per service manual
}


void pmac6200_state::pmac5200(machine_config &config)
{
	pmac6200(config);

	m_model_id = 0xa55a3258;
}

ROM_START( pmac6200 )
	// bootrom is on the 64-bit PowerPC bus, so it should be loaded in a 64-bit space
	ROM_REGION64_BE(0x400000, "bootrom64", 0)
	ROM_LOAD( "63abfd3f.bin", 0x000000, 0x400000, CRC(2f47a6ea) SHA1(0b34d7c692594695b39719c3bf21808985f89f2c) )
ROM_END

} // anonymous namespace


#define rom_pmac5200 rom_pmac6200

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT   CLASS           INIT            COMPANY           FULLNAME                   FLAGS
COMP( 1995, pmac6200, 0,        0,      pmac6200, macadb, pmac6200_state, init_pmac6200,  "Apple Computer", "Power Macintosh 6200/75", MACHINE_NOT_WORKING)
COMP( 1995, pmac5200, pmac6200, 0,      pmac5200, macadb, pmac6200_state, init_pmac6200,  "Apple Computer", "Power Macintosh 5200/75", MACHINE_NOT_WORKING)
