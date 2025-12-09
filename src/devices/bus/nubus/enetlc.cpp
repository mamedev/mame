// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Apple Ethernet LC Card (820-0443-B, ASSY 670-4443)
    Apple Ethernet LC Twisted Pair Card (820-0532-B, ASSY 630-0891-B)

    Both of these cards are original LC slot cards with a DP83932 SONIC
    Ethernet controller, a declaration ROM, and a MAC address PROM.
    Unlike the NuBus cards, these cards use SONIC's bus mastering DMA
    to go directly to and from the LC's RAM.

***************************************************************************/

#include "emu.h"
#include "enetlc.h"

#include "machine/dp83932c.h"

namespace {
class nubus_enetlctp_device :
	public device_t,
	public device_nubus_card_interface
{
public:
	nubus_enetlctp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	nubus_enetlctp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	required_device<dp83932c_device> m_sonic;

private:
	void card_map(address_map &map);
	u16 ethernet_mac_r(offs_t offset, u16 mem_mask);

	u8 m_mac[6];
	std::string m_fulltag;
};

class nubus_enetlc_device : public nubus_enetlctp_device
{
public:
	nubus_enetlc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

ROM_START( enetlctp )
	ROM_REGION(0x8000, "declrom", 0)
	ROM_LOAD( "341-0740.bin", 0x000000, 0x008000, CRC(9d47245c) SHA1(447ce6830600f47f63fdc21dd132539decb76001) )
ROM_END

void nubus_enetlctp_device::device_add_mconfig(machine_config &config)
{
	DP83932C(config, m_sonic, 20_MHz_XTAL);
	if (((nubus_slot_device *)owner())->get_nubus_bustag() != nullptr)
	{
		m_fulltag = string_format(":%s", ((nubus_slot_device *)owner())->get_nubus_bustag());
		m_sonic->set_bus(m_fulltag.c_str(), AS_DATA);
	}
	m_sonic->out_int_cb().set(FUNC(nubus_enetlctp_device::slot_irq_w));
}

const tiny_rom_entry *nubus_enetlctp_device::device_rom_region() const
{
	return ROM_NAME( enetlctp );
}

ROM_START( enetlc )
	ROM_REGION(0x8000, "declrom", 0)
	ROM_DEFAULT_BIOS("ver11")
	ROM_SYSTEM_BIOS(0, "ver11", "Ver. 1.1 (1992)")
	ROM_SYSTEM_BIOS(1, "ver10", "Ver. 1.0 (1990)")

	// 341-0470 v1.1, Copyright (C) 1990-92 Apple Computer, Inc. All Rights Reserved.
	ROMX_LOAD( "341-0470.bin", 0x000000, 0x008000, CRC(12a05575) SHA1(574e7f15f3d074e7bf6a9687ef59d3abebfd865c), ROM_BIOS(0) )
	// 341-0842 v1.0, Copyright (C) 1990 Apple Computer, Inc.
	ROMX_LOAD( "341-0842.bin", 0x000000, 0x008000, CRC(92310cb3) SHA1(ba4fd87765104701d5e6bf7962ed3a453e3c5711), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *nubus_enetlc_device::device_rom_region() const
{
	return ROM_NAME(enetlc);
}

nubus_enetlctp_device::nubus_enetlctp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_enetlctp_device(mconfig, PDSLC_ENETLCTP, tag, owner, clock)
{
}

nubus_enetlctp_device::nubus_enetlctp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_sonic(*this, "sonic")
{
}

nubus_enetlc_device::nubus_enetlc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_enetlctp_device(mconfig, PDSLC_ENETLC, tag, owner, clock)
{
}

void nubus_enetlctp_device::card_map(address_map &map)
{
	// LC and LC II mappings
	map(0x8000'0000, 0x8000'01ff).m(m_sonic, FUNC(dp83932c_device::map)).umask32(0xffff0000);
	map(0x8004'0000, 0x8004'01ff).r(FUNC(nubus_enetlctp_device::ethernet_mac_r));
	map(0x8040'0000, 0x8040'01ff).r(FUNC(nubus_enetlctp_device::ethernet_mac_r));

	// LC III and LC5xx mappings
	map(0xfe00'0000, 0xfe00'01ff).m(m_sonic, FUNC(dp83932c_device::map)).umask32(0xffff0000);
	map(0xfe04'0000, 0xfe04'0007).r(FUNC(nubus_enetlctp_device::ethernet_mac_r));
	map(0xfe40'0000, 0xfe40'0007).r(FUNC(nubus_enetlctp_device::ethernet_mac_r));
}

void nubus_enetlctp_device::device_start()
{
	set_pds_slot(0xe);

	nubus().install_lcpds_map(*this, &nubus_enetlctp_device::card_map);
	install_declaration_rom("declrom");

	// MAC PROM is stored with a bit swizzle (IEEE standard according to SuperMario source?)
	// and must match Apple's assigned OUI block 08:00:07
	const std::array<u8, 6> &MAC = m_sonic->get_mac();
	m_mac[0] = bitswap<8>(0x08, 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[1] = bitswap<8>(0x00, 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[2] = bitswap<8>(0x07, 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[3] = bitswap<8>(MAC[3], 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[4] = bitswap<8>(MAC[4], 0, 1, 2, 3, 7, 6, 5, 4);
	m_mac[5] = bitswap<8>(MAC[5], 0, 1, 2, 3, 7, 6, 5, 4);
	m_sonic->set_mac(&m_mac[0]);
}

u16 nubus_enetlctp_device::ethernet_mac_r(offs_t offset, u16 mem_mask)
{
	offset <<= 1;
	if (mem_mask == 0x00ff)
	{
		offset++;
	}

	// A word-wide read of the MAC address area is expected to return this magic value.  Some kind of protection?
	if (mem_mask == 0xffff)
	{
		return 0x0028;
	}

	if (offset < 6)
	{
		return m_mac[offset] | m_mac[offset] << 8;
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

DEFINE_DEVICE_TYPE_PRIVATE(PDSLC_ENETLC, device_nubus_card_interface, nubus_enetlc_device, "enetlc", "Apple Ethernet LC Card")
DEFINE_DEVICE_TYPE_PRIVATE(PDSLC_ENETLCTP, device_nubus_card_interface, nubus_enetlctp_device, "enetlctp", "Apple Ethernet LC Twisted-Pair Card")
