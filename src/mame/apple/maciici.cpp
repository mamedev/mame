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

#include "egret.h"
#include "macadb.h"
#include "macrtc.h"
#include "macscsi.h"
#include "mactoolbox.h"

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

#include "emupal.h"
#include "render.h"
#include "screen.h"
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
		m_macadb(*this, "macadb"),
		m_ram(*this, RAM_TAG),
		m_asc(*this, "asc"),
		m_scsibus1(*this, "scsi"),
		m_ncr5380(*this, "scsi:7:ncr5380"),
		m_scsihelp(*this, "scsihelp"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_scc(*this, "scc"),
		m_montype(*this, "MONTYPE"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_rtc(*this, "rtc"),
		m_egret(*this, "egret")
	{
	}

	void maciici(machine_config &config);
	void maciisi(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<m68030_device> m_maincpu;
	required_device<via6522_device> m_via1;
	required_device<macadb_device> m_macadb;
	required_device<ram_device> m_ram;
	required_device<asc_device> m_asc;
	required_device<nscsi_bus_device> m_scsibus1;
	required_device<ncr5380_device> m_ncr5380;
	required_device<mac_scsi_helper_device> m_scsihelp;
	required_device<applefdintf_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<z80scc_device> m_scc;
	required_ioport m_montype;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<rtc3430042_device> m_rtc;
	optional_device<egret_device> m_egret;

	emu_timer *m_6015_timer = nullptr;
	emu_timer *m_scanline_timer = nullptr;

	u8 m_rbv_regs[256]{}, m_rbv_ier = 0, m_rbv_ifr = 0, m_rbv_montype = 0, m_rbv_vbltime = 0;
	u32 m_rbv_colors[3]{}, m_rbv_count = 0, m_rbv_clutoffs = 0, m_rbv_immed10wr = 0;
	u32 m_rbv_palette[256]{};

	uint32_t screen_update_macrbv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void rbv_reset();
	void rbv_recalc_irqs();
	void rbv_ramdac_w(offs_t offset, uint32_t data);
	uint8_t rbv_r(offs_t offset);
	void rbv_w(offs_t offset, uint8_t data);
	void nubus_slot_interrupt(uint8_t slot, uint32_t state);

	WRITE_LINE_MEMBER(mac_asc_irq);
	DECLARE_WRITE_LINE_MEMBER(scc_irq_w);
	void set_via2_interrupt(int value);
	void field_interrupts();

	uint32_t m_overlay = 0;
	u32 *m_rom_ptr = nullptr;
	u32 m_rom_size = 0;
	int m_scc_interrupt = false, m_via_interrupt = false, m_via2_interrupt = false, m_last_taken_interrupt = false;
	int m_adb_irq_pending = 0;
	uint8_t m_nubus_irq_state = 0;

	DECLARE_WRITE_LINE_MEMBER(mac_rbv_vbl);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);
	TIMER_CALLBACK_MEMBER(mac_scanline_tick);

	DECLARE_WRITE_LINE_MEMBER(nubus_irq_c_w);
	DECLARE_WRITE_LINE_MEMBER(nubus_irq_d_w);
	DECLARE_WRITE_LINE_MEMBER(nubus_irq_e_w);
	WRITE_LINE_MEMBER(adb_irq_w) { m_adb_irq_pending = state; }

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
	DECLARE_WRITE_LINE_MEMBER(via_irq);
	WRITE_LINE_MEMBER(via_out_cb2);
	WRITE_LINE_MEMBER(via_out_cb2_iisi);

	uint32_t rom_switch_r(offs_t offset);

	void maciici_map(address_map &map);

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

	WRITE_LINE_MEMBER(egret_reset_w)
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
};

TIMER_CALLBACK_MEMBER(maciici_state::mac_6015_tick)
{
	m_via1->write_ca1(0);
	m_via1->write_ca1(1);

	m_macadb->adb_vblank();
}

TIMER_CALLBACK_MEMBER(maciici_state::mac_scanline_tick)
{
	int scanline = m_screen->vpos();

	if (m_rbv_vbltime > 0)
	{
		m_rbv_vbltime--;

		if (m_rbv_vbltime == 0)
		{
			m_rbv_regs[2] |= 0x40;
			rbv_recalc_irqs();
		}
	}

	int next_scanline = (scanline + 1) % 370;	// TODO: fix
	m_scanline_timer->adjust(m_screen->time_until_pos(next_scanline), next_scanline);
}

void maciici_state::machine_start()
{
	m_6015_timer = timer_alloc(FUNC(maciici_state::mac_6015_tick), this);
	m_6015_timer->adjust(attotime::never);

	m_scanline_timer = timer_alloc(FUNC(maciici_state::mac_scanline_tick), this);
	m_scanline_timer->adjust(m_screen->time_until_pos(0, 0));

	m_rom_ptr = (u32 *)memregion("bootrom")->base();
	m_rom_size = memregion("bootrom")->bytes();

	m_last_taken_interrupt = -1;

	save_item(NAME(m_rbv_regs));
	save_item(NAME(m_rbv_ier));
	save_item(NAME(m_rbv_ifr));
	save_item(NAME(m_rbv_colors));
	save_item(NAME(m_rbv_count));
	save_item(NAME(m_rbv_clutoffs));
	save_item(NAME(m_rbv_palette));
}

void maciici_state::machine_reset()
{
	m_rbv_vbltime = 0;
	rbv_reset();
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));

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

WRITE_LINE_MEMBER(maciici_state::mac_asc_irq)
{
	if (state == ASSERT_LINE)
	{
		m_rbv_regs[3] |= 0x10; // any VIA 2 interrupt | sound interrupt
		rbv_recalc_irqs();
	}
	else
	{
		m_rbv_regs[3] &= ~0x10;
		rbv_recalc_irqs();
	}
}

WRITE_LINE_MEMBER(maciici_state::via_irq)
{
	m_via_interrupt = state;
	field_interrupts();
}

WRITE_LINE_MEMBER(maciici_state::scc_irq_w)
{
	m_scc_interrupt = state;
	field_interrupts();
}

void maciici_state::set_via2_interrupt(int value)
{
	m_via2_interrupt = value;
	field_interrupts();
}

WRITE_LINE_MEMBER(maciici_state::nubus_irq_c_w)
{
	nubus_slot_interrupt(0xc, state);
}

WRITE_LINE_MEMBER(maciici_state::nubus_irq_d_w)
{
	nubus_slot_interrupt(0xd, state);
}

WRITE_LINE_MEMBER(maciici_state::nubus_irq_e_w)
{
	nubus_slot_interrupt(0xe, state);
}

void maciici_state::nubus_slot_interrupt(uint8_t slot, uint32_t state)
{
	static const uint8_t masks[8] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};

	slot -= 9;

	if (state)
	{
		m_nubus_irq_state &= ~masks[slot];
	}
	else
	{
		m_nubus_irq_state |= masks[slot];
	}

	m_rbv_regs[2] &= ~0x38;
	m_rbv_regs[2] |= (m_nubus_irq_state & 0x38);
	rbv_recalc_irqs();
}

// do this here - screen_update is called each scanline when stepping in the
// debugger, which means you can't escape the VIA2 IRQ handler
//
// RBV/MDU bits in IER/IFR:
//
// CA1: any slot interrupt = 0x02
// CA2: SCSI interrupt     = 0x01
// CB1: ASC interrupt      = 0x10

WRITE_LINE_MEMBER(maciici_state::mac_rbv_vbl)
{
	if (!state)
		return;

	m_rbv_regs[2] &= ~0x40; // set vblank signal
	m_rbv_vbltime = 10;

	//  printf("RBV: raising VBL!\n");

	if (m_rbv_regs[0x12] & 0x40)
	{
		rbv_recalc_irqs();
	}
}

void maciici_state::rbv_ramdac_w(offs_t offset, uint32_t data)
{
	if (!offset)
	{
		m_rbv_clutoffs = data >> 24;
		m_rbv_count = 0;
	}
	else
	{
		m_rbv_colors[m_rbv_count++] = data >> 24;

		if (m_rbv_count == 3)
		{
			// for portrait display, force monochrome by using the blue channel
			if (m_montype->read() == 1)
			{
				m_palette->set_pen_color(m_rbv_clutoffs, rgb_t(m_rbv_colors[2], m_rbv_colors[2], m_rbv_colors[2]));
				m_rbv_palette[m_rbv_clutoffs] = rgb_t(m_rbv_colors[2], m_rbv_colors[2], m_rbv_colors[2]);
				m_rbv_clutoffs++;
				m_rbv_count = 0;
			}
			else
			{
				m_palette->set_pen_color(m_rbv_clutoffs, rgb_t(m_rbv_colors[0], m_rbv_colors[1], m_rbv_colors[2]));
				m_rbv_palette[m_rbv_clutoffs] = rgb_t(m_rbv_colors[0], m_rbv_colors[1], m_rbv_colors[2]);
				m_rbv_clutoffs++;
				m_rbv_count = 0;
			}
		}
	}
}

void maciici_state::rbv_recalc_irqs()
{
	// check slot interrupts and bubble them down to IFR
	uint8_t slot_irqs = (~m_rbv_regs[2]) & 0x78;
	slot_irqs &= (m_rbv_regs[0x12] & 0x78);

	if (slot_irqs)
	{
		m_rbv_regs[3] |= 2; // any slot
	}
	else // no slot irqs, clear the pending bit
	{
		m_rbv_regs[3] &= ~2; // any slot
	}

	uint8_t ifr = (m_rbv_regs[3] & m_rbv_ier) & 0x1b; // m_rbv_regs[0x13]);

	//  printf("ifr = %02x (reg3 %02x reg13 %02x)\n", ifr, m_rbv_regs[3], m_rbv_regs[0x13]);
	if (ifr != 0)
	{
		m_rbv_regs[3] = ifr | 0x80;
		m_rbv_ifr = ifr | 0x80;

		//      printf("VIA2 raise\n");
		set_via2_interrupt(1);
	}
	else
	{
		//      printf("VIA2 lower\n");
		set_via2_interrupt(0);
	}
}

uint8_t maciici_state::rbv_r(offs_t offset)
{
	int data = 0;

	if (offset < 0x100)
	{
		data = m_rbv_regs[offset];

		if (offset == 0x10)
		{
			data &= ~0x38;
			data |= (m_montype->read() << 3);
			//            printf("%s rbv_r montype: %02x\n", machine().describe_context().c_str(), data);
		}

		// bit 7 of these registers always reads as 0 on RBV
		if ((offset == 0x12) || (offset == 0x13))
		{
			data &= ~0x80;
		}
	}
	else
	{
		offset >>= 9;

		switch (offset)
		{
		case 13: // IFR
				 //              printf("Read IER = %02x (PC=%x) 2=%02x\n", m_rbv_ier, m_maincpu->pc(), m_rbv_regs[2]);
			data = m_rbv_ifr;
			break;

		case 14: // IER
				 //              printf("Read IFR = %02x (PC=%x) 2=%02x\n", m_rbv_ifr, m_maincpu->pc(), m_rbv_regs[2]);
			data = m_rbv_ier;
			break;

		default:
			logerror("rbv_r: Unknown extended RBV VIA register %d access\n", offset);
			break;
		}
	}

	//  printf("rbv_r: %x = %02x (PC=%x)\n", offset, data, m_maincpu->pc());

	return data;
}

void maciici_state::rbv_w(offs_t offset, uint8_t data)
{
	if (offset < 0x100)
	{
		//      if (offset == 0x10)
		//      printf("rbv_w: %02x to offset %x (PC=%x)\n", data, offset, m_maincpu->pc());
		switch (offset)
		{
		case 0x02:
			data &= 0x40;
			m_rbv_regs[offset] &= ~data;
			rbv_recalc_irqs();
			break;

		case 0x03:			 // write here to ack
			if (data & 0x80) // 1 bits write 1s
			{
				m_rbv_regs[offset] |= data & 0x7f;
				m_rbv_ifr |= data & 0x7f;
			}
			else // 1 bits write 0s
			{
				m_rbv_regs[offset] &= ~(data & 0x7f);
				m_rbv_ifr &= ~(data & 0x7f);
			}
			rbv_recalc_irqs();
			break;

		case 0x10:
			if (data != 0)
			{
				m_rbv_immed10wr = 1;
			}
			m_rbv_regs[offset] = data;
			break;

		case 0x12:
			if (data & 0x80) // 1 bits write 1s
			{
				m_rbv_regs[offset] |= data & 0x7f;
			}
			else // 1 bits write 0s
			{
				m_rbv_regs[offset] &= ~(data & 0x7f);
			}
			rbv_recalc_irqs();
			break;

		case 0x13:
			if (data & 0x80) // 1 bits write 1s
			{
				m_rbv_regs[offset] |= data & 0x7f;

				if (data == 0xff)
					m_rbv_regs[offset] = 0x1f; // I don't know why this is special, but the IIci ROM's POST demands it
			}
			else // 1 bits write 0s
			{
				m_rbv_regs[offset] &= ~(data & 0x7f);
			}
			break;

		default:
			m_rbv_regs[offset] = data;
			break;
		}
	}
	else
	{
		offset >>= 9;

		switch (offset)
		{
		case 13: // IFR
				 //              printf("%02x to IFR (PC=%x)\n", data, m_maincpu->pc());
			if (data & 0x80)
			{
				data = 0x7f;
			}
			rbv_recalc_irqs();
			break;

		case 14:			 // IER
							 //              printf("%02x to IER (PC=%x)\n", data, m_maincpu->pc());
			if (data & 0x80) // 1 bits write 1s
			{
				m_rbv_ier |= data & 0x7f;
			}
			else // 1 bits write 0s
			{
				m_rbv_ier &= ~(data & 0x7f);
			}
			rbv_recalc_irqs();
			break;

		default:
			logerror("rbv_w: Unknown extended RBV VIA register %d access\n", offset);
			break;
		}
	}
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
	u8 val = m_macadb->get_adb_state() << 4;

	if (!m_adb_irq_pending)
	{
		val |= 0x08;
	}

	val |= m_rtc->data_r();

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
	m_macadb->mac_adb_newaction((data & 0x30) >> 4);

	m_rtc->ce_w(BIT(data, 2));
	m_rtc->data_w(BIT(data, 0));
	m_rtc->clk_w(BIT(data, 1));
}

void maciici_state::via_out_b_iisi(uint8_t data)
{
	m_egret->set_via_full(BIT(data, 4));
	m_egret->set_sys_session(BIT(data, 5));
}

WRITE_LINE_MEMBER(maciici_state::via_out_cb2)
{
	m_macadb->adb_data_w(state);
}

WRITE_LINE_MEMBER(maciici_state::via_out_cb2_iisi)
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
	map(0x50024000, 0x50024007).w(FUNC(maciici_state::rbv_ramdac_w)).mirror(0x00f00000);
	map(0x50026000, 0x50027fff).rw(FUNC(maciici_state::rbv_r), FUNC(maciici_state::rbv_w)).mirror(0x00f00000);
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

// IIci/IIsi RAM-Based Video (RBV)

void maciici_state::rbv_reset()
{
	int htotal, vtotal;
	double framerate;

	memset(m_rbv_regs, 0, sizeof(m_rbv_regs));

	m_rbv_count = 0;
	m_rbv_clutoffs = 0;
	m_rbv_immed10wr = 0;

	m_rbv_regs[2] = 0x7f;
	m_rbv_regs[3] = 0;

	m_rbv_montype = m_montype->read();
	rectangle visarea;
	switch (m_rbv_montype)
	{
	case 1: // 15" portrait display
		visarea.set(0, 640 - 1, 0, 870 - 1);
		htotal = 832;
		vtotal = 918;
		framerate = 75.0;
		break;

	case 2: // 12" RGB
		visarea.set(0, 512 - 1, 0, 384 - 1);
		htotal = 640;
		vtotal = 407;
		framerate = 60.15;
		break;

	case 6: // 13" RGB
	default:
		visarea.set(0, 640 - 1, 0, 480 - 1);
		htotal = 800;
		vtotal = 525;
		framerate = 59.94;
		break;
	}

	//    logerror("RBV reset: monitor is %dx%d @ %f Hz\n", visarea.width(), visarea.height(), framerate);
	m_screen->configure(htotal, vtotal, visarea, HZ_TO_ATTOSECONDS(framerate));
}

uint32_t maciici_state::screen_update_macrbv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t const *vram8 = (uint8_t *)m_ram->pointer();
	int hres, vres;

	switch (m_rbv_montype)
	{
	case 1: // 15" portrait display
		hres = 640;
		vres = 870;
		break;

	case 2: // 12" RGB
		hres = 512;
		vres = 384;
		break;

	case 6: // 13" RGB
	default:
		hres = 640;
		vres = 480;
		break;
	}

	switch (m_rbv_regs[0x10] & 7)
	{
	case 0: // 1bpp
	{
		for (int y = 0; y < vres; y++)
		{
			uint32_t *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres; x += 8)
			{
				uint8_t const pixels = vram8[(y * (hres / 8)) + ((x / 8) ^ 3)];

				*scanline++ = m_rbv_palette[0xfe | (pixels >> 7)];
				*scanline++ = m_rbv_palette[0xfe | ((pixels >> 6) & 1)];
				*scanline++ = m_rbv_palette[0xfe | ((pixels >> 5) & 1)];
				*scanline++ = m_rbv_palette[0xfe | ((pixels >> 4) & 1)];
				*scanline++ = m_rbv_palette[0xfe | ((pixels >> 3) & 1)];
				*scanline++ = m_rbv_palette[0xfe | ((pixels >> 2) & 1)];
				*scanline++ = m_rbv_palette[0xfe | ((pixels >> 1) & 1)];
				*scanline++ = m_rbv_palette[0xfe | (pixels & 1)];
			}
		}
	}
	break;

	case 1: // 2bpp
	{
		for (int y = 0; y < vres; y++)
		{
			uint32_t *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres / 4; x++)
			{
				uint8_t const pixels = vram8[(y * (hres / 4)) + (BYTE4_XOR_BE(x))];

				*scanline++ = m_rbv_palette[0xfc | ((pixels >> 6) & 3)];
				*scanline++ = m_rbv_palette[0xfc | ((pixels >> 4) & 3)];
				*scanline++ = m_rbv_palette[0xfc | ((pixels >> 2) & 3)];
				*scanline++ = m_rbv_palette[0xfc | (pixels & 3)];
			}
		}
	}
	break;

	case 2: // 4bpp
	{
		for (int y = 0; y < vres; y++)
		{
			uint32_t *scanline = &bitmap.pix(y);

			for (int x = 0; x < hres / 2; x++)
			{
				uint8_t const pixels = vram8[(y * (hres / 2)) + (BYTE4_XOR_BE(x))];

				*scanline++ = m_rbv_palette[0xf0 | (pixels >> 4)];
				*scanline++ = m_rbv_palette[0xf0 | (pixels & 0xf)];
			}
		}
	}
	break;

	case 3: // 8bpp
	{
		for (int y = 0; y < vres; y++)
		{
			uint32_t *scanline = &bitmap.pix(y);

			for (int x = 0; x < hres; x++)
			{
				uint8_t const pixels = vram8[(y * hres) + (BYTE4_XOR_BE(x))];
				*scanline++ = m_rbv_palette[pixels];
			}
		}
	}
	}

	return 0;
}

/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

INPUT_PORTS_START( maciici )
	PORT_START("MONTYPE")
	PORT_CONFNAME(0x0f, 0x06, "Connected monitor")
	PORT_CONFSETTING( 0x01, "15\" Portrait Display (640x870)")
	PORT_CONFSETTING( 0x02, "12\" RGB (512x384)")
	PORT_CONFSETTING( 0x06, "13\" RGB (640x480)")
INPUT_PORTS_END

/***************************************************************************
	MACHINE DRIVERS
***************************************************************************/
void maciici_state::maciici(machine_config &config)
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
	m_asc->irqf_callback().set(FUNC(maciici_state::mac_asc_irq));
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
	NSCSI_CONNECTOR(config, "scsi:3", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", mac_scsi_devices, "cdrom");
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

	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("8M,32M,64M,96M,128M");

	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");

	PALETTE(config, m_palette).set_entries(256);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);
	m_screen->set_size(640, 870);
	m_screen->set_visarea(0, 640 - 1, 0, 480 - 1);
	m_screen->set_screen_update(FUNC(maciici_state::screen_update_macrbv));
	m_screen->screen_vblank().set(FUNC(maciici_state::mac_rbv_vbl));

	/* internal ram */
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,8M,16M,32M,48M,64M,128M");

	nubus_device &nubus(NUBUS(config, "nubus", 0));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irqc_callback().set(FUNC(maciici_state::nubus_irq_c_w));
	nubus.out_irqd_callback().set(FUNC(maciici_state::nubus_irq_d_w));
	nubus.out_irqe_callback().set(FUNC(maciici_state::nubus_irq_e_w));

	NUBUS_SLOT(config, "nbc", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbd", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbe", "nubus", mac_nubus_cards, nullptr);

	MACADB(config, m_macadb, C15M);
	m_macadb->set_mcu_mode(false);
	m_macadb->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_macadb->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	m_macadb->adb_irq_callback().set(FUNC(maciici_state::adb_irq_w));
}

void maciici_state::maciisi(machine_config &config)
{
	maciici(config);

	M68030(config.replace(), m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &maciici_state::maciici_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	MACADB(config.replace(), m_macadb, C15M);

	m_via1->readpa_handler().set(FUNC(maciici_state::via_in_a_iisi));
	m_via1->readpb_handler().set(FUNC(maciici_state::via_in_b_iisi));
	m_via1->writepb_handler().set(FUNC(maciici_state::via_out_b_iisi));
	m_via1->cb2_handler().set(FUNC(maciici_state::via_out_cb2_iisi));

	EGRET(config, m_egret, EGRET_344S0100);
	m_egret->reset_callback().set(FUNC(maciici_state::egret_reset_w));
	m_egret->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_egret->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_egret->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	m_macadb->adb_data_callback().set(m_egret, FUNC(egret_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);
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
