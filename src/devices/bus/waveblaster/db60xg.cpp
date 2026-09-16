// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// Yamaha DB60XG / NEC XR385

// PCB name: XR712, XR385 (NEC)

// It's a revision of the db50xg which brings the half-assed analog
// input back and has a system program upgrade.  The NEC variant is in fact identical

#include "emu.h"
#include "db60xg.h"

#include "cpu/h8/h83002.h"
#include "sound/swp00.h"


namespace {

class db60xg_device : public device_t, public device_waveblaster_interface
{
public:
	db60xg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~db60xg_device();

	virtual void midi_rx(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<h83002_device> m_cpu;
	required_device<swp00_device> m_swp00;

	void map(address_map &map) ATTR_COLD;
};

db60xg_device::db60xg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DB60XG, tag, owner, clock),
	device_waveblaster_interface(mconfig, *this),
	m_cpu(*this, "cpu"),
	m_swp00(*this, "swp00")
{
}

db60xg_device::~db60xg_device()
{
}

void db60xg_device::midi_rx(int state)
{
	m_cpu->sci_rx_w<0>(state);
}

void db60xg_device::map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("cpu", 0);
	map(0x400000, 0x4007ff).m(m_swp00, FUNC(swp00_device::map));
	map(0x600000, 0x607fff).ram();
}

void db60xg_device::device_add_mconfig(machine_config &config)
{
	H83002(config, m_cpu, 12_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &db60xg_device::map);
	m_cpu->write_sci_tx<0>().set([this] (int state) { m_connector->do_midi_tx(state); });

	SWP00(config, m_swp00);
	m_swp00->add_route(0, DEVICE_SELF_OWNER, 1.0, 0);
	m_swp00->add_route(1, DEVICE_SELF_OWNER, 1.0, 1);
}

ROM_START( db60xg )
	ROM_REGION( 0x80000, "cpu", 0 )
	ROM_LOAD16_WORD_SWAP( "xr560b0.ic7", 0x000000, 0x080000, CRC(d9cad989) SHA1(d17e4ac0fc0e50a06d9d505bc87af155356436eb) )

	ROM_REGION( 0x400000, "swp00", 0 )
	ROM_LOAD( "xq730b0.ic9", 0x000000, 0x200000, CRC(d4adbc7e) SHA1(32f653c7644d060f5a6d63a435ae3a7412386d92) )
	ROM_LOAD( "xq731b0.ic10", 0x200000, 0x200000, CRC(7b68f475) SHA1(adf68689b4842ec5bc9b0ea1bb99cf66d2dec4de) )
ROM_END

void db60xg_device::device_start()
{
}

void db60xg_device::device_reset()
{
	// Active-low, wired to gnd
	m_cpu->set_input_line(0, ASSERT_LINE);
}

const tiny_rom_entry *db60xg_device::device_rom_region() const
{
	return ROM_NAME(db60xg);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(DB60XG, device_waveblaster_interface, db60xg_device, "db60xg", "Yamaha DB60XG")
