// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    Mac Quadra 660AV ("Tempest")
    Mac Quadra 840AV ("Cyclone")
    Emulation by R. Belmont

    These machines were a substantial step forward in multimedia and a bridge
    to the first-generation Power Macintosh.

    Major changes from previous high-end Quadras include true DMA support for
    the 53C94 SCSI, the 85C30 ESCC, the new Singer audio CODEC, the uPD72070
    "New Age" floppy controller, and the "MACE" Ethernet controller.

    16-bit stereo sound input and output are supported as is video input
    from NTSC, PAL, and SECAM sources.

    TODO:
    - DSP3210 core
    - Probably other things

****************************************************************************/

#include "emu.h"

#include "civic.h"
#include "cuda.h"
#include "mactoolbox.h"
#include "psc.h"
#include "ymca.h"

#include "bus/adb/adb.h"
#include "bus/adb/cards.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/nubus/cards.h"
#include "bus/nubus/nubus.h"
#include "bus/rs232/rs232.h"
#include "cpu/dsp32/dsp32.h"
#include "cpu/m68000/m68040.h"
#include "machine/am79c940.h"
#include "machine/ncr53c90.h"
#include "machine/nscsi_bus.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/z80scc.h"

#include "softlist_dev.h"

constexpr auto C32M = 31.3344_MHz_XTAL;
constexpr auto C15M = 15.6672_MHz_XTAL;
constexpr auto C7M  = C15M / 2;

namespace {

class quadraav_state : public driver_device
{
public:
	quadraav_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ymca(*this, "ymca"),
		m_psc(*this, "psc"),
		m_mace(*this, "mace"),
		m_civic(*this, "civic"),
		m_adbbus(*this, "adb"),
		m_cuda(*this, "cuda"),
		m_scc(*this, "scc"),
		m_ram(*this, RAM_TAG),
		m_scsibus(*this, "scsi"),
		m_ncr(*this, "ncr53c94"),
		m_enet_prom{},
		m_enet_prom_initialized(false)
	{
	}

	void macqd660(machine_config &config);
	void macqd840(machine_config &config);

	void quadraav_map(address_map &map);

private:
	required_device<m68040_device> m_maincpu;
	required_device<ymca_device> m_ymca;
	required_device<psc_device> m_psc;
	required_device<am79c940_device> m_mace;
	required_device<civic_device> m_civic;
	required_device<adb_bus_device> m_adbbus;
	required_device<cuda_device> m_cuda;
	required_device<z80scc_device> m_scc;
	required_device<ram_device> m_ram;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c94_device> m_ncr;
	std::array<u8, 8> m_enet_prom;
	bool m_enet_prom_initialized;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_reset() override;

	u16 scc_r(offs_t offset);
	void scc_w(offs_t offset, u16 data);
	void cuda_reset_w(int state);
	u8 scsi_r(offs_t offset);
	void scsi_w(offs_t offset, u8 data);
	u8 enet_prom_r(offs_t offset);
};

void quadraav_state::machine_start()
{
	m_ymca->set_ram_info((u32 *) m_ram->pointer(), m_ram->size());
	m_psc->set_scsi_device(m_ncr);
	// Use the same Apple OUI convention as the NuBus Ethernet cards.  The
	// configuration manager persists this address and allows user overrides.
	const u32 suffix = machine().rand();
	const u8 mac[6] = { 0x00, 0x00, 0x1b, u8(suffix >> 16), u8(suffix >> 8), u8(suffix) };
	m_mace->set_mac(mac);
	save_item(NAME(m_enet_prom));
	save_item(NAME(m_enet_prom_initialized));
}

void quadraav_state::device_reset()
{
	if (!m_enet_prom_initialized)
	{
		// Configuration MAC overrides have been applied by the first reset.
		// Capture before child reset: MACE then publishes its uninitialised PADR.
		// The board PROM remains independent of subsequent PADR writes.
		std::copy(m_mace->get_mac().begin(), m_mace->get_mac().end(), m_enet_prom.begin());
		m_enet_prom[7] = 0xff;
		for (unsigned i = 0; i < 6; ++i)
			m_enet_prom[7] ^= m_enet_prom[i];
		m_enet_prom_initialized = true;
	}
}

void quadraav_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

u16 quadraav_state::scc_r(offs_t offset)
{
	m_psc->via_sync();
	u16 result = m_scc->dc_ab_r(offset);
	return (result << 8) | result;
}

void quadraav_state::scc_w(offs_t offset, u16 data)
{
	m_psc->via_sync();
	m_scc->dc_ab_w(offset, data >> 8);
}

void quadraav_state::cuda_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}

u8 quadraav_state::scsi_r(offs_t offset)
{
	return m_ncr->read(offset >> 4);
}

void quadraav_state::scsi_w(offs_t offset, u8 data)
{
	m_ncr->write(offset >> 4, data);
}

u8 quadraav_state::enet_prom_r(offs_t offset)
{
	// Eight bit-reversed bytes, 16 bytes apart; the decoded bytes XOR to FF.
	const unsigned index = (offset >> 4) & 7;
	return bitswap<8>(m_enet_prom[index], 0, 1, 2, 3, 4, 5, 6, 7);
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/
void quadraav_state::quadraav_map(address_map &map)
{
	map(0x0000'0000, 0xffff'ffff).m(m_ymca, FUNC(ymca_device::map));
	map(0x5000'0000, 0x5fff'ffff).m(m_psc, FUNC(psc_device::map));
	map(0x5000'0000, 0x5fff'ffff).m(m_civic, FUNC(civic_device::map));

	map(0x50f04000, 0x50f05fff).rw(FUNC(quadraav_state::scc_r), FUNC(quadraav_state::scc_w));
	map(0x50f08000, 0x50f0807f).r(FUNC(quadraav_state::enet_prom_r));
	map(0x50f18000, 0x50f180ff).rw(FUNC(quadraav_state::scsi_r), FUNC(quadraav_state::scsi_w));
	map(0x50f1c000, 0x50f1c1ff).lrw8(
		NAME([this](offs_t offset) { return m_mace->read(offset >> 4); }),
		NAME([this](offs_t offset, u8 data) { m_mace->write(offset >> 4, data); }));
}

/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

static INPUT_PORTS_START( macqdav )
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void quadraav_state::macqd840(machine_config &config)
{
	M68040(config, m_maincpu, 40_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &quadraav_state::quadraav_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	YMCA(config, m_ymca, 25_MHz_XTAL);
	m_ymca->set_cpu_id(0xf);
	m_ymca->set_maincpu_tag("maincpu");
	m_ymca->set_rom_tag("bootrom");
	m_ymca->write_ntscpalsel().set(m_civic, FUNC(civic_device::clock_select_w));

	PSC(config, m_psc, 25_MHz_XTAL);
	m_psc->set_maincpu_tag("maincpu");
	m_psc->set_space("maincpu", AS_PROGRAM);
	m_psc->set_mace_tag("mace");

	AM79C940(config, m_mace, 0);
	m_mace->irq_out().set(m_psc, FUNC(psc_device::enet_irq_w));
	m_mace->tx_drq_out().set(m_psc, FUNC(psc_device::enet_tx_drq_w));
	m_mace->rx_drq_out().set(m_psc, FUNC(psc_device::enet_rx_drq_w));

	CIVIC(config, m_civic, 40_MHz_XTAL);
	m_civic->vblank_irq().set(m_psc, FUNC(psc_device::vbl_irq_w));

	SCC85C30(config, m_scc, C7M);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
//  m_scc->out_int_callback().set(m_psc, FUNC(psc_device::scc_irq_w));
	m_scc->out_txda_callback().set("modem", FUNC(rs232_port_device::write_txd));
	m_scc->out_txdb_callback().set("printer", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));

	// SCSI bus and devices
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config([](device_t *device)
																							{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^speaker", 1.0, 0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^speaker", 1.0, 1); });
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);

	SPEAKER(config, "speaker", 2).front();

	NCR53C94(config, m_ncr, 25_MHz_XTAL);
	m_scsibus->set_external_device(7, m_ncr);
	m_ncr->set_busmd(ncr53c96_device::BUSMD_3);
	m_ncr->irq_handler_cb().set(m_psc, FUNC(psc_device::scsi_irq_w));
	m_ncr->drq_handler_cb().set(m_psc, FUNC(psc_device::scsi_drq_w));

	ADB_BUS(config, m_adbbus);
	ADB_CONNECTOR(config, "adb:0", adb_devices, "hle_keyboard");
	ADB_CONNECTOR(config, "adb:1", adb_devices, "hle_mouse");

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0788");
	m_cuda->reset_callback().set(FUNC(quadraav_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_adbbus, FUNC(adb_bus_device::adb_host_line_w));
	m_cuda->via_clock_callback().set(m_psc, FUNC(psc_device::cb1_w));
	m_cuda->via_data_callback().set(m_psc, FUNC(psc_device::cb2_w));
	m_cuda->nmi_callback().set_inputline(m_maincpu, M68K_IRQ_7);
	m_adbbus->out_adb_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	m_adbbus->out_poweron_callback().set(m_cuda, FUNC(cuda_device::set_adb_power));
	config.set_perfect_quantum(m_maincpu);

	m_psc->pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	m_psc->pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	m_psc->pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	m_psc->write_cb2().set(m_cuda, FUNC(cuda_device::set_via_data));

//  nubus_device &nubus(NUBUS(config, "nubus", 0));
//  nubus.set_space(m_maincpu, AS_PROGRAM);
//  nubus.out_irqe_callback().set(m_psc, FUNC(psc_device::via2_irq_w<0x20>));
//  NUBUS_SLOT(config, "lcpds", "pds", mac_pdslc_cards, nullptr);

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("8M");
	m_ram->set_extra_options("16M,32M,64M,96M,128M,192M,256M,320M,384M,512M,640M");

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("MC68040");
}

void quadraav_state::macqd660(machine_config &config)
{
	macqd840(config);

	// Set the machine ID.
	m_ymca->set_cpu_id(0xb);

	// The 660AV uses a completely different clock generator for ? reason.
	m_civic->use_icd_clockgen();
}

ROM_START( macqd840av )
	ROM_REGION32_BE(0x200000, "bootrom", 0)
	ROM_SYSTEM_BIOS(0, "default", "Version 10F3")
	ROMX_LOAD( "5bf10fd1.bin", 0x000000, 0x200000, CRC(6973886c) SHA1(532d8cb9c928eb6f3c264561b17bee9a02aad9bb), ROM_BIOS(0) )

	// Chime plays incorrectly and no system boots; all AV enablers check explicitly for 1.0F3.
	// Yet this came from hardware, didn't it?
	ROM_SYSTEM_BIOS(1, "newer", "Version 10F5")
	ROMX_LOAD("87d3c814.bin", 0x000000, 0x200000, CRC(3c20c24f) SHA1(d372bf2edff36cf510ab9f6280fe7ed0773c430e), ROM_BIOS(1))
ROM_END

#define rom_macqd660av rom_macqd840av

} // anonymous namespace

COMP( 1993, macqd840av, 0, 0, macqd840, macqdav, quadraav_state, empty_init,  "Apple Computer", "Macintosh Quadra 840AV", MACHINE_NOT_WORKING)
COMP( 1993, macqd660av, macqd840av, 0, macqd660, macqdav, quadraav_state, empty_init,  "Apple Computer", "Macintosh Quadra 660AV", MACHINE_NOT_WORKING)
