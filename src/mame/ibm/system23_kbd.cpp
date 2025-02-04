#define VERBOSE 1

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
	m_cs = CLEAR_LINE;
	m_scan_r = false;
}

const tiny_rom_entry *system23_kbd_device::device_rom_region() const
{
	return ROM_NAME(system23_kbd);
}

system23_kbd_device::system23_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: device_t(mconfig, SYSTEM23_KEYBOARD, tag, owner, clock),
	m_mcu(*this, "i8048"),
	m_columns(*this, "CL.%u",0),
	m_bus_write(*this)
{
}

void system23_kbd_device::device_add_mconfig(machine_config &config)
{
	I8048(config, m_mcu, 4'350'000);//Oscillation between 4.17 and 4.35 MHz
	m_mcu->bus_out_cb().set(FUNC(system23_kbd_device::bus_w));
	m_mcu->p1_out_cb().set(FUNC(system23_kbd_device::p1_w));
	m_mcu->p2_out_cb().set(FUNC(system23_kbd_device::p2_w));
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
	m_cs = BIT(data, 7);
	LOG("m_cs: %d\n",m_cs);
}

uint8_t system23_kbd_device::read_keyboard()
{
	LOG("Read Keyboard %02x\n", m_bus);
	return m_bus;
}

void system23_kbd_device::p2_w(uint8_t data)
{
	m_bus_write(BIT(data,7));
	m_t1 = BIT(data, 3);
	if(!m_cs)
	{
		int data_to_write = ((data & 0x70) << 4);
		m_counter &= 0xff;
		m_counter |= data_to_write;
		m_select &= data & 0x07;
	}
	LOG("Port 2 Counter: %04x Select: %d\n", m_counter, m_select);
}

void system23_kbd_device::p1_w(uint8_t data)
{
	if(!m_cs)
	{
		m_counter &= 0x700;
		m_counter |= data;
	}
	LOG("Port 1 counter: %04x\n", m_counter);
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

//This routine has the responsibility to process the keyboard matrix and extract the sense line, then feed it to the microcontroller
int system23_kbd_device::t1_r()
{
	if(m_cs)
	{
		uint8_t kbd_data = m_columns[translate_columns()]->read();
		LOG("Counter: %d Select: %d Value: %02x\n", translate_columns(), m_select, kbd_data);
		m_counter = 0;
		m_select = 7;
		return BIT(kbd_data, m_select);
	}
	else
	{
		return m_t1;
	}
}


INPUT_PORTS_START( system23_kbd )
	PORT_START("CL.0")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_START("CL.1")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_START("CL.2")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_START("CL.3")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_START("CL.4")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_START("CL.5")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_START("CL.6")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_START("CL.7")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_START("CL.8")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_START("CL.9")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_START("CL.10")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
INPUT_PORTS_END

ioport_constructor system23_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( system23_kbd );
}

int system23_kbd_device::translate_columns()
{
	switch(m_counter)
	{
		case 0x001: return 0;
		case 0x002: return 1;
		case 0x004: return 2;
		case 0x008: return 3;
		case 0x010: return 4;
		case 0x020: return 5;
		case 0x040: return 6;
		case 0x080: return 7;
		case 0x100: return 8;
		case 0x200: return 9;
		case 0x400: return 10;
		default: return 0;
	}
}
