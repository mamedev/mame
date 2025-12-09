// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Newer Technology Ethernet MicroDock emulation
    By R. Belmont

    This is an Ethernet dock for the PowerBook Duo series.  It uses
    an SMC91C94 Ethernet controller.  MAC address is the first 6 bytes
    of the declaration ROM.

***************************************************************************/

#include "emu.h"
#include "ethernetudock.h"

#include "machine/smc91c9x.h"

namespace {

class etherudock_device : public device_t, public device_pwrbkduo_card_interface
{
public:
	// construction/destruction
	etherudock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	etherudock_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	required_device<smc91c94_device> m_smc919x;
	required_region_ptr<u32> m_rom;

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void ethernetmicrodock_map(address_map &map) ATTR_COLD;

	u16 smc_r(offs_t offset, u16 mem_mask);
	void smc_w(offs_t offset, u16 data, u16 mem_mask);

	void irq_w(int state);
};

void etherudock_device::ethernetmicrodock_map(address_map &map)
{
	map(0x0000'0300, 0x0000'030f).rw(FUNC(etherudock_device::smc_r), FUNC(etherudock_device::smc_w)).mirror(0x00f0'0000);
}

ROM_START( etherudock )
	ROM_REGION32_BE(0x800, "dock", 0)
	ROM_LOAD16_BYTE("newertechethernetmicrodock.rom", 0x000000, 0x000400, CRC(0fd930f0) SHA1(85474adc62de1bdca6ce7bf694a0158d467113cc))
ROM_END

const tiny_rom_entry *etherudock_device::device_rom_region() const
{
	return ROM_NAME(etherudock);
}

void etherudock_device::device_add_mconfig(machine_config &config)
{
	SMC91C94(config, m_smc919x, 20_MHz_XTAL);
	m_smc919x->irq_handler().set(FUNC(etherudock_device::irq_w));
}

etherudock_device::etherudock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	etherudock_device(mconfig, DUODOCK_ETHERUDOCK, tag, owner, clock)
{
}

etherudock_device::etherudock_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_pwrbkduo_card_interface(mconfig, *this),
	m_smc919x(*this, "smc91c94"),
	m_rom(*this, "dock")
{
}

void etherudock_device::device_start()
{
	pwrbkduo().install_bank(0xfefff800, 0xfeffffff, m_rom);
	pwrbkduo().install_map(*this, 0xfe000000, 0xfefdffff, &etherudock_device::ethernetmicrodock_map);
}

u16 etherudock_device::smc_r(offs_t offset, u16 mem_mask)
{
	return swapendian_int16(m_smc919x->read(offset, swapendian_int16(mem_mask)));
}

void etherudock_device::smc_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_smc919x->write(offset, swapendian_int16(data), swapendian_int16(mem_mask));
}

void etherudock_device::irq_w(int state)
{
	pwrbkduo().dock_irq_w(state);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(DUODOCK_ETHERUDOCK, device_nubus_card_interface, etherudock_device, "ethudock", "Newer Technology Ethernet MicroDock")

