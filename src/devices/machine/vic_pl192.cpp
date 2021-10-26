// license:BSD-3-Clause
// copyright-holders:Devin Acker, Melissa Goad

// ARM PrimeCell PL910/PL192 VIC emulation

#include "emu.h"
#include "machine/vic_pl192.h"

#define LOG_GENERAL (1U << 0)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PL190_VIC, vic_pl190_device, "vic_pl190", "ARM PL190 VIC")
DEFINE_DEVICE_TYPE(UPD800468_VIC, vic_upd800468_device, "vic_upd800468", "NEC uPD800468 VIC")
DEFINE_DEVICE_TYPE(PL192_VIC, vic_pl192_device, "vic_pl192", "ARM PL192 VIC")

void vic_pl190_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	u32 intrs = (raw_intr | soft_intr) & intr_en;

	if (intrs & intr_select) m_out_fiq_func(1);
	else m_out_fiq_func(0);

	if (intrs & ~intr_select & priority_mask) m_out_irq_func(1);
	else m_out_irq_func(0);
}

void vic_pl190_device::set_irq_line(int irq, int state)
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

u32 vic_pl190_device::irq_status_r()
{
	return raw_intr & ~intr_select;
}

u32 vic_pl190_device::fiq_status_r()
{
	return raw_intr & intr_select;
}

u32 vic_pl190_device::raw_intr_r()
{
	return raw_intr;
}

u32 vic_pl190_device::int_select_r()
{
	return intr_select;
}

void vic_pl190_device::int_select_w(u32 data)
{
	intr_select = data;
	timer_set(attotime::zero, TIMER_CHECK_IRQ);
}

u32 vic_pl190_device::int_enable_r()
{
	return intr_en;
}

void vic_pl190_device::int_enable_w(u32 data)
{
	intr_en |= data;
	timer_set(attotime::zero, TIMER_CHECK_IRQ);
}

void vic_pl190_device::int_en_clear_w(u32 data)
{
	intr_en &= ~data;
	timer_set(attotime::zero, TIMER_CHECK_IRQ);
}

u32 vic_pl190_device::soft_int_r()
{
	return soft_intr;
}

void vic_pl190_device::soft_int_w(u32 data)
{
	soft_intr |= data;
	timer_set(attotime::zero, TIMER_CHECK_IRQ);
}

void vic_pl190_device::soft_int_clear_w(u32 data)
{
	soft_intr &= ~data;
	timer_set(attotime::zero, TIMER_CHECK_IRQ);
}

u8 vic_pl190_device::protection_r()
{
	return protection;
}

void vic_pl190_device::protection_w(u8 data)
{
	protection = BIT(data, 0);
}

u32 vic_pl190_device::cur_vect_addr_r()
{
	if (!machine().side_effects_disabled())
		update_vector();

	return vicaddress;
}

void vic_pl190_device::cur_vect_addr_w(u32 data)
{
	priority_mask = ~0;
	priority = ~0;
	timer_set(attotime::zero, TIMER_CHECK_IRQ);
}

u32 vic_pl190_device::def_vect_addr_r()
{
	return defaddress;
}

void vic_pl190_device::def_vect_addr_w(u32 data)
{
	defaddress = data;
}

u32 vic_pl190_device::vect_addr_r(offs_t offset)
{
	return vectaddr[offset];
}

void vic_pl190_device::vect_addr_w(offs_t offset, u32 data)
{
	vectaddr[offset] = data;
}

u32 vic_pl190_device::vect_ctl_r(offs_t offset)
{
	return vectctl[offset];
}

void vic_pl190_device::vect_ctl_w(offs_t offset, u32 data)
{
	vectctl[offset] = data;
}

void vic_pl190_device::update_vector()
{
	u32 intrs = (raw_intr | soft_intr) & intr_en & ~intr_select;
	u32 newmask = 0;

	for (u8 i = 0; i < std::min(priority, num_vectors); i++)
	{
		u8 irq = vectctl[i] & 0x1f;

		if (!BIT(vectctl[i], 5))
		{
			// vector is disabled
			continue;
		}
		else if (BIT(intrs, irq))
		{
			// this interrupt is enabled and pending
			// take it and update priority_mask to only allow higher priority interrupts
			priority_mask = newmask;
			priority = i;
			vicaddress = vectaddr[i];
			return;
		}
		else
		{
			// interrupt isn't pending, but it's higher priority
			// allow it to be taken later
			newmask |= (1 << irq);
		}
	}

	// no vectored interrupt taken, use the default
	priority_mask = ~0;
	priority = ~0;
	vicaddress = defaddress;
}

void vic_pl190_device::map(address_map &map)
{
	map(0x000, 0x003).r(FUNC(vic_pl190_device::irq_status_r));
	map(0x004, 0x007).r(FUNC(vic_pl190_device::fiq_status_r));
	map(0x008, 0x00b).r(FUNC(vic_pl190_device::raw_intr_r));
	map(0x00c, 0x00f).rw(FUNC(vic_pl190_device::int_select_r), FUNC(vic_pl190_device::int_select_w));
	map(0x010, 0x013).rw(FUNC(vic_pl190_device::int_enable_r), FUNC(vic_pl190_device::int_enable_w));
	map(0x014, 0x017).w(FUNC(vic_pl190_device::int_en_clear_w));
	map(0x018, 0x01b).rw(FUNC(vic_pl190_device::soft_int_r), FUNC(vic_pl190_device::soft_int_w));
	map(0x01c, 0x01f).w(FUNC(vic_pl190_device::soft_int_clear_w));
	map(0x020, 0x020).rw(FUNC(vic_pl190_device::protection_r), FUNC(vic_pl190_device::protection_w)).umask32(0x000000ff);
	map(0x030, 0x033).rw(FUNC(vic_pl190_device::cur_vect_addr_r), FUNC(vic_pl190_device::cur_vect_addr_w));
	map(0x034, 0x037).rw(FUNC(vic_pl190_device::def_vect_addr_r), FUNC(vic_pl190_device::def_vect_addr_w));
	map(0x100, 0x13f).rw(FUNC(vic_pl190_device::vect_addr_r), FUNC(vic_pl190_device::vect_addr_w));
	map(0x200, 0x23f).rw(FUNC(vic_pl190_device::vect_ctl_r), FUNC(vic_pl190_device::vect_ctl_w));
	map(0xfe0, 0xfe0).lr8([this](offs_t offset){ return periph_id[0]; }, "periph_id0").umask32(0x000000ff);
	map(0xfe4, 0xfe4).lr8([this](offs_t offset){ return periph_id[1]; }, "periph_id1").umask32(0x000000ff);
	map(0xfe8, 0xfe8).lr8([this](offs_t offset){ return periph_id[2]; }, "periph_id2").umask32(0x000000ff);
	map(0xfec, 0xfec).lr8([this](offs_t offset){ return periph_id[3]; }, "periph_id3").umask32(0x000000ff);
	map(0xff0, 0xff0).lr8([this](offs_t offset){ return pcell_id[0]; }, "pcell_id0").umask32(0x000000ff);
	map(0xff4, 0xff4).lr8([this](offs_t offset){ return pcell_id[1]; }, "pcell_id1").umask32(0x000000ff);
	map(0xff8, 0xff8).lr8([this](offs_t offset){ return pcell_id[2]; }, "pcell_id2").umask32(0x000000ff);
	map(0xffc, 0xffc).lr8([this](offs_t offset){ return pcell_id[3]; }, "pcell_id3").umask32(0x000000ff);
}

device_memory_interface::space_config_vector vic_pl190_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_mmio_config)
	};
}

void vic_pl190_device::device_resolve_objects()
{
	// resolve callbacks
	m_out_irq_func.resolve_safe();
	m_out_fiq_func.resolve_safe();
}

void vic_pl190_device::device_start()
{
	save_item(NAME(raw_intr));
	save_item(NAME(intr_select));
	save_item(NAME(intr_en));
	save_item(NAME(soft_intr));
	save_item(NAME(vectaddr));
	save_item(NAME(defaddress));
	save_item(NAME(vicaddress));
	save_item(NAME(priority_mask));
	save_item(NAME(priority));
	save_item(NAME(protection));
}

void vic_pl190_device::device_reset()
{
	raw_intr = intr_select = intr_en = soft_intr = 0;
	defaddress = vicaddress = protection = 0;

	priority_mask = ~0;
	priority = ~0;

	for(int i = 0; i < 32; i++)
	{
		vectaddr[i] = 0;
		vectctl[i] = 0;
	}
}

vic_pl190_device::vic_pl190_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, num_vectors(16)
	, periph_id{0x92, 0x11, 0x04, 0x00}
	, pcell_id{0x0d, 0xf0, 0x05, 0xb1}
	, m_mmio_config("mmio", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_out_irq_func(*this)
	, m_out_fiq_func(*this)
{
}

vic_pl190_device::vic_pl190_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vic_pl190_device(mconfig, PL190_VIC, tag, owner, clock)
{
}




void vic_upd800468_device::int_clear_w(u32 data)
{
	raw_intr &= ~data;
	timer_set(attotime::zero, TIMER_CHECK_IRQ);
}

void vic_upd800468_device::map(address_map &map)
{
	vic_pl190_device::map(map);

	map(0x100, 0x17f).rw(FUNC(vic_pl190_device::vect_addr_r), FUNC(vic_pl190_device::vect_addr_w));
	map(0x200, 0x27f).rw(FUNC(vic_pl190_device::vect_ctl_r), FUNC(vic_pl190_device::vect_ctl_w));
	map(0x2c8, 0x2cb).w(FUNC(vic_upd800468_device::int_clear_w));
}

vic_upd800468_device::vic_upd800468_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vic_pl190_device(mconfig, UPD800468_VIC, tag, owner, clock)
{
	num_vectors = 32;
}




u16 vic_pl192_device::sw_priority_r()
{
	return sw_priority_mask;
}

void vic_pl192_device::sw_priority_w(u16 data)
{
	sw_priority_mask = data;
}

u8 vic_pl192_device::daisy_priority_r()
{
	return daisy_priority;
}

void vic_pl192_device::daisy_priority_w(u8 data)
{
	daisy_priority = data & 0xf;
}

void vic_pl192_device::update_vector()
{
	u32 intrs = (raw_intr | soft_intr) & intr_en & ~intr_select;
	u32 newmask = 0;

	// see if there's a higher priority active interrupt to take
	for (u8 i = 0; i < num_vectors; i++)
	{
		u8 new_prio = vectctl[i] & 0xf;
		if (BIT(sw_priority_mask, new_prio) && BIT(intrs, i) && new_prio < priority)
		{
			// this interrupt is enabled and pending, take it
			priority = new_prio;
			vicaddress = vectaddr[i];
		}
	}

	// update priority_mask to only allow higher priority interrupts
	for (u8 i = 0; i < num_vectors; i++)
	{
		u8 new_prio = vectctl[i] & 0xf;
		if (BIT(sw_priority_mask, new_prio) && new_prio < priority)
		{
			newmask |= (1 << i);
		}
	}
	priority_mask = newmask;
}

void vic_pl192_device::map(address_map &map)
{
	map(0x000, 0x003).r(FUNC(vic_pl190_device::irq_status_r));
	map(0x004, 0x007).r(FUNC(vic_pl190_device::fiq_status_r));
	map(0x008, 0x00b).r(FUNC(vic_pl190_device::raw_intr_r));
	map(0x00c, 0x00f).rw(FUNC(vic_pl190_device::int_select_r), FUNC(vic_pl190_device::int_select_w));
	map(0x010, 0x013).rw(FUNC(vic_pl190_device::int_enable_r), FUNC(vic_pl190_device::int_enable_w));
	map(0x014, 0x017).w(FUNC(vic_pl190_device::int_en_clear_w));
	map(0x018, 0x01b).rw(FUNC(vic_pl190_device::soft_int_r), FUNC(vic_pl190_device::soft_int_w));
	map(0x01c, 0x01f).w(FUNC(vic_pl190_device::soft_int_clear_w));
	map(0x020, 0x020).rw(FUNC(vic_pl190_device::protection_r), FUNC(vic_pl190_device::protection_w)).umask32(0x000000ff);
	map(0x024, 0x025).rw(FUNC(vic_pl192_device::sw_priority_r), FUNC(vic_pl192_device::sw_priority_w)).umask32(0x0000ffff);
	map(0x028, 0x028).rw(FUNC(vic_pl192_device::daisy_priority_r), FUNC(vic_pl192_device::daisy_priority_w)).umask32(0x000000ff);
	map(0x100, 0x17f).rw(FUNC(vic_pl190_device::vect_addr_r), FUNC(vic_pl190_device::vect_addr_w));
	map(0x200, 0x27f).rw(FUNC(vic_pl190_device::vect_ctl_r), FUNC(vic_pl190_device::vect_ctl_w));
	map(0xf00, 0xf03).rw(FUNC(vic_pl190_device::cur_vect_addr_r), FUNC(vic_pl190_device::cur_vect_addr_w));
	map(0xfe0, 0xfe0).lr8([this](offs_t offset) { return periph_id[0]; }, "periph_id0").umask32(0x000000ff);
	map(0xfe4, 0xfe4).lr8([this](offs_t offset) { return periph_id[1]; }, "periph_id1").umask32(0x000000ff);
	map(0xfe8, 0xfe8).lr8([this](offs_t offset) { return periph_id[2]; }, "periph_id2").umask32(0x000000ff);
	map(0xfec, 0xfec).lr8([this](offs_t offset) { return periph_id[3]; }, "periph_id3").umask32(0x000000ff);
	map(0xff0, 0xff0).lr8([this](offs_t offset) { return pcell_id[0]; }, "pcell_id0").umask32(0x000000ff);
	map(0xff4, 0xff4).lr8([this](offs_t offset) { return pcell_id[1]; }, "pcell_id1").umask32(0x000000ff);
	map(0xff8, 0xff8).lr8([this](offs_t offset) { return pcell_id[2]; }, "pcell_id2").umask32(0x000000ff);
	map(0xffc, 0xffc).lr8([this](offs_t offset) { return pcell_id[3]; }, "pcell_id3").umask32(0x000000ff);
}

void vic_pl192_device::device_start()
{
	vic_pl190_device::device_start();

	save_item(NAME(sw_priority_mask));
	save_item(NAME(daisy_priority));
}

void vic_pl192_device::device_reset()
{
	vic_pl190_device::device_start();

	sw_priority_mask = 0xffff;
	daisy_priority = 0xf;

	for (int i = 0; i < 32; i++)
	{
		vectctl[i] = 0xf;
	}
}

vic_pl192_device::vic_pl192_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vic_pl190_device(mconfig, PL192_VIC, tag, owner, clock)
{
	num_vectors = 32;
}
