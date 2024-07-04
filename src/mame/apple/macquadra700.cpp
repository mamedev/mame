// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/macquadra700.cpp
    Mac Quadra 700 ("Spike"), 900 ("Eclipse"), and 950 ("Zydeco") emulation

    Emulation by R. Belmont

    These machines were sort of a "workstation class Mac II", with fast
    built-in video, built-in Ethernet, and enhanced NuBus '90 slots that
    were backwards compatible but also had some new tricks.

    The Quadra 900 replaced the real-time clock with a compatible variant
    of Egret, and offloaded ADB, serial, LocalTalk, and floppy processing
    to the same pair of 65C02-based IOPs (I/O Processors) from the Mac IIfx.
    This setup never appeared again, but its features, including doing real
    DMA to and from the floppy drive, did.

****************************************************************************/

#include "emu.h"

#include "adbmodem.h"
#include "dafb.h"
#include "dfac.h"
#include "egret.h"
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
#include "machine/applepic.h"
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
class quadrax00_state : public driver_device
{
public:
	quadrax00_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via1(*this, "via1"),
		m_via2(*this, "via2"),
		m_macadb(*this, "macadb"),
		m_ram(*this, RAM_TAG),
		m_swim(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_scsibus1(*this, "scsi1"),
		m_ncr1(*this, "scsi1:7:ncr53c96"),
		m_sonic(*this, "sonic"),
		m_dafb(*this, "dafb"),
		m_easc(*this, "easc"),
		m_dfac(*this, "dfac"),
		m_scc(*this, "scc"),
		m_cur_floppy(nullptr),
		m_hdsel(0),
		m_adb_irq_pending(0),
		m_ram_ptr(nullptr), m_rom_ptr(nullptr),
		m_ram_mask(0), m_ram_size(0), m_rom_size(0),
		m_overlay(0),
		m_6015_timer(nullptr),
		m_via2_ca1_hack(0),
		m_nubus_irq_state(0),
		m_via_interrupt(0), m_via2_interrupt(0), m_scc_interrupt(0), m_last_taken_interrupt(0)
	{
	}

	void quadra_base(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	u32 rom_switch_r(offs_t offset);
	u16 via_r(offs_t offset);
	void via_w(offs_t offset, u16 data, u16 mem_mask);
	u16 via2_r(offs_t offset);
	void via2_w(offs_t offset, u16 data, u16 mem_mask);
	u8 ethernet_mac_r(offs_t offset);
	u16 scc_r(offs_t offset);
	void scc_w(offs_t offset, u16 data);
	u16 swim_r(offs_t offset, u16 mem_mask);
	void swim_w(offs_t offset, u16 data, u16 mem_mask);
	void adb_irq_w(int state) { m_adb_irq_pending = state; }
	void nubus_slot_interrupt(u8 slot, u32 state);
	void dafb_irq_w(int state) { nubus_slot_interrupt(0xf, state); }

	required_device<m68040_device> m_maincpu;
	required_device<via6522_device> m_via1, m_via2;
	required_device<macadb_device> m_macadb;
	required_device<ram_device> m_ram;
	required_device<applefdintf_device> m_swim;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<nscsi_bus_device> m_scsibus1;
	required_device<ncr53c96_device> m_ncr1;
	required_device<dp83932c_device> m_sonic;
	required_device<dafb_device> m_dafb;
	required_device<asc_device> m_easc;
	required_device<dfac_device> m_dfac;
	required_device<z80scc_device> m_scc;

	floppy_image_device *m_cur_floppy;
	int m_hdsel;
	int m_adb_irq_pending;
	u32 *m_ram_ptr, *m_rom_ptr;
	u32 m_ram_mask, m_ram_size, m_rom_size;
	bool m_overlay;

	void scc_irq_w(int state);

private:
	void nubus_irq_9_w(int state);
	void nubus_irq_a_w(int state);
	void nubus_irq_b_w(int state);
	void nubus_irq_c_w(int state);
	void nubus_irq_d_w(int state);
	void nubus_irq_e_w(int state);

	u8 via2_in_a();
	u8 via2_in_b();
	void via2_out_a(u8 data);
	void via2_out_b(u8 data);
	void via_sync();
	void field_interrupts();
	void via_irq(int state);
	void via2_irq(int state);
	void phases_w(u8 phases);
	void devsel_w(u8 devsel);

	u8 m_mac[6];
	emu_timer *m_6015_timer;
	int m_via2_ca1_hack, m_nubus_irq_state;
	int m_via_interrupt, m_via2_interrupt, m_scc_interrupt, m_last_taken_interrupt;

	TIMER_CALLBACK_MEMBER(mac_6015_tick);
};

class spike_state : public quadrax00_state
{
public:
	spike_state(const machine_config &mconfig, device_type type, const char *tag) :
		quadrax00_state(mconfig, type, tag),
		m_adbmodem(*this, "adbmodem"),
		m_rtc(*this,"rtc")
	{
	}

	void quadra700_map(address_map &map);
	void macqd700(machine_config &config);

private:
	u8 via_in_a();
	u8 via_in_b();
	void via_out_a(u8 data);
	void via_out_b(u8 data);

	required_device<adbmodem_device> m_adbmodem;
	required_device<rtc3430042_device> m_rtc;
};

class eclipse_state : public quadrax00_state
{
public:
	eclipse_state(const machine_config &mconfig, device_type type, const char *tag) :
		quadrax00_state(mconfig, type, tag),
		m_sccpic(*this, "sccpic"),
		m_swimpic(*this, "swimpic"),
		m_egret(*this, "egret"),
		m_scsibus2(*this, "scsi2"),
		m_ncr2(*this, "scsi2:7:ncr53c96"),
		m_adb_in(0)
	{
	}

	void quadra900_map(address_map &map);
	void macqd900(machine_config &config);
	void macqd950(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void egret_reset_w(int state);
	void fdc_hdsel(int state);

	void set_adb_line(int linestate) { m_adb_in = (linestate == ASSERT_LINE) ? true : false; }
	int adbin_r() { return m_adb_in; }

	required_device<applepic_device> m_sccpic, m_swimpic;
	required_device<egret_device> m_egret;
	required_device<nscsi_bus_device> m_scsibus2;
	required_device<ncr53c96_device> m_ncr2;

	int m_adb_in;

private:
	u8 via_in_a();
	u8 via_in_a_q950();
	u8 via_in_b();
	void via_out_a(u8 data);
	void via_out_b(u8 data);
	void via2_out_b_q900(u8 data);
};

void quadrax00_state::field_interrupts()
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

void quadrax00_state::machine_start()
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

	m_6015_timer = timer_alloc(FUNC(quadrax00_state::mac_6015_tick), this);
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

void quadrax00_state::machine_reset()
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

void eclipse_state::machine_start()
{
	quadrax00_state::machine_start();

	m_dafb->set_turboscsi2_device(m_ncr2);

	save_item(NAME(m_adb_in));
}

void eclipse_state::machine_reset()
{
	quadrax00_state::machine_reset();

	// halt 680x0 until Caboose wakes us up
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void quadrax00_state::nubus_slot_interrupt(u8 slot, u32 state)
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

void quadrax00_state::nubus_irq_9_w(int state) { nubus_slot_interrupt(9, state); }
void quadrax00_state::nubus_irq_a_w(int state) { nubus_slot_interrupt(0xa, state); }
void quadrax00_state::nubus_irq_b_w(int state) { nubus_slot_interrupt(0xb, state); }
void quadrax00_state::nubus_irq_c_w(int state) { nubus_slot_interrupt(0xc, state); }
void quadrax00_state::nubus_irq_d_w(int state) { nubus_slot_interrupt(0xd, state); }
void quadrax00_state::nubus_irq_e_w(int state) { nubus_slot_interrupt(0xe, state); }

void quadrax00_state::scc_irq_w(int state)
{
	m_scc_interrupt = state;
	field_interrupts();
}

u16 quadrax00_state::scc_r(offs_t offset)
{
	via_sync();
	return m_scc->dc_ab_r(offset) << 8;
}

void quadrax00_state::scc_w(offs_t offset, u16 data)
{
	via_sync();
	m_scc->dc_ab_w(offset, data >> 8);
}

u16 quadrax00_state::swim_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu->adjust_icount(-5);
	}

	u16 result = m_swim->read((offset >> 8) & 0xf);
	return result << 8;
}

void quadrax00_state::swim_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_swim->write((offset >> 8) & 0xf, data >> 8);
}

void eclipse_state::fdc_hdsel(int state)
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

void eclipse_state::egret_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}

u16 quadrax00_state::via_r(offs_t offset)
{
	u16 data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		via_sync();

	data = m_via1->read(offset);

	return (data & 0xff) | (data << 8);
}

void quadrax00_state::via_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	via_sync();

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);
}

void quadrax00_state::via_irq(int state)
{
	m_via_interrupt = state;
	field_interrupts();
}

void quadrax00_state::via2_irq(int state)
{
	m_via2_interrupt = state;
	field_interrupts();
}

u16 quadrax00_state::via2_r(offs_t offset)
{
	int data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		via_sync();

	data = m_via2->read(offset);
	return (data & 0xff) | (data << 8);
}

void quadrax00_state::via2_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	via_sync();

	if (ACCESSING_BITS_0_7)
		m_via2->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via2->write(offset, (data >> 8) & 0xff);
}

void quadrax00_state::via_sync()
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

u32 quadrax00_state::rom_switch_r(offs_t offset)
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

	return m_rom_ptr[offset & ((m_rom_size - 1)>>2)];
}

u8 quadrax00_state::ethernet_mac_r(offs_t offset)
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

TIMER_CALLBACK_MEMBER(quadrax00_state::mac_6015_tick)
{
	/* handle ADB keyboard/mouse */
	m_macadb->adb_vblank();
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/
void spike_state::quadra700_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(spike_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50000000, 0x50001fff).rw(FUNC(spike_state::via_r), FUNC(spike_state::via_w)).mirror(0x00fc0000);
	map(0x50002000, 0x50003fff).rw(FUNC(spike_state::via2_r), FUNC(spike_state::via2_w)).mirror(0x00fc0000);
	map(0x50008000, 0x50008007).r(FUNC(spike_state::ethernet_mac_r)).mirror(0x00fc0000);
	map(0x5000a000, 0x5000b0ff).m(m_sonic, FUNC(dp83932c_device::map)).umask32(0x0000ffff).mirror(0x00fc0000);
	// 5000e000 = Orwell controls
	map(0x5000f000, 0x5000f0ff).rw(m_dafb, FUNC(dafb_device::turboscsi_r<0>), FUNC(dafb_device::turboscsi_w<0>)).mirror(0x00fc0000);
	map(0x5000f100, 0x5000f101).rw(m_dafb, FUNC(dafb_device::turboscsi_dma_r<0>), FUNC(dafb_device::turboscsi_dma_w<0>)).select(0x00fc0000);
	map(0x5000c000, 0x5000dfff).rw(FUNC(spike_state::scc_r), FUNC(spike_state::scc_w)).mirror(0x00fc0000);
	map(0x50014000, 0x50015fff).rw(m_easc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00fc0000);
	map(0x5001e000, 0x5001ffff).rw(FUNC(spike_state::swim_r), FUNC(spike_state::swim_w)).mirror(0x00fc0000);

	map(0xf9000000, 0xf91fffff).rw(m_dafb, FUNC(dafb_device::vram_r), FUNC(dafb_device::vram_w));
	map(0xf9800000, 0xf98003ff).m(m_dafb, FUNC(dafb_device::map));
}

void eclipse_state::quadra900_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(eclipse_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50000000, 0x50001fff).rw(FUNC(eclipse_state::via_r), FUNC(eclipse_state::via_w)).mirror(0x00fc0000);
	map(0x50002000, 0x50003fff).rw(FUNC(eclipse_state::via2_r), FUNC(eclipse_state::via2_w)).mirror(0x00fc0000);
	map(0x50008000, 0x50008007).r(FUNC(eclipse_state::ethernet_mac_r)).mirror(0x00fc0000);
	map(0x5000a000, 0x5000b0ff).m(m_sonic, FUNC(dp83932c_device::map)).umask32(0x0000ffff).mirror(0x00fc0000);
	map(0x5000c000, 0x5000cfff).rw(m_sccpic, FUNC(applepic_device::host_r), FUNC(applepic_device::host_w)).mirror(0x00f00000).umask32(0xff00ff00);
	map(0x5000c000, 0x5000cfff).rw(m_sccpic, FUNC(applepic_device::host_r), FUNC(applepic_device::host_w)).mirror(0x00f00000).umask32(0x00ff00ff);
	map(0x5000f000, 0x5000f0ff).rw(m_dafb, FUNC(dafb_device::turboscsi_r<0>), FUNC(dafb_device::turboscsi_w<0>)).mirror(0x00fc0000);
	map(0x5000f100, 0x5000f101).rw(m_dafb, FUNC(dafb_device::turboscsi_dma_r<0>), FUNC(dafb_device::turboscsi_dma_w<0>)).select(0x00fc0000);
	map(0x5000f400, 0x5000f4ff).rw(m_dafb, FUNC(dafb_device::turboscsi_r<1>), FUNC(dafb_device::turboscsi_w<1>)).mirror(0x00fc0000);
	map(0x5000f502, 0x5000f503).rw(m_dafb, FUNC(dafb_device::turboscsi_dma_r<1>), FUNC(dafb_device::turboscsi_dma_w<1>)).select(0x00fc0000);

	map(0x50014000, 0x50015fff).rw(m_easc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00fc0000);
	map(0x5001e000, 0x5001efff).rw(m_swimpic, FUNC(applepic_device::host_r), FUNC(applepic_device::host_w)).mirror(0x00f00000).umask32(0xff00ff00);
	map(0x5001e000, 0x5001efff).rw(m_swimpic, FUNC(applepic_device::host_r), FUNC(applepic_device::host_w)).mirror(0x00f00000).umask32(0x00ff00ff);

	map(0xf9000000, 0xf91fffff).rw(m_dafb, FUNC(dafb_device::vram_r), FUNC(dafb_device::vram_w));
	map(0xf9800000, 0xf98003ff).m(m_dafb, FUNC(dafb_device::map));
}

u8 spike_state::via_in_a()
{
	return 0xc1;
}

u8 spike_state::via_in_b()
{
	u8 val = m_rtc->data_r();

	if (!m_adb_irq_pending)
	{
		val |= 0x08;
	}

	return val;
}

void spike_state::via_out_a(u8 data)
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

void spike_state::via_out_b(u8 data)
{
	m_adbmodem->set_via_state((data & 0x30) >> 4);

	m_rtc->ce_w((data & 0x04)>>2);
	m_rtc->data_w(data & 0x01);
	m_rtc->clk_w((data >> 1) & 0x01);
}

u8 quadrax00_state::via2_in_a()
{
	return 0x80 | m_nubus_irq_state;
}

u8 quadrax00_state::via2_in_b()
{
	return 0xcf;        // indicate no NuBus transaction error
}

void quadrax00_state::via2_out_a(u8 data)
{
}

void quadrax00_state::via2_out_b(u8 data)
{
	// chain 60.15 Hz to VIA1
	m_via1->write_ca1(data>>7);

	m_dfac->data_write(BIT(data, 3));
	m_dfac->clock_write(BIT(data, 4));
	m_dfac->latch_write(BIT(data, 0));
}

void eclipse_state::via2_out_b_q900(u8 data)
{
	// chain 60.15 Hz to VIA1
	m_via1->write_ca1(data >> 7);
}

	u8 eclipse_state::via_in_a()
	{
		return 0xd1;
	}

	u8 eclipse_state::via_in_a_q950()
	{
		return 0x91;
	}

	u8 eclipse_state::via_in_b()
	{
		return m_egret->get_xcvr_session() << 3;
	}

	void eclipse_state::via_out_a(u8 data)
	{
	}

	void eclipse_state::via_out_b(u8 data)
	{
		m_egret->set_via_full(BIT(data, 4));
		m_egret->set_sys_session(BIT(data, 5));
	}

	void quadrax00_state::phases_w(u8 phases)
	{
		if (m_cur_floppy)
		{
			m_cur_floppy->seek_phase_w(phases);
		}
	}

	void quadrax00_state::devsel_w(u8 devsel)
	{
		if (devsel == 1)
		{
			m_cur_floppy = m_floppy[0]->get_device();
		}
		else if (devsel == 2)
		{
			m_cur_floppy = m_floppy[1]->get_device();
		}
		else
		{
			m_cur_floppy = nullptr;
		}
		m_swim->set_floppy(m_cur_floppy);
		if (m_cur_floppy)
		{
			m_cur_floppy->ss_w(m_hdsel);
		}
	}

	/***************************************************************************
	    DEVICE CONFIG
	***************************************************************************/

	static INPUT_PORTS_START(macadb)
		INPUT_PORTS_END

		/***************************************************************************
		    MACHINE DRIVERS
		***************************************************************************/

		void
		quadrax00_state::quadra_base(machine_config & config)
	{
		DAFB(config, m_dafb, 50_MHz_XTAL / 2);
		m_dafb->set_maincpu_tag("maincpu");
		m_dafb->dafb_irq().set(FUNC(quadrax00_state::dafb_irq_w));

		SWIM1(config, m_swim, C15M);
		m_swim->phases_cb().set(FUNC(quadrax00_state::phases_w));
		m_swim->devsel_cb().set(FUNC(quadrax00_state::devsel_w));

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
		NSCSI_CONNECTOR(config, "scsi1:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config([](device_t *device)
																								 {
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^lspeaker", 1.0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^rspeaker", 1.0); });
		NSCSI_CONNECTOR(config, "scsi1:4", mac_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi1:5", mac_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi1:6", mac_scsi_devices, "harddisk");
		NSCSI_CONNECTOR(config, "scsi1:7").option_set("ncr53c96", NCR53C96).clock(50_MHz_XTAL / 2).machine_config([this](device_t *device)
																												  {
			ncr53c96_device &adapter = downcast<ncr53c96_device &>(*device);

			adapter.set_busmd(ncr53c96_device::BUSMD_1);
			adapter.irq_handler_cb().set(m_via2, FUNC(via6522_device::write_cb2)).invert();
			adapter.drq_handler_cb().set(m_dafb, FUNC(dafb_device::turboscsi_drq_w<0>)); });

		DP83932C(config, m_sonic, 40_MHz_XTAL / 2); // clock is C20M on the schematics
		m_sonic->set_bus(m_maincpu, 0);
		m_sonic->out_int_cb().set(m_via2, FUNC(via6522_device::write_pa0)).invert(); // IRQ is active low

		nubus_device &nubus(NUBUS(config, "nubus", 40_MHz_XTAL / 4));
		nubus.set_space(m_maincpu, AS_PROGRAM);
		nubus.out_irq9_callback().set(FUNC(quadrax00_state::nubus_irq_9_w));
		nubus.out_irqa_callback().set(FUNC(quadrax00_state::nubus_irq_a_w));
		nubus.out_irqb_callback().set(FUNC(quadrax00_state::nubus_irq_b_w));
		nubus.out_irqc_callback().set(FUNC(quadrax00_state::nubus_irq_c_w));
		nubus.out_irqd_callback().set(FUNC(quadrax00_state::nubus_irq_d_w));
		nubus.out_irqe_callback().set(FUNC(quadrax00_state::nubus_irq_e_w));
		NUBUS_SLOT(config, "nbd", "nubus", mac_nubus_cards, nullptr);
		NUBUS_SLOT(config, "nbe", "nubus", mac_nubus_cards, nullptr);

		R65NC22(config, m_via1, C7M / 10);
		m_via1->irq_handler().set(FUNC(quadrax00_state::via_irq));

		R65NC22(config, m_via2, C7M / 10);
		m_via2->readpa_handler().set(FUNC(quadrax00_state::via2_in_a));
		m_via2->readpb_handler().set(FUNC(quadrax00_state::via2_in_b));
		m_via2->writepa_handler().set(FUNC(quadrax00_state::via2_out_a));
		m_via2->writepb_handler().set(FUNC(quadrax00_state::via2_out_b));
		m_via2->irq_handler().set(FUNC(quadrax00_state::via2_irq));

		MACADB(config, m_macadb, C15M);

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

		SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
		SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("MC68040");
		SOFTWARE_LIST(config, "flop_mac35_orig").set_original("mac_flop_orig");
		SOFTWARE_LIST(config, "flop_mac35_clean").set_original("mac_flop_clcracked");
		SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
		SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");
	}

	void spike_state::macqd700(machine_config & config)
	{
		quadra_base(config);

		M68040(config, m_maincpu, 50_MHz_XTAL / 2);
		m_maincpu->set_addrmap(AS_PROGRAM, &spike_state::quadra700_map);
		m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

		RTC3430042(config, m_rtc, XTAL(32'768));
		m_rtc->cko_cb().set(m_via1, FUNC(via6522_device::write_ca2));

		ADBMODEM(config, m_adbmodem, C7M);
		m_adbmodem->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
		m_adbmodem->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
		m_adbmodem->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
		m_adbmodem->irq_callback().set(FUNC(spike_state::adb_irq_w));
		m_via1->cb2_handler().set(m_adbmodem, FUNC(adbmodem_device::set_via_data));
		m_macadb->adb_data_callback().set(m_adbmodem, FUNC(adbmodem_device::set_adb_line));
		config.set_perfect_quantum(m_maincpu);

		m_via1->readpa_handler().set(FUNC(spike_state::via_in_a));
		m_via1->readpb_handler().set(FUNC(spike_state::via_in_b));
		m_via1->writepa_handler().set(FUNC(spike_state::via_out_a));
		m_via1->writepb_handler().set(FUNC(spike_state::via_out_b));

		// Q700 has 4 MB soldered in and 4 SIMM slots which can accept 1, 4, 8, or 16 MB SIMMs
		m_ram->set_default_size("4M");
		m_ram->set_extra_options("8M,20M,36M,68M"); // 4M + (4x1M SIMMs), (4x4M), (4x8M), (4x16M)
	}

	void eclipse_state::macqd900(machine_config & config)
	{
		quadra_base(config);

		M68040(config, m_maincpu, 50_MHz_XTAL / 2);
		m_maincpu->set_addrmap(AS_PROGRAM, &eclipse_state::quadra900_map);
		m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

		APPLEPIC(config, m_sccpic, C15M);
		m_sccpic->prd_callback().set(m_scc, FUNC(z80scc_device::dc_ab_r));
		m_sccpic->pwr_callback().set(m_scc, FUNC(z80scc_device::dc_ab_w));
		m_sccpic->hint_callback().set(FUNC(eclipse_state::scc_irq_w));

		m_scc->out_int_callback().set(m_sccpic, FUNC(applepic_device::pint_w));
		m_scc->out_wreqa_callback().set(m_sccpic, FUNC(applepic_device::reqa_w));
		m_scc->out_wreqb_callback().set(m_sccpic, FUNC(applepic_device::reqb_w));

		APPLEPIC(config, m_swimpic, C15M);
		m_swimpic->prd_callback().set(m_swim, FUNC(applefdintf_device::read));
		m_swimpic->pwr_callback().set(m_swim, FUNC(applefdintf_device::write));
		m_swimpic->hint_callback().set(m_via2, FUNC(via6522_device::write_ca2)).invert();
		m_swimpic->gpout0_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w)).invert();
		m_swimpic->gpin_callback().set(FUNC(eclipse_state::adbin_r));

		m_swim->dat1byte_cb().set(m_swimpic, FUNC(applepic_device::reqa_w));
		m_swim->dat1byte_cb().append(m_swimpic, FUNC(applepic_device::reqb_w));
		m_swim->hdsel_cb().set(FUNC(eclipse_state::fdc_hdsel));

		// TODO: this is supposed to use a special version of Egret called "Caboose",
		// but currently that version of the program boots up and refuses to listen
		// to commands from the 68040.  Stock Egret works fine until that gets figured out.
		EGRET(config, m_egret, XTAL(32'768));
		m_egret->set_default_bios_tag("341s0851");
		m_egret->reset_callback().set(FUNC(eclipse_state::egret_reset_w));
		m_egret->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
		m_egret->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
		m_egret->dfac_scl_callback().set(m_dfac, FUNC(dfac_device::clock_write));
		m_egret->dfac_sda_callback().set(m_dfac, FUNC(dfac_device::data_write));
		m_egret->dfac_latch_callback().set(m_dfac, FUNC(dfac_device::latch_write));
		m_via1->cb2_handler().set(m_egret, FUNC(egret_device::set_via_data));
		config.set_perfect_quantum(m_maincpu);

		NSCSI_BUS(config, m_scsibus2);
		NSCSI_CONNECTOR(config, "scsi2:0", mac_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi2:1", mac_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi2:2", mac_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi2:3", mac_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi2:4", mac_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi2:5", mac_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi2:6", mac_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi2:7").option_set("ncr53c96", NCR53C96).clock(50_MHz_XTAL / 2).machine_config([this](device_t *device)
																												  {
		ncr53c96_device &adapter = downcast<ncr53c96_device &>(*device);

		adapter.set_busmd(ncr53c96_device::BUSMD_1);
		adapter.irq_handler_cb().append(m_via2, FUNC(via6522_device::write_cb2)).invert();
		adapter.drq_handler_cb().set(m_dafb, FUNC(dafb_device::turboscsi_drq_w<1>)); });

		// 900 and 950 are 5-slot machines, so add the other 3
		NUBUS_SLOT(config, "nba", "nubus", mac_nubus_cards, nullptr);
		NUBUS_SLOT(config, "nbb", "nubus", mac_nubus_cards, nullptr);
		NUBUS_SLOT(config, "nbc", "nubus", mac_nubus_cards, nullptr);

		m_macadb->adb_data_callback().set(FUNC(eclipse_state::set_adb_line));
		m_macadb->adb_data_callback().append(m_egret, FUNC(egret_device::set_adb_line));

		m_via1->readpa_handler().set(FUNC(eclipse_state::via_in_a));
		m_via1->readpb_handler().set(FUNC(eclipse_state::via_in_b));
		m_via1->writepa_handler().set(FUNC(eclipse_state::via_out_a));
		m_via1->writepb_handler().set(FUNC(eclipse_state::via_out_b));
		m_via2->writepb_handler().set(FUNC(eclipse_state::via2_out_b_q900));

		// Q900/Q950 have no soldered RAM and 16 SIMM slots which can accept 1, 4, 8, or 16 MB SIMMs 4 at a time
		m_ram->set_default_size("4M");
		m_ram->set_extra_options("8M,16M,32M,64M,80M,96M,128M,192M,256M");
	}

	void eclipse_state::macqd950(machine_config & config)
	{
		macqd900(config);

		m_maincpu->set_clock(33.333_MHz_XTAL);

		DAFB_Q950(config.replace(), m_dafb, 50_MHz_XTAL / 2);
		m_dafb->set_maincpu_tag("maincpu");
		m_dafb->dafb_irq().set(FUNC(eclipse_state::dafb_irq_w));

		m_via1->readpa_handler().set(FUNC(eclipse_state::via_in_a_q950));
	}

	ROM_START(macqd700)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212) )
ROM_END

ROM_START( macqd950 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("3dc27823.rom", 0x000000, 0x100000, CRC(0e11206e) SHA1(d61dba4a2d2cf9048244b713eaa294100063658d))
ROM_END

#define rom_macqd900 rom_macqd700

} // anonymous namespace

COMP( 1991, macqd700, 0, 0, macqd700, macadb, spike_state, empty_init, "Apple Computer", "Macintosh Quadra 700", MACHINE_SUPPORTS_SAVE)
COMP( 1991, macqd900, 0, 0, macqd900, macadb, eclipse_state, empty_init, "Apple Computer", "Macintosh Quadra 900", MACHINE_SUPPORTS_SAVE)
COMP( 1992, macqd950, 0, 0, macqd950, macadb, eclipse_state, empty_init, "Apple Computer", "Macintosh Quadra 950", MACHINE_SUPPORTS_SAVE)
