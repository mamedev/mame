// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/macquadra630.cpp
    Mac Quadra 630 ("Show and Tell")
    Mac LC 580 ("Dragonkid")

    By R. Belmont

    These machines took the cost-reduced but still decent Quadra 605/LC 575
    and made them even cheaper by replacing the full-featured DAFB video chip
    with "Valkyrie".  It appears to offer only a few pre-programmed video
    mode timings.

    Further cost reduction occured by replacing the hard disk with an ATA/IDE
    model instead of the by-then traditional Mac SCSI drive.  The ATA interface
    was wedged into the chipset in a somewhat odd manner that impacts VIA2
    interrupt handling (including video VBL IRQs).

    Known problems:
    - If you don't boot an OS, the mouse pointer will stop updating when the
      question-mark disk appears.  If you do boot an OS, everything's fine.

    - The later version boot ROM for the LC 580 can't boot a SCSI CD-ROM.  It
      reads 512 bytes of a 2048 byte sector and expects CyclePhase_96 to read
      and discard the rest of the sector from the drive.  But it sees a (pseudo)
      DMA command was active and waits for DRQ, which doesn't happen because
      the 53C96's transfer count is zero.  The earlier ROM has the same logic as
      previous (and later!) 53C96 machines and works fine.

	  Video in chips


****************************************************************************/

#include "emu.h"

#include "cuda.h"
#include "f108.h"
#include "iosb.h"
#include "macadb.h"
#include "mactoolbox.h"
#include "valkyrie.h"

#include "bus/nubus/cards.h"
#include "bus/nubus/nubus.h"
#include "cpu/m68000/m68040.h"
#include "machine/input_merger.h"
#include "machine/ram.h"
#include "machine/timer.h"

#include "softlist_dev.h"


#define C32M 31.3344_MHz_XTAL
#define C15M (C32M/2)
#define C7M (C32M/4)

namespace {

class quadra630_state : public driver_device
{
public:
	quadra630_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_f108(*this, "f108"),
		m_primetimeii(*this, "primetimeii"),
		m_video(*this, "valkyrie"),
		m_macadb(*this, "macadb"),
		m_cuda(*this, "cuda"),
		m_ram(*this, RAM_TAG)
	{
	}

	void macqd630(machine_config &config);
	void maclc580(machine_config &config);

	void quadra630_map(address_map &map);
	void lc580_map(address_map &map);

	void init_macqd630();

private:
	required_device<m68040_device> m_maincpu;
	required_device<f108_device> m_f108;
	required_device<primetimeii_device> m_primetimeii;
	required_device<valkyrie_device> m_video;
	required_device<macadb_device> m_macadb;
	required_device<cuda_device> m_cuda;
	required_device<ram_device> m_ram;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void cuda_reset_w(int state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	}
};

void quadra630_state::machine_start()
{
	m_f108->set_ram_info((u32 *) m_ram->pointer(), m_ram->size());
}

void quadra630_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void quadra630_state::init_macqd630()
{
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/
void quadra630_state::quadra630_map(address_map &map)
{
	map(0x00000000, 0xffffffff).m(m_f108, FUNC(f108_device::map));
	map(0x00000000, 0xffffffff).m(m_video, FUNC(valkyrie_device::map));
	map(0x50000000, 0x5fffffff).m(m_primetimeii, FUNC(primetime_device::map));

	// 5000a000 = SONIC if comm slot card is installed
	map(0x5000a000, 0x5000bfff).noprw().mirror(0x00fc0000);

	// 2252 = Q630, 225a = LC580
	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a2252; }));
}

void quadra630_state::lc580_map(address_map &map)
{
	quadra630_map(map);

	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a225a; }));
}

/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void quadra630_state::macqd630(machine_config &config)
{
	M68040(config, m_maincpu, 33_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &quadra630_state::quadra630_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	F108(config, m_f108, 33_MHz_XTAL);
	m_f108->set_maincpu_tag("maincpu");
	m_f108->set_primetimeii_tag("primetimeii");
	m_f108->set_rom_tag("bootrom");
	m_f108->write_ata_irq().set(m_primetimeii, FUNC(primetimeii_device::ata_irq_w));

	PRIMETIMEII(config, m_primetimeii, 33_MHz_XTAL);
	m_primetimeii->set_maincpu_tag("maincpu");
	m_primetimeii->set_scsi_tag("f108:scsi:7:ncr53c96");

	VALKYRIE(config, m_video, C32M);
	m_video->write_irq().set(m_primetimeii, FUNC(primetime_device::via2_irq_w<0x40>));

	MACADB(config, m_macadb, C15M);

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->reset_callback().set(FUNC(quadra630_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_cuda->via_clock_callback().set(m_primetimeii, FUNC(primetime_device::cb1_w));
	m_cuda->via_data_callback().set(m_primetimeii, FUNC(primetime_device::cb2_w));
	m_macadb->adb_data_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);

	input_merger_device &sda_merger(INPUT_MERGER_ALL_HIGH(config, "sda"));
	sda_merger.output_handler().append(m_cuda, FUNC(cuda_device::set_iic_sda));

	m_cuda->iic_sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<0>));
	m_cuda->iic_sda_callback().append(m_video, FUNC(valkyrie_device::sda_write));
	m_cuda->iic_scl_callback().set(m_video, FUNC(valkyrie_device::scl_write));

	m_video->sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<1>));

	m_primetimeii->pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	m_primetimeii->pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	m_primetimeii->pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	m_primetimeii->write_cb2().set(m_cuda, FUNC(cuda_device::set_via_data));

	nubus_device &nubus(NUBUS(config, "pds", 0));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irqe_callback().set(m_primetimeii, FUNC(primetime_device::via2_irq_w<0x20>));
	NUBUS_SLOT(config, "lcpds", "pds", mac_pdslc_cards, nullptr);

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,16M,32M");
}

void quadra630_state::maclc580(machine_config &config)
{
	macqd630(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &quadra630_state::lc580_map);
}

ROM_START( macqd630 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "06684214.bin", 0x000000, 0x100000, CRC(1735e7a5) SHA1(47cd505b6a7c46e5c0ffa29f0d5037c83e94a02f) )
ROM_END

ROM_START( maclc580 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_SYSTEM_BIOS(0, "older", "Version 32F1")
	ROMX_LOAD("06684214.bin", 0x000000, 0x100000, CRC(1735e7a5) SHA1(47cd505b6a7c46e5c0ffa29f0d5037c83e94a02f), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "later", "Version 32F2 (bug: can't boot CD-ROM)")
	ROMX_LOAD( "064dc91d.bin", 0x000000, 0x100000, CRC(59e6960f) SHA1(f48a8adf06bce50beee033d0d814da0e5e916d08), ROM_BIOS(1))
ROM_END

} // anonymous namespace

COMP( 1994, macqd630, 0, 0, macqd630, macadb, quadra630_state, init_macqd630,  "Apple Computer", "Macintosh Quadra 630", MACHINE_SUPPORTS_SAVE)
COMP( 1995, maclc580, macqd630, 0, maclc580, macadb, quadra630_state, init_macqd630,  "Apple Computer", "Macintosh LC/Performa 580", MACHINE_SUPPORTS_SAVE)
