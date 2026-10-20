// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 2 MEME186 emulation

*********************************************************************/

#include "emu.h"
#include "meme186.h"

DEFINE_DEVICE_TYPE(NOKIA_MEME186, meme186_device, "nokia_meme186", "Nokia MikroMikko 2 MEME186")

static INPUT_PORTS_START(meme186)
	PORT_START("P1")
	PORT_CONFNAME(0x03, 0x01, "Ready Delay (P1)")
	PORT_CONFSETTING(   0x00, "Advanced Acknowledge (1 Wait State, 150 ns DRAMs)")
	PORT_CONFSETTING(   0x01, "Column Strobe Acknowledge (2 Wait States, 200 ns DRAMs)")
	PORT_CONFSETTING(   0x02, "Executed Acknowledge (3 Wait States)")

	PORT_START("P3")
	PORT_CONFNAME(0x03, 0x02, "Memory Address (P3)")
	PORT_CONFSETTING(   0x00, "256K, 40000H-7FFFFH")
	PORT_CONFSETTING(   0x01, "256K, 80000H-BFFFFH")
	PORT_CONFSETTING(   0x02, "512K, 40000H-BFFFFH")
INPUT_PORTS_END

ioport_constructor meme186_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(meme186);
}

meme186_device::meme186_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NOKIA_MEME186, tag, owner, clock),
	device_mikromikko2_expansion_bus_card_interface(mconfig, *this),
	m_ram(*this, "ram", 0x80000, ENDIANNESS_LITTLE),
	m_p3(*this, "P3")
{
}

void meme186_device::device_start()
{
}

void meme186_device::device_reset()
{
	offs_t start, end;

	switch (m_p3->read())
	{
	case 0:
		start = 0x40000; end = 0x7ffff;
		break;
	case 1:
		start = 0x80000; end = 0xbffff;
		break;
	default:
		start = 0x40000; end = 0xbffff;
		break;
	}

	m_bus->memspace().install_ram(start, end, m_ram);
}
