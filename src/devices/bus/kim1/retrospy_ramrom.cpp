// license:BSD-3-Clause
// copyright-holders:Peter Clark
/*********************************************************************

    retrospy_ramrom.cpp

    RetroSpy SYM-1/AIM-65 RAM/ROM expansion board

    The board can provide RAM, ROM, or no response independently in
    each 4K block of the 6502 address space.  A16/A17/A18 select one
    of eight 64K pages in the on-board 39SF040 flash ROM.

    Manual:
    https://retro-spy.com/wp-content/uploads/2024/06/SYM-1-RAM-Manual.pdf

    TODO:
    - Model the FF80 switch used with Fxxx mappings.  The manual
      states that it is normally off for AIM-65 and on for SYM-1;
      the exact decode behaviour is not currently emulated.

*********************************************************************/

#include "emu.h"
#include "retrospy_ramrom.h"

namespace {

class kim1bus_retrospy_ramrom_device :
		public device_t,
		public device_kim1bus_card_interface
{
public:
	kim1bus_retrospy_ramrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_region_ptr<u8> m_rom;
	required_ioport m_pages;
	required_ioport m_rom_select;
	memory_bank_array_creator<16> m_rom_bank;
	memory_view m_view[16];

	std::unique_ptr<u8[]> m_ram;
};


ROM_START(retrospy_ramrom)
	ROM_REGION(0x80000, "flash", 0)
	ROM_LOAD("sym_aim_rom.bin", 0x00000, 0x80000, CRC(68be2a6f) SHA1(97a584e7ee3c0f8b395b4e80836f032000c26a1b))
ROM_END


static INPUT_PORTS_START(retrospy_ramrom)
	// Default jumper layout is the manual's AIM-65 BASIC + Assembler setup:
	// NAAAAAAA AANOOONN (N=None, A=RAM, O=ROM), for pages 0 through F.
	PORT_START("PAGES")
	PORT_CONFNAME(0x00000003, 0x00000000, "$0000-$0FFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x00000001, "RAM")
	PORT_CONFSETTING(0x00000002, "ROM")
	PORT_CONFNAME(0x0000000c, 0x00000004, "$1000-$1FFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x00000004, "RAM")
	PORT_CONFSETTING(0x00000008, "ROM")
	PORT_CONFNAME(0x00000030, 0x00000010, "$2000-$2FFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x00000010, "RAM")
	PORT_CONFSETTING(0x00000020, "ROM")
	PORT_CONFNAME(0x000000c0, 0x00000040, "$3000-$3FFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x00000040, "RAM")
	PORT_CONFSETTING(0x00000080, "ROM")
	PORT_CONFNAME(0x00000300, 0x00000100, "$4000-$4FFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x00000100, "RAM")
	PORT_CONFSETTING(0x00000200, "ROM")
	PORT_CONFNAME(0x00000c00, 0x00000400, "$5000-$5FFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x00000400, "RAM")
	PORT_CONFSETTING(0x00000800, "ROM")
	PORT_CONFNAME(0x00003000, 0x00001000, "$6000-$6FFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x00001000, "RAM")
	PORT_CONFSETTING(0x00002000, "ROM")
	PORT_CONFNAME(0x0000c000, 0x00004000, "$7000-$7FFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x00004000, "RAM")
	PORT_CONFSETTING(0x00008000, "ROM")
	PORT_CONFNAME(0x00030000, 0x00010000, "$8000-$8FFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x00010000, "RAM")
	PORT_CONFSETTING(0x00020000, "ROM")
	PORT_CONFNAME(0x000c0000, 0x00040000, "$9000-$9FFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x00040000, "RAM")
	PORT_CONFSETTING(0x00080000, "ROM")
	PORT_CONFNAME(0x00300000, 0x00000000, "$A000-$AFFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x00100000, "RAM")
	PORT_CONFSETTING(0x00200000, "ROM")
	PORT_CONFNAME(0x00c00000, 0x00800000, "$B000-$BFFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x00400000, "RAM")
	PORT_CONFSETTING(0x00800000, "ROM")
	PORT_CONFNAME(0x03000000, 0x02000000, "$C000-$CFFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x01000000, "RAM")
	PORT_CONFSETTING(0x02000000, "ROM")
	PORT_CONFNAME(0x0c000000, 0x08000000, "$D000-$DFFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x04000000, "RAM")
	PORT_CONFSETTING(0x08000000, "ROM")
	PORT_CONFNAME(0x30000000, 0x00000000, "$E000-$EFFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x10000000, "RAM")
	PORT_CONFSETTING(0x20000000, "ROM")
	PORT_CONFNAME(0xc0000000, 0x00000000, "$F000-$FFFF")
	PORT_CONFSETTING(0x00000000, "None")
	PORT_CONFSETTING(0x40000000, "RAM")
	PORT_CONFSETTING(0x80000000, "ROM")

	PORT_START("ROMBANK")
	PORT_DIPNAME(0x01, 0x00, "ROM A16")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x01, "1")
	PORT_DIPNAME(0x02, 0x02, "ROM A17")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x02, "1")
	PORT_DIPNAME(0x04, 0x00, "ROM A18")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x04, "1")
INPUT_PORTS_END


kim1bus_retrospy_ramrom_device::kim1bus_retrospy_ramrom_device(
		const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KIM1BUS_RETROSPY_RAMROM, tag, owner, clock)
	, device_kim1bus_card_interface(mconfig, *this)
	, m_rom(*this, "flash")
	, m_pages(*this, "PAGES")
	, m_rom_select(*this, "ROMBANK")
	, m_rom_bank(*this, "rombank%u", 0U)
	, m_view{
		{*this, "view0"}, {*this, "view1"}, {*this, "view2"}, {*this, "view3"},
		{*this, "view4"}, {*this, "view5"}, {*this, "view6"}, {*this, "view7"},
		{*this, "view8"}, {*this, "view9"}, {*this, "view10"}, {*this, "view11"},
		{*this, "view12"}, {*this, "view13"}, {*this, "view14"}, {*this, "view15"} }
{
}


void kim1bus_retrospy_ramrom_device::device_start()
{
	m_ram = std::make_unique<u8[]>(0x10000);

	for (unsigned page = 0; page < 16; page++)
	{
		const offs_t start = page * 0x1000;
		const offs_t end = start + 0x0fff;

		install_view(start, end, m_view[page]);
		m_view[page][0].install_ram(start, end, &m_ram[start]);

		m_rom_bank[page]->configure_entries(0, 8, &m_rom[start], 0x10000);
		m_view[page][1].install_read_bank(start, end, m_rom_bank[page]);
	}

	save_pointer(NAME(m_ram), 0x10000);
}


void kim1bus_retrospy_ramrom_device::device_reset()
{
	const u32 pages = m_pages->read();
	const u8 rom_bank = m_rom_select->read() & 0x07;

	for (unsigned page = 0; page < 16; page++)
	{
		m_rom_bank[page]->set_entry(rom_bank);

		switch ((pages >> (page * 2)) & 0x03)
		{
		case 1: // RAM
			m_view[page].select(0);
			break;

		case 2: // ROM
			m_view[page].select(1);
			break;

		default: // No response from the board
			m_view[page].disable();
			break;
		}
	}
}


ioport_constructor kim1bus_retrospy_ramrom_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(retrospy_ramrom);
}


const tiny_rom_entry *kim1bus_retrospy_ramrom_device::device_rom_region() const
{
	return ROM_NAME(retrospy_ramrom);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(
		KIM1BUS_RETROSPY_RAMROM,
		device_kim1bus_card_interface,
		kim1bus_retrospy_ramrom_device,
		"retrospy_ramrom",
		"RetroSpy SYM-1/AIM-65 RAM/ROM Board")
