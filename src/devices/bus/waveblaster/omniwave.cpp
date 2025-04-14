// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// Samsung Omniwave

#include "emu.h"
#include "omniwave.h"

#include "sound/ks0164.h"


namespace {

class omniwave_device : public device_t, public device_waveblaster_interface
{
public:
	omniwave_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~omniwave_device();

	virtual void midi_rx(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ks0164_device> m_ks0164;
};

omniwave_device::omniwave_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, OMNIWAVE, tag, owner, clock),
	device_waveblaster_interface(mconfig, *this),
	m_ks0164(*this, "ks0164")
{
}

omniwave_device::~omniwave_device()
{
}

void omniwave_device::midi_rx(int state)
{
	m_ks0164->midi_rx(state);
}

void omniwave_device::device_add_mconfig(machine_config &config)
{
	KS0164(config, m_ks0164, 16.9344_MHz_XTAL);
	m_ks0164->add_route(0, DEVICE_SELF_OWNER, 1.0, 0);
	m_ks0164->add_route(1, DEVICE_SELF_OWNER, 1.0, 1);
	m_ks0164->midi_tx().set([this] (int state) { m_connector->do_midi_tx(state); });
}

ROM_START( omniwave )
	ROM_REGION16_BE( 0x100000, "ks0164", 0)
	ROM_LOAD16_WORD_SWAP("ks0174-1m04.bin", 0, 0x100000, CRC(3cabaa2f) SHA1(1e894c0345eaf0ea713f36a75b065f7ee419c63c))
ROM_END

void omniwave_device::device_start()
{
}

const tiny_rom_entry *omniwave_device::device_rom_region() const
{
	return ROM_NAME(omniwave);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(OMNIWAVE, device_waveblaster_interface, omniwave_device, "omniwave", "Samsung Omniwave")
