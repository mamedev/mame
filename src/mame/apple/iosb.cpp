// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "IOSB" and "PrimeTime" I/O ASICs
    Emulation by R. Belmont

    IOSB stands for "I/O Subsystem Buffer".  The chip was originally called "BIOS" ("Buffered I/O Subsystem"),
    but they flipped the order around, presumably to avoid the obvious IBM PC reference.

    IOSB and PrimeTime contain the following:
    - A full VIA (VIA1) and a "pseudo-VIA" for VIA2.  This is not the standard pseudo-VIA found in
      chips like RBV, V8, or Sonora; it acts exactly like a real VIA, just without timers or a shift register.
    - A SWIM2 floppy controller
    - An ASC-like 4-channel audio controller
    - The "Turbo SCSI" logic from the standalone versions of DAFB and DAFB II
    - Support logic for various external subsystems (ADB, SCC, SONIC Ethernet)

    IOSB and PrimeTime are similar to Sonora, but replace the RBV/V8/VASP/Sonora pseudo-VIA with a
    real VIA core that has the timers disabled and some IER and PCR bits hard-wired to specific values.
    The pseudo-VIA returns in AMIC on the PDM-class PowerMacs and goes away forever in the TNT-class.

    PrimeTime swaps the GI/Microchip ADB modem interface for the standard Cuda hookup found in Sonora,
    and doesn't bring out the 4 VIA ID pins.  PrimeTime II is similar.
*/

#include "emu.h"
#include "iosb.h"

#include "formats/ap_dsk35.h"

#define LOG_TURBOSCSI   (1U << 1)
#define LOG_SCSIDRQ     (1U << 2)
#define LOG_IOSBREGS    (1U << 3)

#define VERBOSE (0)
#include "logmacro.h"

static constexpr u32 C7M  = 7833600;
static constexpr u32 C15M = (C7M * 2);

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(IOSB, iosb_device, "iosb", "Apple IOSB I/O ASIC")
DEFINE_DEVICE_TYPE(PRIMETIME, primetime_device, "primetime", "Apple PrimeTime I/O ASIC")
DEFINE_DEVICE_TYPE(PRIMETIMEII, primetimeii_device, "primetime2", "Apple PrimeTime II I/O ASIC")

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void iosb_base::map(address_map &map)
{
	map(0x00000000, 0x00001fff).rw(FUNC(iosb_base::mac_via_r), FUNC(iosb_base::mac_via_w)).mirror(0x00fc0000);
	map(0x00002000, 0x00003fff).rw(FUNC(iosb_base::mac_via2_r), FUNC(iosb_base::mac_via2_w)).mirror(0x00f00000);
	map(0x00010000, 0x000100ff).rw(FUNC(iosb_base::turboscsi_r), FUNC(iosb_base::turboscsi_w)).mirror(0x00fc0000);
	map(0x00010100, 0x00010103).rw(FUNC(iosb_base::turboscsi_dma_r), FUNC(iosb_base::turboscsi_dma_w)).select(0x00fc0000);
	map(0x00014000, 0x00015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00f00000);
	map(0x00018000, 0x00019fff).rw(FUNC(iosb_base::iosb_regs_r), FUNC(iosb_base::iosb_regs_w)).mirror(0x00f00000);
	map(0x0001e000, 0x0001ffff).rw(FUNC(iosb_base::swim_r), FUNC(iosb_base::swim_w)).mirror(0x00f00000);

	// IOSB always gives an ID of "0x2BAD", the VIA I/O pins determine the rest
	map(0x0fff0000, 0x0fffffff).lr32(NAME([](offs_t offset) { return 0xa55a2bad; }));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void iosb_base::device_add_mconfig(machine_config &config)
{
	R65NC22(config, m_via1, C7M / 10);
	m_via1->readpa_handler().set(FUNC(iosb_base::via_in_a));
	m_via1->readpb_handler().set(FUNC(iosb_base::via_in_b));
	m_via1->writepa_handler().set(FUNC(iosb_base::via_out_a));
	m_via1->writepb_handler().set(FUNC(iosb_base::via_out_b));
	m_via1->cb1_handler().set(FUNC(iosb_base::via_out_cb1));
	m_via1->cb2_handler().set(FUNC(iosb_base::via_out_cb2));
	m_via1->irq_handler().set(FUNC(iosb_base::via1_irq));

	R65NC22(config, m_via2, C7M / 10);
	m_via2->readpa_handler().set(FUNC(iosb_base::via2_in_a));
	m_via2->writepb_handler().set(FUNC(iosb_base::via2_out_b));
	m_via2->irq_handler().set(FUNC(iosb_base::via2_irq));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ASC(config, m_asc, C15M, asc_device::asc_type::SONORA);
	m_asc->add_route(0, "lspeaker", 1.0);
	m_asc->add_route(1, "rspeaker", 1.0);
	m_asc->irqf_callback().set(FUNC(iosb_base::asc_irq));

	SWIM2(config, m_fdc, C15M);
	m_fdc->devsel_cb().set(FUNC(iosb_base::devsel_w));
	m_fdc->phases_cb().set(FUNC(iosb_base::phases_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);
}

void iosb_device::device_add_mconfig(machine_config &config)
{
	iosb_base::device_add_mconfig(config);

	RTC3430042(config, m_rtc, XTAL(32'768));
	m_rtc->cko_cb().set(m_via1, FUNC(via6522_device::write_ca2));
}

//-------------------------------------------------
//  iosb_base - constructor
//-------------------------------------------------

iosb_base::iosb_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_adb_st(*this),
	m_cb1(*this),
	m_cb2(*this),
	m_dfac_clock_w(*this),
	m_dfac_data_w(*this),
	m_dfac_latch_w(*this),
	m_pa1(*this, 0),
	m_pa2(*this, 0),
	m_pa4(*this, 0),
	m_pa6(*this, 0),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_ncr(*this, finder_base::DUMMY_TAG),
	m_via1(*this, "via1"),
	m_via2(*this, "via2"),
	m_asc(*this, "asc"),
	m_fdc(*this, "fdc"),
	m_floppy(*this, "fdc:%d", 0U),
	m_nubus_irqs(0xff),
	m_via_interrupt(0),
	m_via2_interrupt(0),
	m_scc_interrupt(0),
	m_last_taken_interrupt(-1),
	m_cur_floppy(nullptr),
	m_hdsel(0),
	m_adb_interrupt(0),
	m_drq(0),
	m_scsi_irq(0),
	m_asc_irq(0),
	m_scsi_read_cycles(3),
	m_scsi_write_cycles(3),
	m_scsi_dma_read_cycles(3),
	m_scsi_dma_write_cycles(3),
	m_scsi_dma_result(0),
	m_scsi_second_half(false)
{
	std::fill(std::begin(m_iosb_regs), std::end(m_iosb_regs), 0);
}

iosb_device::iosb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	iosb_base(mconfig, IOSB, tag, owner, clock),
	m_rtc(*this,"rtc")
{
}

primetime_device::primetime_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	iosb_base(mconfig, type, tag, owner, clock),
	write_pb4(*this),
	write_pb5(*this),
	read_pb3(*this, 0)
{
}

primetime_device::primetime_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	primetime_device(mconfig, PRIMETIME, tag, owner, clock)
{
}

primetimeii_device::primetimeii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	primetime_device(mconfig, PRIMETIMEII, tag, owner, clock),
	m_ata_irq(0)
{
	std::fill(std::begin(m_primetimeii_regs), std::end(m_primetimeii_regs), 0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iosb_base::device_start()
{
	m_maincpu->set_emmu_enable(true);

	m_6015_timer = timer_alloc(FUNC(iosb_base::mac_6015_tick), this);
	m_6015_timer->adjust(attotime::never);

	save_item(NAME(m_via_interrupt));
	save_item(NAME(m_via2_interrupt));
	save_item(NAME(m_scc_interrupt));
	save_item(NAME(m_last_taken_interrupt));
	save_item(NAME(m_hdsel));
	save_item(NAME(m_via2_ca1_hack));
	save_item(NAME(m_nubus_irqs));
	save_item(NAME(m_iosb_regs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void iosb_base::device_reset()
{
	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));

	m_via2_ca1_hack = 1;
	m_via2->write_ca1(1);
	m_via2->write_cb1(1);

	// set defaults that make VIA2 pseudo-ish
	m_via2->write(11, 0xc0);
	m_via2->write(12, 0x26);
	m_via2->write(13, 0x00);
	m_via2->write(14, 0x80);
}

TIMER_CALLBACK_MEMBER(iosb_base::mac_6015_tick)
{
	m_via1->write_ca1(CLEAR_LINE);
	m_via1->write_ca1(ASSERT_LINE);
}

uint8_t iosb_base::via_in_a()
{
	u8 result = m_pa1() << 1;
	result |= m_pa2() << 2;
	result |= m_pa4() << 4;
	result |= m_pa6() << 6;
	return result;
}

uint8_t iosb_base::via_in_b()
{
	return m_adb_interrupt ? 0 : 8;
}

void iosb_base::via_out_cb1(int state)
{
	m_cb1(state & 1);
}

void iosb_base::via_out_cb2(int state)
{
	m_cb2(state & 1);
}

void iosb_base::via_out_a(uint8_t data)
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

void iosb_base::via_out_b(uint8_t data)
{
	m_adb_st((data & 0x30) >> 4);
}

void iosb_base::via1_irq(int state)
{
	m_via_interrupt = state;
	field_interrupts();
}

void iosb_base::via2_out_b(uint8_t data)
{
	m_dfac_latch_w(BIT(data, 0));
	m_dfac_data_w(BIT(data, 3));
	m_dfac_clock_w(BIT(data, 4));
}

void iosb_base::via2_irq(int state)
{
	m_via2_interrupt = state;
	field_interrupts();
}

void iosb_base::field_interrupts()
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

void iosb_base::scc_irq_w(int state)
{
	m_scc_interrupt = (state == ASSERT_LINE) ? 1 : 0;
	field_interrupts();
}

template <u8 mask>
void iosb_base::via2_irq_w(int state)
{
	m_nubus_irqs = m_via2->read(1);

	if (state)
	{
		m_nubus_irqs &= ~mask;
	}
	else
	{
		m_nubus_irqs |= mask;
	}

	m_nubus_irqs |= 0x86;

	if ((m_nubus_irqs & 0x79) != 0x79)
	{
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

template void iosb_base::via2_irq_w<0x40>(int state);
template void iosb_base::via2_irq_w<0x20>(int state);
template void iosb_base::via2_irq_w<0x10>(int state);
template void iosb_base::via2_irq_w<0x08>(int state);
template void iosb_base::via2_irq_w<0x01>(int state);

u8 iosb_base::via2_in_a()
{
	return m_nubus_irqs;
}


void iosb_base::scsi_irq_w(int state)
{
	m_via2->write_cb2(state ^ 1);
	m_scsi_irq = state;
}

void iosb_base::asc_irq(int state)
{
	m_via2->write_cb1(state ^ 1);
	m_asc_irq = state;
}

void iosb_base::cb1_w(int state)
{
	m_via1->write_cb1(state);
}

void iosb_base::cb2_w(int state)
{
	m_via1->write_cb2(state);
}

uint16_t iosb_base::mac_via_r(offs_t offset)
{
	uint16_t data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		via_sync();

	data = m_via1->read(offset);

	return (data & 0xff) | (data << 8);
}

void iosb_base::mac_via_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	via_sync();

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);
}

u16 iosb_base::mac_via2_r(offs_t offset)
{
	int data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		via_sync();

	data = m_via2->read(offset);
	// a little more pseudo-ness: bit 0 of the IFR shows the live line states of the IRQs and DRQ
	if (offset == 13)
	{
		data &= ~0x19;
		data |= (m_drq) ? 0x01 : 0;
		data |= (m_scsi_irq) ? 0x08 : 0;
		data |= (m_asc_irq) ? 0x10 : 0;
	}
	return (data & 0xff) | (data << 8);
}

void iosb_base::mac_via2_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	// what makes VIA2 "pseudo" is that regs 4-10 can't be written, and 11 and 12 have canned values
	switch (offset)
	{
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			return;

		case 11:
			m_via2->write(11, 0xc0);
			return;

		case 12:
			m_via2->write(12, 0x26);
			return;
	}

	via_sync();

	if (ACCESSING_BITS_0_7)
		m_via2->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via2->write(offset, (data >> 8) & 0xff);
}

void iosb_base::via_sync()
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

uint16_t iosb_base::swim_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu->adjust_icount(-5);
	}

	u16 result = m_fdc->read((offset >> 8) & 0xf);
	return result << 8;
}
void iosb_base::swim_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_fdc->write((offset >> 8) & 0xf, data & 0xff);
	else
		m_fdc->write((offset >> 8) & 0xf, data >> 8);
}

void iosb_base::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void iosb_base::devsel_w(uint8_t devsel)
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

u8 iosb_base::turboscsi_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu->adjust_icount(-m_scsi_read_cycles);
	}
	return m_ncr->read(offset>>4);
}

void iosb_base::turboscsi_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_TURBOSCSI, "turboscsi_w: %02x @ %x (PC=%x)\n", data, offset>>4, m_maincpu->pc());
	m_maincpu->adjust_icount(-m_scsi_write_cycles);
	m_ncr->write(offset>>4, data);
}

u32 iosb_base::turboscsi_dma_r(offs_t offset, u32 mem_mask)
{
	if (mem_mask != 0xffffffff && mem_mask != 0xffff0000)
	{
		fatalerror("IOSB: turboscsi_dma_r mem_mask not handled %08x\n", mem_mask);
	}

	if (!machine().side_effects_disabled() && BIT(offset << 1, 18))
	{
		m_maincpu->adjust_icount(-m_scsi_dma_read_cycles);
	}

	if (!m_drq)
	{
		// The real DAFB simply holds off /DTACK here, we simulate that
		// by rewinding and repeating the instruction until DRQ is asserted.
		m_maincpu->restart_this_instruction();
		m_maincpu->spin_until_time(attotime::from_usec(50));
		return 0xffff;
	}

	LOGMASKED(LOG_TURBOSCSI, "dma_r mask %08x (%s)\n", mem_mask, machine().describe_context());

	if (mem_mask == 0xffffffff)
	{
		if (!m_scsi_second_half)
		{
			m_scsi_dma_result = m_ncr->dma16_swap_r()<<16;
			m_scsi_second_half = true;
		}

		if (!m_drq)
		{
			// The real DAFB simply holds off /DTACK here, we simulate that
			// by rewinding and repeating the instruction until DRQ is asserted.
			m_maincpu->restart_this_instruction();
			m_maincpu->spin_until_time(attotime::from_usec(50));
			return 0xffff;
		}

		m_scsi_second_half = false;
		m_scsi_dma_result |= m_ncr->dma16_swap_r();
	}
	else if (mem_mask == 0xffff0000)
	{
		m_scsi_dma_result = m_ncr->dma16_swap_r()<<16;
	}
	return m_scsi_dma_result;
}

void iosb_base::turboscsi_dma_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (!machine().side_effects_disabled() && BIT(offset << 1, 18))
	{
		m_maincpu->adjust_icount(-m_scsi_dma_write_cycles);
	}

	LOGMASKED(LOG_TURBOSCSI, "dma_w %08x (mask %04x)\n", data & mem_mask, mem_mask);

	if (!m_drq)
	{
		m_maincpu->restart_this_instruction();
		m_maincpu->spin_until_time(attotime::from_usec(50));
		return;
	}

	if (mem_mask == 0xffffffff)
	{
		if (!m_scsi_second_half)
		{
			m_ncr->dma16_swap_w(data>>16);
		}
		m_scsi_second_half = true;
		if (!m_drq)
		{
			m_maincpu->restart_this_instruction();
			m_maincpu->spin_until_time(attotime::from_usec(50));
			return;
		}
		m_scsi_second_half = false;
		m_ncr->dma16_swap_w(data & 0xffff);
	}
	else if (mem_mask == 0xffff0000)
	{
		m_ncr->dma16_swap_w(data >> 16);
	}
	else if (mem_mask == 0xff000000)
	{
		m_ncr->dma_w(data >> 24);
	}
	else
	{
		fatalerror("IOSB: turboscsi_dma_w unhandled mask %08x\n", mem_mask);
	}
}

void iosb_base::scsi_drq_w(int state)
{
	LOGMASKED(LOG_SCSIDRQ, "SCSI DRQ %d (was %d)\n", state, m_drq);
	m_drq = state;
	m_via2->write_ca2(state);
}

u16 iosb_base::iosb_regs_r(offs_t offset)
{
	return m_iosb_regs[offset>>7];
}

void iosb_base::iosb_regs_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_IOSBREGS, "iosb_regs_w: %04x @ %x, mask %04x\n", data, offset>>7, mem_mask);
	COMBINE_DATA(&m_iosb_regs[offset>>7]);

	if ((offset>>7) == 2)
	{
		const int times[4] = { 5, 5, 4, 3 };
		m_scsi_dma_read_cycles = times[(m_iosb_regs[2]>>8) & 3];
		m_scsi_dma_write_cycles = (times[(m_iosb_regs[2]>>11) & 3]);
		LOGMASKED(LOG_TURBOSCSI, "SCSI DMA read cycles %d write cycles %d\n", m_scsi_dma_read_cycles, m_scsi_dma_write_cycles);
	}
}

// ------------------------------------------- IOSB device
uint8_t iosb_device::via_in_b()
{
	return iosb_base::via_in_b() | m_rtc->data_r();
}

void iosb_device::via_out_b(uint8_t data)
{
	iosb_base::via_out_b(data);

	m_rtc->ce_w((data & 0x04)>>2);
	m_rtc->data_w(data & 0x01);
	m_rtc->clk_w((data >> 1) & 0x01);
}

// ------------------------------------------- PrimeTime device
uint8_t primetime_device::via_in_b()
{
	return read_pb3() << 3;
}

void primetime_device::via_out_b(uint8_t data)
{
	write_pb4(BIT(data, 4));
	write_pb5(BIT(data, 5));
}

// ------------------------------------------- PrimeTime II device
void primetimeii_device::device_start()
{
	primetime_device::device_start();
	save_item(NAME(m_ata_irq));
	save_item(NAME(m_primetimeii_regs));
}

void primetimeii_device::map(address_map &map)
{
	iosb_base::map(map);
	map(0x0001a100, 0x0001a10f).r(FUNC(primetimeii_device::ata_regs_r)).mirror(0x00f00000);
}

// This may actually be in F108, the boundaries between the two chips aren't completely clear
u16 primetimeii_device::ata_regs_r(offs_t offset)
{
	switch (offset)
	{
		case 0: // special interrupt status: bit 6 = VBL IRQ, bit 5 = ATA IRQ
			return ((m_nubus_irqs ^ 0xff) & 0x40) | (m_ata_irq << 5);

		default:
			LOGMASKED(LOG_IOSBREGS, "%s: Unhandled ata_regs_r @ %x\n", tag(), offset);
			break;
	}

	return 0;
}

void primetimeii_device::ata_irq_w(int state)
{
	via2_irq_w<0x10>(state);
	m_ata_irq = state;
}
