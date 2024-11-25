// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Micro-Robotics Scorpion Intelligent Controller

    Specification:
    - 6303 CMOS micro-processor, 1.23 MHz operation
    - 24K RAM with battery backup
    - 32K system ROM
    - clock calendar

    Programming requirements:
      A computer with a serial port capable of running at 9600 baud, supporting a
      terminal emulator preferably conforming to VT52 or VT100 standards.

    Note: Use with BBC Micro with Scorpion Terminal Emulator ROM.

    TODO:
    - verify serial handshaking
    - LCD display
    - 20 key keypad

******************************************************************************/

#include "emu.h"
#include "scorpion.h"

#include "cpu/m6800/m6801.h"
#include "machine/nvram.h"
#include "machine/pcf8573.h"


#define LOG_PORT2 (1U << 1)
#define LOG_PORT5 (1U << 2)
#define LOG_PORT6 (1U << 3)
#define LOG_PORT7 (1U << 4)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

class scorpion_ic_device : public device_t, public device_rs232_port_interface
{
public:
	scorpion_ic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, SCORPION_IC, tag, owner, clock)
		, device_rs232_port_interface(mconfig, *this)
		, m_maincpu(*this, "maincpu")
		, m_rombank(*this, "rombank")
		, m_rtc(*this, "rtc")
		, m_txd(0)
		, m_rts(0)
	{
	}

	virtual void input_txd(int state) override { m_txd = state; }
	virtual void input_rts(int state) override { m_rts = state; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<hd6301x_cpu_device> m_maincpu;
	required_memory_bank m_rombank;
	required_device<pcf8573_device> m_rtc;

	void scorpion_map(address_map &map) ATTR_COLD;

	uint8_t port2_r();
	uint8_t port5_r();
	uint8_t port6_r();
	void port6_w(uint8_t data);
	void port7_w(uint8_t data);

	int m_txd;
	int m_rts;
};


void scorpion_ic_device::device_start()
{
	m_rombank->configure_entries(0, 2, memregion("sys_rom")->base(), 0x4000);
}

void scorpion_ic_device::device_reset()
{
	m_rombank->set_entry(0);

	output_cts(0);
}


ROM_START( scorpion )
	ROM_REGION(0x8000, "sys_rom", 0)
	ROM_SYSTEM_BIOS(0, "61203", "Version 61203 (3rd Dec 1986)")
	ROMX_LOAD("scorpion_61203.bin", 0x0000, 0x8000, CRC(4ff495d1) SHA1(91c273c94cd0dbca655e192dd01ac6dc3a4f60a8), ROM_BIOS(0))

	ROM_REGION(0x4000, "exp_rom", ROMREGION_ERASE00)
ROM_END

const tiny_rom_entry *scorpion_ic_device::device_rom_region() const
{
	return ROM_NAME( scorpion );
}


void scorpion_ic_device::scorpion_map(address_map &map)
{
	map(0x07f0, 0x07f0).nopr(); // ??
	map(0x0bf0, 0x0bf0).nopr(); // ??
	map(0x0ef0, 0x0ef0).nopr(); // ??
	map(0x0fb0, 0x0fb0).nopr(); // ??
	map(0x0fe0, 0x0fe0).nopr(); // ??
	map(0x2000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xbfff).rom().region("exp_rom", 0);
	map(0xc000, 0xffff).bankr("rombank").nopw();
}


void scorpion_ic_device::device_add_mconfig(machine_config &config)
{
	HD6303X(config, m_maincpu, 4.9152_MHz_XTAL); // TODO: verify clock
	m_maincpu->set_addrmap(AS_PROGRAM, &scorpion_ic_device::scorpion_map);
	m_maincpu->in_p2_cb().set(FUNC(scorpion_ic_device::port2_r));
	//m_maincpu->out_p2_cb().set([this](uint8_t data) { logerror("%s port2_w: %02x\n", machine().describe_context(), data); } );
	m_maincpu->in_p5_cb().set(FUNC(scorpion_ic_device::port5_r));
	m_maincpu->in_p6_cb().set(FUNC(scorpion_ic_device::port6_r));
	m_maincpu->out_p6_cb().set(FUNC(scorpion_ic_device::port6_w));
	m_maincpu->out_p7_cb().set(FUNC(scorpion_ic_device::port7_w));
	m_maincpu->out_ser_tx_cb().set(FUNC(scorpion_ic_device::output_rxd));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	PCF8573(config, m_rtc, 32.768_kHz_XTAL);
}


uint8_t scorpion_ic_device::port2_r()
{
	uint8_t data = 0xf7;

	data |= m_txd << 3;

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_PORT2, "%s port2_r: %02x\n", machine().describe_context(), data);

	return data;
}

uint8_t scorpion_ic_device::port5_r()
{
	uint8_t data = 0xef;

	data |= m_rts << 4;

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_PORT5, "%s port5_r: %02x\n", machine().describe_context(), data);

	return data;
}

uint8_t scorpion_ic_device::port6_r()
{
	uint8_t data = 0x00;

	data |= m_rtc->sda_r() << 3;

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_PORT6, "%s port6_r: %02x\n", machine().describe_context(), data);

	return data;
}

void scorpion_ic_device::port6_w(uint8_t data)
{
	LOGMASKED(LOG_PORT6, "%s port6_w: %02x\n", machine().describe_context(), data);

	m_rtc->sda_w(BIT(data, 3));
	m_rtc->scl_w(BIT(data, 4));

	m_rombank->set_entry(BIT(data, 6));
}

void scorpion_ic_device::port7_w(uint8_t data)
{
	LOGMASKED(LOG_PORT7, "%s port7_w: %02x\n", machine().describe_context(), data);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(SCORPION_IC, device_rs232_port_interface, scorpion_ic_device, "scorpion_ic", "Micro-Robotics Scorpion Intelligent Controller")
