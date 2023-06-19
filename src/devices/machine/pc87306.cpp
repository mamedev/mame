// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/***************************************************************************

National Semiconductor PC87306 Super I/O

TODO:
- Barely enough to make it surpass POST test 0x05 in misc/odyssey.cpp

***************************************************************************/

#include "emu.h"
#include "bus/isa/isa.h"
//#include "machine/ds128x.h"
#include "machine/pc87306.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PC87306, pc87306_device, "pc87306", "National Semiconductor PC87306 Super I/O Enhanced Sidewinder Lite")

pc87306_device::pc87306_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC87306, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("superio_config_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(pc87306_device::config_map), this))
	, m_kbdc(*this, "pc_kbdc")
	, m_rtc(*this, "rtc")
	//, m_logical_view(*this, "logical_view")
	, m_gp20_reset_callback(*this)
	, m_gp25_gatea20_callback(*this)
	, m_irq1_callback(*this)
	, m_irq8_callback(*this)
	, m_irq9_callback(*this)
//	, m_txd1_callback(*this)
//	, m_ndtr1_callback(*this)
//	, m_nrts1_callback(*this)
//	, m_txd2_callback(*this)
//	, m_ndtr2_callback(*this)
//	, m_nrts2_callback(*this)
{ }


void pc87306_device::device_start()
{
	set_isa_device();
	//m_isa->set_dma_channel(0, this, true);
	//m_isa->set_dma_channel(1, this, true);
	//m_isa->set_dma_channel(2, this, true);
	//m_isa->set_dma_channel(3, this, true);
	remap(AS_IO, 0, 0x400);
}

void pc87306_device::device_reset()
{
	m_locked_state = 2;
}

device_memory_interface::space_config_vector pc87306_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void pc87306_device::device_add_mconfig(machine_config &config)
{
	// doc doesn't explicitly mention this being at 8, assume from intialization
	DS12885(config, m_rtc, 32.768_kHz_XTAL);
//	m_rtc->irq().set(FUNC(pc87306_device::irq_rtc_w));
	m_rtc->set_century_index(0x32);
	
	KBDC8042(config, m_kbdc);
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_PS2);
	m_kbdc->set_interrupt_type(kbdc8042_device::KBDC8042_DOUBLE);
//	m_kbdc->input_buffer_full_callback().set(FUNC(pc87306_device::irq_keyboard_w));
//	m_kbdc->input_buffer_full_mouse_callback().set(FUNC(pc87306_device::irq_mouse_w));
//	m_kbdc->system_reset_callback().set(FUNC(pc87306_device::kbdp20_gp20_reset_w));
//	m_kbdc->gate_a20_callback().set(FUNC(pc87306_device::kbdp21_gp25_gatea20_w));
}

void pc87306_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		// TODO: BADDR1/0 config pin controlled
		u16 superio_base = 0x0398;
		m_isa->install_device(superio_base, superio_base + 1, read8sm_delegate(*this, FUNC(pc87306_device::read)), write8sm_delegate(*this, FUNC(pc87306_device::write)));


		m_isa->install_device(0x60, 0x60, read8sm_delegate(*m_kbdc, FUNC(kbdc8042_device::data_r)), write8sm_delegate(*m_kbdc, FUNC(kbdc8042_device::data_w)));
		m_isa->install_device(0x64, 0x64, read8sm_delegate(*this, FUNC(pc87306_device::keybc_status_r)), write8sm_delegate(*this, FUNC(pc87306_device::keybc_command_w)));

		m_isa->install_device(0x70, 0x71, read8sm_delegate(*m_rtc, FUNC(ds12885_device::read)), write8sm_delegate(*m_rtc, FUNC(ds12885_device::write)));
	}
}

u8 pc87306_device::read(offs_t offset)
{
	if (m_locked_state)
	{
		if (!machine().side_effects_disabled())
			m_locked_state --;
		return (m_locked_state) ? 0x88 : 0x00;
	}

	if (offset == 0)
		return m_index;

	return space().read_byte(m_index);
}

void pc87306_device::write(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		m_index = data;
	}
	else
	{
		//remap(AS_IO, 0, 0x400);
		space().write_byte(m_index, data);
	}
}

void pc87306_device::config_map(address_map &map)
{
	map(0x00, 0xff).unmaprw();
}


u8 pc87306_device::keybc_status_r(offs_t offset)
{
	return (m_kbdc->data_r(4) & 0xfb) | 0x10; // bios needs bit 2 to be 0 as powerup and bit 4 to be 1
}

void pc87306_device::keybc_command_w(offs_t offset, u8 data)
{
	m_kbdc->data_w(4, data);
}
