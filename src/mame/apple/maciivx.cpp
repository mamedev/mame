// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    maciivx.cpp
    Mac IIvx
    Mac IIvi

    By R. Belmont

    These 68030 machines were the last Mac IIs, and had development rushed
    after then-CEO John Sculley told MacWorld Tokyo that Apple would soon
    ship machines with a built-in CD-ROM drive.

    They run on the "VASP" system ASIC, which is basically V8 with slightly
    different video and the RAM size limit lifted to 68 MB.

****************************************************************************/

#include "emu.h"

#include "dfac.h"
#include "egret.h"
#include "macadb.h"
#include "macscsi.h"
#include "mactoolbox.h"
#include "vasp.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/nubus/nubus.h"
#include "bus/nubus/cards.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68030.h"
#include "machine/applefdintf.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "machine/ram.h"
#include "machine/swim1.h"
#include "machine/timer.h"
#include "machine/z80scc.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

namespace {

#define C32M    (31.3344_MHz_XTAL)
#define C15M    (C32M/2)
#define C7M     (C32M/4)

class maciivx_state : public driver_device
{
public:
	maciivx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_macadb(*this, "macadb"),
		m_ram(*this, RAM_TAG),
		m_vasp(*this, "vasp"),
		m_dfac(*this, "dfac"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_scsibus1(*this, "scsi"),
		m_ncr5380(*this, "scsi:7:ncr5380"),
		m_scsihelp(*this, "scsihelp"),
		m_scc(*this, "scc"),
		m_egret(*this, "egret"),
		m_cur_floppy(nullptr),
		m_hdsel(0)
	{
	}

	void maciiv_base(machine_config &config);
	void maciivx(machine_config &config);
	void maciivi(machine_config &config);
	void base_map(address_map &map) ATTR_COLD;
	void maciivx_map(address_map &map) ATTR_COLD;
	void maciivi_map(address_map &map) ATTR_COLD;

private:
	required_device<m68030_device> m_maincpu;
	required_device<macadb_device> m_macadb;
	required_device<ram_device> m_ram;
	required_device<vasp_device> m_vasp;
	required_device<dfac_device> m_dfac;
	required_device<applefdintf_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<nscsi_bus_device> m_scsibus1;
	required_device<ncr5380_device> m_ncr5380;
	required_device<mac_scsi_helper_device> m_scsihelp;
	required_device<z80scc_device> m_scc;
	required_device<egret_device> m_egret;

	virtual void machine_start() override ATTR_COLD;

	u16 scc_r(offs_t offset)
	{
		u16 result = m_scc->dc_ab_r(offset);
		return (result << 8) | result;
	}
	void scc_w(offs_t offset, u16 data)
	{
		m_scc->dc_ab_w(offset, data >> 8);
	}

	u16 scsi_r(offs_t offset, u16 mem_mask = ~0);
	void scsi_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u32 scsi_drq_r(offs_t offset, u32 mem_mask = ~0);
	void scsi_drq_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void scsi_berr_w(u8 data)
	{
		m_maincpu->pulse_input_line(M68K_LINE_BUSERROR, attotime::zero);
	}

	void egret_reset_w(int state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	}

	floppy_image_device *m_cur_floppy = nullptr;
	int m_hdsel;

	void phases_w(uint8_t phases);
	void devsel_w(uint8_t devsel);
	uint16_t swim_r(offs_t offset, u16 mem_mask);
	void swim_w(offs_t offset, u16 data, u16 mem_mask);
	void hdsel_w(int state);
};

void maciivx_state::machine_start()
{
	m_vasp->set_ram_info((u32 *) m_ram->pointer(), m_ram->size());

	save_item(NAME(m_hdsel));
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/
void maciivx_state::base_map(address_map &map)
{
	// RAM, ROM, and base I/O mappings come from VASP
	map(0x40000000, 0x600fffff).m(m_vasp, FUNC(vasp_device::map));

	map(0x50004000, 0x50005fff).rw(FUNC(maciivx_state::scc_r), FUNC(maciivx_state::scc_w)).mirror(0x00f00000);
	map(0x50006000, 0x50007fff).rw(FUNC(maciivx_state::scsi_drq_r), FUNC(maciivx_state::scsi_drq_w)).mirror(0x00f00000);
	map(0x50010000, 0x50011fff).rw(FUNC(maciivx_state::scsi_r), FUNC(maciivx_state::scsi_w)).mirror(0x00f00000);
	map(0x50012000, 0x50013fff).rw(FUNC(maciivx_state::scsi_drq_r), FUNC(maciivx_state::scsi_drq_w)).mirror(0x00f00000);
	map(0x50016000, 0x50017fff).rw(FUNC(maciivx_state::swim_r), FUNC(maciivx_state::swim_w)).mirror(0x00f00000);
}

void maciivx_state::maciivx_map(address_map &map)
{
	base_map(map);
	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a2015; }));
}

void maciivx_state::maciivi_map(address_map &map)
{
	base_map(map);
	map(0x5ffffffc, 0x5fffffff).lr32(NAME([](offs_t offset) { return 0xa55a2016; }));
}

u16 maciivx_state::scsi_r(offs_t offset, u16 mem_mask)
{
	const int reg = (offset >> 3) & 0xf;
	const bool pseudo_dma = (reg == 6) && (offset == 0x130);

	return m_scsihelp->read_wrapper(pseudo_dma, reg) << 8;
}

void maciivx_state::scsi_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int reg = (offset >> 3) & 0xf;
	const bool pseudo_dma = (reg == 0) && (offset == 0x100);

	m_scsihelp->write_wrapper(pseudo_dma, reg, data >> 8);
}

u32 maciivx_state::scsi_drq_r(offs_t offset, u32 mem_mask)
{
	switch (mem_mask)
	{
	case 0xff000000:
		return m_scsihelp->read_wrapper(true, 6) << 24;

	case 0xffff0000:
		return (m_scsihelp->read_wrapper(true, 6) << 24) | (m_scsihelp->read_wrapper(true, 6) << 16);

	case 0xffffffff:
		return (m_scsihelp->read_wrapper(true, 6) << 24) | (m_scsihelp->read_wrapper(true, 6) << 16) | (m_scsihelp->read_wrapper(true, 6) << 8) | m_scsihelp->read_wrapper(true, 6);

	default:
		logerror("scsi_drq_r: unknown mem_mask %08x\n", mem_mask);
	}

	return 0;
}

void maciivx_state::scsi_drq_w(offs_t offset, u32 data, u32 mem_mask)
{
	switch (mem_mask)
	{
	case 0xff000000:
		m_scsihelp->write_wrapper(true, 0, data >> 24);
		break;

	case 0xffff0000:
		m_scsihelp->write_wrapper(true, 0, data >> 24);
		m_scsihelp->write_wrapper(true, 0, data >> 16);
		break;

	case 0xffffffff:
		m_scsihelp->write_wrapper(true, 0, data >> 24);
		m_scsihelp->write_wrapper(true, 0, data >> 16);
		m_scsihelp->write_wrapper(true, 0, data >> 8);
		m_scsihelp->write_wrapper(true, 0, data & 0xff);
		break;

	default:
		logerror("scsi_drq_w: unknown mem_mask %08x\n", mem_mask);
		break;
	}
}

uint16_t maciivx_state::swim_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu->adjust_icount(-5);
	}

	u16 result = m_fdc->read((offset >> 8) & 0xf);
	return result << 8;
}
void maciivx_state::swim_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_fdc->write((offset >> 8) & 0xf, data & 0xff);
	else
		m_fdc->write((offset >> 8) & 0xf, data >> 8);
}

void maciivx_state::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void maciivx_state::devsel_w(uint8_t devsel)
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

void maciivx_state::hdsel_w(int state)
{
	if (state != m_hdsel)
	{
		if (m_cur_floppy)
		{
			m_cur_floppy->ss_w(state);
		}
	}
	m_hdsel = state;
}

/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

static INPUT_PORTS_START( maciivx )
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void maciivx_state::maciiv_base(machine_config &config)
{
	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,16M,32M,36M,48M,64M,68M");

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config(
		[](device_t *device)
		{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^lspeaker", 1.0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^rspeaker", 1.0);
		});
	NSCSI_CONNECTOR(config, "scsi:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR53C80).machine_config([this](device_t *device)
	{
		ncr53c80_device &adapter = downcast<ncr53c80_device &>(*device);
		adapter.drq_handler().set(m_scsihelp, FUNC(mac_scsi_helper_device::drq_w));
	});

	MAC_SCSI_HELPER(config, m_scsihelp);
	m_scsihelp->scsi_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::read));
	m_scsihelp->scsi_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::write));
	m_scsihelp->scsi_dma_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_r));
	m_scsihelp->scsi_dma_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_w));
	m_scsihelp->cpu_halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_scsihelp->timeout_error_callback().set(FUNC(maciivx_state::scsi_berr_w));

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("MC68030,MC68030_32");
	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");

	SCC85C30(config, m_scc, C7M);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
	m_scc->out_int_callback().set(m_vasp, FUNC(vasp_device::scc_irq_w));
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

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	APPLE_DFAC(config, m_dfac, 22257);
	m_dfac->add_route(0, "lspeaker", 1.0);
	m_dfac->add_route(1, "rspeaker", 1.0);

	VASP(config, m_vasp, C15M);
	m_vasp->set_maincpu_tag("maincpu");
	m_vasp->set_rom_tag("bootrom");
	m_vasp->hdsel_callback().set(FUNC(maciivx_state::hdsel_w));
	m_vasp->add_route(0, m_dfac, 1.0);
	m_vasp->add_route(1, m_dfac, 1.0);

	MACADB(config, m_macadb, C15M);

	nubus_device &nubus(NUBUS(config, "nubus", 0));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irqc_callback().set(m_vasp, FUNC(vasp_device::slot0_irq_w));
	nubus.out_irqd_callback().set(m_vasp, FUNC(vasp_device::slot1_irq_w));
	nubus.out_irqe_callback().set(m_vasp, FUNC(vasp_device::slot2_irq_w));

	NUBUS_SLOT(config, "nbc", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbd", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbe", "nubus", mac_nubus_cards, nullptr);

	SWIM1(config, m_fdc, C15M);
	m_fdc->devsel_cb().set(FUNC(maciivx_state::devsel_w));
	m_fdc->phases_cb().set(FUNC(maciivx_state::phases_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);
}

void maciivx_state::maciivx(machine_config &config)
{
	M68030(config, m_maincpu, C32M);
	m_maincpu->set_addrmap(AS_PROGRAM, &maciivx_state::maciivx_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	maciiv_base(config);

	EGRET(config, m_egret, XTAL(32'768));
	m_egret->set_default_bios_tag("341s0851");
	m_egret->reset_callback().set(FUNC(maciivx_state::egret_reset_w));
	m_egret->dfac_scl_callback().set(m_dfac, FUNC(dfac_device::clock_write));
	m_egret->dfac_sda_callback().set(m_dfac, FUNC(dfac_device::data_write));
	m_egret->dfac_latch_callback().set(m_dfac, FUNC(dfac_device::latch_write));
	m_egret->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_egret->via_clock_callback().set(m_vasp, FUNC(vasp_device::cb1_w));
	m_egret->via_data_callback().set(m_vasp, FUNC(vasp_device::cb2_w));
	m_macadb->adb_data_callback().set(m_egret, FUNC(egret_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);

	m_vasp->pb3_callback().set(m_egret, FUNC(egret_device::get_xcvr_session));
	m_vasp->pb4_callback().set(m_egret, FUNC(egret_device::set_via_full));
	m_vasp->pb5_callback().set(m_egret, FUNC(egret_device::set_sys_session));
	m_vasp->cb2_callback().set(m_egret, FUNC(egret_device::set_via_data));
}

void maciivx_state::maciivi(machine_config &config)
{
	maciivx(config);

	M68030(config.replace(), m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &maciivx_state::maciivi_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");
}

ROM_START(maciivx)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("4957eb49.rom", 0x000000, 0x100000, CRC(61be06e5) SHA1(560ce203d65178657ad09d03f532f86fa512bb40))
ROM_END

ROM_START(maciivi)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("4957eb49.rom", 0x000000, 0x100000, CRC(61be06e5) SHA1(560ce203d65178657ad09d03f532f86fa512bb40))
ROM_END

}   // anonymous namespace

COMP(1993, maciivx, 0,       0, maciivx, maciivx, maciivx_state, empty_init, "Apple Computer", "Macintosh IIvx", MACHINE_SUPPORTS_SAVE)
COMP(1993, maciivi, maciivx, 0, maciivi, maciivx, maciivx_state, empty_init, "Apple Computer", "Macintosh IIvi", MACHINE_SUPPORTS_SAVE)
