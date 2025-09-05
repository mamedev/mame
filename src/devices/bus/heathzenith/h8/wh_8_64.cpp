// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit WH-8-64 32k/64k Dynamic RAM board

****************************************************************************/

#include "emu.h"

#include "wh_8_64.h"

#include <vector>
#include <algorithm>

#define LOG_SW (1U << 1)    // Shows register setup

//#define VERBOSE (LOG_SW)

#include "logmacro.h"

#define LOGSW(...)        LOGMASKED(LOG_SW, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

namespace {

class wh_8_64_device : public device_t
					 , public device_h8bus_card_interface
{
public:

	wh_8_64_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void map_mem(address_space_installer & space) override ATTR_COLD;
	std::vector<int> get_addr(u8 sw) ATTR_COLD;
	void install_mem_bank(address_space_installer &space,u8 bank) ATTR_COLD;

	memory_share_array_creator<u8, 8> m_ram;
	required_ioport_array<4>          m_sw;
	required_ioport                   m_config;
};


wh_8_64_device::wh_8_64_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H8BUS_WH_8_64, tag, owner, 0)
	, device_h8bus_card_interface(mconfig, *this)
	, m_ram(*this, "rambank%u", 0U, 0x2000U, ENDIANNESS_LITTLE)
	, m_sw(*this, "SW%u", 1U)
	, m_config(*this, "CONFIG")
{
}

void wh_8_64_device::device_start()
{
}

std::vector<int> wh_8_64_device::get_addr(u8 sw)
{
	std::vector<int> result;

	u8 sw_value = m_sw[sw]->read();

	for (int i = 0; i < 8; i++)
	{
		if (BIT(sw_value, i))
		{
			result.push_back(i * 0x2000);
		}
	}

	return result;
}

void wh_8_64_device::install_mem_bank(address_space_installer &space, u8 bank)
{
	std::vector<int> addresses = get_addr(3 - bank);

	// each switch defines where to start each 8k block of the
	// 16k bank. Per manual only 2 should be set. Ensure setting
	// a max of 2 blocks are defined even if it is just selecting
	// the first 2.
	int num_addresses = std::min((int) addresses.size(), 2);
	int block = bank << 1;

	for (int i = 0; i < num_addresses; i++)
	{
		int addr = addresses[i];

		LOGSW("Installing block %d at 0x%04x\n", block, addr);

		space.install_ram(addr, addr + 0x1fff, m_ram[block++]);
	}
}

void wh_8_64_device::map_mem(address_space_installer & space)
{
	ioport_value const config(m_config->read());

	for (int bank = 0; bank < 4; bank++)
	{
		if (BIT(config, bank))
		{
			install_mem_bank(space, bank);
		}
	}
}


static INPUT_PORTS_START( wh_8_64 )
	PORT_START("SW1")
	// Needs an HA-8-6 Z80 CPU or HA-8-8 Extended Configuration board to handle the ROM/RAM
	// swap.
	PORT_DIPNAME( 0x01, 0x01, "Bank 3 - Address Block 0k - 8k")    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bank 3 - Address Block 8k - 16k")   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bank 3 - Address Block 16k - 24k")  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bank 3 - Address Block 24k - 32k")  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Bank 3 - Address Block 32k - 40k")  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Bank 3 - Address Block 40k - 48k")  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bank 3 - Address Block 48k - 56k")  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Bank 3 - Address Block 56k - 64k")  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x00, "Bank 2 - Address Block 0k - 8k")    PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bank 2 - Address Block 8k - 16k")   PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bank 2 - Address Block 16k - 24k")  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bank 2 - Address Block 24k - 32k")  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Bank 2 - Address Block 32k - 40k")  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Bank 2 - Address Block 40k - 48k")  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Bank 2 - Address Block 48k - 56k")  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Bank 2 - Address Block 56k - 64k")  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SW3")
	PORT_DIPNAME( 0x01, 0x00, "Bank 1 - Address Block 0k - 8k")    PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bank 1 - Address Block 8k - 16k")   PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bank 1 - Address Block 16k - 24k")  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Bank 1 - Address Block 24k - 32k")  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Bank 1 - Address Block 32k - 40k")  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Bank 1 - Address Block 40k - 48k")  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bank 1 - Address Block 48k - 56k")  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Bank 1 - Address Block 56k - 64k")  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SW4")
	PORT_DIPNAME( 0x01, 0x00, "Bank 0 - Address Block 0k - 8k")    PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Bank 0 - Address Block 8k - 16k")   PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Bank 0 - Address Block 16k - 24k")  PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bank 0 - Address Block 24k - 32k")  PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Bank 0 - Address Block 32k - 40k")  PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Bank 0 - Address Block 40k - 48k")  PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bank 0 - Address Block 48k - 56k")  PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Bank 0 - Address Block 56k - 64k")  PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x01, "Memory Bank 0 Present")
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x01, DEF_STR( Yes ))
	PORT_CONFNAME(0x02, 0x02, "Memory Bank 1 Present")
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x02, DEF_STR( Yes ))
	PORT_CONFNAME(0x04, 0x04, "Memory Bank 2 Present")
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x04, DEF_STR( Yes ))
	PORT_CONFNAME(0x08, 0x08, "Memory Bank 3 Present")
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x08, DEF_STR( Yes ))

INPUT_PORTS_END

ioport_constructor wh_8_64_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(wh_8_64);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_WH_8_64, device_h8bus_card_interface, wh_8_64_device, "wh8_h_8_64", "Heath WH-8-64 64k Dynamic RAM");
