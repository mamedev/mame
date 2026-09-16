// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit WH-8-16 16k Static RAM board

****************************************************************************/

#include "emu.h"

#include "wh_8_16.h"

#include "machine/ram.h"

#define LOG_INIT (1U << 1)

#define VERBOSE (LOG_INIT)

#include "logmacro.h"

#define LOGINIT(...)        LOGMASKED(LOG_INIT, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


namespace {

class wh_8_16_device : public device_t
					 , public device_h8bus_card_interface
{
public:

	wh_8_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:

	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void map_mem(address_space_installer & space) override ATTR_COLD;

	memory_share_creator<u8> m_ram;
	required_ioport          m_sw1;
};


wh_8_16_device::wh_8_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H8BUS_WH_8_16, tag, owner, 0)
	, device_h8bus_card_interface(mconfig, *this)
	, m_ram(*this, "ram", 0x4000U, ENDIANNESS_LITTLE)
	, m_sw1(*this, "SW1")
{
}

void wh_8_16_device::device_start()
{
}

void wh_8_16_device::map_mem(address_space_installer & space)
{
	ioport_value const sw1(m_sw1->read());

	if (BIT(sw1,0) == 1)
	{
		u16 base_addr = (sw1 & 0x1e) << 11;
		u16 top_addr = base_addr + 0x3fff;

		LOGINIT("%s: base_addr = 0x%04x, top_addr = 0x%04x\n", FUNCNAME, base_addr, top_addr);

		// check for wrap-around
		if (top_addr < base_addr)
		{
			// install wrap around memory
			u8 mem_offset = 0x3fff - top_addr;

			LOGINIT("%s: wrap around mem_offset = 0x%04x\n", FUNCNAME, mem_offset);

			space.install_ram(0x0000, top_addr, m_ram + mem_offset);

			top_addr = 0xffff;
		}

		space.install_ram(base_addr, top_addr, m_ram);
	}
}


static INPUT_PORTS_START( wh_8_16 )
	PORT_START("SW1")
	PORT_CONFNAME(0x01, 0x01, "Board Enabled")
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x01, DEF_STR( Yes ))
	PORT_CONFNAME(0x02, 0x00, "ORG Starting address A12" )
	PORT_CONFSETTING(   0x00, "0")
	PORT_CONFSETTING(   0x02, "1")
	PORT_CONFNAME(0x04, 0x04, "ORG Starting address A13" )
	PORT_CONFSETTING(   0x00, "0")
	PORT_CONFSETTING(   0x04, "1")
	PORT_CONFNAME(0x08, 0x00, "ORG Starting address A14" )
	PORT_CONFSETTING(   0x00, "0")
	PORT_CONFSETTING(   0x08, "1")
	PORT_CONFNAME(0x10, 0x00, "ORG Starting address A15" )
	PORT_CONFSETTING(   0x00, "0")
	PORT_CONFSETTING(   0x10, "1")
INPUT_PORTS_END

ioport_constructor wh_8_16_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(wh_8_16);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_WH_8_16, device_h8bus_card_interface, wh_8_16_device, "h8_wh_8_16", "Heath WH-8-16 16k Static RAM");
