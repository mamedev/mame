// license:BSD-3-Clause
// copyright-holders: Devin Acker
/*
    Casio WG-130

    This is the daughterboard version of the GZ-30M and GZ-70SP modules, using the same ROM.
*/
#include "emu.h"
#include "wg130.h"

#include "cpu/h8/gt913.h"


namespace {

class wg130_device : public device_t, public device_waveblaster_interface
{
public:
	wg130_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~wg130_device();

	virtual void midi_rx(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<gt913_device> m_gt913;

	void map(address_map &map) ATTR_COLD;
};

INPUT_PORTS_START(wg130)
	PORT_START("gt913:kbd:FI0")
	PORT_START("gt913:kbd:FI1")
	PORT_START("gt913:kbd:FI2")
	PORT_START("gt913:kbd:FI3")
	PORT_START("gt913:kbd:FI4")
	PORT_START("gt913:kbd:FI5")
	PORT_START("gt913:kbd:FI6")
	PORT_START("gt913:kbd:FI7")
	PORT_START("gt913:kbd:FI8")
	PORT_START("gt913:kbd:FI9")
	PORT_START("gt913:kbd:FI10")
	PORT_START("gt913:kbd:KI0")
	PORT_START("gt913:kbd:KI1")
	PORT_START("gt913:kbd:KI2")
INPUT_PORTS_END

wg130_device::wg130_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WG130, tag, owner, clock),
	device_waveblaster_interface(mconfig, *this),
	m_gt913(*this, "gt913")
{
}

wg130_device::~wg130_device()
{
}

void wg130_device::midi_rx(int state)
{
	m_gt913->sci_rx_w<1>(state);
}

void wg130_device::map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("gt913", 0);
	map(0x300000, 0x301fff).ram().mirror(0x07e000);
	map(0x380000, 0x380003).noprw(); // DSP is mapped here, but not actually present
}

void wg130_device::device_add_mconfig(machine_config &config)
{
	GT913(config, m_gt913, 30_MHz_XTAL / 2);
	m_gt913->set_addrmap(AS_DATA, &wg130_device::map);
	m_gt913->add_route(0, DEVICE_SELF_OWNER, 1.0, AUTO_ALLOC_INPUT, 0);
	m_gt913->add_route(1, DEVICE_SELF_OWNER, 1.0, AUTO_ALLOC_INPUT, 1);
	m_gt913->read_port1().set_constant(0xff);
	m_gt913->write_port1().set_nop();
	m_gt913->read_port2().set_constant(0xff);
	m_gt913->write_port2().set_nop();
	m_gt913->write_sci_tx<1>().set([this] (int state) { m_connector->do_midi_tx(state); });
}

ROM_START( wg130 )
	ROM_REGION(0x200000, "gt913", 0)
	ROM_LOAD("romsxgm.bin", 0x000000, 0x200000, CRC(c392cf89) SHA1(93ebe213ea7a085c67d88974ed39ac3e9bf8059b)) // from the SW-10 softsynth
ROM_END

void wg130_device::device_start()
{
	/*
	the version of this ROM bundled with the SW-10 softsynth has little endian samples, so byteswap them
	(and stop at the end of sample data, not the end of the whole ROM, otherwise the ROM test fails)
	*/
	uint16_t *const rom = reinterpret_cast<uint16_t *>(memregion("gt913")->base());
	for (uint32_t addr = 0x2f000 >> 1; addr < 0x1fe8c2 >> 1; addr++)
		rom[addr] = swapendian_int16(rom[addr]);
}

const tiny_rom_entry *wg130_device::device_rom_region() const
{
	return ROM_NAME(wg130);
}

ioport_constructor wg130_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(wg130);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(WG130, device_waveblaster_interface, wg130_device, "wg130", "Casio WG-130")
