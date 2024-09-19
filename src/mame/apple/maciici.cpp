// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    maciici.cpp
    Mac IIci ("Cobra II, Aurora, Aurora25/16, Pacific, Atlantic")
    Mac IIsi ("Erickson, Rafiki, Hobie Cat")

    By R. Belmont

    These are the RBV/MDU (RAM Based Video/Memory Decode Unit) near-twins.
    IIci cost-reduced the IIcx and added on-board video.
    IIsi cost-reduced the IIci with a slower CPU and Egret ADB instead of
    the PIC ADB modem and Apple RTC/PRAM chip.

****************************************************************************/

#include "emu.h"

#include "adbmodem.h"
#include "egret.h"
#include "macadb.h"
#include "macrtc.h"
#include "macscsi.h"
#include "mactoolbox.h"
#include "rbv.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/nubus/nubus.h"
#include "bus/nubus/cards.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68030.h"
#include "machine/applefdintf.h"
#include "machine/ram.h"
#include "machine/swim1.h"
#include "machine/timer.h"
#include "machine/6522via.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "machine/z80scc.h"
#include "sound/asc.h"

#include "softlist_dev.h"
#include "speaker.h"

namespace {

static constexpr u32 C7M = 7833600;
static constexpr u32 C15M = (C7M * 2);

class maciici_state : public driver_device
{
public:
	maciici_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via1(*this, "via1"),
		m_rbv(*this, "rbv"),
		m_macadb(*this, "macadb"),
		m_ram(*this, RAM_TAG),
		m_adbmodem(*this, "adbmodem"),
		m_asc(*this, "asc"),
		m_scsibus1(*this, "scsi"),
		m_ncr5380(*this, "scsi:7:ncr5380"),
		m_scsihelp(*this, "scsihelp"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_scc(*this, "scc"),
		m_rtc(*this, "rtc"),
		m_egret(*this, "egret")
	{
	}

	void maciixi_base(machine_config &config);
	void maciici(machine_config &config);
	void maciisi(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68030_device> m_maincpu;
	required_device<via6522_device> m_via1;
	required_device<rbv_device> m_rbv;
	required_device<macadb_device> m_macadb;
	required_device<ram_device> m_ram;
	optional_device<adbmodem_device> m_adbmodem;
	required_device<asc_device> m_asc;
	required_device<nscsi_bus_device> m_scsibus1;
	required_device<ncr5380_device> m_ncr5380;
	required_device<mac_scsi_helper_device> m_scsihelp;
	required_device<applefdintf_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<z80scc_device> m_scc;
	optional_device<rtc3430042_device> m_rtc;
	optional_device<egret_device> m_egret;

	void set_via2_interrupt(int value);
	void field_interrupts();

	uint32_t m_overlay = 0;
	u32 *m_rom_ptr = nullptr;
	u32 m_rom_size = 0;
	int m_scc_interrupt = false, m_via_interrupt = false, m_via2_interrupt = false, m_last_taken_interrupt = false;
	int m_adb_irq_pending = 0;

	uint16_t via_r(offs_t offset);
	void via_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t via_in_a();
	uint8_t via_in_a_iisi();
	uint8_t via_in_b();
	uint8_t via_in_b_iisi();
	void via_out_a(uint8_t data);
	void via_out_b(uint8_t data);
	void via_out_b_iisi(uint8_t data);
	void via_sync();
	void via_irq(int state);
	void via_out_cb2(int state);
	void via_out_cb2_iisi(int state);
	void adb_irq_w(int state) { m_adb_irq_pending = state; }
	void scc_irq_w(int state);

	uint32_t rom_switch_r(offs_t offset);

	void maciici_map(address_map &map) ATTR_COLD;

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

	uint16_t iwm_r(offs_t offset, u16 mem_mask)
	{
		if (!machine().side_effects_disabled())
		{
			m_maincpu->adjust_icount(-5);
		}

		u16 result = m_fdc->read((offset >> 8) & 0xf);
		return result << 8;
	}

	void iwm_w(offs_t offset, u16 data, u16 mem_mask)
	{
		if (ACCESSING_BITS_0_7)
			m_fdc->write((offset >> 8) & 0xf, data & 0xff);
		else
			m_fdc->write((offset >> 8) & 0xf, data >> 8);
	}

	void write_6015(int state)
	{
		if (state)
		{
			m_macadb->adb_vblank();
		}
	}
};

void maciici_state::machine_start()
{
	m_rbv->set_ram_info((u32 *)m_ram->pointer(), m_ram->size());

	m_rom_ptr = (u32 *)memregion("bootrom")->base();
	m_rom_size = memregion("bootrom")->bytes();

	m_last_taken_interrupt = -1;
}

void maciici_state::machine_reset()
{
	// main cpu shouldn't start until Egret wakes it up
	if (m_egret)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}

	// put ROM mirror at 0
	address_space &space = m_maincpu->space(AS_PROGRAM);
	const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.unmap_write(0x00000000, memory_end);
	space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);
	m_overlay = true;
}

uint32_t maciici_state::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (m_overlay)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_end = m_ram->size() - 1;
		void *memory_data = m_ram->pointer();
		offs_t memory_mirror = memory_end & ~memory_end;

		space.install_ram(0x00000000, memory_end & ~memory_mirror, memory_mirror, memory_data);
		m_overlay = false;
	}

	// printf("rom_switch_r: offset %08x ROM_size -1 = %08x, masked = %08x\n", offset, m_rom_size-1, offset & ((m_rom_size - 1)>>2));

	return m_rom_ptr[offset & ((m_rom_size - 1) >> 2)];
}

void maciici_state::field_interrupts()
{
	int take_interrupt = -1;

	if (m_scc_interrupt)
	{
		take_interrupt = 4;
	}
	else if (m_via2_interrupt)
	{
		take_interrupt = 2;
	}
	else if (m_via_interrupt)
	{
		take_interrupt = 1;
	}

	if (m_last_taken_interrupt > -1)
	{
		m_maincpu->set_input_line(m_last_taken_interrupt, CLEAR_LINE);
		m_last_taken_interrupt = -1;
	}

	if (take_interrupt > -1)
	{
		m_maincpu->set_input_line(take_interrupt, ASSERT_LINE);
		m_last_taken_interrupt = take_interrupt;
	}
}

void maciici_state::via_irq(int state)
{
	m_via_interrupt = state;
	field_interrupts();
}

void maciici_state::scc_irq_w(int state)
{
	m_scc_interrupt = state;
	field_interrupts();
}

void maciici_state::set_via2_interrupt(int value)
{
	m_via2_interrupt = value;
	field_interrupts();
}

void maciici_state::via_sync()
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

uint16_t maciici_state::via_r(offs_t offset)
{
	uint16_t data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		via_sync();

	data = m_via1->read(offset);

	return (data & 0xff) | (data << 8);
}

void maciici_state::via_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	via_sync();

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);
}

uint8_t maciici_state::via_in_a()
{
	return 0xc7; // IIci: PA6 | PA2 | PA1
}

uint8_t maciici_state::via_in_a_iisi()
{
	return 0x97; // IIci: PA4 | PA2 | PA1
}

uint8_t maciici_state::via_in_b()
{
	u8 val = m_rtc->data_r();

	if (!m_adb_irq_pending)
	{
		val |= 0x08;
	}

	return val;
}

uint8_t maciici_state::via_in_b_iisi()
{
	return m_egret->get_xcvr_session() << 3;
}

void maciici_state::via_out_a(uint8_t data)
{
	int hdsel = BIT(data, 5);
	if (hdsel != m_hdsel)
	{
		if (m_cur_floppy)
		{
			m_cur_floppy->ss_w(hdsel);
		}
	}
	m_hdsel = hdsel;
}

void maciici_state::via_out_b(uint8_t data)
{
	//  printf("%s VIA1 OUT B: %02x\n", machine().describe_context().c_str(), data);
	m_adbmodem->set_via_state((data & 0x30) >> 4);

m_rtc->ce_w(BIT(data, 2));
	m_rtc->data_w(BIT(data, 0));
	m_rtc->clk_w(BIT(data, 1));
}

void maciici_state::via_out_b_iisi(uint8_t data)
{
	m_egret->set_via_full(BIT(data, 4));
	m_egret->set_sys_session(BIT(data, 5));
}

void maciici_state::via_out_cb2(int state)
{
//  m_macadb->adb_data_w(state);
}

void maciici_state::via_out_cb2_iisi(int state)
{
	m_egret->set_via_data(state & 1);
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void maciici_state::maciici_map(address_map &map)
{
	map(0x40000000, 0x4007ffff).r(FUNC(maciici_state::rom_switch_r)).mirror(0x0ff80000);

	map(0x50000000, 0x50001fff).rw(FUNC(maciici_state::via_r), FUNC(maciici_state::via_w)).mirror(0x00f00000);
	map(0x50004000, 0x50005fff).rw(FUNC(maciici_state::scc_r), FUNC(maciici_state::scc_w)).mirror(0x00f00000);
	map(0x50006000, 0x50007fff).rw(FUNC(maciici_state::scsi_drq_r), FUNC(maciici_state::scsi_drq_w)).mirror(0x00f00000);
	map(0x50010000, 0x50011fff).rw(FUNC(maciici_state::scsi_r), FUNC(maciici_state::scsi_w)).mirror(0x00f00000);
	map(0x50012000, 0x50013fff).rw(FUNC(maciici_state::scsi_drq_r), FUNC(maciici_state::scsi_drq_w)).mirror(0x00f00000);
	map(0x50014000, 0x50015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00f00000);
	map(0x50016000, 0x50017fff).rw(FUNC(maciici_state::iwm_r), FUNC(maciici_state::iwm_w)).mirror(0x00f00000);
	map(0x50024000, 0x50027fff).m(m_rbv, FUNC(rbv_device::map)).mirror(0x00f00000);
	map(0x50040000, 0x50041fff).rw(FUNC(maciici_state::via_r), FUNC(maciici_state::via_w)).mirror(0x00f00000);
}

u16 maciici_state::scsi_r(offs_t offset, u16 mem_mask)
{
	const int reg = (offset >> 3) & 0xf;
	const bool pseudo_dma = (reg == 6) && (offset == 0x130);

	return m_scsihelp->read_wrapper(pseudo_dma, reg) << 8;
}

void maciici_state::scsi_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int reg = (offset >> 3) & 0xf;
	const bool pseudo_dma = (reg == 0) && (offset == 0x100);

	m_scsihelp->write_wrapper(pseudo_dma, reg, data >> 8);
}

u32 maciici_state::scsi_drq_r(offs_t offset, u32 mem_mask)
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

void maciici_state::scsi_drq_w(offs_t offset, u32 data, u32 mem_mask)
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

void maciici_state::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void maciici_state::devsel_w(uint8_t devsel)
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

static INPUT_PORTS_START(maciici)
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/
void maciici_state::maciixi_base(machine_config &config)
{
	M68030(config, m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &maciici_state::maciici_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	RTC3430042(config, m_rtc, XTAL(32'768));
	m_rtc->cko_cb().set(m_via1, FUNC(via6522_device::write_ca2));

	SWIM1(config, m_fdc, C15M);
	m_fdc->devsel_cb().set(FUNC(maciici_state::devsel_w));
	m_fdc->phases_cb().set(FUNC(maciici_state::phases_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");

	SCC85C30(config, m_scc, C7M);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
	m_scc->out_int_callback().set(FUNC(maciici_state::scc_irq_w));
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
	ASC(config, m_asc, C15M, asc_device::asc_type::ASC);
	m_asc->irqf_callback().set(m_rbv, FUNC(rbv_device::asc_irq_w));
	m_asc->add_route(0, "lspeaker", 1.0);
	m_asc->add_route(1, "rspeaker", 1.0);

	R65NC22(config, m_via1, C7M / 10);
	m_via1->readpa_handler().set(FUNC(maciici_state::via_in_a));
	m_via1->readpb_handler().set(FUNC(maciici_state::via_in_b));
	m_via1->writepa_handler().set(FUNC(maciici_state::via_out_a));
	m_via1->writepb_handler().set(FUNC(maciici_state::via_out_b));
	m_via1->cb2_handler().set(FUNC(maciici_state::via_out_cb2));
	m_via1->irq_handler().set(FUNC(maciici_state::via_irq));

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
		adapter.drq_handler().set(m_scsihelp, FUNC(mac_scsi_helper_device::drq_w)); });

	MAC_SCSI_HELPER(config, m_scsihelp);
	m_scsihelp->scsi_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::read));
	m_scsihelp->scsi_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::write));
	m_scsihelp->scsi_dma_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_r));
	m_scsihelp->scsi_dma_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_w));
	m_scsihelp->cpu_halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_scsihelp->timeout_error_callback().set(FUNC(maciici_state::scsi_berr_w));

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("MC68030,MC68030_32");

	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("8M,32M,64M,96M,128M");

	SOFTWARE_LIST(config, "flop_mac35_orig").set_original("mac_flop_orig");
	SOFTWARE_LIST(config, "flop_mac35_clean").set_original("mac_flop_clcracked");
	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");

	RBV(config, m_rbv, C15M);
	m_rbv->via6015_callback().set(m_via1, FUNC(via6522_device::write_ca1));
	m_rbv->via6015_callback().append(FUNC(maciici_state::write_6015));
	m_rbv->irq_callback().set(FUNC(maciici_state::set_via2_interrupt));

	/* internal ram */
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,8M,16M,32M,48M,64M,128M");

	nubus_device &nubus(NUBUS(config, "nubus", 0));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irq9_callback().set(m_rbv, FUNC(rbv_device::slot_irq_w<0x01>));
	nubus.out_irqa_callback().set(m_rbv, FUNC(rbv_device::slot_irq_w<0x02>));
	nubus.out_irqb_callback().set(m_rbv, FUNC(rbv_device::slot_irq_w<0x04>));
	nubus.out_irqc_callback().set(m_rbv, FUNC(rbv_device::slot_irq_w<0x08>));
	nubus.out_irqd_callback().set(m_rbv, FUNC(rbv_device::slot_irq_w<0x10>));
	nubus.out_irqe_callback().set(m_rbv, FUNC(rbv_device::slot_irq_w<0x20>));

	NUBUS_SLOT(config, "nbc", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbd", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbe", "nubus", mac_nubus_cards, nullptr);
}

void maciici_state::maciici(machine_config &config)
{
	maciixi_base(config);

	ADBMODEM(config, m_adbmodem, C7M);
	m_adbmodem->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_adbmodem->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	m_adbmodem->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_adbmodem->irq_callback().set(FUNC(maciici_state::adb_irq_w));
	m_via1->cb2_handler().set(m_adbmodem, FUNC(adbmodem_device::set_via_data));
	config.set_perfect_quantum(m_maincpu);

	MACADB(config, m_macadb, C15M);
	m_macadb->adb_data_callback().set(m_adbmodem, FUNC(adbmodem_device::set_adb_line));
}

void maciici_state::maciisi(machine_config &config)
{
	maciixi_base(config);

	M68030(config.replace(), m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &maciici_state::maciici_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	m_via1->readpa_handler().set(FUNC(maciici_state::via_in_a_iisi));
	m_via1->readpb_handler().set(FUNC(maciici_state::via_in_b_iisi));
	m_via1->writepb_handler().set(FUNC(maciici_state::via_out_b_iisi));
	m_via1->cb2_handler().set(FUNC(maciici_state::via_out_cb2_iisi));

	MACADB(config, m_macadb, C15M);

	EGRET(config, m_egret, XTAL(32'768));
	m_egret->set_default_bios_tag("344s0100");
	m_egret->reset_callback().set(FUNC(maciici_state::egret_reset_w));
	m_egret->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_egret->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_egret->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	m_macadb->adb_data_callback().set(m_egret, FUNC(egret_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);

	config.device_remove("nbc");
	config.device_remove("nbd");
	config.device_remove("nbe");
	config.device_remove("nubus");

	// TODO: IIsi takes an adapter card that can accept either one SE/30 PDS card or one NuBus card
	nubus_device &nubus(NUBUS(config, "pds", 0));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irq9_callback().set(m_rbv, FUNC(rbv_device::slot_irq_w<0x01>));
	nubus.out_irqa_callback().set(m_rbv, FUNC(rbv_device::slot_irq_w<0x02>));
	nubus.out_irqb_callback().set(m_rbv, FUNC(rbv_device::slot_irq_w<0x04>));
	nubus.out_irqc_callback().set(m_rbv, FUNC(rbv_device::slot_irq_w<0x08>));
	nubus.out_irqd_callback().set(m_rbv, FUNC(rbv_device::slot_irq_w<0x10>));
	nubus.out_irqe_callback().set(m_rbv, FUNC(rbv_device::slot_irq_w<0x20>));
	NUBUS_SLOT(config, "siexp", "pds", mac_iisi_cards, nullptr);
}

ROM_START( maciici )
	ROM_REGION32_BE(0x80000, "bootrom", 0)
	ROM_LOAD32_BYTE( "341-0736.um12", 0x000000, 0x020000, CRC(7a1906e6) SHA1(3e39c80b52f40798502fcbdfc97b315545c4c4d3) )
	ROM_LOAD32_BYTE( "341-0735.um11", 0x000001, 0x020000, CRC(a8942189) SHA1(be9f653cab04c304d7ee8d4ec312c23ff5d47efc) )
	ROM_LOAD32_BYTE( "342-0734.um10", 0x000002, 0x020000, CRC(07f56402) SHA1(e11ca97181faf26cd0d05bd639d65998805c7822) )
	ROM_LOAD32_BYTE( "342-0733.um9",  0x000003, 0x020000, CRC(20c28451) SHA1(fecf849c9ac9717c18c13184e24a471888028e46) )
ROM_END

ROM_START( maciisi )
	ROM_REGION32_BE(0x80000, "bootrom", 0)
	ROM_LOAD( "36b7fb6c.rom", 0x000000, 0x080000, CRC(f304d973) SHA1(f923de4125aae810796527ff6e25364cf1d54eec) )
ROM_END

} // anonymous namespace

COMP(1989, maciici, 0, 0, maciici, maciici, maciici_state, empty_init, "Apple Computer", "Macintosh IIci", MACHINE_SUPPORTS_SAVE)
COMP(1990, maciisi, 0, 0, maciisi, maciici, maciici_state, empty_init, "Apple Computer", "Macintosh IIsi", MACHINE_SUPPORTS_SAVE)
