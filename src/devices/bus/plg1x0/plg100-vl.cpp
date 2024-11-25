// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// Yamaha PLG100-VL

// Virtual Acoustic Synthesis, physical-modelling synthesis,  VL70-m on a plugin board

// Build around a h8 for the control and a dsp-v for the synthesis

#include "emu.h"
#include "plg100-vl.h"

#include "cpu/h8/h83002.h"
#include "sound/dspv.h"

namespace {

class plg100_vl_device : public device_t, public device_plg1x0_interface
{
public:
	plg100_vl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~plg100_vl_device();

	virtual void midi_rx(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<h83002_device> m_cpu;
	required_device<dspv_device> m_dspv;

	void map(address_map &map) ATTR_COLD;
};

plg100_vl_device::plg100_vl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PLG100_VL, tag, owner, clock),
	device_plg1x0_interface(mconfig, *this),
	m_cpu(*this, "cpu"),
	m_dspv(*this, "dspv")
{
}

plg100_vl_device::~plg100_vl_device()
{
}

void plg100_vl_device::midi_rx(int state)
{
	m_cpu->sci_rx_w<1>(state);
}

void plg100_vl_device::map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("cpu", 0);
	map(0x400000, 0x40007f).m(m_dspv, FUNC(dspv_device::map));
	map(0x200000, 0x20ffff).ram();
}

void plg100_vl_device::device_add_mconfig(machine_config &config)
{
	H83002(config, m_cpu, 10_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &plg100_vl_device::map);
	m_cpu->write_sci_tx<1>().set([this] (int state) { m_connector->do_midi_tx(state); });

	DSPV(config, m_dspv, 22.5792_MHz_XTAL);
	m_dspv->add_route(0, DEVICE_SELF_OWNER, 1.0, AUTO_ALLOC_INPUT, 0);
	m_dspv->add_route(1, DEVICE_SELF_OWNER, 1.0, AUTO_ALLOC_INPUT, 1);
}

ROM_START( plg100_vl )
	ROM_REGION( 0x100000, "cpu", 0 )
	ROM_LOAD16_WORD_SWAP( "xt47810.ic03", 0x000000, 0x100000, CRC(ef472e45) SHA1(13d54c33a4708c77de2dc1e02210da107add6ce6) )
ROM_END

void plg100_vl_device::device_start()
{
}

void plg100_vl_device::device_reset()
{
	// Active-low, wired to gnd
	m_cpu->set_input_line(0, ASSERT_LINE);
}

const tiny_rom_entry *plg100_vl_device::device_rom_region() const
{
	return ROM_NAME(plg100_vl);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(PLG100_VL, device_plg1x0_interface, plg100_vl_device, "plg100_vl", "Yamaha PLG100-VL")
