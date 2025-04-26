// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit WH-8-64 32k/64k Dynamic RAM board

****************************************************************************/

#include "emu.h"

#include "wh_8_64.h"

#include "machine/ram.h"

namespace {

class wh_8_64_device : public device_t, public device_h8bus_card_interface
{
public:

	wh_8_64_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:

	wh_8_64_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	bool m_installed;

	memory_share_array_creator<u8, 8> m_ram;
};

class wh_8_64_48k_device : public wh_8_64_device
{
public:

	wh_8_64_48k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:

	virtual void device_reset() override ATTR_COLD;
};

class wh_8_64_32k_device : public wh_8_64_device
{
public:

	wh_8_64_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:

	virtual void device_reset() override ATTR_COLD;
};

wh_8_64_device::wh_8_64_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	wh_8_64_device(mconfig, H8BUS_WH_8_64, tag, owner, 0)
{
}

wh_8_64_device::wh_8_64_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, type, tag, owner, 0),
	device_h8bus_card_interface(mconfig, *this),
	m_ram(*this, "rambank%u", 0U, 0x2000U, ENDIANNESS_LITTLE)
{
}

void wh_8_64_device::device_start()
{
	m_installed = false;

	save_item(NAME(m_installed));
}

void wh_8_64_device::device_reset()
{
	if (!m_installed)
	{
		h8bus().space(AS_PROGRAM).install_ram(0x2000, 0x3fff, m_ram[0]);

		h8bus().space(AS_PROGRAM).install_ram(0x4000, 0x5fff, m_ram[1]);

		h8bus().space(AS_PROGRAM).install_ram(0x6000, 0x7fff, m_ram[2]);

		h8bus().space(AS_PROGRAM).install_ram(0x8000, 0x9fff, m_ram[3]);

		h8bus().space(AS_PROGRAM).install_ram(0xa000, 0xbfff, m_ram[4]);

		h8bus().space(AS_PROGRAM).install_ram(0xc000, 0xdfff, m_ram[5]);

		h8bus().space(AS_PROGRAM).install_ram(0xe000, 0xffff, m_ram[6]);

// TODO: Properly map the last 8k, needs an HA-8-6 Z80 CPU or HA-8-8 Extended
// Configuration board to handle the ROM/RAM swap
#if 0
		h8bus().install_mem_device(0x0000, 0x1fff,
			read8sm_delegate(m_ram[7], FUNC(ram_device::read)),
			write8sm_delegate(m_ram[7], FUNC(ram_device::write)));
#endif

		m_installed = true;
	}
}

// TODO - Add support for setting memory map based on switch settings
#if 0
static INPUT_PORTS_START( wh_8_64_jumpers )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x00, "Bank 3 - Address Block 0k - 8k")    PORT_DIPLOCATION("SW1:1")
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
	PORT_DIPNAME( 0x10, 0x10, "Bank 3 - Address Block 32k - 40k")  PORT_DIPLOCATION("SW1:5")
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
	PORT_DIPNAME( 0x01, 0x00, "Bank 2 - Address Block 0k - 8k")    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bank 2 - Address Block 8k - 16k")   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bank 2 - Address Block 16k - 24k")  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bank 2 - Address Block 24k - 32k")  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Bank 2 - Address Block 32k - 40k")  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Bank 2 - Address Block 40k - 48k")  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Bank 2 - Address Block 48k - 56k")  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Bank 2 - Address Block 56k - 64k")  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SW3")
	PORT_DIPNAME( 0x01, 0x00, "Bank 1 - Address Block 0k - 8k")    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bank 1 - Address Block 8k - 16k")   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bank 1 - Address Block 16k - 24k")  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Bank 1 - Address Block 24k - 32k")  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Bank 1 - Address Block 32k - 40k")  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Bank 1 - Address Block 40k - 48k")  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bank 1 - Address Block 48k - 56k")  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Bank 1 - Address Block 56k - 64k")  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SW4")
	PORT_DIPNAME( 0x01, 0x00, "Bank 0 - Address Block 0k - 8k")    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Bank 0 - Address Block 8k - 16k")   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Bank 0 - Address Block 16k - 24k")  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bank 0 - Address Block 24k - 32k")  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Bank 0 - Address Block 32k - 40k")  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Bank 0 - Address Block 40k - 48k")  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bank 0 - Address Block 48k - 56k")  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Bank 0 - Address Block 56k - 64k")  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

INPUT_PORTS_END
#endif

wh_8_64_48k_device::wh_8_64_48k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	wh_8_64_device(mconfig, H8BUS_WH_8_64_48K, tag, owner, 0)
{
}

void wh_8_64_48k_device::device_reset()
{
	if (!m_installed)
	{
		h8bus().space(AS_PROGRAM).install_ram(0x2000, 0x3fff, m_ram[0]);

		h8bus().space(AS_PROGRAM).install_ram(0x4000, 0x5fff, m_ram[1]);

		h8bus().space(AS_PROGRAM).install_ram(0x6000, 0x7fff, m_ram[2]);

		h8bus().space(AS_PROGRAM).install_ram(0x8000, 0x9fff, m_ram[3]);

		h8bus().space(AS_PROGRAM).install_ram(0xa000, 0xbfff, m_ram[4]);

		h8bus().space(AS_PROGRAM).install_ram(0xc000, 0xdfff, m_ram[5]);

		m_installed = true;
	}
}


wh_8_64_32k_device::wh_8_64_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	wh_8_64_device(mconfig, H8BUS_WH_8_64_32K, tag, owner, 0)
{
}

void wh_8_64_32k_device::device_reset()
{
	if (!m_installed)
	{
		h8bus().space(AS_PROGRAM).install_ram(0x2000, 0x3fff, m_ram[0]);

		h8bus().space(AS_PROGRAM).install_ram(0x4000, 0x5fff, m_ram[1]);

		h8bus().space(AS_PROGRAM).install_ram(0x6000, 0x7fff, m_ram[2]);

		h8bus().space(AS_PROGRAM).install_ram(0x8000, 0x9fff, m_ram[3]);
		m_installed = true;
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_WH_8_64,     device_h8bus_card_interface, wh_8_64_device,     "wh8_h_8_64",     "Heath WH-8-64 64k Dynamic RAM");
DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_WH_8_64_48K, device_h8bus_card_interface, wh_8_64_48k_device, "wh8_h_8_64_48k", "Heath WH-8-64 48k Dynamic RAM");
DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_WH_8_64_32K, device_h8bus_card_interface, wh_8_64_32k_device, "wh8_h_8_64_32k", "Heath WH-8-64 32k Dynamic RAM");
