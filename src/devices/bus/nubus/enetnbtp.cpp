// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Apple Ethernet NB Twisted Pair (820-0511-A, ASSY 630-0863)

    This card has a DP83932 SONIC and 128KiB of RAM (4x uPD41464).
    SONIC's bus mastering capability appears to be unused outside of
    the on-card RAM, making this essentially the same as the
    DP8390x cards.

***************************************************************************/

#include "emu.h"
#include "enetnbtp.h"

#include "machine/dp83932c.h"

namespace {
	class nubus_enetnbtp_device :
		public device_t,
		public device_nubus_card_interface,
		public device_memory_interface
	{
	public:
		nubus_enetnbtp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	protected:
		// device-level overrides
		virtual void device_start() override ATTR_COLD;
		virtual space_config_vector memory_space_config() const override;

		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
		virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

		required_device<dp83932c_device> m_sonic;

	private:
		void card_map(address_map &map);

		u32 ram_r(offs_t offset, u32 mem_mask);
		void ram_w(offs_t offset, u32 data, u32 mem_mask);

		u8 ethernet_mac_r(offs_t offset);

		address_space_config m_mem_config;
		u8 m_mac[6];
		std::unique_ptr<u32[]> m_ram;
	};

ROM_START( enetnbtp )
	ROM_REGION(0x8000, "declrom", 0)
	ROM_LOAD( "341-1096.bin", 0x000000, 0x008000, CRC(423f801b) SHA1(dd14bf4328d9c1ea1d2e1d441da0233e6669e919) )
ROM_END

void nubus_enetnbtp_device::device_add_mconfig(machine_config &config)
{
	DP83932C(config, m_sonic, 20_MHz_XTAL);
	m_sonic->set_bus(tag(), AS_PROGRAM);
	m_sonic->out_int_cb().set(FUNC(nubus_enetnbtp_device::slot_irq_w));
}

const tiny_rom_entry *nubus_enetnbtp_device::device_rom_region() const
{
	return ROM_NAME( enetnbtp );
}

nubus_enetnbtp_device::nubus_enetnbtp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NUBUS_ENETNBTP, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	device_memory_interface(mconfig, *this),
	m_sonic(*this, "sonic"),
	m_mem_config("memory_space", ENDIANNESS_BIG, 32, 32)
{
}

void nubus_enetnbtp_device::card_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).rw(FUNC(nubus_enetnbtp_device::ram_r), FUNC(nubus_enetnbtp_device::ram_w));
	map(0x000c0000, 0x000c00ff).m(m_sonic, FUNC(dp83932c_device::map));
	map(0x00400000, 0x0040000f).r(FUNC(nubus_enetnbtp_device::ethernet_mac_r)).umask32(0x00ff00ff);
	map(0x00c20000, 0x00c3ffff).rw(FUNC(nubus_enetnbtp_device::ram_r), FUNC(nubus_enetnbtp_device::ram_w));
}

void nubus_enetnbtp_device::device_start()
{
	m_ram = std::make_unique<u32[]>(0x20000/ sizeof(u32));

	install_declaration_rom("declrom");
	nubus().install_map(*this, &nubus_enetnbtp_device::card_map);

	// MAC PROM is stored with a bit swizzle and must match
	// Apple's assigned OUI block 08:00:07 (the address PROM is even stamped )
	const std::array<u8, 6> &MAC = m_sonic->get_mac();
	m_mac[0] = bitswap<8>(0x08, 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[1] = bitswap<8>(0x00, 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[2] = bitswap<8>(0x07, 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[3] = bitswap<8>(MAC[3], 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[4] = bitswap<8>(MAC[4], 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[5] = bitswap<8>(MAC[5], 0, 1, 2, 3, 7, 6, 5, 4);
	m_sonic->set_mac(&m_mac[0]);

	const u32 slot_base = get_slotspace();
	this->space(AS_PROGRAM).install_read_handler(slot_base, slot_base + 0x1'ffff, emu::rw_delegate(*this, FUNC(nubus_enetnbtp_device::ram_r)));
	this->space(AS_PROGRAM).install_write_handler(slot_base, slot_base + 0x1'ffff, emu::rw_delegate(*this, FUNC(nubus_enetnbtp_device::ram_w)));

	save_pointer(NAME(m_ram), 0x20000 / sizeof(u32));
}

device_memory_interface::space_config_vector nubus_enetnbtp_device::memory_space_config() const
{
	return space_config_vector { std::make_pair(AS_PROGRAM, &m_mem_config) };
}

u32 nubus_enetnbtp_device::ram_r(offs_t offset, u32 mem_mask)
{
	return m_ram[offset];
}

void nubus_enetnbtp_device::ram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_ram[offset]);
}

u8 nubus_enetnbtp_device::ethernet_mac_r(offs_t offset)
{
	if (offset < 6)
	{
		return m_mac[offset];
	}
	else if (offset == 7)
	{
		u8 xor_total = 0;

		for (int i = 0; i < 6; i++)
		{
			xor_total ^= (u8)m_mac[i];
		}

		return xor_total ^ 0xff;
	}

	return 0;
}


} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_ENETNBTP, device_nubus_card_interface, nubus_enetnbtp_device, "enetnbtp", "Apple Ethernet NB Twisted-Pair Card")
