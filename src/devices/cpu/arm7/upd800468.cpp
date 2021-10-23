// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

 NEC uPD800468
 ARM7TDMI core with internal peripherals and external ROM/flash

***************************************************************************/

#include "emu.h"
#include "upd800468.h"

DEFINE_DEVICE_TYPE(UPD800468, upd800468_device, "upd800468", "NEC uPD800468")

void upd800468_device::upd800468_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).view(m_ram_view);
	m_ram_view[0](0x00000000, 0x0000ffff).ram();

	// RAM used for storing user samples (TODO: is this actually supposed to be part of the main 64kb RAM?)
	map(0x1ffe8400, 0x1ffedfff).ram();

	map(0x1fff00a0, 0x1fff00a1).r(m_kbd, FUNC(gt913_kbd_hle_device::read)).umask32(0x0000ffff);
	map(0x1fff00a2, 0x1fff00a3).r(m_kbd, FUNC(gt913_kbd_hle_device::status_r)).umask32(0xffff0000);
	map(0x1fff00a4, 0x1fff00a5).w(m_kbd, FUNC(gt913_kbd_hle_device::status_w)).umask32(0x0000ffff);

	// TODO: 8-channel ADC at 0x1fff00c0

	// TODO: USB controller at 0x50000400

	map(0x2a003e00, 0x2a003e03).rw(FUNC(upd800468_device::ram_enable_r), FUNC(upd800468_device::ram_enable_w));

	map(0xfffff000, 0xffffffff).m(m_vic, FUNC(vic_upd800468_device::map));
}

upd800468_device::upd800468_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm7_cpu_device(mconfig, UPD800468, tag, owner, clock, 4, ARCHFLAG_T, ENDIANNESS_LITTLE)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0, address_map_constructor(FUNC(upd800468_device::upd800468_map), this))
	, m_kbd(*this, "kbd")
	, m_vic(*this, "vic")
	, m_ram_view(*this, "ramview")
{
}

device_memory_interface::space_config_vector upd800468_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void upd800468_device::device_add_mconfig(machine_config &config)
{
	UPD800468_VIC(config, m_vic, 0);
	m_vic->out_irq_cb().set_inputline(DEVICE_SELF, ARM7_IRQ_LINE);
	m_vic->out_fiq_cb().set_inputline(DEVICE_SELF, ARM7_FIRQ_LINE);

	GT913_KBD_HLE(config, m_kbd, 0);
	m_kbd->irq_cb().set(m_vic, FUNC(vic_upd800468_device::irq_w<31>));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd800468_device::device_start()
{
	arm7_cpu_device::device_start();

	save_item(NAME(m_ram_enable));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd800468_device::device_reset()
{
	arm7_cpu_device::device_reset();

	m_ram_enable = 0;
	m_ram_view.disable();
}

u32 upd800468_device::ram_enable_r()
{
	return m_ram_enable;
}

void upd800468_device::ram_enable_w(u32 data)
{
	m_ram_enable = data;

	if (BIT(m_ram_enable, 0))
		m_ram_view.select(0);
	else
		m_ram_view.disable();
}
