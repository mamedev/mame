// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// Yamaha PLG150-AP

// Acoustic Piano, a plugin dedicated to a high-quality piano sound

// Dual SWX00

#include "emu.h"
#include "plg150-ap.h"

#include "cpu/h8/swx00.h"

namespace {

class plg150_ap_device : public device_t, public device_plg1x0_interface
{
public:
	plg150_ap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~plg150_ap_device();

	virtual void midi_rx(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<swx00_device> m_cpu;

	void map(address_map &map) ATTR_COLD;
};

plg150_ap_device::plg150_ap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PLG150_AP, tag, owner, clock),
	device_plg1x0_interface(mconfig, *this),
	m_cpu(*this, "cpu")
{
}

plg150_ap_device::~plg150_ap_device()
{
}

void plg150_ap_device::midi_rx(int state)
{
	logerror("%s rx=%d\n", machine().time().to_string(), state);
	m_cpu->sci_rx_w<1>(state);
}

void plg150_ap_device::map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("cpu", 0);
	map(0x200000, 0x207fff).ram();
}

void plg150_ap_device::device_add_mconfig(machine_config &config)
{
	SWX00(config, m_cpu, 8.4672_MHz_XTAL*2, 1);
	m_cpu->set_addrmap(AS_PROGRAM, &plg150_ap_device::map);
	m_cpu->write_sci_tx<1>().set([this] (int state) { m_connector->do_midi_tx(state); });
	m_cpu->sci_set_external_clock_period(0, attotime::from_hz(500000));
	m_cpu->sci_set_external_clock_period(1, attotime::from_hz(500000));
}

ROM_START( plg150_ap )
	ROM_REGION( 0x80000, "cpu", 0 )
	ROM_LOAD16_WORD_SWAP( "x5757b0.ic03", 0x00000, 0x80000, CRC(5383b363) SHA1(b8e8e0673439c80dc9aecd6638f1c20454c82080) )

	ROM_REGION( 0x1000000, "swx00", 0 )
	ROM_LOAD( "x575810.ic09", 0x000000, 0x800000, CRC(273b4574) SHA1(b33d67f7fe3a020c19133cc52bd6a228227941c5) )
	ROM_LOAD( "x575910.ic11", 0x800000, 0x800000, CRC(32a37ef3) SHA1(c0b5dfb1fbb5a3ddd33e6303039fbc197478abd2) )
ROM_END

void plg150_ap_device::device_start()
{
}

void plg150_ap_device::device_reset()
{
}

const tiny_rom_entry *plg150_ap_device::device_rom_region() const
{
	return ROM_NAME(plg150_ap);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(PLG150_AP, device_plg1x0_interface, plg150_ap_device, "plg150_ap", "Yamaha PLG150-AP")
