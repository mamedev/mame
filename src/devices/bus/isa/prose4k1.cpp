// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 Speech Plus Prose 4001 speech synthesiser

 Full-length ISA card.  Supports operation in IBM PC mode (as an ISA bus
 card) and standalone mode (controlled over a seria connection).  The
 example the ROMs were dumped from had many components unpopulated.

 Major components include:
 * Siemens 80188-N CPU
 * NEC D77P20 DSP with EPROM memory
 * Three TI 27C512 EPROMS
 * 256 kbit SRAM
 * Intel P8251A UART
 * Three banks of eight DIP switches
 */
#include "emu.h"
#include "prose4k1.h"

#include "cpu/i86/i186.h"
#include "machine/i8251.h"


namespace {

class prose4k1_device : public device_t, public device_isa8_card_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::SOUND; }

	prose4k1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		device_t(mconfig, ISA8_PROSE4001, tag, owner, clock),
		device_isa8_card_interface(mconfig, *this)
	{
	}

protected:
	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void main_map(address_map &map) ATTR_COLD;
};


ROM_START(prose4k1)
	ROM_REGION(0x3'0000, "u8", 0)
	ROM_LOAD("v3.4.1_pr4001.u3", 0x0'0000, 0x1'0000, CRC(12dac3ed) SHA1(cf8c0b9de1f00facbc5cb5dc8e2dcbb09d6ff479)) // TMS27C512, printed label
	ROM_LOAD("v3.4.1_pr4001.u2", 0x1'0000, 0x1'0000, CRC(2ee241b7) SHA1(35b81f3b4deb552511f8d8f2d0aac9100fdee49d)) // TMS27C512, printed label
	ROM_LOAD("v3.4.1_pr4001.u1", 0x2'0000, 0x1'0000, CRC(559f4950) SHA1(5c8709c82dadaea7012859c20141ef8f59d5e473)) // TMS27C512, handwritten label

	ROM_REGION32_BE(0x0800, "dsp:prg", 0) // 512*23-bit words
	ROM_LOAD("v3.12_5_04_90.prg", 0x0000, 0x0800, NO_DUMP)

	ROM_REGION16_BE(0x0400, "dsp:dat", 0) // 512*13-bit words
	ROM_LOAD("v3.12_5_04_90.dat", 0x0000, 0x0400, NO_DUMP)
ROM_END


tiny_rom_entry const *prose4k1_device::device_rom_region() const
{
	return ROM_NAME(prose4k1);
}


void prose4k1_device::device_add_mconfig(machine_config &config)
{
	I80188(config, "u8", 8'000'000).set_addrmap(AS_PROGRAM, &prose4k1_device::main_map); // TODO: measure clock

	I8251(config, "u27", 0);
}


void prose4k1_device::device_start()
{
}


void prose4k1_device::main_map(address_map &map)
{
	map(0x0'0000, 0x0'8000).ram();

	map(0xd'0000, 0xf'ffff).rom().region("u8", 0x0'0000);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ISA8_PROSE4001, device_isa8_card_interface, prose4k1_device, "isa_prose4001", "Speech Plus Prose 4001 (IBM PC mode)")
