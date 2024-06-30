// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/macquadra700.cpp
    Mac Quadra 700 emulation.  (900/950 are IOP-based and closer to the IIfx)

    By R. Belmont

****************************************************************************/

#include "emu.h"

#include "adbmodem.h"
#include "dafb.h"
#include "dfac.h"
#include "macadb.h"
#include "macrtc.h"
#include "mactoolbox.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/nubus/cards.h"
#include "bus/nubus/nubus.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68040.h"
#include "machine/6522via.h"
#include "machine/applefdintf.h"
#include "machine/dp83932c.h"
#include "machine/ncr53c90.h"
#include "machine/nscsi_bus.h"
#include "machine/ram.h"
#include "machine/swim1.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "sound/asc.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "formats/ap_dsk35.h"

#define C32M 31.3344_MHz_XTAL
#define C15M (C32M/2)
#define C7M (C32M/4)

namespace {
class macquadra_state : public driver_device
{
public:
	macquadra_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via1(*this, "via1"),
		m_via2(*this, "via2"),
		m_macadb(*this, "macadb"),
		m_adbmodem(*this, "adbmodem"),
		m_ram(*this, RAM_TAG),
		m_swim(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_rtc(*this,"rtc"),
		m_scsibus1(*this, "scsi1"),
		m_ncr1(*this, "scsi1:7:ncr53c96"),
		m_sonic(*this, "sonic"),
		m_dafb(*this, "dafb"),
		m_easc(*this, "easc"),
		m_dfac(*this, "dfac"),
		m_scc(*this, "scc"),
		m_cur_floppy(nullptr),
		m_hdsel(0)
	{
	}

	void macqd700(machine_config &config);
	void quadra700_map(address_map &map);

	void init_macqd700();

private:
	required_device<m68040_device> m_maincpu;
	required_device<via6522_device> m_via1, m_via2;
	required_device<macadb_device> m_macadb;
	required_device<adbmodem_device> m_adbmodem;
	required_device<ram_device> m_ram;
	required_device<applefdintf_device> m_swim;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<rtc3430042_device> m_rtc;
	required_device<nscsi_bus_device> m_scsibus1;
	required_device<ncr53c96_device> m_ncr1;
	required_device<dp83932c_device> m_sonic;
	required_device<dafb_device> m_dafb;
	required_device<asc_device> m_easc;
	required_device<dfac_device> m_dfac;
	required_device<z80scc_device> m_scc;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	u32 *m_ram_ptr = nullptr, *m_rom_ptr = nullptr;
	u32 m_ram_mask = 0, m_ram_size = 0, m_rom_size = 0;
	u8 m_mac[6];

	emu_timer *m_6015_timer = nullptr;

	void nubus_irq_9_w(int state);
	void nubus_irq_a_w(int state);
	void nubus_irq_b_w(int state);
	void nubus_irq_c_w(int state);
	void nubus_irq_d_w(int state);
	void nubus_irq_e_w(int state);
	void nubus_slot_interrupt(u8 slot, u32 state);
	int m_via2_ca1_hack = 0, m_nubus_irq_state = 0;

	void adb_irq_w(int state) { m_adb_irq_pending = state; }
	int m_adb_irq_pending = 0;

	void dafb_irq_w(int state) { nubus_slot_interrupt(0xf, state);}

	void irq_539x_1_w(int state);

	floppy_image_device *m_cur_floppy = nullptr;
	int m_hdsel = 0;

	u16 mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, u16 data, u16 mem_mask);
	u16 mac_via2_r(offs_t offset);
	void mac_via2_w(offs_t offset, u16 data, u16 mem_mask);
	u8 mac_via_in_a();
	u8 mac_via_in_b();
	void mac_via_out_a(u8 data);
	void mac_via_out_b(u8 data);
	u8 mac_via2_in_a();
	u8 mac_via2_in_b();
	void mac_via2_out_a(u8 data);
	void mac_via2_out_b(u8 data);
	void mac_via_sync();
	void field_interrupts();
	void mac_via_irq(int state);
	void mac_via2_irq(int state);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);
	int m_via_interrupt = 0, m_via2_interrupt = 0, m_scc_interrupt = 0, m_last_taken_interrupt = 0;

	u32 rom_switch_r(offs_t offset);
	bool m_overlay = 0;

	u16 mac_scc_r(offs_t offset)
	{
		mac_via_sync();
		u16 result = m_scc->dc_ab_r(offset);
		return (result << 8) | result;
	}
	void mac_scc_2_w(offs_t offset, u16 data) { mac_via_sync(); m_scc->dc_ab_w(offset, data >> 8); }

	void phases_w(u8 phases);
	void devsel_w(u8 devsel);

	u16 swim_r(offs_t offset, u16 mem_mask)
	{
		if (!machine().side_effects_disabled())
		{
			m_maincpu->adjust_icount(-5);
		}

		u16 result = m_swim->read((offset >> 8) & 0xf);
		return result << 8;
	}
	void swim_w(offs_t offset, u16 data, u16 mem_mask)
	{
		if (ACCESSING_BITS_0_7)
			m_swim->write((offset >> 8) & 0xf, data & 0xff);
		else
			m_swim->write((offset >> 8) & 0xf, data>>8);
	}

	u8 ethernet_mac_r(offs_t offset);
};

void macquadra_state::field_interrupts()
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

void macquadra_state::machine_start()
{
	m_dafb->set_turboscsi1_device(m_ncr1);
	m_dafb->set_turboscsi2_device(nullptr);

	// MAC PROM is stored with a bit swizzle and must match one of 2
	// Apple-assigned OUI blocks 00:05:02 or 08:00:07
	const std::array<u8, 6> &MAC = m_sonic->get_mac();
	m_mac[0] = bitswap<8>(0x00, 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[1] = bitswap<8>(0x05, 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[2] = bitswap<8>(0x02, 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[3] = bitswap<8>(MAC[3], 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[4] = bitswap<8>(MAC[4], 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[5] = bitswap<8>(MAC[5], 0, 1, 2, 3, 7, 6, 5, 4);
	m_sonic->set_mac(&m_mac[0]);

	m_ram_ptr = (u32*)m_ram->pointer();
	m_ram_size = m_ram->size()>>1;
	m_ram_mask = m_ram_size - 1;
	m_rom_ptr = (u32*)memregion("bootrom")->base();
	m_rom_size = memregion("bootrom")->bytes();
	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = 0;
	m_last_taken_interrupt = -1;

	m_6015_timer = timer_alloc(FUNC(macquadra_state::mac_6015_tick), this);
	m_6015_timer->adjust(attotime::never);

	save_item(NAME(m_via2_ca1_hack));
	save_item(NAME(m_nubus_irq_state));
	save_item(NAME(m_adb_irq_pending));
	save_item(NAME(m_hdsel));
	save_item(NAME(m_via_interrupt));
	save_item(NAME(m_via2_interrupt));
	save_item(NAME(m_scc_interrupt));
	save_item(NAME(m_last_taken_interrupt));
	save_item(NAME(m_overlay));
}

void macquadra_state::machine_reset()
{
	m_nubus_irq_state = 0xff;
	m_via2_ca1_hack = 1;
	m_via2->write_ca1(1);
	m_via2->write_cb1(1);
	m_overlay = true;
	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = 0;
	m_last_taken_interrupt = -1;

	// put ROM mirror at 0
	address_space& space = m_maincpu->space(AS_PROGRAM);
	const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.unmap_write(0x00000000, memory_end);
	space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);

	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));
}

void macquadra_state::init_macqd700()
{
}

void macquadra_state::nubus_slot_interrupt(u8 slot, u32 state)
{
	static const u8 masks[8] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };
	u8 mask = 0xff;

	slot -= 9;

	if (state)
	{
		m_nubus_irq_state &= ~masks[slot];
	}
	else
	{
		m_nubus_irq_state |= masks[slot];
	}

	if ((m_nubus_irq_state & mask) != mask)
	{
		// HACK: sometimes we miss an ack (possible misbehavior in the VIA?)
		if (m_via2_ca1_hack == 0)
		{
			m_via2->write_ca1(1);
		}
		m_via2_ca1_hack = 0;
		m_via2->write_ca1(0);
	}
	else
	{
		m_via2_ca1_hack = 1;
		m_via2->write_ca1(1);
	}
}

void macquadra_state::nubus_irq_9_w(int state) { nubus_slot_interrupt(9, state); }
void macquadra_state::nubus_irq_a_w(int state) { nubus_slot_interrupt(0xa, state); }
void macquadra_state::nubus_irq_b_w(int state) { nubus_slot_interrupt(0xb, state); }
void macquadra_state::nubus_irq_c_w(int state) { nubus_slot_interrupt(0xc, state); }
void macquadra_state::nubus_irq_d_w(int state) { nubus_slot_interrupt(0xd, state); }
void macquadra_state::nubus_irq_e_w(int state) { nubus_slot_interrupt(0xe, state); }

void macquadra_state::irq_539x_1_w(int state)
{
	if (state)  // make sure a CB1 transition occurs
	{
		m_via2->write_cb2(0);
		m_via2->write_cb2(1);
	}
}

u16 macquadra_state::mac_via_r(offs_t offset)
{
	u16 data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		mac_via_sync();

	data = m_via1->read(offset);

	return (data & 0xff) | (data << 8);
}

void macquadra_state::mac_via_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	mac_via_sync();

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);
}

void macquadra_state::mac_via_irq(int state)
{
	m_via_interrupt = state;
	field_interrupts();
}

void macquadra_state::mac_via2_irq(int state)
{
	m_via2_interrupt = state;
	field_interrupts();
}

u16 macquadra_state::mac_via2_r(offs_t offset)
{
	int data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		mac_via_sync();

	data = m_via2->read(offset);
	return (data & 0xff) | (data << 8);
}

void macquadra_state::mac_via2_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	mac_via_sync();

	if (ACCESSING_BITS_0_7)
		m_via2->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via2->write(offset, (data >> 8) & 0xff);
}

void macquadra_state::mac_via_sync()
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

u32 macquadra_state::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (m_overlay && !machine().side_effects_disabled())
	{
		address_space& space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_end = m_ram->size() - 1;
		void *memory_data = m_ram->pointer();
		offs_t memory_mirror = memory_end & ~memory_end;

		space.install_ram(0x00000000, memory_end & ~memory_mirror, memory_mirror, memory_data);
		m_overlay = false;
	}

	//printf("rom_switch_r: offset %08x ROM_size -1 = %08x, masked = %08x\n", offset, m_rom_size-1, offset & ((m_rom_size - 1)>>2));

	return m_rom_ptr[offset & ((m_rom_size - 1)>>2)];
}

u8 macquadra_state::ethernet_mac_r(offs_t offset)
{
	if (offset < 6)
	{
		return m_mac[offset];
	}
	else if (offset == 7)
	{
		u8 xor_total = 0;

		for (int i = 0; i < 6; i++)
		{
			xor_total ^= (u8)m_mac[i];
		}

		return xor_total ^ 0xff;
	}

	return 0;
}

TIMER_CALLBACK_MEMBER(macquadra_state::mac_6015_tick)
{
	/* handle ADB keyboard/mouse */
	m_macadb->adb_vblank();
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/
void macquadra_state::quadra700_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(macquadra_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50000000, 0x50001fff).rw(FUNC(macquadra_state::mac_via_r), FUNC(macquadra_state::mac_via_w)).mirror(0x00fc0000);
	map(0x50002000, 0x50003fff).rw(FUNC(macquadra_state::mac_via2_r), FUNC(macquadra_state::mac_via2_w)).mirror(0x00fc0000);
	map(0x50008000, 0x50008007).r(FUNC(macquadra_state::ethernet_mac_r)).mirror(0x00fc0000);
	map(0x5000a000, 0x5000b0ff).m(m_sonic, FUNC(dp83932c_device::map)).umask32(0x0000ffff).mirror(0x00fc0000);
	// 5000e000 = Orwell controls
	map(0x5000f000, 0x5000f0ff).rw(m_dafb, FUNC(dafb_device::turboscsi_r<0>), FUNC(dafb_device::turboscsi_w<0>)).mirror(0x00fc0000);
	map(0x5000f100, 0x5000f101).rw(m_dafb, FUNC(dafb_device::turboscsi_dma_r<0>), FUNC(dafb_device::turboscsi_dma_w<0>)).select(0x00fc0000);
	map(0x5000c000, 0x5000dfff).rw(FUNC(macquadra_state::mac_scc_r), FUNC(macquadra_state::mac_scc_2_w)).mirror(0x00fc0000);
	map(0x50014000, 0x50015fff).rw(m_easc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00fc0000);
	map(0x5001e000, 0x5001ffff).rw(FUNC(macquadra_state::swim_r), FUNC(macquadra_state::swim_w)).mirror(0x00fc0000);

	map(0xf9000000, 0xf91fffff).rw(m_dafb, FUNC(dafb_device::vram_r), FUNC(dafb_device::vram_w));
	map(0xf9800000, 0xf98003ff).m(m_dafb, FUNC(dafb_device::map));
}

u8 macquadra_state::mac_via_in_a()
{
	return 0xc1;
}

u8 macquadra_state::mac_via_in_b()
{
	u8 val = m_rtc->data_r();

	if (!m_adb_irq_pending)
	{
		val |= 0x08;
	}

//  printf("%s VIA1 IN_B = %02x\n", machine().describe_context().c_str(), val);

	return val;
}

void macquadra_state::mac_via_out_a(u8 data)
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

void macquadra_state::mac_via_out_b(u8 data)
{
//  printf("%s VIA1 OUT B: %02x\n", machine().describe_context().c_str(), data);
	m_adbmodem->set_via_state((data & 0x30) >> 4);

	m_rtc->ce_w((data & 0x04)>>2);
	m_rtc->data_w(data & 0x01);
	m_rtc->clk_w((data >> 1) & 0x01);
}

u8 macquadra_state::mac_via2_in_a()
{
	return 0x80 | m_nubus_irq_state;
}

u8 macquadra_state::mac_via2_in_b()
{
	return 0xcf;        // indicate no NuBus transaction error
}

void macquadra_state::mac_via2_out_a(u8 data)
{
}

void macquadra_state::mac_via2_out_b(u8 data)
{
	// chain 60.15 Hz to VIA1
	m_via1->write_ca1(data>>7);

	m_dfac->data_write(BIT(data, 3));
	m_dfac->clock_write(BIT(data, 4));
	m_dfac->latch_write(BIT(data, 0));
}

void macquadra_state::phases_w(u8 phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void macquadra_state::devsel_w(u8 devsel)
{
	if (devsel == 1)
		m_cur_floppy = m_floppy[0]->get_device();
	else if (devsel == 2)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;

	m_swim->set_floppy(m_cur_floppy);
	if (m_cur_floppy)
		m_cur_floppy->ss_w(m_hdsel);
}

/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void macquadra_state::macqd700(machine_config &config)
{
	/* basic machine hardware */
	M68040(config, m_maincpu, 50_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &macquadra_state::quadra700_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	DAFB(config, m_dafb, 50_MHz_XTAL / 2);
	m_dafb->set_maincpu_tag("maincpu");
	m_dafb->dafb_irq().set(FUNC(macquadra_state::dafb_irq_w));

	RTC3430042(config, m_rtc, XTAL(32'768));
	m_rtc->cko_cb().set(m_via1, FUNC(via6522_device::write_ca2));

	SWIM1(config, m_swim, C15M);
	m_swim->phases_cb().set(FUNC(macquadra_state::phases_w));
	m_swim->devsel_cb().set(FUNC(macquadra_state::devsel_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	SCC8530N(config, m_scc, C7M);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
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

	// SCSI bus and devices
	NSCSI_BUS(config, m_scsibus1);
	NSCSI_CONNECTOR(config, "scsi1:0", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config(
		[](device_t *device)
		{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^lspeaker", 1.0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^rspeaker", 1.0);
		});
	NSCSI_CONNECTOR(config, "scsi1:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:6", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi1:7").option_set("ncr53c96", NCR53C96).clock(50_MHz_XTAL / 2).machine_config(
		[this] (device_t *device)
		{
			ncr53c96_device &adapter = downcast<ncr53c96_device &>(*device);

			adapter.set_busmd(ncr53c96_device::BUSMD_1);
			adapter.irq_handler_cb().set(*this, FUNC(macquadra_state::irq_539x_1_w));
			adapter.drq_handler_cb().set(m_dafb, FUNC(dafb_device::turboscsi_drq_w<0>));
		});

	DP83932C(config, m_sonic, 40_MHz_XTAL / 2); // clock is C20M on the schematics
	m_sonic->set_bus(m_maincpu, 0);
	m_sonic->out_int_cb().set(m_via2, FUNC(via6522_device::write_pa0)).invert();    // IRQ is active low

	nubus_device &nubus(NUBUS(config, "nubus", 40_MHz_XTAL / 4));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irq9_callback().set(FUNC(macquadra_state::nubus_irq_9_w));
	nubus.out_irqa_callback().set(FUNC(macquadra_state::nubus_irq_a_w));
	nubus.out_irqb_callback().set(FUNC(macquadra_state::nubus_irq_b_w));
	nubus.out_irqc_callback().set(FUNC(macquadra_state::nubus_irq_c_w));
	nubus.out_irqd_callback().set(FUNC(macquadra_state::nubus_irq_d_w));
	nubus.out_irqe_callback().set(FUNC(macquadra_state::nubus_irq_e_w));
	NUBUS_SLOT(config, "nbd", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbe", "nubus", mac_nubus_cards, nullptr);

	R65NC22(config, m_via1, C7M/10);
	m_via1->readpa_handler().set(FUNC(macquadra_state::mac_via_in_a));
	m_via1->readpb_handler().set(FUNC(macquadra_state::mac_via_in_b));
	m_via1->writepa_handler().set(FUNC(macquadra_state::mac_via_out_a));
	m_via1->writepb_handler().set(FUNC(macquadra_state::mac_via_out_b));
	m_via1->irq_handler().set(FUNC(macquadra_state::mac_via_irq));

	R65NC22(config, m_via2, C7M/10);
	m_via2->readpa_handler().set(FUNC(macquadra_state::mac_via2_in_a));
	m_via2->readpb_handler().set(FUNC(macquadra_state::mac_via2_in_b));
	m_via2->writepa_handler().set(FUNC(macquadra_state::mac_via2_out_a));
	m_via2->writepb_handler().set(FUNC(macquadra_state::mac_via2_out_b));
	m_via2->irq_handler().set(FUNC(macquadra_state::mac_via2_irq));

	ADBMODEM(config, m_adbmodem, C7M);
	m_adbmodem->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_adbmodem->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	m_adbmodem->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_adbmodem->irq_callback().set(FUNC(macquadra_state::adb_irq_w));
	m_via1->cb2_handler().set(m_adbmodem, FUNC(adbmodem_device::set_via_data));
	config.set_perfect_quantum(m_maincpu);

	MACADB(config, m_macadb, C15M);
	m_macadb->adb_data_callback().set(m_adbmodem, FUNC(adbmodem_device::set_adb_line));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ASC(config, m_easc, 22.5792_MHz_XTAL, asc_device::asc_type::EASC);
	m_easc->irqf_callback().set(m_via2, FUNC(via6522_device::write_cb1)).invert();
	m_easc->add_route(0, "lspeaker", 1.0);
	m_easc->add_route(1, "rspeaker", 1.0);

	// DFAC is only for audio input on Q700/Q800
	APPLE_DFAC(config, m_dfac, 22257);

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,16M,32M,64M,68M,72M,80M,96M,128M");

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("MC68040");
	SOFTWARE_LIST(config, "flop_mac35_orig").set_original("mac_flop_orig");
	SOFTWARE_LIST(config, "flop_mac35_clean").set_original("mac_flop_clcracked");
	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");
}

ROM_START( macqd700 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212) )
ROM_END

} // anonymous namespace

COMP( 1991, macqd700, 0, 0, macqd700, macadb, macquadra_state, init_macqd700,  "Apple Computer", "Macintosh Quadra 700", MACHINE_SUPPORTS_SAVE)
