// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "MSC" (Main System Controller) system ASIC
    Emulation by R. Belmont

    MSC contains the following:
    - A memory controller for up to 36MB (up to 4MB soldered and 32MB of SIMMs)
    - A modified VIA (VIA1) and a "pseudo-VIA", which is basically a combination GPIO and
      interrupt controller that looks somewhat like a VIA with no timers and no shift register.
    - An EASC-like 4-channel audio controller
    - Power management support functions

    MSC II is identical, but with a 68040 bus interface instead of 68030.
*/

#include "emu.h"
#include "msc.h"

static constexpr u32 C7M  = 7833600;
static constexpr u32 C15M = (C7M * 2);

[[maybe_unused]] static constexpr u8 SOUND_POWER = 0; // 0 = DFAC power off, 1 = DFAC power on
static constexpr u8 SOUND_BUSY      = 6;    // 1 = ASC FIFO accessed since last read of this register
[[maybe_unused]] static constexpr u8 SOUND_LATCH = 7; // 1 = DFAC is powered up

DEFINE_DEVICE_TYPE(MSC, msc_device, "msc", "Apple MSC system ASIC")
DEFINE_DEVICE_TYPE(MSC_VIA, mscvia_device, "mscvia", "Apple MSC integrated VIA")

void msc_device::map(address_map &map)
{
	map(0x00000000, 0x000fffff).r(FUNC(msc_device::rom_switch_r)).mirror(0x0ff00000);

	map(0x10f00000, 0x10f01fff).rw(FUNC(msc_device::via_r), FUNC(msc_device::via_w));
	map(0x10f14000, 0x10f15fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0x10f26000, 0x10f27fff).rw(FUNC(msc_device::via2_r), FUNC(msc_device::via2_w));
	map(0x10fa0000, 0x10fa0003).w(FUNC(msc_device::power_cycle_w));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void msc_device::device_add_mconfig(machine_config &config)
{
	MSC_VIA(config, m_via1, C7M / 10);
	m_via1->readpa_handler().set(FUNC(msc_device::via_in_a));
	m_via1->readpb_handler().set(FUNC(msc_device::via_in_b));
	m_via1->writepa_handler().set(FUNC(msc_device::via_out_a));
	m_via1->writepb_handler().set(FUNC(msc_device::via_out_b));
	m_via1->cb2_handler().set(FUNC(msc_device::via_out_cb2));
	m_via1->irq_handler().set(FUNC(msc_device::via1_irq));

	APPLE_PSEUDOVIA(config, m_pseudovia, C15M);
	m_pseudovia->irq_callback().set(FUNC(msc_device::via2_irq));
	m_pseudovia->readpa_handler().set(FUNC(msc_device::via2_in_a));
	m_pseudovia->readpb_handler().set(FUNC(msc_device::via2_in_b));
	m_pseudovia->writepa_handler().set(FUNC(msc_device::via2_out_a));
	m_pseudovia->writepb_handler().set(FUNC(msc_device::via2_out_b));
	m_pseudovia->readmsc_handler().set(FUNC(msc_device::msc_pseudovia_r));
	m_pseudovia->writemsc_handler().set(FUNC(msc_device::msc_pseudovia_w));
	m_pseudovia->readvideo_handler().set(FUNC(msc_device::msc_config_r));
	m_pseudovia->writevideo_handler().set(FUNC(msc_device::msc_config_w));

	ASC(config, m_asc, C15M, asc_device::asc_type::EASC);
	m_asc->add_route(0, tag(), 1.0);
	m_asc->add_route(1, tag(), 1.0);
	m_asc->irqf_callback().set(m_pseudovia, FUNC(pseudovia_device::asc_irq_w));
}

mscvia_device::mscvia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: via6522_device(mconfig, MSC_VIA, tag, owner, clock)
{
}

msc_device::msc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MSC, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	write_pb4(*this),
	write_pb5(*this),
	write_cb2(*this),
	write_vbl(*this),
	read_pb3(*this, 0),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_pmu(*this, finder_base::DUMMY_TAG),
	m_via1(*this, "via1"),
	m_pseudovia(*this, "pseudovia"),
	m_asc(*this, "asc"),
	m_rom(*this, finder_base::DUMMY_TAG),
	m_via_interrupt(0),
	m_pmu_interrupt(0),
	m_via2_interrupt(0),
	m_scc_interrupt(0),
	m_overlay(false),
	m_pmu_req(1),
	m_pmu_ack(1),
	m_msc_config(0),
	m_msc_clock_ctrl(0),
	m_msc_sound_ctrl(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msc_device::device_start()
{
	m_stream = stream_alloc(8, 2, m_asc->clock(), STREAM_SYNCHRONOUS);

	m_6015_timer = timer_alloc(FUNC(msc_device::msc_6015_tick), this);
	m_6015_timer->adjust(attotime::never);

	save_item(NAME(m_via_interrupt));
	save_item(NAME(m_pmu_interrupt));
	save_item(NAME(m_via2_interrupt));
	save_item(NAME(m_scc_interrupt));
	save_item(NAME(m_overlay));
	save_item(NAME(m_last_taken_interrupt));
	save_item(NAME(m_pmu_ack));
	save_item(NAME(m_pmu_req));
	save_item(NAME(m_msc_config));
	save_item(NAME(m_msc_clock_ctrl));
	save_item(NAME(m_msc_sound_ctrl));

	m_rom_ptr = &m_rom[0];
	m_rom_size = m_rom.length() << 2;

	// power management needs to know if the user is idle but sound is playing, so as not to put the machine to sleep
	m_maincpu->space(AS_PROGRAM).install_write_tap(0x50f14000, 0x50f15fff, "snd_latch_mon", [this](offs_t offset, u32 &data, u32 mem_mask)
	{
		this->m_msc_sound_ctrl |= (1 << SOUND_BUSY);
	});
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void msc_device::device_reset()
{
	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));

	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = 0;
	m_last_taken_interrupt = -1;

	// main cpu shouldn't start until PMU wakes it up
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_overlay = true;

	// put ROM mirror at 0
	address_space &space = m_maincpu->space(AS_PROGRAM);
	const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.unmap_write(0x00000000, memory_end);
	space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);
}

void msc_device::sound_stream_update(sound_stream &stream)
{
	stream.copy(0, 0);
	stream.copy(1, 1);
}

u32 msc_device::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (m_overlay && !machine().side_effects_disabled())
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_end = m_ram_size - 1;
		void *memory_data = m_ram_ptr;
		offs_t memory_mirror = memory_end & ~memory_end;

		space.install_ram(0x00000000, memory_end & ~memory_mirror, memory_mirror, memory_data);
		m_overlay = false;
	}

	return m_rom_ptr[offset & ((m_rom_size - 1) >> 2)];
}

void msc_device::set_cpu_clock(XTAL clock)
{
	m_cpu_clock = clock.value();
}

void msc_device::power_cycle_w(u32 data)
{
	// CPU power down control.  Expects the CPU to reset after a short delay and resume.
	// We make that no delay, since we don't care about saving power.
	// Called with 0 for normal delay and resume, 0xffffffff for actual full system sleep on 210/230,
	// ff000000 for full system sleep on later machines, and 5a000000 for power cycle.
	if ((data == 0) || (data == 0x5a000000))
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
}

u8 msc_device::msc_config_r()
{
	return m_msc_config;
}

void msc_device::msc_config_w(u8 data)
{
	if (BIT(data, 1) != BIT(m_msc_clock_ctrl, 1))
	{
		m_maincpu->set_clock(BIT(data, 1) ? clock() / 2 : m_cpu_clock);
	}

	m_msc_config = data;
}

u8 msc_device::msc_pseudovia_r(offs_t offset)
{
	switch (offset)
	{
		case 1:
			return m_msc_clock_ctrl;

		case 2:
		{
			const u8 retval = m_msc_sound_ctrl;
			m_msc_sound_ctrl &= ~(1 << SOUND_BUSY);
			return retval;
		}
	}

	return 0;
}

void msc_device::msc_pseudovia_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 1:
			m_msc_clock_ctrl = data;
			break;

		case 2:
			m_msc_sound_ctrl = data;
			break;
	}
}

void msc_device::set_ram_info(u32 *ram, u32 size)
{
	m_ram_ptr = ram;
	m_ram_size = size;
}

TIMER_CALLBACK_MEMBER(msc_device::msc_6015_tick)
{
	m_via1->write_ca1(CLEAR_LINE);
	m_via1->write_ca1(ASSERT_LINE);

	write_vbl(ASSERT_LINE);
}

u8 msc_device::via_in_a()
{
	return 0x07;
}

u8 msc_device::via_in_b()
{
	return 0;
}

void msc_device::via_out_cb2(int state)
{
	write_cb2(state & 1);
}

void msc_device::via_out_a(u8 data)
{
}

void msc_device::via_out_b(u8 data)
{
}

void msc_device::via1_irq(int state)
{
	m_via_interrupt = state;
	field_interrupts();
}

u8 msc_device::via2_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		via_sync();
		via_sync();
		via_sync();
		via_sync();
	}

	return m_pseudovia->read(offset);
}

void msc_device::via2_w(offs_t offset, u8 data)
{
	via_sync();
	via_sync();
	via_sync();
	via_sync();
	m_pseudovia->write(offset, data);
}

u8 msc_device::via2_in_a()
{
	return 0;
}

u8 msc_device::via2_in_b()
{
	return ((m_pmu_ack & 1) << 1) | (m_pmu_req << 2);
}

void msc_device::via2_out_a(u8 data)
{
}

void msc_device::via2_out_b(u8 data)
{
	if (m_pmu_req != BIT(data, 2))
	{
		m_pmu_req = BIT(data, 2);
		m_maincpu->spin_until_time(attotime::from_usec(80));
	}
}

void msc_device::via2_irq(int state)
{
	m_via2_interrupt = state;
	field_interrupts();
}

int msc_device::get_pmu_req()
{
	return m_pmu_req;
}

void msc_device::pmu_ack_w(int state)
{
	if (m_pmu_ack != state)
	{
		m_pmu_ack = state;
		m_pmu->spin_until_time(attotime::from_usec(20));
	}
}

void msc_device::pmu_reset_w(int state)
{
	if (!state)
	{
		// put ROM mirror at 0 for reset
		address_space &space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
		const u32 memory_end = memory_size - 1;
		offs_t memory_mirror = memory_end & ~(memory_size - 1);

		space.unmap_write(0x00000000, memory_end);
		space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);
		m_overlay = true;
	}
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}

void msc_device::cb1_int_hack(int state)
{
	if (state)
	{
		m_via1->cb1_int_hack(ASSERT_LINE);
	}
}

void msc_device::field_interrupts()
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
	else if (m_pmu_interrupt && ((m_via1->read(via6522_device::VIA_IER) & 0x90) == 0x90))
	{
		take_interrupt = 1;
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

void msc_device::scc_irq_w(int state)
{
	m_scc_interrupt = (state == ASSERT_LINE);
	field_interrupts();
}

void msc_device::scsi_irq_w(int state)
{
	m_pseudovia->scsi_irq_w(state);
}

void msc_device::slot0_irq_w(int state)
{
	m_pseudovia->slot_irq_w<0x08>(state);
}

void msc_device::slot1_irq_w(int state)
{
	m_pseudovia->slot_irq_w<0x10>(state);
}

void msc_device::slot2_irq_w(int state)
{
	m_pseudovia->slot_irq_w<0x20>(state);
}

void msc_device::lcd_irq_w(int state)
{
	m_pseudovia->slot_irq_w<0x40>(state);
}

void msc_device::cb1_w(int state)
{
	m_via1->write_cb1(state);
}

void msc_device::cb2_w(int state)
{
	m_via1->write_cb2(state);
}

u16 msc_device::via_r(offs_t offset)
{
	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		via_sync();

	return m_via1->read(offset) << 8;
}

void msc_device::via_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;
	data >>= 8;

	via_sync();
	m_via1->write(offset, data & 0xff);
}

void msc_device::via_sync()
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
