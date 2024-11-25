// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    k1016_16k.cpp

    MTU K-1016 16K RAM expansion for the KIM-1

*********************************************************************/

#include "emu.h"
#include "k1016_16k.h"

namespace {

class kim1bus_k1016_device:
		public device_t,
		public device_kim1bus_card_interface
{
public:
	// construction/destruction
	kim1bus_k1016_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	std::unique_ptr<u8[]> m_ram;
};

kim1bus_k1016_device::kim1bus_k1016_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KIM1BUS_K1016, tag, owner, clock)
	, device_kim1bus_card_interface(mconfig, *this)
{
}

void kim1bus_k1016_device::device_start()
{
	m_ram = std::make_unique<u8[]>(0x4000);

	install_bank(0x2000, 0x5fff, &m_ram[0]);

	save_pointer(NAME(m_ram), 0x4000);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(KIM1BUS_K1016, device_kim1bus_card_interface, kim1bus_k1016_device, "mtu_k1016", "MTU K-1016 16K RAM card")
