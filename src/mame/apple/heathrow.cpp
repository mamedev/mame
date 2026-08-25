// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "Grand Central", "O'Hare", "Heathrow" and "Paddington" PCI ASICs
    Emulation by R. Belmont

    These ASICs sit on the PCI bus and provide what came to be known as "Mac I/O",
    including:
    - A VIA to interface with Cuda / an enhanced VIA that can speak SPI to interface with PG&E (O'Hare and later)
    - Serial
    - SWIM3 floppy
    - MESH SCSI ("Macintosh Enhanced SCSI Handler"), a 5394/96 clone with some features Apple didn't use removed)
    - ATA
    - Ethernet (10 Mbps for Heathrow, 10/100 for Paddington)
    - Audio
    - Descriptor-based DMA engine, as described in "Macintosh Technology in the Common Hardware Reference Platform"
    - PRAM for Open Firmware's use, separate from the classic 256 bytes in Cuda or PG&E (O'Hare and later)

    Genealogy order is Grand Central (60x) -> O'Hare (60x) -> Heathrow (G3) -> Paddington (G3) -> KeyLargo (G3/G4) -> K2 (G4/G5) -> Shasta (G5).
*/

#include "emu.h"
#include "heathrow.h"

#include "bus/rs232/rs232.h"
#include "formats/ap_dsk35.h"

#define LOG_IRQ     (1U << 1)
#define LOG_MACIO   (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

static constexpr u32 C7M  = 7833600;
static constexpr u32 C15M = (C7M * 2);
static constexpr u32 C31M = (C15M * 2);

static constexpr u32 DMA_IRQ_MASK = 0x000007ff;

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GRAND_CENTRAL, grandcentral_device, "grndctrl", "Apple Grand Central PCI I/O ASIC")
DEFINE_DEVICE_TYPE(OHARE, ohare_device, "ohare", "Apple O'Hare PCI I/O ASIC")
DEFINE_DEVICE_TYPE(HEATHROW, heathrow_device, "heathrow", "Apple Heathrow PCI I/O ASIC")
DEFINE_DEVICE_TYPE(PADDINGTON, paddington_device, "paddington", "Apple Paddington PCI I/O ASIC")

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------
/*          Grand Central           O'Hare                  Heathrow/Paddington
0x10000     SCSI0                   SCSI                    SCSI
0x11000     MACE Ethernet           (unused)                "BigMac" Ethernet
0x12000     SCC "compatibility"     SCC compat              SCC compat
0x13000     SCC "MacRisc"           SCC MacRisc             SCC MacRisc
0x14000     Audio                   Audio                   Audio
0x15000     SWIM3                   SWIM3                   SWIM3
0x16000     VIA                     VIA                     VIA
0x17000     VIA                     VIA                     VIA
0x18000     SCSI1                   (reads as 0)            (reads as 0)
0x19000     Ethernet MAC PROM       (reads as 0)            ADB Master Cell
0x1a000     (external IOBus)        (external IOBus)        (external IOBus)
0x1b000     (external IOBus)        (external IOBus)        (external IOBus)
0x1c000     (external IOBus)        (external IOBus)        (external IOBus)
0x1d000     (external IOBus)        (external IOBus)        (external IOBus)
0x1e000     (external IOBus)        (reads as 0)            (reads as 0)
0x1f000     (external IOBus)        (reads as 0)            (reads as 0)
0x20000     (128K BAR, no)          ATA bus 0               ATA bus 0
0x21000     (128K BAR, no)          ATA bus 1               ATA bus 1
0x60000     (128K BAR, no)          PRAM                    PRAM
0x70000     (128K BAR, no)          PRAM                    PRAM
*/
void macio_device::base_map(address_map &map)
{
	map(0x00000, 0x00fff).rw(FUNC(grandcentral_device::macio_r), FUNC(grandcentral_device::macio_w));
	map(0x08000, 0x0801f).m(m_dma_scsi0, FUNC(dbdma_device::map));
	map(0x08100, 0x0811f).m(m_dma_floppy, FUNC(dbdma_device::map));
	map(0x08400, 0x0841f).m(m_dma_sccatx, FUNC(dbdma_device::map));
	map(0x08500, 0x0851f).m(m_dma_sccarx, FUNC(dbdma_device::map));
	map(0x08600, 0x0861f).m(m_dma_sccbtx, FUNC(dbdma_device::map));
	map(0x08700, 0x0871f).m(m_dma_sccbrx, FUNC(dbdma_device::map));
	map(0x08800, 0x0881f).m(m_dma_audio_out, FUNC(dbdma_device::map));
	map(0x08900, 0x0891f).m(m_dma_audio_in, FUNC(dbdma_device::map));
	map(0x12000, 0x12fff).rw(FUNC(grandcentral_device::scc_r), FUNC(grandcentral_device::scc_w));
	map(0x13000, 0x13fff).rw(FUNC(grandcentral_device::scc_macrisc_r), FUNC(grandcentral_device::scc_macrisc_w));
	map(0x14000, 0x140ff).rw(FUNC(grandcentral_device::codec_r), FUNC(grandcentral_device::codec_w));
	map(0x15000, 0x15fff).rw(FUNC(grandcentral_device::fdc_r), FUNC(grandcentral_device::fdc_w));
	map(0x16000, 0x17fff).rw(FUNC(grandcentral_device::mac_via_r), FUNC(grandcentral_device::mac_via_w));
	map(0x1a000, 0x1afff).rw(FUNC(macio_device::iobus_r<&macio_device::read_iobus_a>), FUNC(macio_device::iobus_w<&macio_device::write_iobus_a>));
	map(0x1b000, 0x1bfff).rw(FUNC(macio_device::iobus_r<&macio_device::read_iobus_b>), FUNC(macio_device::iobus_w<&macio_device::write_iobus_b>));
	map(0x1c000, 0x1cfff).rw(FUNC(macio_device::iobus_r<&macio_device::read_iobus_c>), FUNC(macio_device::iobus_w<&macio_device::write_iobus_c>));
	map(0x1d000, 0x1dfff).rw(FUNC(macio_device::iobus_r<&macio_device::read_iobus_d>), FUNC(macio_device::iobus_w<&macio_device::write_iobus_d>));
}

void grandcentral_device::map(address_map &map)
{
	base_map(map);
	map(0x08a00, 0x08a1f).m(m_dma_scsi1, FUNC(dbdma_device::map));
	map(0x10000, 0x100ff).rw(FUNC(grandcentral_device::scsi0_r), FUNC(grandcentral_device::scsi0_w));
	map(0x18000, 0x180ff).rw(FUNC(grandcentral_device::scsi1_r), FUNC(grandcentral_device::scsi1_w));
	map(0x1e000, 0x1efff).rw(FUNC(grandcentral_device::iobus_r<&grandcentral_device::read_iobus_e>), FUNC(grandcentral_device::iobus_w<&grandcentral_device::write_iobus_e>));
	map(0x1f000, 0x1ffff).rw(FUNC(grandcentral_device::iobus_r<&grandcentral_device::read_iobus_f>), FUNC(grandcentral_device::iobus_w<&grandcentral_device::write_iobus_f>));
}

void ohare_device::map(address_map &map)
{
	base_map(map);
	map(0x08b00, 0x08b1f).m(m_dma_ata0, FUNC(dbdma_device::map));
	map(0x08c00, 0x08c1f).m(m_dma_ata1, FUNC(dbdma_device::map));
	map(0x10000, 0x100ff).rw(FUNC(ohare_device::mesh_r), FUNC(ohare_device::mesh_w));
	map(0x20000, 0x20fff).rw(FUNC(ohare_device::ata_r<0>), FUNC(ohare_device::ata_w<0>));
	map(0x21000, 0x21fff).rw(FUNC(ohare_device::ata_r<1>), FUNC(ohare_device::ata_w<1>));

	map(0x60000, 0x7ffff).rw(FUNC(ohare_device::nvram_r), FUNC(ohare_device::nvram_w)).umask32(0x000000ff);
}

void heathrow_device::map(address_map &map)
{
	ohare_device::map(map);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void macio_device::device_add_mconfig(machine_config &config)
{
	R65NC22(config, m_via1, C7M / 10);
	m_via1->readpa_handler().set(FUNC(macio_device::via_in_a));
	m_via1->readpb_handler().set(FUNC(macio_device::via_in_b));
	m_via1->writepb_handler().set(FUNC(macio_device::via_out_b));
	m_via1->cb2_handler().set(FUNC(macio_device::via_out_cb2));
	m_via1->irq_handler().set(FUNC(macio_device::set_irq_line<18>));

	DBDMA_CHANNEL(config, m_dma_scsi0, 0);
	m_dma_scsi0->set_width(1);
	m_dma_scsi0->irq_callback().set(FUNC(macio_device::set_irq_line<0>));

	DBDMA_CHANNEL(config, m_dma_floppy, 0);
	m_dma_floppy->set_width(1);
	m_dma_floppy->irq_callback().set(FUNC(macio_device::set_irq_line<1>));

	DBDMA_CHANNEL(config, m_dma_sccatx, 0);
	m_dma_sccatx->set_width(1);
	m_dma_sccatx->irq_callback().set(FUNC(macio_device::set_irq_line<4>));

	DBDMA_CHANNEL(config, m_dma_sccarx, 0);
	m_dma_sccarx->set_width(1);
	m_dma_sccarx->irq_callback().set(FUNC(macio_device::set_irq_line<5>));

	DBDMA_CHANNEL(config, m_dma_sccbtx, 0);
	m_dma_sccbtx->set_width(1);
	m_dma_sccbtx->irq_callback().set(FUNC(macio_device::set_irq_line<6>));

	DBDMA_CHANNEL(config, m_dma_sccbrx, 0);
	m_dma_sccbrx->set_width(1);
	m_dma_sccbrx->irq_callback().set(FUNC(macio_device::set_irq_line<7>));

	DBDMA_CHANNEL(config, m_dma_audio_out, 0);
	m_dma_audio_out->set_width(4);
	m_dma_audio_out->irq_callback().set(FUNC(macio_device::set_irq_line<8>));

	DBDMA_CHANNEL(config, m_dma_audio_in, 0);
	m_dma_audio_in->set_width(4);
	m_dma_audio_in->irq_callback().set(FUNC(macio_device::set_irq_line<9>));

	SWIM3(config, m_fdc, C31M);
	m_fdc->devsel_cb().set(FUNC(macio_device::devsel_w));
	m_fdc->phases_cb().set(FUNC(macio_device::phases_w));
	m_fdc->hdsel_cb().set(FUNC(macio_device::hdsel_w));
	m_fdc->irq_cb().set(FUNC(macio_device::set_irq_line<19>));
	m_fdc->drq_cb().set(m_dma_floppy, FUNC(dbdma_device::drq_w));
	m_dma_floppy->dma_r().set(m_fdc, FUNC(swim3_device::dma_r));
	m_dma_floppy->dma_w().set(m_fdc, FUNC(swim3_device::dma_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	SCC85C30(config, m_scc, C7M);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
	m_scc->out_int_callback().set(FUNC(macio_device::scc_irq_w));
	m_scc->out_txda_callback().set("printer", FUNC(rs232_port_device::write_txd));
	m_scc->out_txdb_callback().set("modem", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));
}

void grandcentral_device::device_add_mconfig(machine_config &config)
{
	macio_device::device_add_mconfig(config);

	m_dma_scsi0->set_width(2);
	m_dma_scsi0->dma_r().set(FUNC(grandcentral_device::scsi0_dma_r));
	m_dma_scsi0->dma_w().set(FUNC(grandcentral_device::scsi0_dma_w));

	DBDMA_CHANNEL(config, m_dma_scsi1, 0);
	m_dma_scsi1->set_width(2);
	m_dma_scsi1->irq_callback().set(FUNC(macio_device::set_irq_line<10>));
	m_dma_scsi1->dma_r().set(FUNC(grandcentral_device::scsi1_dma_r));
	m_dma_scsi1->dma_w().set(FUNC(grandcentral_device::scsi1_dma_w));
}

void ohare_device::device_add_mconfig(machine_config &config)
{
	macio_device::device_add_mconfig(config);

	DBDMA_CHANNEL(config, m_dma_ata0, 0);
	m_dma_ata0->set_width(2);
	m_dma_ata0->irq_callback().set(FUNC(macio_device::set_irq_line<2>));

	DBDMA_CHANNEL(config, m_dma_ata1, 0);
	m_dma_ata1->set_width(2);
	m_dma_ata1->irq_callback().set(FUNC(macio_device::set_irq_line<3>));

	// O'Hare and later have MESH on-chip.  We leave it to the driver to decide
	// what the SCSI devices are, if any.
	nscsi_bus_device &scsibus(NSCSI_BUS(config, "scsi"));
	APPLE_MESH_SCSI(config, m_mesh, 50_MHz_XTAL);
	scsibus.set_external_device(7, m_mesh);
	m_mesh->irq_handler_cb().set(FUNC(macio_device::scsi0_irq));
	m_mesh->drq_handler_cb().set(FUNC(macio_device::scsi0_drq));
	m_dma_scsi0->dma_r().set(m_mesh, FUNC(mesh_device::dma8_r));
	m_dma_scsi0->dma_w().set(m_mesh, FUNC(mesh_device::dma8_w));

	// The MESH's command done, exception, and error outputs show up in the SCSI
	// channel's ChannelStatus so a channel program can wait for a command to
	// finish and branch on how it went.  The lines are active low, so a bit reads
	// 1 while its condition is clear.
	//
	// The O'Hare/Heathrow ERS tables give s7 = MESHCmdDone_L, s6 = MESHException_L,
	// and s5 = MESHError_L, but that ordering is wrong: Apple's own driver source
	// (apple-oss-distributions/AppleMESH, mesh.cpp) sets up the channel with
	//     waitSelect   = 0x20002000;  /* Wait until command done      */
	//     branchSelect = 0xC000C000;  /* Br if Exc or Err             */
	// i.e. it waits on s5 for completion and branches on s6|s7 for trouble.
	// (Apple only ever tests exception and error together, so which of s6/s7 is
	// which is not determined; we keep the ERS's relative order for those two.)
	m_mesh->cmd_done_handler_cb().set([this](int state) { m_dma_scsi0->status_bit_w(5, !state); });
	m_mesh->exception_handler_cb().set([this](int state) { m_dma_scsi0->status_bit_w(6, !state); });
	m_mesh->error_handler_cb().set([this](int state) { m_dma_scsi0->status_bit_w(7, !state); });

	// O'Hare and later also have two ATA buses on-chip, and we also leave
	// device population to the driver.
	ATA_INTERFACE(config, m_ata[0]).options(ata_devices, nullptr, nullptr, false);
	m_ata[0]->irq_handler().set(FUNC(macio_device::set_irq_line<13>));
	m_ata[0]->dmarq_handler().set(FUNC(ohare_device::ata_dmarq<0>));
	m_dma_ata0->dma_r().set(m_ata[0], FUNC(ata_interface_device::read_dma));
	m_dma_ata0->dma_w().set(m_ata[0], FUNC(ata_interface_device::write_dma));

	ATA_INTERFACE(config, m_ata[1]).options(ata_devices, nullptr, nullptr, false);
	m_ata[1]->irq_handler().set(FUNC(macio_device::set_irq_line<14>));
	m_ata[1]->dmarq_handler().set(FUNC(ohare_device::ata_dmarq<1>));
	m_dma_ata1->dma_r().set(m_ata[1], FUNC(ata_interface_device::read_dma));
	m_dma_ata1->dma_w().set(m_ata[1], FUNC(ata_interface_device::write_dma));
}

void macio_device::config_map(address_map &map)
{
	pci_device::config_map(map);
}

//-------------------------------------------------
//  macio_device - constructor
//-------------------------------------------------

macio_device::macio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	pci_device(mconfig, type, tag, owner, clock),
	write_irq(*this),
	write_pb4(*this),
	write_pb5(*this),
	write_cb2(*this),
	read_pb3(*this, 0),
	read_codec(*this, 0),
	write_codec(*this),
	read_iobus_a(*this, 0),
	read_iobus_b(*this, 0),
	read_iobus_c(*this, 0),
	read_iobus_d(*this, 0),
	write_iobus_a(*this),
	write_iobus_b(*this),
	write_iobus_c(*this),
	write_iobus_d(*this),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_via1(*this, "via1"),
	m_fdc(*this, "fdc"),
	m_floppy(*this, "fdc:%d", 0U),
	m_scc(*this, "scc"),
	m_dma_scsi0(*this, "dma_scsi0"),
	m_dma_floppy(*this, "dma_floppy"),
	m_dma_sccatx(*this, "dma_scca_tx"),
	m_dma_sccarx(*this, "dma_scca_rx"),
	m_dma_sccbtx(*this, "dma_sccb_tx"),
	m_dma_sccbrx(*this, "dma_sccb_rx"),
	m_dma_audio_in(*this, "dma_audin"),
	m_dma_audio_out(*this, "dma_audout"),
	m_cur_floppy(nullptr),
	m_hdsel(0)
{
	m_toggle = 0;
}

grandcentral_device::grandcentral_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	macio_device(mconfig, GRAND_CENTRAL, tag, owner, clock),
	m_dma_scsi1(*this, "dma_scsi1"),
	read_scsi0(*this, 0),
	write_scsi0(*this),
	read_scsi0_dma(*this, 0),
	write_scsi0_dma(*this),
	read_scsi1(*this, 0),
	write_scsi1(*this),
	read_scsi1_dma(*this, 0),
	write_scsi1_dma(*this),
	read_iobus_e(*this, 0),
	read_iobus_f(*this, 0),
	write_iobus_e(*this),
	write_iobus_f(*this)
{
}

ohare_device::ohare_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	macio_device(mconfig, type, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_dma_ata0(*this, "dma_ata0"),
	m_dma_ata1(*this, "dma_ata1"),
	m_mesh(*this, "mesh"),
	m_ata(*this, "ata%u", 0U)
{
}

ohare_device::ohare_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ohare_device(mconfig, OHARE, tag, owner, clock)
{
}

heathrow_device::heathrow_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: ohare_device(mconfig, type, tag, owner, clock)
{
}

heathrow_device::heathrow_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ohare_device(mconfig, HEATHROW, tag, owner, clock)
{
}

paddington_device::paddington_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heathrow_device(mconfig, PADDINGTON, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void macio_device::common_init()
{
	pci_device::device_start();

	address_space *bm = get_pci_busmaster_space();
	m_dma_scsi0->set_address_space(bm);
	m_dma_floppy->set_address_space(bm);
	m_dma_sccatx->set_address_space(bm);
	m_dma_sccarx->set_address_space(bm);
	m_dma_sccbtx->set_address_space(bm);
	m_dma_sccbrx->set_address_space(bm);
	m_dma_audio_in->set_address_space(bm);
	m_dma_audio_out->set_address_space(bm);

	command = 2; // enable our memory range
	revision = 1;

	m_InterruptEvents = m_InterruptMask = m_InterruptLevels = 0;
	m_InterruptEvents2 = m_InterruptMask2 = m_InterruptLevels2 = 0;
	recalc_irqs();

	save_item(NAME(m_hdsel));
	save_item(NAME(m_InterruptEvents));
	save_item(NAME(m_InterruptMask));
	save_item(NAME(m_InterruptLevels));
	save_item(NAME(m_InterruptEvents2));
	save_item(NAME(m_InterruptMask2));
	save_item(NAME(m_InterruptLevels2));
}

void grandcentral_device::device_start()
{
	common_init();
	add_map(0x20000, M_MEM, FUNC(grandcentral_device::map));    // Grand Central only has 128K of BAR space, the others have 512K
	set_ids(0x106b0002, 0x02, 0xff0000, 0x000000);
	status = 0x0200;

	m_dma_scsi1->set_address_space(get_pci_busmaster_space());
}

void ohare_device::ohare_start()
{
	save_item(NAME(m_nvram));

	m_ata_config[0] = m_ata_config[1] = 0;
	save_item(NAME(m_ata_config));
	m_ata_selected[0] = m_ata_selected[1] = 0;
	save_item(NAME(m_ata_selected));

	m_dma_ata0->set_address_space(get_pci_busmaster_space());
	m_dma_ata1->set_address_space(get_pci_busmaster_space());
}

void ohare_device::device_start()
{
	common_init();
	add_map(0x80000, M_MEM, FUNC(grandcentral_device::map));
	set_ids(0x106b0007, 0x01, 0xff0000, 0x000000);
	ohare_start();
}

void heathrow_device::device_start()
{
	common_init();
	add_map(0x80000, M_MEM, FUNC(heathrow_device::map));
	set_ids(0x106b0010, 0x01, 0xff0000, 0x000000);
	ohare_start();
}

void paddington_device::device_start()
{
	common_init();
	add_map(0x80000, M_MEM, FUNC(heathrow_device::map));
	set_ids(0x106b0017, 0x01, 0xff0000, 0x000000);
	ohare_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void macio_device::device_reset()
{
	m_hdsel = 0;
}

u8 macio_device::via_in_a()
{
	return 0x80;
}

u8 macio_device::via_in_b()
{
	return read_pb3() << 3;
}

void macio_device::via_out_cb2(int state)
{
	write_cb2(state & 1);
}

void macio_device::hdsel_w(int hdsel)
{
	if (hdsel != m_hdsel)
	{
		if (m_cur_floppy)
		{
			m_cur_floppy->ss_w(hdsel);
		}
	}
	m_hdsel = hdsel;
}

void macio_device::via_out_b(u8 data)
{
	write_pb4(BIT(data, 4));
	write_pb5(BIT(data, 5));
}

void macio_device::scc_irq_w(int state)
{
	// TODO: our SCC only has one IRQ line for both channels.  (I think separate lines is a later ESCC thing?)
	set_irq_line<15>(state);
	set_irq_line<16>(state);
}

void macio_device::cb1_w(int state)
{
	m_via1->write_cb1(state);
}

void macio_device::cb2_w(int state)
{
	m_via1->write_cb2(state);
}

u16 macio_device::mac_via_r(offs_t offset)
{
	u16 data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		via_sync();

	data = m_via1->read(offset);

	return (data & 0xff) | (data << 8);
}

void macio_device::mac_via_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	via_sync();

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);
}

void macio_device::via_sync()
{
	// The via runs at 783.36KHz while the main cpu runs at 15MHz or
	// more, so we need to sync the access with the via clock.  Plus
	// the whole access takes half a (via) cycle and ends when synced
	// with the main cpu again.

	// Get the main cpu time
	u64 cycle = m_maincpu->total_cycles();

	// Get the number of the cycle the via is in at that time
	u64 via_cycle = cycle * m_via1->clock() / m_maincpu->clock();

	// The access is going to start at via_cycle+1 and end at
	// via_cycle+1.5, compute what that means in maincpu cycles (the
	// +1 rounds up, since the clocks are too different to ever be
	// synced).
	u64 main_cycle = (via_cycle * 2 + 3) * m_maincpu->clock() / (2 * m_via1->clock()) + 1;

	// Finally adjust the main cpu icount as needed.
	m_maincpu->adjust_icount(-int(main_cycle - cycle));
}

void macio_device::phases_w(u8 phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void macio_device::devsel_w(u8 devsel)
{
	if (devsel == 1)
		m_cur_floppy = m_floppy[0]->get_device();
	else if (devsel == 2)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;

	m_fdc->set_floppy(m_cur_floppy);
	if (m_cur_floppy)
		m_cur_floppy->ss_w(m_hdsel);
}

u32 macio_device::macio_r(offs_t offset)
{
	// InterruptLevels = live status of all interrupt lines
	// InterruptMask = mask to determine which bits of InterruptLevels matter
	// InterruptEvents = interrupts allowed to fire by InterruptMask
	LOGMASKED(LOG_MACIO, "macio_r: offset %x (%x)\n", offset, offset*4);
	switch (offset << 2)
	{
		case 0x10:
			return m_InterruptEvents2 & m_InterruptMask2;
		case 0x14:
			return m_InterruptMask2;
		case 0x1c:
			return m_InterruptLevels2;
		case 0x20:
			return m_InterruptEvents & m_InterruptMask;
		case 0x24:
			return m_InterruptMask;
		case 0x2c:
			return m_InterruptLevels;
	}
	return 0;
}

void macio_device::macio_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_MACIO, "macio_w: offset %x (%x) data %08x mask %08x\n", offset, offset*4, data, mem_mask);
	switch (offset << 2)
	{
		case 0x14:
			LOGMASKED(LOG_IRQ, "%s: %08x to InterruptMask2\n", tag(), data);
			m_InterruptMask2 = data;
			recalc_irqs();
			break;
		case 0x18:  // InterruptClear
			// which interrupt mode?
			if (BIT(m_InterruptMask2, 31))
			{
				// in Mode 1, "1" to bit 31 clears all active interrupts
				if (BIT(data, 31))
				{
					m_InterruptEvents2 = 0;
				}
				else
				{
					m_InterruptEvents2 &= (data ^ 0xffffffff);
				}
			}
			else
			{
				m_InterruptEvents2 &= (data ^ 0xffffffff);
			}
			recalc_irqs();
			break;
		case 0x24:
			LOGMASKED(LOG_IRQ, "%s: %08x to InterruptMask\n", tag(), data);
			m_InterruptMask = data;
			recalc_irqs();
			break;
		case 0x28:  // InterruptClear
			// which interrupt mode?
			if (BIT(m_InterruptMask, 31))
			{
				// in Mode 1, "1" to bit 31 clears all active interrupts
				if (BIT(data, 31))
				{
					m_InterruptEvents = 0;
				}
				else
				{
					m_InterruptEvents &= (data ^ 0xffffffff);
					// emulation mode: a 1->0 transition on a DBDMA channel
					// re-latches it, so fold the still-asserted levels back
					// into events before dropping them to re-arm the channel.
					const u32 dma_ack = data & DMA_IRQ_MASK;
					m_InterruptEvents |= (m_InterruptLevels & dma_ack);
					m_InterruptLevels &= ~dma_ack;
				}
			}
			else
			{
				m_InterruptEvents &= (data ^ 0xffffffff);
				// native mode: acknowledging a DBDMA channel's completion must
				// drop its held level so the next completion is seen as a fresh
				// rising edge (otherwise the channel can never interrupt again).
				m_InterruptLevels &= ~(data & DMA_IRQ_MASK);
			}
			recalc_irqs();
			break;
	}
}

void macio_device::recalc_irqs()
{
	// IRQs are raised by latched events, not live levels.
	if ((m_InterruptEvents & m_InterruptMask) || (m_InterruptEvents2 & m_InterruptMask2))
	{
		write_irq(ASSERT_LINE);
	}
	else
	{
		write_irq(CLEAR_LINE);
	}
}

template<int bit> void macio_device::set_irq_line(int state)
{
	if (bit < 32)
	{
		const u32 id = (1 << bit);
		// In emulation mode every transition latches an event.  In native mode
		// only a rising edge does.
		if ((m_InterruptMask & 0x80000000) || (state && !(m_InterruptLevels & id)))
		{
			m_InterruptEvents |= id;
		}
		else
		{
			m_InterruptEvents &= ~id;
		}

		if (state == ASSERT_LINE)
		{
			m_InterruptLevels |= id;
		}
		else
		{
			m_InterruptLevels &= ~id;
		}
	}
	else
	{
		const u32 id = (1 << (bit - 32));
		if ((m_InterruptMask2 & 0x80000000) || (state && !(m_InterruptLevels2 & id)))
		{
			m_InterruptEvents2 |= id;
		}
		else
		{
			m_InterruptEvents2 &= ~id;
		}

		if (state == ASSERT_LINE)
		{
			m_InterruptLevels2 |= id;
		}
		else
		{
			m_InterruptLevels2 &= ~id;
		}
	}
	recalc_irqs();
}

template void macio_device::set_irq_line<0>(int state);
template void macio_device::set_irq_line<1>(int state);
template void macio_device::set_irq_line<2>(int state);
template void macio_device::set_irq_line<3>(int state);
template void macio_device::set_irq_line<4>(int state);
template void macio_device::set_irq_line<5>(int state);
template void macio_device::set_irq_line<6>(int state);
template void macio_device::set_irq_line<7>(int state);
template void macio_device::set_irq_line<8>(int state);
template void macio_device::set_irq_line<9>(int state);
template void macio_device::set_irq_line<10>(int state);
template void macio_device::set_irq_line<11>(int state);
template void macio_device::set_irq_line<12>(int state);
template void macio_device::set_irq_line<13>(int state);
template void macio_device::set_irq_line<14>(int state);
template void macio_device::set_irq_line<15>(int state);
template void macio_device::set_irq_line<16>(int state);
template void macio_device::set_irq_line<17>(int state);
template void macio_device::set_irq_line<18>(int state);
template void macio_device::set_irq_line<19>(int state);
template void macio_device::set_irq_line<20>(int state);
template void macio_device::set_irq_line<21>(int state);
template void macio_device::set_irq_line<22>(int state);
template void macio_device::set_irq_line<23>(int state);
template void macio_device::set_irq_line<24>(int state);
template void macio_device::set_irq_line<25>(int state);
template void macio_device::set_irq_line<26>(int state);
template void macio_device::set_irq_line<27>(int state);
template void macio_device::set_irq_line<28>(int state);
template void macio_device::set_irq_line<29>(int state);
template void macio_device::set_irq_line<30>(int state);
template void macio_device::set_irq_line<31>(int state);

u8 macio_device::fdc_r(offs_t offset)
{
	return m_fdc->read(offset >> 4);
}

void macio_device::fdc_w(offs_t offset, u8 data)
{
	m_fdc->write(offset >> 4, data);
}

u16 macio_device::scc_r(offs_t offset)
{
	u16 result = m_scc->dc_ab_r(offset);
	return (result << 8) | result;
}

void macio_device::scc_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_scc->dc_ab_w(offset, data & 0xff);
	}
	else
	{
		m_scc->dc_ab_w(offset, data >> 8);
	}
}

u8 macio_device::scc_macrisc_r(offs_t offset)
{
	switch ((offset >> 4) & 0xf)
	{
		case 0:
			return m_scc->cb_r(0);

		case 1:
			return m_scc->db_r(0);

		case 2:
			return m_scc->ca_r(0);

		case 3:
			return m_scc->da_r(0);
	}
	return 0;
}

void macio_device::scc_macrisc_w(offs_t offset, u8 data)
{
	switch ((offset >> 4) & 0xf)
	{
		case 0:
			return m_scc->cb_w(0, data);

		case 1:
			return m_scc->db_w(0, data);

		case 2:
			return m_scc->ca_w(0, data);

		case 3:
			return m_scc->da_w(0, data);
	}
}

// O'Hare and later have a MESH SCSI controller on-chip
u8 ohare_device::mesh_r(offs_t offset)
{
	return m_mesh->read(offset >> 4);
}

void ohare_device::mesh_w(offs_t offset, u8 data)
{
	m_mesh->write(offset >> 4, data);
}

template <int Ch> u32 ohare_device::ata_r(offs_t offset, u32 mem_mask)
{
	ata_interface_device &ata = *m_ata[Ch];

	switch ((offset >> 2) & 0xff)
	{
		case 0x00:  // data register (16- or 32-bit PIO)
			if (ACCESSING_BITS_16_31)
			{
				const u16 w0 = ata.cs0_r(0);
				const u16 w1 = ata.cs0_r(0);
				return w0 | (u32(w1) << 16);
			}
			return ata.cs0_r(0, mem_mask & 0xffff);

		case 0x01: case 0x02: case 0x03: case 0x04:
		case 0x05: case 0x06: case 0x07:    // command block, byte-wide
		{
			const u8 value = ata.cs0_r((offset >> 2) & 7, 0xff);
			// Apple has a weak pull-down on DB7 so devices that aren't present
			// read as 0x7f
			return ata.slot(m_ata_selected[Ch]).dev() ? value : 0x7f;
		}

		case 0x16:  // control block: alternate status
		{
			const u8 value = ata.cs1_r(6, 0xff);
			return ata.slot(m_ata_selected[Ch]).dev() ? value : 0x7f;
		}

		case 0x20:  // Apple timing configuration register
			return m_ata_config[Ch];
	}

	return 0;
}

template <int Ch> void ohare_device::ata_w(offs_t offset, u32 data, u32 mem_mask)
{
	ata_interface_device &ata = *m_ata[Ch];

	switch ((offset >> 2) & 0xff)
	{
		case 0x00:  // data register (16- or 32-bit PIO)
			if (ACCESSING_BITS_16_31)
			{
				ata.cs0_w(0, data & 0xffff);
				ata.cs0_w(0, (data >> 16) & 0xffff);
			}
			else
			{
				ata.cs0_w(0, data & 0xffff, mem_mask & 0xffff);
			}
			break;

		case 0x01: case 0x02: case 0x03: case 0x04:
		case 0x05: case 0x06: case 0x07:    // command block, byte-wide
			if (((offset >> 2) & 7) == 6)
			{
				m_ata_selected[Ch] = BIT(data, 4);
			}
			ata.cs0_w((offset >> 2) & 7, data & 0xff, 0xff);
			break;

		case 0x16:  // control block: device control
			ata.cs1_w(6, data & 0xff, 0xff);
			break;

		case 0x20:  // Apple timing configuration register
			m_ata_config[Ch] = data;
			break;
	}
}

template <int Ch> void ohare_device::ata_dmarq(int state)
{
	dbdma_device &dma = *(Ch ? m_dma_ata1 : m_dma_ata0);

	if (state)
		m_ata[Ch]->write_dmack(ASSERT_LINE);

	dma.drq_w(state);

	if (!state)
		m_ata[Ch]->write_dmack(CLEAR_LINE);
}

// O'Hare and later OF NVRAM support
u8 ohare_device::nvram_r(offs_t offset)
{
	return m_nvram[offset>>2];
}

void ohare_device::nvram_w(offs_t offset, u8 data)
{
	m_nvram[offset>>2] = data;
	if ((offset>>2) > 0x7fff)
	{
		fatalerror("%s: NVRAM write out of bounds @ %x", tag(), offset>>2);
	}
}

void ohare_device::nvram_default()
{
	std::fill(std::begin(m_nvram), std::end(m_nvram), 0);
}

bool ohare_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_nvram, 0x8000);
	if (!err && (actual == 0x8000))
	{
		return true;
	}
	return false;
}

bool ohare_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_nvram, 0x8000);
	return !err;
}

// Audio support
uint32_t macio_device::codec_r(offs_t offset, uint32_t mem_mask)
{
	return read_codec(offset);
}

void macio_device::codec_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	write_codec(offset, data);
}

u32 macio_device::codec_dma_read(offs_t offset)
{
	return m_dma_audio_out->dma_read(offset);
}

void macio_device::codec_dma_write(offs_t offset, u32 data)
{
	m_dma_audio_in->dma_write(offset, data);
}

void macio_device::scsi0_drq(int state)
{
	m_dma_scsi0->drq_w(state);
}

template <devcb_read32 macio_device::*R> u32 macio_device::iobus_r(offs_t offset, u32 mem_mask)
{
	return (this->*R)(offset, mem_mask);
}

template <devcb_write32 macio_device::*W> void macio_device::iobus_w(offs_t offset, u32 data, u32 mem_mask)
{
	(this->*W)(offset, data, mem_mask);
}

template <devcb_read32 grandcentral_device::*R> u32 grandcentral_device::iobus_r(offs_t offset, u32 mem_mask)
{
	return (this->*R)(offset, mem_mask);
}

template <devcb_write32 grandcentral_device::*W> void grandcentral_device::iobus_w(offs_t offset, u32 data, u32 mem_mask)
{
	(this->*W)(offset, data, mem_mask);
}

u8 grandcentral_device::scsi0_r(offs_t offset)
{
	return read_scsi0(offset >> 4);
}

void grandcentral_device::scsi0_w(offs_t offset, u8 data)
{
	write_scsi0(offset >> 4, data);
}

u32 grandcentral_device::scsi0_dma_r(offs_t offset)
{
	return read_scsi0_dma(offset);
}

void grandcentral_device::scsi0_dma_w(offs_t offset, u32 data)
{
	write_scsi0_dma(offset, data);
}

u8 grandcentral_device::scsi1_r(offs_t offset)
{
	return read_scsi1(offset >> 4);
}

void grandcentral_device::scsi1_w(offs_t offset, u8 data)
{
	write_scsi1(offset >> 4, data);
}

u32 grandcentral_device::scsi1_dma_r(offs_t offset)
{
	return read_scsi1_dma(offset);
}

void grandcentral_device::scsi1_dma_w(offs_t offset, u32 data)
{
	write_scsi1_dma(offset, data);
}
