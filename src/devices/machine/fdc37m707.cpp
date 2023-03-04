// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/***************************************************************************

SMSC FDC37M70x Enhanced Super I/O

***************************************************************************/

#include "emu.h"
#include "bus/isa/isa.h"
//#include "machine/ds128x.h"
#include "machine/fdc37m707.h"

DEFINE_DEVICE_TYPE(FDC37M707, fdc37m707_device, "fdc37m707", "SMSC FDC37M707")

fdc37m707_device::fdc37m707_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FDC37M707, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("config_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(fdc37m707_device::config_map), this))
	, m_kbdc(*this, "pc_kbdc")
	, m_logical_view(*this, "logical_view")
{ }

void fdc37m707_device::device_start()
{
	set_isa_device();
	//m_isa->set_dma_channel(0, this, true);
	//m_isa->set_dma_channel(1, this, true);
	//m_isa->set_dma_channel(2, this, true);
	//m_isa->set_dma_channel(3, this, true);
	remap(AS_IO, 0, 0x400);
}

void fdc37m707_device::device_reset()
{
	m_index = 0;
}

device_memory_interface::space_config_vector fdc37m707_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void fdc37m707_device::device_add_mconfig(machine_config &config)
{
	KBDC8042(config, m_kbdc);
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_PS2);
	m_kbdc->set_interrupt_type(kbdc8042_device::KBDC8042_DOUBLE);
//	m_kbdc->input_buffer_full_callback().set(FUNC(fdc37c93x_device::irq_keyboard_w));
//	m_kbdc->input_buffer_full_mouse_callback().set(FUNC(fdc37c93x_device::irq_mouse_w));
//	m_kbdc->system_reset_callback().set(FUNC(fdc37c93x_device::kbdp20_gp20_reset_w));
//	m_kbdc->gate_a20_callback().set(FUNC(fdc37c93x_device::kbdp21_gp25_gatea20_w));
}


void fdc37m707_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		//if (sysopt_pin == 0)
			m_isa->install_device(0x03f0, 0x03f3, read8sm_delegate(*this, FUNC(fdc37m707_device::read)), write8sm_delegate(*this, FUNC(fdc37m707_device::write)));
		//else
		//	m_isa->install_device(0x0370, 0x0373, read8sm_delegate(*this, FUNC(fdc37c93x_device::read)), write8sm_delegate(*this, FUNC(fdc37c93x_device::write)));
		#if 0
		if (enabled_logical[LogicalDevice::FDC] == true)
			map_fdc_addresses();
		if (enabled_logical[LogicalDevice::Parallel] == true)
			map_lpt_addresses();
		if (enabled_logical[LogicalDevice::Serial1] == true)
			map_serial1_addresses();
		if (enabled_logical[LogicalDevice::Serial2] == true)
			map_serial2_addresses();
		if (enabled_logical[LogicalDevice::RTC] == true)
			map_rtc_addresses();
		if (enabled_logical[LogicalDevice::Keyboard] == true)
			map_keyboard_addresses();
		#endif
	}
}

// TODO: lock/unlock
uint8_t fdc37m707_device::read(offs_t offset)
{
	if (offset == 0)
		return m_index;

	return space().read_byte(m_index);
}

void fdc37m707_device::write(offs_t offset, u8 data)
{
	if (offset == 0)
		m_index = data;
	else
		space().write_byte(m_index, data);

	if (data != 0x55 && data != 0xaa)
		printf("%02x %02x\n", offset, data);
}

void fdc37m707_device::config_map(address_map &map)
{
//	map(0x02, 0x02) configuration control (bit 0 soft reset)
//	map(0x03, 0x03) index
	map(0x07, 0x07).lr8(NAME([this] () { return m_logical_index; })).w(FUNC(fdc37m707_device::logical_device_select_w));
	map(0x20, 0x20).lr8(NAME([] () { return 0x42; })); // device ID
//	map(0x21, 0x21) revision
//	map(0x22, 0x22) power control
//	map(0x23, 0x23) power management
//	map(0x24, 0x24) OSC
//	map(0x26, 0x27) configuration port reloc
//	map(0x2b, 0x2f) TEST regs
	map(0x30, 0xff).view(m_logical_view);
	// FDC
	m_logical_view[0](0x30, 0x30).rw(FUNC(fdc37m707_device::activate_r<0>), FUNC(fdc37m707_device::activate_w<0>));
	m_logical_view[0](0x31, 0xff).unmaprw();
	// <reserved>
	m_logical_view[1](0x30, 0xff).unmaprw();
	// <reserved>
	m_logical_view[2](0x30, 0xff).unmaprw();
	// Parallel Port
	m_logical_view[3](0x30, 0x30).rw(FUNC(fdc37m707_device::activate_r<3>), FUNC(fdc37m707_device::activate_w<3>));
	m_logical_view[3](0x31, 0xff).unmaprw();
	// Serial Port 1
	m_logical_view[4](0x30, 0x30).rw(FUNC(fdc37m707_device::activate_r<4>), FUNC(fdc37m707_device::activate_w<4>));
	m_logical_view[4](0x31, 0xff).unmaprw();
	// Serial Port 2
	m_logical_view[5](0x30, 0x30).rw(FUNC(fdc37m707_device::activate_r<5>), FUNC(fdc37m707_device::activate_w<5>));
	m_logical_view[5](0x31, 0xff).unmaprw();
	// <reserved>
	m_logical_view[6](0x30, 0xff).unmaprw();
	// keyboard
	m_logical_view[7](0x30, 0x30).rw(FUNC(fdc37m707_device::activate_r<7>), FUNC(fdc37m707_device::activate_w<7>));
	m_logical_view[7](0x31, 0xff).unmaprw();
	// Aux I/O
	m_logical_view[8](0x30, 0x30).rw(FUNC(fdc37m707_device::activate_r<8>), FUNC(fdc37m707_device::activate_w<8>));
	m_logical_view[8](0x31, 0xff).unmaprw();
}

void fdc37m707_device::logical_device_select_w(u8 data)
{
	m_logical_index = data;
	if (m_logical_index <= 8)
		m_logical_view.select(m_logical_index);
}

template <unsigned N> u8 fdc37m707_device::activate_r()
{
	return m_activate[N];
}

u8 fdc37m707_device::keybc_status_r()
{
	return (m_kbdc->data_r(4) & 0xfb) | 0x10; // bios needs bit 2 to be 0 as powerup and bit 4 to be 1
}

void fdc37m707_device::keybc_command_w(u8 data)
{
	m_kbdc->data_w(4, data);
}

template <unsigned N> void fdc37m707_device::activate_w(u8 data)
{
	m_activate[N] = data & 1;
	printf("%d Device %s\n", N, data & 1 ? "enabled" : "disabled");
	switch(N)
	{
		case 7:
			if (data & 1)
			{
				m_isa->install_device(0x60, 0x60, read8sm_delegate(*m_kbdc, FUNC(kbdc8042_device::data_r)), write8sm_delegate(*m_kbdc, FUNC(kbdc8042_device::data_w)));
				//m_isa->install_device(0x64, 0x64, read8sm_delegate(*this, FUNC(fdc37m707_device::keybc_status_r)), write8sm_delegate(*this, FUNC(fdc37m707_device::keybc_command_w)));
			}

			break;
	}
}
