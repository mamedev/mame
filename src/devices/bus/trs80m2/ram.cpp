// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II 32K/64K memory board emulation

    4116 DRAM. Each board latches port FF itself: pages 1-15 appear
    at 8000-FFFF as selected by FF bits 0-3, and page 0 is fixed at
    0000-7FFF. With FF bit 7 set the board does not respond at
    F800-FFFF (video RAM). Bank jumpers J19-J30 pick the page pair.

**********************************************************************/

#include "emu.h"
#include "ram.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(TRS80M2_RAM32K, trs80m2_ram32k_device, "trs80m2_ram32k", "Tandy Model II 32K memory board")
DEFINE_DEVICE_TYPE(TRS80M2_RAM64K, trs80m2_ram64k_device, "trs80m2_ram64k", "Tandy Model II 64K memory board")


static INPUT_PORTS_START( trs80m2_ram32k )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x0f, 0x00, "Page" )
	PORT_CONFSETTING( 0x00, "0 (0000-7FFF)" )
	PORT_CONFSETTING( 0x01, "1" )
	PORT_CONFSETTING( 0x02, "2" )
	PORT_CONFSETTING( 0x03, "3" )
	PORT_CONFSETTING( 0x04, "4" )
	PORT_CONFSETTING( 0x05, "5" )
	PORT_CONFSETTING( 0x06, "6" )
	PORT_CONFSETTING( 0x07, "7" )
	PORT_CONFSETTING( 0x08, "8" )
	PORT_CONFSETTING( 0x09, "9" )
	PORT_CONFSETTING( 0x0a, "10" )
	PORT_CONFSETTING( 0x0b, "11" )
	PORT_CONFSETTING( 0x0c, "12" )
	PORT_CONFSETTING( 0x0d, "13" )
	PORT_CONFSETTING( 0x0e, "14" )
	PORT_CONFSETTING( 0x0f, "15" )
INPUT_PORTS_END

static INPUT_PORTS_START( trs80m2_ram64k )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x07, 0x00, "Bank" )
	PORT_CONFSETTING( 0x00, "0 (pages 0/1)" )
	PORT_CONFSETTING( 0x01, "1 (pages 2/3)" )
	PORT_CONFSETTING( 0x02, "2 (pages 4/5)" )
	PORT_CONFSETTING( 0x03, "3 (pages 6/7)" )
	PORT_CONFSETTING( 0x04, "4 (pages 8/9)" )
	PORT_CONFSETTING( 0x05, "5 (pages 10/11)" )
	PORT_CONFSETTING( 0x06, "6 (pages 12/13)" )
	PORT_CONFSETTING( 0x07, "7 (pages 14/15)" )
INPUT_PORTS_END

ioport_constructor trs80m2_ram32k_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( trs80m2_ram32k );
}

ioport_constructor trs80m2_ram64k_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( trs80m2_ram64k );
}


trs80m2_ram_device_base::trs80m2_ram_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int pages) :
	device_t(mconfig, type, tag, owner, clock),
	device_trs80m2_card_interface(mconfig, *this),
	m_config(*this, "CONFIG"),
	m_pages(pages),
	m_ram(*this, "ram", pages * 0x8000, ENDIANNESS_LITTLE),
	m_installed(false)
{
}

trs80m2_ram32k_device::trs80m2_ram32k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	trs80m2_ram_device_base(mconfig, TRS80M2_RAM32K, tag, owner, clock, 1)
{
}

trs80m2_ram64k_device::trs80m2_ram64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	trs80m2_ram_device_base(mconfig, TRS80M2_RAM64K, tag, owner, clock, 2)
{
}

int trs80m2_ram32k_device::first_page()
{
	return m_config->read() & 0x0f;
}

int trs80m2_ram64k_device::first_page()
{
	return (m_config->read() & 0x07) << 1;
}

void trs80m2_ram_device_base::device_start()
{
	std::fill_n(&m_ram[0], m_pages * 0x8000, 0xff);
}

void trs80m2_ram_device_base::device_reset()
{
	if (m_installed)
		return;

	m_installed = true;

	trs80m2_bus_device &bus = m_slot->bus();

	for (int i = 0; i < m_pages; i++)
	{
		int const page = first_page() + i;
		uint8_t *const ram = &m_ram[i * 0x8000];

		if (page)
			bus.install_page_ram(page, 0xffff, ram);
		else
			bus.program().install_ram(0x0000, 0x7fff, ram);
	}
}
