// license:BSD-3-Clause
// copyright-holders:Melissa Goad

// ARM PrimeCell PL192 VIC emulation

#include "emu.h"
#include "machine/bankdev.h"
#include "machine/vic_pl192.h"

#define LOG_GENERAL (1U << 0)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

void vic_pl192_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if(u32 intrs = (raw_intr | soft_intr) & intr_en)
	{
		if(intrs & intr_select) m_out_fiq_func(1);
		else m_out_fiq_func(0);

		if(intrs & ~intr_select) m_out_irq_func(1);
		else m_out_irq_func(0);
	}
}

void vic_pl192_device::set_irq_line(int irq, int state)
{
	u32 mask = (1 << irq);

	if(state)
	{
		raw_intr |= mask;
	}
	else
	{
		raw_intr &= ~mask;
	}

	timer_set(attotime::zero, TIMER_CHECK_IRQ);
}

void vic_pl192_device::map(address_map &map)
{
	map(0x000, 0x003).lr32([this](offs_t offset){ return raw_intr & ~intr_select; }, "irq_status"); //IRQ_STATUS
	map(0x004, 0x007).lr32([this](offs_t offset){ return raw_intr & intr_select; }, "fiq_status"); //FIQ_STATUS
	map(0x008, 0x00b).lr32([this](offs_t offset){ return raw_intr; }, "raw_intr");
	map(0x00c, 0x00f).lrw32(NAME([this](offs_t offset){ return intr_select; }), NAME([this](offs_t offset, u32 data){ intr_select = data; timer_set(attotime::zero, TIMER_CHECK_IRQ); }));
	map(0x010, 0x013).lrw32(NAME([this](offs_t offset){ return intr_en; }), NAME([this](offs_t offset, u32 data){ intr_en = data; timer_set(attotime::zero, TIMER_CHECK_IRQ); }));
	map(0x014, 0x017).lw32([this](u32 data){ intr_en &= ~data; }, "intr_en_clear");
	map(0x018, 0x01b).lrw32(NAME([this](offs_t offset){ return soft_intr; }), NAME([this](offs_t offset, u32 data){ soft_intr = data; timer_set(attotime::zero, TIMER_CHECK_IRQ); }));
	map(0x01c, 0x01f).lw32([this](u32 data){ soft_intr &= ~data; }, "soft_intr_clear");
	map(0x020, 0x020).lrw8(NAME([this](offs_t offset){ return protection; }), NAME([this](offs_t offset, u8 data){ protection = data & 1; })).umask32(0x000000ff);
	map(0x024, 0x025).lrw8(NAME([this](offs_t offset){ return sw_priority_mask; }), NAME([this](offs_t offset, u16 data){ sw_priority_mask = data; })).umask32(0x0000ffff);
	map(0x028, 0x028).lrw8(NAME([this](offs_t offset){ return daisy_priority; }), NAME([this](offs_t offset, u8 data){ daisy_priority = data & 0xf; })).umask32(0x000000ff);
	map(0x100, 0x17f).lrw32(NAME([this](offs_t offset){ return vectaddr[(offset & 0x7c) >> 2]; }), NAME([this](offs_t offset, u32 data){ vectaddr[(offset & 0x7c) >> 2] = data; }));
	map(0x200, 0x27f).lrw8(NAME([this](offs_t offset){ return vectprio[(offset & 0x7c) >> 2]; }), NAME([this](offs_t offset, u32 data){ vectprio[(offset & 0x7c) >> 2] = data & 0xf; }));
	map(0xf00, 0xf03).lrw32(NAME([this](offs_t offset){ return vicaddress; }), NAME([this](offs_t offset, u32 data){ vectaddr[(offset & 0x7c) >> 2] = data; }));
	map(0xfe0, 0xfe0).lr8([this](offs_t offset){ return periph_id[0]; }, "periph_id0").umask32(0x000000ff);
	map(0xfe4, 0xfe4).lr8([this](offs_t offset){ return periph_id[1]; }, "periph_id1").umask32(0x000000ff);
	map(0xfe8, 0xfe8).lr8([this](offs_t offset){ return periph_id[2]; }, "periph_id2").umask32(0x000000ff);
	map(0xfec, 0xfec).lr8([this](offs_t offset){ return periph_id[3]; }, "periph_id3").umask32(0x000000ff);
	map(0xff0, 0xff0).lr8([this](offs_t offset){ return pcell_id[0]; }, "pcell_id0").umask32(0x000000ff);
	map(0xff4, 0xff4).lr8([this](offs_t offset){ return pcell_id[1]; }, "pcell_id1").umask32(0x000000ff);
	map(0xff8, 0xff8).lr8([this](offs_t offset){ return pcell_id[2]; }, "pcell_id2").umask32(0x000000ff);
	map(0xffc, 0xffc).lr8([this](offs_t offset){ return pcell_id[3]; }, "pcell_id3").umask32(0x000000ff);
}

device_memory_interface::space_config_vector vic_pl192_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_mmio_config)
	};
}

void vic_pl192_device::device_resolve_objects()
{
	// resolve callbacks
	m_out_irq_func.resolve_safe();
	m_out_fiq_func.resolve_safe();
}

void vic_pl192_device::device_start()
{
	save_item(NAME(raw_intr));
	save_item(NAME(intr_select));
	save_item(NAME(intr_en));
	save_item(NAME(soft_intr));
	save_item(NAME(vectaddr));
	save_item(NAME(vicaddress));
	save_item(NAME(protection));
	save_item(NAME(sw_priority_mask));
	save_item(NAME(daisy_priority));
	save_item(NAME(vectprio));
}

void vic_pl192_device::device_reset()
{
	raw_intr = intr_select = intr_en = soft_intr = vicaddress = protection = 0;
	sw_priority_mask = 0xffff;
	daisy_priority = 0xf;

	for(int i = 0; i < 32; i++)
	{
		vectaddr[i] = 0;
	}

	for(int i = 0; i < 32; i++)
	{
		vectprio[i] = 0xf;
	}
}

DEFINE_DEVICE_TYPE(PL192_VIC, vic_pl192_device, "vic_pl192", "ARM PL192 VIC")

vic_pl192_device::vic_pl192_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_mmio_config("mmio", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_out_irq_func(*this)
	, m_out_fiq_func(*this)
	, periph_id{0x92, 0x11, 0x04, 0x00}
	, pcell_id{0x0d, 0xf0, 0x05, 0xb1}
{
}

vic_pl192_device::vic_pl192_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vic_pl192_device(mconfig, PL192_VIC, tag, owner, clock)
{
}
