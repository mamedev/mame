// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    maclc.cpp
    Mac LC, LC II, Classic II, Color Classic, Macintosh TV
    By R. Belmont

    These are all lower-end machines based on versions of the "V8" system
    controller, which has a 10 MB hard limit on RAM (8MB in the Mac TV).

    Mac TV video input chips:
    TEA6330T - Sound fader control unit for car stereos
        I2C: address 1000000x
    TDA8708BT - Video analog input interface
    SAA7197 T - Clock signal generator circuit for desktop video systems
    SAA7191 WP - Digital multistandard colour decoder
        I2C: address 1000101x
    SAA7186 H - Digital video scaler
        I2C: address 1011100x

****************************************************************************/

#include "emu.h"

#include "cuda.h"
#include "dfac.h"
#include "egret.h"
#include "macadb.h"
#include "macscsi.h"
#include "mactoolbox.h"
#include "v8.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/nubus/cards.h"
#include "bus/nubus/nubus.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68020.h"
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

class maclc_state : public driver_device
{
public:
	maclc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_macadb(*this, "macadb"),
		m_ram(*this, RAM_TAG),
		m_v8(*this, "v8"),
		m_dfac(*this, "dfac"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_scsibus1(*this, "scsi"),
		m_ncr5380(*this, "scsi:7:ncr5380"),
		m_scsihelp(*this, "scsihelp"),
		m_scc(*this, "scc"),
		m_egret(*this, "egret"),
		m_cuda(*this, "cuda"),
		m_cur_floppy(nullptr),
		m_hdsel(0)
	{
	}

	void maclc_base(machine_config &config);
	void maclc(machine_config &config);
	void maclc2(machine_config &config);
	void macclas2(machine_config &config);
	void maccclas(machine_config &config);
	void mactv(machine_config &config);
	void maclc_map(address_map &map);
	void maccclassic_map(address_map &map);

private:
	required_device<m68000_musashi_device> m_maincpu;
	required_device<macadb_device> m_macadb;
	required_device<ram_device> m_ram;
	required_device<v8_device> m_v8;
	optional_device<dfac_device> m_dfac;
	optional_device<applefdintf_device> m_fdc;
	optional_device_array<floppy_connector, 2> m_floppy;
	required_device<nscsi_bus_device> m_scsibus1;
	required_device<ncr5380_device> m_ncr5380;
	required_device<mac_scsi_helper_device> m_scsihelp;
	required_device<z80scc_device> m_scc;
	optional_device<egret_device> m_egret;
	optional_device<cuda_device> m_cuda;

	virtual void machine_start() override;

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

	void cuda_reset_w(int state)
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

	void egret_reset_w(int state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	}

	void set_hmmu(int state)
	{
		m_maincpu->set_hmmu_enable((state == ASSERT_LINE) ? M68K_HMMU_DISABLE : M68K_HMMU_ENABLE_LC);
	}
};

void maclc_state::machine_start()
{
	m_v8->set_ram_info((u32 *) m_ram->pointer(), m_ram->size());

	save_item(NAME(m_hdsel));
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/
void maclc_state::maclc_map(address_map &map)
{
	map.global_mask(0x80ffffff); // V8 uses bit 31 and 23-0 for address decoding only

	// RAM, ROM, and base I/O mappings come from V8
	map(0xa00000, 0xffffff).m(m_v8, FUNC(v8_device::map));

	map(0xf04000, 0xf05fff).rw(FUNC(maclc_state::scc_r), FUNC(maclc_state::scc_w));
	map(0xf06000, 0xf07fff).rw(FUNC(maclc_state::scsi_drq_r), FUNC(maclc_state::scsi_drq_w));
	map(0xf10000, 0xf11fff).rw(FUNC(maclc_state::scsi_r), FUNC(maclc_state::scsi_w));
	map(0xf12000, 0xf13fff).rw(FUNC(maclc_state::scsi_drq_r), FUNC(maclc_state::scsi_drq_w));
	map(0xf16000, 0xf17fff).rw(FUNC(maclc_state::swim_r), FUNC(maclc_state::swim_w));
}

void maclc_state::maccclassic_map(address_map &map)
{
	map.global_mask(0x80ffffff); // V8 uses bit 31 and 23-0 for address decoding only

	// RAM, ROM, and base I/O mappings come from V8
	map(0xa00000, 0xffffff).m(m_v8, FUNC(v8_device::map));

	map(0xf04000, 0xf05fff).rw(FUNC(maclc_state::scc_r), FUNC(maclc_state::scc_w));
	map(0xf06000, 0xf07fff).rw(FUNC(maclc_state::scsi_drq_r), FUNC(maclc_state::scsi_drq_w));
	map(0xf10000, 0xf11fff).rw(FUNC(maclc_state::scsi_r), FUNC(maclc_state::scsi_w));
	map(0xf12000, 0xf13fff).rw(FUNC(maclc_state::scsi_drq_r), FUNC(maclc_state::scsi_drq_w));
}

u16 maclc_state::scsi_r(offs_t offset, u16 mem_mask)
{
	const int reg = (offset >> 3) & 0xf;
	const bool pseudo_dma = (reg == 6) && (offset == 0x130);

	return m_scsihelp->read_wrapper(pseudo_dma, reg) << 8;
}

void maclc_state::scsi_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int reg = (offset >> 3) & 0xf;
	const bool pseudo_dma = (reg == 0) && (offset == 0x100);

	m_scsihelp->write_wrapper(pseudo_dma, reg, data >> 8);
}

u32 maclc_state::scsi_drq_r(offs_t offset, u32 mem_mask)
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

void maclc_state::scsi_drq_w(offs_t offset, u32 data, u32 mem_mask)
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

uint16_t maclc_state::swim_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu->adjust_icount(-5);
	}

	u16 result = m_fdc->read((offset >> 8) & 0xf);
	return result << 8;
}
void maclc_state::swim_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_fdc->write((offset >> 8) & 0xf, data & 0xff);
	else
		m_fdc->write((offset >> 8) & 0xf, data >> 8);
}

void maclc_state::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void maclc_state::devsel_w(uint8_t devsel)
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

void maclc_state::hdsel_w(int state)
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

static INPUT_PORTS_START( maclc )
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void maclc_state::maclc_base(machine_config &config)
{
	M68020HMMU(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &maclc_state::maclc_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	RAM(config, m_ram);

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
	m_scsihelp->timeout_error_callback().set(FUNC(maclc_state::scsi_berr_w));

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("MC68020,MC68020_32");
	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");

	SCC85C30(config, m_scc, C7M);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
	m_scc->out_int_callback().set(m_v8, FUNC(v8_device::scc_irq_w));
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

	V8(config, m_v8, C15M);
	m_v8->set_maincpu_tag("maincpu");
	m_v8->set_rom_tag("bootrom");
	m_v8->hdsel_callback().set(FUNC(maclc_state::hdsel_w));
	m_v8->hmmu_enable_callback().set(FUNC(maclc_state::set_hmmu));
	m_v8->add_route(0, m_dfac, 1.0);
	m_v8->add_route(1, m_dfac, 1.0);

	nubus_device &nubus(NUBUS(config, "pds", 0));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.set_address_mask(0x80ffffff);
	// V8 supports interrupts for slots $C, $D, and $E, but the LC, LC II, and Color Classic
	// only hook the slot $E IRQ up to the PDS slot.
	nubus.out_irqe_callback().set(m_v8, FUNC(v8_device::slot_irq_w<0x20>));

	MACADB(config, m_macadb, C15M);

	EGRET(config, m_egret, XTAL(32'768));
	m_egret->set_default_bios_tag("341s0850");
	m_egret->reset_callback().set(FUNC(maclc_state::egret_reset_w));
	m_egret->dfac_scl_callback().set(m_dfac, FUNC(dfac_device::clock_write));
	m_egret->dfac_sda_callback().set(m_dfac, FUNC(dfac_device::data_write));
	m_egret->dfac_latch_callback().set(m_dfac, FUNC(dfac_device::latch_write));
	m_egret->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_egret->via_clock_callback().set(m_v8, FUNC(v8_device::cb1_w));
	m_egret->via_data_callback().set(m_v8, FUNC(v8_device::cb2_w));
	m_macadb->adb_data_callback().set(m_egret, FUNC(egret_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);

	m_v8->pb3_callback().set(m_egret, FUNC(egret_device::get_xcvr_session));
	m_v8->pb4_callback().set(m_egret, FUNC(egret_device::set_via_full));
	m_v8->pb5_callback().set(m_egret, FUNC(egret_device::set_sys_session));
	m_v8->cb2_callback().set(m_egret, FUNC(egret_device::set_via_data));

	SWIM1(config, m_fdc, C15M);
	m_fdc->devsel_cb().set(FUNC(maclc_state::devsel_w));
	m_fdc->phases_cb().set(FUNC(maclc_state::phases_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);
}

void maclc_state::maclc(machine_config &config)
{
	maclc_base(config);

	NUBUS_SLOT(config, "lcpds", "pds", mac_pdslc_orig_cards, nullptr);

	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,6M,10M");
	m_v8->set_baseram_is_4M(false);
}

void maclc_state::maclc2(machine_config &config)
{
	maclc_base(config);

	NUBUS_SLOT(config, "lcpds", "pds", mac_pdslc_cards, nullptr);

	M68030(config.replace(), m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &maclc_state::maclc_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M,10M");
	m_v8->set_baseram_is_4M(true);

	SOFTWARE_LIST(config.replace(), "cd_list").set_original("mac_cdrom").set_filter("MC68030,MC68030_32");
}

void maclc_state::maccclas(machine_config &config)
{
	maclc_base(config);

	M68030(config.replace(), m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &maclc_state::maccclassic_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	config.device_remove("egret");
	config.device_remove("fdc");

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0417");
	m_cuda->reset_callback().set(FUNC(maclc_state::egret_reset_w));
	m_cuda->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_cuda->via_clock_callback().set(m_v8, FUNC(v8_device::cb1_w));
	m_cuda->via_data_callback().set(m_v8, FUNC(v8_device::cb2_w));
	m_macadb->adb_data_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);

	SPICE(config.replace(), m_v8, C15M);
	m_v8->set_maincpu_tag("maincpu");
	m_v8->set_rom_tag("bootrom");
	m_v8->hdsel_callback().set(FUNC(maclc_state::hdsel_w));
	m_v8->pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	m_v8->pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	m_v8->pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	m_v8->cb2_callback().set(m_cuda, FUNC(cuda_device::set_via_data));
	m_v8->add_route(0, "lspeaker", 1.0);
	m_v8->add_route(1, "rspeaker", 1.0);

	config.device_remove("dfac");

	NUBUS_SLOT(config, "lcpds", "pds", mac_pdslc_cards, nullptr);

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M,10M");
	m_v8->set_baseram_is_4M(true);

	SOFTWARE_LIST(config.replace(), "cd_list").set_original("mac_cdrom").set_filter("MC68030,MC68030_32");
}

void maclc_state::mactv(machine_config &config)
{
	maclc_base(config);

	M68030(config.replace(), m_maincpu, C32M);
	m_maincpu->set_addrmap(AS_PROGRAM, &maclc_state::maccclassic_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	config.device_remove("egret");
	config.device_remove("fdc");

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0788");   // TODO: 0789 freezes during boot, possible VIA bug or 6522/6523 difference?
	m_cuda->reset_callback().set(FUNC(maclc_state::egret_reset_w));
	m_cuda->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_cuda->via_clock_callback().set(m_v8, FUNC(v8_device::cb1_w));
	m_cuda->via_data_callback().set(m_v8, FUNC(v8_device::cb2_w));
	m_macadb->adb_data_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);

	TINKERBELL(config.replace(), m_v8, C15M);
	m_v8->set_maincpu_tag("maincpu");
	m_v8->set_rom_tag("bootrom");
	m_v8->hdsel_callback().set(FUNC(maclc_state::hdsel_w));
	m_v8->pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	m_v8->pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	m_v8->pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	m_v8->cb2_callback().set(m_cuda, FUNC(cuda_device::set_via_data));
	m_v8->add_route(0, "lspeaker", 1.0);
	m_v8->add_route(1, "rspeaker", 1.0);

	config.device_remove("dfac");

	// Mac TV doesn't have an LC PDS
	config.device_remove("pds");

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("5M,6M,8M");
	m_v8->set_baseram_is_4M(true);

	SOFTWARE_LIST(config.replace(), "cd_list").set_original("mac_cdrom").set_filter("MC68030,MC68030_32");
}

void maclc_state::macclas2(machine_config &config)
{
	maclc_base(config);

	M68030(config.replace(), m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &maclc_state::maclc_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	EAGLE(config.replace(), m_v8, C15M);
	m_v8->set_maincpu_tag("maincpu");
	m_v8->set_rom_tag("bootrom");
	m_v8->hdsel_callback().set(FUNC(maclc_state::hdsel_w));
	m_v8->pb3_callback().set(m_egret, FUNC(egret_device::get_xcvr_session));
	m_v8->pb4_callback().set(m_egret, FUNC(egret_device::set_via_full));
	m_v8->pb5_callback().set(m_egret, FUNC(egret_device::set_sys_session));
	m_v8->cb2_callback().set(m_egret, FUNC(egret_device::set_via_data));
	m_v8->add_route(0, m_dfac, 1.0);
	m_v8->add_route(1, m_dfac, 1.0);

	// Classic II doesn't have an LC PDS slot (and its ROM has the Slot Manager disabled)
	config.device_remove("pds");

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M,10M");
	m_v8->set_baseram_is_4M(true);

	SOFTWARE_LIST(config.replace(), "cd_list").set_original("mac_cdrom").set_filter("MC68030,MC68030_32");
}

ROM_START(maclc)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("350eacf0.rom", 0x000000, 0x080000, CRC(71681726) SHA1(6bef5853ae736f3f06c2b4e79772f65910c3b7d4))
ROM_END

ROM_START( maclc2 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD32_BYTE( "341-0476_ue2-hh.bin", 0x000000, 0x020000, CRC(0c3b0ce4) SHA1(e4e8c883d7f2e002a3f7b7aefaa3840991e57025) )
	ROM_LOAD32_BYTE( "341-0475_ud2-mh.bin", 0x000001, 0x020000, CRC(7b013595) SHA1(0b82d8fac570270db9774f6254017d28611ae756) )
	ROM_LOAD32_BYTE( "341-0474_uc2-ml.bin", 0x000002, 0x020000, CRC(2ff2f52b) SHA1(876850df61d0233c1dd3c00d48d8d6690186b164) )
	ROM_LOAD32_BYTE( "341-0473_ub2-ll.bin", 0x000003, 0x020000, CRC(8843c37c) SHA1(bb5104110507ca543d106f11c6061245fd90c1a7) )
ROM_END

ROM_START( macclas2 )
	ROM_REGION32_BE(0x100000, "bootrom", 0) // 3193670e
	ROM_LOAD32_BYTE( "341-0867__ba16__=c=apple_91.romhh.27c010.u25", 0x000000, 0x020000, CRC(88230887) SHA1(8f45f6d7eb6a8ec9242a46db4773af1d154409c6) )
	ROM_LOAD32_BYTE( "341-0866__5be9__=c=apple_91.rommh.27c010.u24", 0x000001, 0x020000, CRC(eae68c36) SHA1(e6ce79647dfe7e66590a012836d0b6e985ff672b) )
	ROM_LOAD32_BYTE( "341-0865__821e__=c=apple_91.romml.27c010.u23", 0x000002, 0x020000, CRC(cb306c01) SHA1(4d6e409995fd9a4aa9afda0fd790a5b09b1c2aca) )
	ROM_LOAD32_BYTE( "341-0864__6fc6__=c=apple_91.romll.27c010.u22", 0x000003, 0x020000, CRC(21a51e72) SHA1(bb513c1a5b8a41c7534d66aeacaeea47f58dae92) )
ROM_END

ROM_START(maccclas)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("ecd99dc0.rom", 0x000000, 0x100000, CRC(c84c3aa5) SHA1(fd9e852e2d77fe17287ba678709b9334d4d74f1e))
ROM_END

ROM_START(mactv)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("eaf1678d.bin", 0x000000, 0x100000, CRC(0644f05b) SHA1(74975c60d3a560fac9ad63125bb65a750fceaede))
ROM_END

} // anonymous namespace

COMP(1990, maclc,  0, 0, maclc,  maclc, maclc_state, empty_init, "Apple Computer", "Macintosh LC", MACHINE_SUPPORTS_SAVE)
COMP(1991, maclc2, 0, 0, maclc2, maclc, maclc_state, empty_init, "Apple Computer", "Macintosh LC II", MACHINE_SUPPORTS_SAVE)
COMP(1991, macclas2, 0, 0, macclas2, maclc, maclc_state, empty_init, "Apple Computer", "Macintosh Classic II", MACHINE_SUPPORTS_SAVE)
COMP(1993, maccclas, 0, 0, maccclas, maclc, maclc_state, empty_init, "Apple Computer", "Macintosh Color Classic", MACHINE_SUPPORTS_SAVE)
COMP(1994, mactv, 0, 0, mactv, maclc, maclc_state, empty_init, "Apple Computer", "Macintosh TV", MACHINE_SUPPORTS_SAVE)
