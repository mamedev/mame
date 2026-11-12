// license:BSD-3-Clause
// copyright-holders:Tim Douglas
/***************************************************************************

    Sonic Systems Comm Slot Ethernet, SMC version (BD-070 Rev A)

    An SMSC LAN91C96 with a 20 MHz crystal, and a PROM holding the station
    address. The chip's eight 16-bit registers per bank are repeated
    through the I/O window, byte-swapped. The PROM's six bytes are
    bit-reversed and repeated every 32 bytes, and 0 is read from the rest
    of each block. The chip's address registers are initially clear.

***************************************************************************/

#include "emu.h"
#include "sonicsmc.h"

#include "machine/smc91c9x.h"


namespace {

class mac_comm_sonicsmc_device : public device_t, public device_mac_comm_card_interface
{
public:
	mac_comm_sonicsmc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void install(address_space &space, offs_t base) override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;

	u8 prom_r(offs_t offset);
	u16 smc_r(offs_t offset, u16 mem_mask);
	void smc_w(offs_t offset, u16 data, u16 mem_mask);

	required_device<smc91c96_device> m_smc;

	u8 m_prom[6];
};


mac_comm_sonicsmc_device::mac_comm_sonicsmc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MAC_COMM_SONICSMC, tag, owner, clock)
	, device_mac_comm_card_interface(mconfig, *this)
	, m_smc(*this, "smc91c96")
{
}

void mac_comm_sonicsmc_device::device_add_mconfig(machine_config &config)
{
	SMC91C96(config, m_smc, 20_MHz_XTAL);
	m_smc->irq_handler().set(FUNC(mac_comm_sonicsmc_device::irq_w));
}

void mac_comm_sonicsmc_device::device_start()
{
	u32 const num = machine().rand();
	m_prom[0] = 0x00;
	m_prom[1] = 0x40;
	m_prom[2] = 0x10;
	m_prom[3] = u8(num >> 16);
	m_prom[4] = u8(num >> 8);
	m_prom[5] = u8(num);
	m_smc->set_mac(m_prom);

	save_item(NAME(m_prom));
}

void mac_comm_sonicsmc_device::map(address_map &map)
{
	map(0x0000, 0x001f).mirror(0x1fe0).r(FUNC(mac_comm_sonicsmc_device::prom_r));
	map(0x2000, 0x200f).mirror(0x1ff0).rw(FUNC(mac_comm_sonicsmc_device::smc_r), FUNC(mac_comm_sonicsmc_device::smc_w));
}

void mac_comm_sonicsmc_device::install(address_space &space, offs_t base)
{
	space.install_device(base, base + 0x3fff, *this, &mac_comm_sonicsmc_device::map);
}

u8 mac_comm_sonicsmc_device::prom_r(offs_t offset)
{
	return (offset < 6) ? bitswap<8>(m_prom[offset], 0, 1, 2, 3, 4, 5, 6, 7) : 0;
}

u16 mac_comm_sonicsmc_device::smc_r(offs_t offset, u16 mem_mask)
{
	return swapendian_int16(m_smc->read(offset, swapendian_int16(mem_mask)));
}

void mac_comm_sonicsmc_device::smc_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_smc->write(offset, swapendian_int16(data), swapendian_int16(mem_mask));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MAC_COMM_SONICSMC, device_mac_comm_card_interface, mac_comm_sonicsmc_device, "maccomm_sonicsmc", "Sonic Systems Comm Slot Ethernet (SMC)")
