//#define VERBOSE 1

#include "emu.h"
#include "system23_kbd.h"
#include "cpu/mcs48/mcs48.h"

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SYSTEM23_KEYBOARD, system23_kbd_device, "system23_kbd","IBM System/23 Keyboard")

ROM_START(system23_kbd)
ROM_REGION(0x400, "i8048", 0)
ROM_LOAD("kbd_3e88d3bf_8048.bin", 0x0000, 0x0400, CRC(3e88d3bf) SHA1(04884f5d43a940c76bc4d53d2dbd970b80f11fa6))
ROM_END

void system23_kbd_device::device_start()
{

}

void system23_kbd_device::device_reset()
{
	m_reset = ASSERT_LINE;
	m_t0 = CLEAR_LINE;
	m_t1 = CLEAR_LINE;
}

const tiny_rom_entry *system23_kbd_device::device_rom_region() const
{
	return ROM_NAME(system23_kbd);
}

system23_kbd_device::system23_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: device_t(mconfig, SYSTEM23_KEYBOARD, tag, owner, clock),
	m_mcu(*this, "i8048"),
	m_bus_write(*this)
{
}

void system23_kbd_device::device_add_mconfig(machine_config &config)
{
	I8048(config, m_mcu, 4'350'000);//Oscillation between 4.17 and 4.35 MHz
	m_mcu->bus_out_cb().set(FUNC(system23_kbd_device::bus_w));
	m_mcu->p2_out_cb().set(FUNC(system23_kbd_device::data_strobe));
	m_mcu->t0_in_cb().set(FUNC(system23_kbd_device::t0_r));
	m_mcu->t1_in_cb().set(FUNC(system23_kbd_device::t1_r));

}

void system23_kbd_device::reset_w(int state)
{
	if (m_reset ^ state)
	{
		LOG("Reset changed to %d\n", state);
		m_reset = state;
		if (state)
		{
			m_mcu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			m_mcu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		}
	}
}

void system23_kbd_device::bus_w(uint8_t data)
{
	m_bus = 0;
	if(BIT(data,0)) m_bus |= 0x40;
	if(BIT(data,1)) m_bus |= 0x20;
	if(BIT(data,2)) m_bus |= 0x10;
	if(BIT(data,3)) m_bus |= 0x08;
	if(BIT(data,4)) m_bus |= 0x04;
	if(BIT(data,5)) m_bus |= 0x02;
	if(BIT(data,6)) m_bus |= 0x01;
}

uint8_t system23_kbd_device::read_keyboard()
{
	LOG("Read Keyboard %02x\n", m_bus);
	return m_bus;
}

void system23_kbd_device::data_strobe(uint8_t data)
{
	m_bus_write(BIT(data,7));
	m_t1 = BIT(data, 3);
	LOG("T1: %d\n", m_t1);
}

void system23_kbd_device::t0_w(int state)
{
	if(m_t0 ^ state)
	{
		m_t0 = state;
		LOG("Delay strobe %d\n", state);
	}
}

int system23_kbd_device::t0_r()
{
	return m_t0;
}

int system23_kbd_device::t1_r()
{
	return m_t1;
}
