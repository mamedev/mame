// license:BSD-3-Clause
// copyright-holders:R. Belmont

/*
    mboardd.cpp - Mockingboard D emulation
    by R. Belmont

    The Apple IIc didn't have slots, so Sweet Micro Systems came up with this, a Mockingboard
    in a little box that attached to the IIc's modem port.  There's nothing special about the
    IIc though - with a proper cable, anything that speaks 9600 8N1 RS-232 could drive this
    device.

    A decent disassembly of the 6803 firmware is available from the Apple II Documentation Project:
    https://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/Audio/Sweet%20Microsystems%20Mockingboard/Source%20Code/

*/

#include "emu.h"
#include "mboardd.h"

#include "cpu/m6800/m6801.h"
#include "sound/ay8910.h"

#include "speaker.h"


namespace {

ROM_START(mboardd)
	ROM_REGION(0x800, "mbcpu", 0)
	ROM_LOAD("mockingboard d rom.bin", 0x000000, 0x000800, CRC(277b1813) SHA1(e8d20f4b59fe867ff76434d35a14d2cbdc8533e3))
ROM_END

class mockingboard_d_device : public device_t, public device_rs232_port_interface
{
public:
	mockingboard_d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_txd(int state) override;

	required_device<m6803_cpu_device> m_cpu;
	required_device<ay8913_device> m_ay1;
	required_device<ay8913_device> m_ay2;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u8 p2_r() { return m_rx_state<<3; };
	void p1_w(u8 data);

	void ser_tx_w(int state) { output_rxd(state); }

	void c000_w(u8 data) { m_c000_latch = data; };

	void m6803_mem(address_map &map) ATTR_COLD;
	int m_rx_state;
	u8 m_c000_latch;
};

mockingboard_d_device::mockingboard_d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SERIAL_MOCKINGBOARD_D, tag, owner, clock)
	, device_rs232_port_interface(mconfig, *this)
	, m_cpu(*this, "mbdcpu")
	, m_ay1(*this, "ay1")
	, m_ay2(*this, "ay2")
{
}

void mockingboard_d_device::device_add_mconfig(machine_config &config)
{
	M6803(config, m_cpu, 4.9152_MHz_XTAL);  // value reverse-engineered from 9600 baud SCI rate needed
	m_cpu->set_addrmap(AS_PROGRAM, &mockingboard_d_device::m6803_mem);
	m_cpu->in_p2_cb().set(FUNC(mockingboard_d_device::p2_r));
	m_cpu->out_p1_cb().set(FUNC(mockingboard_d_device::p1_w));
	m_cpu->out_ser_tx_cb().set(FUNC(mockingboard_d_device::ser_tx_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	AY8913(config, m_ay1, 1022727);
	m_ay1->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	AY8913(config, m_ay2, 1022727);
	m_ay2->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
}

const tiny_rom_entry *mockingboard_d_device::device_rom_region() const
{
	return ROM_NAME(mboardd);
}

void mockingboard_d_device::device_start()
{
}

void mockingboard_d_device::device_reset()
{
	output_dcd(0);
	output_dsr(0);
	output_cts(0);
}

void mockingboard_d_device::m6803_mem(address_map &map)
{
	map(0x8000, 0x83ff).ram();
	// SSI-263 at a000/a001
	map(0xc000, 0xc000).w(FUNC(mockingboard_d_device::c000_w));
	map(0xf800, 0xffff).rom().region("mbcpu", 0);
}

void mockingboard_d_device::input_txd(int state)
{
	m_rx_state = (state & 1);
}

void mockingboard_d_device::p1_w(u8 data)
{
	if (BIT(data, 3))
	{
		m_ay2->address_w(m_c000_latch);
		//printf("%02x to AY2 address\n", m_c000_latch);
	}
	else if (BIT(data, 2))
	{
		m_ay2->data_w(m_c000_latch);
		//printf("%02x to AY2 data\n", m_c000_latch);
	}
	else if (BIT(data, 1))
	{
		m_ay1->address_w(m_c000_latch);
		//printf("%02x to AY1 address\n", m_c000_latch);
	}
	else if (BIT(data, 0))
	{
		m_ay1->data_w(m_c000_latch);
		//printf("%02x to AY1 data\n", m_c000_latch);
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(SERIAL_MOCKINGBOARD_D, device_rs232_port_interface, mockingboard_d_device, "mockingboardd", "Sweet Micro Systems Mockingboard D")
