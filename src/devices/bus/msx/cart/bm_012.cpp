// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************

Emulation for the MSX BM-012 Midi cartridge that was sold together with Midisaurus.

TODO:
- hook up all the other signals for the CTC, SIO
- which type of SIO hookup is used? tmpz84c015af supports SIO/0, SIO/1, and SIO/2
- since the SIO signals are not hooked up, the midi in/thru/out ports are also not
  implemented yet. Channel A seems to be used for sending midi data.
- proper irq handling taking the irq priority into account is not implemented
- the hookup between 2 PIOs is educated guess work; it could be incorrect

***********************************************************************************/

#include "emu.h"
#include "bm_012.h"
#include "bus/midi/midi.h"
#include "cpu/z80/tmpz84c015.h"
#include "cpu/z80/z80.h"

namespace {

class msx_cart_bm_012_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_bm_012_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MSX_CART_BM_012, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_tmpz84c015af(*this, "tmpz84c015af")
		, m_bm012_pio(*this, "bm012_pio")
		, m_mdthru(*this, "mdthru")
	{ }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void midi_in(int state);

	void bm_012_memory_map(address_map &map) ATTR_COLD;

	required_device<tmpz84c015_device> m_tmpz84c015af;
	required_device<z80pio_device> m_bm012_pio;
	required_device<midi_port_device> m_mdthru;
};


void msx_cart_bm_012_device::bm_012_memory_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xffff).ram();
}


void msx_cart_bm_012_device::device_add_mconfig(machine_config &config)
{
	// 12MHz XTAL @ X1
	// Toshiba TMPZ84C015AF-6 (@U5) components:
	// - Z80
	// - CTC
	// - SIO
	// - PIO
	// - CGC
	// - WDT
	TMPZ84C015(config, m_tmpz84c015af, XTAL(12'000'000)/2);         /* 6 MHz */
	m_tmpz84c015af->set_addrmap(AS_PROGRAM, &msx_cart_bm_012_device::bm_012_memory_map);
	// PIO callbacks
	m_tmpz84c015af->in_pa_callback().set("bm012_pio", FUNC(z80pio_device::port_a_read));
	m_tmpz84c015af->out_pa_callback().set("bm012_pio", FUNC(z80pio_device::port_a_write));
	m_tmpz84c015af->in_pb_callback().set("bm012_pio", FUNC(z80pio_device::port_b_read));
	m_tmpz84c015af->out_pb_callback().set("bm012_pio", FUNC(z80pio_device::port_b_write));
	m_tmpz84c015af->out_brdy_callback().set("bm012_pio", FUNC(z80pio_device::strobe_b));
	// SIO callbacks
	m_tmpz84c015af->out_txda_callback().set("mdout", FUNC(midi_port_device::write_txd));

	// Sony CXK5864BSP-10L  (8KB ram)
	// Sharp LH0081A Z80A-PIO-0 - For communicating between the MSX and the TMP
	Z80PIO(config, m_bm012_pio, XTAL(3'579'545));  // ?????
	m_bm012_pio->out_pa_callback().set("tmpz84c015af", FUNC(tmpz84c015_device::pa_w));
	m_bm012_pio->in_pa_callback().set("tmpz84c015af", FUNC(tmpz84c015_device::pa_r));
	m_bm012_pio->out_pb_callback().set("tmpz84c015af", FUNC(tmpz84c015_device::pb_w));
	m_bm012_pio->in_pb_callback().set("tmpz84c015af", FUNC(tmpz84c015_device::pb_r));
	m_bm012_pio->out_brdy_callback().set("tmpz84c015af", FUNC(tmpz84c015_device::strobe_b));

	// MIDI ports
	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(FUNC(msx_cart_bm_012_device::midi_in));

	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
}


ROM_START( msx_cart_bm_012 )
	ROM_REGION(0x8000, "tmpz84c015af", 0)
	// The rom chip at U4 is a 27256, but it contains the same 8KB duplicated 4 times
	ROM_LOAD("midi_v1.00.u4", 0x0, 0x8000, CRC(840c9e74) SHA1(6d07637ad3a61b509221ed4650eed18442371010))
ROM_END


const tiny_rom_entry *msx_cart_bm_012_device::device_rom_region() const
{
	return ROM_NAME( msx_cart_bm_012 );
}


void msx_cart_bm_012_device::device_start()
{
	// Install IO read/write handlers
	io_space().install_write_handler(0x70, 0x73, emu::rw_delegate(*m_bm012_pio, FUNC(z80pio_device::write_alt)));
	io_space().install_read_handler(0x70, 0x73, emu::rw_delegate(*m_bm012_pio, FUNC(z80pio_device::read_alt)));
}


void msx_cart_bm_012_device::midi_in(int state)
{
	m_mdthru->write_txd(state);
	m_tmpz84c015af->rxb_w(state);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_BM_012, msx_cart_interface, msx_cart_bm_012_device, "msx_cart_bm_012", "MSX Cartridge - BM-012")
