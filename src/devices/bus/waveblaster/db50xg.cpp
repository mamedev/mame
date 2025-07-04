// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// Yamaha DB50XG

// PCB name: XQ791

// It's in large part a mu50 on a board, same swp00, same wave ROMs.
// CPU and program are different though, and obviously there's no
// LCD.  There are also no analog inputs, which were half-assed in the
// mu50 anyway.

#include "emu.h"
#include "db50xg.h"

#include "cpu/h8/h83002.h"
#include "sound/swp00.h"


namespace {

class db50xg_device : public device_t, public device_waveblaster_interface
{
public:
	db50xg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~db50xg_device();

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

db50xg_device::db50xg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DB50XG, tag, owner, clock),
	device_waveblaster_interface(mconfig, *this),
	m_cpu(*this, "cpu"),
	m_swp00(*this, "swp00")
{
}

db50xg_device::~db50xg_device()
{
}

void db50xg_device::midi_rx(int state)
{
	m_cpu->sci_rx_w<0>(state);
}

void db50xg_device::map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("cpu", 0);
	map(0x400000, 0x4007ff).m(m_swp00, FUNC(swp00_device::map));
	map(0x600000, 0x607fff).ram();
}

void db50xg_device::device_add_mconfig(machine_config &config)
{
	H83002(config, m_cpu, 12_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &db50xg_device::map);
	m_cpu->write_sci_tx<0>().set([this] (int state) { m_connector->do_midi_tx(state); });

	SWP00(config, m_swp00);
	m_swp00->add_route(0, DEVICE_SELF_OWNER, 1.0, 0);
	m_swp00->add_route(1, DEVICE_SELF_OWNER, 1.0, 1);
}

ROM_START( db50xg )
	ROM_REGION( 0x80000, "cpu", 0 )
	ROM_LOAD16_WORD_SWAP( "xr253a0.ic7", 0x000000, 0x080000, CRC(6c2a2c49) SHA1(61a80e066c7ff40dcba01b796d9faff3b9952a5b) )

	ROM_REGION( 0x400000, "swp00", 0 )
	ROM_LOAD( "xq730b0.ic9", 0x000000, 0x200000, CRC(d4adbc7e) SHA1(32f653c7644d060f5a6d63a435ae3a7412386d92) )
	ROM_LOAD( "xq731b0.ic10", 0x200000, 0x200000, CRC(7b68f475) SHA1(adf68689b4842ec5bc9b0ea1bb99cf66d2dec4de) )
ROM_END

void db50xg_device::device_start()
{
}

void db50xg_device::device_reset()
{
	// Active-low, wired to gnd
	m_cpu->set_input_line(0, ASSERT_LINE);
}

const tiny_rom_entry *db50xg_device::device_rom_region() const
{
	return ROM_NAME(db50xg);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(DB50XG, device_waveblaster_interface, db50xg_device, "db50xg", "Yamaha DB50XG")
